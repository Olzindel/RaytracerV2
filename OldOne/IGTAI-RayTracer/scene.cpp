#include "scene.h"
#include "scene_types.h"
#include <string.h>
#include <algorithm>

BoundingBox determineBoundingBox(Object *obj)
{
	BoundingBox box;

	switch (obj->geom.type)
	{
	case SPHERE:
	{
		float rV = obj->geom.sphere.radius;
		box.min = obj->geom.sphere.center - rV;
		box.max = obj->geom.sphere.center + rV;
	}
	break;
	case TRIANGLE:
		box.min = min(min(obj->geom.triangle.p1, obj->geom.triangle.p2), obj->geom.triangle.p3);
		box.max = max(max(obj->geom.triangle.p1, obj->geom.triangle.p2), obj->geom.triangle.p3);
		break;
	default:
		box.min = point3(0, 0, 0);
		box.max = point3(0, 0, 0);
		break;
	}

	return box;
}

BoundingBox getObjectBoundingBox(Object *obj)
{
	return obj->geom.box;
}

Object *initSphere(point3 center, float radius, Material mat)
{
	Object *ret;
	ret = (Object *)malloc(sizeof(Object));
	ret->geom.type = SPHERE;
	ret->geom.sphere.center = center;
	ret->geom.sphere.radius = radius;
	memcpy(&(ret->mat), &mat, sizeof(Material));
	ret->geom.box = determineBoundingBox(ret);
	return ret;
}

Object *initPlane(vec3 normal, float d, Material mat)
{
	Object *ret;
	ret = (Object *)malloc(sizeof(Object));
	ret->geom.type = PLANE;
	ret->geom.plane.normal = normalize(normal);
	ret->geom.plane.dist = d;
	memcpy(&(ret->mat), &mat, sizeof(Material));
	return ret;
}

Object *initTriangle(point3 p1, point3 p2, point3 p3, Material mat)
{
	Object *ret;
	ret = (Object *)malloc(sizeof(Object));
	ret->geom.type = TRIANGLE;
	ret->geom.triangle.p1 = p1;
	ret->geom.triangle.p2 = p2;
	ret->geom.triangle.p3 = p3;
	memcpy(&(ret->mat), &mat, sizeof(Material));
	ret->geom.box = determineBoundingBox(ret);
	return ret;
}

void freeObject(Object *obj)
{
	free(obj);
}

Light *initLight(point3 position, color3 color)
{
	Light *light = (Light *)malloc(sizeof(Light));
	light->position = position;
	light->color = color;
	return light;
}

void freeLight(Light *light)
{
	free(light);
}

Scene *initScene()
{
	return new Scene;
}

void freeScene(Scene *scene)
{
	std::for_each(scene->objects.begin(), scene->objects.end(), freeObject);
	std::for_each(scene->lights.begin(), scene->lights.end(), freeLight);
	delete scene;
}

void setCamera(Scene *scene, point3 position, point3 at, vec3 up, float fov, float aspect)
{
	scene->cam.fov = fov;
	scene->cam.aspect = aspect;
	scene->cam.position = position;
	scene->cam.zdir = normalize(at - position);
	scene->cam.xdir = normalize(cross(up, scene->cam.zdir));
	scene->cam.ydir = normalize(cross(scene->cam.zdir, scene->cam.xdir));
	scene->cam.center = 1.f / tanf((scene->cam.fov * M_PI / 180.f) * 0.5f) * scene->cam.zdir;
}

void addObject(Scene *scene, Object *obj)
{
	scene->objects.push_back(obj);
}

void addLight(Scene *scene, Light *light)
{
	scene->lights.push_back(light);
}

void setSkyColor(Scene *scene, color3 c)
{
	scene->skyColor = c;
}
