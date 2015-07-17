#include <limits.h>
#include "simConsolePlugin.h"
#include "simAction.h"
#include "tedPlugin.h"
#include "tedwindow.h"
#include "SimGame.h"
#include "simpalette.h"
#include "simvolume.h"
#include "simCanvas.h"
#include "filename.h"

#define RESULTVAR          "$dlgResult"
#define CANCEL_STR         "[cancel]"
#define OK_STR             "[ok]"
#define YES_STR            "[yes]"
#define NO_STR             "[no]"
#define TED_ERROR          "TED::ERROR"
#define TEDPREFIX          "Ted::"

static const char *FlagOpsNames[] = {
	"rotate", "flipx", "flipy", "empty1", "empty2",
	"empty3", "corner", "edit", "all", 
	NULL };
   
TedPlugin::TedPlugin() :
   simTed( NULL ),
   window( NULL )
{
   mouseDown[0] = mouseDown[1] = false;
}

TedPlugin::~TedPlugin()
{
   // remove the tool window
   if( window )
   {
      delete window;
      window = NULL;
   }      
   
   // remove the commands
   for( Vector< ConsoleCommand * >::iterator i = commands.begin(); i != commands.end(); i++ )
      delete( *i );
}

static const char *stripPath( const char* fname )
{
	// Return the file name portion without the path
	const char* ptr = fname + strlen(fname) - 1;
	for (; ptr != fname; ptr--)
		if (*ptr == '\\' || *ptr == '/' || *ptr == ':')
			return ptr + 1;
	return fname;
}

// create a table of strings and function pointers to the different commands.  
// Not very c++-like, but really cleans up a bunch of junk
// Now to add a command: just add an entry here, and a function def/dec and your done
void TedPlugin::init()
{
   console->printf( "TedPlugin" );

   // add all the commands
   //    ex: addCommand( commandString, errorString, usageString, numArgs, &function );
   addCommand( "initTed", "could not initialize TED.", "", 0, &TedPlugin::FinitTed );
   addCommand( "quitTed", "could not quit TED.", "", 0, &TedPlugin::FquitTed );
   addCommand( "attachToTerrain", "could not attach to terrain.", "<terrain name>", 0, &TedPlugin::FattachToTerrain );
   addCommand( "setSnap", "could not set snapping mode.", "<bool>", 1, &TedPlugin::FsetSnap );
   addCommand( "setFeather", "could not set feather mode.", "<bool>", 1, &TedPlugin::FsetFeather );
   addCommand( "setSelectFrameColor", "could not set Select frame color.", "<color>", 1, &TedPlugin::FsetSelectFrameColor );
   addCommand( "setSelectFillColor", "could not set Select fill color.", "<color>", 1, &TedPlugin::FsetSelectFillColor );
   addCommand( "setHilightFrameColor", "could not set Hilight frame color.", "<color>", 1, &TedPlugin::FsetHilightFrameColor );
   addCommand( "setHilightFillColor", "could not set Hilight fill color.", "<color>", 1, &TedPlugin::FsetHilightFillColor );
   addCommand( "setShadowFrameColor", "could not set Shadow frame color.", "<color>", 1, &TedPlugin::FsetShadowFrameColor );
   addCommand( "setShadowFillColor", "could not set Shadow fill color.", "<color>", 1, &TedPlugin::FsetShadowFillColor );
   addCommand( "setSelectShow", "could not set Select Show.", "<frame fill outline>", 3, &TedPlugin::FsetSelectShow );
   addCommand( "setHilightShow", "could not set Hilight Show.", "<frame fill outline>", 3, &TedPlugin::FsetHilightShow );
   addCommand( "setShadowShow", "could not set Shadow Show.", "<frame fill outline>", 3, &TedPlugin::FsetShadowShow );
   addCommand( "setBlockOutline", "could not change block outline mode.", "< bool >", 1, &TedPlugin::FsetBlockOutline );
   addCommand( "setBlockFrameColor", "could not set block frame color.", "<color>", 1, &TedPlugin::FsetBlockFrameColor );
   addCommand( "setBrushPos", "could not set brush position.", "< x > < y >", 2, &TedPlugin::FsetBrushPos );
   addCommand( "setLButtonAction", "could not set left mouse button action.", "<action>", 1, &TedPlugin::FsetLButtonAction );
   addCommand( "setRButtonAction", "could not set right mouse button action.", "<action>", 1, &TedPlugin::FsetRButtonAction );
   addCommand( "getLButtonActionIndex", "could not get left mouse button action.", "", 0, &TedPlugin::FgetLButtonActionIndex );
   addCommand( "getRButtonActionIndex", "could not get right mouse button action.", "", 0, &TedPlugin::FgetRButtonActionIndex );
   addCommand( "relight", "could not relight the terrain.", "<hires - bool>", 1, &TedPlugin::Frelight );
   addCommand( "clearSelect", "could not clear the current selection.", "", 0, &TedPlugin::FclearSelect );
   addCommand( "undo", "could not undo last action.", "", 0, &TedPlugin::Fundo );
   addCommand( "redo", "could not redo action.", "", 0, &TedPlugin::Fredo );
   addCommand( "setBrushDetail", "could not set brush detail level.", "<detail level>", 1, &TedPlugin::FsetBrushDetail );
   addCommand( "setPinDetailVal", "failed to set pin detail level", "< detail level >", 0, &TedPlugin::FsetPinDetailVal );
   addCommand( "setPinDetailMax", "failed to set max pin detail level", "< detail level >", 0, &TedPlugin::FsetPinDetailMax );
   addCommand( "setHeightVal", "could not set height value.", "< height >", 0, &TedPlugin::FsetHeightVal );
   addCommand( "setAdjustVal", "could not set height adjustment value.", "< height adjustment >", 0, &TedPlugin::FsetAdjustVal );
   addCommand( "setFlagVal", "could not set flag values.", "[rotate] [flipx] [flipy] [empty{0-7}] [edit] [all]", 0, &TedPlugin::FsetFlagVal );
   addCommand( "setScaleVal", "could not set scale value", "", 0, &TedPlugin::FsetScaleVal );
   addCommand( "processAction", "failed to process action.", "<action>", 1, &TedPlugin::FprocessAction );
   addCommand( "terrainAction", "failed to process action on terrain", "<action>", 1, &TedPlugin::FterrainAction );
   addCommand( "setFlags", "could not set flag values.", "[rotate] [flipx] [flipy] [empty{0-7}] [edit] [all]", 0, &TedPlugin::FsetFlags );
   addCommand( "clearFlags", "could not clear flag values.", "[rotate] [flipx] [flipy] [empty{0-7}] [edit] [all]", 0, &TedPlugin::FclearFlags );
   addCommand( "toggleFlags", "could not toggle flag values.", "[rotate] [flipx] [flipy] [empty{0-7}] [edit] [all]", 0, &TedPlugin::FtoggleFlags );
   addCommand( "rotate", "could not rotate selection.", "", 0, &TedPlugin::Frotate );
   addCommand( "setUndoLimit", "could not set undo limit.", "< limit >", 1, &TedPlugin::FsetUndoLimit );
   addCommand( "listCommands", "could not list commands.", "", 0, &TedPlugin::FlistCommands );
   addCommand( "addNamedSelection", "could not add named selection.", "< name >", 0, &TedPlugin::FaddNamedSelection );
   addCommand( "removeNamedSelection", "could not remove named selection.", "", 0, &TedPlugin::FremoveNamedSelection );
   addCommand( "selectNamedSelection", "could not select selection.", "", 0, &TedPlugin::FselectNamedSelection );
   addCommand( "copy", "could not copy selected terrain.", "", 0, &TedPlugin::Fcopy );
   addCommand( "paste", "could not paste terrain.", "< material | height | all >", 0, &TedPlugin::Fpaste );
   addCommand( "selectMaterial", "could not select material", "< material index >", 0, &TedPlugin::FselectMaterial );
   addCommand( "focus", "could not set focus", "", 0, &TedPlugin::Ffocus );
   addCommand( "unfocus", "could not clear focus", "", 0, &TedPlugin::Funfocus );
   addCommand( "forceTerrainType", "could not force the terrain type", "", 0, &TedPlugin::FforceTerrainType );
   addCommand( "setTerrainType", "could not set terrain type", "< type index >", 0, &TedPlugin::FsetTerrainType );
   addCommand( "getTerrainType", "could not get terrain type", "", 0, &TedPlugin::FgetTerrainType );
   addCommand( "setSmoothVal", "failed to set smooth value", "<smooth value>", 0, &TedPlugin::FsetSmoothVal );
   addCommand( "setPasteVal", "failed to set paste value", "<material | height | all>", 0, &TedPlugin::FsetPasteVal );
   addCommand( "clearPinMaps", "failed to clear pinned maps", "", 0, &TedPlugin::FclearPinMaps );

   addCommand( "getBrushDetail", "failed to get the current brush detail", "", 0, &TedPlugin::FgetBrushDetail );
   addCommand( "getMaxBrushDetail", "failed to get max brush detail", "", 0, &TedPlugin::FgetMaxBrushDetail );
   addCommand( "getNumTerrainTypes", "failed to get terain types", "", 0, &TedPlugin::FgetNumTerrainTypes );
   addCommand( "getTerrainTypeName", "failed to get terrain name", "< index >", 1, &TedPlugin::FgetTerrainTypeName );

   addCommand( "loadPalette", "could not load palette file", "< palette file >", 0, &TedPlugin::FloadPalette );   
   addCommand( "assignMatList", "could not assign material list", "< material list file >", 1, &TedPlugin::FassignMatList );
   addCommand( "loadMatList", "could not load material list file", "< material list file >", 0, &TedPlugin::FloadMatList );   
   addCommand( "getMaterialCount", "failed to get material count", "", 0, &TedPlugin::FgetMaterialCount );
   addCommand( "getMaterialName", "failed to retrieve material name", "<material index>", 1, &TedPlugin::FgetMaterialName );
   addCommand( "getMaterialIndex", "", "", 0, &TedPlugin::FgetMaterialIndex );
   addCommand( "new", "failed to create new terrain file", "", 0, &TedPlugin::Fnew );
   addCommand( "open", "failed to open terrain file", "< filename >", 0, &TedPlugin::Fopen );
   addCommand( "close", "failed to close terrain file", "", 0, &TedPlugin::Fclose );
   addCommand( "save", "failed to save terrain file", "< filename >", 0, &TedPlugin::Fsave );
   addCommand( "setMatIndexVal", "failed to set material index value", "< value >", 0, &TedPlugin::FsetMatIndexVal );
   addCommand( "listNamedSelections", "failed to list named selections", "", 0, &TedPlugin::FlistNamedSelections );
   addCommand( "updateToolBar", "failed to update the toolbar", "", 0, &TedPlugin::FupdateToolBar );
   addCommand( "window", "failed to create ted window", "", 0, &TedPlugin::Fwindow );
   addCommand( "setStatusText", "failed to set status text", "< pane > < text >", 2, &TedPlugin::FsetStatusText );
   addCommand( "saveSelection", "failed to save the current selection", "<name>", 0, &TedPlugin::FsaveSelection );
   addCommand( "loadSelection", "failed to load the current selection", "<name>", 0, &TedPlugin::FloadSelection );
   addCommand( "GetConsoleOptions", "failed to grab console variables", "", 0, &TedPlugin::FGetConsoleOptions );
   addCommand( "getNumActions", "Failed to get number of actions", "", 0, &TedPlugin::FgetNumActions );
   addCommand( "getActionName", "Failed to get name for action", "<action index>", 1, &TedPlugin::FgetActionName );
   addCommand( "isActionMouseable", "Failed to get action", "<action index>", 1, &TedPlugin::FisActionMouseable );
   addCommand( "floatSelection", "Failed to float the selection", "", 0, &TedPlugin::FfloatSelection );
   addCommand( "floatPaste", "Could not float paste selection", "", 0, &TedPlugin::FfloatPaste );
   addCommand( "mirrorGridBlock", "Could not mirror grid block", "<top|bottom|left|right> [x,y - block position]", 1, &TedPlugin::FmirrorGridBlock );
   addCommand( "getWorldName", "Failed to get world name", "", 0, &TedPlugin::FgetWorldName );
}

