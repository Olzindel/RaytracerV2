#ifndef __DISTANT_LIGHT__
#define __DISTANT_LIGHT__

#include "./Light.hpp"

class DistantLight::Light{
	public:
		DistantLight(vec3 color_, vec3 position_);
	private:
		vec3 color;
};

#endif