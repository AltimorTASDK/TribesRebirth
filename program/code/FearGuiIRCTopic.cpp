#include <g_surfac.h>
#include <string.h>
#include <m_box.h>
#include "simResource.h"
#include "ircClient.h"
#include "fearGuiIRCTopic.h"
#include "simTagDictionary.h"
#include "fear.strings.h"

namespace FearGui
{

IMPLEMENT_PERSISTENT_TAG( FGIRCTopicCtrl,   FOURCC('F','G','t','c') );

bool FGIRCTopicCtrl::onAdd(void)
{
   if (! Parent::onAdd()) return FALSE;
   
   //set the fonts
   setFont(IDFNT_9_STANDARD);
   setFontHL(IDFNT_9_HILITE);
   setFontDisabled(IDFNT_9_DISABLED);
   hFontShadow = SimResource::loadByTag(manager, IDFNT_9_BLACK, true);
   extent.y = hFont->getHeight() + 4; 
   
   //find the IRC client
   mIRCClient = ::IRCClient::find(manager);
   AssertFatal(mIRCClient, "IRCChatDelegate: unable to locate IRC client");
   
   return TRUE;
}

void FGIRCTopicCtrl::onPreRender(void)
{
   // Get the current channel
   ::IRCClient::Channel *channel = mIRCClient->findChannel(NULL);
   
   //set the current channel topic
   if (channel && channel->topic && channel->topic[0])
   {
      setText(channel->topic);
   }
   else setText("");
   
   //make sure it's extent matches it's parent
   if (parent) extent.x = parent->extent.x - 6;
   position.x = 3;
}

void FGIRCTopicCtrl::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(1);
   if (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG)))
   {
      ghosted = TRUE;
   }
   
   char buf[256];
   strncpy(buf, text, 252);
   buf[252] = '\0';
   
   //make sure the whole string will fit
   char *bufPtr = &buf[strlen(buf)];
   if (hFont->getStrWidth(buf) > extent.x)
   {
      while (bufPtr >= &buf[0] && hFont->getStrWidth(buf) > extent.x - hFont->getStrWidth("..."))
      {
         *bufPtr-- = '\0';
      }
      sprintf(bufPtr, "...");
   }
   
   if (bool(hFontShadow)) sfc->drawText_p(hFontShadow, &Point2I(offset.x, offset.y + 1), buf);
   sfc->drawText_p((ghosted ? hFontDisabled : (active ? hFontHL : hFont)), &Point2I(offset.x + 1, offset.y), buf);
}

}; //namespace