#pragma once

#include <builder.h>

class MeshBuilder : public Builder
{
public:
	MeshBuilder(TBLoader* loader);
	virtual ~MeshBuilder();

	virtual void build_worldspawn(int idx, LMEntity& ent, LMEntityGeometry& geo) override;

protected:
	virtual void build_texture_mesh(int idx, const char* name);
};
