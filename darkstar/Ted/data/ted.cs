function ted( %window )
{
   if( %window == "" )
   {
      $TED::mainWindow = "MainWindow";
   }
   else
   {
      $TED::mainWindow = %window;
   }

   // ---------------------------
   // TED Variables
   // ---------------------------
   // flags
   $TED::flagCorner                 = false;
   $TED::flagEdit                   = false;
   $TED::flagEmpty1                 = false;
   $TED::flagEmpty2                 = false;
   $TED::flagEmpty3                 = false;
   $TED::flagFlipX                  = false;
   $TED::flagFlipY                  = false;
   $TED::flagRotate                 = false;
    
    // paste
   $TED::pasteMaterial              = true;
   $TED::pasteHeight                = true;
    
   // values
   $TED::heightVal                  = 50;
   $TED::adjustVal                  = 5;
   $TED::scaleVal                   = 1;
   $TED::pinDetailVal               = 0;
   $TED::pinMaxVal                  = 12;
   $TED::matIndexVal                = 0;
   $TED::smoothVal                  = 0.5;

   // display
   $TED::selectionDisplayFrame      = true;
   $TED::selectionDisplayOutline    = false;
   $TED::selectionDisplayFill       = false;
   $TED::selectionColorFrame        = 3;
   $TED::selectionColorFill         = 4;
   $TED::hilightDisplayFrame        = true;
   $TED::hilightDisplayOutline      = false;
   $TED::hilightDisplayFill         = false;
   $TED::hilightColorFrame          = 2;
   $TED::hilightColorFill           = 8;
   $TED::shadowDisplayFrame         = true;
   $TED::shadowDisplayOutline       = false;
   $TED::shadowDisplayFill          = false;
   $TED::shadowColorFrame           = 6;
   $TED::shadowColorFill            = 12;
   $TED::blockDisplayOutline        = false;

   // misc
   $TED::brushSnap                  = true;    
   $TED::brushFeather               = true;

   // system
   $TED::castInteriorShadows        = true;
   $TED::success                    = false;
   $TED::diskName                   = "";
   $TED::currFile                   = "";
   $TED::currPath                   = "";
   $TED::terrainNameChange          = false;
   $TED::terrainExt                 = "ted";
   $TED::editValue                  = "";

   Ted::initTed();
   Ted::attachToTerrain();

   // ---------------------------
   // set the default values
   // ---------------------------
   Ted::setHeightVal( $TED::heightVal );
   Ted::setAdjustVal( $TED::adjustVal );
   Ted::setScaleVal( $TED::scaleVal );
   Ted::setPinDetailVal( $TED::pinDetailVal );
   Ted::setPinDetailMax( $TED::pinMaxVal );
   Ted::setSmoothVal( $TED::smoothVal );
   Ted::setMatIndexVal( $TED::matIndexVal );
   
   Ted::setSelectShow( $TED::selectionDisplayFrame, $TED::selectionDisplayFill, $TED::selectionDisplayOutline );
   Ted::setSelectFrameColor( $TED::selectionColorFrame );
   Ted::setSelectFillColor( $TED::selectionColorFill );
   Ted::setHilightShow( $TED::hilightDisplayFrame, $TED::hilightDisplayFill, $TED::hilightDisplayOutline );
   Ted::setHilightFrameColor( $TED::hilightColorFrame );
   Ted::setHilightFillColor( $TED::hilightColorFill );
   Ted::setShadowShow( $TED::shadowDisplayFrame, $TED::shadowDisplayFill, $TED::shadowDisplayOutline );
   Ted::setShadowFrameColor( $TED::shadowColorFrame );
   Ted::setShadowFillColor( $TED::shadowColorFill );
   
   Ted::setSnap( $TED::brushSnap );
   Ted::setFeather( $TED::brushFeather );
   
   Ted::setBlockOutline( $TED::blockDisplayOutline );

   // only show if not in new editor   
   if( !$ME::Loaded )   
   {
      Ted::window( $Ted::mainWindow );

      setMainWindow( $Ted::mainWindow );

      cursorOn( MainWindow );
      GuiNewContentCtrl( MainWindow, SimGui::TSControl );
   }
}
