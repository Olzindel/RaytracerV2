#include "Raytracer.hpp"
#include <iostream>

using namespace std;

Raytracer::Raytracer(Scene _scene, Image * _image, int _maxRayDepth, int _numberSamplePerPixel) : scene(_scene), image(_image), maxRayDepth(_maxRayDepth), numberSamplePerPixel(_numberSamplePerPixel){

}

//=====================================================================================================================================================================================================

Raytracer::~Raytracer(){

}

vec3 Raytracer::shade_facingRatio(vec3 lightDir, vec3 intersectionPoint, vec3 normal, Light *light, Material const& material){
    float LdotN = dot(lightDir, normal);

    if(LdotN < ZERO_EPS){
        return vec3(0.f);
    }

    return (material.diffuseColor / (float)M_PI) * LdotN * light->Color(intersectionPoint) + material.ambiantColor;
}

//=====================================================================================================================================================================================================

vec3 Raytracer::castRay(Ray *r){

    if(r->depth >= maxRayDepth) return scene.skyColor;

    vec3 returnColor = vec3(0.f,0.f,0.f);

    Intersection intersection = Intersection();
    if(scene.intersectScene(r, &intersection)){
        for(Light * light : scene.lights){
            vec3 intersectionPoint = intersection.position + intersection.normal * ACNE_EPS;
            vec3 lightDir = normalize(light->position - intersectionPoint);

            Ray shadowRay = Ray(intersectionPoint,lightDir, r->depth, 0.f, distance(light->position,intersectionPoint));

            Intersection shadowIntersection = Intersection();

            if(!scene.intersectScene(&shadowRay, &shadowIntersection)){
                returnColor += shade_facingRatio(lightDir,intersectionPoint, intersection.normal, light, intersection.material);
            }
        }

        return returnColor;
    }

    return scene.skyColor;
}

//=====================================================================================================================================================================================================

void Raytracer::renderScene(){
    
    float nbRaysPerPixel = this->numberSamplePerPixel * this->numberSamplePerPixel;
    float dAntiAliasing = 1.f / float(this->numberSamplePerPixel);
  
    float aspect = 1.f / scene.camera.aspect;

    float delta_y = 1.f / (image->height * 0.5f);   //! one pixel size
    vec3 dy = delta_y * aspect * scene.camera.ydir; //! one pixel step
    vec3 ray_delta_y = (0.5f - image->height * 0.5f) / (image->height * 0.5f) * aspect * scene.camera.ydir;

    float delta_x = 1.f / (image->width * 0.5f);
    vec3 dx = delta_x * scene.camera.xdir;
    vec3 ray_delta_x = (0.5f - image->width * 0.5f) / (image->width * 0.5f) * scene.camera.xdir;

    for (size_t j = 0; j < image->height; j++)
    {
        if (j != 0)
            printf("\033[A\r");
        float progress = (float)j / image->height * 100.f;
        printf("progress\t[");
        int cpt = 0;
        for (cpt = 0; cpt < progress; cpt += 5)
            printf(".");
        for (; cpt < 100; cpt += 5)
            printf(" ");
        printf("]\n");
#pragma omp parallel for
        for (size_t i = 0; i < image->width; i++)
        {
            vec3 *ptr = getPixelPtr(image, i, j);
            vec3 c(0.f,0.f,0.f);
            for (int k = 0; k < nbRaysPerPixel; k++)
            {
                vec3 ray_dir = scene.camera.center + ray_delta_x + ray_delta_y + (float(i) + dAntiAliasing * float(k % numberSamplePerPixel)) * dx + (float(j) + dAntiAliasing * (1 - float(k % numberSamplePerPixel))) * dy;

                Ray rx = Ray(scene.camera.position, normalize(ray_dir));
                c += castRay(&rx);
            }
            *ptr = c / (float)nbRaysPerPixel;
        }
    }
}

