//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "ircClient.h"
#include "console.h"
#include "esf.strings.h"
#include "m_qsort.h"
#include <simTagDictionary.h>


//#define IRCDEBUG  // comment out to turn OFF extra IRC debugging output

#define VERSION_FLOOD_TIMEOUT  5.0f
#define PING_FLOOD_TIMEOUT     5.0f
            
#define CHANNEL_STATUS "Status"
#define VERSION_STRING "Dynamix IRC Chat 1.2.0"


IMPLEMENT_PERSISTENT( IRCClient );


DWORD IRCClient::Channel::nextId = 0;
DWORD IRCClient::Person::nextId = 0;
DWORD IRCClient::Message::nextId = 0;


//------------------------------------------------------------------------------
bool isInSet(char c, char *set)
{
   if (set)
   {
      while (*set)
      {
         if (c == *set++)
         {
            return (true);
         }
      }
   }
   return (false);
}   

//------------------------------------------------------------------------------
char* nextToken(char *str, char *&token, char *delim)
{
   if (str)
   {
      // skip over any characters that are a member of delim
      // no need for special '\0' check since it can never be in delim
      while (isInSet(*str, delim))
      {
         str++;
      }
   
      token = str;
   
      // skip over any characters that are NOT a member of delim
      while (*str && !isInSet(*str, delim))
      {
         str++;
      }

      // terminate the token
      if (*str) 
      {
         *str++ = 0;
      }

      // advance str past the 'delim space'
      while (isInSet(*str, delim))
      {
         str++;
      }
   }

   return (str);
}   

static int __cdecl comparePeople(const void *a,const void *b)
{
   IRCClient::PersonInfo *person_A = (IRCClient::PersonInfo*)(a);
   IRCClient::PersonInfo *person_B = (IRCClient::PersonInfo*)(b);

   int ao = person_A->flags & (IRCClient::PERSON_SPEAKER|IRCClient::PERSON_OPERATOR);
   int bo = person_B->flags & (IRCClient::PERSON_SPEAKER|IRCClient::PERSON_OPERATOR);
   if (ao != bo )
      return (bo-ao);

   return (stricmp(person_A->person->nick, person_B->person->nick));
} 


//------------------------------------------------------------------------------
IRCClient::IRCClient()
{
   // Invalidate everything
   iMsgHistory      = 0;
   logFile          = NULL;
   id               = IRCClientObjectId;
   hSocket          = INVALID_SOCKET;
   bytesInBuffer    = 0;
   state            = IDIRC_DISCONNECTED;
   ircServerAddress = NULL;
   echo             = ECHO_NONE;
   notifyObject     = NULL;
   rLastVersioned   = 0.0f;
   rLastPinged      = 0.0f;

   people.push_back(new Person(NULL, NULL, NULL));
   (*people.begin())->incRef(); // set my ref count so I never get deleted
   currentChannel = findChannel(CHANNEL_STATUS, true);

   incomming.push_back(new OnHandler("PING",   &IRCClient::onPing));
   incomming.push_back(new OnHandler("PONG",   &IRCClient::onPong));
   incomming.push_back(new OnHandler("PRIVMSG",&IRCClient::onPrivMsg));
   incomming.push_back(new OnHandler("JOIN",   &IRCClient::onJoin));
   incomming.push_back(new OnHandler("NICK",   &IRCClient::onNick));
   incomming.push_back(new OnHandler("QUIT",   &IRCClient::onQuit));
   incomming.push_back(new OnHandler("ERROR",  &IRCClient::onError));
   incomming.push_back(new OnHandler("TOPIC",  &IRCClient::onTopic));
   incomming.push_back(new OnHandler("PART",   &IRCClient::onPart));
   incomming.push_back(new OnHandler("KICK",   &IRCClient::onKick));
   incomming.push_back(new OnHandler("MODE",   &IRCClient::onMode));
   incomming.push_back(new OnHandler("AWAY",   &IRCClient::onAway));
   incomming.push_back(new OnHandler("NOTICE", &IRCClient::onNotice));
   incomming.push_back(new OnHandler("PING",   &IRCClient::onPing));
   incomming.push_back(new OnHandler("PONG",   &IRCClient::onPong));
   incomming.push_back(new OnHandler("VERSION",&IRCClient::onVersion));
   incomming.push_back(new OnHandler("ACTION", &IRCClient::onAction));
   incomming.push_back(new OnHandler("INVITE", &IRCClient::onInvite));

   incomming.push_back(new OnHandler("301",    &IRCClient::onAwayReply));
   incomming.push_back(new OnHandler("305",    &IRCClient::onUnAwayReply));
   incomming.push_back(new OnHandler("306",    &IRCClient::onNowAwayReply));
   incomming.push_back(new OnHandler("311",    &IRCClient::onWhoisUserReply));// WHOIS user
   incomming.push_back(new OnHandler("312",    &IRCClient::onWhoisReply));    // WHOIS server
   incomming.push_back(new OnHandler("315",    &IRCClient::onEndOfWho));      // WHO end
   incomming.push_back(new OnHandler("317",    &IRCClient::onWhoisIdle));     // WHOIS idle
   incomming.push_back(new OnHandler("318",    &IRCClient::onWhoisReply));    // WHOIS end
   incomming.push_back(new OnHandler("319",    &IRCClient::onWhoisReply));    // WHOIS channels

   incomming.push_back(new OnHandler("322",    &IRCClient::onList));
   incomming.push_back(new OnHandler("323",    &IRCClient::onListEnd));
   incomming.push_back(new OnHandler("324",    &IRCClient::onModeReply));
   incomming.push_back(new OnHandler("331",    &IRCClient::onNoTopic));
   incomming.push_back(new OnHandler("332",    &IRCClient::onTopic));

   incomming.push_back(new OnHandler("341",    &IRCClient::onInviteReply));
   
   incomming.push_back(new OnHandler("352",    &IRCClient::onWhoReply));
   incomming.push_back(new OnHandler("353",    &IRCClient::onNameReply));
   incomming.push_back(new OnHandler("367",    &IRCClient::onBanList));
   incomming.push_back(new OnHandler("368",    &IRCClient::onBanListEnd));
   incomming.push_back(new OnHandler("372",    &IRCClient::onMOTD));
   incomming.push_back(new OnHandler("376",    &IRCClient::onMOTDEnd));
   incomming.push_back(new OnHandler("422",    &IRCClient::onMOTDEnd));    // just call end
   incomming.push_back(new OnHandler("433",    &IRCClient::onBadNick));
   incomming.push_back(new OnHandler("465",    &IRCClient::onServerBanned));
   incomming.push_back(new OnHandler("468",    &IRCClient::onInvalidNick)); // undernet
   incomming.push_back(new OnHandler("471",    &IRCClient::onChannelFull));
   incomming.push_back(new OnHandler("473",    &IRCClient::onChannelInviteOnly));
   incomming.push_back(new OnHandler("474",    &IRCClient::onChannelBanned));
   incomming.push_back(new OnHandler("475",    &IRCClient::onBadChannelKey));

   // we don't need to handle all outgoing commands just a few
   outgoing.push_back(new OutHandler("PING",   &IRCClient::ping));
   outgoing.push_back(new OutHandler("PART",   &IRCClient::part));
   outgoing.push_back(new OutHandler("AWAY",   &IRCClient::away));
   outgoing.push_back(new OutHandler("ME",     &IRCClient::sendAction));
   outgoing.push_back(new OutHandler("ACTION", &IRCClient::sendAction));
   outgoing.push_back(new OutHandler("JOIN",   &IRCClient::join));
   outgoing.push_back(new OutHandler("QUIT",   &IRCClient::quit));
}   


//------------------------------------------------------------------------------
IRCClient::~IRCClient()
{
   // All deletion is now done in onRemove
}   


//------------------------------------------------------------------------------
static bool gbDisconnect = FALSE;
static int __stdcall blockingHook()
{
   if (gbDisconnect)
      WSACancelBlockingCall();
   return false;
}

