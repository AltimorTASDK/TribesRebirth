#include "simGuiArrayCtrl.h"
#include "PlayerManager.h"
#include "g_bitmap.h"
#include "g_surfac.h"
#include "fear.strings.h"
#include "simResource.h"
#include <g_font.h>
#include "fearGuiShellPal.h"
#include "fearGlobals.h"
#include "player.h"
#include "console.h"
#include "SimGame.h"
#include "tagDictionary.h"
#include "simGuiTextList.h"
#include "simGuiTextEdit.h"
#include "simGuiTestButton.h"
#include "simGuiTestCheck.h"
#include "simGuiTestRadial.h"
#include "simSetIterator.h"
#include "simGuiScrollCtrl.h"
#include "darkstar.strings.h"

enum {
   TextField,
   Boolean,
   Description,
};

//------------------------------------------------------------------

class METextList : public SimGui::TextList
{
   typedef SimGui::TextList Parent;
public:
   void setCellSize( Point2I & size ){ cellSize = size; }
   void getCellSize( Point2I & size ){ size = cellSize; }
   bool onAdd();
   void onPreRender(void);
   bool becomeFirstResponder();
   DECLARE_PERSISTENT(METextList);
};

IMPLEMENT_PERSISTENT_TAG(METextList, FOURCC('M','e','t','L'));

bool METextList::onAdd()
{
   if(!Parent::onAdd())
      return false;
   setFont(IDFNT_EDITOR);
   setFontHL(IDFNT_EDITOR);
   return true;
}

bool METextList::becomeFirstResponder()
{
   return false;
}

void METextList::onPreRender(void)
{
   setSize(Point2I(1, textList.size()));
}
   
//------------------------------------------------------------------

class MEButton : public SimGui::TestButton
{
   typedef SimGui::TestButton Parent;
public:
   bool onAdd();
   bool becomeFirstResponder();
   DECLARE_PERSISTENT(MEButton);
};

IMPLEMENT_PERSISTENT_TAG(MEButton, FOURCC('M','e','b','T'));

bool MEButton::onAdd()
{
   if(!Parent::onAdd())
      return false;
   setFont(IDFNT_EDITOR);
   setFontHL(IDFNT_EDITOR);
   setFontDisabled(IDFNT_EDITOR);
   return true;
}

bool MEButton::becomeFirstResponder()
{
   return false;
}   

//------------------------------------------------------------------

class MEPopup : public SimGui::SimpleText
{
   typedef SimGui::SimpleText Parent;
   struct Entry
   {
      char buf[256];
      int id;
   };

   Vector<Entry> entries;
   int selIndex;
   int width;
   METextList *tl;
   static bool consoleFunctionsAdded;
public:
   bool onAdd();
   bool becomeFirstResponder();

   void addEntry(const char *buf, int id);
   void onRender(GFXSurface *sfc, Point2I offset, const Box2I & box);
   void onAction();
   void clear();

   int getSelected();
   void setSelected(int id);
   void popupWillClose();
   const char *getScriptValue();

   DECLARE_PERSISTENT(MEPopup);
};

IMPLEMENT_PERSISTENT_TAG(MEPopup, FOURCC('M','e','p','L'));

class MEPopupBkgnd : public SimGui::Control
{
   typedef SimGui::Control Parent;
   MEPopup *popup;
public:
   void setPopup(MEPopup *popup);
   void onDeleteNotify(SimObject *obj);
   void onMouseDown(const SimGui::Event &event);
   DWORD onMessage(SimObject *, DWORD msg);
};

void MEPopupBkgnd::setPopup(MEPopup *in_popup)
{
   popup = in_popup;
   deleteNotify(popup);
}

void MEPopupBkgnd::onDeleteNotify(SimObject *obj)
{
   if(obj == popup)
      popup = NULL;
   Parent::onDeleteNotify(obj);
}

void MEPopupBkgnd::onMouseDown(const SimGui::Event &)
{
   root->popDialogControl();
}

DWORD MEPopupBkgnd::onMessage(SimObject *, DWORD msg)
{
   if(popup)
      popup->popupWillClose();
   root->popDialogControl();
   return msg;
}

bool MEPopup::consoleFunctionsAdded = false;

namespace SimGui {
extern Control *findControl(const char *name);
};

