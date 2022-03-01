#include <builder.h>

Builder::Builder(TBLoader* loader)
{
	m_loader = loader;
	m_map = std::make_shared<LMMapData>();
}

Builder::~Builder()
{
}

void Builder::load_map(const String& path)
{
	UtilityFunctions::print(String("Building map %s") % path);

	File f;
	if (!f.file_exists(path)) {
		UtilityFunctions::printerr("Map file does not exist!");
		return;
	}

	f.open(path, File::READ);
	LMMapParser parser(m_map);
	parser.load_from_godot_file(f);
	f.close();

	LMGeoGenerator geogen(m_map);
	geogen.run();
}

void Builder::build_map()
{
	for (int i = 0; i < m_map->entity_count; i++) {
		auto& ent = m_map->entities[i];

		for (int j = 0; j < ent.property_count; j++) {
			auto& prop = ent.properties[j];
			if (!strcmp(prop.key, "classname")) {
				build_entity(i, ent, prop.value);
			}
		}
	}
}

void Builder::build_worldspawn(LMEntity& ent, LMEntityGeometry& geo)
{
}

void Builder::build_entity(int idx, LMEntity& ent, const String& classname)
{
	if (classname == "worldspawn") {
		build_worldspawn(ent, m_map->entity_geo[idx]);
		return;
	}

	if (classname == "info_player_start") {
		//TODO
		return;
	}

	//TODO: More common entities
	//TODO: Entity callback to GDScript?
}
