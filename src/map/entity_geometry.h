#ifndef LIBMAP_ENTITY_GEOMETRY_H
#define LIBMAP_ENTITY_GEOMETRY_H

#include "vector.h"
#include <stdlib.h>

typedef struct LMVertexUV {
	double u;
	double v;
} LMVertexUV;

typedef struct LMVertexTangent {
	double x;
	double y;
	double z;
	double w;
} LMVertexTangent;

typedef struct LMFaceVertex {
	vec3 vertex;
	vec3 normal;
	LMVertexUV uv;
	LMVertexTangent tangent;
} LMFaceVertex;

typedef struct LMFaceGeometry {
	int vertex_count = 0;
	LMFaceVertex *vertices = NULL;
	int index_count = 0;
	int *indices = NULL;
} LMFaceGeometry;

typedef struct LMBrushGeometry {
	LMFaceGeometry *faces = NULL;
} LMBrushGeometry;

typedef struct LMEntityGeometry {
	LMBrushGeometry *brushes = NULL;
} LMEntityGeometry;

#endif
