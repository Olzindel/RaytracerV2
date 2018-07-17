#include "Plane.hpp"
#include "../../../../defines.hpp"

Plane::Plane(vec3 normal_, float dist_) : normal(normal_), dist(dist_){}

bool Plane::intersectPrimitive(Ray *ray, Intersection *intersection){
	float DdotN = dot(ray->dir, normal);

	float t;

	if (abs(DdotN) <= ZERO_EPS)
		return false;

	t = -(dot(ray->origin, normal) + dist) / DdotN;

	if (t < ZERO_EPS || t <= ray->tmin || t >= ray->tmax)
		return false;

	intersection->position = ray->rayAt(t);
	intersection->normal = DdotN <= ZERO_EPS ? normal : -normal;
	ray->tmax = t;

	return true;
}