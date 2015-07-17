//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _WSOCK_H_
#define _WSOCK_H_

#include <base.h>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threadBase.h>

//------------------------------------------------------------------------------
// wsock.cpp

void logString(const char *fmt, ...);
char* logAddr(const struct sockaddr FAR *from);
char* logBuffer(const char *buf, int len);

struct Packet
{
   int   sendTime;
   SOCKET s;
   char  buf[8024];
   int   len;
   int   flags;
   char  sockaddr[256];
   int   tolen;
   int   error;
   Packet *next;
};
#define QUEUE_SIZE    300
#define THREAD_FREQ    30      

//--------------------------------------
class PacketQueue
{
private:
   Packet store[QUEUE_SIZE];
   Packet *freeList;
   Packet *head;
   Packet *tail;

public:
   PacketQueue();
   bool isEmpty() { return ( head == tail); }
   bool isFull()  { return ( freeList == NULL); }
   Packet*  alloc();
   void     free(Packet *);
   void     enqueue(Packet *);
   Packet*  dequeue();
   Packet*  peek();
};

//--------------------------------------
#define LOGMODE      // comment out to turn OFF logging

class WSockLag: public ThreadBase
{
private:
   HINSTANCE hDll;
   CRITICAL_SECTION csLock;
   int lag;                // base lag in ms
   int lagDeviation;       // addional lag in ms
   int dropRate;           // 0-100 percent droped packet rate

   PacketQueue recvQueue;
   PacketQueue sendQueue;

   void lock();
   void unlock();
   void fillPacket(Packet *p, SOCKET s, const char FAR * buf, int len);
   void fillPacket(Packet *p, SOCKET s, const char FAR * buf, int len, int flags,const struct sockaddr FAR *to, int tolen);
   void flushPackets();

   bool isCommand(SOCKET s, const char FAR *buf, int len, int flags,const struct sockaddr FAR *to, int tolen);

public:
   WSockLag();
   ~WSockLag();

   int threadProcess();     
   bool open();
   void updateReg();
   void restoreReg();

   int recvfrom (SOCKET s, char FAR * buf, int len, int flags,struct sockaddr FAR *from, int FAR * fromlen);
   int sendto (SOCKET s, const char FAR * buf, int len, int flags,const struct sockaddr FAR *to, int tolen);
};

extern WSockLag *wsockLag;


//------------------------------------------------------------------------------
// wsockPatch.cpp
void bindFunctions( HINSTANCE hDll );

//------------------------------------------------------------------------------
extern "C" BOOL WINAPI WsockEntryProc(HINSTANCE hInst, DWORD reason, LPVOID reserved);

