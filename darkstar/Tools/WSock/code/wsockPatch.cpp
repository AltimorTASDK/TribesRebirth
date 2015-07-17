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


typ_fp___WSAFDIsSet     fp___WSAFDIsSet = NULL;
typ_fp_accept           fp_accept = NULL;
typ_fp_bind             fp_bind = NULL;
typ_fp_closesocket      fp_closesocket = NULL;
typ_fp_connect          fp_connect = NULL;
typ_fp_ioctlsocket      fp_ioctlsocket = NULL;
typ_fp_getpeername      fp_getpeername = NULL;
typ_fp_getsockname      fp_getsockname = NULL;
typ_fp_getsockopt       fp_getsockopt = NULL;
typ_fp_htonl            fp_htonl = NULL;
typ_fp_htons            fp_htons = NULL;
typ_fp_listen           fp_listen = NULL;
typ_fp_ntohl            fp_ntohl = NULL;
typ_fp_ntohs            fp_ntohs = NULL;
typ_fp_recv             fp_recv = NULL;
typ_fp_recvfrom         fp_recvfrom = NULL;
typ_fp_select           fp_select = NULL;
typ_fp_send             fp_send = NULL;
typ_fp_sendto           fp_sendto = NULL;
typ_fp_setsockopt       fp_setsockopt = NULL;
typ_fp_shutdown         fp_shutdown = NULL;
typ_fp_socket           fp_socket = NULL;
typ_fp_gethostname      fp_gethostname = NULL;
typ_fp_WSAStartup       fp_WSAStartup = NULL;
typ_fp_WSACleanup       fp_WSACleanup = NULL;
typ_fp_WSASetLastError  fp_WSASetLastError = NULL;
typ_fp_WSAGetLastError  fp_WSAGetLastError = NULL;
typ_fp_WSAIsBlocking    fp_WSAIsBlocking = NULL;
typ_fp_WSAUnhookBlockingHook     fp_WSAUnhookBlockingHook = NULL;
typ_fp_WSASetBlockingHook        fp_WSASetBlockingHook = NULL;
typ_fp_WSACancelBlockingCall     fp_WSACancelBlockingCall = NULL;
typ_fp_WSAAsyncGetServByName     fp_WSAAsyncGetServByName = NULL;
typ_fp_WSAAsyncGetServByPort     fp_WSAAsyncGetServByPort = NULL;
typ_fp_WSAAsyncGetProtoByName    fp_WSAAsyncGetProtoByName = NULL;
typ_fp_WSAAsyncGetProtoByNumber  fp_WSAAsyncGetProtoByNumber = NULL;
typ_fp_WSAAsyncGetHostByName     fp_WSAAsyncGetHostByName = NULL;
typ_fp_WSAAsyncGetHostByAddr     fp_WSAAsyncGetHostByAddr = NULL;
typ_fp_WSACancelAsyncRequest     fp_WSACancelAsyncRequest = NULL;
typ_fp_WSAAsyncSelect   fp_WSAAsyncSelect = NULL;
typ_fp_WSARecvEx        fp_WSARecvEx = NULL;
typ_fp_inet_addr        fp_inet_addr = NULL;
typ_fp_inet_ntoa        fp_inet_ntoa = NULL;
typ_fp_gethostbyaddr    fp_gethostbyaddr = NULL;
typ_fp_gethostbyname    fp_gethostbyname = NULL;
typ_fp_getservbyport    fp_getservbyport = NULL;
typ_fp_getservbyname    fp_getservbyname = NULL;
typ_fp_getprotobynumber fp_getprotobynumber = NULL;
typ_fp_getprotobyname   fp_getprotobyname = NULL;


static int instanceCount = 0;