static const char *MEPopupAddEntry(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 4)
   {
      Console->printf("%s(control, string, id);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   MEPopup *pop = NULL;
   if(ctrl)
      pop = dynamic_cast<MEPopup *>(ctrl);

   if(!pop)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   pop->addEntry(argv[2], atoi(argv[3]));
   return "true";
}

static const char *MEPopupClear(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
   {
      Console->printf("%s(control);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   MEPopup *pop = NULL;
   if(ctrl)
      pop = dynamic_cast<MEPopup *>(ctrl);

   if(!pop)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   pop->clear();
   return "true";
}

static const char *MEPopupSetSelected(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 3)
   {
      Console->printf("%s(control, id);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   MEPopup *pop = NULL;
   if(ctrl)
      pop = dynamic_cast<MEPopup *>(ctrl);

   if(!pop)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   pop->setSelected(atoi(argv[2]));
   return "true";
}

static const char *MEPopupGetSelected(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
   {
      Console->printf("%s(control);", argv[0]);
      return "0";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   MEPopup *pop = NULL;
   if(ctrl)
      pop = dynamic_cast<MEPopup *>(ctrl);

   if(!pop)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "0";
   }
   static char buf[20];
   sprintf(buf, "%d", pop->getSelected());
   return buf;
}

bool MEPopup::onAdd()
{
   if(!consoleFunctionsAdded)
   {
      consoleFunctionsAdded = true;
      Console->addCommand(0, "Popup::addLine", MEPopupAddEntry);
      Console->addCommand(0, "Popup::clear", MEPopupClear);
      Console->addCommand(0, "Popup::setSelected", MEPopupSetSelected);
      Console->addCommand(0, "Popup::getSelected", MEPopupGetSelected);
   }
   if(!Parent::onAdd())
      return false;
   width = 0;
   selIndex = -1;
   setText("");
   setFont(IDFNT_EDITOR);
   setFontHL(IDFNT_EDITOR);
   setFontDisabled(IDFNT_EDITOR);
   return true;
}

bool MEPopup::becomeFirstResponder()
{
   return false;
}   

void MEPopup::clear()
{
   entries.setSize(0);
   setText("");
   selIndex = -1;
}

void MEPopup::addEntry(const char *buf, int id)
{
   Entry e;
   strcpy(e.buf, buf);
   e.id = id;
   entries.push_back(e);
   width = max(width, (int) hFont->getStrWidth(buf));
}

int MEPopup::getSelected()
{
   if(selIndex == -1)
      return 0;
   return entries[selIndex].id;
}

void MEPopup::setSelected(int id)
{
   int i;
   for(i = 0; i < entries.size(); i++)
   {
      if(id == entries[i].id)
      {
         selIndex = i;
         setText(entries[i].buf);
         return;
      }
   }
   setText("");
   selIndex = -1;
}

const char *MEPopup::getScriptValue()
{
   static char buf[24];
   if(selIndex != -1)
      sprintf(buf, "%d", entries[selIndex].id);
   else
      strcpy(buf, "0");
   return buf;
}

void MEPopup::onRender(GFXSurface *sfc, Point2I offset, const Box2I & box)
{
   RectI textClip(offset.x, offset.y, offset.x + extent.x - 1, offset.y + extent.y - 1);
   if(active)
      sfc->drawRect2d(&textClip, 255);
   position.x += 3;
   Parent::onRender(sfc, offset, box);
   position.x -= 3;
}

void MEPopup::popupWillClose()
{
   selIndex = tl->getSelectedCell().y;
   if(selIndex != -1)
   {
      setText(entries[selIndex].buf);
      setIntVariable(entries[selIndex].id);
   }
   else
   {
      setText("");
      setIntVariable(0);
   }
   if(getName())
   {
      char buf[256];
      sprintf(buf, "%s::onSelect", getName());
      if(Console->isFunction(buf))
      {
         char idval[24];
         if(selIndex == -1)
            Console->executef(3, buf, "", "-1");
         else
         {
            sprintf(idval, "%d", entries[selIndex].id);
            Console->executef(3, buf, entries[selIndex].buf, idval);
         }
      }
   }
   if(consoleCommand[0])
      CMDConsole::getLocked()->evaluate(consoleCommand, false);
}

void MEPopup::onAction()
{
   tl = new METextList;
   tl->noDuplicates = false;
   manager->registerObject(tl);
   width = extent.x;
   tl->resize(Point2I(0,0), Point2I(width, 0));
   
   Point2I size;
   tl->getCellSize( size );
   size.x = width;
   tl->setCellSize( size );
   
   int i;
   for(i = 0; i < entries.size(); i++)
      tl->addEntryBack(entries[i].buf);
      
   //tl->onPreRender();
   tl->setSize(Point2I( 1, entries.size()));
   
   tl->setMessage(1);
   tl->setSelectedCell(Point2I(0, selIndex));

   Point2I windowExt = root->getContentControl()->extent;
   SimGui::ScrollCtrl *sc = new SimGui::ScrollCtrl;
   manager->registerObject(sc);
   
   sc->forceVScrollBar = SimGui::ScrollCtrl::ScrollBarAlwaysOn;
   sc->forceHScrollBar = SimGui::ScrollCtrl::ScrollBarAlwaysOff;
   sc->mbOpaque = true;
   sc->mbBoarder = true;
   sc->fillColor = 0;
   sc->boarderColor = 255;
   
   Point2I scExt(tl->extent.x + 16, min(tl->extent.y+2, windowExt.y));
   Point2I scPos = localToGlobalCoord(Point2I(0, extent.y));

   if(scPos.x + scExt.x > windowExt.x)
      scPos.x = windowExt.x - scExt.x;
   if(scPos.y + scExt.y > windowExt.y)
      scPos.y = windowExt.y - scExt.y;

   sc->resize(scPos, scExt);
   sc->addObject(tl);

   MEPopupBkgnd *ctrl = new MEPopupBkgnd;
   manager->addObject(ctrl);
   ctrl->setPopup(this);

   ctrl->resize(Point2I(0,0), windowExt);
   ctrl->addObject(sc);
   root->pushDialogControl(ctrl);
}

//------------------------------------------------------------------

class MEPopupButton : public MEPopup
{
      typedef MEPopup Parent;
      bool  mCurrent;
      
   public:
      MEPopupButton() : mCurrent( false ){}
      void setCurrent( bool b ){ mCurrent = b; }
      bool isCurrent(){ return( mCurrent ); }
      void makeCurrent();
      void onRightMouseDown( const SimGui::Event & event );
      void onRightMouseUp( const SimGui::Event & event );
      void onRightAction();
      void onAction();
      bool onAdd();
      void onRender(GFXSurface *sfc, Point2I offset, const Box2I & box);
      DECLARE_PERSISTENT(MEPopupButton);
};

IMPLEMENT_PERSISTENT_TAG(MEPopupButton, FOURCC('M','e','p','b'));

static const char *MEPopupButtonMakeCurrent(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
   {
      Console->printf("%s(control);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   MEPopupButton *pop = NULL;
   if(ctrl)
      pop = dynamic_cast<MEPopupButton *>(ctrl);

   if(!pop)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   pop->makeCurrent();
   return "true";
}

bool MEPopupButton::onAdd()
{
   if(!Console->isFunction(  "PopupButton::makeCurrent"))
      Console->addCommand(0, "PopupButton::makeCurrent", MEPopupButtonMakeCurrent);
   return(Parent::onAdd());
}
      
void MEPopupButton::onRightMouseDown( const SimGui::Event & event )
{
   if( !active )
   {
      Parent::onMouseDown(event);
      return;   
   }
   if( stateOver )
   {
      root->mouseLock(this);
      root->makeFirstResponder(this);
      stateDepressed = true;
   }
   setUpdate();
}

void MEPopupButton::onRightMouseUp( const SimGui::Event & event )
{
   if( !active )
   {
      Parent::onRightMouseUp(event);
      return;
   }
   
   if( root->getMouseLockedControl() == this )
      root->mouseUnlock();
   
   if( stateDepressed )
   {
      stateDepressed = false;
      onRightAction();
   }
   setUpdate();
}

void MEPopupButton::makeCurrent()
{
   // there can be only one
   for( SimSet::iterator itr = parent->begin(); itr != parent->end(); itr++ )
   {
      MEPopupButton * popButton = dynamic_cast< MEPopupButton *>(*itr);
      if(popButton)
         popButton->setCurrent(false);
   }
   setCurrent(true);
}

void MEPopupButton::onRightAction()
{
   makeCurrent();
   Parent::onAction();
}

void MEPopupButton::onAction()
{
   if(getName())
   {
      char buf[256];
      sprintf(buf, "%s::onPressed", getName());
      if(Console->isFunction(buf))
         Console->executef( 1, buf );
      makeCurrent();
   }
}

void MEPopupButton::onRender( GFXSurface * sfc, Point2I offset, const Box2I & box )
{
   if( isCurrent() )
   {
      RectI textClip( offset.x + 1, offset.y + 1, offset.x + extent.x - 2, offset.y + extent.y - 2 );
      sfc->drawRect2d( &textClip, 255 );
   }

   position.x += 3;
   Parent::onRender( sfc, offset, box );
   position.x -= 3;
}

//------------------------------------------------------------------

class MECheck : public SimGui::TestCheck
{
   typedef SimGui::TestCheck Parent;
public:
   bool onAdd();
   bool becomeFirstResponder();
   DECLARE_PERSISTENT(MECheck);

};

IMPLEMENT_PERSISTENT_TAG(MECheck, FOURCC('M','e','c','K'));

bool MECheck::onAdd()
{
   if(!Parent::onAdd())
      return false;
   setFont(IDFNT_EDITOR);
   setFontHL(IDFNT_EDITOR);
   setFontDisabled(IDFNT_EDITOR);
   selectFillColor = 255;
   fillColor = 0;
   return true;
}

bool MECheck::becomeFirstResponder()
{
   return false;
}   

//------------------------------------------------------------------

class MERadio : public SimGui::TestRadial
{
   typedef SimGui::TestRadial Parent;
public:
   bool onAdd();
   bool becomeFirstResponder();
   DECLARE_PERSISTENT(MERadio);

};

IMPLEMENT_PERSISTENT_TAG(MERadio, FOURCC('M','e','r','b'));

bool MERadio::onAdd()
{
   if(!Parent::onAdd())
      return false;
   setFont(IDFNT_EDITOR);
   setFontHL(IDFNT_EDITOR);
   setFontDisabled(IDFNT_EDITOR);
   return true;
}

bool MERadio::becomeFirstResponder()
{
   return false;
}   

//------------------------------------------------------------------

class METextEdit : public SimGui::TextEdit
{
   typedef SimGui::TextEdit Parent;
public:
   bool onAdd();
   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);
   void onKeyDown(const SimGui::Event &event);
   DECLARE_PERSISTENT(METextEdit);
};

IMPLEMENT_PERSISTENT_TAG(METextEdit, FOURCC('M','e','T','e'));

bool METextEdit::onAdd()
{
   if(!Parent::onAdd())
      return false;

   setFont(IDFNT_EDITOR);
   setFontHL(IDFNT_EDITOR_HILITE);
   setFontDisabled(IDFNT_EDITOR);
   setCursorColor(255);
   setHighlightColor(255);
   return true;
}

void METextEdit::onRender(GFXSurface *sfc, Point2I offset, const Box2I & /*updateRect*/)
{
   RectI textClip(offset.x, offset.y, offset.x + extent.x - 1, offset.y + extent.y - 1);
   setTextClipRect(textClip);
   if(active)
      sfc->drawRect2d(&textClip, 255);
   DrawText(sfc);
}

void METextEdit::onKeyDown(const SimGui::Event &event)
{
   if(event.diKeyCode == DIK_RETURN || event.diKeyCode == DIK_NUMPADENTER)
      root->makeFirstResponder(NULL);
   else
      Parent::onKeyDown(event);
}

//------------------------------------------------------------------

class MEInspectTagList : public SimGui::Control
{
   typedef SimGui::Control Parent;
   bool locked;
public:
   enum
   {
      MaxWidth = 120,
      MaxNameLen = 256,
   };

   struct Entry
   {
      char name[MaxNameLen + 1];
      int y;
      int strWidth;
      SimGui::Control *ectrl;
      const char *getValue();
   };

   Vector<Entry> entries;
   Resource<GFXFont> font;

   bool onAdd();
   void addDescription(int y, const char *tagStr, const char *stbuf, int type);
   MEPopup *addComboBox(int y, const char *tagStr);
   void addDesFinished();

   void reset(bool in_locked);
   virtual void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);

   DECLARE_PERSISTENT(MEInspectTagList);
};
IMPLEMENT_PERSISTENT_TAG(MEInspectTagList, FOURCC('M','e','I','T'));

const char *MEInspectTagList::Entry::getValue()
{
   if(ectrl)
      return ectrl->getScriptValue();
   return "";
}

bool MEInspectTagList::onAdd()
{
   if(!Parent::onAdd())
      return false;
   SimObject::assignName(stringTable.insert("InspectTagList"));
   //get the fonts
   font = SimResource::loadByTag(manager, IDFNT_EDITOR, true);
   return true;
}

void MEInspectTagList::reset(bool in_locked)
{
   locked = in_locked;
   resize(position, Point2I(0,1));
   Vector<Entry>::iterator i;
   if(root)
      root->makeFirstResponder(NULL);

   for(i = entries.begin(); i != entries.end(); i++)
      if(i->ectrl)
         i->ectrl->deleteObject();
   entries.clear();
}

void MEInspectTagList::addDescription(int y, const char *tagStr, const char *stBuf, int type)
{
   Entry e;
   e.y = y;
   e.ectrl = NULL;
   strncpy(e.name, tagStr, MaxNameLen);
   e.name[MaxNameLen] = 0;
   e.strWidth = font->getStrWidth(tagStr);
   Point2I ext = extent;
   ext.y = y + font->getHeight() + 4;
   ext.x = min((int)MaxWidth, max(e.strWidth+1, (int)ext.x));
   resize(position, ext);
   if(type == TextField)
   {
      METextEdit *ctrl = new METextEdit;
      ctrl->active = !locked;
      Point2I pos(0, y);
      Point2I ext(parent->extent.x, font->getHeight() + 4);
      manager->registerObject(ctrl);
      parent->addObject(ctrl);
      ctrl->resize(pos, ext);
      ctrl->setScriptValue(stBuf);
      
      e.ectrl = ctrl;
   }
   else if(type == Boolean)
   {
      MECheck *ctrl = new MECheck;
      ctrl->active = !locked;
      Point2I pos(0, y+2);
      Point2I ext(12, 12);
      manager->registerObject(ctrl);
      parent->addObject(ctrl);
      ctrl->setText("");
      ctrl->resize(pos, ext);
      ctrl->setScriptValue(stBuf);
      e.ectrl = ctrl;
   }
   entries.push_back(e);
}

MEPopup *MEInspectTagList::addComboBox(int y, const char *tagStr)
{
   Entry e;
   e.y = y;
   e.ectrl = NULL;
   strncpy(e.name, tagStr, MaxNameLen);
   e.name[MaxNameLen] = 0;
   e.strWidth = font->getStrWidth(tagStr);
   Point2I ext = extent;
   ext.y = y + font->getHeight() + 4;
   ext.x = min((int)MaxWidth, max(e.strWidth+1, (int)ext.x));
   resize(position, ext);

   MEPopup *cb = new MEPopup;
   cb->active = !locked;
   Point2I pos(0, y);
   Point2I cext(parent->extent.x, font->getHeight() + 4);
   manager->registerObject(cb);
   parent->addObject(cb);
   cb->resize(pos, cext);

   e.ectrl = cb;
   Point2I cpos(0, y);
   entries.push_back(e);

   return cb;
}

void MEInspectTagList::addDesFinished()
{
   Vector<Entry>::iterator i;
   for(i = entries.begin(); i != entries.end(); i++)
   {
      if(i->ectrl)
      {
         Point2I pos = i->ectrl->getPosition();
         Point2I ext = i->ectrl->getExtent();
         pos.x = extent.x + 1;
         ext.x = 256; //min(ext.x, parent->extent.x - extent.x - 1);
         i->ectrl->resize(pos, ext);
      }
   }
}

void MEInspectTagList::onRender(GFXSurface *sfc, Point2I offset, const Box2I& box)
{
   Parent::onRender(sfc,offset,box);
   Vector<Entry>::iterator i;
   for(i = entries.begin(); i != entries.end(); i++)
   {
      Point2I p(offset.x + extent.x - i->strWidth, offset.y + i->y);
      if(p.x < offset.x)
         p.x = offset.x;
      sfc->drawText_p(font, &p, i->name);
   }
}

//------------------------------------------------------------------

class MEObjectList : public SimGui::ArrayCtrl
{
   typedef SimGui::ArrayCtrl Parent;
   Resource<GFXFont> hFont;
   Resource<GFXFont> hSelFont;
   MEInspectTagList *itl;

   struct DOEntry
   {
      int level;
      SimObject *object;
      bool lastInGroup;
      int parentIndex;
      DOEntry(int in_level, SimObject *in_object, bool in_last, int in_pi)
         { level = in_level; object = in_object; lastInGroup = in_last; parentIndex = in_pi; }
   };

   Vector<DOEntry> displayObjects;
   Resource<GFXBitmapArray> bma;
   Resource<GFXBitmapArray> selbma;
   Inspect inspector;
   struct GroupEntry
   {
      SimGroup *group;
      int displayWorld;
   };

   Vector<GroupEntry> displayGroups;
   SimObject *inspectObject;

   enum
   {
      SingleSelect = 0,
      GroupRecurse = 1,
   };
   int selMode;

	enum BitmapIndices
	{
		BmpNone = -1,
		BmpChildAbove,         
		BmpChildBelow,         
		BmpChildBetween,
		       
		BmpParentOpen,
		BmpParentOpenAbove,    
		BmpParentOpenBelow,    
		BmpParentOpenBetween,  
		         
		BmpParentClosed,
		BmpParentClosedAbove,  
		BmpParentClosedBelow,  
		BmpParentClosedBetween,
		       
		BmpParentContinue,
		       
		BmpCount,
	};
   
public:
   MEObjectList();
   ~MEObjectList();
   bool onAdd();
   void onWake();
   
	bool becomeFirstResponder() { return FALSE; }
   void onMouseDown(const SimGui::Event &event);
   
   void addDisplayGroup(int world, SimGroup *group);
   void buildObjectDisplayList(SimGroup *, int, int);
   void onPreRender();
   void onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool selected, bool mouseOver);
   void inspect(SimObject *);
   int addInspectComboBox(int y, int tagDes, MEPopup **tl);
   int addInspectControl(int y, int tagDes, const char *buf, int type);

   void apply();
   void onDeleteNotify(SimObject *object);

   void setSelMode(int mode) { selMode = mode; }
   void clearSelection();
   void selectObject(int world, const char *object);
   void unselectObject(int world, const char *object);
   void expandToObject(int world, const char *object);
   void selectObject(SimObject *object);
   void unselectObject(SimObject *object);
   void clearDisplayGroups();

   DECLARE_PERSISTENT(MEObjectList);
};

IMPLEMENT_PERSISTENT_TAG(MEObjectList, FOURCC('M','e','O','L'));

static const char *MEApply(CMDConsole *, int name, int, const char **)
{
   MEObjectList *gMEObjectList;
   SimGui::Control *ctrl = SimGui::findControl((const char *) name);
   if(!ctrl)
      return "False";
   gMEObjectList = dynamic_cast<MEObjectList *>(ctrl);

   if(gMEObjectList)
      gMEObjectList->apply();
   return "True";
}

static const char *MEClearDisplayGroups(CMDConsole *, int name, int, const char **)
{
   MEObjectList *gMEObjectList;
   SimGui::Control *ctrl = SimGui::findControl((const char *) name);
   if(!ctrl)
      return "False";
   gMEObjectList = dynamic_cast<MEObjectList *>(ctrl);

   if(gMEObjectList)
      gMEObjectList->clearDisplayGroups();
   return "True";
}

//------------------------------------------------------------------

static const char *MESetSelMode(CMDConsole *, int name, int argc, const char **argv)
{
   MEObjectList *gMEObjectList;
   SimGui::Control *ctrl = SimGui::findControl((const char *) name);
   if(!ctrl)
      return "False";
   gMEObjectList = dynamic_cast<MEObjectList *>(ctrl);

   if(!gMEObjectList || argc != 2)
      return "False";

   gMEObjectList->setSelMode(atoi(argv[1]));
   return "True";
}

static const char *MEClearSelection(CMDConsole *, int name, int argc, const char **)
{
   MEObjectList *gMEObjectList;
   SimGui::Control *ctrl = SimGui::findControl((const char *) name);
   if(!ctrl)
      return "False";
   gMEObjectList = dynamic_cast<MEObjectList *>(ctrl);

   if(!gMEObjectList || argc != 1)
      return "False";

   gMEObjectList->clearSelection();
   return "True";
}

static const char *MEInspect(CMDConsole *, int name, int argc, const char **argv)
{
   MEObjectList *gMEObjectList;
   SimGui::Control *ctrl = SimGui::findControl((const char *) name);
   if(!ctrl)
      return "False";
   gMEObjectList = dynamic_cast<MEObjectList *>(ctrl);

   if(argc != 3)
      return "False";
   SimManager *smanager = SimGame::get()->getManager(SimGame::WorldType(atoi(argv[1])));
   if(!smanager)
      return "False";
   SimObject *object = smanager->findObject(argv[2]);
   gMEObjectList->inspect(object);
   return "True";
}

static const char *MEAddDisplayGroup(CMDConsole *, int name, int argc, const char **argv)
{
   MEObjectList *gMEObjectList;
   SimGui::Control *ctrl = SimGui::findControl((const char *) name);
   if(!ctrl)
      return "False";
   gMEObjectList = dynamic_cast<MEObjectList *>(ctrl);

   SimManager *smanager;
   if(argc != 3 || !bool(smanager = SimGame::get()->getManager(SimGame::WorldType(atoi(argv[1])))))
   {
      Console->printf("%s(worldId, group);");
      return "False";
   }
   if(smanager && gMEObjectList)
   {
      SimGroup *displayGroup = (SimGroup *) smanager->findObject(argv[2]);
      int displayWorld = atoi(argv[1]);
      gMEObjectList->addDisplayGroup(displayWorld, displayGroup);
   }
   return "True";
}      

static const char *MESelectObject(CMDConsole *, int name, int argc, const char **argv)
{
   MEObjectList *gMEObjectList;
   SimGui::Control *ctrl = SimGui::findControl((const char *) name);
   if(!ctrl)
      return "False";
   gMEObjectList = dynamic_cast<MEObjectList *>(ctrl);

   if(!gMEObjectList || argc != 3)
      return "False";

   gMEObjectList->selectObject(atoi(argv[1]), argv[2]);
   return "True";
}

static const char *MEUnselectObject(CMDConsole *, int name, int argc, const char **argv)
{
   MEObjectList *gMEObjectList;
   SimGui::Control *ctrl = SimGui::findControl((const char *) name);
   if(!ctrl)
      return "False";
   gMEObjectList = dynamic_cast<MEObjectList *>(ctrl);

   if(!gMEObjectList || argc != 3)
      return "False";

   gMEObjectList->unselectObject(atoi(argv[1]), argv[2]);
   return "True";
}

static const char *MEExpandToObject(CMDConsole *, int name, int argc, const char **argv)
{
   MEObjectList *gMEObjectList;
   SimGui::Control *ctrl = SimGui::findControl((const char *) name);
   if(!ctrl)
      return "False";
   gMEObjectList = dynamic_cast<MEObjectList *>(ctrl);

   if(!gMEObjectList || argc != 3)
      return "False";

   gMEObjectList->expandToObject(atoi(argv[1]), argv[2]);
   return "True";
}

//------------------------------------------------------------------

MEObjectList::MEObjectList()
{
   inspectObject = NULL;
   selMode = SingleSelect;
}

MEObjectList::~MEObjectList()
{
}

void MEObjectList::clearDisplayGroups()
{
   Vector<GroupEntry>::iterator i;
   for(i = displayGroups.begin(); i != displayGroups.end(); i++)
      clearNotify(i->group);
   displayGroups.setSize(0);
}

void MEObjectList::addDisplayGroup(int world, SimGroup *group)
{
   GroupEntry e;
   e.group = group;
   e.displayWorld = world;
   deleteNotify(group);
   displayGroups.push_back(e);
}

void MEObjectList::onWake()
{
   Parent::onWake();
   setSize(Point2I(1,0));
   selectedCell.set(-1, -1);
   SimGui::Control *p = (SimGui::Control *) getParent();
   while(p->getParent())
      p = (SimGui::Control *) p->getParent();
   p->flags.set(SimGui::Control::ArrowsDontMakeFirstResponder);
}

bool MEObjectList::onAdd()
{
   if(!Parent::onAdd())
      return false;

   if(!getName())
      getGroup()->assignName(this, "MissionObjectList");

   int name = (int) getName();

   char buf[256];
   sprintf(buf, "%s::Apply", getName());
   Console->addCommand(name, buf, MEApply);
   sprintf(buf, "%s::Inspect", getName());
   Console->addCommand(name, buf, MEInspect);
   sprintf(buf, "%s::ClearDisplayGroups", getName());
   Console->addCommand(name, buf, MEClearDisplayGroups);
   sprintf(buf, "%s::AddDisplayGroup", getName());
   Console->addCommand(name, buf, MEAddDisplayGroup);
   sprintf(buf, "%s::SetSelMode", getName());
   Console->addCommand(name, buf, MESetSelMode);
   sprintf(buf, "%s::ClearSelection", getName());
   Console->addCommand(name, buf, MEClearSelection);
   sprintf(buf, "%s::SelectObject", getName());
   Console->addCommand(name, buf, MESelectObject);
   sprintf(buf, "%s::UnselectObject", getName());
   Console->addCommand(name, buf, MEUnselectObject);
   sprintf(buf, "%s::ExpandToObject", getName());
   Console->addCommand(name, buf, MEExpandToObject);
                   
   //get the fonts
   hFont = SimResource::loadByName(manager, "mefont.pft", true);
   hSelFont = SimResource::loadByName(manager, "mefonthl.pft", true);
   
   //load the pba
   bma = SimResource::loadByName(manager, "METreeView.PBA", true);
   selbma = SimResource::loadByName(manager, "METreeViewSel.PBA", true);

   // remapping code:
/*   for(int i = 0; i < BmpCount; i++)
   {
      GFXBitmap *bmp = bma->getBitmap(i);
      for(int j = 0; j < bmp->getWidth();j++)
         for(int k = 0; k < bmp->getHeight(); k++)
            if(*(bmp->getAddress(j,k)) != 0)
               *(bmp->getAddress(j,k)) = 244;
      bmp->paletteIndex = 0xFFFFFFFF;
   }
   bma->write("METreeViewSel.PBA", 0);
   for(int i = 0; i < BmpCount; i++)
   {
      GFXBitmap *bmp = bma->getBitmap(i);
      for(int j = 0; j < bmp->getWidth();j++)
         for(int k = 0; k < bmp->getHeight(); k++)
            if(*(bmp->getAddress(j,k)) != 0)
               *(bmp->getAddress(j,k)) = 255;
      bmp->paletteIndex = 0xFFFFFFFF;
   }
   bma->write("METreeView.PBA", 0);*/

   for (int i = 0; i < BmpCount; i++)
   {
      bma->getBitmap(i)->attribute |= BMA_TRANSPARENT;
      selbma->getBitmap(i)->attribute |= BMA_TRANSPARENT;
   }
   cellSize.set(640, hFont->getHeight() + 2);
   return true;
}

//------------------------------------------------------------------

int MEObjectList::addInspectComboBox(int y, int tagDes, MEPopup **tl)
{
   StringTableEntry tag = tagDictionary.idToString(tagDes);
   *tl = itl->addComboBox(y, tag);
   return itl->extent.y + 1;
}

int MEObjectList::addInspectControl(int y, int tagDes, const char *buf, int type)
{
   StringTableEntry tag;
   if(!tagDes)
      tag = buf;
   else
      tag = tagDictionary.idToString(tagDes);
   itl->addDescription(y, tag, buf, type);
   return itl->extent.y + 1;
}

void MEObjectList::apply()
{
   if(!inspectObject)
      return;

   inspector.top();
   Inspect out;

   out.clear();

   Inspect::Type type;
   SimTag des;
   char buf[256];
   Vector<MEInspectTagList::Entry>::iterator i;
   i = itl->entries.begin();

   while((type = inspector.peekNextType()) != Inspect::Type_Invalid)
   {
      des = inspector.peekNextDes();
      switch(type)
      {
         case Inspect::Type_Divider:
            inspector.readDivider();
            out.writeDivider();
            break;
         case Inspect::Type_Point3F: {
            Point3F val;
            inspector.read(des, val);
            sscanf(i->getValue(), "%f %f %f", &val.x, &val.y, &val.z);
            out.write(des, val);
            i++;
            break;
         }
         case Inspect::Type_Point2F: {
            Point2F val;
            inspector.read(des, val);
            sscanf(i->getValue(), "%f %f", &val.x, &val.y);
            out.write(des, val);
            i++;
            break;
         }
         case Inspect::Type_Point3I: {
            Point3I val;
            inspector.read(des, val);
            sscanf(i->getValue(), "%d %d %d", val.x, val.y, val.z);
            out.write(des, val);
            i++;
            break;
         }
         case Inspect::Type_Point2I: {
            Point2I val;
            inspector.read(des, val);
            sscanf(i->getValue(), "%d %d", val.x, val.y);
            out.write(des, val);
            i++;
            break;
         }
         case Inspect::Type_Bool: {
            Bool val;
            inspector.read(des, val);
            val = CMDConsole::getBool(i->getValue());
            out.write(des, val);
            i++;
            break;
         }
         case Inspect::Type_RealF: {
            RealF val;
            inspector.read(des, val);
            val = atof(i->getValue());
            out.write(des, val);
            i++;
            break;
         }
         case Inspect::Type_Int32: {
            Int32 val;
            inspector.read(des, val);
            val = atoi(i->getValue());
            out.write(des, val);
            i++;
            break;
         }
         case Inspect::Type_IString: {
            char val[Inspect::MAX_STRING_LEN + 1];
            inspector.read(des, val);
            out.write(des, (Inspect::IString)i->getValue());
            i++;
            break;
         }
         case Inspect::Type_Description: {
            inspector.read(des);
            out.write(des);
            i++;
            break;
         }
         case Inspect::Type_StringDescription: {
            const char *val;
            inspector.readStringDes(val);
            out.writeStringDes(val);
            i++;
            break;
         }
         case Inspect::Type_StringIndexDescription: {
            const char *val;
            int index;
            inspector.readStringIndexDes(val, index);
            out.writeStringIndexDes(val, index);
            i++;
            break;
         }

         // unsupported types:
         case Inspect::Type_Tag: {
            Inspect_Tag tag;
            inspector.read(des, &tag.listDefines, tag.filter, tag.tagVal );
            out.write(des, tag.listDefines, tag.filter, atoi(i->getValue()));
            i++;
            break;
         }
         case Inspect::Type_TagRange: {
            Inspect_TagRange tag;
            inspector.read(des, &tag.listDefines, &tag.minTag, &tag.maxTag, tag.initTag );
            out.write(des, tag.listDefines, tag.minTag, tag.maxTag, atoi(i->getValue()));
            i++;
            break;
         }
         case Inspect::Type_ActionBtn: {
            Inspect_ActionBtn val;

            inspector.readActionButton(des, &val.recipient, &val.simMessage);
            out.writeActionButton(des, val.recipient, val.simMessage);
            break;
         }
         case Inspect::Type_List: {
            switch(inspector.peekNextListType())
            {
               case Inspect::Type_SimTagValPair: {
                  Int32 stvLen = inspector.peekNextListSize();
                  Inspect::SimTagValPair *stvList = new Inspect::SimTagValPair[stvLen];
                  Int32 val;                        
                  inspector.read(des, val, stvLen, stvList);
                  out.write(des, atoi(i->getValue()), stvLen, stvList);
                  i++;
                  delete[] stvList;
                  break;
               }
               case Inspect::Type_IStringValPair: {
                  Int32 svLen = inspector.peekNextListSize();
                  Inspect::IStringValPair *svList = new Inspect::IStringValPair[svLen];
                  Int32 val;

                  inspector.read(des, val, svLen, svList);
                  out.writeIStringPairList(des, atoi(i->getValue()), svLen, svList);
                  i++;
                  delete[] svList;
                  break;
               }
            }
            break;
         }
         default:
            return;
      }
   }
   out.top();
   inspectObject->inspectRead(&out);
   inspect(inspectObject);
}

//------------------------------------------------------------------

void MEObjectList::onDeleteNotify(SimObject *object)
{
   if(object == inspectObject)
   {
      inspectObject = NULL;
      itl->reset(true);
   }
   Parent::onDeleteNotify(object);
}

void MEObjectList::inspect(SimObject *object)
{
   if(inspectObject)
      clearNotify(inspectObject);
   inspectObject = object;
   inspector.clear();

   itl = (MEInspectTagList *) findRootNamedControl(stringTable.insert("InspectTagList"));
   if(!itl)
      return;
   itl->reset(object ? object->isLocked() : true);

   if(!inspectObject)
      return;

   deleteNotify(inspectObject); // should work across managers just fine

   int curY = 1;
   object->inspectWrite(&inspector);
   inspector.top();
   Inspect::Type type;
   SimTag des;
   char buf[256];

   while((type = inspector.peekNextType()) != Inspect::Type_Invalid)
   {
      des = inspector.peekNextDes();
      switch(type)
      {
         case Inspect::Type_Divider:
            curY += 10;
            inspector.readDivider();
            break;
         case Inspect::Type_Point3F: {
            Point3F val;
            inspector.read(des, val);
            sprintf(buf, "%g %g %g", val.x, val.y, val.z);
            curY = addInspectControl(curY, des, buf, TextField);
            break;
         }
         case Inspect::Type_Point2F: {
            Point2F val;
            inspector.read(des, val);
            sprintf(buf, "%g %g", val.x, val.y);
            curY = addInspectControl(curY, des, buf, TextField);
            break;
         }
         case Inspect::Type_Point3I: {
            Point3I val;
            inspector.read(des, val);
            sprintf(buf, "%d %d %d", val.x, val.y, val.z);
            curY = addInspectControl(curY, des, buf, TextField);
            break;
         }
         case Inspect::Type_Point2I: {
            Point2I val;
            inspector.read(des, val);
            sprintf(buf, "%d %d", val.x, val.y);
            curY = addInspectControl(curY, des, buf, TextField);
            break;
         }
         case Inspect::Type_Bool: {
            Bool val;
            inspector.read(des, val);
            curY = addInspectControl(curY, des, val ? "True" : "False", Boolean);
            break;
         }
         case Inspect::Type_RealF: {
            RealF val;
            inspector.read(des, val);
            sprintf(buf, "%g", val);
            curY = addInspectControl(curY, des, buf, TextField);
            break;
         }
         case Inspect::Type_Int32: {
            Int32 val;
            inspector.read(des, val);
            sprintf(buf, "%d", val);
            curY = addInspectControl(curY, des, buf, TextField);
            break;
         }
         case Inspect::Type_IString: {
            char val[Inspect::MAX_STRING_LEN + 1];
            inspector.read(des, val);
            curY = addInspectControl(curY, des, val, TextField);
            break;
         }
         case Inspect::Type_Description: {
            inspector.read(des);
            curY = addInspectControl(curY, des, NULL, Description);
            break;
         }
         case Inspect::Type_StringDescription: {
            const char *val;
            inspector.readStringDes(val);
            curY = addInspectControl(curY, 0, val, Description);
            break;
         }
         case Inspect::Type_StringIndexDescription: {
            const char *val;
            int index;
            inspector.readStringIndexDes(val, index);
            sprintf(buf, "%s (%d)", val, index);
            curY = addInspectControl(curY, 0, buf, Description);
            break;
         }

         // unsupported types:
         case Inspect::Type_Tag: {
            Inspect_Tag tag;
            inspector.read(des, &tag.listDefines, tag.filter, tag.tagVal );
            MEPopup *tl;
            curY = addInspectComboBox(curY, des, &tl);
            if(tag.filter)
            {
               Vector<int> v;
               char *searchStrBeg = tag.filter;
               char *searchStrEnd;
               bool done = false;
               while (! done)
               {
                  // filter can have multiple search strings separated by commas
                  searchStrEnd = searchStrBeg;
                  while (*searchStrEnd != ',')
                  {
                     if (*searchStrEnd == '\0')
                     {
                        done = true;
                        break;
                     }
                     searchStrEnd++;
                  }
                  *searchStrEnd = '\0';

                  v.clear();
                  tagDictionary.findDefines(v, searchStrBeg);
                  for (Vector<int>::iterator iter = v.begin(); iter != v.end(); iter++)
                  {
                     tl->addEntry(tag.listDefines ? 
                        tagDictionary.idToDefine(*iter) :
                        tagDictionary.idToString(*iter), *iter);
                  }               
               }
            }
            tl->setSelected(tag.tagVal);
            break;
         }
         case Inspect::Type_TagRange: {
            Inspect_TagRange tag;
            inspector.read(des, &tag.listDefines, &tag.minTag, &tag.maxTag, tag.initTag );
            MEPopup *tl;
            curY = addInspectComboBox(curY, des, &tl);
            Vector<int> v;
            tl->addEntry("<NONE>", 0);
            tagDictionary.findIDs(v, tag.minTag, tag.maxTag);
            for(Vector<int>::iterator i = v.begin(); i != v.end(); i++)
            {
               const char *name;
               if(tag.listDefines)
                  name = tagDictionary.idToDefine(*i);
               else
                  name = tagDictionary.idToString(*i);
               tl->addEntry(name, *i);
            }
            tl->setSelected(tag.initTag);
            break;
         }
         case Inspect::Type_ActionBtn: {
            Inspect_ActionBtn val;

            inspector.readActionButton(des, &val.recipient, &val.simMessage);
            break;
         }
         case Inspect::Type_List: {
            switch(inspector.peekNextListType())
            {
               case Inspect::Type_SimTagValPair: {
                  Int32 stvLen = inspector.peekNextListSize();
                  Inspect::SimTagValPair *stvList = new Inspect::SimTagValPair[stvLen];
                  Int32 val;                        
                  inspector.read(des, val, stvLen, stvList);
                  MEPopup *tl;
                  curY = addInspectComboBox(curY, des, &tl);
                  int i;
                  for(i = 0; i < stvLen; i++)
                     tl->addEntry(tagDictionary.idToString(stvList[i].tag), stvList[i].val);

                  delete[] stvList;
                  break;
               }
               case Inspect::Type_IStringValPair: {
                  Int32 svLen = inspector.peekNextListSize();
                  Inspect::IStringValPair *svList = new Inspect::IStringValPair[svLen];
                  Int32 val;

                  inspector.read(des, val, svLen, svList);
                  MEPopup *tl;
                  curY = addInspectComboBox(curY, des, &tl);
                  int i;
                  for(i = 0; i < svLen; i++)
                     tl->addEntry(svList[i].string, svList[i].val);

                  delete[] svList;
                  break;
               }
            }
            break;
         }
         default:
            return;
      }
   }
   itl->addDesFinished();
}

//------------------------------------------------------------------

void MEObjectList::buildObjectDisplayList(SimGroup *group, int level, int parentIndex)
{
   SimGroup::iterator i;
   for(i = group->begin(); i != group->end(); i++)
   {
      displayObjects.push_back(DOEntry(level, *i, i+1 == group->end(), parentIndex));
      SimGroup *g = dynamic_cast<SimGroup *>(*i);
      if(g && g->isExpanded())
         buildObjectDisplayList(g, level + 1, displayObjects.size() - 1);
   }
}

void MEObjectList::onPreRender()
{
   displayObjects.clear();
   Vector<GroupEntry>::iterator i;
   for(i = displayGroups.begin(); i != displayGroups.end(); i++)
   {
      displayObjects.push_back(DOEntry(0, i->group, true, -1));
      if(i->group->isExpanded())
         buildObjectDisplayList(i->group, 1, displayObjects.size() - 1);
   }
   setSize(Point2I(1, displayObjects.size()));
   setUpdate();
}

void MEObjectList::onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool, bool)
{
   Point2I cellOffset = offset;
   
   GFXBitmap *bmp;
   int bmpIndex;
   int statusWidth = bma->getBitmap(BmpParentContinue)->getWidth();
   
   DOEntry *cur = &(displayObjects[cell.y]);
   DOEntry *prev = cell.y ? &(displayObjects[cell.y - 1]) : NULL;

   //draw the background rectangle
   bool sel = cur->object->isSelected();
   if (sel)
   {
      RectI selRect(cellOffset, cellOffset);
      selRect.lowerR.x += extent.x - 1;
      selRect.lowerR.y += extent.y - 1;
      selRect.upperL.x += 2;
      
      sfc->drawRect2d_f(&selRect, 255);
   }
   SimSet *grp = dynamic_cast<SimSet *>(cur->object);
   if(grp && grp->size())
   {
      if(grp->isExpanded())
         bmpIndex = BmpParentOpen;
      else
         bmpIndex = BmpParentClosed;
   }
   else
      bmpIndex = BmpNone;

   //add to the index based on the status
   if (! prev || cur->level == 0)
   {
      if (!cur->lastInGroup)
         bmpIndex += 1;
   }
   else if (cur->lastInGroup)
      bmpIndex += 2;
   else
      bmpIndex += 3;
   
   //now we have our bmpIndex, draw the status
   if (bmpIndex >= 0)
   {
      bmp = sel ? selbma->getBitmap(bmpIndex) : bma->getBitmap(bmpIndex);
      sfc->drawBitmap2d(bmp, &Point2I(cellOffset.x + (cur->level * statusWidth), cellOffset.y));
   }
   
   //draw in all the required continuation lines
   bmp = sel ? selbma->getBitmap(BmpParentContinue) : bma->getBitmap(BmpParentContinue);
   int parent = cur->parentIndex;
   while(parent != -1)
   {
      DOEntry *p = &(displayObjects[parent]);
      if (!p->lastInGroup)
         sfc->drawBitmap2d(bmp, &Point2I(cellOffset.x + (p->level * statusWidth), cellOffset.y));
      parent = p->parentIndex;
   }
   
   char buf[256];
   sprintf(buf, "%d: %s - %s", cur->object->getId(), cur->object->getName() ? cur->object->getName() : "", cur->object->getClassName());
   sfc->drawText_p(sel ? hSelFont : hFont, &Point2I(cellOffset.x + ((cur->level + 1) * statusWidth), cellOffset.y - 2), buf);
}

//------------------------------------------------------------------

void MEObjectList::onMouseDown(const SimGui::Event &event)
{
   if(!active)
   {
      Parent::onMouseDown(event);
      return;
   }
   Point2I pt = globalToLocalCoord(event.ptMouse);
   
   //find out which cell was hit
   Point2I cell((pt.x < 0 ? -1 : pt.x / cellSize.x), (pt.y < 0 ? -1 : pt.y / cellSize.y));
   if(cell.x >= 0 && cell.x < size.x && cell.y >= 0 && cell.y < size.y)
   {
      //see where in the cell the hit happened
      DOEntry *hit = &displayObjects[cell.y];
      int statusWidth = bma->getBitmap(BmpParentContinue)->getWidth();
      int statusOffset = statusWidth * hit->level;
      
      //if we clicked on the expanded icon
      SimSet *grp = dynamic_cast<SimSet *>(hit->object);
      if (pt.x >= statusOffset && pt.x <= statusOffset + statusWidth && grp && grp->size())
         grp->setExpanded(!grp->isExpanded());
      
      //if we clicked on the object's name...
      else if (pt.x >= 2 + ((hit->level + 1) * statusWidth))
      {
         //find the player and set the selected cell(s)
         if( CMDConsole::getLocked()->getBoolVariable( "ME::Mod2", false ) ) // shift
         {
            SimGroup *parent = hit->object->getGroup();
            if(parent)
            {
               SimGroup::iterator i;
               bool before = false;
               for(i = parent->begin(); i != parent->end(); i++)
               {
                  if((*i)->isSelected())
                  {
                     before = true;
                     break;
                  }
                  if((*i) == hit->object)
                     break;
               }
               if(before)
               {
                  for(; *i != hit->object; i++)
                     selectObject(*i);
                  selectObject(hit->object);
                  return;
               }
               SimGroup::iterator j = i;
               // make sure something is selected after us...
               for(;j != parent->end();j++)
               {
                  if((*j)->isSelected())
                  {
                     for(;i != j; i++)
                        selectObject(*i);
                     return;
                  }
               }
               // fall through
            }
         }
         
         //toggle the object selected
         if( CMDConsole::getLocked()->getBoolVariable( "ME::Mod1", false ) ) // control
         {
            if(hit->object->isSelected())
               unselectObject(hit->object);
            else
               selectObject(hit->object);
            return;
         }
         clearSelection();
         selectObject(hit->object);
      }
   }
}

//------------------------------------------------------------------

void MEObjectList::clearSelection()
{
   for(Vector<GroupEntry>::iterator gi = displayGroups.begin(); gi != displayGroups.end(); gi++)
   {
      SimSetIterator i(gi->group);
      gi->group->setSelected(false);
      while(*i)
      {
         (*i)->setSelected(false);
         ++i;
      }
      Console->evaluatef("%s::onSelectionCleared();", getName());
   }
}

void MEObjectList::selectObject(int world, const char *object)
{
   SimManager *smanager = SimGame::get()->getManager((SimGame::WorldType)world);
   if(!smanager)
      return;

   SimObject *sobject = smanager->findObject(object);
   if(sobject)
      selectObject(sobject);
}

void MEObjectList::selectObject(SimObject *object)
{
   int world;
   if(object->getManager() == SimGame::get()->getManager(SimGame::SERVER))
      world = SimGame::SERVER;
   else
      world = SimGame::CLIENT;

   SimSet *grp = dynamic_cast<SimSet *>(object);
   if(object->isSelected() && !grp)
      return;
   object->setSelected(true);
   Console->evaluatef("%s::onSelected(%d, %d);", getName(), world, object->getId());

   if(grp && selMode == GroupRecurse)
   {
      SimSetIterator i(grp);
      while(*i)
      {
         if(!(*i)->isSelected())
         {
            (*i)->setSelected(true);
            Console->evaluatef("%s::onSelected(%d, %d);", getName(), world, (*i)->getId());
         }
         ++i;
      }
   }
}

void MEObjectList::unselectObject(int world, const char *object)
{
   SimManager *smanager = SimGame::get()->getManager((SimGame::WorldType)world);
   if(!smanager)
      return;

   SimObject *sobject = smanager->findObject(object);
   if(sobject)
      unselectObject(sobject);
}

void MEObjectList::unselectObject(SimObject *object)
{
   if(object->isSelected())
   {
      int world;
      if(object->getManager() == SimGame::get()->getManager(SimGame::SERVER))
         world = SimGame::SERVER;
      else
         world = SimGame::CLIENT;

      object->setSelected(false);
      Console->evaluatef("%s::onUnselected(%d, %d);", getName(), world, object->getId());
   }
}

void MEObjectList::expandToObject(int world, const char *object)
{
   SimManager *smanager = SimGame::get()->getManager((SimGame::WorldType)world);
   if(!smanager)
      return;

   SimObject *sobject = smanager->findObject(object);
   if(object)
   {
      SimGroup *parent = sobject->getGroup();
      while(parent)
      {
         parent->setExpanded(true);
         parent = parent->getGroup();
      }
   }
}

//------------------------------------------------------------------
