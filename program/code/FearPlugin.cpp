 //----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <sim.h>
#include <simGame.h>
#include <simcanvas.h>
#include <simtrigger.h>
#include <simLightGrp.h>
#include <itrrender.h>
#include "simConsoleEvent.h"

#include "version.h"
#include "simConsolePlugin.h"
#include "FearPlugin.h"
#include "simMoveobj.h"
#include "simTerrain.h"
#include "Sky.h"
#include "StarField.h"

#include "Player.h"
#include "SimDLLObject.h"

#include "feardcl.h"
#include "PlayerManager.h"
#include "fearPlayerPSC.h"
#include "fearCSDelegate.h"
#include "netGhostManager.h"
#include "fearGlobals.h"
#include "FearGuiTSCommander.h"
#include "FearGuiServerList.h"
#include "dlgPlay.h"
#include "gameBase.h"
#include "vehicle.h"
#include "sensor.h"
#include "dataBlockManager.h"
#include "interiorShape.h"
#include "tsfx.h"
#include "sensorManager.h"
#include "turret.h"
#include "moveable.h"
#include "chatMenu.h"
#include "simSetIterator.h"
#include "smokePuff.h"
#include "fearguishapeview.h"
#include "snowfall.h"
#include "fearGuiCFGButton.h"
#include "explosion.h"
#include "flier.h"
#include "udpnet.h"

#include <projectile.h>
#include <projectileFactory.h>

#include <decalmanager.h>

//----------------------------------------------------------------------------

#define onCommand(T) \
	case T: return on##T(argc,argv);


//------------------------------------------------------------------------------
//-------------------------------------- Effects variables...
//
static char ReturnBuffer[256];
bool filterBadWords = false;
float screenError = 0;
float farDistance = 500;

//----------------------------------------------------------------------------
namespace SimGui {
extern Control *findControl(const char *name);
};

bool gAnimateTransitions = false;

enum CallbackID {
	// Example stuff
   StartGhosting,
	NewMoveObj,
	NewNetObj,
	LocalNetObj,

	LoadDLL,
	NewDLLObject,

	// Fear objects
	LoadMission,
   NewPlayer,
	NewPlayerCamera,
	
   ListPlayers,
   
   //chat commands
   IssueCommand,
   IssueCommandI,
   IssueTargCommand,
   RemoteSelectCommander,
	SetCommandStatus,

	RemoteKill,
   RenderCanvas,
   ResetPlayerManager,
   ResetGhostManagers,
	RemotePlayAnim,
   AddChatCommand,
   SetChatMode,
   GetTransportAddress,
};


//----------------------------------------------------------------------------

static const char *intToStr(int d)
{
   static char buf[16];
   sprintf(buf, "%d", d);
   return buf;
}

static const char *floatToStr(float d)
{
   static char buf[16];
   sprintf(buf, "%g", d);
   return buf;
}

//----------------------------------------------------------------------------

static Vehicle* findVehicleObject (const char* name)
{
	if (sg.manager)
		if (SimObject* sobj = sg.manager->findObject(name))
			if (Vehicle* vobj = dynamic_cast<Vehicle*>(sobj))
				return vobj;
	return 0;
}

static Sensor* findSensorObject(const char* name)
{
	if (sg.manager)
		if (SimObject* sobj = sg.manager->findObject(name))
			if (Sensor* gobj = dynamic_cast<Sensor*>(sobj))
				return gobj;
	return 0;
}

GameBase* findGameObject(const char* name)
{
	if (!sg.manager)
		return 0;

	if (SimObject* sobj = sg.manager->findObject(name))
		if (GameBase* gobj = dynamic_cast<GameBase*>(sobj))
			return gobj;

   int iPlayerID = atoi(name);

   // Try looking up the player ID
   if (iPlayerID >= 2049 && iPlayerID <= 2176 && sg.playerManager)
   {
		if (PlayerManager::ClientRep *p = 
          sg.playerManager->findBaseRep(iPlayerID))
      {
			return p->ownedObject;
      }
   }
	return 0;
}

static InteriorShape*
findInteriorShapeObject(const char* name)
{
	if (sg.manager)
		if (SimObject* sobj = sg.manager->findObject(name))
			if (InteriorShape* gobj = dynamic_cast<InteriorShape*>(sobj))
				return gobj;
	return NULL;
}

static ShapeBase* findShapeBaseObject(const char* name)
{
	if (sg.manager)
		if (SimObject* sobj = sg.manager->findObject(name))
			if (ShapeBase* gobj = dynamic_cast<ShapeBase*>(sobj))
				return gobj;
	return 0;
}

static StaticBase* findStaticBaseObject(const char* name)
{
	if (sg.manager)
		if (SimObject* sobj = sg.manager->findObject(name))
			if (StaticBase* gobj = dynamic_cast<StaticBase*>(sobj))
				return gobj;
	return 0;
}

static Player* findPlayerObject(const char* name)
{
	if (sg.manager)
		if (SimObject* sobj = sg.manager->findObject(name)) {
			if (Player* gobj = dynamic_cast<Player*>(sobj))
				return gobj;
	   }
	// If it's wasn't the player object, it might be a playerid

	if (sg.playerManager) {
	   if (PlayerManager::ClientRep *p = 
		   	sg.playerManager->findBaseRep(atoi(name)))
			return p->ownedObject ? dynamic_cast<Player *>(p->ownedObject) : NULL;
	}
	return 0;
}

static Turret* findTurretObject(const char* name)
{
	if (sg.manager)
		if (SimObject* sobj = sg.manager->findObject(name))
			if (Turret* tobj = dynamic_cast<Turret*>(sobj))
				return tobj;
	return 0;
}

static Item* findItemObject(const char* name)
{
	if (sg.manager)
		if (SimObject* sobj = sg.manager->findObject(name))
			if (Item* gobj = dynamic_cast<Item*>(sobj))
				return gobj;
	return 0;
}

static Moveable* findMoveableObject(const char* name)
{
	if (sg.manager)
		if (SimObject* sobj = sg.manager->findObject(name))
			if (Moveable* gobj = dynamic_cast<Moveable*>(sobj))
				return gobj;
	return 0;
}

//----------------------------------------------------------------------------
// Misc
//----------------------------------------------------------------------------

static const char *c_version(CMDConsole *, int, int, const char **)
{
   return FearVersion;
}

static const char *c_timeStamp(CMDConsole *, int, int, const char **)
{
	sprintf(ReturnBuffer,"%s %s",BuildTimeStamp,BuildDateStamp);
	return ReturnBuffer;
}

static const char *c_groupRemove(CMDConsole *, int, int argc, const char **argv)
{
   if(!wg->manager || argc != 3)
      return "True";
   SimSet *gobj = NULL;
   SimObject *sobj;
   if((sobj = wg->manager->findObject(argv[1])) != NULL)
      gobj = dynamic_cast<SimSet*>(sobj);

   if(!gobj)
      return "False";
   if((sobj = wg->manager->findObject(argv[2])) != NULL)
      sobj->removeFromSet(argv[1]);
   return "True";
} 

static const char *c_groupGetCount(CMDConsole *, int, int argc, const char **argv)
{
   if(!wg->manager || argc != 2)
      return "0";
   if(SimObject *sobj = wg->manager->findObject(argv[1]))
      if(SimSet *gobj = dynamic_cast<SimSet*>(sobj))
         return intToStr(gobj->size());
   return "0";
} 

static const char *c_groupIterate(CMDConsole *, int, int argc, const char **argv)
{
   const char *newArgv[20];

   if(!wg->manager || argc < 3)
      return "0";
   if(SimObject *sobj = wg->manager->findObject(argv[1])) {
      if(SimSet *gobj = dynamic_cast<SimSet*>(sobj))
      {
         SimSetIterator it(gobj);
         newArgv[0] = argv[2];
         int i;
         for(i = 3; i < argc; i++)
            newArgv[i-1] = argv[i];

         while(*it)
         {
            newArgv[1] = intToStr((*it)->getId());
            Console->execute(argc - 1, newArgv);
            ++it;
         }
      }
   }
   return "0";
      
}

static const char *c_groupGetObject(CMDConsole *, int, int argc, const char **argv)
{
   if(!wg->manager || argc != 3)
      return "-1";
   if(SimObject *sobj = wg->manager->findObject(argv[1])) {
      if(SimSet *gobj = dynamic_cast<SimSet*>(sobj))
      {
         int index = atoi(argv[2]);
  	      if(index >= 0 && index < gobj->size())
     	      return intToStr((*gobj)[index]->getId());
		}
   }
   return "-1";
}


static const char *c_dotVec(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to dotVec(vec,vec);");
		return "0";
	}
	Point3F vec1,vec2;
	sscanf(argv[1],"%f %f %f",&vec1.x,&vec1.y,&vec1.z);
	sscanf(argv[2],"%f %f %f",&vec2.x,&vec2.y,&vec2.z);
	return floatToStr(m_dot(vec1,vec2));
}

static const char origin3text[] = "0 0 0";

static const char *c_addVec(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to %s(vec,vec);",argv[0]);
		return origin3text;
	}
	Point3F vec1,vec2,vec3;
	sscanf(argv[1],"%f %f %f",&vec1.x,&vec1.y,&vec1.z);
	sscanf(argv[2],"%f %f %f",&vec2.x,&vec2.y,&vec2.z);
	vec3 = vec1 + vec2;
	sprintf(ReturnBuffer,"%g %g %g",vec3.x,vec3.y,vec3.z);
	return ReturnBuffer;
}

static const char *c_subVec(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {										
		Console->printf("wrong argcount to %s(vec,vec);",argv[0]);
		return origin3text;     // "0 0 0";
	}
	Point3F vec1,vec2,vec3;
	sscanf(argv[1],"%f %f %f",&vec1.x,&vec1.y,&vec1.z);
	sscanf(argv[2],"%f %f %f",&vec2.x,&vec2.y,&vec2.z);
	vec3 = vec1 - vec2;
	sprintf(ReturnBuffer,"%g %g %g",vec3.x,vec3.y,vec3.z);
	return ReturnBuffer;
}

static const char *c_vecDistance(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to %s(vec,vec);",argv[0]);
		return "0";
	}
	Point3F vec1,vec2;
	sscanf(argv[1],"%f %f %f",&vec1.x,&vec1.y,&vec1.z);
	sscanf(argv[2],"%f %f %f",&vec2.x,&vec2.y,&vec2.z);
	return floatToStr(m_dist(vec1,vec2));
}

static const char *c_negVec(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to negVec(vec);");
		return origin3text;     // "0 0 0";
	}
	Point3F vec1;
	sscanf(argv[1],"%f %f %f",&vec1.x,&vec1.y,&vec1.z);
	sprintf(ReturnBuffer,"%g %g %g",-vec1.x,-vec1.y,-vec1.z);
	return ReturnBuffer;
}

static const char *c_normalize(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to Vector::normalize(vec);");
		return "false";
	}
	Point3F vec;
	sscanf(argv[1],"%f %f %f",&vec.x,&vec.y,&vec.z);
	vec.normalize();
	sprintf(ReturnBuffer,"%g %g %g",vec.x,vec.y,vec.z);
	return ReturnBuffer;
}

static const char *c_setVelocity(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		return "false";
	}
   GameBase *gb = findGameObject(argv[1]);
   if(!gb)
      return "False";
	Point3F vec(0,0,0);
	sscanf(argv[2],"%f %f %f",&vec.x,&vec.y,&vec.z);

   gb->setLinearVelocity(vec);
	return "True";
}

static const char *c_getVelocity(CMDConsole *, int, int argc, const char **argv)
{
   static char ReturnBuffer[256];
	if (argc != 2) {
		return "0 0 0";
	}
   GameBase *gb = findGameObject(argv[1]);
   if(!gb)
      return "0 0 0";
   Point3F vec = gb->getLinearVelocity();
	sprintf(ReturnBuffer,"%g %g %g",vec.x,vec.y,vec.z);
   return ReturnBuffer;
}

static const char *c_nameToID(CMDConsole *, int, int argc, const char **argv)
{
   if(!wg->manager)
      return "-1";

	// item count
	if (argc != 2) 
	{
		Console->printf("wrong argcount to namToID(name)");
		return "-1";
	}
	if( SimObject *simobj = wg->manager->findObject(argv[1]))
		return intToStr(simobj->getId());
	return ("-1");
}


static const char *c_getWord(CMDConsole *, int, int argc, const char **argv)
{
	// item count
	if (argc != 3) 
	{
		Console->printf("wrong argcount to getWord(String,nWord)");
		return "-1";
	}
	static char buffer[1024];
   int nWord = atoi(argv[2]);
   if(nWord < 0)
      return "-1";
	int counter = 0;
	int index = 0;

   const char *pt = argv[1];
   for(;;)
   {
      // strip off leading spaces
      while(*pt == ' ')
         pt++;
      if(!*pt)
         return "-1";
      if(nWord == counter)
      {
         char *bptr = buffer;
         while(*pt != ' ' && *pt)
            *bptr++ = *pt++;
         *bptr = 0;
         return buffer;
      }
      else
      {
         counter++;
         while(*pt && *pt != ' ')
            pt++;
      }
   }
}

static const char *c_sqrt(CMDConsole *, int, int argc, const char **argv)
{
	if (argc !=2 ) 
	{
		Console->printf("wrong argcount to sqrt(Value)");
		return "-1";
	}
	float arg = atof (argv[1]);
	if (arg < 0)
		arg = 0;
	return floatToStr(sqrt(arg));	
}

static const char *c_pow(CMDConsole *, int, int argc, const char **argv)
{
	if (argc !=3 ) 
	{
		Console->printf("wrong argcount to pow(value, power)");
		return "-1";
	}
	return floatToStr(pow(atof(argv[1]),atof(argv[2])));	
}

static const char *c_getBoxCenter(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) 
		Console->printf("wrong argcount to getBoxCenter(object)");
	else
		if (GameBase *obj = findGameObject(argv[1])) {
			Point3F boxCenter = obj->getBoxCenter();
			sprintf(ReturnBuffer,"%g %g %g",boxCenter.x,boxCenter.y,boxCenter.z);
			return ReturnBuffer;
		}
	return "0 0 0";	
}

static const char *c_sprintf(CMDConsole *, int, int argc, const char **argv)
{	
	if (argc < 2 || argc > 10) 
		Console->printf("wrong argcount to %s(string, [Arg1 ... Arg9] )",argv[0]);
	else {	
		static char holder[1024];
		const char *ptrStr = argv[1],*argStr;
		char numTest,*ptrHold = holder;
		while(*ptrStr) {
			if(*ptrStr == '%') {
				numTest = *(ptrStr + 1);
				if(numTest > 48 && numTest < 58) {
					argStr = argv[numTest - 47];
					while(*argStr) {
						*ptrHold = *argStr; 
						argStr++;
						ptrHold++;
					}
					ptrStr+=2;
				}
			} 	
			while(*ptrStr && *ptrStr != '%') {
				*ptrHold = *ptrStr; 
				ptrStr++;
				ptrHold++;
			}
		}
		*ptrHold = 0;
		return holder;
	}
	return "false";
}

static const char *c_applyRadiusDamage(CMDConsole *, int, int argc, const char **argv)
{	
	if (argc != 7) 
		Console->printf("wrong argcount to %s(type,pos,radius,damageValue,force,srcObjectId)",argv[0]);
	else
		if (sg.manager) {
			Point3F pos;
			sscanf(argv[2],"%g %g %g",&pos.x,&pos.y,&pos.z);
			Explosion::applyRadiusDamage(sg.manager,atoi(argv[1]),
				pos, Point3F(0,0,0), atof(argv[3]), atof(argv[4]),
				atof(argv[5]), atoi(argv[6]));		
			return "true";
		}
	return "false";
}

static const char *c_getRadius(CMDConsole *, int, int argc, const char **argv)
{	
	if (argc != 2) 
		Console->printf("wrong argcount to %s(ObjectId)",argv[0]);
	else {	
		if(ShapeBase* obj = findShapeBaseObject(argv[1]))
			return floatToStr(obj->getRadius());
	}
	return "false";
}

