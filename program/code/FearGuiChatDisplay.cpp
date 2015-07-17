//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <string.h>
#include "console.h"
#include <g_surfac.h>
#include <g_bitmap.h>
#include "ircClient.h"
#include <soundFX.h>
#include <inspect.h>
#include "SimGuiCtrl.h"
#include <FearDcl.h>
#include "fearGlobals.h"
#include "esf.strings.h"
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

IMPLEMENT_PERSISTENT_TAG( FearGuiChatDisplay, FOURCC('F','G','c','d') );

static int gDisplayBoarderY = 6;
static int gDisplayBoarderX = 12;
static int gDiagMin = 3;
static int gDiagMax = 13;

static int messageCount = 0;
static int lastMessageIndex = -1;

static long frame = 0;
static FearGuiChatDisplay::Message messages[MessageHistoryLimit + 1]; // the last message will store the last command message

void FearGuiChatDisplay::clear(void)
{
   messageCount = 0;
   lastMessageIndex = -1;
   lineOffset = 0;
   pgUpCount = 0;
   lastRenderTime = 0;
}
   
GFXFont *FearGuiChatDisplay::getFont(int msgType)
{
   GFXFont *font;
   if (mLowRes)
   {
      if (msgType == MSG_System) font = lr_systemFont;
      else if (msgType == MSG_Command) font = lr_commandFont;
      else if (msgType == MSG_Game) font = lr_gameFont;
      else if (msgType == MSG_TeamChat) font = lr_teamMsgFont;
      else font = lr_msgFont;
   }
   else
   {
      if (msgType == MSG_System) font = systemFont;
      else if (msgType == MSG_Command) font = commandFont;
      else if (msgType == MSG_Game) font = gameFont;
      else if (msgType == MSG_TeamChat) font = teamMsgFont;
      else font = msgFont;
   }
   
   return font;
}
void FearGuiChatDisplay::newMessage(const char *msg, int msgType)
{
   GFXFont *font = getFont(msgType);
   
   //first decide where to put it
   Message *curMsg;
   if (msgType == MSG_Command)
   {
      curMsg = &messages[MessageHistoryLimit];
   }
   else
   {
      lastMessageIndex++;
      if (lastMessageIndex >= MessageHistoryLimit - 1) lastMessageIndex = 0;
      curMsg = &messages[lastMessageIndex];
   }
   
   //get the mesage pointer, and copy the message w/o any existing args
   int lengthAvail = 319;
   int lengthToCopy;
   const char *msgPtr = msg;
   char *destPtr = curMsg->msg;
   char argBuffer[320];
   const char *argPtr;
   bool dontShowMsg = FALSE;
   
   //parse out any args
   while (*msgPtr != '\0' && lengthAvail)
   {
      //search for any args
      argPtr = strchr(msgPtr, '~');
      if (argPtr)
      {
         //if we're not at the beginning of the message, copy the message body
         if (argPtr > msgPtr)
         {
            lengthToCopy = min(argPtr - msgPtr, lengthAvail);
            int length = min(lengthToCopy, int(strlen(msgPtr)));
            if (! dontShowMsg)
            {
               strncpy(destPtr, msgPtr, lengthToCopy);
               destPtr[length] = '\0';
               destPtr += length;
               lengthAvail -= length;
            }
            msgPtr += length;
         }
            
         //now process the arg and advance the msgPtr beyond
         const char *argEnd;
         if (argPtr[2] == '[')
         {
            argEnd = strchr(&argPtr[3], ']');
         }
         else
         {
            argEnd = strchr(argPtr, ' ');
         }
         if (argEnd)
         {
            lengthToCopy = min(argEnd - argPtr + 1, lengthAvail);
            strncpy(argBuffer, argPtr, lengthToCopy);
            int length = min(lengthToCopy, int(strlen(argPtr)));
            argBuffer[length] = '\0';
            msgPtr += length;
         }
         //otherwise the arg is the last portion of the message
         else
         {
            strcpy(argBuffer, argPtr);
            msgPtr = &argPtr[strlen(argPtr)];
         }
         
         //process the arg
         if (msgType != MSG_Command)
            dontShowMsg = processArg(argBuffer) || dontShowMsg;
      }
      else
      {
         int length = min(lengthAvail, int(strlen(msgPtr)));
         if (! dontShowMsg)
         {
            strncpy(destPtr, msgPtr, lengthAvail);
            destPtr[length] = '\0';
            destPtr += length;
            lengthAvail -= length;
         }
         msgPtr += length;
      }
   }

   //if the message arg is tagged to not show the message, decriment the msg index
   if ((msgType != MSG_Command) && (dontShowMsg || (lengthAvail == 319)))
   {
      lastMessageIndex--;
      if ((lastMessageIndex < 0) && (messageCount > 0)) {
         lastMessageIndex = MessageHistoryLimit - 1;
      }
      return;
   }
   
   //update the number of messages
   if (messageCount < MessageHistoryLimit - 1) messageCount++;
   
   //adjust the pageUp count if required
   if ((pgUpCount > 0) && (pgUpCount < messageCount - lineCount)) pgUpCount++;
   
   //set the type
   curMsg->type = msgType;
   
   //set the line offset
   //curMsg->time = manager->getCurrentTime();
   curMsg->lineOffset = 0;
   
   //now format the string
   formatMessage((msgType != MSG_Command ? lastMessageIndex : MessageHistoryLimit), extent.x - (2 * gDisplayBoarderX));
   
   //update the lineOffset
   if (! pgUpCount) {
      lineOffset += curMsg->lineCount * font->getHeight(); 
   }
}