// add a command into the list of commwands
void TedPlugin::addCommand( const char * command, const char * error, const char * usage, 
   int numArgs, const char * ( TedPlugin::*callback )( int argc, const char * argv[] ) )
{
   ConsoleCommand * newCommand = new ConsoleCommand;;
   
   // copy the data
   newCommand->commandString = command;
   newCommand->errorString = error;
   newCommand->usageString = usage;
   newCommand->numArgs = numArgs;
   newCommand->callback = callback;

   // create the string with the prefix
   char * commandString = new char[ strlen( TEDPREFIX ) + strlen( command ) + 1 ];
   sprintf( commandString, "%s%s", TEDPREFIX, command );
   
   // add the command to the console
   console->addCommand( commands.size(), commandString, this );

   // add to the list
   commands.push_back( newCommand );
      
   delete [] commandString;
} 

bool TedPlugin::mouseCallback( bool up, int button )
{
   // only concerned with the first 2 buttons
   if( button >= 2 )
      button = 1;
      
   mouseDown[ button ] = !up;
   
   return( TRUE );
}

// ----------------------------------------------------------------------
// this function calls the function that was added into the command array
// ----------------------------------------------------------------------
const char *TedPlugin::consoleCallback( CMDConsole*, int id, int argc, const char *argv[] )
{  
   // check that it exists in our table
   if( id < commands.size() )
   {
      // check for something that appears valid
      if( commands[id]->callback == 0 )
      {
         console->printf( "No function associated with command!" );
         setSuccess( false );
         return( 0 );
      }
      
      // we can check the number of commands here ( just check for enough )      
      if( ( ( argc == 2 ) && !stricmp( argv[1], "?" ) ) || 
         ( argc <= commands[id]->numArgs ) )
      {
         // create a message to display to the user that there are improper # of args
         char * message = new char[ strlen( TEDPREFIX ) + commands[id]->commandString.length() +
            strlen( ": " ) + commands[id]->usageString.length() + 1 ];

         sprintf( message, "%s%s: %s", TEDPREFIX, commands[id]->commandString.c_str(),
            commands[id]->usageString.c_str() );
         console->printf( message );
         
         delete [] message;
         
         return( 0 );
      }

      setSuccess( true );      
      
      // call the callback
      const char * retVal = ( this->*commands[id]->callback )( argc, argv );
      if( !stricmp( retVal ? retVal : "", TED_ERROR ) )
      {
         char * message = new char[ strlen( TED_ERROR ) + commands[id]->errorString.length() + 1 ];

         sprintf( message, "%s%s", TED_ERROR, commands[id]->errorString.c_str() );         
         console->printf( message );
         
         setSuccess( false );
         
         delete [] message;
         
         return( 0 );
      }
      
      // update the toolbar windo
      if( window )
         window->updateToolBar();
      
      setSuccess( true );
      return( retVal );
   }
   
   return( 0 );
}

void TedPlugin::setSuccess( bool success )
{
   console->setVariable( "$TED::success", success ? "true" : "false" );
}

bool TedPlugin::getSuccess()
{
   if( !stricmp( console->getVariable( "$TED::success" ), "true" ) )
      return( true );
   return( false );
}

