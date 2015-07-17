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
#include <g_surfac.h>
#include <g_bitmap.h>
#include <inspect.h>
#include "SimGuiTextEdit.h"
#include "FearGuiChat.h"
#include <FearDcl.h>
#include "ircClient.h"
#include "esf.strings.h"
#include "fear.strings.h"
#include "FearGuiHudCtrl.h"
#include "simResource.h"
#include "console.h"
#include "fearGuiShellPal.h"
#include "fearGuiChatDisplay.h"

namespace FearGui
{

#define HISTORY_LIMIT 20
static char gTextHistory[(HISTORY_LIMIT + 1) * 256];
static int gLastHistoryIndex = -1;
static int gCurHistoryIndex = -1;
static bool gVisible = FALSE;
static bool gInitialized = FALSE;

static const char * getHistory(int index)
{
   if (index >= 0 && index < HISTORY_LIMIT + 1)
   {
      return &gTextHistory[index * 256];   
   }
   return "";
}

static void setHistory(int index, const char *txt)
{
   if (index >= 0 && index < HISTORY_LIMIT + 1)
   {
      char *ptr = &gTextHistory[index * 256];
      if (txt) strncpy(ptr, txt, 255);
      else *ptr = '\0';
      ptr[255] = '\0';
   }
}

static void UpdateHistory(const char *txt)
{
   if (txt && txt[0])
   {
      gLastHistoryIndex++;
      if (gLastHistoryIndex >= HISTORY_LIMIT) gLastHistoryIndex = 0;
      setHistory(gLastHistoryIndex, txt);
      setHistory(HISTORY_LIMIT, "");
      gCurHistoryIndex = -1;
   }
}

int gDisplayBoarderY = 10;
int gDisplayBoarderX = 12;

IMPLEMENT_PERSISTENT_TAG( FearGuiChat, FOURCC('F','G','c','t') );

bool FearGuiChat::onAdd(void)
{
   if (! Parent::onAdd())
      return FALSE;
      
   //make sure we have a font
   msgFont = SimResource::get(manager)->load("sf_yellow_10b.pft");
   teamMsgFont = SimResource::get(manager)->load("if_g_10b.pft");
   lr_msgFont = SimResource::get(manager)->load("sf_yellow_6.pft");
   lr_teamMsgFont = SimResource::get(manager)->load("sf_green_6.pft");
   lr_msgFont->fi.flags |= FONT_LOWERCAPS;
   lr_teamMsgFont->fi.flags |= FONT_LOWERCAPS;
      
   //clear out any remnant text
   setText("");
   lineCount = 2;
   extent.x = 370;
   extent.y = lineCount *  msgFont->getHeight() + gDisplayBoarderY;
   
   //chat should start out hidden
   setVisible(FALSE);

   mLowRes = FALSE;
   
   //get the IRC client
   mIRCClient = ::IRCClient::find(manager);
   AssertFatal(mIRCClient, "unable to locate IRC client");
   
   //initialize the history
   if (! gInitialized)
   {
      gInitialized = TRUE;
      for (int i = 0; i < HISTORY_LIMIT + 1; i++)
      {
         setHistory(i, "");
      }
      gVisible = FALSE;
   }
   
   return TRUE;
}

void FearGuiChat::onWake()
{
   //position.x = fracPos.x * (parent->extent.x - extent.x);
   //position.y = fracPos.y * (parent->extent.y - extent.y);
   setVisible(gVisible);
   if (gVisible)
   {
      setText(getHistory(HISTORY_LIMIT));
      
      //format the message
      formatMessage(extent.x - (2 * gDisplayBoarderX));
   }
}

void FearGuiChat::formatMessage(int width)
{
   //make sure the window has some width
   if (width <= 32) return;
   
   const char *var;
   var = Console->getVariable("pref::msgChannel");
   char chanBuf[32];
   int channel = max(min(atoi(var), 31), 0); 
   if (channel > 0)
   {
      sprintf(chanBuf, "SAY TEAM: ", channel);
   }
   else
   {
      sprintf(chanBuf, "SAY: ");
   }
   GFXFont *font;
   if (mLowRes)
   {
      font = (channel > 0 ? lr_teamMsgFont : lr_msgFont);
   }
   else
   {
      font = (channel > 0 ? teamMsgFont : msgFont);
   }
   int commandWidth = font->getStrWidth(chanBuf);
   int stringWidth = width - commandWidth;
   
   char buf[256];
   char *bufPtr = &buf[0];
   getText(buf);
   int lengthLeft = strlen(bufPtr);
   char *tempPtr, *noSpacePtr;
   char temp;
   
   //initialize the number of lines in the message
   msgLineCount = 0;
   
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
         if (font->getStrWidth(bufPtr) <= stringWidth)
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
      charIndex[msgLineCount] = tempPtr - &buf[0];
      
      //update the lengthLeft
      lengthLeft -= strlen(bufPtr);
      stringWidth = width;
      
      //replace the missing character and set bufPtr to be the next string portion
      *tempPtr = temp;
      bufPtr = tempPtr;
      
      //on to the next line
      msgLineCount++;
      AssertFatal(msgLineCount <= MaxLineCount, "Message is too long!");
   }
}

