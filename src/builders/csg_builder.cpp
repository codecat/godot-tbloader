#include <builders/csg_builder.h>

#include <godot_cpp/classes/csg_combiner3d.hpp>
#include <godot_cpp/classes/csg_mesh3d.hpp>
#include <godot_cpp/classes/array_mesh.hpp>

#include <tb_loader.h>

CSGBuilder::CSGBuilder(TBLoader* loader)
	: Builder(loader)
{
}

CSGBuilder::~CSGBuilder()
{
}

void CSGBuilder::build_worldspawn(int idx, LMEntity& ent, LMEntityGeometry& geo)
{
	UtilityFunctions::print("Creating new worldspawn");

	auto combiner = memnew(CSGCombiner3D());
	m_loader->add_child(combiner);
	combiner->set_owner(m_loader->get_owner());
	combiner->set_use_collision(m_loader->m_collision);

	int inverse_scale = m_loader->m_inverse_scale;

	for (int i = 0; i < ent.brush_count; i++) {
		auto& brush = ent.brushes[i];
		auto& geo_brush = geo.brushes[i];

		auto csg_mesh = memnew(CSGMesh3D());
		combiner->add_child(csg_mesh);
		csg_mesh->set_owner(m_loader->get_owner());

		PackedVector3Array vertices;
		PackedFloat32Array tangents;
		PackedVector3Array normals;
		PackedVector2Array uvs;
		PackedInt32Array indices;

		int index_offset = 0;

		// Find mesh origin
		Vector3 vertex_min, vertex_max;
		bool has_vertex_min = false;

		for (int j = 0; j < brush.face_count; j++) {
			auto& geo_face = geo_brush.faces[j];

			for (int k = 0; k < geo_face.vertex_count; k++) {
				auto& v = geo_face.vertices[k];

				Vector3 vertex(v.vertex.y / inverse_scale, v.vertex.z / inverse_scale, v.vertex.x / inverse_scale);
				if (!has_vertex_min || vertex.length_squared() < vertex_min.length_squared()) {
					vertex_min = vertex;
					has_vertex_min = true;
				}
				if (vertex.length_squared() > vertex_max.length_squared()) {
					vertex_max = vertex;
				}
			}
		}

		Vector3 origin = vertex_min + (vertex_max - vertex_min) / 2;
		csg_mesh->set_position(origin);

		// Add all vertices minus origin
		for (int j = 0; j < brush.face_count; j++) {
			auto& face = brush.faces[j]; //TODO: Do we need this?
			auto& geo_face = geo_brush.faces[j];

			for (int k = 0; k < geo_face.vertex_count; k++) {
				auto& v = geo_face.vertices[k];

				Vector3 vertex(v.vertex.y / inverse_scale, v.vertex.z / inverse_scale, v.vertex.x / inverse_scale);

				vertices.push_back(vertex - origin);
				tangents.push_back(v.tangent.y);
				tangents.push_back(v.tangent.z);
				tangents.push_back(v.tangent.x);
				tangents.push_back(v.tangent.w);
				normals.push_back(Vector3(v.normal.y, v.normal.z, v.normal.x));
				uvs.push_back(Vector2(v.uv.u, v.uv.v));
			}

			for (int k = 0; k < geo_face.index_count; k++) {
				indices.push_back(index_offset + geo_face.indices[k]);
			}
			index_offset += geo_face.vertex_count;
		}

		Array arrays;
		arrays.resize(Mesh::ARRAY_MAX);
		arrays[Mesh::ARRAY_VERTEX] = vertices;
		arrays[Mesh::ARRAY_TANGENT] = tangents;
		arrays[Mesh::ARRAY_NORMAL] = normals;
		arrays[Mesh::ARRAY_TEX_UV] = uvs;
		arrays[Mesh::ARRAY_INDEX] = indices;

		Ref<ArrayMesh> mesh = memnew(ArrayMesh());
		mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
		csg_mesh->set_mesh(mesh);
		//TODO: Can we even have materials on entire CSG brushes with multiple faces?
		//TODO: mesh->set_material(..);
	}
}
