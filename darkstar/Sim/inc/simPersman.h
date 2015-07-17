//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#ifndef _PERSMAN_H_
#define _PERSMAN_H_

#include <base.h>
#include <sim.h>
#include <volstrm.h>
#include <tvector.h>

//--------------------------------------------------------------------------- 


struct SimPersistAddQuery: public SimQuery
{
	SphereF		zone;
	int			flags;

	SimPersistAddQuery() { type = SimPersistAddQueryType; }
};

struct SimPersistTimeoutQuery: public SimQuery
{
	SimTime		interval;
	Bool			timeout;
	int			flags;
	int			size;

	SimPersistTimeoutQuery() { type = SimPersistTimeoutQueryType; }
};

struct SimLookupQuery: public SimQuery
{
	SimObject	*obj;
	SimObjectId	id;
	const char *name;

	SimLookupQuery() { type = SimLookupQueryType; }
};

class SimPersistManager:  public SimSet
{
	#define VID_VOL_PART(a)	(a & 0xf0000000)	// current Volume tag
	#define VID_ORG_PART(a)	(a & 0x0f000000)	// original Volume tag
	#define VID_ID_PART(a)	(a & 0x00ffffff)	// ID number
	#define VID_BOTH(a)		(a & 0x0fffffff)	// ID and Original Volume tag

	#define VID_ONE_VOL		0x10000000
	#define VID_TWO_VOL		0x20000000
	#define VID_THREE_VOL	0x30000000
	#define VID_FOUR_VOL		0x40000000
	#define VID_TEMP_VOL		0x50000000

	#define FLAG_ACTIVE	0x01
	#define FLAG_EXPIRE	0x02
	#define FLAG_ALWAYS	0x04

	typedef SimSet Parent;

   struct DIDObj
   {
		SimObjectId	ID;
		unsigned int	VolID;
		SphereF		sphere;
		int			flags;
		SimTime		time;
		SimObject	*sim_obj;
   };

	Vector<DIDObj>	DIDList;
	typedef Vector<DIDObj>::iterator iterator;

	unsigned int next_vid;

	VolumeRWStream tempVolume;
	bool createTempVolume(void);

	VolumeRWStream *baseVolume[4];
	char				*volumeName[4];

	void save_vol(VolumeRWStream *volume, unsigned int vid_mask);
	unsigned int newVolID(void);

	void timeout(Vector<DIDObj>::iterator i, bool in_list);
	SimObject* restoreObject(unsigned int vid);

public:
	SimPersistManager (SimObjectId id);
	~SimPersistManager ();

   bool onAdd();

	bool processEvent(const SimEvent*);
	bool processQuery (SimQuery *query);

	SimObject* addObject (SimObject *obj);
	void addObject (SimObject *sobj, SphereF sphere, int flags);
	void removeObject (SimObject *obj);
	void purge (void);
	SimObject* getObject (SimObjectId ID);
	SimObject* getObject (const char* name);
	Bool openVol (char *filename, int index);
	Bool readToTemp (char *filename);
	Bool saveVol (char *filename);
	Bool saveGame (char *filename);
	Bool loadGame (char *filename);
	void wakeUp (SphereF zone);
	float updateTimer (SimObject *sobj);
};

#endif
