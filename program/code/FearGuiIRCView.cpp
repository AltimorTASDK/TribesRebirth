#include <ircclient.h>
#include <fearguiircview.h>
#include <fear.strings.h>
#include <fearguiircactivetextformat.h>
#include <console.h>

namespace FearGui
{
FGIRCView::FGIRCView()
: fEnabled(true), iMouseTag(0)
{
   active = true;
}

FGIRCView::~FGIRCView()
{
}

Int32 FGIRCView::getMouseCursorTag()
{
   if (Console->getBoolVariable("$ConnectedToServer")) return 0;
   else return (iMouseTag);
}

void FGIRCView::onMouseMove(const SimGui::Event &event)
{
   Point2I loc_extent;
   Point2I loc_offset = getPosition();
   Point2I mouse = globalToLocalCoord(event.ptMouse);

   if (inHitArea(mouse))
   {
      iMouseTag    = 0;
      loc_offset.y = 0;

      for (const Message *current = messages.getHead();
           current; current = current->next)
      {
         loc_extent    = current->msg->getExtent();
         loc_extent.y -= 2;

         if (mouse.x >= loc_offset.x && mouse.y >= loc_offset.y &&
             mouse.x <= loc_offset.x + loc_extent.x && mouse.y <= loc_offset.y + loc_extent.y)
         {
            current->msg->setStateOver(true);
         
            if (current->msg->isClickable())
            {
               iMouseTag = IDBMP_CURSOR_HAND;
            }
            setHelpText(current->msg->getHelpText(0.0f));
         }
         else
         {
            current->msg->setStateOver(false);
         }

         loc_offset.y += loc_extent.y;
      }
   }
}

void FGIRCView::onMouseDown(const SimGui::Event &event)
{
   Point2I loc_extent;
   Point2I loc_offset = getPosition();
   Point2I mouse = globalToLocalCoord(event.ptMouse);
   loc_offset.y = 0;
   
   if (Console->getBoolVariable("$ConnectedToServer")) return;
   
   if (inHitArea(mouse))
   {
      for (const Message *current = messages.getHead();
           current; current = current->next)
      {
         loc_extent    = current->msg->getExtent();
         loc_extent.y -= 2;

         // Can only MouseDown on one format ctrl
         if (mouse.x >= loc_offset.x && mouse.y >= loc_offset.y &&
             mouse.x <= loc_offset.x + loc_extent.x && mouse.y <= loc_offset.y + loc_extent.y)
         {
            current->msg->onMouseDown(event);
            break;
         }

         loc_offset.y += loc_extent.y;
      }
   }
}

void FGIRCView::onMouseLeave(const SimGui::Event &)
{
   for (const Message *current = messages.getHead();
        current; current = current->next)
   {
      current->msg->setStateOver(false);
   }
}

void FGIRCView::enable(bool fEnable)
{
   fEnabled = fEnable;
}

void FGIRCView::parseEmbeddedCmds(FGIRCActiveTextFormat *msg, const char *lpszMsg)
{
   int   iDots;
   char  szBuf[512];
   bool  fPasswordRequired;
   char *lpszStart, *lpszEnd, *lpszTok, *lpszTmp, *lpszPort;
   char *lpszDup = strupr(strnew(lpszMsg));

   bool connectedToServer = Console->getBoolVariable("$ConnectedToServer");
   
   // Hypertext embedded commands
   if ((lpszStart = strstr(lpszDup, lpszTok = "HTTP://")) != NULL ||
       (lpszStart = strstr(lpszDup, lpszTok = "WWW."))    != NULL)
   {
      lpszEnd = lpszStart;

      while (*lpszEnd != ' ' && *lpszEnd != '\t' && *lpszEnd != '\0')
      {
         lpszEnd ++;
      }

      // Make sure there's more than the prefix on the line
      if ((lpszStart + strlen(lpszTok)) != lpszEnd)
      {
         
         // Get original format of string
         strcpy(lpszDup, lpszMsg);

         // Truncate at end of hypertext
         *lpszEnd = '\0';

         //if there is no "http://" prior to "www." insert it...
         if (! strnicmp(lpszStart, "http://", 7))
         {
            sprintf(szBuf, "HTMLOpenAndGoWin(MainWindow, \"%s\");", lpszStart);
         }
         else
         {
            sprintf(szBuf, "HTMLOpenAndGoWin(MainWindow, \"http://%s\");", lpszStart);
         }
         msg->setClickAction(szBuf);

         sprintf(szBuf, "Click to go to %s%s", 
            strnicmp(lpszStart, "http://", 7) ? "http://" : "", lpszStart);
         msg->setHelpText(szBuf);
      }
   }

   // Join-server embedded commands
   else if ((lpszStart = strstr(lpszDup, lpszTok = "[IP:")) != NULL)
   {
      // Skip label
      lpszStart += strlen(lpszTok);
      lpszEnd    = lpszStart;

      // Seek end delimiter
      while (*lpszEnd != ']' && *lpszEnd != '\0' && *lpszEnd != ' ' && *lpszEnd != '\t')
      {
         lpszEnd ++;
      }

      // Make sure there's more than the delimiters, and that the delimiters
      // are correct
      if (lpszStart != lpszEnd && *lpszEnd == ']')
      {
         // First validate the IP address
         for (iDots = 0, lpszTok = lpszStart; lpszTok < lpszEnd; lpszTok ++, iDots ++)
         {
            lpszTmp = lpszTok;

            // Find the next dot
            if ((lpszTok = strchr(lpszTok, '.')) != NULL)
            {
               *lpszTok = '\0';

               // Check to be sure that this is a valid sub-IP
               if (atoi(lpszTmp) || *lpszTmp == '0')
               {
                  continue;
               }

               // If this was an invalid sub-IP, whole IP is invalid
               iDots = 0;
            }
            break;
         }

         // If we have three dots, IP is valid, now check port
         if (iDots == 3)
         {
            // Get rid of interim NULs
            strcpy(lpszDup, lpszMsg);

            // Terminate string at token and find port delimiter
            *lpszEnd = '\0';

            // See if a port was specified
            if ((lpszPort = strchr(lpszStart, ':')) != NULL)
            {
               *(lpszPort ++) = '\0';

               // Check for 'password may be required' boolean
               if ((lpszTok = strchr(lpszPort, ':')) != NULL)
               {
                  *(lpszTok ++) = '\0';

                  // Make sure port is valid
                  if (atoi(lpszPort))
                  {
                     // Make sure this isn't the end of the string ...
                     if (*lpszTok != '\0')
                     {
                        // And check for the 'password is required' boolean
                        if (*(++ lpszTok) != '\0')
                        {
                           fPasswordRequired = (*lpszTok == '1');

                           if (connectedToServer)
                           {
                              sprintf(szBuf, "You must first disconnect from your current game before you can join server %s", lpszStart);
                           }
                           else
                           {
                              sprintf(szBuf, "Click to join server %s", lpszStart);
                           }
                           msg->setHelpText(szBuf);

                           if (fPasswordRequired)
                           {
                              sprintf(szBuf, 
                                 "$Server::Address = \"IP:%s:%s\";"
                                 "GuiPushDialog(MainWindow, \"gui\\\\password.gui\");",
                                 lpszStart, lpszPort, 
                                 SimTagDictionary::getString(
                                    manager, IDSTR_PASSWORD_REQUIRED));
                           }
                           else
                           {
                              sprintf(szBuf, 
                                 "$Server::Address = \"IP:%s:%s\";"
                                 "JoinGame();", 
                                 lpszStart, lpszPort);
                           }
                           msg->setClickAction(szBuf);
                        }
                     }
                  }
               }
            }
         }
      }
   }

   delete [] lpszDup;
}

void FGIRCView::addMessage(const char *lpszMsg)
{
   Message *message = new Message;

   message->msg = new FGIRCActiveTextFormat;

   // Check for "active" text
   parseEmbeddedCmds(message->msg, lpszMsg);

   // Set the fonts
   message->msg->setFont(0, fGhosted ? IDFNT_9_DISABLED : IDFNT_9_STANDARD);
   message->msg->setFont(1, fGhosted ? "sf_grey100_9b.pft" : "sf_yellow200_9b.pft");
   message->msg->setFont(2, fGhosted ? "sf_grey100_9b.pft" : "sf_yellow200_9b.pft");
   message->msg->setFont(3, fGhosted ? IDFNT_9_DISABLED : IDFNT_9_STATIC);
   message->msg->setFont(4, fGhosted ? "sf_grey100_9b.pft" : "sf_yellow_9b.pft");
   message->msg->setFont(5, fGhosted ? IDFNT_9_DISABLED : IDFNT_9_STATIC);
   message->msg->setFont(6, fGhosted ? IDFNT_9_DISABLED : IDFNT_9_SELECTED);

   // Format the string and size the control
   message->msg->formatControlString(lpszMsg, extent.x - 15);
   message->msg->resize(Point2I(0, 0), Point2I(extent.x, message->msg->getHeight()));

   // Add it to the vector and get our new needed size
   messages.push(message);

   // Step ahead
   extent.y += message->msg->getHeight() - 2;

   // If this fills up our buffer, pop the front and resize 
   // the control
   while (messages.size() > objIRCClient->getHistorySize())
   {
      extent.y -= messages.getHead()->msg->getHeight() - 2;

      messages.pop();   
   }

   // Scroll to bottom
   if (extent.y >= parent->getExtent().y)
   {
      resize(getPosition() - 
         (message->msg->getHeight() - 2), Point2I(extent.x, extent.y));
   }
}

void FGIRCView::clear()
{
   // Delete everything
   while (messages.size())
   {
      messages.pop();
   }

   // Resize the control
   resize(getPosition(), Point2I(extent.x, 5));
}

void FGIRCView::onWake()
{
   objIRCClient = IRCClient::find(manager);
   AssertFatal(objIRCClient, "FGIRCView: cannot locate IRC Client");

   lpmLastRead     = NULL;
   dwLastChannelID = -1;
   
   fGhosted = FALSE;
   const Message *current;
   for (current = messages.getHead(); current; current = current->next)
   {
      current->msg->setFont(0, IDFNT_9_STANDARD);
      current->msg->setFont(1, "sf_yellow200_9b.pft");
      current->msg->setFont(2, "sf_yellow200_9b.pft");
      current->msg->setFont(3, IDFNT_9_STATIC);
      current->msg->setFont(4, "sf_yellow_9b.pft");
      current->msg->setFont(5, IDFNT_9_STATIC);
      current->msg->setFont(6, IDFNT_9_SELECTED);
   }

   resize(getPosition(), Point2I(extent.x, 5));
}

void FGIRCView::onRemove()
{
   clear();
   Parent::onRemove();
}

void FGIRCView::onPreRender(void)
{
   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(1);
   if (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG)))
   {
      ghosted = TRUE;
   }
   
   if (ghosted != fGhosted)
   {
      fGhosted = ghosted;
      
      const Message *current;
      for (current = messages.getHead(); current; current = current->next)
      {
         current->msg->setFont(0, fGhosted ? IDFNT_9_DISABLED : IDFNT_9_STANDARD);
         current->msg->setFont(1, fGhosted ? "sf_grey100_9b.pft" : "sf_yellow200_9b.pft");
         current->msg->setFont(2, fGhosted ? "sf_grey100_9b.pft" : "sf_yellow200_9b.pft");
         current->msg->setFont(3, fGhosted ? IDFNT_9_DISABLED : IDFNT_9_STATIC);
         current->msg->setFont(4, fGhosted ? "sf_grey100_9b.pft" : "sf_yellow_9b.pft");
         current->msg->setFont(5, fGhosted ? IDFNT_9_DISABLED : IDFNT_9_STATIC);
         current->msg->setFont(6, fGhosted ? IDFNT_9_DISABLED : IDFNT_9_SELECTED);
      }
   }
   
   if (parent && extent.x != parent->extent.x)
   {
      extent.x = parent->extent.x;
      int newHeight = 0;
      const Message *current;
      for (current = messages.getHead(); current; current = current->next)
      {
         current->msg->formatControlString(NULL, parent->extent.x - 15, TRUE);
         current->msg->resize(Point2I(0, 0), Point2I(parent->extent.x, current->msg->getHeight()));
         newHeight += current->msg->getHeight();
      }
      extent.y = newHeight;
   }

   Point2I newExtent = extent;
   
   // Get the current channel
   IRCClient::Channel *channel = objIRCClient->findChannel(NULL);

   // Handle new messages
   if (channel)
   {
      // If we changed channels, or for some reason don't have a
      // last-read pointer, we need to update the entire display
      if (channel->id != dwLastChannelID || lpmLastRead == NULL)
      {
         // Clear the display
         clear();

         // We've changed channels
         dwLastChannelID = channel->id;

         // Read and dump the entire queue
         for (lpmLastRead = channel->messages.getHead();
              lpmLastRead; lpmLastRead = lpmLastRead->next)
         {
            addMessage(lpmLastRead->text);
         } 

         // Now the last-read message is the last message
         lpmLastRead = channel->messages.getTail();

         // Re-set the new message flag
         channel->flags &= ~IRCClient::CHANNEL_NEWMESSAGE;
      }
      // Read any new messages
      else if (channel->newMessages())
      {
         // Don't repeat the last read one, cycle until we 
         // run out of messages
         for (lpmLastRead = lpmLastRead->next; 
              lpmLastRead; lpmLastRead = lpmLastRead->next)
         {
            addMessage(lpmLastRead->text);
         }

         // Now the last-read message is the last message
         lpmLastRead = channel->messages.getTail();

         // Re-set the new message flag
         channel->flags &= ~IRCClient::CHANNEL_NEWMESSAGE;
      }
   }
}

void FGIRCView::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   int            iHeight;
   const Message *current;

   Parent::onRender(sfc, offset, updateRect);

   offset.x += 2;

   // Skip the messages we can't see
   for (current = messages.getHead(); current; current = current->next)
   {
      iHeight = current->msg->getHeight() - 2;

      // If any part of it is visible, we need to draw it
      if (offset.y >= (updateRect.fMin.y - iHeight))
      {
         break;
      }
      offset.y += iHeight;
   }

   // Render those we can see
   for (; current && offset.y < updateRect.fMax.y; current = current->next)
   {
      current->msg->onRender(sfc, offset, updateRect);
      offset.y += current->msg->getHeight() - 2;
   }
}

IMPLEMENT_PERSISTENT_TAG(FGIRCView, FOURCC('F','G','i','x'));
};