//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _REGISTRY_H_
#define _REGISTRY_H_

#include <tVector.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif


class Registry
{
private:
   VectorPtr<char*> keyName;
   Vector<HKEY> keyList;
   enum { MAX_KEY_PATH = 256 };
   char path[MAX_KEY_PATH];
   HKEY  getCurrentKey();
   const char* getCurrentKeyName();
   DWORD remove(HKEY hStartKey, const char *pKeyName );
         
public:
   Registry();
   ~Registry();
   bool open(const char *key, bool create=false);
   bool close();
   bool enumerateKeys(DWORD index, char *name, DWORD len);
   bool enumerateTags(DWORD index, char *name, DWORD lenName, BYTE *data, DWORD *lenData, DWORD *type=NULL);
   bool remove(const char *key);

   // string
   bool readString(const char *tag, char *str, DWORD len);
   bool writeString(const char *tag, const char *str);

   // multi-string
   bool readMultiString(const char *tag, char *str, DWORD len);
   bool writeMultiString(const char *tag, const char *str);

   // int    
   bool read(const char *tag, int *data);
   bool write(const char *tag, int data);

   // float
   bool read(const char *tag, float *data);
   bool write(const char *tag, float data);

   // raw BYTE block
   bool read(const char *tag, BYTE *data, DWORD *len);
   bool write(const char *tag, BYTE *data, DWORD len);
};


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_REGISTRY_H_
