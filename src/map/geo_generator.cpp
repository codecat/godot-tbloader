#include "geo_generator.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "brush.h"
#include "entity.h"
#include "face.h"
#include "libmap_math.h"

const vec3 UP_VECTOR = { 0.0, 0.0, 1.0 };
const vec3 RIGHT_VECTOR = { 0.0, 1.0, 0.0 };
const vec3 FORWARD_VECTOR = { 1.0, 0.0, 0.0 };

bool smooth_normals = false;

int wind_entity_idx = 0;
int wind_brush_idx = 0;
int wind_face_idx = 0;
vec3 wind_face_center;
vec3 wind_face_basis;
vec3 wind_face_normal;

static LMMapData *sort_map_data;
int sort_vertices_by_winding(const void *lhs_in, const void *rhs_in) {
	const vec3 *lhs = (const vec3 *)lhs_in;
	const vec3 *rhs = (const vec3 *)rhs_in;

	face *face_inst = &sort_map_data->entities[wind_entity_idx].brushes[wind_brush_idx].faces[wind_face_idx];
	LMFaceGeometry *face_geo_inst = &sort_map_data->entity_geo[wind_entity_idx].brushes[wind_brush_idx].faces[wind_face_idx];

	vec3 u = vec3_normalize(wind_face_basis);
	vec3 v = vec3_normalize(vec3_cross(u, wind_face_normal));

	vec3 local_lhs = vec3_sub(*lhs, wind_face_center);
	double lhs_pu = vec3_dot(local_lhs, u);
	double lhs_pv = vec3_dot(local_lhs, v);

	vec3 local_rhs = vec3_sub(*rhs, wind_face_center);
	double rhs_pu = vec3_dot(local_rhs, u);
	double rhs_pv = vec3_dot(local_rhs, v);

	double lhs_angle = atan2(lhs_pv, lhs_pu);
	double rhs_angle = atan2(rhs_pv, rhs_pu);

	if (lhs_angle < rhs_angle) {
		return -1;
	} else if (lhs_angle > rhs_angle) {
		return 1;
	}

	return 0;
}

