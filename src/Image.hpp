#ifndef __IMAGE__
#define __IMAGE__

#include "GLM/glm/glm/glm.hpp"

using namespace glm;

typedef struct iamge_s{
    size_t width;
    size_t height;
    vec3 *data;
} Image;

vec3 *getPixelPtr(Image *img, size_t x, size_t y);
Image *initImage(size_t width, size_t height);
void freeImage(Image *img);
void saveImage(Image *img, char *baseName);

#endif