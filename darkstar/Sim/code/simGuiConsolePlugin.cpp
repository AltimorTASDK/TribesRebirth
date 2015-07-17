#include <g_pal.h>
#include <simgame.h>
#include <siminput.h>
#include <simcanvas.h>
#include <simresource.h>
#include <simtagdictionary.h>
#include <darkstar.strings.h>
#include <simguiconsoleplugin.h>
#include <simConsoleEvent.h>

DWORD SimGuiConsolePlugin::lastLineTimeout = 4000;

SimGuiConsolePlugin::SimGuiConsolePlugin()
: lpszHiStart(NULL), pagedBack(false), lpszHiEnd(NULL), 
  rsFont(NULL), pEvent(NULL), fEnabled(true), 
  fOverwrite(false), iCurrentCmd(0), lpszPos(szCmd), 
  iPageDir(0), iLastLog(0), lpszSuffix(NULL), fActive(false)
{
   pe_fore.peRed = pe_fore.peGreen = pe_fore.peBlue = 255;
   pe_back.peRed = pe_back.peGreen = pe_back.peBlue = 0;
   pe_fore.peFlags = 0;
   pe_back.peFlags = 0;
   szCmd[0]      = '\0';
   szPrefix[0]   = '\0';
   colorChanged = true;
}

SimGuiConsolePlugin::~SimGuiConsolePlugin()
{
   delete pEvent;
   while(history.size())
   {
      delete history.last();
      history.pop_back();
   }
}

static int renderedLines = 0;

void SimGuiConsolePlugin::activate(bool fActivate)
{
   timer.reset();

   if (pEvent)
   {
      delete pEvent;
      pEvent = NULL;
   }

   fActive       = fActivate;
   renderedLines = 0;
   colorChanged = true;

   if (bool(rsFont) == false)
   {
      rsFont = SimResource::get(manager)->load("console.pft", true);
   }
}

bool SimGuiConsolePlugin::active()
{
   return (fActive);
}

bool SimGuiConsolePlugin::enabled()
{
   return (fEnabled);
}