void LMGeoGenerator::geo_generator_run() {
	map_data->entity_geo = (LMEntityGeometry *)malloc(map_data->entity_count * sizeof(LMEntityGeometry));

	for (int e = 0; e < map_data->entity_count; ++e) {
		LMEntity *ent_inst = &map_data->entities[e];

		LMEntityGeometry *entity_geo_inst = &map_data->entity_geo[e];
		*entity_geo_inst = { 0 };

		entity_geo_inst->brushes = (LMBrushGeometry *)malloc(ent_inst->brush_count * sizeof(LMBrushGeometry));

		for (int b = 0; b < ent_inst->brush_count; ++b) {
			LMBrush *brush_inst = &ent_inst->brushes[b];

			LMBrushGeometry *brush_geo_inst = &entity_geo_inst->brushes[b];
			*brush_geo_inst = { 0 };

			brush_geo_inst->faces = (LMFaceGeometry *)malloc(brush_inst->face_count * sizeof(LMFaceGeometry));

			for (int f = 0; f < brush_inst->face_count; ++f) {
				LMFaceGeometry *face_geo_inst = &brush_geo_inst->faces[f];
				*face_geo_inst = { 0 };
			}
		}
	}

	for (int e = 0; e < map_data->entity_count; ++e) {
		LMEntity *ent_inst = &map_data->entities[e];
		ent_inst->center = { 0.0, 0.0, 0.0 };

		for (int b = 0; b < ent_inst->brush_count; ++b) {
			LMBrush *brush_inst = &ent_inst->brushes[b];
			brush_inst->center = { 0.0, 0.0, 0.0 };
			int vert_count = 0;

			generate_brush_vertices(e, b);

			LMBrushGeometry *brush_geo_inst = &map_data->entity_geo[e].brushes[b];
			for (int f = 0; f < brush_inst->face_count; f++) {
				LMFaceGeometry *face_geo_inst = &brush_geo_inst->faces[f];

				for (int v = 0; v < face_geo_inst->vertex_count; ++v) {
					brush_inst->center = vec3_add(brush_inst->center, face_geo_inst->vertices[v].vertex);
					vert_count++;
				}
			}

			if (vert_count > 0) {
				brush_inst->center = vec3_div_double(brush_inst->center, vert_count);
			}

			ent_inst->center = vec3_add(ent_inst->center, brush_inst->center);
		}

		if (ent_inst->brush_count > 0) {
			ent_inst->center = vec3_div_double(ent_inst->center, ent_inst->brush_count);
		}
	}

	// Wind face vertices
	for (int e = 0; e < map_data->entity_count; ++e) {
		LMEntity *entity_inst = &map_data->entities[e];
		LMEntityGeometry *entity_geo_inst = &map_data->entity_geo[e];
		for (int b = 0; b < entity_inst->brush_count; ++b) {
			LMBrush *brush_inst = &entity_inst->brushes[b];
			LMBrushGeometry *brush_geo_inst = &entity_geo_inst->brushes[b];

			for (int f = 0; f < brush_inst->face_count; ++f) {
				face *face_inst = &brush_inst->faces[f];
				LMFaceGeometry *face_geo_inst = &brush_geo_inst->faces[f];

				if (face_geo_inst->vertex_count < 3) {
					continue;
				}

				wind_entity_idx = e;
				wind_brush_idx = b;
				wind_face_idx = f;

				wind_face_basis = vec3_sub(face_geo_inst->vertices[1].vertex, face_geo_inst->vertices[0].vertex);
				wind_face_center = { 0 };
				wind_face_normal = face_inst->plane_normal;

				for (int v = 0; v < face_geo_inst->vertex_count; ++v) {
					wind_face_center = vec3_add(wind_face_center, face_geo_inst->vertices[v].vertex);
				}

				wind_face_center = vec3_div_double(wind_face_center, face_geo_inst->vertex_count);

				sort_map_data = map_data.get();
				qsort(face_geo_inst->vertices, face_geo_inst->vertex_count, sizeof(LMFaceVertex), sort_vertices_by_winding);

				wind_entity_idx = 0;
			}
		}
	}

	// Index face vertices
	for (int e = 0; e < map_data->entity_count; ++e) {
		LMEntity *entity_inst = &map_data->entities[e];
		LMEntityGeometry *entity_geo_inst = &map_data->entity_geo[e];
		for (int b = 0; b < entity_inst->brush_count; ++b) {
			LMBrush *brush_inst = &entity_inst->brushes[b];
			LMBrushGeometry *brush_geo_inst = &entity_geo_inst->brushes[b];

			for (int f = 0; f < brush_inst->face_count; ++f) {
				LMFaceGeometry *face_geo_inst = &brush_geo_inst->faces[f];

				if (face_geo_inst->vertex_count < 3) {
					continue;
				}

				face_geo_inst->indices = (int *)malloc((face_geo_inst->vertex_count - 2) * 3 * sizeof(int));
				for (int i = 0; i < face_geo_inst->vertex_count - 2; i++) {
					face_geo_inst->indices[face_geo_inst->index_count++] = 0;
					face_geo_inst->indices[face_geo_inst->index_count++] = i + 1;
					face_geo_inst->indices[face_geo_inst->index_count++] = i + 2;
				}
			}
		}
	}
}

