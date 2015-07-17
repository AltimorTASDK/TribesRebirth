//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <palMap.h>


IMPLEMENT_PERSISTENT_TAG(PaletteMap, FOURCC('P','M','A','P'));


PaletteMap::PaletteMap()
{
   for (int i = 0; i<256; i++)
   {
      haze.inclusion[i].setSize(256);
      haze.inclusion[i].set();
      shade.inclusion[i].setSize(256);
      shade.inclusion[i].set();
   }
   shade.max   =  1.0;
   shade.levels= 32;
   haze.max    =  1.0;
   haze.levels = 16;
   hazeR       = 255;
   hazeG       = 255;
   hazeB       = 255;
   gamma       =  1.0;
   useLUV      = false;
}   

//------------------------------------------------------------------------------
int PaletteMap::version()
{
   return (PMP_VERSION);
}   


//------------------------------------------------------------------------------
Persistent::Base::Error PaletteMap::read( StreamIO &sio, int version, int user )
{
   version, user;
   for (int i = 0; i<256; i++)
   {
      sio.read( 8, haze.inclusion[i].begin() );
      sio.read( 8, shade.inclusion[i].begin() );
   }
   sio.read( &shade.max );
   sio.read( &shade.levels );
   sio.read( &haze.max );
   sio.read( &haze.levels );
   sio.read( &hazeR );
   sio.read( &hazeG );
   sio.read( &hazeB );
   sio.read( &gamma );
   sio.read( &useLUV );

   if (sio.getStatus() == STRM_OK)
      return (Ok);
   return (ReadError);
}   


//------------------------------------------------------------------------------
Persistent::Base::Error PaletteMap::write( StreamIO & sio, int version, int user )
{
   version, user;
   for (int i = 0; i<256; i++)
   {
      sio.write( 8, haze.inclusion[i].begin() );
      sio.write( 8, shade.inclusion[i].begin() );
   }
   sio.write( shade.max );
   sio.write( shade.levels );
   sio.write( haze.max );
   sio.write( haze.levels );
   sio.write( hazeR );
   sio.write( hazeG );
   sio.write( hazeB );
   sio.write( gamma );
   sio.write( useLUV );

   if (sio.getStatus() == STRM_OK)
      return (Ok);
   return (WriteError);
}   
