//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#ifndef _TERRAINPLUGIN_H_
#define _TERRAINPLUGIN_H_

// Must include after sim stuff

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class TerrainPlugin: public SimConsolePlugin
{
	enum CallbackID {
      NewTerrain,
      NewTerrainParam,
      LoadTerrain,
      SaveTerrain,
      SetTerrainDetail,
		SetTerrainVisibility,
		SetTerrainContainer,
		LightTerrain,
      ShowTerrain,
	};

public:
	void init();
	void endFrame();
	const char *consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
};

#endif
