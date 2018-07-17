#ifndef __MATERIAL__
#define __MATERIAL__

#include "../../../GLM/glm/glm/glm.hpp"

using namespace glm;

enum MaterialCategory {Dielectric,Conductor};

class Material{
	public:
		MaterialCategory materialCategory = Dielectric;
		bool isDiffuse = true;
		bool isReflective = false;
		bool isRefractive = false;
		vec3 diffuseColor = vec3(1.f,1.f,1.f);
		vec3 specularColor = vec3(1.f,1.f,1.f);
		float IOR =1.5f;
		float roughness = 0.1f;
};

#endif