//================================================================
//   
// $Workfile:   streamio.cpp  $
// $Version$
// $Revision:   1.2  $
//   
// DESCRIPTION:
//   
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================



//---------------------------------------------------------------------------


#include "streamio.h"
#include "tString.h"
#include "stringTable.h"

//---------------------------------------------------------------------------

StreamIO::StreamIO()
{
   lastBytes   = 0;
   strm_status = STRM_OK;
}

StreamIO::~StreamIO()
{
   flush();
}


StreamStatus 
StreamIO::getStatus() const
{
   return strm_status;
}

UInt32 
StreamIO::getLastSize() const
{
   return (lastBytes);
}   


//---------------------------------------------------------------------------

Int32 StreamIO::getCapabilities(StreamCap cap) const
{
	switch(cap)
	{
		case STRM_CAP_TYPE:
			return STRM_BASE;
	}
	return 0;
}

void StreamIO::close()
{
}

Int32 StreamIO::getPosition() const
{
	return 0;
}

Bool StreamIO::setPosition(Int32)
{
	return 1;
}

Bool StreamIO::flush()
{
	return 1;
}

Bool StreamIO::write(int, const void*)
{
	return 1;
}

Bool StreamIO::read(int, void*)
{
	return 1;
}

void* StreamIO::lock()
{
   return (NULL);
}   


Bool 
StreamIO::write(const String& in_rStr)
{
   UInt32 strLen;
   if (in_rStr.empty() == true) {
      // Note: string returns empty on NULL and "".  On depersist, both map
      //  to ""
      //
      if (in_rStr.c_str() == NULL) {
         return write(UInt32(0));
      } else {
         write(UInt32(1));
         return write(1, in_rStr.c_str());
      }
   } else {
      strLen = in_rStr.length() + 1;
      if (write(strLen) == false) return false;
      return write(strLen, in_rStr.c_str());
   }
}


Bool 
StreamIO::read(String* out_pStr)
{
   UInt32 strLen;
   if (read(&strLen) == false)
      return false;

   if (strLen != 0) {
      char* temp = new char[strLen];

      if (read(strLen, temp) == false) {
         delete [] temp;
         return false;
      }
      *out_pStr = temp;

      delete [] temp;
   } else {
      // Empty string
      //
      *out_pStr = NULL;
   }

   return true;
}


//---------------------------------------------------------------------------

void StreamIO::writeString(const char *string, int maxLen)
{
   int len = string ? strlen(string) : 0;
   AssertWarn(len < 256, "String too long in StreamIO::writeString");
   if(len > maxLen)
      len = maxLen;

   write(BYTE(len));
   if(len)
      write(len, string);
}

void StreamIO::readString(char buf[256])
{
   BYTE len;
   read(&len);
   read(int(len), buf);
   buf[len] = 0;
}

const char *StreamIO::readSTString(bool casesens)
{
   char buf[256];
   readString(buf);
   return stringTable.insert(buf, casesens);
}

//---------------------------------------------------------------------------

Bool StreamIO::writeCharString(const char* str)
{
	AssertFatal(str ? strlen(str) < 256 : true,
			"StreamIO::write(const char*): String longer than 255 char.");
	UInt8 size = str ? strlen(str): 0;
	if (write(size)) {
		if (size)
			return write(size,str);
		return true;
	}
	return false;
}

Bool StreamIO::readCharString(char** str)
{
	UInt8 size;
	if (read(&size)) {
		if (size) {
			*str = new char[size+1];
			if (read(size,*str)) {
				(*str)[size] = '\0';
				return true;
			}
			delete *str;
		}
		else {
			*str = 0;
			return true;
		}
	}
	*str = 0;
	return false;
}

