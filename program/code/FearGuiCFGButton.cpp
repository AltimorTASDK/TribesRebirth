#include "console.h"
#include "g_bitmap.h"
#include "g_pal.h"
#include "g_surfac.h"
#include "simGame.h"
#include "inspect.h"
#include "Darkstar.Strings.h"
#include "editor.strings.h"
#include "fear.strings.h"

#include "FearGuiShellPal.h"
#include "FearGuiCFGButton.h"
#include "simResource.h"

namespace FearGui
{

static const int gCtrlVersion = 0;

//----------------------------------------------------------------------------
struct PairDef
{
   char *name;
   BYTE  type;
};

static PairDef DeviceTypes[] = 
{
   { "Keyboard",  SI_KEYBOARD },
   { "Mouse",     SI_MOUSE },
   { "Joystick",  SI_JOYSTICK },
   { "Device",    SI_DEVICE },
   { 0,           0 }
};

static PairDef ActionTypes[] =
{
   { "make",      SI_MAKE },
   { "break",     SI_BREAK },
   { "move",      SI_MOVE },
   { 0,           0 }
};   

static PairDef ObjTypes[] =
{
   { "X-Axis",     SI_XAXIS },
   { "Y-Axis",     SI_YAXIS },
   { "Z-Axis",     SI_ZAXIS },
   { "RX-Aaxis",    SI_RXAXIS },
   { "RY_Axis",    SI_RYAXIS },
   { "RZ-Axis",    SI_RZAXIS },
   { "Slider",    SI_SLIDER },
   { "POV",       SI_POV },
   { "X-POV",      SI_XPOV },
   { "Y-POV",      SI_YPOV },
   { "U-POV",      SI_UPOV },
   { "D-POV",      SI_DPOV },
   { "L-POV",      SI_LPOV },
   { "R-POV",      SI_RPOV },
   { "Button",    SI_BUTTON },
   { "Key",       SI_KEY },
   { 0,           0 }
};   

static PairDef ModifierTypes[] =
{
   { "\1",     SI_SHIFT },
   { "\2",      SI_CTRL },
   { "\3",       SI_ALT },
/*
   { "L SHIFT",   SI_LSHIFT },
   { "R SHIFT",   SI_RSHIFT },
   { "L CTRL",    SI_LCTRL },
   { "R CTRL",    SI_RCTRL },
   { "L ALT",     SI_LALT },
   { "R ALT",     SI_RALT },
*/   
   { 0,           0 }
};   

//------------------------------------------------------------------------------
static BYTE getPairType( PairDef *table, const char *str )
{
   for(;table->name; table++)
      if (strnicmp( str, table->name, strlen(table->name) ) == 0)
         break;
   return (table->type);
}   


//------------------------------------------------------------------------------
static char* getPairStr( PairDef *table, BYTE type )
{
   for(;table->name; table++)
      if (table->type == type)
         break;
   return (table->name);
}   

//------------------------------------------------------------------------------
static BYTE getInst( const char *str )
{
   const char *end = str+strlen(str);
   do {
      end--;
   }while (*end >= '0' && *end <= '9');
   end++;
   return (*end ? atoi(end) : 0);
}   

//------------------------------------------------------------------------------
static void getModifiers( char *buffer, BYTE &flags )
{
   PairDef *table;
   //first, make sure all Left and Right modifiers work together
   if (flags & SI_LSHIFT)
   {
      flags &= ~SI_LSHIFT;
      flags |= SI_SHIFT;
   }
   if (flags & SI_RSHIFT)
   {
      flags &= ~SI_RSHIFT;
      flags |= SI_SHIFT;
   }
   if (flags & SI_LALT)
   {
      flags &= ~SI_LALT;
      flags |= SI_ALT;
   }
   if (flags & SI_RALT)
   {
      flags &= ~SI_RALT;
      flags |= SI_RALT;
   }
   if (flags & SI_LCTRL)
   {
      flags &= ~SI_LCTRL;
      flags |= SI_CTRL;
   }
   if (flags & SI_RCTRL)
   {
      flags &= ~SI_RCTRL;
      flags |= SI_RCTRL;
   }
   
   BYTE tempFlags = flags;
   for (table = ModifierTypes; table->name; table++)
   {
      //if ((tempFlags & table->type) == table->type)
      if (tempFlags & table->type)
      {
         strcat(buffer, avar("%s", table->name));
         tempFlags &= ~table->type;
      }
   }
}   

//------------------------------------------------------------------------------

CFGButton::CFGButton(void) : SimGui::SimpleText()
{
   mActionMap[0] = '\0';
   mBinding[0] = '\0';
   
	mMakeAction = 0;
	mMakeValue = 0;
	mBreakAction = 0;
	mBreakValue = 0;
	mFlags = 0;
   ascii = 0;
   modifier = 0;
}

Int32 CFGButton::getMouseCursorTag(void)
{
   return (active ? IDBMP_CURSOR_HAND : 0);
}

float CFGButton::getMouseSensitivity(const char *actionMapName)
{
   //make sure we've loaded an action map and have a binding
   if (! actionMapName || ! actionMapName[0]) return 1.0f;

   SimManager* manager = SimGame::get()->getManager();
   Resource<SimActionMap> actionMap = SimResource::get(manager)->load(actionMapName);
   if (!(bool)actionMap)
   {
      Console->printf( "error: must call 'editActionMap' or 'newActionMap' first. (%s)", actionMapName );
      return -1.0f;
   }
   
   //search for the mouse xaxis event
   SimActionMap::Map::iterator i;
   for (i = actionMap->map.begin(); i != actionMap->map.end(); i++)
   {
      //make sure it's not a keyboard event
      if (i->event.deviceType == SI_KEYBOARD) continue;
      
      //make sure it's not a make or break
      if (i->event.action == SI_MAKE || i->event.action == SI_BREAK) continue;
      
      //make sure the modifier is 0
      if (i->event.modifier != 0) continue;
      
      //look for the x-axis
      if (i->event.objType == SI_XAXIS)
      {
         return i->action.scale;
      }
   }
   
   return -1.0f;
}

bool CFGButton::getMouseAxisFlip(const char *actionMapName, bool xAxis)
{
   //make sure we've loaded an action map and have a binding
   if (! actionMapName || ! actionMapName[0]) return FALSE;

   SimManager* manager = SimGame::get()->getManager();
   Resource<SimActionMap> actionMap = SimResource::get(manager)->load(actionMapName);
   if (!(bool)actionMap)
   {
      Console->printf( "error: must call 'editActionMap' or 'newActionMap' first. (%s)", actionMapName );
      return FALSE;
   }
   
   //search for the mouse xaxis event
   SimActionMap::Map::iterator i;
   for (i = actionMap->map.begin(); i != actionMap->map.end(); i++)
   {
      //make sure it's not a keyboard event
      if (i->event.deviceType == SI_KEYBOARD) continue;
      
      //make sure it's not a make or break
      if (i->event.action == SI_MAKE || i->event.action == SI_BREAK) continue;
      
      //make sure the modifier is 0
      if (i->event.modifier != 0) continue;
      
      //look for the x-axis
      if ((i->event.objType == SI_XAXIS && xAxis) || (i->event.objType == SI_YAXIS && (! xAxis)))
      {
         return (! i->flags.test(SimActionFlagFlip));
      }
   }
   
   return FALSE;
}

bool CFGButton::getMouseXAxisFlip(const char *actionMapName)
{
   return getMouseAxisFlip(actionMapName, TRUE);
}
bool CFGButton::getMouseYAxisFlip(const char *actionMapName)
{
   return getMouseAxisFlip(actionMapName, FALSE);
}

void CFGButton::onAction(void)
{
   //don't eval the console command - in this class, it's used to store a 'make bindCommand'
   const char *name = getName();
   if(name)
   {
      char buf[256];
      sprintf(buf, "%s::onAction", name);
      if(Console->isFunction(buf))
         Console->executef(1, buf);
   }
   if (message)
   {
      Parent::onMessage(this, message);
   }
}

void CFGButton::onPreRender(void)
{
}

void CFGButton::onRender(GFXSurface* sfc, Point2I offset, const Box2I& /*updateRect*/)
{
   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(1);
   if ((! active) ||
       (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG))))
   {
      ghosted = TRUE;
   }
   
   //draw the text
   Point2I bmpOffset;
   bmpOffset.y = offset.y;
   bmpOffset.y += hFontHL->getHeight() - 1;
   
   //draw the button bmp
   GFXBitmap *bmp = (ghosted ? mGhostedBMP : (stateDepressed || mbDefine ? mPressedBMP : mDefaultBMP));
   bmpOffset.x = offset.x + ((extent.x - bmp->getWidth() ) / 2);
   sfc->drawBitmap2d(bmp, &bmpOffset);
   
   //find the textWidth and the offset
   int textWidth = 0;
   textWidth += hFont->getStrWidth(text) + 1;
   bmpOffset.x = offset.x + ((extent.x - textWidth) / 2);
   bmpOffset.y = offset.y;
   
   //draw the text
   sfc->drawText_p(hFontShadow, &Point2I(bmpOffset.x - 1, bmpOffset.y + 1), text);
   sfc->drawText_p(ghosted ? hFontDisabled : (mBinding[0] ? hFontHL : hFontDisabled), &bmpOffset, text);
   bmpOffset.y += hFontHL->getHeight() + 1;
   
   //draw the action binding string
   char tempBinding[16];
   strcpy(tempBinding, mBinding);
   if (tempBinding[0])
   {
      //find the offset
      int modCount = 0;
      textWidth = 0;
      char *textPtr = tempBinding;
      while (*textPtr >= '\1' && *textPtr <= '\3')
      {
         modCount++;
         textWidth += mShiftIcon->getWidth() + 2;
         textPtr++;
      }
      if (modCount == 3) textPtr[3] = '\0';
      textWidth += hFont->getStrWidth(textPtr) + 1;
      bmpOffset.x = offset.x + ((extent.x - textWidth) / 2);
      
      //draw the icons
      textPtr = tempBinding;
      while (*textPtr >= '\1' && *textPtr <= '\3')
      {
         bmp = (*textPtr == '\1' ? mShiftIcon : (*textPtr == '\2' ? mCtrlIcon : mAltIcon));
         if (! ghosted) sfc->drawBitmap2d(bmp, &Point2I(bmpOffset.x, bmpOffset.y + 4));
         bmpOffset.x += bmp->getWidth() + 2;
         textPtr++;
      }
      
      sfc->drawText_p(hFontShadow, &Point2I(bmpOffset.x - 1, bmpOffset.y + 1), textPtr);
      sfc->drawText_p((ghosted ? hFontDisabled : hFont), &bmpOffset, textPtr);
   }
}

