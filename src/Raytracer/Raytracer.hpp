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
        Raytracer(Scene _scene, Image * _image, int _maxRayDepth = 5, int _numberSamplePerPixel = 1, bool _useIndirectLighting = true, int _numberRaysIndirectLighting = 16);
        ~Raytracer();
        void renderScene();
    private:
        Scene scene;
        Image * image;
        int maxRayDepth;
        int numberSamplePerPixel;
        int numberRaysIndirectLighting;
        bool useIndirectLighting;
        

        vec3 castRay(Ray * ray);

        vec3 shade_facingRatio(vec3 lightDir, vec3 intersectionPoint, vec3 normal, Light *light, Material const& material);
        vec3 shade(vec3 rayDir, vec3 lightDir, vec3 intersectionPoint, vec3 normal, Light *light, Material const& material, vec3 materialColor);
        vec3 shade_BDSM(vec3 rayDir, vec3 lightDir, vec3 intersectionPoint, vec3 normal, Light *light, Material const& material, vec3 materialColor);

        vec3 reflection(Ray *ray, const Intersection intersection);
        vec3 refraction(Ray *ray, const Intersection intersection);
        float fresnel(Ray *ray, const Intersection intersection);

        vec3 RDM_bsdf(float LdotH, float NdotH, float VdotH, float LdotN, float VdotN, Material const& m, vec3 materialColor);
        vec3 RDM_bsdf_d(vec3 materialColor);
        vec3 RDM_bsdf_s(float LdotH, float NdotH, float VdotH, float LdotN, float VdotN, Material const& m);
        float RDM_Smith(float LdotH, float LdotN, float VdotH, float VdotN, float alpha);
        float RDM_G1(float DdotH, float DdotN, float alpha);
        float RDM_Fresnel(float LdotH, float extIOR, float intIOR);
        float RDM_chiplus(float c);
        float RDM_Beckmann(float NdotH, float alpha);

};

#endif