int IRCClient::threadProcess()
{
   state = IDIRC_CONNECTING_SOCKET;

   // resolve the server address
   SOCKADDR_IN addr;
   hostent *hp;
   addr.sin_addr.s_addr = inet_addr(ircServerAddress);
   
   //set the hook to cancel the blocking function
   gbDisconnect = FALSE;
   WSASetBlockingHook(blockingHook);
   
   if(addr.sin_addr.s_addr == INADDR_NONE)
   {
      if((hp = gethostbyname(ircServerAddress)) == NULL)
      {
         state = IDIRC_ERR_HOSTNAME;
         return (0); 
      }
      else
         memcpy(&addr.sin_addr.s_addr, hp->h_addr, sizeof(IN_ADDR));
   }
   addr.sin_port   = htons(port);
   addr.sin_family = AF_INET;

   if (state != IDIRC_CONNECTING_SOCKET)
      return (0);

   // try to connect to the IRC server
   hSocket = socket(AF_INET, SOCK_STREAM, 0);
   if (::connect(hSocket, (PSOCKADDR)&addr, sizeof(addr)) != 0)
   {
      closesocket(hSocket);
      state = IDIRC_ERR_CONNECT;
      return (0);
   }

   if (state != IDIRC_CONNECTING_SOCKET)
      return (0);

   bind(hSocket, (LPSOCKADDR) &addr, sizeof(addr));

   //make it a non-blocking connection
   DWORD noblock = true;
   ioctlsocket(hSocket, FIONBIO, &noblock);
   
   //unhook the "cancel the blocking" function
   WSAUnhookBlockingHook();
   
   state = IDIRC_CONNECTING_IRC;
   return (0);   
}   

//------------------------------------------------------------------------------
void IRCClient::_sendf(const char *format, ...)
{
   char buffer[2048];
   va_list pArgs;
   va_start(pArgs, format);
   vsprintf(buffer, format, pArgs);
   _send( buffer );
}   


//------------------------------------------------------------------------------
void IRCClient::_send(const char *message)
{
   // Per RFC_1459 the max message length 
   // is 512 characters and must be terminated with CR-LF
   static char buffer[513];
   strncpy(buffer, message, 510);
   buffer[510] = '\0';
   int length = strlen(buffer);
   strcat(buffer, "\r\n");
#ifdef IRCDEBUG
   Console->printf("IRC SEND: %s", buffer);
#endif
   ::send(hSocket, buffer, length+2, 0);
}   


//------------------------------------------------------------------------------
void IRCClient::recv()
{
   int bytesRead;
   char *head = recvbuffer+bytesInBuffer;
   do
   {
      bytesRead = ::recv(hSocket, head, SZ_RECV_PACKET-bytesInBuffer, 0);
      if (bytesRead == SOCKET_ERROR)
      {
         if (WSAGetLastError() != WSAEWOULDBLOCK)
         {
            state = IDIRC_ERR_DROPPED;
            return;
         }
         return;
      }
      
      bytesInBuffer += bytesRead;
      if (!bytesInBuffer)                          // are we done?
         return;

      recvbuffer[bytesInBuffer] = 0;               // terminate the packet
      head = processPacket(recvbuffer);            // process the packet
      
      // if we get an ERROR packet we disconnect during processPacket
      // so we need to early out here
      if (hSocket == INVALID_SOCKET)
         return;

      bytesInBuffer -= (head - recvbuffer);        // bytes not processed
      memmove(recvbuffer, head, bytesInBuffer+1);  // shift them to front of buffer 
      head = recvbuffer + bytesInBuffer;           // reposition head

   }while(bytesInBuffer);
}

//------------------------------------------------------------------------------

void IRCClient::getClientAddress()
{
   SOCKADDR_IN addr_in;
   int size = sizeof(SOCKADDR_IN);
   getsockname(hSocket, (PSOCKADDR) &addr_in, &size);
   char buf[256];
   sprintf(buf, "IP:%d.%d.%d.%d", addr_in.sin_addr.s_net,
      addr_in.sin_addr.s_host, addr_in.sin_addr.s_lh,
      addr_in.sin_addr.s_impno);

   Console->setVariable("$Client::IPAddress", buf);
}

//------------------------------------------------------------------------------
char* IRCClient::processPacket(char *buffer)
{
   // RFC_1459: Message Packet format
   //
   // <message>  ::= [':' <prefix> <SPACE> ] <command> <params> <crlf>
   // <prefix>   ::= <servername> | <nick> [ '!' <user> ] [ '@' <host> ]
   // <command>  ::= <letter> { <letter> } | <number> <number> <number>
   // <SPACE>    ::= ' ' { ' ' }
   // <params>   ::= <SPACE> [ ':' <trailing> | <middle> <params> ]
   //
   // <middle>   ::= <Any *non-empty* sequence of octets not including SPACE
   //                or NUL or CR or LF, the first of which may not be ':'>
   // <trailing> ::= <Any, possibly *empty*, sequence of octets not including
   //                   NUL or CR or LF>
   // <crlf>     ::= CR LF

   char *prefix;
   char *command;
   char *params;
   char *src = buffer;
   char *head = src;

   while (*src && (strstr(src, "\n") != NULL))
   { 
      head = src;
      prefix  = NULL;

#ifdef IRCDEBUG    
      char echo[SZ_RECV_PACKET+1];
      strcpy(echo, src);
      *strstr(echo, "\n") = 0;
//      AssertMessage(0, avar("PACKET: %s", echo));
      Console->printf("PACKET: %s", echo);
#endif

      // check for empty packet
      if (*src == '\r' || *src == '\n')   
      {
         src += 2;
         continue;
      }

      // check for prefix
      if (*src == ':')   
         src = nextToken(++src, prefix, " ");

      // this is the command
      src = nextToken(src, command, " :\r\n");
      
      // folowed by its params
      // src is advanced just past the '\n'
      src = nextToken(src, params, "\r\n");

#ifndef IRCDEBUG
      dispatch(prefix, command, params);
#else
      if (!dispatch(prefix, command, params))
      {
         char *t = strstr(echo, "\n");
         if (t) *t = 0;            
//         AssertMessage(0, avar("   ???: %s", echo));
         Console->printf("IRC ???: %s", echo);
      }
#endif

      head = src;
   }
   return (head);
}   


//------------------------------------------------------------------------------
bool IRCClient::dispatch(char* prefix, char* command, char* params)
{
   // dispatch the command to the correct handler
   OnHandlerVec::iterator itr = incomming.begin();
   for (; itr != incomming.end(); itr++)
      if (stricmp((*itr)->cmd, command) == 0)
      {
         (this->*((*itr)->fn))(prefix, params);
         return (true);
      }
   return (false);
}   


//------------------------------------------------------------------------------
void IRCClient::connect()
{
   const char *nick = Console->getVariable("$IRC::nickname");
   const char *real = Console->getVariable("$IRC::realname");

   if ( nick && real)
   {
      disconnect();   

      // If they want a log file, clear out the previous one and
      // start over
      if (Console->getBoolVariable("$IRC::UseLogFile") && 
          Console->getVariable    ("$IRC::LogFile"))
      {
         char fileName[256];
         sprintf(fileName, "temp\\%s", Console->getVariable("$IRC::LogFile"));
         logFile = new FileWStream(fileName);
      }

      connectTimeout = GetTickCount();
      (*people.begin())->setNick(nick);
      (*people.begin())->setReal(real);

      ircServerAddress = Console->getVariable("$IRC::server");
      port = Console->getIntVariable("$IRC::serverPort");;
      newMessage(NULL, IDIRC_CONNECTING_SOCKET, ircServerAddress, port);
      state = IDIRC_CONNECTING_SOCKET;
      startThread();
   }
   else
   {
      newMessage(NULL, IDIRC_ERR_SET_NICK);
   }
}   


//------------------------------------------------------------------------------
void IRCClient::connect(const char * ircServerAddress, int port)
{
   Console->setVariable("$IRC::server", ircServerAddress);
   Console->setIntVariable("$IRC::serverPort", port);
   connect();
}   


//------------------------------------------------------------------------------
void IRCClient::reset()
{
   // Clear the global channel list
   while (channelList.size())
   {
      delete channelList.last();
      channelList.pop_back();
   }

   // Clear all the channels except status
   for (VectorPtr<Channel *>::iterator iter = channels.begin(); iter < channels.end(); iter ++)
   {
      if (stricmp((*iter)->name, CHANNEL_STATUS))
      {
         delete (*iter);
         channels.erase(iter);
      }
      else
      {
         currentChannel = (*iter);
      }
   }

   // Delete everyone except myself
   while (people.size() != 1)
   {
      delete people.last();
      people.pop_back();
   }

   if (logFile)
   {
      logFile->close();
      delete logFile;
      logFile = NULL;
   }
}   


