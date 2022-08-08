#include <builder.h>

#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/omni_light3d.hpp>
#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/shape3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>

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

void Builder::build_worldspawn(int idx, LMEntity& ent)
{
	// Create node for this entity
	auto container_node = memnew(Node3D());
	m_loader->add_child(container_node);
	container_node->set_owner(m_loader->get_owner());

	// Decide generated collision type
	ColliderType collider = ColliderType::None;
	ColliderShape collider_shape = ColliderShape::Concave;
	if (m_loader->m_collision) {
		collider = ColliderType::Static;
		collider_shape = ColliderShape::Concave;
	}

	// Create mesh instance for each texture
	for (int i = 0; i < m_map->texture_count; i++) {
		auto& tex = m_map->textures[i];
		build_texture_mesh(idx, tex.name, ent, container_node, collider, collider_shape);
	}

	// Delete container if we added nothing to it
	if (container_node->get_child_count() == 0) {
		container_node->queue_free();
		return;
	}

	// Find name for entity
	const char* tb_name;
	if (!strcmp(ent.get_property("classname"), "worldspawn")) {
		tb_name = "Default Layer";
	} else {
		tb_name = ent.get_property("_tb_name", nullptr);
	}

	// Add container to loader
	if (tb_name != nullptr) {
		container_node->set_name(tb_name);
	}
	container_node->set_position(lm_transform(ent.center));
}

void Builder::build_entity(int idx, LMEntity& ent, const String& classname)
{
	if (classname == "worldspawn" || classname == "func_group") {
		// Skip worldspawn if the layer is hidden and the "skip hidden layers" option is checked
		if (m_loader->m_skip_hidden_layers) {
			bool is_visible = (ent.get_property_int("_tb_layer_hidden", 0) == 0);
			if (!is_visible) {
				return;
			}
		}
		build_worldspawn(idx, ent);
		return;
	}

	if (m_loader->m_entity_common) {
		if (classname == "light") {
			build_entity_light(idx, ent);
			return;
		}

		if (classname == "area") {
			build_entity_area(idx, ent);
			return;
		}

		//TODO: More common entities
	}

	build_entity_custom(idx, ent, m_map->entity_geo[idx], classname);
}

void Builder::build_entity_custom(int idx, LMEntity& ent, LMEntityGeometry& geo, const String& classname)
{
	// m_loader->m_entity_path => "res://entities/"
	// "info_player_start" => "info/player/start.tscn", "info/player_start.tscn", "info_player_start.tscn"
	// "thing" => "thing.tscn"

	auto resource_loader = ResourceLoader::get_singleton();

	auto arr = classname.split("_");
	for (int i = 0; i < arr.size(); i++) {
		String path = m_loader->m_entity_path + "/";
		for (int j = 0; j < arr.size(); j++) {
			if (j > 0) {
				if (j <= i) {
					path = path + "/";
				} else {
					path = path + "_";
				}
			}
			path = path + arr[j];
		}
		path = path + ".tscn";

		if (resource_loader->exists(path, "PackedScene")) {
			Ref<PackedScene> scene = resource_loader->load(path);

			auto instance = scene->instantiate();
			m_loader->add_child(instance);
			instance->set_owner(m_loader->get_owner());

			if (instance->is_class("Node3D")) {
				set_entity_node_common((Node3D*)instance, ent);
				if (ent.brush_count > 0) {
					set_entity_brush_common(idx, (Node3D*)instance, ent);
				}
			}

			for (int j = 0; j < ent.property_count; j++) {
				auto& prop = ent.properties[j];

				auto var = instance->get(prop.key);
				switch (var.get_type()) {
					case Variant::BOOL: instance->set(prop.key, atoi(prop.value) == 1); break;
					case Variant::INT: instance->set(prop.key, (int64_t)atoll(prop.value)); break;
					case Variant::FLOAT: instance->set(prop.key, atof(prop.value)); break; //TODO: Locale?
					case Variant::STRING: instance->set(prop.key, prop.value); break;

					case Variant::STRING_NAME: instance->set(prop.key, StringName(prop.value));
					case Variant::NODE_PATH: instance->set(prop.key, NodePath(prop.value)); //TODO: More TrenchBroom focused node path conversion?

					case Variant::VECTOR2: {
						vec2 v = vec2_parse(prop.value);
						instance->set(prop.key, Vector2(v.x, v.y));
						break;
					}
					case Variant::VECTOR2I: {
						vec2 v = vec2_parse(prop.value);
						instance->set(prop.key, Vector2i((int)v.x, (int)v.y));
						break;
					}
					case Variant::VECTOR3: {
						vec3 v = vec3_parse(prop.value);
						instance->set(prop.key, Vector3(v.x, v.y, v.z));
						break;
					}
					case Variant::VECTOR3I: {
						vec3 v = vec3_parse(prop.value);
						instance->set(prop.key, Vector3i((int)v.x, (int)v.y, (int)v.z));
						break;
					}

					case Variant::COLOR: {
						vec3 v = vec3_parse(prop.value);
						instance->set(prop.key, Color(v.x / 255.0f, v.y / 255.0f, v.z / 255.0f));
						break;
					}
				}
			}
			return;
		}
	}

	UtilityFunctions::printerr("Path to entity resource could not be resolved: ", classname);
}

