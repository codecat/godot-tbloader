#ifndef MAP_PARSER_H
#define MAP_PARSER_H

#include "brush.h"
#include "entity.h"
#include "face.h"
#include "libmap.h"
#include "map_data.h"
#include <memory>

#include <godot_cpp/classes/file.hpp>

typedef enum PARSE_SCOPE {
	PS_FILE,
	PS_COMMENT,
	PS_ENTITY,
	PS_PROPERTY_VALUE,
	PS_BRUSH,
	PS_PLANE_0,
	PS_PLANE_1,
	PS_PLANE_2,
	PS_TEXTURE,
	PS_U,
	PS_V,
	PS_VALVE_U,
	PS_VALVE_V,
	PS_ROT,
	PS_U_SCALE,
	PS_V_SCALE,
} PARSE_SCOPE;

class LMMapParser {
private:
	PARSE_SCOPE scope = PS_FILE;
	bool comment = false;
	int entity_idx = -1;
	int brush_idx = -1;
	int face_idx = -1;
	int component_idx = 0;
	char *current_property = NULL;
	bool valve_uvs = false;

	LMFace current_face;
	LMBrush current_brush;
	LMEntity current_entity;
	bool strings_match(const char *lhs, const char *rhs);

public:
	std::shared_ptr<LMMapData> map_data;

	bool load_from_path(const char *map_file);
	void load_from_godot_file(const godot::File& f);

	void token(const char *buf);
	void newline();

	void commit_face();
	void commit_brush();
	void commit_entity();
	LMMapParser(std::shared_ptr<LMMapData> _map_data) :
			map_data(_map_data){};

private:
	void reset_current_face();
	void reset_current_entity();
	void reset_current_brush();
	void set_scope(PARSE_SCOPE new_scope);
};

#endif