//------------------------------------------------------------------------------
void IRCClient::disconnect()
{
   //DWORD prevState = state;
   state = IDIRC_DISCONNECTED;

   //if (prevState == IDIRC_CONNECTING_SOCKET)   
   //{
   //  WSACancelBlockingCall();
   //}
   //use the hook function instead
   gbDisconnect = TRUE;

   stopThread();

   if (hSocket != INVALID_SOCKET)
   {
      closesocket(hSocket);
      hSocket = INVALID_SOCKET;
      newMessage(NULL, IDIRC_DISCONNECTED);
   }

   state          = IDIRC_DISCONNECTED;
   retries        = 0;
   bytesInBuffer  = 0;
   silentList     = 0;
   silentBanList  = 0;
   connectTimeout = 0;

   reset();
}   


//------------------------------------------------------------------------------
bool IRCClient::reconnect()
{
   if (!Console->getBoolVariable("$IRC::autoReconnect"))
      return (false);

   retries++;
   if (retries < 5)
   {
      newMessage(NULL, IDIRC_RECONNECT, retries, ircServerAddress); 
      notify(IDIRC_RECONNECT);
      connect();
   }
   else
   {
      newMessage(NULL, IDIRC_ERR_RECONNECT, retries, ircServerAddress); 
      notify(IDIRC_ERR_RECONNECT);
      state = IDIRC_DISCONNECTED;
   }
   return (true);
}   

//------------------------------------------------------------------------------
bool IRCClient::onSimFrameEndNotifyEvent(const SimFrameEndNotifyEvent*)
{
   int iNewMsgHistory = Console->getIntVariable("$IRC::MsgHistory");

   // Check new history level against the upper and lower bounds
   if (iNewMsgHistory < MIN_HISTORY || iNewMsgHistory > MAX_HISTORY)
   {
      // If there is no message history set, set the minimum
      iNewMsgHistory = iMsgHistory ? iMsgHistory : MIN_HISTORY;

      // Set it in the console
      Console->setIntVariable("$IRC::MsgHistory", iMsgHistory);
   }

   // If there is a disparity, fix it
   if (iNewMsgHistory != iMsgHistory)
   {
      // If the size was shrunk, pop all channel fronts until we
      // hit the proper size
      if (iMsgHistory < iNewMsgHistory)
      {
         for (int i = 0; i < channels.size(); i ++)
         {
            while (channels[i]->messages.size() > iNewMsgHistory)
            {
               channels[i]->messages.pop();
            }
         }
      }

      // And set the size
      iMsgHistory = iNewMsgHistory;
   }

   switch (state)
   {
      case IDIRC_CONNECTING_SOCKET:
         if (GetTickCount()-connectTimeout > TIMEOUT )
         {
            newMessage(NULL, IDIRC_ERR_TIMEOUT); 
            disconnect();
            state = IDIRC_DISCONNECTED;
            notify(IDIRC_ERR_TIMEOUT);
         }
         break;

      case IDIRC_CONNECTING_WAITING:
         if (GetTickCount()-connectTimeout > TIMEOUT )
         {
            newMessage(NULL, IDIRC_ERR_TIMEOUT); 
            disconnect();
            state = IDIRC_DISCONNECTED;
            notify(IDIRC_ERR_TIMEOUT);
         }
         else
            recv(); 
         break;

      case IDIRC_CONNECTED:      
         recv(); 
         break;
      
      case IDIRC_CONNECTING_DONE:
      {
         getClientAddress();

         newMessage(NULL, IDIRC_CONNECTING_DONE, ircServerAddress, port);
         state = IDIRC_CONNECTED;
         notify(IDIRC_CONNECTED);
         
         //auto join a room if requested
         const char *roomName = Console->getVariable("IRC::room");
         if (roomName[0]) send(avar("/join %s", roomName));
         break;
      }

      case IDIRC_ERR_CONNECT:    
         newMessage(NULL, IDIRC_ERR_CONNECT, ircServerAddress);
         if (!reconnect())
         {
            state = IDIRC_DISCONNECTED;
            notify(IDIRC_ERR_CONNECT);
         }
         break;

      case IDIRC_ERR_HOSTNAME:   
         newMessage(NULL, IDIRC_ERR_HOSTNAME, ircServerAddress); 
         state = IDIRC_DISCONNECTED;
         notify(IDIRC_ERR_HOSTNAME);
         break;

      case IDIRC_CONNECTING_IRC: {
         Person *me = *people.begin();
         _sendf("USER %s ignore ignore :%s", me->nick, me->real);
         _sendf("NICK %s", me->nick);
         state = IDIRC_CONNECTING_WAITING;
         }break;

      case IDIRC_ERR_DROPPED:
         newMessage(NULL, IDIRC_ERR_DROPPED, ircServerAddress); 
         if (!reconnect())
         {
            state = IDIRC_DISCONNECTED;
            notify(IDIRC_ERR_DROPPED);
         }
         break;

      case IDIRC_DISCONNECTED:
      default:
         break;
   }

   return (true);
}   


//------------------------------------------------------------------------------
void IRCClient::onGroupAdd()
{
   addToSet(SimFrameEndNotifySetId);      
}   


//------------------------------------------------------------------------------
void IRCClient::onRemove()
{
   Parent::onRemove();
   
   if (state == IDIRC_CONNECTED)
   {
      _sendf("QUIT");
   }

   disconnect();

   // clear the global channel List
   while (channelList.size())
   {
      delete channelList.last();
      channelList.pop_back();
   }

   // clear all the channels
   while (channels.size())
   {
      delete channels.last();
      channels.pop_back();
   }

   // delete all the people
   while (people.size())
   {
      delete people.last();
      people.pop_back();
   }
   while(incomming.size())
   {
      delete incomming.last();
      incomming.pop_back();
   }
   while(outgoing.size())
   {
      delete outgoing.last();
      outgoing.pop_back();
   }
   if (notifyObject) 
   { 
      clearNotify(notifyObject);   
   }
}   


//------------------------------------------------------------------------------
void IRCClient::onDeleteNotify(SimObject *object)
{
   if (notifyObject == object)
      notifyObject = NULL;
}   


//------------------------------------------------------------------------------
bool IRCClient::processEvent(const SimEvent* event)
{
   switch (event->type)
   {
      onEvent(SimFrameEndNotifyEvent);
      default:
         return (false);
   }
}   

//------------------------------------------------------------------------------
IRCClient::Person* IRCClient::findPerson(const char *nick)
{
   if (*nick == '@' || *nick == '+')
      nick++;

   VectorPtr<Person*>::iterator itr = people.begin();
   for (; itr != people.end(); itr++)
      if (stricmp((*itr)->nick, nick) == 0)
         return (*itr);
   return (NULL);
}   


//------------------------------------------------------------------------------
IRCClient::Person* IRCClient::findPerson(char *prefix, bool create)
{
   // typical name
   // Yakuza|!yakuzasama@pool032-max7.ds23-ca-us.dialup.earthlink.net
   char *nick;
   char *ident;

   prefix = nextToken(prefix, nick, " !");
   nextToken(prefix, ident, " ");
   
   if (nick)
   {
      if (*nick == '@' || *nick == '+')
         nick++;

      // look 'em up
      Person *p = findPerson(nick);
      if (p)
         return (p);

      if (create)
      {
         people.push_back(new Person(nick, NULL, ident));
         return (people.last());
      }
   }
   return (NULL);
}   


//------------------------------------------------------------------------------
IRCClient::Channel* IRCClient::findChannel(const char *name, bool create)
{
   if (!name)
      return (currentChannel);

   VectorPtr<Channel*>::iterator itr = channels.begin();
   for (; itr != channels.end(); itr++)
      if (stricmp((*itr)->name, name) == 0)
         return (*itr);
   if (create)
   {
      channels.push_back(new Channel(name));
      Channel *c = channels.last();
      if (*name != '#' && *name != '&' && (stricmp(name,CHANNEL_STATUS) != 0))
         c->flags |= CHANNEL_PRIVATE;
      return (c);   
   }
   return (NULL);
}   


//------------------------------------------------------------------------------
void IRCClient::newMessage(Channel *c, DWORD tag, ...)
{
   if (!c)
      c = findChannel(CHANNEL_STATUS, true);   //pick the STATUS channel

   char buffer[2048];
   va_list pArgs;
   va_start(pArgs, tag);
   const char *message = SimTagDictionary::getString(manager, tag);
   vsprintf(buffer, message, pArgs);

   newMessage(c, buffer);
}   