bool TedPlugin::boolify( const char * val )
{
   // check all the true possibilities
   if( !stricmp( val, "true" ) || !stricmp( val, "on" ) || 
      !stricmp( val, "1" ) )
      return( true );
    
   if( !stricmp( val, "false" ) || !stricmp( val, "off" ) || 
      !stricmp( val, "0" ) )
      return( false );
   
   return( false );
}

// ----------------------------------------------------------------------
//  All of the console functions are below
// ----------------------------------------------------------------------
const char * TedPlugin::FinitTed( int argc, const char *argv[] )
{
   argc;argv;

   // get the main window
   const char * winName = console->getVariable( "$TED::mainWindow" );
   SimObject * obj = manager->findObject( winName );
   if( !obj )
      return( TED_ERROR );
   
   SimGui::Canvas * sc = dynamic_cast< SimGui::Canvas* >( manager->findObject( winName ) );
   if( !sc )
   {
      console->printf( "%s: can't find parent canvas", winName );
      return( TED_ERROR );
   }
   
   obj = manager->findObject("TedObject");
   simTed = dynamic_cast<SimTed*>(obj);
   if(!simTed)
      simTed = new SimTed;

   // add to the manager
   simTed->setCanvas( sc );

   manager->addObject( simTed );
   manager->assignName( simTed, "TedObject" );   

   simTed->init();
   
   simTed->setMouseHandler( this );

   return( 0 );
}


const char * TedPlugin::FsetSnap( int argc, const char *argv[] )
{
   argc;
   bool snap = boolify( argv[1] );
   simTed->setSnap( snap );
   return( 0 );
}

const char * TedPlugin::FsetFeather( int argc, const char *argv[] )
{
   argc;
   bool feather = boolify( argv[1] );
   simTed->setFeather( feather );
   return( 0 );
}

const char * TedPlugin::FquitTed( int argc, const char *argv[] )
{
   argc;argv;
   
   // remove the simObject
   if( simTed )
   {
		simTed->deleteObject();
		simTed = NULL;
   }
   
   if( window )
   {
      delete window;
      window = NULL;
   }
   
   return( 0 );
}

const char * TedPlugin::FattachToTerrain( int argc, const char *argv[] )
{
   return( simTed->attachToTerrain( argc >= 2 ? argv[1] : NULL ) ? 0 : TED_ERROR );
}

const char * TedPlugin::FsetSelectFrameColor( int argc, const char *argv[] )
{
   argc;
   int col = atoi( argv[1] );
   if( col >= 256 )
      return( TED_ERROR );
   simTed->setSelectFrameColor( col );
   return( 0 );
}

const char * TedPlugin::FsetSelectFillColor( int argc, const char *argv[] )
{
   argc;
   int col = atoi( argv[1] );
   if( col >= 256 )
      return( TED_ERROR );
   simTed->setSelectFillColor( col );
   return( 0 );
}

const char * TedPlugin::FsetHilightFrameColor( int argc, const char *argv[] )
{
   argc;
   int col = atoi( argv[1] );
   if( col >= 256 )
      return( TED_ERROR );
   simTed->setHilightFrameColor( col );
   return( 0 );
}

const char * TedPlugin::FsetHilightFillColor( int argc, const char *argv[] )
{
   argc;
   int col = atoi( argv[1] );
   if( col >= 256 )
      return( TED_ERROR );
   simTed->setHilightFillColor( col );
   return( 0 );
}

const char * TedPlugin::FsetShadowFrameColor( int argc, const char *argv[] )
{
   argc;
   int col = atoi( argv[1] );
   if( col >= 256 )
      return( TED_ERROR );
   simTed->setShadowFrameColor( col );
   return( 0 );
}

const char * TedPlugin::FsetShadowFillColor( int argc, const char *argv[] )
{
   argc;
   int col = atoi( argv[1] );
   if( col >= 256 )
      return( TED_ERROR );
   simTed->setShadowFillColor( col );
   return( 0 );
}

const char * TedPlugin::FsetSelectShow( int argc, const char *argv[] )
{
   argc;
   BitSet32 flags(0);
   flags.set( SimTed::Frame, boolify( argv[1] ) );
   flags.set( SimTed::Fill, boolify( argv[2] ) );
   flags.set( SimTed::Outline, boolify( argv[3] ) );
   simTed->setSelectShow( flags );
   return( 0 );
}

const char * TedPlugin::FsetHilightShow( int argc, const char *argv[] )
{
   argc;
   BitSet32 flags(0);
   flags.set( SimTed::Frame, boolify( argv[1] ) );
   flags.set( SimTed::Fill, boolify( argv[2] ) );
   flags.set( SimTed::Outline, boolify( argv[3] ) );
   simTed->setHilightShow( flags );
   return( 0 );
}

const char * TedPlugin::FsetShadowShow( int argc, const char *argv[] )
{
   argc;
   BitSet32 flags(0);
   flags.set( SimTed::Frame, boolify( argv[1] ) );
   flags.set( SimTed::Fill, boolify( argv[2] ) );
   flags.set( SimTed::Outline, boolify( argv[3] ) );
   simTed->setShadowShow( flags );
   return( 0 );
}

const char * TedPlugin::FsetBrushDetail( int argc, const char *argv[] )
{
   argc;
   int size = atoi( argv[1] );
   if( size > simTed->getMaxBrushDetail() )
      return( TED_ERROR );
   simTed->setBrushDetail( size );
   return( 0 );
}

const char * TedPlugin::FgetBrushDetail( int argc, const char * argv[] )
{
   argc;argv;
   return( avar( "%d", simTed->getBrushDetail() ) );
}

const char * TedPlugin::FgetMaxBrushDetail( int argc, const char * argv[] )
{
   argc;argv;
   return( avar( "%d", simTed->getMaxBrushDetail() ) );
}

const char * TedPlugin::FgetNumTerrainTypes( int argc, const char * argv[] )
{
   argv;argc;
   LSMapper *ls = simTed->getLSMapper();
   if( !ls )
   {
      console->printf("TedPlugin: No LSMapper object.");
      return( TED_ERROR );
   }
      
   // go through and add all the terrain types to the listbox
   char *name = ls->getTypeDesc( true );
   int count = 0;
   while( name )
   {
      count++;
      name = ls->getTypeDesc();
   }

   return( avar( "%d", count ) );
}

const char * TedPlugin::FgetTerrainTypeName( int argc, const char * argv[] )
{
   argc;
   int index = atoi( argv[1] );
   LSMapper *ls = simTed->getLSMapper();

   if( !ls )
   {
      console->printf("TedPlugin: No LSMapper object.");
      return( TED_ERROR );
   }
   
   const char * name = ls->getTypeDesc( true );
   int count = 0;
   while( name && index != count )
   {
      count++;
      name = ls->getTypeDesc();
   }
   if( index != count )
      return( TED_ERROR );
   return( name );
}

const char * TedPlugin::FsetBrushPos( int argc, const char *argv[] )
{
   argc;
   Point2I pos;
   pos.x = atoi( argv[1] );
   pos.y = atoi( argv[2] );
   simTed->setBrushPos( pos );
   return( 0 );
}

const char * TedPlugin::FsetLButtonAction( int argc, const char *argv[] )
{
   argc;
   simTed->setLButtonAction( argv[1] );
   return( 0 );
}

const char * TedPlugin::FsetRButtonAction( int argc, const char *argv[] )
{
   argc;
   simTed->setRButtonAction( argv[1] );
   return( 0 );
}

const char * TedPlugin::Frelight( int argc, const char *argv[] )
{
   argc;

   simTed->relight(!boolify(argv[1]));
   
   return( 0 );
}

const char * TedPlugin::FclearSelect( int argc, const char *argv[] )
{
   argc;argv;
   
   simTed->clearSelect();
   return( 0 );
}

const char * TedPlugin::Fundo( int argc, const char *argv[] )
{
   argc; argv;
   simTed->undo();
   return( 0 );
}

