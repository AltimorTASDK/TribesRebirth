// commcode.h


#ifndef _H_COMMCODE
#define _H_COMMCODE

#include "dnet.h"

// OVERLAPPED vs NON_OVERLAPPED file i/o operations
#define OVERLAPPED_FLAG 1
#define USE_QUEUE 0

#define READ_BUFFER_SIZE 21000
#define WRITE_BUFFER_SIZE 3000

namespace DNet
{

typedef void  (*RECEIVE_DATA)( BYTE *, int, void * );

const BYTE check1 = 0xf9, check2 = 0xfa;

class Comm {

 private:

   typedef struct COMM_HEADER
   {
      BYTE  check1,check2;
      WORD  checkSum;
      short dataSize;
   } CommHeader, *lpCommHeader;

   HANDLE commFile;
   int    baud_rate;

#if OVERLAPPED_FLAG
   DWORD  g_dwReadThreadID;
	DWORD  g_dwWriteThreadID;
	HANDLE g_hReadThread;
	HANDLE g_hWriteThread;
	HANDLE g_hCloseEvent;
   HANDLE g_hWriteEvent;
#endif

   BYTE   readBuffer[READ_BUFFER_SIZE]; // buffer that hold immediate data read from comm stream
	int    readBufferPos;
#if USE_QUEUE
   typedef struct COMM_QUEUE_ELEMENT
   {
      DWORD  data_size;
      struct COMM_QUEUE_ELEMENT *next;
   	BYTE  *data;
   } CommQueueElement;

   CommQueueElement *CommQueue;
#else
	BYTE   writeBufferA[WRITE_BUFFER_SIZE];
   BYTE   writeBufferB[WRITE_BUFFER_SIZE];
   DWORD  writeBufferPos;
   bool   writeBufferFlag;
#endif

	DWORD  bytesRead;
   bool   nullComm; // TRUE if this is NULL-modem, FALSE if TAPI
   bool   commFailure;
   CRITICAL_SECTION ReadSection;
   CRITICAL_SECTION WriteSection;

	WORD   computeChecksum( char * dataPtr, WORD dataLength );
	void	 shiftReadBuffer( int start, int amount );

 public:

   Comm(bool bNull, RECEIVE_DATA recv_data_func_ptr);
   ~Comm();
   HANDLE OpenComm(const char *address);
	bool   StartComm(void);
	void   StopComm(void );
	Error  WriteCommString(const BYTE *dataPtr, const DWORD dataSize);
	Error  HandleWriteData(LPOVERLAPPED overlapped, BYTE *data, DWORD dataSize);
   bool   Process( void *myThis );
   bool   IsCommOpen( void ) { return commFile != NULL; };
   void   SetComm( HANDLE comm ) { commFile = comm; };
   HANDLE GetComm( void ) { return commFile; };
   DWORD  GetNULLDevices(int,DWORD []);
   void   ReportCommStatus( DWORD dwErrors, LPCOMSTAT lpStat);
   int    GetBaudRate( void ) { return baud_rate; };
#if USE_QUEUE
   bool   QueueData(BYTE *data, DWORD data_size);
   BYTE  *GetLatestData( DWORD *data_size, DWORD elements_to_keep );
   bool   IsDataWaiting( void );
   int    QueueSize(void);
#endif

#if OVERLAPPED_FLAG
	void CloseReadThread();
	void CloseWriteThread();
	static DWORD WINAPI StartReadThreadProc(LPVOID lpvParam);
   static DWORD WINAPI StartWriteThreadProc(LPVOID lpvParam);
	BOOL SetupReadEvent(LPOVERLAPPED lpOverlappedRead,
        LPSTR lpszInputBuffer, DWORD dwSizeofBuffer,
        LPDWORD lpnNumberOfBytesRead);
	BOOL HandleReadEvent(LPOVERLAPPED lpOverlappedRead,
        LPSTR lpszInputBuffer, DWORD dwSizeofBuffer,
        LPDWORD lpnNumberOfBytesRead);
	BOOL HandleReadData(LPCSTR lpszInputBuffer, DWORD dwSizeofBuffer);
	BOOL HandleCommEvent(LPOVERLAPPED lpOverlappedCommEvent,
        LPDWORD lpfdwEvtMask, BOOL fRetrieveEvent);
	BOOL SetupCommEvent(LPOVERLAPPED lpOverlappedCommEvent,
        LPDWORD lpfdwEvtMask);
#endif
};

};

#endif