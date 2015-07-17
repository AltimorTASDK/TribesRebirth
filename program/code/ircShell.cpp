
#include "stdio.h"
#include "stdlib.h"
#include "console.h"
#include "ircShell.h"

#define BUFFER_SIZE 1020
char gRecvBuffer[BUFFER_SIZE + 1];
char *gRecvBufPtr;

IRCShell::IRCShell()
{
   mSocket = INVALID_SOCKET;
   mbConnected = FALSE;

   WSADATA stWSAData;
	//WORD wVersionRequested = MAKEWORD( 2, 0 ); 
   WSAStartup(0x0101, &stWSAData);  //version 1.1
   
   //initialize the buffer vars
   gRecvBuffer[0] = '\0';
   gRecvBufPtr = &gRecvBuffer[0];
}

IRCShell::~IRCShell()
{
   Disconnect();
	WSACleanup();
}

bool IRCShell::CheckForPackage(void)
{
   bool gotAPackage = FALSE;
   bool disconnect = FALSE;

   if (mbConnected)
   {
      //if we're pointing to the end of a string, the buffer has been completely ready, so fill from start
      if (*gRecvBufPtr  == '\0') gRecvBufPtr = &gRecvBuffer[0];
      
      // see if we have any input to process... also make sure we have space
      int space = BUFFER_SIZE - (int(gRecvBufPtr) - int(&gRecvBuffer[0]));
      if (space <= 0) return FALSE;
      int numBytes = recv(mSocket, gRecvBufPtr, space, 0);
      if(numBytes == SOCKET_ERROR)
      {
         if(WSAGetLastError() != WSAEWOULDBLOCK)
            disconnect = true;
      }
      else if (numBytes == 0)
         disconnect = true;
      else
      {
         //make sure the string is null terminated
         gRecvBufPtr[numBytes] = '\0';
         Console->printf("INCOMING: %s", gRecvBufPtr);
         
         //set the pointer back to the beginning of the buffer
         gRecvBufPtr = &gRecvBuffer[0];
         
         //set the return bool
         gotAPackage = TRUE;
      }
      if (disconnect)
      {
         Disconnect();
      }
   }
   return gotAPackage;
}

bool IRCShell::getPackage(char *src, char *cmd, char *dest, char *params)
{
   //first, make sure we have a package, and that it is complete  (ending in '\r\n')
   if (*gRecvBufPtr == '\0') return NULL;
   
   char *nextMsgPtr = strchr(gRecvBufPtr, '\r');
   bool finished = (nextMsgPtr == NULL);
   while (! finished)
   {
      if (nextMsgPtr[1] == '\n') finished = TRUE;
      else nextMsgPtr = strchr(&nextMsgPtr[1], '\r');
      
      if (! nextMsgPtr) finished = TRUE; 
   }
   if (nextMsgPtr)
   {
      //NULL terminate the string
      *nextMsgPtr = '\0';
      nextMsgPtr++;
      
      //bypass the '\n' and all trailing ' ' as well
      nextMsgPtr++;
      while (*nextMsgPtr == ' ') nextMsgPtr++;
      
      //set the pointer to the next msg block
      char *startMsgPtr = gRecvBufPtr;
      gRecvBufPtr = nextMsgPtr;
      
      //return the processed message
      processPackage(startMsgPtr, src, cmd, dest, params);
      return TRUE;
   }
   
   //else move what the incomplete package to the beginning of the buffer, and wait for the rest
   else
   {
      //how much is in the buffer already
      int totalLengthToCopy = min(BUFFER_SIZE, int(strlen(gRecvBufPtr)));
      int distanceToCopy = int(gRecvBufPtr) - int(&gRecvBuffer[0]);
      
      //if there is room in the buffer for more
      if (totalLengthToCopy < BUFFER_SIZE)
      {
         //if we're not already at the beginning of the buffer
         if (distanceToCopy > 0)
         {
            char *destToCopy = &gRecvBuffer[0];
            while (totalLengthToCopy > 0)
            {
               int lengthToCopy = min(distanceToCopy, totalLengthToCopy);
               strncpy(destToCopy, gRecvBufPtr, lengthToCopy);
               destToCopy += lengthToCopy;
               gRecvBufPtr += lengthToCopy;
               totalLengthToCopy -= lengthToCopy;
            }
            //set the recvBufPtr to the end of the string
            gRecvBufPtr = destToCopy;
         }
         else
         {
            gRecvBufPtr = &gRecvBuffer[strlen(gRecvBuffer)];
         }
            
         //to ensure the next packet will be copied onto the end, make sure gRecvBufPtr != '\0';
         *gRecvBufPtr = 'X';
      }
      
      //else the buffer is full, send what you got and hope the packets straighten out
      else
      {
         //SHOULD NEVER GET HERE!!!!!
         gRecvBufPtr = &gRecvBuffer[BUFFER_SIZE];
         
         //ensure the string is null terminated, so the stream will read into the beginning next pass
         *gRecvBufPtr = '\0';
      }
      return FALSE;
   }
}
   
