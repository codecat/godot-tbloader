#include <TBLoader.h>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/classes/file.hpp>
#include <godot_cpp/classes/csg_combiner3d.hpp>

#include <map_parser.h>

void TBLoader::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("set_map", "path"), &TBLoader::set_map);
	ClassDB::bind_method(D_METHOD("get_map"), &TBLoader::get_map);
	ClassDB::bind_method(D_METHOD("build"), &TBLoader::build);

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

void TBLoader::build()
{
	UtilityFunctions::print(String("Building map %s") % m_map_path);

	File f;
	if (!f.file_exists(m_map_path)) {
		UtilityFunctions::printerr("Map file does not exist!");
		return;
	}

	f.open(m_map_path, File::READ);

	LMMapParser parser(std::make_shared<LMMapData>());
	parser.load_from_godot_file(f);

	UtilityFunctions::print("..done?");

	f.close();
}