static const char *c_getDisabledDamage(CMDConsole *, int, int argc, const char **argv)
{	
	if (argc != 2) 
		Console->printf("wrong argcount to %s(object)",argv[0]);
	else {	
		if (StaticBase* obj = findStaticBaseObject(argv[1]))
			return floatToStr(obj->getDisableLevel());
	}
	return "false";
}

//----------------------------------------------------------------------------

static const char *c_getObjectType(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to getObjectType(object);");
		return "False";
	}
	if (sg.manager) {
		if (SimObject* obj = sg.manager->findObject(argv[1])) {
			return obj->getClassName();
		}
		Console->printf("getObjectType: could not find object");
	}
	return "False";
}	


static const char *c_getTargObjectByIndex(CMDConsole *, int, int argc, const char **argv)
{
   if (argc != 2 || !sg.manager)
   {
		return "-1";
	}
   int index = atoi(argv[1]);
   
   //get the sensor manager
   SimObject *mgr = sg.manager->findObject(SensorManagerId);
   if(!mgr)
      return "-1";
   SensorManager *sensorMngr = dynamic_cast<SensorManager*>(mgr);
   if (! sensorMngr)
      return "-1";
   
   //find the target in the array
   if (! sensorMngr->targets[index])
      return "-1";
   return intToStr(sensorMngr->targets[index]->getId());
}

//----------------------------------------------------------------------------

static const char *c_containerBoxFillSet(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 7 && argc !=8) {
		Console->printf("wrong argcount to containerBoxFillSet(set,mask,position,length,width,height,optional height);");
		return "False";
	}

	Point3F pos(0,0,0);
	Point3F max; 
	Point3F min; 
	SimContainerQuery query;
	SimContainerList iList;

	SimObject *sobj = sg.manager->findObject(argv[1]);
	SimSet *set = dynamic_cast<SimSet*>(sobj);
	if (!set)
		return "False";

	BitSet32 mask = (BitSet32)atof(argv[2]);
//	mask = (BitSet32)atof(argv[2]);
	float length = (atof(argv[4]))/2;
	float width  = (atof(argv[5]))/2;
	float height = (atof(argv[6]))/2;

	sscanf(argv[3],"%f %f %f",&pos.x,&pos.y,&pos.z);
	max.x = pos.x + length;   
	max.y = pos.y + width;
	max.z = pos.z + height;

	min.x = pos.x - length;
	min.y = pos.y - width;
	if(atoi(argv[7]) == 1)
		min.z = pos.z;
	else
		min.z = pos.z - height;

	Box3F box(min,max);
	query.id = -1;
	query.type = -1;
	query.mask = mask; 
	query.detail = SimContainerQuery::DefaultDetail;	  
	query.box = box;
	
	SimContainer* root = findObject(sg.manager,SimRootContainerId,root);
	root->findIntersections(query,&iList);

	for (SimContainerList::iterator itr = iList.begin(); itr != iList.end(); itr++) 
		set->addObject((*itr));

	return intToStr(iList.size());
}

//----------------------------------------------------------------------------
// Client
//----------------------------------------------------------------------------

static const char *c_setMenuScoreVis(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 3 || !sg.playerManager)
   {
      Console->printf("invalid call to %s()", argv[0]);
      return "0";
   }
   sg.playerManager->setDisplayScores(atoi(argv[1]), CMDConsole::getBool(argv[2]));
   return "1";
}

static const char *c_clientGetFirst(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 1 || !sg.playerManager)
   {
      Console->printf("invalid call to %s()", argv[0]);
      return "-1";
   }
   PlayerManager::ClientRep *cl = sg.playerManager->getClientList();
   if(!cl)
      return "-1";
   else
      return intToStr(cl->id);
}

static const char *c_clientGetNext(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2 || !sg.playerManager)
   {
      Console->printf("invalid call to %s(client)", argv[0]);
      return "-1";
   }
   PlayerManager::ClientRep *cl = sg.playerManager->findClient(atoi(argv[1]));
   if(!cl || !cl->nextClient)
      return "-1";
   else
      return intToStr(cl->nextClient->id);
}

static const char *c_clientGetOwnedObject(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2 || !sg.playerManager)
   {
      Console->printf("invalid call to %s(client)", argv[0]);
      return "-1";
   }
   PlayerManager::ClientRep *cl = sg.playerManager->findBaseRep(atoi(argv[1]));
   if(!cl || !cl->ownedObject)
      return "-1";
   else
      return intToStr(cl->ownedObject->getId());   
}

static const char *c_clientSetOwnedObject(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 3 || !sg.playerManager)
   {
      Console->printf("invalid call to %s(client, object)", argv[0]);
      return "false";
   }
   PlayerManager::ClientRep *cl = sg.playerManager->findBaseRep(atoi(argv[1]));
   GameBase *gb = findGameObject(argv[2]);
   if(!cl)
      return "false";
   if(!gb)
   {
      if(cl->ownedObject)
         cl->ownedObject->setOwnerClient(0);
   }
   else
      gb->setOwnerClient(cl->id);
   return "true";
}

static const char *c_clientGetControlObject(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2 || !sg.playerManager) {
      Console->printf("invalid call to %s(client)", argv[0]);
      return "-1";
   }
   PlayerManager::ClientRep *cl = sg.playerManager->findClient(atoi(argv[1]));
   if(!cl || !cl->controlObject)
      return "-1";
   else
      return intToStr(cl->controlObject->getId());   
}

static const char *c_clientSetControlObject(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 3 || !sg.playerManager) {
      Console->printf("invalid call to %s(client, object)", argv[0]);
      return "false";
   }
   PlayerManager::ClientRep *cl = sg.playerManager->findClient(atoi(argv[1]));
   GameBase *gb = findGameObject(argv[2]);
   if(!cl)
      return "false";
   if(!gb)
   {
      if(cl->controlObject)
         cl->controlObject->setControlClient(0);
   }
   else
      gb->setControlClient(cl->id);
   return "true";
}

static const char *c_getOwnerClient(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2 || !sg.playerManager)
   {
      Console->printf("invalid call to %s(object)", argv[0]);
      return "-1";
   }
   GameBase *gb = findGameObject(argv[1]);
   if(!gb)
      return "-1";
   else
      return gb->getOwnerClient() ? intToStr(gb->getOwnerClient()) : "-1";
}

static const char *c_getControlClient(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2 || !sg.playerManager)
   {
      Console->printf("invalid call to %s(object)", argv[0]);
      return "-1";
   }
   GameBase *gb = findGameObject(argv[1]);
   if(!gb)
      return "-1";
   else
      return gb->getControlClient() ? intToStr(gb->getControlClient()) : "-1";
}

//----------------------------------------------------------------------------
static const char *c_clientGetVoice(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2 || !sg.playerManager)
   {
      Console->printf("invalid call to %s(client)", argv[0]);
      return "";
   }
   PlayerManager::ClientRep *cl = sg.playerManager->findBaseRep(atoi(argv[1]));
   if(!cl)
      return "";
   return cl->voiceBase;
}

static const char *c_clientGetSkin(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2 || !sg.playerManager)
   {
      Console->printf("invalid call to %s(client)", argv[0]);
      return "";
   }
   PlayerManager::ClientRep *cl = sg.playerManager->findBaseRep(atoi(argv[1]));
   if(!cl)
      return "";
   return cl->skinBase;
}

static const char *c_clientGetGender(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2 || !sg.playerManager)
   {
      Console->printf("invalid call to %s(client)", argv[0]);
      return "";
   }
   PlayerManager::ClientRep *cl = sg.playerManager->findBaseRep(atoi(argv[1]));
   if(!cl)
      return "";
   return cl->gender == PlayerManager::Male ? "Male" : "Female";
}

static const char *c_clientSetDataFinished(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2 || !sg.playerManager)
   {
      Console->printf("invalid call to %s(client)", argv[0]);
      return "";
   }
   PlayerManager::ClientRep *cl = sg.playerManager->findBaseRep(atoi(argv[1]));
   if(!cl)
      return "";
   cl->dataFinished = true;
   return "True";
}

static const char *c_getMouseSensitivity(CMDConsole *, int, int argc, const char **argv)
{
   if (argc != 2)
   {
      Console->printf("invalid call to %s(actionMap)", argv[0]);
      return "";
   }
   
   float value = FearGui::CFGButton::getMouseSensitivity(argv[1]);
   return floatToStr(value);
}

static const char *c_getMouseXAxisFlip(CMDConsole *, int, int argc, const char **argv)
{
   if (argc != 2)
   {
      Console->printf("invalid call to %s(actionMap)", argv[0]);
      return "";
   }
   
   if (FearGui::CFGButton::getMouseXAxisFlip(argv[1])) return "TRUE";
   else return "FALSE";
}

static const char *c_getMouseYAxisFlip(CMDConsole *, int, int argc, const char **argv)
{
   if (argc != 2)
   {
      Console->printf("invalid call to %s(actionMap)", argv[0]);
      return "";
   }
   
   if (FearGui::CFGButton::getMouseYAxisFlip(argv[1])) return "TRUE";
   else return "FALSE";
}

//----------------------------------------------------------------------------

static const char *c_setItemShoppingList(CMDConsole *, int, int argc, const char **argv)
{
	if (argc < 2) {
		Console->printf("wrong argcount to setItemShoppingList(client,<type,...>)");
		return "False";
	}
   if (PlayerManager::ClientRep *p = 
   		sg.playerManager->findBaseRep(atoi(argv[1]))) {
		if (argc == 2)
			// Default is set them all
			p->playerPSC->setItemShoppingList();
		else {
			int type;
			for (int i = 2; i < argc; i++)
				if ((type = Player::getItemType(argv[i])) != -1)
					p->playerPSC->setItemShoppingList(type);
		}
		return "True";
	}
	return "False";
}

static const char *c_clearItemShoppingList(CMDConsole *, int, int argc, const char **argv)
{
	if (argc < 2) {
		Console->printf("wrong argcount to clearItemShoppingList(client,<type,...>)");
		return "False";
	}
   if (PlayerManager::ClientRep *p = 
   		sg.playerManager->findBaseRep(atoi(argv[1]))) {
		if (argc == 2)
			// Default is clear everything.
			p->playerPSC->clearItemShoppingList();
		else {
			int type;
			for (int i = 2; i < argc; i++)
				if ((type = Player::getItemType(argv[i])) != -1)
					p->playerPSC->clearItemShoppingList(type);
		}
		return "True";
	}
	return "False";
}


static const char *c_isItemShoppingOn(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to isItemShoppingOn(client,type)");
		return "False";
	}
   if (PlayerManager::ClientRep *p = 
   		sg.playerManager->findBaseRep(atoi(argv[1]))) {
		int type = Player::getItemType(argv[2]);
		if (type != -1 && p->playerPSC->isItemShoppingOn(type))
			return "True";
	}
	return "False";
}

static const char *c_setItemBuyList(CMDConsole *, int, int argc, const char **argv)
{
	if (argc < 2) {
		Console->printf("wrong argcount to setItemBuyList(client,<type,...>)");
		return "False";
	}
   if (PlayerManager::ClientRep *p = 
   		sg.playerManager->findBaseRep(atoi(argv[1]))) {
		if (argc == 2)
			// Default is set them all
			p->playerPSC->setItemBuyList();
		else {
			int type;
			for (int i = 2; i < argc; i++)
				if ((type = Player::getItemType(argv[i])) != -1)
					p->playerPSC->setItemBuyList(type);
		}
		return "True";
	}
	return "False";
}

static const char *c_clearItemBuyList(CMDConsole *, int, int argc, const char **argv)
{
	if (argc < 2) {
		Console->printf("wrong argcount to clearItemBuyList(client,<type,...>)");
		return "False";
	}
   if (PlayerManager::ClientRep *p = 
   		sg.playerManager->findBaseRep(atoi(argv[1]))) {
		if (argc == 2)
			// Default is clear everything.
			p->playerPSC->clearItemBuyList();
		else {
			int type;
			for (int i = 2; i < argc; i++)
				if ((type = Player::getItemType(argv[i])) != -1)
					p->playerPSC->clearItemBuyList(type);
		}
		return "True";
	}
	return "False";
}


static const char *c_isItemBuyOn(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to isItemBuyOn(client,type)");
		return "False";
	}
   if (PlayerManager::ClientRep *p = 
   		sg.playerManager->findBaseRep(atoi(argv[1]))) {
		int type = Player::getItemType(argv[2]);
		if (type != -1 && p->playerPSC->isItemBuyOn(type))
			return "True";
	}
	return "False";
}

static const char *c_setClientScore(CMDConsole *, int, int argc, const char **argv)
{
   if(!sg.playerManager)
      return "False";
   if(argc != 4)
   {
      Console->printf("wrong argcount to %s(playerId, scoreString, sortValue);", argv[0]);
      return "False";
   }
   sg.playerManager->setClientScore(atoi(argv[1]), argv[2], atoi(argv[3]));
   return "True";
}

static const char *c_setClientScoreHeading(CMDConsole *, int, int argc, const char **argv)
{
   if(!sg.playerManager)
      return "False";
   if(argc != 2)
   {
      Console->printf("wrong argcount to %s(headingString);", argv[0]);
      return "False";
   }
   sg.playerManager->setClientScoreHeading(argv[1]);
   return "True";
}

static const char *c_setClientSkin(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 3 || !sg.playerManager)
      return "False";
   sg.playerManager->setClientSkin(atoi(argv[1]), argv[2]);
   return "True";
}

static const char *c_setTeamScore(CMDConsole *, int, int argc, const char **argv)
{
   if(!sg.playerManager)
      return "False";
   if(argc != 4)
   {
      Console->printf("wrong argcount to %s(playerId, scoreString, sortValue);", argv[0]);
      return "False";
   }
   sg.playerManager->setTeamScore(atoi(argv[1]), argv[2], atoi(argv[3]));
   return "True";
}

static const char *c_setTeamScoreHeading(CMDConsole *, int, int argc, const char **argv)
{
   if(!sg.playerManager)
      return "False";
   if(argc != 2)
   {
      Console->printf("wrong argcount to %s(headingString);", argv[0]);
      return "False";
   }
   sg.playerManager->setTeamScoreHeading(argv[1]);
   return "True";
}

static const char *c_getPlayerName(CMDConsole *, int, int argc, const char **argv)
{
   if(!wg->playerManager)
      return "";
   if(argc != 2)
   {
      Console->printf("wrong argcount to getPlayerName(playerId);");
      return "";
   }
   PlayerManager::ClientRep *p = wg->playerManager->findBaseRep(atoi(argv[1]));
   if(p)
      return p->name;
   else
      return "";
}

static const char *c_setClientGuiMode(CMDConsole *, int, int argc, const char **argv)
{
   if(!sg.playerManager)
      return "False";
   if(argc != 3)
   {
      Console->printf("wrong argcount to %s(playerId,guiId);",argv[0]);
      return "False";
   }
   PlayerManager::ClientRep *p = sg.playerManager->findClient(atoi(argv[1]));
   if(p)
   {
      if(!p->dataFinished)
         return "False";

      int newGuiMode = atoi(argv[2]);
      int oldMode = p->playerPSC->getCurrentGuiMode();

      if(newGuiMode >= PlayerPSC::InitialGuiMode && newGuiMode < PlayerPSC::NumGuiModes && newGuiMode != oldMode)
      {
         p->playerPSC->setCurrentGuiMode(newGuiMode);
         sg.playerManager->clientGuiModeChanged(p->id, newGuiMode, oldMode);
      }
      return "True";
   }
   else
      return "False";
}

static const char *c_getClientGuiMode(CMDConsole *, int, int argc, const char **argv)
{
   if(!sg.playerManager)
      return "-1";
   if(argc != 2)
      Console->printf("wrong argcount to %s(playerId);",argv[0]);
	else {
	   if (PlayerManager::ClientRep *p = sg.playerManager->findClient(atoi(argv[1])))
			return intToStr(p->playerPSC->getCurrentGuiMode());
	}
   return "-1";
}

static const char *c_exitLobbyMode(CMDConsole *, int, int, const char **)
{
   if (cg.playDelegate) cg.playDelegate->activate();
   return "TRUE";
}

