//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "simResource.h"
#include "FearGuiStandardCombo.h"
#include "FearGuiBuddyCombo.h"
#include "g_bitmap.h"
#include "g_surfac.h"
#include "soundFX.h"
#include "fear.strings.h"
#include "console.h"

namespace FearGui
{

//------------------------------------------------------------------------------
bool FGBuddyComboBox::onAdd(void)
{
   if (! Parent::onAdd()) return FALSE;
   
   setSort(FALSE);
   
   return TRUE;
}

void FGBuddyComboBox::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   //Compiler Warning
   updateRect;
   
   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(1);
   if ((! active) ||
       (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG))))
   {
      ghosted = TRUE;
   }
   
   GFXBitmap *bmp;
   if (ghosted) bmp = mBitmaps[BMP_Ghosted];
   else if (stateDepressed) bmp = mBitmaps[BMP_Pressed];
   else if (stateOver) bmp = mBitmaps[BMP_MouseOverStandard]; 
   else bmp = mBitmaps[BMP_Standard];
   if (! bmp) bmp = mBitmaps[BMP_Standard];
   
   //draw the down arrow bitmap
   sfc->drawBitmap2d(bmp, &Point2I(offset.x + extent.x - 7 - bmp->getWidth(), offset.y + 2));
} 

IMPLEMENT_PERSISTENT_TAG(FGBuddyComboBox, FOURCC('F','G','x','x'));

};