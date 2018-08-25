#include "Raytracer.hpp"
#include <iostream>

using namespace std;

Raytracer::Raytracer(Scene _scene, Image * _image, int _maxRayDepth, int _numberSamplePerPixel, bool _useIndirectLighting, int _numberRaysIndirectLighting) : scene(_scene), image(_image), maxRayDepth(_maxRayDepth), numberSamplePerPixel(_numberSamplePerPixel),useIndirectLighting(_useIndirectLighting),numberRaysIndirectLighting(_numberRaysIndirectLighting){

}
//=====================================================================================================================================================================================================

Raytracer::~Raytracer(){

}

//=====================================================================================================================================================================================================

vec3 Raytracer::shade_facingRatio(vec3 lightDir, vec3 intersectionPoint, vec3 normal, Light *light, Material const& material){
    float LdotN = dot(lightDir, normal);

    if(LdotN < ZERO_EPS){
        return vec3(0.f);
    }

    return (material.diffuseColor / (float)M_PI) * LdotN * light->Color(intersectionPoint);
}

//=====================================================================================================================================================================================================

vec3 Raytracer::shade_BDSM(vec3 rayDir, vec3 lightDir, vec3 intersectionPoint, vec3 normal, Light *light, Material const& material, vec3 materialColor){
     float LdotN = dot(lightDir, normal);

    if(LdotN < ZERO_EPS){
        return vec3(0.f);
    }
    vec3 v = -rayDir;
    vec3 l = lightDir;
    vec3 n = normal;
	vec3 h = (v + l) / length(v + l);
	float LdotH = dot(l, h);
	float NdotH = dot(n, h);
	float VdotH = dot(v, h);
	float VdotN = dot(v, n);

	return light->Color(intersectionPoint) * RDM_bsdf(LdotH, NdotH, VdotH, LdotN, VdotN, material, materialColor) * LdotN;
}

//=====================================================================================================================================================================================================

vec3 Raytracer::shade(vec3 rayDir, vec3 lightDir, vec3 intersectionPoint, vec3 normal, Light *light, Material const& material, vec3 materialColor){
    return shade_BDSM(rayDir, lightDir, intersectionPoint, normal, light, material, materialColor);
    // return shade_facingRatio(lightDir, intersectionPoint, normal, light, material);
}


//=====================================================================================================================================================================================================

vec3 Raytracer::castRay(Ray *r){

    vec3 returnColor = vec3(0.f,0.f,0.f);


    if(r->depth >= maxRayDepth) return returnColor;

    Intersection intersection = Intersection();
    if(scene.intersectScene(r, &intersection)){

        if(intersection.material.bumpMap != NULL){
			intersection.normal = intersection.material.bumpMap(intersection.position, intersection.normal);
		}

        if(intersection.material.isDiffuse){

            bool doesMaterialHaveProceduralTexture = false;
            vec3 proceduralTexturingColor = vec3(0.f);
             if(intersection.material.proceduralTexturingColor != NULL){
                doesMaterialHaveProceduralTexture = true;
                proceduralTexturingColor = intersection.material.proceduralTexturingColor(intersection.position);
            }

            for(Light * light : scene.lights){
                vec3 intersectionPoint = intersection.position + intersection.normal * ACNE_EPS;
                vec3 lightDir = normalize(light->position - intersectionPoint);

                Ray shadowRay = Ray(intersectionPoint,lightDir, r->depth, 0.f, distance(light->position,intersectionPoint));

                Intersection shadowIntersection = Intersection();

                if(!scene.intersectScene(&shadowRay, &shadowIntersection) || shadowIntersection.material.isRefractive){
                    returnColor += shade(r->dir, lightDir,intersectionPoint, intersection.normal, light, intersection.material, (doesMaterialHaveProceduralTexture) ? proceduralTexturingColor : intersection.material.diffuseColor);
                }
            }
        }


        if(intersection.material.isReflective){ 
            if(intersection.material.isRefractive){
                float kr = fresnel(r, intersection);
                returnColor += kr * reflection(r, intersection) +  (1 - kr) * refraction(r, intersection);
            } else {
                returnColor += reflection(r, intersection);
            }
        } 


        return returnColor;
    }

    return scene.skyColor;
}

