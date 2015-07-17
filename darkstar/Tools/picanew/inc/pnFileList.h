//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _PNFILELIST_H_
#define _PNFILELIST_H_

//Includes
#include <base.h>
#include <tVector.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class PNUnquantizedImage;

struct PNFileEntry {
   char*  pFileName;
   UInt32 fileWeight;

   PNUnquantizedImage* pUQImage;
   UInt32 __bogus;
};

class PNFileList {
  private:
   Vector<PNFileEntry> m_fileEntries;

  public:
   PNFileList();
   ~PNFileList();
   
   void processFileString(const char* in_pFileLine);
   
   int getNumFiles() const { return m_fileEntries.size(); }
   PNFileEntry& getFile(const int in_index);
};

inline PNFileEntry&
PNFileList::getFile(const int in_index)
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

#endif //_PNFILELIST_H_