void Builder::build_entity_light(int idx, LMEntity& ent)
{
	auto light = memnew(OmniLight3D());

	light->set_bake_mode(Light3D::BAKE_STATIC);
	light->set_param(Light3D::PARAM_RANGE, ent.get_property_double("range", 10));
	light->set_param(Light3D::PARAM_ENERGY, ent.get_property_double("energy", 1));
	light->set_param(Light3D::PARAM_ATTENUATION, ent.get_property_double("attenuation", 1));
	light->set_param(Light3D::PARAM_SPECULAR, ent.get_property_double("specular", 0.5));
	set_entity_node_common(light, ent);

	vec3 color = ent.get_property_vec3("light_color", { 255, 255, 255 });
	light->set_color(Color(color.x / 255.0f, color.y / 255.0f, color.z / 255.0f));

	m_loader->add_child(light);
	light->set_owner(m_loader->get_owner());
}

void Builder::build_entity_area(int idx, LMEntity& ent)
{
	Vector3 center = lm_transform(ent.center);

	// Gather surfaces for the area
	LMSurfaceGatherer surf_gather(m_map);
	surf_gather.surface_gatherer_set_entity_index_filter(idx);
	surf_gather.surface_gatherer_run();

	auto& surfs = surf_gather.out_surfaces;
	if (surfs.surface_count == 0) {
		return;
	}

	for (int i = 0; i < surfs.surface_count; i++) {
		auto& surf = surfs.surfaces[i];
		if (surf.vertex_count == 0) {
			continue;
		}

		// Create the mesh
		auto mesh = create_mesh_from_surface(surf);

		// Create the area
		auto area = memnew(Area3D());
		m_loader->add_child(area);
		area->set_owner(m_loader->get_owner());
		area->set_position(center);

		// Create collision shape for the area
		add_collider_from_mesh(area, mesh, ColliderShape::Concave);
	}
}

void Builder::set_entity_node_common(Node3D* node, LMEntity& ent)
{
	// Target name
	auto targetname = ent.get_property("targetname", nullptr);
	if (targetname != nullptr) {
		node->set_name(targetname);
	}

	// Position
	if (ent.has_property("origin")) {
		Vector3 origin = lm_transform(ent.get_property_vec3("origin"));
		node->set_position(origin);
	}

	// Rotation
	double pitch = 0;
	double yaw = 0;
	double roll = 0;

	if (ent.has_property("angle")) {
		// "angle" is yaw rotation only
		yaw = ent.get_property_double("angle");
	} else if (ent.has_property("angles")) {
		// "angles" is "pitch yaw roll"
		vec3 angles = ent.get_property_vec3("angles");
		pitch = angles.x;
		yaw = angles.y;
		roll = angles.z;
	} else if (ent.has_property("mangle")) {
		vec3 mangle = ent.get_property_vec3("mangle");
		// "mangle" depends on whether the classname starts with "light"
		const char* classname = ent.get_property("classname");
		if (strstr(classname, "light") == classname) {
			// "yaw pitch roll", if classname starts with "light"
			yaw = mangle.x;
			pitch = mangle.y;
			roll = mangle.z;
		} else {
			// "pitch yaw roll", just like "angles"
			pitch = mangle.x;
			yaw = mangle.y;
			roll = mangle.z;
		}
	}

	node->set_rotation(Vector3(
		Math::deg2rad(-pitch),
		Math::deg2rad(yaw + 180),
		Math::deg2rad(roll)
	));
}

