#include "filename.h"

//----------------------------------------------------------------
// c:\test\apple.abc - 'abc'
String FileName::getExt()
{
   const char * str = rep.c_str();
   if( !str )
      return( String( "" ) );
   
   AssertFatal( ( strlen( str ) < FILENAME_MAX_LENGTH ), "Filename too long." );
   
   char * end = const_cast< char * >( str + strlen( str ) );
   while( ( end >= str ) && ( *end != '.' ) )
      end--;
   
   if( end < str )
      return( String( "" ) );
      
   return( String( end + 1 ) );
}

//----------------------------------------------------------------
// c:\test\apple.abc - 'apple'
String FileName::getBase()
{
   const char * str = rep.c_str();
   if( !str )
      return( String( "" ) );
   
   AssertFatal( ( strlen( str ) < FILENAME_MAX_LENGTH ), "Filename too long." );

   char * pos = const_cast< char * >( str + strlen( str ) );
   while( ( pos >= str ) && ( *pos != '\\' ) && ( *pos != ':' ) && ( *pos != '/' ) )
      pos--;
      
   char buf[ FILENAME_MAX_LENGTH ];
   sprintf( buf, pos + 1 );
   
   char * end = buf + strlen( buf );
   while( ( end >= buf ) && ( *end != '.' ) )
      end--;
      
   if( end < buf )
      return( String( buf ) );

   *end = '\0';
   
   return( String( buf ) );
}

//----------------------------------------------------------------
// c:\test\apple.abc - 'apple.abc'
String FileName::getTitle()
{
   const char * str = rep.c_str();
   if( !str )
      return( String( "" ) );

   AssertFatal( ( strlen( str ) < FILENAME_MAX_LENGTH ), "Filename too long." );

   char * pos = const_cast< char * >( str + strlen( str ) );
   while( ( pos >= str ) && ( *pos != '\\' ) && ( *pos != ':' ) && ( *pos != '/' ) )
      pos--;
      
   if( pos < str )
      return( String( str ) );
      
   return( String( pos + 1 ) );
}

//----------------------------------------------------------------
// c:\test\apple.abc - 'c:\test\'
String FileName::getPath()
{
   const char * str = rep.c_str();
   if( !str )
      return( String( "" ) );
      
   AssertFatal( ( strlen( str ) < FILENAME_MAX_LENGTH ), "Filename too long." );

   char buf[ FILENAME_MAX_LENGTH ];
   sprintf( buf, str );
   
   char * pos = buf + strlen( buf );
   while( ( pos >= buf ) && ( *pos != '\\' ) && ( *pos != ':' ) && ( *pos != '/' ) )
      pos--;
   
   if( pos < buf )
      return( String( "" ) );
   
   *( pos+1 ) = '\0';
   
   return( String( buf ) );
}

//----------------------------------------------------------------

void FileName::setExt( const char * ext )
{
   String path = getPath();
   String base = getBase();
   
   char buf[ FILENAME_MAX_LENGTH ];
   if( ext )
   {
      AssertFatal( ( ( strlen( path.c_str() ) + strlen( base.c_str() ) + strlen( ext ) ) < FILENAME_MAX_LENGTH ), "Filename too long." );
      sprintf( buf, "%s%s.%s", path.c_str(), base.c_str(), ext );
   }
   else
      sprintf( buf, "%s%s", path.c_str(), base.c_str() );
   rep = buf;
}

//----------------------------------------------------------------

void FileName::setBase( const char * base )
{
   String path = getPath();
   String ext = getExt();
   
   char buf[ FILENAME_MAX_LENGTH ];
   if( base )
   {
      AssertFatal( ( ( strlen( path.c_str() ) + strlen( base ) + strlen( ext.c_str() ) ) < FILENAME_MAX_LENGTH ), "Filename too long." );
      sprintf( buf, "%s%s.%s", path.c_str(), base, ext.c_str() );
   }
   else
      sprintf( buf, "%s.%s", path.c_str(), ext.c_str() );
   rep = buf;
}

//----------------------------------------------------------------

void FileName::setTitle( const char * title )
{
   String path = getPath();
   
   char buf[ FILENAME_MAX_LENGTH ];
   if( title )
   {
      AssertFatal( ( ( strlen( path.c_str() ) + strlen( title ) ) < FILENAME_MAX_LENGTH ), "Filename too long." );
      sprintf( buf, "%s%s", path.c_str(), title );
   }
   else
      sprintf( buf, "%s", path.c_str() );
   rep = buf;
}

//----------------------------------------------------------------

void FileName::setPath( const char * path )
{
   String title = getTitle();
   
   char buf[ FILENAME_MAX_LENGTH ];
   if( path )
   {
      AssertFatal( ( ( strlen( title.c_str() ) + strlen( path ) ) < FILENAME_MAX_LENGTH ), "Filename too long." );

      // check if a backslash should be added
      if( path[strlen(path)-1] != '\\' )
         sprintf( buf, "%s\\%s", path, title.c_str() );
      else
         sprintf( buf, "%s%s", path, title.c_str() );
   }
   else
      sprintf( buf, "%s", title.c_str() );
   rep = buf;
}

//----------------------------------------------------------------
