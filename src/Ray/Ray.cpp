#include "Ray.hpp"

Ray::Ray(vec3 origin, vec3 dir) : origin(origin), dir(dir), depth(0){}

Ray(vec3 origin, vec3 dir, float depth) : origin(origin), dir(dir), depth(depth){}