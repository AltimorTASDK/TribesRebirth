#ifndef _H_SIMGUIBITMAPBOX
#define _H_SIMGUIBITMAPBOX

#include "simGuiCtrl.h"
#include "g_barray.h"
#include "g_bitmap.h"
#include "inspect.h"
#include "inspectWindow.h"

namespace SimGui
{

class BitmapBox : public Control
{
private:
   typedef Control Parent;
   Int32 bmaTag;

   Resource<GFXBitmapArray> bma;
   GFXBitmap *upperLeft, *upperCenter, *upperRight;
   GFXBitmap *centerLeft, *center, *centerRight;
   GFXBitmap *lowerLeft, *lowerCenter, *lowerRight;
public:
   DECLARE_PERSISTENT(BitmapBox);
   
   BitmapBox();

   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);

   bool onAdd();
   void inspectRead(Inspect *);
   void inspectWrite(Inspect *);
   
   void setBitmapArray(Int32 tag);
   Persistent::Base::Error read( StreamIO &sio, int version, int user);
   Persistent::Base::Error write( StreamIO &sio, int version, int user );
};

};

#endif
