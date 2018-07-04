#ifndef __RAY__
#define __RAY__

#include "../GLM/glm/glm/glm.hpp"

using namespace glm;

class Ray{
	public:
		vec3 origin;
		vec3 dir;
		float depth; //! The number of times a ray has bounce 
		Ray(vec3 origin, vec3 dir);
		Ray(vec3 origin, vec3 dir, float depth);
};

#endif