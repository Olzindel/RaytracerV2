#ifndef __RAYTRACER__
#define __RAYTRACER__

#include "../Scene/Scene.hpp"
#include "../Image.hpp"
#include "../Ray/Ray.hpp"
#include "../defines.hpp"
#include <string>

using namespace std;

class Raytracer {
    public:
        Raytracer(Scene _scene, Image * _image, int _maxRayDepth = 5, int _numberSamplePerPixel = 1);
        ~Raytracer();
        void renderScene();
    private:
        Scene scene;
        Image * image;
        int maxRayDepth;
        int numberSamplePerPixel;
        vec3 castRay(Ray * ray);
        vec3 shade_facingRatio(vec3 lightDir, vec3 intersectionPoint, vec3 normal, Light *light, Material const& material);
};

#endif