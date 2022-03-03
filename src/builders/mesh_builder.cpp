#include <builders/mesh_builder.h>

#include <godot_cpp/classes/standard_material3d.hpp>

#include <tb_loader.h>

MeshBuilder::MeshBuilder(TBLoader* loader)
	: Builder(loader)
{
}

MeshBuilder::~MeshBuilder()
{
}

void MeshBuilder::build_worldspawn(int idx, LMEntity& ent, LMEntityGeometry& geo)
{
	//TODO: Queue these meshes up so that we can put multiple worldspawn entities into 1 mesh

	for (int i = 0; i < m_map->texture_count; i++) {
		auto& tex = m_map->textures[i];
		build_texture_mesh(idx, tex.name, ent);
	}
}

void MeshBuilder::build_texture_mesh(int idx, const char* name, LMEntity& ent)
{
	// Load texture
	auto res_texture = texture_from_name(name);

	// Get entity properties
	Vector3 center = lm_transform(ent.center);

	// Create material
	Ref<StandardMaterial3D> material;
	if (res_texture != nullptr) {
		material = Ref<StandardMaterial3D>(memnew(StandardMaterial3D()));
		material->set_texture(BaseMaterial3D::TEXTURE_ALBEDO, res_texture);
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
		m_loader->add_child(mesh_instance);
		mesh_instance->set_owner(m_loader->get_owner());

		// Find mesh origin
		Vector3 origin = get_origin_from_surface(surf);
		mesh_instance->set_position(origin + center);

		// Create mesh
		auto mesh = create_mesh_from_surface(surf, origin);

		// Give mesh material
		if (material != nullptr) {
			mesh->surface_set_material(0, material);
		}

		// Give mesh to mesh instance
		mesh_instance->set_mesh(mesh);

		if (m_loader->m_collision) {
			//TODO: Experiment with the other colliders
			mesh_instance->create_trimesh_collision();
		}
	}
}
