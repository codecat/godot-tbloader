#pragma once

#include <gdextension_interface.h>

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

	bool m_collision = true;
	bool m_skip_hidden_layers = true;
	bool m_filter_nearest = false;

	bool m_entity_common = true;
	String m_entity_path = "res://entities";
	String m_texture_path = "res://textures";
	String m_clip_texture_name = "";
	String m_skip_texture_name = "";
	uint32_t m_visual_layer_mask = 1;

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

	// Options
	void set_collision(bool enabled);
	bool get_collision();
	void set_skip_hidden_layers(bool enabled);
	bool get_skip_hidden_layers();
	void set_filter_nearest(bool enabled);
	bool get_filter_nearest();
	void set_clip_texture_name(const String& clip_texture);
	String get_clip_texture_name();
	void set_skip_texture_name(const String& skip_texture);
	String get_skip_texture_name();
	uint32_t get_visual_layer_mask();
	void set_visual_layer_mask(uint32_t visual_layer_mask);

	// Entities
	void set_entity_common(bool enabled);
	bool get_entity_common();
	void set_entity_path(const String& path);
	String get_entity_path();
	void set_texture_path(const String& path);
	String get_texture_path();

	void clear();
	void build_meshes();
};
