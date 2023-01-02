
#include <gdextension_interface.h>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/classes/editor_import_plugin.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/resource_saver.hpp>

#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <tb_loader.h>
#include <map_import.h>

TBMapImportPlugin::TBMapImportPlugin()
{
}

TBMapImportPlugin::~TBMapImportPlugin()
{
}

String TBMapImportPlugin::_get_save_extension() const
{
	return "scn";
}

String TBMapImportPlugin::_get_importer_name() const
{
	return "tbloader.map";
}

String TBMapImportPlugin::_get_resource_type() const
{
	return "PackedScene";
}

String TBMapImportPlugin::_get_visible_name() const
{
	return "Map";
}

bool TBMapImportPlugin::_get_option_visibility(const String &path, const StringName &option_name, const Dictionary &options) const
{
	return true;
}

double TBMapImportPlugin::_get_priority() const
{
	return 1.0;
}

int64_t TBMapImportPlugin::_get_import_order() const
{
	return IMPORT_ORDER_SCENE - 1; // after textures, before scenes
}

template <typename N>
inline Dictionary assemble_import_option(String name, PropertyHint type, String hint, N default_value)
{
	Dictionary dictionary;
	dictionary["name"] = name;
	dictionary["hint_string"] = hint;
	dictionary["property_hint"] = type;
	dictionary["default_value"] = default_value;
	return dictionary;
}

template <typename N>
inline Dictionary assemble_import_option(String name, N default_value)
{
	Dictionary dictionary;
	dictionary["name"] = name;
	dictionary["default_value"] = default_value;
	return dictionary;
}

inline Dictionary assemble_import_option_category(String name)
{
	Dictionary dictionary;
	dictionary["name"] = name;
	dictionary["default_value"] = "";
	dictionary["usage"] = PROPERTY_USAGE_CATEGORY;
	return dictionary;
}

TypedArray<Dictionary> TBMapImportPlugin::_get_import_options(const String &path, int64_t preset_index) const
{
	bool quake = preset_index == 1;
	TypedArray<Dictionary> out;

	out.append(assemble_import_option_category("map"));
	out.append(assemble_import_option("inverse_scale", 16));

	out.append(assemble_import_option_category("lighting"));
	out.append(assemble_import_option("unwrap_uv2", false));

	out.append(assemble_import_option_category("options"));
	out.append(assemble_import_option("collisions_enabled", true));
	out.append(assemble_import_option("skip_hidden_layers", true));
	out.append(assemble_import_option("nearest_texture_filter", quake));
	out.append(assemble_import_option("clip_texture_path", PROPERTY_HINT_ENUM_SUGGESTION, "special/clip,tools/toolsclip", quake ? "special/clip" : ""));
	out.append(assemble_import_option("skip_texture_path", PROPERTY_HINT_ENUM_SUGGESTION, "special/skip,tools/toolsskip", quake ? "special/skip" : ""));
	out.append(assemble_import_option("visual_layer_mask", PROPERTY_HINT_LAYERS_3D_RENDER, "", 1));

	out.append(assemble_import_option_category("entities"));
	out.append(assemble_import_option("common_entities", true));
	out.append(assemble_import_option("entities_folder_path", PROPERTY_HINT_DIR, "", "res://entities"));
	out.append(assemble_import_option("textures_folder_path", PROPERTY_HINT_DIR, "", "res://textures"));

	return out;
}

int64_t TBMapImportPlugin::_get_preset_count() const
{
	return 2;
}

String TBMapImportPlugin::_get_preset_name(int64_t preset_index) const
{
	switch (preset_index)
	{
	case 0:
		return "Default";
	case 1:
		return "Quake";
	default:
		return "Unknown";
	}
}

PackedStringArray TBMapImportPlugin::_get_recognized_extensions() const
{
	PackedStringArray array;
	array.append("map");
	return array;
}

int64_t TBMapImportPlugin::_import(const String &source_file, const String &save_path, const Dictionary &options, const TypedArray<String> &platform_variants, const TypedArray<String> &gen_files) const
{
	Ref<PackedScene> scene = memnew(PackedScene);

	TBLoader *loader = memnew(TBLoader);
	loader->set_name(source_file.get_file());

	// Map
	loader->set_map(source_file);
	loader->set_inverse_scale(options["inverse_scale"]);

	// Lighting
	loader->set_lighting_unwrap_uv2(options["unwrap_uv2"]);

	// Options
	loader->set_collision(options["collisions_enabled"]);
	loader->set_skip_hidden_layers(options["skip_hidden_layers"]);
	loader->set_filter_nearest(options["nearest_texture_filter"]);
	loader->set_clip_texture_name(options["clip_texture_path"]);
	loader->set_skip_texture_name(options["skip_texture_path"]);
	loader->set_visual_layer_mask(options["visual_layer_mask"]);

	// Entities
	loader->set_entity_common(options["common_entities"]);
	loader->set_entity_path(options["entities_folder_path"]);
	loader->set_texture_path(options["textures_folder_path"]);

	loader->build_meshes();
	scene->pack(loader);

	auto save_path_formatted = save_path + String(".scn");
	int64_t err = ResourceSaver::get_singleton()->save(scene, save_path_formatted);

	ERR_FAIL_COND_V_MSG(err != OK, err, "Cannot save scene to file '" + save_path_formatted + "'.");
	return err;
}

