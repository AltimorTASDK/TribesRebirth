#include "simResource.h"
#include "g_surfac.h"
#include "g_bitmap.h"
#include "soundFX.h"
#include "fear.strings.h"
#include "FearGuiArrayCtrl.h"
#include "m_qsort.h"
#include "console.h"
#include "g_font.h"
#include "help.strings.h"
#include "FearGuiShellPal.h"
#include "ircClient.h"
#include "FearGuiIRCChanCtrl.h"
#include "fearguipopupmenu.h"

namespace FearGui
{

IMPLEMENT_PERSISTENT_TAG(FGIRCChannelCtrl, FOURCC('F','G','i','h'));

static Sfx::Profile base2dSfxProfile;

bool FGIRCChannelCtrl::onAdd()
{
   if(!Parent::onAdd())
      return false;

   //load the fonts
   hFont = SimResource::loadByTag(manager, IDFNT_9_STANDARD, true);
   hFontHL = SimResource::loadByTag(manager, IDFNT_9_HILITE, true);
   hFontNA = SimResource::loadByTag(manager, IDFNT_9_DISABLED, true);
   hFontShadow = SimResource::loadByTag(manager, IDFNT_9_BLACK, true);
   
   //status bitmaps
   mIncomingBMP = SimResource::get(manager)->load("I_PingRed.bmp");
   AssertFatal(mIncomingBMP, "Unable to load I_PingRed.bmp");
   mIncomingBMP->attribute |= BMA_TRANSPARENT;
   
   //status bitmaps
   mPrivateBMP = SimResource::get(manager)->load("I_Locked.bmp");
   AssertFatal(mPrivateBMP, "Unable to load I_Locked.bmp");
   mPrivateBMP->attribute |= BMA_TRANSPARENT;
   
   //local vars
	mOffset = 0;
	cellWidth = 92;
   
   //find the IRC client
   mIRCClient = ::IRCClient::find(manager);
   AssertFatal(mIRCClient, "IRCChatDelegate: unable to locate IRC client");
   
   entries.clear();
   
   return TRUE;
}

void FGIRCChannelCtrl::onWake(void)
{
   if (parent)
   {
      extent.x = parent->extent.x - 6;
   }
}

bool FGIRCChannelCtrl::canScrollLeft()
{
   if (mOffset > 0) return TRUE;
   else return FALSE;
}

void FGIRCChannelCtrl::scrollLeft()
{
   if (mOffset > 0) mOffset--;
}

bool FGIRCChannelCtrl::canScrollRight()
{
   //how many channels can fit on a page
   int numChannels = extent.x / cellWidth;
   
   //see if we have that many channels
   if (entries.size() - mOffset > numChannels) return TRUE;
   
   return FALSE;
}

void FGIRCChannelCtrl::scrollRight()
{
   //how many channels can fit on a page
   int numChannels = extent.x / cellWidth;
   
   //see if we have that many channels
   if (entries.size() - mOffset > numChannels)
   {
      mOffset++;
   }
}

bool FGIRCChannelCtrl::msgWaitingLeft()
{
   AssertFatal(mOffset < entries.size(), "mOffset is invalid");
   for (int i = 0; i < mOffset; i++)
   {
      if (entries[i].msgWaiting) return TRUE;
   }
   return FALSE;
}

bool FGIRCChannelCtrl::msgWaitingRight()
{
   //how many channels can fit on a page
   int numChannels = extent.x / cellWidth;
   
   for (int i = numChannels + mOffset; i < entries.size(); i++)
   {
      if (entries[i].msgWaiting) return TRUE;
   }
   return FALSE;
}

Int32 FGIRCChannelCtrl::getHelpTag(float /*elapsedTime*/)
{
   if (! root) return 0;
   
   Point2I cursorPos = root->getCursorPos();
   Point2I pt = globalToLocalCoord(cursorPos);
   
   int cell = pt.x / cellWidth;
   if (cell >= 0 && cell + mOffset < entries.size())
   {
      if (cell + mOffset == 0)
      {
         return IDHELP_IRC_CHANNEL_STATUS;
      }
      else
      {
         return IDHELP_IRC_CHANNELS;
      }
   }
   return 0;
}
   
Int32 FGIRCChannelCtrl::getMouseCursorTag(void)
{
   if (! root) return 0;
   
   Point2I cursorPos = root->getCursorPos();
   Point2I pt = globalToLocalCoord(cursorPos);
   
   int cell = pt.x / cellWidth;
   if (cell >= 0 && cell < entries.size())
   {
      return IDBMP_CURSOR_HAND;
   }
   else
   {
      return 0;
   }
}

void FGIRCChannelCtrl::onMouseDown(const SimGui::Event &event)
{
   //find the soundfx manager
   Sfx::Manager *sfx = Sfx::Manager::find(manager);
   
   //find the IRC client
   if (! active)
   {
      Parent::onMouseDown(event);
      return;
   }
   Point2I pt = globalToLocalCoord(event.ptMouse);
   
   int cell = pt.x / cellWidth;
   if (cell >= 0 && cell + mOffset < entries.size())
   {
      //play the sound
      if (sfx) sfx->play(stringTable.insert("Button5.wav"), 0, &base2dSfxProfile);
   
      mIRCClient->setCurrentChannel(entries[cell + mOffset].name);
   }
   
   Parent::onMouseDown(event);
}

void FGIRCChannelCtrl::onPreRender()
{
   // Get the current channel
   ::IRCClient::Channel *channel = mIRCClient->findChannel(NULL);
   
   // Get the list of channels
   VectorPtr<::IRCClient::Channel *> *channels = mIRCClient->getChannels();

   //rebuild the list of channels
   entries.clear();
   for (int i = 0; i < channels->size(); i ++)
   {
      ChannelRep newChannel;
      
      //copy the name
      strcpy(newChannel.name, ((*channels)[i])->name);
      
      //copy the id
      newChannel.id = ((*channels)[i])->id;
      
      //set the messages pending flag
      newChannel.msgWaiting = (((*channels)[i])->newMessages() && ((*channels)[i]) != channel);
      
      //set the private flag
      newChannel.bPrivate = ((*channels)[i])->isPrivate();
      
      entries.push_back(newChannel);
   }
   
   //set the cellWidth
   int tempNumCells = max(6, min(10, channels->size()));
   cellWidth = extent.x / tempNumCells;
   
   setUpdate();
}

void FGIRCChannelCtrl::drawInfoText(GFXSurface *sfc, GFXFont *font, const char* text, Point2I &offset, Point2I &dest, bool shadow)
{
   //if the string fits
   if (font->getStrWidth(text) < dest.x)
   {
      if (shadow && bool(hFontShadow))
      {
         sfc->drawText_p(hFontShadow, &Point2I(offset.x - 1 + (dest.x - font->getStrWidth(text)) / 2,
                      offset.y + 1 + (dest.y - font->getHeight()) / 2), text);
      }
      sfc->drawText_p(font, &Point2I(offset.x + (dest.x - font->getStrWidth(text)) / 2,
                      offset.y + (dest.y - font->getHeight()) / 2), text);
   }
   
   //else create a string that will 
   else
   {
      char buf[260], *temp;
      char etcBuf[4] = "...";
      int etcWidth = font->getStrWidth(etcBuf);
      
      //make sure we can at least hold the etc
      if (dest.x < etcWidth) return;
      
      //copy the string into a temp buffer
      strncpy(buf, text, 255);
      buf[255] = '\0';
      int stringlen = strlen(buf);
      temp = &buf[stringlen];
      
      //search for how many chars can be displayed
      while (stringlen && (font->getStrWidth(buf) > dest.x - etcWidth))
      {
         stringlen--;
         temp--;
         *temp = '\0';
      }
      
      //now copy the etc onto the end of the string, and draw the text
      strcpy(temp, etcBuf);
      if (shadow && bool(hFontShadow))
      {
         sfc->drawText_p(hFontShadow, &Point2I(offset.x - 1, offset.y + 1 + (dest.y - font->getHeight()) / 2), buf);
      }
      sfc->drawText_p(font, &Point2I(offset.x, offset.y + (dest.y - font->getHeight()) / 2), buf);
   }
}

void FGIRCChannelCtrl::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(1);
   if ((! active) ||
       (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG))))
   {
      ghosted = TRUE;
   }
   
   // Get the current channel
   int curChannelID;
   ::IRCClient::Channel *channel = mIRCClient->findChannel(NULL);
   if (channel) curChannelID = channel->id;
   else curChannelID = -1;
   
   if (mbOpaque)
   {
      sfc->drawRect2d_f(&RectI(offset.x, offset.y, offset.x + extent.x - 1, offset.y + extent.y - 1),
                        (! ghosted ? fillColor : ghostFillColor));
   }
   
   //draw the cell boundaries
   int cell = mOffset;
   Point2I cellOffset = offset;
   while (cellOffset.x < offset.x + extent.x - 1)
   {
      //if this is the selected cell, fill it with the hilite color
      if (cell < entries.size() && int(entries[cell].id) == curChannelID)
      {
         sfc->drawRect2d_f(&RectI(cellOffset.x + (cell > 0 ? 1 : 2), cellOffset.y + 2, cellOffset.x + cellWidth - 3, cellOffset.y + extent.y - 3),
                              (! ghosted ? HILITE_COLOR : BOX_GHOST_INSIDE));
      }
      
      //draw the end line
      sfc->drawLine2d(&Point2I(cellOffset.x + cellWidth - 1, cellOffset.y),
                      &Point2I(cellOffset.x + cellWidth - 1, cellOffset.y + extent.y - 1),
                              (! ghosted ? SELECT_COLOR : 249)); 
      cellOffset.x += cellWidth;
      cell++;
   }
   
   //now draw in the cell labels
   cell = mOffset;
   cellOffset = offset;
   while ((cellOffset.x < offset.x + extent.x - 1) && (cell < entries.size()))
   {
      int tempWidth = cellWidth - 4;
      Point2I bmpOffset = cellOffset;
      bmpOffset.x += 4;
      if (entries[cell].msgWaiting)
      {
         bmpOffset.y = cellOffset.y + (extent.y - mIncomingBMP->getHeight()) / 2 + 1;
         if (! ghosted) sfc->drawBitmap2d(mIncomingBMP, &bmpOffset);
         bmpOffset.x += mIncomingBMP->getWidth() + 3;
         tempWidth -= mIncomingBMP->getWidth() + 3;
      }
      
      //set the private channel flag
      if (entries[cell].bPrivate)
      {
         bmpOffset.y = cellOffset.y + (extent.y - mPrivateBMP->getHeight()) / 2 + 1;
         if (! ghosted) sfc->drawBitmap2d(mPrivateBMP, &bmpOffset);
         bmpOffset.x += mPrivateBMP->getWidth() + 3;
         tempWidth -= mPrivateBMP->getWidth() + 3;
      }
      
      //if this is the selected cell, fill it with the hilite color
      drawInfoText(sfc, (ghosted ? hFontNA : (int(entries[cell].id) == curChannelID ? hFontHL : hFont)), entries[cell].name,
                  Point2I(bmpOffset.x, cellOffset.y), Point2I(tempWidth - 4, extent.y - 6), TRUE);
      
      cellOffset.x += cellWidth;
      cell++;
   }
   
   //draw the frame
   if (mbBoarder)
   {
      sfc->drawRect2d(&RectI(offset.x, offset.y, offset.x + extent.x - 1, offset.y + extent.y - 1),
                              (! ghosted ? boarderColor : ghostBoarderColor));
   }
}