//------------------------------------------------------------------------------
void bindFunctions( HINSTANCE hDll )
{
	fp_accept = (typ_fp_accept) GetProcAddress(hDll, "accept");
	fp_bind = (typ_fp_bind) GetProcAddress(hDll, "bind");
	fp_closesocket = (typ_fp_closesocket) GetProcAddress(hDll, "closesocket");
	fp_connect = (typ_fp_connect) GetProcAddress(hDll, "connect");
	fp_ioctlsocket = (typ_fp_ioctlsocket) GetProcAddress(hDll, "ioctlsocket");
	fp_getpeername = (typ_fp_getpeername) GetProcAddress(hDll, "getpeername");
	fp_getsockname = (typ_fp_getsockname) GetProcAddress(hDll, "getsockname");
	fp_getsockopt = (typ_fp_getsockopt) GetProcAddress(hDll, "getsockopt");
	fp_htonl = (typ_fp_htonl) GetProcAddress(hDll, "htonl");
	fp_htons = (typ_fp_htons) GetProcAddress(hDll, "htons");
	fp_listen = (typ_fp_listen) GetProcAddress(hDll, "listen");
	fp_ntohl = (typ_fp_ntohl) GetProcAddress(hDll, "ntohl");
	fp_ntohs = (typ_fp_ntohs) GetProcAddress(hDll, "ntohs");
	fp_recv = (typ_fp_recv) GetProcAddress(hDll, "recv");
	fp_recvfrom = (typ_fp_recvfrom) GetProcAddress(hDll, "recvfrom");
	fp_select = (typ_fp_select) GetProcAddress(hDll, "select");
	fp_send = (typ_fp_send) GetProcAddress(hDll, "send");
	fp_sendto = (typ_fp_sendto) GetProcAddress(hDll, "sendto");
	fp_setsockopt = (typ_fp_setsockopt) GetProcAddress(hDll, "setsockopt");
	fp_shutdown = (typ_fp_shutdown) GetProcAddress(hDll, "shutdown");
	fp_socket = (typ_fp_socket) GetProcAddress(hDll, "socket");
	fp_gethostname = (typ_fp_gethostname) GetProcAddress(hDll, "gethostname");
	fp_WSAStartup = (typ_fp_WSAStartup) GetProcAddress(hDll, "WSAStartup");
	fp_WSACleanup = (typ_fp_WSACleanup) GetProcAddress(hDll, "WSACleanup");
	fp_WSASetLastError = (typ_fp_WSASetLastError) GetProcAddress(hDll, "WSASetLastError");
	fp_WSAGetLastError = (typ_fp_WSAGetLastError) GetProcAddress(hDll, "WSAGetLastError");
	fp_WSAIsBlocking = (typ_fp_WSAIsBlocking) GetProcAddress(hDll, "WSAIsBlocking");
	fp_WSAUnhookBlockingHook = (typ_fp_WSAUnhookBlockingHook) GetProcAddress(hDll, "WSAUnhookBlockingHook");
	fp_WSASetBlockingHook = (typ_fp_WSASetBlockingHook) GetProcAddress(hDll, "WSASetBlockingHook");
	fp_WSACancelBlockingCall = (typ_fp_WSACancelBlockingCall) GetProcAddress(hDll, "WSACancelBlockingCall");
	fp_WSAAsyncGetServByName = (typ_fp_WSAAsyncGetServByName) GetProcAddress(hDll, "WSAAsyncGetServByName");
	fp_WSAAsyncGetServByPort = (typ_fp_WSAAsyncGetServByPort) GetProcAddress(hDll, "WSAAsyncGetServByPort");
	fp_WSAAsyncGetProtoByName = (typ_fp_WSAAsyncGetProtoByName) GetProcAddress(hDll, "WSAAsyncGetProtoByName");
	fp_WSAAsyncGetProtoByNumber = (typ_fp_WSAAsyncGetProtoByNumber) GetProcAddress(hDll, "WSAAsyncGetProtoByNumber");
	fp_WSAAsyncGetHostByName = (typ_fp_WSAAsyncGetHostByName) GetProcAddress(hDll, "WSAAsyncGetHostByName");
	fp_WSAAsyncGetHostByAddr = (typ_fp_WSAAsyncGetHostByAddr) GetProcAddress(hDll, "WSAAsyncGetHostByAddr");
	fp_WSACancelAsyncRequest = (typ_fp_WSACancelAsyncRequest) GetProcAddress(hDll, "WSACancelAsyncRequest");
	fp_WSAAsyncSelect = (typ_fp_WSAAsyncSelect) GetProcAddress(hDll, "WSAAsyncSelect");
	fp_WSARecvEx = (typ_fp_WSARecvEx) GetProcAddress(hDll, "WSARecvEx");
	fp_inet_addr = (typ_fp_inet_addr) GetProcAddress(hDll, "inet_addr");
	fp_inet_ntoa = (typ_fp_inet_ntoa) GetProcAddress(hDll, "inet_ntoa");
	fp_gethostbyaddr = (typ_fp_gethostbyaddr) GetProcAddress(hDll, "gethostbyaddr");
	fp_gethostbyname = (typ_fp_gethostbyname) GetProcAddress(hDll, "gethostbyname");
	fp_getservbyport = (typ_fp_getservbyport) GetProcAddress(hDll, "getservbyport");
	fp_getservbyname = (typ_fp_getservbyname) GetProcAddress(hDll, "getservbyname");
	fp_getprotobynumber = (typ_fp_getprotobynumber) GetProcAddress(hDll, "getprotobynumber");
	fp_getprotobyname = (typ_fp_getprotobyname) GetProcAddress(hDll, "getprotobyname");
	fp___WSAFDIsSet = (typ_fp___WSAFDIsSet) GetProcAddress(hDll, "__WSAFDIsSet");
}   