//=====================================================================================================================================================================================================

/* --------------------------------------------------------------------------- */
/*
 *	The following functions are coded from Cook-Torrance bsdf model description and are suitable only
 *  for rough dielectrics material (RDM. Code has been validated with Mitsuba renderer)
 */

// Shadowing and masking function. Linked with the NDF. Here, Smith function, suitable for Beckmann NDF
float Raytracer::RDM_chiplus(float c)
{
	return (c > 0.f) ? 1.f : 0.f;
}

/** Normal Distribution Function : Beckmann
 * NdotH : Norm . Half
 */
float Raytracer::RDM_Beckmann(float NdotH, float alpha)
{
	if (abs(NdotH) < ZERO_EPS)
		return 0.f;

	float cosTetaSquared = NdotH * NdotH;
	float tanTetaSquared = (1.f - cosTetaSquared) / cosTetaSquared;
	float alphaSquared = alpha * alpha;

	return RDM_chiplus(NdotH) * exp(-(tanTetaSquared / alphaSquared)) / (M_PI * alphaSquared * cosTetaSquared * cosTetaSquared);
}

// Fresnel term computation. Implantation of the exact computation. we can use the Schlick approximation
// LdotH : Light . Half
float Raytracer::RDM_Fresnel(float LdotH, float extIOR, float intIOR)
{
	float Rs, Rp;
	float sinSquared_t = (extIOR / intIOR) * (extIOR / intIOR) * (1.f - LdotH * LdotH);

	if (sinSquared_t > 1.f - ZERO_EPS)
		return 1.f;

	float cosT = sqrt(1.f - sinSquared_t);

	Rs = (extIOR * LdotH - intIOR * cosT) * (extIOR * LdotH - intIOR * cosT) / ((extIOR * LdotH + intIOR * cosT) * (extIOR * LdotH + intIOR * cosT));
	Rp = (extIOR * cosT - intIOR * LdotH) * (extIOR * cosT - intIOR * LdotH) / ((extIOR * cosT + intIOR * LdotH) * (extIOR * cosT + intIOR * LdotH));

	return (Rs + Rp) * 0.5f;
}

