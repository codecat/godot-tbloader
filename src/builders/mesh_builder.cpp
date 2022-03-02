#include <builders/mesh_builder.h>

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

#include <tb_loader.h>
#include <surface_gatherer.h>

MeshBuilder::MeshBuilder(TBLoader* loader)
	: Builder(loader)
{
}

MeshBuilder::~MeshBuilder()
{
}

void MeshBuilder::build_worldspawn(int idx, LMEntity& ent, LMEntityGeometry& geo)
{
	for (int i = 0; i < m_map->texture_count; i++) {
		auto& tex = m_map->textures[i];
		build_texture_mesh(idx, tex.name);
	}
}

void MeshBuilder::build_texture_mesh(int idx, const char* name)
{
	int inverse_scale = m_loader->m_inverse_scale;

	// Load texture
	auto res_texture = texture_from_name(name);

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

		PackedVector3Array vertices;
		PackedFloat32Array tangents;
		PackedVector3Array normals;
		PackedVector2Array uvs;
		PackedInt32Array indices;

		// Find mesh origin
		Vector3 vertex_min, vertex_max;
		bool has_vertex_min = false;

		for (int k = 0; k < surf.vertex_count; k++) {
			auto& v = surf.vertices[k];

			Vector3 vertex(v.vertex.y / inverse_scale, v.vertex.z / inverse_scale, v.vertex.x / inverse_scale);
			if (!has_vertex_min || vertex.length_squared() < vertex_min.length_squared()) {
				vertex_min = vertex;
				has_vertex_min = true;
			}
			if (vertex.length_squared() > vertex_max.length_squared()) {
				vertex_max = vertex;
			}
		}

		Vector3 origin = vertex_min + (vertex_max - vertex_min) / 2;
		mesh_instance->set_position(origin);

		// Add all vertices minus origin
		for (int k = 0; k < surf.vertex_count; k++) {
			auto& v = surf.vertices[k];

			Vector3 vertex(v.vertex.y / inverse_scale, v.vertex.z / inverse_scale, v.vertex.x / inverse_scale);

			vertices.push_back(vertex - origin);
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
		if (material != nullptr) {
			mesh->surface_set_material(0, material);
		}
		mesh_instance->set_mesh(mesh);

		if (m_loader->m_collision) {
			//TODO: Experiment with the other colliders
			mesh_instance->create_trimesh_collision();
		}
	}
}