int PASCAL FAR __WSAFDIsSet(SOCKET s, fd_set FAR *set)
{
	return fp___WSAFDIsSet(s, set);
}

SOCKET PASCAL FAR accept (SOCKET s, struct sockaddr FAR *addr, int FAR *addrlen)
{
	return fp_accept (s, addr, addrlen);
}

int PASCAL FAR bind (SOCKET s, const struct sockaddr FAR *addr, int namelen)
{
	return fp_bind (s, addr, namelen);
}

int PASCAL FAR closesocket (SOCKET s)
{
	return fp_closesocket (s);
}

int PASCAL FAR connect (SOCKET s, const struct sockaddr FAR *name, int namelen)
{
	return fp_connect (s, name, namelen);
}

int PASCAL FAR ioctlsocket (SOCKET s, long cmd, u_long FAR *argp)
{
	return fp_ioctlsocket (s, cmd, argp);
}

int PASCAL FAR getpeername (SOCKET s, struct sockaddr FAR *name,int FAR * namelen)
{
	return fp_getpeername (s, name,namelen);
}

int PASCAL FAR getsockname (SOCKET s, struct sockaddr FAR *name,int FAR * namelen)
{
	return fp_getsockname (s, name,namelen);
}

int PASCAL FAR getsockopt (SOCKET s, int level, int optname,char FAR * optval, int FAR *optlen)
{
	return fp_getsockopt (s, level, optname,optval, optlen);
}

u_long PASCAL FAR htonl (u_long hostlong)
{
	return fp_htonl (hostlong);
}

u_short PASCAL FAR htons (u_short hostshort)
{
	return fp_htons (hostshort);
}

int PASCAL FAR listen (SOCKET s, int backlog)
{
	return fp_listen (s, backlog);
}

u_long PASCAL FAR ntohl (u_long netlong)
{
	return fp_ntohl (netlong);
}

u_short PASCAL FAR ntohs (u_short netshort)
{
	return fp_ntohs (netshort);
}

int PASCAL FAR recv (SOCKET s, char FAR * buf, int len, int flags)
{
	return fp_recv (s, buf, len, flags);
}

int PASCAL FAR recvfrom (SOCKET s, char FAR * buf, int len, int flags,struct sockaddr FAR *from, int FAR * fromlen)
{
//   return fp_recvfrom (s, buf, len, flags, from, fromlen);
   return wsockLag->recvfrom (s, buf, len, flags, from, fromlen);
}

int PASCAL FAR select (int nfds, fd_set FAR *readfds, fd_set FAR *writefds, fd_set FAR *exceptfds, const struct timeval FAR *timeout)
{
	return fp_select (nfds, readfds, writefds, exceptfds, timeout);
}

int PASCAL FAR send (SOCKET s, const char FAR * buf, int len, int flags)
{
	return fp_send (s, buf, len, flags);
}

int PASCAL FAR sendto (SOCKET s, const char FAR * buf, int len, int flags,const struct sockaddr FAR *to, int tolen)
{
//   return fp_sendto(s, buf, len, flags, to, tolen);
   return wsockLag->sendto(s, buf, len, flags, to, tolen);
}

int PASCAL FAR setsockopt (SOCKET s, int level, int optname,const char FAR * optval, int optlen)
{
	return fp_setsockopt (s, level, optname,optval, optlen);
}

int PASCAL FAR shutdown (SOCKET s, int how)
{
   if (instanceCount) instanceCount--;
   if (!instanceCount)
   {
      delete wsockLag;
      wsockLag = NULL;
   }
	return fp_shutdown (s, how);
}

SOCKET PASCAL FAR socket (int af, int type, int protocol)
{
	return fp_socket (af, type, protocol);
}

int PASCAL FAR gethostname (char FAR * name, int namelen)
{
	return fp_gethostname (name, namelen);
}

