#include "g_bitmap.h"
#include "g_surfac.h"
#include "fear.strings.h"
#include "FearHudMatrixCtrl.h"

namespace FearGui
{

FGHudMatrix::FGHudMatrix(void)
{
   mLowRes = FALSE;
   position.set(0, 0);
   extent.set(100, 50);
   
   loResPosition.set(0, 0);
   loResExtent.set(100, 50);
}

void FGHudMatrix::onWake(void)
{
   hiResPosition = position;
   hiResExtent = extent;
}

void FGHudMatrix::setLowRes(bool t_or_f)
{
   mLowRes = t_or_f;
   
   if (mLowRes)
   {
      position = loResPosition;
      extent = loResExtent;
   }
   else
   {
      position = hiResPosition;
      extent = hiResExtent;
   }
}

void FGHudMatrix::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);
   insp->write( IDITG_LR_POSITION,      loResPosition );
   insp->write( IDITG_LR_EXTENT,        loResExtent );
}   

void FGHudMatrix::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   insp->read( IDITG_LR_POSITION,      loResPosition);  
   insp->read( IDITG_LR_EXTENT,        loResExtent);  
}

Persistent::Base::Error FGHudMatrix::write( StreamIO &sio, int version, int user )
{
   loResPosition.write(sio);
   loResExtent.write(sio);
   return Parent::write(sio, version, user);
}

Persistent::Base::Error FGHudMatrix::read( StreamIO &sio, int version, int user)
{
   loResPosition.read(sio);
   loResExtent.read(sio);
   return Parent::read(sio, version, user);
}

IMPLEMENT_PERSISTENT_TAG( FearGui::FGHudMatrix,   FOURCC('F','G','m','x') );

};