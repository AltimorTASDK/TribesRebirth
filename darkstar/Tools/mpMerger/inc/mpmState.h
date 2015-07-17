//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _MPMSTATE_H_
#define _MPMSTATE_H_

//Includes
#include <base.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class MPMGlobalState {
   static MPMGlobalState* sm_mpmGlobalState;

  private:
   int   m_verbosityLevel;
   char* m_pResponseFileName;

  private:
   MPMGlobalState();
   ~MPMGlobalState();
   
   // only main should call these...
  public:
   static void initGlobalState();
   static void shutdownGlobalState();

   void setVerbosityLevel(const int in_verbosity);
   void setResponseFileName(const char* in_pFileName);

   // Anyone can access this to get to the global options object...
  public:
   static MPMGlobalState* getGlobalState();
   
   // And call these to get the current state...
  public:
   int         getVerbosityLevel() const;
   const char* getResponseFileName() const;
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_MPMSTATE_H_
