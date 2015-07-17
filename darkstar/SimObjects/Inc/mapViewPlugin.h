//----------------------------------------------------------------------------
//
// mapViewPlugin.h
//
//----------------------------------------------------------------------------

#ifndef _MAPVIEWPLUGIN_H_
#define _MAPVIEWPLUGIN_H_

class MapViewPlugin : public SimConsolePlugin
{
	enum CallbackID {
      LoadMap,
      DeleteMap,
      LoadMapPalette,
	};

   GFXPalette *palette;

public:
   MapViewPlugin() { palette = NULL; }
	void init();
	void startFrame();
	void endFrame();
	int  consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
};

#endif