static void drawBracket(GFXSurface *sfc, int x, int ty, int by, int dx)
{
   int dir = 1;
   if(dx < 0)
      dir = -1;
   sfc->drawLine2d(&Point2I(x, ty), &Point2I(x, by), GREEN_132);
   sfc->drawLine2d(&Point2I(x+dir, ty), &Point2I(x+dir, by), GREEN);
   sfc->drawLine2d(&Point2I(x+dir, ty), &Point2I(x+dir+dx, ty), GREEN);
   sfc->drawLine2d(&Point2I(x+dir, by), &Point2I(x+dir+dx, by), GREEN);
}

void FearGuiChat::onPreRender(void)
{
   if (parent)
   {
      position.set((parent->extent.x - extent.x) / 2, parent->extent.y - 40 - lineCount *  msgFont->getHeight());
   }
}

void FearGuiChat::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   
   Point2I canvasSize(sfc->getWidth(), sfc->getHeight());
   
   //check for lowres
   if (canvasSize.x < 512 || canvasSize.y < 384)
   {
      if (! mLowRes)
      {
         mLowRes = TRUE;
         resize(position, extent);
         return;
      }
   }
   else
   {
      if (mLowRes)
      {
         mLowRes = FALSE;
         resize(position, extent);
         return;
      }
   }
   
   if (lineCount <= 0) return;
   
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
   
   drawBracket(sfc, topLeft->x, topLeft->y, bottomRight->y - 1, 4);
   drawBracket(sfc, bottomRight->x - 1, topLeft->y, bottomRight->y - 1, -4);
   
   //set the clip rect to prevent text from drawing outside the translucency
   displayRect.upperL.x += 12;
   displayRect.upperL.y += 4;
   displayRect.lowerR.x -= 12;
   displayRect.lowerR.y -= 4;
   sfc->setClipRect(&displayRect);
   
   //check the cursor timer
   SimTime timeElapsed = manager->getCurrentTime() - timeLastCursorFlipped;
   numFramesElapsed++;
   if ((timeElapsed > SimTime(0.5f)) && (numFramesElapsed > 3) )
   {
      cursorOn = !cursorOn;
      timeLastCursorFlipped = manager->getCurrentTime();   
      numFramesElapsed = 0;
   }
   
   //draw the command
   Point2I msgOffset;
   msgOffset.x = displayRect.upperL.x;
   msgOffset.y = topLeft->y - (mLowRes ? 0 : 2);
   
   const char *var;
   var = Console->getVariable("pref::msgChannel");
   char chanBuf[32];
   int channel;
   channel = max(min(atoi(var), 31), 0); 
   GFXFont *font;
   if (mLowRes)
   {
      font = (channel > 0 ? lr_teamMsgFont : lr_msgFont);
   }
   else
   {
      font = (channel > 0 ? teamMsgFont : msgFont);
   }
   if (channel > 0)
   {
      sprintf(chanBuf, "SAY TEAM: ", channel);
   }
   else
   {
      sprintf(chanBuf, "SAY: ");
   }
   sfc->drawText_p(font, &msgOffset, chanBuf);
   msgOffset.x += font->getStrWidth(chanBuf);
   
   // now draw the text
   char buf[256];
   getText(buf);
   char *linePtr = &buf[0];
   int tempCursorPos = cursorPos;
   
   for (int ii = 0; ii < msgLineCount; ii++)
   {
      //temporarily replace the eolIndex char with a '\0';
      int eolIndex = charIndex[ii];
      char *tempPtr = &buf[eolIndex];
      char temp = *tempPtr;
      *tempPtr = '\0';
      
      //print out the line
      sfc->drawText_p(font, &msgOffset, linePtr);
      
      //see if the cursor should be drawn here
      if (cursorOn && (tempCursorPos >= 0) && (tempCursorPos <= (int)strlen(linePtr)))
      {
         char temp2 = linePtr[tempCursorPos];
         linePtr[tempCursorPos] = '\0';
         int cursorStrLength = font->getStrWidth(linePtr);
         linePtr[tempCursorPos] = temp2;
         tempCursorPos = -1;
   
         Point2I cursorStart = msgOffset;
         cursorStart.x += cursorStrLength;
         cursorStart.y += (mLowRes ? 0 : 3);
         Point2I cursorEnd = cursorStart;
         cursorEnd.y += font->getHeight();
         sfc->drawLine2d(&cursorStart, &cursorEnd, GREEN);
      }
      else
      {
         tempCursorPos -= strlen(linePtr);
      }
      
      //update the message offset
      msgOffset.x = topLeft->x;
      msgOffset.y += font->getHeight();
      
      //replace the eolIndex char
      *tempPtr = temp;
      linePtr = tempPtr;
   }
   
   //if the string was "", the cursor won't have been drawn yet
   if (cursorOn && (tempCursorPos == 0))
   {
      Point2I cursorStart = msgOffset;
      cursorStart.y += (mLowRes ? 0 : 3);
      Point2I cursorEnd = cursorStart;
      cursorEnd.y += font->getHeight();
      sfc->drawLine2d(&cursorStart, &cursorEnd, GREEN);
   }
}

