function saveMission( %objName, %fileName )
{
   focusServer();
	exportObjectToScript( %objName, %fileName);
   focusClient();
}

function missionSaveObject( %objName, %fileName )
{
	focusServer();
	exportObjectToScript( %objName, "temp\\" @ %fileName );
   $ConsoleWorld::DefaultSearchPath = $ConsoleWorld::DefaultSearchPath;
	focusClient();
}

function missionLoadObject( %objParentName, %fileName )
{
	focusServer();
	setInstantGroup( %objParentName );
	exec( %fileName ); // temp is in the path.
	focusClient();
}

function edit()
{
   GuiEditMode(MainWindow);
   GuiInspect(MainWindow);
   cursorOn(MainWindow);
}

function ObjectToCamera::onAction()
{
   ME::ObjectToCamera();
}

function CameraToObject::onAction()
{
   ME::CameraToObject();
}

function ObjectToScreenCenter::onAction()
{
   ME::ObjectToSC();
}

function ME::onCreateVolume()
{
   Control::setVisible(AddVolume, true);
}

function ME::CreateVolume(%name)
{
   MissionCreateObject(Volume, SimVolume, %name, true);
}

function ME::AddFlag()
{
   %obj = MissionCreateObject(flag, Item, Flag, 1, false);
   focusServer();
   if($Game::missionType == "F&R")
      %obj.scoreValue = 2;
   else 
      %obj.scoreValue = 1;
}

function ME::AddTowerSwitch()
{
   %obj = MissionCreateObject(Switch, StaticShape, TowerSwitch);
   focusServer();
   if($Game::missionType == "C&H")
      %obj.scoreValue = 12;
   else
      %obj.scoreValue = 1;
}

function MissionRegObject(%group, %name, %func, %a1, %a2, %a3, %a4, %a5, %a6, %a7, %a8, %a9, %a10)
{
   if($ME::NumGroups == "")
      $ME::NumGroups = 0;

   for(%i = 0; %i < $ME::NumGroups; %i++)
      if($ME::Group[%i] == %group)
         break;
   if(%i == $ME::NumGroups)
   {
      $ME::Group[%i] = %group;
      $ME::NumGroups++;
   }
   $ME::GroupEC[%group]++;
   $ME::Group[%group, ($ME::GroupEC[%group] - 1)] = %name;

   %funcStr = %func @ "(";
   if(%a1 == "")
      %funcStr = %funcStr @ ");";
   for(%j = 1; %a[%j] != ""; %j++)
   {
      if(%a[%j + 1] == "")
         %funcStr = %funcStr @ "\"" @ %a[%j] @ "\");";
      else
         %funcStr = %funcStr @ "\"" @ %a[%j] @ "\", ";
   }
   $ME::Func[%group, %name] = %funcStr;
}

function GroupList::onAction()
{
   TextList::clear("NameList");
   %group = Control::getValue("GroupList");
   $ME::curGroup = %group;

   for(%i = 0; %i < $ME::GroupEC[%group]; %i++)
      TextList::AddLine("NameList", $ME::Group[%group, %i]);
}

function NameList::onAction()
{
   %name = Control::getValue("NameList");
   %func = $ME::Func[$ME::curGroup, %name];
   eval(%func);
}

function MissionObjectList::onUnselected(%world, %obj)
{
   if(%obj == $ME::InspectObject && %world == $ME::InspectWorld)
   {
      MissionObjectList::Inspect(1, -1);
      $ME::InspectObject = "";
   }
   
   ME::onUnselected( %world, %obj );
}

function MissionObjectList::onSelectionCleared()
{
   MissionObjectList::Inspect(1, -1);
   $ME::InspectObject = "";
   
   ME::onSelectionCleared();
}

function MissionObjectList::onSelected(%world, %obj)
{
   if($ME::InspectObject == "")
   {
      $ME::InspectObject = %obj;
      $ME::InspectWorld = %world;
      MissionObjectList::Inspect($ME::InspectWorld, %obj);
      
      // it's on the server:
      focusServer();
      %locked = %obj.locked;
      focusClient();

      if(%locked)
         Control::setText("LockButton", "Unlock");
      else
         Control::setText("LockButton", "Lock");
   }
   
   ME::onSelected( %world, %obj );
}