void LMGeoGenerator::generate_brush_vertices(int entity_idx, int brush_idx) {
	LMEntity *ent_inst = &map_data->entities[entity_idx];
	LMBrush *brush_inst = &ent_inst->brushes[brush_idx];

	for (int f0 = 0; f0 < brush_inst->face_count; ++f0) {
		for (int f1 = 0; f1 < brush_inst->face_count; ++f1) {
			for (int f2 = 0; f2 < brush_inst->face_count; ++f2) {
				vec3 vertex = { 0 };
				if (intersect_faces(brush_inst->faces[f0], brush_inst->faces[f1], brush_inst->faces[f2], &vertex)) {
					if (vertex_in_hull(brush_inst->faces, brush_inst->face_count, vertex)) {
						face *face_inst = &map_data->entities[entity_idx].brushes[brush_idx].faces[f0];
						LMFaceGeometry *face_geo_inst = &map_data->entity_geo[entity_idx].brushes[brush_idx].faces[f0];

						vec3 normal;

						const char *phong_property = map_data->map_data_get_entity_property(entity_idx, "_phong");
						bool phong = phong_property != NULL && strcmp(phong_property, "1") == 0;
						if (phong) {
							const char *phong_angle_property = map_data->map_data_get_entity_property(entity_idx, "_phong_angle");
							if (phong_angle_property != NULL) {
								double threshold = cos((atof(phong_angle_property) + 0.01) * 0.0174533);
								normal = brush_inst->faces[f0].plane_normal;
								if (vec3_dot(brush_inst->faces[f0].plane_normal, brush_inst->faces[f1].plane_normal) > threshold) {
									normal = vec3_add(normal, brush_inst->faces[f1].plane_normal);
								}
								if (vec3_dot(brush_inst->faces[f0].plane_normal, brush_inst->faces[f2].plane_normal) > threshold) {
									normal = vec3_add(normal, brush_inst->faces[f2].plane_normal);
								}
								normal = vec3_normalize(normal);
							} else {
								normal = vec3_normalize(
										vec3_add(
												brush_inst->faces[f0].plane_normal,
												vec3_add(
														brush_inst->faces[f1].plane_normal,
														brush_inst->faces[f2].plane_normal)));
							}
						} else {
							normal = face_inst->plane_normal;
						}

						LMTextureData *texture = map_data->map_data_get_texture(face_inst->texture_idx);

						LMVertexUV uv;
						if (face_inst->is_valve_uv) {
							uv = get_valve_uv(vertex, face_inst, texture->width, texture->height);
						} else {
							uv = get_standard_uv(vertex, face_inst, texture->width, texture->height);
						}

						LMVertexTangent tangent;
						if (face_inst->is_valve_uv) {
							tangent = get_valve_tangent(face_inst);
						} else {
							tangent = get_standard_tangent(face_inst);
						}

						bool unique_vertex = true;
						int duplicate_index = -1;

						for (int v = 0; v < face_geo_inst->vertex_count; ++v) {
							vec3 comp_vertex = face_geo_inst->vertices[v].vertex;
							if (vec3_length(vec3_sub(vertex, comp_vertex)) < CMP_EPSILON) {
								unique_vertex = false;
								duplicate_index = v;
								break;
							}
						}

						if (unique_vertex) {
							face_geo_inst->vertex_count++;
							face_geo_inst->vertices = (LMFaceVertex *)realloc(face_geo_inst->vertices, face_geo_inst->vertex_count * sizeof(LMFaceVertex));
							face_geo_inst->vertices[face_geo_inst->vertex_count - 1] = { vertex, normal, uv, tangent };
						} else if (phong) {
							face_geo_inst->vertices[duplicate_index].normal = vec3_add(face_geo_inst->vertices[duplicate_index].normal, normal);
						}
					}
				}
			}
		}
	}

	for (int f = 0; f < brush_inst->face_count; ++f) {
		LMFaceGeometry *face_geo_inst = &map_data->entity_geo[entity_idx].brushes[brush_idx].faces[f];

		for (int v = 0; v < face_geo_inst->vertex_count; ++v) {
			face_geo_inst->vertices[v].normal = vec3_normalize(face_geo_inst->vertices[v].normal);
		}
	}
}

