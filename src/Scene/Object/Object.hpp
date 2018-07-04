#ifndef __OBJECT__
#define __OBJECT__

#include "./Primitive/Primitive.hpp"
#include "./Material/Material.hpp"
#include "../../Ray/Ray.hpp"
#include "../../Intersection/Intersection.hpp"

class Object{
	public:
		Object(Primitive * primitive_ , Material material_);
		virtual ~Object();
		bool intersectObject(Ray *ray, Intersection *intersection);
	private:
		Primitive* primitive;
		Material material;
};

#endif