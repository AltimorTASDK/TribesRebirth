//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include "SimDLLObject.h"
#include "console.h"

//----------------------------------------------------------------------------

typedef SimObject * (*CreateObjFunc)( int argc, const char *argv[] );
typedef void (*InitDLLFunc)();
typedef void (*CloseDLLFunc)();


//----------------------------------------------------------------------------
// Static list shared by all DLL objects

SimDLLObject::DLLList SimDLLObject::list;
CRITICAL_SECTION SimDLLObject::DLLList::cs;


//----------------------------------------------------------------------------

SimDLLObject::DLLList::DLLList()
{
   InitializeCriticalSection(&cs);
}

SimDLLObject::DLLList::~DLLList()
{
   DeleteCriticalSection(&cs);
}

HMODULE SimDLLObject::DLLList::open(const char* file)
{
	EnterCriticalSection(&cs);

	for (iterator itr = begin(); itr != end(); itr++) {
		if (!stricmp(file,(*itr).fileName)) {
			(*itr).refCount++;
			return (*itr).hDLL;
		}
	}

	// Creat new entry and load DLL.
	increment();
	Entry& entry = last();
	AssertFatal(strlen(file) + 1 < MaxPathLength,
		"SimDLLObject::DLLList::open: DLL file name too long");
	strcpy(entry.fileName,file);
   entry.hDLL = LoadLibrary(file);
	entry.refCount = 1;

	// Init the DLL.
	InitDLLFunc open_fptr = (InitDLLFunc)GetProcAddress(entry.hDLL,"_open");
	if ( open_fptr )
		open_fptr();

	LeaveCriticalSection(&cs);
	return entry.hDLL;
}

void SimDLLObject::DLLList::close(HMODULE handle)
{
	EnterCriticalSection(&cs);
	for (iterator itr = begin(); itr != end(); itr++) {
		if ((*itr).hDLL == handle) {
			if (!--(*itr).refCount) {
				CloseDLLFunc close_fptr = (InitDLLFunc)GetProcAddress((*itr).hDLL,"_close");
				if ( close_fptr )
					close_fptr();

				FreeLibrary((*itr).hDLL);
				erase(itr);
			}
			break;
		}
	}
	LeaveCriticalSection(&cs);
}


//----------------------------------------------------------------------------

SimDLLObject::SimDLLObject( ) : SimSet( TRUE )
{
}

bool SimDLLObject::processArguments(int argc, const char **argv)
{
   CMDConsole *console = CMDConsole::getLocked();
   if(argc != 1)
   {
      console->printf("SimDLLObject: dllFileName");
      return false;
   }
   hDLL = list.open(argv[0]);
   return true;
}

SimDLLObject::~SimDLLObject()
{
	list.close(hDLL);
}

IMPLEMENT_PERSISTENT(SimDLLObject);