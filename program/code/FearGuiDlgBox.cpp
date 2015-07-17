
#include "simResource.h"
#include "g_surfac.h"
#include "g_bitmap.h"
#include "editor.strings.h"
#include "fear.strings.h"
#include "FearGuiShellPal.h"
#include "fearGuiUnivButton.h"
#include "fearGuiDlgBox.h"

namespace SimGui {
extern Control *findControl(const char *name);
};

namespace FearGui
{

const int gControlVersion = 0;

bool FGDlgBox::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   mTopLeft    = SimResource::get(manager)->load("DLG_TopLeft.BMP");
   mTopEdge    = SimResource::get(manager)->load("DLG_TopEdge.BMP");
   mLeftEdge   = SimResource::get(manager)->load("DLG_LeftEdge.BMP");
   mBottomLeft = SimResource::get(manager)->load("DLG_BottomLeft.BMP");
   mBottomEdge = SimResource::get(manager)->load("DLG_BottomEdge.BMP");

   AssertFatal(mTopLeft.operator bool(), "Unable to TopLeft load bitmap");
   AssertFatal(mTopEdge.operator bool(), "Unable to TopEdge load bitmap");
   AssertFatal(mLeftEdge.operator bool(), "Unable to LeftEdge load bitmap");
   AssertFatal(mBottomLeft.operator bool(), "Unable to BottomLeft load bitmap");
   AssertFatal(mBottomEdge.operator bool(), "Unable to BottomEdge load bitmap");
   
   mTopLeft->attribute |= BMA_TRANSPARENT;
   mTopEdge->attribute |= BMA_TRANSPARENT;
   mLeftEdge->attribute |= BMA_TRANSPARENT;
   mBottomLeft->attribute |= BMA_TRANSPARENT;
   mBottomEdge->attribute |= BMA_TRANSPARENT;
   
   //set the dialog tag
   setTag(IDCTG_DIALOG);
   active = TRUE;

   return true;
}

void FGDlgBox::onPreRender(void)
{
   const char *myName = getName();
   if (myName && (! strcmp(myName, "ExitGameDialog")))
   {
      root->makeFirstResponder(this);
   }
   else if (myName && (! strcmp(myName, "RemoveDemoDialog")))
   {
      root->makeFirstResponder(this);
   }
   else if (myName && (! strcmp(myName, "RemovePlayerDialog")))
   {
      root->makeFirstResponder(this);
   }
   else if (myName && (! strcmp(myName, "MOTDDialog")))
   {
      root->makeFirstResponder(this);
   }
   else if (myName && (! strcmp(myName, "MessageDialog")))
   {
      root->makeFirstResponder(this);
   }
   else if (myName && (! strcmp(myName, "ServerInfoDialog")))
   {
      root->makeFirstResponder(this);
   }
   else if (myName && (! strcmp(myName, "HostWarnDialog")))
   {
      root->makeFirstResponder(this);
   }
   else if (myName && (! strcmp(myName, "DNSWarningDialog")))
   {
      root->makeFirstResponder(this);
   }
}

void FGDlgBox::onKeyDown(const SimGui::Event &event)
{
   if (event.diKeyCode == DIK_RETURN)
   {
      handleReturnKey();
   }
   else if (event.diKeyCode == DIK_ESCAPE)
   {
      handleEscapeKey();
   }
   else Parent::onKeyDown(event);
}
   
void FGDlgBox::handleReturnKey(void)
{
   FGUniversalButton *button = NULL;
   SimGui::Control *ctrl = SimGui::findControl("DialogReturnButton");
   if (! ctrl) ctrl = SimGui::findControl("DialogCloseButton");
   if (ctrl) button = dynamic_cast<FearGui::FGUniversalButton*>(ctrl);
   if (button)
   {
      button->onAction();
   }
}

void FGDlgBox::handleEscapeKey(void)
{
   FGUniversalButton *button = NULL;
   SimGui::Control *ctrl = SimGui::findControl("DialogEscapeButton");
   if (! ctrl) ctrl = SimGui::findControl("DialogCloseButton");
   if (ctrl) button = dynamic_cast<FGUniversalButton*>(ctrl);
   if (button)
   {
      button->onAction();
   }
}

void FGDlgBox::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   //draw the background
   sfc->drawRect2d_f(&RectI(offset.x, offset.y,
                            offset.x + extent.x,
                            offset.y + extent.y), BLACK);
   
   Point2I bmpOffset = offset;
   
   //top row
   sfc->drawBitmap2d(mTopLeft, &bmpOffset);
   bmpOffset.x += mTopLeft->getWidth();
   
   sfc->drawBitmap2d(mTopLeft, &bmpOffset);
   int topRight = offset.x + extent.x - mTopLeft->getWidth();
   while (bmpOffset.x < topRight)
   {
      sfc->drawBitmap2d(mTopEdge, &bmpOffset);
      bmpOffset.x += mTopEdge->getWidth();
   }
   bmpOffset.x = topRight;
   sfc->drawBitmap2d(mTopLeft, &bmpOffset, GFX_FLIP_X);
   
   //the sides
   int rightSide = offset.x + extent.x - mLeftEdge->getWidth();
   int bottomEdge = offset.y + extent.y - mBottomLeft->getHeight();
   bmpOffset.x = offset.x;
   bmpOffset.y = offset.y + mTopLeft->getHeight();
   while (bmpOffset.y < bottomEdge)
   {
      sfc->drawBitmap2d(mLeftEdge, &bmpOffset);
      sfc->drawBitmap2d(mLeftEdge, &Point2I(rightSide, bmpOffset.y), GFX_FLIP_X);
      bmpOffset.y += mLeftEdge->getHeight();
   }
   
   //bottom row
   bmpOffset.y = bottomEdge;
   sfc->drawBitmap2d(mBottomLeft, &bmpOffset);
   bmpOffset.x += mBottomLeft->getWidth();
   int bottomRight = offset.x + extent.x - mBottomLeft->getWidth();
   bmpOffset.y = offset.y + extent.y - mBottomEdge->getHeight();
   while (bmpOffset.x < bottomRight)
   {
      sfc->drawBitmap2d(mBottomEdge, &bmpOffset);
      bmpOffset.x += mBottomEdge->getWidth();
   }
   bmpOffset.x = bottomRight;
   bmpOffset.y = bottomEdge;
   sfc->drawBitmap2d(mBottomLeft, &bmpOffset, GFX_FLIP_X);
   
   //draw any chil'en
   renderChildControls(sfc, offset, updateRect);
}
                            
IMPLEMENT_PERSISTENT_TAG( FearGui::FGDlgBox,   FOURCC('F','G','d','x') );

Persistent::Base::Error FGDlgBox::write( StreamIO &sio, int version, int user)
{
   sio.write(gControlVersion);
   return Parent::write(sio, version, user);
}

Persistent::Base::Error FGDlgBox::read( StreamIO &sio, int version, int user)
{
   int localVersion;
   sio.read(&localVersion);
   return Parent::read(sio, version, user);
}

};