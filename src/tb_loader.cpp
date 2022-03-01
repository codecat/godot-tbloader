#include <tb_loader.h>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <builder.h>
#include <builders/csg_builder.h>

void TBLoader::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("set_map", "path"), &TBLoader::set_map);
	ClassDB::bind_method(D_METHOD("get_map"), &TBLoader::get_map);

	ClassDB::bind_method(D_METHOD("clear"), &TBLoader::clear);
	ClassDB::bind_method(D_METHOD("build_meshes"), &TBLoader::build_meshes);
	ClassDB::bind_method(D_METHOD("build_combined_csg"), &TBLoader::build_combined_csg);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "map", PROPERTY_HINT_FILE, "*.map"), "set_map", "get_map");
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

	//TODO:
	// - make surface gatherer for each texture, build meshes for each texture!

	UtilityFunctions::print("TODO!");
}

void TBLoader::build_combined_csg()
{
	clear();

	CSGBuilder builder;
	builder.load_map(m_map_path);
	builder.build(this);
}
