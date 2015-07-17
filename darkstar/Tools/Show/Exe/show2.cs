// ***********************************************************************
//
// Show tool functions, aliases.  What were previously scripts have been 
//      mostly moved in here as function()s for the update to the new (May '98) 
//      console.  
//
// Startup code is below.  
// 
// ***********************************************************************


showPing();

function lightOn()
{
   newObject( lite1, SimLight, Directional, 1, 1, 1, 1, 0, 0 );
   newObject( lite2, SimLight, Directional, 1, 1, 1 , 1, 0, 0 );
   newObject( lite3, SimLight, Directional, 1, 1, 1, 0, 1, 0 );
   newObject( lite4, SimLight, Directional, 1, 1, 1, 0 , 1, 0 );
   newObject( lite5, SimLight, Directional, 1, 1, 1, 0, 0, 1 );
   newObject( lite6, SimLight, Directional, 1, 1, 1, 0, 0 , 1 );
}

showPing();

function lightOff()
{
   deleteObject( lite1 );
   deleteObject( lite2 );
   deleteObject( lite3 );
   deleteObject( lite4 );
   deleteObject( lite5 );
   deleteObject( lite6 );
}

showPing();

function toggleLightsOnOff()
{
   if ( $LightsAreOn )
   {
      lightOff();
      $LightsAreOn = "False";
   }
   else
   {
      lightOn();
      $LightsAreOn = "True";
   }
}

// e.g. parameter = "mars"
function terr( %whichTerr )
{
   deleteObject ( Terrain );
   // cls();
    
   // arg construction
   %simVol = strcat ( %whichTerr, ".sim.vol" );
   %tedVol = strcat ( %whichTerr, ".ted.vol" );
   %terrainDtf = strcat ( %whichTerr, ".dtf" );
    
   // default volumes
   newObject ( terrainVol, SimVolume, %simVol );
   newObject ( tedVolume, SimVolume, %tedVol );

   newObject ( Terrain, SimTerrain, Load, %terrainDtf, -24576, -24576, -250, 0,0,0 );
   newObject ( sky, SimSky );
}


function viewShape()
{
   openFile( Show::diskFile, "DTS shape files", "*.dts" );
   if ( $dlgResult == "[ok]" )
    {
      view ( $Show::diskFile );      // a showPlugin command
   }
}


function showGfxSW()
{
   $ConsoleWorld::Eval = "echo($ConsoleWorld::FrameRate, \" P:\", $GFXMetrics::EmittedPolys, \", \", $GFXMetrics::RenderedPolys, \"S:\", $GFXMetrics::UsedSpans, \" TSU:\", $GFXMetrics::textureSpaceUsed);";
}


function loadPal()
{
   openFile ( Show::diskFile, "Palette files", "*.ppl" );
   if ( $dlgResult == "[ok]" )
   {
      newObject( "newPal", SimPalette, $Show::diskFile, "true" );
   }
}

function saveShapeAs()
{
   saveFileAs( Show::diskFile, "DTS shape files", "*.dts" );
   if ( $dlgResult == "[ok]" )
   {
      saveShape( $Show::diskFile );
   }
}

function impSeq()
{
   openFile( Show::diskFile, "Shape files", "*.dts" );
   if ( $dlgResult == "[ok]" )
    {
        editBox( "Import which sequence?", "Enter sequence #", Show::seqNum );
        importSequence( $Show::diskFile, $Show::seqNum );
   }
}

function impSeq1()
{
   openFile( Show::diskFile, "Shape files", "*.dts" );
   if ( $dlgResult == "[ok]" )
    {
        importSequence( $Show::diskFile, 0 );
   }
}

function impTrans()
{
   openFile( Show::diskFile, "Shape files", "*.dts" );
   if ( $dlgResult == "[ok]" )
   {
      importTransitions( $Show::diskFile );
   }
}




// **********************************************************************************
// **********************************************************************************
//
// Show tool startup code.  Environment specific data should be kept in 
//      autoexec.cs.  
//
// **********************************************************************************
// **********************************************************************************

$WinConsoleEnabled = "True";
$Console::Prompt = "% ";
$LightsAreOn = "False";