bool FearGuiChatDisplay::processArg(const char *arg)
{
   bool retValue = FALSE;
   switch (arg[1])
   {
      case 'z':
      case 'Z':
      /*
         //essentially, this will play an anonymous message...
         //back up to "replace" the message that sent this argument
         lastMessageIndex--;
         if ((lastMessageIndex < 0) && (messageCount > 0)) {
            lastMessageIndex = MessageHistoryLimit - 1;
         }
         
         //display a different message
         char newMsg[320];
         newMsg[319] = '\0';
         strncpy(newMsg, (char*)&arg[3], 319);
         char *temp = strrchr(newMsg, ']');
         if (temp) *temp = '\0';
         newMessage(newMsg, MSG_Chat);
         
         //increment the index since returning TRUE will cause the lastMessageIndex
         //to be decrimented again.
         //this is so args such as ~w don't generate a message.
         lastMessageIndex++;
         if (lastMessageIndex >= MessageHistoryLimit) lastMessageIndex = 0;
         retValue = TRUE;
      */
         break;
   }
   return retValue;
}

void FearGuiChatDisplay::formatMessage(int index, int width)
{
   if( width <= 0 )
      return;
      
   //get the mesage pointer, and copy the message
   Message *curMsg = &messages[index];
   
   GFXFont *font = getFont(curMsg->type);
   
   char *bufPtr = &curMsg->msg[0];
   int lengthLeft = strlen(bufPtr);
   char *tempPtr, *noSpacePtr;
   char temp;
   
   //initialize the number of lines in the message
   curMsg->lineCount = 0;
   
   //make sure we eliminate trailing spaces
   char *spacePtr = &bufPtr[lengthLeft - 1];
   while ((*spacePtr == ' ') && (spacePtr > bufPtr))
   {
      spacePtr--;
      lengthLeft--;
   }
   if (lengthLeft > 0) *(spacePtr + 1) = '\0';
   
   //write the text, wrapping around if necessary
   while (lengthLeft > 0)
   {
      //set the tempPtr
      tempPtr = &bufPtr[lengthLeft];
      temp = '\0';
      noSpacePtr = NULL;
      
      //if the string is too long to fit, find a space on which to break
      while (1)
      {
         //if the string fits
         if (font->getStrWidth(bufPtr) <= width)
         {
            //if we found a space, break and print the string
            if (temp == ' ' || temp == '\0')
            {
               break;
            }
            else
            {
               //noSpacePtr points to the rightmost char in case there are spaces at all
               if (! noSpacePtr) noSpacePtr = tempPtr;
               
               //see if we've at the beginning of the string, print up to noSpacePtr
               if (tempPtr <= bufPtr + 1)
               {
                  *tempPtr = temp;
                  tempPtr = noSpacePtr;
                  temp = *tempPtr;
                  *tempPtr = '\0';
                  break;
               }
            }
         }
         
         //at this point, we haven't found a space break, so back up a char
         *tempPtr = temp;
         tempPtr--;
         temp = *tempPtr;
         *tempPtr = '\0';
      }
      
      //we have now found a string which fits - set the vars
      curMsg->charIndex[curMsg->lineCount] = tempPtr - &curMsg->msg[0];
      
      //update the lengthLeft
      lengthLeft -= strlen(bufPtr);
      
      //replace the missing character and set bufPtr to be the next string portion
      *tempPtr = temp;
      bufPtr = tempPtr;
      
      //on to the next line
      curMsg->lineCount++;
      AssertFatal(curMsg->lineCount <= LinesPerMessage, "Message is too long!");
   }
}