//------------------------------------------------------------------------------
void IRCClient::newMessage(Channel *c, const char *message)
{
   char szLog[MAX_PACKET];

   if (!c)
   {
      c = findChannel(CHANNEL_STATUS, true);   //pick the STATUS channel
   }

   c->flags.set( CHANNEL_NEWMESSAGE );
   /*
   c->messages.push_back(new Message(message));
   if (c->messages.size() > MAX_MESSAGE_QUEUE)
   {
      delete c->messages.first();
      c->messages.pop_front();
   }
   */
   c->messages.push(new Message(message));

   if (c->messages.size() > iMsgHistory)
   {
      c->messages.pop();
   }

   if (logFile)
   {
      sprintf(szLog, "%s: %s\n", c->name, message);
      logFile->write(strlen(szLog), szLog);
   }

   switch (echo)
   {
      case ECHO_CURRENT:
         if (c == currentChannel) 
            Console->printf(message); break;

      case ECHO_STATUS:
         if (c == findChannel(CHANNEL_STATUS)) 
            Console->printf(message); break;

      case ECHO_ALL:
         Console->printf(message); break;
   }
}   

//------------------------------------------------------------------------------
void IRCClient::onPing(char *prefix, char *params)
{  
   float rTime = manager->getCurrentTime();

   if ((rTime - rLastPinged) >= PING_FLOOD_TIMEOUT)
   {
      rLastPinged = rTime;

      if (prefix && *prefix)
      {
         char *nick;
         nextToken(prefix, nick, "!");
         Person *p = findPerson(nick);
         if (p)
         {
            if (p->ping)
            {
               newMessage(currentChannel, IDIRC_PING, nick, (GetTickCount()-p->ping)/1000);
               p->ping = 0;
            }
            else
            {
               char *key;
               params = nextToken(params, nick, " :");
               nextToken(params, key, " \x01");
               _sendf("NOTICE %s :" "\x01" "PING %s" "\x01", nick, key);
            }
         }
      }
      else
         _sendf("PONG %s", params);
   }
}   


//------------------------------------------------------------------------------
void IRCClient::onPong(char *prefix, char *params)
{  prefix;
   // EXAMPLE :rick-266.dynamix.com PONG rick-266.dynamix.com :ricko
   char *nick;
   nextToken(params, nick, " ");      
   Person *p = findPerson(nick);
   if (p && p->ping)
   {
      newMessage(currentChannel, IDIRC_PING, nick, (GetTickCount()-p->ping)/1000);
      p->ping = 0;
   }
}   

//------------------------------------------------------------------------------
void IRCClient::onJoin(char *prefix, char *params)
{
   Person  *p = findPerson(prefix, true);
   Channel *c = findChannel(params, true);

   c->join(p);

   if (Console->getBoolVariable("$IRC::ShowJoinLeave") && !(p->flags & PERSON_IGNORE))
   {
      newMessage(c, IDIRC_JOIN, p->nick, 
         (p->identity && *(p->identity)) ? p->identity : p->nick);
   }

   // if this is me then set this as the current channel
   if (p == *people.begin())
   {
      currentChannel = c;   
      _sendf("MODE %s", currentChannel->name);
   }
}   

//------------------------------------------------------------------------------
void IRCClient::onPrivMsg(char *prefix, char *params)
{
   char *ch, *nick, *msg;
   Channel *c;
   Person  *p = *people.begin();
   params = nextToken(params, ch, " ");
   msg    = params;
   
   //messages always lead with a ':'
   char *temp = strchr(msg, ':');
   if (temp) msg = &temp[1];

   nick = ch;
   if (*ch == '@' || *ch == '+')
      nick++;
   if (stricmp(nick, p->nick) == 0)
      nextToken(prefix, ch, " !");
   
   // are we IGNORING this person?
   p = findPerson(prefix, true);
   if (p && (p->flags & PERSON_IGNORE))
      return;

   if (*msg != '\x01')
   {
      c = findChannel(ch, true);
      if ( !c )
         return;
      newMessage(c, IDIRC_SAYS, p->nick, msg);
   }
   else
   {
      // otherwise it's a command imbeded inside PRIVMSG (oh great!)
      char *command, buffer[MAX_PACKET];
      msg = nextToken(msg, command, " \x01");
      sprintf(buffer, "%s: %s", ch, msg);
      dispatch(prefix, command, buffer);
   }
}   

//------------------------------------------------------------------------------
void IRCClient::onNick(char *prefix, char *params)
{
   Channel *channel;
   Person  *person;
   
   if ((person = findPerson(prefix, false)) != NULL)
   { 
      if (!(person->flags & PERSON_IGNORE))
      {
         newMessage(currentChannel, IDIRC_NICK, person->nick, params);
      }

      if ((channel = findChannel(person->nick)) != NULL)
      {
         if (channel->name)
         {
            delete [] channel->name;
         }
         channel->name = strnew(params); 
      }

      person->setNick(params);

      // If this is me, re-set the console variable
      if (person == *people.begin())
      {
         Console->setVariable("$IRC::NickName", person->nick);
      }
   }
}   

//------------------------------------------------------------------------------
void IRCClient::onQuit(char *prefix, char *params)
{
   params;
   Channel *c;
   Person  *p = findPerson(prefix, false);

   if (p)
   {
      // For every channel
      VectorPtr<Channel*>::iterator citr = channels.begin();
      for (; citr != channels.end(); citr++)
      {
         if((*citr)->part(p))
         {
            //newMessage(*citr, IDIRC_QUIT, p->nick, p->identity);
         }
      }

      // If this person's name is a channel, we are private chatting
      // with them
      if ((c = findChannel(p->nick)) != NULL)
      {
         // Print notice that other person has left, if we are not ignoring them
         if (!(p->flags & PERSON_IGNORE))
         {
            newMessage(c, IDIRC_PART_PRIVATE, p->nick,
               (p->identity && (*p->identity)) ? p->identity : p->nick);
         }

         /*
         // Clean up the channel
         VectorPtr<Channel*>::iterator iter = channels.begin();

         for (Channel *prev = *iter; iter != channels.end(); prev = *iter, iter ++)
         {
            if (*iter == c)
            {
               channels.erase(iter);

               if (currentChannel == c)
               {
                  currentChannel = prev;
               }
               break;
            }
         }
         */
      }

      // clean up the Person
      AssertFatal(p->ref == 0, "IRCClient::onQuit: Person ref counnt not ZERO.");
      VectorPtr<Person*>::iterator pitr = people.begin();
      for (; pitr != people.end(); pitr++)
      {
         if (*pitr == p)
         {
            people.erase(pitr);
            delete p;
            break;
         }
      }
   }
}   

//------------------------------------------------------------------------------
void IRCClient::onError(char *prefix, char *params)
{
   prefix, params;
   newMessage(currentChannel, IDIRC_ERROR, params);
   notify(IDIRC_ERROR);
   disconnect();
}   


//------------------------------------------------------------------------------
void IRCClient::onUsers(char *prefix, char *params)
{
   prefix, params;
   
}   

//------------------------------------------------------------------------------
void IRCClient::onMOTD(char *prefix, char *params)
{  // command 372   
   // EXAMPLE :StLouis.MO.US.UnderNet.org 372 homer128 :- ==> Disclaimer/ Rules:
   char *msg  = nextToken(params, prefix, ":");
   newMessage(currentChannel, IDIRC_MOTD, msg);
}   

//------------------------------------------------------------------------------
void IRCClient::onMOTDEnd(char *prefix, char *params)
{  // command 372   
   // EXAMPLE :StLouis.MO.US.UnderNet.org 372 homer128 :- ==> Disclaimer/ Rules:
   prefix, params;
   if (state == IDIRC_CONNECTING_WAITING)
   {
      state = IDIRC_CONNECTING_DONE;
      onSimFrameEndNotifyEvent(NULL);
   }
}   

//------------------------------------------------------------------------------
void IRCClient::onNameReply(char *prefix, char *params)
{  // command 353   
   // EXAMPLE homer128 = #Starsiege :homer128 Fusion^WP KM-UnDead Rick-wrk Lord-Star Apoc0410
   prefix;
   char *nick; 
   params = nextToken(params, nick, " =@");
   Person *me = findPerson(nick);
   AssertFatal(me, "IRCClient::onNameReply: My nick should have existed.");

   // find the end of channel name
   char *ch = params;
   params = nextToken(params, ch, " :");
   Channel *c = findChannel(ch);
   //AssertFatal(c, "IRCClient::onNameReply: This channel should have existed.");
   if (!c) return;

   // loop through the nick and add them to the channel
   while (*params)
   {
      params = nextToken(params, nick, " ");
      Person *p = findPerson(nick, true);

      DWORD flags = 0;
      if (*nick == '@')
      {
         flags |= PERSON_OPERATOR;
      }
      else if (*nick == '+')
      {
         flags |= PERSON_SPEAKER;
      }

      // If it's not me, add them to the channel
      if (p != me)
      {
         c->join(p, flags);
      }
      // If it is me, just set the flags
      else
      {
         PersonInfo *info = c->findPerson(nick);
         
         if (info)
         {
            info->flags = flags;
         }
      }
   }
}      

