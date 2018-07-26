#include "./GLM/glm/glm/glm.hpp"
#include "./Scene/Scene.hpp"
#include "./Raytracer/Raytracer.hpp"
#include "./Image.hpp"
#include "./Scene/Light/DistantLight.hpp"
#include "Scene/Object/Primitive/Sphere/Sphere.hpp"
#include "Scene/Object/Primitive/Plane/Plane.hpp"

#include <iostream>

#define HEIGHT 600
#define WIDTH 800

mat3 fromEuler(vec3 ang) {
	vec2 a1 = vec2(sin(ang.x),cos(ang.x));
    vec2 a2 = vec2(sin(ang.y),cos(ang.y));
    vec2 a3 = vec2(sin(ang.z),cos(ang.z));
    mat3 m;
    m[0] = vec3(a1.y*a3.y+a1.x*a2.x*a3.x,a1.y*a2.x*a3.x+a3.y*a1.x,-a2.y*a3.x);
	m[1] = vec3(-a2.y*a1.x,a1.y*a2.y,a2.x);
	m[2] = vec3(a3.y*a1.x*a2.x+a1.y*a3.x,a1.x*a3.x-a1.y*a3.y*a2.x,a2.y*a3.y);
	return m;
}
float hash_( vec2 p ) {
	float h = dot(p,vec2(127.1,311.7));	
    return fract(sin(h)*43758.5453123);
}
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract( p );	
	vec2 u = f*f*(3.0f-2.0f*f);
    return -1.0+2.0*mix( mix( hash_( i + vec2(0.0f,0.0f) ), 
                     hash_( i + vec2(1.0,0.0) ), u.x),
                mix( hash_( i + vec2(0.0,1.0) ), 
                     hash_( i + vec2(1.0,1.0) ), u.x), u.y);
}

const int ITER_GEOMETRY = 3;
const int ITER_FRAGMENT = 5;
const float SEA_HEIGHT = 0.9;
const float SEA_CHOPPY = 4.0;
const float SEA_SPEED = 0.8;
const float SEA_FREQ = 0.16;
const vec3 SEA_BASE = vec3(0.1,0.19,0.22);
const vec3 SEA_WATER_COLOR = vec3(0.8,0.9,0.6);
#define SEA_TIME (1.0f + 1.f * SEA_SPEED)
const mat2 octave_m = mat2(1.6,1.2,-1.2,1.6);

float sea_octave(vec2 uv, float choppy) {
    uv += noise(uv);        
    vec2 wv = 1.0f-abs(sin(uv));
    vec2 swv = abs(cos(uv));    
    wv = mix(wv,swv,wv);
    return pow(1.0-pow(wv.x * wv.y,0.65),choppy);
}

vec3 map(vec3 p, vec3 diffuseColorMat) {
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    vec2 uv = vec2(p.x, p.z); uv.x *= 0.75;
    
    float d, h = 0.0;    
    for(int i = 0; i < ITER_GEOMETRY; i++) {        
    	d = sea_octave((uv+SEA_TIME)*freq,choppy);
    	d += sea_octave((uv-SEA_TIME)*freq,choppy);
        h += d * amp;        
    	uv = uv * octave_m; freq *= 1.9; amp *= 0.22;
        choppy = mix(choppy,1.0f,0.2f);
    }
    return (p.y - h) * diffuseColorMat;
}

float map_detailed(vec3 p) {
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    vec2 uv = vec2(p.x, p.z); uv.x *= 0.75;
    
    float d, h = 0.0;    
    for(int i = 0; i < ITER_FRAGMENT; i++) {        
    	d = sea_octave((uv+SEA_TIME)*freq,choppy);
    	d += sea_octave((uv-SEA_TIME)*freq,choppy);
        h += d * amp;        
    	uv = uv * octave_m; freq *= 1.9; amp *= 0.22;
        choppy = mix(choppy,1.0f,0.2f);
    }
    return p.y - h;
}

vec3 getNormal(vec3 p) {
    vec3 n;
	float eps = 1.e-3f;
    n.y = map_detailed(p);    
    n.x = map_detailed(vec3(p.x+eps,p.y,p.z)) - n.y;
    n.z = map_detailed(vec3(p.x,p.y,p.z+eps)) - n.y;
    n.y = eps;
    return normalize(n);
}

//=====================================================================================================

inline float modulo(const float &x)
{
	return x - std::floor(x);
}

