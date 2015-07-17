//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _MPMFILELIST_H_
#define _MPMFILELIST_H_

//Includes
#include <base.h>
#include <tVector.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class GFXPalette;

struct MPMFileEntry {
   char*  pFileName;
   UInt32 fileWeight;

   GFXPalette* pRawPalette;
   UInt32 __bogus;
};

class MPMFileList {
  private:
   Vector<MPMFileEntry> m_fileEntries;

  public:
   MPMFileList();
   ~MPMFileList();
   
   void processFileString(const char* in_pFileLine);
   
   int getNumFiles() const { return m_fileEntries.size(); }
   MPMFileEntry& getFile(const int in_index);
};

inline MPMFileEntry&
MPMFileList::getFile(const int in_index)
{
   AssertFatal(in_index >= 0 && in_index < m_fileEntries.size(), "Out of bounds index");
   
   return m_fileEntries[in_index];
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_MPMFILELIST_H_
