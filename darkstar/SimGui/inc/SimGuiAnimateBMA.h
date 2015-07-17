//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GUIBMAANIMATE_H_
#define _GUIBMAANIMATE_H_

#include <simGuiSimpleText.h>

namespace SimGui
{

class AnimateBMA : public ActiveCtrl
{
private:
   typedef ActiveCtrl Parent;

   Resource<GFXBitmapArray> hBma;
   SimTime lastFrameTime;
   Int32 frame;
   Int32 bmaTag;
   float timePerFrame;
   Int32 sleepBetweenLoop;
   bool animating;
   bool looping;

protected:
   bool onAdd();

public:   
   AnimateBMA();
   
   void doNotLoop();
   void startAnim();
                  
   void loadBma(Int32 tag);
   void onPreRender();
   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);

   void inspectRead(Inspect *inspector);
   void inspectWrite(Inspect *inspector);

   DECLARE_PERSISTENT(AnimateBMA);
   Persistent::Base::Error write( StreamIO &sio, int version, int user);
   Persistent::Base::Error read( StreamIO &sio, int version, int user);
};

};

#endif //_GUIBMAANIMATE_H_

