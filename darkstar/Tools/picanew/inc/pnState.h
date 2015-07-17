//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _PNSTATE_H_
#define _PNSTATE_H_

//Includes
#include <base.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class PNGlobalState {
   static PNGlobalState* sm_pnGlobalState;

  private:
   int   m_verbosityLevel;
   char* m_pResponseFileName;

  private:
   PNGlobalState();
   ~PNGlobalState();
   
   // only main should call these...
  public:
   static void initGlobalState();
   static void shutdownGlobalState();

   void setVerbosityLevel(const int in_verbosity);
   void setResponseFileName(const char* in_pFileName);

   // Anyone can access this to get to the global options object...
  public:
   static PNGlobalState* getGlobalState();
   
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

#endif //_PNSTATE_H_
