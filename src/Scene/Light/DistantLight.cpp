#include "DistantLight.hpp"

DistantLight::DistantLight(vec3 color_, vec3 position_) : color(color_), position(position_){

}

vec3 DistantLight::Color(vec3 observatorPosition){
	return color;
}