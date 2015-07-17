//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <wsock.h>
#include <registry.h>
#pragma option -a1

//------------------------------------------------------------------------------
WSockLag *wsockLag = NULL;
bool logOn = true;


//------------------------------------------------------------------------------
WSockLag::WSockLag()
{
   lag          = 0;    // default lag in millitseconds
   lagDeviation = 0;    // default lag variance
   dropRate     = 0;    // drop packet percentage 0-100
   InitializeCriticalSection( &csLock );   
   randomize();
}   

//------------------------------------------------------------------------------
WSockLag::~WSockLag()
{
   if (hDll) 
      FreeLibrary( hDll );
   DeleteCriticalSection( &csLock );   
}   


//------------------------------------------------------------------------------
int WSockLag::threadProcess()
{
   while (1)
   {
      //sleep until it's time to stop the thread
      //OR time to flush the packet queues
      switch (WaitForSingleObject( getStopEvent(), THREAD_FREQ ))
      {
         case WAIT_OBJECT_0:  
            return (1);
         case WAIT_TIMEOUT:
            flushPackets();
            break;
         default:             
            return (0);
      }
   }
}   


//------------------------------------------------------------------------------
bool WSockLag::open()
{
	char libFileBuffer[MAX_PATH + 25];
	logString("Log file opened ----------------------------\n");

	GetSystemDirectory(libFileBuffer, MAX_PATH + 25);
	strcat(libFileBuffer, "\\WSOCK32.DLL");
	hDll = LoadLibrary(libFileBuffer);
	if(hDll)
	{
      bindFunctions(hDll);
      startThread();
	}
   else
		logString("Error: unable to load WSOCK32.DLL.");

   return (hDll != NULL);
}


//------------------------------------------------------------------------------
void WSockLag::updateReg()
{
   Registry reg;
   if(reg.open("Software", true))
      if(reg.open("Dynamix", true))
         if(reg.open("WSockLag", true))
         {
            reg.write("lag", lag);              
            reg.write("ladDeviation", lagDeviation);              
            reg.write("dropRate", dropRate);              
         }
}   

//------------------------------------------------------------------------------
void WSockLag::restoreReg()
{
   Registry reg;
   if(reg.open("Software", false))
      if(reg.open("Dynamix", false))
         if(reg.open("WSockLag", false))
         {
            reg.read("lag", &lag);              
            reg.read("ladDeviation", &lagDeviation);              
            reg.read("dropRate", &dropRate);              
         }
}   




//------------------------------------------------------------------------------
void WSockLag::lock()    
{ 
   EnterCriticalSection( &csLock ); 
}


//------------------------------------------------------------------------------
void WSockLag::unlock()  
{ 
   LeaveCriticalSection( &csLock ); 
}


//------------------------------------------------------------------------------
void WSockLag::fillPacket(Packet *p, SOCKET s, const char FAR * buf, int len)
{
   p->s     = s;
   p->len   = len;
   p->flags = 0;
   p->tolen = 16;
   memcpy(p->buf, buf, len);
   memset(p->sockaddr, sizeof(p->sockaddr), 0);
}   


//------------------------------------------------------------------------------
void WSockLag::fillPacket(Packet *p, SOCKET s, const char FAR * buf, int len, int flags,const struct sockaddr FAR *to, int tolen)
{
   p->s   = s;
   p->len = len;
   p->flags = flags;
   p->tolen = tolen;
   memcpy(p->buf, buf, len);
   memcpy(p->sockaddr, to, tolen);
}   


//------------------------------------------------------------------------------
void WSockLag::flushPackets()
{
   lock();   
   int time = GetTickCount();
   Packet *p = sendQueue.peek();
   while ( p && (time > p->sendTime)) 
   {
      fp_sendto(p->s, p->buf, p->len, p->flags, (const struct sockaddr FAR *)p->sockaddr, p->tolen);
      logString(avar("flush %d time %6d\n", p->s, p->sendTime));
      sendQueue.dequeue();
      sendQueue.free(p);
      p = sendQueue.peek();
      fp_WSASetLastError(0);
   }
   unlock();   
}   


//------------------------------------------------------------------------------
bool WSockLag::isCommand(SOCKET s, const char FAR *buf, int len, int flags,const struct sockaddr FAR *to, int tolen)
{
   struct ConsoleEvent
   {
      char   packetHeader[7];
      DWORD  type;
      DWORD  size;
      DWORD  objectId;
      DWORD  managerId;
      float  time;
      BYTE   len;
      char   script[200];
   };
   if (len > 28)
   {
      ConsoleEvent *event = (ConsoleEvent*)buf;   
      if (event->type == FOURCC('S','C','O','N'))
      {
         //------------- setLag {lag} {lagDeviation} 
         if (strnicmp(event->script, "lag::setLag", 11)== 0)
         {
            int i1, i2;
            int x = sscanf(event->script+11, "%d %d", &i1, &i2);
            switch (x)
            {
               case 1:
                  lag = i1;
                  lagDeviation = 0;
                  break;
               case 2:
                  lag = i1;
                  lagDeviation = i2;
                  break;
            }
         }
         //------------- metrics
         else if (strnicmp(event->script, "lag::metrics", 12) == 0)
         {
            ConsoleEvent ev = *event;
            sprintf(ev.script, 
               "echo 'lag: %d/+%dms';"
               "echo 'drop rate: %d%% ' "
               , lag, lagDeviation, dropRate);
            ev.len  = strlen(ev.script)+1;
            ev.size = ev.len+13;  //13 for event overhead

            fp_sendto(s, (char*)&ev, ev.size+11, flags, to, tolen);
            return (true);
         }
         //------------- dropRate
         else if (strnicmp(event->script, "lag::dropRate", 13) == 0)
         {
            int i;
            if (sscanf(event->script+13, "%d", &i))
               dropRate = max(0, min(i, 99));   // clamp 0 to 99
            else
               dropRate = 0;
         }
         //------------- restoreReg
         else if (stricmp(event->script, "lag::restore") == 0)
         {
            restoreReg();
         }
         //------------- Log
         else if (strnicmp(event->script, "lag::log", 8) == 0)
         {
            int i;
            if (sscanf(event->script+8, "%d", &i))
               logOn = (i == 1);
         }
         else
            return (false);


         // must have been a SCON lag command
         // the SCON packets are guaranteed so we need to pass them along
         // but the scripts are zeroed out so they don't do anything.
         updateReg();
         event->len = 1;         // zero out script 
         event->script[0] = 0; 
         fp_sendto(s, buf, len, flags, to, tolen);
         return (true);
      }
   }
   return (false);   
}   