void SimGuiConsolePlugin::handleInput()
{
   char szBuf[MAX_CMD];

   switch (pEvent->objInst)
   {
      case DIK_TAB :
         if (!pEvent->modifier)
         {
            iPageDir    = 0;
            lpszHiStart = NULL;
            lpszHiEnd   = NULL;

            // We can only complete if we have something, and we are at the "end"
            // of the entered command
            if (strlen(szCmd) && (lpszPos == &szCmd[strlen(szCmd)] || lpszSuffix))
            {
               char       *lpszTmp;
               const char *lpszCompletion = NULL;

               if (lpszSuffix)
               {
                  *lpszSuffix = '\0';
               }

               lpszTmp = &szCmd[strlen(szCmd)];

               // Search backward to beginning of line or to last
               // separating token
               while ( lpszTmp >= szCmd &&
                      *lpszTmp != ' ' && *lpszTmp != '"' && *lpszTmp != '(' &&
                      *lpszTmp != ')' && *lpszTmp != ';')
               {
                  lpszTmp --;
               }

               // Advance to skip past separator, or to get back to our
               // valid pointer
               lpszTmp ++;

               if (strlen(lpszTmp))
               {
                  if (szPrefix[0] == '\0')
                  {
                     memset(szPrefix, '\0', MAX_CMD);
                     strcpy(szPrefix, lpszTmp);
                  }

                  lpszCompletion = console->
                     tabComplete(lpszTmp, strlen(szPrefix), !(bool)(pEvent->modifier & SI_SHIFT));
               }

               if (lpszCompletion && strlen(lpszCompletion))
               {
                  if (lpszHiStart && lpszHiEnd)
                  {
                     lpszHiStart = lpszHiEnd = NULL;
                  }
                  *lpszTmp = '\0';
                  memset (szBuf, '\0', MAX_CMD);

                  if (*lpszCompletion != '$')
                  {
                     sprintf(szBuf, "%s%s();", szCmd, lpszCompletion);
                     memset (szCmd, '\0', MAX_CMD);
                     strcpy (szCmd, szBuf);
                     lpszPos    = &szCmd[strlen(szCmd) - 2];
                     lpszSuffix = &szCmd[strlen(szCmd) - 3];
                  }
                  else
                  {
                     sprintf(szBuf, "%s%s", szCmd, lpszCompletion);
                     memset (szCmd, '\0', MAX_CMD);
                     strcpy (szCmd, szBuf);
                     lpszPos = &szCmd[strlen(szCmd)];
                  }
               }
               else
               {
                  // Strip out any previous completions
                  szCmd[strlen(szCmd) - strlen(lpszTmp) + strlen(szPrefix)] = '\0';
                  memset(szPrefix, '\0', MAX_CMD);

                  if (console->isFunction(szCmd) && szCmd[0] != '$')
                  {
                     sprintf(szBuf, "%s();", szCmd);
                     strcpy (szCmd, szBuf);
                     lpszPos = &szCmd[strlen(szCmd) - 2];
                  }
                  else
                  {
                     lpszPos = &szCmd[strlen(szCmd)];
                  }
               }
            }
         }
         break;
      case DIK_ESCAPE :
         memset(szCmd,    '\0', MAX_CMD);
         memset(szPrefix, '\0', MAX_CMD);
         iPageDir    = 0;
         lpszPos     = szCmd;
         iCurrentCmd = history.size();
         lpszHiStart = lpszHiEnd = NULL;
         lpszSuffix  = NULL;
         break;
      case DIK_INSERT:
         fOverwrite = !fOverwrite;
         break;
      case DIK_PGUP:
         if(pEvent->modifier & SI_SHIFT)
         {
            pagedBack = true;
            iLastLog = 0;
         }
         else
         {
            if(renderedLines)
            {
               int scrollAmt = renderedLines > 1 ? renderedLines - 1 : 1;
               if(pagedBack)
               {
                  iLastLog -= scrollAmt;
                  if(iLastLog < renderedLines)
                     iLastLog = renderedLines;
               }
               else
               {
                  if(console->getLog()->size() > scrollAmt)
                  {
                     pagedBack = true;
                     iLastLog = console->getLog()->size() - scrollAmt;
                  }
               }
            }
         }
         break;
      case DIK_PGDN:
         if(pEvent->modifier & SI_SHIFT)
            pagedBack = false;
         else
         {
            if(renderedLines)
            {
               iLastLog += renderedLines > 1 ? renderedLines - 1 : 1;
               if(iLastLog >= console->getLog()->size())
                  pagedBack = false;
            }
         }
         break;
      case DIK_UP:
         memset(szPrefix, '\0', MAX_CMD);
         lpszSuffix  = NULL;
         lpszHiStart = lpszHiEnd = NULL;

         if (iCurrentCmd > 0)
         {
            memset(szCmd, '\0', MAX_CMD);
            strcpy(szCmd, history[-- iCurrentCmd]->c_str());
            lpszPos = &szCmd[strlen(szCmd)];
         }
         break;
      case DIK_DOWN:
         iPageDir    = 0;
         lpszSuffix  = NULL;
         lpszPos     = &szCmd[0];
         lpszHiStart = lpszHiEnd = NULL;
         memset(szCmd,    '\0', MAX_CMD);
         memset(szPrefix, '\0', MAX_CMD);
         if (iCurrentCmd < history.size())
         {
            // "Ghost" command to give blank line at end
            iCurrentCmd ++;
            if (iCurrentCmd < history.size())
            {
               strcpy(szCmd, history[iCurrentCmd]->c_str());
               lpszPos = &szCmd[strlen(szCmd)];
            }
         }
         break;
      case DIK_LEFT:
         iPageDir   = 0;
         lpszSuffix = NULL;
         memset(szPrefix, '\0', MAX_CMD);
         if (lpszPos > szCmd)
         {
            char *lpszTmp;

            if (lpszHiStart)
            {
               lpszTmp = lpszHiStart;
            }
            else
            {
               lpszTmp = lpszPos;
            }

            // Word skipping
            if (pEvent->modifier & SI_CTRL)
            {

               while ((lpszTmp > szCmd) && (*(lpszTmp - 1) == ' '))
                  lpszTmp --;

               while ((lpszTmp > szCmd) && (*(lpszTmp - 1) != ' '))
                  lpszTmp --;
            }
            // Letter skipping
            else
            {
               if (lpszTmp > szCmd)
               {
                  lpszTmp --;
               }
            }

            if (pEvent->modifier & SI_SHIFT)
            {
               // If highlighting, set end of highlight region
               if (lpszHiEnd == NULL)
               {
                  lpszHiEnd   = lpszPos;
                  lpszHiStart = lpszPos = lpszTmp;
               }
               else if (lpszPos == lpszHiEnd)
               {
                  lpszPos = -- lpszHiEnd;

                  if (lpszHiEnd == lpszHiStart)
                  {
                     lpszHiStart = lpszHiEnd = NULL;
                  }
               }
               else
               {
                  lpszHiStart = lpszPos = lpszTmp;
               }
            }
            else
            {
               lpszPos = lpszTmp;
               lpszHiStart = lpszHiEnd = NULL;
            }
         }
         else
         {
            if (!(pEvent->modifier & SI_SHIFT))
            {
               lpszHiStart = lpszHiEnd = NULL;
            }
         }
         break;
      case DIK_RIGHT:
         iPageDir   = 0;
         lpszSuffix = NULL;
         memset(szPrefix, '\0', MAX_CMD);
         if ((lpszPos - szCmd) < (int)strlen(szCmd))
         {
            char *lpszTmp;

            if (lpszHiEnd)
            {
               lpszTmp = lpszHiEnd;
            }
            else
            {
               lpszTmp = lpszPos;
            }

            if (pEvent->modifier & SI_CTRL)
            {
               char *lpszEnd = &szCmd[strlen(szCmd)];

               while ((lpszTmp < lpszEnd) && (*lpszTmp != ' '))
                  lpszTmp ++;

               while ((lpszTmp < lpszEnd) && (*lpszTmp == ' '))
                  lpszTmp ++;
            }
            else
            {
               if ((lpszTmp - szCmd) < (int)strlen(szCmd))
               {
                  lpszTmp ++;
               }
            }

            if (pEvent->modifier & SI_SHIFT)
            {
               if (lpszHiStart == NULL)
               {
                  lpszHiStart = lpszPos;
                  lpszHiEnd = lpszPos = lpszTmp;
               }
               else if (lpszPos == lpszHiStart)
               {
                  lpszPos = ++ lpszHiStart;

                  if (lpszHiStart == lpszHiEnd)
                  {
                     lpszHiStart = lpszHiEnd = NULL;
                  }
               }
               else
               {
                  lpszHiEnd = lpszPos = lpszTmp;
               }
            }
            else
            {
               lpszPos = lpszTmp;
               lpszHiStart = lpszHiEnd = NULL;
            }
         }
         else
         {
            if (!(pEvent->modifier & SI_SHIFT))
            {
               lpszHiStart = lpszHiEnd = NULL;
            }
         }
         break;
      case DIK_HOME:
         iPageDir   = 0;
         lpszSuffix = NULL;
         memset(szPrefix, '\0', MAX_CMD);
         if (pEvent->modifier & SI_SHIFT)
         {
            if (lpszHiEnd == NULL)
            {
               lpszHiEnd = lpszPos;
            }
            else
            {
               lpszHiEnd = lpszHiStart;
            }
            lpszPos = lpszHiStart = szCmd;
         }
         else
         {
            lpszHiStart = lpszHiEnd = NULL;
            lpszPos = szCmd;
         }
         break;
      case DIK_END:
         iPageDir   = 0;
         
         lpszSuffix = NULL;
         memset(szPrefix, '\0', MAX_CMD);
         if (pEvent->modifier & SI_SHIFT)
         {
            if (lpszHiStart == NULL)
            {
               lpszHiStart = lpszPos;
            }
            else
            {
               lpszHiStart = lpszHiEnd;
            }
            lpszPos = lpszHiEnd = &szCmd[strlen(szCmd)];
         }
         else
         {
            lpszHiStart = lpszHiEnd = NULL;
            lpszPos = &szCmd[strlen(szCmd)];
         }
         break;
      case DIK_DELETE:
         iPageDir   = 0;
         lpszSuffix = NULL;
         memset(szPrefix, '\0', MAX_CMD);
         if (lpszHiStart && lpszHiEnd)
         {
            *lpszHiStart = '\0';
            lpszPos = &szCmd[strlen(szCmd)];
            sprintf(szBuf, "%s%s", szCmd, lpszHiEnd);
            memset (szCmd, '\0', MAX_CMD);
            strcpy (szCmd, szBuf);
          
            lpszHiStart = lpszHiEnd = NULL;
         }
         else if ((lpszPos - szCmd) < (int)strlen(szCmd))
         {
            strcpy(szBuf, lpszPos + 1);
            strcpy(lpszPos, szBuf);
         }
         break;
      case DIK_BACK:
         iPageDir   = 0;
         lpszSuffix = NULL;
         memset(szPrefix, '\0', MAX_CMD);
         if (lpszPos > szCmd)
         {
            if (lpszHiStart && lpszHiEnd)
            {
               *lpszHiStart = '\0';
               lpszPos = &szCmd[strlen(szCmd)];
               sprintf(szBuf, "%s%s", szCmd, lpszHiEnd);
               memset (szCmd, '\0', MAX_CMD);
               strcpy (szCmd, szBuf);
          
               lpszHiStart = lpszHiEnd = NULL;
            }
            else if ((lpszPos - szCmd) < (int)strlen(szCmd))
            {
               strcpy(szBuf, lpszPos);
               lpszPos --;
               strcpy(lpszPos, szBuf);
            }
            else
            {
               *(-- lpszPos) = '\0';
            }
         }
         break;
      default :
         iPageDir   = 0;
         if (pEvent->ascii)
         {
            lpszSuffix = NULL;
            memset(szPrefix, '\0', MAX_CMD);
         }
         switch (pEvent->ascii)
         {
            case '\b':
               if (lpszPos > szCmd)
               {
                  if (lpszHiStart && lpszHiEnd)
                  {
                     *lpszHiStart = '\0';
                     lpszPos = &szCmd[strlen(szCmd)];
                     sprintf(szBuf, "%s%s", szCmd, lpszHiEnd);
                     memset (szCmd, '\0', MAX_CMD);
                     strcpy (szCmd, szBuf);
          
                     lpszHiStart = lpszHiEnd = NULL;
                  }
                  else if ((lpszPos - szCmd) < (int)strlen(szCmd))
                  {
                     strcpy(szBuf, lpszPos);
                     lpszPos --;
                     strcpy(lpszPos, szBuf);
                  }
                  else
                  {
                     *(-- lpszPos) = '\0';
                  }
               }
               break;
            case '\n':
            case '\r':
            {
               // Get rid of that nasty screen-scroll thingy when 
               // a command that removes DirectInput is executed
               delete pEvent;
               pEvent = NULL;

               if (lpszHiStart && lpszHiEnd)
               {
                  lpszHiStart = lpszHiEnd = NULL;
               }

               if (strlen(szCmd))
               {
                  SimConsoleEvent::postFocused(szCmd, true, 0);

                  // Add command to history buffer
                  int iConsoleHistory = console->getIntVariable(CONSOLE_HISTORY);

                  if (iConsoleHistory)
                  {
                     history.push_back(new String(szCmd));

                     iCurrentCmd = history.size();

                     // Keep the history buffer down
                     if (iCurrentCmd > iConsoleHistory)
                     {
                        delete history.first();
                        history.pop_front();
                        iCurrentCmd --;
                     }
                  }
                  else
                  {
                     history.clear();
                  }
                  memset(szPrefix, '\0', MAX_CMD);
                  memset(szCmd,    '\0', MAX_CMD);
                  lpszPos = szCmd;
               }
               else
               {
                  console->printf("\n");
               }
               break;
            }

            case 'c':
               if (pEvent->modifier & SI_CTRL)
               {
                  if (lpszHiStart && lpszHiEnd)
                  {
                     HGLOBAL hGlobal;
                     LPVOID  pGlobal;
                     char    c = *lpszHiEnd;

                     *lpszHiEnd = '\0';
                     hGlobal = GlobalAlloc(GHND, strlen(szCmd) + 1);
                     pGlobal = GlobalLock (hGlobal);

                     strcpy((char *)pGlobal, lpszHiStart);

                     *lpszHiEnd = c;

                     GlobalUnlock(hGlobal);

                     OpenClipboard(NULL);
                     EmptyClipboard();
                     SetClipboardData(CF_TEXT, hGlobal);
                     CloseClipboard();
                  }
                  break;
               }
            case 'v':
               if (pEvent->modifier & SI_CTRL)
               {
                  if (IsClipboardFormatAvailable(CF_TEXT))
                  {
                     HGLOBAL hGlobal;
                     LPVOID  pGlobal;
                     char    szTmp[MAX_CMD];
                     int     iCurLen;

                     memset(szTmp, '\0', MAX_CMD);

                     OpenClipboard(NULL);
                     hGlobal = GetClipboardData(CF_TEXT);
                     pGlobal = GlobalLock(hGlobal);
                     iCurLen = strlen(szCmd);

                     if (lpszHiStart && lpszHiEnd)
                     {
                        iCurLen -= (lpszHiStart - lpszHiEnd);
                     }

                     if ((iCurLen + strlen((char *)pGlobal)) >= MAX_CMD)
                     {
                        strncpy(szTmp, (char *)pGlobal, MAX_CMD - iCurLen - 1);
                     }
                     else
                     {
                        strcpy(szTmp, (char *)pGlobal);
                     }

                     if (lpszHiStart && lpszHiEnd)
                     {
                        *lpszHiStart = '\0';
                        sprintf(szBuf, "%s%s%s", szCmd, szTmp, lpszHiEnd);
                        memset (szCmd, '\0', MAX_CMD);
                        strcpy (szCmd, szBuf);
                        lpszPos += strlen(szTmp);

                        lpszHiStart = lpszHiEnd = NULL;
                     }
                     else
                     {
                        char c = *lpszPos;
                        *lpszPos = '\0';
                        sprintf(szBuf, "%s%s%c%s", szCmd, szTmp, c, lpszPos + 1);
                        memset (szCmd, '\0', MAX_CMD);
                        strcpy (szCmd, szBuf);
                        lpszPos += strlen(szTmp);
                     }

                     GlobalUnlock(hGlobal);
                     CloseClipboard();
                  }
                  break;
               }
            default :
               // If it's a printable character ...
               if (pEvent->ascii && isprint(pEvent->ascii))
               {
                  // If they have a block of text highlighted, there's
                  // no way adding one character will make the buffer larger
                  if (lpszHiStart && lpszHiEnd)
                  {
                     *lpszHiStart = '\0';
                     sprintf(szBuf, "%s%c%s", szCmd, pEvent->ascii, lpszHiEnd);
                     memset (szCmd, '\0', MAX_CMD);
                     strcpy (szCmd, szBuf);
          
                     lpszPos = lpszHiStart + 1;
                     lpszHiStart = lpszHiEnd = NULL;
                  }
                  // Otherwise, if this a destructive character add ...
                  else if (fOverwrite)
                  {
                     // They can only add to the end of the buffer if
                     // they have room left ... they can overwrite 
                     // anywhere in the buffer b/c that won't increase
                     // its size
                     if (lpszPos != &szCmd[strlen(szCmd)] || 
                         strlen(szCmd) < MAX_CMD - 1)
                     {
                        *(lpszPos ++) = pEvent->ascii;
                     }
                     else
                     {
                        MessageBeep(MB_ICONEXCLAMATION);
                     }
                  }
                  // If this is an insertion
                  else
                  {
                     // Make sure the inserted character won't overflow
                     if (strlen(szCmd) < MAX_CMD - 1)
                     {
                        char c = *lpszPos;
                        *lpszPos = '\0';
                        sprintf(szBuf, "%s%c", szCmd, pEvent->ascii);
                        *lpszPos = c;
                        strcat (szBuf, lpszPos);
                        strcpy (szCmd, szBuf);
                        lpszPos ++;
                     }
                     else
                     {
                        MessageBeep(MB_ICONEXCLAMATION);
                     }
                  }
               }
               break;
         }
   }
}

