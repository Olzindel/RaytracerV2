#include "Ray.hpp"

Ray::Ray(vec3 origin, vec3 dir, float depth, float tmin, float tmax): origin(origin), dir(dir), depth(depth), tmin(tmin), tmax(tmax){}

vec3 Ray::rayAt(float t){
    return origin + t * dir;
}