//------------------------------------------------------------------------------
void IRCClient::onWhoReply(char *prefix, char *params)
{  prefix;
   // command 352   
   // EXAMPLE: :StLouis.MO.US.UnderNet.org 352 homer128 #Starsiege ~DrAwkward 198.74.39.31 los-angeles.ca.us.undernet.org DrAwkward H :3 JR
   char *me, *ch, *user, *at, *server, *nick, *HG, *hops, *real;
   params = nextToken(params, me, " ");
   params = nextToken(params, ch, " ");
   params = nextToken(params, user, " ");
   params = nextToken(params, at, " ");
   params = nextToken(params, server, " ");
   params = nextToken(params, nick, " *+@");
   params = nextToken(params, HG, " ");
   params = nextToken(params, hops, " ");
   nextToken(params, real, " ");

   Person *p = findPerson(nick);
   if (!p) return;

   // update person in question
   p->setReal(real);
   p->setIdent(avar("%s@%s",user, at));

   // Send it to the status channel
   newMessage(NULL, IDIRC_WHO_REPLY, nick, user, at, real, server);
}

void IRCClient::onEndOfWho(char *, char *)
{
   notify(IDIRC_END_OF_WHO);
   newMessage(NULL, IDIRC_END_OF_WHO);
}

//------------------------------------------------------------------------------
void IRCClient::onPart(char *prefix, char *params)
{
   Person  *p = findPerson(prefix, false);
   Channel *c = findChannel(params);

   if (p && c)
   {
      if (c->part(p) && Console->
          getBoolVariable("$IRC::ShowJoinLeave") && !(p->flags & PERSON_IGNORE))
      {
         newMessage(c, IDIRC_PART, p->nick, 
            (p->identity && (*p->identity)) ? p->identity : p->nick);
      }
   }

   // if this was me parting clean up the channel
   if (p == *people.begin())
   {
      VectorPtr<Channel*>::iterator itr = channels.begin();
      Channel *prev = *itr;
      for (; itr != channels.end(); prev = *itr, itr++)
         if (*itr == c)
         {
            channels.erase(itr);
            if (currentChannel == c)
               currentChannel = prev;
            return;
         }
   }
}   

void IRCClient::onNoTopic(char *, char *params)
{
   char *channel;

   params = nextToken(params, channel, " ");
   params = nextToken(params, channel, " ");

   // Just a message
   newMessage(currentChannel, IDIRC_NO_TOPIC, channel);
}

//------------------------------------------------------------------------------
void IRCClient::onTopic(char *prefix, char *params)
{  prefix;
   //  332 EXAMPLE :rick-266.dynamix.com 332 homer128 #starsiege :this is the topic
   //TOPIC EXAMPLE :RickO!ricko@rick-266.dynamix.com TOPIC #starsiege :this is the topic
   char *ch;
   params = nextToken(params, ch, " :");
   Channel *c = findChannel(ch);
   if (!c)
   {
      params = nextToken(params, ch, " :");
      c = findChannel(ch);
   }

   if (!c)
      return;
   c->setTopic(params);
}  


//------------------------------------------------------------------------------
void IRCClient::onKick(char *prefix, char *params)
{
   // EXAMPLE: BarbieGrl!Forgot@tsbrk3-204.gate.net KICK #HackPhreak RiseR :0,1 Shitlisted 
   char *ch, *nick, *host;
   nextToken(prefix, host, " !");
   params = nextToken(params, ch, " ");
   params = nextToken(params, nick, " :");

   Person  *p = findPerson(nick);
   Channel *c = findChannel(ch);
   if (p && c)
   {
      if (c->part(p))
      {
         // was it me?
         if (p == *people.begin())
         {  // if so delete all the members
            while(c->members.size())
            {
               p = c->members.last().person;
               p->decRef();
               c->members.pop_back();
            }
            // Also gotta delete the channel
            VectorPtr<Channel*>::iterator itr = channels.begin();
            for (Channel *prev = *itr; itr != channels.end(); prev = *itr, itr++)
            {
               if (*itr == c)
               {
                  channels.erase(itr);
                  if (currentChannel == c)
                     currentChannel = prev;
                  break;
               }
            }
            statusMessage(IDIRC_KICK, host, nick, params);
            notify(IDIRC_KICK);
         }
         else
         {
            newMessage(currentChannel, IDIRC_KICK, host, nick, params);
         }
      }
   }
}   

void IRCClient::onModeReply(char *prefix, char *params)
{
   char *lpszPerson;

   // Strip the person name
   params = nextToken(params, lpszPerson, " ");

   // Send the rest on to the usual handler
   onMode(prefix, params);
}

//------------------------------------------------------------------------------
void IRCClient::onMode(char *prefix, char *params)
{
   // EXAMPLE: :RickO!ricko@rick-266.dynamix.com MODE #starsiege +v homer128
   // EXAMPLE: PACKET: :RickO!ricko@rick-266.dynamix.com MODE #starsiege +m
   // EXAMPLE: PACKET: :RickO!ricko@rick-266.dynamix.com MODE #starsiege -m
   // EXAMPLE: PACKET: :RickO!ricko@rick-266.dynamix.com MODE #starsiege +lo 50 nick
   // EXAMPLE: PACKET: :RickO!ricko@rick-266.dynamix.com MODE #starsiege +l 500
   // EXAMPLE: PACKET: :RickO!ricko@rick-266.dynamix.com MODE #starsiege -l
   // EXAMPLE: PACKET: :RickO!ricko@rick-266.dynamix.com MODE #starsiege +s
   // EXAMPLE: PACKET: :RickO!ricko@rick-266.dynamix.com MODE #starsiege -s
   // EXAMPLE: PACKET: :RickO!ricko@rick-266.dynamix.com MODE #starsiege +p
   // EXAMPLE: PACKET: :RickO!ricko@rick-266.dynamix.com MODE #starsiege -p

   char *ch, *mode, *arg;
   PersonInfo *p;
   newMessage(NULL, params );
   params = nextToken(params, ch, " ");
   params = nextToken(params, mode, " ");

   bool enable = (*mode++ == '+');
   Channel *c = findChannel(ch);
   if (!c) return;

   while (*mode)
   {
      switch (*mode)
      {
         // PERSON Mode command
         case 'o':   // Operator
            params = nextToken(params, arg, " ");
            p = *arg ? c->findPerson(arg) : NULL;
            if (!p)
               break;
            p->flags.set(PERSON_OPERATOR, enable);
            // only display the message if my privilages are modified.
            if (p->person != *people.begin())
               break;

            nextToken(prefix, arg, "!");
            switch (p->flags & (PERSON_OPERATOR|PERSON_SPEAKER))
            {
               case PERSON_OPERATOR: newMessage(c, IDIRC_OPERATOR, arg); break;
               case PERSON_SPEAKER:  newMessage(c, IDIRC_SPEAKER, arg); break;
               default:              newMessage(c, IDIRC_SPECTATOR, arg); break;
            }
            c->sort();
            break;

         case 'v':   // Speaker (voice)
            params = nextToken(params, arg, " ");
            p = *arg ? c->findPerson(arg) : NULL;
            if (!p)
               break;
            p->flags.set(PERSON_SPEAKER, enable);

            // only display the message if my privilages are modified.
            if (p->person != *people.begin())
               break;

            nextToken(prefix, arg, "!");
            switch (p->flags & (PERSON_OPERATOR|PERSON_SPEAKER))
            {
               case PERSON_OPERATOR: newMessage(c, IDIRC_OPERATOR, arg); break;
               case PERSON_SPEAKER:  newMessage(c, IDIRC_SPEAKER, arg); break;
               default:              newMessage(c, IDIRC_SPECTATOR, arg); break;
            }
            c->sort();
            break;

         // CHANNEL Mode command
         case 'b':   // Ban
            params = nextToken(params, arg, " ");
            break;

         case 'i':   // Channel is Invite only
            c->flags.set(CHANNEL_INVITE, enable);
            notify(IDIRC_CHANNEL_FLAGS);
            break;

         case 'l':   // Channel has limited members
            params = nextToken(params, arg, " ");
            c->personLimit = enable ? atoi(arg) : 0;
            c->flags.set(CHANNEL_LIMITED, enable);
            notify(IDIRC_CHANNEL_FLAGS);
            break;

         case 'm':   // Channel is moderated
            c->flags.set(CHANNEL_MODERATED, enable);
            notify(IDIRC_CHANNEL_FLAGS);
            break;

         case 'n':   // External messages are ignored
            c->flags.set(CHANNEL_IGNORE_EXTERN, enable);
            notify(IDIRC_CHANNEL_FLAGS);
            break;

         case 'p':   // Channel is Private
            c->flags.set(CHANNEL_PRIVATE, enable);
            notify(IDIRC_CHANNEL_FLAGS);
            break;

         case 's':   // Channel is Secret
            c->flags.set(CHANNEL_SECRET, enable);
            notify(IDIRC_CHANNEL_FLAGS);
            break;

         case 't':   // Channel topic limited to Operators
            c->flags.set(CHANNEL_TOPIC_LIMITED, enable);
            notify(IDIRC_CHANNEL_FLAGS);
            break;

         case 'k':   // Channel has secret key
            params = nextToken(params, arg, " ");
            c->setKey(enable ? arg : NULL);
            c->flags.set(CHANNEL_HAS_KEY, enable);
            notify(IDIRC_CHANNEL_FLAGS);
            break;

         default:
            break;
      }
      mode++;
   }
}  