void FearGuiChatDisplay::parentResized(const Point2I &, const Point2I &newParentExtent)
{
   //check for lowres
   bool changed;
   if (newParentExtent.x < 512 || newParentExtent.y < 384)
   {
      changed = !mLowRes;
      mLowRes = TRUE;
   }
   else
   {
      changed = mLowRes;
      mLowRes = FALSE;
   }
   
   if (mLowRes)
   {
      gDisplayBoarderX = 12;
      gDiagMin = 2;
      gDiagMax = 6;
      if (changed)
      { 
         extent.set(240, lineCount * lr_msgFont->getHeight() + gDisplayBoarderY);
      }
      extent.x = max(min(max(extent.x, Int32(240)), newParentExtent.x - 1), Int32(1));
   }
   else
   {
      gDisplayBoarderX = 8;
      gDiagMin = 3;
      gDiagMax = 13;
      if (changed)
      {
         extent.set(370, lineCount * msgFont->getHeight() + gDisplayBoarderY);
      }
      extent.x = max(min(max(extent.x, Int32(370)), newParentExtent.x - 1), Int32(1));
   }
   
   extent.y = max(min(extent.y, newParentExtent.y - 1), Int32(1));
   lineCount = (extent.y - gDisplayBoarderY) / (mLowRes ? lr_msgFont->getHeight() : msgFont->getHeight());
   extent.y = lineCount *  (mLowRes ? lr_msgFont->getHeight() : msgFont->getHeight()) + gDisplayBoarderY;
   
   //reformat all the existing messages
   for (int i = 0; i < messageCount; i++)
   {
      formatMessage(i, extent.x - (2 * gDisplayBoarderX));
   }
   
   position.x = min(position.x, newParentExtent.x - extent.x);
   position.y = min(position.y, newParentExtent.y - extent.y);
}

void FearGuiChatDisplay::resize(const Point2I &, const Point2I &newExtent)
{
   extent.x = max((int)min(max((int)newExtent.x, (mLowRes ? 240 : 370)), (int)parent->extent.x - 1), 1);
   extent.y = max((int)min(newExtent.y, parent->extent.y - 1), 1);
   lineCount = (extent.y - gDisplayBoarderY) / (mLowRes ? lr_msgFont->getHeight() : msgFont->getHeight());
   extent.y = lineCount *  (mLowRes ? lr_msgFont->getHeight() : msgFont->getHeight()) + gDisplayBoarderY;
   
   //reformat all the existing messages
   for (int i = 0; i < messageCount; i++)
   {
      formatMessage(i, extent.x - (2 * gDisplayBoarderX));
   }
   
   snap();
}

bool FearGuiChatDisplay::onAdd(void)
{
   if (! Parent::onAdd())
      return FALSE;
      
   //load the fonts
   systemFont = SimResource::get(manager)->load("if_w_10b.pft");
   commandFont = SimResource::get(manager)->load("sf_red_10b.pft");
   gameFont = commandFont;
   msgFont = SimResource::get(manager)->load("sf_yellow_10b.pft");
   teamMsgFont = SimResource::get(manager)->load("if_g_10b.pft");
   
   lr_systemFont = SimResource::get(manager)->load("sf_white_6.pft");
   lr_commandFont = SimResource::get(manager)->load("sf_red_6.pft");
   lr_gameFont = lr_commandFont;
   lr_msgFont = SimResource::get(manager)->load("sf_yellow_6.pft");
   lr_teamMsgFont = SimResource::get(manager)->load("sf_green_6.pft");
   
   lr_systemFont->fi.flags  |= FONT_LOWERCAPS;
   lr_commandFont->fi.flags |= FONT_LOWERCAPS;
   lr_gameFont->fi.flags    |= FONT_LOWERCAPS;
   lr_msgFont->fi.flags     |= FONT_LOWERCAPS;
   lr_teamMsgFont->fi.flags |= FONT_LOWERCAPS;
   
   //load the bitmaps
   mPageDown      = SimResource::get(manager)->load("HC_Down.bmp");
   AssertFatal(mPageDown   , "Unable to load mPageDown   ");
   mPageDown->attribute |= BMA_TRANSPARENT;
   
   lr_mPageDown      = SimResource::get(manager)->load("LR_HC_Down.bmp");
   AssertFatal(lr_mPageDown   , "Unable to load mPageDown   ");
   lr_mPageDown->attribute |= BMA_TRANSPARENT;
   
   //initialize the display vars
   mLowRes = FALSE;
   prevWidth = 0;
   lineOffset = 0;
   pgUpCount = 0;
   
   //get the IRC client
   mIRCClient = ::IRCClient::find(manager);
   AssertFatal(mIRCClient, "unable to locate IRC client");
   
   return TRUE;
}