static const char *c_resetPlayDelegate(CMDConsole *, int, int, const char **)
{
   if (cg.playDelegate) cg.playDelegate->reset();
   return "TRUE";
}

static const char *c_messageToPlayer(CMDConsole *, int, int argc, const char **argv)
{
   if(!sg.playerManager)
      return "False";
   int msgsrc;
   if(argc == 4)
      msgsrc = 0;
   else if(argc == 5)
      msgsrc = atoi(argv[4]);
   else
      return "false";
   
   sg.playerManager->messageClient(atoi(argv[1]), msgsrc, atoi(argv[2]), argv[3]);
   return "True";
}

static const char *c_getNumPlayers(CMDConsole *, int, int, const char **)
{
   if(!sg.playerManager)
      return "0";
   static char buffer[10];
   sprintf(buffer, "%d", sg.playerManager->getNumClients());
   return buffer;
}

static const char *c_getPlayerByIndex(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
      return "0";
   int pi = atoi(argv[1]);

   if(!sg.playerManager || pi < 0 || pi >= sg.playerManager->getNumClients())
      return "0";
   static char buffer[10];
   PlayerManager::ClientRep *cl = sg.playerManager->getClientList();
   while(pi--)
      cl = cl->nextClient;

   sprintf(buffer, "%d", cl->id);   
   return buffer;
}

static const char *c_getNumTeams(CMDConsole *, int, int , const char **)
{
   if(!sg.playerManager)
      return "0";
   static char buffer[10];
   sprintf(buffer, "%d", sg.playerManager->getNumTeams());
   return buffer;
}

static const char *c_getTeamName(CMDConsole *, int, int argc, const char **argv)
{
   if(!sg.playerManager || argc != 2)
      return "0";
	PlayerManager::TeamRep *t = sg.playerManager->findTeam(atoi(argv[1]));
	if(!t)
		return "0";
	return t->name;
}

static const char *c_getPlayerTeam(CMDConsole *, int, int argc, const char **argv)
{
   if(!wg->playerManager)
      return "-1";
   if(argc != 2)
   {
      Console->printf("wrong argcount to getPlayerName(playerId);");
      return "-1";
   }
   PlayerManager::ClientRep *p = wg->playerManager->findBaseRep(atoi(argv[1]));
   if(p)
   {
      static char buffer[10];
      sprintf(buffer, "%d", p->team);
      return buffer;
   }
   else
      return "-1";
}

static const char *c_setInitialTeam(CMDConsole *, int, int argc, const char **argv)
{
   if(!sg.playerManager)
      return "False";
   if(argc != 3)
   {
      Console->printf("wrong argcount to setInitialTeam(playerId, teamId);");
      return "False";
   }
   int playerId = atoi(argv[1]);
   int teamId = atoi(argv[2]);

   PlayerManager::ClientRep *p = sg.playerManager->findBaseRep(playerId);
   if(teamId >= 0 && teamId < sg.playerManager->getNumTeams() && p)
   {
      p->team = teamId;
      return "True";
   }
   else
      return "False";
}

static const char *c_setTeamObjective(CMDConsole *, int, int argc, const char **argv)
{
   if(!sg.playerManager)
      return "0";
      
   if(argc != 4)
   {
      Console->printf("wrong argcount to Team::setObjective(teamId, objNum, text);");
      return "0";
   }
   else
   {
      sg.playerManager->setTeamObjective(atoi(argv[1]), atoi(argv[2]), argv[3]);
   }
   return "0";
}

static const char *c_clearTeamObjectives(CMDConsole *, int, int argc, const char **argv)
{
   if(!sg.playerManager)
      return "0";
      
   if(argc != 2)
   {
      Console->printf("wrong argcount to Team::setObjective(teamId);");
      return "0";
   }
   else
   {
      sg.playerManager->clearTeamObjectives(atoi(argv[1]));
   }
   return "0";
}

static const char* verifyTerrainPrefDist(CMDConsole* console,int,int,
	const char *argv[])
{
	float val = atof(argv[1]);
	if (val < 0.0f)
		console->setFloatVariable(argv[0],0.0);
	return "";
}

static const char *c_rebuildCommandMap(CMDConsole *, int, int, const char **)
{
   if(cg.commandTS)
   {
      // notify the command gui that it's time to render its
      // squares

      cg.commandTS->buildMap();
      return "True";
   }
   return "False";
}

//----------------------------------------------------------------------------
// Misc client
//----------------------------------------------------------------------------
static const char *c_getPlayerByName(CMDConsole *, int, int argc, const char **argv)
{
   if((argc != 2) || (! cg.playerManager))
      return "0";

   static char buffer[256];
   PlayerManager::ClientRep *cl = cg.playerManager->getClientList();
   while(cl)
   {
      if (! stricmp(argv[1], cl->name)) break;
      cl = cl->nextClient;
   }
   
   if (! cl) return "0";

   sprintf(buffer, "%d", cl->id);   
   return buffer;
}


//----------------------------------------------------------------------------
// Client database-like functions
//----------------------------------------------------------------------------

static const char *c_addGameServer(CMDConsole *, int, int argc, const char **argv)
{
   if (argc < 6)
   {
      Console->printf("addGameServer(address, name, version, ping, favorite <TRUE OR FALSE>);");
      return "FALSE";
   }
   else if (cg.gameServerList)
   {
      cg.gameServerList->addGameServer(argc, argv);
      return "TRUE";
   }
   else
   {
      Console->printf("Unable to locate the GAME server list control");
      return "FALSE";
   }
}

//----------------------------------------------------------------------------
// Misc network
//----------------------------------------------------------------------------
static const char *c_rebuildServerList(CMDConsole *, int, int, const char **)
{
	SimManager* manager = SimGame::get()->getManager(SimGame::FOCUSED);
   FearCSDelegate *del = (FearCSDelegate *) manager->findObject(SimCSDelegateId);
   if(del) del->rebuildServerList();
   else Console->printf("rebuildServerList(): no CSDelegate installed.");
   return "";
}

//----------------------------------------------------------------------------
// Misc support
//----------------------------------------------------------------------------

static int getItemDescriptionType(const char* desc)
{
	// Convert item description into item typeId
	int itemTypeCount = cg.dbm->getDataGroupSize(DataBlockManager::ItemDataType);
	for (int i = 0; i < itemTypeCount; i++) {
		Item::ItemData* data = static_cast<Item::ItemData*>
			(cg.dbm->lookupDataBlock(i,DataBlockManager::ItemDataType));
		if (!stricmp(desc,data->description))
			return i;
	}
	return -1;
}

static const char *c_getItemData(CMDConsole *, int, int argc, const char **argv)
{
	// Convert item typeId into item name
	if (argc != 2) {
		Console->printf("wrong argcount to %s(type)",argv[0]);
		return "False";
	}
	int typeId = atoi(argv[1]);
	if (typeId >= 0 && typeId < wg->dbm->getDataGroupSize(DataBlockManager::ItemDataType)) {
		if (const char* name = wg->dbm->lookupBlockName(typeId,DataBlockManager::ItemDataType))
			return name;
	}
	return "";
}

static const char *c_getNumItems(CMDConsole *, int, int argc, const char **argv)
{
	// Convert item typeId into item name
	if (argc != 1) {
		Console->printf("wrong argcount to %s()",argv[0]);
		return "False";
	}
	return intToStr(wg->dbm->getDataGroupSize(DataBlockManager::ItemDataType));
}

static const char *c_getClientItemType(CMDConsole *, int, int argc, const char **argv)
{
	// Convert item description into item name
	if (argc != 2) {
		Console->printf("wrong argcount to %s(description)",argv[0]);
		return "False";
	}
	return intToStr(getItemDescriptionType(argv[1]));
}

static const char *c_getClientItemCount(CMDConsole *, int, int argc, const char **argv)
{
	// Get's item count on the client
	if (argc != 2) {
		Console->printf("wrong argcount to %s(description)",argv[0]);
	}
	else {
		int type = getItemDescriptionType(argv[1]);
		if (cg.psc && type != -1)
			return intToStr(cg.psc->itemCount(type));
	}
	return "0";
}

static const char *c_getClientMountedItem(CMDConsole *, int, int argc, const char **argv)
{
	// Get's item count on the client
	if (argc != 2) {
		Console->printf("wrong argcount to %s(mountSlot)",argv[0]);
	}
	else {
		int imageSlot = atoi(argv[1]);
		if (cg.player && imageSlot >= 0 && imageSlot < Player::MaxImageSlots) {
			return intToStr(cg.player->getMountedItem(imageSlot));
		}
	}
	return "-1";
}

static const char* c_activateGroup(CMDConsole *, int, int argc, const char **argv)
{
	// Triggers everything in object
	if (argc != 2 && argc != 3) {
		Console->printf("wrong argcount to %s(%object1,<value>)",argv[0]);
		return "False";
	}
	SimObject* obj = wg->manager->findObject(argv[1]);
	if (obj) {
		SimGroup* group = dynamic_cast<SimGroup*>(obj);
		if (!group)
			group = obj->getGroup();

		// Trigger everything in the group except the object itself.
		float value = (argc > 2)? atof(argv[2]): 0.0f;
		SimTriggerEvent event(SimTriggerEvent::Activate, value);
		for (SimGroup::iterator iter = group->begin(); iter < group->end(); iter++)
			if (*iter != obj)
				(*iter)->processEvent(&event);
		return "True";
	}
	return "False";
}

static const char *c_getRandom(CMDConsole *, int, int , const char **)
{
	static Random random;
	return floatToStr(random.getFloat(0,1.0));
}

static const char *c_getRotationFromNormal(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to %s(normal)",argv[0]);
	}
	else {
		EulerF rot;
		Point3F normal(0,0,0);
		sscanf(argv[1],"%f %f %f",&normal.x,&normal.y,&normal.z);
		if (normal.y + normal.x == 0) {
			rot.y = rot.y = 0;
			rot.x = normal.z >= 0? 0: M_PI;
		}
		else {
			rot.z = -atan2(normal.x,normal.y);
			rot.x = -atan2(m_hyp(normal.x,normal.y),normal.z);
			rot.y = 0;
		}
		sprintf(ReturnBuffer,"%g %g %g",rot.x,rot.y,rot.z);
		return ReturnBuffer;
	}
	return origin3text;
}

static const char * c_getWorldFromWaypoint(CMDConsole *, int, int argc, const char **argv)
{
   if (argc != 2) {
      Console->printf("%s <waypoint coords>",argv[0]);
      Console->printf("Converts waypoint coords (x,y in [0,1023]) to world coords.");
   }
   else {
      Point2F  waypt(0,0), world = sg.missionCenterExt;
      
      sscanf(argv[1], "%f %f", &waypt.x, &waypt.y);
      waypt /= 1024.0;
      world.x *= waypt.x;
      world.y *= waypt.y;
      world += sg.missionCenterPos;
      
      sprintf(ReturnBuffer,"%g %g 0", world.x, world.y );
      return ReturnBuffer;
   }
   return origin3text;
}

static const char *c_getVecFromRot(CMDConsole *, int, int argc, const char **argv)
{
	if (argc < 2 || argc > 4 ) {
		Console->printf( "%s: <Rot 3F> [<length>]", argv[0] );
		Console->printf( "Returns a vector in the direction given by <Rot 3F>," );
		Console->printf( "of the given length if supplied, else of unit length." );
	   return origin3text;     // "0 0 0";
	}
	else {
		EulerF rot;
      rot.set();
		sscanf( argv[1], "%f %f %f", &rot.x, &rot.y, &rot.z );
      RMat3F   mat( rot );

      // Get a vector looking 'straight ahead' - up the Y axis - and run it through
      //    the matrix.  
      Point3F  p_in ( 0.0, 1.0, 0.0 ), p_out;
      if( argc > 2 )
         p_in.y = atof( argv[2] );
		if(argc > 3)
			p_in.z = atof( argv[3] );

      m_mul( p_in, mat, & p_out );
      
		sprintf(ReturnBuffer,"%g %g %g", p_out.x, p_out.y, p_out.z );
		return ReturnBuffer;
	}
}

static const char *c_findPointOnSphere(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 5) 
		Console->printf( "%s: <playerPos, objectPos, vec, object>", argv[0]);
	else {
		Point3F E,O,EO,V,P,finalVec;
		float  r,v,disc,d,val;
		int scanCount;
		scanCount =  sscanf( argv[1], "%f %f %f", &E.x, &E.y, &E.z );
		scanCount += sscanf( argv[2], "%f %f %f", &O.x, &O.y, &O.z );
		scanCount += sscanf( argv[3], "%f %f %f", &V.x, &V.y, &V.z );
		if (scanCount == 9)
			if(ShapeBase* obj = findShapeBaseObject(argv[4])) {
				r = obj->getRadius()-0.5;
            float len = V.len();
            if(len < 0.01)
               return "0 0 0";
               
				V /= len;
				EO = O - E ;
				v = m_dot(EO,V);
				disc = (r * r) - ((m_dot(EO,EO)) - (v * v));
				d = (disc < 0)? sqrt(disc*-1): sqrt(disc); 
				val = v - d;
				P = E + (V * val);

				finalVec = O - P;
            len = finalVec.len();
            if(len < 0.01)
               return "0 0 0";

		 		finalVec /= len;

				sprintf(ReturnBuffer,"%g %g %g", finalVec.x, finalVec.y, finalVec.z );
				return ReturnBuffer;
			}
	}
	return "0 0 0";
}


//----------------------------------------------------------------------------
// Misc GameBase functions
//----------------------------------------------------------------------------

static const char *c_generatePower(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to generatePower(object,bool)");
	}
	else {
		if (GameBase* obj = findGameObject(argv[1])) {
			obj->generatePower(CMDConsole::getBool(argv[2]));
			return "True";
		}
	}
	return "False";
}

static const char *c_isPowerGenerator(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to isPowerGenerator(object)");
	}
	else {
		if (GameBase* obj = findGameObject(argv[1])) {
			if (obj->isGenerator())
				return "True";
		}
	}
	return "False";
}

static const char *c_getPowerCount(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to getPowerCount(object)");
	}
	else {
		if (GameBase* obj = findGameObject(argv[1])) {
			return intToStr(obj->getPowerCount());
		}
	}
	return "0";
}

static const char *c_isPowered(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to isPowered(object)");
	}
	else {
		if (GameBase* obj = findGameObject(argv[1])) {
			if (obj->isPowered())
				return "True";
		}
	}
	return "False";
}

static const char *c_getTeam(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to getTream(object)");
	}
	else {
	   if (!sg.manager)
		   return "-1";

	   if (SimObject* sobj = sg.manager->findObject(argv[1]))
		   if (GameBase* gobj = dynamic_cast<GameBase*>(sobj))
			   return intToStr(gobj->getTeam());

      int iPlayerID = atoi(argv[1]);

      // Try looking up the player ID
      if (iPlayerID >= 2049 && iPlayerID <= 2176 && sg.playerManager)
      {
		   if (PlayerManager::ClientRep *p = 
            sg.playerManager->findBaseRep(iPlayerID))
			   return intToStr(p->team);
      }
	}
	return "-1";
}

static const char *c_setTeam(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to setTeam(object,team)");
	}
	else {
		if (GameBase* obj = findGameObject(argv[1]))
      {
			obj->setTeam(atoi(argv[2]));
			return "True";
      }
      else
      {
         int id = atoi(argv[1]);
         PlayerManager::ClientRep *p = sg.playerManager->findBaseRep(id);
         if(!p)
            return "-1";
         sg.playerManager->setTeam(id, atoi(argv[2]));
      }
	}
	return "-1";
}

static const char *c_getPosition(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to %s(object)",argv[0]);
	}
	else {
		if (GameBase* obj = findGameObject(argv[1])) {
			const Point3F& pos = obj->getTransform().p;
			sprintf(ReturnBuffer,"%g %g %g",pos.x,pos.y,pos.z);
			return ReturnBuffer;
		}
	}
	return origin3text;
}