void Builder::set_entity_brush_common(int idx, Node3D* node, LMEntity& ent)
{
	// Position
	Vector3 center = lm_transform(ent.center);
	node->set_position(center);

	// Check what we actually need
	bool need_visual = node->is_class("VisualInstance3D");
	ColliderType need_collider = ColliderType::None;
	ColliderShape need_collider_shape = ColliderShape::Concave;

	if (node->is_class("RigidDynamicBody3D")) {
		// RigidDynamicBody3D requires convex collision meshes
		need_collider = ColliderType::Mesh;
		need_collider_shape = ColliderShape::Convex;

	} else if (node->is_class("CollisionObject3D")) {
		// If it's not a dynamic body, we can just use a concave trimesh collider
		need_collider = ColliderType::Static;
		need_collider_shape = ColliderShape::Concave;
	}

	if (node->is_class("PhysicsBody3D")) {
		need_visual = true;
	}

	//TODO: Modify visual or collider from ent properties

	// Stop if we don't need to do anything
	if (!need_visual && need_collider == ColliderType::None) {
		UtilityFunctions::printerr("Brush entity class has no need for visual nor collision: ", node->get_class());
		return;
	}

	// If we need at least visual, create mesh instance for each texture, and create colliders in the process
	if (need_visual) {
		for (int i = 0; i < m_map->texture_count; i++) {
			auto& tex = m_map->textures[i];
			build_texture_mesh(idx, tex.name, ent, node, need_collider, need_collider_shape);
		}
		return;
	}

	// We don't need visual, we only need collision
	if (need_collider != ColliderType::None) {
		// Gather brush surfaces
		LMSurfaceGatherer surf_gather(m_map);
		surf_gather.surface_gatherer_set_entity_index_filter(idx);
		surf_gather.surface_gatherer_run();

		auto& surfs = surf_gather.out_surfaces;
		if (surfs.surface_count == 0) {
			return;
		}

		for (int i = 0; i < surfs.surface_count; i++) {
			auto& surf = surfs.surfaces[i];
			if (surf.vertex_count == 0) {
				continue;
			}

			// Create the mesh
			auto mesh = create_mesh_from_surface(surf);
			add_collider_from_mesh(node, mesh, need_collider_shape);
		}
	}
}

Vector3 Builder::lm_transform(const vec3& v)
{
	vec3 sv = vec3_div_double(v, m_loader->m_inverse_scale);
	return Vector3(sv.y, sv.z, sv.x);
}

void Builder::add_collider_from_mesh(Node3D* node, Ref<ArrayMesh>& mesh, ColliderShape colshape)
{
	Ref<Shape3D> mesh_shape;
	switch (colshape) {
	case ColliderShape::Convex: mesh_shape = mesh->create_convex_shape(); break;
	case ColliderShape::Concave: mesh_shape = mesh->create_trimesh_shape(); break;
	}

	auto collision_shape = memnew(CollisionShape3D());
	collision_shape->set_shape(mesh_shape);
	node->add_child(collision_shape);
	collision_shape->set_owner(m_loader->get_owner());
}

