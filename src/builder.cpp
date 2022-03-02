#include <builder.h>

#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/omni_light3d.hpp>

#include <tb_loader.h>

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
	UtilityFunctions::print("Building map ", path);

	File f;
	if (!f.file_exists(path)) {
		UtilityFunctions::printerr("Map file does not exist!");
		return;
	}

	// Parse the map from the file
	f.open(path, File::READ);
	LMMapParser parser(m_map);
	parser.load_from_godot_file(f);
	f.close();

	// We have to manually set the size of textures
	for (int i = 0; i < m_map->texture_count; i++) {
		auto& tex = m_map->textures[i];

		auto res_texture = texture_from_name(tex.name);
		if (res_texture != nullptr) {
			tex.width = res_texture->get_width();
			tex.height = res_texture->get_height();
		} else {
			// Make sure we don't divide by 0 and create NaN UV's
			tex.width = 1;
			tex.height = 1;
		}
	}

	// Run geometry generator (this also generates UV's, so we do this last)
	LMGeoGenerator geogen(m_map);
	geogen.run();
}

void Builder::build_map()
{
	for (int i = 0; i < m_map->entity_count; i++) {
		auto& ent = m_map->entities[i];
		build_entity(i, ent, ent.get_property("classname"));
	}
}

void Builder::build_worldspawn(int idx, LMEntity& ent, LMEntityGeometry& geo)
{
}

void Builder::build_entity(int idx, LMEntity& ent, const String& classname)
{
	if (classname == "worldspawn") {
		build_worldspawn(idx, ent, m_map->entity_geo[idx]);
		return;
	}

	if (m_loader->m_common_entities) {
		if (classname == "light") {
			build_entity_light(idx, ent);
			return;
		}

		//TODO: More common entities
	}

	//TODO: Entity callback to GDScript?
}

void Builder::build_entity_light(int idx, LMEntity& ent)
{
	auto light = memnew(OmniLight3D());

	light->set_bake_mode(Light3D::BAKE_STATIC);
	light->set_param(Light3D::PARAM_RANGE, ent.get_property_double("range", 10));
	light->set_position(lm_transform(ent.get_property_vec3("origin")));

	vec3 color = ent.get_property_vec3("light_color");
	light->set_color(Color(color.x / 255.0f, color.y / 255.0f, color.z / 255.0f));

	m_loader->add_child(light);
	light->set_owner(m_loader->get_owner());
}

Vector3 Builder::lm_transform(const vec3& v)
{
	vec3 sv = vec3_div_double(v, m_loader->m_inverse_scale);
	return Vector3(sv.y, sv.z, sv.x);
}

String Builder::texture_path(const char* name)
{
	//TODO: .png might not always be correct!
	return String("res://textures/") + name + ".png";
}

Ref<Texture2D> Builder::texture_from_name(const char* name)
{
	auto path = texture_path(name);

	auto resource_loader = ResourceLoader::get_singleton();
	if (!resource_loader->exists(path)) {
		return nullptr;
	}

	return resource_loader->load(path);
}
