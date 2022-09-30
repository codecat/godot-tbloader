#include "map_parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "brush.h"
#include "entity.h"
#include "face.h"
#include "map_data.h"
#include "platform.h"

#define DEBUG false

void LMMapParser::reset_current_face() {
	current_face = { 0 };
}

void LMMapParser::reset_current_brush() {
	if (current_brush.faces != NULL) {
		free(current_brush.faces);
		current_brush.faces = NULL;
	}

	current_brush.face_count = 0;
}

void LMMapParser::reset_current_entity() {
	for (int i = 0; i < current_entity.brush_count; ++i) {
		if (current_entity.brushes[i].faces != NULL) {
			free(current_entity.brushes[i].faces);
			current_entity.brushes[i].faces = NULL;
		}
	}

	if (current_entity.properties != NULL) {
		for (int p = 0; p < current_entity.property_count; ++p) {
			if (current_entity.properties[p].key != NULL) {
				free(current_entity.properties[p].key);
			}

			if (current_entity.properties[p].value != NULL) {
				free(current_entity.properties[p].value);
			}
		}

		free(current_entity.properties);
		current_entity.properties = NULL;
	}

	current_entity.property_count = 0;

	if (current_entity.brushes != NULL) {
		free(current_entity.brushes);
		current_entity.brushes = NULL;
	}

	current_entity.brush_count = 0;
}

bool LMMapParser::load_from_path(const char *map_file) {
	map_data->map_data_reset();

	reset_current_face();
	reset_current_brush();
	reset_current_entity();

	scope = PS_FILE;
	comment = false;
	entity_idx = -1;
	brush_idx = -1;
	face_idx = -1;
	component_idx = 0;
	valve_uvs = false;

	FILE *map = fopen(map_file, "r");

	if (!map) {
		printf("Error: Failed to open map file.\n");
		return false;
	}

	int c;
	char buf[255];
	int buf_head = 0;
	while ((c = fgetc(map)) != EOF) {
		if (c == '\n') {
			buf[buf_head] = '\0';
			token(buf);
			buf_head = 0;

			newline();
		} else if (isspace(c)) {
			buf[buf_head] = '\0';
			token(buf);
			buf_head = 0;
		} else {
			buf[buf_head++] = c;
		}
	}

	fclose(map);

	return true;
}

void LMMapParser::load_from_godot_file(godot::Ref<godot::FileAccess> f) {
	map_data->map_data_reset();

	reset_current_face();
	reset_current_brush();
	reset_current_entity();

	scope = PS_FILE;
	comment = false;
	entity_idx = -1;
	brush_idx = -1;
	face_idx = -1;
	component_idx = 0;
	valve_uvs = false;

	int c;
	char buf[255];
	int buf_head = 0;
	while (!f->eof_reached()) {
		c = (int)f->get_8();

		if (c == '\n') {
			buf[buf_head] = '\0';
			token(buf);
			buf_head = 0;

			newline();
		} else if (isspace(c)) {
			buf[buf_head] = '\0';
			token(buf);
			buf_head = 0;
		} else {
			buf[buf_head++] = c;
		}
	}
}

void LMMapParser::set_scope(PARSE_SCOPE new_scope) {
#if DEBUG
	switch (new_scope) {
		case PS_FILE:
			puts("Switching to file scope\n");
			break;
		case PS_ENTITY:
			printf("Switching to entity %d scope\n", entity_idx);
			break;
		case PS_PROPERTY_VALUE:
			puts("Switching to property value scope\n");
			break;
		case PS_BRUSH:
			printf("Switching to brush %d scope\n", brush_idx);
			break;
		case PS_PLANE_0:
			printf("Switching to face %d plane 0 scope\n", face_idx);
			break;
		case PS_PLANE_1:
			printf("Switching to face %d plane 1 scope\n", face_idx);
			break;
		case PS_PLANE_2:
			printf("Switching to face %d plane 2 scope\n", face_idx);
			break;
		case PS_TEXTURE:
			puts("Switching to texture scope\n");
			break;
		case PS_U:
			puts("Switching to U scope\n");
			break;
		case PS_V:
			puts("Switching to V scope\n");
			break;
		case PS_VALVE_U:
			puts("Switching to Valve U scope\n");
			break;
		case PS_VALVE_V:
			puts("Switching to Valve V scope\n");
			break;
		case PS_ROT:
			puts("Switching to rotation scope\n");
			break;
		case PS_U_SCALE:
			puts("Switching to U scale scope\n");
			break;
		case PS_V_SCALE:
			puts("Switching to V scale scope\n");
			break;
	}

#endif
	scope = new_scope;
}

bool LMMapParser::strings_match(const char *lhs, const char *rhs) {
	return strcmp(lhs, rhs) == 0;
}