bool SimGuiConsolePlugin::onSimInputEvent(const SimInputEvent *event)
{
   if (event->action == SI_BREAK && pEvent)
   {
      delete pEvent;
      pEvent = NULL;
   }

   if (event->action == SI_MAKE)
   {
      delete pEvent;
      pEvent = new SimInputEvent;

      pEvent->deviceInst = event->deviceInst;
      pEvent->fValue     = event->fValue;
      pEvent->deviceType = event->deviceType;
      pEvent->objType    = event->objType;
      pEvent->objInst    = event->objInst;
      pEvent->action     = event->action;
      pEvent->ascii      = event->ascii;
      pEvent->modifier   = event->modifier;

      handleInput();
      timer.reset();
      iRepeatsPerSec = 1;
   }
   return (true);
}

void SimGuiConsolePlugin::render(GFXSurface *sfc)
{
   Point2I               point;
   GFXFontInfo           info;
   Vector<const char *> *log = console->getLog();
   char                  szBuf[MAX_CMD + MAX_PROMPT];

   if (sfc != NULL && bool(rsFont))
   {
      if (colorChanged && sfc->getPalette())
      {
         rsFont->setForeColor(sfc->getPalette()->GetNearestColor(pe_fore));
         rsFont->setBackColor(sfc->getPalette()->GetNearestColor(pe_back));
         colorChanged = false;
      }

      sfc->setHazeSource (GFX_HAZE_NONE);
      sfc->setAlphaSource(GFX_ALPHA_NONE);
      sfc->setShadeSource(GFX_SHADE_NONE);

      if(fActive && fEnabled)
      {
         if(iLastLog > log->size())
            pagedBack = false;

         // Get info about our font
         rsFont->getExtents(&info);

         // Format the current command line
         sprintf(szBuf, "%s%s", console->getVariable(CONSOLE_PROMPT), szCmd);
 
         // Get our starting draw point
         point.set(2, sfc->getHeight() - info.baseline - 10);

         // Draw line of text
         renderLine(sfc, &info, point, szBuf, lpszPos, lpszHiStart, lpszHiEnd);
         int i;
         if(pagedBack)
         {
            renderLine(sfc, &info, point, "-V-V-V-V-");
            i = iLastLog - 1;
         }
         else
            i = log->size() - 1;

         // Now display history of console
         renderedLines = 0;
         for (; i >= 0 && point.y > 0; i--)
         {
            renderLine(sfc, &info, point, (*log)[i]);
            renderedLines++;
         }
      }
      else if(lastLineTime + lastLineTimeout > GetTickCount() && log->size())
         renderLine(sfc, &info, Point2I(0,0), (*log)[log->size() - 1]);
   }
}

