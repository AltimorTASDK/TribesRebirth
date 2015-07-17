#ifndef _TEDPLUGIN_H_
#define _TEDPLUGIN_H_

#include <sim.h>
#include <TString.h>
#include "simTed.h"
#include "toolPlugin.h"
#include "tedwindow.h"

class TedPlugin : public SimConsolePlugin, SimTedMouseCallback 
{
   typedef SimConsolePlugin Parent;
   
   public:
   
      // all the console commands
   	const char * FinitTed( int argc, const char *argv[] );
   	const char * FquitTed( int argc, const char *argv[] );
      const char * FattachToTerrain( int argc, const char *argv[] );
      const char * FsetSnap( int argc, const char *argv[] );
      const char * FsetFeather( int argc, const char *argv[] );
      const char * FsetSelectFrameColor( int argc, const char *argv[] );
      const char * FsetSelectFillColor( int argc, const char *argv[] );
      const char * FsetHilightFrameColor( int argc, const char *argv[] );
      const char * FsetHilightFillColor( int argc, const char *argv[] );
      const char * FsetShadowFrameColor( int argc, const char *argv[] );
      const char * FsetShadowFillColor( int argc, const char *argv[] );
      const char * FsetSelectShow( int argc, const char *argv[] );
      const char * FsetHilightShow( int argc, const char *argv[] );
      const char * FsetShadowShow( int argc, const char *argv[] );
      const char * FsetBrushDetail( int argc, const char *argv[] );

      const char * FgetBrushDetail( int argc, const char *argv[] );
      const char * FgetMaxBrushDetail( int argc, const char *argv[] );
      const char * FgetNumTerrainTypes( int argc, const char *argv[] );
      const char * FgetTerrainTypeName( int argc, const char *argv[] );

      const char * FsetBrushPos( int argc, const char *argv[] );
      const char * FsetLButtonAction( int argc, const char *argv[] );
      const char * FsetRButtonAction( int argc, const char *argv[] );
      const char * Frelight( int argc, const char *argv[] );
      const char * FclearSelect( int argc, const char *argv[] );
      const char * Fundo( int argc, const char *argv[] );
      const char * Fredo( int argc, const char *argv[] );
      const char * FsetHeightVal( int argc, const char *argv[] );
      const char * FsetAdjustVal( int argc, const char *argv[] );
      const char * FsetFlags( int argc, const char *argv[] );
      const char * FtoggleFlags( int argc, const char *argv[] );
      const char * FclearFlags( int argc, const char *argv[] );
      const char * FsetFlagVal( int argc, const char *argv[] );
      const char * FprocessAction( int argc, const char *argv[] );
      const char * Frotate( int argc, const char *argv[] );
      const char * FlistCommands( int argc, const char *argv[] );
      const char * FsetUndoLimit( int argc, const char *argv[] );
      const char * FaddNamedSelection( int argc, const char *argv[] );
      const char * FremoveNamedSelection( int argc, const char *argv[] );
      const char * FselectNamedSelection( int argc, const char *argv[] );
      const char * FselectMaterial( int argc, const char *argv[] );
      const char * Fcopy( int argc, const char *argv[] );
      const char * Fpaste( int argc, const char *argv[] );
      const char * FloadPalette( int argc, const char * argv[] );
      const char * FloadMatList( int argc, const char * argv[] );
      const char * Fwindow( int argc, const char * argv[] );
      const char * FassignMatList( int argc, const char * argv[] );
      const char * Fopen( int argc, const char * argv[] );
      const char * Fclose(int argc, const char * argv[] );
      const char * Fnew( int argc, const char * argv[] );
      const char * Ffocus( int argc, const char * argv[] );
      const char * Funfocus( int argc, const char * argv[] );
      const char * Fsave( int argc, const char * argv[] );
      const char * FforceTerrainType( int argc, const char * argv[] );
      const char * FsetTerrainType( int argc, const char * argv[] );
      const char * FsetMatIndexVal( int argc, const char * argv[] );
      const char * FgetMaterialCount(int argc, const char * argv[]);
      const char * FgetMaterialName(int argc, const char * argv[]);
      const char * FgetMaterialIndex(int argc, const char * argv[]);
      const char * FlistNamedSelections( int argc, const char * argv[] );
      const char * FupdateToolBar( int argc, const char * argv[] );
      const char * FsetScaleVal( int argc, const char * argv[] );
      const char * FsetPinDetailVal( int argc, const char * argv[] );
      const char * FsetPinDetailMax( int argc, const char * argv[] );
      const char * FsetSmoothVal( int argc, const char * argv[] );
      const char * FsetPasteVal( int argc, const char * argv[] );
      const char * FsetStatusText( int argc, const char * argv[] );
      const char * FloadSelection( int argc, const char * argv[] );
      const char * FsaveSelection( int argc, const char * argv[] );
      const char * FsetBlockOutline( int argc, const char * argv[] );
      const char * FsetBlockFrameColor( int argc, const char * argv[] );
      const char * FclearPinMaps( int argc, const char * argv[] );
      const char * FGetConsoleOptions( int argc, const char * argv[] );
      const char * FgetTerrainType( int argc, const char * argv[] );
      const char * FgetNumActions( int argc, const char * argv[] );
      const char * FgetActionName( int argc, const char * argv[] );
      const char * FisActionMouseable( int argc, const char * argv[] );
      const char * FgetActionIndex( int argc, const char * argv[] );
      const char * FfloatSelection( int argc, const char * argv[] );
      const char * FfloatPaste( int argc, const char * argv[] );
      const char * FmirrorGridBlock(int argc, const char * argv[]);
      const char * FterrainAction(int argc, const char * argv[]);
      const char * FgetLButtonActionIndex(int argc, const char * argv[]);
      const char * FgetRButtonActionIndex(int argc, const char * argv[]);
      const char * FgetWorldName(int argc, const char * argv[]);
      
      struct ConsoleCommand 
      {
         String commandString;
         String errorString;
         String usageString;
         int numArgs;
         const char * ( TedPlugin::*callback )( int argc, const char * argv[] );
      };
      
      TedPlugin();
      ~TedPlugin();
      void init();
      const char *consoleCallback( CMDConsole*, int id, int argc, const char *argv[] );
      bool mouseCallback( bool up, int button );

   private:
      
      Vector< ConsoleCommand * > commands;
      
      TedWindow * window;
      bool mouseDown[ 2 ];
      SimTed * simTed;
      bool boolify( const char * val );
      bool listBoxShow( ToolPlugin::ListBox& lbox );
      bool setFlagsFlag( const char * str, UInt8 & flags );
      void setSuccess( bool success );
      bool getSuccess();
      void onDeleteNotify(SimObject * object);
      
      void addCommand( const char * command, const char * error, const char * usage, 
         int numArgs, const char * ( TedPlugin::*callback )( int argc, const char * argv[] ) );
};

#endif