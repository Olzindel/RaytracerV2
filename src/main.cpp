#include "./GLM/glm/glm/glm.hpp"
#include "./Scene/Scene.hpp"
#include "./Raytracer/Raytracer.hpp"
#include "./Image.hpp"
#include "./Scene/Light/DistantLight.hpp"
#include "Scene/Object/Primitive/Sphere/Sphere.hpp"
#include "Scene/Object/Primitive/Plane/Plane.hpp"

#include <iostream>

#define WIDTH 1000
#define HEIGHT 750


/*Water waves

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
*/

//=====================================================================================================

/*Checked pattern
*/

inline float modulo(const float &x)
{
	return x - std::floor(x);
}

vec3 patternChecked(vec3 coordinates){
	float angle = M_PI/4.f;
	float s = coordinates.x * cos(angle) - coordinates.z * sin(angle);
	float t = coordinates.z * cos(angle) + coordinates.x * sin(angle);
	float scaleS = 5, scaleT = 5;
	float pattern = (modulo(s * scaleS) < 0.5) ^ (modulo(t * scaleT) < 0.5);
	return pattern * vec3(1.f);
}

//=====================================================================================================

/*Marble
*/

struct perlin
{
    int p[512];
    perlin(void);
    static perlin & getInstance(){static perlin instance; return instance;}
};

static int permutation[] = { 151,160,137,91,90,15,
   131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
   190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
   88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
   77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
   102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
   135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
   5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
   223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
   129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
   251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
   49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
   138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
   };

static double fade(double t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}
static double lerp(double t, double a, double b) {
    return a + t * (b - a);
}
static double grad(int hash, double x, double y, double z) {
    int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
    double u = h<8||h==12||h==13 ? x : y,   // INTO 12 GRADIENT DIRECTIONS.
            v = h<4||h==12||h==13 ? y : z;
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}
  
double noise(double x, double y, double z) {

    perlin & myPerlin = perlin::getInstance();
    int X = (int)floor(x) & 255,                  // FIND UNIT CUBE THAT
        Y = (int)floor(y) & 255,                  // CONTAINS POINT.
        Z = (int)floor(z) & 255;
    x -= floor(x);                                // FIND RELATIVE X,Y,Z
    y -= floor(y);                                // OF POINT IN CUBE.
    z -= floor(z);
    double u = fade(x),                                // COMPUTE FADE CURVES
            v = fade(y),                                // FOR EACH OF X,Y,Z.
            w = fade(z);
    int A = myPerlin.p[X  ]+Y, AA = myPerlin.p[A]+Z, AB = myPerlin.p[A+1]+Z,      // HASH COORDINATES OF
        B = myPerlin.p[X+1]+Y, BA = myPerlin.p[B]+Z, BB = myPerlin.p[B+1]+Z;      // THE 8 CUBE CORNERS,

    return lerp(w, lerp(v, lerp(u, grad(myPerlin.p[AA  ], x  , y  , z   ),  // AND ADD
                                    grad(myPerlin.p[BA  ], x-1, y  , z   )), // BLENDED
                            lerp(u, grad(myPerlin.p[AB  ], x  , y-1, z   ),  // RESULTS
                                    grad(myPerlin.p[BB  ], x-1, y-1, z   ))),// FROM  8
                    lerp(v, lerp(u, grad(myPerlin.p[AA+1], x  , y  , z-1 ),  // CORNERS
                                    grad(myPerlin.p[BA+1], x-1, y  , z-1 )), // OF CUBE
                            lerp(u, grad(myPerlin.p[AB+1], x  , y-1, z-1 ),
                                    grad(myPerlin.p[BB+1], x-1, y-1, z-1 ))));
}

perlin::perlin (void)
{
    for (int i=0; i < 256 ; i++) {
        p[256+i] = p[i] = permutation[i];
    }
}

vec3 water_bumpMapY(vec3 coordinates, vec3 normal){
	float eps = 1e-3;
	float X = normal.x * 50.f + coordinates.x * 100.f;
	float Y = normal.y + coordinates.y;
	float Z = normal.z * 50.f+ coordinates.z * 100.f;

	float x = (float)noise(X - eps, Y, Z) - (float)noise(X + eps, Y, Z); 
	float y = (float)noise(X, Y - eps,Z) - (float)noise(X,Y + eps, Z);
	float z = (float)noise(X, Y, Z - eps) - (float)noise(X, Y, Z + eps);
	return  vec3(normal.x + x, normal.y + y, normal.z + z);
}

double turbulence(double x, double y, double z, double size)
{
  double value = 0.0, initialSize = size;
  double coef = 55;

  while(size >= 1)
  {
    value += noise(x * coef / size, y * coef / size, z *coef / size) * size;
    size /= 2.0;
  }

  return(value / initialSize);
}

vec3 turbulence_text(vec3 coordinates){
	return vec3((float)turbulence(coordinates.x, coordinates.y , coordinates.z, 64));
}

//=====================================================================================================

int main(int argc, char *argv[]){
	if(argc < 2) {
		cout << "Need a image name";
		exit(1);
	}

	Camera c = Camera(vec3(0,0,0), vec3(0,0,1), vec3(0,1,0), 60, (float)WIDTH/(float)HEIGHT);
	// Scene s = Scene(c, vec3(0.f,0.4f,0.7f));
	Scene s = Scene(c, vec3(.4f,.7f,1.f));

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
	m.proceduralTexturingColor = &turbulence_text;
	Plane plane = Plane(vec3(0.,1, 0),0.2f);
	Object plane_ = Object(&plane, m);
	s.addObject(&plane_);

	m.diffuseColor = vec3(.2f,.6f, 0.8f);
	m.isDiffuse = false;
	m.isReflective = true;
	m.isRefractive = true;
	m.roughness = 0.4f;
	m.IOR = 1.33f;
	m.proceduralTexturingColor = NULL;
	m.bumpMap = &water_bumpMapY;
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


	// DistantLight light = DistantLight(vec3(0.f,0.f,1.f), vec3(0.f,0.f,2.f));
	// s.addLight(&light);
	// DistantLight light2 = DistantLight(vec3(0.f,1.f,0.f), vec3(1.f,0.f,1.f));
	// s.addLight(&light2);
	// DistantLight light3 = DistantLight(vec3(1.f,0.f,0.f), vec3(1.f,1.f,0.f));
	// s.addLight(&light3);
	DistantLight sun = DistantLight(vec3(1.f, 1.f, 1.f), vec3(0,100,1));
	s.addLight(&sun);

	Image * img = initImage(WIDTH,HEIGHT);

	Raytracer r = Raytracer(s, img, 10, 3);
	r.renderScene();
	
	saveImage(img, argv[1]);
	freeImage(img);

	return 0;
}