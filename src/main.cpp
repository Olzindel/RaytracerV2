#include "./GLM/glm/glm/glm.hpp"
#include "./Scene/Scene.hpp"

#include <iostream>

int main(int argc, char *argv[]){
	Camera c = Camera(vec3(0,0,0), vec3(0,0,1), vec3(0,1,0), 60, 800, 600);
	Scene s = Scene(c, vec3(0,0,1));
	cout << "R:" << s.skyColor.x << " G:" << s.skyColor.y << " B:" << s.skyColor.z<< "\n";

	return 0;
}