#ifndef __CAMERA__
#define __CAMERA__

#include "../GLM/glm/glm/glm.hpp"

using namespace glm;

class Camera{
	public:
		virtual ~Camera();
		Camera(vec3 position, vec3 at, vec3 up, float fov, float height, float width);

		vec3 position; //! eye position
		vec3 zdir;		 //! view direction
		vec3 xdir;		 //! right direction
		vec3 ydir;		 //! up direction
		vec3 center;   //! center of the image plane
		float fov;		 //! field of view
		float width;	//! width of the computed image
		float height;	//! height of the computed image
};

#endif