void FearGuiChat::activate(void)
{
   setVisible(TRUE);
   mbFirstKeyPress = TRUE;
   gVisible = TRUE;
   setText(getHistory(HISTORY_LIMIT));
   
   //format the message
   formatMessage(extent.x - (2 * gDisplayBoarderX));

   root->makeFirstResponder(this);
}

bool FearGuiChat::becomeFirstResponder()
{
   numFramesElapsed = 0;
   timeLastCursorFlipped = manager->getCurrentTime();
   cursorOn = 1;
   return Parent::becomeFirstResponder();   
}

void FearGuiChat::onKeyDown(const SimGui::Event &event)
{
   GFXFont *font = (mLowRes ? lr_msgFont : msgFont);
   
   //int stringLen = strlen(text);
   switch(event.diKeyCode)
   {
      case DIK_DELETE:
      case DIK_BACK:
         if (mbFirstKeyPress)
         {
            setText("");
            setHistory(HISTORY_LIMIT, "");
      
            //format the message
            formatMessage(extent.x - (2 * gDisplayBoarderX));
            
            mbFirstKeyPress = FALSE;
            return;
         }
         break;
      
      case DIK_UP:
      case DIK_DOWN:
         mbFirstKeyPress = FALSE;
         if (gLastHistoryIndex >= 0)
         {
            if (gCurHistoryIndex < 0) gCurHistoryIndex = gLastHistoryIndex;
            else
            {
               if (event.diKeyCode == DIK_UP)
               {
                  gCurHistoryIndex--;
                  if (gCurHistoryIndex < 0)
                  {
                     const char *temp = getHistory(HISTORY_LIMIT - 1);
                     if (temp[0]) gCurHistoryIndex = HISTORY_LIMIT - 1;
                     else gCurHistoryIndex = gLastHistoryIndex;
                  }
               }
               else
               {
                  gCurHistoryIndex++;
                  if (gCurHistoryIndex > HISTORY_LIMIT - 1) gCurHistoryIndex = 0;
                  else if (gCurHistoryIndex > gLastHistoryIndex)
                  {
                     const char *temp = getHistory(gCurHistoryIndex);
                     if (! temp[0]) gCurHistoryIndex = 0;
                  }
               }
               if (gCurHistoryIndex == gLastHistoryIndex) gCurHistoryIndex = -1;
            }
            if (gCurHistoryIndex >= 0)
            {
               setText(getHistory(gCurHistoryIndex));
               setHistory(HISTORY_LIMIT, getHistory(gCurHistoryIndex));
            }
            else
            {
               setText("");
               setHistory(HISTORY_LIMIT, "");
            }
      
            //format the message
            formatMessage(extent.x - (2 * gDisplayBoarderX));
         }
         return;
         
      case DIK_ESCAPE:
         mbFirstKeyPress = FALSE;
         root->makeFirstResponder(NULL);
         setVisible(FALSE);
         gVisible = FALSE;
         gCurHistoryIndex = -1;
         return;
         
      case DIK_RETURN:
         mbFirstKeyPress = FALSE;
         root->makeFirstResponder(NULL);
         setVisible(FALSE);
         gVisible = FALSE;
         const char *var = Console->getVariable("pref::msgChannel");
         char buf[256], buf2[256], buf3[256], *bufPtr, *destPtr;
         getText(buf3);
         UpdateHistory(buf3);
         
         //go through, and preceed all quotes with a backslashe
         bufPtr = &buf3[0];
         destPtr = &buf[0];
         while (*bufPtr != '\0')
         {
            if (*bufPtr == '\"' || *bufPtr == '\\') *destPtr++ = '\\';
            *destPtr++ = *bufPtr++;
         }
         *destPtr = '\0';
         
         
         //see if it's meant for IRC or not
         if (buf[0] == '/')
         {
            if (mIRCClient->getState() != IDIRC_DISCONNECTED)
            {
               char temp[5];
               strncpy(temp, &buf[1], 4);
               temp[4] = '\0';
               char *addressee = NULL;
               char *bufPtr = &buf[1];
               
               //see if it's a private message
               if (! stricmp(temp, "msg "))
               {
                  //get the addressee
                  bufPtr = &buf[5];
                  while (*bufPtr != ' ' && *bufPtr != '\0') bufPtr++;
                  if (*bufPtr == ' ' && bufPtr != &buf[1])
                  {
                     *bufPtr++ = '\0';
                     addressee = &buf[5];
                  }
               }
               mIRCClient->send(bufPtr, addressee); 
            }
            else
            {
               FearGuiChatDisplay *chatDisplay = dynamic_cast<FearGuiChatDisplay*>(findRootControlWithTag(IDCTG_HUD_CHAT_DISPLAY));
               if (chatDisplay) chatDisplay->newMessage("Not connected to an IRC server.", FearGuiChatDisplay::MSG_IRC);
            }
         }
         else
         {
            sprintf(buf2, "say(%s, \"%s\");", var, buf);
            Console->evaluate(buf2, false);
         }
         setText("");
         lineCount = 2;
         extent.y = lineCount *  font->getHeight() + gDisplayBoarderY;
         resize(extent, extent);
         
         return;
   }
   
   mbFirstKeyPress = FALSE;
         
   //let the parent class do the rest
   Parent::onKeyDown(event);
   char buf[256];
   getText(buf);
   setHistory(HISTORY_LIMIT, buf);
   
   //reformat the message
   formatMessage(extent.x - (2 * gDisplayBoarderX));
   
   //see if the screen needs to be enlarged
   if (msgLineCount > lineCount)
   {
      lineCount = msgLineCount;
      extent.y = lineCount *  font->getHeight() + gDisplayBoarderY;
      resize(extent, extent);
   }
}