void FGIRCChannelCtrl::onRightMouseDown(const Event &event)
{
   onMouseDown(event);
}
 
void FGIRCChannelCtrl::onRightMouseUp(const Event &event)
{
   bool                   fShowOpts;

   // Get the current channel
   ::IRCClient::Channel *channel = mIRCClient->findChannel(NULL);
   if (! channel) return;
   
   //find out in which cell the mouse was released
   Point2I pt = globalToLocalCoord(event.ptMouse);
   if (pt.y < 0 || pt.y >= extent.y) return;
   int cell = pt.x / cellWidth;
   if (cell >= 0 && cell + mOffset < entries.size())
   {
      //make sure the cell is the same as the currently selected channel
      if (entries[cell + mOffset].id == channel->id)
      {
         fShowOpts = (channel && channel->members.size() > 0) ||
                     (stricmp(entries[cell + mOffset].name, "status"));
         
         FGPopUpMenu *menu = new FGPopUpMenu;

         manager->addObject(menu);
         menu->onWake();
         menu->setPos(event.ptMouse);

         menu->appendMenu(MENU_STRING |
            (fShowOpts ? 0 : MENU_DISABLED), 
             IDIRC_MENUOPT_LEAVE, 
             SimTagDictionary::getString(manager, IDSTR_LEAVE_ROOM));

         menu->appendMenu(MENU_SEPARATOR);

         fShowOpts = (channel && channel->members.size() > 0);

         menu->appendMenu(MENU_STRING |
            (fShowOpts ? 0 : MENU_DISABLED),
             IDIRC_MENUOPT_INVITE,
             SimTagDictionary::getString(manager, IDSTR_INVITE_USER));

         menu->appendMenu(MENU_STRING |
            (fShowOpts ? 0 : MENU_DISABLED), 
             IDIRC_MENUOPT_CHANNEL_PROPERTIES,
             SimTagDictionary::getString(manager, IDSTR_PROPERTIES));

         root->pushDialogControl(menu);
      }
   }
}
};
