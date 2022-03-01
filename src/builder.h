#pragma once

#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/file.hpp>

#include <map_parser.h>
#include <geo_generator.h>
#include <surface_gatherer.h>

using namespace godot;

class TBLoader;

class Builder
{
protected:
	std::shared_ptr<LMMapData> m_map;

public:
	Builder();
	virtual ~Builder();

	virtual void load_map(const String& path);
	virtual void build(TBLoader* parent) = 0;
};