static void getActionString(char *destBuf, SimActionMap::Map::iterator action)
{
   destBuf[0] = '\0';
   
   if (action->event.deviceType == SI_KEYBOARD)
   {
      getModifiers(destBuf, action->event.modifier);
      if (action->event.objType == SI_KEY)
      {
         // get the keyStroke
         if (action->event.objInst)
         {
            const DIKCode *code = SimInputManager::getDIK_STRUCT(action->event.objInst);
            char *destBufPtr = &destBuf[strlen(destBuf)];
            if (code->name) strcat(destBuf, avar("%s", code->name));
            else strcat(destBuf, avar("%c", code->lower));
            *destBufPtr = toupper(*destBufPtr);
         }
      }
      else
      {
         AssertFatal(0, "NON-KEY event in a KEYBOARD device???");
      }
   }
   
   else
   {
      if (action->event.objType == SI_BUTTON)
      {
         // is a button
         strcat(destBuf, avar("%s%d", getPairStr(ObjTypes, action->event.objType), action->event.objInst));
         
         //if it's a mouse, change to right or left button
         if (action->event.deviceType == SI_MOUSE)
         {
            if (! stricmp(destBuf, "button0")) strcpy(destBuf, "L Button");
            else if (! stricmp(destBuf, "button1")) strcpy(destBuf, "R Button");
            else if (! stricmp(destBuf, "button2")) strcpy(destBuf, "C Button");
         }
      }
      else
      {
         // probably an axis
         strcat(destBuf, avar("%s%d", getPairStr(ObjTypes, action->event.objType), action->event.objInst));
      }
   }
}

