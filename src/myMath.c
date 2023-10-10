#include "myMath.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>


// VECTOR

void fvec2_random(fvec2* vec) {
	vec->x = gauss(0, 1);
	vec->y = gauss(0, 1);

	float mag = sqrtf(vec->x * vec->x + vec->y * vec->y);
	
	vec->x /= mag;
	vec->y /= mag;
}

float fvec2_dot(const fvec2 v1, const fvec2 v2) {
	return v1.x * v2.x + v1.y * v2.y;
}

fvec3 fvec3_scaled(float s, fvec3 v) {
	v.x *= s;
	v.y *= s;
	v.z *= s;
	return v;
}

float fvec3_magnitude(fvec3 v) {
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

fvec3 fvec3_normalize(fvec3 v) {
	return fvec3_scaled(1.0f / fvec3_magnitude(v), v);
}


// PERLIN NOISE

Perlin* createPerlinNoise(int w, int h) {
	Perlin* p = malloc(sizeof(Perlin));
	if (p == NULL) {
		fprintf(stderr, "Out of memory.");
		exit(1);
	}
	p->w = w;
	p->h = h;
	p->vecMap = calloc((size_t)w * h, sizeof(fvec2));
	if (p->vecMap == NULL) {
		fprintf(stderr, "Out of memory.");
		exit(1);
	}

	for (int i = 0; i < w * h; i++) {
		fvec2_random(&p->vecMap[i]);
	}

	return p;
}

void destroyPerlinNoise(Perlin* p) {
	free(p->vecMap);
	free(p);
}

float perlinAt(Perlin* p, float x, float y) {
	float fnx = x * (p->w - 1);
	float fny = y * (p->h - 1);

	int nx = floorf(fnx);
	int ny = floorf(fny);

	float rel_x = fnx - nx;
	float rel_y = fny - ny;

	float dots[4] = {
		fvec2_dot(p->vecMap[ny * p->w + nx], (fvec2) { rel_x, rel_y }),
		fvec2_dot(p->vecMap[ny * p->w + (nx + 1)], (fvec2) { rel_x - 1, rel_y }),
		fvec2_dot(p->vecMap[(ny + 1) * p->w + nx], (fvec2) { rel_x, rel_y - 1 }),
		fvec2_dot(p->vecMap[(ny + 1) * p->w + (nx + 1)], (fvec2) { rel_x - 1, rel_y - 1 })
	};

	float i1 = lerp(dots[0], dots[1], smoothstep(rel_x));
	float i2 = lerp(dots[2], dots[3], smoothstep(rel_x));
	return lerp(i1, i2, smoothstep(rel_y));
}

// ADDITIONAL FUNCTIONS

float gauss(float mean, float stddev) {
	static int has_spare = 0;
	static float spare;

	if (has_spare) {
		has_spare = 0;
		return spare * stddev + mean;
	}
	else {
		float u, v, s;
		do {
			u = (rand() / ((float)RAND_MAX)) * 2.0f - 1.0f;
			v = (rand() / ((float)RAND_MAX)) * 2.0f - 1.0f;
			s = u * u + v * v;
		} while (s >= 1.0f || s == 0.0f);

		float mul = sqrtf(-2.0f * logf(s) / s);
		spare = v * mul;
		has_spare = 1;

		return mean + stddev * u * mul;
	}
}

float smoothstep(float t) {
	return t * t * t * (t * (t * 6 - 15) + 10);
}

float flerp(float a, float b, float t) {
	return a + t * (b - a);
}
