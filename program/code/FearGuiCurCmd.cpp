//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "g_font.h"
#include "g_surfac.h"
#include "g_bitmap.h"
#include "darkstar.strings.h"
#include "fear.strings.h"
#include "FearGuiCurCmd.h"

namespace FearGui
{

void FGCurrentCommand::clear(void)
{
   setText("");
   messageSiblings(IDSYS_SIBLING_DISABLE);
}

void FGCurrentCommand::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   GFXFont *font = (! mLowRes ? hFont : hFontHL);
   if (font)
   {
      sfc->drawText_p(font, &Point2I(offset.x, (!mLowRes ? offset.y : offset.y - 2)), text);
   }
}


IMPLEMENT_PERSISTENT_TAG(FGCurrentCommand, FOURCC('F','G','c','z'));

};

