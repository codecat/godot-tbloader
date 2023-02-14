#include <fgd_gen.h>

#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/script.hpp>

#include <tb_loader.h>

FGDGen::FGDGen(TBLoader* loader)
{
    m_loader = loader;
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
    // fgd entry consists of type (point/solid), fgd predefined properties (base/angle/size/etc), name, description, and custom properties
    // auto gen_fgd_for_property = [](String )
    auto resource_loader = ResourceLoader::get_singleton();
    Ref<PackedScene> scene = resource_loader->load(entity_path);
    Node* instance = scene->instantiate();
    auto properties = instance->get_property_list();
    UtilityFunctions::print("-------------------------------");
    // UtilityFunctions::print(properties);
    Ref<Script> attached_script = instance->get_script();
    // auto attached_script_loaded = resource_loader->load(attached_script->get_path());
    UtilityFunctions::print(attached_script->get_path());
    // UtilityFunctions::print(attached_script_loaded);

    String type = "PointClass";
    String fgd_properties = "";
    String name = entity_path.replace(this->m_loader->get_entity_path() + "/", "").replace("/", "_").replace(".tscn", "");
    String description = instance->get_name();
    String custom_properties = "";

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