static const char *c_getRotation(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to %s(object)",argv[0]);
	}
	else {
		if (GameBase* obj = findGameObject(argv[1])) {
			EulerF rot;
			obj->getTransform().angles(&rot);
			sprintf(ReturnBuffer,"%g %g %g",rot.x,rot.y,rot.z);
			return ReturnBuffer;
		}
	}
	return origin3text;
}

static const char *c_setPosition(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to %s(object,pos)",argv[0]);
	}
	else {
		if (GameBase* obj = findGameObject(argv[1])) {
			SimObjectTransformEvent event;
			Point3F pos(0,0,0);
			sscanf(argv[2],"%f %f %f",&pos.x,&pos.y,&pos.z);
			event.tmat = obj->getTransform();
			event.tmat.p = pos;
			obj->processEvent(&event);
			return "True";
		}
	}
	return "False";
}

static const char *c_setRotation(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to %s(object,rot)",argv[0]);
	}
	else {
		if (GameBase* obj = findGameObject(argv[1])) {
			SimObjectTransformEvent event;
			EulerF rot(0,0,0);
			sscanf(argv[2],"%f %f %f",&rot.x,&rot.y,&rot.z);
			event.tmat.set(rot,obj->getTransform().p);
			obj->processEvent(&event);
			return "True";
		}
	}
	return "False";
}

static const char *c_getTransform(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to %s(object)",argv[0]);
	}
	else {
		if (GameBase* obj = findGameObject(argv[1])) {
			const TMat3F& mat = obj->getTransform();
			EulerF rot;
			mat.angles(&rot);
			sprintf(ReturnBuffer,"%g %g %g %g %g %g",
				mat.p.x,mat.p.y,mat.p.z,rot.x,rot.y,rot.z);
			return ReturnBuffer;
		}
	}
	return "0 0 0 0 0 0";
}

static const char *c_setTransform(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to %s(object,trans)",argv[0]);
	}
	else {
		if (GameBase* obj = findGameObject(argv[1])) {
			SimObjectTransformEvent event;
			Point3F pos(0,0,0); EulerF rot(0,0,0);
			sscanf(argv[2],"%f %f %f %f %f %f",
				&pos.x,&pos.y,&pos.z,&rot.x,&rot.y,&rot.z);
			event.tmat.set(rot,pos);
			obj->processEvent(&event);
			return "True";
		}
	}
	return "False";
}

static const char *c_throwObject(CMDConsole *, int, int argc, const char **argv)
{
	// player item count
	if (argc != 5) {
		Console->printf("wrong argcount to throwObject(item,object,vel,bool random)");
		return "False";
	}
	if (GameBase* item = findGameObject(argv[1])) {
		if (GameBase* object = findGameObject(argv[2])) {
			item->throwObject(object,atof(argv[3]),CMDConsole::getBool(argv[4]));
			return "True";
		}
		else
			Console->printf("%s: could not find object",argv[0]);
	}
	else
		Console->printf("%s: could not find item to throw",argv[0]);
	return "False";
}

static const char *c_getDataName(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to %s(object)",argv[0]);
	}
	else {
		if (GameBase* obj = findGameObject(argv[1])) {
		   return obj->getDatFileName();
		}
	}
	return "";
}

static const char *c_isAtRest(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to %s(object)",argv[0]);
	}
	else {
		if (GameBase* obj = findGameObject(argv[1])) {
		   if (obj->isAtRest())
				return "True";
		}
	}
	return "False";
}

static const char * c_virtual(CMDConsole *, int, int argc, const char **argv)
{
	// player item count
	if (argc < 3) {
		Console->printf("wrong argcount to %s(object,functionname,...)",argv[0]);
	}
	else {
		if (GameBase* obj = findGameObject(argv[1])) {
			AssertFatal(argc < 40, "GameBase::virtual: Too many arguments");
			const char* larg[40];
			if ((larg[0] = obj->scriptName(argv[2])) != 0) {
				larg[1] = obj->scriptThis();
				for (int i = 0; i < argc - 3; i++)
					larg[2 + i] = argv[i + 3];
				return Console->execute(argc-1,larg);
			}
		}
	}
	return "";
}

static const char *c_setActive(CMDConsole *, int, int argc, const char **argv)
{
	// player item count
	if (argc != 3) {
		Console->printf("wrong argcount to %s(sensor,bool)",argv[0]);
		return "False";
	}
	if (GameBase* shape = findGameObject(argv[1])) {
		if (shape->setActive(CMDConsole::getBool(argv[2])))
			return "True";
	}
	return "False";
}

static const char *c_isActive(CMDConsole *, int, int argc, const char **argv)
{
	// player item count
	if (argc != 2) {
		Console->printf("wrong argcount to %s(sensor)",argv[0]);
		return "False";
	}
	if (GameBase* shape = findGameObject(argv[1]))
		if (shape->isActive())
			return "True";
	return "False";
}

static const char *c_testPosition(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to %s(object,pos)",argv[0]);
	}
	else {
		if (GameBase* obj = findGameObject(argv[1])) {
			Point3F pos(0,0,0);
			sscanf(argv[2],"%f %f %f",&pos.x,&pos.y,&pos.z);
			TMat3F mat = obj->getTransform();
			mat.p = pos;

			SimMovement::MovementInfo sinfo;
			sinfo.image = 0;
			if (obj->testPosition(mat,&sinfo))
				return "True";
		}
	}
	return "False";
}


//----------------------------------------------------------------------------
// ShapeBase functions
//----------------------------------------------------------------------------

static const char *c_setSequenceDir(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 4) {
		Console->printf("wrong argcount to setSequenceDirection(object, thread, bool forward)");
		return "False";
	}
	if (ShapeBase* sbobj = findShapeBaseObject(argv[1])) {
		int slot = atoi(argv[2]);
		if (slot >= 0 && slot < ShapeBase::MaxScriptThreads) {
			if (sbobj->setSequenceDir(slot,CMDConsole::getBool(argv[3])))
				return "True";
		}
	}
	return "False";
}


static const char *c_stopSequence(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to stopSequence(object, thread)");
		return "False";
	}
	if (ShapeBase* sbobj = findShapeBaseObject(argv[1])) {
		int slot = atoi(argv[2]);
		if (slot >= 0 && slot < ShapeBase::MaxScriptThreads) {
			if (sbobj->stopSequence(slot))
				return "True";
		}
	}
	return "False";
}

static const char *c_pauseSequence(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to pauseSequence(object, thread)");
		return "False";
	}
	if (ShapeBase* sbobj = findShapeBaseObject(argv[1])) {
		int slot = atoi(argv[2]);
		if (slot >= 0 && slot < ShapeBase::MaxScriptThreads) {
			if (sbobj->pauseSequence(slot))
				return "True";
		}
	}
	return "False";
}

static const char *c_playSequence(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3 && argc != 4) {
		Console->printf("wrong argcount to playSequence(object, thread, <sequence>)");
		return "False";
	}
	if (ShapeBase* sbobj = findShapeBaseObject(argv[1])) {
		int slot = atoi(argv[2]);
		if (slot >= 0 && slot < ShapeBase::MaxScriptThreads) {
			if (argc == 4) {
				int seq = sbobj->getTSShape().getSequenceIndex(argv[3]);
				if (seq != -1 && sbobj->setSequence(slot,seq))
					return "True";
			}
			else
				if (sbobj->playSequence(slot))
					return "True";
		}
	}
	return "False";
}

static const char *c_setRechargeRate(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to setRechargeRate(object,value)");
		return "False";
	}
	if (ShapeBase* obj = findShapeBaseObject(argv[1])) {
		obj->setRechargeRate(atof(argv[2]));
		return "True";
	}
	return "False";
}

static const char *c_getRechargeRate(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to getRechargeRate(object)");
		return "0";
	}
	if (ShapeBase* obj = findShapeBaseObject(argv[1])) {
		return floatToStr(obj->getRechargeRate());
		//return "True";
	}
	return "0";
}

static const char *c_setAutoRepairRate(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to setAutoRepairRate(object,value)");
		return "False";
	}
	if (ShapeBase* obj = findShapeBaseObject(argv[1])) {
		obj->setAutoRepairRate(atof(argv[2]));
		return "True";
	}
	return "False";
}

static const char *c_getAutoRepairRate(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to getAutoRepairRate(object)");
		return "0";
	}
	if (ShapeBase* obj = findShapeBaseObject(argv[1])) {
		return floatToStr(obj->getAutoRepairRate());
	}
	return "0";
}

static const char *c_repairDamage(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to repairDamage(object,amount)");
		return "False";
	}
	if (ShapeBase* obj = findShapeBaseObject(argv[1])) {
		obj->repairDamage(atof(argv[2]));
		return "True";
	}
	return "False";
}

static const char *c_setEnergy(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to setEnergy(object,value)");
		return "False";
	}
	if (ShapeBase* obj = findShapeBaseObject(argv[1])) {
		obj->setEnergy(atof(argv[2]));
		return "True";
	}
	return "False";
}

static const char *c_getEnergy(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to getEnergy(object)");
		return "0";
	}
	if (ShapeBase* obj = findShapeBaseObject(argv[1])) {
		return floatToStr(obj->getEnergy());
	}
	return "0";
}

static const char *c_getDamageLevel(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to %s(object)",argv[0]);
		return "0";
	}
	if (ShapeBase* obj = findShapeBaseObject(argv[1])) {
		return floatToStr(obj->getDamageLevel());
	}
	return "0";
}

static const char *c_setDamageLevel(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to setDamageLevel(object,value)");
	}
	else {
		if (ShapeBase* obj = findShapeBaseObject(argv[1])) {
			obj->setDamageLevel(atof(argv[2]));
			return "True";
		}
	}
	return "False";
}

static const char *c_getDamageState(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to %s(object)",argv[0]);
		return "0";
	}
	if (ShapeBase* obj = findShapeBaseObject(argv[1])) {
		return obj->getDamageState();
	}
	return "0";
}

static const char *c_applyDamage(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 8) {
		Console->printf("wrong argcount to applyDamage(object,type,value,pos,vec,mom,sourceObj)");
	}
	else {
		if (ShapeBase* obj = findShapeBaseObject(argv[1])) {
			Point3F pos,vec,mom;
			int scanCount;
			scanCount =  sscanf(argv[4],"%f %f %f",&pos.x,&pos.y,&pos.z);
			scanCount += sscanf(argv[5],"%f %f %f",&vec.x,&vec.y,&vec.z);
			scanCount += sscanf(argv[6],"%f %f %f",&mom.x,&mom.y,&mom.z);
			if (scanCount == 9) {
				obj->applyDamage(atoi(argv[2]),atof(argv[3]),
					pos,vec,mom,atoi(argv[7]));
				return "True";
			}
		}
	}
	return "False";
}

static const char *c_activateShield(CMDConsole *, int, int argc, const char **argv)
{
	if (argc < 3 || argc > 4) {
		Console->printf("wrong argcount to activateShield(object,vec,[zOffset])");
	}
	else {
		if (ShapeBase* obj = findShapeBaseObject(argv[1])) {
			Point3F vec;
			float zoff = 0;
			sscanf(argv[2],"%f %f %f",&vec.x,&vec.y,&vec.z);
			if (argc == 4)
				sscanf (argv[3], "%f",&zoff);
			obj->setShieldVec(vec, zoff);
			return "True";
		}
	}
	return "False";
}

static const char *c_getLOSInfo(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3 && argc != 4) {
		Console->printf("wrong argcount to %s(object,range,<rotation>)",argv[0]);
		return "False";
	}
	if (ShapeBase* obj = findShapeBaseObject(argv[1])) {
		int mask = 
			SimTerrainObjectType   | 
			SimInteriorObjectType  | 
			StaticObjectType       |
			SimPlayerObjectType    | 
			VehicleObjectType  | 
			MoveableObjectType |
			ItemObjectType;

		TMat3F mat;
		if (argc == 4) {
			EulerF rot(0,0,0);
			sscanf(argv[3],"%f %f %f",&rot.x,&rot.y,&rot.z);
			TMat3F rmat(rot,Point3F(0,0,0));
			m_mul(rmat,obj->getLOSTransform(),&mat);
		}
		else
			mat = obj->getLOSTransform();

		Point3F pos,normal;
		if (SimObject* target = obj->findLOSObject(mat,atof(argv[2]),mask,&pos,&normal)) {
			sprintf(ReturnBuffer,"%g %g %g",pos.x,pos.y,pos.z);
			Console->setVariable("los::position",ReturnBuffer);
			sprintf(ReturnBuffer,"%g %g %g",normal.x,normal.y,normal.z);
			Console->setVariable("los::normal",ReturnBuffer);
			Console->setVariable("los::object",intToStr(target->getId()));
			return "True";
		}
	}
	return "False";
}

static const char *c_getMapName(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to %s(object)",argv[0]);
	}
	else
		if (ShapeBase* obj = findShapeBaseObject(argv[1])) {
			return obj->getName();
		}
	return "";
}

static const char *c_setMapName(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to %s(object,name)",argv[0]);
	}
	else
		if (ShapeBase* obj = findShapeBaseObject(argv[1])) {
			obj->setName(argv[2]);
			return "True";
		}
	return "False";
}


//----------------------------------------------------------------------------
// Static Shape Base
//----------------------------------------------------------------------------

static const char *c_staticSetIsTarget(CMDConsole *, int, int argc, const char **argv)
{
	// player item count
	if (argc != 3) {
		Console->printf("wrong argcount to %s(staticbase, bool)",argv[0]);
		return "False";
	}
	if (StaticBase* shape = findStaticBaseObject(argv[1]))
		if (shape->setIsTarget(CMDConsole::getBool(argv[2])))
			return "True";
	return "False";
}


//----------------------------------------------------------------------------
// Player
//----------------------------------------------------------------------------
static bool getPlayerAndItem(const char **argv,Player** player,int* type)
{
	if ((*player = findPlayerObject(argv[1])) != 0) {
		if ((*type = Player::getItemType(argv[2])) != -1)
			return true;
		else
      {
         if(atoi(argv[2]) != -1)
			   Console->printf("%s: incorrect item type: %s",argv[0],argv[2]);
      }
	}
	else
		Console->printf("%s: could not find player",argv[0]);
	return false;
}

static const char* c_setPlayerArmor(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to %s(player,armor)",argv[0]);
		return "False";
	}
	if (Player* player = findPlayerObject(argv[1]))
	{
		player->setDatFileName(argv[2]);
      return "True";
   }
	return "False";
}


static const char* c_setPlayerAnimation(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to %s(player,anim)",argv[0]);
		return "False";
	}
	if (Player* player = findPlayerObject(argv[1]))
		player->serverPlayAnim(atoi(argv[2]));
	return "True";
}

static const char* c_getPlayerArmor(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to %s(player)",argv[0]);
		return "False";
	}
	if (Player* player = findPlayerObject(argv[1]))
		return player->getDatFileName();
	return "False";
}

static const char *c_incItemCount(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3 && argc != 4) {
		Console->printf("wrong argcount to incItemCount(player,type,<count>)");
		return "0";
	}
	Player* player; int type;
	if (getPlayerAndItem(argv,&player,&type)) {
		int count = (argc >= 4)? atoi(argv[3]): 1;
		return intToStr(player->incItemCount(type,count));
	}
	return "0";
}

static const char *c_decItemCount(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3 && argc != 4) {
		Console->printf("wrong argcount to decItemCount(player,type,<count>)");
		return "0";
	}
	Player* player; int type;
	if (getPlayerAndItem(argv,&player,&type)) {
		int count = (argc >= 4)? atoi(argv[3]): 1;
		return intToStr(player->decItemCount(type,count));
	}
	return "0";
}

static const char *c_setItemCount(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 4) {
		Console->printf("wrong argcount to setItemCount(player,type,count)");
		return "0";
	}
	Player* player; int type;
	if (getPlayerAndItem(argv,&player,&type))
		return intToStr(player->setItemCount(type,atoi(argv[3])));
	return "0";
}

static const char *c_getItemCount(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to getItemCount(player,type)");
		return "0";
	}
	Player* player; int type;
	if (getPlayerAndItem(argv,&player,&type))
		return intToStr(player->getItemCount(type));
	return "0";
}

