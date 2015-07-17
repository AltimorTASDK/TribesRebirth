//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _MPMPSEUDOTRACE_H_
#define _MPMPSEUDOTRACE_H_

//Includes
#include <base.h>
#include <tVector.h>
#include <iostream.h>
#include "mpmState.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class PseudoTrace {
   static PseudoTrace sg_pseudoTracer;

   typedef Vector<int> VerbosityStack;

  private:
   PseudoTrace()  { m_isInErrorState = false; }
   ~PseudoTrace() { }

  public:
   VerbosityStack m_verbosityLevels;
   bool           m_isInErrorState;
   
  public:
   static PseudoTrace& getTracerObject();
   
   void pushVerbosityLevel(const int in_level);
   void popVerbosityLevel();

   void flush() { ::flush(cout); }

   void setErrorState();   // Note: is it assumed that the program will exit immediately
                           //  after setting the error state, and tracing the error
                           //  message, so we provide no mechanism for returning to
                           //  normal operation.
};

template<class T> PseudoTrace&
operator<< (PseudoTrace& in_trace, T in_arg)
{
   if (in_trace.m_isInErrorState == true) {
      cerr << in_arg;
   } else {
      MPMGlobalState* pGlobalState = MPMGlobalState::getGlobalState();
      AssertFatal(in_trace.m_verbosityLevels.size() > 0, "Error, no verbosity level pushed");
      
      if (pGlobalState->getVerbosityLevel() >= in_trace.m_verbosityLevels.last())
         cout << in_arg;
   }
      
   return in_trace;
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_MPMPSEUDOTRACE_H_