void SimGuiConsolePlugin::renderLine(GFXSurface  *sfc, 
                                     GFXFontInfo *info, 
                                     Point2I     &point,
                                     const char  *lpszBuf,
                                     char        *lpszPos,
                                     char        *lpszHiStart,
                                     char        *lpszHiEnd)
{
   int iHighColor = 255; // Should eventually be based upon the color of the text
   int start, hi_start, hi_end;
   int lines = console->getIntVariable(CONSOLE_CURSORLINES);
   int iMaxLines = stricmp(console->getVariable(
      CONSOLE_CURSORTYPE), "horizontal") ? info->fontWidth : info->fontHeight;

   // Set cursor thickness
   if (lines <= 0)
   {
      lines = 1;
   }
   else if (lines > iMaxLines)
   {
      lines = iMaxLines;
   }

   // If the text just fits, go ahead and draw it
   if (rsFont->getStrWidth(lpszBuf) <= (sfc->getWidth() - 5))
   {
      // Get some info for drawing
      start = hi_start = hi_end = rsFont->getStrWidth(lpszBuf);

      // To draw the highlight rectangle
      if (lpszHiStart && lpszHiEnd)
      {
         hi_start -= rsFont->getStrWidth(lpszHiStart);
         hi_end   -= rsFont->getStrWidth(lpszHiEnd);

         sfc->drawRect2d_f(&RectI(hi_start, 
            point.y + 5, hi_end, point.y + info->baseline + 5), iHighColor);
      }

      // Draw the text
      sfc->drawText_p(rsFont, &point, lpszBuf);

      // If the cursor is not at the end of the line, reposition it
      if (lpszPos)
      {
         // Get starting position
         start -= rsFont->getStrWidth(lpszPos);

         // Draw the cursor rectangle
         if (stricmp(console->getVariable(CONSOLE_CURSORTYPE), "horizontal"))
         {
            sfc->drawRect2d_f(&RectI(start, 
               point.y + 5, start + lines, point.y + info->baseline + 5), info->foreColor);
         }
         else
         {
            sfc->drawRect2d_f(&RectI(start + 1, point.y + info->
               baseline - lines + 2, start + 11, point.y + info->baseline + 2), info->foreColor);
         }
      }

      point.y -= info->fontHeight + info->spacing;
   }
   // Otherwise we have to break it up into chunks
   else
   {
      int  iAbsolutePos = 0;
      int  iHiStartLine = 0, iHiEndLine = 0;
      int  iHiStartPos = 0, iHiEndPos = 0, iCursorPos = 0, iCursorLine = 0;
      int  iCurLine = 0, iCurChar = 0;
      char szBuf[MAX_LINES][MAX_CMD + MAX_PROMPT];

      memset(szBuf[0], '\0', MAX_CMD + MAX_PROMPT);

      // Get the number of the character on which these
      // special things reside
      if (lpszHiStart && lpszHiEnd)
      {
         iHiStartPos = strlen(lpszBuf) - strlen(lpszHiStart);
         iHiEndPos   = strlen(lpszBuf) - strlen(lpszHiEnd);
      }
      if (lpszPos)
      {
         iCursorPos  = strlen(lpszBuf) - strlen(lpszPos);
      }

      while (*lpszBuf != '\0' && iCurLine < MAX_LINES && iAbsolutePos < MAX_CMD + MAX_PROMPT)
      {
         // Copy the current character
         szBuf[iCurLine][iCurChar] = *lpszBuf;

         // If we've gone over the edge of our printable area, begin
         // a new line
         if (rsFont->getStrWidth(szBuf[iCurLine]) >= (sfc->getWidth() - 4))
         {
            szBuf[iCurLine][iCurChar] = '\0';
            iCurLine ++;

            // If the cursor is not on this line, subtract this line's
            // length from its total offset
            if (lpszPos && (iCursorPos > iCurChar))
            {
               iCursorLine ++;
               iCursorPos -= iCurChar;
            }

            // Do the same for highlighting
            if (lpszHiStart && (iHiStartPos > iCurChar))
            {
               iHiStartLine ++;
               iHiStartPos -= iCurChar;
            }

            if (lpszHiEnd && (iHiEndPos > iCurChar))
            {
               iHiEndLine ++;
               iHiEndPos -= iCurChar;
            }

            iCurChar = 0;
            memset(szBuf[iCurLine], '\0', MAX_CMD + MAX_PROMPT);

         }
         // Otherwise go to the next character
         else
         {
            iCurChar ++;
            lpszBuf  ++;
            iAbsolutePos ++;
         }
      }

      // And draw out our text lines
      while (iCurLine >= 0)
      {
         if (lpszHiEnd && (iHiEndLine == iCurLine))
         {
            char c = szBuf[iCurLine][iHiEndPos];
            szBuf[iCurLine][iHiEndPos] = '\0';
            hi_end = point.x + rsFont->getStrWidth(szBuf[iCurLine]);
            szBuf[iCurLine][iHiEndPos] = c;

            if (iHiEndLine > iHiStartLine)
            {
               iHiEndLine --;
               iHiEndPos = strlen(szBuf[iHiEndLine]);

               // draw from beginning of line to end
               sfc->drawRect2d_f(&RectI(point.x, point.y + 5,
                  hi_end, point.y + info->baseline + 5), iHighColor);
            }
            else
            {
               char c = szBuf[iCurLine][iHiStartPos];
               szBuf[iCurLine][iHiStartPos] = '\0';
               hi_start = point.x + rsFont->getStrWidth(szBuf[iCurLine]);
               szBuf[iCurLine][iHiStartPos] = c;

               // draw from start to end
               sfc->drawRect2d_f(&RectI(hi_start, point.y + 5,
                  hi_end, point.y + info->baseline + 5), iHighColor);
            }
         }

         sfc->drawText_p(rsFont, &point, szBuf[iCurLine]);

         if (lpszPos && (iCursorLine == iCurLine))
         {
            // Get drawpoint for cursor
            char c = szBuf[iCurLine][iCursorPos];
            szBuf[iCurLine][iCursorPos] = '\0';
            start = point.x + rsFont->getStrWidth(szBuf[iCurLine]);
            szBuf[iCurLine][iCursorPos] = c;

            // Draw the cursor rectangle
            if (stricmp(console->getVariable(CONSOLE_CURSORTYPE), "horizontal"))
            {
               sfc->drawRect2d_f(&RectI(start, 
                  point.y + 5, start + lines, point.y + info->baseline + 5), info->foreColor);
            }
            else
            {
               sfc->drawRect2d_f(&RectI(start + 1, point.y + info->
                  baseline - lines + 2, start + 11, point.y + info->baseline + 2), info->foreColor);
            }
         }

         point.y -= info->fontHeight + info->spacing;
         iCurLine --;
      }
   }
}