typedef int (PASCAL FAR *typ_fp___WSAFDIsSet) (SOCKET, fd_set FAR *);
typedef SOCKET (PASCAL FAR *typ_fp_accept)(SOCKET s, struct sockaddr FAR *addr, int FAR *addrlen);
typedef int (PASCAL FAR *typ_fp_bind) (SOCKET s, const struct sockaddr FAR *addr, int namelen);
typedef int (PASCAL FAR *typ_fp_closesocket) (SOCKET s);
typedef int (PASCAL FAR *typ_fp_connect) (SOCKET s, const struct sockaddr FAR *name, int namelen);
typedef int (PASCAL FAR *typ_fp_ioctlsocket) (SOCKET s, long cmd, u_long FAR *argp);
typedef int (PASCAL FAR *typ_fp_getpeername) (SOCKET s, struct sockaddr FAR *name,int FAR * namelen);
typedef int (PASCAL FAR *typ_fp_getsockname) (SOCKET s, struct sockaddr FAR *name,int FAR * namelen);
typedef int (PASCAL FAR *typ_fp_getsockopt) (SOCKET s, int level, int optname,char FAR * optval, int FAR *optlen);
typedef u_long (PASCAL FAR *typ_fp_htonl) (u_long hostlong);
typedef u_short (PASCAL FAR *typ_fp_htons) (u_short hostshort);
typedef unsigned long (PASCAL FAR *typ_fp_inet_addr) (const char FAR * cp);
typedef char FAR * (PASCAL FAR *typ_fp_inet_ntoa) (struct in_addr in);
typedef int (PASCAL FAR *typ_fp_listen) (SOCKET s, int backlog);
typedef u_long (PASCAL FAR *typ_fp_ntohl) (u_long netlong);
typedef u_short (PASCAL FAR *typ_fp_ntohs) (u_short netshort);
typedef int (PASCAL FAR *typ_fp_recv) (SOCKET s, char FAR * buf, int len, int flags);
typedef int (PASCAL FAR *typ_fp_recvfrom) (SOCKET s, char FAR * buf, int len, int flags,struct sockaddr FAR *from, int FAR * fromlen);
typedef int (PASCAL FAR *typ_fp_select) (int nfds, fd_set FAR *readfds, fd_set FAR *writefds, fd_set FAR *exceptfds, const struct timeval FAR *timeout);
typedef int (PASCAL FAR *typ_fp_send) (SOCKET s, const char FAR * buf, int len, int flags);
typedef int (PASCAL FAR *typ_fp_sendto) (SOCKET s, const char FAR * buf, int len, int flags,const struct sockaddr FAR *to, int tolen);
typedef int (PASCAL FAR *typ_fp_setsockopt) (SOCKET s, int level, int optname,const char FAR * optval, int optlen);
typedef int (PASCAL FAR *typ_fp_shutdown) (SOCKET s, int how);
typedef SOCKET (PASCAL FAR *typ_fp_socket) (int af, int type, int protocol);
typedef struct hostent FAR * (PASCAL FAR *typ_fp_gethostbyaddr)(const char FAR * addr,int len, int type);
typedef struct hostent FAR * (PASCAL FAR *typ_fp_gethostbyname)(const char FAR * name);
typedef int (PASCAL FAR *typ_fp_gethostname) (char FAR * name, int namelen);
typedef struct servent FAR * (PASCAL FAR *typ_fp_getservbyport)(int port, const char FAR * proto);
typedef struct servent FAR * (PASCAL FAR *typ_fp_getservbyname)(const char FAR * name,const char FAR * proto);
typedef struct protoent FAR * (PASCAL FAR *typ_fp_getprotobynumber)(int proto);
typedef struct protoent FAR * (PASCAL FAR *typ_fp_getprotobyname)(const char FAR * name);
typedef int (PASCAL FAR *typ_fp_WSAStartup)(WORD wVersionRequired, LPWSADATA lpWSAData);
typedef int (PASCAL FAR *typ_fp_WSACleanup)(void);
typedef void (PASCAL FAR *typ_fp_WSASetLastError)(int iError);
typedef int (PASCAL FAR *typ_fp_WSAGetLastError)(void);
typedef BOOL (PASCAL FAR *typ_fp_WSAIsBlocking)(void);
typedef int (PASCAL FAR *typ_fp_WSAUnhookBlockingHook)(void);
typedef FARPROC (PASCAL FAR *typ_fp_WSASetBlockingHook)(FARPROC lpBlockFunc);
typedef int (PASCAL FAR *typ_fp_WSACancelBlockingCall)(void);
typedef HANDLE (PASCAL FAR *typ_fp_WSAAsyncGetServByName)(HWND hWnd, u_int wMsg,const char FAR * name,const char FAR * proto,char FAR * buf, int buflen);
typedef HANDLE (PASCAL FAR *typ_fp_WSAAsyncGetServByPort)(HWND hWnd, u_int wMsg, int port,const char FAR * proto, char FAR * buf,int buflen);
typedef HANDLE (PASCAL FAR *typ_fp_WSAAsyncGetProtoByName)(HWND hWnd, u_int wMsg, const char FAR * name, char FAR * buf, int buflen);
typedef HANDLE (PASCAL FAR *typ_fp_WSAAsyncGetProtoByNumber)(HWND hWnd, u_int wMsg, int number, char FAR * buf, int buflen);
typedef HANDLE (PASCAL FAR *typ_fp_WSAAsyncGetHostByName)(HWND hWnd, u_int wMsg, const char FAR * name, char FAR * buf, int buflen);
typedef HANDLE (PASCAL FAR *typ_fp_WSAAsyncGetHostByAddr)(HWND hWnd, u_int wMsg, const char FAR * addr, int len, int type, char FAR * buf, int buflen);
typedef int (PASCAL FAR *typ_fp_WSACancelAsyncRequest)(HANDLE hAsyncTaskHandle);
typedef int (PASCAL FAR *typ_fp_WSAAsyncSelect)(SOCKET s, HWND hWnd, u_int wMsg, long lEvent);
typedef int (PASCAL FAR *typ_fp_WSARecvEx) (SOCKET s, char FAR * buf, int len, int FAR *flags);

