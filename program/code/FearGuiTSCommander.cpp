
#include <sim.h>
#include <ml.h>
#include <ts.h>
#include "simRenderGrp.h"
#include "SimGuiTSCtrl.h"
#include "g_surfac.h"
#include "g_pal.h"
#include "ts_PointArray.h"
#include "simCamera.h"
#include "simContainer.h"

#include "Player.h"
#include "PlayerManager.h"
#include "FearGuiTSCommander.h"
#include "FearGuiHudCtrl.h"
#include "FearGuiUnivButton.h"
#include "editor.strings.h"
#include "fear.strings.h"
#include "gwDeviceManager.h"
#include "g_bitmap.h"
#include "simLightGrp.h"
#include "fearGlobals.h"
#include "fearGuiCurCmd.h"
#include "simGame.h"
#include "dataBlockManager.h"
#include "fearPlayerPSC.h"
#include "dlgPlay.h"
#include "sensor.h"
#include "chatmenu.h"
#include "fearGuiShellPal.h"
#include "simPalette.h"

static TSPointArray  DefaultPointArray;

MissionCenterPos::MissionCenterPos()
{
   netFlags.set(Ghostable);
   netFlags.set(ScopeAlways);
   id = MissionCenterPosId;
   pos(1230, 1230);
   extent(1400,1400);
}

bool MissionCenterPos::onAdd()
{
   if(!Parent::onAdd())
      return false;
   wg->missionCenterPos = pos;
   wg->missionCenterExt = extent;
   return TRUE;
}

bool MissionCenterPos::processArguments(int argc, const char **argv)
{
	if (argc == 0)
		return true;
   if (argc == 4) {
   	pos.x = atof(argv[0]);
   	pos.y = atof(argv[1]);
		extent.x = atof(argv[2]);
		extent.y = atof(argv[3]);
		return true;
   }
	return false;
}	

void MissionCenterPos::initPersistFields()
{
   addField("x", TypeFloat, Offset(pos.x, MissionCenterPos));
   addField("y", TypeFloat, Offset(pos.y, MissionCenterPos));
   addField("w", TypeFloat, Offset(extent.x, MissionCenterPos));
   addField("h", TypeFloat, Offset(extent.y, MissionCenterPos));
}

void MissionCenterPos::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);

   insp->write(IDITG_POSITION, pos);
   insp->write(IDITG_EXTENT_DIM, extent);
}   

void MissionCenterPos::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);

   insp->read(IDITG_POSITION, pos);
   insp->read(IDITG_EXTENT_DIM, extent);
   sg.missionCenterPos = pos;
   sg.missionCenterExt = extent;
   setMaskBits(1);
}   

Persistent::Base::Error MissionCenterPos::write( StreamIO &sio, int version, int user )
{
   sio.write(pos.x);
   sio.write(pos.y);
   sio.write(extent.x);
   sio.write(extent.y);
   return Parent::write(sio, version, user);
}

Persistent::Base::Error MissionCenterPos::read( StreamIO &sio, int version, int user)
{
   sio.read(&pos.x);
   sio.read(&pos.y);
   sio.read(&extent.x);
   sio.read(&extent.y);
   return Parent::read(sio, version, user);
}

DWORD MissionCenterPos::packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream)
{
   mask;
   gm;

   stream->write(pos.x);
   stream->write(pos.y);
   stream->write(extent.x);
   stream->write(extent.y);
   return 0;
}

void MissionCenterPos::unpackUpdate(Net::GhostManager *, BitStream *stream)
{
   stream->read(&pos.x);
   stream->read(&pos.y);
   stream->read(&extent.x);
   stream->read(&extent.y);

   wg->missionCenterPos = pos;
   wg->missionCenterExt = extent;
   if(cg.commandTS)
      cg.commandTS->resetPos();
}

IMPLEMENT_PERSISTENT_TAG(MissionCenterPos, MissionCenterPosId);


