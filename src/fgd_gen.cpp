#include <fgd_gen.h>

#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <tb_loader.h>

FGDGen::FGDGen(TBLoader* loader)
{
    m_loader = loader;
    this->fgd_properties = std::map<String, std::function<String(String)>>();
    this->fgd_properties["fgd_size"] = [](String size) {
        return vformat("size(-%s -%s -%s, %s %s %s)", size, size, size, size, size, size);
    };
}

FGDGen::~FGDGen()
{
}

PackedStringArray FGDGen::find_all_entity_paths_in_dir(String dir_path) {
    Ref<DirAccess> dir = DirAccess::open(dir_path);
    if (dir->get_open_error() != Error::OK) {
        UtilityFunctions::print("Error opening directory");
        return PackedStringArray();
    }

    PackedStringArray entity_paths = PackedStringArray();
    for (auto entity_path : dir->get_files()) {
        if (entity_path.ends_with(".tscn")) entity_paths.append(entity_path);
    }
    std::transform(entity_paths.begin(), entity_paths.end(), entity_paths.begin(), [&dir_path, this](String path) {
        // return (dir_path + "/" + path).replace(this->m_loader->get_entity_path() + "/", "");
        return (dir_path + "/" + path);
    });

    PackedStringArray subdirs = dir->get_directories();
    if (subdirs.size() > 0) {
        for (int i = 0; i < subdirs.size(); i++) {
            String subdir_path = dir_path + "/" + subdirs[i];
            PackedStringArray sub_entity_paths = this->find_all_entity_paths_in_dir(subdir_path);
            entity_paths.append_array(sub_entity_paths);
        }
    }

    return entity_paths;
}

String FGDGen::generate_fgd_for_entity(String entity_path) {
    auto resource_loader = ResourceLoader::get_singleton();
    Ref<PackedScene> scene = resource_loader->load(entity_path);
    Node* instance = scene->instantiate();
    // auto properties = instance->get_property_list();
    Ref<Script> attached_script = instance->get_script();

    auto stringify_variant_type = [&] (int64_t t) {
        switch (t) {
            case Variant::Type::BOOL: return "bool";
            case Variant::Type::INT: return "integer";
            case Variant::Type::FLOAT: return "float";
            case Variant::Type::COLOR: return "color";
            case Variant::Type::DICTIONARY: return "flags";
            case Variant::Type::ARRAY: return "choices";
            default: return "string";
        }
    };
    
    String type = "PointClass";
    String name = entity_path.replace(this->m_loader->get_entity_path() + "/", "").replace("/", "_").replace(".tscn", "");
    String description = instance->get_name();
    String fgd_properties = "";
    String custom_properties = "";

    if (attached_script.is_valid()) {
        TypedArray<Dictionary> property_list = attached_script->get_script_property_list();
        for (int i = 0; i < property_list.size(); i++) {
            Dictionary property = property_list[i];
            if (property["hint_string"].stringify().find(".gd") != -1 || (int64_t)property["type"] == Variant::Type::NIL) continue;
            auto property_name = property["name"].stringify();
            if (property_name == "fgd_solid") {
                if (instance->get(property_name)) type = "SolidClass";
                continue;
            }
            if (this->fgd_properties.find(property_name) != this->fgd_properties.end()) {
                fgd_properties += this->fgd_properties[property_name](instance->get(property_name).stringify()) + " ";
                continue;
            }
            else {
                custom_properties += vformat("\n\t%s(%s) : \"%s\" : %s : \"%s\"", property["name"], stringify_variant_type(property["type"]), property["name"], instance->get(property_name), "");
            }
        }
        // fgd_properties += "\n";
        custom_properties += "\n";
    }

    String fgd_entry = vformat("@%s %s = %s : \"%s\" [ %s ]", type, fgd_properties, name, description, custom_properties);

    return fgd_entry;
}

void FGDGen::generate() {
    PackedStringArray entity_paths = this->find_all_entity_paths_in_dir(m_loader->get_entity_path());

    Ref<FileAccess> f = FileAccess::open(String(ProjectSettings::get_singleton()->get_setting("application/config/name")) + ".fgd", FileAccess::WRITE);
    for (auto entity_path : entity_paths) {
        String fgd_line = this->generate_fgd_for_entity(entity_path);
        f->store_string(fgd_line + "\n");
        UtilityFunctions::print(fgd_line);
    }
}