function ApplyButton::onAction()
{
   MissionObjectList::Apply();
}

function LockButton::onAction()
{
   if($ME::InspectObject != "")
   {
      focusServer();
      %locked = !$ME::InspectObject.locked;
      $ME::InspectObject.locked = %locked;
      focusClient();
      if(%locked)
         Control::setText("LockButton", "Unlock");
      else
         Control::setText("LockButton", "Lock");
      MissionObjectList::Inspect($ME::InspectWorld, $ME::InspectObject);
   }
}

function ME::SnapToGrid()
{
   Control::setActive(XGridSnapCtrl, $ME::SnapToGrid);
   Control::setActive(YGridSnapCtrl, $ME::SnapToGrid);
   Control::setActive(ZGridSnapCtrl, $ME::SnapToGrid);
   Control::setActive(UseTerrainGrid, $ME::SnapToGrid);
}

function UseTerrainGrid::onAction()
{
	ME::onUseTerrainGrid();
	Control::setValue(XGridSnapCtrl, $ME::XGridSnap);
	Control::setValue(YGridSnapCtrl, $ME::YGridSnap);
	Control::setValue(ZGridSnapCtrl, $ME::ZGridSnap);
}

function ME::SnapRotations()
{
   Control::setActive(RotationSnapCtrl, $ME::SnapRotations);
}

// just toggle the value for this flag variable
function METogglePlaneMovement()
{
	if( $ME::UsePlaneMovement == "true" )
    {
    	$ME::UsePlaneMovement = false;
    }
    else
    {
    	$ME::UsePlaneMovement = true;
    }
    ME::GetConsoleOptions();
}

function MEHideOptions()
{
   if( Control::getVisible("OptionsCtrl") )
   {
      Control::setVisible("OptionsCtrl", false);
      ME::GetConsoleOptions();
   }

   if( Control::getVisible("TedOptionsCtrl") )
   {
      Control::setVisible("TedOptionsCtrl", false );
      Ted::GetConsoleOptions();
      Control::setText(SeedTerrain, "Gen: " @ $ME::terrainSeed);
   }
}

function MEShowOptions()
{
   if($ME::Loaded)
   {
      if( Control::getVisible("HelpCtrl"))
         return;
        
      if( Control::getVisible("MEObjectList") )
      {
         Control::setVisible("OptionsCtrl", true);
         Control::setValue( MEUsePlaneMovement, $ME::UsePlaneMovement );
         ME::SnapRotations();
         ME::SnapToGrid();
         return;
      }
      
      if( Control::getVisible("TedBar") )
      {
         Control::setVisible("TedOptionsCtrl", true );
         Control::setValue(TerrainSeedText, $ME::terrainSeed);
         return;
      }
   }
}

function METoggleOptions()
{
   if( Control::getVisible("MEObjectList" ) )
      %optionName = "OptionsCtrl";
   if( Control::getVisible("TedBar" ) )
      %optionName = "TedOptionsCtrl";
      
   if( Control::getVisible( %optionName ) )
   {
      MEHideOptions();
   }
   else
   {
      MEShowOptions();
   }
}

//-----------------------------------------------------

function MEShowHelp()
{
   if($ME::Loaded)
   {
      if(Control::getVisible("OptionsCtrl"))
         return;
      if(Control::getVisible("TedOptionsCtrl"))
         return;
      Control::setVisible("HelpCtrl", true );
   }
}

function MEHideHelp()
{
   if($ME::Loaded)
      Control::setVisible("HelpCtrl", false );
}

function METoggleHelp()
{
   if( Control::getVisible("HelpCtrl"))
   {
      MEHideHelp();
   }
   else
   {
      MEShowHelp();
   }
}

//-----------------------------------------------------


