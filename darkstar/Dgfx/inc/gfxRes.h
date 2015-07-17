//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _RESCORE_H_
#define _RESCORE_H_

#include <streams.h>

class GFXBitmap;
class GFXBitmapArray;
class GFXPalette;
class GFXFont;

//-------------------------------------- GFXBitmap
class ResourceTypeGFXBitmap: public ResourceType
{
  public:   
   ResourceTypeGFXBitmap(const char *ext = ".dib"):
   ResourceType( ResourceType::typeof(ext) )  { }
   void* construct(StreamIO *stream, int);
   void destruct(void *p);
};   

//-------------------------------------- GFXBitmapArray
class ResourceTypeGFXBitmapArray: public ResourceType
{
  public:   
   ResourceTypeGFXBitmapArray(const char *ext = ".pba"):
   ResourceType( ResourceType::typeof(ext) )  { }
   void* construct(StreamIO *stream,int);
   void destruct(void *p);
};   

//-------------------------------------- GFXPalette
class ResourceTypeGFXPalette: public ResourceType
{
  public:   
   ResourceTypeGFXPalette(const char *ext = ".pal"):
   ResourceType( ResourceType::typeof(ext) )  { }
   void* construct(StreamIO *stream,int);
   void destruct(void *p);
};   

//-------------------------------------- GFXFont
class ResourceTypeGFXFont: public ResourceType
{
  public:   
   ResourceTypeGFXFont(const char *ext = ".pft"):
   ResourceType( ResourceType::typeof(ext) )  { }
   void* construct(StreamIO *stream,int);
   void destruct(void *p);
};   


#endif //_RESCORE_H_