const char * TedPlugin::Fredo( int argc, const char *argv[] )
{
   argc; argv;
   simTed->redo();
   return( 0 );
}

const char * TedPlugin::FsetMatIndexVal( int argc, const char * argv[] )
{
   int value;
   
   if( argc > 1 )
      console->setVariable( "$TED::matIndexVal", argv[1] );
   else
      console->evaluate( "editBox( MatIndexVal, \"Enter a new value for the material index value\", \"$TED::matIndexVal\" );", false );

   // grab the value
   value = atoi( console->getVariable( "$TED::matIndexVal" ) );
   
   // set the height var in simTed
   simTed->setMaterialIndex( ( UInt8 )value );
   
   return( 0 );
}

const char * TedPlugin::FsetPinDetailVal( int argc, const char * argv[] )
{
   if( argc > 1 )
   {
      if( atoi( argv[1] ) > simTed->getMaxBrushDetail() )
         return( TED_ERROR );
      console->setVariable( "$TED::pinDetailVal", argv[1] );
   }
   else
   {
      char buff[32];
      console->setVariable( "$TED::editValue", "" );
      sprintf( buff, "%d", simTed->getPinDetail() );
      console->setVariable( "$TED::editValue", buff );
      
      // call the edit box and check the value
      console->evaluate( "editBox( pinDetailVal, \"Enter a new value for the pinDetail value\", \"$TED::editValue\" );", false );
      if( stricmp( console->getVariable( RESULTVAR ), OK_STR ) )
         return( 0 );
         
      // check the detail level
      if( atoi( console->getVariable( "$TED::editValue" ) ) >=
         simTed->getMaxBrushDetail() )
         return( TED_ERROR );
         
      // set the variable
      console->setVariable( "$TED::pinDetailVal", 
         console->getVariable( "$TED::editValue" ) );
   }

   // set the pin detail level   
   simTed->setPinDetail( atoi( console->getVariable( "$TED::pinDetailVal" ) ) );
//   simTed->processAction( "pin" );
         
   return( 0 );
}

const char * TedPlugin::FsetPinDetailMax( int argc, const char * argv[] )
{
   if( argc > 1 )
   {
      if( atoi( argv[1] ) > simTed->getMaxBrushDetail() )
         console->setIntVariable( "$TED::pinMaxVal", simTed->getMaxBrushDetail() );
      console->setVariable( "$TED::pinMaxVal", argv[1] );
   }
   else
   {
      char buff[32];
      console->setVariable( "$TED::editValue", "" );
      sprintf( buff, "%d", simTed->getPinDetailMax() );
      console->setVariable( "$TED::editValue", buff );
      
      // call the edit box and check the value
      console->evaluate( "editBox( pinMaxVal, \"Enter a new value for the max pin detail value\", \"$TED::editValue\" );", false );
      if( stricmp( console->getVariable( RESULTVAR ), OK_STR ) )
         return( 0 );
         
      // check the detail level
      if( atoi( console->getVariable( "$TED::editValue" ) ) >=
         simTed->getMaxBrushDetail() )
         return( TED_ERROR );
         
      // set the variable
      console->setVariable( "$TED::pinMaxVal", 
         console->getVariable( "$TED::editValue" ) );
   }

   // set the pin detail level   
   simTed->setPinDetailMax( atoi( console->getVariable( "$TED::pinMaxVal" ) ) );
         
   return( 0 );
}

const char * TedPlugin::FsetHeightVal( int argc, const char * argv[] )
{
   float height;
   
   if( argc > 1 )
      console->setVariable( "$TED::heightVal", argv[1] );
   else
      console->evaluate( "editBox( HeightVal, \"Enter a new value for the height value\", \"$TED::heightVal\" );", false );

   // grab the value
   height = atof( console->getVariable( "$TED::heightVal" ) );
   
   // set the height var in simTed
   simTed->setHeightVal( height );
   
   return( 0 );
}

const char * TedPlugin::FsetSmoothVal( int argc, const char * argv[] )
{
   float smooth;
   
   if( argc > 1 )
      console->setVariable( "$TED::smoothVal", argv[1] );
   else
      console->evaluate( "editBox( SmoothVal, \"Enter a new value for the smooth value\", \"$TED::smoothVal\" );", false );

   // grab the value
   smooth = atof( console->getVariable( "$TED::smoothVal" ) );
   
   // set the height var in simTed
   simTed->setSmoothVal( smooth );
   
   return( 0 );
}

const char * TedPlugin::FsetScaleVal( int argc, const char * argv[] )
{
   float scale;
   
   if( argc > 1 )
      console->setVariable( "$TED::scaleVal", argv[1] );
   else
      console->evaluate( "editBox( ScaleVal, \"Enter a new value for the scale value\", \"$TED::scaleVal\" );", false );

   // grab the value
   scale = atof( console->getVariable( "$TED::scaleVal" ) );
   
   // set the height var in simTed
   simTed->setScaleVal( scale );
   
   return( 0 );
}

const char * TedPlugin::FsetAdjustVal( int argc, const char * argv[] )
{
   float height;
   
   if( argc > 1 )
      console->setVariable( "$TED::adjustVal", argv[1] );
   else
      console->evaluate( "editBox( AdjustVal, \"Enter a new value for the height increment\", \"$TED::adjustVal\" );", false );

   // grab the value
   height = atof( console->getVariable( "$TED::adjustVal" ) );
   
   // set the height var in simTed
   simTed->setAdjustVal( height );
   
   return( 0 );
}


bool TedPlugin::setFlagsFlag( const char * str, UInt8 & flags )
{         
   // check the value
   if( !stricmp( str, "all" ) ) 
      flags = 0xff;
   else if ( !stricmp( str, "rotate" ) ) 
      flags |= GridBlock::Material::Rotate;
   else if ( !stricmp( str, "flipx" ) ) 
      flags |= GridBlock::Material::FlipX;
   else if ( !stricmp(str, "flipy" ) )
      flags |= GridBlock::Material::FlipY;
   else if ( !stricmp( str, "empty1" ) )
      flags |= 1 << GridBlock::Material::EmptyShift;
   else if ( !stricmp( str, "empty2" ) )
      flags |= 2 << GridBlock::Material::EmptyShift;
   else if ( !stricmp( str, "empty3" ) )
      flags |= 4 << GridBlock::Material::EmptyShift;
   else if ( !stricmp( str, "corner" ) )
      flags |= GridBlock::Material::Corner;
   else if ( !stricmp( str, "edit" ) )
      flags |= GridBlock::Material::Edit;
   else
      return( false );
      
   return( true );
}

const char * TedPlugin::FsetPasteVal( int argc, const char * argv[] )
{
   char * valueStr;
   
   // create a multi-select listbox and get the flags
	ToolPlugin::ListBox lbox( false );
   
   if( argc == 1 )
   {
      lbox.addItem( "material" );
      lbox.addItem( "height" );
      lbox.addItem( "all" );
   	lbox.setTexts( "Flags", "Choose paste flags:" );
   
      // show the listbox
      if( !listBoxShow( lbox ) )
         return( TED_ERROR );
      valueStr = lbox.result;
   }
   else
   {
      // set to the arg
      valueStr = ( char * )argv[1];
   }
   
   BitSet32 flag = 0;
   
   // look at the value
   if( !stricmp( valueStr, "material" ) )
      flag.set( SimTed::Material );
   else if( !stricmp( valueStr, "height" ) )
      flag.set( SimTed::Height );
   else if( !stricmp( valueStr, "all" ) )
      flag.set( SimTed::Material | SimTed::Height );
   else
      return( TED_ERROR );
      
   // set it
   simTed->setPasteValue( flag );
   
   return( 0 );
}

