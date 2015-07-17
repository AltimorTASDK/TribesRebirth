
#include "console.h"
#include "simGuiTextFormat.h"
#include "fear.strings.h"
#include "fearGuiIRCClient.h"

namespace FearGui
{

IRCClient::IRCClient(void)
{
   //initialize the channels
   for (int i = 1; i < MAX_NUM_CHANNELS; i++)
   {
      mChannels[i].inUse = FALSE;
   }
   mSelectedChannel = -1;
   mForceVisibleChannel = -1;
}

IRCClient::~IRCClient(void)
{
   for (int i = 1; i < MAX_NUM_CHANNELS; i++)
   {
      clearChannel(i);
   }
}

bool IRCClient::onAdd(void)
{
   if (! Parent::onAdd()) return FALSE;
   
   //get the server name and port
   const char *prefServer = Console->getVariable("Pref::IRCServer");
   const char *prefPort = Console->getVariable("Pref::IRCPort");
   if ((! prefServer) || (! prefServer[0]))
   {
      strcpy(mServer, "irc.stomped.com");
      mPort = 6667;
   }
   else
   {
      strcpy(mServer, prefServer);
      if ((! prefPort) || (! prefPort[0])) mPort = 6667;
      else mPort = atoi(prefPort);
   }
   
   //get the name and nick
   const char *prefName = Console->getVariable("Pref::PlayerName");
   const char *prefRealName = Console->getVariable("Pref::IRCRealName");
   const char *prefNickname = Console->getVariable("Pref::IRCNickname");
   if ((! prefNickname) || (! prefNickname[0]))
   {
      if ((! prefName) || (! prefName[0]))
      {
         strncpy(mNickname, "TribePlyr", 9);
      }
      else
      {
         strncpy(mNickname, prefName, 9);
      }
   }
   else
   {
      strncpy(mNickname, prefNickname, 9);
   }
   mNickname[9] = '\0';
   
   //now make sure the nick name is valid
   //nicks may use letters, numbers, and these specials only: '-', '[', ']', '\', '`', '^', '{', '}'
   //first char must be a letter
   if (! ((mNickname[0] >= 'a' && mNickname[0] <= 'z') || (mNickname[0] >= 'A' && mNickname[0] <= 'Z')))
   {
      mNickname[0] = 'x';
   }
   
   const int specialCount = 8;
   char validSpecials[specialCount] = {'-', '[', ']', '\\', '`', '^', '{', '}' };
   
   //now validate the rest of the letters
   for (int i = 1; i < 9; i++)
   {
      //if the nick is < 9 chars, no problem
      if (mNickname[i] == '\0') break;
      
      //if it's not a letter, check that it's a valid special
      if (! ((mNickname[0] >= 'a' && mNickname[0] <= 'z') || (mNickname[0] >= 'A' && mNickname[0] <= 'Z')))
      {
         bool valid = FALSE;
         for  (int j = 0; j < specialCount; j++)
         {
            if (mNickname[i] == validSpecials[j])
            {
               valid = TRUE;
               break;
            }
         }
         
         if (! valid) mNickname[i] = '-';
      }
   }
   
   //now get the real name
   if ((! prefRealName) || (! prefRealName[0]))
   {
      if ((! prefName) || (! prefName[0]))
      {
         strcpy(mRealName, mNickname);
      }
      else
      {
         strcpy(mRealName, prefName);
      }
   }
   else
   {
      strcpy(mRealName, prefRealName);
   }
   
   bool result = Connect(mServer, mPort, mRealName, mNickname);
   if (result)
   {
      Console->printf("Connecting to: %s:%d  NAME: %s  NICK: %s", mServer, mPort, mRealName, mNickname);
   }
   else
   {
      Console->printf("Unable to connect to: %s:%d", mServer, mPort);
   }
   
   //initialize the System channel
   mChannels[0].inUse = TRUE;
   strcpy(mChannels[0].name, "System");
   
   //add a new message
   char buf[256];
   newMessage("~F1All system messages will appear here.", "System");
   sprintf(buf, "~f1Connecting to: %s:%d  NAME: %s  NICK: %s", mServer, mPort, mRealName, mNickname);
   newMessage(buf, "System");
   
   //set the current channel
   mSelectedChannel = 0;
   mForceVisibleChannel = 0;
   return TRUE;
}

void IRCClient::removeChannel(const char *chan)
{
   //start at 1 - never remove the system channel
   for (int i = 1; i < MAX_NUM_CHANNELS; i++)
   {
      if (mChannels[i].inUse && (! stricmp(mChannels[i].name, chan)))
      {
         //clean out the used channel
         clearChannel(i);
         
         //set the current channel back to the system channel
         mSelectedChannel = 0;
         mForceVisibleChannel = 0;
         break;
      }
   }
}

void IRCClient::clearChannel(int chan)
{
   if (mChannels[chan].inUse)
   {
      mChannels[chan].inUse = FALSE;
      mChannels[chan].nicknames.clear();
      while (mChannels[chan].messages.size() > 0)
      {
         destroy(&(mChannels[chan].messages.last()));
         mChannels[chan].messages.decrement();
      }
   }
}

IRCClient::ChannelRep* IRCClient::getCurrentChannel(bool resetIncoming)
{
   if (mSelectedChannel >= 0)
   {
      if (resetIncoming) mChannels[mSelectedChannel].msgWaiting = FALSE;
      return &mChannels[mSelectedChannel];
   }
   else return NULL;
}

const char* IRCClient::getForcedChannel(void)
{
   if (mForceVisibleChannel >= 0)
   {
      int retChannel = mForceVisibleChannel;
      mForceVisibleChannel = -1;
      return &(mChannels[retChannel].name[0]);
   }
   else return NULL;
}

void IRCClient::setSelectedChannel(const char *chan)
{
   for (int i = 0; i < MAX_NUM_CHANNELS; i++)
   {
      if (mChannels[i].inUse && (! stricmp(mChannels[i].name, chan)))
      {
         mSelectedChannel = i;
         mForceVisibleChannel = i;
         mChannels[i].msgWaiting = FALSE;
         return;
      }
   }
   
   mSelectedChannel = -1;
   mForceVisibleChannel = -1;
}

bool IRCClient::channelExists(const char *chName)
{
   //does the channel exist?
   for (int i = 0; i < MAX_NUM_CHANNELS; i++)
   {
      if (mChannels[i].inUse && (! stricmp(mChannels[i].name, chName)))
      {
         return TRUE;
      }
   }
   return FALSE;
}

void IRCClient::channelHasIncoming(const char *chName)
{
   //first, find the channel
   for (int i = 0; i < MAX_NUM_CHANNELS; i++)
   {
      if (mChannels[i].inUse && (! stricmp(mChannels[i].name, chName)))
      {
         mChannels[i].msgWaiting = TRUE;
         break;
      }
   }
}

int IRCClient::findChannelIndex(const char *chName)
{
   //first, find the channel
   int chIndex = -1;
   for (int i = 0; i < MAX_NUM_CHANNELS; i++)
   {
      if (mChannels[i].inUse && (! stricmp(mChannels[i].name, chName)))
      {
         chIndex = i;
         break;
      }
   }
   
   //if there isn't a channel yet create one
   if (chIndex < 0)
   {
      for (int i = 0; i < MAX_NUM_CHANNELS; i++)
      {
         if (mChannels[i].inUse == FALSE)
         {
            mChannels[i].msgWaiting = FALSE;
            mChannels[i].inUse = TRUE;
            strcpy(mChannels[i].name, chName);
            chIndex = i;
            break;
         }
      }
   }
   
   return chIndex;
}


int IRCClient::newMessage(const char *msg, const char *chName)
{
   //first, find the channel
   int chIndex = findChannelIndex(chName);
   
   //hopefully we found (or created) a channel
   if (chIndex >= 0)
   {
      mChannels[chIndex].messages.increment();
      new(&(mChannels[chIndex].messages.last())) SimGui::TextFormat;
      SimGui::TextFormat *newTextFormat = &(mChannels[chIndex].messages.last());
      
      //add the message
      newTextFormat->setFont(0, IDFNT_8_STANDARD);
      newTextFormat->setFont(1, IDFNT_8_HILITE);
      newTextFormat->setFont(2, IDFNT_8_SELECTED);
      newTextFormat->formatControlString(msg, extent.x - 6);
      
      //set the waiting flag
      mChannels[chIndex].msgWaiting = TRUE;
      return chIndex;
   }
   
   return -1;
}

void IRCClient::newNickname(const char *nick, const char *chName)
{
   //sanity checks
   if ((! nick) || (! nick[0])) return;
   if ((! chName) || (! chName[0])) return;
   
   //first, find the channel
   int chIndex = findChannelIndex(chName);
   
   //hopefully we found (or created) a channel
   if (chIndex >= 0)
   {
      //check and see first if the name is already in the channel
      bool found = FALSE;
      const char *nickPtr = nick;
      while (*nickPtr != '\0')
      {
         if ((*nickPtr >= 'a' && *nickPtr <= 'z') || (*nickPtr >= 'A' && *nickPtr <= 'Z'))
         {
            break;
         }
         nickPtr++;
      }
      if (*nickPtr == '\0') return;
      
      Vector<NickRep>::iterator i;
      for (i = mChannels[chIndex].nicknames.begin(); i != mChannels[chIndex].nicknames.end(); i++)
      {
         char *namePtr = i->name;
         while (*namePtr != '\0')
         {
            if ((*namePtr >= 'a' && *namePtr <= 'z') || (*namePtr >= 'A' && *namePtr <= 'Z'))
            {
               break;
            }
            namePtr++;
         }
         if (*namePtr == '\0') return;
         
         if (! stricmp(nickPtr, namePtr))
         {
            found = TRUE;
            //make sure the name contains all authority chars
            if ((strlen(nick) > strlen(i->name)) || (*nick == '@'))
            {
               strcpy(i->name, nick);
            }
            break;
         }
      }
      
      if (! found)
      {
         NickRep newNick;
         strcpy(newNick.name, nick);
         mChannels[chIndex].nicknames.push_back(newNick);
      }
   }
}

void IRCClient::removeNickname(const char *nick, const char *chName, const char *goodbyeMsg)
{
   //first, find the channel
   int chIndex = -1;
   for (int i = 0; i < MAX_NUM_CHANNELS; i++)
   {
      if (mChannels[i].inUse && (! stricmp(mChannels[i].name, chName)))
      {
         chIndex = i;
         break;
      }
   }
   if (chIndex < 0) return;
   
   //remove the nickname
   char *nickPtr;
   int index = -1;
   for (int i = 0; i < mChannels[chIndex].nicknames.size(); i++)
   {
      nickPtr = &(mChannels[chIndex].nicknames[i].name[0]);
      while (*nickPtr != '\0')
      {
         if ((*nickPtr >= 'a' && *nickPtr <= 'z') || (*nickPtr >= 'A' && *nickPtr <= 'Z'))
         {
            break;
         }
         nickPtr++;
      }
      if (*nickPtr == '\0') continue;
      if (! stricmp(nick, nickPtr))
      {
         index = i;
         break;
      }
   }
   if (index < 0) return;

   char buf[512];
   if ((! goodbyeMsg) || (goodbyeMsg[0] == '\0')) sprintf(buf, "*** %s has left %s.", nickPtr, chName);
   else sprintf(buf, "*** %s has left %s.  (%s)", nickPtr, chName, goodbyeMsg);
   newMessage(buf, chName);
   
   //now erase the nickname from the channel
   mChannels[chIndex].nicknames.erase(index);
}

void IRCClient::onWake(void)
{
   if (parent) extent = parent->extent;
}

const char *IRCClient::getNextToken(char *dest, const char *src)
{
   //first, eat white space
   const char *srcPtr = src;
   char *destPtr = dest;
   while (*srcPtr == ' ') srcPtr++;
   
   //now copy everything until we hit whitespace into the dest buffer
   while ((*srcPtr != ' ') && (*srcPtr != '\0')) *destPtr++ = *srcPtr++;
   
   //now terminate the string 
   *destPtr = '\0';
   
   return srcPtr;
}

void IRCClient::processMessage(void)
{
   //textFormat buf
   char msgBuffer[536];
   
   //find out which command was given
   //PING
   if (! strcmp(msgCommand, "PING"))
   {
      newMessage("PING? PONG!", "System");
   }
   
   //JOIN
   else if (! strcmp(msgCommand, "JOIN"))
   {
      //find the channels
      char *chName = &msgParams[0];
      char *endPtr = chName + strlen(msgParams);
      
      //see if it's me who joined
      bool joinChannel = FALSE;
      char *nickPtr = msgSource;
      while (*nickPtr != '\0')
      {
         if ((*nickPtr >= 'a' && *nickPtr <= 'z') || (*nickPtr >= 'A' && *nickPtr <= 'Z'))
         {
            break;
         }
         nickPtr++;
      }
      if (! stricmp(nickPtr, mNickname)) joinChannel = TRUE;
      
      while (chName < endPtr)
      {
         if (*chName == '#' || *chName == '&')
         {
            char *temp = chName;
            while (*temp != ' ' && *temp != ',' && *temp != '\0') temp++;
            *temp = '\0';
            
            //create the message, and echo it to channel chName[]
            sprintf(msgBuffer, "~F1*** %s has joined channel %s", msgSource, chName);
            int chan = newMessage(msgBuffer, chName);
            newNickname(&msgSource[0], chName);
            
            if (joinChannel)
            {
               mSelectedChannel = chan;
               mForceVisibleChannel = chan;
               joinChannel = FALSE;
            }
            
            //look for the next channel
            chName = temp + 1;
         }
         else chName++;
      }
   }
   
   //353 - the channel name list
   else if (! strcmp(msgCommand, "353"))
   {
      char *bufPtr;
      char *chName = &msgDestination[0];
      char *endPtr = chName + strlen(chName);
      while (*chName != '#' && *chName != '&' && chName < endPtr) chName++;
      if (*chName == '#' || *chName == '&')
      {
         endPtr = strchr(chName, ' ');
         if (endPtr) *endPtr = '\0';
         sprintf(msgBuffer, "The following are currently in channel %s: ", chName);
      }
      else return;
      
      bufPtr = &msgBuffer[strlen(msgBuffer)];
      
      //now add all the nick names
      bool firstOne = TRUE;
      char *nick = &msgParams[0];
      while (*nick != '\0')
      {
         //add the nick to the msg buffer
         if (*nick == ':') nick++;
         endPtr = strchr(nick, ' ');
         if (endPtr) *endPtr = '\0';
         if (firstOne)
         {
            sprintf(bufPtr, "%s", nick);
            bufPtr += strlen(nick);
            firstOne = FALSE;
         }
         else
         {
            sprintf(bufPtr, ", %s", nick);
            bufPtr += strlen(nick + 2);
         }
         
         //add it to the channel
         newNickname(nick, chName);
         
         //get the next one
         if (endPtr) nick = endPtr + 1;
         else break;
      }
      
      //create the message
      newMessage(msgBuffer, chName);
   }
   
   else if (! strcmp(msgCommand, "NOTICE"))
   {
      sprintf(msgBuffer, "%s", &msgParams);
      newMessage(msgBuffer, "System");
   }
   
   else if (! strcmp(msgCommand, "PRIVMSG"))
   {
      //see if it's a channel message
      if (msgDestination[0] == '#' || msgDestination[0] == '&')
      {
         sprintf(msgBuffer, "<%s> %s", msgSource, msgParams);
         newMessage(msgBuffer, msgDestination);
         //channelHasIncoming(msgDestination);
      }
      
      //else the message is private
      else
      {
         sprintf(msgBuffer, "<%s> %s", msgSource, msgParams);
         
         //if the channel doesn't already exists, switch to it...
         bool incoming = TRUE;
         if (! channelExists(msgSource)) incoming = FALSE;
         
         newMessage(msgBuffer, msgSource);
         
//debug
         //if (incoming) channelHasIncoming(msgSource);
         //else setSelectedChannel(msgSource);
         if (! incoming) setSelectedChannel(msgSource);
      }
      
   }
   
   else if (! strcmp(msgCommand, "PART"))
   {
      //a user has left a channel
      char *msgPtr;
      if (msgParams[0] == ':') msgPtr = &msgParams[1];
      else msgPtr = &msgParams[0];
      removeNickname(msgSource, msgDestination, msgPtr);
   }
   
   //everything else, just dump for now
   else
   {
      sprintf(msgBuffer, "%s %s %s %s", msgSource, msgCommand, msgDestination, msgParams);
      newMessage(msgBuffer, "System");
   }
}

void IRCClient::onPreRender(void)
{
   if (mIRCSocket.CheckForPackage())
   {
      bool pkgAvail = mIRCSocket.getPackage(msgSource, msgCommand, msgDestination, msgParams);
      while (pkgAvail)
      {
         processMessage();
         pkgAvail = mIRCSocket.getPackage(msgSource, msgCommand, msgDestination, msgParams);
      }
   }
}

bool IRCClient::Connect(char *server, int port, char *name, char *nick)
{
   //sanity checks
   if ((! server) || (! server[0])) return FALSE;
   if (port < 0) return FALSE;
   if ((! name) || (! name[0])) return FALSE;
   if ((! nick) || (! nick[0])) return FALSE;
   
   bool result;
   result = mIRCSocket.Connect(server, port);
   if (result)
   {
      mIRCSocket.Register(name, nick);
   }
   return result;
}

void IRCClient::Evaluate(const char *msg)
{
   char buf[511], *bufPtr;
   buf[510] = '\0';
   int length;
   if ((! msg) || (! msg[0])) return;
   
   IRCClient::ChannelRep* curChannel = getCurrentChannel(FALSE);
   //it's a PRIVMSG command
   if (msg[0] != '/')
   {
      //make sure we're in a channel
      if (curChannel == &mChannels[0] || (! curChannel))
      {
         newMessage("** You must be in a channel to speak.", "System");
         return;
      }
      else
      {
         sprintf(buf, "PRIVMSG %s :", curChannel->name);
         length = strlen(buf);
         strncpy(&buf[length], msg, 509 - length);
         Send(buf);
         
         //now echo to yourself
         sprintf(buf, "<%s> ", mNickname);
         length = strlen(buf);
         strncpy(&buf[length], msg, 509 - length);
         newMessage(buf, curChannel->name);
      }
   }
   
   //else it's an IRC command
   else
   {
      //first get the command, and convert to upper case
      strncpy(msgCommand, &msg[1], 63);
      msgCommand[63] = '\0';
      const char *msgPtr = &msg[1];
      char *cmdPtr = &msgCommand[0];
      while (*cmdPtr != '\0' && *cmdPtr != ' ')
      {
         *cmdPtr++ = toupper(*cmdPtr);
         msgPtr++;
      }
      *cmdPtr = '\0';
      
      //eat white space
      while (*msgPtr == ' ') msgPtr++;
      
      //find out which it is
      if ((! stricmp(msgCommand, "MSG")) || (! stricmp(msgCommand, "PRIVMSG")) || (! stricmp(msgCommand, "M")))
      {
         strcpy(buf, "PRIVMSG ");
         bufPtr = &buf[8];
         
         //find out where it's going
         strncpy(msgDestination, msgPtr, 255);
         msgDestination[255] = '\0';
         char *temp = strchr(msgDestination, ' ');
         if (temp)
         {
            *temp = '\0';
            msgPtr += strlen(msgDestination);
         }
         
         //eat white space
         while (*msgPtr == ' ') msgPtr++;
         
         //copy into the bufPtr
         strcpy(bufPtr, msgDestination);
         bufPtr += strlen(msgDestination);
         *bufPtr++ = ' ';
         
         if (*msgPtr == ':') msgPtr++;
         *bufPtr++ = ':';
         
         //now copy in the message
         length = strlen(buf);
         strncpy(bufPtr, msgPtr, 509 - length);
         
         //and send the message
         Send(buf);
         
         //now echo to yourself
         sprintf(buf, "<TO: %s> ", msgDestination);
         strcpy(&buf[strlen(buf)], msgPtr);
         
         ChannelRep *curChannel = IRCClient::getCurrentChannel(FALSE);
         if (curChannel)
         {
            newMessage(buf, curChannel->name);
         }
      }
      
      //changing your nickname
      else if (! stricmp(msgCommand, "NICK"))
      {
         sprintf(buf, "NICK %s", msgPtr);
         Send(buf);
      }
      
      //join
      else if (! stricmp(msgCommand, "J"))
      {
         sprintf(buf, "JOIN %s", msgPtr);
         Send(buf);
      }
      
      //part
      else if ((! stricmp(msgCommand, "PART")) || (! stricmp(msgCommand,"P")))
      {
         //get the channel name
         msgPtr = getNextToken(msgDestination, msgPtr);
         
         //get the reason (params);
         if (*msgPtr == ':') strcpy(msgParams, &msgPtr[1]);
         else strcpy(msgParams, msgPtr);
         
         //send the command
         sprintf(buf, "PART %s :%s", msgDestination, msgParams);
         Send(buf);
         
         //now echo the result here
         char *destPtr = &msgDestination[0];
         char *nextPtr;
         do
         {
            //find the next channel
            nextPtr = strchr(destPtr, ',');
            if (nextPtr) *nextPtr = '\0';
            
            //remove this channel
            removeChannel(destPtr);
            
            //set the destPtr
            if (nextPtr) destPtr = &nextPtr[1];
            else destPtr = NULL;
            
         } while (destPtr != NULL);
      }
      
      //else send whatever they sent
      else
      {
         Send(&msg[1]);
      }
   }
}

void IRCClient::Send(const char *msg)
{
   Console->printf("Sending: %s", msg);
   mIRCSocket.Send(msg);
}


IMPLEMENT_PERSISTENT_TAG( IRCClient,   FOURCC('F','G','i','c') );

};

