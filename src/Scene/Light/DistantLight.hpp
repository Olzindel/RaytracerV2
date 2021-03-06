#ifndef __DISTANT_LIGHT__
#define __DISTANT_LIGHT__

#include "./Light.hpp"

class DistantLight : public Light{
	public:
		DistantLight(vec3 color_, vec3 position_);
		vec3 Color(vec3 observatorPosition) override;
	private:
		vec3 color;
};

#endif