//------------------------------------------------------------------------------
void IRCClient::onWhoisReply(char *prefix, char *params)
{  prefix;
   // EXAMPLE :rick-266.dynamix.com 311 homer128 RickO ricko rick-266.dynamix.com 198.74.38.222 :Rick
   // EXAMPLE :rick-266.dynamix.com 319 homer128 RickO :@#starsiege 
   // EXAMPLE :rick-266.dynamix.com 312 homer128 RickO rick-266.dynamix.com :WIRCSRV Windows IRC Server
   char *ch;
   params = nextToken(params, ch, " ");
   newMessage(NULL, IDIRC_MOTD, params);
}   

void IRCClient::onWhoisUserReply(char *, char *params)
{
   Person *person;
   char    szBuf[256];
   char   *chunk, *nick, *ident, *real, *tmp;

   params = nextToken(params, chunk, " ");
   tmp    = strnew(params);
   params = nextToken(params, nick,  " ");
   params = nextToken(params, chunk, " ");
   params = nextToken(params, ident, " ");
   sprintf(szBuf, "%s@%s", chunk, ident);
   real   = nextToken(params, chunk, ":");

   if ((person = findPerson(nick)) != NULL)
   {
      person->setIdent(szBuf);
      person->setReal(real);
   }

   newMessage(NULL, IDIRC_MOTD, tmp);
   delete [] tmp;
}   

//------------------------------------------------------------------------------
void IRCClient::onWhoisIdle(char *prefix, char *params)
{  prefix;
   // EXAMPLE :rick-266.dynamix.com 317 homer128 RickO 5350 907453369 :seconds idle
   // the date is encoded in '907453369' but I have not figured out how to decode it :(
   char *nick, *seconds, *date;
   params = nextToken(params, nick, " "); // strip out caller's name
   params = nextToken(params, nick, " ");
   params = nextToken(params, seconds, " ");
   params = nextToken(params, date, " ");
   int iSeconds, iMinutes;
   iSeconds = atoi(seconds);
   iMinutes = iSeconds/60;
   iSeconds = iSeconds%60;
   if (iMinutes)
      newMessage(NULL, IDIRC_WHOIS_IDLE_M, nick, iMinutes, iSeconds, params);
   else
      newMessage(NULL, IDIRC_WHOIS_IDLE_S, nick, iSeconds, params);
}   

//------------------------------------------------------------------------------
bool IRCClient::censor(const char *str)
{
   if (!str) return (false);
   if (censorWords.size() == 0)
   {
      const char *str = Console->getVariable("$IRC::censorChannel");
      if (str)
      {
         char buffer[MAX_PACKET];
         char *p = buffer;
         char *word; 

         strncpy(buffer, str, MAX_PACKET);
         buffer[MAX_PACKET] = 0;
         strlwr(buffer);

         while (*p)
         {
            p =nextToken(p, word, " ,");
            censorWords.push_back(strnew(word));
         }
      }
   }

   char buffer[MAX_PACKET+1];
   strncpy(buffer, str, MAX_PACKET);
   buffer[MAX_PACKET] = 0;
   strlwr(buffer);

   VectorPtr<char*>::iterator itr = censorWords.begin();
   for (; itr != censorWords.end(); itr++)
   {
      if (strstr(buffer, *itr))
         return (true);
   }
   return (false);
}   


//------------------------------------------------------------------------------
void IRCClient::onList(char *prefix, char *params)
{  prefix;
   //EXAMPLE: :StLouis.MO.US.UnderNet.org 322 homer128 #bmx 9 :BMX Rules!
   
   char *nick, *ch, *users, *topic;
   params = nextToken(params, nick, " ");
   params = nextToken(params, ch, " ");
   topic  = nextToken(params, users, " :");
   
   if (censor(topic))
      return;

   if (silentList)
      channelList.push_back(new Channel(ch, topic));
   else
   {
      newMessage(currentChannel, IDIRC_CHANNEL, atoi(users), ch, topic ? topic : "");
   }
}  

void IRCClient::onBanList(char *, char *params)
{
   char *banned;

   params = nextToken(params, banned, " ");
   params = nextToken(params, banned, " ");
   params = nextToken(params, banned, " ");

   if (silentBanList)
   {
      banList.push_back(strnew(banned));
   }
}

//------------------------------------------------------------------------------
void IRCClient::onListEnd(char *prefix, char *params)
{  prefix, params;
   if (silentList)
   {
      silentList = false;
      notify(IDIRC_CHANNEL_LIST);
   }
}  

void IRCClient::onBanListEnd(char *prefix, char *params)
{
   prefix, params;
   if (silentBanList)
   {
      silentBanList = false;
      notify(IDIRC_BAN_LIST);
   }
}
//------------------------------------------------------------------------------
void IRCClient::onBadNick(char *prefix, char *params)
{  prefix, params;
   newMessage( NULL, IDIRC_ERR_NICK_IN_USE, (*people.begin())->nick );
   //notify( IDIRC_ERR_NICK_IN_USE );
}  


//------------------------------------------------------------------------------
void IRCClient::onAway(char *prefix, char *params)
{  
   Person *p = findPerson(prefix);
   if (!p)
      return;
   char *msg;
   msg = nextToken(params, msg, " :");

   if (*msg)
   {
      p->flags |= PERSON_AWAY;
      newMessage(currentChannel, IDIRC_AWAY, prefix, msg);
   }
   else
   {
      p->flags &= ~PERSON_AWAY;
      newMessage(currentChannel, IDIRC_RETURNED, prefix);
   }
}  


//------------------------------------------------------------------------------
void IRCClient::onAction(char *prefix, char *params)
{  
   char *ch, *msg;
   Channel *c;

   msg = nextToken(params, ch, " :");
   c = findChannel(ch, true);

   newMessage(c, IDIRC_ACTION, prefix, msg);
}  


//------------------------------------------------------------------------------
void IRCClient::onAwayReply(char *prefix, char *params)
{  prefix;
   //EXAMPLE :rick-266.dynamix.com 301 homer128 RickO :Gone fishing.   
   char *nick;
   params = nextToken(params, nick, " :");
   newMessage(NULL, IDIRC_AWAY, nick, params);
}  

void IRCClient::onUnAwayReply(char *, char *)
{
   newMessage(NULL, IDIRC_UNAWAY_REPLY);
}

void IRCClient::onNowAwayReply(char *, char *)
{
   newMessage(NULL, IDIRC_NOWAWAY_REPLY);
}

//------------------------------------------------------------------------------
void IRCClient::onNotice(char *prefix, char *params)
{
   //EXAMPLE NOTICE AUTH :*** Found your hostname

   char   *lpszSender;
   Person *sender;

   sender = findPerson(prefix, true);

   if ((sender == NULL) || !(sender->flags & PERSON_IGNORE))
   {
      char *msg = strchr(params, '\x01');
      if (!msg)
      {
         // Skip past the target name
         params = nextToken(params, params, ":");

         newMessage(NULL, IDIRC_NOTICE, prefix, params);
      }
      else
      {
         // otherwise it's a command imbeded inside NOTICE (oh great!)
         char *command;
         msg = nextToken(msg, command, " \x01");
         dispatch(prefix, command, msg);
      }
   }
}  