void drawBracket(GFXSurface *sfc, int x, int ty, int by, int dx)
{
   int dir = 1;
   if(dx < 0)
      dir = -1;
   sfc->drawLine2d(&Point2I(x, ty), &Point2I(x, by), GREEN_132);
   sfc->drawLine2d(&Point2I(x+dir, ty), &Point2I(x+dir, by), GREEN);
   sfc->drawLine2d(&Point2I(x+dir, ty), &Point2I(x+dir+dx, ty), GREEN);
   sfc->drawLine2d(&Point2I(x+dir, by), &Point2I(x+dir+dx, by), GREEN);
}

void FearGuiChatDisplay::onPreRender(void)
{
   //make sure only the game chat display, not the commander receives IRC chat
   if (mbCommanderChat) return;
   
   //make sure the IRC client is connected
   if (mIRCClient->getState() == IDIRC_DISCONNECTED) return;
   
   // Get the current channel
   ::IRCClient::Channel *curChannel = mIRCClient->findChannel(NULL);
   
   // Get the list of channels
   VectorPtr<::IRCClient::Channel *> *channels = mIRCClient->getChannels();
   
   //loop through the channels, looking for private channels with new messages
   for (int i = 0; i < channels->size(); i ++)
   {
      ::IRCClient::Channel *channelPtr = (*channels)[i];
      
      //make sure the channel has messages waiting
      if (! channelPtr->newMessages()) continue;
      
      //make sure the channel is private or is the current channel
      if (! channelPtr->isPrivate())
      {
         if (channelPtr != curChannel) continue;
         else if (stricmp(Console->getVariable("IRC::ListenInSim"), "TRUE")) continue;
      }
      
      //find the channel, or create a new entry if necessary
      IRCChannelRep *newChannel = NULL;
      for (int j = 0; j < mPrivateIRCMsgs.size(); j++)
      {
         if (mPrivateIRCMsgs[j].dwChannelID == channelPtr->id)
         {
            newChannel = &mPrivateIRCMsgs[j];
            break;
         }
      }
      
      //if not found, create
      if (! newChannel)
      {
         IRCChannelRep newRep;
         newRep.dwChannelID = channelPtr->id;
         newRep.lpmLastRead = NULL;
         mPrivateIRCMsgs.push_back(newRep);
         newChannel = &mPrivateIRCMsgs.last();
      }

      if (newChannel->lpmLastRead == NULL)
      {
         newChannel->lpmLastRead = channelPtr->messages.getHead();
      }
      else
      {
         newChannel->lpmLastRead = newChannel->lpmLastRead->next;
      }

      if (newChannel->lpmLastRead)
      {
         for (/* NOP */; newChannel->lpmLastRead; newChannel->lpmLastRead = newChannel->lpmLastRead->next)
         {
            //strip out all the '<' '>' format control strings
            char buf[512], *bufPtr, *textPtr, *msgPtr;
            bufPtr = &buf[0];
            *bufPtr++ = '<';
            *bufPtr = '\0';
            msgPtr = newChannel->lpmLastRead->text;
            textPtr = strchr(msgPtr, '<');
            while (textPtr)
            {
               if (textPtr[1] == '<')
               {
                  textPtr = strchr(&textPtr[2], '<');
               }
               else
               {
                  char *tempPtr = strchr(&textPtr[1], '>');
                  if (tempPtr)
                  {
                     int length = int(textPtr) - int(msgPtr);
                     strncpy(bufPtr, msgPtr, length);
                     bufPtr += length;
                     *bufPtr = '\0';
                     msgPtr = &tempPtr[1];
                     textPtr = strchr(msgPtr, '<');
                  }
                  else
                  {
                     textPtr = NULL;
                  }
               }
            }
            //copy the last of the string into the buf
            strcpy(bufPtr, msgPtr);
            bufPtr += strlen(msgPtr);
            *bufPtr++ = '>';
            *bufPtr = '\0';
            
            //send the message to the chat display
            newMessage(buf, MSG_TeamChat);
         }
      }
      
      // Re-set the flag
      channelPtr->flags &= ~::IRCClient::CHANNEL_NEWMESSAGE;
      newChannel->lpmLastRead = channelPtr->messages.getTail();
   }
}