static SimActionMap::Map::iterator findAction(Resource<SimActionMap> actionMap, SimActionMap::Map::iterator startAction,
                                             const char *consoleCmd, Int32 action, float value, Int32 flags, bool makeOrBreak)
{
   //sanity check
   if (! bool(actionMap)) return NULL;
   
   SimActionMap::Map::iterator returnAction;
   if (! bool(startAction)) startAction = actionMap->map.begin();
   for (returnAction = startAction; returnAction != actionMap->map.end(); returnAction++)
   {
      //check for bindCommand
      if (consoleCmd && consoleCmd[0])
      {
         if ((! returnAction->action.consoleCommand) || stricmp(consoleCmd, returnAction->action.consoleCommand))
         {
            continue;
         }
      }
      else if (action > 0)
      {
         //flags must be the same for actions
         if ((Int32)returnAction->flags != flags) continue;
         
         float tolerance = (value < 0 ? value * -0.01f : value * 0.01f);
         if ((returnAction->action.action != action) ||
               (returnAction->action.fValue < value - tolerance) ||
               (returnAction->action.fValue > value + tolerance))
         {
            continue;
         }
      }
      else
      {
         return NULL;
      }
      
      //found a match - either it's a make, or it's not a break
      if ((returnAction->event.action == (makeOrBreak ? SI_MAKE : SI_BREAK)) ||
            (returnAction->event.action != (makeOrBreak ? SI_BREAK : SI_MAKE)))
      {
         return returnAction;
      }
   }

   return NULL;   
}

