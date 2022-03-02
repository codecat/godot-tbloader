#ifndef LIBMAP_ENTITY_H
#define LIBMAP_ENTITY_H

#include "vector.h"
#include <stdlib.h>

typedef struct LMBrush LMBrush;

enum ENTITY_SPAWN_TYPE {
	EST_WORLDSPAWN = 0,
	EST_MERGE_WORLDSPAWN = 1,
	EST_ENTITY = 2,
	EST_GROUP = 3
};

typedef struct LMProperty {
	char *key = NULL;
	char *value = NULL;
} LMProperty;

class LMEntity {
public:
	int property_count = 0;
	LMProperty *properties = NULL;

	int brush_count = 0;
	LMBrush *brushes = NULL;

	vec3 center;
	enum ENTITY_SPAWN_TYPE spawn_type;

	const char* get_property(const char* key, const char* def = "");
	int get_property_int(const char* key, int def = 0);
	float get_property_float(const char* key, float def = 0.0f);
	double get_property_double(const char* key, double def = 0.0);
	vec3 get_property_vec3(const char* key, const vec3& def = { 0 });
};

#endif