Ref<ArrayMesh> Builder::create_mesh_from_surface(LMSurface& surf)
{
	PackedVector3Array vertices;
	PackedFloat32Array tangents;
	PackedVector3Array normals;
	PackedVector2Array uvs;
	PackedInt32Array indices;

	// Add all vertices
	for (int k = 0; k < surf.vertex_count; k++) {
		auto& v = surf.vertices[k];

		vertices.push_back(lm_transform(v.vertex));
		tangents.push_back(v.tangent.y);
		tangents.push_back(v.tangent.z);
		tangents.push_back(v.tangent.x);
		tangents.push_back(v.tangent.w);
		normals.push_back(Vector3(v.normal.y, v.normal.z, v.normal.x));
		uvs.push_back(Vector2(v.uv.u, v.uv.v));
	}

	// Add all indices
	for (int k = 0; k < surf.index_count; k++) {
		indices.push_back(surf.indices[k]);
	}

	Array arrays;
	arrays.resize(Mesh::ARRAY_MAX);
	arrays[Mesh::ARRAY_VERTEX] = vertices;
	arrays[Mesh::ARRAY_TANGENT] = tangents;
	arrays[Mesh::ARRAY_NORMAL] = normals;
	arrays[Mesh::ARRAY_TEX_UV] = uvs;
	arrays[Mesh::ARRAY_INDEX] = indices;

	// Create mesh
	Ref<ArrayMesh> mesh = memnew(ArrayMesh());
	mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
	return mesh;
}

void Builder::build_texture_mesh(int idx, const char* name, LMEntity& ent, Node3D* parent, ColliderType coltype, ColliderShape colshape)
{
	// Create material
	Ref<Material> material;

	// Use name for the mesh instance
	String instance_name = String(name).replace("/", "_");

	// Attempt to load material
	material = material_from_name(name);

	if (material == nullptr) {
		// Load texture
		auto res_texture = texture_from_name(name);

		// Create material
		if (res_texture != nullptr) {
			Ref<StandardMaterial3D> new_material = memnew(StandardMaterial3D());
			new_material->set_texture(BaseMaterial3D::TEXTURE_ALBEDO, res_texture);
			if (m_loader->m_filter_nearest) {
				new_material->set_texture_filter(BaseMaterial3D::TEXTURE_FILTER_NEAREST);
			}
			material = new_material;
		}
	}

	// Gather surfaces for this texture
	LMSurfaceGatherer surf_gather(m_map);
	surf_gather.surface_gatherer_set_entity_index_filter(idx);
	surf_gather.surface_gatherer_set_texture_filter(name);
	surf_gather.surface_gatherer_run();

	auto& surfs = surf_gather.out_surfaces;
	if (surfs.surface_count == 0) {
		return;
	}

	for (int i = 0; i < surfs.surface_count; i++) {
		auto& surf = surfs.surfaces[i];
		if (surf.vertex_count == 0) {
			continue;
		}

		// Create mesh instance
		auto mesh_instance = memnew(MeshInstance3D());
		if (instance_name != "") {
			mesh_instance->set_name(instance_name);
		}
		parent->add_child(mesh_instance);
		mesh_instance->set_owner(m_loader->get_owner());

		// Create mesh
		auto mesh = create_mesh_from_surface(surf);

		// Give mesh material
		if (material != nullptr) {
			mesh->surface_set_material(0, material);
		}

		// Give mesh to mesh instance
		mesh_instance->set_mesh(mesh);

		// Unwrap UV2's if needed
		if (m_loader->m_lighting_unwrap_uv2) {
			mesh->lightmap_unwrap(mesh_instance->get_global_transform(), 0.05);
			mesh_instance->set_gi_mode(GeometryInstance3D::GI_MODE_STATIC);
		}

		// Create collisions if needed
		switch (coltype) {
		case ColliderType::Mesh:
			add_collider_from_mesh(parent, mesh, colshape);
			break;

		case ColliderType::Static:
			switch (colshape) {
			case ColliderShape::Convex: mesh_instance->create_multiple_convex_collisions(); break;
			case ColliderShape::Concave: mesh_instance->create_trimesh_collision(); break;
			}
			break;
		}
	}
}

String Builder::texture_path(const char* name)
{
	//TODO: .png might not always be correct!
	return String("res://textures/") + name + ".png";
}

String Builder::material_path(const char* name)
{
	//TODO: .material might not always be correct!
	return String("res://textures/") + name + ".material";
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

Ref<Material> Builder::material_from_name(const char* name)
{
	auto path = material_path(name);

	auto resource_loader = ResourceLoader::get_singleton();
	if (!resource_loader->exists(path)) {
		return nullptr;
	}

	return resource_loader->load(path);
}
