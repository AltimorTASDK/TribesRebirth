#include <fearguiircbancontrol.h>

#include <ircclient.h>
#include <fear.strings.h>
#include <esf.strings.h>
#include <console.h>
#include <fearguitextlist.h>
#include <fearguitextformat.h>

namespace FearGui
{
FGIRCBanControl::FGIRCBanControl()
{
}

FGIRCBanControl::~FGIRCBanControl()
{
}

void FGIRCBanControl::onWake()
{
   const char         *lpszUser;
   char                szBuf[256];
   IRCClient::Channel *channel;
   FGTextFormat       *format;

   ban   = static_cast<SimGui::ActiveCtrl *>
      (findControlWithTag(IDCTG_IRCDLG_BAN_BTN_BAN));
   unban = static_cast<SimGui::ActiveCtrl *>
      (findControlWithTag(IDCTG_IRCDLG_BAN_BTN_UNBAN));
   edit  = static_cast<SimGui::TextEdit *>  
      (findControlWithTag(IDCTG_IRCDLG_BAN_EDT_BAN));
   list  = static_cast<FGTextList *>  
      (findControlWithTag(IDCTG_IRCDLG_BAN_LBX_BAN));
   format = static_cast<FGTextFormat *>
      (findControlWithTag(IDCTG_IRCDLG_BAN_BANNER));

   AssertFatal(ban && unban && edit && list && format,
      "FGIRCBanControl: unable to locate child controls");

   edit->setText("");

   edit->active  = false;
   ban->active   = false;
   unban->active = false;
   list->active  = false;

   sprintf(szBuf, 
      "<JC>People banned from %s", Console->getVariable("$IRCMsg::BanChannel"));

   format->formatControlString(szBuf, format->getExtent().x);

   objIRCClient = IRCClient::find(manager);
   objIRCClient->setNotifyObject(this);

   channel = objIRCClient->findChannel(NULL);
   objIRCClient->requestBanList(channel->name);

   if ((lpszUser = Console->getVariable("$IRCMsg::BanUser")) != NULL)
   {
      sprintf(szBuf, "/WHO %s", lpszUser);
      objIRCClient->send(szBuf);
   }

   fGotUserInfo = false;
   fGotBanList  = false;
}

bool FGIRCBanControl::onSimMessageEvent(const SimMessageEvent *event)
{
   switch (event->message)
   {
      case IDIRC_BAN_LIST :
      {
         VectorPtr<const char *> *banned = objIRCClient->getBanList();

         for (int i = 0; i < banned->size(); i ++)
         {
            list->addEntryBack((*banned)[i]);
         }

         fGotBanList = true;

         if (fGotUserInfo)
         {
            edit->active  = true;
            ban->active   = true;
            unban->active = true;
            list->active  = true;
         }
         break;
      }

      case IDIRC_END_OF_WHO  :
      {
         IRCClient::Person *person = 
            objIRCClient->findPerson(Console->getVariable("$IRCMsg::BanUser"));

         if (person)
         {
            char szBuf[256];
            char *lpszTok;

            if ((lpszTok = strchr(person->identity, '@')) != NULL)
            {
               sprintf(szBuf, "*!*%s", lpszTok);
            }
            else
            {
               strcpy(szBuf, person->identity);
            }
            edit->setText(szBuf);
         }

         fGotUserInfo = true;

         if (fGotBanList)
         {
            edit->active  = true;
            ban->active   = true;
            unban->active = true;
            list->active  = true;
         }
         break;
      }
   }
   return (true);
}

bool FGIRCBanControl::processEvent(const SimEvent *event)
{
   switch (event->type)
   {
      onEvent(SimMessageEvent);
   }

   return (Parent::processEvent(event));
}

DWORD FGIRCBanControl::onMessage(SimObject *obj, DWORD dwMsg)
{
   switch (dwMsg)
   {
      case IDCTG_IRCDLG_BAN_LBX_BAN :
      {
         const char *lpszText;

         lpszText = list->getSelectedText();

         if (lpszText)
         {
            edit->setText(lpszText);
         }
         break;
      }

      case IDCTG_IRCDLG_BAN_BTN_BAN :
      {
         char                szBuf[256];
         char                szTwo[512];
         IRCClient::Channel *channel;

         edit->getText(szBuf);
         channel = objIRCClient->findChannel(NULL);

         if (strlen(szBuf) && channel)
         {
            if (list->findEntry(szBuf) == -1)
            {
               sprintf(szTwo, "/MODE %s +b %s", channel->name, szBuf);
               objIRCClient->send(szTwo);
               list->addEntryBack(szBuf);
            }
         }
         else
         {
            //Console->setVariable("$Dlg::Error", "Please enter the address of the person you wish to ban");
            //root->onMessage(this, IDDLG_ERROR);
         }
         break;
      }

      case IDCTG_IRCDLG_BAN_BTN_UNBAN :
      {
         Int32               iSel;
         char                szBuf[256];
         char                szTwo[512];
         IRCClient::Channel *channel;

         edit->getText(szBuf);
         channel = objIRCClient->findChannel(NULL);

         if (strlen(szBuf) && channel)
         {
            if ((iSel = list->findEntry(szBuf)) != -1)
            {
               sprintf(szTwo, "/MODE %s -b %s", channel->name, szBuf);
               objIRCClient->send(szTwo);
               list->setSelection(iSel);
               list->deleteSelected();

               list->setSelection(iSel);
               
               if (list->getSelectedPos() == -1)
               {
                  list->setSelection(iSel - 1);
               }

               if (list->getNumEntries() == 0)
               {
                  edit->setText("");
               }
            }
         }
         else
         {
            //Console->setVariable("$Dlg::Error", "Please enter the address of the person you wish to unban");
            //root->onMessage(this, IDDLG_ERROR);
         }
         break;
      }
   }

   return (Parent::onMessage(obj, dwMsg));
}

IMPLEMENT_PERSISTENT_TAG(FGIRCBanControl, FOURCC('F','G','i','b'));
};
