#include <string.h>
#include "console.h"
#include <g_surfac.h>
#include <g_bitmap.h>
#include <soundFX.h>
#include <inspect.h>
#include "SimGuiCtrl.h"
#include <FearDcl.h>
#include "fearGlobals.h"
#include "fear.strings.h"
#include "PlayerManager.h"
#include "player.h"
#include "FearGuiHudCtrl.h"
#include "FearGuiChatDisplay.h"
#include "simResource.h"
#include "g_font.h"
#include "chatmenu.h"
#include "fearGuiShellPal.h"

namespace FearGui
{

class MenuDisplayCtrl : public SimGui::ActiveCtrl
{
   typedef SimGui::ActiveCtrl Parent;
   Resource<GFXFont> hFont;
   Resource<GFXFont> hFontHL;
   
public:
   bool onAdd();
   void onMouseDown(const SimGui::Event &event);
   void onRender(GFXSurface *, Point2I, const Box2I &);

   DECLARE_PERSISTENT(MenuDisplayCtrl);
   Persistent::Base::Error MenuDisplayCtrl::write( StreamIO &sio, int a, int b);
   Persistent::Base::Error MenuDisplayCtrl::read( StreamIO &sio, int a, int b);
};

IMPLEMENT_PERSISTENT(MenuDisplayCtrl);

bool MenuDisplayCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;
   
   if (getName() && (! stricmp(getName(), "LowResServerMenu")))
   {
      hFont = SimResource::get(manager)->load("sf_Yellow_6.pft");
      AssertFatal(hFont.operator bool(), "Unable to load font.");
      hFontHL = SimResource::get(manager)->load("sf_white_6.pft");
      AssertFatal(hFontHL.operator bool(), "Unable to load font.");
      hFont->fi.flags |= FONT_LOWERCAPS;
      hFontHL->fi.flags |= FONT_LOWERCAPS;
   }
   else
   {
      hFont = SimResource::get(manager)->load("sf_Orange255_10.pft");
      AssertFatal(hFont.operator bool(), "Unable to load font.");
      hFontHL = SimResource::get(manager)->load("sf_white_10.pft");
      AssertFatal(hFontHL.operator bool(), "Unable to load font.");
   }
   
   return true;
}

void MenuDisplayCtrl::onMouseDown(const SimGui::Event &event)
{
   //make sure we have an active menu
   ChatMenu *menu = dynamic_cast<ChatMenu *>(manager->findObject("CurServerMenu"));
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

void MenuDisplayCtrl::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   // temporary menu stuff:
   SimObject *obj = manager->findObject("CurServerMenu");
   ChatMenu *menu = NULL;
   if(obj)
      menu = dynamic_cast<ChatMenu *>(obj);
   if(!menu)
      return;
      
   Point2I curDrawPoint(offset.x, offset.y);

   char buf[1024];

   if(!menu->curMenu || !(menu->curMenu->parent))
      sfc->drawText_p(hFont, &curDrawPoint, menu->heading);
   else if(menu->curMenu)
   {
      ChatMenu::Node *parent = menu->curMenu->parent;
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
   
   Point2I cursorPos = root->getCursorPos();
   Point2I pt = globalToLocalCoord(cursorPos);
   
   curDrawPoint.y += hFont->getHeight();
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

Persistent::Base::Error MenuDisplayCtrl::write( StreamIO &sio, int a, int b)
{
   return Parent::write(sio, a, b);
}

Persistent::Base::Error MenuDisplayCtrl::read( StreamIO &sio, int a, int b)
{
   return Parent::read(sio, a, b);
}

};