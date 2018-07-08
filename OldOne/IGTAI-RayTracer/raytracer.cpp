
#include "raytracer.h"
#include "scene_types.h"
#include "ray.h"
#include "image.h"
#include "kdtree.h"
#include <stdio.h>

#define PI (float)M_PI

/// acne_eps is a small constant used to prevent acne when computing intersection
//  or boucing (add this amount to the position before casting a new ray !
const float acne_eps = 1e-4;
const float zero_eps = 1e-5;

bool intersectPlane(Ray *ray, Intersection *intersection, Object *obj)
{

	if (obj->geom.type != PLANE)
		return false;
	Geometry geomPlane = obj->geom;

	vec3 planeNormal = geomPlane.plane.normal;

	float DdotN = dot(ray->dir, planeNormal);

	float t;

	if (abs(DdotN) <= zero_eps)
		return false;

	t = -(dot(ray->orig, planeNormal) + geomPlane.plane.dist) / DdotN;

	if (t < zero_eps || t <= ray->tmin || t >= ray->tmax)
		return false;

	intersection->position = rayAt(*ray, t);
	intersection->normal = DdotN <= zero_eps ? planeNormal : -planeNormal;
	intersection->mat = &(obj->mat);
	ray->tmax = t;

	return true;
}

bool intersectTriangle(Ray *ray, Intersection *intersection, Object *obj)
{
	//Using the Möller-Trumbore algorithm

	if (obj->geom.type != TRIANGLE)
		return false;
	Geometry geomTriangle = obj->geom;

	point3 A = geomTriangle.triangle.p1;
	point3 B = geomTriangle.triangle.p2;
	point3 C = geomTriangle.triangle.p3;

	vec3 AB = B - A;
	vec3 AC = C - A;

	vec3 pvec = cross(ray->dir, AC);

	float det = dot(AB, pvec);

	if (abs(det) < zero_eps)
		return false;

	float invDet = 1.f / det;

	vec3 tvec = ray->orig - A;

	float u = dot(tvec, pvec) * invDet;

	if (u < 0.f || u > 1.f)
		return false;

	vec3 qvec = cross(tvec, AB);
	float v = dot(ray->dir, qvec) * invDet;

	if (v < -zero_eps || u + v > 1.f)
		return false;

	float t = dot(AC, qvec) * invDet;

	if (t < ray->tmin || t > ray->tmax)
		return false;

	vec3 triangleNormal = normalize(cross(AB, AC));

	float DdotN = dot(ray->dir, triangleNormal);

	intersection->position = rayAt(*ray, t);
	intersection->normal = DdotN <= zero_eps ? triangleNormal : -triangleNormal;
	intersection->mat = &(obj->mat);
	ray->tmax = t;

	return true;
}

bool intersectSphere(Ray *ray, Intersection *intersection, Object *obj)
{
	if (obj->geom.type != SPHERE)
		return false;
	Geometry geomSphere = obj->geom;

	vec3 oMinusC = ray->orig - geomSphere.sphere.center;
	float b = 2.f * dot(ray->dir, oMinusC);
	float c = dot(oMinusC, oMinusC) - geomSphere.sphere.radius * geomSphere.sphere.radius;
	float delta = b * b - 4 * c;
	float t;

	point3 intersectionPoint;

	if (delta < -zero_eps)
		return false;

	if (delta > zero_eps)
	{
		t = (-b - sqrt(delta)) / 2.f;
		if (t < -zero_eps)
		{
			t = (-b + sqrt(delta)) / 2.f;
			if (t < -zero_eps)
				return false;
		}
	}
	else
	{
		t = (-b) / 2.f;
		if (t < -zero_eps)
			return false;
	}

	if (t < ray->tmin || t > ray->tmax)
		return false;

	intersectionPoint = rayAt(*ray, t);
	intersection->position = intersectionPoint;
	intersection->normal = normalize(intersectionPoint - geomSphere.sphere.center);
	intersection->mat = &(obj->mat);
	ray->tmax = t;

	return true;
}

bool intersectScene(const Scene *scene, Ray *ray, Intersection *intersection)
{
	bool hasIntersection = false;

	for (Object *obj : scene->objects)
	{
		switch (obj->geom.type)
		{
		case SPHERE:
			if (intersectSphere(ray, intersection, obj))
				hasIntersection = true;
			break;
		case PLANE:
			if (intersectPlane(ray, intersection, obj))
				hasIntersection = true;
			break;
		case TRIANGLE:
			if (intersectTriangle(ray, intersection, obj))
				hasIntersection = true;
			break;
		default:
			break;
		}
	}

	return hasIntersection;
}

/* --------------------------------------------------------------------------- */
/*
 *	The following functions are coded from Cook-Torrance bsdf model description and are suitable only
 *  for rough dielectrics material (RDM. Code has been validated with Mitsuba renderer)
 */

