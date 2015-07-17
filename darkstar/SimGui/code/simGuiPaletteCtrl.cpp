#include <g_surfac.h>
#include <string.h>
#include <m_box.h>
#include "simGuiTestCtrl.h"
#include "simGuiPaletteCtrl.h"
#include "simTagDictionary.h"
#include "inspect.h"
#include "editor.strings.h"
#include "darkstar.strings.h"
#include "simGame.h"

namespace SimGui
{

PaletteCtrl::PaletteCtrl()
{
   palTag = IDPAL_PAL_DEFAULT;
}

void PaletteCtrl::loadPalette()
{
   hPal = SimResource::loadByTag(manager, palTag, true);
   if ((bool)hPal)
      SimGame::get()->getWorld()->setPalette(hPal);
}  
 
void PaletteCtrl::onWake()
{
   loadPalette();
}   

void PaletteCtrl::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);

   insp->read(IDITG_PAL_TAG, NULL, NULL, palTag); 
   loadPalette();
}   

void PaletteCtrl::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);
   insp->write(IDITG_PAL_TAG, true, "IDPAL_*", (Int32)palTag);
}   

Persistent::Base::Error PaletteCtrl::write( StreamIO &sio, int version, int user )
{
   Inspect_Str temp;

   sio.write(palTag);
   sio.write(Inspect::MAX_STRING_LEN + 1, temp);
   
   return Parent::write(sio, version, user);
}

Persistent::Base::Error PaletteCtrl::read( StreamIO &sio, int version, int user)
{
   Inspect_Str temp;

   sio.read(&palTag);
   sio.read(Inspect::MAX_STRING_LEN + 1, temp);

   return Parent::read(sio, version, user);
}

}; //namespace GUI