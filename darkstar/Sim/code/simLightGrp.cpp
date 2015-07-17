//--------------------------------------------------------------------------- 
//
// simLightGrp.cpp
//
//--------------------------------------------------------------------------- 

#include <sim.h>
#include "simLightGrp.h"

float SimLightSet::sm_lightIgnoreDistance = 1e10f;

TSSceneLighting* SimLightSet::getSceneLighting(const Point3F& cameraPosition)
{
   float lightIgnoreDistSq = sm_lightIgnoreDistance * sm_lightIgnoreDistance;

   sceneLighting.clear();
   ColorF ambientIntensity(0.0f, 0.0f, 0.0f);
	iterator ptr;
	SimLightQuery query;
   for (ptr = begin(); ptr != end(); ptr++)
   {
      query.ambientIntensity.set();
   	if ((*ptr)->processQuery(&query))
      {
         ambientIntensity += query.ambientIntensity;
   		for (int i = 0; i < query.count; i++) {
            if (query.light[i]->fLight.fType != TS::Light::LightPoint ||
                ((cameraPosition - query.light[i]->fLight.fWPosition).lenSq() <=
                 lightIgnoreDistSq)) {
   			   sceneLighting.installLight(query.light[i]);
            }
         }
      }
   }

   sceneLighting.setAmbientIntensity(ambientIntensity);
   return &sceneLighting;
}

TSSceneLighting*
SimLightSet::getSceneLighting()
{
   sceneLighting.clear();
   ColorF ambientIntensity(0.0f, 0.0f, 0.0f);
	iterator ptr;
	SimLightQuery query;
   for (ptr = begin(); ptr != end(); ptr++)
   {
      query.ambientIntensity.set();
   	if ((*ptr)->processQuery(&query))
      {
         ambientIntensity += query.ambientIntensity;
   		for (int i = 0; i < query.count; i++) {
   	      sceneLighting.installLight(query.light[i]);
         }
      }
   }

   sceneLighting.setAmbientIntensity(ambientIntensity);
   return &sceneLighting;
}
