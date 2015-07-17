//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMPALETTE_H_
#define _SIMPALETTE_H_

#include "streams.h"
#include "simBase.h"
#include "simNetObject.h"
#include "g_pal.h"

class SimPalette: public SimNetObject
{
private:
   typedef SimNetObject Parent;

protected:
   Resource<GFXPalette> palette;
   const char *fileName;

   bool onAdd();

public:
	SimPalette();
	~SimPalette();

   DWORD packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream);
   void unpackUpdate(Net::GhostManager *gm, BitStream *stream);
	virtual bool open(const char* filename, bool forceLoad = false);

   Resource<GFXPalette> getPalette() { return palette; }
   void setInGame();
   void inspectRead(Inspect *insp);
   void inspectWrite(Inspect *insp);

   bool processArguments(int argc, const char **argv);

   DECLARE_PERSISTENT( SimPalette );
   static void initPersistFields();
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
};

#endif