void FearGuiChatDisplay::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   frame++;
   
   if (lineCount <= 0) return;
   
   //compiler warning
   updateRect;
   
   //set the clip rectangle
   Point2I clipMax = parent->localToGlobalCoord(position);
   clipMax.x += extent.x - 1;
   clipMax.y += extent.y - 1;
   RectI displayRect;
   displayRect.upperL.x = offset.x;
   displayRect.upperL.y = offset.y;
   displayRect.lowerR.x = min(clipMax.x, offset.x + extent.x) + 1;
   displayRect.lowerR.y = min(clipMax.y, offset.y + extent.y) + 1;
   
   Point2I *topLeft = &displayRect.upperL;
   Point2I *bottomRight = &displayRect.lowerR;
   
   if ((! mbCommanderChat) || (lineCount > 3))
   {
      //initialize our vertices
      Point3F poly[4];
      for (int j = 0; j < 4; j++)
         poly[j].z = 1.0f;
      
      //DRAW THE TRANSPARENT BACKGROUND
      poly[0].x = topLeft->x;
      poly[0].y = topLeft->y;
      
      poly[1].x = bottomRight->x;
      poly[1].y = topLeft->y;
      
      poly[2].x = bottomRight->x;
      poly[2].y = bottomRight->y;
      
      poly[3].x = topLeft->x;
      poly[3].y = bottomRight->y;
      
      //draw the translucent box
      sfc->setFillMode(GFX_FILL_CONSTANT);
      sfc->setShadeSource(GFX_SHADE_NONE);
      sfc->setHazeSource(GFX_HAZE_NONE);
      sfc->setAlphaSource(GFX_ALPHA_FILL);
      sfc->setTransparency (false);
      sfc->setFillColor(254, ALPHAPAL_INDEX);

      for (int i = 0; i < 4; i++)
         sfc->addVertex(poly + i);

      sfc->emitPoly();
      sfc->setAlphaSource(GFX_ALPHA_NONE);
   }
   
   if (! mbCommanderChat)
   {
      drawBracket(sfc, topLeft->x, topLeft->y, bottomRight->y - 1, 4);
      drawBracket(sfc, bottomRight->x - 1, topLeft->y, bottomRight->y - 1, -4);
   }   
   if ((lineCount > 1) && (pgUpCount > 0))
   {
      sfc->drawBitmap2d((! mLowRes ? mPageDown : lr_mPageDown),
                           &Point2I(offset.x + extent.x - mPageDown->getWidth() - (! mLowRes ? 3 : 0),
                                    offset.y + extent.y - mPageDown->getHeight() - (! mLowRes ? 3 : 0)));
   }
   
   //set the clip rect to prevent text from drawing outside the translucency
   Message *cmdMsg = &messages[MessageHistoryLimit];
   
   displayRect.upperL.x += 4;
   displayRect.upperL.y += 2;
   displayRect.lowerR.x -= 4;
   displayRect.lowerR.y -= 2;
   
   //see if a cmd exists
   bool cmdExists = FALSE;
   bool cmdAcknowledged = FALSE;
   if (cg.playerManager && cg.player)
   {
      PlayerManager::ClientRep *me = cg.playerManager->findClient(manager->getId());
      if (me)
      {
         if (me->curCommand >= 0)
         {
            cmdExists = TRUE;
            if (me->commandStatus == PlayerManager::CMD_ACKNOWLEDGED) cmdAcknowledged = TRUE;
         }
      }
   }
   
   GFXFont *cmdFont = getFont(cmdAcknowledged ? MSG_TeamChat : MSG_Command);
   
   if (cmdExists)
   {
      GFXFont *font = getFont(cmdAcknowledged ? MSG_TeamChat : MSG_Command);
      displayRect.lowerR.y -= font->getHeight();
   }
   sfc->setClipRect(&displayRect);
   
   int linesLeft = (cmdExists ? lineCount - 1 : lineCount);
   int msgsLeft = messageCount;
   int curMsgIndex = lastMessageIndex;
   
   if (pgUpCount > 0)
   {
      if (msgsLeft < MessageHistoryLimit) msgsLeft -= pgUpCount;
      curMsgIndex = (MessageHistoryLimit + lastMessageIndex - pgUpCount ) % MessageHistoryLimit;
   }
   
   Point2I msgOffset, cmdrOffset;
   msgOffset.x = topLeft->x + (gDisplayBoarderX / 2);
   msgOffset.y = bottomRight->y - (mLowRes ? 0 : 4);
   
   cmdrOffset.y = msgOffset.y - cmdFont->getHeight();
   
   //update the lineOffset
   //draw the cursor
   SimTime curTime = manager->getCurrentTime();
   SimTime timeElapsed = curTime - lastRenderTime;
   lastRenderTime = curTime;
   if (lineOffset > 0) {
      int scrollVal = max((int)(10 * max(lineOffset, 3) * timeElapsed), 2);
      lineOffset -= scrollVal;
      if (lineOffset < 0) lineOffset = 0;
      msgOffset.y += lineOffset;
      
      //if we've offset the lines, draw one more
      if (lineOffset > 0) linesLeft++;
   }
      
   while (linesLeft && msgsLeft)
   {
      Message *curMsg = &messages[curMsgIndex];
      char *linePtr = &curMsg->msg[0];
      int startLine = 0;
      if (curMsg->lineCount > linesLeft)
      {
         startLine = curMsg->lineCount - linesLeft;
         linePtr = &curMsg->msg[curMsg->charIndex[startLine - 1]];
      }
      
      //choose the appropriate font
      GFXFont *font = getFont(curMsg->type);
      
      msgOffset.y -= font->getHeight() * (curMsg->lineCount - startLine);
      
      //print the line
      for (int i = startLine; i < curMsg->lineCount; i++)
      {
         //temporarily replace the eolIndex char with a '\0';
         int eolIndex = curMsg->charIndex[i];
         char *tempPtr = &curMsg->msg[eolIndex];
         char temp = *tempPtr;
         *tempPtr = '\0';
         
         //print out the line
         sfc->drawText_p(font, &msgOffset, linePtr);
         
         //update the message offset
         msgOffset.y += font->getHeight();
         
         //replace the eolIndex char
         *tempPtr = temp;
         linePtr = tempPtr;
         
         //see if we should continue
         linesLeft--;
         if (linesLeft <= 0) break;
      }
      
      //set up for the next message
      msgOffset.y -= font->getHeight() * curMsg->lineCount;
      msgsLeft--;
      curMsgIndex--;
      if (curMsgIndex < 0) curMsgIndex = MessageHistoryLimit - 1;
      
      //update the individual line offsets (used only in the commander chat)
      if (frame % 2)
      {
         curMsg->lineOffset--; 
      }
   }
   
   //now draw the command message if it exists
   if (cmdExists)
   {
      displayRect.upperL.y = displayRect.lowerR.y - (mLowRes ? 0 : 2);
      displayRect.lowerR.y += cmdFont->getHeight();
      sfc->setClipRect(&displayRect);
      
      //temporarily replace the eolIndex char with a '\0';
      int eolIndex = cmdMsg->charIndex[0];
      char *tempPtr = &cmdMsg->msg[eolIndex];
      char temp = *tempPtr;
      *tempPtr = '\0';
         
      //print out the line
      sfc->drawText_p(cmdFont, &Point2I(displayRect.upperL.x + (gDisplayBoarderX / 2), displayRect.upperL.y), cmdMsg);
      
      //put the char back
      *tempPtr = temp;
   }
   
   setUpdate();

   if (! mbCommanderChat)
   {
      // temporary menu stuff:
      SimObject *obj = manager->findObject("PlayChatMenu"); 
      ChatMenu *menu = NULL;
      if(obj)
         menu = dynamic_cast<ChatMenu *>(obj);
      if(!menu)
         return;
      
      Point2I curDrawPoint(offset.x, offset.y + extent.y);
      sfc->setClipRect(&RectI(0,0,sfc->getWidth() -1, sfc->getHeight()-1));

      if(menu->getMode() == ChatMenu::Inactive || !menu->curMenu)
         return;
      ChatMenu::Node *parent = menu->curMenu->parent;
      char buf[1024];

      //find the height of the entire menu
      int menuHeight = teamMsgFont->getHeight();
      ChatMenu::Node *walk = menu->curMenu;
      while (walk)
      {
         menuHeight += teamMsgFont->getHeight();
         walk = walk->nextSibling;
      }
      
      if (curDrawPoint.y + menuHeight >= sfc->getHeight())
      {
         curDrawPoint.y = offset.y - menuHeight - 8;
      }
      
      if(!parent)
      {
         sfc->drawText_p(teamMsgFont, &curDrawPoint, menu->heading);
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
         sfc->drawText_p(teamMsgFont, &curDrawPoint, buf + lastPt);
      }
      curDrawPoint.y += msgFont->getHeight();
      walk = menu->curMenu;
      while(walk)
      {
         sprintf(buf, "%c: %s", walk->key, walk->heading);
         sfc->drawText_p(teamMsgFont, &curDrawPoint, buf);
         curDrawPoint.y += teamMsgFont->getHeight();
         walk = walk->nextSibling;
      }
   }
}

