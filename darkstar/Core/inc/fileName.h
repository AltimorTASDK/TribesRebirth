#ifndef _INC_FILENAME
#define _INC_FILENAME

#include "tstring.h"

//----------------------------------------------------------------
// member get functions return at least an empty string - '\0'
//  -- would liked to have derived from String, but would have requried
//     making ~String a virtual destructor
//  -- implements several of the string functions

#define FILENAME_MAX_LENGTH   1024

class FileName
{
   private:
      String rep;
      
   public:
      FileName(){};
      FileName(const char * fn) : rep( fn ){};
      FileName(const String & fn) : rep( fn ){};

   	void operator=( const char * p ){ rep = p; };
   	bool operator==( const char * p ) const { return( rep == p ); };
   	bool operator!=(const char * p) const { return( rep != p ); };

   	void operator=( const String & p ){ rep = p; };
   	bool operator==( const String & p ) const { return( rep == p ); };
   	bool operator!=( const String & p ) const { return( rep != p ); };
      
      operator String(){ return( rep ); };
      const char * c_str() const { return( rep.c_str() ); };
       
      // examplle: c:\test\this.abc
      String getExt();     // 'abc'
      String getBase();    // 'this'
      String getTitle();   // 'this.abc'
      String getPath();    // 'c:\test\'
      
      void setExt( const char * ext = NULL );
      void setBase( const char * base = NULL );
      void setTitle( const char * title = NULL );
      void setPath( const char * path = NULL );
};

#endif
