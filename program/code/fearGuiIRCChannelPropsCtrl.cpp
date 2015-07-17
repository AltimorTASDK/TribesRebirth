#include <fearguiircchannelpropsctrl.h>
#include <ircclient.h>
#include <fear.strings.h>
#include <console.h>
#include <simguiactivectrl.h>
#include <simguitextwrap.h>
#include <fearguiunivbutton.h>
#include <fearguitestedit.h>
#include <darkstar.strings.h>

namespace FearGui
{
FGIRCChannelPropertiesControl::FGIRCChannelPropertiesControl()
: objIRCClient(NULL), fCanSetTopic(false), fCanSetProps(false)
{
}

FGIRCChannelPropertiesControl::~FGIRCChannelPropertiesControl()
{
}

void FGIRCChannelPropertiesControl::onWake()
{
   char                   szBuf[15];
   TestEdit              *edit;
   SimGui::TextWrap      *wrap;
   FGUniversalButton     *button;
   SimGui::ActiveCtrl    *control;
   IRCClient::Person     *person;
   IRCClient::Channel    *channel;
   IRCClient::PersonInfo *info;

   fCanSetTopic = false;
   fCanSetProps = false;
   objIRCClient = IRCClient::find(manager);

   AssertFatal(objIRCClient, "GuiIRCChannelPropsCtrl: cannot locate IRC client");

   person  = objIRCClient->getMe();
   channel = objIRCClient->findChannel(NULL);

   wrap = static_cast<SimGui::TextWrap *>
      (findControlWithTag(IDCTG_IRCDLG_PROPS_WRP_BANNER));

   if (person && channel)
   {
      info = channel->findPerson(person->nick);

      if (info)
      {
         if (info->flags & IRCClient::PERSON_OPERATOR)
         {
            fCanSetTopic = true;
            fCanSetProps = true;
            wrap->setText(SimTagDictionary::
               getString(manager, IDSTR_CHANPROPS_OPER_BANNER));
         }
         else if (!(channel->flags & IRCClient::CHANNEL_TOPIC_LIMITED))
         {
            fCanSetTopic = true;
            fCanSetProps = false;

            if (info->flags & IRCClient::PERSON_SPEAKER || 
              !(channel->flags & IRCClient::CHANNEL_MODERATED))
            {
               wrap->setText(SimTagDictionary::
                  getString(manager, IDSTR_CHANPROPS_SPKR_BANNER));
            }
            else
            {
               wrap->setText(SimTagDictionary::
                  getString(manager, IDSTR_CHANPROPS_SPEC_BANNER));
            }
         }
         else
         {
            fCanSetTopic = false;
            fCanSetProps = false;

            wrap->setText(SimTagDictionary::
               getString(manager, IDSTR_CHANPROPS_NOT_OPER_BANNER));
         }
      }

      button = static_cast<FGUniversalButton *>
         (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_MODERATED));
      button->setMode((bool)(channel->flags & IRCClient::CHANNEL_MODERATED));

      button = static_cast<FGUniversalButton *>
         (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_SET_TOPIC_ANYONE));
      button->setMode((bool)(!(channel->flags & IRCClient::CHANNEL_TOPIC_LIMITED)));

      button = static_cast<FGUniversalButton *>
         (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_INVITE_ONLY));
      button->setMode((bool)(channel->flags & IRCClient::CHANNEL_INVITE));

      button = static_cast<FGUniversalButton *>
         (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_HIDDEN));
      button->setMode((bool)(channel->flags & IRCClient::CHANNEL_SECRET));

      button = static_cast<FGUniversalButton *>
         (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_PRIVATE));
      button->setMode((bool)(channel->flags & IRCClient::CHANNEL_PRIVATE));

      edit = static_cast<TestEdit *>
         (findControlWithTag(IDCTG_IRCDLG_PROPS_EDT_TOPIC));
      edit->setText(channel->topic ? channel->topic : "");

      edit = static_cast<TestEdit *>
         (findControlWithTag(IDCTG_IRCDLG_PROPS_EDT_USER_LIMIT));
      if (channel->personLimit > 0)
      {
         sprintf(szBuf, "%d", channel->personLimit);
         edit->setText(szBuf);
      }
      else
      {
         edit->setText("0");
      }

      edit = static_cast<TestEdit *>
         (findControlWithTag(IDCTG_IRCDLG_PROPS_EDT_PASSWORD));
      edit->setText(channel->key ? channel->key : "");

      button = static_cast<FGUniversalButton *>
         (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_SET_USER_LIMIT));
      button->setMode(channel->personLimit > 0);

      button = static_cast<FGUniversalButton *>
         (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_SET_PASSWORD));
      button->setMode((channel->key && strlen(channel->key)));
   }

   control = static_cast<SimGui::ActiveCtrl *>
      (findControlWithTag(IDCTG_IRCDLG_PROPS_EDT_TOPIC));
   control->active = fCanSetTopic;

   control = static_cast<SimGui::ActiveCtrl *>
      (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_MODERATED));
   control->active = fCanSetProps;

   control = static_cast<SimGui::ActiveCtrl *>
      (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_SET_TOPIC_ANYONE));
   control->active = fCanSetProps;

   control = static_cast<SimGui::ActiveCtrl *>
      (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_INVITE_ONLY));
   control->active = fCanSetProps;

   control = static_cast<SimGui::ActiveCtrl *>
      (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_HIDDEN));
   control->active = fCanSetProps;

   control = static_cast<SimGui::ActiveCtrl *>
      (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_PRIVATE));
   control->active = fCanSetProps;

   control = static_cast<SimGui::ActiveCtrl *>
      (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_SET_USER_LIMIT));
   control->active = fCanSetProps;

   control = static_cast<SimGui::ActiveCtrl *>
      (findControlWithTag(IDCTG_IRCDLG_PROPS_EDT_USER_LIMIT));
   control->active = fCanSetProps && channel && (channel->personLimit > 0);

   control = static_cast<SimGui::ActiveCtrl *>
      (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_SET_PASSWORD));
   control->active = fCanSetProps;

   control = static_cast<SimGui::ActiveCtrl *>
      (findControlWithTag(IDCTG_IRCDLG_PROPS_EDT_PASSWORD));
   control->active = fCanSetProps && channel && (channel->key && strlen(channel->key));
}

