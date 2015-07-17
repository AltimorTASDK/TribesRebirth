//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#ifndef _SIMWORLD_H_
#define _SIMWORLD_H_

#include "simBase.h"
#include "simEv.h"
#include "simAction.h"
#include "timer.h"

class GFXPalette;


//------------------------------------------------------------------------------
class SimWorld
{
protected:
   static GUID gameGuid;

   SimManager manager;
   Resource<GFXPalette> hPalette;
   bool renderEnabled;

	void setPalette(int transition);

public:
   SimWorld();
   virtual ~SimWorld();

	Resource<GFXPalette>& getPalette();
	bool setPalette(const char *paletteName, int transition=0);
	bool setPalette(Resource<GFXPalette> &hPalette, int transition=0);
   void delPalette();
   bool enableRender(bool);
   SimManager* getManager();

	virtual void advanceToTime(double time);
	virtual void render();
};   


inline Resource<GFXPalette>& SimWorld::getPalette()
{
   return ( hPalette );
}

inline bool SimWorld::enableRender(bool tf)
{
   return renderEnabled = tf;
}

inline SimManager* SimWorld::getManager()
{
   return ( &manager );
}

#endif

