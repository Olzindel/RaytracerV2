#ifndef __PRIMITIVE_WITH_BOUNDING_BOX__
#define __PRIMITIVE_WITH_BOUNDING_BOX__

#include "Primitive.hpp"

class PrimitiveWithBoundingBox : Primitive {
	public:
		struct boundingBox{
			vec3 min;
			vec3 max;
		};
};

#endif