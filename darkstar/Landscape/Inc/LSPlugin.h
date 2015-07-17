//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#ifndef _LSPLUGIN_H_
#define _LSPLUGIN_H_

//----------------------------------------------------------------------------

class LSPlugin: public SimConsolePlugin
{
	enum CallbackID
	{
		LSCreate,            // to create a new LS object
		LSRules,             // to set a different terrain rules
		LSTextures,          // to set different textures
		LSScript,            // to set different landscape
      LSApplyTextures,     // to apply textures to terrain
      LSApplyLandScape,    // to apply LandScape to terrain
      LSEdit,              // to bring up a pls editor dialog
      flushTextures,        // flush textures and terrain types from internal lists
      addTerrainTexture,   // add a terrain texture
      addTerrainType,      // register a new terrain type
      createGridFile,      // create a terrain data file
      flushRules,          // clear the rules list
      addRule,             // add a rule
      flushCommands,       // clear the commands
      addCommand,          // add a command
      parseCommands,       // parses the commands to allow LSApplyLandscape to work
	};

public:
	void init();
	const char *consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
};

#endif   // _LSPLUGIN_H_
