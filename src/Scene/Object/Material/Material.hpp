#ifndef __MATERIAL__
#define __MATERIAL__

#include "../../../GLM/glm/glm/glm.hpp"

using namespace glm;

enum MaterialCategory {Dielectric,Conductor};
enum MaterialType {Diffuse, Reflective, ReflectiveAndRefractive};

class Material{
	public:
		MaterialCategory materialCategory;
		MaterialType materialType;
		vec3 ambiantColor;
		vec3 diffuseColor;
		vec3 specularColor;
		float IOR;
		float roughness;
};

#endif