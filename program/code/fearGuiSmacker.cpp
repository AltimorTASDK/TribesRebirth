//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------



#include <g_surfac.h>
#include "fearGuiSmacker.h"
//#include "itag.strings.h"
//#include "gui.strings.h"
//#include "esCore.h"
#include "soundFx.h"

#include <std.h>
#include <signal.h>
#include "simConsolePlugin.h"
#include "gfxPlugin.h"
#include "interiorPlugin.h"
#include "TerrainPlugin.h"
#include "skyPlugin.h"
#include "netPlugin.h"
#include "soundFXPlugin.h"
#include "preferencePlugin.h"
#include "simGuiPlugin.h"
#include "simInputPlugin.h"
#include "simTreePlugin.h"
#include "MissionPlugin.h"
#include "tedPlugin.h"
#include "shellOpenPlugin.h"
#include "simShapePlugin.h"
#include "LSPlugin.h"
#include "RedBookPlugin.h"
#include "fearMissionPlugin.h"

#include "FearPlugin.h"
#include "esfPlugin.h"
#include "simTelnetPlugin.h"
#include "simWinConsolePlugin.h"
#include <feardynamicdataplugin.h>
#include "simGame.h"
#include "fearglobals.h"
#include "netPacketStream.h"
#include "FearCSDelegate.h"
#include "simCanvas.h"
#include <scriptplugin.h>
#include <fearplayerpsc.h>
#include <cpuSpeed.h>
#include "simguicanvas.h"

#include "fear.strings.h"

