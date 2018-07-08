#include "./Object.hpp"


Object::Object(Primitive * primitive_ , Material material_) : primitive(primitive_),material(material_){

}

Object::~Object(){

}

bool Object::intersectObject(Ray *ray, Intersection * intersection){

	if(primitive->intersectPrimitive(ray, intersection)){
		intersection->material = material;
		return true;
	}

	return false;
}
