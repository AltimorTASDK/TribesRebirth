//--------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//--------------------------------------------------------------------------------

#ifndef _GWMAIN_H_
#define _GWMAIN_H_

//----------------------------------------------------------------------------


#include <core.h>
#include <tVector.h>

class GWMain
{
public:
	static GWMain* pMain;
	static HINSTANCE hInstance;
	static HINSTANCE hPrevInstance;
	static int cmdShow;
	static LPSTR cmdLine;
	static Vector<const char*> processedCmdLine;
	static int exitCode;
	static MSG currentMsg;
	static DWORD processId;
   static HACCEL hAccel;
   static bool foreground;

	// If true (the default) the app sleeps when it
	// does not have the focus.
	bool checkFocus;
	int  nonfocusTimeout;      // how often (ms) the the process will wake up
                              // and do onIdle when it does NOT have focus

	// Methods
	GWMain();
	virtual ~GWMain();
	virtual bool initApplication();
	virtual bool initInstance();

	virtual bool processMessage();
	virtual int  messageLoop();
	virtual void onIdle();
	virtual void onExit();
};




#endif //_GWMAIN_H_
