//
//  MODULE: CommCode.c
//
//  PURPOSE: Handles all the COMM routines for TapiComm.
//

#include <windows.h>
#include <winbase.h>
#include <string.h>

#include "commcode.h"
#include "logfile.h"

// This is the message posted to the WriteThread
// When we have something to write.
#define PWM_COMMWRITE   WM_USER+1

// Default size of the Input Buffer used by this code.
#define INPUTBUFFERSIZE 2048

extern class LogFile log_file;

namespace DNet {

RECEIVE_DATA receive_data;

Comm::Comm(bool bNull, RECEIVE_DATA recv_data_fnc_ptr)
{
   commFile = NULL;
   readBufferPos=0;
#if USE_QUEUE
   CommQueue = NULL;
#else
   writeBufferPos=0;
   writeBufferFlag=false;
#endif
   bytesRead=0;
   nullComm=bNull;
   commFailure=false;
   receive_data = recv_data_fnc_ptr;

   DWORD  g_dwReadThreadID  = 0;
	DWORD  g_dwWriteThreadID = 0;
	HANDLE g_hReadThread  = NULL;
	HANDLE g_hWriteThread = NULL;
	HANDLE g_hCloseEvent = NULL;
   HANDLE g_hWriteEvent = NULL;
#if OVERLAPPED_FLAG
	InitializeCriticalSection(&ReadSection);
	InitializeCriticalSection(&WriteSection);
#endif
};

Comm::~Comm()
{
	StopComm(); // just in case someone forgot
#if OVERLAPPED_FLAG
	DeleteCriticalSection(&ReadSection);
	DeleteCriticalSection(&WriteSection);
#endif
}

DWORD Comm::GetNULLDevices(int array_size, DWORD device_ids[])
{
	DWORD num_devices=0;
	HANDLE hCom;
   char comm_file[] = "COM9999";

   for(int i=1; i<5; i++)
   {
   	comm_file[3]='0'+(char)i;
      comm_file[4]=0;

		hCom = CreateFile( comm_file,
   	                   GENERIC_READ | GENERIC_WRITE,
							    0,    /* comm devices must be opened w/exclusive-access */
							    NULL, /* no security attrs */
							    OPEN_EXISTING, /* comm devices must use OPEN_EXISTING */
							    0,    /* not overlapped I/O */
							    NULL );  /* hTemplate must be NULL for comm devices */


		if (hCom == INVALID_HANDLE_VALUE)
		{
//   		log_file.log("   CreateFile on COM%ld failed with error: %ld",(long)i,(long)GetLastError());
   	}
      else
      {
      	device_ids[num_devices++] = i;
		   CloseHandle(hCom);
      }
   }
   return num_devices;
}

HANDLE Comm::OpenComm(const char *address)
{
	HANDLE hCom;

	hCom = CreateFile( address,
                      GENERIC_READ | GENERIC_WRITE,
						    0,    /* comm devices must be opened w/exclusive-access */
						    NULL, /* no security attrs */
						    OPEN_EXISTING, /* comm devices must use OPEN_EXISTING */
#if OVERLAPPED_FLAG
							 FILE_FLAG_OVERLAPPED,
#else
						    0,    /* not overlapped I/O */
#endif
						    NULL );  /* hTemplate must be NULL for comm devices */

	if (hCom == INVALID_HANDLE_VALUE)
	{
   	log_file.log(LOG_ERROR,"COM_ERROR:CreateFile on %s failed with error: %ld",address,(long)GetLastError());
      return NULL;
   }

	return hCom;
}

//
//  FUNCTION: StartComm()
//
//  PURPOSE: Starts communications over the comm port.
//
//  PARAMETERS:
//
//  RETURN VALUE:
//    TRUE if able to setup the communications.
//
//  COMMENTS:
//
//    OpenComm must have already been called or a comm handle obtained through
//    TAPI before StartComm is called.  StartComm sets up the comm port parameters.
//

bool Comm::StartComm()
{
   COMMTIMEOUTS commtimeouts;
   DCB dcb;
   DWORD fdwEvtMask;

	// Is this a valid comm handle?
   if (GetFileType(commFile) != FILE_TYPE_CHAR)
   {
      log_file.log(LOG_ERROR,"COM_ERROR:File handle is not a comm handle.");
      return false;
   }

   /*-----------------------*/
   /* Set the dcb structure */
   /*-----------------------*/

   if( nullComm )
   {
	   FillMemory(&dcb, sizeof(dcb), 0);
   	dcb.DCBlength = sizeof(dcb);
#if 0
	   if (!BuildCommDCB("baud=28800 parity=N data=8 stop=1", &dcb))
      {
   	   // Couldn't build the DCB. Usually a problem
      	// with the communications specification string.
   		log_file.log(LOG_ERROR,"COM_ERROR:BuildCommDCB failed: %ld",GetLastError());
	      return false;
   	}
#else
	   dcb.BaudRate    = CBR_56000;
//   	SRegLoadValue("Network Providers\\Serial","Baud Rate",0,&dcb.BaudRate);
	   dcb.fBinary     = 1;
   	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	   dcb.fRtsControl = RTS_CONTROL_DISABLE;
   	dcb.ByteSize    = 8;
	   dcb.Parity      = NOPARITY;
   	dcb.StopBits    = ONESTOPBIT;
	   dcb.ErrorChar   = 63;
   	dcb.EofChar     = 26;
#endif
   }
   else
   {
	   // fAbortOnError is the only DCB dependancy in TapiComm.
   	// Can't guarentee that the service provider will set this to what we expect.
   	GetCommState(commFile, &dcb);
      baud_rate = dcb.BaudRate;
      log_file.log(LOG_ERROR,"CONNECT %ld",(long)baud_rate);
	   dcb.fAbortOnError = FALSE;
   }

   if( !SetCommState(commFile, &dcb))
   {
   	log_file.log(LOG_ERROR,"COM_ERROR:SetCommState failed: %ld",GetLastError());
      return false;
   }

	/*----------------------------------------------------------------*/
   /* Set the time outs.  For OVERLAPPED, just use 250 milliseconds. */
   /* For nonOVERLAPPED, set the timeout so reads return immediately */
   /* even if no data is in the queue.  This prevents blocking       */
   /* (since we are not threaded.)                                   */
   /*----------------------------------------------------------------*/

   GetCommTimeouts(commFile, &commtimeouts);
#if OVERLAPPED_FLAG
	commtimeouts.ReadIntervalTimeout         = 250;
#else
	commtimeouts.ReadIntervalTimeout      = MAXDWORD;
#endif
   commtimeouts.ReadTotalTimeoutMultiplier  = 0;
   commtimeouts.ReadTotalTimeoutConstant    = 0;
   commtimeouts.WriteTotalTimeoutMultiplier = 0;
   commtimeouts.WriteTotalTimeoutConstant   = 0;

   if(!SetCommTimeouts(commFile, &commtimeouts))
   {
  		log_file.log(LOG_ERROR,"COM_ERROR:SetCommTimeouts failed: %ld",GetLastError());
      return false;
   }

	/*----------------------------------------------------*/
   /* Set the comm mask so we receive all error signals. */
	/*----------------------------------------------------*/

   GetCommMask(commFile, &fdwEvtMask);
   if (!SetCommMask(commFile, EV_ERR))
   {
  		log_file.log(LOG_ERROR,"COM_ERROR:SetCommMask failed: %ld",GetLastError());
      return false;
   }

#if OVERLAPPED_FLAG
    // Create the event that will signal the threads to close.
    g_hCloseEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    g_hWriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (g_hCloseEvent == NULL || g_hWriteEvent == NULL)
    {
        log_file.log(LOG_ERROR,"Unable to create close event %ld",(long)GetLastError());
        commFile = NULL;
        return FALSE;
    }

    // Create the Read thread.
    g_hReadThread =
        CreateThread(NULL, 0, StartReadThreadProc, (LPVOID)this, 0, &g_dwReadThreadID);
//log_file.log("ReadThread created with id %lx",(long)g_dwReadThreadID);

    if (g_hReadThread == NULL)
    {
        log_file.log(LOG_ERROR,"Unable to create Read thread %ld",(long)GetLastError());

        g_dwReadThreadID = 0;
        commFile = 0;
        return FALSE;
    }

    // Comm threads should to have a higher base priority than the UI thread.
    // If they don't, then any temporary priority boost the UI thread gains
    // could cause the COMM threads to loose data.
    SetThreadPriority(g_hReadThread, THREAD_PRIORITY_HIGHEST);

    // Create the Write thread.
    g_hWriteThread =
        CreateThread(NULL, 0, StartWriteThreadProc, (LPVOID)this, 0, &g_dwWriteThreadID);
//log_file.log("WriteThread created with id %lx",(long)g_dwWriteThreadID);
    if (g_hWriteThread == NULL)
    {
        log_file.log(LOG_ERROR,"Unable to create Write thread %ld",(long)GetLastError());

        CloseReadThread();
        g_dwWriteThreadID = 0;
        commFile = 0;
        return FALSE;
    }

    SetThreadPriority(g_hWriteThread, THREAD_PRIORITY_ABOVE_NORMAL);
#endif

   // Everything was created ok.  Ready to go!
   return true;
}


//
//  FUNCTION: StopComm
//
//  PURPOSE: Stop and end all communication threads.
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
//    Tries to gracefully signal all communication threads to
//    close, but terminates them if it has to.
//
//

void Comm::StopComm()
{
   // No need to continue if we're not communicating.
   if (commFile == NULL)
      return;

	log_file.log("Comm::StopComm");

#if OVERLAPPED_FLAG
   // Close the threads.
   CloseReadThread();
   CloseWriteThread();

   // Not needed anymore.
   CloseHandle(g_hCloseEvent);
   CloseHandle(g_hWriteEvent);
#else
   PurgeComm(commFile, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);
#endif
   // Now close the comm port handle.
   if(!CloseHandle(commFile))
   {
   	log_file.log("StopComm::CloseHandle failed with %ld",GetLastError());
   }
   commFile = NULL;
}


//
//  FUNCTION: WriteCommString(BYTE *, DWORD)
//
//  PURPOSE: Write a string to the Comm.
//
//  PARAMETERS:
//    data     - String to Write to Comm port.
//    dataSize - length of data.
//
//  RETURN VALUE:
//    Returns NoError if successful.
//    Returns SendError if fails
//
//  COMMENTS:
//
//
//
//   If OVERLAPPED, posts a write message, otherwise calls
//   HandleWriteData directly.
//
//    Note that it is assumed that the write function will delete data
//

Error Comm::WriteCommString(const BYTE *data, const DWORD dataSize)
{
#if OVERLAPPED_FLAG
   lpCommHeader header;

   // No need to continue if we're not communicating.
   if (commFile == NULL)
   {
   	log_file.log(LOG_ERROR,"COM_ERROR:sending data with comm port open");
      return SendError;
   }

   if(!g_hWriteThread)
   {
   	log_file.log(LOG_ERROR,"COM_ERROR:sending data without write thread");
   	return SendError;
   }

	// make sure we aren't BYTEing off more than we can chew
   DWORD packetSize = dataSize + sizeof(CommHeader);
   if( packetSize > MaxPacketSize || packetSize > WRITE_BUFFER_SIZE)
   {
   	log_file.log(LOG_ERROR,"COM_ERROR:data too big: %ld",(long)packetSize);
      return SendError;
   }

   // construct header and copy data to a new buffer
   // that will be deleted by actual sending function
   BYTE *packet = new BYTE[packetSize];
   if(packet == NULL)
   {
		log_file.log(LOG_ERROR,"COM_ERROR:allocation error %lx",(long)packetSize);
   	return SendError;
   }

   header = (lpCommHeader)packet;
   header->check1        = check1;
   header->check2        = check2;
   header->dataSize      = (short)dataSize;
   header->checkSum      = computeChecksum((char *)data, (WORD)dataSize);
   memcpy(&packet[sizeof(CommHeader)], data, dataSize);

	// PostThreadMessage might return error if thread message queue hasn't
   // fired up yet.  If we get ERROR_INVALID_THREAD_ID we should Sleep()
   // for a bit and try again.

	// this code uses two buffers in the Comm class to hold data to be sent

	EnterCriticalSection(&WriteSection);
#if USE_QUEUE
	QueueData(packet, packetSize);
#else
  	BYTE *buffer = (writeBufferFlag?writeBufferA:writeBufferB);
   memcpy( buffer, packet, packetSize );
  	writeBufferPos = packetSize;
#endif
	LeaveCriticalSection(&WriteSection);

  	if(!SetEvent(g_hWriteEvent))  // signal write thread that data is ready
   {
		log_file.log(LOG_ERROR,"SetEvent:g_hWriteEvent failed with error %ld",GetLastError());
   	return SendError;
   }
   return NoError;

#if 0 // this section uses the thread's message queue to hold the data
   char waiting_for_queue = 10;
	DWORD error;
  	do
   {
	   if (PostThreadMessage(g_dwWriteThreadID, PWM_COMMWRITE,
            (WPARAM) packetSize, (LPARAM) packet))
     	{
//	         log_file.log("PostThreadMessage posted %lx bytes to Write thread",(long)packetSize);
         return NoError;
     	}
	   else
      {
			error = GetLastError();
         log_file.log(LOG_ERROR,"PostThreadMessage failed with error %ld, thread id %lx",error,(long)g_dwWriteThreadID);
      }
		Sleep(1);
   }
   while( --waiting_for_queue && error == ERROR_INVALID_THREAD_ID);

   return SendError;
#endif

#else

	return HandleWriteData( (LPOVERLAPPED)NULL, packet, packetSize );
#endif
}

#if OVERLAPPED_FLAG
//
//  FUNCTION: HandleWriteData(LPOVERLAPPED, LPCSTR, DWORD)
//
//  PURPOSE: Writes a given string to the comm file handle.
//
//  PARAMETERS:
//    lpOverlappedWrite      - Overlapped structure to use in WriteFile
//    lpszStringToWrite      - String to write.
//    dwNumberOfBytesToWrite - Length of String to write.
//
//  RETURN VALUE:
//    TRUE if all bytes were written.  False if there was a failure to
//    write the whole string.
//
//  COMMENTS:
//
//    This function is a helper function for the Write Thread.  It
//    is this call that actually writes a string to the comm file.
//    Note that this call blocks and waits for the Write to complete
//    or for the CloseEvent object to signal that the thread should end.
//    Another possible reason for returning FALSE is if the comm port
//    is closed by the service provider.
//
//

Error Comm::HandleWriteData(LPOVERLAPPED lpOverlappedWrite, BYTE *data, DWORD dataSize)
{
    DWORD dwLastError;
    DWORD dataWritten = 0;
    DWORD dataIndex = 0; // Start at the beginning.
    DWORD dwHandleSignaled;
    HANDLE HandlesToWaitFor[2];

    HandlesToWaitFor[0] = g_hCloseEvent;
    HandlesToWaitFor[1] = lpOverlappedWrite -> hEvent;

    // Keep looping until all characters have been written.
    do
    {
        // Start the overlapped I/O.
        if (!WriteFile(commFile, &data[ dataIndex ], dataSize, &dataWritten, lpOverlappedWrite))
        {
            // WriteFile failed.  Expected; lets handle it.
            dwLastError = GetLastError();

            // Its possible for this error to occur if the
            // service provider has closed the port.  Time to end.
            if (dwLastError == ERROR_INVALID_HANDLE)
            {
                log_file.log(LOG_ERROR,"ERROR_INVALID_HANDLE: Likely that the Service Provider has closed the port.");
                goto ErrorExit;
            }

            // Unexpected error.  No idea what.
            if (dwLastError != ERROR_IO_PENDING)
            {
                log_file.log(LOG_ERROR,"Error to writing to CommFile: %ld",(long)dwLastError);
                commFailure = true;
                goto ErrorExit;
            }

            // This is the expected ERROR_IO_PENDING case.


            // Wait for either overlapped I/O completion,
            // or for the CloseEvent to get signaled.
            dwHandleSignaled =
                WaitForMultipleObjects(2, HandlesToWaitFor, FALSE, INFINITE);

            switch(dwHandleSignaled)
            {
                case WAIT_OBJECT_0:     // CloseEvent signaled!
                {
                    // Time to exit.
                    goto ErrorExit;
                }

                case WAIT_OBJECT_0 + 1: // Wait finished.
                {
                    // Time to get the results of the WriteFile
                    break;
                }

                case WAIT_FAILED: // Wait failed.  Shouldn't happen.
                {
                    log_file.log(LOG_ERROR,"WriteFile WAIT_FAILED: %ld",(long)GetLastError());
                    commFailure = true;
                    goto ErrorExit;
                }

                default: // This case should never occur.
                {
                    log_file.log(LOG_ERROR,"Unexpected Wait return value '%ld'",(long)dwHandleSignaled);
                    commFailure = true;
                    goto ErrorExit;
                }
            }

            if (!GetOverlappedResult(commFile,
                     lpOverlappedWrite,
                     &dataWritten, TRUE))
            {
                dwLastError = GetLastError();

                // Its possible for this error to occur if the
                // service provider has closed the port.
                if (dwLastError == ERROR_INVALID_HANDLE)
                {
                    log_file.log(LOG_ERROR,"ERROR_INVALID_HANDLE: Likely that the Service Provider has closed the port.");
                    goto ErrorExit;
                }

                // No idea what could cause another error.
                log_file.log(LOG_ERROR,"Error writing to CommFile while waiting %ld",(long)dwLastError);
                log_file.log(LOG_ERROR,"Closing TAPI\n");
                commFailure = true;
                goto ErrorExit;
            }
        }

        // Some data was written.  Make sure it all got written.
//        log_file.log("Wrote %lx bytes to comm port",(long)dataWritten);

        dataSize  -= dataWritten;
        dataIndex += dataWritten;
        if( dataSize > 0) log_file.log(LOG_ERROR,"Data not completely written!");
    }
    while(dataSize > 0);  // Write the whole thing!

    // Wrote the whole string.
//    if(!FlushFileBuffers(commFile))
//    {
//    	log_file.log(LOG_ERROR,"FlushFileBuffers error %ld",(long)GetLastError);
//    }
    return NoError;

ErrorExit:
    return SendError;
}


#else

Error Comm::HandleWriteData(LPOVERLAPPED unused, BYTE *data, DWORD dataSize)
{
   lpCommHeader header;
	COMSTAT comStat;
   DWORD   dwErrors;
   DWORD   dataWritten = 0;

   // No need to continue if we're not communicating.
   if (commFile == NULL)
   {
   	log_file.log(LOG_ERROR,"COM_ERROR:sending data with comm port open");
      return SendError;
   }

	// make sure we aren't BYTEing off more than we can chew
   DWORD packetSize = dataSize + sizeof(CommHeader);
   if( packetSize > MaxPacketSize )
   {
   	log_file.log(LOG_ERROR,"COM_ERROR:data exceeds MAX_DATA_SIZE: %ld",(long)packetSize);
      return SendError;
   }


   // construct header and copy data to a new buffer
   // that will be deleted by actual sending function
   BYTE *packet = new BYTE[packetSize];
   if(packet == NULL)
   {
		log_file.log(LOG_ERROR,"WriteCommString allocation error %lx",(long)packetSize);
   	return SendError;
   }

   header = (lpCommHeader)packet;
   header->check1        = check1;
   header->check2        = check2;
   header->dataSize      = (short)dataSize;
   header->checkSum      = computeChecksum((char *)data, (WORD)dataSize);
   memcpy(&packet[sizeof(CommHeader)], data, dataSize);

   //	log_file.log("Comm::HandleWriteData handle %lx data %lx, size %lx, check %lx",(long)commFile,(long)data,(long)dataSize);
	// send message on its merry way
   if (!WriteFile(commFile, packet, packetSize, &dataWritten, NULL))
	{

   	// WriteFile failed.
      log_file.log(LOG_ERROR,"COMM_ERROR - WriteFile: %ld, commFile %lx, dataWritten %lx",(long)GetLastError(),(long)commFile,(long)dataWritten);
   	if (!ClearCommError(commFile, &dwErrors, &comStat))
      {
      	log_file.log(LOG_ERROR,"COM_ERROR:ClearCommError failure %ld",(long)GetLastError());
      }
      else
      {
   		ReportCommStatus( dwErrors, &comStat);
      }

      return SendError;
   }

   if( dataWritten != dataSize )
   {
   	log_file.log(LOG_ERROR,"COM_ERROR:Error to writing to CommFile: BytesWritten < BytesToWrite");
      return SendError;
   }

   return NoError;
}
#endif

//
//  FUNCTION: Process( void *)
//
//  PURPOSE: Checks comm for pending data and processes it
//
//  PARAMETERS:
//     myThis - pointer to calling class for callBack function
//
//  RETURN VALUE:
//    TRUE if packet was read in from port.  FALSE otherwise.
//
//  COMMENTS:
//
//    This function reads the data out of the Comm stream and puts it into
//    the Comm buffer.  It checks for data integrity and Post the data if a
//    full packet is received.  NOTE that for ReadFile to return if no data
//    is present requires the correct TIMEOUT settings
//
//    We assume that partial messages are possible and sometimes data will
//    get corrupted.

bool Comm::Process( void *myThis)
{
   lpCommHeader header;
   BYTE *dataPtr;
   int   currentByte,deadBytes=0;

   if( !commFile || commFailure)
   	return false;


#if OVERLAPPED_FLAG
	// the reading is going on in a separate thread so the bytes will appear in the readBuffer

#else
   // Read whatever bytes are in the stream; we set the timeouts so that
   // ReadFile will return immediately with whatever data is available

   if (!ReadFile(commFile, &readBuffer[readBufferPos],
                 READ_BUFFER_SIZE-readBufferPos,&bytesRead, NULL))
   {
      log_file.log(LOG_ERROR,"COMM_ERROR - ReadFile: %ld, commFile %lx, bytesRead %ld",(long)GetLastError(),(long)commFile,(long)bytesRead);
   	if (!ClearCommError(commFile, &dwErrors, &comStat))
      {
      	log_file.log(LOG_ERROR,"COM_ERROR:ClearCommError failure %ld",(long)GetLastError());
      }
      else
      {
   		ReportCommStatus( dwErrors, &comStat);
      }
      return false;
   }
#endif

#if OVERLAPPED_FLAG
	EnterCriticalSection(&ReadSection);
#endif
	// process any data read in
   if( bytesRead > 0 )
   {
//		log_file.log("Comm::ReadFile %lx bytes, readBufferPos %lx",(long)bytesRead,(long)readBufferPos);
   	// we start searching through the data receieved on the lookout for a
      // likely packet header.  Usually, it should be the first bytes in
      // the buffer, but we can't assume that.
   	currentByte = 0;
   	readBufferPos += bytesRead; // bytes in readBuffer = any previous + newly read in data
      while(currentByte < readBufferPos)
      {
      	header = (lpCommHeader)&readBuffer[currentByte];
      	if( ( header->check1 == check1 ) && // first byte matches
             ( header->check2 == check2 ) && // second byte matches
             ( header->dataSize <= MaxPacketSize ) && // simple bounds checking here
             ( header->dataSize >= 0 ) &&
             ( readBufferPos >= ( currentByte + (int)sizeof(CommHeader) + header->dataSize))) // make sure full packet has arrived
         {
         	// we have a strong potential packet start, calculate checksum
           	dataPtr = &readBuffer[currentByte + sizeof(CommHeader)];
         	if(header->checkSum == computeChecksum((char *)dataPtr,header->dataSize))
  	         {
   	      	// packet is good, post data
				   if( receive_data )
				   	receive_data( dataPtr,header->dataSize, myThis );

               // readjust buffer variables
					currentByte += sizeof(CommHeader) + header->dataSize;
               readBufferPos -= currentByte;

               // shift remaining data(if any) to beginning of buffer
               // Note that if we had some corrupt data, it will not get
               // process and will be shifted out of the buffer as soon as
               // some good data is processed.
               if( readBufferPos )
               	shiftReadBuffer( currentByte, readBufferPos );

               currentByte = 0;
            }
            else
            {
              	// checksum didn't compute, it may not have been the right
               // header or data could be corrupt, in any case, skip and
               // keep trying
               currentByte++;
               deadBytes++;
            }
         }
         else
	      	currentByte++;
      }
   }
#if OVERLAPPED_FLAG
	bytesRead = 0;
   LeaveCriticalSection(&ReadSection);
#endif

   if(deadBytes)
   	log_file.log(LOG_ERROR,"Comm::***deadBytes: %lx",(long)deadBytes);
	// otherwise, no data was pending
   return true;
}


#if OVERLAPPED_FLAG

//
//  FUNCTION: CloseReadThread
//
//  PURPOSE: Close the Read Thread.
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
//    Closes the Read thread by signaling the CloseEvent.
//    Purges any outstanding reads on the comm port.
//
//    Note that terminating a thread leaks memory (read the docs).
//    Besides the normal leak incurred, there is an event object
//    that doesn't get closed.  This isn't worth worrying about
//    since it shouldn't happen anyway.
//
//

void Comm::CloseReadThread()
{
    // If it exists...
    if (g_hReadThread)
    {
//        log_file.log("Closing Read Thread");

        // Signal the event to close the worker threads.
        SetEvent(g_hCloseEvent);

        // Purge all outstanding reads
        PurgeComm(commFile, PURGE_RXABORT | PURGE_RXCLEAR);

        // Wait 10 seconds for it to exit.  Shouldn't happen.
        if (WaitForSingleObject(g_hReadThread, 10000) == WAIT_TIMEOUT)
        {
            log_file.log(LOG_ERROR,"Read thread not exiting.  Terminating it.");

            TerminateThread(g_hReadThread, 0);

            // The ReadThread cleans up these itself if it terminates
            // normally.
            CloseHandle(g_hReadThread);
            g_hReadThread = 0;
            g_dwReadThreadID = 0;
        }
    }
}


//
//  FUNCTION: CloseWriteThread
//
//  PURPOSE: Closes the Write Thread.
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
//    Closes the write thread by signaling the CloseEvent.
//    Purges any outstanding writes on the comm port.
//
//    Note that terminating a thread leaks memory (read the docs).
//    Besides the normal leak incurred, there is an event object
//    that doesn't get closed.  This isn't worth worrying about
//    since it shouldn't happen anyway.
//
//

void Comm::CloseWriteThread()
{
    // If it exists...
    if (g_hWriteThread)
    {
//        log_file.log("Closing Write Thread");

        // Signal the event to close the worker threads.
        SetEvent(g_hCloseEvent);

        // Purge all outstanding writes.
        PurgeComm(commFile, PURGE_TXABORT | PURGE_TXCLEAR);

        // Wait 10 seconds for it to exit.  Shouldn't happen.
        if (WaitForSingleObject(g_hWriteThread, 10000) == WAIT_TIMEOUT)
        {
            log_file.log(LOG_ERROR,"Write thread not exiting.  Terminating it.");

            TerminateThread(g_hWriteThread, 0);

            // The WriteThread cleans up these itself if it terminates
            // normally.
            CloseHandle(g_hWriteThread);
            g_hWriteThread = 0;
            g_dwWriteThreadID = 0;

        }
    }
}

//
//  FUNCTION: StartWriteThreadProc(LPVOID)
//
//  PURPOSE: The starting point for the Write thread.
//
//  PARAMETERS:
//    lpvParam - unused.
//
//  RETURN VALUE:
//    DWORD - unused.
//
//  COMMENTS:
//
//    The Write thread uses a PeekMessage loop to wait for a string to write,
//    and when it gets one, it writes it to the Comm port.  If the CloseEvent
//    object is signaled, then it exits.  The use of messages to tell the
//    Write thread what to write provides a natural desynchronization between
//    the UI and the Write thread.
//
//

DWORD WINAPI Comm::StartWriteThreadProc(LPVOID lpvParam)
{
    BYTE *buffer;
    DWORD bufferSize;
    HANDLE HandlesToWaitFor[2];
    DWORD dwHandleSignaled;
    OVERLAPPED overlappedWrite = {0, 0, 0, 0, NULL}; // Needed for overlapped I/O.
    class Comm *comm = (class Comm *)lpvParam;  // get this pointer to the associated comm class

    HandlesToWaitFor[0] = comm->g_hCloseEvent;
    HandlesToWaitFor[1] = comm->g_hWriteEvent;

    overlappedWrite.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (overlappedWrite.hEvent == NULL)
    {
        log_file.log(LOG_ERROR,"Unable to CreateEvent: %ld",(long)GetLastError());
        comm->commFailure = true;
        goto EndWriteThread;
    }

    // This is the main loop.  Loop until we break out.
    while (TRUE)
    {
	    dwHandleSignaled =
                  WaitForMultipleObjects(2, HandlesToWaitFor, FALSE, INFINITE);

       switch(dwHandleSignaled)
       {
          case WAIT_OBJECT_0:     // CloseEvent signaled!
          {
             // Time to exit.
             goto EndWriteThread;
          }

          case WAIT_OBJECT_0 + 1: // New message was received.
          {
#if !USE_QUEUE
             // There are 2 buffers; if writeBufferFlag is false, the data is
             // in writeBufferA, otherwise it is in writeBufferB

				 EnterCriticalSection(&comm->WriteSection);
             buffer = (comm->writeBufferFlag?comm->writeBufferA:comm->writeBufferB);
             bufferSize = comm->writeBufferPos;
             comm->writeBufferFlag = !comm->writeBufferFlag;
             LeaveCriticalSection(&comm->WriteSection);

             // Write the string to the comm port.  HandleWriteData
             // does not return until the whole string has been written,
             // an error occurs or until the CloseEvent is signaled.
             if (comm->HandleWriteData(&overlappedWrite, buffer, bufferSize) != NoError)
             {
                // If it failed, either we got a signal to end or there
                // really was a failure.
					 log_file.log(LOG_ERROR,"HandleWriteData failed or close event occured!");
                goto EndWriteThread;
             }
             ResetEvent(comm->g_hWriteEvent);
#else
             EnterCriticalSection(&comm->WriteSection);
				 buffer = comm->GetLatestData(&bufferSize,1);  // we are responsible for freeing buffer
             LeaveCriticalSection(&comm->WriteSection);

             while( buffer )
             {

	             if (comm->HandleWriteData(&overlappedWrite, buffer, bufferSize) != NoError)
   	          {
      	          // If it failed, either we got a signal to end or there
         	       // really was a failure.
						 log_file.log(LOG_ERROR,"HandleWriteData failed or close event occured!");
                   delete buffer;
               	 goto EndWriteThread;
	             }

                delete buffer;

	             EnterCriticalSection(&comm->WriteSection);
					 buffer = comm->GetLatestData(&bufferSize,0);  // we are responsible for freeing buffer
      	       LeaveCriticalSection(&comm->WriteSection);
             }

             ResetEvent(comm->g_hWriteEvent);
#endif
             break;
          }

          case WAIT_FAILED:       // Wait failed.  Shouldn't happen.
          {
             log_file.log(LOG_ERROR,"WriteThread WAIT_FAILED: %ld",(long)GetLastError());
             comm->commFailure = true;
             goto EndWriteThread;
          }

          default:                // This case should never occur.
          {
             log_file.log(LOG_ERROR,"Unexpected Wait return value '%lx'",(long)dwHandleSignaled);
             comm->commFailure = true;
             goto EndWriteThread;
          }

       }

    } // End of main loop.

    // Thats the end.  Now clean up.
EndWriteThread:

//    log_file.log("Write thread shutting down!");

    PurgeComm(comm->commFile, PURGE_TXABORT | PURGE_TXCLEAR);

//    log_file.log("Write thread shutting down2");
    CloseHandle(overlappedWrite.hEvent);

    comm->g_dwWriteThreadID = 0;
    CloseHandle(comm->g_hWriteThread);
    comm->g_hWriteThread = 0;
//    log_file.log("Write thread shutting down3");
    return 0;
}

//
//  FUNCTION: StartReadThreadProc(LPVOID)
//
//  PURPOSE: This is the starting point for the Read Thread.
//
//  PARAMETERS:
//    lpvParam - unused.
//
//  RETURN VALUE:
//    DWORD - unused.
//
//  COMMENTS:
//
//    The Read Thread uses overlapped ReadFile and sends any strings
//    read from the comm port to the UI to be printed.  This is
//    eventually done through a PostMessage so that the Read Thread
//    is never away from the comm port very long.  This also provides
//    natural desynchronization between the Read thread and the UI.
//
//    If the CloseEvent object is signaled, the Read Thread exits.
//
//    Note that there is absolutely *no* interpretation of the data,
//    which means no terminal emulation.  It basically means that this
//    sample is pretty useless as a TTY program.
//
//	  Separating the Read and Write threads is natural for a application
//    like this sample where there is no need for synchronization between
//    reading and writing.  However, if there is such a need (for example,
//    most file transfer algorithms synchronize the reading and writing),
//    then it would make a lot more sense to have a single thread to handle
//    both reading and writing.
//
//

DWORD WINAPI Comm::StartReadThreadProc(LPVOID lpvParam)
{
    char szInputBuffer[INPUTBUFFERSIZE];
    DWORD nNumberOfBytesRead;

    HANDLE HandlesToWaitFor[3];
    DWORD dwHandleSignaled;

    DWORD fdwEvtMask;

    // Needed for overlapped I/O (ReadFile)
    OVERLAPPED overlappedRead  = {0, 0, 0, 0, NULL};

    // Needed for overlapped Comm Event handling.
    OVERLAPPED overlappedCommEvent = {0, 0, 0, 0, NULL};

    // get this pointer to the associated comm class
    class Comm *comm = (class Comm *)lpvParam;

    // Lets put an event in the Read overlapped structure.
    overlappedRead.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (overlappedRead.hEvent == NULL)
    {
        log_file.log(LOG_ERROR,"Unable to CreateEvent: %ld",(long)GetLastError());
        comm->commFailure = true;
        goto EndReadThread;
    }

    // And an event for the CommEvent overlapped structure.
    overlappedCommEvent.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (overlappedCommEvent.hEvent == NULL)
    {
        log_file.log(LOG_ERROR,"Unable to CreateEvent: %ld",(long)GetLastError());
        comm->commFailure = true;
        goto EndReadThread;
    }

    // We will be waiting on these objects.
    HandlesToWaitFor[0] = comm->g_hCloseEvent;
    HandlesToWaitFor[1] = overlappedCommEvent.hEvent;
    HandlesToWaitFor[2] = overlappedRead.hEvent;


    // Setup CommEvent handling.

        // Set the comm mask so we receive error signals.
    if (!SetCommMask(comm->commFile, EV_ERR))
    {
        log_file.log(LOG_ERROR,"Unable to SetCommMask: %ld",(long)GetLastError());
        comm->commFailure = true;
        goto EndReadThread;
    }


    // Start waiting for CommEvents (Errors)
    if (!comm->SetupCommEvent(&overlappedCommEvent, &fdwEvtMask))
    {
        log_file.log(LOG_ERROR,"Unable to SetupCommEvent: %ld",(long)GetLastError());
        comm->commFailure = true;
        goto EndReadThread;
    }

    // Start waiting for Read events.
    if (!comm->SetupReadEvent(&overlappedRead,
                szInputBuffer, INPUTBUFFERSIZE,
                &nNumberOfBytesRead))
    {
        log_file.log(LOG_ERROR,"Unable to SetupReadEvent: %ld",(long)GetLastError());
        comm->commFailure = true;
        goto EndReadThread;
    }

    // Keep looping until we break out.
    while (TRUE)
    {
        // Wait until some event occurs (data to read; error; stopping).
        dwHandleSignaled =
            WaitForMultipleObjects(3, HandlesToWaitFor, FALSE, INFINITE);

        // Which event occured?
        switch(dwHandleSignaled)
        {
            case WAIT_OBJECT_0:     // Signal to end the thread.
            {
                // Time to exit.
                goto EndReadThread;
            }

            case WAIT_OBJECT_0 + 1: // CommEvent signaled.
            {
                // Handle the CommEvent.
                if (!comm->HandleCommEvent(&overlappedCommEvent, &fdwEvtMask, TRUE))
                {
						  log_file.log(LOG_ERROR,"HandleCommEvent failed");
                    comm->commFailure = true;
                    goto EndReadThread;
                }

                // Start waiting for the next CommEvent.
                if (!comm->SetupCommEvent(&overlappedCommEvent, &fdwEvtMask))
                {
                    log_file.log(LOG_ERROR,"SetupCommEvent failed");
                    comm->commFailure = true;
                    goto EndReadThread;
                }
                break;
            }

            case WAIT_OBJECT_0 + 2: // Read Event signaled.
            {
                // Get the new data!
//log_file.log("*****data to read event signalled");
                if (!comm->HandleReadEvent(&overlappedRead,
                            szInputBuffer, INPUTBUFFERSIZE,
                            &nNumberOfBytesRead))
                {
                    comm->commFailure = true;
                    goto EndReadThread;
                }

                // Wait for more new data.
                if (!comm->SetupReadEvent(&overlappedRead,
                            szInputBuffer, INPUTBUFFERSIZE,
                            &nNumberOfBytesRead))
                {
                    comm->commFailure = true;
                    goto EndReadThread;
                }
                break;
            }

            case WAIT_FAILED:       // Wait failed.  Shouldn't happen.
            {
                log_file.log(LOG_ERROR,"Read WAIT_FAILED: %ld",(long)GetLastError());
                comm->commFailure = true;
                goto EndReadThread;
            }

            default:    // This case should never occur.
            {
                log_file.log(LOG_ERROR,"Unexpected Wait return value '%lx'",(long)dwHandleSignaled);
                comm->commFailure = true;
                goto EndReadThread;
            }
        } // End of switch(dwHandleSignaled).

    } // End of while(TRUE) loop.


    // Time to clean up Read Thread.
  EndReadThread:

//log_file.log("Read thread shutting down.");
    PurgeComm(comm->commFile, PURGE_RXABORT | PURGE_RXCLEAR);
    CloseHandle(overlappedRead.hEvent);
    CloseHandle(overlappedCommEvent.hEvent);
    comm->g_dwReadThreadID = 0;
    CloseHandle(comm->g_hReadThread);
    comm->g_hReadThread = 0;
    return 0;
}


//
//  FUNCTION: SetupReadEvent(LPOVERLAPPED, LPSTR, DWORD, LPDWORD)
//
//  PURPOSE: Sets up an overlapped ReadFile
//
//  PARAMETERS:
//    lpOverlappedRead      - address of overlapped structure to use.
//    lpszInputBuffer       - Buffer to place incoming bytes.
//    dwSizeofBuffer        - size of lpszInputBuffer.
//    lpnNumberOfBytesRead  - address of DWORD to place the number of read bytes.
//
//  RETURN VALUE:
//    TRUE if able to successfully setup the ReadFile.  FALSE if there
//    was a failure setting up or if the CloseEvent object was signaled.
//
//  COMMENTS:
//
//    This function is a helper function for the Read Thread.  This
//    function sets up the overlapped ReadFile so that it can later
//    be waited on (or more appropriatly, so the event in the overlapped
//    structure can be waited upon).  If there is data waiting, it is
//    handled and the next ReadFile is initiated.
//    Another possible reason for returning FALSE is if the comm port
//    is closed by the service provider.
//
//
//

BOOL Comm::SetupReadEvent(LPOVERLAPPED lpOverlappedRead,
    LPSTR lpszInputBuffer, DWORD dwSizeofBuffer,
    LPDWORD lpnNumberOfBytesRead)
{
    DWORD dwLastError;

  StartSetupReadEvent:

    // Make sure the CloseEvent hasn't been signaled yet.
    // Check is needed because this function is potentially recursive.
    if (WAIT_TIMEOUT != WaitForSingleObject(g_hCloseEvent,0))
        return FALSE;

    // Start the overlapped ReadFile.
    if (ReadFile(commFile,
            lpszInputBuffer, dwSizeofBuffer,
            lpnNumberOfBytesRead, lpOverlappedRead))
    {
        // This would only happen if there was data waiting to be read.

//        log_file.log("Data waiting for ReadFile.");

        // Handle the data.
        if (!HandleReadData(lpszInputBuffer, *lpnNumberOfBytesRead))
        {
            return FALSE;
        }

        // Start waiting for more data.
        goto StartSetupReadEvent;
    }

    // ReadFile failed.  Expected because of overlapped I/O.
    dwLastError = GetLastError();


    // LastError was ERROR_IO_PENDING, as expected.
    if (dwLastError == ERROR_IO_PENDING)
    {
//        log_file.log("Waiting for data from comm connection.");
        return TRUE;
    }

    // Its possible for this error to occur if the
    // service provider has closed the port.  Time to end.
    if (dwLastError == ERROR_INVALID_HANDLE)
    {
        log_file.log(LOG_ERROR,"ERROR_INVALID_HANDLE: Likely that the Service Provider has closed the port.");
        return FALSE;
    }

    // Unexpected error. No idea what could cause this to happen.
    log_file.log(LOG_ERROR,"Unexpected ReadFile error: %ld",(long)dwLastError);

    commFailure = true;
    return FALSE;
}


//
//  FUNCTION: HandleReadEvent(LPOVERLAPPED, LPSTR, DWORD, LPDWORD)
//
//  PURPOSE: Retrieves and handles data when there is data ready.
//
//  PARAMETERS:
//    lpOverlappedRead      - address of overlapped structure to use.
//    lpszInputBuffer       - Buffer to place incoming bytes.
//    dwSizeofBuffer        - size of lpszInputBuffer.
//    lpnNumberOfBytesRead  - address of DWORD to place the number of read bytes.
//
//  RETURN VALUE:
//    TRUE if able to successfully retrieve and handle the available data.
//    FALSE if unable to retrieve or handle the data.
//
//  COMMENTS:
//
//    This function is another helper function for the Read Thread.  This
//    is the function that is called when there is data available after
//    an overlapped ReadFile has been setup.  It retrieves the data and
//    handles it.
//
//

BOOL Comm::HandleReadEvent(LPOVERLAPPED lpOverlappedRead,
    LPSTR lpszInputBuffer, DWORD dwSizeofBuffer,
    LPDWORD lpnNumberOfBytesRead)
{
    DWORD dwLastError;

    if (GetOverlappedResult(commFile,
            lpOverlappedRead, lpnNumberOfBytesRead, FALSE))
    {
        return HandleReadData(lpszInputBuffer, *lpnNumberOfBytesRead);
    }

    // Error in GetOverlappedResult; handle it.

    dwLastError = GetLastError();

    // Its possible for this error to occur if the
    // service provider has closed the port.  Time to end.
    if (dwLastError == ERROR_INVALID_HANDLE)
    {
        log_file.log(LOG_ERROR,"ERROR_INVALID_HANDLE: Likely that the Service Provider has closed the port.");
        return FALSE;
    }

    log_file.log(LOG_ERROR,"Unexpected GetOverlappedResult Read Error: %ld",(long)dwLastError);

    commFailure=true;
    return FALSE;
}

//
//  FUNCTION: HandleReadData(LPCSTR, DWORD)
//
//  PURPOSE: Deals with data after its been read from the comm file.
//
//  PARAMETERS:
//    lpszInputBuffer  - Buffer to place incoming bytes.
//    dwSizeofBuffer   - size of lpszInputBuffer.
//
//  RETURN VALUE:
//    TRUE if able to successfully handle the data.
//    FALSE if unable to allocate memory or handle the data.
//
//  COMMENTS:
//
//    This function is yet another helper function for the Read Thread.
//    It copies the new data into the readBuffer where it will be process
//    when the Comm::process function is called.
//
//

BOOL Comm::HandleReadData(LPCSTR lpszInputBuffer, DWORD dwSizeofBuffer)
{
//log_file.log("HandleReadData with dwSizeofBuffer %lx and bytesRead %ld",(long)dwSizeofBuffer,(long)bytesRead);
	EnterCriticalSection(&ReadSection);

	// If we got data and didn't just time out empty...
//log_file.log("HandleReadData 1 with dwSizeofBuffer %lx and bytesRead %ld",(long)dwSizeofBuffer,(long)bytesRead);
   if (dwSizeofBuffer)
   {
      // Do something with the bytes read.
//      log_file.log("Got something from Comm port!!!");


      // check to make sure new data won't overflow read buffer
      if( readBufferPos + dwSizeofBuffer > READ_BUFFER_SIZE )
      {
      	log_file.log(LOG_ERROR,"COMM_ERROR: readBuffer overflow of %lx bytes",(long)(readBufferPos+dwSizeofBuffer-READ_BUFFER_SIZE));
			LeaveCriticalSection(&ReadSection);
         return false;
      }

      memcpy(&readBuffer[readBufferPos], lpszInputBuffer, dwSizeofBuffer);
      bytesRead += dwSizeofBuffer;
    }

	LeaveCriticalSection(&ReadSection);
   return true;
}

//
//  FUNCTION: SetupCommEvent(LPOVERLAPPED, LPDWORD)
//
//  PURPOSE: Sets up the overlapped WaitCommEvent call.
//
//  PARAMETERS:
//    lpOverlappedCommEvent - Pointer to the overlapped structure to use.
//    lpfdwEvtMask          - Pointer to DWORD to received Event data.
//
//  RETURN VALUE:
//    TRUE if able to successfully setup the WaitCommEvent.
//    FALSE if unable to setup WaitCommEvent, unable to handle
//    an existing outstanding event or if the CloseEvent has been signaled.
//
//  COMMENTS:
//
//    This function is a helper function for the Read Thread that sets up
//    the WaitCommEvent so we can deal with comm events (like Comm errors)
//    if they occur.
//
//

BOOL Comm::SetupCommEvent(LPOVERLAPPED lpOverlappedCommEvent,
    LPDWORD lpfdwEvtMask)
{
    DWORD dwLastError;

  StartSetupCommEvent:

    // Make sure the CloseEvent hasn't been signaled yet.
    // Check is needed because this function is potentially recursive.
    if (WAIT_TIMEOUT != WaitForSingleObject(g_hCloseEvent,0))
        return FALSE;

    // Start waiting for Comm Errors.
    if (WaitCommEvent(commFile, lpfdwEvtMask, lpOverlappedCommEvent))
    {
        // This could happen if there was an error waiting on the
        // comm port.  Lets try and handle it.

        log_file.log(LOG_ERROR,"Event (Error) waiting before WaitCommEvent.");

        if (!HandleCommEvent(NULL, lpfdwEvtMask, FALSE))
            return FALSE;

        // What could cause infinite recursion at this point?
        goto StartSetupCommEvent;
    }

    // We expect ERROR_IO_PENDING returned from WaitCommEvent
    // because we are waiting with an overlapped structure.

    dwLastError = GetLastError();

    // LastError was ERROR_IO_PENDING, as expected.
    if (dwLastError == ERROR_IO_PENDING)
    {
        log_file.log("Waiting for a CommEvent (Error) to occur.");
        return TRUE;
    }

    // Its possible for this error to occur if the
    // service provider has closed the port.  Time to end.
    if (dwLastError == ERROR_INVALID_HANDLE)
    {
        log_file.log(LOG_ERROR,"ERROR_INVALID_HANDLE: Likely that the Service Provider has closed the port.");
        return FALSE;
    }

    // Unexpected error. No idea what could cause this to happen.
    log_file.log(LOG_ERROR,"Unexpected WaitCommEvent error: %ld",(long)dwLastError);
    return FALSE;
}

//
//  FUNCTION: HandleCommEvent(LPOVERLAPPED, LPDWORD, BOOL)
//
//  PURPOSE: Handle an outstanding Comm Event.
//
//  PARAMETERS:
//    lpOverlappedCommEvent - Pointer to the overlapped structure to use.
//    lpfdwEvtMask          - Pointer to DWORD to received Event data.
//     fRetrieveEvent       - Flag to signal if the event needs to be
//                            retrieved, or has already been retrieved.
//
//  RETURN VALUE:
//    TRUE if able to handle a Comm Event.
//    FALSE if unable to setup WaitCommEvent, unable to handle
//    an existing outstanding event or if the CloseEvent has been signaled.
//
//  COMMENTS:
//
//    This function is a helper function for the Read Thread that (if
//    fRetrieveEvent == TRUE) retrieves an outstanding CommEvent and
//    deals with it.  The only event that should occur is an EV_ERR event,
//    signalling that there has been an error on the comm port.
//
//    Normally, comm errors would not be put into the normal data stream
//    as this sample is demonstrating.  Putting it in a status bar would
//    be more appropriate for a real application.
//
//

BOOL Comm::HandleCommEvent(LPOVERLAPPED lpOverlappedCommEvent,
    LPDWORD lpfdwEvtMask, BOOL fRetrieveEvent)
{
    DWORD dwDummy;
    LPSTR lpszOutput;
    char szError[128] = "";
    DWORD dwErrors;
    DWORD nOutput;
    DWORD dwLastError;


    lpszOutput = (char *)LocalAlloc(LPTR,256);
    if (lpszOutput == NULL)
    {
        log_file.log(LOG_ERROR,"HandleCommEvent::LocalAlloc: %ld",(long)GetLastError());
        return FALSE;
    }

    // If this fails, it could be because the file was closed (and I/O is
    // finished) or because the overlapped I/O is still in progress.  In
    // either case (or any others) its a bug and return FALSE.
    if (fRetrieveEvent)
        if (!GetOverlappedResult(commFile,
                lpOverlappedCommEvent, &dwDummy, FALSE))
        {
            dwLastError = GetLastError();

            // Its possible for this error to occur if the
            // service provider has closed the port.  Time to end.
            if (dwLastError == ERROR_INVALID_HANDLE)
            {
                log_file.log(LOG_ERROR,"ERROR_INVALID_HANDLE: Likely that the Service Provider has closed the port.");
                return FALSE;
            }

            log_file.log(LOG_ERROR,"Unexpected GetOverlappedResult for WaitCommEvent: %ld",(long)dwLastError);
            return FALSE;
        }

    // Was the event an error?
    if (*lpfdwEvtMask & EV_ERR)
    {
        // Which error was it?
        if (!ClearCommError(commFile, &dwErrors, NULL))
        {
            dwLastError = GetLastError();

            // Its possible for this error to occur if the
            // service provider has closed the port.  Time to end.
            if (dwLastError == ERROR_INVALID_HANDLE)
            {
                log_file.log(LOG_ERROR,"ERROR_INVALID_HANDLE: Likely that the Service Provider has closed the port.");
                return FALSE;
            }

            log_file.log(LOG_ERROR,"ClearCommError: %ld",(long)GetLastError());
            return FALSE;
        }

        // Its possible that multiple errors occured and were handled
        // in the last ClearCommError.  Because all errors were signaled
        // individually, but cleared all at once, pending comm events
        // can yield EV_ERR while dwErrors equals 0.  Ignore this event.
        if (dwErrors == 0)
        {
            strcat(szError, "NULL Error");
        }

        if (dwErrors & CE_FRAME)
        {
            if (szError[0])
                strcat(szError," and ");

            strcat(szError,"CE_FRAME");
        }

        if (dwErrors & CE_OVERRUN)
        {
            if (szError[0])
                strcat(szError," and ");

            strcat(szError,"CE_OVERRUN");
        }

        if (dwErrors & CE_RXPARITY)
        {
            if (szError[0])
                strcat(szError," and ");

            strcat(szError,"CE_RXPARITY");
        }

        if (dwErrors & ~ (CE_FRAME | CE_OVERRUN | CE_RXPARITY))
        {
            if (szError[0])
                strcat(szError," and ");

            strcat(szError,"EV_ERR Unknown EvtMask");
        }


        nOutput = wsprintf(lpszOutput,
            "Comm Event: '%s', EvtMask = '%lx'\n",
            szError, dwErrors);

//        PostCommData(lpszOutput, nOutput);
		  log_file.log(LOG_ERROR,lpszOutput);
        LocalFree(lpszOutput);
        return TRUE;

    }

    // Should not have gotten here.  Only interested in ERR conditions.

    log_file.log(LOG_ERROR,"Unexpected comm event %lx",(long)lpfdwEvtMask);
    return FALSE;
}

#endif


//
//  FUNCTION: ReportCommStatus( DWORD dwErrors, LPCOMSTAT lpStat)
//
//  PURPOSE: logs extended comm error information
//
//  PARAMETERS:
//    dwErrors  - error code bit field
//    lpStat - pointer to comm status
//
//  RETURN VALUE:
//    none.
//
//  COMMENTS:
//

void Comm::ReportCommStatus( DWORD dwErrors, LPCOMSTAT lpStat)
{
	bool    fOOP, fOVERRUN, fPTO, fRXOVER, fRXPARITY, fTXFULL;
   bool    fBREAK, fDNS, fFRAME, fIOE, fMODE;

   // Get error flags.
   fDNS = dwErrors & CE_DNS;
   fIOE = dwErrors & CE_IOE;
   fOOP = dwErrors & CE_OOP;
   fPTO = dwErrors & CE_PTO;
   fMODE = dwErrors & CE_MODE;
   fBREAK = dwErrors & CE_BREAK;
   fFRAME = dwErrors & CE_FRAME;
   fRXOVER = dwErrors & CE_RXOVER;
   fTXFULL = dwErrors & CE_TXFULL;
   fOVERRUN = dwErrors & CE_OVERRUN;
   fRXPARITY = dwErrors & CE_RXPARITY;
	log_file.log(LOG_ERROR,"**************************************************************");
   log_file.log(LOG_ERROR,"   Comm Error flags: fDNS(%ld),fIOE(%ld),fOOP(%ld),fPTO(%ld)",(long)fDNS,(long)fIOE,(long)fOOP,(long)fPTO);
   log_file.log(LOG_ERROR,"                     fMODE(%ld),fBREAK(%ld),fFRAME(%ld),fRXOVER(%ld)",(long)fMODE,(long)fBREAK,(long)fFRAME,(long)fRXOVER);
   log_file.log(LOG_ERROR,"                     fTXFULL(%ld),fOVERRUN(%ld),fRXPARITY(%ld)",(long)fTXFULL,(long)fOVERRUN,(long)fRXPARITY);
   log_file.log(LOG_ERROR,"   Comm status:      fCtsHold(%ld),fDsrHold(%ld),fRlsdHold(%ld)",(long)lpStat->fCtsHold,(long)lpStat->fDsrHold,(long)lpStat->fRlsdHold);
   log_file.log(LOG_ERROR,"                     fXoffHold(%ld),fXoffSent(%ld),fEof(%ld)",(long)lpStat->fXoffHold,(long)lpStat->fXoffSent,(long)lpStat->fEof);
   log_file.log(LOG_ERROR,"                     fTxim(%ld),cbInQue(%ld),cbOutQue(%ld)",(long)lpStat->fTxim,(long)lpStat->cbInQue,(long)lpStat->cbOutQue);
	log_file.log(LOG_ERROR,"**************************************************************");
}

//
//  FUNCTION: shiftReadBuffer( int start, int amount )
//
//  PURPOSE: copies data in readBuffer to beginning of buffer
//
//  PARAMETERS:
//    start  - byte to start the copying at
//    amount - amount of data to copy
//
//  RETURN VALUE:
//    none.
//
//  COMMENTS:
//
//    If more than one message worth of data is in the read buffer, after
//    processing the first message, we 'shift' the remaining data to the
//    beginning of the buffer and process it again.
//

void Comm::shiftReadBuffer( int start, int amount )
{
	// It is possible for the source and destination to overlap,
   // and although memcpy should work, theoretically, its behaviour is
   // undefined in this situation; so we don't use it
   //
	// memcpy(&readBuffer[0],&readBuffer[start],amount);

   for( int i = 0; i < amount; i++ )
   {
   	readBuffer[i] = readBuffer[i+start];
   }
}

#if USE_QUEUE
/* The following is a specialized data queue for the comm port */
/* */

bool Comm::QueueData(BYTE *data, DWORD data_size)
{
	CommQueueElement *packet = new CommQueueElement;
   CommQueueElement *element;


   if( packet == NULL || data == NULL || data_size == 0)
   	return false;

	packet->data = new BYTE[data_size];
   if( packet->data == NULL ) return false;

   packet->next      = NULL;
   packet->data_size = data_size;
   memcpy(packet->data, data, data_size);

	if( CommQueue )
   {
   	element = CommQueue;
      while( element->next )
      	element = element->next;
      element->next = packet;
   }
   else
   {
      CommQueue = packet;
   }
}

BYTE *Comm::GetLatestData( DWORD *data_size, DWORD elements_to_keep )
{
	CommQueueElement *next;
	if( CommQueue == NULL )
   	return NULL;

	int size = QueueSize();
   int elements_to_delete = size - elements_to_keep -1;
   if( elements_to_delete < 0) elements_to_delete = 0;

	// get to last packet
	while(CommQueue->next && elements_to_delete)
   {
   	delete CommQueue->data;
      next = CommQueue->next;
      delete CommQueue;
      CommQueue = next;
      elements_to_delete--;
   }

   *data_size = CommQueue->data_size;
   BYTE *data = CommQueue->data; // calling routine responsible for freeing data
   next = CommQueue->next;
   delete CommQueue;
  	CommQueue = next;
   return data;
}

bool Comm::IsDataWaiting( void )
{
	return CommQueue?true:false;
}

int Comm::QueueSize(void)
{
	CommQueueElement *element = CommQueue;

   int count = 0;
   while(element != NULL)
   {
      count++;
   	element = element->next;
   }

   return count;
}
#endif


/*****************************************************************/
/*                                                               */
/* CRC LOOKUP TABLE                                              */
/* ================                                              */
/* The following CRC lookup table was generated automatically    */
/* by the Rocksoft^tm Model CRC Algorithm Table Generation       */
/* Program V1.0 using the following model parameters:            */
/*                                                               */
/*    Width   : 2 bytes.                                         */
/*    Poly    : 0x1021                                           */
/*    Reverse : TRUE.                                            */
/*                                                               */
/* For more information on the Rocksoft^tm Model CRC Algorithm,  */
/* see the document titled "A Painless Guide to CRC Error        */
/* Detection Algorithms" by Ross Williams                        */
/* (ross@guest.adelaide.edu.au.). This document is likely to be  */
/* in the FTP archive "ftp.adelaide.edu.au/pub/rocksoft".        */
/*                                                               */
/*****************************************************************/

WORD crctable[256] =
{
 0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
 0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7,
 0x1081, 0x0108, 0x3393, 0x221A, 0x56A5, 0x472C, 0x75B7, 0x643E,
 0x9CC9, 0x8D40, 0xBFDB, 0xAE52, 0xDAED, 0xCB64, 0xF9FF, 0xE876,
 0x2102, 0x308B, 0x0210, 0x1399, 0x6726, 0x76AF, 0x4434, 0x55BD,
 0xAD4A, 0xBCC3, 0x8E58, 0x9FD1, 0xEB6E, 0xFAE7, 0xC87C, 0xD9F5,
 0x3183, 0x200A, 0x1291, 0x0318, 0x77A7, 0x662E, 0x54B5, 0x453C,
 0xBDCB, 0xAC42, 0x9ED9, 0x8F50, 0xFBEF, 0xEA66, 0xD8FD, 0xC974,
 0x4204, 0x538D, 0x6116, 0x709F, 0x0420, 0x15A9, 0x2732, 0x36BB,
 0xCE4C, 0xDFC5, 0xED5E, 0xFCD7, 0x8868, 0x99E1, 0xAB7A, 0xBAF3,
 0x5285, 0x430C, 0x7197, 0x601E, 0x14A1, 0x0528, 0x37B3, 0x263A,
 0xDECD, 0xCF44, 0xFDDF, 0xEC56, 0x98E9, 0x8960, 0xBBFB, 0xAA72,
 0x6306, 0x728F, 0x4014, 0x519D, 0x2522, 0x34AB, 0x0630, 0x17B9,
 0xEF4E, 0xFEC7, 0xCC5C, 0xDDD5, 0xA96A, 0xB8E3, 0x8A78, 0x9BF1,
 0x7387, 0x620E, 0x5095, 0x411C, 0x35A3, 0x242A, 0x16B1, 0x0738,
 0xFFCF, 0xEE46, 0xDCDD, 0xCD54, 0xB9EB, 0xA862, 0x9AF9, 0x8B70,
 0x8408, 0x9581, 0xA71A, 0xB693, 0xC22C, 0xD3A5, 0xE13E, 0xF0B7,
 0x0840, 0x19C9, 0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF,
 0x9489, 0x8500, 0xB79B, 0xA612, 0xD2AD, 0xC324, 0xF1BF, 0xE036,
 0x18C1, 0x0948, 0x3BD3, 0x2A5A, 0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E,
 0xA50A, 0xB483, 0x8618, 0x9791, 0xE32E, 0xF2A7, 0xC03C, 0xD1B5,
 0x2942, 0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF, 0x4C74, 0x5DFD,
 0xB58B, 0xA402, 0x9699, 0x8710, 0xF3AF, 0xE226, 0xD0BD, 0xC134,
 0x39C3, 0x284A, 0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C,
 0xC60C, 0xD785, 0xE51E, 0xF497, 0x8028, 0x91A1, 0xA33A, 0xB2B3,
 0x4A44, 0x5BCD, 0x6956, 0x78DF, 0x0C60, 0x1DE9, 0x2F72, 0x3EFB,
 0xD68D, 0xC704, 0xF59F, 0xE416, 0x90A9, 0x8120, 0xB3BB, 0xA232,
 0x5AC5, 0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68, 0x3FF3, 0x2E7A,
 0xE70E, 0xF687, 0xC41C, 0xD595, 0xA12A, 0xB0A3, 0x8238, 0x93B1,
 0x6B46, 0x7ACF, 0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9,
 0xF78F, 0xE606, 0xD49D, 0xC514, 0xB1AB, 0xA022, 0x92B9, 0x8330,
 0x7BC7, 0x6A4E, 0x58D5, 0x495C, 0x3DE3, 0x2C6A, 0x1EF1, 0x0F78
};

#define INIT_REFLECTED 0xFFFF
#define XOROT          0xFFFF
WORD Comm::computeChecksum( char * dataPtr, WORD dataLength )
{
	WORD crc = INIT_REFLECTED;
	while (dataLength--)
   	crc = crctable[(crc ^ *dataPtr++) & 0xFFL] ^ (crc >> 8);
	return crc ^ XOROT;
}

};
