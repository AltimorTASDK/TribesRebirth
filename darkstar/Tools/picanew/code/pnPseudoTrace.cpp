//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <iostream.h>

#include "pnState.h"
#include "pnPseudoTrace.h"

PseudoTrace PseudoTrace::sg_pseudoTracer;

PseudoTrace&
PseudoTrace::getTracerObject()
{
   return sg_pseudoTracer;
}

void
PseudoTrace::pushVerbosityLevel(const int in_level)
{
   m_verbosityLevels.push_back(in_level);
}

void
PseudoTrace::popVerbosityLevel()
{
   AssertFatal(m_verbosityLevels.size() > 0, "Unbalanced verbosity pop");

   m_verbosityLevels.pop_back();
}

void
PseudoTrace::setErrorState()
{
   m_isInErrorState = true;
}

