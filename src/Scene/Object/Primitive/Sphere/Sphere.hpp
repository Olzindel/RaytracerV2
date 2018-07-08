#ifndef __SPHERE__
#define __SPHERE__

#include "../PrimitiveWithBoundingBox.hpp"
#include "../../Material/Material.hpp"
#include "../../../../GLM/glm/glm/glm.hpp"

using namespace glm;

class Sphere : public PrimitiveWithBoundingBox
{
    public:
        Sphere(vec3 _center, float _radius);
        bool intersectPrimitive(Ray *ray, Intersection *intersection) override;
    private:
        vec3 center;
        float radius;
};

#endif