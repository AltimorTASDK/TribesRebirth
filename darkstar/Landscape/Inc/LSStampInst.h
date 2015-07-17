//---------------------------------------------------------------------------
//
// LSStampInst.h
//
//---------------------------------------------------------------------------

#ifndef _LSSTAMPINST_H_
#define _LSSTAMPINST_H_

#include <sim.h>
#include <simresource.h>
#include <LS.h>
#include <LSTerrainStamp.h>

class LSTerrain;

class LSStampInstance : public SimObject
{
	private:
		typedef SimObject Parent;

	private:
		// private utility methods
		void loadResources();
		void unloadResources();
		void convertRotationToEuler(LS_ROT_90S, EulerF &);
		LS_ROT_90S convertVectorToRotation(Point2F & p);

		Resource<LSTerrainStamp> frStamp;
		LSTerrainStamp * fpStamp;

		// placement data (puts the "instance" in LSStampInstance)
		Point2I block;
		Point3F origin;
		LS_ROT_90S rotation;

		// know resource by ID or just have ptr to stamp?
		bool useID; // note: if !useID, we own stamp
		DWORD stampID;

		// bools to keep track of resource loading
		bool hasResources;
		bool resourcesLoaded;

	public:
		// methods for reading placement data and stamp
		const Point2I & getBlock(LSTerrain * terrain);
		const Point3F & getOrigin() { return origin; }
		LS_ROT_90S getRotation() { return rotation; }
		LSTerrainStamp * getStamp() { return fpStamp; }
		DWORD getStampTag() { return stampID; }

		// methods for writing placement data and stamp
		void setOrigin(const Point3F & p) { origin =p; }
		void setRotation(LS_ROT_90S rot) { rotation=rot; }
		bool setStamp(DWORD id);
			// note:  can't write StampInst if stamp set following ways
		bool setStamp(LSTerrainStamp * pStamp);

		// apply all stamps in group to terrain
		static bool applyStamps( LSTerrain * terrain, const Point2I & blk, SimManager * m );
		static void clearStamps( SimManager * m);

		// apply this stamp to terrain
		bool applyStamp( LSTerrain * terrain, bool blockValid=true );

	   bool processEvent(const SimEvent *event);
   	bool processQuery(SimQuery *query);
	
      bool onAdd();
   
		LSStampInstance();
		~LSStampInstance();

		// Persistent functions
		DECLARE_PERSISTENT(LSStampInstance);
		Persistent::Base::Error read(StreamIO &, int version, int user);
		Persistent::Base::Error write(StreamIO &, int version, int user);
};

#endif