void LMMapParser::token(const char *buf) {
	LMProperty *prop = NULL;

	if (comment) {
		return;
	} else if (strings_match(buf, "//")) {
		comment = true;
		return;
	}

#if DEBUG
	puts(buf);
#endif

	switch (scope) {
		case PS_FILE: {
			if (strings_match(buf, "{")) {
				entity_idx++;
				brush_idx = -1;
				set_scope(PS_ENTITY);
			}
			break;
		}
		case PS_ENTITY: {
			if (buf[0] == '"') {
				current_entity.properties = (LMProperty *)realloc(current_entity.properties, (current_entity.property_count + 1) * sizeof(LMProperty));
				prop = &current_entity.properties[current_entity.property_count];
				*prop = { 0 };
				prop->key = STRDUP(&buf[1]);

				size_t last = strlen(prop->key) - 1;
				if (prop->key[last] == '"') {
					prop->key[strlen(prop->key) - 1] = '\0';
					set_scope(PS_PROPERTY_VALUE);
				}
			} else if (strings_match(buf, "{")) {
				brush_idx++;
				face_idx = -1;
				set_scope(PS_BRUSH);
			} else if (strings_match(buf, "}")) {
				commit_entity();
				set_scope(PS_FILE);
			}
			break;
		}
		case PS_PROPERTY_VALUE: {
			prop = &current_entity.properties[current_entity.property_count];

			size_t current_length = 0;
			if (current_property != NULL) {
				current_length = strlen(current_property);
			}

			size_t buf_length = strlen(buf);

			bool is_first, is_last;
			if (buf_length == 1 && buf[0] == '"') {
				is_first = current_length == 0;
				is_last = !is_first;
			} else if (buf_length == 0) {
				is_first = is_last = false;
			} else {
				is_first = buf[0] == '"';
				is_last = buf[buf_length - 1] == '"';
			}

			if (!is_first && is_last) {
				current_property = (char *)realloc(current_property, current_length + buf_length + 2);
				current_property[current_length] = ' ';
				memcpy(&current_property[current_length + 1], buf, buf_length + 1);

			} else if (is_first || is_last) {
				current_property = (char *)realloc(current_property, current_length + buf_length + 1);
				memcpy(&current_property[current_length], buf, buf_length + 1);

			} else {
				current_property = (char *)realloc(current_property, current_length + buf_length + 2);
				current_property[current_length] = ' ';
				if (buf_length > 0) {
					memcpy(&current_property[current_length + 1], buf, buf_length);
				}
				current_property[current_length + buf_length + 1] = '\0';
			}

			if (is_last) {
				prop->value = STRDUP(&current_property[1]);
				prop->value[strlen(prop->value) - 1] = '\0';
				current_entity.property_count++;
				set_scope(PS_ENTITY);

				free(current_property);
				current_property = NULL;
			}
			break;
		}
		case PS_BRUSH: {
			if (strings_match(buf, "(")) {
				face_idx++;
				component_idx = 0;
				set_scope(PS_PLANE_0);
			} else if (strings_match(buf, "}")) {
				commit_brush();
				set_scope(PS_ENTITY);
			}
			break;
		}
		case PS_PLANE_0: {
			if (strings_match(buf, ")")) {
				component_idx = 0;
				set_scope(PS_PLANE_1);
			} else {
				switch (component_idx) {
					case 0:
						current_face.plane_points.v0.x = atof(buf);
						break;
					case 1:
						current_face.plane_points.v0.y = atof(buf);
						break;
					case 2:
						current_face.plane_points.v0.z = atof(buf);
						break;
					default:
						break;
				}
				component_idx++;
			}
			break;
		}
		case PS_PLANE_1: {
			if (strings_match(buf, "(")) {
				break;
			} else if (strings_match(buf, ")")) {
				component_idx = 0;
				set_scope(PS_PLANE_2);
			} else {
				switch (component_idx) {
					case 0:
						current_face.plane_points.v1.x = atof(buf);
						break;
					case 1:
						current_face.plane_points.v1.y = atof(buf);
						break;
					case 2:
						current_face.plane_points.v1.z = atof(buf);
						break;
					default:
						break;
				}
				component_idx++;
			}
			break;
		}
		case PS_PLANE_2: {
			if (strings_match(buf, "(")) {
				break;
			} else if (strings_match(buf, ")")) {
				set_scope(PS_TEXTURE);
			} else {
				switch (component_idx) {
					case 0:
						current_face.plane_points.v2.x = atof(buf);
						break;
					case 1:
						current_face.plane_points.v2.y = atof(buf);
						break;
					case 2:
						current_face.plane_points.v2.z = atof(buf);
						break;
					default:
						break;
				}
				component_idx++;
			}
			break;
		}
		case PS_TEXTURE: {
			current_face.texture_idx = map_data->map_data_register_texture(buf);
			set_scope(PS_U);
			break;
		}
		case PS_U: {
			if (strings_match(buf, "[")) {
				valve_uvs = true;
				component_idx = 0;
				set_scope(PS_VALVE_U);
			} else {
				valve_uvs = false;
				current_face.uv_standard.u = atof(buf);
				set_scope(PS_V);
			}
			break;
		}
		case PS_V: {
			current_face.uv_standard.v = atof(buf);
			set_scope(PS_ROT);
			break;
		}
		case PS_VALVE_U: {
			if (strings_match(buf, "]")) {
				component_idx = 0;
				set_scope(PS_VALVE_V);
			} else {
				switch (component_idx) {
					case 0:
						current_face.uv_valve.u.axis.x = atof(buf);
						break;
					case 1:
						current_face.uv_valve.u.axis.y = atof(buf);
						break;
					case 2:
						current_face.uv_valve.u.axis.z = atof(buf);
						break;
					case 3:
						current_face.uv_valve.u.offset = atof(buf);
						break;
					default:
						break;
				}

				component_idx++;
			}
			break;
		}
		case PS_VALVE_V: {
			if (strings_match(buf, "[")) {
				break;
			} else if (strings_match(buf, "]")) {
				set_scope(PS_ROT);
			} else {
				switch (component_idx) {
					case 0:
						current_face.uv_valve.v.axis.x = atof(buf);
						break;
					case 1:
						current_face.uv_valve.v.axis.y = atof(buf);
						break;
					case 2:
						current_face.uv_valve.v.axis.z = atof(buf);
						break;
					case 3:
						current_face.uv_valve.v.offset = atof(buf);
						break;
					default:
						break;
				}

				component_idx++;
			}
			break;
		}
		case PS_ROT: {
			current_face.uv_extra.rot = atof(buf);
			set_scope(PS_U_SCALE);
			break;
		}
		case PS_U_SCALE: {
			current_face.uv_extra.scale_x = atof(buf);
			set_scope(PS_V_SCALE);
			break;
		}
		case PS_V_SCALE: {
			current_face.uv_extra.scale_y = atof(buf);

			commit_face();

			set_scope(PS_BRUSH);
			break;
		}
		default:
			break;
	}
}

