#ifndef __INTERSECTION__
#define __INTERSECTION__

#include "../GLM/glm/glm/glm.hpp"
#include "../Scene/Object/Material/Material.hpp"

class Intersection {
	public:
		Material material;
		vec3 normal;
		vec3 position;
};

#endif