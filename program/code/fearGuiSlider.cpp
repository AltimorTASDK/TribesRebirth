//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "console.h"
#include <g_surfac.h>
#include <g_bitmap.h>
#include "simResource.h"
#include <inspect.h>
#include "editor.strings.h"
#include "fear.strings.h"
#include "fearGuiSlider.h"

namespace SimGui {
extern Control *findControl(const char *name);
};

namespace FearGui
{

const int gControlVersion = 0;

static bool gConsoleFunctionAdded = FALSE;

IMPLEMENT_PERSISTENT_TAG( FearGui::FGSlider,   FOURCC('F','G','s','k') );

//------------------------------------------------------------------------------
static const char *FGSliderSetDiscretePositions(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 3)
   {
      Console->printf("%s(control, numPositions);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   FGSlider *slider = NULL;
   if (ctrl) slider = dynamic_cast<FGSlider *>(ctrl);

   if (! slider)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   slider->setDiscretePositions(atoi(argv[2]));
   return "TRUE";
}

bool FGSlider::onAdd(void)
{
   if (! Parent::onAdd()) return FALSE;
   
   //load the bma
   const char *pbaName = SimTagDictionary::getString(manager, IDPBA_SLIDER_SHELL);
   if(pbaName)
   {
      bma = SimResource::get(manager)->load(pbaName, true);
      if(bool(bma))
      {
         for (int i = 0; i < bma->numBitmaps; i++)
         {
            GFXBitmap *bmp = bma->getBitmap(i);
            if (bmp) bmp->attribute |= BMA_TRANSPARENT;
         }
      }
      else return FALSE;
   }
   else return FALSE;
   
   //set the thumb width
   thumbWidth = 30;
                  
	minThumbX = bma->getBitmap(BmpLeftArrow)->getWidth();
   maxThumbX = extent.x - bma->getBitmap(BmpRightArrow)->getWidth() - thumbWidth;
   curThumbX = minThumbX;
   
   //set the extent
   extent.y = bma->getBitmap(BmpLeftArrow)->getHeight();
   
   //ensure the min and max values are valid
   if (minVal >= maxVal)
   {
      minVal = 0.0f;
      maxVal = 1.0f;
   }
   if (numDiscreteValues <= 1)
   {
      minVal = 0.0;
      maxVal = 1.0;
   }
   
   if (! gConsoleFunctionAdded)
   {
      Console->addCommand(0, "FGSlider::setDiscretePositions", FGSliderSetDiscretePositions);
      gConsoleFunctionAdded = TRUE;
   }
   
   return TRUE;
}

int FGSlider::getRegion(Point2I pos)
{
   if (pos.x < 0) return RegionOutside;
   else if (pos.x < minThumbX) return RegionLeftArrow;
   else if (pos.x < curThumbX) return RegionPageLeft;
   else if (pos.x < curThumbX + thumbWidth) return RegionThumb;
   else if (pos.x < maxThumbX) return RegionPageRight;
   else if (pos.x < extent.x) return RegionRightArrow;
   else return RegionOutside;
}

void FGSlider::onMouseDown(const SimGui::Event &event)
{
   //lock the mouse
   root->mouseLock(this); 
   stateDepressed = TRUE;
   
   Point2I localPt = globalToLocalCoord(event.ptMouse);
   int region = getRegion(localPt);
   switch (region)
   {
      case RegionLeftArrow:
         if (numDiscreteValues <= 1)
         {
            curThumbX = max(minThumbX, curThumbX - 1);
            break;
         }
         //else fall through to page
         
      case RegionPageLeft:
      {
         int pageValue;
         if (numDiscreteValues > 1) pageValue = (maxThumbX - minThumbX) / (numDiscreteValues - 1);
         else pageValue = (maxThumbX - minThumbX) / 8;
         curThumbX = max(minThumbX, curThumbX - pageValue);
         break;
      }
      
      case RegionThumb:
         dragginThumb = TRUE;
         dragOffsetX = localPt.x - curThumbX;
         break;
         
      case RegionRightArrow:
         if (numDiscreteValues <= 1)
         {
            curThumbX = min(maxThumbX, curThumbX + 1);
            break;
         }
         //else fall through to page
      
      case RegionPageRight:
      {
         int pageValue;
         if (numDiscreteValues > 1) pageValue = (maxThumbX - minThumbX) / (numDiscreteValues - 1);
         else pageValue = (maxThumbX - minThumbX) / 8;
         curThumbX = min(maxThumbX, curThumbX + pageValue);
         break;
      }
   }
   setUpdate();
}

void FGSlider::onMouseDragged(const SimGui::Event &event)
{
   Point2I localPt = globalToLocalCoord(event.ptMouse);
   if (! dragginThumb) return;
   curThumbX = min(maxThumbX, max(minThumbX, int(localPt.x - dragOffsetX)));
   
   //call the altconsole function if exists
   if (altConsoleCommand[0])
   {
      Console->evaluate(altConsoleCommand, FALSE);
   }
   
   setUpdate();
}

void FGSlider::onMouseUp(const SimGui::Event &)
{
   root->mouseUnlock();
   stateDepressed = FALSE;
   dragginThumb = FALSE;
   if (numDiscreteValues > 1)
   {
      float pageValue = float(maxThumbX - minThumbX) / float(numDiscreteValues - 1);
      int curPage = int(float(curThumbX - minThumbX) / pageValue);
      
      //round up
      if ((curThumbX - minThumbX) - int(curPage * pageValue) >= int(pageValue / 2.0f))
      {
         curPage++;
      }
      
      //bound the curPage
      curPage = max(0, min(numDiscreteValues - 1, curPage));
      
      //set the thumb
      curThumbX = max(minThumbX, min(maxThumbX, minThumbX + int(curPage * pageValue)));
   }
   
   //call the console function if exists
   if (consoleCommand[0])
   {
      Console->evaluate(consoleCommand, FALSE);
   }
   setUpdate();
}

void FGSlider::onMouseRepeat(const SimGui::Event &event)
{
   if (! dragginThumb) onMouseDown(event);
}

void FGSlider::onPreRender(void)
{
   maxThumbX = extent.x - bma->getBitmap(BmpRightArrow)->getWidth() - thumbWidth;
}

//------------------------------------------------------------------------------
void FGSlider::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(1);
   if ((! active) ||
       (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG))))
   {
      ghosted = TRUE;
   }
   
   Point2I bmpOffset = offset;
   GFXBitmap *bmp;
   int stretch;
   
   int region = RegionOutside;
   if (root)
   {
      Point2I cursorPos = root->getCursorPos();
      Point2I localPt = globalToLocalCoord(cursorPos);
      region = getRegion(localPt);
   }
   if (dragginThumb) region = RegionThumb;
   
   //left
   if (ghosted) bmp = bma->getBitmap(BmpLeftArrowDisabled);
   else if (stateDepressed && region == RegionLeftArrow) bmp = bma->getBitmap(BmpLeftArrowSelected);
   else bmp = bma->getBitmap(BmpLeftArrow);
   sfc->drawBitmap2d(bmp, &bmpOffset);
   bmpOffset.x += bmp->getWidth();
   
   //mid
   stretch = extent.x - bmp->getWidth() - bma->getBitmap(BmpRightArrow)->getWidth();
   bmp = bma->getBitmap(! ghosted ? BmpHorizontalPage : BmpHorizontalPageDisabled);
   sfc->drawBitmap2d(bmp, &bmpOffset, &Point2I(stretch, bmp->getHeight()));
   bmpOffset.x += stretch;
   
   //right
   if (ghosted) bmp = bma->getBitmap(BmpRightArrowDisabled);
   else if (stateDepressed && region == RegionRightArrow) bmp = bma->getBitmap(BmpRightArrowSelected);
   else bmp = bma->getBitmap(BmpRightArrow);
   sfc->drawBitmap2d(bmp, &bmpOffset);
   
   //now the thumb
   if (ghosted) return;
   bmpOffset.x = offset.x + curThumbX;
   bmp = bma->getBitmap(dragginThumb ? BmpHorizontalThumbLeftCapSelected : BmpHorizontalThumbLeftCap);
   sfc->drawBitmap2d(bmp, &bmpOffset);
   bmpOffset.x += bmp->getWidth();
   
   stretch = thumbWidth - bmp->getWidth() - bma->getBitmap(BmpHorizontalThumbRightCap)->getWidth();
   bmp = bma->getBitmap(dragginThumb ? BmpHorizontalThumbSelected : BmpHorizontalThumb);
   sfc->drawBitmap2d(bmp, &bmpOffset, &Point2I(stretch, bmp->getHeight()));
   bmpOffset.x += stretch;
   
   bmp = bma->getBitmap(dragginThumb ? BmpHorizontalThumbRightCapSelected : BmpHorizontalThumbRightCap);
   sfc->drawBitmap2d(bmp, &bmpOffset);
}