// DdotH : Dir . Half
// HdotN : Half . Norm
float Raytracer::RDM_G1(float DdotH, float DdotN, float alpha)
{

	float b = 1.0f / (alpha * sqrt(1.0f - DdotN * DdotN) / DdotN);
	float k = DdotH / DdotN;

	if (k > ZERO_EPS && b < (1.6f - ZERO_EPS))
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
float Raytracer::RDM_Smith(float LdotH, float LdotN, float VdotH, float VdotN, float alpha)
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
vec3 Raytracer::RDM_bsdf_s(float LdotH, float NdotH, float VdotH, float LdotN, float VdotN, Material const& m)
{
	return m.specularColor * RDM_Beckmann(NdotH, m.roughness) * RDM_Fresnel(LdotH, skyIOR, m.IOR) * RDM_Smith(LdotH, LdotN, VdotH, VdotN, m.roughness) / (4.f * LdotN * VdotN);
}
// diffuse term of the cook torrance bsdf
vec3 Raytracer::RDM_bsdf_d(vec3 materialColor)
{
	return materialColor / (float)M_PI;
}

// The full evaluation of bsdf(wi, wo) * cos (thetai)
// LdotH : Light . Half
// NdotH : Norm . Half
// VdotH : View . Half
// LdotN : Light . Norm
// VdtoN : View . Norm
// compute bsdf * cos(Oi)
vec3 Raytracer::RDM_bsdf(float LdotH, float NdotH, float VdotH, float LdotN, float VdotN, Material const& m, vec3 materialColor)
{
	return RDM_bsdf_d(materialColor) + RDM_bsdf_s(LdotH, NdotH, VdotH, LdotN, VdotN, m);
}

//=====================================================================================================================================================================================================


vec3 Raytracer::reflection(Ray *ray, const Intersection intersection){
    vec3 reflectionRayDir = reflect(ray->dir, intersection.normal);
    vec3 reflectionRayOrigin = intersection.position + intersection.normal * ACNE_EPS;
    Ray reflectionRay = Ray(reflectionRayOrigin, reflectionRayDir,ray->depth + 1);
    return castRay(&reflectionRay);
}

//=====================================================================================================================================================================================================

vec3 Raytracer::refraction(Ray *ray, const Intersection intersection){
    float cosi = clamp(dot(ray->dir,intersection.normal), -1.f, 1.f);
    float etai = 1.0003f, etat = intersection.material.IOR;
    vec3 n = intersection.normal;

    if (cosi < 0.f) { 
        cosi = -cosi; 
    } else { 
        std::swap(etai, etat); 
        n = -intersection.normal; 
    }

    float eta = etai / etat;
    float k = 1.f - eta * eta * (1.f - cosi * cosi);

    if(k < 0.f) return vec3(0.f);
    vec3 refractionRayDir = normalize(eta * ray->dir + (eta * cosi - sqrtf(k)) * n);


    // vec3 refractionRayDir = refract(ray->dir,intersection.normal, 1.0003f / intersection.material.IOR);
    vec3 refractionRayOrigin = intersection.position - n * ACNE_EPS;
    Ray refractionRay = Ray(refractionRayOrigin, refractionRayDir, ray->depth + 1);
    return castRay(&refractionRay);
}

//=====================================================================================================================================================================================================

float Raytracer::fresnel(Ray *ray, const Intersection intersection){
    float kr =0.f;
    float cosi = clamp(dot(ray->dir,intersection.normal), -1.f, 1.f);
    float etai = 1.0003f, etat = intersection.material.IOR;

    if (cosi > 0) { 
        std::swap(etai, etat); 
    }
    // Compute sini using Snell's law
    float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
    // Total internal reflection
    if (sint >= 1) {
        kr = 1;
    }
    else {
        float cost = sqrtf(std::max(0.f, 1 - sint * sint));
        cosi = fabsf(cosi);
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        kr = (Rs * Rs + Rp * Rp) / 2;
    }

    // As a consequence of the conservation of energy, transmittance is given by:
    // kt = 1 - kr;

    return kr;
}


//=====================================================================================================================================================================================================

void Raytracer::renderScene(){
    
    float nbRaysPerPixel = this->numberSamplePerPixel * this->numberSamplePerPixel;
    float dAntiAliasing = 1.f / float(this->numberSamplePerPixel);
  
    float aspect = 1.f / scene.camera.aspect;

    float delta_y = 1.f / (image->height * 0.5f);   //! one pixel size
    vec3 dy = delta_y * aspect * scene.camera.ydir; //! one pixel step
    vec3 ray_delta_y = (0.5f - image->height * 0.5f) / (image->height * 0.5f) * aspect * scene.camera.ydir;

    float delta_x = 1.f / (image->width * 0.5f);
    vec3 dx = delta_x * scene.camera.xdir;
    vec3 ray_delta_x = (0.5f - image->width * 0.5f) / (image->width * 0.5f) * scene.camera.xdir;

    for (size_t j = 0; j < image->height; j++)
    {
        if (j != 0)
            printf("\033[A\r");
        float progress = (float)j / image->height * 100.f;
        printf("progress\t[");
        int cpt = 0;
        for (cpt = 0; cpt < progress; cpt += 5)
            printf(".");
        for (; cpt < 100; cpt += 5)
            printf(" ");
        printf("]\n");
#pragma omp parallel for
        for (size_t i = 0; i < image->width; i++)
        {
            vec3 *ptr = getPixelPtr(image, i, j);
            vec3 c(0.f,0.f,0.f);
            for (int k = 0; k < nbRaysPerPixel; k++)
            {
                vec3 ray_dir = scene.camera.center + ray_delta_x + ray_delta_y + (float(i) + dAntiAliasing * float(k % numberSamplePerPixel)) * dx + (float(j) + dAntiAliasing * (1 - float(k % numberSamplePerPixel))) * dy;

                Ray rx = Ray(scene.camera.position, normalize(ray_dir));
                c += castRay(&rx);
            }
            *ptr = c / (float)nbRaysPerPixel;
        }
    }
}

