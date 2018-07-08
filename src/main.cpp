#include "./GLM/glm/glm/glm.hpp"
#include "./Scene/Scene.hpp"
#include "./Raytracer/Raytracer.hpp"
#include "./Image.hpp"
#include "./Scene/Light/DistantLight.hpp"
#include "Scene/Object/Primitive/Sphere/Sphere.hpp"

#include <iostream>

#define HEIGHT 600
#define WIDTH 800

int main(int argc, char *argv[]){
	if(argc < 2) {
		cout << "Need a image name";
		exit(1);
	}

	Camera c = Camera(vec3(0,0,0), vec3(0,0,1), vec3(0,1,0), 60, (float)WIDTH/(float)HEIGHT);
	Scene s = Scene(c, vec3(0,0.5f,0.7f));

	Material m = Material();
	m.ambiantColor = vec3(0.f,0.f,0.f);
	m.diffuseColor = vec3(1.f, 1.f, 1.f);

	Sphere sphere = Sphere(vec3(0,0,1), 0.25f);
	Object o = Object(&sphere,m);
	s.addObject(&o);

	DistantLight light = DistantLight(vec3(0.f,0.f,1.f), vec3(0.f,1.f,1.f));
	s.addLight(&light);
	DistantLight light2 = DistantLight(vec3(0.f,1.f,0.f), vec3(1.f,0.f,1.f));
	s.addLight(&light2);
	DistantLight light3 = DistantLight(vec3(1.f,0.f,0.f), vec3(1.f,1.f,0.f));
	s.addLight(&light3);

	Image * img = initImage(WIDTH,HEIGHT);

	Raytracer r = Raytracer(s, img);
	r.renderScene();
	
	saveImage(img, argv[1]);
	freeImage(img);

	return 0;
}