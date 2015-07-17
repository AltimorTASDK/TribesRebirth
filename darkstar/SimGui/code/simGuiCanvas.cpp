#include "simGuiCanvas.h"
#include "g_surfac.h"
#include "simGuiCtrl.h"
#include "simGuiEditCtrl.h"
#include "simGuiTextFormat.h"
#include "simGuiHelpCtrl.h"
#include "g_surfac.h"
#include "simGuiDelegate.h"
#include "simTagDictionary.h"
#include "darkstar.strings.h"
#include "g_bitmap.h"
#include "g_cds.h"
#include "simGame.h"
#include "windows.h"
#include <simguiconsoleplugin.h>

namespace SimGui
{
extern int dummy_link;
extern int DelegateLinkFlag;

#ifdef DEBUG
bool Canvas::canvasCursorTrapped = false;
#else
bool Canvas::canvasCursorTrapped = true;
#endif

bool g_prefPoliteGui = false;

bool Canvas::onAdd()
{
   if(!Parent::onAdd())
      return false;

   addToSet(SimInputConsumerSetId);

   return true;
}

Canvas::Canvas()
{
   hasFocus    = false;
   useWindowsMouseEvents = false;

#ifdef WINKEYBOARD
   useWindowsKeyboardEvents = false;
   lastAscii = lastDIKCode = 0;
#endif
   defaultCursor.simTag     = 0;
   defaultCursor.hBmp       = NULL;
   defaultCursor.rsBMA      = NULL;
   defaultCursor.rFrameTime = 0.0f;

   controlCursor.simTag     = 0;
   controlCursor.hBmp       = NULL;
   controlCursor.rsBMA      = NULL;
   controlCursor.rFrameTime = 0.0f;

   defaultCursor.hotspot.set(0, 0);
   controlCursor.hotspot.set(0, 0);

   cursorON    = false; 
   forwardMouseMove = false;
   lastCursorON    = false; 
   cursorOverApp = false;
   cursorVisible = true;
   skipNextDelta = false;
   pixelsPerMickey = 1.0f;
   rLastFrameTime = 0.0f;

   delegate = NULL;
   firstResponder = this;
   editControl = NULL;
   topContentControl = NULL;
   mouseCapturedControl = NULL;
   mouseControl = NULL;
   mouseControlClicked = FALSE;
   mouseButtonDown = false;
   mouseRightButtonDown = false;
   dummy_link = 0;
   DelegateLinkFlag = 0;

   lastCursor = NULL;
   lastCursorPt.set(0,0);
   cursorPt.set(0,0);
   canvasUpdateRegion(0,0,0,0);
   
   helpCtrl = NULL;

   initialMouseDelay = 300;
   repeatMouseDelay = 50;
   initialKeyDelay = 300;
   repeatKeyDelay = 50;
   
   nextKeyTime = 0xFFFFFFFF;
   nextMouseTime = 0xFFFFFFFF;
   
	lastMouseDownCount = 0;
	lastMouseDownTime = 0;
   willCaptureAllInputs = false;
	prevMouseTime = 0.0f;
}

Canvas::~Canvas()
{
   if (helpCtrl) delete helpCtrl;
}

void Canvas::preRender()
{
   hasFocus = isFocused();

   // Render the GUI console
   if (SimGame::get()->getConsole()->active())
   {
      resetUpdateRegions();
   }

   // Update animating cursor
}

bool Canvas::processEvent(const SimEvent *event)
{
   switch(event->type)
   {
      onEvent(SimInputEvent);
      default:
         return Parent::processEvent(event);
   }
}

//------------------------------------------------------------------------------
bool Canvas::processQuery(SimQuery *query)
{
   if (query->type == SimInputPriorityQueryType)
   {
      SimInputPriorityQuery *q = (SimInputPriorityQuery*)query;
      q->priority = SI_PRIORITY_HIGH;
      return (true);
   }
   return Parent::processQuery(query);
}

//------------------------------------------------------------------------------
#if 0
void Canvas::preRender()
{
   if ( hasFocus != isFocused() )
   {
      SimInputManager *im = findObject( manager, SimInputManagerId, (SimInputManager*)0 );
      if ( im )
      {
         if ( isFocused() )
            im->activate( SI_KEYBOARD, 0 );
         else
            im->deactivate( SI_KEYBOARD, 0 );
      }
      hasFocus = isFocused();
   }
}
#endif

void Canvas::setCursorON(bool onOff)               
{ 
   cursorON = onOff;
   if(!cursorON)
   {
      SimInputManager *im = SimGame::get()->getInputManager();
      im->activate(SI_MOUSE,0);
   }
}

void Canvas::setCursor(Resource<GFXBitmap> hBmp, Point2I hotspot)
{
   defaultCursor.hBmp = hBmp;
   if ((bool)hBmp)
   {
      defaultCursor.hBmp->attribute |= BMA_TRANSPARENT;
   }
   defaultCursor.hotspot = hotspot;
   resetUpdateRegions();
}

void Canvas::setCursor(Resource<GFXBitmapArray> rsBMA, Point2I hotspot, float rFrameTime)
{
   defaultCursor.rsBMA = rsBMA;

   if ((bool)rsBMA)
   {
      for (int i = 0; i < rsBMA->numBitmaps; i ++)
      {
         rsBMA->getBitmap(i)->attribute |= BMA_TRANSPARENT;
      }
   }
   defaultCursor.hotspot    = hotspot;
   defaultCursor.rFrameTime = rFrameTime;
   defaultCursor.iFrame     = 0;
   rLastFrameTime = manager->getCurrentTime();
   resetUpdateRegions();
}

void Canvas::setCursor(Cursor &cur, Int32 simTag)
{
   // simTag specifies a cursor definition string
   // which has the form <bmp name>,<hotspot x>,<hotspot y>
   // (ie "arrow.bmp,2,4" => a bitmap named arrow.bmp with
   // it's hotspot at (2,4))
   // <hotspot x>  and <hotspot y> are optional

   if (simTag == cur.simTag)
   {
      return; // already loaded
   }

   cur.simTag     = simTag;
   cur.hBmp       = NULL;
   cur.rsBMA      = NULL;
   cur.iFrame     = 0;
   cur.rFrameTime = DEFAULT_FRAME_TIME;
   cur.hotspot.set(0,0);

   const char *cursorDef = SimTagDictionary::getString(manager, simTag);

   if (cursorDef)
   {
      char buffer[256];
      int i;
      for (i = 0; *cursorDef && (*cursorDef != ','); i++)
      {
         buffer[i] = *(cursorDef ++);
      }
      buffer[i] = '\0';

      ResourceManager *rm = SimResource::get(manager);
      if (rm)
      {  
         const char *lpszTail = strrchr(buffer, '.');

         if (lpszTail && stricmp(lpszTail, ".PBA") == 0)
         {
            cur.rsBMA = rm->load(buffer, true);

            if ((bool)cur.rsBMA)
            {
               for (int i = 0; i < cur.rsBMA->numBitmaps; i ++)
               {
                  cur.rsBMA->getBitmap(i)->attribute |= BMA_TRANSPARENT;
               }
            }
         }
         else
         {
            cur.hBmp = rm->load(buffer, true);
            
            if ((bool)cur.hBmp)
            {
               cur.hBmp->attribute |= BMA_TRANSPARENT;
            }
         }

         if (cursorDef && ((bool)cur.hBmp || (bool)cur.rsBMA))
         {
            sscanf(cursorDef, ",%d,%d,%f", 
               &cur.hotspot.x, &cur.hotspot.y, &cur.rFrameTime); 
         }
      }
   }
   rLastFrameTime = manager->getCurrentTime();
}  
 
//------------------------------------------------------------------------------
void Canvas::processMouseEvent(int x, int y, int type)
{
   SimInputEvent ev;
   ev.deviceType = SI_POINTER;
   ev.deviceInst = id;
   SimInputManager *manage = SimInputManager::find(manager);
   if (!manage)
      return;
   ev.modifier = manage->getModifierKeys();

   if(type == GWMouseEvent::MouseMove)
   {
      cursorPt.x = x;
      cursorPt.y = y;

		ev.objInst = 0;
		ev.action = SI_MOVE;
      ev.objType = SI_XAXIS;
      ev.fValue = x;
      onSimInputEvent(&ev);
      ev.objType = SI_YAXIS;
      ev.fValue = y;
      onSimInputEvent(&ev);
   }
   else
   {
      ev.objType = SI_BUTTON;
      ev.action = (type == GWMouseEvent::LButtonDown ||
         type == GWMouseEvent::RButtonDown) ? SI_MAKE : SI_BREAK;
      ev.objInst = (type == GWMouseEvent::LButtonDown ||
         type == GWMouseEvent::LButtonUp) ? 0 : 1;
      onSimInputEvent(&ev);
   }
}

//------------------------------------------------------------------------------
bool Canvas::onSimInputEvent(const SimInputEvent *event)
{
   // when we get a simInput event
   // pass it through the gui...
   // only process mouse events if the cursor is on
   // only process keyboard events if there is a first responder
   // that is not this canvas.
   // swallow those events - let the action handler get the rest.

   Event evt;
   bool moved = false;

   if(!hasFocus)
      return false;

   switch(event->deviceType)
   {
      case SI_POINTER:
      case SI_MOUSE:
         if(!cursorON)
            return false;
         evt.modifier = event->modifier;
         switch(event->objType)
         {
            case SI_XAXIS:
            case SI_YAXIS:
               if( forwardMouseMove ) 
                  return false;
               if(event->deviceType == SI_MOUSE)
               {
                  if ( skipNextDelta )
                  {
                     skipNextDelta--;
                     evt.ptMouse.x = int(cursorPt.x);
                     evt.ptMouse.y = int(cursorPt.y);
                     rootMouseMove(evt);
                     break;
                  }
                  Point2I oldpt(cursorPt.x, cursorPt.y);
                  Point2F pt(cursorPt.x, cursorPt.y);

                  if(!topContentControl)
                     return false;

                  Point2I maxExtent = topContentControl->getExtent();

                  if(event->objType == SI_XAXIS)
                  {
                     pt.x += (event->fValue * pixelsPerMickey);
                     cursorPt.x = limit(pt.x, maxExtent.x - 1);
                     if(oldpt.x != int(cursorPt.x))
                        moved = true;
                  }
                  else
                  {
                     pt.y += (event->fValue * pixelsPerMickey);
                     cursorPt.y = limit(pt.y, maxExtent.y - 1);
                     if(oldpt.y != int(cursorPt.y))
                        moved = true;
                  }
                  if ( !canvasCursorTrapped || !isFullScreen() || (GWMap::getWindowCount()>1) )
                  {
                     bool outside;
                     outside = ((!insideLimit(pt.x, maxExtent.x-1)) || 
                                (!insideLimit(pt.y, maxExtent.y-1)));
                     POINT mspt; mspt.x=pt.x; mspt.y=pt.y;
                     ClientToScreen( getHandle(), &mspt );
                     if ( (outside || WindowFromPoint(mspt)!=hWnd) )
                     {
                        SimGame::get()->getInputManager()->deactivate(SI_MOUSE,0);
                        SetCursorPos(mspt.x,mspt.y);
                        cursorOverApp = false;
                        skipNextDelta = 6;
                     }
                  }
               }
               else
                  moved = true;

               if(moved)
               {
                  evt.ptMouse.x = int(cursorPt.x);
                  evt.ptMouse.y = int(cursorPt.y);
                  if(mouseButtonDown)
                     rootMouseDragged(evt);
                  else if(mouseRightButtonDown)
                     rootRightMouseDragged(evt);
                  else
                     rootMouseMove(evt);
               }

               break;
            case SI_BUTTON:
               switch(event->objInst)
               {
                  case 0: { // left button
                     evt.ptMouse.x = int(cursorPt.x);
                     evt.ptMouse.y = int(cursorPt.y);
                     if(event->action == SI_MAKE)
                     {
                        DWORD curTime = GetTickCount();
                        nextMouseTime = curTime + initialMouseDelay;
                        
                        //if the last button pressed was the left...
                        if (leftMouseLast)
                        {
                           //if it was within the double click time count the clicks
                           if (curTime - lastMouseDownTime <= GetDoubleClickTime())
                           {
                              lastMouseDownCount++;
                           }
                           else lastMouseDownCount = 1;
                        }
                        else
                        {
                           leftMouseLast = TRUE;
                           lastMouseDownCount = 1;
                        }
                        
                        lastMouseDownTime = curTime;
                        
                        evt.mouseDownCount = lastMouseDownCount;
                        
                        lastMouseEvent = evt;
                        rootMouseDown(evt);
                     }
                     else
                     {
                        nextMouseTime = 0xFFFFFFFF;
                        rootMouseUp(evt);
                     }
                     break;
                  }
                  case 1: { // right button
                     evt.ptMouse.x = int(cursorPt.x);
                     evt.ptMouse.y = int(cursorPt.y);
                     if(event->action == SI_MAKE)
                     {
                        DWORD curTime = GetTickCount();
                        
                        //if the last button pressed was the right...
                        if (! leftMouseLast)
                        {
                           //if it was within the double click time count the clicks
                           if (curTime - lastMouseDownTime <= GetDoubleClickTime())
                           {
                              lastMouseDownCount++;
                           }
                           else lastMouseDownCount = 1;
                        }
                        else
                        {
                           leftMouseLast = FALSE;
                           lastMouseDownCount = 1;
                        }
                        
                        lastMouseDownTime = curTime;
                        evt.mouseDownCount = lastMouseDownCount;
                        
                        rootRightMouseDown(evt);
                     }
                     else
                        rootRightMouseUp(evt);
                     break;
                  }
               }
               break;
         }
         
         return true;
         
      case SI_KEYBOARD:
         if ( event->objInst==DIK_RETURN && event->modifier&SI_ALT )
            return false;
            
         //new code to handle the escape button
         if ((event->objInst == DIK_ESCAPE) && (event->action == SI_MAKE) &&
            Console->isFunction("Canvas::handleEscapeKey") && (contentControlStack.size() < 2))
         {
            const char *result = Console->evaluate("Canvas::handleEscapeKey();", false);
            if (! stricmp(result, "true"))
            {
               return true;
            }
         }
            
         Responder *prevResponder;
         prevResponder = firstResponder;
         if(event->deviceInst == 0 && firstResponder)
         {
            evt.ascii = event->ascii;
            evt.modifier = event->modifier;
            evt.diKeyCode = event->objInst;
            if(event->action == SI_MAKE)
            {
               lastKeyEvent = evt;

               // Tabs do not repeat
               if (evt.diKeyCode != DIK_TAB) {
                  nextKeyTime = GetTickCount() + initialKeyDelay;
               } else {
                  nextKeyTime = 0xFFFFFFFF;
               }
               firstResponder->onKeyDown(evt);
            }
            else
            {
               nextKeyTime = 0xFFFFFFFF;
               firstResponder->onKeyUp(evt);
            }
         }
         if(firstResponder != this || prevResponder != this)
            return true;
         break;
   }
   if (willCaptureAllInputs)
      return true;
   return false;
}

void Canvas::rootMouseDown(const Event &event)
{
   prevMouseTime = GetTickCount();
   mouseButtonDown = true;
   if(mouseCapturedControl)
      mouseCapturedControl->onMouseDown(event);
   else if(topContentControl)
      (topContentControl->findHitControl(event.ptMouse))->onMouseDown(event);
   if (mouseControl) mouseControlClicked = TRUE;
}

void Canvas::rootMouseUp(const Event &event)
{
   prevMouseTime = GetTickCount();
   mouseButtonDown = false;
   if(mouseCapturedControl)
      mouseCapturedControl->onMouseUp(event);
   else if(topContentControl)
      (topContentControl->findHitControl(event.ptMouse))->onMouseUp(event);
}

void Canvas::rootMouseDragged(const Event &event)
{
   if(mouseCapturedControl)
      mouseCapturedControl->onMouseDragged(event);
   else if(topContentControl)
   {
      Control *controlHit = topContentControl->findHitControl(event.ptMouse);
      if(controlHit != mouseControl)
      {
         if(mouseControl)
            mouseControl->onMouseLeave(event);
         mouseControl = controlHit;
         mouseControl->onMouseEnter(event);
      }
      mouseControl->onMouseDragged(event);
   }
}

void Canvas::rootMouseMove(const Event &event)
{
   if(mouseCapturedControl)
      mouseCapturedControl->onMouseMove(event);
   else if(topContentControl)
   {
      Control *controlHit = topContentControl->findHitControl(event.ptMouse);
      if(controlHit != mouseControl)
      {
         if(mouseControl)
            mouseControl->onMouseLeave(event);
         prevMouseTime = GetTickCount();
         mouseControl = controlHit;
         mouseControlClicked = FALSE;
         mouseControl->onMouseEnter(event);
      }
      mouseControl->onMouseMove(event);
   }
}

void Canvas::rootRightMouseDown(const Event &event)
{
   prevMouseTime = GetTickCount();
   mouseRightButtonDown = true;
   if(mouseCapturedControl)
      mouseCapturedControl->onRightMouseDown(event);
   else if(topContentControl)
      (topContentControl->findHitControl(event.ptMouse))->onRightMouseDown(event);
}

void Canvas::rootRightMouseUp(const Event &event)
{
   prevMouseTime = GetTickCount();
   mouseRightButtonDown = false;
   if(mouseCapturedControl)
      mouseCapturedControl->onRightMouseUp(event);
   else if(topContentControl)
      (topContentControl->findHitControl(event.ptMouse))->onRightMouseUp(event);
}

void Canvas::rootRightMouseDragged(const Event &event)
{
   prevMouseTime = GetTickCount();
   if(mouseCapturedControl)
      mouseCapturedControl->onRightMouseDragged(event);
   else if(topContentControl)
   {
      Control *controlHit = topContentControl->findHitControl(event.ptMouse);
      if(controlHit != mouseControl)
      {
         if(mouseControl)
            mouseControl->onMouseLeave(event);
         mouseControl = controlHit;
         mouseControlClicked = FALSE;
         mouseControl->onMouseEnter(event);
      }
      mouseControl->onRightMouseDragged(event);
   }
}


void Canvas::setDelegate(Delegate *del)
{
   // set our delegate object...
#if 0 // delegate is now before canvas in responder chain
	if(del)
		setNextResponder(del);
#endif
	
   delegate = del;
}

Delegate* Canvas::getDelegate()
{
   return delegate;
}

void Canvas::makeFirstResponder(Control *newFirstResponder)
{
   if(!firstResponder || !newFirstResponder || firstResponder->loseFirstResponder())
   {
      Responder *prevFirstResponder = firstResponder;
      firstResponder = newFirstResponder;
      if (firstResponder)
      {
         if(!firstResponder->becomeFirstResponder())
            firstResponder = this;
      }
      else
         firstResponder = this;

      // if we have a different first responder and
      // we are transitioning to the canvas or away from the canvas, then 
      // flush the made keys in the action handler
      if ((prevFirstResponder != firstResponder) && ((firstResponder == NULL) || (prevFirstResponder == this)) )
      {
         SimActionHandler *sah = SimActionHandler::find(manager);
         if (sah)
         {
            nextKeyTime = 0xFFFFFFFF;
            sah->breakAllMadeKeys();
         }
      }
   }
}

Control* Canvas::getContentControl()
{
   if(!contentControlStack.size())
      return NULL;
   else
      return contentControlStack.first();
}

void Canvas::setContentControl(Control *control, Delegate *newDlgt, bool firstTime, const char *dlgtName)
{
   mouseCapturedControl = NULL;
   mouseControl = NULL;
   mouseControlClicked = FALSE;
   resetUpdateRegions();
   // force pop all dialogs
   while (contentControlStack.size() > 1)
   {
      if(delegate && delegate->onDialogPop(topContentControl, 0) == -1)
      {
         AssertFatal(0, "could not pop all dialogs in order to switch to new content control");
      }
      popDialogControl();
   }
	
	// if newDlgt was created, set it as the delegate and
	// remove the old one.
	if (newDlgt)
	{
		Delegate *oldDlgt = getDelegate();
      if (newDlgt != oldDlgt)
      {
		   if (oldDlgt) 
		   {
		      oldDlgt->onLoseDelegation();
		      setDelegate(NULL);
            if (oldDlgt->deleteOnLoseContent())
            {
   		      manager->deleteObject(oldDlgt);
            }
		   }
         if (firstTime)
         {
            manager->addObject(newDlgt);
            if (dlgtName) manager->assignName(newDlgt, dlgtName);
         }
		   setDelegate(newDlgt);
		   newDlgt->onGainDelegation();
	   }
	}


   if(contentControlStack.size() == 1)
      contentControlStack.pop_back();

   topContentControl = control;
   if (control)
   {
      makeFirstResponder(NULL);
      contentControlStack.push_back(control);
      control->setCanvas(this);

		// now, if we have a delegate, we insert it before 
		// the canvas in the responder chain
		if (delegate)
		{
			control->setNextResponder(delegate);
			delegate->setNextResponder( this );
		}
		else
			control->setNextResponder(this);

      control->awaken();
      rebuildTabList();
      resetMouse();
      tabNext();
   }  
}

void Canvas::resetMouse()
{
   makeFirstResponder(NULL);
   Event evt;
   evt.ptMouse.x = int(cursorPt.x);
   evt.ptMouse.y = int(cursorPt.y);
   evt.modifier = 0;
   evt.mouseDownCount = 0;
	lastMouseDownCount = 0;
	lastMouseDownTime = 0;
   mouseButtonDown = false;
   mouseRightButtonDown = false;
   rootMouseMove(evt);
}   

void Canvas::mouseLock(Control *lockingControl)
{
   mouseCapturedControl = lockingControl;
}

void Canvas::mouseUnlock()
{
   Control *controlHit;
   Event evt;
   
   evt.ptMouse.x = int(cursorPt.x);
   evt.ptMouse.y = int(cursorPt.y);
   
   controlHit = (topContentControl == NULL) ? NULL : topContentControl->findHitControl(evt.ptMouse);
   
   if(controlHit != mouseCapturedControl)
   {
      if (mouseCapturedControl)
         mouseCapturedControl->onMouseLeave(evt);
      mouseControl = controlHit;
      mouseControlClicked = FALSE;
      if (mouseControl)
         mouseControl->onMouseEnter(evt);
   }
   mouseCapturedControl = NULL;
}

void Canvas::setHelpCtrl(HelpCtrl *ctrl)
{
   if (helpCtrl) delete helpCtrl;
   helpCtrl = ctrl;
   if (helpCtrl) helpCtrl->setCanvas(this);
}

void Canvas::paint()
{
   resetUpdateRegions();
   GWCanvas::paint();
}

void Canvas::flip()
{
   if(current)
   {
      current->flip(&canvasUpdateRegion);
   }
}

void Canvas::render()
{
   if(!contentControlStack.size() || ::IsIconic(getHandle()))
      return;

   GFXSurface *srf = getSurface();
   AssertWarn(srf, "Tried to render w/ a null surface!");
   if (srf == NULL)
      return;
   if(srf->getWidth() < 10 || srf->getHeight() < 10)
      return;

	Vector<Control *>::iterator ct;
	Delegate *lastDlgt = NULL; // call preRender for all delegates
	for(ct = contentControlStack.begin(); ct != contentControlStack.end(); ct++)
   {
      Control *contentControl = *ct;
		Delegate *dlgt = dynamic_cast<Delegate *>( contentControl->getNextResponder() ); 
      if( dlgt && (dlgt != lastDlgt ) ) // make sure preRender was not already called
			dlgt->preRender();
		lastDlgt = dlgt;
   } 
   
   // dispatch key/mouse repeat events here
   DWORD time = GetTickCount();
   if(time > nextKeyTime && firstResponder != this)
   {
      firstResponder->onKeyRepeat(lastKeyEvent);

      // Tabs do not repeat
      if (lastKeyEvent.diKeyCode == DIK_TAB) {
         nextKeyTime = 0xFFFFFFFF;
      } else {
         nextKeyTime = time + repeatKeyDelay;
      }
   }
   if(time > nextMouseTime && mouseCapturedControl)
   {
      lastMouseEvent.ptMouse.set(cursorPt.x, cursorPt.y);
      mouseCapturedControl->onMouseRepeat(lastMouseEvent);
   }

   ColorF alphaBlend(0,0,0);

   srf->setSurfaceAlphaBlend(&alphaBlend, 0);
   srf->setZTest(false);
   srf->setHazeSource(GFX_HAZE_NONE);
   srf->setAlphaSource(GFX_ALPHA_NONE);
   srf->setShadeSource(GFX_SHADE_NONE);   

   Point2I offset(0,0);
   Box2I screenRect(0,0,srf->getWidth(), srf->getHeight());
   

   if(editControl) // || CMDConsole::getLocked()->isActive())
      resetUpdateRegions();

   for(ct = contentControlStack.begin(); ct != contentControlStack.end(); ct++)
   {
      Control *contentControl = *ct;
      Point2I extent = contentControl->getExtent();
      Point2I pos = contentControl->getPosition();

      if(pos != screenRect.fMin || extent != screenRect.fMax)
      {
         contentControl->resize(screenRect.fMin, screenRect.fMax);
         resetUpdateRegions();
      }
      contentControl->preRender();
   } 

   // determine the cursor to draw
   Int32 mouseTag = (mouseCapturedControl) ? mouseCapturedControl->getMouseCursorTag() :
                                 ((mouseControl) ? mouseControl->getMouseCursorTag() : 0);
   if (mouseTag)
   {
      if (mouseTag != controlCursor.simTag)
      {
         setCursor(controlCursor, mouseTag);
         // flush all the bitmaps associated with the cursor
         if ((bool)controlCursor.hBmp)
            srf->flushTexture(controlCursor.hBmp, true);
         else
         {
            if ((bool)controlCursor.rsBMA)
            {
               for (int i = 0; i < controlCursor.rsBMA->numBitmaps; i++)
                  srf->flushTexture(controlCursor.rsBMA->getBitmap(i), true);
            }
         }
      }
   }
   Cursor *currentCursor = (mouseTag) ? &controlCursor : &defaultCursor;
   
   //determine if a help ctrl should be drawn
   Int32 helpTag = 0;
   const char *helpText = NULL;
   if (mouseControl && cursorON && (! mouseCapturedControl) && (! mouseButtonDown) && (! mouseRightButtonDown))
   {
      helpTag  = mouseControl->getHelpTag (time - prevMouseTime);
      helpText = mouseControl->getHelpText(time - prevMouseTime);
   }
   if (helpCtrl) 
   {
      helpCtrl->setHelpTag (helpTag,  time - prevMouseTime, mouseControlClicked);
      helpCtrl->setHelpText(helpText, time - prevMouseTime, mouseControlClicked);
   }
   if (delegate) 
   {
      delegate->setHelpTag (helpTag,  time - prevMouseTime, mouseControlClicked);
      delegate->setHelpText(helpText, time - prevMouseTime, mouseControlClicked);
   }

   Point2I cursorPos(cursorPt.x, cursorPt.y);

   if(lastCursorON && lastCursor && ((bool)lastCursor->hBmp || ((bool)lastCursor->rsBMA && lastCursor->rsBMA->numBitmaps >= lastCursor->iFrame)))
   {
      Point2I pos = lastCursorPt - lastCursor->hotspot;

      GFXBitmap *bmp = ((bool)lastCursor->hBmp) ? 
         ((GFXBitmap *)lastCursor->hBmp) : (lastCursor->rsBMA->getBitmap(lastCursor->iFrame));

      addUpdateRegion(pos - Point2I(2, 2), Point2I(bmp->getWidth() + 4, bmp->getHeight() + 4));
   }
   if(cursorON && currentCursor && ((bool)currentCursor->hBmp || ((bool)currentCursor->rsBMA && currentCursor->rsBMA->numBitmaps >= currentCursor->iFrame)))
   {
      Point2I pos = cursorPos - currentCursor->hotspot;

      GFXBitmap *bmp = ((bool)currentCursor->hBmp) ? 
         ((GFXBitmap *)currentCursor->hBmp) : (currentCursor->rsBMA->getBitmap(currentCursor->iFrame));
      
      addUpdateRegion(pos - Point2I(2, 2), Point2I(bmp->getWidth() + 4, bmp->getHeight() + 4));
   }
   lastCursor = currentCursor;
   lastCursorON = cursorON;
   lastCursorPt = cursorPos;

   RectI updateUnion;
   buildUpdateUnion(&updateUnion);
   
   if(updateUnion.upperL.x < screenRect.fMin.x)
      updateUnion.upperL.x = screenRect.fMin.x;
   if(updateUnion.upperL.y < screenRect.fMin.y)
      updateUnion.upperL.y = screenRect.fMin.y;
   if(updateUnion.lowerR.x > screenRect.fMax.x)
      updateUnion.lowerR.x = screenRect.fMax.x;
   if(updateUnion.lowerR.y > screenRect.fMax.y)
      updateUnion.lowerR.y = screenRect.fMax.y;

   canvasUpdateRegion = updateUnion;

   if(updateUnion.lowerR.x > updateUnion.upperL.x &&
      updateUnion.lowerR.y > updateUnion.upperL.y)
   {
      screenRect(updateUnion.upperL.x, updateUnion.upperL.y,
                 updateUnion.lowerR.x, updateUnion.lowerR.y);

      updateUnion.lowerR.x--;
      updateUnion.lowerR.y--;

      for(ct = contentControlStack.begin(); ct != contentControlStack.end(); ct++)
      {
         srf->setClipRect(&updateUnion);
         (*ct)->onRender(srf, offset, &screenRect);
      }
      if(editControl)
      {
         srf->setClipRect(&updateUnion);
         editControl->onRender(srf, offset, &screenRect);
      }
      srf->setClipRect(&updateUnion);

      if (cursorON && currentCursor && cursorOverApp && cursorVisible)
      {
         // First draw the help panel if required
         if (helpCtrl) 
         {
            helpCtrl->render(srf);
         }

         GFXBitmap *bmp = NULL;

         if ((bool)currentCursor->hBmp)
         {
            bmp = (GFXBitmap *)currentCursor->hBmp;
         }
         else if ((bool)currentCursor->rsBMA)
         {
            bmp = (*currentCursor->rsBMA)[currentCursor->iFrame];

            float rDelta = manager->getCurrentTime() - rLastFrameTime;

            if (rDelta > currentCursor->rFrameTime)
            {
               currentCursor->iFrame ++;

               if (currentCursor->iFrame >= currentCursor->rsBMA->getCount())
               {
                  currentCursor->iFrame = 0;
               }

               rLastFrameTime = manager->getCurrentTime();
            }
         }

         if (bmp)
         {
            Point2I pos(cursorPt.x, cursorPt.y);

            pos -= currentCursor->hotspot;
            srf->drawBitmap2d(bmp, &pos);
         }
      }
   }

   // Render the GUI console
   SimGame::get()->getConsole()->render(srf);
   Parent::render();
}

void Canvas::editMode(bool mode)
{
   if(mode && !editControl)
   {
      editControl = new EditControl();
      manager->addObject(editControl, "EditControl");
      editControl->setCanvas(this);
      mouseLock(editControl);
      firstResponder = editControl;
   }
   else if(!mode && editControl)
   {
      editControl->deleteObject();
      editControl = NULL;
      mouseUnlock();
      firstResponder = this;
      rebuildTabList();
      tabNext();
   }
}

void Canvas::invalidateControl(Control *ctrl)
{
   if(ctrl == mouseControl)
      mouseControl = NULL;
   if(ctrl == editControl)
      editControl = NULL;
   if(ctrl == firstResponder)
      firstResponder = this;
   if(ctrl == mouseCapturedControl)
      mouseCapturedControl = NULL;
   if(ctrl == topContentControl)
      topContentControl = NULL;
}

void Canvas::onKeyDown(const Event &event)
{
   switch(event.diKeyCode)
   {
      case DIK_TAB:
         if(event.modifier & SI_SHIFT)
            tabPrev();
         else
            tabNext();
         break;
      case DIK_RIGHT:
      case DIK_DOWN:
         if(!topContentControl->flags.test(Control::ArrowsDontMakeFirstResponder))
            tabNext();
         break;
      case DIK_LEFT:
      case DIK_UP:
         if(!topContentControl->flags.test(Control::ArrowsDontMakeFirstResponder))
            tabPrev();
         break;
      default:
         Responder::onKeyDown(event);
   }

}

void Canvas::tabPrev()
{
   if(!tabList.size())
      return;
   if(tabList.size() == 1)
   {
      makeFirstResponder(*tabList.begin());
      return;
   }
   // find the current tabbed item
   Vector<Control *>::iterator i;
   for(i = tabList.begin(); i != tabList.end(); i++)
      if(*i == firstResponder)
         break;
   Vector<Control *>::iterator j;

   if(i == tabList.end())
      i = tabList.begin();
   j = i;
   
   do
   {
      if(j == tabList.begin())
         j = tabList.end();
      j--;
      Control *ct = dynamic_cast<Control*>(*j);
      makeFirstResponder(ct);
      if(firstResponder == ct)
         return;
   } while(j != i);
}

void Canvas::tabNext()
{
   if(!tabList.size())
      return;
   if(tabList.size() == 1)
   {
      makeFirstResponder(*tabList.begin());
      return;
   }
   // find the current tabbed item
   Vector<Control *>::iterator i;
   for(i = tabList.begin(); i != tabList.end(); i++)
      if(*i == firstResponder)
         break;
   Vector<Control *>::iterator j;

   if(i == tabList.end())
      i--;
   j = i;
   
   do
   {
      j++;
      if(j == tabList.end())
         j = tabList.begin();
      Control *ct = dynamic_cast<Control*>(*j);
      makeFirstResponder(ct);
      if(firstResponder == ct)
         return;
   } while(j != i);
}

void Canvas::addTabs(Control *ctrl)
{
   Control::iterator i;
	if(ctrl->isVisible())
   {
		if(ctrl->wantsTabListMembership())
			tabList.push_back(ctrl);
		for(i = ctrl->begin(); i != ctrl->end(); i++)
			addTabs(dynamic_cast<Control *>(*i));
	}
}

void Canvas::rebuildTabList(void)
{
   tabList.clear();
   addTabs(topContentControl);
}

void Canvas::pushDialogControl(Control *ctrl, Delegate *dlgDelegate /* = NULL */)
{
	if( dlgDelegate ) // dialogs can have delegates now
	{
		manager->addObject(dlgDelegate);
		setDelegate( dlgDelegate );
	}

   resetUpdateRegions();
   makeFirstResponder(NULL);
   mouseUnlock();

	//if has a new delegate, add to responder chain 
	//(we kind of have a stack of delegates within the responder chain)
	Responder *topResp = topContentControl ? topContentControl->getNextResponder() : NULL; 
	Responder *resp = (dlgDelegate != NULL) ? dlgDelegate : topResp;
	if( resp && (resp != topResp) ) // if we have a new delegate 
		resp->setNextResponder( topResp ); 

   contentControlStack.push_back(ctrl);
   topContentControl = ctrl;
   topContentControl->setCanvas(this);
   topContentControl->setNextResponder(resp);
   topContentControl->awaken();
   rebuildTabList();
   resetMouse();
   tabNext();

	if( dlgDelegate ) //call after topContentControl is set to the new control
      dlgDelegate->onGainDelegation();

}

void Canvas::popDialogControl()
{
   resetUpdateRegions();
   if (contentControlStack.size() <= 1)
      return;

	// now must look at responder chain to see if the dialog being popped
	// has a delegate... so get the responders...
	Responder *oldResp = topContentControl->getNextResponder();
   Control *oldDialog = topContentControl;
   if (oldDialog && oldDialog->deleteOnLoseContent())
      manager->deleteObject(oldDialog);
   contentControlStack.pop_back();
   topContentControl = contentControlStack.last();

	// now check to see if have same next responder.... if not, delete old one if it is a delegate
	if( oldResp != topContentControl->getNextResponder() )
	{
		Delegate *oldDlgt = (oldResp != NULL) ? dynamic_cast<Delegate *>(oldResp) : NULL;
		if( oldDlgt ) // if responder was a delegate...
		{
			oldDlgt->onLoseDelegation();
			Delegate *newDlgt = dynamic_cast<Delegate *>(topContentControl->getNextResponder());
			setDelegate( newDlgt ); // set it, even if NULL
			manager->deleteObject(oldDlgt);
			if( newDlgt )
				newDlgt->onRegainTopDelegation(); // notify delegate that it has regained top of stack
		}
	}

	makeFirstResponder( NULL ); // just in case tabNext() fails to set to something
   rebuildTabList();
   resetMouse();
   tabNext();
}

Control* Canvas::getTopDialog()
{
   if(contentControlStack.size() < 2)
      return NULL;
   else
      return contentControlStack.last();
}

Control* Canvas::getDialogNumber(int stackNumber)
{
   if (stackNumber >= contentControlStack.size()) return NULL;
   return contentControlStack[stackNumber];
}

int Canvas::findDialogNumber(Control *ctrl)
{
   const Control *rootCtrl = ctrl->getTopMostParent();
   for (int i = 0; i < contentControlStack.size(); i++)
   {
      if (contentControlStack[i] == rootCtrl) return i;
   }
   return 0;
}

DWORD Canvas::onMessage(SimObject *, DWORD msg)
{
   if (editControl)
      return 0;

   if (!manager) return msg;
   
   // if the delegate didn't handle the message,
   // then do default processing
   if (msg)
   {
      const char* cmd = SimTagDictionary::getString(manager, msg); 
      if(msg > IDRES_BEG_GUI && msg < IDRES_END_GUI)
      {
         AssertWarn(cmd,avar("SimGui::Canvas failed to handle undefined message %d", msg));
         if( cmd )
            handleGUIMessage(cmd);
         return 1;
      }
      else if(msg > IDRES_BEG_DLG && msg < IDRES_END_DLG)
      {
         switch(msg)
         {
            case IDDLG_OK:
            case IDDLG_CANCEL:
               handleDLGClose(msg);
               break;
            default:
               AssertWarn(cmd,avar("SimGui::Canvas failed to handle undefined message %d", msg));
               if( cmd )
                  handleDLGMessage(cmd, msg);
               break;
         }
         return 1;
      }
      else if(msg > IDRES_BEG_CMD && msg < IDRES_END_CMD)
      {
         AssertWarn(cmd,avar("SimGui::Canvas failed to handle undefined message %d", msg));
         if( cmd )
            CMDConsole::getLocked()->evaluate(cmd, false);
         return 1;
      }
   }

   return 0; 
}  

void Canvas::handleDLGClose(Int32 message)
{
   Delegate *delegate = getDelegate();
   if(delegate && delegate->onDialogPop(topContentControl, message) == -1)
      return;
   popDialogControl();
}

void Canvas::handleGUIMessage(const char *guiName)
{
   ResourceManager *rm = SimResource::get(manager);
   CMDConsole *console = CMDConsole::getLocked();

   if (guiName && guiName[0] && rm)
   {
      // if a delegate with the same name as the gui exists,
      // then instantiate it and set it as the new delegate.
      
      // construct Delgate Class name from guiName
      char delegateClassName[256];
      char guiObjectName[256];

      strcpy(delegateClassName, "SimGui::");
      const char *guiStripName = strrchr(guiName, '\\');
      if(!guiStripName)
         guiStripName = guiName;
      else
         guiStripName++;

      strcpy(guiObjectName, guiStripName);

      strcat(delegateClassName, guiStripName);
      char *extension = strchr(delegateClassName, '.');
      strcpy(extension, "Delegate");

      extension = strchr(guiObjectName, '.');
      strcpy(extension, "Gui");
   
      // first, see if the gui we want is already loaded...

      SimObject *temp = manager->findObject(guiObjectName);
      Control *newGui = NULL;
      Delegate *newDlgt = NULL;
      bool firstTime = TRUE;
      Control *oldGui = getContentControl();
      if (temp)
      {
         newGui = dynamic_cast<Control *>(temp);
         if(newGui == oldGui && oldGui->deleteOnLoseContent()) // force a reload
            newGui = NULL;
      }
      
      if (oldGui && oldGui->deleteOnLoseContent())
         oldGui->close();

      if (! newGui)
      {
         newGui = Control::load(manager, guiName);

         if(!(bool)newGui)
         {
            console->printf("Could not load gui: %s",guiName);
            return;
         }
         manager->addObject(newGui);
         manager->assignName(newGui, guiObjectName);
         
      }

      if (newGui != oldGui)
      {
         
         SimObject *temp = manager->findObject(delegateClassName);
         if (temp)
         {
            newDlgt = dynamic_cast<Delegate *>(temp);
            firstTime = FALSE;
         }
         else
         {
            // attempt to create the delegate
            SimObject *o = (SimObject *) Persistent::AbstractTaggedClass::create(delegateClassName);
            newDlgt = (o != NULL) ? dynamic_cast<Delegate *>(o) : NULL;
            firstTime = TRUE;
         }
         if (newDlgt && newGui)
         {
            newDlgt->deleteOnLoseContent(newGui->deleteOnLoseContent());
         }
      }

      // next load the new gui

      char scriptName[256];
      if(oldGui)
      {
         sprintf(scriptName, "%s::onClose", oldGui->getName());
         if(Console->isFunction(scriptName))
            Console->executef(1, scriptName);
      }
      setContentControl(newGui, newDlgt, firstTime, delegateClassName);

      if (delegate) 
         delegate->onNewContentControl(oldGui, newGui);

      sprintf(scriptName, "%s::onOpen", newGui->getName());
      if(Console->isFunction(scriptName))
         Console->executef(1, scriptName);

      if (oldGui && oldGui->deleteOnLoseContent() && oldGui != newGui)
         manager->deleteObject(oldGui);
   }
}   

void Canvas::handleDLGMessage(const char *guiName, Int32 message)
{
   ResourceManager *rm = SimResource::get(manager);

   if (guiName && rm)
   {
      // first load the new gui
      Control *newGui = Control::load(manager, guiName);
      AssertFatal((bool)newGui, avar("Error loading gui: %s",guiName))
      manager->addObject(newGui);

      Delegate *delegate = getDelegate();
      if(delegate)
         delegate->onDialogPush(topContentControl, newGui, message);

		// construct potential new delegate name for the dialog
		char delegateClassName[256];

      strcpy(delegateClassName, "SimGui::");
      const char *guiStripName = strrchr(guiName, '\\');
      if(!guiStripName)
         guiStripName = guiName;
      else
         guiStripName++;

      strcat(delegateClassName, guiStripName);
      char *extension = strchr(delegateClassName, '.');
      strcpy(extension, "Delegate");

		// attempt to create the new delegate for this dialog
      SimObject *o = (SimObject *) Persistent::AbstractTaggedClass::create(delegateClassName);
      Delegate *newDlgt = (o != NULL) ? dynamic_cast<Delegate *>(o) : NULL;
		
      pushDialogControl(newGui, newDlgt);
   }
}   

void Canvas::buildUpdateUnion(RectI *updateUnion)
{
   *updateUnion = oldUpdateRects[0];

   if(oldUpdateRects[1].lowerR.x && updateUnion->lowerR.x)
   {
      if(oldUpdateRects[1].upperL.x < updateUnion->upperL.x)
         updateUnion->upperL.x = oldUpdateRects[1].upperL.x;
      if(oldUpdateRects[1].upperL.y < updateUnion->upperL.y)
         updateUnion->upperL.y = oldUpdateRects[1].upperL.y;
      if(oldUpdateRects[1].lowerR.x > updateUnion->lowerR.x)
         updateUnion->lowerR.x = oldUpdateRects[1].lowerR.x;
      if(oldUpdateRects[1].lowerR.y > updateUnion->lowerR.y)
         updateUnion->lowerR.y = oldUpdateRects[1].lowerR.y;
   }
   else if(oldUpdateRects[1].lowerR.x)
      *updateUnion = oldUpdateRects[1];

   if(curUpdateRect.lowerR.x && updateUnion->lowerR.x)
   {
      if(curUpdateRect.upperL.x < updateUnion->upperL.x)
         updateUnion->upperL.x = curUpdateRect.upperL.x;
      if(curUpdateRect.upperL.y < updateUnion->upperL.y)
         updateUnion->upperL.y = curUpdateRect.upperL.y;
      if(curUpdateRect.lowerR.x > updateUnion->lowerR.x)
         updateUnion->lowerR.x = curUpdateRect.lowerR.x;
      if(curUpdateRect.lowerR.y > updateUnion->lowerR.y)
         updateUnion->lowerR.y = curUpdateRect.lowerR.y;
   }
   else if(curUpdateRect.lowerR.x)
      *updateUnion = curUpdateRect;

   oldUpdateRects[0] = oldUpdateRects[1];
   oldUpdateRects[1] = curUpdateRect;
   curUpdateRect.lowerR.set(0,0);
   curUpdateRect.upperL.set(0,0);
}

void Canvas::addUpdateRegion(Point2I pos, Point2I ext)
{
   if(curUpdateRect.lowerR.x == 0)
   {
      curUpdateRect.upperL.x = pos.x;
      curUpdateRect.upperL.y = pos.y;
      curUpdateRect.lowerR.x = pos.x + ext.x;
      curUpdateRect.lowerR.y = pos.y + ext.y;
   }
   else
   {
      if(pos.x < curUpdateRect.upperL.x)
         curUpdateRect.upperL.x = pos.x;
      if(pos.y < curUpdateRect.upperL.y)
         curUpdateRect.upperL.y = pos.y;
      if(pos.x + ext.x > curUpdateRect.lowerR.x)
         curUpdateRect.lowerR.x = pos.x + ext.x;
      if(pos.y + ext.y > curUpdateRect.lowerR.y)
         curUpdateRect.lowerR.y = pos.y + ext.y;
   }
}

void Canvas::surfaceChanged()
{
   Parent::surfaceChanged();

   GFXSurface *srf = getSurface();
   if (srf == NULL)
      return;

   resetUpdateRegions(); 
   canvasUpdateRegion.upperL.set(0,0);
   canvasUpdateRegion.lowerR.set(srf->getWidth() - 1, srf->getHeight() - 1);

   POINT pt;
   pt.x = srf->getWidth()/2;
   pt.y = srf->getHeight()/2;
   ClientToScreen( getHandle(), &pt );
   SetCursorPos( pt.x, pt.y );
}  

void Canvas::resetUpdateRegions()
{
   GFXSurface *srf = getSurface();
   if (srf == NULL)
      return;

   oldUpdateRects[0].upperL.set(0,0);
   oldUpdateRects[0].lowerR.set(srf->getWidth(),srf->getHeight());
   oldUpdateRects[1] = oldUpdateRects[0];
   curUpdateRect = oldUpdateRects[0];
}

void Canvas::setUseWindowsMouseEvents(bool use)
{
   useWindowsMouseEvents = use;
}
			
#ifdef WINKEYBOARD
void Canvas::setUseWindowsKeyboardEvents(bool use)
{
   useWindowsKeyboardEvents = use;
}
#endif


void Canvas::onMouseMove(const Event &theEvent)
{
   Responder::onMouseMove(theEvent);
}

void Canvas::onMouseMove(int x, int y, UINT keyFlags)
{
   if ( (!canvasCursorTrapped || !isFullScreen() || (GWMap::getWindowCount()>1)) && !cursorOverApp && !g_prefPoliteGui)
   {
      cursorOverApp = true;
      SetForegroundWindow(getHandle());
      SimInputManager *im = SimGame::get()->getInputManager();
      im->activate(SI_MOUSE,0);
      setCursorPos( Point2I(x,y) );
   }

   if(!useWindowsMouseEvents)
   {
      Parent::onMouseMove(x,y,keyFlags);
      return;
   }
   processMouseEvent(x, y, GWMouseEvent::MouseMove);
}   

UINT Canvas::onNCHitTest(int x, int y)
{
   if ( !cursorOverApp && (getHandle()==GetForegroundWindow()) )
   {
      UINT whereMouse = Parent::onNCHitTest(x,y);
      if ( whereMouse==HTCLIENT ) 
      {
         cursorOverApp = true;
         POINT pt;
         GetCursorPos( &pt );
         ScreenToClient(getHandle(),&pt);
         SimInputManager *im = SimGame::get()->getInputManager();
         im->activate(SI_MOUSE,0);
         setCursorPos( Point2I(pt.x,pt.y) );
      }
      return whereMouse;
   }
   return Parent::onNCHitTest(x,y);
}   

void Canvas::onEnterMenuLoop( bool )
{
   if ( !canvasCursorTrapped || !isFullScreen() || (GWMap::getWindowCount()>1) )
   {
      POINT pt;
      pt.x = cursorPt.x;
      pt.y = cursorPt.y;
      ClientToScreen( getHandle(), &pt );
      SimGame::get()->getInputManager()->deactivate(SI_MOUSE,0);
      cursorOverApp = false;
      SetCursorPos(pt.x,pt.y);
   }
}   

void Canvas::onLButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   if ( (!canvasCursorTrapped || !isFullScreen() || (GWMap::getWindowCount()>1)) && !cursorOverApp )
   {
      cursorOverApp = true;
      SimInputManager *im = SimGame::get()->getInputManager();
      im->activate(SI_MOUSE,0);
      setCursorPos( Point2I(x,y) );
   }
   if(!useWindowsMouseEvents)
   {
      Parent::onLButtonDown(fDoubleClick, x,y,keyFlags);
      return;
   }
   SetCapture(getHandle());
   processMouseEvent(x, y, GWMouseEvent::LButtonDown);
}   

