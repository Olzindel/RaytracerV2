#include "./GLM/glm/glm/glm.hpp"
#include "./Scene/Scene.hpp"
#include "./Raytracer/Raytracer.hpp"
#include "./Image.hpp"
#include "./Scene/Light/DistantLight.hpp"
#include "Scene/Object/Primitive/Sphere/Sphere.hpp"
#include "Scene/Object/Primitive/Plane/Plane.hpp"

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

	m.diffuseColor = vec3(1.f, 1.f, 1.f);
	m.specularColor = vec3(1.f, 1.f, 1.f);
	m.roughness = 0.02f;
	m.isDiffuse = true;
	m.isReflective = false;
	m.IOR = 1.3f;
	Sphere sphere = Sphere(vec3(0,0,10), 0.6f);
	Object o = Object(&sphere,m);
	s.addObject(&o);

	m.isDiffuse = true;
	m.isReflective = true;
	m.roughness = 0.001f;
	Plane plane = Plane(vec3(0.,1, 0),0.25);
	Object plane_ = Object(&plane, m);
	s.addObject(&plane_);

	m.isDiffuse = false;
	m.isReflective = true;
	m.isRefractive = true;
	Sphere sphere2 = Sphere(vec3(0.15f,0.1f,3), 0.3f);
	Object o2 = Object(&sphere2,m);
	s.addObject(&o2);

	DistantLight light = DistantLight(vec3(0.f,0.f,1.f), vec3(0.f,0.f,2.f));
	s.addLight(&light);
	DistantLight light2 = DistantLight(vec3(0.f,1.f,0.f), vec3(1.f,0.f,1.f));
	s.addLight(&light2);
	DistantLight light3 = DistantLight(vec3(1.f,0.f,0.f), vec3(1.f,1.f,0.f));
	s.addLight(&light3);
	DistantLight sun = DistantLight(vec3(1.f, 1.f, 1.f), vec3(100,100,1));
	s.addLight(&sun);

	Image * img = initImage(WIDTH,HEIGHT);

	Raytracer r = Raytracer(s, img, 10, 8);
	r.renderScene();
	
	saveImage(img, argv[1]);
	freeImage(img);

	return 0;
}