int PASCAL FAR WSAStartup (WORD wVersionRequired, LPWSADATA lpWSAData)
{
   if (!instanceCount)
   {
      wsockLag = new WSockLag;
      wsockLag->open();
   }
   instanceCount++;
	return fp_WSAStartup (wVersionRequired, lpWSAData);
}

int PASCAL FAR WSACleanup (void)
{
	return fp_WSACleanup ();
}

void PASCAL FAR WSASetLastError (int iError)
{
	fp_WSASetLastError (iError);
}

int PASCAL FAR WSAGetLastError (void)
{
	return fp_WSAGetLastError ();
}

BOOL PASCAL FAR WSAIsBlocking (void)
{
	return fp_WSAIsBlocking ();
}

int PASCAL FAR WSAUnhookBlockingHook (void)
{
	return fp_WSAUnhookBlockingHook ();
}

FARPROC PASCAL FAR WSASetBlockingHook (FARPROC lpBlockFunc)
{
	return fp_WSASetBlockingHook (lpBlockFunc);
}

int PASCAL FAR WSACancelBlockingCall (void)
{
	return fp_WSACancelBlockingCall ();
}

HANDLE PASCAL FAR WSAAsyncGetServByName (HWND hWnd, u_int wMsg,const char FAR * name,const char FAR * proto,char FAR * buf, int buflen)
{
	return fp_WSAAsyncGetServByName (hWnd, wMsg,name,proto,buf, buflen);
}

HANDLE PASCAL FAR WSAAsyncGetServByPort (HWND hWnd, u_int wMsg, int port,const char FAR * proto, char FAR * buf,int buflen)
{
	return fp_WSAAsyncGetServByPort (hWnd, wMsg, port,proto, buf,buflen);
}

HANDLE PASCAL FAR WSAAsyncGetProtoByName (HWND hWnd, u_int wMsg, const char FAR * name, char FAR * buf, int buflen)
{
	return fp_WSAAsyncGetProtoByName (hWnd, wMsg, name, buf, buflen);
}

HANDLE PASCAL FAR WSAAsyncGetProtoByNumber (HWND hWnd, u_int wMsg, int number, char FAR * buf, int buflen)
{
	return fp_WSAAsyncGetProtoByNumber (hWnd, wMsg, number, buf, buflen);
}

HANDLE PASCAL FAR WSAAsyncGetHostByName (HWND hWnd, u_int wMsg, const char FAR * name, char FAR * buf, int buflen)
{
	return fp_WSAAsyncGetHostByName (hWnd, wMsg, name, buf, buflen);
}

HANDLE PASCAL FAR WSAAsyncGetHostByAddr (HWND hWnd, u_int wMsg, const char FAR * addr, int len, int type, char FAR * buf, int buflen)
{
	return fp_WSAAsyncGetHostByAddr (hWnd, wMsg, addr, len, type, buf, buflen);
}

int PASCAL FAR WSACancelAsyncRequest (HANDLE hAsyncTaskHandle)
{
	return fp_WSACancelAsyncRequest (hAsyncTaskHandle);
}

int PASCAL FAR WSAAsyncSelect (SOCKET s, HWND hWnd, u_int wMsg, long lEvent)
{
	return fp_WSAAsyncSelect (s, hWnd, wMsg, lEvent);
}

int PASCAL FAR WSARecvEx (SOCKET s, char FAR * buf, int len, int FAR *flags)
{
	return fp_WSARecvEx (s, buf, len, flags);
}

unsigned long PASCAL FAR inet_addr (const char FAR * cp)
{
	return fp_inet_addr (cp);
}

char FAR * PASCAL FAR inet_ntoa (struct in_addr in)
{
	return fp_inet_ntoa(in);
}

struct hostent FAR * PASCAL FAR gethostbyaddr (const char FAR * addr,int len, int type)
{
	return fp_gethostbyaddr (addr,len, type);
}

struct hostent FAR * PASCAL FAR gethostbyname (const char FAR * name)
{
	return fp_gethostbyname (name);
}

struct servent FAR * PASCAL FAR getservbyport (int port, const char FAR * proto)
{
	return fp_getservbyport (port, proto);
}

struct servent FAR * PASCAL FAR getservbyname (const char FAR * name,const char FAR * proto)
{
	return fp_getservbyname (name,proto);
}

struct protoent FAR * PASCAL FAR getprotobynumber (int proto)
{
	return fp_getprotobynumber (proto);
}

struct protoent FAR * PASCAL FAR getprotobyname (const char FAR * name)
{
	return fp_getprotobyname (name);
}


