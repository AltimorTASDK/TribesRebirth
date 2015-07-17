//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include "pnState.h"


PNGlobalState* PNGlobalState::sm_pnGlobalState = NULL;

PNGlobalState::PNGlobalState()
{
   m_verbosityLevel    = 1;      // Verbosity level defaults to just above silent
   m_pResponseFileName = NULL;
}

PNGlobalState::~PNGlobalState()
{
   delete [] m_pResponseFileName;
   m_pResponseFileName = NULL;
}

void
PNGlobalState::initGlobalState()
{
   AssertFatal(sm_pnGlobalState == NULL, "initGlobalState(): "
                                         "Already initialized");

   sm_pnGlobalState = new PNGlobalState;
}

void
PNGlobalState::shutdownGlobalState()
{
   AssertFatal(sm_pnGlobalState != NULL, "shutdownGlobalState(): "
                                         "Not yet initialized");

   delete sm_pnGlobalState;
   sm_pnGlobalState = NULL;
}

PNGlobalState*
PNGlobalState::getGlobalState()
{
   AssertFatal(sm_pnGlobalState != NULL, "getGlobalState(): "
                                         "Not yet initialized or already shutdown");

   return sm_pnGlobalState;
}


//------------------------------------------------------------------------------
//--------------------------------------
// Accessor functions
//--------------------------------------
//
void
PNGlobalState::setVerbosityLevel(const int in_verbosity)
{
   m_verbosityLevel = in_verbosity;
}

int
PNGlobalState::getVerbosityLevel() const
{
   return m_verbosityLevel;
}

void
PNGlobalState::setResponseFileName(const char* in_pFileName)
{
   AssertFatal(in_pFileName != NULL, "Null file name...");
   AssertFatal(strlen(in_pFileName) > 0, "0 length Response filename");

   delete [] m_pResponseFileName;
   m_pResponseFileName = new char[strlen(in_pFileName) + 1];
   strcpy(m_pResponseFileName, in_pFileName);
}

const char*
PNGlobalState::getResponseFileName() const
{
   AssertFatal(m_pResponseFileName != NULL, "getRepsponseFileName(): "
                                            "Not yet set");
   return m_pResponseFileName;
}

