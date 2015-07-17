//================================================================
//   
// $Workfile:   filwstrm.cpp  $
// $Version$
// $Revision:   1.5  $
//   
// DESCRIPTION:
//   
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#include "FilStrm.h"
#include "resManager.h"


//----------------------------------------------------------------------------
// Class "FileWStream" constructors

FileWStream::FileWStream(bool assertIfInvalid)
{
   hFile       = INVALID_HANDLE_VALUE;
   strm_status = STRM_CLOSED;
	isMine      = NO;
   assertIsv = assertIfInvalid;
}


FileWStream::FileWStream(HANDLE in_hFile, bool assertIfInvalid)
{
   assertIsv = assertIfInvalid;
   hFile = in_hFile;
	isMine= NO;

   if (in_hFile == INVALID_HANDLE_VALUE)
   {
      AssertWarn(0, "FileWStream::setFile: Bad file Handle.");
      setStatus();
   }
   else
	   clrStatus();
}


FileWStream::FileWStream(const char* in_filename, bool assertIfInvalid)
{
   assertIsv = assertIfInvalid;
	hFile = INVALID_HANDLE_VALUE;
	isMine = NO;
	open(in_filename);
}


FileWStream::~FileWStream()
{
	close();
}


//----------------------------------------------------------------------------

Int32 FileWStream::getCapabilities(StreamCap cap) const
{
	switch(cap)
	{
		case STRM_CAP_TYPE:
			return STRM_FILE_W;

		case STRM_CAP_WRITE:
		case STRM_CAP_REPOSITION:
			return 1;
	}
	return 0;
}


//----------------------------------------------------------------------------

void FileWStream::setHandle(HANDLE in_hFile)
{
	close();
	hFile = in_hFile;
	isMine= NO;

   if (in_hFile == INVALID_HANDLE_VALUE)
   {
      AssertWarn(0, "FileWStream::setFile: Bad file Handle.");
      setStatus();
   }
   else
	   clrStatus();
}


//----------------------------------------------------------------------------

void FileWStream::close()
{
	if (isMine && hFile != INVALID_HANDLE_VALUE)
		if (!CloseHandle(hFile))
		{
			strm_status = STRM_UNKNOWN_ERROR;
			return;
		}

   hFile       = INVALID_HANDLE_VALUE;
	strm_status = STRM_CLOSED;
}


//----------------------------------------------------------------------------

Bool FileWStream::open(const char* in_filename)
{
   if(assertIsv)
   {
      AssertISV(ResourceManager::sm_pManager == NULL ||
               ResourceManager::sm_pManager->isValidWriteFileName(in_filename) == true,
               avar("Attempted write to file: %s.\n"
                     "File is not in a writable directory.", in_filename));
   }
	close();

	// Try to open the file for binary writing.
	// if the file exists truncate it.
   // if the file does not exist, create a new one

   hFile = CreateFile(in_filename, GENERIC_WRITE, 0, NULL,
                      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
   if ( hFile == INVALID_HANDLE_VALUE)
	{
      setStatus();
		return FALSE;
	}
	clrStatus();
	isMine = YES;
	return TRUE;
}


//----------------------------------------------------------------------------

Int32 FileWStream::getPosition() const
{
   return SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
}


//----------------------------------------------------------------------------

Bool FileWStream::setPosition(Int32 in_position)
{
   if (strm_status == STRM_EOS)
      strm_status = STRM_OK;

   if (strm_status != STRM_OK)
      return FALSE;

   if (SetFilePointer(hFile, in_position, 0, FILE_BEGIN) != (DWORD)-1)
      return TRUE;

   return setStatus();  //Failed to set file pointer
}


//----------------------------------------------------------------------------

UInt32 FileWStream::getSize() const
{
	if (strm_status == STRM_OK || strm_status == STRM_EOS)
	{
		DWORD high;
      return GetFileSize(hFile, &high);
	}
	return( 0 );
}


//----------------------------------------------------------------------------

Bool FileWStream::flush()
{
	return FlushFileBuffers( hFile ) != 0;
}


//----------------------------------------------------------------------------

Bool FileWStream::setStatus()
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

Bool FileWStream::write(int size,const void* d)
{
   if (strm_status != STRM_OK)
      return FALSE;

   if (size == 0)
      return TRUE;

   if (WriteFile(hFile, (void *)d, size, &lastBytes, NULL))
      return TRUE;

	return setStatus();  // Write Failed
}


//----------------------------------------------------------------------------

Bool FileWStream::read(int, void*)
{
	AssertFatal(0,"FileWStream::read: Cannot read from write only stream.");
	strm_status = STRM_ILLEGAL_CALL;
   return FALSE;
}