void Canvas::onLButtonUp(int x, int y, UINT keyFlags)
{
   if(!useWindowsMouseEvents)
   {
      Parent::onLButtonUp(x,y,keyFlags);
      return;
   }
   ReleaseCapture();
   processMouseEvent(x, y, GWMouseEvent::LButtonUp);
}   

void Canvas::onRButtonDown(BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   if(!useWindowsMouseEvents)
   {
      Parent::onRButtonDown(fDoubleClick, x,y,keyFlags);
      return;
   }
   SetCapture(getHandle());
   processMouseEvent(x, y, GWMouseEvent::RButtonDown);
}   

void Canvas::onRButtonUp(int x, int y, UINT flags)
{
   if(!useWindowsMouseEvents)
   {
      Parent::onRButtonUp(x,y,flags);
      return;
   }
   ReleaseCapture();
   processMouseEvent(x, y, GWMouseEvent::RButtonUp);
}

#ifdef WINKEYBOARD

void  Canvas::onKey(UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	// This is hack to allow the regular window keyboard message handling to generate
	// the same events as DirectInput.  It DOES NOT WORK right in all cases, specifically
	// when there is a repeat code, and when you push multiple keys at once.  But its
	// better than nothing...

	SimCanvas::onKey(vk, fDown, cRepeat, flags);
	if (!useWindowsKeyboardEvents) return;

	// Make an appropriate event and send it to the input set.
	SimInputEvent event; 

	event.deviceType = SI_KEYBOARD;
	event.deviceInst	= 0;		// Pretend to be keyboard 0.
	event.objType    = SI_KEY;

	BYTE keyStates[256];

	// Get the state of all the keys.  We'll need this now (for the modifier keys) and
	// later for the call to get ascii:
	GetKeyboardState (keyStates);

	// Set the modifier flags.  SimInputManager ignores these, but somebody might not.
	event.modifier = 0;
	if (keyStates [VK_LSHIFT] & (1 << 31))			event.modifier |= (SI_LSHIFT | SI_SHIFT);
	if (keyStates [VK_RSHIFT] & (1 << 31))			event.modifier |= (SI_RSHIFT | SI_SHIFT);
	if (keyStates [VK_LCONTROL] &  (1 << 31))		event.modifier |= (SI_LCTRL | SI_CTRL);
	if (keyStates [VK_RCONTROL] &  (1 << 31))		event.modifier |= (SI_RCTRL | SI_CTRL);
	// Can't get state of alt key from keyStates....

	event.action = (fDown) ? SI_MAKE : SI_BREAK;
	event.fValue = (fDown) ? 1.0f : 0.0f;

	// Tricky part is filling in event.objInst.  SimInputManager will turn this into an
	// ascii code later by calling getAscii.  So we need to pass a value for objInst that
	// gives the correct ascii code:

	SimInputManager *im = dynamic_cast<SimInputManager*>(manager->findObject(SimInputManagerId));
	if (!im) return;


	// The WM_KEY* message don't distinguish left and right shift, control and alt.
	// So we'll pretend its the left:
	if		(vk == VK_SHIFT)	event.objInst = DIK_LSHIFT;
	else if (vk == VK_CONTROL)	event.objInst = DIK_LCONTROL;
	else if (vk == VK_MENU)		event.objInst = DIK_RMENU;
	else if (fDown)
	{
		// Convert the virtual key code to ASCII:
		WORD c;
		if (ToAscii (vk, flags, keyStates, &c, FALSE) == 1)
		{
			// It has an ASCII code.  
			event.ascii  = (char) c;	// SimInputManager also ignores ascii....
			event.objInst = im->getDIK_CODE ((char) c);
		}
		else 
		{
			event.ascii = 0;
			// No ASCII code, its something weird.  
			switch (vk)
			{
				case VK_LEFT:
					event.objInst = DIK_LEFTARROW;
					break;
				case VK_UP:    
					event.objInst = DIK_UPARROW;
					break;
				case VK_RIGHT:
					event.objInst = DIK_RIGHTARROW;
					break;
				case VK_DOWN:
					event.objInst = DIK_DOWNARROW;
					break;

				default:
					// Don't pass it on.
					lastAscii = lastDIKCode = 0;
					return;
			}
		}
		// Save these for use on the break:
		lastAscii	= event.ascii;
		lastDIKCode	= event.objInst;
	}
	else
	{
		// This is a keyup.  We can't rely on ToAscii to do a proper translation.
		// So just send a break for the last make we received.
		if (lastAscii || lastDIKCode)
		{
			event.ascii	  = lastAscii;		// These we saved on the make.
			event.objInst = lastDIKCode;

			// If multiple keys are pressed at once, we can get confused.  So don't
			// send anything.
			lastAscii = lastDIKCode = 0;
		}
		else
		{
			return;
		}
	}


	im->processEvent (&event);
}
#endif // WINKEYBOARD

// From GWCanvas...
void
Canvas::lock()
{
   if (current) {
      if (contentControlStack.size()) {
         contentControlStack[0]->lockDevice(current);
      } else {
         current->lock();
      }
   }
}

void
Canvas::unlock()
{
   if (current) {
      if (contentControlStack.size()) {
         contentControlStack[0]->unlockDevice(current);
      } else {
         current->unlock();
      }
   }
}

void
Canvas::onActivateApp(BOOL f, DWORD t)
{
   if (f == FALSE) {
      SimGame::get()->getInputManager()->deactivate(SI_MOUSE,0);
      cursorOverApp = false;
      skipNextDelta = 0;
      SimGame::get()->getInputManager()->deactivate( SI_KEYBOARD, 0 );
   } else {
      SimGame::get()->getInputManager()->activate( SI_KEYBOARD, 0 );
   }

   Parent::onActivateApp(f, t);
}

void
Canvas::onSysCommand(UINT cmd, int x, int y)
{
   switch (cmd) {
     case SC_MAXIMIZE:
      cursorOverApp = true;
      SimGame::get()->getInputManager()->activate(SI_MOUSE,0);
     
     default:
      Parent::onSysCommand(cmd, x, y);
   }
}

};