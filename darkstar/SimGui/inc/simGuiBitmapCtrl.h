//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMGUIBITMAPCTRL_H_
#define _SIMGUIBITMAPCTRL_H_

#include "simGuiActiveCtrl.h"
#include "chunkedBitmap.h"
#include "inspect.h"

namespace SimGui
{

// The SimGui::BitmapCtrl fills its bounding rect with a GFXBitmap.
// If the GFXBitmap is too large to fit on hardware, the SimGui::BitmapCtrl
// control will first dice up the GFXBitmap into a ChunkedBitmap
class BitmapCtrl : public ActiveCtrl
{
private:
   typedef ActiveCtrl Parent;

   ChunkedBitmap *cBmp;

   Int32 bmpTag;
   bool transparent;

   bool flushTextureCache;

protected:
   Resource<GFXBitmap> hBmp;  

public:   
   BitmapCtrl();
   ~BitmapCtrl();
   
   void setBitmap();
   void setBitmap(const char *name);
   void setBitmap(Resource<GFXBitmap> bmp);
   void setTransparent(bool tf);
   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);

   bool onAdd();
   void inspectRead(Inspect *insp);
   void inspectWrite(Inspect *insp);

   DECLARE_PERSISTENT(BitmapCtrl);
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
};

};

#endif //_SIMGUIBITMAPCTRL_H_