//------------------------------------------------------------------------------
void IRCClient::onChannelFull(char *, char *params)
{  
   char *channel;
   channel = nextToken(params, channel, " ");
   nextToken(channel, channel, " ");

   statusMessage(IDIRC_CHANNEL_FULL, channel);
   notify(IDIRC_CHANNEL_FULL);
}  


//------------------------------------------------------------------------------
void IRCClient::onChannelInviteOnly(char *, char *params)
{  
   char *channel;
   channel = nextToken(params, channel, " ");
   nextToken(channel, channel, " ");

   statusMessage(IDIRC_INVITE_ONLY, channel);
   notify(IDIRC_INVITE_ONLY);
}  


void IRCClient::onInvite(char *prefix, char *params)
{
   char    *lpszChannel;
   Person  *person;

   // Find or create the person (should never be NULL)
   if ((person = findPerson(prefix, true)) != NULL)
   {
      params      = nextToken(params, lpszChannel, ":");
      lpszChannel = params;

      // Only bother the user if they aren't ignoring this person
      if (!(person->flags & PERSON_IGNORE))
      {
         // Set vars and notify the responder
         Console->setVariable("$IRC::InviteChannel", lpszChannel);
         Console->setVariable("$IRC::InvitePerson",  person->nick);
         notify(IDIRC_INVITED);
      }
   }
}

void IRCClient::onInviteReply(char *, char *params)
{
   char *channel;
   char *person;

   params = nextToken(params, channel, " ");
   params = nextToken(params, channel, " ");
   params = nextToken(params, person,  " ");

   newMessage(currentChannel, IDIRC_INVITE_REPLY, channel, person);
}

//------------------------------------------------------------------------------
void IRCClient::onChannelBanned(char *, char *params)
{  
   char *channel;
   channel = nextToken(params, channel, " ");
   nextToken(channel, channel, " ");

   statusMessage(IDIRC_BANNED_CH, channel);
   notify(IDIRC_BANNED_CH);
}  


//------------------------------------------------------------------------------
void IRCClient::onServerBanned(char *, char *)
{  
   state = IDIRC_DISCONNECTED;

   statusMessage(IDIRC_BANNED_SERVER,NULL); 
   //notify(IDIRC_BANNED_SERVER);
}  

void IRCClient::onInvalidNick(char *, char *params)
{
   char *name;
   state = IDIRC_DISCONNECTED;
   params = nextToken(params, name, ":");
   newMessage(NULL, IDIRC_MOTD, params);
}

void IRCClient::onBadChannelKey(char *, char *params)
{
   char *channel;
   
   channel = nextToken(params, channel, " ");
   nextToken(channel, channel, " ");

   statusMessage(IDIRC_CHANNEL_HAS_KEY, channel);
   notify(IDIRC_CHANNEL_HAS_KEY);
}


//------------------------------------------------------------------------------
void IRCClient::onVersion(char *prefix, char *params)
{  
   float rTime = manager->getCurrentTime();

   if ((rTime - rLastVersioned) >= VERSION_FLOOD_TIMEOUT)
   {
      rLastVersioned = rTime;

      char *msg, *nick;
   
      nextToken(prefix, nick, "!");
      params = nextToken(params, msg, ": ");
      nextToken(params, msg, "\x01");
      Person *p = findPerson(nick);
      if (p && *msg)
         newMessage(currentChannel, IDIRC_VERSION, nick, msg);
      else
         _sendf("NOTICE %s :" "\x01" "VERSION " VERSION_STRING "\x01", prefix);
   }
}  



//------------------------------------------------------------------------------
void IRCClient::send(const char *message, const char *to)
{
   if ((state != IDIRC_CONNECTING_IRC && state != IDIRC_CONNECTED) || 
       !message)
      return;

   if (!*message)   
      return;

   if (*message == '/')   
   {
      char *command, *params;
      char buffer[MAX_PACKET+1];
      strncpy(buffer, message+1, MAX_PACKET);
      buffer[MAX_PACKET] = 0;

      params = nextToken(buffer, command, " ");

      // dispatch the command to a handler -- if one exists
      OutHandlerVec::iterator itr = outgoing.begin();
      for (; itr != outgoing.end(); itr++)
         if (stricmp((*itr)->cmd, command) == 0)
         {
            (this->*((*itr)->fn))(params);
            break;
         }

      // otherwise ship it to the server, RAW
      if (itr == outgoing.end())
         _send(message+1);
   }
   else
   {
      if (to)
      {
         _sendf("PRIVMSG %s :%s", to, message);
         Person *p = findPerson(to);
         if (p)      
         {
            Channel *c = findChannel(p->nick);
            Person *me = *people.begin();
            newMessage(c, IDIRC_SAYS_ME, me->nick, message);
         }
      }
      else
      {
         if (currentChannel && currentChannel != findChannel(CHANNEL_STATUS))
         {
            _sendf("PRIVMSG %s :%s", currentChannel->name, message);
            Person *me = *people.begin();
            newMessage(currentChannel, IDIRC_SAYS_ME, me->nick, message);
         }
      }
   }
} 


//------------------------------------------------------------------------------
void IRCClient::sendAction(const char *message)
{
   if ((state != IDIRC_CONNECTING_IRC && state != IDIRC_CONNECTED) || 
       !message)
      return;

   if (!*message)         return;

   if (*message == '/')   
      send(message, NULL);
   else
   {
      _sendf("PRIVMSG %s :" "\x01" "ACTION %s" "\x01", currentChannel->name, message);
      Person *me = *people.begin();
      newMessage(currentChannel, IDIRC_ACTION, me->nick, message);
   }
}   
  
void IRCClient::join(const char *params)
{
   Channel *channel;
   char    *lpszChannel, *tmp, *key;

   tmp = nextToken(const_cast<char *>(params), lpszChannel, " ");
   nextToken(tmp, key, " ");

   // Only allow one channel / key combination at a time - no
   // comma delimiting
   nextToken(lpszChannel, lpszChannel, ",");
   nextToken(key, key, ",");

   if ((channel = findChannel(lpszChannel)) != NULL)
   {
      currentChannel = channel;
   }
   else
   {
      _sendf("JOIN %s %s", lpszChannel, key);
   }
}

void IRCClient::quit(const char *)
{
   _sendf("QUIT");

   disconnect();
}

//------------------------------------------------------------------------------
void IRCClient::nick(const char *nick)
{
   if ((state == IDIRC_CONNECTED || state == IDIRC_CONNECTING_IRC) && nick)
   {
      if ( stricmp(nick, people.first()->nick) )
         _sendf("NICK %s", nick);
   }
   else
   {
      (*people.begin())->setNick(nick);
      Console->setVariable("$IRC::NickName", (*people.begin())->nick);
   }
}  

 
//------------------------------------------------------------------------------
void IRCClient::name(const char *name)
{
   if (people.size())
   {
      Person *me = *people.begin();
      me->setReal(name);
   }
}   


//------------------------------------------------------------------------------
void IRCClient::part(const char *params)
{
   char *ch;
   nextToken(const_cast<char*>(params), ch, " ");
   if ( stricmp(ch, CHANNEL_STATUS) == 0 )
      return;

   Channel *c = findChannel(ch);
   if (c)
   {
      VectorPtr<Channel*>::iterator itr = channels.begin();
      Channel *prev = *itr;
      for (; itr != channels.end(); prev = *itr, itr++)
         if (*itr == c)
         {
            if (*ch != '#' && *ch != '&')
            {
               channels.erase(itr);
               if (currentChannel == c)
                  currentChannel = prev;
            }
            else
               _sendf("PART %s", params);
            return;
         }
   }
}   

//------------------------------------------------------------------------------
void IRCClient::away(const char *params)
{
   Person *me = *people.begin();
   if (params && *params)
   {
      me->flags |= PERSON_AWAY;         
      _sendf("AWAY %s", params);
      _sendf("PRIVMSG %s :" "\x01" "AWAY %s" "\x01", currentChannel->name, params);
   }
   else
   {
      me->flags &= ~PERSON_AWAY;         
      _sendf("AWAY");
      _sendf("PRIVMSG %s :" "\x01" "AWAY" "\x01", currentChannel->name);
   }
}   

//------------------------------------------------------------------------------
void IRCClient::ping(const char *params)
{
   char *nick;
   params = nextToken(const_cast<char*>(params), nick, " ");
   while (*nick)
   {
      Person *p = findPerson(nick);
      if (p)
      {
         p->ping = GetTickCount();
         _sendf("PRIVMSG %s :" "\x01" "PING 0" "\x01", nick);
      }
      params = nextToken(const_cast<char*>(params), nick, " ");
   }
}   