// Shadowing and masking function. Linked with the NDF. Here, Smith function, suitable for Beckmann NDF
float RDM_chiplus(float c)
{
	return (c > 0.f) ? 1.f : 0.f;
}

/** Normal Distribution Function : Beckmann
 * NdotH : Norm . Half
 */
float RDM_Beckmann(float NdotH, float alpha)
{
	if (abs(NdotH) < zero_eps)
		return 0.f;

	float cosTetaSquared = NdotH * NdotH;
	float tanTetaSquared = (1.f - cosTetaSquared) / cosTetaSquared;
	float alphaSquared = alpha * alpha;

	return RDM_chiplus(NdotH) * exp(-(tanTetaSquared / alphaSquared)) / (PI * alphaSquared * cosTetaSquared * cosTetaSquared);
}

// Fresnel term computation. Implantation of the exact computation. we can use the Schlick approximation
// LdotH : Light . Half
float RDM_Fresnel(float LdotH, float extIOR, float intIOR)
{
	float Rs, Rp;
	float sinSquared_t = (extIOR / intIOR) * (extIOR / intIOR) * (1.f - LdotH * LdotH);

	if (sinSquared_t > 1.f - zero_eps)
		return 1.f;

	float cosT = sqrt(1.f - sinSquared_t);

	Rs = (extIOR * LdotH - intIOR * cosT) * (extIOR * LdotH - intIOR * cosT) / ((extIOR * LdotH + intIOR * cosT) * (extIOR * LdotH + intIOR * cosT));
	Rp = (extIOR * cosT - intIOR * LdotH) * (extIOR * cosT - intIOR * LdotH) / ((extIOR * cosT + intIOR * LdotH) * (extIOR * cosT + intIOR * LdotH));

	return (Rs + Rp) * 0.5f;
}

// DdotH : Dir . Half
// HdotN : Half . Norm
float RDM_G1(float DdotH, float DdotN, float alpha)
{

	float b = 1.0f / (alpha * sqrt(1.0f - DdotN * DdotN) / DdotN);
	float k = DdotH / DdotN;

	if (k > zero_eps && b < (1.6f - zero_eps))
	{
		return RDM_chiplus(k) * (3.535f * b + 2.181f * b * b) / (1.0f + 2.276f * b + 2.577f * b * b);
	}
	else
	{
		return RDM_chiplus(k);
	}
}

// LdotH : Light . Half
// LdotN : Light . Norm
// VdotH : View . Half
// VdotN : View . Norm
float RDM_Smith(float LdotH, float LdotN, float VdotH, float VdotN, float alpha)
{
	return RDM_G1(LdotH, LdotN, alpha) * RDM_G1(VdotH, VdotN, alpha);
}

const float skyIOR = 1.f;

// Specular term of the Cook-torrance bsdf
// LdotH : Light . Half
// NdotH : Norm . Half
// VdotH : View . Half
// LdotN : Light . Norm
// VdotN : View . Norm
color3 RDM_bsdf_s(float LdotH, float NdotH, float VdotH, float LdotN, float VdotN, Material *m)
{
	return m->specularColor * RDM_Beckmann(NdotH, m->roughness) * RDM_Fresnel(LdotH, skyIOR, m->IOR) * RDM_Smith(LdotH, LdotN, VdotH, VdotN, m->roughness) / (4.f * LdotN * VdotN);
}
// diffuse term of the cook torrance bsdf
color3 RDM_bsdf_d(Material *m)
{
	return m->diffuseColor / PI;
}

// The full evaluation of bsdf(wi, wo) * cos (thetai)
// LdotH : Light . Half
// NdotH : Norm . Half
// VdotH : View . Half
// LdotN : Light . Norm
// VdtoN : View . Norm
// compute bsdf * cos(Oi)
color3 RDM_bsdf(float LdotH, float NdotH, float VdotH, float LdotN, float VdotN, Material *m)
{
	return RDM_bsdf_d(m) + RDM_bsdf_s(LdotH, NdotH, VdotH, LdotN, VdotN, m);
}

/* --------------------------------------------------------------------------- */

color3 shade(vec3 n, vec3 v, vec3 l, color3 lc, Material *mat)
{
	float LdotN = dot(l, n);

	if (LdotN < zero_eps)
		return color3(0, 0, 0);
	/*
	* Basic shader
	* Uncomment the following line to use it
	*/
	// return (mat->diffuseColor / PI) * LdotN * lc;

	/*
	* More complex shader using bsdf
	*/

	vec3 h = (v + l) / length(v + l);
	float LdotH = dot(l, h);
	float NdotH = dot(n, h);
	float VdotH = dot(v, h);
	float VdotN = dot(v, n);

	return lc * RDM_bsdf(LdotH, NdotH, VdotH, LdotN, VdotN, mat) * LdotN;
}