const char * TedPlugin::FsetFlagVal( int argc, const char * argv[] )
{
	//parse the params
   UInt8 flags = 0;

   // get the number of args
   if( argc == 1 )
   {
      // create a multi-select listbox and get the flags
   	ToolPlugin::ListBox lbox( true );

   	int i = 0;
      
      // add the items to the listbox
   	while( FlagOpsNames[ i ] )
   		lbox.addItem( FlagOpsNames[ i++ ] );
   	lbox.setTexts( "Flags", "Choose a combination of flags:" );
      
      // show the listbox
      if( listBoxShow(lbox) )
      {
         // go through the string
         char * curr = lbox.result;
         int len = strlen( curr );
         char * buf = new char[ strlen( lbox.result ) + 1 ];
         int pos = 0;
         
         // go through and add all the flags
         while( ( pos <= len ) && ( sscanf( curr, "%s", buf ) ) )
         {
            // set the flag and adjust the ptr to the next item
            setFlagsFlag( buf, flags );
            pos += ( strlen( buf ) + 1 );
            curr = ( lbox.result + pos );
         }
         
         delete [] buf;
      }
      else
         return( 0 );
   }
   else
   {
      int foundCount=0;

      for( int i = 1; i < argc; i++ )
      {
         foundCount++;
         
         if( !setFlagsFlag( argv[1], flags ) )
            foundCount--;
      }

      if( !foundCount )
         return( TED_ERROR );
   }
   
   // set the flag value
   simTed->setFlagVal( flags );
   
   return( 0 );
}

// operations that occur on the current selection
const char * TedPlugin::Frotate( int argc, const char *argv[] )
{
   argc;argv;
   return( simTed->processAction( "rotate" ) ? 0 : TED_ERROR );
}

const char * TedPlugin::FprocessAction( int argc, const char *argv[] )
{
   argc;
   
   // send to simTed
   return( simTed->processAction( argv[1] ) ? 0 : TED_ERROR );
}

const char * TedPlugin::FsetFlags( int argc, const char *argv[] )
{
   // process the flags
   if( !FsetFlagVal( argc, argv ) )
      return( TED_ERROR );
      
   // call the command
   return( simTed->processAction( "setFlags" ) ? 0 : TED_ERROR );
}

const char * TedPlugin::FclearFlags( int argc, const char *argv[] )
{
   // process the flags
   if( !FsetFlagVal( argc, argv ) )
      return( TED_ERROR );
      
   // call the command
   return( simTed->processAction( "clearFlags" ) ? 0 : TED_ERROR );
}

const char * TedPlugin::FtoggleFlags( int argc, const char *argv[] )
{
   // process the flags
   if( !FsetFlagVal( argc, argv ) )
      return( TED_ERROR );
      
   // call the command
   return( simTed->processAction( "toggleFlags" ) ? 0 : TED_ERROR );
}

const char * TedPlugin::FlistCommands( int argc, const char *argv[] )
{
   argc; argv;
   
   ToolPlugin::ListBox lbox(false);
   
   // TEMP
   console->evaluate( "setMainWindow( MainWindow );" );
   
   // go through and add all the commands
   for( int i = 0; i < commands.size(); i++ )
      lbox.addItem( commands[i]->commandString.c_str() );

   lbox.setTexts("Commands", "Command list:");
   lbox.showIt( ToolPlugin::getMainWindow() );
   if( ToolPlugin::resultCheck( lbox.result ) )
   {
      char * commandString = new char[ strlen( TEDPREFIX ) + strlen( lbox.result ) + 1 ];
      if( strlen( lbox.result ) )
      {
         sprintf( commandString, "%s(\"%s\");", TEDPREFIX, lbox.result );
         console->evaluate( commandString );
      }
      delete [] commandString;
   }

   return( 0 );
}

const char * TedPlugin::FsetUndoLimit( int argc, const char *argv[] )
{
   argc;
   int limit = atoi( argv[1] );
   simTed->setUndoLimit( limit );
   return( 0 );
}

const char * TedPlugin::FaddNamedSelection( int argc, const char *argv[] )
{
   if( argc > 1 )
      return( simTed->addNamedSelection( argv[1] ) ? 0 : TED_ERROR );
   else
   {
      // get the value
      console->setVariable( "$TED::editValue", " " );
      console->evaluate( "editBox( Selection, \"Enter a name for the selection\", \"$TED::editValue\" );", false );

      // check if successfull and add the named selection      
      if( !stricmp( console->getVariable( RESULTVAR ), OK_STR ) )
         return( simTed->addNamedSelection( console->getVariable( "$TED::editValue" ) ) ? 0 : TED_ERROR );
   }
   
   return( TED_ERROR );
}

const char * TedPlugin::FremoveNamedSelection( int argc, const char *argv[] )
{
   // check if need to create list
   if( argc > 1 )
      return( simTed->removeNamedSelection( argv[1] ) ? 0 : TED_ERROR );
   else
   {
      console->evaluate( "Ted::listNamedSelections();", false );
      if( !stricmp( console->getVariable( RESULTVAR ), OK_STR ) )
         return( simTed->removeNamedSelection( console->getVariable( "$TED::editValue" ) ) ? 0 : TED_ERROR );
   }
      
   return( 0 );
}

const char * TedPlugin::FselectNamedSelection( int argc, const char *argv[] )
{
   // check if need to create list
   if( argc > 1 )
      return( simTed->selectNamedSelection( argv[1] ) ? 0 : TED_ERROR );
   else
   {
      console->evaluate( "Ted::listNamedSelections();", false );
      if( !stricmp( console->getVariable( RESULTVAR ), OK_STR ) )
         return( simTed->selectNamedSelection( console->getVariable( "$TED::editValue" ) ) ? 0 : TED_ERROR );
   }
   
   return( 0 );
}

//---------------------------------------------------------------------

const char * intToStr(int d)
{
   static char buf[16];
   sprintf(buf, "%d", d);
   return buf;
}

const char * TedPlugin::FgetMaterialCount(int argc, const char * argv[])
{
   argc;argv;
   TSMaterialList * matList = simTed->getMaterialList();
   if(matList)
      return(intToStr(matList->getMaterialsCount()));
      
   return(TED_ERROR);
}

const char * TedPlugin::FgetMaterialName(int argc, const char * argv[])
{
   argc;argv;
   // get the material list
   TSMaterialList * matList = simTed->getMaterialList();
   if(!matList)
      return(TED_ERROR);
   
   // get the index to look
   int index = atoi(argv[1]);
   if(index >= matList->getMaterialsCount())
      return(TED_ERROR);
         
   // get the filename
   TSMaterial mat = matList->getMaterial(index);
   return(avar("%s", mat.fParams.fMapFile));
}

const char * TedPlugin::FgetMaterialIndex( int, const char ** )
{
   return(intToStr(simTed->getMaterialIndex()));
}

//---------------------------------------------------------------------

const char * TedPlugin::FselectMaterial( int argc, const char *argv[] )
{
   int val = 0;
   
   // check if need to make list of the materials
   if( argc > 1 )
      val = atoi( argv[1] );
   else
   {
      typedef Vector<char *> Items;
      Items items;

	   ToolPlugin::ListBox lbox( false );
	   TSMaterialList *matList= simTed->getMaterialList();

	   if( matList )
	   {           
	   	int numMats = matList->getMaterialsCount();
	   	for( int i=0; i < numMats; i++ )
	   	{                              
      		TSMaterial mat = matList->getMaterial( i );
            char *matTex = mat.fParams.fMapFile;    
            if( matTex && strlen( matTex ) )
            {
               char * newitem = new char[ strlen( matTex )+1 ];
               if( newitem ) 
               {
                  strcpy( newitem, matTex );
                  items.push_back( newitem );
                  lbox.addItem( newitem );
               }
            }
	   	}
	   	lbox.setTexts("Materials", "Choose a material:");
         int ok = listBoxShow(lbox);
	   	int res = 0;

         for( Items::iterator i2=items.begin(); i2!= items.end(); i2++, res++ )
	   	{
	   		// find # of selection (windows rearranges them alphabetically)			
	   		if( ok && !strcmp( *i2, lbox.result ) )
	   		{
               // set the value
               val = res;
	   		}
	   		
            delete [] (*i2);
	   	}
	   }
      else
         return( TED_ERROR );
   }

   // set the mat index value      
   char buf[ 32 ];
   sprintf( buf, "%d", val );
   console->setVariable( "$TED::matIndexVal", buf );
  
   // assign the index value and call the action routine
   simTed->setMaterialIndex( ( UInt8 )val );
   
   return( 0 );
}

