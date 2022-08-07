#include <builders/mesh_builder.h>

#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

#include <tb_loader.h>

MeshBuilder::MeshBuilder(TBLoader *loader)
	: Builder(loader)
{
}

MeshBuilder::~MeshBuilder()
{
}

void MeshBuilder::build_worldspawn(int idx, LMEntity &ent, LMEntityGeometry &geo)
{
	// TODO: Queue these meshes up so that we can put multiple worldspawn entities into 1 mesh

	// Create node for this entity
	auto container_node = memnew(Node3D());
	m_loader->add_child(container_node);
	container_node->set_owner(m_loader->get_owner());

	// Create mesh instance
	build_entity_mesh(idx, ent, container_node, true);

	// Delete container if we added nothing to it
	if (container_node->get_child_count() == 0)
	{
		container_node->queue_free();
		return;
	}

	// Find name for entity
	const char *tb_name;
	if (!strcmp(ent.get_property("classname"), "worldspawn"))
	{
		tb_name = "Default Layer";
	}
	else
	{
		tb_name = ent.get_property("_tb_name", nullptr);
	}

	// Add container to loader
	if (tb_name != nullptr)
	{
		container_node->set_name(tb_name);
	}
	container_node->set_position(lm_transform(ent.center));
}

void MeshBuilder::build_entity_mesh(int idx, LMEntity &ent, Node3D *parent, bool build_collision)
{
	// Create material
	Ref<Material> material;

	// Use name for the mesh instance
	String instance_name = String("entity_{0}_geometry").format(Array::make(idx));

	auto mesh_instance = memnew(MeshInstance3D());
	mesh_instance->set_name(instance_name);
	parent->add_child(mesh_instance);
	mesh_instance->set_owner(m_loader->get_owner());

	// Create mesh
	Ref<ArrayMesh> mesh = memnew(ArrayMesh());

	// Give mesh to mesh instance
	mesh_instance->set_mesh(mesh);

	for (int i = 0; i < m_map->texture_count; i++)
	{
		auto &tex = m_map->textures[i];
		char* name = tex.name;

		// Attempt to load material
		material = material_from_name(name);

		if (material == nullptr)
		{
			// Load texture
			auto res_texture = texture_from_name(name);

			// Create material
			if (res_texture != nullptr)
			{

				Ref<StandardMaterial3D> new_material = memnew(StandardMaterial3D());
				new_material->set_texture(BaseMaterial3D::TEXTURE_ALBEDO, res_texture);
				if (m_loader->m_filter_nearest)
				{
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

		auto &surfs = surf_gather.out_surfaces;
		if (surfs.surface_count == 0)
		{
			return;
		}

		for (int i = 0; i < surfs.surface_count; i++)
		{
			auto &surf = surfs.surfaces[i];
			if (surf.vertex_count == 0)
			{
				continue;
			}

			// Create mesh
			add_surface_to_mesh(surf, mesh);

			// Give mesh material
			if (material != nullptr)
			{
				mesh->surface_set_material(mesh->get_surface_count()-1, material);
			}
		}
	}
	
	// Unwrap UV2's if needed
	if (m_loader->m_lighting_unwrap_uv2)
	{
		mesh->lightmap_unwrap(mesh_instance->get_global_transform(), 0.05);
		mesh_instance->set_gi_mode(GeometryInstance3D::GI_MODE_STATIC);
	}

	// Create collisions if needed
	if (m_loader->m_collision && build_collision)
	{
		// TODO: Experiment with the other colliders
		mesh_instance->create_trimesh_collision();
	}
}
