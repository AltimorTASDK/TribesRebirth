//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _TS_RES_H_
#define _TS_RES_H_

#include <streams.h>
#include "ts_shape.h"
#include "ts_material.h"

namespace TS
{
//-------------------------------------- TS::Shape
class ResourceTypeTSShape : public ResourceType
{
public:   
   ResourceTypeTSShape(const char *ext = ".dts"):
      ResourceType( ResourceType::typeof(ext) )  { }
   void* construct(StreamIO *stream,int) 
      { 
      Persistent::Base::Error err; 
      return (void*)Persistent::Base::load(*stream,&err); 
      }
   void destruct(void *p)            { delete (Shape*)p; }
};   

//-------------------------------------- TS::MaterialList
class ResourceTypeTSMaterialList : public ResourceType
{
public:   
   ResourceTypeTSMaterialList(const char *ext = ".dml"):
      ResourceType( ResourceType::typeof(ext) )  { }
   void* construct(StreamIO *stream,int)
      { 
      Persistent::Base::Error err; 
      return Persistent::Base::load(*stream,&err); 
      }
   void destruct(void *p)            { delete (MaterialList*)p; }
};   

   //---------------------------------------------------------------------------
}; // namespace TS
#endif