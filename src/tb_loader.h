#pragma once

#include <godot/gdnative_interface.h>

#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/defs.hpp>

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/control.hpp>

using namespace godot;

class TBLoader : public Node3D
{
	GDCLASS(TBLoader, Node3D);

public:
	String m_map_path;
	int m_inverse_scale = 16;

	bool m_lighting_unwrap_uv2 = false;

	PackedStringArray m_texture_import_extensions;
	bool m_filter_nearest = false;

	bool m_collision = true;
	bool m_skip_hidden_layers = true;

	bool m_entity_common = true;
	String m_entity_path = "res://entities";

protected:
	static void _bind_methods();

public:
	TBLoader();
	~TBLoader();

	// Map
	void set_map(const String& map);
	String get_map() const;
	void set_inverse_scale(int scale);
	int get_inverse_scale();

	// Lighting
	void set_lighting_unwrap_uv2(bool enabled);
	bool get_lighting_unwrap_uv2();

	// Textures
	void set_texture_import_extensions(const PackedStringArray& import_extensions);
	const PackedStringArray& get_texture_import_extensions() const;
	void set_filter_nearest(bool enabled);
	bool get_filter_nearest();

	// Options
	void set_collision(bool enabled);
	bool get_collision();
	void set_skip_hidden_layers(bool enabled);
	bool get_skip_hidden_layers();

	// Entities
	void set_entity_common(bool enabled);
	bool get_entity_common();
	void set_entity_path(const String& path);
	String get_entity_path();

	void clear();
	void build_meshes();
};
