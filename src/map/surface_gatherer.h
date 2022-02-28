#ifndef LIBMAP_SURFACE_GATHERER_H
#define LIBMAP_SURFACE_GATHERER_H

#include <stdint.h>

#include "entity_geometry.h"
#include "libmap.h"
#include "map_data.h"

#include <memory>

typedef struct LMSurface {
	int vertex_count = 0;
	LMFaceVertex *vertices = NULL;
	int index_count = 0;
	int *indices = NULL;
} LMSurface;

typedef struct LMSurfaces {
	int surface_count = 0;
	LMSurface *surfaces = NULL;
} LMSurfaces;

enum SURFACE_SPLIT_TYPE {
	SST_NONE,
	SST_ENTITY,
	SST_BRUSH
};

class LMSurfaceGatherer {
	std::shared_ptr<LMMapData> map_data;

public:
	LMSurfaceGatherer(std::shared_ptr<LMMapData> _map_data) :
			map_data(_map_data) {
	}

	SURFACE_SPLIT_TYPE split_type = SST_NONE;
	int entity_filter_idx = -1;
	int texture_filter_idx = -1;
	int brush_filter_texture_idx;
	int face_filter_texture_idx;
	bool filter_worldspawn_layers;

	LMSurfaces out_surfaces;

	void surface_gatherer_set_split_type(SURFACE_SPLIT_TYPE split_type);
	void surface_gatherer_set_brush_filter_texture(const char *texture_name);
	void surface_gatherer_set_face_filter_texture(const char *texture_name);
	void surface_gatherer_set_entity_index_filter(int entity_idx);
	void surface_gatherer_set_texture_filter(const char *texture_name);
	void surface_gatherer_set_worldspawn_layer_filter(bool filter);
	void surface_gatherer_run();
	const LMSurfaces *surface_gatherer_fetch();

	bool surface_gatherer_filter_entity(int entity_idx);
	bool surface_gatherer_filter_brush(int entity_idx, int brush_idx);
	bool surface_gatherer_filter_face(int entity_idx, int brush_idx, int face_idx);

	LMSurface *surface_gatherer_add_surface();
	void surface_gatherer_reset_state();
	void surface_gatherer_reset_params();
};

#endif
