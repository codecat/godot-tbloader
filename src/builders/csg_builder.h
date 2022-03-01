#pragma once

#include <builder.h>

class CSGBuilder : public Builder
{
public:
	CSGBuilder();
	virtual ~CSGBuilder();

	virtual void build(TBLoader* parent) override;

protected:
	virtual void build_worldspawn(TBLoader* parent, LMEntity& ent, LMEntityGeometry& geo);
};