bool compareActions(SimActionMap::Map::iterator makeAction, SimActionMap::Map::iterator breakAction)
{
   //first make sure the devices match
   if (makeAction->event.deviceType != breakAction->event.deviceType) return FALSE;
   
   //get the string for each, the string compare
   char makeString[128];
   char breakString[128];
   
   getActionString(makeString, makeAction);
   getActionString(breakString, breakAction);
   
   //if the strings are the same event, we completely found a match, return the string
   if (! strcmp(makeString, breakString)) return TRUE;
   else return FALSE;
}

bool CFGButton::bindingConflict(const SimInputEvent *event)
{
   //sanity check
   if (! event || (! mBinding[0])) return FALSE;
   
   if (boundEvent.deviceInst != event->deviceInst) return FALSE;
   if (boundEvent.deviceType != event->deviceType) return FALSE;
   
   switch (boundEvent.deviceType)
   {
      case SI_KEYBOARD:
      {
         char dummy[256];
         BYTE dummyMod = event->modifier;
         getModifiers(dummy, dummyMod);
         if (boundEvent.modifier   != dummyMod) return FALSE;
         if (boundEvent.objType    != event->objType) return FALSE;
         if (boundEvent.objInst    != event->objInst) return FALSE;
         break;
      }
         
      default:
         if (boundEvent.objType    != event->objType) return FALSE;
         if (boundEvent.objInst    != event->objInst) return FALSE;
         break;
   }
   
   //no differences found, therefore there is a conflict
   return TRUE;
}