enum {
   ConsoleFontForeRGB = 0,
   ConsoleFontBackRGB,
   ConsoleLastLineTimeout,
   ConsoleEnable,
};

void SimGuiConsolePlugin::init()
{
   Console->addCommand (ConsoleEnable,          "Console::enable", this, 1);
   Console->addVariable(ConsoleFontForeRGB,     "Console::ForeRGB", this, "255 255 255");
   Console->addVariable(ConsoleFontBackRGB,     "Console::BackRGB", this, "0 0 0");
   Console->addVariable(ConsoleLastLineTimeout, "Console::LastLineTimeout", CMDConsole::Int, (void *) &lastLineTimeout);
   Console->addConsumer(this);
}

const char *SimGuiConsolePlugin::consoleCallback(CMDConsole *, int id, int argc, const char *argv[])
{
   PALETTEENTRY p;
   int          r, g, b;

   switch(id)
   {
      case ConsoleEnable :
         if (argc == 2)
         {
            if (stricmp(argv[1], "true") == 0)
            {
               fEnabled = true;
               return ("True");
            }
            else if (stricmp(argv[1], "false") == 0)
            {
               fEnabled = false;
               return ("True");
            }
         }
         Console->printf("Console::enable(<true/false>);");
         return ("False");

      case ConsoleFontForeRGB:
      case ConsoleFontBackRGB:
         if (argc == 2)
         {
            sscanf(argv[1], "%d %d %d", &r, &g, &b);

            p.peRed   = (BYTE) r;
            p.peGreen = (BYTE) g;
            p.peBlue  = (BYTE) b;
            p.peFlags = 0;

            if(id == ConsoleFontForeRGB)
            {
               pe_fore = p;
            }
            else
            {
               pe_back = p;
            }

            // Mapping fore and back to the same color, or mapping
            // the back to a higher RGB than the fore, causes 
            // text "blocking" -- so we have to prevent that.  If
            // this doesn't work, I may take the damn back color
            // change var out
            if (pe_fore.peRed <= pe_back.peRed)
            {
               if (pe_fore.peRed > 128)
               {
                  pe_back.peRed = 0;
               }
               else
               {
                  pe_fore.peRed = 255;
               }
            }

            if (pe_fore.peGreen <= pe_back.peGreen)
            {
               if (pe_fore.peGreen > 128)
               {
                  pe_back.peGreen = 0;
               }
               else
               {
                  pe_fore.peGreen = 255;
               }
            }

            if (pe_fore.peBlue <= pe_back.peBlue)
            {
               if (pe_fore.peBlue > 128)
               {
                  pe_back.peBlue = 0;
               }
               else
               {
                  pe_fore.peBlue = 255;
               }
            }

            colorChanged = true;
            break;
         }
   }
   return NULL;
}

void SimGuiConsolePlugin::consoleCallback(CMDConsole *, const char *)
{
   lastLineTime = GetTickCount();
}

void SimGuiConsolePlugin::endFrame()
{
   if(pEvent)
   {
      if (timer.getElapsed() >= iRepeatsPerSec)
      {
         iRepeatsPerSec = console->getIntVariable(CONSOLE_REPEATSPERSEC);

         if (iRepeatsPerSec <= 0 || iRepeatsPerSec > MAX_REPEATSPERSEC)
         {
            iRepeatsPerSec = MAX_REPEATSPERSEC / 2;
         }

         iRepeatsPerSec = 1.0f / iRepeatsPerSec;

         timer.reset();
         handleInput();
      }
   }
   return;
}
