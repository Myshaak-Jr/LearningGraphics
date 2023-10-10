#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

// VECTOR

typedef struct fvec2 {
	float x;
	float y;
} fvec2;
typedef struct fvec3 {
	float x;
	float y;
	float z;
} fvec3;


void fvec2_random(fvec2* vec);
float fvec2_dot(fvec2 v1, fvec2 v2);

fvec3 fvec3_scaled(float s, fvec3 v);
float fvec3_magnitude(fvec3 v);
fvec3 fvec3_normalize(fvec3 v);

// PERLIN NOISE

typedef struct Perlin {
	int w;
	int h;
	fvec2* vecMap;
} Perlin;

Perlin* createPerlinNoise(int w, int h);
void destroyPerlinNoise(Perlin* p);
float perlinAt(Perlin* p, float x, float y);

// ADDITIONAL FUNCTIONS

// Generates a random number using Gaussian/Normal distribution
float gauss(float mean, float stddev);
float smoothstep(float t);
float flerp(float a, float b, float t);

#define lerp(a, b, t) (a) + (t) * ((b) - (a))