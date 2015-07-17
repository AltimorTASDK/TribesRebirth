//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#include "itrmetrics.h"

ITRMetricsInfo ITRMetrics;

void ITRMetricsInfo::Render::reset()
{
	leafs = surfaces = planes = polys = 
	outsideBits = textureCache = pixels = 0;
   surfTransformCacheHits =  surfTransformCacheMisses = 0;
}

void ITRMetricsInfo::Collision::reset()
{
	leafs = surfaces = nodes = planes = polys = 0;
}

void ITRMetricsInfo::reset()
{
	numRenderedInteriors = 0;
   collision.reset();
   render.reset();
}