extern typ_fp___WSAFDIsSet     fp___WSAFDIsSet;
extern typ_fp_accept           fp_accept;
extern typ_fp_bind             fp_bind;
extern typ_fp_closesocket      fp_closesocket;
extern typ_fp_connect          fp_connect;
extern typ_fp_ioctlsocket      fp_ioctlsocket;
extern typ_fp_getpeername      fp_getpeername;
extern typ_fp_getsockname      fp_getsockname;
extern typ_fp_getsockopt       fp_getsockopt;
extern typ_fp_htonl            fp_htonl;
extern typ_fp_htons            fp_htons;
extern typ_fp_listen           fp_listen;
extern typ_fp_ntohl            fp_ntohl;
extern typ_fp_ntohs            fp_ntohs;
extern typ_fp_recv             fp_recv;
extern typ_fp_recvfrom         fp_recvfrom;
extern typ_fp_select           fp_select;
extern typ_fp_send             fp_send;
extern typ_fp_sendto           fp_sendto;
extern typ_fp_setsockopt       fp_setsockopt;
extern typ_fp_shutdown         fp_shutdown;
extern typ_fp_socket           fp_socket;
extern typ_fp_gethostname      fp_gethostname;
extern typ_fp_WSAStartup       fp_WSAStartup;
extern typ_fp_WSACleanup       fp_WSACleanup;
extern typ_fp_WSASetLastError  fp_WSASetLastError;
extern typ_fp_WSAGetLastError  fp_WSAGetLastError;
extern typ_fp_WSAIsBlocking    fp_WSAIsBlocking;
extern typ_fp_WSAUnhookBlockingHook     fp_WSAUnhookBlockingHook;
extern typ_fp_WSASetBlockingHook        fp_WSASetBlockingHook;
extern typ_fp_WSACancelBlockingCall     fp_WSACancelBlockingCall;
extern typ_fp_WSAAsyncGetServByName     fp_WSAAsyncGetServByName;
extern typ_fp_WSAAsyncGetServByPort     fp_WSAAsyncGetServByPort;
extern typ_fp_WSAAsyncGetProtoByName    fp_WSAAsyncGetProtoByName;
extern typ_fp_WSAAsyncGetProtoByNumber  fp_WSAAsyncGetProtoByNumber;
extern typ_fp_WSAAsyncGetHostByName     fp_WSAAsyncGetHostByName;
extern typ_fp_WSAAsyncGetHostByAddr     fp_WSAAsyncGetHostByAddr;
extern typ_fp_WSACancelAsyncRequest     fp_WSACancelAsyncRequest;
extern typ_fp_WSAAsyncSelect   fp_WSAAsyncSelect;
extern typ_fp_WSARecvEx        fp_WSARecvEx;
extern typ_fp_inet_addr        fp_inet_addr;
extern typ_fp_inet_ntoa        fp_inet_ntoa;
extern typ_fp_gethostbyaddr    fp_gethostbyaddr;
extern typ_fp_gethostbyname    fp_gethostbyname;
extern typ_fp_getservbyport    fp_getservbyport;
extern typ_fp_getservbyname    fp_getservbyname;
extern typ_fp_getprotobynumber fp_getprotobynumber;
extern typ_fp_getprotobyname   fp_getprotobyname;

#endif //_WSOCK_H_