namespace FearGui
{

// statics
HMODULE SmackerCtrl::hSmacker = NULL;
int SmackerCtrl::smackerInstanceCount = 0;
SmackerCtrl::FN_SmackOpen       SmackerCtrl::SmackOpen = NULL;
SmackerCtrl::FN_SmackClose      SmackerCtrl::SmackClose = NULL;
SmackerCtrl::FN_SmackDoFrame    SmackerCtrl::SmackDoFrame = NULL;
SmackerCtrl::FN_SmackNextFrame  SmackerCtrl::SmackNextFrame = NULL;
SmackerCtrl::FN_SmackToBuffer   SmackerCtrl::SmackToBuffer = NULL;
SmackerCtrl::FN_SmackSoundOnOff SmackerCtrl::SmackSoundOnOff = NULL;
SmackerCtrl::FN_SmackWait       SmackerCtrl::SmackWait = NULL;
SmackerCtrl::FN_SmackUseMMX     SmackerCtrl::SmackUseMMX = NULL;
SmackerCtrl::FN_SmackColorTrans SmackerCtrl::SmackColorTrans = NULL;
SmackerCtrl::FN_SmackSoundUseDirectSound SmackerCtrl::SmackSoundUseDirectSound = NULL;
SmackerCtrl::FN_SmackVolumePan  SmackerCtrl::SmackVolumePan = NULL;
SmackerCtrl::FN_SmackGoto       SmackerCtrl::SmackGoto = NULL;
SmackerCtrl::FN_SmackSimulate   SmackerCtrl::SmackSimulate = NULL;


//------------------------------------------------------------------------------
SmackerCtrl::SmackerCtrl()
{
   smk   = NULL;
   bmp   = NULL;
   loop  = false;
   preload = false;
   stretch = false;
   palStart  = 0;
   palColors = 256;
   smkTag= 0;
   movieDoneTag = 0;
   volume = 0.0f;
   paused = false;
}   

int SmackerCtrl::smackerCtrlVersion = 1;
   
//------------------------------------------------------------------------------
void SmackerCtrl::openSmacker()
{
   if (smackerInstanceCount == 0)
   {
      hSmacker = LoadLibrary("smackW32.dll");
      if (hSmacker)
      {
         SmackOpen       = (FN_SmackOpen)GetProcAddress( hSmacker, "_SmackOpen@12");
         SmackClose      = (FN_SmackClose)GetProcAddress( hSmacker, "_SmackClose@4");
         SmackDoFrame    = (FN_SmackDoFrame)GetProcAddress( hSmacker, "_SmackDoFrame@4");
         SmackNextFrame  = (FN_SmackNextFrame)GetProcAddress( hSmacker, "_SmackNextFrame@4");
         SmackToBuffer   = (FN_SmackToBuffer)GetProcAddress( hSmacker, "_SmackToBuffer@28");
         SmackSoundOnOff = (FN_SmackSoundOnOff)GetProcAddress( hSmacker, "_SmackSoundOnOff@8");
         SmackWait       = (FN_SmackWait)GetProcAddress( hSmacker, "_SmackWait@4");
         SmackUseMMX     = (FN_SmackUseMMX)GetProcAddress( hSmacker, "_SmackUseMMX@4");
         SmackColorTrans = (FN_SmackColorTrans)GetProcAddress( hSmacker, "_SmackColorTrans@8");
         SmackSoundUseDirectSound = (FN_SmackSoundUseDirectSound)GetProcAddress( hSmacker, "_SmackSoundUseDirectSound@4");
         SmackVolumePan  = (FN_SmackVolumePan)GetProcAddress( hSmacker, "_SmackVolumePan@16"); 
         SmackGoto       = (FN_SmackGoto)GetProcAddress( hSmacker, "_SmackGoto@8"); 
         SmackSimulate   = (FN_SmackSimulate)GetProcAddress( hSmacker, "_SmackSimulate@4"); 

         if (!(SmackOpen      
               && SmackClose     
               && SmackDoFrame   
               && SmackNextFrame 
               && SmackToBuffer  
               && SmackSoundOnOff 
               && SmackWait
               && SmackUseMMX
               && SmackColorTrans
               && SmackSoundUseDirectSound
               && SmackVolumePan
               && SmackGoto
               && SmackSimulate
              ))
            closeSmacker();
         
         SmackUseMMX(2);  // if MMX is available use it
         Sfx::Manager *sfx = Sfx::Manager::find(manager);
         IDirectSound *ds  = sfx ? sfx->getDSound() : NULL;
         if (ds)
            SmackSoundUseDirectSound(ds);
      }
   }
   if (hSmacker)
      smackerInstanceCount++;
   volume = 1.0f;
}

//------------------------------------------------------------------------------
void SmackerCtrl::closeSmacker()
{
   if (smackerInstanceCount)
      smackerInstanceCount--;
   if (smackerInstanceCount == 0)
   {
      FreeLibrary(hSmacker);
      hSmacker = NULL;   
   }
}   


//------------------------------------------------------------------------------
void SmackerCtrl::update(GFXSurface *sfc)
{
   if (smk && !finished)
   {
      // smoothly adjust volume up and down, just a nice simple touch
//      if (GWMain::foreground)
//         volume = volume*0.9f + 0.1f;
//      else
//         volume = volume*0.6f;
      SmackVolumePan(smk, SMACKTRACKS, 32768*volume, 32768);

      // HANDLE Palette changes
      if (smk->NewPalette)
      {  //do palette stuff
         Resource<GFXPalette> pal = SimGame::get()->getWorld()->getPalette();
         if ((bool)pal)
         {
            char *src = (char*)&smk->Palette[0];
            PALETTEENTRY *dst = &pal->palette[0].color[palStart];
            for (int i = 0; i < palColors; i++, dst++)
            {  
               dst->peRed  = *src++;
               dst->peGreen= *src++;
               dst->peBlue = *src++;
            }
            cBmp.flush(sfc, true);
            SimGame::get()->getWorld()->setPalette(pal, -1);
         }
      }

      // HANDLE frame changes
      if (!SmackWait(smk) && !paused)
      {  // SmackWait returns 0 if it's time to advance a frame
         SmackDoFrame(smk);
         cBmp.set(bmp);
         cBmp.flush(sfc, true);
         
         //loop stuff
         // check for a non-looping movie where the smacker is at the last frame
         // or for a smacker built without a ring frame and on the second-to-last frame
         if (!loop && ((smk->FrameNum == smk->Frames) || ((!smk->SmackerType & 0x1) && (smk->FrameNum == smk->Frames - 1))))
         {
            finished = true;
            
            // call up to Delegate here
            SimMessageEvent::post(this, movieDoneTag);
            return;
         }
         SmackNextFrame(smk);
      }
   }
}   


//------------------------------------------------------------------------------
void SmackerCtrl::open(int tag)
{
   const char *filename = SimTagDictionary::getString(manager,tag);
   open(filename);
}   

//------------------------------------------------------------------------------
void SmackerCtrl::open(const char *name)
{
   if (smk)
      SmackClose(smk);

   if (hSmacker && name)
   {
      char buffer[1024];
      SimResource::get(manager)->getFullPath(name, buffer);
      smk = SmackOpen(buffer, SMACKTRACKS | SMACKNEEDVOLUME | (preload ? SMACKPRELOADALL : 0), SMACKAUTOEXTRA );
      //smk = SmackOpen(buffer, SMACKTRACKS | SMACKNEEDVOLUME | SMACKSIMULATE | (preload ? SMACKPRELOADALL : 0), SMACKAUTOEXTRA );
      if (smk)
      {
         delete bmp;

         // setup rendering into the GFXBitmap
         bmp = GFXBitmap::create(smk->Width,smk->Height);
         memset((void *)bmp->pBits, 0, bmp->imageSize);      
         cBmp.set(bmp);
         SmackToBuffer(smk, 0,0, bmp->getWidth(), bmp->getHeight(), bmp->pBits, 0);
         SmackSoundOnOff(smk, 1);   // turn sound on if present

         //SmackSimulate(300000);
         translate();
         finished = false;
      }
      else
      {
         SimMessageEvent::post(this, -1);
      }   
   }
}   

//------------------------------------------------------------------------------
void SmackerCtrl::close()
{
   if (smk)   
   {
      SmackSoundOnOff(smk, 0);   // turn sound off
      SmackClose(smk);
   }
   smk    = NULL;
   
   finished = true;
}

//------------------------------------------------------------------------------
void SmackerCtrl::translate()
{
   if (smk)
   {
      // build translation table
      for (int i=0; i<palColors; i++)
         remap[i] = i+palStart;
      for (int j=palColors; j<256; j++)
         remap[j] = j; //palStart;

      SmackColorTrans(smk, remap);
   }
}   


//------------------------------------------------------------------------------
bool SmackerCtrl::onAdd()
{
   if (!Parent::onAdd()) return (false);
   openSmacker();
   open(smkTag);
   return (true);
}  

//------------------------------------------------------------------------------ 
void SmackerCtrl::onRemove()
{
   delete bmp;
   close();
   closeSmacker();
   Parent::onRemove();
}   

void SmackerCtrl::onKeyDown(const SimGui::Event &)
{
   // call up to Delegate here
   SimMessageEvent::post(this, movieDoneTag);
}

//------------------------------------------------------------------------------
void SmackerCtrl::onMouseDown(const SimGui::Event &)
{
   // call up to Delegate here
   SimMessageEvent::post(this, movieDoneTag);
}

//------------------------------------------------------------------------------
void SmackerCtrl::onPreRender()
{
   if (!finished && !paused)
      setUpdate();
}   

//------------------------------------------------------------------------------
void SmackerCtrl::onClose()
{
   close();   
}   
   
//------------------------------------------------------------------------------
void SmackerCtrl::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   update(sfc);