void FearGuiChat::resize(const Point2I &, const Point2I &newExtent)
{
   if (! parent) return;
   GFXFont *font = (mLowRes ? lr_msgFont : msgFont);
   extent.x = max((int)min((int)max((int)newExtent.x, (mLowRes ? 200 : 370)), (int)parent->extent.x - 1), 1);
   extent.y = max((int)min(newExtent.y, parent->extent.y - 1), 1);
   lineCount = (extent.y - gDisplayBoarderY) / font->getHeight();
   extent.y = lineCount *  font->getHeight() + gDisplayBoarderY;
   
   //reformat the message
   formatMessage(extent.x - (2 * gDisplayBoarderX));
}

void FearGuiChat::inspectWrite(Inspect* insp)
{
   Parent::inspectWrite(insp);
}

void FearGuiChat::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   lineCount = 2;
   extent.y = lineCount *  msgFont->getHeight() + gDisplayBoarderY;
   resize(extent, extent);
}   

Persistent::Base::Error FearGuiChat::write( StreamIO &sio, int a, int b)
{
   fracPos.write(sio);
   extent.write(sio);
   return Parent::write(sio, a, b);
}

Persistent::Base::Error FearGuiChat::read( StreamIO &sio, int a, int b)
{
   fracPos.read(sio);
   extent.read(sio);
   setVisible(false);
   return Parent::read(sio, a, b);
}

};