static const char *c_getItemClassCount(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to getItemClassCount(player,className)");
		return "0";
	}
	if (Player* player = findPlayerObject(argv[1]))
		return intToStr(player->getItemClassCount(argv[2]));
	return "0";
}

static const char *c_mountItem(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 4 && argc != 5) {
		Console->printf("wrong argcount to mountItem(player,type,imageSlot,<team>)");
		return "False";
	}
	Player* player; int type;
	if (getPlayerAndItem(argv,&player,&type)) {
		int imageSlot = atoi(argv[3]);
		if (imageSlot >= 0 && imageSlot < Player::MaxImageSlots) {
			int team = (argc == 5)? atoi(argv[4]): -1;
			if (player->mountItem(type,imageSlot,team))
				return "True";
		}
		else
			Console->printf("mountItem: invalid imageSlot");
	}
	return "False";
}

static const char *c_unmountItem(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to unmountItem(player,imageSlot)");
		return "False";
	}
	if (Player* player = findPlayerObject(argv[1])) {
		int imageSlot = atoi(argv[2]);
		if (imageSlot >= 0 && imageSlot < Player::MaxImageSlots) {
			if (player->unmountItem(imageSlot))
				return "True";
		}
		else
			Console->printf("unmountItem: invalid imageSlot");
	}
	return "False";
}

static const char *c_getMountedItem(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to %s(player,imageSlot)",argv[0]);
		return "-1";
	}
	if (Player* player = findPlayerObject(argv[1])) {
		int imageSlot = atoi(argv[2]);
		if (imageSlot >= 0 && imageSlot < Player::MaxImageSlots) {
			int item = player->getMountedItem(imageSlot);
			if (item != -1)
				return sg.dbm->lookupBlockName(item,
					DataBlockManager::ItemDataType);
		}
		else
			Console->printf("%s: invalid imageSlot",argv[0]);
	}
	return "-1";
}

static const char *c_getNextMountedItem(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to %s(player,imageSlot)",argv[0]);
		return "-1";
	}
	if (Player* player = findPlayerObject(argv[1])) {
		int imageSlot = atoi(argv[2]);
		if (imageSlot >= 0 && imageSlot < Player::MaxImageSlots) {
			int item = player->getNextMountedItem(imageSlot);
			if (item != -1)
				return sg.dbm->lookupBlockName(item,
					DataBlockManager::ItemDataType);
		}
		else
			Console->printf("%s: invalid imageSlot",argv[0]);
	}
	return "-1";
}

static const char *c_useItem(CMDConsole *, int, int argc, const char **argv)
{
	// player item count
	if (argc != 3) {
		Console->printf("wrong argcount to useItem(player,type)");
		return "False";
	}
	Player* player; int type;
	if (getPlayerAndItem(argv,&player,&type)) {
		if (player->useItem(type))
			return "True";
	}
	return "False";
}

static const char *c_dropItem(CMDConsole *, int, int argc, const char **argv)
{
	// player item count
	if (argc != 3) {
		Console->printf("wrong argcount to dropItem(player,type)");
		return "False";
	}
	Player* player; int type;
	if (getPlayerAndItem(argv,&player,&type)) {
		if (player->dropItem(type))
			return "True";
	}
	return "False";
}

static const char *c_deployItem(CMDConsole *, int, int argc, const char **argv)
{
	// player item count
	if (argc != 3) {
		Console->printf("wrong argcount to deployItem(player,type)");
		return "False";
	}
	Player* player; int type;
	if (getPlayerAndItem(argv,&player,&type)) {
		if (player->deployItem(type))
			return "True";
	}
	return "False";
}

static const char *c_mountObject (CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 4)
	{
		Console->printf ("wrong argcount to mountObject(Player, Object, mountPoint)");
		return "False";
	}
	
	Player *player = findPlayerObject (argv[1]);
	GameBase *object = findGameObject (argv[2]);
   int mountPoint = atoi(argv[3]);
	
	if (player)
		player->setMountObject(object, mountPoint);
		
	return "True";
}

static const char *c_getMountObject (CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2)
	{
		Console->printf ("wrong argcount to %s(Player)",argv[0]);
		return "False";
	}
	Player *player = findPlayerObject (argv[1]);
   if(!player)
      return "-1";
   return player->getMountObject() ? intToStr(player->getMountObject()->getId()) : "-1";
}

static const char *c_trigger(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3 && argc != 4) {
		Console->printf("wrong argcount to trigger(player,imageSlot,<bool>)");
		return "False";
	}
	if (Player* player = findPlayerObject(argv[1])) {
		int imageSlot = atoi(argv[2]);
		if (imageSlot >= 0 && imageSlot < Player::MaxImageSlots) {
			bool trigger;
			if (argc == 4)
				trigger = CMDConsole::getBool(argv[3]);
			else {
				// Toggle the state
				trigger = !player->isImageTriggerDown(imageSlot);
			}
			if (trigger)
				player->setImageTriggerDown(imageSlot);
			else
				player->setImageTriggerUp(imageSlot);
			return "True";
		}
		else
			Console->printf("trigger: invalid imageSlot");
	}
	return "False";
}

static const char *c_isTriggered(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to isTriggered(player,imageSlot)");
		return "False";
	}
	if (Player* player = findPlayerObject(argv[1])) {
		int imageSlot = atoi(argv[2]);
		if (imageSlot >= 0 && imageSlot < Player::MaxImageSlots) {
			if (player->isImageTriggerDown(imageSlot))
				return "True";
		}
		else
			Console->printf("isTriggered: invalid imageSlot");
	}
	return "False";
}

static const char *c_setSensorSupression(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to setSensorSupression(player,value)");
		return "False";
	}
	if (Player* player = findPlayerObject(argv[1])) {
		player->setSupressionField(atof(argv[2]));
		return "True";
	}
	return "False";
}

static const char *c_getSensorSupression(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to getSensorSupression(player)");
		return "0";
	}
	if (Player* player = findPlayerObject(argv[1])) {
		return floatToStr(player->getSupressionField());
	}
	return "0";
}

static const char *c_isDead(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to isDead(player)");
	}
	else
		if (Player* player = findPlayerObject(argv[1]))
			if (player->isDead())
				return "True";
	return "False";
}

static const char *c_isExposed(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to isExposed(player)");
	}
	else
		if (Player* player = findPlayerObject(argv[1]))
			if (player->isExposed())
				return "True";
	return "False";
}

static const char *c_setDetectParams(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 4)
      return "False";
   Player *player = findPlayerObject(argv[1]);
   if(!player)
      return "False";
   player->setDetectParams(atof(argv[2]), atof(argv[3]));
   return "True";
}

static const char *c_isJetting(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2)
		Console->printf("wrong argcount to %s(player)", argv[0]);
	else if (Player* player = findPlayerObject(argv[1]))
	   if (player->isJetting())
		   return "True";
	return "False";
}

static const char *c_lastJetTime(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2)
		Console->printf("wrong argcount to %s(player)", argv[0]);
	else if (Player* player = findPlayerObject(argv[1]))
      return intToStr(player->getLastJetTime());
	return "0";
}

static const char *c_playerBlowUp(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2)
		Console->printf("wrong argcount to %s(player)", argv[0]);
	else if (Player* player = findPlayerObject(argv[1]))
      player->serverBlowUp();

	return "True";
}

static const char *c_applyImpulse(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to applyImpulse(player,vec)");
	}
	else
		if (Player* player = findPlayerObject(argv[1])) {
			Point3F vec;
			sscanf(argv[2],"%f %f %f",&vec.x,&vec.y,&vec.z);
			player->applyImpulse(vec);
		}
	return "False";
}

static const char *c_kill(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to kill(player)");
	}
	else
		if (Player* player = findPlayerObject(argv[1])) {
			player->kill();
			return "True";
		}
	return "False";
}

static const char *c_getDamageFlash(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf("wrong argcount to %s(player)",argv[0]);
	}
	else
		if (Player* player = findPlayerObject(argv[1])) {
			return floatToStr(player->getDamageFlash());
		}
	return "0";
}

static const char *c_setDamageFlash(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to %s(player,value)",argv[0]);
	}
	else
		if (Player* player = findPlayerObject(argv[1])) {
			player->setDamageFlash(atof(argv[2]));
			return "True";
		}
	return "False";
}

static const char *c_lastContactCount(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) 
		Console->printf("wrong argcount to %s(player)",argv[0]);
	else if(Player* player = findPlayerObject(argv[1]))  
	  	return intToStr(player->getLastContactCount());
	
	return "-1";
}

static const char *c_isCrouching(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) 
		Console->printf("wrong argcount to %s(player)",argv[0]);
	else if(Player* player = findPlayerObject(argv[1]))  
	  	return player->isCrouching() ? "True" : "False";
	
	return "-1";
}

static const char *c_isAiControlled(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) 
		Console->printf("wrong argcount to %s(player)",argv[0]);
	else if(Player * player = findPlayerObject(argv[1]))
	  	if( player->isAIControlled() )
         return "True";
	return "False";
}

//----------------------------------------------------------------------------
// Turrets
//----------------------------------------------------------------------------

static const char *c_turretSetTarget (CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3)
		{
			Console->printf ("wrong argcount to setTarget (Turret, Player)");
			return "False";
		}

	Turret *turret = findTurretObject (argv[1]);
	Player *player = findPlayerObject (argv[2]);

	if (turret && player)
		return turret->setPrimaryTarget (player) ? "True" : "False";

	return "False";
}


//----------------------------------------------------------------------------
// Vehicles
//----------------------------------------------------------------------------

static const char *c_vehicleCanMount (CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3)
		{
			Console->printf ("wrong argcount to canMount (Vehicle, Player)");
			return "False";
		}

	Vehicle *vehicle = dynamic_cast<Vehicle*> (findGameObject (argv[1]));
	ShapeBase *player = findShapeBaseObject (argv[2]);

	if (vehicle && player)
		return vehicle->canMount (player) ? "True" : "False";

	return "False";
}

static const char *c_vehicleCanRide (CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3)
		{
			Console->printf ("wrong argcount to canRide (Vehicle, Player)");
			return "False";
		}

	Vehicle *vehicle = dynamic_cast<Vehicle*> (findGameObject (argv[1]));
	ShapeBase *player = findShapeBaseObject (argv[2]);

	if (vehicle && player)
		return vehicle->canRide (player) ? "True" : "False";

	return "False";
}


static const char *c_vehicleNextPassengerPoint (CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
		Console->printf ("wrong argcount to nextPassengerPoint (Vehicle)");
		return "-1";
	}

	if (GameBase* gobj = findGameObject (argv[1]))
		if (Vehicle *vehicle = dynamic_cast<Vehicle*> (gobj))
			return intToStr (vehicle->nextPassengerPoint ());
	return "-1";
}

static const char *c_vehicleIncPassengers (CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2) {
			Console->printf ("wrong argcount to incPassengers (Vehicle)");
			return "-1";
	}

	if (GameBase* gobj = findGameObject (argv[1]))
		if (Vehicle *vehicle = dynamic_cast<Vehicle*> (gobj)) {
			vehicle->changePassengerCount (1);
			return "True";
		}

	return "False";
}

static const char *c_vehicleDecPassengers (CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 2)
		{
			Console->printf ("wrong argcount to decPassengers (Vehicle)");
			return "-1";
		}

	if (GameBase* gobj = findGameObject (argv[1]))
		if (Vehicle *vehicle = dynamic_cast<Vehicle*> (gobj)) {
			vehicle->changePassengerCount (-1);
			return "True";
		}

	return "False";
}

static const char *c_vehicleGetMountPoint (CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf ("wrong argcount to getMountPoint (Vehicle, MountPoint)");
		return "-1";
	}

	if (GameBase* gobj = findGameObject (argv[1]))
		if (Vehicle *vehicle = dynamic_cast<Vehicle*> (gobj)) {
			TMat3F tmat;
			vehicle->getObjectMountTransform (atoi(argv[2]), &tmat);
			sprintf (ReturnBuffer, "%g %g %g", tmat.p.x, tmat.p.y, tmat.p.z);
			return ReturnBuffer;
		}

	return "False";
}

//----------------------------------------------------------------------------
// Client side inventory
//----------------------------------------------------------------------------

static const char *c_clientUseItem(CMDConsole *, int, int argc, const char **argv)
{
	if (!cg.psc)
		return "False";
	if (argc != 2) {
		Console->printf("wrong argcount to %s(itemDescription)",argv[0]);
		return "False";
	}
	int itemType = atoi(argv[1]);
	if (itemType >= 0 &&
			itemType < cg.dbm->getDataGroupSize(DataBlockManager::ItemDataType)) {
		// We go through the player PSC instead of the remote eval so
		// that we can guarente use order with trigger actions.
	   // Console->executef(4, "remoteEval", "2048", "useItem", intToStr(itemType));
		cg.psc->useItem(itemType);
		return "True";
	}
	return "False";
}

static const char *c_clientBuyFavorites(CMDConsole *, int, int, const char **)
{
   const char *argv[SimConsoleEvent::MaxArgs];
   char cmdbuf[1024];
   char buf[256];
   char *bptr = cmdbuf;

   argv[0] = "remoteEval";
   argv[1] = "2048";
   argv[2] = "buyFavorites";
   int argc;
   for(argc = 3; argc < SimConsoleEvent::MaxArgs; argc++)
   {
      sprintf(buf, "$pref::itemFavorite%d", argc - 3);
      const char *val = Console->getVariable(buf);
      if(!*val)
         break;
      sprintf(bptr, "%d", getItemDescriptionType(val));
      argv[argc++] = bptr;
      bptr += strlen(bptr);
   }
   if(argc > 3)
      Console->execute(argc, argv);
   return "True";
}

//----------------------------------------------------------------------------
// Item functions
//----------------------------------------------------------------------------

static const char *c_hideItem(CMDConsole *, int, int argc, const char **argv)
{
	// player item count
	if (argc != 3) {
		Console->printf("wrong argcount to hideItem(item,bool)");
		return "False";
	}
	if (Item* item = findItemObject(argv[1])) {
		item->hideItem(CMDConsole::getBool(argv[2]));
		return "True";
	}
	Console->printf("%s: could not find item (%s)", argv[0], argv[1] );
	return "False";
}

static const char *c_getObjectItemData(CMDConsole *, int, int argc, const char **argv)
{
	// player item count
	if (argc != 2) {
		Console->printf("wrong argcount to %s(item)",argv[0]);
		return "-1";
	}
	if (Item* item = findItemObject(argv[1]))
		return item->getDatFileName();
	Console->printf("%s: could not find item",argv[0]);
	return "-1";
}

static const char *c_getObjectItemCount(CMDConsole *, int, int argc, const char **argv)
{
	// player item count
	if (argc != 2) {
		Console->printf("wrong argcount to getObjectItemCount(item)");
		return "-1";
	}
	if (Item* item = findItemObject(argv[1]))
		return intToStr(item->getCount());
	Console->printf("%s: could not find item",argv[0]);
	return "0";
}

static const char *c_isPopItem(CMDConsole *, int, int argc, const char **argv)
{
	// player item count
	if (argc != 2) {
		Console->printf("wrong argcount to isPopItem(item)");
		return "False";
	}
	if (Item* item = findItemObject(argv[1]))
		return item->isPop()? "True": "False";
	Console->printf("%s: could not find item",argv[0]);
	return "False";
}

static const char *c_isRotatingItem(CMDConsole *, int, int argc, const char **argv)
{
	// player item count
	if (argc != 2) {
		Console->printf("wrong argcount to isRotatingItem(item)");
		return "False";
	}
	if (Item* item = findItemObject(argv[1]))
		return item->isRotating()? "True": "False";
	Console->printf("%s: could not find item",argv[0]);
	return "False";
}

static const char *c_playSound(CMDConsole *, int, int argc, const char **argv)
{
   if(!sg.manager || !sg.dbm || argc != 3)
      return "False";
   int blockId;
   sg.dbm->lookupDataBlock(argv[1], DataBlockManager::SoundDataType, blockId);
   Point3F pos;
   if(blockId == -1)
      return "False";
   sscanf(argv[2], "%f %f %f", &pos.x, &pos.y, &pos.z);
   TSFX::serverPlaySound(blockId, pos);
   return "True";
}