DWORD FGIRCChannelPropertiesControl::onMessage(SimObject *obj, DWORD dwMsg)
{
   switch (dwMsg)
   {
      case IDCTG_IRCDLG_PROPS_CHK_SET_USER_LIMIT :
      {
         FGUniversalButton *button = static_cast<FGUniversalButton *>
            (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_SET_USER_LIMIT));
         SimGui::ActiveCtrl *control = static_cast<SimGui::ActiveCtrl *>
            (findControlWithTag(IDCTG_IRCDLG_PROPS_EDT_USER_LIMIT));

         control->active = button->isSet();
         control->setUpdate();
         break;
      }

      case IDCTG_IRCDLG_PROPS_CHK_SET_PASSWORD :
      {
         FGUniversalButton *button = static_cast<FGUniversalButton *>
            (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_SET_PASSWORD));
         SimGui::ActiveCtrl *control = static_cast<SimGui::ActiveCtrl *>
            (findControlWithTag(IDCTG_IRCDLG_PROPS_EDT_PASSWORD));

         control->active = button->isSet();
         control->setUpdate();
         break;
      }

      case IDCTG_IRCDLG_PROPS_CHK_PRIVATE :
      {
         FGUniversalButton *button1 = static_cast<FGUniversalButton *>
            (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_PRIVATE));
         FGUniversalButton *button2 = static_cast<FGUniversalButton *>
            (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_HIDDEN));
  
         if (button1->isSet())
         {
            button2->setMode(false);
         }
         break;
      }

      case IDCTG_IRCDLG_PROPS_CHK_HIDDEN :
      {
         FGUniversalButton *button1 = static_cast<FGUniversalButton *>
            (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_HIDDEN));
         FGUniversalButton *button2 = static_cast<FGUniversalButton *>
            (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_PRIVATE));
         
         if (button1->isSet())
         {
            button2->setMode(false);
         }
         break;
      }

      case IDDLG_OK :
      {
         unsigned int        iLen = 0;
         IRCClient::Channel *channel;
         FGUniversalButton  *button;
         TestEdit           *edit;
         char                szTopic[256];
         char                szMode[256], szText[50], szExtra[256];

         memset(szMode,  '\0', 256);
         memset(szText,  '\0',  50);
         memset(szExtra, '\0', 256);
         memset(szTopic, '\0', 256);

         channel = objIRCClient->findChannel(NULL);

         if (channel && stricmp(channel->name, "status"))
         {
            if (fCanSetTopic)
            {
               edit = static_cast<TestEdit *>
                  (findControlWithTag(IDCTG_IRCDLG_PROPS_EDT_TOPIC));
               edit->getText(szText);
               if (channel->topic == NULL || strcmp(szText, channel->topic))
               {
                  sprintf(szTopic, "/TOPIC %s :%s", channel->name, szText);
               }
            }

            if (fCanSetProps)
            {
               // This has to be formatted in a specific way, with the
               // modes that require arguments up front, and the rest
               // all scrunched together at the end
               iLen = sprintf(szMode, "/MODE %s ", channel->name);

               // Set/unset max users status
               button = static_cast<FGUniversalButton *>
                  (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_SET_USER_LIMIT));
               edit   = static_cast<TestEdit *>
                  (findControlWithTag(IDCTG_IRCDLG_PROPS_EDT_USER_LIMIT));
               if (button->isSet())
               {
                  edit->getText(szText);
            
                  // No text implies a zero limit
                  if (strlen(szText) == 0)
                  {
                     strcpy(szText, "0");
                  }

                  // If they entered a value greater than zero ...
                  if (atoi(szText) > 0)
                  {
                     // If this is not a limited channel, or the limit is
                     // not the same as the old limit, change the limit
                     if (!(channel->flags & IRCClient::CHANNEL_LIMITED) ||
                          (atoi(szText) != channel->personLimit))
                     {
                        sprintf(szMode + strlen(szMode), "+l %s ", szText);
                     }
                  }
                  // If the value was zero, clear the limit
                  else
                  {
                     if (channel->flags & IRCClient::CHANNEL_LIMITED)
                     {
                        strcat(szMode, "-l ");
                     }
                  }
               }
               // If they turned the button OFF, clear the LIMIT
               else if (channel->flags & IRCClient::CHANNEL_LIMITED)
               {
                  strcat(szMode, "-l ");
               }

               // Set/unset password status
               button = static_cast<FGUniversalButton *>
                  (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_SET_PASSWORD));
               edit   = static_cast<TestEdit *>
                  (findControlWithTag(IDCTG_IRCDLG_PROPS_EDT_PASSWORD));
               if (button->isSet())
               {
                  // Get the new password
                  edit->getText(szText);

                  // Need to clear IF:
                  // channel HAS KEY and text is EMPTY
                  // channel HAS KEY and text is DIFFERENT FROM KEY
                  if ((channel->flags & IRCClient::CHANNEL_HAS_KEY) &&
                      (strlen(szText) == 0 || (channel->key && strcmp(szText, channel->key))))
                  {
                     sprintf(szExtra, 
                        "/MODE %s -k %s", channel->name, channel->key);
                  }

                  // Need to set IF:
                  // text is NOT EMPTY AND
                  // channel DOESN't have key OR
                  // channel HAS key and new text is DIFFERENT FROM KEY
                  if (strlen(szText) && 
                     (!(channel->flags & IRCClient::CHANNEL_HAS_KEY) ||
                        strcmp(channel->key, szText)))
                  {
                     sprintf(szMode + strlen(szMode), "+k %s ", szText);
                  }
               }
               // If they turned the button OFF, clear the KEY
               else if ((channel->flags & IRCClient::CHANNEL_HAS_KEY) && channel->key)
               {
                  sprintf(szExtra, 
                     "/MODE %s -k %s", channel->name, channel->key);
               }

               // Set/unset moderated status
               button = static_cast<FGUniversalButton *>
                  (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_MODERATED));
               if (button->isSet())
               {
                  if (!(channel->flags & IRCClient::CHANNEL_MODERATED))
                  {
                     strcat(szMode, "+m");
                  }
               }
               else if (channel->flags & IRCClient::CHANNEL_MODERATED)
               {
                  strcat(szMode, "-m");
               }

               // Set/unset set topic status
               button = static_cast<FGUniversalButton *>
                  (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_SET_TOPIC_ANYONE));
               if (button->isSet())
               {
                  if (channel->flags & IRCClient::CHANNEL_TOPIC_LIMITED)
                  {
                     strcat(szMode, "-t");
                  }
               }
               else if (!(channel->flags & IRCClient::CHANNEL_TOPIC_LIMITED))
               {
                  strcat(szMode, "+t");
               }

               // Set/unset invite only status
               button = static_cast<FGUniversalButton *>
                  (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_INVITE_ONLY));
               if (button->isSet())
               {
                  if (!(channel->flags & IRCClient::CHANNEL_INVITE))
                  {
                     strcat(szMode, "+i");
                  }
               }
               else if (channel->flags & IRCClient::CHANNEL_INVITE)
               {
                  strcat(szMode, "-i");
               }

               // Set/unset hidden status
               button = static_cast<FGUniversalButton *>
                  (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_HIDDEN));
               if (button->isSet())
               {
                  if (!(channel->flags & IRCClient::CHANNEL_SECRET))
                  {
                     strcat(szMode, "+s");
                  }
               }
               else if (channel->flags & IRCClient::CHANNEL_SECRET)
               {
                  strcat(szMode, "-s");
               }

               // Set/unset private status
               button = static_cast<FGUniversalButton *>
                  (findControlWithTag(IDCTG_IRCDLG_PROPS_CHK_PRIVATE));
               if (button->isSet())
               {
                  if (!(channel->flags & IRCClient::CHANNEL_PRIVATE))
                  {
                     strcat(szMode, "+p");
                  }
               }
               else if (channel->flags & IRCClient::CHANNEL_PRIVATE)
               {
                  strcat(szMode, "-p");
               }
            }
         }

         if (strlen(szTopic))
         {
            objIRCClient->send(szTopic);
         }

         if (strlen(szExtra))
         {
            objIRCClient->send(szExtra);
         }

         // Only send the new mode string if it was altered
         if (strlen(szMode) > iLen)
         {
            objIRCClient->send(szMode);
         }
      }
   }

   return (Parent::onMessage(obj, dwMsg));
}

IMPLEMENT_PERSISTENT_TAG(FGIRCChannelPropertiesControl, FOURCC('F','i','c','p'));
};