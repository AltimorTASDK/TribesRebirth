//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <string.h>
#include <g_surfac.h>
#include "g_bitmap.h"
#include "fear.strings.h"
#include "FearGuiPage.h"

namespace FearGui
{

bool FearGuiPage::onAdd(void)
{
   setVisible(false);
   return Parent::onAdd();
}
   
DWORD FearGuiPage::onMessage(SimObject *sender, DWORD msg)
{
   if (msg == getTag())
   {
      setVisible(TRUE);
      messageSiblings(IDCTG_GS_PAGE_DISABLE);
   }
   else if (msg == IDCTG_GS_PAGE_DISABLE)
   {
      setVisible(FALSE);
   }
   
   return Parent::onMessage(sender, msg);
}   

void FearGuiPage::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   renderChildControls(sfc, offset, updateRect);
}

Persistent::Base::Error FearGuiPage::write( StreamIO &sio, int version, int user )
{
   return Parent::write(sio, version, user);
}

Persistent::Base::Error FearGuiPage::read( StreamIO &sio, int version, int user)
{
   return Parent::read(sio, version, user);
}

IMPLEMENT_PERSISTENT_TAG(FearGuiPage, FOURCC('F','G','p','g'));

};