// copy the selection
const char * TedPlugin::Fcopy( int argc, const char *argv[] )
{
   argc; argv;
   
   return( simTed->processAction( "copy" ) ? 0 : TED_ERROR  );
}

// paste the selection
const char * TedPlugin::Fpaste( int argc, const char *argv[] )
{
   UInt8 val = SimTed::Material | SimTed::Height;
   
   // check the param
   if( argc > 1 )
   {
      if( !stricmp( argv[1], "material" ) )
         val = SimTed::Material;
      else if( !stricmp( argv[1], "height" ) )
         val = SimTed::Height;

      simTed->setPasteValue( val );
   }
   
   // go to action
   return( simTed->processAction( "paste" ) ? 0 : TED_ERROR );
}

// create the window and display it
const char * TedPlugin::Fwindow( int argc, const char *argv[] )
{
   argc;
   
   // check if already exists
   if( window )
      delete window;

   // check to see if they specified a valid canvas 
   SimCanvas * sc = dynamic_cast< SimCanvas* >( manager->findObject( argv[1] ) );
   if( !sc )
   {
      console->printf( "%s: can't find parent canvas", argv[0] );
      return( TED_ERROR );
   }
      
   // create the window
   window = new TedWindow( simTed, sc );
   
   // check it's creation
   if( !window )
      return( TED_ERROR );
      
   return( 0 );
}

const char * TedPlugin::FloadPalette( int argc, const char * argv[] )
{
   const char * name;
   
   if( argc == 1 )
   {
      //c reate an open dialog box to grab the name
      console->evaluate( "openFile( \"$TED::diskName\", \"Palette Files\" \"*.ppl\");", false );

      // get the result
   	name = console->getVariable( "$TED::diskName" );
      if( !strlen( name ) )
         return( TED_ERROR );
   }
   else
      name = argv[1];
      
   SimPalette * pal = new SimPalette;
   
   // add to the sim manager      
   manager->addObject( pal );
   manager->assignName( pal, name );
   
   // open the palette
   if( !pal->open( name ) )
   {
      manager->unregisterObject( pal );
      delete pal;
      console->printf( "TedPlugin: File not found or error reading file." );
      return( TED_ERROR );
   }
   
   return( 0 );
}

const char * TedPlugin::FassignMatList( int argc, const char *argv[] )
{
	argc;
   
   // load up the material list
   if( !simTed->loadMatList( argv[1] ) )
   {
      console->printf( "TedPlugin: File not found or error reading file." );
      return( TED_ERROR );
   }
   
   simTed->lockMatList();
   
   return( 0 );
}

const char * TedPlugin::FloadMatList( int argc, const char * argv[] )
{
   const char * name;
   
   if( argc == 1 )
   {
      //c reate an open dialog box to grab the name
      console->evaluate( "openFile( \"$TED::diskName\", \"Material Lists\", \"*.dml\"", false );

      // get the result
   	name = console->getVariable( "$TED::diskName" );
      if( !strlen( name ) )
         return( TED_ERROR );
   }
   else
   {
      console->setVariable( "$TED::diskName", argv[1] );
   }

   console->evaluate( "Ted::assignMatList( \"$TED::diskName\" );", false );

   return( 0 );
}

const char * TedPlugin::Ffocus( int argc, const char * argv[] )
{
   argc; argv;
   GWWindow *win = ToolPlugin::getMainWindow();
   if( !win )
   {
      console->printf( "use setMainWindow to set main window name." );
      return(0);
   }

   // now post the gain focus event to the simTed object
   SimGainFocusEvent::post( simTed );

   return( 0 );
}

const char * TedPlugin::Funfocus( int argc, const char * argv[] )
{
   argc; argv;

   SimLoseFocusEvent::post( simTed );
   
   return( 0 );
}

const char * TedPlugin::Fclose( int argc, const char * argv[] )
{
	argc, argv;
   
	SimTerrain * terr = simTed->getTerrain();
	
	if( terr )		
	{
		simTed->checkSaveChanges();
	   console->evaluate( "Ted::unfocus();", false );
		simTed->deleteTerrain();
		
      // does not know about any volume other than this
		SimObject *obj = manager->findObject( "tedTerrVol" );
		if( obj )
			obj->deleteObject();
         
	   return( 0 );
	}
   
   return( 0 );
}

const char * TedPlugin::Fnew( int argc, const char * argv[] )
{ 
//   console->addVariable( currFile, TEDPREFIX "currFile", this );
//   
//   const char fnname[] = PREFIX "newTedFile";
//
//   //check #of params
//   if ( !Checkparams( (argc == 8 ), fnname, "objectName scale #blocksW #blocksH"
//               " blockDim matListName GrdBlockPatternType") )
//      return ( true );
//
//   if ( manager->findObject(argv[1]) )
//   {
//      console->printf("%s: %s", fnname, "Object name already taken. "
//                  "Delete it or use another name.");
//      return ( true );
//   }
//
//   //initialize the gridFile
//	int fs = (argc>2) ? atoi(argv[2]) : 8;
//   int fw = (argc>3) ? atoi(argv[3]) : 1;
//   int fh = (argc>4) ? atoi(argv[4]) : 1;
//   int bd = (argc>5) ? atoi(argv[5]) : 8;
//                                                                   
//   if ( fw==0 ) fw++;
//   if ( fh==0 ) fh++;
//   if ( bd==0 ) bd=8;
//
//   int bdshift = 0; int bdcopy=bd;
// 	while (!(bdcopy & 1))
//			bdshift++, bdcopy >>= 1;
//   if ( (1<<bdshift) != bd )
//   {                                                            
//      console->printf("TedPlugin: block dimension must be a power of 2."); 
//      return ( false );                                         
//   }
//
//	char* tname = SimTed::fixName( stripPath( argv[1] ), ".dtf" );
//	bool success = false;
//
//   GridFile::GBPatternMap bp = GridFile::GBPatternMap(atoi(argv[7]));
//   
//   if ( simTed->newFile( fs, fw, fh, bd, tname, bp ) )
//	{
//	   simTed->loadMatList(argv[6]);
//		
//		char* vname = SimTed::fixName( argv[1], ".ted" );
//	   console->setVariable(currFileVar, vname );
//		delete [] vname;
//	   console->setVariable(terrNameVar, tname );
//		success = true;
//	}
//	
//	delete [] tname;
//   return success;
   argc;argv;
   return( 0 );
}

const char * TedPlugin::Fsave( int argc, const char * argv[] )
{
   argv;
   
   // make sure there is a terrain to save
   if( !simTed->getTerrain() )
      return( TED_ERROR );

   const char * fName = NULL;
   
   // check if need to create a dialog box   
   if( ( argc == 1 ) || !stricmp( "", console->getVariable( "$TED::currFile" ) ) )
   {
      char buf[1024];
      sprintf( buf, "saveFileAs( \"$TED::diskName\", \"Ted volume\", \"*.%s\");", 
         console->getVariable( "$TED::terrainExt" ) );
      
      // save this off
      console->evaluate( buf, false );
      
      fName = console->getVariable( "$TED::diskName" );
      
      if( !stricmp( CANCEL_STR, fName ) )
         return( 0 );

      // set the filename
      console->setVariable( "$TED::currFile", fName );
   }
   else if( argc == 2 )
      fName = argv[1];

   // get the path
   FileName fn( console->getVariable( "$TED::currFile" ) );
   console->setVariable( "$TED::currPath", fn.getPath().c_str() );
   
   // save the current file
   return( simTed->save( fName ) ? 0 : TED_ERROR );
}
   