function MEShowCreator()
{
   if(!$ME::Loaded)
      return;
   
   MEHideOptions();
   MEHideHelp();
   
   unfocus(TedObject);
   focus(MissionEditor);
   focus(editCamera);
   postAction(EditorGui, Attach, editCamera);
   cursorOn(MainWindow);

   Control::setVisible("MEObjectList", true);
   Control::setVisible("Inspector", false);
   Control::setVisible("Creator", true);
   Control::setVisible("TedBar", false);
   Control::setVisible("SaveBar", true);
   Control::setVisible("LightingBox", false);
   Control::setVisible("AddVolume", false);

   TextList::clear("GroupList");
   TextList::clear("NameList");

   for(%i = 0; %i < $ME::NumGroups; %i++)
      TextList::AddLine("GroupList", $ME::Group[%i]);
}

function MEShowInspector()
{
   if(!$ME::Loaded)
      return;

   MEHideOptions();
   MEHideHelp();

   unfocus(TedObject);
   focus(MissionEditor);
   focus(editCamera);
   postAction(EditorGui, Attach, editCamera);
   cursorOn(MainWindow);

   Control::setVisible("MEObjectList", true);
   Control::setVisible("Inspector", true);
   Control::setVisible("Creator", false);
   Control::setVisible("TedBar", false);
   Control::setVisible("SaveBar", true);
   Control::setVisible("LightingBox", false);
   Control::setVisible("AddVolume", false);
}

function MEShowTed()
{
   if(!$ME::Loaded)
      return;

   MEHideOptions();
   MEHideHelp();
   
   Control::setVisible("TedBar", true);
   Control::setVisible("SaveBar", true);
   Control::setVisible("MEObjectList", false);
   Control::setVisible("Inspector", false);
   Control::setVisible("Creator", false);
   Control::setVisible("LightingBox", false);
   Control::setVisible("AddVolume", false);
   unfocus(MissionEditor);
   focus(TedObject);
   focus(editCamera);
   postAction(EditorGui, Attach, editCamera);
   cursorOn(MainWindow);

   // set the right button to nothing
   TED::setRButtonAction( "" );
    
   // set the terrain type popup
   Popup::clear(TerrainTypes);
   
   %i = Ted::getNumTerrainTypes();
   if( $TED::success == true )
      for( %j = 0; %j < %i; %j++ )
         Popup::addLine( TerrainTypes, Ted::getTerrainTypeName(%j), %j );
   Popup::setSelected(TerrainTypes, Ted::getTerrainType() );

   // set the detail level popup
   Popup::clear(BrushDetail);
   %i = Ted::getMaxBrushDetail();
   if( $TED::success == true )
      for( %j = 0; %j < %i; %j++ )
         Popup::addLine( BrushDetail, "Level " @ %j, %j );
   Popup::setSelected(BrushDetail, Ted::getBrushDetail() );

   // setup the material list popup
   Popup::clear(MaterialList);
   %i = Ted::getMaterialCount();
   if($TED::success == true)
      for(%j = 0; %j < %i; %j++)
         Popup::addLine(MaterialList, Ted::getMaterialName(%j), %j);
   Popup::setSelected(MaterialList, Ted::getMaterialIndex());
   
   // setup the buttons with default's if no pref
   MESetupTedButton( TEDModeOne, select, true );
   MESetupTedButton( TEDModeTwo, deselect, true );

   // setup the terrain generation stuff
   Control::setText(SeedTerrain, "Gen: " @ $ME::terrainSeed);

   // get the terrain name (obj 8)
   focusServer();
   %typeName = 8.terrainType;
   focusClient();
   
   Popup::clear(Terrains);
   %select = 0;
   for(%i = 0; $terrainTypes[%i, type] != ""; %i++)
   {
      if($terrainTypes[%i, type] == %typeName)
         %select = %i;
      Popup::addLine(Terrains, $terrainTypes[%i, description], %i);
   }
   Popup::setSelected(Terrains, %select);

   // select the first button
   PopupButton::makeCurrent(TEDModeOne);
   TEDModeOne::onPressed();
      
   MESetupTedButton( TEDProcessAction, "" );
   Control::setText( TEDProcessAction, " -- Selection Action --" );
}