void CFGButton::removeBinding(void)
{
   //make sure we've loaded an action map and have a binding
   if (! mActionMap[0]) return;
   if (! mBinding[0]) return;
   
   Resource<SimActionMap> actionMap = SimResource::get(manager)->load(mActionMap);
   if (!(bool)actionMap)
   {
      Console->printf( "error: must call 'editActionMap' or 'newActionMap' first. (%s)", mActionMap );
      return;
   }
   
   //unmap both the make and the break
   boundEvent.action = SI_MAKE;
   actionMap->unmapEvent(&boundEvent);
   
   boundEvent.action = SI_BREAK;
   actionMap->unmapEvent(&boundEvent);
   
   //now refresh in case there is another binding that does the same action
   refreshBinding();
}

void CFGButton::newBinding(const SimInputEvent *event)
{
   //make sure we've loaded an action map and have a binding
   if (! mActionMap[0]) return;
   
   Resource<SimActionMap> actionMap = SimResource::get(manager)->load(mActionMap);
   if (!(bool)actionMap)
   {
      Console->printf( "error: must call 'editActionMap' or 'newActionMap' first. (%s)", mActionMap );
      return;
   }
   
   //first remove the current
   if (mBinding[0])
   {
      removeBinding();
   }
   
   //now add the new binding
   SimInputEvent tempEvent = *event;
	SimActionEvent tempAction;
   tempAction.device = 0;
   tempAction.deadZone = 0.0f;
   tempAction.scale = 0.0f;
   
   //add in the make
   tempEvent.action = SI_MAKE;
   if (consoleCommand[0])
   {
      char *newCmd = new char[strlen(consoleCommand) + 1];
      strcpy(newCmd, consoleCommand);
      tempAction.consoleCommand = newCmd;
      tempAction.action = 0;
      tempAction.fValue = 0.0f;
      
      //now add the make action
      actionMap->mapEvent(&tempEvent, &tempAction, mFlags);
      
      //clean up the freshly added string
      delete [] tempAction.consoleCommand;
      tempAction.consoleCommand = NULL;
   }
   else if (mMakeAction > 0)
   {
      tempAction.action = mMakeAction;
      tempAction.fValue = mMakeValue;
   
      //now add the make action
      actionMap->mapEvent(&tempEvent, &tempAction, mFlags);
   }
   
   //add in the break
   tempEvent.action = SI_BREAK;
   if (altConsoleCommand[0])
   {
      char *newCmd = new char[strlen(altConsoleCommand) + 1];
      strcpy(newCmd, altConsoleCommand);
      tempAction.consoleCommand = newCmd;
      tempAction.action = 0;
      tempAction.fValue = 0.0f;
      
      //now add the make action
      actionMap->mapEvent(&tempEvent, &tempAction, mFlags);
      
      //clean up the freshly added string
      delete [] tempAction.consoleCommand;
      tempAction.consoleCommand = NULL;
   }
   else if (mBreakAction > 0)
   {
      tempAction.action = mBreakAction;
      tempAction.fValue = mBreakValue;
   
      //now add the make action
      actionMap->mapEvent(&tempEvent, &tempAction, mFlags);
   }
   
   //now refresh, hope it got added correctly
   refreshBinding();
}