// newObject MainWindow MyCanvas "Show Tool" 640 480 t 1\n
//  sim\simGame.cpp:
newObject(MainWindow, MyCanvas, "Show Tool", 640, 480, True, 1);

// GuiNewContentCtrl MainWindow SimGui::TSControl
//      simGui\simGuiPlugin.cpp:
GuiNewContentCtrl(MainWindow, SimGui::TSControl);

// setMainWindow MainWindow
setMainWindow( MainWindow );

// Defined in show\showPlugin.cpp:
loadMainMenu();

// echo() Defined twice:  console\console.cpp, sim\simConsole.cpp:
// set ConsoleWorld::DefaultSearchPath "..\art;d:\show\art\"
echo( "First echo...?" );
$ConsoleWorld::DefaultSearchPath = ".;DTS;c:\\work\\fear\\darkstar\\tools\\show\\exe";

//  simGui\simGuiPlugin.cpp:
// setCursor MainWindow arrow.bmp
// cursorOn MainWindow
setCursor ( MainWindow, "Arrow.bmp" );
cursorOn ( MainWindow );
// cursorOff ( MainWindow );


// Load in default GFXFont
$Console::GFXFont = "console.pft";

// ***********************************************************************
//      
// ***********************************************************************


$zRotIncrement = 0.0;
$xRotIncrement = 0.0;
$distIncrement = 0.0;

function setSpeed( %factor )
{
   $zRotIncrement = %factor * 0.008;
   $xRotIncrement = %factor * 0.008;
   $distIncrement = %factor * 0.024;
    
    // We could check the Showcam::* variables and set them if they're currently
    //      not zero at this point..?  
    
   if( $Showcam::zInc != 0.0 )
   {
      if( $Showcam::zInc < 0.0 ) 
      {
         $Showcam::zInc = -$zRotIncrement;
      }
      else
      {
          $Showcam::zInc = $zRotIncrement;
      }
   }
}

function incIncs(%pct)
{
   $Showcam::zInc = $Showcam::zInc + $Showcam::zInc * %pct;
   $Showcam::xInc = $Showcam::xInc + $Showcam::xInc * %pct;
   $Showcam::dInc = $Showcam::dInc + $Showcam::dInc * %pct;
}
function speed1(){
   setSpeed( 1 );
}
function speed2(){
   setSpeed( 2 );
}
function speed3(){
   setSpeed( 4 );
}
function speed4(){
   setSpeed( 7 );
}
function speed5(){
   setSpeed( 10 );
}
function speed6(){
   setSpeed( 14 );
}
function speed7(){
   setSpeed( 21 );
}
function speed8(){
   setSpeed( 29 );
}
function speed9(){
   setSpeed( 40 );
}


speed2();


function panLeft()       {       $Showcam::zInc = -$zRotIncrement;      }
function panRight()    {       $Showcam::zInc = $zRotIncrement;      }
function stopLeftRight(){      $Showcam::zInc = 0.0;               }

function panUp()      {      $Showcam::xInc = -$xRotIncrement;      }
function panDown()      {      $Showcam::xInc = $xRotIncrement;      }
function stopUpDown()   {      $Showcam::xInc = 0.0;               }

function panIn()      {      $Showcam::dInc = -$distIncrement;      }
function panOut()      {      $Showcam::dInc = $distIncrement;      }
function stopInOut()   {      $Showcam::dInc = 0.0;               }


function setFullScreenDriver()
{
   if (isGfxDriver(MainWindow, "Glide"))
   {
      setFullscreenDevice(MainWindow, Glide);
      echo("Fullscreen Driver: Glide");
   }
   else
   {
      setFullscreenDevice(MainWindow, Software);
      echo("Fullscreen Driver: Software");
   }
}


// newActionMap ( "showMap.sae" );


// alias action postAction '$ActionTarget'
//  example...:  postAction(TS_0, Attach, editCamera);
//  example:   bindAction( keyboard, make, d ,TO, IDACTION_MOVEFORWARD, 1.0);

//-------------


