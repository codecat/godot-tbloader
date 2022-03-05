#pragma once

#include <godot/gdnative_interface.h>

#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/defs.hpp>

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/control.hpp>

#include <builder.h>

using namespace godot;

class TBLoader : public Node3D
{
	GDCLASS(TBLoader, Node3D);

public:
	String m_map_path;
	int m_inverse_scale = 16;

	bool m_collision = true;
	bool m_skip_hidden_layers = true;

	bool m_entity_common = true;

protected:
	static void _bind_methods();

public:
	TBLoader();
	~TBLoader();

	void set_map(const String& map);
	String get_map() const;

	void set_inverse_scale(int scale);
	int get_inverse_scale();

	void set_collision(bool enabled);
	bool get_collision();

	void set_skip_hidden_layers(bool enabled);
	bool get_skip_hidden_layers();

	void set_entity_common(bool enabled);
	bool get_entity_common();

	void clear();

	void build_meshes();
	void build_combined_csg();
};
