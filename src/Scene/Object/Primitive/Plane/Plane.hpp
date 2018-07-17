#ifndef __PLANE__
#define __PLANE__

#include "../Primitive.hpp"
#include "../../Material/Material.hpp"
#include "../../../../GLM/glm/glm/glm.hpp"

using namespace glm;

class Plane : public Primitive
{
    public:
       Plane(vec3 normal_, float dist_);
        bool intersectPrimitive(Ray *ray, Intersection *intersection) override;
    private:
        vec3 normal;
        float dist;
};

#endif