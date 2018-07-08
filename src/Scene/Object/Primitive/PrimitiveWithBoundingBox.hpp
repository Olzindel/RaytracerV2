#ifndef __PRIMITIVE_WITH_BOUNDING_BOX__
#define __PRIMITIVE_WITH_BOUNDING_BOX__

#include "Primitive.hpp"

class PrimitiveWithBoundingBox : public Primitive {
	public:
		struct boundingBox{
			vec3 min;
			vec3 max;
		};
		virtual bool intersectPrimitive(Ray *ray, Intersection *intersection) = 0;
};

#endif