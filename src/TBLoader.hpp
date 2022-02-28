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

private:
	String m_map_path;

protected:
	static void _bind_methods();

public:
	TBLoader();
	~TBLoader();

	void set_map(const String &path);
	String get_map() const;

	void build();
};