//------------------------------------------------------------------------------
int WSockLag::sendto (SOCKET s, const char FAR * buf, int len, int flags,const struct sockaddr FAR *to, int tolen)
{
//   lock();
//   if (!isCommand(s, buf, len, flags, to, tolen))
//   {
//      if (lag)
//      {
//         if (dropRate==0 || (random(100)+1) > dropRate)
//         {
//            Packet *p = sendQueue.alloc();
//            if (p)
//            {
//               fillPacket(p, s, buf, len, flags, to, tolen);
//               p->sendTime = GetTickCount() + lag + random(lagDeviation);
//               sendQueue.enqueue( p );
//
//               logString(avar("delay %d until %d\n", p->s, p->sendTime));
//               fp_WSASetLastError(0);
//            }
//         }
//         else
//            logString("dropped packet\n");
//      }
//      else
//         fp_sendto(s, buf, len, flags, to, tolen);
//   }
//   unlock();
   fp_sendto(s, buf, len, flags, to, tolen);
   logString("send (%s) length %d\n%s\n", logAddr(to), len, logBuffer(buf, len));
   return (len);   
}


//------------------------------------------------------------------------------
int WSockLag::recvfrom (SOCKET s, char FAR * buf, int len, int flags,struct sockaddr FAR *from, int FAR * fromlen)
{
//   lock();
////   Packet *p = recvQueue.dequeue();
////   if (p)
////   {
////      memcpy(buf, p->buf, p->len);
////      memcpy(from, p->sockaddr, p->tolen);
////      *fromlen = p->tolen;
////      len = p->len;
////   }
////   else
//   {
//      flushPackets();
//      len = fp_recvfrom (s, buf, len, flags, from, fromlen);
//   }   
//   unlock();
   len = fp_recvfrom (s, buf, len, flags, from, fromlen);
   if (len > 0 && len != SOCKET_ERROR)
      logString("recv (%s) length %d\n%s\n", logAddr(from), len, logBuffer(buf, len));
   return (len);
}


//------------------------------------------------------------------------------
void logString(const char *fmt, ...)
{
   if (!logOn) return;
	FILE *f = fopen("wsock32.log", "a");
   if (f)
   {
      char buffer[5000];
      va_list pArgs;
      va_start(pArgs, fmt);
      vsprintf(buffer, fmt, pArgs);

	   fputs(buffer, f);
	   fclose(f);
   }
}

char* logAddr(const struct sockaddr FAR *from)
{
   static char buf[256];
   struct sockaddr_in *si = (struct sockaddr_in *) from;
   unsigned int port = si->sin_port;
   sprintf(buf, "%d.%d.%d.%d:%d", si->sin_addr.s_net, si->sin_addr.s_host,
      si->sin_addr.s_lh, si->sin_addr.s_impno, (port >> 8) | ((port & 0xFF) << 8));
   return (buf);
}

char* logBuffer(const char *buf, int len)
{
   int i;
   if(!len) return ("");
   static char sbuf[4096];
   const char *src;
   char *ptr;

   ptr = sbuf;
   src = buf;

   while (len)
   {
      for(i = 0; i < 5; i++)  *ptr++ = ' ';  // white space padding
      
      int count = 16;
      char *hex = ptr+count+1;
      while (count)
      {
         if (len)
         {
            if(*src < ' ' || *src > 'z')
	            *ptr++ = '.';
            else
	            *ptr++ = *src;
            src++;

            hex += sprintf(hex, "%02x ", (unsigned char)(*src));
            len--;
         }
         else
            *ptr++ = ' ';
         count--;
      }
      *ptr = ' ';
      *hex++ = '\n';
      ptr = hex;
   }

   return (sbuf);
}



//------------------------------------------------------------------------------
PacketQueue::PacketQueue()
{
   freeList = store;
   head  = tail = NULL;  
   for (int i=0; i<QUEUE_SIZE-1; i++)
      store[i].next = &store[i+1];
   store[QUEUE_SIZE-1].next = NULL;
}   

Packet* PacketQueue::alloc()
{
   Packet *p = freeList;
   if (freeList) 
      freeList = freeList->next;
   p->next = NULL;
   return (p);
}   

void PacketQueue::free(Packet *p)
{
   if (p)
   {
      p->next = freeList;
      freeList = p;   
   }
}   

void PacketQueue::enqueue(Packet *p)
{
   if (p && tail)
      tail->next = p;
   tail = p;
   if (!head)
      head = tail;
}   

Packet* PacketQueue::dequeue()
{
   Packet *p = head;
   if (head)   
      head = head->next;
   if (!head)
      tail = NULL;
   return (p);
}   

Packet* PacketQueue::peek()
{
   return (head);
}   