static const char *c_playObjectSound(CMDConsole *, int, int argc, const char **argv)
{
   if(!sg.manager || !sg.dbm || argc != 4)
      return "False";
   GameBase *object = findGameObject(argv[1]);
   if(!object)
      return "False";
   int blockId;
   sg.dbm->lookupDataBlock(argv[2], DataBlockManager::SoundDataType, blockId);
   Point3F pos;
   if(blockId == -1)
      return "False";
   int  channel = atoi(argv[3]);

   TSFX::serverPlaySound(blockId, object, channel);
   return "True";
}

static const char *c_playVoice(CMDConsole *, int, int argc, const char **argv)
{
   if(!sg.manager || !sg.dbm || argc != 3 || !sg.playerManager)
      return "False";
   sg.playerManager->serverPlayVoice( atoi(argv[1]), argv[2]);
   return "True";
}

static const char *c_setIssueCommand(CMDConsole *, int, int argc, const char **argv)
{
   if(!cg.commandTS || argc != 3)
      return "False";
   cg.commandTS->issueCommand(atoi(argv[1]), stringTable.insert(argv[2], true));
   return "True";
}

static const char *c_limitCommandBand(CMDConsole *, int, int argc, const char **argv)
{
   if(!sg.playerManager || argc != 3)
      return "false";
   PlayerManager::ClientRep *cr = sg.playerManager->findClient(atoi(argv[1]));
   bool limit = CMDConsole::getBool(argv[2]);
   if(!cr || !cr->playerPSC)
      return "false";
   cr->playerPSC->setCommandBandLimit(limit);
   return "true";
}


//------------------------------------------------------------------------------
// Moveable (elevators & doors)
//------------------------------------------------------------------------------

static const char *c_moveToWaypoint(CMDConsole *, int, int argc, const char **argv)
{
	// player item count
	if (argc != 3) {
		Console->printf("wrong argcount to %s(object,waypoint)",argv[0]);
	}
	else {
		if (Moveable* obj = findMoveableObject(argv[1])) {
			if (obj->moveToWaypoint(atoi(argv[2])))
				return "True";
		}
	}
	return "False";
}

static const char *c_moveForward(CMDConsole *, int, int argc, const char **argv)
{
	// player item count
	if (argc != 2) {
		Console->printf("wrong argcount to %s(object)",argv[0]);
	}
	else {
		if (Moveable* obj = findMoveableObject(argv[1])) {
			obj->moveForward();
			return "True";
		}
	}
	return "False";
}

static const char *c_moveBackward(CMDConsole *, int, int argc, const char **argv)
{
	// player item count
	if (argc != 2) {
		Console->printf("wrong argcount to %s(object)",argv[0]);
	}
	else {
		if (Moveable* obj = findMoveableObject(argv[1])) {
			obj->moveBackward();
			return "True";
		}
	}
	return "False";
}

static const char *c_moveStop(CMDConsole *, int, int argc, const char **argv)
{
	// player item count
	if (argc != 2) {
		Console->printf("wrong argcount to %s(object)",argv[0]);
	}
	else {
		if (Moveable* obj = findMoveableObject(argv[1])) {
			obj->stop();
			return "True";
		}
	}
	return "False";
}

static const char *c_moveGetState(CMDConsole *, int, int argc, const char **argv)
{
	// player item count
	if (argc != 2) {
		Console->printf("wrong argcount to %s(object)",argv[0]);
	}
	else {
		if (Moveable* obj = findMoveableObject(argv[1])) {
			return obj->getMoveStateName();
		}
	}
	return "False";
}

static const char *c_moveGetCount(CMDConsole *, int, int argc, const char **argv)
{
	// player item count
	if (argc != 2) {
		Console->printf("wrong argcount to %s(object)",argv[0]);
	}
	else {
		if (Moveable* obj = findMoveableObject(argv[1])) {
			return intToStr(obj->getWaypointCount());
		}
	}
	return "0";
}

static const char *c_moveGetPosition(CMDConsole *, int, int argc, const char **argv)
{
	// player item count
	if (argc != 2) {
		Console->printf("wrong argcount to %s(object)",argv[0]);
	}
	else {
		if (Moveable* obj = findMoveableObject(argv[1])) {
			return floatToStr(obj->getWaypointPosition());
		}
	}
	return "0";
}

static const char *c_setWaypoint(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to %s(object,%wayPoint)",argv[0]);
	}
	else {
		if (Moveable* obj = findMoveableObject(argv[1])) {
			if(obj->setWaypoint(atoi(argv[2])))
				return "True";
		}
	}
	return "False";
}

static const char *c_setHudTimer(CMDConsole *, int, int argc, const char **argv)
{
   if(argc == 2)
      cg.clockTime = atof(argv[1]);
   return "0";
}

//------------------------------------------------------------------------------
// Interior Light, wire these up later
//------------------------------------------------------------------------------
static const char*
c_interiorToggleSwitchLight(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to %s(Interior, Light)",argv[0]);
		return "False";
	}

	if (InteriorShape* interior = findInteriorShapeObject(argv[1])) {
      ITRInstance* pInstance = interior->getInstance();
      AssertFatal(pInstance != NULL, "InteriorShape should never be wo/ ITRInstance");
      ITRLighting* pLighting = pInstance->getLighting();
      AssertFatal(pInstance != NULL, "ITRInstance should never be wo/ Lighting");

      int lightId = pLighting->findLightId(argv[2]);
      if (lightId == -1)
         return "False";

      int state = pInstance->getLightState(lightId);
      if (state == -1) {
         AssertWarn(0, "After getting lightid, error should really never happen");
         return "False";
      }

      if (state == 0)
         pInstance->setLightState(lightId, 1);
      else
         pInstance->setLightState(lightId, 0);
	}

   return "False";
}

static const char*
c_interiorTurnOnSwitchLight(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to %s(Interior, Light)",argv[0]);
		return "False";
	}

	if (InteriorShape* interior = findInteriorShapeObject(argv[1])) {
      ITRInstance* pInstance = interior->getInstance();
      AssertFatal(pInstance != NULL, "InteriorShape should never be wo/ ITRInstance");
      ITRLighting* pLighting = pInstance->getLighting();
      AssertFatal(pInstance != NULL, "ITRInstance should never be wo/ Lighting");

      int lightId = pLighting->findLightId(argv[2]);
      if (lightId == -1)
         return "False";

      pInstance->setLightState(lightId, 1);
	}

   return "False";
}

static const char*
c_interiorTurnOffSwitchLight(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 3) {
		Console->printf("wrong argcount to %s(Interior, Light)",argv[0]);
		return "False";
	}

	if (InteriorShape* interior = findInteriorShapeObject(argv[1])) {
      ITRInstance* pInstance = interior->getInstance();
      AssertFatal(pInstance != NULL, "InteriorShape should never be wo/ ITRInstance");
      ITRLighting* pLighting = pInstance->getLighting();
      AssertFatal(pInstance != NULL, "ITRInstance should never be wo/ Lighting");

      int lightId = pLighting->findLightId(argv[2]);
      if (lightId == -1)
         return "False";

      pInstance->setLightState(lightId, 1);
	}

   return "False";
}

static const char*
c_interiorSetLightTime(CMDConsole *, int, int argc, const char **argv)
{
	if (argc != 4) {
		Console->printf("wrong argcount to %s(Interior, Light, Time)",argv[0]);
		return "False";
	}

	if (InteriorShape* interior = findInteriorShapeObject(argv[1])) {
      ITRInstance* pInstance = interior->getInstance();
      AssertFatal(pInstance != NULL, "InteriorShape should never be wo/ ITRInstance");
      ITRLighting* pLighting = pInstance->getLighting();
      AssertFatal(pInstance != NULL, "ITRInstance should never be wo/ Lighting");

      float time = atof(argv[3]);

      int lightId = pLighting->findLightId(argv[2]);
      if (lightId == -1)
         return "False";

      pInstance->setLightTime(lightId, time);
	}

   return "False";
}

static const char*
c_spawnProjectile(CMDConsole*, int, int argc, const char* argv[])
{
   if (sg.manager == NULL)
      return "-1";

   extern int ProjectileGroups[8];
   if (argc != 5 && argc != 6)
      return "-1";

   ProjectileDataType dType;
	bool found = false;
	int typeId;
	for (int i = 0; i < sizeof(ProjectileGroups)/sizeof(int); i++) {
	   if (sg.dbm->lookupDataBlock(argv[1], ProjectileGroups[i], typeId)) {
			dType.type = ProjectileGroups[i];
			dType.dataType = typeId;
			found = true;
         break;
		}
   }
   if (found == false) {
		Console->printf("Projectile data block \"%s\" is undefined",argv[1]);
      return "-1";
   }

   Projectile* pProjectile = createProjectile(dType);
   if (pProjectile == NULL)
      return "-1";

   // Ok, now we have a projectile.  Extract the position, velocity, shooterId,
   //  shooter Velocity, and, if present, the targetId.
   //
   TMat3F  total;
   int     shooterId;
   Point3F shooterVel;
   int     targetId;

   sscanf(argv[2], "%f %f %f %f %f %f %f %f %f %f %f %f",
          &total.m[0][0], &total.m[0][1], &total.m[0][2],
          &total.m[1][0], &total.m[1][1], &total.m[1][2],
          &total.m[2][0], &total.m[2][1], &total.m[2][2],
          &total.p.x,     &total.p.y,     &total.p.z);
   total.flags = RMat3F::Matrix_HasRotation | RMat3F::Matrix_HasTranslation;

   sscanf(argv[3], "%d", &shooterId);
   sscanf(argv[4], "%f %f %f", &shooterVel.x, &shooterVel.y, &shooterVel.z);

   pProjectile->initProjectile(total, shooterVel, shooterId);
   if (argc == 6 && pProjectile->isTargetable()) {
      ShapeBase* pSB = findShapeBaseObject(argv[5]);
      if (pSB != NULL)
         pProjectile->setTarget(pSB);
   }
   
   if(sg.manager->registerObject(pProjectile))
   {
      if(SimObject *obj = sg.manager->findObject("MissionCleanup"))
         if(SimGroup *grp = dynamic_cast<SimGroup *>(obj))
            grp->addObject(pProjectile);
      if(!pProjectile->getGroup())
         sg.manager->addObject(pProjectile);
      return intToStr(pProjectile->getId());
   }
   else
   {
      delete pProjectile;
      return "-1";
   }
}

const char*
c_getMuzzleTransform(CMDConsole *, int, int argc, const char **argv)
{
   static char retBuffer[512];
   if (argc == 2) {
      ShapeBase* pSB = findShapeBaseObject(argv[1]);
      if (pSB != NULL || (pSB = findPlayerObject(argv[1])) != NULL) {
         TMat3F muzzle;
         TMat3F total;
         if (pSB->getMuzzleTransform(0, &muzzle) == true) {
            m_mul(muzzle, pSB->getTransform(), &total);
         } else {
            muzzle = pSB->getEyeTransform();
         }

         sprintf(retBuffer, "%f %f %f %f %f %f %f %f %f %f %f %f",
                 total.m[0][0], total.m[0][1], total.m[0][2],
                 total.m[1][0], total.m[1][1], total.m[1][2],
                 total.m[2][0], total.m[2][1], total.m[2][2],
                 total.p.x,     total.p.y,     total.p.z);
         return retBuffer;
      }
   }
   
   return "1 0 0 0 1 0 0 0 1 0 0 0";
}

const char*
c_startFadeOut(CMDConsole *, int, int argc, const char **argv)
{
   if (argc == 2) {
      ShapeBase* pSB = findShapeBaseObject(argv[1]);
      if (pSB != NULL || (pSB = findPlayerObject(argv[1])) != NULL) {
         pSB->startFadeOut();
         return "True";
      }
   }
   
   return "False";
}

const char*
c_startFadeIn(CMDConsole *, int, int argc, const char **argv)
{
   if (argc == 2) {
      ShapeBase* pSB = findShapeBaseObject(argv[1]);
      if (pSB != NULL || (pSB = findPlayerObject(argv[1])) != NULL) {
         pSB->startFadeIn();
         return "True";
      }
   }
   
   return "False";
}

//----------------------------------------------------------------------------
// Gui Controls
//----------------------------------------------------------------------------

static const char *ShapeViewSetShape(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 3)
      return "False";
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   if(!ctrl)
      return "False";
   FearGui::ShapeView *sv = dynamic_cast<FearGui::ShapeView *>(ctrl);
   if(!sv)
      return "False";
   sv->loadShape(argv[2]);

   return "True";
}

static const char *ShapeViewSetSkin(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 3)
      return "False";
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   if(!ctrl)
      return "False";
   FearGui::ShapeView *sv = dynamic_cast<FearGui::ShapeView *>(ctrl);
   if(!sv)
      return "False";
   //sv->setSkin(argv[2]);

   return "True";
}

static const char *ShapeViewSetItem(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 3 || !cg.dbm)
      return "False";
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   if(!ctrl)
      return "False";
   FearGui::ShapeView *sv = dynamic_cast<FearGui::ShapeView *>(ctrl);
   if(!sv)
      return "False";

   int item = atoi(argv[2]);
   Item::ItemData *dat = (Item::ItemData *) cg.dbm->lookupDataBlock(item, DataBlockManager::ItemDataType);
   if(!dat)
      return "false";
   char buf[256];
   strcpy(buf, dat->fileName);
   strcat(buf, ".dts");
   sv->loadShape(buf);

   return "True";
}

char centerPrintBuffer[2048] = "";
int centerPrintPos = 0;

static const char *c_centerprint(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 3)
      return "False";
   strcpy(centerPrintBuffer, argv[1]);
   centerPrintPos = atoi(argv[2]);
   return "True";
}

static const char *c_serverPreload(CMDConsole *, int, int argc, const char **)
{
   if(argc != 1 || !sg.dbm)
      return "False";

   if(sg.dbm->serverPreload())
      return "True";
   return "False";
}

static const char *c_getIntTime(CMDConsole *, int, int argc, const char **argv)
{
   DWORD time;
   if(argc != 2)
      return "";
   if(CMDConsole::getBool(argv[1]))
      time = sg.currentTime;
   else
      time = cg.currentTime;
   static char retBuf[32];
   sprintf(retBuf, "%d.%d", time >> 5, time & 0x1F);
   return retBuf;
}

static const char*
c_isIn8BitMode(CMDConsole *, int, int, const char**)
{
   if (cg.manager) {
      SimGui::Canvas* pRoot = (SimGui::Canvas*)cg.manager->findObject("MainWindow");
      AssertFatal(pRoot != NULL, "No root canvas");

      if (pRoot->isFullScreen() == true)
         return "false";


      HDC l_hdc = GetDC(pRoot->getHandle());
      AssertFatal(l_hdc, "No HDC?");

      const char* pReturn;
      if (GetDeviceCaps(l_hdc, BITSPIXEL) == 8)
         pReturn = "true";
      else
         pReturn = "false";
      ReleaseDC(pRoot->getHandle(), l_hdc);

      return pReturn;
   }
   Console->printf("Could not find manager");
   return "false";
}

static const char*
c_issue8BitWarning(CMDConsole *, int, int, const char**)
{
   MessageBox(NULL, "Your display is in 8-Bit (256 color) Mode. Tribes\n"
                    "may not display correctly in Windowed mode unless\n"
                    "the display resolution is changed to 16-bit (65536\n"
                    "color) mode or above.  Fullscreen display will display\n"
                    "properly however.  In addition, OpenGL will be disabled.",
              "Color Depth Warning",
              MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL | MB_TOPMOST);

   extern bool g_oglDisabled;
   g_oglDisabled = true;

   return "true";
}

