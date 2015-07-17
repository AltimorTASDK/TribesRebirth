//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------


#include <sim.h>
#include "simGuiBase.h"
#include "simGuiCtrl.h"
#include "simConsolePlugin.h"
#include "simGuiPlugin.h"
#include "simGuiEditCtrl.h"
#include "simTagDictionary.h"
#include "inspectWindow.h"
#include "simGuiCanvas.h"
#include "simGame.h"
#include "g_surfac.h"
#include "simVolume.h"
#include "g_bitmap.h"
#include "simguiactivectrl.h"
#include "simguisimpletext.h"
#include "simguitextlist.h"
	enum CallbackID {
      GuiInspect,
      EditMode,
      EditNewControl,
      SetCurrentAddSet,
      SetSelection,
      NewContent,
      LoadContent,
      SaveContent,
      SaveSelection,
      LoadSelection,
      GuiJustify,
      GuiToolbar,
      GuiSendToBack,
      GuiBringToFront,
      SendRootMessage,
      SetCCursor,
      IsCursorOn,
      CursorOn,
      CursorOff,
      WindowsMouseEnable,
      WindowsMouseDisable,
      GuiPushDialog,
      GuiPopDialog,
      GuiSetValue,
      GuiGetValue,
      GuiSetActive,
      GuiGetActive,
      GuiSetVisible,
      GuiGetVisible,
      GuiPerformClick,
      GuiSetText,
      GuiGetText,
      TextListClear,
      TextListAdd,

#ifdef WINKEYBOARD
	  WindowsKeyboardEnable,
	  WindowsKeyboardDisable,
#endif

   };

