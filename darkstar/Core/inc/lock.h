#ifndef _H_LOCK
#define _H_LOCK

#include <winbase.h>

class Lock
{
private:
   CRITICAL_SECTION sect;
public:
   Lock();
   ~Lock();

   void acquire();
   void release();
};

inline Lock::Lock()
{
   InitializeCriticalSection(&sect);
}

inline Lock::~Lock()
{
   DeleteCriticalSection(&sect);
}

inline void Lock::acquire()
{
   EnterCriticalSection(&sect);
}

inline void Lock::release()
{
   LeaveCriticalSection(&sect);
}

#endif