#include <gamebase.h>
#include <feardcl.h>
#include <moveablebase.h>
#include <simconsoleplugin.h>
#include <scriptplugin.h>
#include <player.h>
#include <TeamGroup.h>
#include <DropPoints.h>
#include <PlayerManager.h>
#include <fearglobals.h>
#include <fearplayerpsc.h>
#include <netGhostManager.h>
#include <observercamera.h>
#include <fearguicmdobserve.h>
#include <grdBlock.h>

#define onCommand(T) \
	case T : return on##T(argc, argv);

enum CallbackID
{
};

ScriptPlugin::ScriptPlugin()
{
}

ScriptPlugin::~ScriptPlugin()
{
}

static const char *intToStr(int d)
{
   static char buf[16];
   sprintf(buf, "%d", d);
   return buf;
}

extern GameBase *findGameObject(const char *name);

static const char *c_getObserverCamera(CMDConsole *, int, int argc, const char **argv)
{
   if(sg.playerManager == NULL || sg.manager == NULL || argc != 2)
      return "-1";
   int clientId = atoi(argv[1]);
   PlayerManager::ClientRep *cl = sg.playerManager->findClient(clientId);
   if(!cl || !cl->observerCamera )
      return "-1";

   return intToStr(cl->observerCamera->getId());
}

static const char *c_spawnPlayer(CMDConsole *, int, int argc, const char **argv)
{
   if(!sg.playerManager || !sg.manager || argc != 4)
      return "-1";

   Point3F pos(0, 0, 0), rot(0, 0, 0);
   sscanf( argv[2], "%f %f %f", &pos.x, &pos.y, &pos.z);
   sscanf( argv[3], "%f %f %f", &rot.x, &rot.y, &rot.z);

   Player *player = new Player();
   player->setInitInfo(argv[1], pos, rot);

   if(!sg.manager->registerObject(player))
   {
      delete player;
      return "-1";
   }

   SimGroup *cleanupGroup = (SimGroup *) sg.manager->findObject("MissionCleanup");
   if(cleanupGroup)
      cleanupGroup->addObject(player);
   return intToStr(player->getId());
}

static const char *c_obsSetOrbitObject(CMDConsole *, int, int argc, const char **argv)
{
   if(!sg.playerManager || !sg.manager || argc < 6)
      return "false";
   Point3F rot(0,0,0);
   float cd, minD, maxD;
   PlayerManager::ClientRep *pl = sg.playerManager->findClient(atoi(argv[1]));
   if(!pl)
      return "false";

   ObserverCamera *oc = pl->observerCamera;
   GameBase *gb = findGameObject(argv[2]);
   if(!oc || !gb)
      return "false";
   cd = atof(argv[3]);
   minD = atof(argv[4]);
   maxD = atof(argv[5]);
   if(argc == 7)
      sscanf(argv[6], "%f %f %f", &rot.x, &rot.y, &rot.z);
   oc->setOrbitMode(gb, Point3F(0,0,0), rot, minD, maxD, cd);
   return "True";
}

static const char *c_obsSetOrbitPoint(CMDConsole *, int, int argc, const char **argv)
{
   if(!sg.playerManager || !sg.manager || argc < 6)
      return "false";
   Point3F rot(0,0,0), pos(0,0,0);
   float cd, minD, maxD;
   PlayerManager::ClientRep *pl = sg.playerManager->findClient(atoi(argv[1]));
   if(!pl)
      return "false";

   ObserverCamera *oc = pl->observerCamera;
   if(!oc)
      return "false";
   sscanf(argv[2], "%f %f %f", &pos.x, &pos.y, &pos.z);
   cd = atof(argv[3]);
   minD = atof(argv[4]);
   maxD = atof(argv[5]);
   if(argc == 7)
      sscanf(argv[6], "%f %f %f", &rot.x, &rot.y, &rot.z);
   oc->setOrbitMode(NULL, pos, rot, minD, maxD, cd);
   return "True";
}

static const char *c_obsSetDamageObject(CMDConsole *, int, int argc, const char **argv)
{
   if(!sg.playerManager || !sg.manager || argc != 3)
      return "false";
   PlayerManager::ClientRep *pl = sg.playerManager->findClient(atoi(argv[1]));
   if(!pl)
      return "false";

   ObserverCamera *oc = pl->observerCamera;
   GameBase *gb = findGameObject(argv[2]);
   if(!oc || !gb)
      return "false";
   oc->setDamageObject(gb);
	return "true";
}

static const char *c_obsSetFlyMode(CMDConsole *, int, int argc, const char **argv)
{
   if(!sg.playerManager || !sg.manager || argc != 6)
      return "false";
   Point3F rot(0,0,0), pos(0,0,0);
   bool canMove, canRotate;
   PlayerManager::ClientRep *pl = sg.playerManager->findClient(atoi(argv[1]));
   if(!pl)
      return "false";

   ObserverCamera *oc = pl->observerCamera;

   sscanf(argv[2], "%f %f %f", &pos.x, &pos.y, &pos.z);
   sscanf(argv[3], "%f %f %f", &rot.x, &rot.y, &rot.z);
   canMove = CMDConsole::getBool(argv[4]);
   canRotate = CMDConsole::getBool(argv[5]);
   oc->setFlyMode(canMove, canRotate, pos, rot);
   return "True";
}

static const char *c_toggleCMDObserver(CMDConsole *, int, int argc, const char **argv)
{
   bool fActivate;

   if (cg.cmdObserve && argc == 2)
   {
      if (stricmp(argv[1], "true") == 0)
      {
         fActivate = true;
      }
      else if (stricmp(argv[1], "false") == 0)
      {
         fActivate = false;
      }
      else
      {
         fActivate = (bool)atoi(argv[1]);
      }

      cg.cmdObserve->activate(fActivate);
      return "True";
   }
   return "False";
}

static const char *c_cmdObservePlayer(CMDConsole *, int, int argc, const char **argv)
{
   if (cg.cmdObserve && argc == 2)
   {
      if (cg.cmdObserve->observePlayer(atoi(argv[1])))
      {
         return "True";
      }
   }
   return "False";
}

void ScriptPlugin::init()
{
   console->printf("ScriptPlugin");

   console->addCommand(0, "spawnPlayer", c_spawnPlayer, 1);
   console->addCommand(0, "Client::getObserverCamera", c_getObserverCamera, 1);
   console->addCommand(0, "Observer::setOrbitObject", c_obsSetOrbitObject, 1);
   console->addCommand(0, "Observer::setOrbitPoint", c_obsSetOrbitPoint, 1);
   console->addCommand(0, "Observer::setFlyMode", c_obsSetFlyMode, 1);
   console->addCommand(0, "Observer::setDamageObject", c_obsSetDamageObject, 1);
   console->addCommand(0, "Client::toggleCMDObserver", c_toggleCMDObserver, 1);
   console->addCommand(0, "Client::cmdObservePlayer", c_cmdObservePlayer, 1);
   
   console->addVariable(0, "pref::HrlmDetail", CMDConsole::Int, &GridHrlmList::PrefDetail );
}

