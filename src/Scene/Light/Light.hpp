#ifndef __LIGHT__
#define __LIGHT__

#include "../../GLM/glm/glm/glm.hpp"

using namespace glm;

class Light{
	public:
		vec3 position;
		virtual vec3 Color(vec3 observatorPosition) = 0;
};

#endif