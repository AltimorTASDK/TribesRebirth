#include "g_bitmap.h"
#include "g_surfac.h"
#include "SimGuiCtrl.h"
#include "SimGuiScrollCtrl.h";
#include "FearGuiUnivButton.h"
#include "FearGuiCurCmd.h"
#include "FearGuiCommandTeamList.h"
#include "fear.strings.h"
#include "fearGuiShellPal.h"

namespace FearGui
{

class FGCommandPanel : public SimGui::Control
{

private:
   typedef SimGui::Control Parent;
   
   bool mLowRes;
   
public:
	FGCommandPanel(void) { mLowRes = FALSE; }
   
   bool isLowRes(void) { return mLowRes; }
   
   void onPreRender(void);
   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);
   
   DECLARE_PERSISTENT(FGCommandPanel);
};

class FGCommandBox : public SimGui::Control
{

private:
   typedef SimGui::Control Parent;
   
   bool mLowRes;
   
public:
   
	FGCommandBox(void);
   
   bool isLowRes(void) { return mLowRes; }
   void setLowRes(bool value, const Point2I &newPosition, const Point2I &newExtent);
   
   DECLARE_PERSISTENT(FGCommandBox);
};

//-------------------------------------------------------------------------------------------------

void  FGCommandPanel::onPreRender(void)
{
   bool updateChildren = FALSE;
   if ((! mLowRes) && (parent->extent.x < 512 || parent->extent.y < 384))
   {
      mLowRes = TRUE;
      position.set(parent->extent.x - 113, 5);
      extent.set(103, parent->extent.y - 12);
      updateChildren = TRUE;
   }
   else if (mLowRes && (parent->extent.x >= 512 && parent->extent.y >= 384))
   {
      mLowRes = FALSE;
      position.set(parent->extent.x - 226, 10);
      extent.set(207, parent->extent.y - 24);
      updateChildren = TRUE;
   }
   
   if (updateChildren)
   {
      if (mLowRes)
      {
         //set the individual box position and extents
         FGCommandBox *ctrl = (FGCommandBox*)(findControlWithTag(IDCTG_CMD_BOX_0));
         if (ctrl) ctrl->setLowRes(TRUE, Point2I(2, 2), Point2I(99, 20));
         
         ctrl = (FGCommandBox*)(findControlWithTag(IDCTG_CMD_BOX_1));
         if (ctrl) ctrl->setLowRes(TRUE, Point2I(2, parent->extent.y - 105), Point2I(99, 40));
         
         ctrl = (FGCommandBox*)(findControlWithTag(IDCTG_CMD_BOX_2));
         if (ctrl) ctrl->setLowRes(TRUE, Point2I(2, parent->extent.y - 65), Point2I(99, 19));
         
         ctrl = (FGCommandBox*)(findControlWithTag(IDCTG_CMD_BOX_3));
         if (ctrl) ctrl->setLowRes(TRUE, Point2I(6, parent->extent.y - 43), Point2I(18, 27));
         
         ctrl = (FGCommandBox*)(findControlWithTag(IDCTG_CMD_BOX_4));
         if (ctrl) ctrl->setLowRes(TRUE, Point2I(26, parent->extent.y - 42), Point2I(48, 25));
         
         FGUniversalButton *radio = (FGUniversalButton*)(findControlWithTag(IDCTG_CMD_RADIO_0));
         if (radio)
         {
            radio->setLowRes(mLowRes);
            radio->position.set(76, parent->extent.y - 38);
         }
         
         FGCurrentCommand *ccText = (FGCurrentCommand*)(findControlWithTag(IDCTG_CMD_TEXT_0));
         if (ccText)
         {
            ccText->setLowRes(mLowRes);
            ccText->position.set(2, 2);
         }
         ccText = (FGCurrentCommand*)(findControlWithTag(IDCTG_CURRENT_COMMAND));
         if (ccText)
         {
            ccText->setLowRes(mLowRes);
            ccText->position.set(2, 9);
         }
         
         SimGui::ScrollCtrl *sc = (SimGui::ScrollCtrl*)(findControlWithTag(IDCTG_CMD_SCROLL_0));
         if (sc)
         {
            sc->resize(Point2I(2, 22), Point2I(99, 112 + parent->extent.y - 240));
         }
         
         FearGui::CommandTeamCtrl *teamList = (FearGui::CommandTeamCtrl*)(findControlWithTag(IDCTG_COMMAND_PLAYER_LIST));
         if (teamList) teamList->setLowRes(mLowRes);
         
         sc = (SimGui::ScrollCtrl*)(findRootControlWithTag(IDCTG_CMD_SCROLL_1));
         if (sc)
         {
            sc->resize(Point2I(0, 0), Point2I(parent->extent.x - extent.x, parent->extent.y));
         }
      }
      else
      {
         //set the individual box position and extents
         FGCommandBox *ctrl = (FGCommandBox*)(findControlWithTag(IDCTG_CMD_BOX_0));
         if (ctrl) ctrl->setLowRes(FALSE, Point2I(3, 2), Point2I(201, 41));
         
         ctrl = (FGCommandBox*)(findControlWithTag(IDCTG_CMD_BOX_1));
         if (ctrl) ctrl->setLowRes(FALSE, Point2I(3, parent->extent.y - 211), Point2I(201, 80));
         
         ctrl = (FGCommandBox*)(findControlWithTag(IDCTG_CMD_BOX_2));
         if (ctrl) ctrl->setLowRes(FALSE, Point2I(3, parent->extent.y - 130), Point2I(201, 39));
         
         ctrl = (FGCommandBox*)(findControlWithTag(IDCTG_CMD_BOX_3));
         if (ctrl) ctrl->setLowRes(FALSE, Point2I(12, parent->extent.y - 86), Point2I(37, 54));
         
         ctrl = (FGCommandBox*)(findControlWithTag(IDCTG_CMD_BOX_4));
         if (ctrl) ctrl->setLowRes(FALSE, Point2I(52, parent->extent.y - 84), Point2I(96, 50));
         
         FGUniversalButton *radio = (FGUniversalButton*)(findControlWithTag(IDCTG_CMD_RADIO_0));
         if (radio)
         {
            radio->setLowRes(mLowRes);
            radio->position.set(152, parent->extent.y - 76);
         }
         
         FGCurrentCommand *ccText = (FGCurrentCommand*)(findControlWithTag(IDCTG_CMD_TEXT_0));
         if (ccText)
         {
            ccText->setLowRes(mLowRes);
            ccText->position.set(4, 2);
         }
         ccText = (FGCurrentCommand*)(findControlWithTag(IDCTG_CURRENT_COMMAND));
         if (ccText)
         {
            ccText->setLowRes(mLowRes);
            ccText->position.set(4, 18);
         }
         
         SimGui::ScrollCtrl *sc = (SimGui::ScrollCtrl*)(findControlWithTag(IDCTG_CMD_SCROLL_0));
         if (sc)
         {
            sc->resize(Point2I(3, 44), Point2I(201, 224 + parent->extent.y - 480));
         }
         
         FearGui::CommandTeamCtrl *teamList = (FearGui::CommandTeamCtrl*)(findControlWithTag(IDCTG_COMMAND_PLAYER_LIST));
         if (teamList) teamList->setLowRes(mLowRes);
         
         sc = (SimGui::ScrollCtrl*)(findRootControlWithTag(IDCTG_CMD_SCROLL_1));
         if (sc)
         {
            sc->resize(Point2I(0, 0), Point2I(parent->extent.x - extent.x, parent->extent.y));
         }
      }
   }
}