void FearGuiChatDisplay::page(int value)
{
   if (value < 0)
   {
      if (messageCount - pgUpCount > lineCount)
      {
         pgUpCount += max(lineCount - 1, 1);
         pgUpCount = min(pgUpCount, messageCount - lineCount);
         lineOffset = 0;
      }
   }
   else
   {
      if (pgUpCount > 0)
      {
         pgUpCount = max(0, pgUpCount - (lineCount - 1));
      }
   }
}
void FearGuiChatDisplay::setSize(int value)
{
   if (value < 0)
   {
      switch (lineCount)
      {
         case 3: lineCount = 5; break;
         case 5: lineCount = 20; break;
         case 20: lineCount = 3; break;
         default: lineCount = 3; break;
      }
   }
   else lineCount = value;
   pgUpCount = 0;
   extent.y = lineCount *  (mLowRes ? lr_msgFont->getHeight() : msgFont->getHeight()) + gDisplayBoarderY;
   resize(extent, extent);
   setVisible(TRUE);
}

bool FearGuiChatDisplay::pointInControl(Point2I &parentCoordPoint)
{
   if (! mbCommanderChat) return Parent::pointInControl(parentCoordPoint);
   else return FALSE;
}

void FearGuiChatDisplay::onMouseDown(const SimGui::Event &event)
{
   if (! mbCommanderChat) Parent::onMouseDown(event);
}

