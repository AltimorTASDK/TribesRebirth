#include <feardcl.h>
#include <ircclient.h>
#include <fearguiircchannellist.h>
#include <fear.strings.h>
#include <esf.strings.h>
#include <fearguitestedit.h>
#include <fearguiircjoincontrol.h>
#include <console.h>
#include <fearguiunivbutton.h>
#include <darkstar.strings.h>
#include <fearguiircactivetextedit.h>

namespace FearGui
{
FGIRCJoinControl::FGIRCJoinControl()
{
}

FGIRCJoinControl::~FGIRCJoinControl()
{
}

void FGIRCJoinControl::onWake()
{
   FGIRCActiveTextEdit *edit;
   FGIRCChannelList    *list;

   objIRCClient = IRCClient::find(manager);
   AssertFatal(objIRCClient, "FGIRCJoinControl: unable to locate IRC client");

   if (Console->getBoolVariable("$Gui::NoAutoCreate") == false)
   {
      edit = static_cast<FGIRCActiveTextEdit *>
         (findControlWithTag(IDCTG_IRCDLG_JOIN_EDT_CHANNEL));

      list = static_cast<FGIRCChannelList *>
         (findControlWithTag(IDCTG_IRCDLG_JOIN_LBX_CHANNELS));

      edit->associateList(list);

      if (Console->getBoolVariable("$IRC::ShowAllChannels"))
      {
         FGUniversalButton *button = static_cast<FGUniversalButton *>
            (findControlWithTag(IDCTG_IRCDLG_JOIN_CHK_LISTALL));

         list->clear();
         objIRCClient->setNotifyObject(this);
         objIRCClient->requestChannelList();
         button->active = false;
         button->setMode(true);
      }
      else
      {
         getDefaultChannelList();
      }
   }
}

void FGIRCJoinControl::getDefaultChannelList()
{
   const char          *lpszName, *lpszTopic;
   FGIRCActiveTextEdit *edit;
   FGIRCChannelList    *list;
   char                 szBuf[256];

   list = static_cast<FGIRCChannelList *>
      (findControlWithTag(IDCTG_IRCDLG_JOIN_LBX_CHANNELS));
   edit = static_cast<FGIRCActiveTextEdit *>
      (findControlWithTag(IDCTG_IRCDLG_JOIN_EDT_CHANNEL));

   for (int i = 0; i < 100; i ++)
   {
      sprintf(szBuf, "IRC::ChannelName%d", i);
      lpszName = Console->getVariable(szBuf);

      if (*lpszName == '\0')
      {
         break;
      }

      sprintf(szBuf, "IRC::ChannelTopic%d", i);
      lpszTopic = Console->getVariable(szBuf);

      list->addChannel(lpszName, lpszTopic, false);
   }

   edit->getText(szBuf);
   list->showMatches(szBuf);
}

DWORD FGIRCJoinControl::onMessage(SimObject *obj, DWORD dwMsg)
{
   switch (dwMsg)
   {
      case IDCTG_IRCDLG_JOIN_LBX_CHANNELS :
      {
         const char       *lpszChannel;
         FGIRCChannelList *list;
         TestEdit      *edit;

         edit = static_cast<TestEdit *>
            (findControlWithTag(IDCTG_IRCDLG_JOIN_EDT_CHANNEL));
         list = static_cast<FGIRCChannelList *>
            (findControlWithTag(IDCTG_IRCDLG_JOIN_LBX_CHANNELS));

         if (edit && list)
         {
            lpszChannel = list->getSelectedChannel();

            if (lpszChannel)
            {
               edit->setText(lpszChannel);
            }
         }
         break;
      }

      case IDCTG_IRCDLG_JOIN_CHK_LISTALL :
      {
         FGUniversalButton *button = static_cast<FGUniversalButton *>
            (findControlWithTag(IDCTG_IRCDLG_JOIN_CHK_LISTALL));
         FGIRCChannelList *list = static_cast<FGIRCChannelList *>
            (findControlWithTag(IDCTG_IRCDLG_JOIN_LBX_CHANNELS));

         list->clear();

         if (button->isSet())
         {
            Console->setBoolVariable("$IRC::ShowAllChannels", true);
            objIRCClient->setNotifyObject(this);
            objIRCClient->requestChannelList();
            button->active = false;
         }
         else
         {
            Console->setBoolVariable("$IRC::ShowAllChannels", false);
            getDefaultChannelList();
         }
         break;
      }

      case IDDLG_OK :
      {
         char         szBuf[256];
         TestEdit *edit;

         edit = static_cast<TestEdit *>
            (findControlWithTag(IDCTG_IRCDLG_JOIN_EDT_CHANNEL));

         edit->getText(szBuf);

         if (strlen(szBuf))
         {
            if (szBuf[0] != '#' && szBuf[0] != '&')
            {
               char szTmp[257];

               sprintf(szTmp, "#%s", szBuf);
               strcpy (szBuf, szTmp);
            }
            Console->setVariable("$IRCMsg::Channel", szBuf);
         }
         else
         {
            return (0);
         }

         edit = static_cast<TestEdit *>
            (findControlWithTag(IDCTG_IRCDLG_JOIN_EDT_PASSWORD));

         edit->getText(szBuf);

         if (strlen(szBuf))
         {
            Console->setVariable("$IRCMsg::Key", szBuf);
         }
         break;
      }

      case IDDLG_CANCEL :
      {
         szChannel[0] = '\0';
         break;
      }
   }

   return (Parent::onMessage(obj, dwMsg));
}

bool FGIRCJoinControl::onSimMessageEvent(const SimMessageEvent *event)
{
   switch (event->message)
   {
      case IDIRC_CHANNEL_LIST :
      {
         FGIRCChannelList    *list;
         VectorPtr<IRCClient::Channel *> *channels = objIRCClient->getChannelList();

         list = static_cast<FGIRCChannelList *>
            (findControlWithTag(IDCTG_IRCDLG_JOIN_LBX_CHANNELS));

         if (list)
         {
            for (int i = 0; i < channels->size(); i ++)
            {
               list->addChannel((*channels)[i]->name, (*channels)[i]->topic);
            }

            // Add default channels
            getDefaultChannelList();
         }
         FGUniversalButton *button = static_cast<FGUniversalButton *>
            (findControlWithTag(IDCTG_IRCDLG_JOIN_CHK_LISTALL));

         button->active = true;
         return (true);
      }
   }
   return (false);
}

bool FGIRCJoinControl::processEvent(const SimEvent *event)
{
   switch (event->type)
   {
      onEvent(SimMessageEvent);
   }
   return (Parent::processEvent(event));
}

IMPLEMENT_PERSISTENT_TAG(FGIRCJoinControl, FOURCC('F','i','j','c'));
};