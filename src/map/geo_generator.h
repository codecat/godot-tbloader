#ifndef LIBMAP_GEO_GENERATOR_H
#define LIBMAP_GEO_GENERATOR_H

#include "brush.h"
#include "libmap.h"
#include "vector.h"
#include "map_data.h"
#include <memory>

class LMGeoGenerator {
public:
	std::shared_ptr<LMMapData> map_data;
	void geo_generator_run();

	void generate_brush_vertices(int entity_idx, int brush_idx);
	bool intersect_faces(LMFace f0, LMFace f1, LMFace f2, vec3 *o_vertex);
	bool vertex_in_hull(LMFace *faces, int face_count, vec3 vertex);

	LMVertexUV get_standard_uv(vec3 vertex, const LMFace *face, int texture_width, int texture_height);
	LMVertexUV get_valve_uv(vec3 vertex, const LMFace *face, int texture_width, int texture_height);

	LMVertexTangent get_standard_tangent(const LMFace *face);
	LMVertexTangent get_valve_tangent(const LMFace *face);

	void geo_generator_print_entities();
	const LMEntityGeometry *geo_generator_get_entities();
	int geo_generator_get_brush_vertex_count(int entity_idx, int brush_idx);
	int geo_generator_get_brush_index_count(int entity_idx, int brush_idx);
	void geo_generator_get_brush_vertices(int entity_idx, int brush_idx, LMFaceVertex *o_vertices, int *o_indices);

	LMGeoGenerator(std::shared_ptr<LMMapData> _map_data) :
			map_data(_map_data) {}
};

#endif
