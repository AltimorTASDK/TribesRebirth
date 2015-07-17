
#include "console.h"
#include "simResource.h"
#include "g_font.h"
#include "g_surfac.h"
#include "simGuiCtrl.h"
#include "fear.strings.h"
#include "fearGlobals.h"
#include "FearGuiTSCommander.h"
#include "chatMenu.h"

namespace FearGui
{

static const int gCtrlVersion = 0;

class FGChatMenu : public SimGui::Control
{
   private:
      typedef SimGui::Control Parent;
      
      Resource<GFXFont> hFont;
      Resource<GFXFont> hFontHL;
      char mMenuName[64];
   
   public:
      FGChatMenu(void) { mMenuName[0] = '\0'; }
      
      bool onAdd(void);
      void onWake(void);
      void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);
      
      void onMouseDown(const SimGui::Event &event);

   void inspectWrite(Inspect* insp);
   void inspectRead(Inspect* insp);

   Persistent::Base::Error write( StreamIO &sio, int version, int user );
   Persistent::Base::Error read( StreamIO &sio, int version, int user );

   DECLARE_PERSISTENT(FGChatMenu);
};

bool FGChatMenu::onAdd(void)
{
   if (! Parent::onAdd()) return FALSE;
   
   hFont = SimResource::get(manager)->load("sf_Orange255_10.pft");
   AssertFatal(hFont.operator bool(), "Unable to load font.");
   hFontHL = SimResource::get(manager)->load("sf_white_10.pft");
   AssertFatal(hFontHL.operator bool(), "Unable to load font.");
   
   return TRUE;
}

void FGChatMenu::onWake(void)
{
   //make sure it is the commander chat, and that it exists
   if (! mMenuName[0]) return;
   ChatMenu *menu = dynamic_cast<ChatMenu *>(manager->findObject(mMenuName));
   if (! menu) return;
   if (stricmp(mMenuName, "CommandChatMenu")) return;
   
   //see if anyone is selected
   if (cg.commandTS && (cg.commandTS->getSelectedPlayer() >= 0))
   {
      menu->setMode(1);
   }
   else
   {
      menu->setMode(0);
   }
}

void FGChatMenu::onMouseDown(const SimGui::Event &event)
{
   //make sure we have an active menu
   if (! mMenuName[0]) return;
   ChatMenu *menu = dynamic_cast<ChatMenu *>(manager->findObject(mMenuName));
   if (! menu) return;
   
   if(menu->getMode() == ChatMenu::Inactive || !menu->curMenu) return;
   
   Point2I pt = globalToLocalCoord(event.ptMouse);
   ChatMenu::Node *walk = menu->curMenu;
   while (walk)
   {
      if (pt.y >= walk->y_offset && pt.y < walk->y_offset + hFont->getHeight())
      {
         if (walk->key != 0)
         {
            menu->processKey(walk->key);
         }
         break;
      }
      walk = walk->nextSibling;
   }
}            

void FGChatMenu::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   if (! mMenuName[0]) return;
   ChatMenu *menu = dynamic_cast<ChatMenu *>(manager->findObject(mMenuName));
   if (! menu) return;
   
   Point2I curDrawPoint(offset.x + 3, offset.y + 1);
   if(menu->getMode() == ChatMenu::Inactive || !menu->curMenu)
   {
      sfc->drawText_p(hFont, &curDrawPoint, "Menu inactive");
      return;
   }
   ChatMenu::Node *parent = menu->curMenu->parent;
   char buf[1024];

   if(!parent)
   {
      sfc->drawText_p(hFont, &curDrawPoint, "Root Menu:");
   }
   else
   {
      int lastPt = 1023;
      while(parent)
      {
         int len = strlen(parent->heading) + 1;
         if(len < lastPt)
         {
            lastPt -= len;
            strncpy(buf + lastPt, parent->heading, len + 1);
            buf[lastPt + len] = ':';
         }
         parent = parent->parent;
      }
      buf[1023] = 0;
      sfc->drawText_p(hFont, &curDrawPoint, buf + lastPt);
   }
   curDrawPoint.y += hFont->getHeight();
   
   Point2I cursorPos = root->getCursorPos();
   Point2I pt = globalToLocalCoord(cursorPos);
   
   ChatMenu::Node *walk = menu->curMenu;
   while(walk)
   {
      sprintf(buf, "%c: %s", walk->key, walk->heading);
      GFXFont *font;
      if (pt.x < 0 || pt.x >= extent.x) font = hFont;
      else if (pt.y >= curDrawPoint.y - offset.y && pt.y < curDrawPoint.y - offset.y + hFont->getHeight()) font = hFontHL;
      else font = hFont;
      sfc->drawText_p(font, &curDrawPoint, buf);
      walk->y_offset = curDrawPoint.y - offset.y;
      curDrawPoint.y += hFont->getHeight();
      walk = walk->nextSibling;
   }
}

void FGChatMenu::inspectWrite(Inspect* insp)
{
   Parent::inspectWrite(insp);
   insp->write(IDITG_OBJECT_NAME, mMenuName);
}

void FGChatMenu::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   insp->read(IDITG_OBJECT_NAME, mMenuName);
}

Persistent::Base::Error FGChatMenu::write( StreamIO &sio, int version, int user )
{
   sio.write(gCtrlVersion);
   
   BYTE len = strlen(mMenuName);
   sio.write(len);
   if(len) sio.write(len, mMenuName);
      
   return Parent::write(sio, version, user);
}

Persistent::Base::Error FGChatMenu::read( StreamIO &sio, int version, int user)
{
   int currentVersion;
   sio.read(&currentVersion);
   
   BYTE len;
   sio.read(&len);
   if(len) sio.read(len, mMenuName);
   mMenuName[len] = 0;
   
   return Parent::read(sio, version, user);
}

IMPLEMENT_PERSISTENT_TAG( FearGui::FGChatMenu,   FOURCC('F','G','c','m') );

};
