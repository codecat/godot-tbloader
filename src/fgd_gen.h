#pragma once

#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/classes/file_access.hpp>

using namespace godot;

class TBLoader;

class FGDGen 
{
public:
    TBLoader* m_loader;
public:
    FGDGen(TBLoader* loader);
    ~FGDGen();
    void generate();
private:
    PackedStringArray find_all_entity_paths();
    String generate_fgd_line_for_entity(String entity_path);
};