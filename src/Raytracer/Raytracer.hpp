#ifndef __RAYTRACER__
#define __RAYTRACER__

#include "../Scene/Scene.hpp"
#include <string>

using namespace std;

class Raytracer {
    public:
        Raytracer(Scene _scene, string _imageName, int _maxRayDepth = 5, int _numberSamplePerPixel = 1);
        ~Raytracer();
    private:
        Scene scene;
        string imageName;
        int maxRayDepth;
        int numberSamplePerPixel;
};

#endif