void FearGuiChatDisplay::onMouseDragged(const SimGui::Event &event)
{
   if (! mbCommanderChat) Parent::onMouseDragged(event);
}

void FearGuiChatDisplay::onMouseUp(const SimGui::Event &event)
{
   if (! mbCommanderChat) Parent::onMouseUp(event);
}
   
void FearGuiChatDisplay::inspectWrite(Inspect* insp)
{
   Parent::inspectWrite(insp);
   RealF tempLineCount = (int)lineCount;
   insp->write(IDITG_CHAT_DISPLAY_LINES, tempLineCount);
   insp->write(IDSTR_COMMANDER_VERSION, mbCommanderChat);
}

void FearGuiChatDisplay::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   RealF tempLineCount;
   insp->read(IDITG_CHAT_DISPLAY_LINES, tempLineCount);
   lineCount = max((int)tempLineCount, 0);
   extent.y = lineCount *  (mLowRes ? lr_msgFont->getHeight() : msgFont->getHeight()) + gDisplayBoarderY;
   resize(extent, extent);
   insp->read(IDSTR_COMMANDER_VERSION, mbCommanderChat);
}   

Persistent::Base::Error FearGuiChatDisplay::write( StreamIO &sio, int a, int b)
{
   extent.write(sio);
   sio.write(lineCount);
   Int32 output = isVisible();
   sio.write(output);
   output = mbCommanderChat;
   sio.write(output);
   return Parent::write(sio, a, b);
}

Persistent::Base::Error FearGuiChatDisplay::read( StreamIO &sio, int a, int b)
{
   extent.read(sio);
   sio.read(&lineCount);
   Int32 input;
   sio.read(&input);
   if (input) 
   {
      setVisible(true);
   }
   else
   {
      setVisible(false);
   }
   sio.read(&input);
   mbCommanderChat = input;
   return Parent::read(sio, a, b);
}

};