const char * TedPlugin::Fopen( int argc, const char * argv[] )
{
   const char * name;
   
   // check if a dialog is needed
   if( argc == 1 )
   {
      char buf[1024];
      sprintf( buf, "openFile( \"$TED::diskName\", \"Ted volume\", \"*.%s\");", 
         console->getVariable( "$TED::terrainExt" ) );
         
      //c reate an open dialog box to grab the name
      console->evaluate( buf, false );

      // get the result
   	name = console->getVariable( "$TED::diskName" );
      if( !strlen( name ) )
         return( 0 );
   }
   else
   {
      console->setVariable( "$TED::diskName", argv[1] );
   }
   
   // close it
   console->evaluate( "Ted::close();" );
   
   // create the volume
   console->evaluate( "newObject( tedTerrVol, SimVolume, \"$TED::diskName\" );", false );
   
   // get the object
   SimObject * obj = manager->findObject( "tedTerrVol" );
   if( !obj )
      return( TED_ERROR );
   SimVolume * vol = dynamic_cast< SimVolume * > ( obj );
   if( !vol )
      return( TED_ERROR );   

   // get the stream
   ResourceObject *robj = SimResource::get(manager)->find(vol->getFileName());

   char buf[1024];
   if(!robj)
      return( TED_ERROR );

   sprintf(buf, "%s\\%s", robj->filePath, robj->fileName);

   VolumeRStream stream;
   stream.openVolume(buf);
   
   // search for the first .dtf file
   FindMatch fm( "*.dtf" );
   
   stream.findMatches( &fm );
   
   if( !fm.numMatches() )
   {
      console->printf( "TedPlugin: no .dtf file located in ted volume" );
      return( TED_ERROR );
   }
   
   // get the name of the terrain file ( can differ from ted name )
   char * tname = new char[ strlen( fm.matchList[0] ) + 1 ];
   strcpy( tname, fm.matchList[0] );
   
   // load the terrain
   char * eval = new char[ strlen( tname ) + 100 ];
   sprintf( eval, "newObject( Terrain, SimTerrain, Load, \"%s\");", tname );
   delete [] tname;
   console->evaluate( eval, false );
   delete [] eval;
   
   // attach the terrain
	if( !simTed->attachToTerrain( NULL ) )
   {
      console->printf( "TedPlugin: failed to attach terrain" );
      return( TED_ERROR );
   }

   // set the current file   
   console->setVariable( "$TED::currFile", console->getVariable( "$TED::diskName" ) );

   // set focus
   console->evaluate( "Ted::focus();", false );
      
   return( 0 );
}

bool TedPlugin::listBoxShow( ToolPlugin::ListBox& lbox )
{
   GWWindow *wnd = ToolPlugin::getMainWindow();
   if( wnd )
   {
      lbox.showIt( wnd );
      return( ToolPlugin::resultCheck( lbox.result ) ? true : false );
   }
   return( false );
}

const char * TedPlugin::FforceTerrainType( int argc, const char * argv[] )
{
   argc; argv;
   
   LSMapper *ls = simTed->getLSMapper();
   if( !ls )
   {
      console->printf("TedPlugin: No LSMapper object.");
      return( TED_ERROR );
   }
      
   // create a listbox
	ToolPlugin::ListBox lbox( false );

   // go through and add all the terrain types to the listbox
   char *name = ls->getTypeDesc( true );
   while( name )
   {
      lbox.addItem( name );
      name = ls->getTypeDesc();
   }

   // ask for which type to set to
   lbox.setTexts("Terrain Types", "Choose a type:");

   if( listBoxShow( lbox ) )
   {
      simTed->setTerrainType( lbox.intResult );
      simTed->processAction( "setTerrainType" );
   }
      
   return( 0 );
}

// set teh terrain type variable
const char * TedPlugin::FsetTerrainType( int argc, const char * argv[] )
{
   int val;
   // check if should get a listing
   if( argc == 1 )
   {
      LSMapper *ls = simTed->getLSMapper();
      if( !ls )
      {
         console->printf("TedPlugin: No LSMapper object.");
         return( TED_ERROR );
      }
         
      // create a listbox
   	ToolPlugin::ListBox lbox( false );

      // go through and add all the terrain types to the listbox
      char *name = ls->getTypeDesc( true );
      while( name )
      {
         lbox.addItem( name );
         name = ls->getTypeDesc();
      }

      // ask for which type to set to
      lbox.setTexts("Terrain Types", "Choose a type:");

      // set the terrain type
      if( listBoxShow( lbox ) )
         simTed->setTerrainType( lbox.intResult );
         
      return (0);
   }
   else
      val = atoi( argv[1] );
      
   simTed->setTerrainType( val );
   
   return( 0 );
}

const char * TedPlugin::FgetTerrainType( int argc, const char * argv[] )
{
   argc; argv;
   return( avar( "%d", simTed->getTerrainType() ) );
}

const char * TedPlugin::FlistNamedSelections( int argc, const char * argv[] )
{
   argc; argv;
   int count = simTed->getNumNamedSelections();
   if( !count )
      return( TED_ERROR );
         
   ToolPlugin::ListBox lbox(false);
   
   // add all the items
   for( int i = 0; i < count; i++ )
      lbox.addItem( simTed->getNamedSelection( i ) );
   
   lbox.setTexts( "Selection", "Choose a named selection:" );
   if( listBoxShow( lbox ) )
      console->setVariable( "$TED::editValue", lbox.result );
   
   return( 0 );
}

const char * TedPlugin::FupdateToolBar( int argc, const char * argv[] )
{
   argc;argv;
   if( !window )
      return( 0 );
      
   window->updateToolBar();
   
   return( 0 );
}

const char * TedPlugin::FsetStatusText( int argc, const char * argv[] )
{
   argc;
   if( !window )
      return( 0 );
   window->setStatusText( atoi( argv[1] ), argv[2] );
   return( 0 );
}

const char * TedPlugin::FloadSelection( int argc, const char * argv[] )
{
   const char * name;
   
   if( argc == 1 )
   {
      // save this off
      console->evaluate( "openFile( \"$TED::diskName\", \"Ted Selection Map\", \"*.map\");", false );
   
      name = console->getVariable( "$TED::diskName" );
      if( !stricmp( CANCEL_STR, name ) )
         return( 0 );
   }
   else
      name = argv[1];
    
   // load it 
   return( simTed->loadSel( name ) ? 0 : TED_ERROR );
}

const char * TedPlugin::FsaveSelection( int argc, const char * argv[] )
{
   const char * name;

   if( argc == 1 )
   {
      // save this off
      console->evaluate( "saveFileAs( \"$TED::diskName\", \"Ted Selection Map\", \"*.map\");", false );
   
      name = console->getVariable( "$TED::diskName" );
      if( !stricmp( CANCEL_STR, name ) )
         return( 0 );
   }
   else
      name = argv[1];
   
   // save it
   return( simTed->saveCurrentSel( name ) ? 0 : TED_ERROR );
}

// set the visibility of a frame around gridblocks
const char * TedPlugin::FsetBlockOutline( int argc, const char * argv[] )
{
   argc;
   bool outline = boolify( argv[1] );
   simTed->setBlockOutline( outline );
   return( 0 );
}

// set the color of the frame that surrounds gridblocks
const char * TedPlugin::FsetBlockFrameColor( int argc, const char * argv[] )
{
   argc;
   int col = atoi( argv[1] );
   if( col >= 256 )
      return( TED_ERROR );
   simTed->setBlockFrameColor( col );
   return( 0 );
}