const char *FGSlider::getScriptValue()
{
   static char buffer[32];
   buffer[0] = '\0';
   if (numDiscreteValues > 1)
   {
      float pageValue = float(maxThumbX - minThumbX) / float(numDiscreteValues - 1);
      int curPage = int(float(curThumbX - minThumbX) / pageValue);
      
      //round up
      if ((curThumbX - minThumbX) - int(curPage * pageValue) >= int(pageValue / 2.0f))
      {
         curPage++;
      }
      
      //bound the curPage
      curPage = max(0, min(numDiscreteValues - 1, curPage));
      
      sprintf(buffer, "%d", curPage);
   }
   else
   {
      float value = minVal + ((float(curThumbX - minThumbX) / float(maxThumbX - minThumbX)) * (maxVal - minVal));
      sprintf(buffer, "%f", value);
   }   
   return buffer;
}

void FGSlider::setScriptValue(const char *value)
{
   if (numDiscreteValues > 1)
   {
      float pageValue = float(maxThumbX - minThumbX) / float(numDiscreteValues - 1);
      int curPage = atoi(value);
      
      //bound the curPage
      curPage = max(0, min(numDiscreteValues - 1, curPage));
      
      //set the thumb
      curThumbX = max(minThumbX, min(maxThumbX, minThumbX + int(curPage * pageValue)));
   }
   
   else
   {
      float newValue = float(max(minVal, float(min(maxVal, float(atof(value))))));
      int newThumbX = minThumbX + int(((newValue - minVal) / (maxVal - minVal)) * float(maxThumbX - minThumbX));
      curThumbX = max(minThumbX, min(maxThumbX, newThumbX));
   }
}

