#ifndef FACE_H
#define FACE_H

#include "stdbool.h"
#include "vector.h"

typedef struct LMFacePoints {
	vec3 v0;
	vec3 v1;
	vec3 v2;
} LMFacePoints;

typedef struct LMStandardUV {
	double u;
	double v;
} LMStandardUV;

typedef struct LMValveTextureAxis {
	vec3 axis;
	double offset;
} LMValveTextureAxis;

typedef struct LMValveUV {
	LMValveTextureAxis u;
	LMValveTextureAxis v;
} LMValveUV;

typedef struct LMFaceUVExtra {
	double rot;
	double scale_x;
	double scale_y;
} LMFaceUVExtra;

typedef struct LMFace {
	LMFacePoints plane_points;
	vec3 plane_normal;
	double plane_dist;

	int texture_idx;

	bool is_valve_uv;
	LMStandardUV uv_standard;
	LMValveUV uv_valve;
	LMFaceUVExtra uv_extra;
} face;

#endif