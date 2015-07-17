//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMVOLUME_H_
#define _SIMVOLUME_H_

#include "simBase.h"
#include "volstrm.h"
#include "simNetObject.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class SimVolume: public SimNetObject
{
private:
   typedef SimNetObject Parent;
	// Entry for this object
   const char *fileName;
   bool opened;
public:
	SimVolume();
	~SimVolume();

   DWORD packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream);
   void unpackUpdate(Net::GhostManager *gm, BitStream *stream);

	const char* getFileName() const;
	bool open(const char* filename);
	bool close();

   bool processArguments(int argc, const char **argv);
   bool onAdd();
   void onRemove();

   void inspectRead(Inspect *insp);
   void inspectWrite(Inspect *insp);

   DECLARE_PERSISTENT( SimVolume );
   static void initPersistFields();
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
};


//------------------------------------------------------------------------------

inline const char* SimVolume::getFileName() const
{
   return fileName;
}

#endif