void FGSlider::setDiscretePositions(int numPositions)
{
   if (numPositions < 2) numDiscreteValues = 0;
   else numDiscreteValues = numPositions;
   setScriptValue("0");
}

//------------------------------------------------------------------------------
void FGSlider::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);
   insp->write(IDITG_PC_INCREMENT, long(numDiscreteValues));
}   

//------------------------------------------------------------------------------
void FGSlider::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   insp->read(IDITG_PC_INCREMENT, (long &)numDiscreteValues);
}   

//------------------------------------------------------------------------------
Persistent::Base::Error FGSlider::write( StreamIO &sio, int version, int user )
{
   sio.write(gControlVersion);
   sio.write(numDiscreteValues);
   
   return Parent::write(sio, version, user);
}

//------------------------------------------------------------------------------
Persistent::Base::Error FGSlider::read( StreamIO &sio, int version, int user)
{
   int curVersion;
   sio.read(&curVersion);
   sio.read(&numDiscreteValues);
   
   if (numDiscreteValues <= 1)
   {
      minVal = 0.0;
      maxVal = 1.0;
   }

   return Parent::read(sio, version, user);
}

Int32 FGSlider::getMouseCursorTag(void)
{
   return (active ? IDBMP_CURSOR_HAND : 0);
}

};
