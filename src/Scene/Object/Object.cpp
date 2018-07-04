#include "./Object.hpp"


Object::Object(Primitive * primitive_ , Material material_) : primitive(primitive_),material(material_){

}

Object::~Object(){

}

bool Object::intersectObject(Ray *ray, Intersection * intersection){
	return primitive->intersectPrimitive(ray, intersection);
}
