#include "Raytracer.hpp"

using namespace std;

Raytracer::Raytracer(Scene _scene, string _imageName, int _maxRayDepth, int _numberSamplePerPixel) : scene(_scene), imageName(_imageName), maxRayDepth(_maxRayDepth), numberSamplePerPixel(_numberSamplePerPixel){

}

Raytracer::~Raytracer(){

}