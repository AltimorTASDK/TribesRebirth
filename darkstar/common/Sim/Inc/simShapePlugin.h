//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMSHAPEPLUGIN_H_
#define _SIMSHAPEPLUGIN_H_

//Includes

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class SimShapePlugin: public SimConsolePlugin
{
	enum CallbackID {
      ToggleBoundingBox
	};

public:
	void init();
	void endFrame();
	const char * consoleCallback(CMDConsole*, int id, int argc, const char *argv[]);
};


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_SIMSHAPEPLUGIN_H_
