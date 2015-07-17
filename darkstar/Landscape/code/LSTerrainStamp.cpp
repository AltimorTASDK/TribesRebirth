//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "g_bitmap.h"
#include "LSTerrainStamp.h"

IMPLEMENT_PERSISTENT_TAG(LSTerrainStamp,FOURCC('L','S','T','S'));

Persistent::Base::Error LSTerrainStamp::write(StreamIO &sio, int, int)
{
   Vector<LSTerrainStamp::TextureType>::iterator tex_iter;
   Vector<Point2I>::iterator flat_iter;
 
   // write the list of TextureTypes, lenght first
   if (!sio.write(texList.size()))    return Persistent::Base::WriteError;
   for (tex_iter = texList.begin(); tex_iter != texList.end(); tex_iter++)
   {
      if (!tex_iter->tile.write(sio)) return Persistent::Base::WriteError;   
      if (!sio.write(tex_iter->flags)) return Persistent::Base::WriteError;
      if (!sio.write(tex_iter->textureID)) return Persistent::Base::WriteError;
   }   

   // write the list of vertices to flatten, length first
   if (!sio.write(flatList.size()))     return Persistent::Base::WriteError;
   for (flat_iter = flatList.begin(); flat_iter != flatList.end(); flat_iter++)
   {
      if (!flat_iter->write(sio))        return Persistent::Base::WriteError;   
   }   

   // write the clamping values
   if (!sio.write(clamp_max_detail))  return Persistent::Base::WriteError;

   return Persistent::Base::Ok;   
}   

//--------------------------------------------------------------------------- 

Persistent::Base::Error LSTerrainStamp::read(StreamIO &sio, int, int)
{
   TextureType texType;
   Point2I pt; 
   int lenght, i;

   texList.clear();
   flatList.clear();

   // Read the list of TextureTypes, getting length first
   if (!sio.read(&lenght))            return Persistent::Base::ReadError;
   for (i = 0; i < lenght; i++)
   {
      if (!texType.tile.read(sio))    return Persistent::Base::ReadError;     
      if (!sio.read(&texType.flags))   return Persistent::Base::ReadError;
      if (!sio.read(&texType.textureID))   return Persistent::Base::ReadError;
      texList.push_back(texType);
   }

   // Read the list of TextureTypes, getting length first
   if (!sio.read(&lenght))            return Persistent::Base::ReadError;
   for (i = 0; i < lenght; i++)
   {
      if (!pt.read(sio))                 return Persistent::Base::ReadError;     
      flatList.push_back(pt);
   }

   // Read the clamping value
   if (!sio.read(&clamp_max_detail))  return Persistent::Base::ReadError;

   return Persistent::Base::Ok;   
} 
