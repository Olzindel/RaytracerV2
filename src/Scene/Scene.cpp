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

Scene::~Scene(){
	
}