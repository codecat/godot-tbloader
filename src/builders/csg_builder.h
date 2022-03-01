#pragma once

#include <builder.h>

class CSGBuilder : public Builder
{
public:
	CSGBuilder(TBLoader* loader);
	virtual ~CSGBuilder();

protected:
	virtual void build_worldspawn(LMEntity& ent, LMEntityGeometry& geo) override;
};
