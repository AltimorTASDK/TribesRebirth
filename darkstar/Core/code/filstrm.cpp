//================================================================
//   
// $Workfile:   filstrm.cpp  $
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
// Class "FileRWStream" constructors

FileRWStream::FileRWStream(bool assertIfInvalid)
{
   hFile       = INVALID_HANDLE_VALUE;;
	strm_status = STRM_CLOSED;
	isMine      = NO;
   assertIsv = assertIfInvalid;
}

FileRWStream::FileRWStream(HANDLE in_hFile, bool assertIfInvalid)
{
   hFile = in_hFile;
	isMine= NO;

   if (in_hFile == INVALID_HANDLE_VALUE)
   {
      AssertWarn(0, "FileRWStream::FileRWStream: Bad file Handle.");
      setStatus();
   }
   else
	   clrStatus();
   assertIsv = assertIfInvalid;
}


FileRWStream::FileRWStream(const char* file, bool assertIfInvalid)
{
	hFile = INVALID_HANDLE_VALUE;
	isMine= NO;
   assertIsv = assertIfInvalid;
	open(file);
}


FileRWStream::~FileRWStream()
{
	close();
}


//----------------------------------------------------------------------------

Int32 FileRWStream::getCapabilities(StreamCap cap) const
{
	switch(cap)
	{
		case STRM_CAP_TYPE:
			return STRM_FILE_RW;

		case STRM_CAP_WRITE:
		case STRM_CAP_READ:
		case STRM_CAP_REPOSITION:
			return 1;
	}
	return 0;
}


//----------------------------------------------------------------------------

void FileRWStream::setHandle(HANDLE in_hFile)
{
	close();
	hFile = in_hFile;
	isMine= NO;

   if (in_hFile == INVALID_HANDLE_VALUE)
   {
      AssertWarn(0, "FileRWStream::FileRWStream: Bad file Handle.");
      setStatus();
   }
   else
	   clrStatus();
}


//----------------------------------------------------------------------------

void FileRWStream::close()
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

Bool FileRWStream::open(const char* in_filename, bool readOnly)
{
   if(!readOnly && assertIsv)
   {
      AssertISV(ResourceManager::sm_pManager == NULL ||
               ResourceManager::sm_pManager->isValidWriteFileName(in_filename) == true,
               avar("Attempted write to file: %s.\n"
                     "File is not in a writable directory.", in_filename));
   }
	close();

	// Try to open the file for binary writing.
	// if the file exists truncate it.
   // if the file does not exist, then instruct open
	// to create a file with RW attributes

   if(!readOnly)
      hFile = CreateFile(in_filename, GENERIC_WRITE|GENERIC_READ, 0, NULL,
                      OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
   else
      hFile = CreateFile( in_filename, GENERIC_READ, FILE_SHARE_READ, NULL, 
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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

Int32 FileRWStream::getPosition() const
{
   return SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
}


//----------------------------------------------------------------------------

Bool FileRWStream::setPosition(Int32 in_position)
{
   if (strm_status == STRM_EOS)
      strm_status = STRM_OK;

   if (strm_status != STRM_OK)
      return FALSE;

   if (SetFilePointer(hFile, in_position, 0, FILE_BEGIN) != (DWORD)-1)
      return TRUE;

   return setStatus();  // Failed to set file pointer
}


//----------------------------------------------------------------------------

UInt32 FileRWStream::getSize() const
{
	if (strm_status == STRM_OK || strm_status == STRM_EOS)
	{
		DWORD high;
      return GetFileSize(hFile, &high);
	}
	return( 0 );
}


//----------------------------------------------------------------------------

Bool FileRWStream::flush()
{
	return FlushFileBuffers(hFile) != 0;
}


//----------------------------------------------------------------------------

Bool FileRWStream::setStatus()
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

Bool FileRWStream::write(int size,const void* d)
{
   if (strm_status != STRM_OK)
      return FALSE;

	if (size==0)
      return TRUE;
	
	if (WriteFile(hFile, (void *)d, size, &lastBytes, NULL))
      return TRUE;

	return setStatus();  // Failed
}


//----------------------------------------------------------------------------

Bool FileRWStream::read(int size,void* d)
{
   if (strm_status != STRM_OK)
      return FALSE;

	if (size)
   {
	   if (ReadFile(hFile, d, size, &lastBytes, NULL))
	   {
         if ((Int32)lastBytes != size)
         {
            strm_status = STRM_EOS;
            return FALSE;
         }
      }
      else
      {
	      return setStatus();
      }
   }
   return TRUE;
}