const char *CFGButton::findActionBinding(SimInputEvent *destEvent)
{
   static char returnBuf[256];       
   
   //make sure we've loaded an action map
   if (! mActionMap[0]) return NULL;
   
   Resource<SimActionMap> actionMap = SimResource::get(manager)->load(mActionMap);
   if (!(bool)actionMap)
   {
      Console->printf( "error: must call 'editActionMap' or 'newActionMap' first. (%s)", mActionMap );
      return NULL;
   }
   
   //get the device and device instance
   //strcat(buffer, avar("%s%d, ",
   //    getPairStr(DeviceTypes, i->event.deviceType),
   //    i->event.deviceInst));
   
   //search for a make command    
   SimActionMap::Map::iterator makeAction, breakAction;
   if (consoleCommand[0] || mMakeAction > 0)
   {
      SimActionMap::Map::iterator startAction = NULL;
      bool finished = FALSE;
      while (! finished)
      {
         makeAction = findAction(actionMap, startAction, consoleCommand, mMakeAction, mMakeValue, mFlags, TRUE);
         
         //if nothing was found, we're finished
         if (! bool(makeAction))
         {
            return NULL;
         }
         
         //else we found a make action and don't need to find a break action
         else if ((! altConsoleCommand[0]) && (mBreakAction <= 0))
         {
            //get the string for the makeAction, and return it
            getActionString(returnBuf, makeAction);
            if (destEvent) *destEvent = makeAction->event;
            
            //update the makeAction value to prevent roundoff errors
            makeAction->action.fValue = mMakeValue;
            
            return returnBuf;
         }
         
         //else search for a matching break action
         else
         {
            bool finishedBreak = FALSE;
            SimActionMap::Map::iterator startBreakAction = NULL;
            
            while (! finishedBreak)
            {
               breakAction = findAction(actionMap, startBreakAction, altConsoleCommand, mBreakAction, mBreakValue, mFlags, FALSE);
               
               //if nothing was found, search for a different instance of the make action
               if (! bool(breakAction))
               {
                  startAction = makeAction + 1;
                  finishedBreak = TRUE;
               }
               
               //else see if the key binding matches the event
               else if (compareActions(makeAction, breakAction))
               {
                  //get the string for one of them and return it
                  getActionString(returnBuf, makeAction);
                  if (destEvent) *destEvent = makeAction->event;
                  
                  //update the breakAction value to prevent roundoff errors
                  makeAction->action.fValue = mMakeValue;
                  breakAction->action.fValue = mBreakValue;
            
                  return returnBuf;
               }
               
               //else, search for another break action
               else
               {
                  startBreakAction = breakAction + 1;
               }
            }
         }
      }
   }
   
   //else we have only a break, no make
   else if (altConsoleCommand[0] || (mBreakAction > 0))
   {
      breakAction = findAction(actionMap, NULL, altConsoleCommand, mBreakAction, mBreakValue, mFlags, FALSE);
      
      //if we found a match, return it
      if (bool(breakAction))
      {
         getActionString(returnBuf, breakAction);
         if (destEvent) *destEvent = makeAction->event;
                  
         //update the action value to prevent roundoff errors
         breakAction->action.fValue = mBreakValue;
                  
         return returnBuf;
      }
      else return NULL;
   }
   
   //else we have no action to look for
   return NULL;
}

bool CFGButton::onAdd()
{
   if(!Parent::onAdd())
      return false;
      
   //set the message - LAZY
   setMessage(IDCTG_OPTS_CTRL_CFG_SELECT);
      
   //check the fonts
   hFont = SimResource::loadByTag(manager, IDFNT_9_STANDARD, true);
   hFontHL = SimResource::loadByTag(manager, IDFNT_9_HILITE, true);
   hFontDisabled = SimResource::loadByTag(manager, IDFNT_9_DISABLED, true);
   hFontShadow = SimResource::loadByTag(manager, IDFNT_9_BLACK, true);
   
   //load the bitmaps
   mDefaultBMP = SimResource::get(manager)->load("CFG_Controls_DF.BMP");
   AssertFatal(mDefaultBMP.operator bool(), "Unable to load CFG_Controls_DF.bmp");
   mDefaultBMP->attribute |= BMA_TRANSPARENT;
      
   mGhostedBMP = SimResource::get(manager)->load("CFG_Controls_NA.BMP");
   AssertFatal(mGhostedBMP.operator bool(), "Unable to load CFG_Controls_NA.bmp");
   mGhostedBMP->attribute |= BMA_TRANSPARENT;
   
   mPressedBMP = SimResource::get(manager)->load("CFG_Controls_ON.BMP");
   AssertFatal(mPressedBMP.operator bool(), "Unable to load CFG_Controls_ON.bmp");
   mPressedBMP->attribute |= BMA_TRANSPARENT;
   
   mShiftIcon = SimResource::get(manager)->load("I_Shift.BMP");
   AssertFatal(mShiftIcon.operator bool(), "Unable to load I_Shift.bmp");
   mShiftIcon->attribute |= BMA_TRANSPARENT;
   
   mAltIcon = SimResource::get(manager)->load("I_Alt.BMP");
   AssertFatal(mAltIcon.operator bool(), "Unable to load I_Alt.bmp");
   mAltIcon->attribute |= BMA_TRANSPARENT;
   
   mCtrlIcon = SimResource::get(manager)->load("I_Ctrl.BMP");
   AssertFatal(mCtrlIcon.operator bool(), "Unable to load I_Ctrl.bmp");
   mCtrlIcon->attribute |= BMA_TRANSPARENT;
   
   //set the extent
   extent.set(max(mDefaultBMP->getWidth(), hFontHL->getStrWidth(text) + 2),
                  hFontHL->getHeight() + mDefaultBMP->getHeight());
   
   //get the new binding
   refreshBinding();
   
   //set the state
   mbDefine = FALSE;
   
   return true;
}