bind( keyboard, make, t, to, "light();" );


   bind( keyboard, make, 1, to, "speed1();" );
   bind( keyboard, make, 2, to, "speed2();" );
   bind( keyboard, make, 3, to, "speed3();" );
   bind( keyboard, make, 4, to, "speed4();" );
   bind( keyboard, make, 5, to, "speed5();" );
   bind( keyboard, make, 6, to, "speed6();" );
   bind( keyboard, make, 7, to, "speed7();" );
   bind( keyboard, make, 8, to, "speed8();" );
   bind( keyboard, make, 9, to, "speed9();" );
   bind( keyboard, make, 0, to, "speed0();" );

//***** 
//***** View panning keys.  Note I, O and P can be used to turn on / stop a continuous
//*****   pan around the shape.  
//***** 
bind( keyboard, make, e, to,  "panIn();" );
bind( keyboard, break, e, to, "stopInOut();" );
bind( keyboard, make, c, to,  "panOut();" );
bind( keyboard, break, c, to, "stopInOut();" );

bind( keyboard, make, a, to,  "panLeft();" );
bind( keyboard, break, a, to, "stopLeftRight();" );
bind( keyboard, make, d, to,  "panRight();" );
bind( keyboard, break, d, to, "stopLeftRight();" );

bind( keyboard, make, w, to,  "panUp();" );
bind( keyboard, break, w, to, "stopUpDown();" );
bind( keyboard, make, x, to,  "panDown();" );
bind( keyboard, break, x, to, "stopUpDown();" );

bind( keyboard, make, i, to,  "panLeft();" );
bind( keyboard, make, o, to,  "panRight();" );
bind( keyboard, make, p, to,  "stopLeftRight();" );
    
bind( keyboard, make, control, q, to, "quit();" );
bind( keyboard, make, g, to, "toggleLightsOnOff();" );

bind( keyboard, make, "-", to, "incIncs(-0.06);" );
bind( keyboard, make, "+", to, "incIncs( 0.06);" );
bind( keyboard, make, F1, to, "viewshape();"  );
bind( keyboard, make, F2, to, "shapeDialog(1);"  );
bind( keyboard, make, F3, to, "hideToolWin(terrainBar); showToolWin(terrainBar);"  );
bind( keyboard, make, F4, to, "loadpal();"  );
bind( keyboard, make, f5, to, "swapSurfaces(MainWindow);");
bind( keyboard, make, f6, to, "setFullScreenDevice(MainWindow,Software);");
bind( keyboard, make, f7, to, "setFullScreenDevice(MainWindow,Glide);");

bind( keyboard, make, F6, to, "messageCanvasDevice ( MainWindow, outline );" );

//-------------


// alias close "echo"
// alias setGlide setFullScreenDevice MainWindow Glide
// bind make F9 setGlide

$AllowMultipleShapes = False;
exec( "autoexec.cs" );
newObject( defaultPalette, SimPalette, $PaletteName, true );

// These commands are in simObjects\toolPlugin.cpp:
//       newToolWindow terrainBar MainWindow
//       addToolGap terrainBar gap1
newToolWindow( terrainBar, MainWindow );
addToolGap( terrainBar, gap1 );

addToolButton( terrainBar, MarsTerrain, "arrow.bmp" );
setButtonHelp( terrainBar, MarsTerrain, "Mars Day" );
setToolCommand( terrainBar, MarsTerrain, "terr( \"Mars\" );" );

addToolButton( terrainBar, IceTerrain, "arrow.bmp" );
setButtonHelp( terrainBar, IceTerrain, "Ice Day" );
setToolCommand( terrainBar, IceTerrain, "terr( \"Ice\" );" );

addToolGap( terrainBar, gap2 );
setToolWinPos( terrainBar, 600, 600 );


// set Perf::params "10 10"
// bind F5 perfchk

// bind F7 impseq1
// bind F8 camDialog
// # bind F9 line
// bind F10 "deleteObject sky; not GFX::DrawOutline"
// bind F11 alias action postAction showCam
// bind F12 alias action postAction '$ActionTarget'

// # inputDeactivate joystick 0

// These two were called from show\Main.cpp:
newObject( showsky, SimSky, 0, 0, 0 );
inputActivate ( keyboard0 );
setFullScreenDriver();