bool LMGeoGenerator::intersect_faces(face f0, face f1, face f2, vec3 *o_vertex) {
	vec3 normal0 = f0.plane_normal;
	vec3 normal1 = f1.plane_normal;
	vec3 normal2 = f2.plane_normal;

	double denom = vec3_dot(vec3_cross(normal0, normal1), normal2);

	if (denom < CMP_EPSILON) {
		return false;
	}

	if (o_vertex) {
		*o_vertex = vec3_div_double(
				vec3_add(
						vec3_add(
								vec3_mul_double(
										vec3_cross(normal1, normal2),
										f0.plane_dist),
								vec3_mul_double(
										vec3_cross(normal2, normal0),
										f1.plane_dist)),
						vec3_mul_double(
								vec3_cross(normal0, normal1),
								f2.plane_dist)),
				denom);
	}

	return true;
}

bool LMGeoGenerator::vertex_in_hull(face *faces, int face_count, vec3 vertex) {
	for (int f = 0; f < face_count; f++) {
		face face_inst = faces[f];

		double proj = vec3_dot(face_inst.plane_normal, vertex);

		if (proj > face_inst.plane_dist && fabs(face_inst.plane_dist - proj) > CMP_EPSILON) {
			return false;
		}
	}

	return true;
}

LMVertexUV LMGeoGenerator::get_standard_uv(vec3 vertex, const face *face, int texture_width, int texture_height) {
	LMVertexUV uv_out;

	double du = fabs(vec3_dot(face->plane_normal, UP_VECTOR));
	double dr = fabs(vec3_dot(face->plane_normal, RIGHT_VECTOR));
	double df = fabs(vec3_dot(face->plane_normal, FORWARD_VECTOR));

	if (du >= dr && du >= df) {
		uv_out = { vertex.x, -vertex.y };
	} else if (dr >= du && dr >= df) {
		uv_out = { vertex.x, -vertex.z };
	} else if (df >= du && df >= dr) {
		uv_out = { vertex.y, -vertex.z };
	}

	LMVertexUV rotated;
	double angle = DEG_TO_RAD(face->uv_extra.rot);
	rotated.u = uv_out.u * cos(angle) - uv_out.v * sin(angle);
	rotated.v = uv_out.u * sin(angle) + uv_out.v * cos(angle);
	uv_out = rotated;

	uv_out.u /= texture_width;
	uv_out.v /= texture_height;

	uv_out.u /= face->uv_extra.scale_x;
	uv_out.v /= face->uv_extra.scale_y;

	uv_out.u += face->uv_standard.u / texture_width;
	uv_out.v += face->uv_standard.v / texture_height;

	return uv_out;
}

LMVertexUV LMGeoGenerator::get_valve_uv(vec3 vertex, const face *face, int texture_width, int texture_height) {
	LMVertexUV uv_out;

	vec3 u_axis = face->uv_valve.u.axis;
	double u_shift = face->uv_valve.u.offset;
	vec3 v_axis = face->uv_valve.v.axis;
	double v_shift = face->uv_valve.v.offset;

	uv_out.u = vec3_dot(u_axis, vertex);
	uv_out.v = vec3_dot(v_axis, vertex);

	uv_out.u /= texture_width;
	uv_out.v /= texture_height;

	uv_out.u /= face->uv_extra.scale_x;
	uv_out.v /= face->uv_extra.scale_y;

	uv_out.u += u_shift / texture_width;
	uv_out.v += v_shift / texture_height;

	return uv_out;
}

double sign(double v) {
	if (v > 0) {
		return 1.0;
	} else if (v < 0) {
		return -1.0;
	}

	return 0.0;
}

LMVertexTangent LMGeoGenerator::get_standard_tangent(const face *face) {
	LMVertexTangent tangent_out;

	double du = vec3_dot(face->plane_normal, UP_VECTOR);
	double dr = vec3_dot(face->plane_normal, RIGHT_VECTOR);
	double df = vec3_dot(face->plane_normal, FORWARD_VECTOR);

	double dua = fabs(du);
	double dra = fabs(dr);
	double dfa = fabs(df);

	vec3 u_axis;
	double v_sign = 0;

	if (dua >= dra && dua >= dfa) {
		u_axis = FORWARD_VECTOR;
		v_sign = sign(du);
	} else if (dra >= dua && dra >= dfa) {
		u_axis = FORWARD_VECTOR;
		v_sign = -sign(dr);
	} else if (dfa >= dua && dfa >= dra) {
		u_axis = RIGHT_VECTOR;
		v_sign = sign(df);
	}

	v_sign *= sign(face->uv_extra.scale_y);
	u_axis = vec3_rotate(u_axis, face->plane_normal, -face->uv_extra.rot * v_sign);

	tangent_out.x = u_axis.x;
	tangent_out.y = u_axis.y;
	tangent_out.z = u_axis.z;
	tangent_out.w = v_sign;

	return tangent_out;
}

