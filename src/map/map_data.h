#ifndef LIBMAP_MAP_DATA_H
#define LIBMAP_MAP_DATA_H

#include "entity.h"
#include "entity_geometry.h"
#include "libmap.h"
#include <stdlib.h>

typedef struct LMTextureData {
	char *name;
	int width;
	int height;
} LMTextureData;

typedef struct LMWorldspawnLayer {
	int texture_idx;
	bool build_visuals;
} LMWorldspawnLayer;

class LMMapData {
public:
	int entity_count = 0;
	LMEntity *entities = NULL;
	LMEntityGeometry *entity_geo = NULL;

	int texture_count = 0;
	LMTextureData *textures = NULL;

	int worldspawn_layer_count = 0;
	LMWorldspawnLayer *worldspawn_layers = NULL;
	void map_data_register_worldspawn_layer(const char *name, bool build_visuals);
	int map_data_find_worldspawn_layer(int texture_idx);
	int map_data_get_worldspawn_layer_count();
	LMWorldspawnLayer *map_data_get_worldspawn_layers();

	void map_data_set_texture_size(const char *name, int width, int height);
	int map_data_get_texture_count();
	LMTextureData *map_data_get_textures();
	LMTextureData *map_data_get_texture(int texture_idx);

	void map_data_set_spawn_type_by_classname(const char *key, int spawn_type);

	void map_data_print_entities();
	int map_data_get_entity_count();
	const LMEntity *map_data_get_entities();

	LMMapData();
	void map_data_reset();
	int map_data_register_texture(const char *name);
	int map_data_find_texture(const char *texture_name);
	const char *map_data_get_entity_property(int entity_idx, const char *key);
};

#endif