function MESetupTedButton( %button, %func, %usePref )
{   	
   if( %usePref == "true" )
   {
      if( $pref::[%button] == "" )
         $pref::[%button] = %func;
      else
         %func = $pref::[%button];
   }
   else if( %button != "TEDProcessAction" )
   {
      $pref::[%button] = %func;
   }
   
   // set the ted mouse buttons..
   Popup::clear(%button);
   %selItem = 0;
   %i = Ted::getNumActions();
   if( $TED::success == true )
   {
      for( %j = 0; %j < %i; %j++ )
      {
         if( Ted::isActionMouseable( %j ) )
         {
            Popup::addLine( %button, Ted::getActionName(%j), %j );
            if( Ted::getActionName(%j) == %func )
               %selItem = %j;
         }
      }
   }
   Popup::setSelected(%button, %selItem );
}

function MEMode()
{
   $ME::InspectObject = -1;
   if($ME::loaded != true)
      ME::init();
   
   if(!isObject(editCamera))
      return;

   $Server::timeLimit = 0;
   if(!isObject(EditorGui))
      GuiLoadContentCtrl("MainWindow", "gui\\editor.gui");

   if(!$ME::Loaded)
      return;

   MEHideOptions();
   MEHideHelp();
   
   MissionObjectList::ClearDisplayGroups();
   MissionObjectList::AddDisplayGroup(1, "MissionGroup");
   MissionObjectList::AddDisplayGroup(1, "MissionCleanup");
   MissionObjectList::SetSelMode(1);
   MEShowInspector();
   ME::GetConsoleOptions();
}

function MEGameMode(%doRebuild)
{
   if(%doRebuild != false)
      ME::RebuildCommandMap();
   unfocus(TedObject);
   unfocus(MissionEditor);
   unfocus(editCamera);
   GuiLoadContentCtrl(MainWindow, "gui\\play.gui");
   cursorOff(MainWindow);
   focus(playDelegate);
}

function MEHide()
{
   if(!isObject(EditorGui))
      GuiLoadContentCtrl("MainWindow", "gui\\editor.gui");

   focus(editCamera);
   postAction(EditorGui, Attach, editCamera);

   Control::setVisible("TedBar", false);
   Control::setVisible("MEObjectList", false);
   Control::setVisible("Inspector", false);
   Control::setVisible("Creator", false);
   Control::setVisible("SaveBar", false);
   Control::setVisible("LightingBox", false);
   MEHideOptions();
   MEHideHelp();
}

function MELSEdit()
{
   LS::Editor( MainWindow );
   flushTextureCache();
}

//--------------------------------------------------------------
// generic callback functions to pass along to current control

function MEDeleteSelection()
{
   if( Control::getVisible("MEObjectList") )
      ME::DeleteSelection();
}

function MECopySelection()
{
   if( Control::getVisible("TedBar") )
      Ted::copy();
   if( Control::getVisible("MEObjectList") )
      ME::CopySelection();
}

function MECutSelection()
{
   if( Control::getVisible("MEObjectList") )
      ME::CutSelection();
}

function MEPasteSelection()
{
   if( Control::getVisible("TedBar") )
      Ted::floatPaste();
   if( Control::getVisible("MEObjectList") )
      ME::PasteSelection();
}

function MEDropSelection()
{
   if(Control::getVisible("MEObjectList"))
      ME::DropSelection();
}

function MEDuplicateSelection()
{
   if( Control::getVisible("TedBar") )
      Ted::floatSelection();
   if( Control::getVisible("MEObjectList") )
      ME::DuplicateSelection();
}

function MEUndo()
{
   if( Control::getVisible("TedBar") )
      Ted::undo();
   if( Control::getVisible("MEObjectList") )
      ME::Undo();
}

function MERedo()
{
   if( Control::getVisible("TedBar") )
      Ted::redo();
   if( Control::getVisible("MEObjectList") )
      ME::Redo();
}

//--------------------------------------------------------------
// the popup buttons - right click brings up popup with functions

function TEDModeOne::onPressed()
{
   Ted::setLButtonAction( Ted::getActionName( Popup::getSelected( TEDModeOne ) ) );
   Ted::setRButtonAction( "" );
}
function TEDModeOne::onSelect( %string, %level )
{
   MESetupTedButton( TEDModeOne, %string );
   Ted::setLButtonAction( %string );
   Ted::setRButtonAction( "" );
}