static const char*
c_issueInternetWarning(CMDConsole* console, int, int, const char**)
{
   if (console->getBoolVariable("pref::ignoreInternetWarning", false) == true) {
      return "true";
   }

   int retVal = MessageBox(NULL, "Starsiege TRIBES requires an active internet connection\n"
                                 "to function properly.  If you are not connected to your\n"
                                 "Internet Service Provider, you may experience pauses when\n"
                                 "Windows attempts to automatically connect your computer to\n"
                                 "the Internet"
                                 "\n\nPress Yes if you want to be warned about this in the future.\n"
                                 "Press No if you do not want to see this message again.\n"
                                 "(Cancel to exit TRIBES)",
                                 "Internet Connection Warning",
                           MB_ICONINFORMATION | MB_TASKMODAL | MB_TOPMOST | MB_YESNOCANCEL);

   if (retVal == IDYES) {
      return "true";
   } else if (retVal == IDNO) {
      console->setBoolVariable("pref::ignoreInternetWarning", true);
      return "true";
   } else {
      return "false";
   }
}

extern void expandEscape(char *dest, const char *src);

static const char *c_resetSimTime(CMDConsole *, int, int, const char **)
{
   if(sg.manager)
   {
      sg.timeBase = sg.currentTime;
      sg.manager->resetTime();
   }
   if(cg.manager)
   {
      cg.timeBase = cg.currentTime;
      cg.manager->resetTime();
   }
	return "true";
}

static const char *c_escapeStr(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
      return "";
   static char tbuf[2048];
   expandEscape(tbuf, argv[1]);
   return tbuf;
}

static char ipBuf[10][256];
static char xlipBuf[10][256];
static int numIps;
static HANDLE ipResolveThread = NULL;
static bool addressesResolved = false;

static unsigned long CALLBACK resolveThreadProcess(void *)
{
   int i;
   SOCKADDR_IN addr;
   for(i = 0; i < 10; i++)
      xlipBuf[i][0] = 0;
   for(i = 0; i < numIps; i++)
   {
      if(DNet::UDPTransport::STgetAddressFromString(ipBuf[i], &addr))
         DNet::UDPTransport::STgetStringFromAddress(&addr, xlipBuf[i]);
   }
   addressesResolved = true;
   ipResolveThread = NULL;
   return 0;
}

static const char *c_getResolvedMaster(CMDConsole *, int, int argc, const char **argv)
{
   if(!addressesResolved)
      return "";
   if(argc != 2)
      return "";
   int idx = atoi(argv[1]);
   if(idx < 0 || idx > 9)
      return "";
   return xlipBuf[idx];
}

static const char *c_getManagerId(CMDConsole *, int, int , const char **)
{
   return intToStr(wg->manager->getId());
}

static const char *c_resolveMasters(CMDConsole *, int, int argc, const char **argv)
{
   if(ipResolveThread)
   {
      CloseHandle(ipResolveThread);
      ipResolveThread = NULL;
   }
   addressesResolved = false;
   if(argc < 2)
      return NULL;
   if(argc > 11)
      argc = 11;
   numIps = argc - 1;
   int i;
   for(i = 0; i < numIps; i++)
   {
      if(argv[i+1][0] == 0)
         break;
      strncpy(ipBuf[i], argv[i+1], 255);
      ipBuf[i][255] = 0;
   }

   DWORD threadId;
   ipResolveThread = CreateThread(NULL, 0, resolveThreadProcess, NULL, 0, &threadId);
   return intToStr(i);
}

static const char *c_which(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
   {
      Console->printf("Invalid argcount for %s.", argv[0]);
      return "False";
   }
   ResourceObject *ro = cg.resManager->find(argv[1]);
   if(!ro)
   {
      Console->printf("Couldn't find file: %s", argv[1]);
      return "";
   }
   Console->printf("path: %s\\%s", ro->filePath, ro->fileName);
   return "";      
}

static const char*
c_registerStaticTextures(CMDConsole*, int, int argc, const char** argv)
{
   if (argc != 2) {
      Console->printf("usage: RegisterStaticTextures(<Canvas Name>)");
      return "false";
   }

   SimCanvas* pCanvas = dynamic_cast<SimCanvas*>(cg.manager->findObject(argv[1]));
   if (pCanvas != NULL) {
      GFXSurface* pSurface = pCanvas->getSurface();
      AssertFatal(pSurface != NULL, "No surface!");
      
      SimRegisterTextureEvent event(pSurface);

      cg.manager->processEvent(&event);

      return "true";
   } else {
      return "false";
   }
}

//----------------------------------------------------------------------------
// Sensor
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
extern void addFilePluginCommands();

void FearPlugin::init()
{
	console->printf("FearPlugin");

   addFilePluginCommands();

	// Misc functions
	console->addCommand(0, "version", c_version);
	console->addCommand(0, "timeStamp", c_timeStamp);
	console->addCommand(0, "sprintf", c_sprintf);
	console->addCommand(0, "getIntegerTime", c_getIntTime);
	console->addCommand(ListPlayers, "listPlayers", this, 1);
	console->addCommand(0, "Vector::dot", c_dotVec);
	console->addCommand(0, "Vector::add", c_addVec);
	console->addCommand(0, "Vector::sub", c_subVec);
	console->addCommand(0, "Vector::neg", c_negVec);
	console->addCommand(0, "Vector::getFromRot", c_getVecFromRot);
	console->addCommand(0, "Vector::getDistance", c_vecDistance);
	console->addCommand(0, "Vector::getRotation", c_getRotationFromNormal);
	console->addCommand(0, "Vector::normalize", c_normalize);
	console->addCommand(0, "WaypointToWorld", c_getWorldFromWaypoint);
	console->addCommand(0, "nameToID", c_nameToID);
	console->addCommand(0, "sqrt", c_sqrt);
	console->addCommand(0, "pow",c_pow);
	console->addCommand(0, "getWord", c_getWord);
	console->addCommand(0, "getBoxCenter", c_getBoxCenter);
	console->addCommand(0, "containerBoxFillSet", c_containerBoxFillSet);
	console->addCommand(0, "findPointOnSphere", c_findPointOnSphere);
	console->addCommand(0, "Group::iterateRecursive", c_groupIterate);
	console->addCommand(0, "Group::objectCount", c_groupGetCount);
	console->addCommand(0, "Group::getObject", c_groupGetObject);
   console->addCommand(0, "removeFromSet", c_groupRemove);
   console->addCommand(0, "escapeString", c_escapeStr);
   console->addCommand(0, "getManagerId", c_getManagerId);

	// Client only functions
	console->addCommand(RenderCanvas, "renderCanvas", this, 1);
	console->addCommand(0, "rebuildCommandMap", c_rebuildCommandMap);
	console->addCommand(0, "useItem", c_clientUseItem);
	console->addCommand(0, "buyFavorites", c_clientBuyFavorites);
	console->addCommand(0, "getItemType", c_getClientItemType);
	console->addCommand(0, "getItemCount", c_getClientItemCount);
	console->addCommand(0, "getMountedItem", c_getClientMountedItem);
   
   //Client database-like functions
	console->addCommand(0, "addGameServer", c_addGameServer);
   
   // Network functions
	console->addCommand(0, "DNet::resolveMasters", c_resolveMasters);
	console->addCommand(0, "DNet::getResolvedMaster", c_getResolvedMaster);
   console->addCommand(0, "rebuildServerList", c_rebuildServerList);

   console->addCommand(AddChatCommand, "addCMCommand", this);
   console->addCommand(SetChatMode, "setCMMode", this);
	// Server remote access functions 
   console->addCommand(RemotePlayAnim, "remotePlayAnim", this);
   console->addCommand(0, "setIssueCommand", c_setIssueCommand);
   console->addCommand(IssueCommand, "issueCommand", this, 1);
   console->addCommand(IssueCommandI, "issueCommandI", this, 1);
   console->addCommand(IssueTargCommand, "issueTargCommand", this, 1);
   console->addCommand(RemoteSelectCommander, "remoteSelectCommander", this, 1);
   console->addCommand(SetCommandStatus, "setCommandStatus", this, 1);

	// Server support functions
   console->addCommand(StartGhosting, "startGhosting", this);
   console->addCommand(ResetPlayerManager, "resetPlayerManager", this);
   console->addCommand(ResetGhostManagers, "resetGhostManagers", this);

   console->addCommand(0, "preloadServerDataBlocks", c_serverPreload);
   console->addCommand(0, "resetSimTime", c_resetSimTime);

   console->addCommand(0, "playVoice", c_playVoice);
   console->addCommand(0, "playSound", c_playSound);
   console->addCommand(0, "getNumClients", c_getNumPlayers);
   console->addCommand(0, "getClientByIndex", c_getPlayerByIndex);
   console->addCommand(0, "getNumTeams", c_getNumTeams);
	console->addCommand(0, "getTeamName", c_getTeamName);

	console->addCommand(0, "getObjectType", c_getObjectType);
	console->addCommand(0, "getObjectByTargetIndex", c_getTargObjectByIndex);
	console->addCommand(0, "getItemData", c_getItemData);
	console->addCommand(0, "getNumItems", c_getNumItems);
   console->addCommand(0, "activateGroup", c_activateGroup);
   console->addCommand(0, "getRandom", c_getRandom);

	// Team functions
   console->addCommand(0, "Team::setScore", c_setTeamScore);
   console->addCommand(0, "setTeamScoreHeading", c_setTeamScoreHeading);
   console->addCommand(0, "Team::setObjective", c_setTeamObjective);
   console->addCommand(0, "Team::clearObjectives", c_clearTeamObjectives);
   console->addCommand(0, "setHudTimer", c_setHudTimer);

	// Client ID functions (on server)

   console->addCommand(0, "Client::setDataFinished", c_clientSetDataFinished);

   console->addCommand(0, "Client::limitCommandBandwidth", c_limitCommandBand);
   console->addCommand(0, "Client::setMenuScoreVis", c_setMenuScoreVis);
   console->addCommand(0, "Player::getClient", c_getOwnerClient);
   console->addCommand(0, "Client::getOwnedObject", c_clientGetOwnedObject);
   console->addCommand(0, "Client::getControlObject", c_clientGetControlObject);
   console->addCommand(0, "Client::setOwnedObject", c_clientSetOwnedObject);
   console->addCommand(0, "Client::setControlObject", c_clientSetControlObject);
   console->addCommand(0, "GameBase::getOwnerClient", c_getOwnerClient);
   console->addCommand(0, "GameBase::getControlClient", c_getControlClient);
   console->addCommand(0, "Client::setSkin", c_setClientSkin);
   

   console->addCommand(GetTransportAddress, "Client::getTransportAddress", this);
   console->addCommand(0, "Client::getFirst", c_clientGetFirst);
   console->addCommand(0, "Client::getNext", c_clientGetNext);
	console->addCommand(0, "Client::setItemShopping", c_setItemShoppingList);
	console->addCommand(0, "Client::clearItemShopping", c_clearItemShoppingList);
	console->addCommand(0, "Client::isItemShoppingOn", c_isItemShoppingOn);
	console->addCommand(0, "Client::setItemBuying", c_setItemBuyList);
	console->addCommand(0, "Client::clearItemBuying", c_clearItemBuyList);
	console->addCommand(0, "Client::isItemBuyingOn", c_isItemBuyOn);
   console->addCommand(0, "Client::setInitialTeam", c_setInitialTeam);
   console->addCommand(0, "Client::getName", c_getPlayerName);
   console->addCommand(0, "Client::getTeam", c_getPlayerTeam);
   console->addCommand(0, "Client::sendMessage", c_messageToPlayer);
   console->addCommand(0, "Client::setScore", c_setClientScore);
   console->addCommand(0, "setClientScoreHeading", c_setClientScoreHeading);
   console->addCommand(0, "Client::setGuiMode", c_setClientGuiMode);
   console->addCommand(0, "Client::getGuiMode", c_getClientGuiMode);
   console->addCommand(0, "Client::ExitLobbyMode", c_exitLobbyMode);
   console->addCommand(0, "resetPlayDelegate", c_resetPlayDelegate);
   console->addCommand(0, "Client::getVoiceBase", c_clientGetVoice);
   console->addCommand(0, "Client::getSkinBase", c_clientGetSkin);
   console->addCommand(0, "Client::getGender", c_clientGetGender);
   console->addCommand(0, "Client::centerPrint", c_centerprint);
   console->addCommand(0, "Client::getMouseSensitivity", c_getMouseSensitivity);
   console->addCommand(0, "Client::getMouseXAxisFlip", c_getMouseXAxisFlip);
   console->addCommand(0, "Client::getMouseYAxisFlip", c_getMouseYAxisFlip);

	// Client functions (on client)
   console->addCommand(0, "getClientByName", c_getPlayerByName);

	// GameBase object functions
   console->addCommand(0, "GameBase::playSound", c_playObjectSound);
	console->addCommand(0, "GameBase::generatePower", c_generatePower);
	console->addCommand(0, "GameBase::isPowerGenerator", c_isPowerGenerator);
	console->addCommand(0, "GameBase::getPowerCount", c_getPowerCount);
	console->addCommand(0, "GameBase::isPowered", c_isPowered);
	console->addCommand(0, "GameBase::getTeam", c_getTeam);
	console->addCommand(0, "GameBase::setTeam", c_setTeam);
	console->addCommand(0, "GameBase::getPosition", c_getPosition);
	console->addCommand(0, "GameBase::getRotation", c_getRotation);
	console->addCommand(0, "GameBase::setPosition", c_setPosition);
	console->addCommand(0, "GameBase::setRotation", c_setRotation);
	console->addCommand(0, "GameBase::getTransform", c_getTransform);
	console->addCommand(0, "GameBase::getDataName", c_getDataName);
	console->addCommand(0, "GameBase::isAtRest", c_isAtRest);
	console->addCommand(0, "GameBase::testPosition", c_testPosition);

	// ShapeBase object functions. Called GameBase:: to simply
	// out public script interface a little.  Functions should
	// behave gracefully if object is not derived from ShapeBase
	console->addCommand(0, "GameBase::setSequenceDirection", c_setSequenceDir);
	console->addCommand(0, "GameBase::stopSequence", c_stopSequence);
	console->addCommand(0, "GameBase::playSequence", c_playSequence);
	console->addCommand(0, "GameBase::pauseSequence", c_pauseSequence);
	console->addCommand(0, "GameBase::setRechargeRate", c_setRechargeRate);
	console->addCommand(0, "GameBase::getRechargeRate", c_getRechargeRate);
	console->addCommand(0, "GameBase::setAutoRepairRate", c_setAutoRepairRate);
	console->addCommand(0, "GameBase::getAutoRepairRate", c_getAutoRepairRate);
	console->addCommand(0, "GameBase::repairDamage", c_repairDamage);
	console->addCommand(0, "GameBase::throw", c_throwObject);
	console->addCommand(0, "GameBase::setEnergy", c_setEnergy);
	console->addCommand(0, "GameBase::getEnergy", c_getEnergy);
	console->addCommand(0, "GameBase::getDamageLevel", c_getDamageLevel);
	console->addCommand(0, "GameBase::setDamageLevel", c_setDamageLevel);
	console->addCommand(0, "GameBase::getDamageState",c_getDamageState);
	console->addCommand(0, "GameBase::applyDamage", c_applyDamage);
	console->addCommand(0, "GameBase::activateShield", c_activateShield);
	console->addCommand(0, "GameBase::getLOSInfo",c_getLOSInfo);
	console->addCommand(0, "GameBase::setMapName",c_setMapName);
	console->addCommand(0, "GameBase::getMapName",c_getMapName);
	console->addCommand(0, "GameBase::virtual", c_virtual);
	console->addCommand(0, "GameBase::setActive", c_setActive);
	console->addCommand(0, "GameBase::isActive", c_isActive);
   console->addCommand(0, "GameBase::getMuzzleTransform", c_getMuzzleTransform);
   console->addCommand(0, "GameBase::startFadeOut", c_startFadeOut);
   console->addCommand(0, "GameBase::startFadeIn", c_startFadeIn);
   console->addCommand(0, "GameBase::applyRadiusDamage", c_applyRadiusDamage);
   console->addCommand(0, "GameBase::getRadius", c_getRadius);
   console->addCommand(0, "GameBase::getDisabledDamage", c_getDisabledDamage);

	// StaticBase object functions, called GameBase::
	console->addCommand(0, "GameBase::setIsTarget", c_staticSetIsTarget);


	// Player object functions
   console->addCommand(0, "Player::setDetectParameters", c_setDetectParams);
	console->addCommand(0, "Player::setAnimation", c_setPlayerAnimation);
	console->addCommand(0, "Player::getArmor", c_getPlayerArmor);
	console->addCommand(0, "Player::incItemCount", c_incItemCount);
	console->addCommand(0, "Player::decItemCount", c_decItemCount);
	console->addCommand(0, "Player::setItemCount", c_setItemCount);
	console->addCommand(0, "Player::getItemCount", c_getItemCount);
	console->addCommand(0, "Player::getItemClassCount", c_getItemClassCount);
	console->addCommand(0, "Player::mountItem", c_mountItem);
	console->addCommand(0, "Player::unmountItem", c_unmountItem);
	console->addCommand(0, "Player::getMountedItem", c_getMountedItem);
	console->addCommand(0, "Player::getNextMountedItem", c_getNextMountedItem);
	console->addCommand(0, "Player::useItem", c_useItem);
	console->addCommand(0, "Player::dropItem", c_dropItem);
	console->addCommand(0, "Player::deployItem", c_deployItem);
	console->addCommand(0, "Player::setMountObject", c_mountObject);
	console->addCommand(0, "Player::getMountObject", c_getMountObject);
	console->addCommand(0, "Player::trigger", c_trigger);
	console->addCommand(0, "Player::isTriggered", c_isTriggered);
	console->addCommand(0, "Player::setSensorSupression", c_setSensorSupression);
	console->addCommand(0, "Player::getSensorSupression", c_getSensorSupression);
	console->addCommand(0, "Player::isDead",c_isDead);
	console->addCommand(0, "Player::applyImpulse",c_applyImpulse);
	console->addCommand(0, "Player::kill",c_kill);
	console->addCommand(0, "Player::getDamageFlash",c_getDamageFlash);
	console->addCommand(0, "Player::setDamageFlash",c_setDamageFlash);
   console->addCommand(0, "Player::setArmor",c_setPlayerArmor);
	console->addCommand(0, "Player::isExposed",c_isExposed);
	console->addCommand(0, "Player::isJetting",c_isJetting);
	console->addCommand(0, "Player::lastJetTime",c_lastJetTime);
	console->addCommand(0, "Player::blowUp", c_playerBlowUp);
	console->addCommand(0, "Player::getLastContactCount", c_lastContactCount);
	console->addCommand(0, "Player::isCrouching", c_isCrouching);
	console->addCommand(0, "Player::isAiControlled", c_isAiControlled);

	// Turret functions
	console->addCommand (0, "Turret::setTarget", c_turretSetTarget);

	// Vehicle functions
	console->addCommand (0, "Vehicle::canMount", c_vehicleCanMount);
	console->addCommand (0, "Vehicle::canRide", c_vehicleCanRide);
	console->addCommand (0, "Vehicle::nextPassengerPoint", c_vehicleNextPassengerPoint);
	console->addCommand (0, "Vehicle::incPassengers", c_vehicleIncPassengers);
	console->addCommand (0, "Vehicle::decPassengers", c_vehicleDecPassengers);
	console->addCommand (0, "Vehicle::getMountPoint", c_vehicleGetMountPoint);

	// Item object functions
	console->addCommand(0, "Item::getItemData", c_getObjectItemData);
	console->addCommand(0, "Item::setVelocity", c_setVelocity);
	console->addCommand(0, "Item::getVelocity", c_getVelocity);
	console->addCommand(0, "Item::getCount", c_getObjectItemCount);
	console->addCommand(0, "Item::isRotating",c_isRotatingItem);
	console->addCommand(0, "Item::hide", c_hideItem);
	console->addCommand(0, "which", c_which);

	// Moveable object functions
	console->addCommand(0, "Moveable::moveToWaypoint", c_moveToWaypoint);
	console->addCommand(0, "Moveable::moveForward", c_moveForward);
	console->addCommand(0, "Moveable::moveBackward", c_moveBackward);
	console->addCommand(0, "Moveable::stop", c_moveStop);
	console->addCommand(0, "Moveable::getState", c_moveGetState);
	console->addCommand(0, "Moveable::getWaypointCount", c_moveGetCount);
	console->addCommand(0, "Moveable::getPosition", c_moveGetPosition);
	console->addCommand(0, "Moveable::setWaypoint", c_setWaypoint);

	// Interior object functions
	console->addCommand(0, "Interior::setLightTime",   c_interiorSetLightTime);
	console->addCommand(0, "Interior::toggleLight",    c_interiorToggleSwitchLight);
	console->addCommand(0, "Interior::switchOnLight",  c_interiorTurnOnSwitchLight);
	console->addCommand(0, "Interior::switchOffLight", c_interiorTurnOffSwitchLight);

   // ShapeView functions
	console->addCommand(0, "ShapeView::setItem", ShapeViewSetItem);
	console->addCommand(0, "ShapeView::setShape", ShapeViewSetShape);
	console->addCommand(0, "ShapeView::setSkin", ShapeViewSetSkin);

   console->addCommand(0, "Projectile::spawnProjectile", c_spawnProjectile);

   console->addCommand(0, "GUI::isIn8BitMode", c_isIn8BitMode);
   console->addCommand(0, "GUI::issue8BitWarning", c_issue8BitWarning);
   console->addCommand(0, "GUI::issueInternetWarning", c_issueInternetWarning);

   console->addCommand(0, "RegisterStaticTextures", c_registerStaticTextures);

	// Preferences
   extern bool gui_ignoreTargetsPref;
   extern bool gSensorNetworkEnabled;

	console->addVariable(0, "pref::IgnoreTargets", CMDConsole::Bool, &gui_ignoreTargetsPref);

   console->addVariable(0, "pref::NumDecals", CMDConsole::Int, &DecalManager::iPrefNumDecals);
	console->addVariable(0, "pref::TerrainVisibleDistance",CMDConsole::Float,&SimTerrain::PrefVisibleDistance);
	console->addVariable(0, "pref::TerrainPerspectiveDistance",CMDConsole::Unit,&SimTerrain::PrefPerspectiveDistance);
	console->addVariable(0, "pref::TerrainDetail",CMDConsole::Unit,&SimTerrain::PrefDetail);
	console->addVariable(0, "pref::TerrainTextureDetail",CMDConsole::Unit,&SimTerrain::PrefTerrainTextureDetail);
	console->addVariable(0, "pref::TerrainVisibleDistance",verifyTerrainPrefDist);
	console->addVariable(0, "pref::InteriorTextureDetail",CMDConsole::Int,&ITRRender::PrefInteriorTextureDetail);

   console->addVariable(0, "SensorNetworkEnabled", CMDConsole::Bool, &gSensorNetworkEnabled);
   console->addVariable(0, "net::predictForwardTime", CMDConsole::Int, &cg.predictForwardTime);
   console->addVariable(0, "net::interpolateTime", CMDConsole::Int, &cg.interpolateTime);
   console->addVariable(0, "pref::damageSkinDetail", CMDConsole::Int, &ShapeBase::prefDamageSkinDetail);
   console->addVariable(0, "pref::useCustomSkins", CMDConsole::Bool, &ShapeBase::prefUseCustomSkins);

   console->addVariable(0, "pref::shadowDetailMask", CMDConsole::Int, &ShapeBase::prefShadowDetailMask);
   console->addVariable(0, "pref::shadowDetailScale", CMDConsole::Unit, &ShapeBase::prefShadowDetailScale);
   console->addVariable(0, "pref::playerShapeDetail", CMDConsole::Unit, &Player::prefShapeDetail);
   console->addVariable(0, "pref::vehicleShapeDetail", CMDConsole::Unit, &Vehicle::prefShapeDetail);
   console->addVariable(0, "pref::staticShapeDetail", CMDConsole::Unit, &StaticBase::prefShapeDetail);
   console->addVariable(0, "pref::skyDetail", CMDConsole::Unit, &Sky::PrefSkyDetail);
   console->addVariable(0, "pref::starDetail", CMDConsole::Unit, &StarField::PrefStars);
   console->addVariable(0, "pref::freeLook", CMDConsole::Bool, &Player::prefFreeLook);

   console->addVariable(0, "pref::interiorLightUpdateMS", CMDConsole::Int, &InteriorShape::sm_minLightUpdateMS);
   console->addVariable(0, "pref::dynamicLightDistance", CMDConsole::Float, &SimLightSet::sm_lightIgnoreDistance);

   console->addVariable(0, "pref::maxNumSmokePuffs", CMDConsole::Int, &SmokeManager::sm_numPuffsAllowed);

   console->addVariable(0, "pref::weatherPrecipitation", CMDConsole::Bool, &Snowfall::precipitation);
   console->addVariable(0, "pref::flipFlierY", CMDConsole::Bool, &Flier::flipYAxis);
   console->addVariable(0, "pref::filterBadWords", CMDConsole::Bool, &filterBadWords);
   console->addVariable(0, "screenError", CMDConsole::Float, &screenError);
   console->addVariable(0, "farDistance", CMDConsole::Float, &farDistance);

	// Debugging
#ifdef DEBUG
   console->addVariable(0, "ShapeBase::renderBoundingBox", CMDConsole::Bool, &ShapeBase::renderBoundingBox);
   console->addVariable(0, "ShapeBase::renderLightRays", CMDConsole::Bool,   &ShapeBase::renderLightRays);
#endif
   console->addVariable(0, "pref::animateWithTransitions", CMDConsole::Bool,   &gAnimateTransitions);
	console->addVariable(0, "GameBase::upFov", CMDConsole::Float, &GameBase::upFov);
	console->addVariable(0, "GameBase::upDistance", CMDConsole::Float, &GameBase::upDistance);
	console->addVariable(0, "GameBase::upVelocity", CMDConsole::Float, &GameBase::upVelocity);
	console->addVariable(0, "GameBase::upSkips", CMDConsole::Float, &GameBase::upSkips);
	console->addVariable(0, "GameBase::upOwnership", CMDConsole::Float, &GameBase::upOwnership);
	console->addVariable(0, "GameBase::upInterest", CMDConsole::Float, &GameBase::upInterest);
}


