//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

#ifndef _SIMNETOBJECT_H
#define _SIMNETOBJECT_H

#include "simbase.h"
#include "ml.h"


//-----------------------------------------------------------------------------

class BitStream;

namespace Net
{

class GhostManager;
struct GhostInfo;
struct GhostRef
{
   GhostManager *ghostManager;
   GhostInfo *ghostInfo;
};


};

#define INVALID_GHOST_INDEX   1023

class SimClientRep;

//-----------------------------------------------------------------------------

class DLLAPI SimNetObject: public SimObject
{
	// The Ghost Manager needs read/write access
	friend Net::GhostManager;
private:
   typedef SimObject Parent;
protected:
	enum NetFlag
	{ 
		IsGhost = 				BIT(1),	// This is a ghost
      PolledGhost =        BIT(5),  // Poll this ghost for updates to non-ghost
      ScopeAlways =        BIT(6),  // if set, object always ghosts to clientReps
      ScopeLocal =         BIT(7),  // Ghost only to local client 2049
      Ghostable =          BIT(8),  // new flag -- set if this object CAN ghost
      Locked =             BIT(9),  // set if the object is locked for editing
		MaxNetFlagBit =		15,
	};
	BitSet32 netFlags;
   DWORD netIndex;               // the index of this ghost in the GhostManager on the server

   Net::GhostManager *owner;
public:
   Vector<Net::GhostRef> ghosts;
   struct CameraInfo
   {
      SimNetObject *camera;
      Point3F pos;				// Pos in world space
      Point3F orientation;		// Vector in world space
      float fov; 					// viwing angle/2
      float sinFov;           // sin(fov/2);
      float cosFov;           // cos(fov/2);
		float visibleDistance;  
   };

	SimNetObject();

	// Base class provides IO for it's members but is
	// not declared as persitent.

   static void initPersistFields();
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );

   bool onAdd();
   void onRemove();

   void inspectRead(Inspect *insp);
   void inspectWrite(Inspect *insp);

   void setMaskBits(DWORD orMask);
   virtual float getUpdatePriority(CameraInfo *focusObject, DWORD updateMask, int updateSkips);
   virtual DWORD getGhostTag();
   virtual DWORD packUpdate(Net::GhostManager *, DWORD mask, BitStream *stream);
   virtual void unpackUpdate(Net::GhostManager *, BitStream *stream);
   virtual void buildScopeAndCameraInfo(Net::GhostManager *cr, CameraInfo *camInfo);
   
   DWORD getNetIndex() { return netIndex; }

	bool isPolledGhost();
	bool isGhost();
	bool isScopeLocal();
   bool isScopeable();

   int getGhostedIndex(Net::GhostManager *gm);
};


//-----------------------------------------------------------------------------

inline bool SimNetObject::isPolledGhost()
{
	return netFlags.test(PolledGhost);
}

inline bool SimNetObject::isGhost()
{
	return netFlags.test(IsGhost);
}

inline bool SimNetObject::isScopeLocal()
{
	return netFlags.test(ScopeLocal);
}

inline bool SimNetObject::isScopeable()
{
	return netFlags.test(Ghostable) && !netFlags.test(ScopeAlways);
}

#endif
