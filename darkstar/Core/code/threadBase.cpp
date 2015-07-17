//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <base.h>
#include <ThreadBase.h>


ThreadBase::ThreadBase()
{
   hThread  = 0;
   threadId = 0;
   priority = THREAD_PRIORITY_NORMAL;
   hStop   = CreateEvent(NULL, false, false, NULL);
}


ThreadBase::~ThreadBase()
{
   stopThread();   
   if (hStop) CloseHandle( hStop );
}


void ThreadBase::startThread()
{
   if (!hThread)
   {
      ResetEvent( hStop );                      //force to non-signaled
      hThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadRoutine,this, 
                     0,(LPDWORD)&threadId);
      if (hThread)
         SetThreadPriority( hThread, priority );
      AssertWarn(hThread, "ThreadBase::startThread: Thread craetion failed");
   }
}


void ThreadBase::stopThread()
{
   if (hThread)   
   {
      SetEvent( hStop );                        //signal hStop so threadProcess can return.
      WaitForSingleObject(hThread,INFINITE);    //wait for threadProcess to sucessfully exit
      CloseHandle(hThread);                     //close the thread
      hThread = 0;
   }
}


int CALLBACK ThreadBase::ThreadRoutine( ThreadBase *pTB ) 
{
   return (pTB->threadProcess());
}


int ThreadBase::threadProcess()
{
   while (1)
   {
      //default behavior: sleep until it's time to stop the thread
      switch (WaitForSingleObject( getStopEvent(), INFINITE ))
      {
         case WAIT_OBJECT_0:  return (1);
         case WAIT_TIMEOUT:   
         default:             return (0);
      }
   }
}