//----------------------------------------------------------------------------

void FearPlugin::startFrame()
{
}

void FearPlugin::endFrame()
{
}


//----------------------------------------------------------------------------

const char *FearPlugin::consoleCallback(CMDConsole*,int id,int argc,const char *argv[])
{
   FearCSDelegate *del;
	switch(id) {
      case GetTransportAddress:
         if(sg.playerManager && argc == 2)
         {
            PlayerManager::ClientRep *cl = sg.playerManager->findClient(atoi(argv[1]));
            if(!cl || !cl->packetStream || !cl->packetStream->getVC())
               return "";
            return cl->packetStream->getVC()->getAddressString();
         }
         return "";
      case ListPlayers:
      {
         if(sg.manager && sg.playerManager)
         {
            console->printf("Server: %d clients", sg.playerManager->getNumClients());
            for(PlayerManager::ClientRep *i = sg.playerManager->getClientList();i;i = i->nextClient)
            {
               DNet::VC *vc = i->packetStream->getVC();
               console->printf("%d (%s): %s - %s", i->id, vc->getAddressString(), i->name, i->scoreString);
            }
         }
         else if(cg.playerManager)
         {
            console->printf("Client: %d clients", cg.playerManager->getNumClients());
            for(PlayerManager::ClientRep *i = cg.playerManager->getClientList();i;i = i->nextClient)
            {
               console->printf("%d: %s - %s", i->id, i->name, i->scoreString);
            }
         }
      }
      break;

//----------------------------------------------------------------------------
// Fear Objects
//----------------------------------------------------------------------------
      case ResetPlayerManager:
         if(sg.playerManager)
            sg.playerManager->missionReset();
         return "True";
      case ResetGhostManagers:
         {
            SimSet *gmSet = (SimSet *) manager->findObject(GhostManagerSetId);
            SimSet::iterator i;

            // set all the ghost managers back into ghost always mode
            for(i = gmSet->begin(); i != gmSet->end(); i++)
               ((Net::GhostManager *) (*i))->reset();
         }
         return "True";
      case StartGhosting:
         {
            if(argc != 2)
               return "False";
            Net::PacketStream *ps = dynamic_cast<Net::PacketStream *>(manager->findObject(atoi(argv[1])));
            if(!ps)
               return "False";
            ps->getGhostManager()->activate();
         }
         return "True";

      case RenderCanvas:
         if (argc > 0)
         {
            SimCanvas *pCanvas = dynamic_cast<SimCanvas *>(manager->findObject(argv[1]));
            if (pCanvas)
            {
               pCanvas->lock();
               pCanvas->paint();
               pCanvas->unlock();
            }
            else
            {
               console->printf("renderCanvas: could not find specified canvas");
            }
         }
         break;

      case RemotePlayAnim:
         if(argc == 3 && sg.playerManager)
         {
            int pid = atoi(argv[1]);
            int anim = atoi(argv[2]);
            PlayerManager::ClientRep *pr = sg.playerManager->findBaseRep(pid);
            if(pr && pr->ownedObject)
            {
               Player *pl = dynamic_cast<Player *>(pr->ownedObject);
               if(pl)
                  pl->serverWave(anim);
            }
         }
         break;
         
      case IssueCommandI:
         if(argc == 7 && sg.playerManager)
            sg.playerManager->issueCommand(atoi(argv[1]), atoi(argv[2]),
               atoi(argv[3]), stringTable.insert(argv[4], true), 
               Point2I(atoi(argv[5]), atoi(argv[6])) );
         break;
      case IssueCommand:
         if(argc == 7 && sg.playerManager)
         {
            Point2I wayPoint(
               (atof(argv[5]) - sg.missionCenterPos.x) * 1023.0 / sg.missionCenterExt.x,
               (atof(argv[6]) - sg.missionCenterPos.y) * 1023.0 / sg.missionCenterExt.y);

            sg.playerManager->issueCommand(atoi(argv[1]), atoi(argv[2]),
               atoi(argv[3]), stringTable.insert(argv[4], true), wayPoint);
         }
         break;
      case IssueTargCommand:
         if(argc == 6 && sg.playerManager)
            sg.playerManager->issueCommand(atoi(argv[1]), atoi(argv[2]),
               atoi(argv[3]), stringTable.insert(argv[4], true), Point2I(0,0), atoi(argv[5]));
         break;
         
      case RemoteSelectCommander:
         // format is: 
         // remoteSelectCommander <commandeeID> <commanderID>
         if(argc == 3 && sg.playerManager)
         {
            int peonId = atoi(argv[1]);
            int cmdrId = atoi(argv[2]);

            sg.playerManager->selectCommander(peonId, cmdrId);
         }
         else
         {
            console->printf("RemoteSelectCommander with %d args.", argc);
         }
         break;
         
      case SetCommandStatus:
         if(argc == 4 && sg.playerManager)
         {
            int peonId = atoi(argv[1]);
            int status = atoi(argv[2]);
            StringTableEntry message = stringTable.insert(argv[3], true);
            if(sg.playerManager->commandStatusReport(peonId, status, message))
               return "True";
            else
               return "False";
         }
         break;
      case AddChatCommand:
         {
            if(argc < 4)
               Console->printf("%s(object, menuString, command, ...);", argv[0]);
            else
            {
               SimObject *obj = manager->findObject(argv[1]);
               ChatMenu *cmenu = NULL;
               if(obj)
                  cmenu = dynamic_cast<ChatMenu *>(obj);
               if(!cmenu)
                  Console->printf("%s - invalid menu object.", argv[0]);
               else
               {
                  if(!cmenu->addMenuString(argv[2], argc - 3, argv + 3))
                     Console->printf("Error adding menu to %s", argv[1]);
               }
            }
         }               
         break;
      case SetChatMode:
         {
            if(argc == 3)
            {
               SimObject *obj = manager->findObject(argv[1]);
               ChatMenu *cmenu = NULL;
               if(obj)
                  cmenu = dynamic_cast<ChatMenu *>(obj);
               if(!cmenu)
                  Console->printf("%s - invalid menu object.", argv[0]);
               else
                  cmenu->setMode(atoi(argv[2]));
            }
            else
               Console->printf("%s(menuObject, mode);", argv[0]);
	      }
	}
	return 0;
}


