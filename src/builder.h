#pragma once

#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/static_body3d.hpp>

#include <map_parser.h>
#include <geo_generator.h>
#include <surface_gatherer.h>

using namespace godot;

class TBLoader;

enum class ColliderType
{
	// Does not create any collider
	None,

	// Creates a collider from a Mesh (can create a single "merged" convex shape)
	Mesh,

	// Creates a static collider from a MeshInstance3D (can decompose into multiple convex shapes)
	Static,
};

enum class ColliderShape
{
	Convex,
	Concave,
};

class Builder
{
public:
	TBLoader* m_loader;
	std::shared_ptr<LMMapData> m_map;
	Dictionary m_loaded_map_textures; // Texture Name(const char*) - Ref<Texture2D>

public:
	Builder(TBLoader* loader);
	~Builder();

	void load_map(const String& path);
	void build_map();

	void build_worldspawn(int idx, LMEntity& ent);
	void build_brush(int idx, Node3D* node, LMEntity& ent);

	void build_entity(int idx, LMEntity& ent, const String& classname);
	void build_entity_custom(int idx, LMEntity& ent, const String& classname);
	void build_entity_light(int idx, LMEntity& ent);
	void build_entity_area(int idx, LMEntity& ent);

	void set_entity_node_common(Node3D* node, LMEntity& ent);
	void set_entity_brush_common(int idx, Node3D* node, LMEntity& ent);

protected:
	Vector3 lm_transform(const vec3& v);

	void add_collider_from_mesh(Node3D* area, Ref<ArrayMesh>& mesh, ColliderShape colshape);
	void add_surface_to_mesh(Ref<ArrayMesh>& mesh, LMSurface& surf);
	MeshInstance3D* build_entity_mesh(int idx, LMEntity& ent, Node3D* parent, ColliderType coltype, ColliderShape colshape);

protected:
	void load_and_cache_map_textures();

	String texture_path(const char* name, const char* extension);
	String material_path(const char* name);
	Ref<Texture2D> texture_from_name(const char* name);
	Ref<Material> material_from_name(const char* name);
};