void IRCShell::processPackage(char *msg, char *src, char *cmd, char *dest, char *params)
{
   char *parsePtr = msg, *temp;
   
   //init all buffers
   *src = '\0';
   *cmd = '\0';
   *dest = '\0';
   *params = '\0';
   
   //check first for a source address - copy it to the src buffer
   if (*parsePtr == ':')
   {
      parsePtr++;
      temp = src;
      while (*parsePtr != ' ' && *parsePtr != '\0')
      {
         *temp++ = *parsePtr++;
      }
      //next, eat all white space
      if (*parsePtr == ' ')
      {
         *temp = '\0';
         while (*parsePtr == ' ') parsePtr++;
      }
   }
   //make sure the nickname '!' has been removed
   temp = strchr(src, '!');
   if (temp) *temp = '\0';
   
   
   //now process the commands
   
   //PING
   if (! strncmp(parsePtr, "PING ", 5))
   {
      //reply automatically with a PONG same address
      strcpy(cmd, "PING");
      parsePtr += 5;
      while (*parsePtr == ' ') parsePtr++;
      
      //everything after ping is the return address to ping back to
      strcpy(params, parsePtr);
      
      //now return the pong
      char buf[512];
      sprintf(buf, "PONG %s", params);
      
      Send(buf);
   }
   
   //default
   else
   {
      temp = cmd;
      while (*parsePtr != ' ' && *parsePtr != '\0')
      {
         *temp++ = *parsePtr++;
      }
      *temp = '\0';
      
      //eat more white space
      while (*parsePtr == ' ') parsePtr++;
      
      //EXCEPTION FOR 353 channel name list
      if (! strcmp(cmd, "353"))
      {
         //advance to the channel designation
         while (*parsePtr != '#' && *parsePtr != '&' && *parsePtr != '\0') parsePtr++;
      }
      
      //copy in the destination
      temp = dest;
      while (*parsePtr != ':' && *parsePtr != ' ' && *parsePtr != '\0')
      {
         *temp++ = *parsePtr++;   
      }
      *temp = '\0';
      
      //make sure the dest doesn't have any '!' either
      temp = strchr(dest, '!');
      if (temp) *temp = '\0';
      
      //eat white space
      while (*parsePtr == ' ') parsePtr++;
      
      //now search for the args
      temp = strchr(parsePtr, ':');
      if (temp)
      {
         strcpy(params, &temp[1]);
      }
   }
}

bool IRCShell::Connect(const char *ircServer, int port)
{
   //disconnect to previous ircServer
   if (mbConnected)
   {
      Disconnect();
   }
   
	PHOSTENT	pHostEnt;
	SOCKADDR_IN Address;

	pHostEnt = gethostbyname(ircServer);
	if (!pHostEnt)
	{
		WSACleanup();
		Console->printf("Failed to get host by name: %s", ircServer);
      return FALSE;
	}
   
   memcpy((char *)&(Address.sin_addr), pHostEnt->h_addr, pHostEnt->h_length );
	Address.sin_port = htons(port);
	Address.sin_family = AF_INET;

	mSocket = socket(AF_INET, SOCK_STREAM, 0);
	
	if (connect(mSocket, (PSOCKADDR)&Address, sizeof(Address)) < 0)
	{
	   closesocket(mSocket);
      mbConnected = FALSE;
	   WSACleanup();
	   Console->printf("Failed to connect to IRC server: %s", ircServer);
      return FALSE;
	}
   
   //bind the socket and the address, and listen to the socket
	bind(mSocket, (LPSOCKADDR) &Address, sizeof(Address));
	listen(mSocket, 4);

   //make it a non-blocking connection
   DWORD noblock = true;
   ioctlsocket(mSocket, FIONBIO, &noblock);
   
   //set the bool && the global
   mbConnected = TRUE;
   
   return TRUE;
}

void IRCShell::Disconnect(void)
{
   if (mbConnected)
   {
   	closesocket(mSocket);
      mbConnected = FALSE;
   }
}

void IRCShell::Send(const char *msg)
{
   if (! mbConnected) return;
   
   char buf[513];
   strncpy(buf, msg, 510);
   buf[510] = '\0';
   //now append '\r\n'
   
   int length = strlen(buf);
   buf[length] = '\r';
   buf[length + 1] = '\n';
   buf[length + 2] = '\0';
   
   send(mSocket, buf, length + 2, 0);
}

void IRCShell::Register(const char *name, const char *nick)
{
   if (mbConnected)
   {
      char buf[1024];
      sprintf(buf,"USER %s ignore ignore :%s", nick, name);
      Send(buf);
      sprintf(buf,"NICK %s", nick);
      Send(buf);
   }
}




         
