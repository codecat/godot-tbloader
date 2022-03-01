#include <tb_loader.h>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <builders/csg_builder.h>
#include <builders/mesh_builder.h>

void TBLoader::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("set_map", "path"), &TBLoader::set_map);
	ClassDB::bind_method(D_METHOD("get_map"), &TBLoader::get_map);

	ClassDB::bind_method(D_METHOD("clear"), &TBLoader::clear);
	ClassDB::bind_method(D_METHOD("build_meshes"), &TBLoader::build_meshes);
	ClassDB::bind_method(D_METHOD("build_combined_csg"), &TBLoader::build_combined_csg);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "map", PROPERTY_HINT_FILE, "*.map"), "set_map", "get_map");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "inverse_scale", PROPERTY_HINT_NONE, "Inverse Scale"), "set_inverse_scale", "get_inverse_scale");
}

TBLoader::TBLoader()
{
}

TBLoader::~TBLoader()
{
}

void TBLoader::set_map(const String &path)
{
	m_map_path = path;
}

String TBLoader::get_map() const
{
	return m_map_path;
}

void TBLoader::set_inverse_scale(int scale)
{
	m_inverse_scale = scale;
}

int TBLoader::get_inverse_scale()
{
	return m_inverse_scale;
}

void TBLoader::set_collision(bool enabled)
{
	m_collision = enabled;
}

bool TBLoader::get_collision()
{
	return m_collision;
}

void TBLoader::clear()
{
	while (get_child_count() > 0) {
		auto child = get_child(0);
		remove_child(child);
		child->queue_free();
	}
}

void TBLoader::build_meshes()
{
	clear();

	MeshBuilder builder(this);
	builder.load_map(m_map_path);
	builder.build_map();
}

void TBLoader::build_combined_csg()
{
	clear();

	CSGBuilder builder(this);
	builder.load_map(m_map_path);
	builder.build_map();
}
