#include "simResource.h"
#include "g_surfac.h"
#include "g_bitmap.h"
#include "fear.strings.h"
#include "FearGuiArrayCtrl.h"
#include "m_qsort.h"
#include "console.h"
#include "g_font.h"
//#include "FearGuiIRCClient.h"
#include "FearGuiIRCNicks.h"
#include "fearguipopupmenu.h"

namespace FearGui
{

static int __cdecl nickNameAscend(const void *a,const void *b)
{
   FGIRCNicknames::NickRep **entry_A = (FGIRCNicknames::NickRep **)(a);
   FGIRCNicknames::NickRep **entry_B = (FGIRCNicknames::NickRep **)(b);
   return (stricmp((*entry_A)->nick, (*entry_B)->nick));
}

enum {
   
   NIName = 0,
   NICount
};

static FGArrayCtrl::ColumnInfo gNicknameInfo[NICount] =
{
   { IDSTR_NICKNAMES,          10, 300,   0,   0,    TRUE, 90, nickNameAscend, NULL },
};                                       
static FGArrayCtrl::ColumnInfo *gInfoPtrs[NICount];
                                         
IMPLEMENT_PERSISTENT_TAG(FGIRCNicknames, FOURCC('F','G','i','n'));

bool FGIRCNicknames::onAdd()
{
   if(!Parent::onAdd())
      return false;

   //reset the fonts
   hFont = SimResource::loadByTag(manager, IDFNT_9_STANDARD, true);
   hFontHL = SimResource::loadByTag(manager, IDFNT_9_STATIC, true);
   hFontNA = SimResource::loadByTag(manager, IDFNT_9_DISABLED, true);
   hFontMO = SimResource::loadByTag(manager, IDFNT_9_HILITE, true);
   
   //load the icons
   mAwayBMP = SimResource::get(manager)->load("irc_icon_away.bmp");
   AssertFatal(mAwayBMP, "Unable to load irc_icon_away.bmp");
   mAwayBMP->attribute |= BMA_TRANSPARENT;
   
   mIgnoreBMP = SimResource::get(manager)->load("irc_icon_Ignore.bmp");
   AssertFatal(mIgnoreBMP, "Unable to load irc_icon_Ignore.bmp");
   mIgnoreBMP->attribute |= BMA_TRANSPARENT;
   
   mLockBMP = SimResource::get(manager)->load("irc_icon_Lock.bmp");
   AssertFatal(mLockBMP, "Unable to load irc_icon_Lock.bmp");
   mLockBMP->attribute |= BMA_TRANSPARENT;
   
   mMesgBMP = SimResource::get(manager)->load("irc_icon_Mesg.bmp");
   AssertFatal(mMesgBMP, "Unable to load irc_icon_Mesg.bmp");
   mMesgBMP->attribute |= BMA_TRANSPARENT;
   
   mOperBMP = SimResource::get(manager)->load("irc_icon_Oper.bmp");
   AssertFatal(mOperBMP, "Unable to load irc_icon_Oper.bmp");
   mOperBMP->attribute |= BMA_TRANSPARENT;
   
   mSpecBMP = SimResource::get(manager)->load("irc_icon_Spec.bmp");
   AssertFatal(mSpecBMP, "Unable to load irc_icon_Spec.bmp");
   mSpecBMP->attribute |= BMA_TRANSPARENT;
   
   mSpkrBMP = SimResource::get(manager)->load("irc_icon_Spkr.bmp");
   AssertFatal(mSpkrBMP, "Unable to load irc_icon_Spkr.bmp");
   mSpkrBMP->attribute |= BMA_TRANSPARENT;
   
   //other misc vars
   refresh = TRUE;
	numColumns = NICount;
	columnInfo = gInfoPtrs;
   
   //set the ptrs table
   for (int i = 0; i < NICount; i++)
   {
      gInfoPtrs[i] = &gNicknameInfo[i];
   }
   
   //set the cell dimensions
   int width = 0;
   for (int k = 0; k < NICount; k++)
   {
      width += gInfoPtrs[k]->width;
   }
   cellSize.set(width, hFont->getHeight() + 6);
   
   prevSelected.set(-1, -1);
   
   //find the IRC client
   mIRCClient = ::IRCClient::find(manager);
   AssertFatal(mIRCClient, "IRCChatDelegate: unable to locate IRC client");
   
   return true;
}

void FGIRCNicknames::onWake()
{
   cellSize.set(max(cellSize.x, parent->extent.x), cellSize.y);
   setSize(Point2I( 1, 0));
   refresh = TRUE;
}

void FGIRCNicknames::onPreRender()
{
   // Keep the cell size up to date
   cellSize.set(max(cellSize.x, parent->extent.x), cellSize.y);
   
   // Build up the list of nicknames
   entries.clear();
   entryPtrs.clear();
   
   // Get the current channel
   ::IRCClient::Channel *channel = mIRCClient->findChannel(NULL);

   if (channel)
   {
      // Build up the nickname list
      for (int i = 0; i < channel->members.size(); i ++)
      {
         // Add the client to the list
         NickRep newNick;

         strcpy(newNick.nick, channel->members[i].person->nick);
         newNick.status = 
            (channel->members[i].person->flags | channel->members[i].flags); 
         entries.push_back(newNick);
      }

      setSize(Point2I(1, entries.size()));
   
      // Set up the pointers array
      for (int j = 0; j < entries.size(); j++)
      {
         entryPtrs.push_back(&entries[j]);
      }
   }
   else
   {
      setSize(Point2I(1, 0));
   }
}

void FGIRCNicknames::onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool sel, bool mouseOver)
{
   IRCClient::Channel *channel = mIRCClient->findChannel(NULL);
   Point2I parentOffset = parent->localToGlobalCoord(Point2I(0, 0));
   
   bool ghosted = FALSE;
   if (root)
   {
      SimGui::Control *topDialog = root->getDialogNumber(1);
      if ((! active) || 
          (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG))))
      {
         ghosted = TRUE;
      }
   }
   
   //initialize the font
   GFXFont *font;
   if (ghosted || (! cellSelectable(cell))) font = hFontNA;
   else if (sel) font = hFontHL;
   else if (mouseOver) font = hFontMO;
   else font = hFont;
   
   //initialize the draw offset
   Point2I drawOffset = offset;
   drawOffset.x += 2;
   
   //first draw the icon
   int iconWidth = mOperBMP->getWidth() + 3;
   GFXBitmap *bmp;

   if (entryPtrs[cell.y]->status & IRCClient::PERSON_IGNORE)
   {
      bmp = mIgnoreBMP;
   }
   else
   {
      if (entryPtrs[cell.y]->status & IRCClient::PERSON_AWAY)
      {
         bmp = mAwayBMP;
      }
      else if (entryPtrs[cell.y]->status & IRCClient::PERSON_OPERATOR)
      {
         bmp = mOperBMP;
      }
      else if (entryPtrs[cell.y]->status & IRCClient::PERSON_SPEAKER ||
              (channel && !(channel->flags & IRCClient::CHANNEL_MODERATED)))
      {
         bmp = mSpkrBMP;
      }
      else
      {
         bmp = mSpecBMP;
      }
   }

   if (bmp && (! ghosted))
   {
      sfc->drawBitmap2d(bmp, &Point2I(drawOffset.x, drawOffset.y + 2 + (cellSize.y - bmp->getHeight()) / 2));
   }
   drawOffset.x += iconWidth;
      
   char *entryText = entryPtrs[cell.y]->nick; 
   if (entryText)
   {
      drawInfoText(sfc, font, entryText, Point2I(drawOffset.x, drawOffset.y - 2),
                                         Point2I(columnInfo[0]->width - 4 - iconWidth, cellSize.y), TRUE, FALSE);
   }
   drawOffset.x += columnInfo[0]->width;
}

