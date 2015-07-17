//------------------------------------------------------------------------------
// Description 
//    Simple progress control
//    
// $Workfile$ SimGuiProgressCtrl.cpp
// $Revision$ 1.0
// $Author  $ Robert Mobbs (robert.mobbs@dynamix.com)
// $Modtime $ 5/15/98
//------------------------------------------------------------------------------

#include <g_surfac.h>
#include <inspect.h>
#include <simguiprogressctrl.h>
#include <editor.strings.h>

namespace SimGui
{
   ProgressCtrl::ProgressCtrl() :
      iPos(50),
      iRangeHi(100),
      iRangeLo(0),
      iIncrement(1)
   {
   }

   void ProgressCtrl::setStep(int iIncrement)
   {
      AssertFatal(iIncrement <= (iRangeHi - iRangeLo),
         "ProgressCtrl::setStep: invalid incremental value");

      this->iIncrement = iIncrement;
   }

   int ProgressCtrl::offsetPos(int iOffset)
   {
      int iOldPos = iPos;

      iPos += iOffset;

      if (iPos > iRangeHi || iPos < iRangeLo)
      {
         iPos = iOldPos;
      }

      // Update the progress bar
      setUpdate();

      // Return old position
      return (iOldPos);
   }

   void ProgressCtrl::setRange(int iRangeLo, int iRangeHi)
   {
      AssertFatal(iRangeHi < PROGRESSCTRL_MAX_RANGE,
         "ProgressCtrl::setRange: specified range is too large");

      AssertFatal(iRangeLo > PROGRESSCTRL_MIN_RANGE,
         "ProgressCtrl::setRange: specified range is too low");

      this->iRangeLo = iRangeLo;
      this->iRangeHi = iRangeHi;
   }

   int ProgressCtrl::stepIt()
   {
      int iOldPos = iPos;

      iPos += iIncrement;

      if (iPos >= iRangeHi || iPos <= iRangeLo)
      {
         iPos = iOldPos;
      }

      if (iPos != iOldPos)
      {
         // Update the progress bar
         setUpdate();
      }

      // Return old position
      return (iOldPos);
   }
   
   void ProgressCtrl::setScriptValue(const char *value)
   {
      setPercentDone(atof(value));
   }
   
   void ProgressCtrl::setPercentDone(float percentDone)
   {
      if (percentDone <= 0.0f) iPos = iRangeLo;
      else if (percentDone >= 1.0f) iPos = iRangeHi;
      else iPos = iRangeLo + int(percentDone * float(iRangeHi - iRangeLo));
      setUpdate();
   }

   void ProgressCtrl::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
   {
      // Get our current pos in pixels
      float iPelsPerStep = float(extent.x) / float(iRangeHi - iRangeLo);
      Int32 iPosInPels   = int(iPelsPerStep * float(iPos));

      // Create the current rectangle
      RectI outline (offset.x, offset.y, offset.x + extent.x - 1,   offset.y + extent.y - 1);
      RectI progress(offset.x, offset.y, offset.x + iPosInPels, offset.y + extent.y - 1);

      // Draw the rectangles -- outline and progress
      sfc->drawRect2d_f(&progress, fillColor);
      sfc->drawRect2d(&outline,  boarderColor);
   }

   void ProgressCtrl::inspectWrite(Inspect *inspect)
   {
      Parent::inspectWrite(inspect);

      inspect->write(IDITG_PC_RANGELO,   (Int32)iRangeLo);
      inspect->write(IDITG_PC_RANGEHI,   (Int32)iRangeHi);
      inspect->write(IDITG_PC_INCREMENT, (Int32)iIncrement);
   }   

   void ProgressCtrl::inspectRead(Inspect *inspect)
   {
      int iLo, iHi, iInc;

      Parent::inspectRead(inspect);

      inspect->read(IDITG_PC_RANGELO,   (Int32 &)iLo);
      inspect->read(IDITG_PC_RANGEHI,   (Int32 &)iHi);
      inspect->read(IDITG_PC_INCREMENT, (Int32 &)iInc);

      if (iLo != iRangeLo || iHi != iRangeHi)
      {
         setRange(iLo, iHi);
      }

      if (iInc != iIncrement)
      {
         setStep(iInc);
      }
   }   

   Persistent::Base::Error ProgressCtrl::write(StreamIO &sio, int iVersion, int iUser)
   {
      sio.write(iRangeLo);
      sio.write(iRangeHi);
      sio.write(iIncrement);

      return Parent::write(sio, iVersion, iUser);
   }

   Persistent::Base::Error ProgressCtrl::read (StreamIO &sio, int iVersion, int iUser)
   {
      sio.read(&iRangeLo);
      sio.read(&iRangeHi);
      sio.read(&iIncrement);

      return Parent::read(sio, iVersion, iUser);
   }
};