const int rayDepth_limit = 3;

color3 reflection(Scene *scene, Ray *ray, const Intersection intersection, KdTree *tree);
color3 refraction(Scene *scene, Ray *ray, const Intersection intersection, KdTree *tree);

//! if tree is not null, use intersectKdTree to compute the intersection instead of intersect scene
color3 trace_ray(Scene *scene, Ray *ray, KdTree *tree)
{
	color3 ret = color3(0, 0, 0);
	Intersection intersection, shadowInter;

	if (ray->depth >= rayDepth_limit)
		return ret;

	if (intersectKdTree(scene, tree, ray, &intersection))
	{
		//Shadows
		for (Light *light : scene->lights)
		{
			vec3 l = normalize(light->position - intersection.position);
			Ray shadowRay;
			rayInit(&shadowRay, intersection.position + acne_eps * l, l, 0, ((light->position - intersection.position + acne_eps * l) / l).x);

			if (!intersectKdTree(scene, tree, &shadowRay, &shadowInter))
			{

				ret += shade(intersection.normal, -ray->dir, l, light->color, intersection.mat);
			}
		}

		//Reflection
		ret += reflection(scene, ray, intersection, tree);
		if(intersection.mat->refractivity > zero_eps) ret += refraction(scene, ray, intersection, tree);
	}
	else
	{
		ret = scene->skyColor;
	}

	return ret;
}

color3 reflection(Scene *scene, Ray *ray, const Intersection intersection, KdTree *tree)
{
	Ray reflectionRay;
	vec3 newDir = reflect(ray->dir, intersection.normal);
	rayInit(&reflectionRay, intersection.position + acne_eps * newDir, newDir, 0, 100000, ray->depth + 1);
	return RDM_Fresnel(dot(newDir, normalize(-ray->dir + newDir)), skyIOR, intersection.mat->IOR) * intersection.mat->specularColor * trace_ray(scene, &reflectionRay, tree);
}

color3 refraction(Scene *scene, Ray *ray, const Intersection intersection, KdTree *tree){
	Ray reflectionRay;
	vec3 newDir = refract(ray->dir,intersection.normal, intersection.mat->IOR);
	rayInit(&reflectionRay, intersection.position + acne_eps * newDir, newDir, 0, 100000, ray->depth + 1);
	return intersection.mat->refractivity * trace_ray(scene, &reflectionRay, tree); 
}

const int squarerootNbRaysPerPixel = 1;

void renderImage(Image *img, Scene *scene)
{
	float nbRaysPerPixel = squarerootNbRaysPerPixel * squarerootNbRaysPerPixel;
	float dAntiAliasing = 1.f / float(squarerootNbRaysPerPixel);
	//! This function is already operational, you might modify it for antialiasing and kdtree initializaion
	float aspect = 1.f / scene->cam.aspect;

	KdTree *tree = NULL;

	tree = initKdTree(scene);

	float delta_y = 1.f / (img->height * 0.5f);   //! one pixel size
	vec3 dy = delta_y * aspect * scene->cam.ydir; //! one pixel step
	vec3 ray_delta_y = (0.5f - img->height * 0.5f) / (img->height * 0.5f) * aspect * scene->cam.ydir;

	float delta_x = 1.f / (img->width * 0.5f);
	vec3 dx = delta_x * scene->cam.xdir;
	vec3 ray_delta_x = (0.5f - img->width * 0.5f) / (img->width * 0.5f) * scene->cam.xdir;

	for (size_t j = 0; j < img->height; j++)
	{
		if (j != 0)
			printf("\033[A\r");
		float progress = (float)j / img->height * 100.f;
		printf("progress\t[");
		int cpt = 0;
		for (cpt = 0; cpt < progress; cpt += 5)
			printf(".");
		for (; cpt < 100; cpt += 5)
			printf(" ");
		printf("]\n");
#pragma omp parallel for
		for (size_t i = 0; i < img->width; i++)
		{
			color3 *ptr = getPixelPtr(img, i, j);
			color3 c;
			for (int k = 0; k < nbRaysPerPixel; k++)
			{
				vec3 ray_dir = scene->cam.center + ray_delta_x + ray_delta_y + (float(i) + dAntiAliasing * float(k % squarerootNbRaysPerPixel)) * dx + (float(j) + dAntiAliasing * (1 - float(k % squarerootNbRaysPerPixel))) * dy;

				Ray rx;
				rayInit(&rx, scene->cam.position, normalize(ray_dir));
				c += trace_ray(scene, &rx, tree);
			}
			*ptr = c / (float)nbRaysPerPixel;
		}
	}
}