void FGIRCNicknames::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   Grandparent::onRender(sfc, offset, updateRect);
}

static char buffer[256];
char* FGIRCNicknames::getCellText(GFXSurface *, const Point2I &cell, const Point2I &, const Point2I &)
{
   switch (cell.x)
   {
      case NIName:
         //NAME
         return entryPtrs[cell.y]->nick;  
   }
   return NULL;
}

const char *FGIRCNicknames::getSelectedText()
{
   if (selectedCell.y != -1)
   {
      return (entryPtrs[selectedCell.y]->nick);
   }
   return (NULL);
}

void FGIRCNicknames::onRightMouseDown(const Event &event)
{
   onMouseDown(event);
}

void FGIRCNicknames::onMouseDown(const Event &event)
{
   Parent::onMouseDown(event);
   if (event.mouseDownCount > 1)
   {
      if (selectedCell.y == prevSelected.y)
      {
         onMessage(this, IDIRC_MENUOPT_PRIVATE_CHAT);
      }
   }
   prevSelected = selectedCell;
}

void FGIRCNicknames::onRightMouseUp(const Event &event)
{
   bool                   fIsMe;
   IRCClient             *objIRCClient;
   IRCClient::Person     *person;
   IRCClient::PersonInfo *info;
   IRCClient::Channel *channel = mIRCClient->findChannel(NULL);

   //find out in which cell the mouse was released
   Point2I pt = globalToLocalCoord(event.ptMouse);
   if (pt.x < 0 || pt.x >= extent.x) return;
   int cell = pt.y / cellSize.y;
   if (cell < 0 || cell != prevSelected.y) return;
   
   // Get our information
   person  = mIRCClient->getMe();
   
   if (person && channel)
   {
      info = channel->findPerson(person->nick);

      if (info && selectedCell.y != -1)
      {
         FGPopUpMenu *menu = new FGPopUpMenu;

         manager->addObject(menu);
         menu->onWake();
         menu->setPos(event.ptMouse);

         // KICK menu entry is DISABLED if the current user (me) is not
         // an OPERATOR, or if the selected entry is me
         menu->appendMenu(MENU_STRING |
            ((!(info->flags & IRCClient::PERSON_OPERATOR) || 
              !(stricmp(entryPtrs[selectedCell.y]->nick, person->nick))) ? MENU_DISABLED : 0),    
            IDIRC_MENUOPT_KICK,
            SimTagDictionary::getString(manager, IDSTR_KICK_USER));

         // BAN menu entry is DISABLED if the current user (me) is not
         // an OPERATOR, or if the selected entry is me
         menu->appendMenu(MENU_STRING |
            ((!(info->flags & IRCClient::PERSON_OPERATOR) || 
              !(stricmp(entryPtrs[selectedCell.y]->nick, person->nick))) ? MENU_DISABLED : 0),    
            IDIRC_MENUOPT_BAN, 
            SimTagDictionary::getString(manager, IDSTR_BAN_UNBAN_USER));

         // Space the entries
         menu->appendMenu(MENU_SEPARATOR);

         fIsMe = (stricmp(entryPtrs[selectedCell.y]->nick, person->nick) == 0);

         menu->appendMenu(MENU_STRING |
            (fIsMe ? MENU_DISABLED : 0), 
            IDIRC_MENUOPT_PRIVATE_CHAT, 
            SimTagDictionary::getString(manager, IDSTR_PRIVATE_CHAT));
         menu->appendMenu(MENU_STRING |
            (fIsMe ? MENU_DISABLED : 0), 
            IDIRC_MENUOPT_PING_USER, 
            SimTagDictionary::getString(manager, IDSTR_PING_USER));
         menu->appendMenu(MENU_STRING | 
            (fIsMe ? MENU_DISABLED : 0), 
            IDIRC_MENUOPT_WHOIS_USER, 
            SimTagDictionary::getString(manager, IDSTR_WHO_IS_THIS));
         menu->appendMenu(MENU_SEPARATOR);

         menu->appendMenu(MENU_STRING |
            (entryPtrs[selectedCell.y]->status & IRCClient::PERSON_AWAY ? MENU_CHECKED : 0) |
            (fIsMe ? 0 : MENU_DISABLED),
            IDIRC_MENUOPT_AWAY, 
            SimTagDictionary::getString(manager, IDSTR_AWAY_FROM_KEYBOARD));

         // IGNORE menu entry is CHECKED if we are currently ignoring the
         // selected entry
         menu->appendMenu(MENU_STRING | 
            (entryPtrs[selectedCell.y]->status & IRCClient::PERSON_IGNORE ? MENU_CHECKED : 0) |
            (fIsMe ? MENU_DISABLED : 0),
            IDIRC_MENUOPT_IGNORE,
            SimTagDictionary::getString(manager, IDSTR_IGNORE_USER));

         // Space the entries
         menu->appendMenu(MENU_SEPARATOR);

         // HOST menu entry is CHECKED if the selected entry is an OPERATOR, 
         // and is DISABLED if the current user (me) is not an OPERATOR
         menu->appendMenu(MENU_STRING | 
            ((entryPtrs[selectedCell.y]->status & IRCClient::PERSON_OPERATOR) ? MENU_CHECKED : 0) |
            ((info->flags        & IRCClient::PERSON_OPERATOR) ? 0 : MENU_DISABLED), 
            IDIRC_MENUOPT_OPER, 
            SimTagDictionary::getString(manager, IDSTR_OPERATOR));

         // SPEAKER menu entry is CHECKED if the selected entry is NOT an 
         // OPERATOR and is either explicitly set as a SPEAKER or this
         // channel is NOT MODERATED.  It is DISABLED if the current user
         // (me) is not an OPERATOR
         menu->appendMenu(MENU_STRING |
            ((!(entryPtrs[selectedCell.y]->status & IRCClient::PERSON_OPERATOR) && 
              ((entryPtrs[selectedCell.y]->status & IRCClient::PERSON_SPEAKER) || 
             (!(channel->flags     & IRCClient::CHANNEL_MODERATED)))) ? MENU_CHECKED : 0) |
            ((info->flags & IRCClient::PERSON_OPERATOR) ? 0 : MENU_DISABLED), 
            IDIRC_MENUOPT_SPKR, 
            SimTagDictionary::getString(manager, IDSTR_SPEAKER));

         // SPECTATOR menu entry is CHECKED if the selected entry is NOT an
         // OPERATOR and is NOT a SPEAKER and the channel is MODERATED.  It
         // is DISABLED if the current user (me) is not an OPERATOR
         menu->appendMenu(MENU_STRING |
            ((info->flags        & IRCClient::PERSON_OPERATOR) &&
             (channel->flags     & IRCClient::CHANNEL_MODERATED) ? 0 : MENU_DISABLED) |
          ((!(entryPtrs[selectedCell.y]->status & IRCClient::PERSON_OPERATOR) &&
           (!(entryPtrs[selectedCell.y]->status & IRCClient::PERSON_SPEAKER)) && 
             (channel->flags     & IRCClient::CHANNEL_MODERATED)) ? MENU_CHECKED : 0),
             IDIRC_MENUOPT_SPEC, 
             SimTagDictionary::getString(manager, IDSTR_SPECTATOR));

         // Done, display the menu
         root->pushDialogControl(menu);
      }
   }
}
   
};