vec3 patternChecked(vec3 coordinates, vec3 matDiffuseColor){
	float angle = M_PI/4.f;
	float s = coordinates.x * cos(angle) - coordinates.z * sin(angle);
	float t = coordinates.z * cos(angle) + coordinates.x * sin(angle);
	float scaleS = 5, scaleT = 5;
	float pattern = (modulo(s * scaleS) < 0.5) ^ (modulo(t * scaleT) < 0.5);
	return pattern * matDiffuseColor;
}

int main(int argc, char *argv[]){
	if(argc < 2) {
		cout << "Need a image name";
		exit(1);
	}

	Camera c = Camera(vec3(0,0,0), vec3(0,0,1), vec3(0,1,0), 60, (float)WIDTH/(float)HEIGHT);
	Scene s = Scene(c, vec3(0.f,0.6f,0.7f));

	Material m = Material();

	m.diffuseColor = vec3(1.f, .0f, .0f);
	m.specularColor = vec3(1.f, 1.f, 1.f);
	m.roughness = 0.02f;
	m.isDiffuse = true;
	m.isReflective = false;
	m.IOR = 1.008f;
	Sphere sphere = Sphere(vec3(0,0,10), 0.6f);
	Object o = Object(&sphere,m);
	s.addObject(&o);

	m.diffuseColor = vec3(1.f, 1.f, 1.f);
	m.isDiffuse = true;
	m.isReflective = false;
	m.roughness = 0.001f;
	m.proceduralTexturingColor = &patternChecked;
	Plane plane = Plane(vec3(0.,1, 0),1.f);
	Object plane_ = Object(&plane, m);
	s.addObject(&plane_);

	m.diffuseColor = SEA_WATER_COLOR;
	m.isDiffuse = true;
	m.isReflective = true;
	m.isRefractive = true;
	m.roughness = 0.4f;
	m.IOR = 1.33f;
	m.proceduralTexturingColor = NULL;
	m.bumpMap = NULL;
	Plane water = Plane(vec3(0.,1, 0),0.05f);
	Object water_ = Object(&water, m);
	s.addObject(&water_);

	m.diffuseColor = vec3(1.f, .5f, 1.f);
	m.specularColor = vec3(1.f, 1.f, 1.f);
	m.roughness = 0.4f;
	m.isDiffuse = true;
	m.isReflective = true;
	m.isRefractive = false;
	m.bumpMap = NULL;
	m.proceduralTexturingColor = NULL;
	Sphere sphere3 = Sphere(vec3(-.5f,0.f,2.f), 0.3f);
	Object o3 = Object(&sphere3,m);
	s.addObject(&o3);

	m.diffuseColor = vec3(1.f, .5f, .5f);
	m.specularColor = vec3(1.f, 1.f, 1.f);
	m.roughness = 0.4f;
	m.isDiffuse = true;
	m.isReflective = true;
	m.isRefractive = false;
	m.proceduralTexturingColor = NULL;
	Sphere sphere5 = Sphere(vec3(0.11f,0.01f,0.5f), 0.1f);
	Object o5 = Object(&sphere5,m);
	s.addObject(&o5);

	m.diffuseColor = vec3(0.f, .5f, 1.f);
	m.specularColor = vec3(1.f, 1.f, 1.f);
	m.roughness = 0.4f;
	m.isDiffuse = true;
	m.isReflective = false;
	m.isRefractive = false;
	m.proceduralTexturingColor = NULL;
	Sphere sphere4 = Sphere(vec3(-.5f,0.f,1.f), 0.1f);
	Object o4 = Object(&sphere4,m);
	s.addObject(&o4);
	
	m.isDiffuse = false;
	m.isReflective = true;
	m.isRefractive = true;
	m.IOR = 1.5f;
	m.proceduralTexturingColor = NULL;
	Sphere sphere2 = Sphere(vec3(0.15f,0.1f,1.f), 0.3f);
	Object o2 = Object(&sphere2,m);
	s.addObject(&o2);


	DistantLight light = DistantLight(vec3(0.f,0.f,1.f), vec3(0.f,0.f,2.f));
	s.addLight(&light);
	DistantLight light2 = DistantLight(vec3(0.f,1.f,0.f), vec3(1.f,0.f,1.f));
	s.addLight(&light2);
	DistantLight light3 = DistantLight(vec3(1.f,0.f,0.f), vec3(1.f,1.f,0.f));
	s.addLight(&light3);
	DistantLight sun = DistantLight(vec3(1.f, 1.f, 1.f), vec3(100,100,1));
	s.addLight(&sun);

	Image * img = initImage(WIDTH,HEIGHT);

	Raytracer r = Raytracer(s, img, 10, 4);
	r.renderScene();
	
	saveImage(img, argv[1]);
	freeImage(img);

	return 0;
}