void LMMapParser::newline() {
	if (comment) {
		comment = false;
	}
}

void LMMapParser::commit_face() {
	vec3 v0v1 = vec3_sub(current_face.plane_points.v1, current_face.plane_points.v0);
	vec3 v1v2 = vec3_sub(current_face.plane_points.v2, current_face.plane_points.v1);
	current_face.plane_normal = vec3_normalize(vec3_cross(v1v2, v0v1));
	current_face.plane_dist = vec3_dot(current_face.plane_normal, current_face.plane_points.v0);
	current_face.is_valve_uv = valve_uvs;

	current_brush.face_count++;
	current_brush.faces = (LMFace *)realloc(current_brush.faces, current_brush.face_count * sizeof(LMFace));
	current_brush.faces[current_brush.face_count - 1] = current_face;

	reset_current_face();
}

void LMMapParser::commit_brush() {
	current_entity.brush_count++;
	current_entity.brushes = (LMBrush *)realloc(current_entity.brushes, current_entity.brush_count * sizeof(LMBrush));

	LMBrush *dest_brush = &current_entity.brushes[current_entity.brush_count - 1];
	*dest_brush = { 0 };

	dest_brush->face_count = current_brush.face_count;
	dest_brush->faces = (LMFace *)realloc(dest_brush->faces, dest_brush->face_count * sizeof(LMFace));
	for (int i = 0; i < dest_brush->face_count; ++i) {
		dest_brush->faces[i] = current_brush.faces[i];
	}

	reset_current_brush();
}

void LMMapParser::commit_entity() {
	map_data->entity_count++;
	map_data->entities = (LMEntity *)realloc(map_data->entities, map_data->entity_count * sizeof(LMEntity));

	LMEntity *dest_entity = &map_data->entities[map_data->entity_count - 1];
	*dest_entity = { 0 };
	dest_entity->spawn_type = EST_ENTITY;

	dest_entity->property_count = current_entity.property_count;
	dest_entity->properties = (LMProperty *)realloc(dest_entity->properties, dest_entity->property_count * sizeof(LMProperty));
	for (int p = 0; p < dest_entity->property_count; ++p) {
		LMProperty *dest_property = &dest_entity->properties[p];
		*dest_property = { 0 };

		dest_property->key = STRDUP(current_entity.properties[p].key);
		dest_property->value = STRDUP(current_entity.properties[p].value);
	}

	dest_entity->brush_count = current_entity.brush_count;
	dest_entity->brushes = (LMBrush *)realloc(dest_entity->brushes, dest_entity->brush_count * sizeof(LMBrush));
	for (int b = 0; b < dest_entity->brush_count; ++b) {
		LMBrush *dest_brush = &dest_entity->brushes[b];
		*dest_brush = { 0 };

		dest_brush->face_count = current_entity.brushes[b].face_count;
		dest_brush->faces = (LMFace *)realloc(dest_brush->faces, dest_brush->face_count * sizeof(LMFace));
		for (int f = 0; f < dest_brush->face_count; ++f) {
			dest_brush->faces[f] = current_entity.brushes[b].faces[f];
		}
	}

	reset_current_entity();
}