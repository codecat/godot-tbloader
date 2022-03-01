#include <builders/mesh_builder.h>

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/array_mesh.hpp>

#include <tb_loader.h>

MeshBuilder::MeshBuilder(TBLoader* loader)
	: Builder(loader)
{
}

MeshBuilder::~MeshBuilder()
{
}

void MeshBuilder::build_map()
{
	//
}