// clears the pinmap for the current gridblock ( the one with the mouse in it )
const char * TedPlugin::FclearPinMaps( int argc, const char * argv[] )
{
   argc; argv;
   simTed->clearPinMaps();
   return( 0 );
}

// grab all the console variables and set the simted object up...
const char * TedPlugin::FGetConsoleOptions( int argc, const char * argv[] )
{
   argc;argv;
   
   BitSet32 displayFlags;
   
   // set the display vars
   displayFlags = 0;
   displayFlags.set( SimTed::Frame, console->getBoolVariable( "$TED::selectionDisplayFrame", true ) );
   displayFlags.set( SimTed::Fill, console->getBoolVariable( "$TED::selectionDisplayFill", true ) );
   displayFlags.set( SimTed::Outline, console->getBoolVariable( "$TED::selectionDisplayOutline", false ) );
   simTed->setSelectShow( displayFlags );
   
   displayFlags = 0;
   displayFlags.set( SimTed::Frame, console->getBoolVariable( "$TED::shadowDisplayFrame", true ) );
   displayFlags.set( SimTed::Fill, console->getBoolVariable( "$TED::shadowDisplayFill", true ) );
   displayFlags.set( SimTed::Outline, console->getBoolVariable( "$TED::shadowDisplayOutline", false ) );
   simTed->setShadowShow( displayFlags );

   displayFlags = 0;
   displayFlags.set( SimTed::Frame, console->getBoolVariable( "$TED::hilightDisplayFrame", true ) );
   displayFlags.set( SimTed::Fill, console->getBoolVariable( "$TED::hilightDisplayFill", true ) );
   displayFlags.set( SimTed::Outline, console->getBoolVariable( "$TED::hilightDisplayOutline", false ) );
   simTed->setHilightShow( displayFlags );

   // do the tile flags...   
   BitSet32 tileFlags = 0;
   tileFlags.set( GridBlock::Material::Corner, console->getBoolVariable( "$TED::flagCorner", false ) );
   tileFlags.set( GridBlock::Material::Edit, console->getBoolVariable( "$TED::flagEdit", false ) );
   tileFlags.set( 1 << GridBlock::Material::EmptyShift, console->getBoolVariable( "$TED::flagEmpty1", false ) );
   tileFlags.set( 2 << GridBlock::Material::EmptyShift, console->getBoolVariable( "$TED::flagEmpty2", false ) );
   tileFlags.set( 4 << GridBlock::Material::EmptyShift, console->getBoolVariable( "$TED::flagEmpty3", false ) );
   tileFlags.set( GridBlock::Material::FlipX, console->getBoolVariable( "$TED::flagFlipX", false ) );
   tileFlags.set( GridBlock::Material::FlipY, console->getBoolVariable( "$TED::flagFlipY", false ) );
   tileFlags.set( GridBlock::Material::Rotate, console->getBoolVariable( "$TED::flagRotate", false ) );
   simTed->setFlagVal( tileFlags );

   // do the paste flags
   BitSet32 pasteFlags = 0;
   pasteFlags.set( SimTed::Material, console->getBoolVariable( "$TED::pasteMaterial", true ) );
   pasteFlags.set( SimTed::Height, console->getBoolVariable( "$TED::pasteHeight", true ) );
   simTed->setPasteValue( pasteFlags );
   
   // misc
   simTed->setBlockOutline( console->getBoolVariable( "$TED::blockDisplayOutline", false ) );
   simTed->setSnap( console->getBoolVariable( "$TED::brushSnap", true ) );
   simTed->setFeather( console->getBoolVariable( "$TED::brushFeather", true ) );
   
   // get the values
   simTed->setHeightVal( console->getFloatVariable( "$TED::heightVal", 50.f ) );
   simTed->setAdjustVal( console->getFloatVariable( "$TED::adjustVal", 5.f ) );
   simTed->setScaleVal( console->getFloatVariable( "$TED::scaleVal", 1.f ) );
   simTed->setPinDetail( console->getIntVariable( "$TED::pinDetailVal", 0 ) );
   simTed->setPinDetailMax( console->getIntVariable( "$TED::pinMaxVal", 8 ) );
   simTed->setSmoothVal( console->getFloatVariable( "$TED::smoothVal", 0.5f ) );

   return(0);
}

const char * TedPlugin::FgetNumActions( int argc, const char * argv[] )
{
   argc;argv;
   return( avar( "%d", simTed->getNumActions() ) );
}

const char * TedPlugin::FgetActionName( int argc, const char * argv[] )
{
   argc;
   SimTed::TedAction action;
   if( !simTed->getActionInfo( atoi( argv[1] ), action ) )
      return( TED_ERROR );
   return( avar( "%s", action.name.c_str() ) );
}

const char * TedPlugin::FisActionMouseable( int argc, const char * argv[] )
{
   argc;
   SimTed::TedAction action;
   if( !simTed->getActionInfo( atoi( argv[1] ), action ) )
      return( TED_ERROR );
   return( action.mouseAction ? "True" : "False" );
}

const char * TedPlugin::FfloatSelection( int argc, const char * argv[] )
{
   argc;argv;
   if( simTed->currentSelValid() )
      simTed->floatCurrent();
   return( 0 );
}

const char * TedPlugin::FfloatPaste( int argc, const char * argv[] )
{
   argc;argv;
   if( simTed->pasteBufferValid() )
   {
      simTed->setFloatPasteVal( simTed->getPasteValue() );
      simTed->setSelFloating( true );
   }
   return( 0 );
}

// this will mirror only the central block in a OneBlockMapsToAll grid pattern
const char * TedPlugin::FmirrorGridBlock( int argc, const char * argv[] )
{
   if(!strlen(argv[1]))
      return(TED_ERROR);
   
   int sides = 0;
   if(strstr(argv[1], "top"))
      sides |= SimTed::top;
   if(strstr(argv[1], "bottom"))
      sides |= SimTed::bottom;
   if(strstr(argv[1], "left"))
      sides |= SimTed::left;
   if(strstr(argv[1], "right"))
      sides |= SimTed::right;
      
   if(!simTed->mirrorTerrain(sides))
      return(TED_ERROR);
      
   return(0);
}

// apply's an action on all the terrain
const char * TedPlugin::FterrainAction(int argc, const char * argv[])
{
   argc;
   if(!simTed->terrainAction(argv[1]))
      return(TED_ERROR);
   return(0);
}

// get's the currnet ted mouse action indices
const char * TedPlugin::FgetLButtonActionIndex(int argc, const char * argv[])
{
   argc;argv;
   return(avar("%d",simTed->getLButtonActionIndex()));
}

const char * TedPlugin::FgetRButtonActionIndex(int argc, const char * argv[])
{
   argc;argv;
   return(avar("%d",simTed->getRButtonActionIndex()));
}

//-----------------------------------------------------------------
// get the world name from the world xxx from the SimVolume xxxWorld.vol
const char * TedPlugin::FgetWorldName(int argc, const char * argv[])
{
   argc;argv;
   SimManager * serverManager = SimGame::get()->getManager(SimGame::SERVER);
   SimVolume * worldVol = static_cast<SimVolume*>(serverManager->findObject("MissionGroup\\Volumes\\World"));
   if(!worldVol)
      return(TED_ERROR);
   if(!worldVol->getFileName() || !strlen(worldVol->getFileName()))
      return(TED_ERROR);
      
   // get the filename and then look for 'world' and grab the stuff before it   
   char fileName[MAX_FILE];
   strcpy(fileName, worldVol->getFileName());

   for(unsigned int i = 0; i < strlen(fileName); i++)
      fileName[i] = tolower(fileName[i]);
   
   // grab it
   char * pos = strstr(fileName, "world");
   if(!pos)
      return(TED_ERROR);
   *pos = '\0';
   
   return(avar("%s", fileName));
}