   if (smk && bmp)
   {
      Box2I box;
      if (stretch)
         box(offset.x, offset.y, offset.x + extent.x, offset.y + extent.y);
      else
         box(offset.x, offset.y, offset.x + bmp->getWidth(), offset.y + bmp->getHeight());
      cBmp.drawBitmap(sfc, box);
   }

   renderChildControls(sfc, offset, updateRect);
}  

//--------------------------------------------------------------------------- 

bool SmackerCtrl::processEvent(const SimEvent *event)
{
   switch (event->type) {
      onEvent(SimMessageEvent);
   }

   return Parent::processEvent(event);
}

//--------------------------------------------------------------------------- 

bool SmackerCtrl::onSimMessageEvent(const SimMessageEvent *)
{
   // the movie has ended due to some unforseen circumstance
  
   // signal the end of the movie
   if (movieDoneTag)
   {
      Parent::onMessage(this, movieDoneTag);
   }
   else if (consoleCommand[0])
   {
      Console->evaluate(consoleCommand, FALSE);
   }
   
   return true;
}

//------------------------------------------------------------------------------
void SmackerCtrl::inspectRead(Inspect *insp)
{
   Int32 newTag;
   Parent::inspectRead(insp);
   insp->read(IDITG_SMACK_TAG, NULL, NULL, newTag);
	smkTag = newTag;
   insp->read(IDITG_LOOP, loop);
   insp->read(IDITG_MOVIE_DONE_TAG, NULL, NULL, (long &)movieDoneTag);
   insp->read(IDITG_PRELOAD, preload);
   insp->read(IDITG_STRETCH, stretch);
   insp->read(IDITG_PALSTART, (long &)palStart);
   insp->read(IDITG_PALCOLORS, (long &)palColors);
   if (palStart+palColors > 256)
   {
      palStart  = 0;
      palColors = 256;
   }

   open(smkTag);
}   

//------------------------------------------------------------------------------
void SmackerCtrl::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);
   insp->write(IDITG_SMACK_TAG, true, "IDSMK_*", (Int32)smkTag);
   insp->write(IDITG_LOOP, loop);
   insp->write(IDITG_MOVIE_DONE_TAG, true, "*", (Int32)movieDoneTag);
   insp->write(IDITG_PRELOAD, preload);
   insp->write(IDITG_STRETCH, stretch);
   insp->write(IDITG_PALSTART, (Int32)palStart);
   insp->write(IDITG_PALCOLORS, (Int32)palColors);
}   

//------------------------------------------------------------------------------
Persistent::Base::Error SmackerCtrl::write( StreamIO &sio, int version, int user )
{
   sio.write(smkTag);
   sio.write(loop);
   sio.write(movieDoneTag);   
   sio.write(preload);
   sio.write(palStart);
   sio.write(palColors);
   sio.write(stretch);

   return Parent::write(sio, version, user);
}

//------------------------------------------------------------------------------
Persistent::Base::Error SmackerCtrl::read( StreamIO &sio, int version, int user)
{
   sio.read(&smkTag);
   sio.read(&loop);
   if(version >= smackerCtrlVersion)
      sio.read(&movieDoneTag);
   sio.read(&preload);
   sio.read(&palStart);
   sio.read(&palColors);
   sio.read(&stretch);

   return Parent::read(sio, version, user);
}


IMPLEMENT_PERSISTENT_TAG(SmackerCtrl, FOURCC('F','G','s','m'));

}; //namespace GUI
