//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include "mpmState.h"


MPMGlobalState* MPMGlobalState::sm_mpmGlobalState = NULL;

MPMGlobalState::MPMGlobalState()
{
   m_verbosityLevel    = 1;      // Verbosity level defaults to just above silent
   m_pResponseFileName = NULL;
}

MPMGlobalState::~MPMGlobalState()
{
   delete [] m_pResponseFileName;
   m_pResponseFileName = NULL;
}

void
MPMGlobalState::initGlobalState()
{
   AssertFatal(sm_mpmGlobalState == NULL, "initGlobalState(): "
                                          "Already initialized");

   sm_mpmGlobalState = new MPMGlobalState;
}

void
MPMGlobalState::shutdownGlobalState()
{
   AssertFatal(sm_mpmGlobalState != NULL, "shutdownGlobalState(): "
                                          "Not yet initialized");

   delete sm_mpmGlobalState;
   sm_mpmGlobalState = NULL;
}

MPMGlobalState*
MPMGlobalState::getGlobalState()
{
   AssertFatal(sm_mpmGlobalState != NULL, "getGlobalState(): "
                                          "Not yet initialized or already shutdown");

   return sm_mpmGlobalState;
}


//------------------------------------------------------------------------------
//--------------------------------------
// Accessor functions
//--------------------------------------
//
void
MPMGlobalState::setVerbosityLevel(const int in_verbosity)
{
   m_verbosityLevel = in_verbosity;
}

int
MPMGlobalState::getVerbosityLevel() const
{
   return m_verbosityLevel;
}

void
MPMGlobalState::setResponseFileName(const char* in_pFileName)
{
   AssertFatal(in_pFileName != NULL, "Null file name...");
   AssertFatal(strlen(in_pFileName) > 0, "0 length Response filename");

   delete [] m_pResponseFileName;
   m_pResponseFileName = new char[strlen(in_pFileName) + 1];
   strcpy(m_pResponseFileName, in_pFileName);
}

const char*
MPMGlobalState::getResponseFileName() const
{
   AssertFatal(m_pResponseFileName != NULL, "getRepsponseFileName(): "
                                            "Not yet set");
   return m_pResponseFileName;
}