function TEDModeTwo::onPressed()
{
   Ted::setLButtonAction( Ted::getActionName( Popup::getSelected( TEDModeTwo ) ) );
   Ted::setRButtonAction( "" );
}
function TEDModeTwo::onSelect( %string, %level )
{
   MESetupTedButton( TEDModeTwo, %string );
   Ted::setLButtonAction( %string );
   Ted::setRButtonAction( "" );
}

function Terrains::onSelect( %string, %level )
{
   //
}

//--------------------------------------------------------------

function SeedTerrain::onAction()
{
   // get the terrain base and initialize
   %world = Ted::getWorldName();
   exec(%world @ ".terrain.cs");
   
   // set the rules
   eval("Terrain::" @ %world @ "::setRules();");

   // process the terrain commands for this type
   LS::flushCommands();
   
   // get the terrain type
   %tt = Popup::getSelected(Terrains);
   if($terrainTypes[%tt, type] == "")
      return;

   %ts = 1 << (Ted::getMaxBrushDetail() - 1);
   eval(Terrain:: @ $terrainTypes[%tt, type] @ "::create(" @ %ts @ "," @ $ME::terrainSeed @ ");");
   LS::parseCommands();
   
   LS::ApplyLandscape();
   LS::ApplyTextures();

   // get a new seed number
   if($ME::genRandSeed == true)
      $ME::terrainSeed = floor(getRandom() * 65535);

   Control::setText(SeedTerrain, "Gen: " @ $ME::terrainSeed);
}

//--------------------------------------------------------------

function TEDProcessAction::onSelect( %string, %level )
{
   Ted::processAction( Ted::getActionName( %level ) );
   Control::setText( TEDProcessAction, " -- Selection Action --" );
}

//--------------------------------------------------------------

function TerrainTypes::onSelect(%string, %level)
{								
	TED::setTerrainType( %level );
}

function BrushDetail::onSelect(%string, %level)
{
   Ted::setBrushDetail(%level);
}

function MaterialList::onSelect(%string, %level)
{
   Ted::setMatIndexVal(%level);
}

//--------------------------------------------------------------

function ME::ReloadMission(%mission)
{
   disconnect();
   deleteServer();
   deleteObject(ConsoleScheduler);
   newObject(ConsoleScheduler, SimConsoleScheduler);
   purgeResources();
   purgeResources();
   if(%mission == "")
      %mission = $pref::lastMission;
   schedule("createServer(" @ %mission @ ", false);", 3);
}

//--------------------------------------------------------------

function ME::init()
{
   // set the vars once
   if($ME::varsInitialized != true)
   {   
      $ME::varsInitialized = true;
   
      $ME::ShowEditObjects = true;
      $ME::ShowGrabHandles = true;
      $ME::SnapToGrid = false;

      $ME::XGridSnap = 1;
      $ME::YGridSnap = 1;
      $ME::ZGridSnap = 0.001;

      $ME::ConstrainX = false;
      $ME::ConstrainY = false;
      $ME::ConstrainZ = false;

      $ME::RotateXAxis = false;
      $ME::RotateYAxis = false;
      $ME::RotateZAxis = true;
      $ME::RotationSnap = 90.0;
      $ME::SnapRotations = false;

      $ME::DropAtCamera = false;
      $ME::DropWithRotAtCamera = false;
      $ME::DropBelowCamera = false;
      $ME::DropToScreenCenter = true;
      $ME::DropToSelectedObject = false;

      $ME::ObjectsSnapToTerrain = false;
      $ME::UsePlaneMovement = false;

      $ME::LightQuick = false;
      $ME::terrainSeed = 0;
      $ME::genRandSeed = true;
      $ME::newVolFile = "";
   }
      
   focusClient();
   if($ME::Loaded == false)
   {
      %obj = newObject("Prefabs", SimVolume, "prefabs.vol");
      addToSet("MissionCleanup", %obj);
      
      $Server::timeLimit = 0;
      $ME::Loaded = true;
      exec("move.cs" );
      ME::Move();
      exec("med.cs");
      exec("ted.cs");
      exec("terrains.cs");
      exec("RegisterObjects.cs");

      // execute the user's stuff - do here to allow overriding of ted/med variables
      exec("editorconfig.cs");
      ted();
      MEMode();
   }
}

//--------------------------------------------------------------

if(focusServer())
   ME::init();