void FGCommandPanel::onRender(GFXSurface* sfc, Point2I offset, const Box2I& updateRect)
{
   int cornerWidth = (! mLowRes ? 9 : 4);
   
   //draw the bounding rect
   Point2I bottomRight, topLeft;
   topLeft.x = offset.x;
   topLeft.y = offset.y;
   bottomRight.x = offset.x + extent.x - 1;
   bottomRight.y = offset.y + extent.y - 1;
   
   //DRAW THE TRANSPARENT BACKGROUND
   //initialize our vertices
   Point3F poly[5];
   for (int j = 0; j < 5; j++)
   {
      poly[j].z = 1.0f;
   }
   
   //set the vertices
   poly[0].x = topLeft.x;
   poly[0].y = topLeft.y;
   poly[1].x = bottomRight.x;
   poly[1].y = topLeft.y;
   poly[2].x = bottomRight.x;
   poly[2].y = bottomRight.y;
   poly[3].x = topLeft.x + cornerWidth;
   poly[3].y = bottomRight.y;
   poly[4].x = topLeft.x;
   poly[4].y = bottomRight.y - cornerWidth;
   
   //draw the box
   sfc->setFillMode(GFX_FILL_CONSTANT);
   sfc->setShadeSource(GFX_SHADE_NONE);
   sfc->setHazeSource(GFX_HAZE_NONE);
   sfc->setAlphaSource(GFX_ALPHA_NONE);
   sfc->setTransparency (false);
   sfc->setFillColor(GREEN_40);

   for (int i = 0; i < 5; i++)
   {
      sfc->addVertex(poly + i);
   }
   sfc->emitPoly();
   
   //draw the outline
   for (int k = 0; k < 5; k++)
   {
      sfc->drawLine2d(&Point2I(poly[k].x, poly[k].y), &Point2I(poly[(k + 1) % 5].x, poly[(k + 1) % 5].y), GREEN_132);
   }
   
   //draw any chil'en
   renderChildControls(sfc, offset, updateRect);
}

IMPLEMENT_PERSISTENT_TAG( FearGui::FGCommandPanel,   FOURCC('F','G','c','p') );

//-------------------------------------------------------------------------------------------------

FGCommandBox::FGCommandBox(void)
{
   mLowRes = FALSE;
   position.set(0, 0);
   extent.set(100, 50);
}

void FGCommandBox::setLowRes(bool value, const Point2I &newPosition, const Point2I &newExtent)
{
   mLowRes = value;
   position = newPosition;
   extent = newExtent;
   
   //loop through all it's children, switch all command radios to low res
   SimGui::Control::iterator i;
   for (i = begin(); i != end(); i++)
   {
      FGUniversalButton *radio = dynamic_cast<FGUniversalButton*>(*i);
      if (radio) radio->setLowRes(mLowRes);
   }
   
   setUpdate();
}

IMPLEMENT_PERSISTENT_TAG( FearGui::FGCommandBox,   FOURCC('F','G','c','x') );

};