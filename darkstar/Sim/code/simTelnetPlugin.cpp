#include "simConsolePlugin.h"
#include "simTelnetPlugin.h"

enum CallbackId
{
   TelnetPort,
};

SimTelnetPlugin::SimTelnetPlugin()
{
   acceptSocket = INVALID_SOCKET;
   acceptPort = -1;

   WSADATA stWSAData;
   WSAStartup(0x0101, &stWSAData);
}

SimTelnetPlugin::~SimTelnetPlugin()
{
   Vector<TelnetClient>::iterator i;
   for(i = connectionList.begin(); i != connectionList.end(); i++)
      closesocket(i->fd);
   if(acceptPort)
      closesocket(acceptPort);
   WSACleanup();
}

void SimTelnetPlugin::init()
{
   console->addVariable(TelnetPort, "TelnetPort", this, "0");
   console->addConsumer(this);
}

void SimTelnetPlugin::consoleCallback(CMDConsole *, const char *consoleLine)
{
   // ok, spew this line out to all our subscribers...
   Vector<TelnetClient>::iterator client;
   int len = strlen(consoleLine)+1;
   for(client = connectionList.begin(); client != connectionList.end();client++)
   {
      if(client->state == Connected)
      {
         send(client->fd, consoleLine, len, 0);
         send(client->fd, "\r\n", 2, 0);
      }
   }
}

// callback to test the port variables, etc...
const char *SimTelnetPlugin::consoleCallback(CMDConsole *, int id, int argc, const char *argv[])
{
   argc;
   argv;

   if(id == TelnetPort)
   {
      int newPort;
      newPort = console->getIntVariable("TelnetPort");
      if(newPort != acceptPort)
      {
         if(acceptSocket != INVALID_SOCKET)
         {
            closesocket(acceptSocket);
            acceptSocket = INVALID_SOCKET;
         }
         acceptPort = newPort;
         if(acceptPort != -1 && acceptPort != 0)
         {
	         acceptSocket = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
         	SOCKADDR_IN sockAddr;

         	sockAddr.sin_family = AF_INET;	
         	sockAddr.sin_port   = htons(console->getIntVariable("TelnetPort"));
         	sockAddr.sin_addr.s_addr   = INADDR_ANY;

         	bind(acceptSocket, (LPSOCKADDR) &sockAddr, sizeof(sockAddr));
         	listen(acceptSocket, 4);

            DWORD noblock = true;
            ioctlsocket(acceptSocket, FIONBIO, &noblock);
         }
      }
   }
   return 0;
} 

void SimTelnetPlugin::endFrame()
{
  	SOCKADDR_IN sockAddr;
   int iAddrLength = sizeof (sockAddr);			

   if(acceptSocket != INVALID_SOCKET)
   {
      // ok, see if we have any new connections:
      SOCKET newConnection;
      newConnection = accept(acceptSocket, (sockaddr *) &sockAddr, &iAddrLength);

      if(newConnection != INVALID_SOCKET)
      {
   		console->printf ("Telnet connection from %i.%i.%i.%i",
   				sockAddr.sin_addr.S_un.S_un_b.s_b1,	sockAddr.sin_addr.S_un.S_un_b.s_b2,	
   				sockAddr.sin_addr.S_un.S_un_b.s_b3,	sockAddr.sin_addr.S_un.S_un_b.s_b4); 
         TelnetClient cl;
         cl.fd = newConnection;
         cl.curPos = 0;
         cl.state = EnteringPassword;
         cl.passcount = 0;

         DWORD noblock = true;
         ioctlsocket(cl.fd, FIONBIO, &noblock);

         char *connectMessage = "SimEngine Telnet Provider\r\n\r\nEnter Password:";
         
         send(cl.fd, connectMessage, strlen(connectMessage)+1, 0);
         connectionList.push_back(cl);
      }
   }
   
   // see if we have any input to process...
   Vector<TelnetClient>::iterator client;
   char recvBuf[256];
   char reply[1024];

   for(client = connectionList.begin(); client != connectionList.end();)
   {
      bool eraseClient = false;
      int numBytes = recv(client->fd, recvBuf, sizeof(recvBuf), 0);
      if(numBytes == SOCKET_ERROR)
      {
         if(WSAGetLastError() != WSAEWOULDBLOCK)
         {
            closesocket(client->fd);
            eraseClient = true;
         }
      }
      else if(numBytes == 0)
         eraseClient = true;
      else
      {
         int i;
         int replyPos = 0;
         for(i = 0; i < numBytes;i++)
         {
            // execute the current command
            if(recvBuf[i] == '\r')
            {
            }
            else if(recvBuf[i] == '\n')
            {
               reply[replyPos++] = '\r';
               reply[replyPos++] = '\n';

               client->curLine[client->curPos] = 0;
               client->curPos = 0;

               if(client->state == Connected)
               {
                  send(client->fd, reply, replyPos, 0);
                  replyPos = 0;
                  console->evaluate(client->curLine, false, NULL, 0);
                  // note - send prompt next
                  const char *prompt = console->getVariable("Console::Prompt");
                  send(client->fd, prompt, strlen(prompt), 0);
               }
               else
               {
                  client->passcount++;
                  if(!strcmp(client->curLine, console->getVariable("TelnetPassword")))
                  {
                     send(client->fd, reply, replyPos, 0);
                     replyPos = 0;
                     
                     // send prompt
                     const char *prompt = console->getVariable("Console::Prompt");
                     send(client->fd, prompt, strlen(prompt), 0);
                     client->state = Connected;
                  }
                  else
                  {
                     const char *sendStr;
                     if(client->passcount > 3)
                        sendStr = "Too many tries... cya.";
                     else
                        sendStr = "Nope... try agian.\r\nEnter Password:";
                     send(client->fd, sendStr, strlen(sendStr), 0);
                     if(client->passcount > 3)
                     {
                        closesocket(client->fd);
                        eraseClient = true;
                     }
                  }
               }
            }
            else if(recvBuf[i] == '\b')
            {
               // pull the old backspace manuever...
               if(client->curPos > 0)
               {
                  client->curPos--;
                  if(client->state == Connected)
                  {
                     reply[replyPos++] = '\b';
                     reply[replyPos++] = ' ';
                     reply[replyPos++] = '\b';
                  }
               }
            }
            else if(client->curPos < CMDConsole::MaxLineLength-1)
            {
               client->curLine[client->curPos++] = recvBuf[i];
               // don't echo password chars...
               if(client->state == Connected)
                  reply[replyPos++] = recvBuf[i];
            }
         }
         if(replyPos)
            send(client->fd, reply, replyPos, 0);
      }
      if(eraseClient)
         connectionList.erase(client);
      else
         client++;
   }
}