//------------------------------------------------------------------------------
void IRCClient::setOperator(const char *nick)
{
   Person *p = findPerson(nick);
   if (p)
   {
      _sendf("MODE %s +o %s", currentChannel->name, nick);
   }
}   
                                                         

//------------------------------------------------------------------------------
void IRCClient::setSpeaker(const char *nick)
{
   if (currentChannel)
   {
      PersonInfo *info = currentChannel->findPerson(nick);

      if (info)
      {
         if (info->flags & PERSON_OPERATOR)
            _sendf("MODE %s -o %s", currentChannel->name, nick);

         _sendf("MODE %s +v %s", currentChannel->name, nick);
      }
   }
}  

 
//------------------------------------------------------------------------------
void IRCClient::setSpectator(const char *nick)
{
   if (currentChannel)
   {
      PersonInfo *info = currentChannel->findPerson(nick);

      if (info)
      {
         if (info->flags & PERSON_OPERATOR)
            _sendf("MODE %s -o %s", currentChannel->name, nick);
         if (info->flags & PERSON_SPEAKER)
            _sendf("MODE %s -v %s", currentChannel->name, nick);
      }
   }
}   


//------------------------------------------------------------------------------
void IRCClient::ignore(const char *nick, bool tf)
{
   AssertFatal(currentChannel, "IRCClient::ignore: no current channel.");
   Person *p = findPerson(nick);
   if (p)   
   {
      if (tf)
      {
         p->flags |= PERSON_IGNORE;
         newMessage(NULL, IDIRC_IGNORE, nick);
      }
      else
      {
         p->flags &= ~PERSON_IGNORE;
         newMessage(NULL, IDIRC_NO_IGNORE, nick);
      }
   }
}   


//------------------------------------------------------------------------------
void IRCClient::requestChannelList()
{
   // clear the global channel List
   while (channelList.size())
   {
      delete channelList.last();
      channelList.pop_back();
   }

   silentList=true;
   _sendf("LIST");
}   

void IRCClient::requestBanList(const char *lpszChannel)
{
   // clear list
   while (banList.size())
   {
      delete (char*)banList.last();
      banList.pop_back();
   }

   silentBanList = true;
   _sendf("MODE %s +b", lpszChannel);
}


//------------------------------------------------------------------------------
void IRCClient::setCurrentChannel(Channel *c)
{
   currentChannel = c;   
}   

//------------------------------------------------------------------------------
void IRCClient::setCurrentChannel(const char* c)
{
   VectorPtr<Channel*>::iterator itr = channels.begin();
   for (; itr != channels.end(); itr++)
      if(stricmp((*itr)->name, c) == 0)
         setCurrentChannel(*itr);
   return;
}   


//------------------------------------------------------------------------------
void IRCClient::setNotifyObject(SimObject *obj)
{
   if (notifyObject)   
      clearNotify(notifyObject);
   notifyObject = obj;
   deleteNotify(notifyObject);
}   


//------------------------------------------------------------------------------
void IRCClient::notify(DWORD msg)
{
   if (notifyObject)   
   {
      SimMessageEvent event;
      event.message = msg;
      notifyObject->processEvent(&event);
   }
}   

void IRCClient::statusMessage(DWORD dwTag, ...)
{
   Channel *c = findChannel(CHANNEL_STATUS, true);   //pick the STATUS channel

   char buffer[2048];
   va_list pArgs;
   va_start(pArgs, dwTag);
   const char *message = SimTagDictionary::getString(manager, dwTag);
   vsprintf(buffer, message, pArgs);

   newMessage(c, buffer);
}

void IRCClient::onJoinServer(const char *lpszMission, 
                             const char *lpszServer, 
                             const char *lpszAddress, 
                             bool        fPasswordMayBeRequired, 
                             bool        fPasswordRequired)
{
   char szBuf[512];

   // Server and address are required, and we have to be connected
   if (lpszServer && lpszAddress && state == IDIRC_CONNECTED)
   {
      sprintf(szBuf, 
         SimTagDictionary::getString(manager, IDIRC_JOIN_IP_MESSAGE), 
         lpszMission, lpszServer, lpszAddress, 
         fPasswordMayBeRequired ? 1 : 0, fPasswordRequired ? 1 : 0);

      _sendf("PRIVMSG %s :" "\x01" "ACTION %s" "\x01", currentChannel->name, szBuf);
      newMessage(currentChannel, IDIRC_ACTION, people[0]->nick, szBuf);
   }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
IRCClient::Person::Person(const char *_nick, const char *_real, const char *_ident)
{
   nick = "";
   real = "";
   identity = "";
   id = nextId++;
   setNick (_nick);
   setReal (_real);
   setIdent(_ident);
   ref   = 0;
   flags = 0;
   ping  = 0;
}

//------------------------------------------------------------------------------
IRCClient::Person::~Person()
{
   if (*real) delete [] real;
   if (*identity) delete [] identity;
}


//------------------------------------------------------------------------------
void IRCClient::Person::setNick(const char *_nick)
{
   if (!_nick) return;
   if (stricmp(nick, _nick))
   {
      if (*nick) delete [] nick;
      nick = (_nick && *_nick) ? strnew(_nick) : "";   

      // If this is me, set my nick
   }
}   

void IRCClient::Person::setReal(const char *_real)
{
   if (!_real) return;
   if (stricmp(real, _real))
   {
      if (*real) delete [] real;
      real = (_real && *_real) ? strnew(_real) : "";   
   }
}  
 
void IRCClient::Person::setIdent(const char *_ident)
{
   if (!_ident) return;
   if (stricmp(identity, _ident))
   {
      if (*identity) delete [] identity;
      identity = (_ident && *_ident) ? strnew(_ident) : "";
   }
}   


//------------------------------------------------------------------------------
IRCClient::Channel::Channel(const char *_name, const char *_topic)
{ 
   flags = CHANNEL_NEW;
   name  = strnew(_name); 
   topic = "";
   key   = "";
   id    = nextId++;
   personLimit = 0;     // 0 == no limit
   setTopic(_topic);
}


//------------------------------------------------------------------------------
IRCClient::Channel::~Channel()
{
   delete [] name;  
   if (*topic) delete [] topic; 
   if (*key) delete [] key;   
   reset();
}   


//------------------------------------------------------------------------------
void IRCClient::Channel::setTopic(const char *t)
{
   if (*topic) delete [] topic;   
   topic = ( t == NULL ) ? "" : strnew(t);
}   


//------------------------------------------------------------------------------
void IRCClient::Channel::setKey(const char *k)
{
   if (*key) delete [] key;   
   key = ( k == NULL ) ? "" : strnew(k);
}   


//------------------------------------------------------------------------------
bool IRCClient::Channel::join(Person *p, BYTE f)
{
   Vector<PersonInfo>::iterator itr = members.begin();
   for (; itr != members.end(); itr++)
      if (itr->person == p)
         break;

   if (itr == members.end())
   {
      members.increment();
      PersonInfo *pi = members.end()-1;
      pi->person = p;
      pi->flags  = f;
      p->incRef();
      sort();
      return (true);
   }
   return (false);
}   


//------------------------------------------------------------------------------
bool IRCClient::Channel::part(Person *p)
{
   Vector<PersonInfo>::iterator itr = members.begin();
   for (; itr != members.end(); itr++)
      if (itr->person == p)
      {
         members.erase(itr);
         p->decRef();
         return (true);
      }
   return (false);
}   


//------------------------------------------------------------------------------
void IRCClient::Channel::reset()
{
   // delete all the messages
   /*
   while (messages.size())
   {
      delete messages.last();
      messages.pop_back();
   } 
   */
   while (messages.size())
   {
      messages.pop();
   }

   flags &= ~ CHANNEL_NEWMESSAGE;
   members.clear();
}   


//------------------------------------------------------------------------------
IRCClient::PersonInfo* IRCClient::Channel::findPerson(const char *nick)
{
   if (*nick == '@' || *nick == '+')
      nick++;

   Vector<PersonInfo>::iterator itr = members.begin();
   for (; itr != members.end(); itr++)
      if (stricmp(itr->person->nick, nick) == 0)
         return (itr);
   return (NULL);
}   


//------------------------------------------------------------------------------
void IRCClient::Channel::sort()
{
   if (members.size())
      m_qsort((void *)&(members[0]), members.size(), sizeof(PersonInfo), comparePeople);
}   