LMVertexTangent LMGeoGenerator::get_valve_tangent(const face *face) {
	LMVertexTangent tangent_out;

	vec3 u_axis = vec3_normalize(face->uv_valve.u.axis);
	vec3 v_axis = vec3_normalize(face->uv_valve.v.axis);

	double v_sign = -sign(vec3_dot(vec3_cross(face->plane_normal, u_axis), v_axis));

	tangent_out.x = u_axis.x;
	tangent_out.y = u_axis.y;
	tangent_out.z = u_axis.z;
	tangent_out.w = v_sign;

	return tangent_out;
}

void LMGeoGenerator::geo_generator_print_entities() {
	for (int e = 0; e < map_data->entity_count; ++e) {
		LMEntity *entity_inst = &map_data->entities[e];
		LMEntityGeometry *entity_geo_inst = &map_data->entity_geo[e];
		printf("Entity %d\n", e);
		for (int b = 0; b < entity_inst->brush_count; ++b) {
			LMBrush *brush_inst = &entity_inst->brushes[b];
			LMBrushGeometry *brush_geo_inst = &entity_geo_inst->brushes[b];
			printf("Brush %d\n", b);

			for (int f = 0; f < brush_inst->face_count; ++f) {
				LMFaceGeometry *face_geo_inst = &brush_geo_inst->faces[f];
				printf("Face %d\n", f);
				for (int i = 0; i < face_geo_inst->vertex_count; ++i) {
					LMFaceVertex vertex = face_geo_inst->vertices[i];
					printf("vertex: (%f %f %f), normal: (%f %f %f)\n",
							vertex.vertex.x, vertex.vertex.y, vertex.vertex.z,
							vertex.normal.x, vertex.normal.y, vertex.normal.z);
				}

				puts("Indices:");
				for (int i = 0; i < (face_geo_inst->vertex_count - 2) * 3; ++i) {
					printf("index: %d\n", face_geo_inst->indices[i]);
				}
			}

			putchar('\n');
			putchar('\n');
		}
	}
}

const LMEntityGeometry *LMGeoGenerator::geo_generator_get_entities() {
	return map_data->entity_geo;
}

int LMGeoGenerator::geo_generator_get_brush_vertex_count(int entity_idx, int brush_idx) {
	int vertex_count = 0;

	LMBrush *brush_inst = &map_data->entities[entity_idx].brushes[brush_idx];
	LMBrushGeometry *brush_geo_inst = &map_data->entity_geo[entity_idx].brushes[brush_idx];

	for (int i = 0; i < brush_inst->face_count; ++i) {
		LMFaceGeometry *face_geo_inst = &brush_geo_inst->faces[i];
		vertex_count = vertex_count + face_geo_inst->vertex_count;
	}

	return vertex_count;
}

int LMGeoGenerator::geo_generator_get_brush_index_count(int entity_idx, int brush_idx) {
	int index_count = 0;

	LMBrush *brush_inst = &map_data->entities[entity_idx].brushes[brush_idx];
	LMBrushGeometry *brush_geo_inst = &map_data->entity_geo[entity_idx].brushes[brush_idx];

	for (int i = 0; i < brush_inst->face_count; ++i) {
		LMFaceGeometry *face_geo_inst = &brush_geo_inst->faces[i];
		index_count = index_count + face_geo_inst->index_count;
	}

	return index_count;
}
