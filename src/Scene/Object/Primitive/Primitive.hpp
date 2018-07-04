#ifndef __PRIMITIVE__
#define __PRIMITIVE__

#include "../../../Ray/Ray.hpp"
#include "../../../Intersection/Intersection.hpp"

class Primitive{
	public:
		virtual bool intersectPrimitive(Ray *ray, Intersection *intersection) = 0;
};

#endif