#include <fearguipopupmenu.h>
#include <g_surfac.h>
#include <simguisimpletext.h>
#include <g_font.h>
#include <g_bitmap.h>
#include <simresource.h>
#include <darkstar.strings.h>
#include <simguidelegate.h>
#include <fear.strings.h>
#include <fearguishellpal.h>

namespace FearGui
{
FGPopUpMenu::FGPopUpMenu()
{
   pos.set(0, 0);
   ext.set(0, 0);
}

FGPopUpMenu::~FGPopUpMenu()
{
   for (int i = 0; i < menuItems.size(); i ++)
   {
      delete menuItems[i];
   }
   menuItems.clear();
}

void FGPopUpMenu::appendMenu(DWORD dwFlags, DWORD dwTag, LPCTSTR lpszText)
{
   MenuEntry *menuEntry = new MenuEntry;

   // Debug check - cannot specify MENU_STRING items with no string
   AssertFatal(!(dwFlags & MENU_STRING) || lpszText, 
      "FGPopUpMenu: check appendMenu parameters");

   if ((dwFlags & MENU_STRING) && lpszText)
   {
      // Set the text
      menuEntry->lpszText = strnew(lpszText);

      // Set the size
      menuEntry->extent.x = rsFont->getStrWidth(lpszText) + 15;
      menuEntry->extent.y = rsFont->fi.baseline + 4;
      menuEntry->offset.x = pos.x + 20;
      menuEntry->offset.y = pos.y + ext.y;
   }
   else if (dwFlags & MENU_SEPARATOR)
   {
      // Set the size
      menuEntry->extent.x = ext.x - 7;
      menuEntry->extent.y = rsFont->fi.baseline + 4;
      menuEntry->offset.x = pos.x + 5;
      menuEntry->offset.y = pos.y + ext.y;
   }

   // Common stuff
   menuEntry->fTargeted = false;
   menuEntry->dwTag     = dwTag;
   menuEntry->dwFlags   = dwFlags;

   // Minimum size should be the size of this entry, but should
   // be at least 150 pels (just to look nice)
   ext.x  = max(ext.x, max(menuEntry->extent.x + 20, 150L));
   ext.y += menuEntry->extent.y - 2;

   menuItems.push_back(menuEntry);
}

void FGPopUpMenu::setPos(const Point2I &pos)
{
   this->pos = pos;
   setUpdate();
}

void FGPopUpMenu::onWake()
{
   rsCheck   = SimResource::loadByTag(manager, IDBMP_MENU_CIRC_NORMAL,   true);
   rsCheckHL = SimResource::loadByTag(manager, IDBMP_MENU_CIRC_SELECTED, true);
   rsCheckDS = SimResource::loadByTag(manager, IDBMP_MENU_CIRC_DISABLED, true);

   rsFont    = SimResource::loadByTag(manager, IDFNT_9_STANDARD, true);
   rsFontHL  = SimResource::loadByTag(manager, IDFNT_9_HILITE,   true);
   rsFontDS  = SimResource::loadByTag(manager, IDFNT_9_DISABLED, true);

   AssertFatal(bool(rsCheck) && 
      bool(rsCheckDS) && bool(rsFont) && 
      bool(rsFontHL)  && bool(rsFontDS), "FGPopUpMenu: cannot load resources");
}

bool FGPopUpMenu::inHitArea(const Point2I &point)
{
   return (point.x >= pos.x         && point.y >= pos.y &&
           point.x <= pos.x + ext.x && point.y <= pos.y + ext.y);
}

void FGPopUpMenu::onRightMouseUp(const Event &event)
{
   onMouseUp(event);
}

void FGPopUpMenu::onMouseUp(const Event &event)
{
   if (this == root->getTopDialog())
   {
      // See if the click was within the pop-up
      if (inHitArea(event.ptMouse))
      {
         // Find the control clicked and perform its action
         for (int i = 0; i < menuItems.size(); i ++)
         {
            if (menuItems[i]->fTargeted)
            {
               // Pop this dialog
               root->makeFirstResponder(NULL);
               // Pass the message up the chain
               onMessage(this, menuItems[i]->dwTag);
               // close the dialog
               root->handleDLGClose(-1);
               break;
            }
         }
      }
      else
      {
         root->makeFirstResponder(NULL);
         //root->popDialogControl();
         root->handleDLGClose(-1);
      }
   }
}

void FGPopUpMenu::onRightMouseDragged(const Event &event)
{
   onMouseMove(event);
}

void FGPopUpMenu::onMouseDragged(const Event &event)
{
   onMouseMove(event);
}

void FGPopUpMenu::onMouseMove(const Event &event)
{
   if (this == root->getTopDialog())
   {
      // First deactivate all of them (to prevent borderline clashes)
      for (int i = 0; i < menuItems.size(); i ++)
      {
         menuItems[i]->fTargeted = false;
      }

      if (inHitArea(event.ptMouse))
      {
         // Find the control we are hovering over
         for (int i = 0; i < menuItems.size(); i ++)
         {
            Point2I &cpos = menuItems[i]->offset;
            Point2I &cext = menuItems[i]->extent;

            // If this is the control, send its message to the delegate
            if (event.ptMouse.x >= pos.x         && event.ptMouse.y >= cpos.y &&
                event.ptMouse.x <= pos.x + ext.x && event.ptMouse.y <= cpos.y + cext.y)
            {
               // Can't select separator bars
               if (!(menuItems[i]->dwFlags & MENU_SEPARATOR) && 
                   !(menuItems[i]->dwFlags & MENU_DISABLED))
               {
                  menuItems[i]->fTargeted = true;
               }
               break;
            }
         }
      }
   }
}

void FGPopUpMenu::onRender(GFXSurface *sfc, Point2I, const Box2I &)
{
   Point2I diff(0, 0);
   Resource<GFXFont>   rsUseFont;
   Resource<GFXBitmap> rsBmp;

   // Figure out, based upon our actual extent, where to draw
   // the menu
   if ((pos.x + ext.x) >= (extent.x - 1))
   {
      diff.x = pos.x + ext.x - extent.x + 1;
   }
   if ((pos.y + ext.y + 6) >= (extent.y - 1))
   {
      diff.y = pos.y + ext.y + 6 - extent.y + 1;
   }

   pos -= diff;

   sfc->drawRect2d_f(&RectI(pos.x, 
      pos.y, pos.x + ext.x, pos.y + ext.y + 6), GREEN_40);
   sfc->drawRect2d  (&RectI(pos.x, 
      pos.y, pos.x + ext.x, pos.y + ext.y + 6), GREEN_78);

   for (int i = 0; i < menuItems.size(); i ++)
   {
      rsBmp     = rsCheck;
      rsUseFont = rsFont;

      menuItems[i]->offset -= diff;

      if (menuItems[i]->dwFlags & MENU_DISABLED)
      {
         rsBmp     = rsCheckDS;
         rsUseFont = rsFontDS;
      }
      else if (menuItems[i]->fTargeted)
      {
         Point2I cpos = menuItems[i]->offset;
         Point2I cext = menuItems[i]->extent;

         sfc->drawRect2d_f(&RectI(
            pos.x + 2, cpos.y + 4, pos.x + ext.x - 3, cpos.y + cext.y), GREEN_132);

         rsBmp     = rsCheckHL;
         rsUseFont = rsFontHL;
      }

      // Draw the item
      if (menuItems[i]->dwFlags & MENU_STRING)
      {
         if (menuItems[i]->dwFlags & MENU_CHECKED)
         {
            sfc->drawBitmap2d(rsBmp,
               &Point2I(pos.x + 6, menuItems[i]->offset.y + 5));
         }

         sfc->drawText_p(rsUseFont, 
            &menuItems[i]->offset, menuItems[i]->lpszText);
      }
      else if (menuItems[i]->dwFlags & MENU_SEPARATOR)
      {
         sfc->drawLine2d(
            &Point2I(pos.x + 5, 
               menuItems[i]->offset.y + 4 + menuItems[i]->extent.y / 2),
            &Point2I(pos.x + 5 + ext.x - 11,
               menuItems[i]->offset.y + 4 + menuItems[i]->extent.y / 2),
            GREEN_78);
      }
   }
}

IMPLEMENT_PERSISTENT_TAG(FGPopUpMenu, FOURCC('F','G','p','m'));
};
