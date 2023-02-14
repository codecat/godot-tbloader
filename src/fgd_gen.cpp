#include <fgd_gen.h>

#include <godot_cpp/classes/dir_access.hpp>

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
        return (dir_path + "/" + path).replace(this->m_loader->get_entity_path() + "/", "");
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

String FGDGen::generate_fgd_line_for_entity(String entity_path) {
    String fgd_line;
    return fgd_line;
}

void FGDGen::generate() {
    PackedStringArray entity_paths = this->find_all_entity_paths_in_dir(m_loader->get_entity_path());
    UtilityFunctions::print(entity_paths);
    // auto entity_base_path = m_loader->get_entity_path().replace("res://", "");

}