namespace FearGui
{


IMPLEMENT_PERSISTENT_TAG(TSCommander, FOURCC('F','G','C','c'));

int TSCommander::prefMapFilter = MapFilterPlayers;
bool TSCommander::prefShowSensor = false;
bool TSCommander::prefShowNametags = true;
bool TSCommander::prefSensorTranslucent = false;

const char *cmdZoomPrefVar = "pref::CommanderZoomTime";
int TSCommander::mZoomTransitionTime = 250;
const char *TSCommander::validateZoomTime(CMDConsole*,int id,int argc, const char *argv[])
{
	id, argc, argv;
   
   if (mZoomTransitionTime < 1) mZoomTransitionTime = 1;
   if (mZoomTransitionTime > 2000) mZoomTransitionTime = 2000;
   
   return 0;
}

TSCommander::TSCommander()
{
   execStarredCommand = false;
   draggingMouse = false;
   scrollingMouse = false;
   zoomMode = false;
   centerMode = false;
   mouseMode = SelectingMode;

	playerFacingAngle = 0;
   
   deselectUnits = FALSE;
   selectType = DataBlockManager::PlayerDataType;
   
   worldPosition(1230, 1230);
   worldExtent(1400,1400);
   
   //initialize smooth zoom vars
   mbZooming = FALSE;
   mbPreventDragging = FALSE;
   zoomLevel = 0;
    
   int i, j;
   for(i = 0; i < 4; i++)
      for(j = 0; j < 4; j++)
         bitmapList[j][i] = GFXBitmap::create(256, 256);

   cg.commandTS = this;
   curCommand = 0;
   curCommandString = NULL;
   
	Console->addVariable(0,cmdZoomPrefVar, CMDConsole::Int, &mZoomTransitionTime);
	Console->addVariable(0,cmdZoomPrefVar, validateZoomTime);
	Console->addVariable(0, "pref::mapFilter", CMDConsole::Int, &TSCommander::prefMapFilter);
	Console->addVariable(0, "pref::mapSensorRange", CMDConsole::Bool, &TSCommander::prefShowSensor);
	Console->addVariable(0, "pref::mapNames", CMDConsole::Bool, &TSCommander::prefShowNametags);
	Console->addVariable(0, "pref::mapSensorTranslucent", CMDConsole::Bool, &TSCommander::prefSensorTranslucent);

   for(i = 0; i < 256; i++)
   {
      targs[i].datId = -1;
      targs[i].datGroup = -1;
   }
}

TSCommander::~TSCommander()
{
   int i, j;
   for(i = 0; i < 4; i++)
      for(j = 0; j < 4; j++)
         delete bitmapList[j][i];
   if(cg.commandTS == this)
      cg.commandTS = NULL;
}

void TSCommander::resetPos()
{
   MissionCenterPos *mpos = (MissionCenterPos *) manager->findObject(MissionCenterPosId);
   if(mpos)
   {
      worldPosition = mpos->pos;
      worldExtent = mpos->extent;
   }

   worldScale.x = extent.x / worldExtent.x;
   worldScale.y = extent.y / worldExtent.y;
}

void TSCommander::buildMap()
{
   zoomLevel = 0;
   zoomMode = FALSE;
   mbZooming = FALSE;
   mbPreventDragging = FALSE;
   centerMode = FALSE;
   
   resetPos();
   int i, j;

   Resource<GFXPalette> pal;
   SimGroup *group = (SimGroup *) manager->findObject("GhostGroup");
   if(group)
   {
      SimGroup::iterator i;
      for(i = group->begin(); i != group->end(); i++)
      {
         SimPalette *spal = dynamic_cast<SimPalette *>(*i);
         if(spal)
         {
            pal = spal->getPalette();
            break;
         }
      }  
   }
   if(!bool(pal))
      return;

   Box2I boundsBox;
   Box2I visibleBox;
   boundsBox.fMin.x = 0;
   boundsBox.fMin.y = 0;
   boundsBox.fMax.x = 1024;
   boundsBox.fMax.y = 1024;
   GFXBitmap *scratch = GFXBitmap::create(128, 128);

   GFXSurface *mem = deviceManager.getGFXDeviceManager()->createMemSurface(scratch);
   mem->setPalette(pal);

   numTargets = 0;
   for(i = 0; i < MaxNumTargs; i++)
   {
      targs[i].datId = -1;
      targs[i].name = NULL;
      selectTarget(i, FALSE);
   }

   for(i = 0; i < 8; i++)
   {
      for(j = 0; j < 8;j++)
      {
         visibleBox.fMin.x = i * 128;
         visibleBox.fMin.y = j * 128;
         visibleBox.fMax.x = i * 128 + 128;
         visibleBox.fMax.y = j * 128 + 128;
         setupCamera(visibleBox, boundsBox);

         // Get the scene lighting information
         TSSceneLighting *sceneLights;
         SimLightSet *simLightSet = (SimLightSet *)(manager->findObject(SimLightSetId));
         sceneLights = simLightSet->getSceneLighting();
            
      	// Setup the render context
      	renderContext.setCamera(camera);
      	renderContext.setPointArray(&DefaultPointArray);
         DefaultPointArray.setOrtho(true);
      	renderContext.setSurface(mem);
      	renderContext.setLights(sceneLights);

      	// Lock
      	renderContext.lock();

         mem->setZMode(false);
         mem->setSurfaceAlphaBlend(&alphaColor, alphaBlend);
      	// Render through the container render context
      	SimContainerRenderContext cContext;
      	cContext.lock(this,&renderContext);

      	// Render items in the RenderSet first
         //
         cContext.notRenderingContainers = true;
         
         SimRenderSet* SRgrp = ::findObject(manager,SimRenderSetId,(SimRenderSet*)0);
      	if (SRgrp != 0)
      		for (SimRenderSet::iterator ptr = SRgrp->begin(); ptr != SRgrp->end(); ptr++)
      			cContext.render(*ptr);

         cContext.notRenderingContainers = false;

      	// Render items in the containers
      	SimContainer* root = ::findObject(manager,SimRootContainerId,(SimContainer*)0);
      	if (root)
      		root->render(cContext);

      	// Unlock
      	cContext.unlock();
      	renderContext.unlock();

         GFXBitmap *dest = bitmapList[j >> 1][i >> 1];
         int xo = (i & 1) << 7;
         int yo = (j & 1) << 7;
         int rc;
         for(rc = 0; rc < 128; rc++)
            memcpy(dest->getAddress(xo, yo + rc), scratch->getAddress(0,rc), 128);
      }
   }
   deviceManager.getGFXDeviceManager()->freeMemSurface(mem);
   delete scratch;
   
   //now adjust the scale to fit the parent
   if (parent)
   {
      int maxExtent = min(parent->extent.x, parent->extent.y);
      float adjust = float(maxExtent) / max(extent.x, extent.y);
      extent.x = int(adjust * float(extent.x));
      extent.y = int(adjust * float(extent.y));
      
      if (extent.x < parent->extent.x) position.x = (parent->extent.x - extent.x) / 2;
      if (extent.y < parent->extent.y) position.y = (parent->extent.y - extent.y) / 2;
      
      //reset the world scale
      worldScale.x = extent.x / worldExtent.x;
      worldScale.y = extent.y / worldExtent.y;
   }
}

bool TSCommander::onAdd()
{
   if(!Parent::onAdd())
      return false;

   playerManager = (PlayerManager *) manager->findObject(PlayerManagerId);
   textFont = SimResource::get(manager)->load("if_small.pft");

   bma = SimResource::get(manager)->load("CmdIconsCtrl.pba", true);
   AssertFatal((bool)bma, "Unable to load bma");
   
   GFXBitmap *bitmap;
   int i;
   for (i = 0; i < BmpCount; i++)
   {
      bitmap = bma->getBitmap(i);
      bitmap->attribute |= BMA_TRANSPARENT;
   }  
   
   playerBMA = SimResource::get(manager)->load("CmdWhereAmI.pba", true);
   AssertFatal((bool)playerBMA, "Unable to load bma");
   
   for (i = 0; i < BmpWhereAmI_Count; i++)
   {
      bitmap = playerBMA->getBitmap(i);
      bitmap->attribute |= BMA_TRANSPARENT;
   }  
   
   bmpXMarksTheSpot = SimResource::get(manager)->load("M_X_Target.bmp");
   AssertFatal(bmpXMarksTheSpot, "Unable to load M_X_Target bitmap");
   bmpXMarksTheSpot->attribute |= BMA_TRANSPARENT;
   
   return true;
}

void TSCommander::onPreRender(void)
{
   if(root->getMouseLockedControl() != this)
   {
      scrollingMouse = false;
      draggingMouse = false;
   }

   //if we're zooming, adjust the position and extent
   if (mbZooming)
   {
      Point2I newPos, newExt;
      //see if we're past the zoom time
      DWORD curTime = GetTickCount();
      if (curTime >= startZoomTime + mZoomTransitionTime)
      {
         newPos = targetZoomPosition;
         newExt = targetZoomExtent;
         mbZooming = FALSE;
      }
      else
      {
         float zoomPercent = float(curTime - startZoomTime) / float(mZoomTransitionTime);
         newPos.x = startZoomPosition.x + int(float(targetZoomPosition.x - startZoomPosition.x) * zoomPercent);
         newPos.y = startZoomPosition.y + int(float(targetZoomPosition.y - startZoomPosition.y) * zoomPercent);
         newExt.x = startZoomExtent.x + int(float(targetZoomExtent.x - startZoomExtent.x) * zoomPercent);
         newExt.y = startZoomExtent.y + int(float(targetZoomExtent.y - startZoomExtent.y) * zoomPercent);
      }
    
      //now resize with the new zoom position and extent  
      resize(newPos, newExt);
      worldScale.x = extent.x / worldExtent.x;
      worldScale.y = extent.y / worldExtent.y;
   }
   
   //if we're centering
   if (centerMode)
   {
      centerView();
   }
   
   Point2I parentOffset;
   parentOffset.x = -position.x;
   parentOffset.y = -position.y;
   setUpdateRegion(parentOffset, parent->extent);
   //Parent::onPreRender();
}

int gWhereAmIBmpTable[36] =
{
    12, 0, 0,                        32, 1, 0,                           52, 2, 0,
    76, 3, 0,                        96, 2, GFX_FLIP_Y,                 116, 1, GFX_FLIP_Y,
   140, 0, GFX_FLIP_Y,              160, 1, GFX_FLIP_X + GFX_FLIP_Y,    180, 2, GFX_FLIP_X + GFX_FLIP_Y,
   204, 3, GFX_FLIP_X,              224, 2, GFX_FLIP_X,                 244, 1, GFX_FLIP_X,
};

enum iconModes
{
   ITEM_ICONS = 0,
   ITEM_NAMES,
   PLAYER_ICONS,
   PLAYER_NAMES,
   MY_ICON,
   MODE_COUNT
};

void TSCommander::drawCommanderIcons(GFXSurface *sfc, Point2I offset, BYTE mode)
{
   //now draw the objects      
   Point2I cpos = getCanvas()->getCursorPos();
   cpos -= offset;
   cpos.x -= 4; cpos.y -= 4; // name distance check
   Point2I rectSize(4, 4);
   PlayerManager::ClientRep *me = playerManager->findClient(manager->getId());
   int myTeam = 0;
   if(me) myTeam = me->team;

   for(int i = 0; i < 256; i++)
   {
      if(targs[i].datId == -1)
         continue;
      
      //get the data block
		GameBase::GameBaseData* data = dynamic_cast<GameBase::GameBaseData*>
			(cg.dbm->lookupDataBlock(targs[i].datId,targs[i].datGroup));
      if (! data || (!isFilterOn(MapFilterAll) && !isFilterOn(data->mapFilter)))
      	continue;
      
      if (mode == ITEM_ICONS || mode == ITEM_NAMES)
      {
         if (targs[i].datGroup == DataBlockManager::PlayerDataType) continue;
      }
      else
      {
         if (targs[i].datGroup != DataBlockManager::PlayerDataType) continue;
      }
      
      Point2I pos = targs[i].pos;
      // pos runs 0 to 1023
      // it needs to run 0 to extent.[x,y]

      pos.x = (pos.x * extent.x) >> 10;
      pos.y = (pos.y * extent.y) >> 10;

      pos.y = extent.y - pos.y;
      
      RectI targRect(offset.x + pos.x - rectSize.x, offset.y + (pos.y - rectSize.y),
              offset.x + pos.x + rectSize.x, offset.y + (pos.y + rectSize.y));
                 
      //get the icon if it exists
   	char iconName[256];
   	Resource<GFXBitmap> mapIcon = NULL;
      if (data->mapIcon && data->mapIcon[0])
      {
         sprintf(iconName, "%s_%s.bmp", data->mapIcon, targs[i].team == myTeam ? "green" : "red");
         mapIcon = SimResource::get(manager)->load(iconName);
      }
      
      //see if it is the playerData which represent this client
      if (targs[i].datGroup == DataBlockManager::PlayerDataType && targs[i].playerId == me->id && mode == MY_ICON)
      {
			GameBase *fo = cg.psc->getControlObject();
			if (fo)
			{
	         //calculate the degree range where the middle range is the players
	         Vector3F playerDirection = fo->getAngulerPosition();
	         playerFacingAngle = int(256.0f - (playerDirection.z * 256 / M_2PI));
	         if (playerFacingAngle < 0) playerFacingAngle += 256;
	         if (playerFacingAngle > 256) playerFacingAngle -= 256;  //now 0 <= angle <= 256
			}
         
         //loop through and look for the correct bitmap corresp with the angle
         int bmpIndex = 0;
         int flipFlag = 0;
         for (int i = 0; i < 12; i++)
         {
            if (playerFacingAngle <= gWhereAmIBmpTable[3 * i])
            {
               bmpIndex = gWhereAmIBmpTable[3 * i + 1];
               flipFlag = gWhereAmIBmpTable[3 * i + 2];
               break;
            }
         }
         if (targs[i].selected) bmpIndex += 4;
         
         //get the bitmap
         GFXBitmap *bmp = playerBMA->getBitmap(bmpIndex);
         if (bmp)
         {
            sfc->drawBitmap2d(bmp, &Point2I(offset.x + pos.x - (bmp->getWidth() / 2),
                                            offset.y + pos.y - (bmp->getHeight() /2)),
                                   &Point2I(bmp->getWidth(), bmp->getHeight()), (GFXFlipFlag)(flipFlag));
         }         
      }
      
      else if (bool(mapIcon) && (mode == ITEM_ICONS ||
                                 (mode == PLAYER_ICONS && targs[i].playerId != me->id)))
      {
         mapIcon->attribute |= BMA_TRANSPARENT;
         sfc->drawBitmap2d(mapIcon, &Point2I(offset.x + pos.x - (mapIcon->getWidth() / 2),
                                             offset.y + pos.y - (mapIcon->getHeight() /2)));
      }
      //else if (mode == ITEM_ICONS || (mode == PLAYER_ICONS && targs[i].playerId != me->id))
      //{
      //   int color = targs[i].team == myTeam && cg.playerManager->getNumTeams() > 1 ? GREEN : RED;
      //   sfc->drawRect2d_f(&targRect, color);
      //}

      if (targs[i].datGroup == DataBlockManager::PlayerDataType)
      {
         targRect.upperL -= 2;
         targRect.lowerR += 2;
      
         //find the player from the player manager
         PlayerManager::ClientRep *tg;
         tg = playerManager->findBaseRep(targs[i].playerId);
         if(!tg)
            continue;
            
         if (targs[i].selected && targs[i].playerId != me->id && (mode == PLAYER_ICONS || mode == MY_ICON))
         {
            sfc->drawRect2d(&targRect, WHITE);
         }
         
         if (tg->curCommand != -1 && (mode == PLAYER_ICONS || mode == MY_ICON))
         {
            Point2I drawPos;

            if(tg->curCommandTarget != -1 && targs[tg->curCommandTarget].datId != -1)
            {
               drawPos = targs[tg->curCommandTarget].pos;
               tg->wayPoint = drawPos;
               if(tg == me)
               {
                  cg.wayPoint = cg.missionCenterPos;
                  cg.wayPoint.x += (drawPos.x * cg.missionCenterExt.x) / 1024.0f;
                  cg.wayPoint.y += (drawPos.y * cg.missionCenterExt.y) / 1024.0f;
               }               
            }
            else
               drawPos = tg->wayPoint;

            drawPos.x = (drawPos.x * extent.x) >> 10;
            drawPos.y = (drawPos.y * extent.y) >> 10;
            drawPos.y = extent.y - drawPos.y;
            
            int color;
            if (tg->commandStatus == PlayerManager::CMD_RECEIVED) color = YELLOW;
            else color = WHITE;
            sfc->drawLine2d(&Point2I(offset.x + pos.x, offset.y + pos.y), &Point2I(offset.x + drawPos.x, offset.y + drawPos.y), color);

            //draw the icon at the end
            GFXBitmap *bmp = NULL;
            if ((tg->curCommand > 0) && (tg->curCommand <= BmpCount))
               bmp = bma->getBitmap(tg->curCommand - 1);
            if (bmp)
               sfc->drawBitmap2d(bmp, &Point2I(offset.x + drawPos.x - bmp->getWidth()/2, offset.y + drawPos.y - bmp->getHeight()/2));
         }
         
         if (mode == PLAYER_NAMES || mode == MY_ICON)
         {
            //draw the players name
            Point2I strPos(offset.x + pos.x + rectSize.x, offset.y + pos.y - rectSize.y);
            sfc->drawText_p(textFont, &strPos, tg->name);
         }
      }
      else
      {
         if ((targs[i].datGroup == DataBlockManager::TurretDataType) ||
             (targs[i].datGroup == DataBlockManager::StaticShapeDataType) ||
             (targs[i].datGroup == DataBlockManager::SensorDataType))
         {
            targRect.upperL -= 2;
            targRect.lowerR += 2;
         
            if (targs[i].selected && mode == ITEM_ICONS)
               sfc->drawRect2d(&targRect, WHITE);
         }
         
         Point2I dist = pos - cpos;
         if(targs[i].name && (targs[i].selected || prefShowNametags || (dist.x >= 0 && dist.x <= 8 && dist.y >= 0 && dist.y <= 8)))
         {
            if (mode == ITEM_NAMES)
            {
               const char *itemName = targs[i].name;
               if ((! itemName) || (! itemName[0])) itemName = data->description;
               Point2I strPos(offset.x + pos.x + rectSize.x, offset.y + pos.y - rectSize.y);
               sfc->drawText_p(textFont, &strPos, itemName);
            }
         }
      }
   }
}

void TSCommander::onRender(GFXSurface *sfc, Point2I offset, const Box2I& updateRect)
{
   //tinman - rewrote the drawing of the tiles, as roundoff errors caused cracks in the tiles
   
   Point2I pos = parent->localToGlobalCoord(Point2I(0,0));
   Point2I ext = parent->extent;
   RectI cr(pos.x, pos.y, pos.x + ext.x - 1, pos.y + ext.y - 1);
   sfc->setClipRect(&cr);

   int xb, yb;
   Point2I tileExtent;
   tileExtent.x = int(float(extent.x) / 4.0f); 
   tileExtent.y = int(float(extent.y) / 4.0f); 
   //make sure the width and the height are ceilings
   if (tileExtent.x * 4 < extent.x) tileExtent.x++;
   if (tileExtent.y * 4 < extent.y) tileExtent.y++;
   
   Point2I tileOffset = offset;
   for(yb = 0; yb < 4; yb++)
   {
      tileOffset.x = offset.x;
      for(xb = 0; xb < 4; xb++)
      {
         sfc->drawBitmap2d(bitmapList[yb][xb], &tileOffset, &tileExtent);
         tileOffset.x += tileExtent.x;
      }
      tileOffset.y += tileExtent.y;
   }

   //make sure we have access to the data block manager
   if (! cg.dbm) return;

   Point2I rectSize(4, 4);

   PlayerManager::ClientRep *me = playerManager->findClient(manager->getId());
   int myTeam = 0;
   if(me)
      myTeam = me->team;
   
   //draw the sensor ranges first
   int i;
   if (prefShowSensor)
   {
      for(i = 0; i < 256; i++)
      {
         if ((targs[i].datGroup != DataBlockManager::SensorDataType) &&
             (targs[i].datGroup != DataBlockManager::TurretDataType))
         {
            continue;
         }
            
         if (targs[i].team != myTeam || !targs[i].activeStatus)
            continue;
            
   		Sensor::SensorData* data = (Sensor::SensorData*)(cg.dbm->lookupDataBlock(targs[i].datId,targs[i].datGroup));
         if (! data) continue;
         Point2I pos = targs[i].pos;
         int range = int(float(data->iRange) * worldScale.x);

         pos.x = (pos.x * extent.x) >> 10;
         pos.y = (pos.y * extent.y) >> 10;

         pos.y = extent.y - pos.y;
         
         
         //draw the circular sensor range using through the point array
         TS::PointArray pArray( sfc, RectF(updateRect.fMin.x, updateRect.fMin.y, updateRect.fMax.x, updateRect.fMax.y ));

         //draw the circular sensor range
         const int numPoints = 32;
         const float period = 2 * M_PI / numPoints;
         float r = period;
         
         //initialize our vertices
         Point3F poly[numPoints];
         for (int j = 0; j < numPoints; j++)
         {
            poly[j].z = 1.0f;
         }
         
         //set the vertices
         for (int k = 0; k < numPoints; k++)
         {
            poly[k].x = range * cos(r) + offset.x + pos.x;
            poly[k].y = range * sin(r) + offset.y + pos.y;
               
            r += period;
         }
         
         if (prefSensorTranslucent)
         {
            pArray.useIntensities(false);
            pArray.useTextures(false);
         	//pArray.setVisibility( TS::ClipMask );
            pArray.useHazes(false);
   
            int color = (data->supression ? 252 : 251);
            sfc->setFillMode(GFX_FILL_CONSTANT);
            sfc->setShadeSource(GFX_SHADE_NONE);
            sfc->setHazeSource(GFX_HAZE_NONE);
            sfc->setAlphaSource(GFX_ALPHA_FILL);
            sfc->setTransparency (false);
            sfc->setFillColor(color, ALPHAPAL_INDEX);
            
            TS::VertexIndexPair paPoints[numPoints];
            pArray.reset();
            
            //add the points to the array
            for (int j = 0; j < numPoints; j++)
            {
               paPoints[j].fVertexIndex = pArray.addProjectedPoint(poly[j]);
               paPoints[j].fTextureIndex = j;
            }
            
            //now draw the poly
            pArray.drawProjectedPoly(numPoints, paPoints, 0);
            sfc->setAlphaSource(GFX_ALPHA_NONE);
         }
         else
         {
            int color = (data->supression ? YELLOW : BLUE_HILITE);
            for (int m = 0; m < numPoints; m++)
            {
               sfc->drawLine2d(&Point2I(poly[m].x, poly[m].y),
                               &Point2I(poly[(m + 1) % numPoints].x, poly[(m + 1) % numPoints].y), color); 
            }
         }
      }
   }
   
   SimSet* targetSet = static_cast<SimSet*>(manager->findObject(TargetableSetId));
   AssertFatal(targetSet != NULL, "No targetable set?  Should never happen");

   for (Vector<SimObject*>::iterator itr = targetSet->begin(); itr != targetSet->end(); itr++) {
      Point3F targetPoint;
      int team;

      GameBase* pGBase = (GameBase*)(*itr);
      if (pGBase->getTarget(&targetPoint, &team) == false)
         continue;
      if (team != me->team)
         continue;
      
      Point2F wayPos;
      wayPos.x = targetPoint.x;
      wayPos.y = targetPoint.y;
      wayPos -= worldPosition;
      wayPos *= worldScale;
      wayPos.y = extent.y - wayPos.y;
      
      if (bool(bmpXMarksTheSpot)) sfc->drawBitmap2d(bmpXMarksTheSpot, &Point2I(offset.x + wayPos.x - bmpXMarksTheSpot->getWidth()/2,
                                                                              offset.y + wayPos.y - bmpXMarksTheSpot->getHeight()/2));
   }
   
   //now draw the commander icons
   drawCommanderIcons(sfc, offset, ITEM_ICONS);
   drawCommanderIcons(sfc, offset, ITEM_NAMES);
   drawCommanderIcons(sfc, offset, PLAYER_ICONS);
   drawCommanderIcons(sfc, offset, PLAYER_NAMES);
   drawCommanderIcons(sfc, offset, MY_ICON);

   //draw the rect where the mouse is dragged
   if(draggingMouse)
   {
      Point2I upperL( offset.x + min(mouseDownPoint.x, curMousePoint.x),
                      offset.y + min(mouseDownPoint.y, curMousePoint.y));
      Point2I lowerR( offset.x + max(mouseDownPoint.x, curMousePoint.x),
                      offset.y + max(mouseDownPoint.y, curMousePoint.y));

      sfc->drawRect2d(&RectI(upperL, lowerR), WHITE);
   }
}

bool TSCommander::setupCamera(const Box2I &visibleBox, const Box2I &boundsBox)
{
   TMat3F tmat;
   Point3F p(worldPosition.x + worldExtent.x * .5, 
            worldPosition.y + worldExtent.y * .5, 500);

   tmat.identity();
   tmat.postTranslate(p);
   tmat.preRotateX(-90 * float(M_PI/180.0));

   tmat.inverse();
   camera = orthographicCamera;
   
   nearPlane = 1.0;
   farPlane = 1000;

   worldBounds.upperL.set(-worldExtent.x * .5, worldExtent.y * .5);
   worldBounds.lowerR.set(worldExtent.x * .5, -worldExtent.y * .5);

   camera->setTWC(tmat);
   Point2I size(boundsBox.len_x(), boundsBox.len_y());
  	RectF rect;

   int left = visibleBox.fMin.x - boundsBox.fMin.x;
   int right = visibleBox.fMax.x - boundsBox.fMin.x;
   int top = visibleBox.fMin.y - boundsBox.fMin.y;
   int bottom = visibleBox.fMax.y - boundsBox.fMin.y;

   float hscale = worldBounds.len_x() / float(boundsBox.len_x());
   float vscale = worldBounds.len_y() / float(boundsBox.len_y());

   rect.upperL.set(worldBounds.upperL.x + left * hscale, worldBounds.upperL.y + top * vscale);
   rect.lowerR.set(worldBounds.upperL.x + right * hscale, worldBounds.upperL.y + bottom * vscale);

   camera->setWorldViewport(rect);
	
	camera->setScreenViewport(RectI(Point2I(0,0),Point2I(128, 128)));
	camera->setNearDist(nearPlane);
	camera->setFarDist(farPlane);

   return true;
}

Int32 TSCommander::getMouseCursorTag(void)
{
   if (inZoomMode()) return IDBMP_CURSOR_ZOOM;
   else if (scrollingMouse)
   {
      if (position.x == origPosition.x && position.y == origPosition.y) return IDBMP_CURSOR_OPENHAND;
      else return IDBMP_CURSOR_GRAB;
   }
   else if (mouseMode == WaypointMode) return IDBMP_CURSOR_WAYPOINT;
   else return 0;
}

bool TSCommander::processEvent(const SimEvent *event)
{
   switch(event->type)
   {
      onEvent(SimInputEvent);
      default:
         return Parent::processEvent(event);
   }
}

bool TSCommander::onSimInputEvent(const SimInputEvent *event)
{
   if (event->objType != SI_KEY) return FALSE;
   if (event->objInst != DIK_ESCAPE) return FALSE;
   if (mouseMode == WaypointMode)
   {
      setMouseMode(SelectingMode);
      return TRUE;
   }
   return FALSE;
}

void TSCommander::onMouseDown(const SimGui::Event &event)
{
   //do nothing if we're already dragging the screen
   if (scrollingMouse) return;
   
   //make sure to cancel center mode
   if (centerMode)
   {
      centerMode = FALSE;
      FearGui::FGUniversalButton *ctrl = dynamic_cast<FearGui::FGUniversalButton*>(findRootControlWithTag(IDCTG_CENTER_MODE));
      if (ctrl) ctrl->setMode(FALSE);
   }
   
   if (zoomMode) {
      zoomIn();
      return;
   }
   
   if (mbZooming) return;
   
   Point2I clickPt = globalToLocalCoord(event.ptMouse);

   //find my rep
   PlayerManager::ClientRep *me = cg.playerManager->findClient(manager->getId());
   if (! me) return;
   
   if(mouseMode == SelectingMode)
   {
      //if the shift key wasn't down, we deselect the previous players if a player was clicked or dragged
      if (!(event.modifier & SI_SHIFT)) deselectUnits = TRUE;
      
      bool selectOneOnly = TRUE;
      bool oneAlreadySelected = FALSE;
      switch (selectType)
      {
         case DataBlockManager::PlayerDataType:
            selectOneOnly = FALSE;
            break;
            
         case DataBlockManager::TurretDataType:
            oneAlreadySelected = (getSelectedTurret() >= 0);
            break;
            
         case DataBlockManager::StaticShapeDataType:
            oneAlreadySelected = (getSelectedObject() >= 0);
            break;
      }
      
      int j;
      for(j = 0; j < MaxNumTargs; j++)
      {
          if(targs[j].datId == -1 || targs[j].team != me->team)
            continue;
            
         //get the data block
   		GameBase::GameBaseData* data = dynamic_cast<GameBase::GameBaseData*>
   			(cg.dbm->lookupDataBlock(targs[j].datId,targs[j].datGroup));
         if (! data || (data->mapFilter != -1 && !isFilterOn(data->mapFilter)))
         	continue;
         
         if ((targs[j].datGroup == selectType) ||
            (selectType == DataBlockManager::StaticShapeDataType && targs[j].datGroup == DataBlockManager::SensorDataType))
         {
         
            //first see if we clicked on a unit
            Point2I pos = targs[j].pos;
            pos.x = (pos.x * extent.x) >> 10;
            pos.y = extent.y - ((pos.y * extent.y) >> 10);

            if(abs(clickPt.x - pos.x) <= 3 && abs(clickPt.y - pos.y) <= 3)
            {
               
               if (targs[j].team == me->team)
               {
                  //deselect all the previously selected targets of a different type
                  for (int m = 0; m < MaxNumTargs; m++)
                  {
                     if (selectType == DataBlockManager::StaticShapeDataType)
                     {
                        if (targs[m].datGroup != DataBlockManager::StaticShapeDataType &&
                              targs[m].datGroup != DataBlockManager::SensorDataType)
                        {
                           selectTarget(m, FALSE);
                        }
                     }
                     else if (targs[m].datGroup != selectType) selectTarget(m, FALSE);
                  }
                  
                  //select the target we clicked on
                  if (event.modifier & SI_SHIFT)
                  {
                     if ((! selectOneOnly) || (! oneAlreadySelected))
                     {
                        selectTarget(j, !targs[j].selected);
                     }
                  }
                  else
                  {
                     //deselect all the previously selected targets of the same type
                     for (int m = 0; m < MaxNumTargs; m++)
                     {
                        if (selectType == DataBlockManager::StaticShapeDataType)
                        {
                           if (targs[m].datGroup == DataBlockManager::StaticShapeDataType ||
                                 targs[m].datGroup == DataBlockManager::SensorDataType)
                           {
                              selectTarget(m, FALSE);
                           }
                        }
                        else if (targs[m].datGroup == selectType) selectTarget(m, FALSE);
                     }
                        
                     //set the flag
                     deselectUnits = FALSE;
                        
                     //select the target you just selected...
                     selectTarget(j, TRUE);
                     
                     //and break since only one target should be selected on a mouse down
                     break;
                  }
               }
            }
         }
      }
      mouseDownPoint = clickPt;
      curMousePoint = clickPt;
      draggingMouse = true;
      
      //clear the command panel
      FGCurrentCommand *ctrl = dynamic_cast<FGCurrentCommand*>(root->getContentControl()->findControlWithTag(IDCTG_CURRENT_COMMAND));
      if (ctrl) ctrl->clear();
   }
   else
   {
      // we're in waypoint mode:
      // loop through all the visible targets and find the nearest one.
      int targIdx = -1;
      float targDist = 4; // gotta be within 4 pixels of a thing to select it.

      int i;
      for(i = 0; i < MaxNumTargs; i++)
      {
         if(targs[i].datId == -1)
            continue;
         Point2I pos = targs[i].pos;
         Point2F dist;
         dist.x = ((pos.x * extent.x) >> 10) - clickPt.x;
         dist.y = extent.y - ((pos.y * extent.y) >> 10) - clickPt.y;

         float len = dist.len();
         if(len < targDist)
         {
            targIdx = i;
            targDist = len;
         }
      }
      if(targIdx == -1)
      {
         Point2I wPos((clickPt.x * 1023) / extent.x, ((extent.y - clickPt.y) * 1023) / extent.y);
         Console->setVariable("CommandTarget", "waypoint");
         setCommandWaypoint(wPos);
      }
      else
      {
         SensorTargetInfo *ti = &targs[targIdx];
         if(targIdx < 128)
         {
            PlayerManager::ClientRep *cl = cg.playerManager->findClient(targIdx + 2048);
            if(cl)
               Console->setVariable("CommandTargetName", cl->name);
            else
               Console->setVariable("CommandTargetName", "");
         }
         else
         {
            if(ti->name && ti->name[0])
               Console->setVariable("CommandTargetName", ti->name);
            else
               Console->setVariable("CommandTargetName", "");
         }
         Point2I  * wPos = NULL;
         if(ti->team != me->team)
         {
            switch(ti->datGroup)
            {
               case DataBlockManager::StaticShapeDataType:
                  Console->setVariable("CommandTarget", "enemy static");
                  break;
               case DataBlockManager::PlayerDataType:
                  Console->setVariable("CommandTarget", "enemy player");
                  break;
               case DataBlockManager::FlierDataType:
                  Console->setVariable("CommandTarget", "enemy vehicle");
                  break;
               case DataBlockManager::SensorDataType:
                  Console->setVariable("CommandTarget", "enemy sensor");
                  break;
               case DataBlockManager::TurretDataType:
                  Console->setVariable("CommandTarget", "enemy turret");
                  break;
               default:
                  Console->setVariable("CommandTarget", "waypoint");
                  wPos = & targs[targIdx].pos;
                  break;
            }
         }
         else
         {
            switch(ti->datGroup)
            {
               case DataBlockManager::StaticShapeDataType:
                  Console->setVariable("CommandTarget", "friendly static");
                  break;
               case DataBlockManager::PlayerDataType:
                  Console->setVariable("CommandTarget", "friendly player");
                  break;
               case DataBlockManager::FlierDataType:
                  Console->setVariable("CommandTarget", "friendly vehicle");
                  break;
               case DataBlockManager::SensorDataType:
                  Console->setVariable("CommandTarget", "friendly sensor");
                  break;
               case DataBlockManager::TurretDataType:
                  Console->setVariable("CommandTarget", "friendly turret");
                  break;
               default:
                  Console->setVariable("CommandTarget", "waypoint");
                  wPos = & targs[targIdx].pos;
                  break;
            }
         }
         
         if( wPos != NULL )
            setCommandWaypoint( * wPos );
         else
            setCommandWaypointItem(targIdx);
      }
   }
   mouseLock();
}

void TSCommander::setMouseMode(int mode)
{
   mouseMode = mode;
   if (mode == SelectingMode)
   {
      if (root) root->setCursor(IDBMP_CURSOR_DEFAULT);
   }
   else if (mode == WaypointMode)
   {
      if (root) root->setCursor(IDBMP_CURSOR_WAYPOINT_WAIT);
   }
}

void TSCommander::setCommandWaypoint(const Point2I &cmdPoint, int targIdx )
{

   //find my rep
   PlayerManager::ClientRep *me = cg.playerManager->findBaseRep(manager->getId());
   if (! me) return;
   
   int tempCommand = curCommand;
   const char *tempCommandString = curCommandString;

   if(curCommandString && *curCommandString == '*')
   {
      execStarredCommand = true;
      Console->executef(2, "Commander::StarCommand", curCommandString);
      execStarredCommand = false;
   }
   char cmdBuf[1024];
   char *curBuf = cmdBuf;

   const char *argv[MaxNumTargs + 7];
   int numArgs;
   argv[0] = "remoteEval";
   argv[1] = "2048";
   argv[2] = targIdx == -1 ? "IssueCommand" : "IssueTargCommand";

   argv[3] = curBuf;
   curBuf += sprintf(curBuf, "%d", curCommand) + 1;
   argv[4] = curCommandString;

   if(targIdx == -1)
   {
      numArgs = 7;
      argv[5] = curBuf;
      curBuf += sprintf(curBuf, "%d", cmdPoint.x) + 1;
      argv[6] = curBuf;
      curBuf += sprintf(curBuf, "%d", cmdPoint.y) + 1;
   }
   else
   {
      numArgs = 6;
      argv[5] = curBuf;
      curBuf += sprintf(curBuf, "%d", targIdx) + 1;
   }
   for (int j = 0; j < MaxNumTargs; j++)
   {
      if (targs[j].selected && targs[j].datGroup == DataBlockManager::PlayerDataType &&
                                                      targs[j].team == me->team)
      {
         argv[numArgs++] = curBuf;
         curBuf += sprintf(curBuf, "%d", targs[j].playerId) + 1;
      }
   }
   Console->execute(numArgs, argv);
   curCommandString = tempCommandString;
   curCommand = tempCommand;
}

void TSCommander::setCommandWaypointItem(int index)
{
   //make sure we have a valid target
   if (targs[index].datGroup == -1)
      return;
   
   setCommandWaypoint(Point2I(0,0), index);
   //setMouseMode(SelectingMode);
}

void TSCommander::issueCommand(int command, const char *message)
{
   if(execStarredCommand)
   {
      curCommand = command;
      curCommandString = message;
      return;
   }
   //find my rep
   PlayerManager::ClientRep *me = cg.playerManager->findClient(manager->getId());
   if (! me) return;
   
   SimGui::SimpleText *textCtrl = (SimGui::SimpleText*)(findRootControlWithTag(IDCTG_CURRENT_COMMAND));
   if(!message)
      message = "";
   
   //remove the "~w option from the end"
   char buf[256];
   strncpy(buf, message, 255);
   buf[255] = '\0';
   char *bufPtr = strchr(buf, '~');
   if (bufPtr) *bufPtr = '\0';
   textCtrl->setText(buf);

   if(command == -1)
   {
      setMouseMode(SelectingMode);
      return;
   }
   int selCount = 0;
   
   for (int j = 0; j < MaxNumTargs; j++)
   {
      if (targs[j].selected && targs[j].datGroup == DataBlockManager::PlayerDataType &&
                                                      targs[j].team == me->team)
      {
         selCount++;
      }
   }
         
   if(selCount)
   {
      setMouseMode(WaypointMode);
      curCommand = command;
      curCommandString = message;
      if (root)
         root->setCursor(IDBMP_CURSOR_WAYPOINT_WAIT);
   }
   else
      setMouseMode(SelectingMode);
}

void TSCommander::onMouseDragged(const SimGui::Event &event)
{
   if (inZoomMode() || mbZooming) return;
   curMousePoint = globalToLocalCoord(event.ptMouse);
}

int TSCommander::getSelectedTurret(void)
{
   //make sure turrets are supposed to be selected
   if (selectType != DataBlockManager::TurretDataType)
      return -1;
   
   for (int i = 0; i < MaxNumTargs; i++)
   {
      if (targs[i].datGroup == DataBlockManager::TurretDataType)
      {
         if (targs[i].selected) return i - 128;
      }
   }
   return -1;
}

void TSCommander::setSelectedTurret(int index)
{
   //deselect all the previous targets
   for (int i = 0; i < MaxNumTargs; i++)
   {
      if (targs[i].datGroup == DataBlockManager::TurretDataType)
         selectTarget(i, FALSE);
   }
   if (index < 0) return;
   
   //make sure turrets are supposed to be selected
   if (selectType != DataBlockManager::TurretDataType)
      return;
   
   //select the turret
   if (targs[index].datGroup == DataBlockManager::TurretDataType)
   {
      selectTarget(index, TRUE);
   }
}

int TSCommander::getSelectedObject(void)
{
   //make sure static objects are supposed to be selected
   if (selectType != DataBlockManager::StaticShapeDataType) return -1;
   
   for (int i = 0; i < MaxNumTargs; i++)
   {
      if ((targs[i].datGroup == DataBlockManager::StaticShapeDataType) ||
            (targs[i].datGroup == DataBlockManager::SensorDataType))
      {
         if (targs[i].selected) return i - 128;
      }
   }
   return -1;
}

int TSCommander::getSelectedPlayer(void)
{
   //make sure players are supposed to be selected
   if (selectType != DataBlockManager::PlayerDataType) return - 1;
   
   for (int i = 0; i < MaxNumTargs; i++)
   {
      if (targs[i].datGroup == DataBlockManager::PlayerDataType && targs[i].selected)
      {
         return i;
      }
   }
   return -1;
}

void TSCommander::setSelectedObject(int index)
{
   //deselect all the previous targets
   for (int i = 0; i < MaxNumTargs; i++)
   {
      if ((targs[i].datGroup == DataBlockManager::StaticShapeDataType) ||
            (targs[i].datGroup == DataBlockManager::SensorDataType))
      {
         selectTarget(i, FALSE);
      }
   }
   if (index < 0) return;
   
   //make sure static objects are supposed to be selected
   if (selectType != DataBlockManager::StaticShapeDataType) return;
   
   //select the static object
   if ((targs[index].datGroup == DataBlockManager::StaticShapeDataType) ||
         (targs[index].datGroup == DataBlockManager::SensorDataType))
   {
      selectTarget(index, TRUE);
   }
}

void TSCommander::setSelectedPlayer(int id, bool value)
{
   //deselect all the previous targets
   if (id < 0)
   {
      for (int i = 0; i < MaxNumTargs; i++)
      {
         if (targs[i].datGroup == DataBlockManager::PlayerDataType) selectTarget(i, FALSE);
      }
      return;
   }
   
   //make sure players are supposed to be selected
   if (selectType != DataBlockManager::PlayerDataType) return;
   
   //find the target that is a player, and has the given id
   int index = -1;
   for (int i = 0; i < MaxNumTargs; i++)
   {
      if (targs[i].datGroup == DataBlockManager::PlayerDataType && targs[i].playerId == id)
      {
         index = i;
         break;
      }
   }
   
   if (index >= 0)
   {
      //select the player
      if (targs[index].datGroup == DataBlockManager::PlayerDataType)
      {
         //since this is a player, set the observer camera to view this player
         if (value)
         {
            char buf[8];
            sprintf(buf, "%d", targs[index].playerId); 
            Console->executef(2, "Client::cmdObservePlayer", buf);
         }
         selectTarget(index, value);
      }
   }
}

int TSCommander::getPlayerDamage(int playerID)
{
   for (int i = 0; i < MaxNumTargs; i++)
   {
      if (targs[i].datGroup == DataBlockManager::PlayerDataType && targs[i].playerId == playerID)
      {
         return targs[i].damageStatus;
      }
   }
   return 0;
}

int TSCommander::getPlayerStatus(int playerID)
{
   for (int i = 0; i < MaxNumTargs; i++)
   {
      if (targs[i].datGroup == DataBlockManager::PlayerDataType && targs[i].playerId == playerID)
      {
         return targs[i].activeStatus;
      }
   }
   return 0;
}

int TSCommander::getPlayerTargIndex(int playerID)
{
   for (int i = 0; i < MaxNumTargs; i++)
   {
      if (targs[i].datGroup == DataBlockManager::PlayerDataType && targs[i].playerId == playerID)
      {
         return i;
      }
   }
   return -1;
}

int TSCommander::getTargetDamage(int index)
{
   if (targs[index].datId != -1)
   {
      return targs[index].damageStatus;
   }
   return 0;
}

int TSCommander::getTargetStatus(int index)
{
   if (targs[index].datId != -1)
   {
      return targs[index].activeStatus;
   }
   return 0;
}

void TSCommander::selectTarget(int index, bool value)
{
   ChatMenu *cmenu = (ChatMenu *) manager->findObject("CommandChatMenu");
   if(cmenu && !value)
      cmenu->setMode(0);

   //set the flag    
   targs[index].selected = value;
                     
   //mirror the selected flag in the player manager
   if (targs[index].datGroup == DataBlockManager::PlayerDataType)
   {
      if(value)
      {
         if(cmenu)
            cmenu->setMode(1);
      }
      PlayerManager::ClientRep *plyr = cg.playerManager->findBaseRep(targs[index].playerId);
      if (plyr) plyr->selected = targs[index].selected;
   }
}

void TSCommander::onMouseUp(const SimGui::Event &event)
{
   event;
   draggingMouse = false;
   mouseUnlock();

   if (inZoomMode() || mbZooming) return;
   
   if (mouseMode == SelectingMode)
   {
      Box2I bnds(mouseDownPoint, curMousePoint);
      
      //find my rep
      PlayerManager::ClientRep *me = cg.playerManager->findClient(manager->getId());
      if (! me) return;
   
      bool selectOneOnly = TRUE;
      bool oneAlreadySelected = FALSE;
      switch (selectType)
      {
         case DataBlockManager::PlayerDataType:
            selectOneOnly = FALSE;
            break;
            
         case DataBlockManager::TurretDataType:
            oneAlreadySelected = (getSelectedTurret() >= 0);
            break;
            
         case DataBlockManager::StaticShapeDataType:
            oneAlreadySelected = (getSelectedObject() >= 0);
            break;
      }
      
      int j;
      for(j = 0; j < MaxNumTargs; j++)
      {
         if (targs[j].datId == -1 || targs[j].team != me->team)
            continue;
            
         if ((targs[j].datGroup == selectType) ||
            (selectType == DataBlockManager::StaticShapeDataType && targs[j].datGroup == DataBlockManager::SensorDataType))
         {
            Point2I pos = targs[j].pos;
            pos.x = (pos.x * extent.x) >> 10;
            pos.y = extent.y - ((pos.y * extent.y) >> 10);

            if(bnds.contains(pos))
            {
               //if we selected something and the deselect flag is still set...
               //deselect the units previously selected
               if (deselectUnits)
               {
                  //now deselect previous targets
                  for (int l = 0; l < MaxNumTargs; l++)
                  {
                     selectTarget(l, FALSE);
                  }
                     
                  //set the flags
                  deselectUnits = FALSE;
                  oneAlreadySelected = FALSE;
               }
               
               //now select the target
               if ((! selectOneOnly) || (! oneAlreadySelected))
               {
                  selectTarget(j, TRUE);
               }
               
               if (selectOneOnly) break;
            }
         }
      }
   }
   setMouseMode(SelectingMode);
   
   //make sure the deselection flag was cleard
   deselectUnits = FALSE;
}

void TSCommander::onRightMouseDown(const SimGui::Event &event)
{
   //do nothing if we're dragging a selections rectangle
   if (draggingMouse) return;
   
   //see if we need to prevent screen dragging ie.  we're still zooming, or about to
   if (zoomMode || mbZooming)
   {
      mbPreventDragging = TRUE;
   }
   
   //make sure to cance center mode
   if (centerMode)
   {
      centerMode = FALSE;
      FearGui::FGUniversalButton *ctrl = dynamic_cast<FearGui::FGUniversalButton*>(findRootControlWithTag(IDCTG_CENTER_MODE));
      if (ctrl) ctrl->setMode(FALSE);
   }
   
   if (zoomMode) {
      zoomOut();
      return;
   }

   //wait until we've stopped moving before trying to drag
   if (mbZooming) return;
   
   //set the bool
   scrollingMouse = TRUE;
   
   mouseDownPoint = event.ptMouse;
   origPosition = position;
   
   //lock the mouse
   mouseLock();
   
   Parent::onRightMouseDown(event);
}

void TSCommander::onRightMouseDragged(const SimGui::Event &event)
{
   if (zoomMode) return;
   if (centerMode) return;
   
   //dragging the screen, not a selection rectangle
   if (mbPreventDragging) return;
   
   curMousePoint = event.ptMouse;
   position.x = origPosition.x + (curMousePoint.x - mouseDownPoint.x);
   position.y = origPosition.y + (curMousePoint.y - mouseDownPoint.y);
   
   //make sure the new position is within prescribed boundarys
   position.x = max(position.x, -abs(parent->extent.x - extent.x) - (parent->extent.x / 2));
   position.x = min(position.x, parent->extent.x / 2);
   position.y = max(position.y, -abs(parent->extent.y - extent.y) - (parent->extent.y / 2));
   position.y = min(position.y, parent->extent.y / 2);
   
   Parent::onRightMouseDragged(event);
}

void TSCommander::onRightMouseUp(const SimGui::Event &event)
{
   //screen dragging, not the selections rect
   mbPreventDragging = FALSE;
   
   //unlock the mouse
   mouseUnlock();
   
   scrollingMouse = false;
   Parent::onRightMouseUp(event);
}


void TSCommander::getNewCenterModePos(Point2I &newPos, const Point2I &newExt)
{
   PlayerManager::ClientRep *me = playerManager->findClient(manager->getId());
   
   //calculate the new target center  
   int count = 0;
   Point2I center(0, 0);
   for (int i = 0; i < MaxNumTargs; i++)
   if (targs[i].selected && targs[i].team == me->team)
   {
      //get the data block
		GameBase::GameBaseData* data = dynamic_cast<GameBase::GameBaseData*>
			(cg.dbm->lookupDataBlock(targs[i].datId,targs[i].datGroup));
         
      //make sure the map filter is on for the centered item(s)
      if (! data || (data->mapFilter != -1 && !isFilterOn(data->mapFilter)))
      	continue;
         
      center.x += targs[i].pos.x;
      center.y += targs[i].pos.y;
      count++;
   }
   
   newPos = position;
   if (count)
   {
      //find the average center
      center.x = center.x / count;
      center.y = center.y / count;
      
      //convert to gui coords
      center.x = (center.x * newExt.x) >> 10;
      center.y = (center.y * newExt.y) >> 10;

      center.y = newExt.y - center.y;
      
      //set the position according to the new center position
      newPos = parent->extent;
      newPos >>= 1;
      newPos -= center;
      
      //make sure the new position is within prescribed boundarys
      newPos.x = max(newPos.x, -abs(parent->extent.x - newExt.x) - (parent->extent.x / 2));
      newPos.x = min(newPos.x, parent->extent.x / 2);
      newPos.y = max(newPos.y, -abs(parent->extent.y - newExt.y) - (parent->extent.y / 2));
      newPos.y = min(newPos.y, parent->extent.y / 2);
   }
}

void TSCommander::setCenterMode(bool on_off)
{
   if (on_off == FALSE)
   {
      centerMode = FALSE;
   }
   else
   {
      centerMode = TRUE;
      startCenterTime = GetTickCount();
      startCenterPosition = position;
    
      Point2I newPos;
      getNewCenterModePos(newPos, extent);
      targetCenterPosition = newPos;
   }
}

void TSCommander::setCenterMode(Point2I &newPos)
{
   startCenterTime = GetTickCount();
   startCenterPosition = position;
   targetCenterPosition = newPos;
}

void TSCommander::centerView(void)
{
   Point2I newPos;
   getNewCenterModePos(newPos, extent);
   
   //see if the target has changed
   if (newPos != targetCenterPosition)
   {
      setCenterMode(newPos);
   }
   
   //see if we're past the centering time
   DWORD curTime = GetTickCount();
   if (curTime >= startCenterTime + mZoomTransitionTime)
   {
      newPos = targetCenterPosition;
   }
   else
   {
      float centerPercent = float(curTime - startCenterTime) / float(mZoomTransitionTime);
      newPos.x = startCenterPosition.x + int(float(targetCenterPosition.x - startCenterPosition.x) * centerPercent);
      newPos.y = startCenterPosition.y + int(float(targetCenterPosition.y - startCenterPosition.y) * centerPercent);
   }
 
   //now set the new position  
   resize(newPos, extent);
}

void TSCommander::parentResized(const Point2I &, const Point2I &newParentExtent)
{
   int maxDim = max(newParentExtent.x, newParentExtent.y);
   resize(Point2I(0, 0), Point2I(maxDim, maxDim));
   worldScale.x = extent.x / worldExtent.x;
   worldScale.y = extent.y / worldExtent.y;
}

void TSCommander::zoomIn()
{
   if (zoomLevel > 3) return;

   zoomRect r;
   //if we're already zooming, push the target, not the current
   if (mbZooming)
   {
      r.pos = targetZoomPosition;
      r.ext = targetZoomExtent;
   }
   else
   {
      r.pos = position;
      r.ext = extent;
   }
   zoomLevel++;

   //find the zoom center
   Point2I cursorPos = root->getCursorPos();
   Point2I zoomCenter, clickPoint;
   if(root->getContentControl()->findHitControl(cursorPos) == this)
   {
      clickPoint = parent->globalToLocalCoord(cursorPos);
   }
   else
   {
      clickPoint = parent->extent;
      clickPoint >>= 1;
   }
   zoomCenter = clickPoint - r.pos;

   Point2I newExt = r.ext;
   newExt <<= 1;
   zoomCenter <<= 1;

   Point2I newPos;
   if (! centerMode)
   {
      newPos = clickPoint;
      newPos -= zoomCenter;
   }
   else
   {
      getNewCenterModePos(newPos, newExt);
      setCenterMode(newPos);
   }
   
   //make sure the new position is within prescribed boundarys
   newPos.x = max(newPos.x, -abs(parent->extent.x - newExt.x) - (parent->extent.x / 2));
   newPos.x = min(newPos.x, parent->extent.x / 2);
   newPos.y = max(newPos.y, -abs(parent->extent.y - newExt.y) - (parent->extent.y / 2));
   newPos.y = min(newPos.y, parent->extent.y / 2);

   //set the start and target zoom
   mbZooming = TRUE;
   startZoomTime = GetTickCount();
   startZoomPosition = position;
   startZoomExtent = extent;
   targetZoomPosition = newPos;
   targetZoomExtent = newExt;
   
   //the following actually do the zoom, and have been moved to the preRender() method
   /*
   resize(newPos, newExt);
   worldScale.x = extent.x / worldExtent.x;
   worldScale.y = extent.y / worldExtent.y;
   */
}

void TSCommander::zoomOut()
{
   if (zoomLevel <= 0)
   {
      //adjust the scale to fit the parent
      if (parent)
      {
         int maxExtent = min(parent->extent.x, parent->extent.y);
         float adjust = float(maxExtent) / max(extent.x, extent.y);
         extent.x = int(adjust * float(extent.x));
         extent.y = int(adjust * float(extent.y));
         
         if (extent.x < parent->extent.x) position.x = (parent->extent.x - extent.x) / 2;
         if (extent.y < parent->extent.y) position.y = (parent->extent.y - extent.y) / 2;
         
         //reset the world scale
         worldScale.x = extent.x / worldExtent.x;
         worldScale.y = extent.y / worldExtent.y;
      }
      return;
   }
   
   zoomLevel--;
   zoomRect r;
   //if we're already zooming
   if (mbZooming)
   {
      r.pos = targetZoomPosition;
      r.ext = targetZoomExtent;
   }
   else
   {
      r.pos = position;
      r.ext = extent;
   }
   //find the zoom center
   Point2I cursorPos = root->getCursorPos();
   Point2I zoomCenter, clickPoint;
   if(root->getContentControl()->findHitControl(cursorPos) == this)
   {
      clickPoint = parent->globalToLocalCoord(cursorPos);
   }
   else
   {
      clickPoint = parent->extent;
      clickPoint >>= 1;
   }
   zoomCenter = clickPoint - r.pos;

   //find the new pos and extent
   Point2I newExt = r.ext;
   newExt >>= 1;
   zoomCenter >>= 1;

   Point2I newPos;
   if (! centerMode)
   {
      newPos = clickPoint;
      newPos -= zoomCenter;
   }
   else
   {
      getNewCenterModePos(newPos, newExt);
      setCenterMode(newPos);
   }

   //make sure the new position is within prescribed boundarys
   newPos.x = max(newPos.x, -abs(parent->extent.x - newExt.x) - (parent->extent.x / 2));
   newPos.x = min(newPos.x, parent->extent.x / 2);
   newPos.y = max(newPos.y, -abs(parent->extent.y - newExt.y) - (parent->extent.y / 2));
   newPos.y = min(newPos.y, parent->extent.y / 2);

   //set the start and target zoom
   mbZooming = TRUE;
   startZoomTime = GetTickCount();
   startZoomPosition = position;
   startZoomExtent = extent;
   targetZoomPosition = newPos;
   targetZoomExtent = newExt;
   
   //the following actually do the zoom, and have been moved to the preRender() method
   /*
   resize(r.pos, r.ext);
   worldScale.x = extent.x / worldExtent.x;
   worldScale.y = extent.y / worldExtent.y;
   */
}

};