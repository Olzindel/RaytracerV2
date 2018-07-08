#include "Sphere.hpp"
#include "../../../../defines.hpp"

Sphere::Sphere(vec3 _center, float _radius) : center(_center), radius(_radius){}

bool Sphere::intersectPrimitive(Ray *ray, Intersection *intersection){

    vec3 oMinusC = ray->origin - this->center;
	float b = 2.f * dot(ray->dir, oMinusC);
	float c = dot(oMinusC, oMinusC) - this->radius * this->radius;
	float delta = b * b - 4 * c;
	float t;

	vec3 intersectionPoint;

	if (delta < ZERO_EPS)
		return false;

	if (delta > ZERO_EPS)
	{
		t = (-b - sqrt(delta)) / 2.f;
		if (t < -ZERO_EPS)
		{
			t = (-b + sqrt(delta)) / 2.f;
			if (t < -ZERO_EPS)
				return false;
		}
	}
	else
	{
		t = (-b) / 2.f;
		if (t < -ZERO_EPS)
			return false;
	}

	if (t < ray->tmin || t > ray->tmax)
		return false;

	intersectionPoint = ray->rayAt(t);
	intersection->position = intersectionPoint;
	intersection->normal = normalize(intersectionPoint - this->center);
	ray->tmax = t;

	return true;
}