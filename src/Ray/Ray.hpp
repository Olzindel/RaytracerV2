#ifndef __RAY__
#define __RAY__

#include "../GLM/glm/glm/glm.hpp"

using namespace glm;

class Ray{
	public:
		vec3 origin;
		vec3 dir;
		float depth; //! The number of times a ray has bounced
		float tmin;
		float tmax;
		Ray(vec3 origin, vec3 dir, float depth = 0, float tmin = 0 , float tmax = 100000);
		vec3 rayAt(float t);
};

#endif