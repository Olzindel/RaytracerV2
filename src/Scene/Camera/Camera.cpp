#include "Camera.hpp"
#include "../GLM/glm/glm/glm.hpp"

Camera::~Camera(){
}

Camera::Camera(vec3 position, vec3 at, vec3 up, float fov, float height, float width){
	this->fov = fov;
	this->height = height;
	this->width = width;
	this->position = position;
	this->zdir = normalize(at - position);
	this->xdir = normalize(cross(up, this->zdir));
	this->ydir = normalize(cross(this->zdir, this->xdir));
	this->center = 1.f / tanf((this->fov * M_PI / 180.f) * 0.5f) * this->zdir;
}