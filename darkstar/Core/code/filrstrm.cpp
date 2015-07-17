//================================================================
//   
// $Workfile:   filrstrm.cpp  $
// $Version$
// $Revision:   1.5  $
//   
// DESCRIPTION:
//   
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================


#include "FilStrm.h"
#include <stdlib.h>

//----------------------------------------------------------------------------
// Class "FileRStream" constructors
int a_min(int a, int b)
{
   return a < b ? a : b;
}

FileRStream::FileRStream()
{
   hFile       = INVALID_HANDLE_VALUE;
	isMine      = NO;
   buffHead    = -1;
   strm_status = STRM_CLOSED;
}


FileRStream::FileRStream(HANDLE in_hFile)
{
   if (in_hFile == INVALID_HANDLE_VALUE)
   {
      AssertWarn(0, "FileRStream::FileRStream: Bad file Handle.");
      setStatus();
   }
   else
	   clrStatus();

   hFile    = in_hFile;
	isMine   = NO;
   buffHead = -1;
}


FileRStream::FileRStream(const char* in_filename)
{
	hFile    = INVALID_HANDLE_VALUE;
	isMine   = NO;
   buffHead = -1;
	open(in_filename);
}


FileRStream::~FileRStream()
{
	close();
}


//----------------------------------------------------------------------------

Int32 FileRStream::getCapabilities(StreamCap cap) const
{
	switch(cap)
	{
		case STRM_CAP_TYPE:
			return STRM_FILE_R;

		case STRM_CAP_READ:
		case STRM_CAP_REPOSITION:
			return 1;
	}
	return 0;
}


//----------------------------------------------------------------------------

void FileRStream::setHandle(HANDLE in_hFile)
{
	close();
   if (in_hFile == INVALID_HANDLE_VALUE)
   {
      AssertWarn(0, "FileRStream::setFile: Bad file Handle.");
      setStatus();
   }
   else
	   clrStatus();

	hFile = in_hFile;
	isMine= NO;
}


//----------------------------------------------------------------------------

void FileRStream::close()
{
	if (isMine && hFile != INVALID_HANDLE_VALUE)
		if (!CloseHandle(hFile))
		{
			strm_status = STRM_UNKNOWN_ERROR;
			return;
		}

   hFile       = INVALID_HANDLE_VALUE;
	strm_status = STRM_CLOSED;
   buffHead    = -1;
}


//----------------------------------------------------------------------------

Bool FileRStream::open(const char* in_filename)
{
	close();

	// Try to open the file for binary reading.
	// if the file does not exist, fail.

   hFile = CreateFile( in_filename, GENERIC_READ, FILE_SHARE_READ, NULL, 
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
   if (hFile != INVALID_HANDLE_VALUE)
   {
	   clrStatus();
	   isMine = YES;
      return (TRUE);
	}

	strm_status = STRM_IO_ERROR;
	return FALSE;
}


//----------------------------------------------------------------------------

Int32 FileRStream::getPosition() const
{
   return ((buffHead != -1) ? 
      buffPos :
      SetFilePointer(hFile, 0, NULL, FILE_CURRENT) );
}


//----------------------------------------------------------------------------

Bool FileRStream::setPosition(Int32 in_position)
{
   if (strm_status == STRM_EOS)
      strm_status = STRM_OK;

   if (strm_status != STRM_OK)
      return 0;

   if ( (buffHead != -1) && (in_position >= buffHead) && (in_position <= buffTail) )
   {
      buffPos = in_position;
      return (TRUE);
   }
   else
      if (SetFilePointer(hFile, in_position, 0, FILE_BEGIN) != (DWORD)-1)
      {
         buffHead = -1;
         return (TRUE);
      }

   return setStatus();  //Failed to set file pointer
}


//----------------------------------------------------------------------------

UInt32 FileRStream::getSize() const
{
	if (strm_status == STRM_OK || strm_status == STRM_EOS)
	{
		DWORD high;
      return GetFileSize(hFile, &high);
	}
	return 0;
}


//----------------------------------------------------------------------------

Bool FileRStream::flush()
{
	return 1;
}


//----------------------------------------------------------------------------

Bool FileRStream::setStatus()
{
   switch (GetLastError())
   {
      case ERROR_INVALID_HANDLE:
      case ERROR_INVALID_ACCESS:
      case ERROR_TOO_MANY_OPEN_FILES:
      case ERROR_FILE_NOT_FOUND:
      case ERROR_SHARING_VIOLATION:
      case ERROR_HANDLE_DISK_FULL:
         strm_status = STRM_IO_ERROR;
         break;

      default:
         strm_status = STRM_UNKNOWN_ERROR;
   }
   return FALSE;     // Always returns false
}


//----------------------------------------------------------------------------

Bool FileRStream::write(int, const void*)
{
	AssertFatal(0,"FileRStream::write: Cannot write to read only stream.");
	strm_status = STRM_ILLEGAL_CALL;
   return FALSE;
}


//----------------------------------------------------------------------------

Bool FileRStream::read(int size, void* d)
{
   Int32 remaining;
   BYTE *p;

   if (strm_status != STRM_OK)                        // exit on stream error
      return FALSE;

	if (size)                                          // if were reading anything...
   {
      remaining = size; p = (BYTE*)d;

      if (buffHead == -1)                             // if the buffer is empty and
      {                                               // this block we want to read is
         if (size < (STRM_BUFFER_SIZE/2))             // less than half the buffer
            if (!fillCache(getPosition()))            // then fill the buffer
               return (FALSE); 
      }

      if (buffHead != -1)                             // if there is a buffer fulfill
      {                                               // as much of the request as 
         size = a_min(buffTail-buffPos+1, remaining);   // possible from the buffer
         memcpy(p, buffer+(buffPos-buffHead), size);
         remaining-= size;
         buffPos  += size;
         p        += size;   
      }
                                                      // if not all fulfilled from the
      if (remaining)                                  // buffer go to disk 
   	   if (ReadFile(hFile, p, remaining, &lastBytes, NULL))
   	   {
            if ((Int32)lastBytes != remaining)
            {
               strm_status = STRM_EOS;
               return FALSE;
            }
         }
         else
         {
   	      return setStatus();
         }

      if (buffPos > buffTail)  buffHead = -1;
   }
   return TRUE;
}


Bool FileRStream::fillCache(int in_newHead)
{
   if (getPosition() != in_newHead) 
      setPosition(in_newHead);

   if (ReadFile(hFile, buffer, STRM_BUFFER_SIZE, &lastBytes, NULL))
   {
      buffHead = in_newHead;
      buffPos  = in_newHead;
      buffTail = in_newHead + lastBytes - 1;
   }
   else
   {
      buffHead = -1;
      return setStatus();
   }
   return (TRUE);
}


