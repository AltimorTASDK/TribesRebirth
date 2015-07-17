//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMGUIPALETTECTRL_H_
#define _SIMGUIPALETTECTRL_H_

#include "simGuiCtrl.h"
#include "chunkedBitmap.h"
#include "inspect.h"

namespace SimGui
{

class PaletteCtrl : public Control
{
private:
   typedef Control Parent;

protected:
   Resource<GFXPalette> hPal;  

   Int32 palTag;
   virtual void loadPalette();

public:   
   PaletteCtrl();
   
   void inspectRead(Inspect *);
   void inspectWrite(Inspect *);
   
   void onWake();

   DECLARE_PERSISTENT(PaletteCtrl);
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
};

};

#endif //_SIMGUIPALETTECTRL_H_
