#pragma once

#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/classes/file.hpp>
#include <godot_cpp/classes/texture2d.hpp>

#include <map_parser.h>
#include <geo_generator.h>
#include <surface_gatherer.h>

using namespace godot;

class TBLoader;

class Builder
{
public:
	TBLoader* m_loader;
	std::shared_ptr<LMMapData> m_map;

public:
	Builder(TBLoader* loader);
	virtual ~Builder();

	virtual void load_map(const String& path);
	virtual void build_map();

	virtual void build_worldspawn(int idx, LMEntity& ent, LMEntityGeometry& geo);
	virtual void build_entity(int idx, LMEntity& ent, const String& classname);
	virtual void build_entity_light(int idx, LMEntity& ent);

protected:
	Vector3 lm_transform(const vec3& v);

protected:
	static String texture_path(const char* name);
	static Ref<Texture2D> texture_from_name(const char* name);
};