#define object_get(object,cls,name) {object=NULL;if(SimObject*obj=manager->findObject(name))object=dynamic_cast<cls*>(obj);}
//------------------------------------------------------------------------------
namespace SimGui
{

extern Point2I gGridSnap;
extern bool g_prefPoliteGui;

void Plugin::init()
{
	console->printf("SimGuiPlugin");

   console->addCommand(EditMode, "GuiEditMode", this);
   console->addCommand(EditNewControl, "GuiEditNewControl", this);
   console->addCommand(SetCurrentAddSet, "GuiSetAddSet", this);
   console->addCommand(SetSelection, "GuiSetSelection", this);
   console->addCommand(NewContent, "GuiNewContentCtrl", this);
   console->addCommand(LoadContent, "GuiLoadContentCtrl", this);
   console->addCommand(SaveContent, "GuiSaveContentCtrl", this);
   console->addCommand(SaveSelection, "GuiSaveSelection", this);
   console->addCommand(LoadSelection, "GuiLoadSelection", this);
   console->addCommand(GuiInspect, "GuiInspect", this);
   console->addCommand(GuiJustify, "GuiJustify", this);
   console->addCommand(GuiToolbar, "GuiToolbar", this);
   console->addCommand(GuiSendToBack, "GuiSendToBack", this);
   console->addCommand(GuiBringToFront, "GuiBringToFront", this);
   console->addCommand(SendRootMessage, "GuiSendRootMessage", this);
   console->addCommand(SetCCursor, "setCursor",this);
   console->addCommand(IsCursorOn, "isCursorOn",this);
   console->addCommand(CursorOn, "cursorOn",this);
   console->addCommand(CursorOff, "cursorOff",this);
   console->addCommand(WindowsMouseEnable,"windowsMouseEnable",this);
   console->addCommand(WindowsMouseDisable,"windowsMouseDisable",this);
   console->addCommand(GuiPushDialog,"GuiPushDialog", this);
   console->addCommand(GuiPopDialog,"GuiPopDialog", this);
   console->addCommand(GuiPerformClick,"Control::performClick", this);
   console->addCommand(GuiSetValue,"Control::setValue", this);
   console->addCommand(GuiGetValue,"Control::getValue", this);
   console->addCommand(GuiSetActive,"Control::setActive", this);
   console->addCommand(GuiGetActive,"Control::getActive", this);
   console->addCommand(GuiSetVisible,"Control::setVisible", this);
   console->addCommand(GuiGetVisible,"Control::getVisible", this);
   console->addCommand(GuiSetText,"Control::setText", this);
   console->addCommand(GuiGetText,"Control::getText", this);
   console->addCommand(TextListClear,"TextList::Clear", this);
   console->addCommand(TextListAdd,"TextList::AddLine", this);
   console->addVariable(0, "GuiEdit::GridSnapX", CMDConsole::Int, &gGridSnap.x);
   console->addVariable(0, "GuiEdit::GridSnapY", CMDConsole::Int, &gGridSnap.y);

#ifdef WINKEYBOARD
   console->addCommand(WindowsKeyboardEnable, "windowsKeyboardEnable",this);
   console->addCommand(WindowsKeyboardDisable,"windowsKeyboardDisable",this);
#endif

   console->addVariable(0, "pref::politeGui", CMDConsole::Bool, &g_prefPoliteGui);

}


//----------------------------------------------------------------------------
void Plugin::startFrame()
{
}

void Plugin::endFrame()
{
}

Control *findControl(const char *name)
{
   SimManager *manager = SimGame::get()->getManager(SimGame::CLIENT);
   int tag = atoi(name);
   if(tag)
   {
      SimSet *s = (SimSet *) manager->findObject(TaggedGuiSetId);
      SimSet::iterator i;
      for(i = s->begin(); i != s->end(); i++)
      {
         Control *c = dynamic_cast<Control *>(*i);
         if(c && c->getTag() == DWORD(tag))
            return c;
      }
      return NULL;   
   }
   else
   {
      StringTableEntry n = stringTable.insert(name);
      SimSet *s = (SimSet *) manager->findObject(NamedGuiSetId);
      SimSet::iterator i;
      for(i = s->begin(); i != s->end(); i++)
      {
         Control *c = dynamic_cast<Control *>(*i);
         if(c && c->getName() == n)
            return c;
      }
      return NULL;   
   }
}

//----------------------------------------------------------------------------
const char *Plugin::consoleCallback(CMDConsole *, int id,int argc,const char *argv[])
{
   if (!manager) return "False";

   switch(id)
   {   
      case GuiGetValue:
      case GuiGetVisible:
      case GuiGetActive:
      case GuiGetText:
      case GuiPerformClick:
      case TextListClear:
         if(argc != 2)
            Console->printf("%s(control);", argv[0]);
         else
         {
            Control *ctrl = findControl(argv[1]);
            if(!ctrl)
            {
               return "";
            }
            switch(id)
            {
               case GuiPerformClick:
               {
                  Event ev;
                  ev.time = GetTickCount();
                  ev.ptMouse = ctrl->localToGlobalCoord(Point2I(0,0));
                  ev.modifier = 0;
                  ev.mouseDownCount = 1;
                  ctrl->onMouseDown(ev);
                  return "";
               }
               case GuiGetValue:
                  return ctrl->getScriptValue();
               case GuiGetVisible:
                  return ctrl->isVisible() ? "True" : "False";
               case GuiGetActive: {
                  ActiveCtrl *c = dynamic_cast<ActiveCtrl*>(ctrl);
                  if(c)
                     return c->active ? "True" : "False";
                  else
                     return "False";
               }
               case GuiGetText: {
                  SimpleText *c = dynamic_cast<SimpleText*>(ctrl);
                  if(c)
                     return c->getText();
                  else
                     return "";
               }
               case TextListClear: {
                  TextList *c = dynamic_cast<TextList*>(ctrl);
                  if(c)
                     c->clear();
                  return "True";
               }
            }
         }
      case GuiSetValue:
      case GuiSetVisible:
      case GuiSetActive:
      case GuiSetText:
      case TextListAdd:
         if (argc != 3)
            Console->printf("%s(control, value);", argv[0]);
         else
         {
            Control *ctrl = findControl(argv[1]);
            if(!ctrl)
            {
               return "";
            }
            switch(id)
            {
               case GuiSetValue:
                  ctrl->setScriptValue(argv[2]);
                  break;
               case GuiSetVisible:
                  ctrl->setVisible(CMDConsole::getBool(argv[2]));
                  ctrl->setUpdate();
                  break;
               case GuiSetActive: {
                  ActiveCtrl *c = dynamic_cast<ActiveCtrl*>(ctrl);
                  if(c)
                  {
                     c->active = CMDConsole::getBool(argv[2]);
                     c->setUpdate();
                  }
                  break;
               }
               case GuiSetText: {
                  SimpleText *c = dynamic_cast<SimpleText*>(ctrl);
                  if(c)
                  {
                     c->setText(argv[2]);
                     c->setUpdate();
                  }
                  break;
               }
               case TextListAdd: {
                  TextList *c = dynamic_cast<TextList*>(ctrl);
                  if(c)
                  {
                     c->addEntryBack(argv[2]);
                     c->setUpdate();
                  }
                  break;
               }
            }
            return "True";
         }
         return "False";
   }

   Control *ctrl, *newCtrl = NULL, *newCtrlParent = NULL;
   EditControl *ectrl = NULL;
   Persistent::Base::Error err;
   Canvas *canvas = NULL;
   Canvas *root;
   SimObject *obj;
   bool argBool = FALSE;

   if(argc >= 2)
   {
      if((obj=manager->findObject(argv[1])) != NULL)
         canvas = dynamic_cast<Canvas *>(obj);
      if(!canvas)
      {
         console->printf("%s: %s is not a valid SimGui::Canvas", argv[0], argv[1]);
         return "False";
      }
      root = canvas;
      ectrl = dynamic_cast<EditControl*>(root->getMouseLockedControl());
   }
   // check the arguments here...

   switch(id)
   {
      case GuiSendToBack:
      case GuiBringToFront:
         if (argc != 2)
         {
            console->printf("%s: canvasName", argv[0]);
            return "False";
         }
         if(!ectrl)
         {
            console->printf("%s: must be in edit mode", argv[0]);
            return "False";
         }
         break;
      case SetCCursor:
         if (argc != 3)
         {
            console->printf("%s: console bitmap", argv[0]);
            return "False";
         }
         break;
      case IsCursorOn:
      case CursorOn:
      case CursorOff:
      case WindowsMouseEnable:
      case WindowsMouseDisable:

#ifdef WINKEYBOARD 
      case WindowsKeyboardEnable:
      case WindowsKeyboardDisable:
#endif
         if(argc != 2)
         {
            console->printf("%s: canvasName", argv[0]);
            return "False";
         }
         break;
      case SendRootMessage:
      case GuiJustify:
      case GuiPopDialog:
         if(argc != 3)
         {
            console->printf("%s: simCanvas int", argv[0]);
            return "False";
         }
         break;
      case SaveSelection:
         if(!ectrl)
         {
            console->printf("%s: must be in edit mode", argv[0]);
            return "False";
         }
         // fall through
      case LoadSelection:
      case LoadContent:
      case SaveContent:
      case GuiPushDialog:
         if(argc != 3)
         {
            console->printf("%s: simCanvas filename", argv[0]);
            return "False";
         }
         break;
         
      case SetCurrentAddSet:
         if (argc != 3)
         {
            console->printf("%s: simCanvas objectID", argv[0]);
            return "False";
         }
         newCtrlParent = dynamic_cast<Control *>(manager->findObject(argv[2]));
         
         break;
         
      case SetSelection:
         if (argc < 3)
         {
            console->printf("%s: simCanvas objectID <inclusive T or F>", argv[0]);
            return "False";
         }
         ctrl = dynamic_cast<Control *>(manager->findObject(argv[2]));
         
         if (argc == 4)
         {
            if (! stricmp(argv[3], "TRUE")) argBool = TRUE;
            else argBool = FALSE;
         }
         break;
         
      case EditNewControl:
      case NewContent:
         if(argc != 3)
         {
            console->printf("%s: simCanvas classname", argv[0]);
            return "False";
         }
         obj = (SimObject *) Persistent::AbstractTaggedClass::create(argv[2]);
         if(obj)
         {
            newCtrl = dynamic_cast<Control *>(obj);
            if(!newCtrl)
            {
               delete obj;
               console->printf("%s: %s is not a control class", argv[0], argv[2]);
               return "False";
            }
         }
         else
         {
            console->printf("%s: %s is not a valid class name", argv[0], argv[2]);
            return "False";
         }
         break;
      case GuiInspect:
         if(argc < 2 || argc > 3)
         {
            console->printf("%s: simCanvas [simTagDictionary]", argv[0]);
            return "False";
         }
         break;
      case EditMode:
      case GuiToolbar:
         if(argc != 2)
         {
            console->printf("%s: simCanvas", argv[0]);
            return "False";
         }
         break;
   }

   // all arguments checked
   
	switch(id) 
	{
      case SetCCursor:
         {
         // kinda hacky
         Resource<GFXBitmap> curs = SimResource::get(manager)->load(argv[2], true);
         if ((bool)curs)
		      canvas->setCursor( curs );
         else
				console->printf("%s: canvas bitmap", argv[0]);
         return "False";
         }
      case IsCursorOn:
         return(canvas->isCursorON() ? "True" : "False");
      case CursorOn:
      case CursorOff:
         canvas->setCursorON(id == CursorOn);   
         return "False";
      case WindowsMouseEnable:
         canvas->setUseWindowsMouseEvents(true);
         return "False";
      case WindowsMouseDisable:
         canvas->setUseWindowsMouseEvents(false);
         return "False";

#ifdef WINKEYBOARD
      case WindowsKeyboardEnable:
         canvas->setUseWindowsKeyboardEvents(true);
         return "False";
      case WindowsKeyboardDisable:
         canvas->setUseWindowsKeyboardEvents(false);
         return "False";
#endif

      case SendRootMessage:
         root->onMessage(NULL, atoi(argv[2]));
         return "False";
      case GuiToolbar:
         {
         SimObject *tb = manager->findObject("GuiEditBar");
         if (tb)
         {
            console->printf("GuiToolbar is already running");
            return "False";
         }

         // load up the toolbar art volume
         SimVolume* vol = new SimVolume;
         manager->addObject(vol);

	      if ( !vol->open("guiToolbar.vol")) 
			{
            manager->deleteObject(vol);
            console->printf("Could not load guiToolbar.vol");
            console->printf("copy it from phoenix/data into your exe directory");
            return "False";   
         }

			SimResource::find(manager)->addObject(vol);

         // remember root
         console->evaluate(avar("$SimGui::TBRoot = %s;", argv[1]), false); 
            
         // setup the toolbar
         console->evaluate(avar("newToolWindow(GuiEditBar, %s);", argv[1]), false); 
         console->evaluate("exec(\"guiTBSetup.cs\");", false);
         return "False";
         }
      case GuiJustify:
         {
            int just = atoi(argv[2]);
            if(just < 0 || just > 6)
            {
               console->printf("%s: justification must be 0-6", argv[0]);
               break;
            }
            ectrl->justifySelection((EditControl::Justification) just);
         }
         break;
      case GuiSendToBack:
         ectrl->sendToBack();
         break;
      case GuiBringToFront:
         ectrl->bringToFront();
         break;
      case GuiInspect:
         if(argc != 2 && argc != 3)
         {
            console->printf("GuiInspect: simCanvas [simTagDictionary]");
            break;
         }
         InspectWindow *gi;
         gi = dynamic_cast<InspectWindow *>(manager->findObject("guiInspector"));
         if (gi)
         {
            console->printf("Gui Inspector is already running");
            break;
         }
         RECT winrect;
         winrect.left = 50;
         winrect.top = 50;

         POINT cursorPos;
         if ( GetCursorPos(&cursorPos) )
         {
            winrect.left = cursorPos.x;
            winrect.top = cursorPos.y;
         }

         winrect.right = winrect.left + 30;
         winrect.bottom = winrect.top + 30;
         
         GWWindow *parent;
         parent = canvas;

         SimTagDictionary *std;
         std = NULL;
         if (argc == 3)
         {
            std = dynamic_cast<SimTagDictionary*>(manager->findObject(argv[2]));
         }
      
         gi = new InspectWindow(parent, "Gui Inspector", winrect, std);
         if (gi)
            manager->addObject(gi,"guiInspector");
         
         return (0);
      case GuiPopDialog:
         canvas->handleDLGClose(atoi(argv[2]));
         break;
      case GuiPushDialog:
         canvas->handleDLGMessage(argv[2], 0);
         break;
      case SaveSelection:
         ectrl->saveSelection(argv[2]);
         break;
      case LoadSelection:
         ectrl->loadSelection(argv[2]);
         break;
      case SaveContent:
         ctrl = root->getContentControl();
         if(!ctrl)
         {
            console->printf("No content control on root: %s", argv[1]);
            break;
         }
         err = ctrl->fileStore(argv[2]);
         if (err != Persistent::Base::Ok)
            console->printf("Error saving content control to file: %s", argv[2]);
         break;
      case LoadContent:
         if(root->inEditMode())
            root->editMode(0);

         root->handleGUIMessage(argv[2]);
         break;
      case NewContent:
         if(root->inEditMode())
            root->editMode(0);
         Control *prev;
         prev = root->getContentControl();

         manager->addObject(newCtrl);
         root->setContentControl(newCtrl, NULL);
         if(prev && prev->deleteOnLoseContent())
            prev->deleteObject();
         return 0;
         
      case SetCurrentAddSet:
         if (newCtrlParent)
         {
            if(!root->getContentControl())
            {
               console->printf("GuiSetAddSet: no content control for %s", argv[1]);
               return 0;
            }
            if(!root->inEditMode())
               root->editMode(1);
               
            ectrl = dynamic_cast<EditControl*>(root->getMouseLockedControl());
            if (ectrl) ectrl->setCurrentAddSet(newCtrlParent);
         }
         return 0;
         
      case SetSelection:
      if (ctrl)
      {
         if (!root->getContentControl())
         {
            console->printf("GuiSetSelection: no content control for %s", argv[1]);
            return 0;
         }
         if (!root->inEditMode())
         {
            root->editMode(1);
         }
            
         ectrl = dynamic_cast<EditControl*>(root->getMouseLockedControl());
         if (ectrl) ectrl->setSelection(ctrl, argBool);
      }
      return 0;
         
      case EditNewControl:
         if(!root->getContentControl())
         {
            console->printf("GuiEditNewControl: no content control for %s", argv[1]);
            return 0;
         }
         if(!root->inEditMode())
            root->editMode(1);

         ectrl = dynamic_cast<EditControl*>(root->getMouseLockedControl());
         manager->addObject(newCtrl);
         ectrl->addNewControl(newCtrl);
         return 0;
      case EditMode:
         if(root->inEditMode())
            root->editMode(0);
         else
            root->editMode(1);
         break;
	}
	return 0;
}

};
