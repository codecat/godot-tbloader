#include <builders/csg_builder.h>

#include <godot_cpp/classes/csg_combiner3d.hpp>
#include <godot_cpp/classes/csg_mesh3d.hpp>
#include <godot_cpp/classes/array_mesh.hpp>

#include <tb_loader.h>

CSGBuilder::CSGBuilder()
{
}

CSGBuilder::~CSGBuilder()
{
}

void CSGBuilder::build(TBLoader* parent)
{
	for (int i = 0; i < m_map->entity_count; i++) {
		auto& ent = m_map->entities[i];

		for (int j = 0; j < ent.property_count; j++) {
			auto& prop = ent.properties[j];
			if (!strcmp(prop.key, "classname")) {
				if (!strcmp(prop.value, "worldspawn")) {
					build_worldspawn(parent, ent, m_map->entity_geo[i]);

				} else if (!strcmp(prop.value, "info_player_start")) {
					//TODO
				}
			}
		}
	}
}

void CSGBuilder::build_worldspawn(TBLoader* parent, LMEntity& ent, LMEntityGeometry& geo)
{
	UtilityFunctions::print("Creating new worldspawn");

	auto combiner = memnew(CSGCombiner3D());
	parent->add_child(combiner);
	combiner->set_owner(parent->get_owner());
	combiner->set_use_collision(true); //TODO: Configurable

	const int SCALE = 16; //TODO: Configurable

	for (int i = 0; i < ent.brush_count; i++) {
		auto& brush = ent.brushes[i];
		auto& geo_brush = geo.brushes[i];

		auto csg_mesh = memnew(CSGMesh3D());
		combiner->add_child(csg_mesh);
		csg_mesh->set_owner(parent->get_owner());

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

				Vector3 vertex(v.vertex.y / SCALE, v.vertex.z / SCALE, v.vertex.x / SCALE);
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

				Vector3 vertex(v.vertex.y / SCALE, v.vertex.z / SCALE, v.vertex.x / SCALE);

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
		//TODO: mesh->set_material(..);
	}
}
