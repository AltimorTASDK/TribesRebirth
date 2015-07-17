#include "simGuiDelegate.h"
#include "simResource.h"
#include "console.h"
#include "fear.strings.h"
#include "esf.strings.h"
#include "ircClient.h"
#include "fearGuiIRCTextEdit.h"
#include "fearguiircnicks.h"
#include "darkstar.strings.h"
#include "fearguiunivbutton.h"
#include "fearGuiHelpCtrl.h"

namespace SimGui
{
extern Control *findControl(const char *name);

using namespace FearGui;
class IRCChatDelegate : public Delegate
{

private:
   typedef Delegate Parent;
   Control        *curGui;
   IRCClient      *mIRCClient;
   DWORD           dwPendingMsg;
   FGIRCNicknames *people;
   
public:
   bool onAdd(void);
   void onNewContentControl(Control *oldGui, Control *newGui);
   DECLARE_PERSISTENT(IRCChatDelegate);
   IRCChatDelegate() { curGui = NULL; }
   
   bool onSimMessageEvent(const SimMessageEvent *);
   bool processEvent(const SimEvent *event);
   
   void verifyConnection(void);
   
   DWORD onMessage  (SimObject *, DWORD);
   Int32 onDialogPop(Control *, Int32);
};


bool IRCChatDelegate::onAdd()
{
   if (! Parent::onAdd()) return FALSE;
   
   mIRCClient = IRCClient::find(manager);
   AssertFatal(mIRCClient, "IRCChatDelegate: unable to locate IRC client");
   mIRCClient->setNotifyObject(this);
   
   return TRUE;
}

void IRCChatDelegate::onNewContentControl(Control *, Control *newGui)
{
   SimGui::Canvas *root;

   if (newGui && newGui->getTag() == IDGUI_IRC_CHAT) 
   {
      curGui = newGui;
      root   = curGui->getCanvas();
      AssertFatal(root, "Unable to get the root");

      dwPendingMsg = 0;
      people = static_cast<FGIRCNicknames *>
         (newGui->findControlWithTag(IDCTG_IRC_PEOPLE_LIST));

      AssertFatal(people, "Delegate: unable to locate people list");
      if (mIRCClient->getState() == IDIRC_DISCONNECTED)
      {
         root->makeFirstResponder(NULL);
         root->popDialogControl();
         Console->executef(3, "GuiPushDialog", "MainWindow", "gui\\IRCDisconnected.gui");
      }
      
      verifyConnection();
   }
}

void IRCChatDelegate::verifyConnection(void)
{
   if (mIRCClient && curGui)
   {
      bool tempActive = (mIRCClient->getState() == IDIRC_CONNECTED);
/*      
      bool joinRoom = (stricmp(Console->getVariable("IRCJoinedRoom"), "TRUE"));
      if (tempActive && joinRoom)
      {
         //now try to join the default room if it exists
         Console->setVariable("$IRCJoinedRoom", "TRUE");
         const char *roomName = Console->getVariable("IRC::room");
         if (roomName[0])
         {
            mIRCClient->send(avar("/join %s", roomName));
         }
      }
*/      
      SimGui::ActiveCtrl *ctrl;
      ctrl = dynamic_cast<SimGui::ActiveCtrl*>(curGui->findControlWithTag(IDIRC_MENUOPT_LEAVE));
      if (ctrl)
      {
         ctrl->active = tempActive;
         ctrl->setUpdate();
      }
      ctrl = dynamic_cast<SimGui::ActiveCtrl*>(curGui->findControlWithTag(IDIRC_BTN_JOIN));
      if (ctrl)
      {
         ctrl->active = tempActive;
         ctrl->setUpdate();
      }
      ctrl = dynamic_cast<SimGui::ActiveCtrl*>(curGui->findControlWithTag(IDCTG_IRC_COMMAND_LINE));
      if (ctrl)
      {
         ctrl->active = tempActive;
         ctrl->setUpdate();
      }
   }
}

bool IRCChatDelegate::onSimMessageEvent(const SimMessageEvent *event)
{
   char szBuf[256];

   SimGui::Canvas *root;
   if (curGui) root = curGui->getCanvas();
   else return TRUE;
   AssertFatal(root, "Unable to get the root");
   
   verifyConnection();
   
   switch (event->message)
   {
      case IDIRC_ERR_SET_NICK    :
      case IDIRC_ERR_NICK        :
      {
         root->makeFirstResponder(NULL);
         root->popDialogControl();
         Console->executef(3, "GuiPushDialog", "MainWindow", "gui\\IRCNickInvalid.gui");
         break;
      }
         
      case IDIRC_ERR_NICK_IN_USE :
      {
         root->makeFirstResponder(NULL);
         root->popDialogControl();
         Console->executef(3, "GuiPushDialog", "MainWindow", "gui\\IRCNickInUse.gui");
         break;
      }
      
      case IDIRC_ERR_CONNECT     :
      case IDIRC_ERR_HOSTNAME    :
      case IDIRC_ERR_RECONNECT   :
      case IDIRC_ERR_DROPPED     :
      case IDIRC_ERR_TIMEOUT     :
      {
         root->makeFirstResponder(NULL);
         root->popDialogControl();
         Console->executef(3, "GuiPushDialog", "MainWindow", "gui\\IRCDisconnected.gui");
         break;
      }

      case IDIRC_INVITED :
      {
         sprintf(szBuf, "%s has invited you to join room %s",
            Console->getVariable("$IRC::InvitePerson"),
            Console->getVariable("$IRC::InviteChannel"));

         Console->setVariable("$IRCMsg::DlgBanner", szBuf);
         Console->executef(3, "GuiPushDialog", "MainWindow", "gui\\IRCInvited.gui");
         break;
      }
   }
   return TRUE;
}

bool  IRCChatDelegate::processEvent(const SimEvent *event)
{
   switch (event->type)
   {
      onEvent(SimMessageEvent);
   }
   return (Parent::processEvent(event));
}

DWORD IRCChatDelegate::onMessage(SimObject *sender, DWORD msg)
{
   char            szBuf[256];
   SimGui::Canvas *root;

   if (curGui) 
   {
      root = curGui->getCanvas();
   }
   else 
   {
      return Parent::onMessage(sender, msg);
   }

   AssertFatal(root, "Unable to get the root");

   switch (msg)
   {
      case IDCTG_IRC_CONNECT_DONE :
      {
         SimGui::TextEdit *ctrl = static_cast<SimGui::TextEdit*>
            (root->getTopDialog()->findControlWithTag(IDCTG_IRC_NICK_TEXT));

         if (ctrl)
         {
            ctrl->getText(szBuf);

            if (szBuf[0] != '\0')
            {
               const char *realName    = Console->getVariable("PCFG::RealName");
               const char *ircRealName = Console->getVariable("IRC::realName");

               if (realName) 
               {
                  Console->setVariable("IRC::RealName", realName);
               }
               else if ((!ircRealName) || (!ircRealName[0]))
               {
                  Console->setVariable("IRC::RealName", szBuf);
               }

               mIRCClient->connect();
            }
         }
      
         root->makeFirstResponder(NULL);
         root->popDialogControl();
      
         return (-1);
      }

      case IDCTG_IRC_COMMAND_LINE :
      {
         // Find the irc client, and send the command
         FearGui::FGIRCTextEdit *textCtrl = 
            static_cast<FearGui::FGIRCTextEdit*>(sender);

         if (textCtrl && textCtrl->isCommandReady())
         {
            textCtrl->getIRCText(szBuf);
            
            //here we filter the command line for common shortcuts
            if (szBuf[0] == '/')
            {
               char temp[256], *tempPtr = &temp[0];
               char *bufPtr = &szBuf[0];
               while (*bufPtr != '\0' && *bufPtr != ' ')
               {
                  *tempPtr++ = *bufPtr++;
               } 
               *tempPtr = '\0';
               
               //now check for shortcuts
               if ((! stricmp(temp, "/msg")) || (! stricmp(temp, "/message")) ||
                   (! stricmp(temp, "/m")) || (! stricmp(temp, "/w")) || (! stricmp(temp, "/whisper")))
               {
                  //find the addressee
                  tempPtr = &temp[0];
                  while (*bufPtr == ' ') bufPtr++;
                  while (*bufPtr != '\0' && *bufPtr != ' ')
                  {
                     *tempPtr++ = *bufPtr++;
                  } 
                  *tempPtr = '\0';
                  if (*bufPtr == ' ') bufPtr++;
                  
                  //addressee is now in temp, send the rest as the message
                  if (*bufPtr != '\0')
                  {
                     mIRCClient->send(bufPtr, temp);
                  }
               }
               else if (szBuf[0])
               {
                  mIRCClient->send(szBuf);
               }
            }
            else if (szBuf[0])
            {
               mIRCClient->send(szBuf);
            }
         }
         break;
      }

      case IDIRC_BTN_JOIN :
      {
         Console->executef(3, "GuiPushDialog", "MainWindow", "gui\\ircchannels.gui");
         break;
      }

      case IDIRC_MENUOPT_LEAVE :
      {
         IRCClient::Channel *channel = mIRCClient->findChannel(NULL);

         if (channel)
         {
            sprintf(szBuf, "/part %s", channel->name);
            mIRCClient->send(szBuf);
         }
         return (0);
      }

      case IDIRC_MENUOPT_INVITE :
      {
         Console->setVariable("$IRCMsg::InvitePerson", "");
         Console->setVariable("$IRCMsg::DlgBanner", 
            "Enter the nick name of the person you would like to invite");
         Console->executef(3, "GuiPushDialog", "MainWindow", "gui\\ircinvite.gui");
         return (0);
      }

      case IDIRC_MENUOPT_CHANNEL_PROPERTIES :
      {
         // Need to create dialog for this
         Console->executef(3, "GuiPushDialog", "MainWindow", "gui\\ircproperties.gui");
         return (0);
      }

      case IDIRC_MENUOPT_AWAY :
      {
         IRCClient::Person *person = mIRCClient->getMe();

         if (person)
         {
            if (person->flags & IRCClient::PERSON_AWAY)
            {
               mIRCClient->send("/away");
            }
            else
            {
               if (strlen(Console->getVariable("$IRCMsg::AwayMsg")) == 0)
               {
                  Console->setVariable("$IRCMsg::AwayMsg", "Playing TRIBES");
               }
               Console->executef(3, "GuiPushDialog", "MainWindow", "gui\\ircaway.gui");
            }
         }
         return (0);
      }

      case IDIRC_MENUOPT_KICK :
      {
         const char            *lpszNick;
         IRCClient::Channel    *channel;
         IRCClient::PersonInfo *person;

         channel  = mIRCClient->findChannel(NULL);
         lpszNick = people->getSelectedText();

         if (lpszNick && channel)
         {
            person = channel->findPerson(lpszNick);

            if (person)
            {
               sprintf(szBuf, 
                  "Why are you kicking %s from the channel?",
                  lpszNick);

               Console->setVariable("$IRCMsg::KickPerson", lpszNick);
               Console->setVariable("$IRCMsg::KickBanner", szBuf);
               Console->executef(3, "GuiPushDialog", "MainWindow", "gui\\irckick.gui");
            }
         }
         return (0);
      }

      case IDIRC_MENUOPT_BAN :
      {
         const char            *lpszNick;
         IRCClient::Channel    *channel;
         IRCClient::PersonInfo *person;

         channel  = mIRCClient->findChannel(NULL);
         lpszNick = people->getSelectedText();

         if (lpszNick && channel)
         {
            person = channel->findPerson(lpszNick);

            if (person)
            {
               Console->setVariable("$IRCMsg::BanChannel", channel->name);
               Console->setVariable("$IRCMsg::BanUser",    lpszNick);
               Console->executef(3, "GuiPushDialog", "MainWindow", "gui\\ircban.gui");
            }
         }
         return (0);
      }

      case IDIRC_MENUOPT_PING_USER :
      {
         const char *lpszNick = people->getSelectedText();

         if (lpszNick)
         {
            sprintf(szBuf, "/PING %s", lpszNick);
            mIRCClient->send(szBuf);
         }
         return (0);
      }

      case IDIRC_MENUOPT_PRIVATE_CHAT :
      {
         const char *lpszNick = people->getSelectedText();

         if (lpszNick)
         {
            mIRCClient->findChannel(lpszNick, true);

            sprintf(szBuf, "/JOIN %s", lpszNick);
            mIRCClient->send(szBuf);
         }
         return (0);
      }

      case IDIRC_MENUOPT_WHOIS_USER :
      {
         const char *lpszNick = people->getSelectedText();

         if (lpszNick)
         {
            sprintf(szBuf, "/WHOIS %s", lpszNick);
            mIRCClient->send(szBuf);
         }
         return (0);
      }

      case IDIRC_MENUOPT_IGNORE :
      {
         const char            *lpszNick;
         IRCClient::Channel    *channel;
         IRCClient::PersonInfo *person;

         channel  = mIRCClient->findChannel(NULL);
         lpszNick = people->getSelectedText();

         if (lpszNick && channel)
         {
            person = channel->findPerson(lpszNick);

            if (person)
            {
               mIRCClient->ignore(lpszNick, 
                  !(bool)(person->person->flags & IRCClient::PERSON_IGNORE));
            }
         }
         return (0);
      }

      case IDIRC_MENUOPT_OPER :
      {
         const char *lpszNick;

         lpszNick = people->getSelectedText();

         if (lpszNick)
         {
            mIRCClient->setOperator(lpszNick);
         }
         return (0);
      }

      case IDIRC_MENUOPT_SPEC :
      {
         const char *lpszNick;

         lpszNick = people->getSelectedText();

         if (lpszNick)
         {
            mIRCClient->setSpectator(lpszNick);
         }
         return (0);
      }

      case IDIRC_MENUOPT_SPKR :
      {
         const char *lpszNick;

         lpszNick = people->getSelectedText();

         if (lpszNick)
         {
            mIRCClient->setSpeaker(lpszNick);
         }
         return (0);
      }

      case IDCTG_SERVER_JOIN_PASSWORD :
      {
         // Kill the dialog
         root->makeFirstResponder(NULL);
         root->popDialogControl();
      
         // Join the game...
         Console->evaluate("JoinGame();", FALSE);
         return (0);
      }
   }
   
   return Parent::onMessage(sender, msg);
}

Int32 IRCChatDelegate::onDialogPop(Control *dialog, Int32 iMsg)
{
   char szBuf[256];

   switch (dialog->getTag())
   {
      case IDDLG_IRC_AWAY :
      {
         IRCClient::Person *person = mIRCClient->getMe();

         // If they ARE away, they are returning
         if (person && person->flags & IRCClient::PERSON_AWAY)
         {
            if (iMsg == IDDLG_OK)
            {
               mIRCClient->send("/away");
            }

            if (dwPendingMsg)
            {
               onMessage(this, dwPendingMsg);
               dwPendingMsg = 0;
            }
         }
         // If they are NOT away, they are leaving
         else
         {
            if (iMsg == IDDLG_OK)
            {
               const char *lpszMsg;

               lpszMsg = Console->getVariable("$IRCMsg::AwayMsg");
               sprintf(szBuf, "/away %s", lpszMsg);
               Console->setVariable("$IRC::AwayMsg", "");
               mIRCClient->send(szBuf);
            }
         }
         break;
      }

      case IDDLG_IRC_KICK :
      {
         if (iMsg == IDDLG_OK)
         {
            const char            *lpszNick;
            const char            *lpszReason;
            IRCClient::Channel    *channel;

            channel  = mIRCClient->findChannel(NULL);
            lpszNick = Console->getVariable("$IRCMsg::KickPerson");

            if (lpszNick && channel)
            {
               lpszReason = Console->getVariable("$IRCMsg::KickReason");

               sprintf(szBuf, "/kick %s %s :%s", 
                  channel->name, lpszNick, 
                 (lpszReason && *lpszReason) ? lpszReason : "Goodbye");
               mIRCClient->send(szBuf);
            }
         }
         break;
      }

      case IDDLG_IRC_INVITE :
      {
         const char         *lpszNick;
         IRCClient::Channel *channel;

         if (iMsg == IDDLG_OK)
         {
            lpszNick = Console->getVariable("$IRCMsg::InvitePerson");
            channel  = mIRCClient->findChannel(NULL);

            if (channel && lpszNick && *lpszNick)
            {
               sprintf(szBuf, "/invite %s %s", lpszNick, channel->name);
               mIRCClient->send(szBuf);
            }
         }
         break;
      }

      case IDDLG_IRC_INVITED :
      {
         FGUniversalButton *button;
         const char        *lpszRoom, *lpszPerson;

         if (iMsg == IDDLG_OK)
         {
            lpszRoom = Console->getVariable("$IRC::InviteChannel");

            if (lpszRoom && *lpszRoom)
            {
               sprintf(szBuf, "/join %s", lpszRoom);
               mIRCClient->send(szBuf);
            }
         }

         lpszPerson = Console->getVariable("$IRC::InvitePerson");

         button = static_cast<FGUniversalButton *>
            (dialog->findControlWithTag(IDCTG_IRCDLG_INVITED_BTN_IGNORE));

         if (lpszPerson && *lpszPerson && button && button->isSet())
         {
            mIRCClient->ignore(lpszPerson, true);
         }
         break;
      }

      case IDDLG_IRC_CHANNELS :
      {
         const char *lpszChannel, *lpszKey;

         if (iMsg == IDDLG_OK)
         {
            lpszChannel = Console->getVariable("$IRCMsg::Channel");
            lpszKey     = Console->getVariable("$IRCMsg::Key");

            if (strlen(lpszChannel))
            {
               sprintf(szBuf, "/JOIN %s %s", lpszChannel, lpszKey);
               mIRCClient->send(szBuf);
            }
         }

         mIRCClient->setNotifyObject(this);
         break;
      }
   }
   
   //keep the edit field in focus
   SimGui::Control *ctrl = SimGui::findControl("IRCTextEntry");
   SimGui::Canvas *root = NULL;
   if (curGui) root = curGui->getCanvas();
   if (root && ctrl) root->makeFirstResponder(ctrl);
   else
   {
      Console->printf("Debug:  root? %s,  ctrl? %s", (root ? "TRUE" : "FALSE"), (ctrl ? "TRUE" : "FALSE"));
   }
   
   Console->printf("Debug:  onDialogPop()");
   return (0);
}

IMPLEMENT_PERSISTENT(IRCChatDelegate);

};