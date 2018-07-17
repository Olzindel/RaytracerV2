#include "./Scene.hpp"

void Scene::addObject(Object *object){
	objects.push_back(object);
}

void Scene::addLight(Light *light){
	lights.push_back(light);
}

Scene::Scene(Camera camera_, vec3 skyColor_) : camera(camera_), skyColor(skyColor_){
	objects = list<Object*>();
	lights = list<Light*>();
}

bool Scene::intersectScene(Ray * ray, Intersection * intersection){
	bool intersect = false;
	for(Object * o : objects){
		intersect = o->intersectObject(ray, intersection) || intersect;
	}

	return intersect;
}

Scene::~Scene(){
	
}