void CFGButton::refreshBinding(void)
{
   //clear the binding
   mBinding[0] = '\0';
   
   //find the action in the action map
   const char *result = findActionBinding(&boundEvent);
   
   //copy the string
   if (result) strcpy(mBinding, result);
   
   setUpdate();
}

void CFGButton::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);
   
   insp->write(IDITG_ACTION_MAP, mActionMap);
   insp->write(IDITG_ACTION_MAKE,true, "IDACTION*", mMakeAction);
   insp->write(IDITG_ACTION_MAKE_VALUE, mMakeValue);
   insp->write(IDITG_ACTION_BREAK,true, "IDACTION*", mBreakAction);
   insp->write(IDITG_ACTION_BREAK_VALUE, mBreakValue);
   insp->write(IDITG_ACTION_FLAGS, mFlags);
}

void CFGButton::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   
   insp->read(IDITG_ACTION_MAP, mActionMap);
   insp->read(IDITG_ACTION_MAKE, NULL, NULL, mMakeAction);
   insp->read(IDITG_ACTION_MAKE_VALUE, mMakeValue);
   insp->read(IDITG_ACTION_BREAK, NULL, NULL, mBreakAction);
   insp->read(IDITG_ACTION_BREAK_VALUE, mBreakValue);
   insp->read(IDITG_ACTION_FLAGS, mFlags);
   
   hFont = SimResource::loadByTag(manager, IDFNT_9_STANDARD, true);
   hFontHL = SimResource::loadByTag(manager, IDFNT_9_HILITE, true);
   hFontDisabled = SimResource::loadByTag(manager, IDFNT_9_DISABLED, true);
   hFontShadow = SimResource::loadByTag(manager, IDFNT_9_BLACK, true);
   
   //find the action in the action map
   refreshBinding();
   
   //set the extent
   extent.set(max(mDefaultBMP->getWidth(), hFontHL->getStrWidth(text) + 2),
                  hFontHL->getHeight() + mDefaultBMP->getHeight());
}

Persistent::Base::Error CFGButton::write( StreamIO &sio, int version, int user)
{
   sio.write(gCtrlVersion);
   
   sio.writeString(mActionMap);
   sio.write(mMakeAction);
   sio.write(mMakeValue);
   sio.write(mBreakAction);
   sio.write(mBreakValue);
   sio.write(mFlags);
   
   return Parent::write(sio, version, user);
}

Persistent::Base::Error CFGButton::read( StreamIO &sio, int version, int user)
{
   int localVersion;
   sio.read(&localVersion);
   
   sio.readString(mActionMap);
   sio.read(&mMakeAction);
   sio.read(&mMakeValue);
   sio.read(&mBreakAction);
   sio.read(&mBreakValue);
   sio.read(&mFlags);
   
   return Parent::read(sio, version, user);
}

IMPLEMENT_PERSISTENT_TAG( FearGui::CFGButton,   FOURCC('F','G','c','f') );

};