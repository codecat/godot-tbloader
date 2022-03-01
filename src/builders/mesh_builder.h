#pragma once

#include <builder.h>

class MeshBuilder : public Builder
{
public:
	MeshBuilder(TBLoader* loader);
	virtual ~MeshBuilder();

	virtual void build_map() override;
};
