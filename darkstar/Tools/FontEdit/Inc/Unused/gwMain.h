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

class GWMain
{
public:
	static GWMain* pMain;
	static HINSTANCE hInstance;
	static HINSTANCE hPrevInstance;
	static int cmdShow;
	static LPSTR cmdLine;
	static int exitCode;
	static MSG currentMsg;

	GWMain();
	virtual ~GWMain();
	virtual bool initApplication();
	virtual bool initInstance();

	virtual bool processMessage();
	virtual int messageLoop();
	virtual void onIdle();
};




#endif //_GWMAIN_H_
