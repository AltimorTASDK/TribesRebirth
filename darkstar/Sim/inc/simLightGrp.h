//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#ifndef _SIMLIGHTGRP_H_
#define _SIMLIGHTGRP_H_

#include <ts_light.h>


//--------------------------------------------------------------------------- 

struct SimLightQuery: public SimQuery
{
	enum LightSize { MaxLights = 5 };
	int count;
   ColorF ambientIntensity;
	TSLight* light[MaxLights];
	SimLightQuery() { type = SimLightQueryType; }
};

class SimLightSet: public SimSet
{
   private:
      TSSceneLighting sceneLighting;

  public:
   static float sm_lightIgnoreDistance;   // pref variable, beyond which, 
                                          //  lights are disregarded. Defaults
                                          //  to 1e10.  Note, this only affects
                                          //  point lights, the sun is always with
                                          //  us.
   public:
   	SimLightSet() { id = SimLightSetId; }
      TSSceneLighting* getSceneLighting(const Point3F&);
      TSSceneLighting* getSceneLighting();
};


#endif
