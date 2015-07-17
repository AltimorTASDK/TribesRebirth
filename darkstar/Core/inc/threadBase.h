//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _THREADBASE_H_
#define _THREADBASE_H_

#include <types.h>

class DLLAPI ThreadBase
{
private:
   HANDLE hThread;         // Thread associated with the object
   HANDLE hStop;           // EventObject to signal a desire to stop the thread
   int    threadId;        // needed by the CreateThread() function
   int    priority;

   static int CALLBACK ThreadRoutine( ThreadBase *pTB );
     
public:
   ThreadBase();
   virtual ~ThreadBase();

   //override this function: follow the example loop or risk deadlock!!!
   virtual int threadProcess();     
   virtual void startThread();
   virtual void stopThread();

   HANDLE getHandle();
   int  getPriority();
   void setPriority(int priority);
   HANDLE getStopEvent();

};   


inline HANDLE ThreadBase::getHandle()
{
   return ( hThread );
}


inline HANDLE ThreadBase::getStopEvent()
{
   return hStop;
}


inline void ThreadBase::setPriority(int p)
{
   priority = p;
   if (hThread) SetThreadPriority(hThread, priority);
}

inline int ThreadBase::getPriority()
{
   return (priority);
}


#endif //_THREADBASE_H_
