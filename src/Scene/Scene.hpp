#ifndef __SCENE__
#define __SCENE__

#include "./Object/Object.hpp"
#include "./Light/Light.hpp"
#include "./Camera/Camera.hpp"
#include <list>

using namespace std;
using namespace glm;

class Scene{
	public:
		list<Object *> objects;
		list<Light *> lights;
		Camera camera;
		vec3 skyColor;
		Scene(Camera camera_, vec3 skyColor_);
		~Scene();
		void addObject(Object *object);
		void addLight(Light * light);
		bool intersectScene(Ray * ray, Intersection * intersection);
};

#endif