//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _IRCCLIENT_H_
#define _IRCCLIENT_H_


#include "simBase.h"
#include "simEv.h"
#include "stringTable.h"
#include "winsock.h"
#include "threadBase.h"
#include "tfifo.h"

// handy string parsing fn's
bool  isInSet(char c, char *set);
char* nextToken(char *str, char *&token, char *delim);

class IRCClient: public SimObject, private ThreadBase
{
public:
   typedef SimObject Parent;
   enum Constants {
      DEFAULT_PORT      = 6667,
      MAX_CHANNEL       = 200,
      MAX_PACKET        = 512,
      MAX_MESSAGE_QUEUE = 150,
      SZ_RECV_PACKET    = 2048,
      MIN_HISTORY       = 50,
      MAX_HISTORY       = 500,
      ECHO_NONE,
      ECHO_CURRENT,
      ECHO_STATUS,
      ECHO_ALL,
      
      TIMEOUT           = (1000*35),  // timeout in 35 seconds

      // Person flags
      PERSON_SPEAKER       = BIT(0),
      PERSON_OPERATOR      = BIT(1),
      PERSON_IGNORE        = BIT(2),
      PERSON_AWAY          = BIT(3),

      // Channel flags
      CHANNEL_PRIVATE      = BIT(0),
      CHANNEL_MODERATED    = BIT(1),
      CHANNEL_INVITE       = BIT(2),
      CHANNEL_LIMITED      = BIT(3),
      CHANNEL_NEWMESSAGE   = BIT(4),
      CHANNEL_IGNORE_EXTERN= BIT(5),
      CHANNEL_SECRET       = BIT(6),
      CHANNEL_TOPIC_LIMITED= BIT(7),
      CHANNEL_HAS_KEY      = BIT(8),
      CHANNEL_NEW          = BIT(9),
   };

   //--------------------------------------
   class Message
   {
      public :
         Message *next;
         static DWORD nextId;
         char *text;
         DWORD id;
         Message(const char *tx)
         {
            next = NULL;
            text = strnew(tx);
            id   = nextId++;
         }
         ~Message() { delete [] text; }
   };

   //--------------------------------------
   struct Person;
   struct PersonInfo
   {
      Person *person;
      BitSet32 flags;
   };

   struct Channel
   {
      static DWORD nextId;
      char *name;
      char *topic;
      char *key;
      BitSet32 flags;
      DWORD id;
      int personLimit;
      FIFO<Message> messages;
      Vector<PersonInfo>  members;
      Channel(const char *_name, const char *_topic=NULL);
      ~Channel();
      bool join(Person *p, BYTE f=0);
      bool part(Person *p);
      void reset();
      void setTopic(const char *t);
      void setKey(const char *k);
      bool newMessages()   { return (flags & CHANNEL_NEWMESSAGE); }
      bool isPrivate()     { return (flags & CHANNEL_PRIVATE); }
      void sort();
      PersonInfo* findPerson(const char *name);
   };

   //--------------------------------------
   struct Person
   {
      static DWORD nextId;
      char *nick;    // many servers only support nicks up to 9 characters
      char *real;
      char *identity;
      int  ref;
      BitSet32 flags;
      DWORD id;
      DWORD ping;
         
      Person(const char *_nick, const char *_real, const char *_ident);
      ~Person();
      void setNick(const char *_nick);
      void setReal(const char *_real);
      void setIdent(const char *_ident);
      void incRef()  { ref++; }
      void decRef()  { if (ref) ref--; }
   };
protected:
   DECLARE_PERSISTENT( IRCClient );
      
   SOCKET hSocket;
   FileWStream *logFile;
   const char *ircServerAddress;
   int  iMsgHistory;
   int  port;
   int  echo;
   int  retries;
   int  silentList;
   int  silentBanList;
   float rLastPinged;
   float rLastVersioned;
   DWORD connectTimeout;
   SimObject *notifyObject;

   // circular packet buffer vars
   char recvbuffer[SZ_RECV_PACKET+1];
   int bytesInBuffer;


   DWORD state;

   //--------------------------------------
   VectorPtr<Person*> people;
   VectorPtr<Channel*> channels;
   VectorPtr<Channel*> channelList;
   VectorPtr<const char *> banList;
   VectorPtr<char*> censorWords;
   Channel *currentChannel;
   void getClientAddress();

   //--------------------------------------
   typedef void (IRCClient::*ON_FN)(char *prefix, char *params);
   typedef void (IRCClient::*OUT_FN)(const char *params);

   struct OnHandler
   {
      char *cmd;
      ON_FN fn; 
      OnHandler(char *cs, ON_FN f) { cmd = cs; fn = f; }
   };

   struct OutHandler
   {
      char *cmd;
      OUT_FN fn; 
      OutHandler(char *cs, OUT_FN f) { cmd = cs; fn = f; }
   };

