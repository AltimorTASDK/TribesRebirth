//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "simGuiComboBox.h"
#include "simGuiScrollCtrl.h"
#include "simGuiArrayCtrl.h"
#include "simGuiTextList.h"
#include "g_surfac.h"

namespace SimGui
{
 
//------------------------------------------------------------------------------
ComboPopUp::ComboPopUp(SimGui::ComboBox *_parentCb)
{
   parentCb = _parentCb;   
   ac = NULL;
   sc = NULL;
}  
 
void ComboPopUp::setDim(Int32 w, Int32 hMin, Int32 hMax)
{
   if (sc && ac)
   {
      sc->extent.set(w,hMax);
      dim.set(w,hMax);
      sc->onWake();

      //RectI viewRect(0,0,sc->getScrollContentCtrl()->extent.x - 1, sc->getScrollContentCtrl()->extent.y - 1);

      //Int32 borderLen = hMax - viewRect.len_y() + 1;
      //Int32 maxTlLen = tl->getNumEntries() * tl->scroll_Line_V(viewRect) + borderLen;
      
      //if (maxTlLen < hMin)
      //   maxTlLen = hMin;
              
      //if (maxTlLen < hMax)
      //{
         sc->extent.y = hMin;
         sc->onWake();
         dim.y = hMin;
      //}
      ac->extent.x = sc->getScrollContentCtrl()->extent.x - 1; //viewRect.len_x();
   }
}  

void ComboPopUp::getDim(Point2I &p)
{
   p = dim;
}   

void ComboPopUp::setPos(Int32 x, Int32 y)
{
   if (sc)
      sc->position.set(x,y);
   setUpdate();
} 

void ComboPopUp::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   renderChildControls(sfc, offset, updateRect);
}

void ComboPopUp::onMouseDown(const Event &event)
{
   Point2I scMousePos = sc->globalToLocalCoord(event.ptMouse);
   ScrollCtrl::Region rgn = sc->findHitRegion(scMousePos);
   
   if ((rgn == ScrollCtrl::None) && parentCb)
   {
      ac->onMouseDown(event);
      root->mouseUnlock();
      parentCb->closePopUpCtrl();
   }
   else
   {
      sc->onMouseDown(event);
      root->mouseLock(this);
   }
}   

void ComboPopUp::onMouseMove(const Event &event)
{
   Point2I scMousePos = sc->globalToLocalCoord(event.ptMouse);
   ScrollCtrl::Region rgn = sc->findHitRegion(scMousePos);
   
   if (rgn == ScrollCtrl::None)
   {
      ac->onMouseMove(event);
   }
}   

void ComboPopUp::onMouseDragged(const Event &event)
{
   sc->onMouseDragged(event);
   root->mouseLock(this);
}   

void ComboPopUp::onMouseUp(const Event &event)
{
   sc->onMouseUp(event);
}   

DWORD ComboPopUp::onMessage(SimObject *, DWORD msg)
{
   if (parentCb)
      parentCb->closePopUpCtrl();
   return msg;
}   

bool ComboPopUp::onAdd()
{
   if(!Parent::onAdd())
      return false;

   deleteNotify(parentCb);
 
   sc = new ScrollCtrl();
   AssertFatal(sc, "could not allocate memory");
   sc->forceVScrollBar = ScrollCtrl::ScrollBarAlwaysOn;
   sc->forceHScrollBar = ScrollCtrl::ScrollBarAlwaysOff;
   sc->willFirstRespond = false;
   manager->addObject(sc);
   addObject(sc);
   
   ac = new TextList();
   AssertFatal(ac, "could not allocate memory");
   manager->addObject(ac);
   sc->addObject(ac);
   ac->message = IDSTR_STRING_DEFAULT; // we need the text list to
                                       // give the pop up ctrl some
                                       // message indicating that
                                       // mouseUp or keyUp has occured
   return true;
}  
 
void ComboPopUp::onRemove()
{
   parentCb = NULL;
   Parent::onRemove();
}  
 
void ComboPopUp::onDeleteNotify(SimObject *object)
{
   if (object == parentCb)
      parentCb = NULL;
   if (object == ac)
      ac = NULL;
   if (object == sc)
      sc = NULL;

   Parent::onDeleteNotify(object);
}  

//------------------------------------------------------------------------------
void ComboBox::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   RectI r(offset.x, offset.y, offset.x + extent.x - 1, offset.y + extent.y - 1);
   sfc->drawRect2d_f(&r, 253); 
   sfc->drawRect2d(&r, 254);
   Parent::onRender(sfc, offset, updateRect);
}

bool ComboBox::onAdd()
{
   if(!Parent::onAdd())
      return false;

   popUpCtrl = new ComboPopUp(this);
   AssertFatal(popUpCtrl, "could not allocate memory");
   manager->addObject(popUpCtrl);
   deleteNotify(popUpCtrl);
   return true;
}  
 
void ComboBox::onRemove()
{
   if (popUpCtrl)
      manager->deleteObject(popUpCtrl);

   Parent::onRemove();
}  
 
void ComboBox::onDeleteNotify(SimObject *object)
{
   if (object == popUpCtrl)
   {
      clearNotify(popUpCtrl);
      popUpCtrl = NULL;
   }
   Parent::onDeleteNotify(object);
}  
 
void ComboBox::onAction()
{
   openPopUpCtrl();
}


void ComboBox::openPopUpCtrl()
{
   if (popUpCtrl)
   {
      // set up the data for the pop up ctrl
      TextList *tl = (TextList*)getArrayCtrl();
      AssertFatal(tl, "pop up control is missing a text list");
      AssertFatal(fontNameTag, "Combo box needs a font");
      tl->setFont(fontNameTag);
      AssertFatal(fontNameTagHL, "Combo box needs a highlight font");
      tl->setFontHL(fontNameTagHL);
      tl->onWake();
      Point2I globalPos = localToGlobalCoord(Point2I(0,0));
      popUpCtrl->setPos(globalPos.x, globalPos.y + extent.y);
      popUpCtrl->setDim(extent.x, 100, 100);
      root->pushDialogControl(popUpCtrl);
      root->tabNext();
      popUpActive = true;
      root->mouseLock(this);
   }
}   

void ComboBox::closePopUpCtrl()
{
   root->mouseUnlock();
   if (popUpCtrl == root->getTopDialog())
   {
      // get data from the pop up ctrl
      updateFromArrayCtrl();
      root->popDialogControl();
      root->makeFirstResponder(this);
      popUpActive = false;
      if (consoleVariable[0]) setVariable(text);
      Parent::onMessage(this, message);    
   }
}  

ArrayCtrl * ComboBox::getArrayCtrl()
{
   AssertFatal(popUpCtrl, "pop up control is missing");
   return popUpCtrl->getArrayCtrl();   
}   

void ComboBox::updateFromArrayCtrl()
{
   AssertFatal(popUpCtrl && popUpCtrl->getArrayCtrl(), "pop up control is missing a text list");
   TextList *tl = (TextList*)(popUpCtrl->getArrayCtrl());
   setText(tl->getSelectedText());
}  
 
};