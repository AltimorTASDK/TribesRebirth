//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "ircPlugin.h"
#include "ircClient.h"
#include "link.h"

DeclareLink(IRCPlugin)


enum CallbackID {
   Connect,
   Disconnect,
   Nick,
   Name,
   Send,
   Who,
   Join,
   Part,
   Messages,
   ListChannels,
   ListPeople,
   SetChannel,
   SetEcho,
};


//------------------------------------------------------------------------------
void IRCPlugin::init()
{
   console->addCommand(Connect,           "ircConnect", this);
   console->addCommand(Disconnect,        "ircDisconnect", this);
   console->addCommand(Nick,              "ircNick", this);
   console->addCommand(Name,              "ircName", this);
   console->addCommand(Send,              "ircSend", this);
   console->addCommand(Who,               "ircWho", this);
   console->addCommand(ListChannels,      "ircListChannels", this);
   console->addCommand(ListPeople,        "ircListPeople", this);
   console->addCommand(SetChannel,        "ircSetChannel", this);
   console->addCommand(SetEcho,           "ircEcho", this);
 }


//----------------------------------------------------------------------------
const char* IRCPlugin::consoleCallback(CMDConsole *console, int id,int argc,const char *argv[])
{
   console;
   switch(id) 
   {
      //--------------------------------------
      case Connect:
         if (argc >= 2)
         {
            int port = IRCClient::DEFAULT_PORT;
            if (argc == 3)   
               port = atoi(argv[2]);
            IRCClient *irc = IRCClient::find(manager);
            if (irc)
               irc->connect(argv[1], port);
            break;
         }
         Console->printf("ircConnect( ircServerAddress, {port} );");
         return (0);

      //--------------------------------------
      case Disconnect:
         if (argc == 1)
         {
            IRCClient *irc = IRCClient::find(manager);
            if (irc)
               irc->disconnect();
            break;
         }
         Console->printf("ircDisconnect();");
         return (0);

      //--------------------------------------
      case Nick:
         if (argc == 2)
         {
            IRCClient *irc = IRCClient::find(manager);
            if (irc)
               irc->nick(argv[1]);
            break;
         }
         Console->printf("ircNick( nickname );");
         return (0);

      //--------------------------------------
      case Name:
         if (argc == 2)
         {
            IRCClient *irc = IRCClient::find(manager);
            if (irc)
               irc->name(argv[1]);
            break;
         }
         Console->printf("ircName( realname );");
         return (0);

      //--------------------------------------
      case Send:
         if (argc == 2 || argc == 3)
         {
            IRCClient *irc = IRCClient::find(manager);
            if (irc)
               irc->send(argv[1], (argc == 3) ? argv[2] : NULL);
            break;
         }
         Console->printf("ircSend( \"message\" );");
         return (0);

      //--------------------------------------
      case ListChannels:
         if (argc == 1)
         {
            IRCClient *irc = IRCClient::find(manager);
            if (irc)
            {
               VectorPtr<IRCClient::Channel*> *c = irc->getChannels();
               VectorPtr<IRCClient::Channel*>::iterator itr = c->begin();
               Console->printf("IRC Channels:");
               for (; itr != c->end(); itr++)
                  Console->printf("  %s", (*itr)->name);
            }
         }
         break;

      //--------------------------------------
      case ListPeople:
         if (argc == 1)
         {
            IRCClient *irc = IRCClient::find(manager);
            if (irc)
            {
               VectorPtr<IRCClient::Person*> *p = irc->getPeople();
               VectorPtr<IRCClient::Person*>::iterator itr = p->begin();
               Console->printf("IRC People:");
               for (; itr != p->end(); itr++)
                  Console->printf("  %02d %s (%s) -- %s", (*itr)->ref, (*itr)->nick, (*itr)->identity, (*itr)->real);
            }
         }
         if (argc == 2)
         {
            IRCClient *irc = IRCClient::find(manager);
            if (irc)
            {
               IRCClient::Channel *c = irc->findChannel(argv[1]);
               if (c)
               {
                  VectorPtr<IRCClient::PersonInfo>::iterator itr = c->members.begin();
                  Console->printf("IRC People on: %s", argv[1]);
                  for (; itr != c->members.end(); itr++)
                  {
                     char flag = ' ';
                     if (itr->flags & IRCClient::PERSON_OPERATOR)
                        flag = '@';
                     else if (itr->flags & IRCClient::PERSON_SPEAKER)
                        flag = '+';

                     Console->printf("  %02d %c%s (%s) -- %s", 
                        itr->person->ref, 
                        flag,
                        itr->person->nick, 
                        itr->person->identity, 
                        itr->person->real);
                  }
               }
            }
         }
         break;

      //--------------------------------------
      case SetChannel:
         if (argc == 2)
         {
            IRCClient *irc = IRCClient::find(manager);
            if (irc)
               irc->setCurrentChannel(argv[1]);  
         }
         else
            Console->printf("ircSetChannel( channel_name );");
         break;

      //--------------------------------------
      case SetEcho:
         if (argc == 2)
         {
            int echo = IRCClient::ECHO_NONE;
            if (!stricmp(argv[1], "all")) echo = IRCClient::ECHO_ALL;
            else if (!stricmp(argv[1], "current")) echo = IRCClient::ECHO_CURRENT;
            else if (!stricmp(argv[1], "status")) echo = IRCClient::ECHO_STATUS;

            IRCClient *irc = IRCClient::find(manager);
            if (irc)
               irc->setEcho(echo);  
         }
         else
            Console->printf("ircEcho( {all|system|current|none} );");
         break;

   }
   return "true";
}
