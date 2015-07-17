#ifndef _H_MAINPLAYER
#define _H_MAINPLAYER

#include <sim.h>
#include <move.h>
#include "ts_shapeinst.h"
#include "simContainer.h"
#include "simCollideable.h"
//#include "shadowRenderImage.h"
#include "simMovement.h"
#include "simaction.h"
#include "simresource.h"
#include "simgame.h"

class MainPlayer :  public SimMovement
{
   typedef SimMovement Parent;
protected:
   SimRenderDefaultImage image;
	Point3F throttle;
	Point3F desired_throttle;
	SimCollisionSphereImage collisionImage;
	Resource<SimActionMap> actionMap;

	void update(SimTime time);
   Net::GhostManager *ghostManager;
   Int32 playerId;
public:

   void setGhostInfo(Net::GhostManager *, Int32);

	MoveLinear movement;
	MoveLinear rotation;

	const Point3F& getPos(void);
	void setPos(Point3F pos);
	void setActionMap(const char* file);

	MainPlayer();
	~MainPlayer();
   bool processArguments(int argc, const char **argv);
	bool processEvent(const SimEvent*);
	bool processQuery(SimQuery*);
   bool onAdd();
   
   DWORD packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream);
   void unpackUpdate(Net::GhostManager *, BitStream *stream);

   DECLARE_PERSISTENT(MainPlayer);

   enum
   {
      PositionMask = 1 << 0,
      InitMask = 1 << 31,
   };
};



#endif