   typedef VectorPtr<OnHandler*> OnHandlerVec;
   typedef VectorPtr<OutHandler*> OutHandlerVec;
   OnHandlerVec  incomming;
   OutHandlerVec outgoing;

   //--------------------------------------
   // raw packet send routines
   void _sendf(const char *format, ...);
   void _send(const char *message);

   //--------------------------------------
   // incomming command handlers
   void onPing(char *prefix, char *params);
   void onPong(char *prefix, char *params);
   void onJoin(char *prefix, char *params);
   void onPrivMsg(char *prefix, char *params);
   void onNick(char *prefix, char *params);
   void onQuit(char *prefix, char *params);
   void onError(char *prefix, char *params);
   void onUsers(char *prefix, char *params);
   void onMOTD(char *prefix, char *params);           // 372
   void onMOTDEnd(char *prefix, char *params);        // 376
   void onNameReply(char *prefix, char *params);      // 353
   void onWhoReply(char *prefix, char *params);       // 352
   void onEndOfWho(char *, char *);
   void onPart(char *prefix, char *params);  
   void onLeave(char *prefix, char *params);  
   void onNoTopic(char *, char *);
   void onTopic(char *prefix, char *params);          // 331, 332
   void onKick(char *prefix, char *params);
   void onMode(char *prefix, char *params);
   void onList(char *prefix, char *params);           // 322
   void onBanList(char *prefix, char *params);
   void onListEnd(char *prefix, char *params);        // 323
   void onBanListEnd(char *prefix, char *params);
   void onBadNick(char *prefix, char *params);        // 433
   void onAway(char *prefix, char *params); 
   void onAction(char *prefix, char *params); 
   void onAwayReply(char *prefix, char *params);      // 301
   void onNotice(char *prefix, char *params);
   void onWhoisReply(char *prefix, char *params);
   void onWhoisUserReply(char *, char *);
   void onWhoisIdle(char *prefix, char *params);
   void onVersion(char *prefix, char *params);
   void onInvite(char *, char *);
   void onInviteReply(char *, char *);
   void onUnAwayReply(char *, char *);
   void onNowAwayReply(char *, char *);
   void onModeReply(char *, char *);
   void onBadChannelKey(char *, char*);
   void onChannelFull(char *, char *);
   void onChannelInviteOnly(char *, char *);
   void onChannelBanned(char *, char *);
   void onServerBanned(char *, char *);
   void onInvalidNick(char *, char *);

   int threadProcess();
   void recv();
   char* processPacket(char *buffer);

   void onGroupAdd();
   void onRemove();
   bool onSimFrameEndNotifyEvent(const SimFrameEndNotifyEvent*);
   void onDeleteNotify(SimObject *object);
   bool processEvent(const SimEvent*);
   void newMessage(Channel *c, DWORD tag, ...);
   virtual void newMessage(Channel *c, const char *messasge);
   void notify(DWORD msg);
   void reset();
   bool reconnect();
   bool censor(const char *str);
   bool dispatch(char* prefix, char* command, char* parmas);
   virtual void statusMessage(DWORD, ...);

public:
   IRCClient();
   ~IRCClient();

   inline const DWORD getState() const { return state; };
   inline int getHistorySize() { return iMsgHistory; }
   void connect(const char * ircServerAddress, int port=DEFAULT_PORT);
   void connect();
   void disconnect();

   Channel* findChannel(const char *name, bool create=false);
   Person*  findPerson(char *prefix, bool create);
   Person*  findPerson(const char *nick);
   VectorPtr<Channel*>* getChannels() { return &channels; }
   VectorPtr<Person*>*  getPeople() { return &people; }
   Person *             getMe()     { return (people.size() ? *people.begin() : NULL); }

   void requestChannelList();
   void requestBanList(const char *);
   VectorPtr<Channel*> *getChannelList() { return &channelList; }
   VectorPtr<const char *> *getBanList() { return &banList; }

   void setCurrentChannel(Channel *c);
   void setCurrentChannel(const char* c);
   void setEcho(int e) { echo = e; }
   void setNotifyObject(SimObject *obj);

   //--------------------------------------
   // Generic out going parser
   void send(const char *message, const char *to=NULL);
   void sendAction(const char *message);

   // Outgoing comamnd handlers
   virtual void join(const char *);
   virtual void quit(const char *);
   void nick(const char *nick);
   void name(const char *name);

   void part(const char *params);
   void away(const char *params);
   void ping(const char *params);
   void setOperator(const char *nick);
   void setSpeaker(const char *nick);
   void setSpectator(const char *nick);

   void ignore(const char *nick, bool tf);

   static IRCClient* find(SimManager *manager);

   void onJoinServer(const char *, const char *, const char *, bool, bool);
};


inline IRCClient* IRCClient::find(SimManager *manager)
{
   return static_cast<IRCClient*>(manager->findObject(IRCClientObjectId));  
}   


#endif //_IRCCLIENT_H_
