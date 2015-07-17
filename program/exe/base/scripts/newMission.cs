//---------------------------------------------------------------------

function NewMission::setValues()
{
   // fill in 'Number of Teams'   
   Popup::clear(NumTeams);
   for(%i = $missionTypes[$NM::missionIndex, minTeams];
      %i <= $missionTypes[$NM::missionIndex, maxTeams]; %i++)
      Popup::addLine(NumTeams, %i @ " teams", %i);
   Popup::setSelected(NumTeams, $NM::numTeams);
      
   // do 'missiontype'
   Popup::clear(MissionType);
   for(%i = 0; $missionTypes[%i, type] != ""; %i++)
      Popup::addLine(MissionType, $missionTypes[%i, description], %i);
   Popup::setSelected(MissionType, $NM::missionIndex);

   // do 'WorldType'
   Popup::clear(WorldType);
   for(%i = 0; $worldTypes[%i, type] != ""; %i++)
      Popup::addLine(WorldType, $worldTypes[%i, type], %i);
   Popup::setSelected(WorldType, $NM::worldIndex);
   
   // do 'skytype'
   Popup::clear(SkyType);
   %wi = $NM::worldIndex;
   for(%i = 0; $worldTypes[%wi, skyname, %i] != ""; %i++)
      Popup::addLine(SkyType, $worldTypes[%wi, skyname, %i], %i);
   Popup::setSelected(SkyType, $NM::skyIndex);
   
   // do 'terraintype'
   Popup::clear(TerrainType);
   for(%i = 0; $terrainTypes[%i, type] != ""; %i++)
      Popup::addLine(TerrainType, $terrainTypes[%i, description], %i);
   Popup::setSelected(TerrainType, $NM::terrainIndex);

   // do 'terrainsize'
   Popup::clear(TerrainSize);
   for(%i = 0; $terrainSizes[%i, name] != ""; %i++)
      Popup::addLine(TerrainSize, $terrainSizes[%i, name], %i);
   Popup::setSelected(TerrainSize, $NM::terrainSizeIndex);
   
   Control::setValue(MissionName, $NM::MissionName);
   
   // setup the variables
   %mi = $NM::missionIndex;
   Popup::clear(varSelect);
   for(%i = 0; $missionTypes[%mi, varName, %i] != ""; %i++)
      Popup::addLine(varSelect, $missionTypes[%mi, varDesc, %i], %i);
   Popup::setSelected(varSelect, $NM::varIndex);
      
   eval("%i = $" @ $missionTypes[%mi, type] @ "::" @ $missionTypes[%mi, varName, $NM::varIndex] @ ";" );
   Control::setValue(varValue, %i);
}

//---------------------------------------------------------------------
// callbacks

function NumTeams::onSelect(%string, %level)
{								
   $NM::numTeams = %level;
   NewMission::setValues();
}

function MissionType::onSelect(%string, %level)
{		
   if(%level != $NM::missionIndex)
   {
      // check if need to select a new number of teams
      if(($NM::numTeams < $missionTypes[%level, minTeams]) ||
         ($NM::numTeams > $missionTypes[%level, maxTeams]))
         $NM::numTeams = $missionTypes[%level, minTeams];
         
      $NM::missionIndex = %level;
      $NM::varIndex = 0;
   }					
   NewMission::setValues();
}

function WorldType::onSelect(%string, %level)
{
   if($NM::worldIndex != %level)
   {
      $NM::worldIndex = %level;
      $NM::skyIndex = 0;
   }
   
   NewMission::setValues();
}

function SkyType::onSelect(%string, %level)
{
   $NM::skyIndex = %level;
   NewMission::setValues();
}

function TerrainType::onSelect(%string, %level)
{
   $NM::terrainIndex = %level;
   NewMission::setValues();
}

function TerrainSize::onSelect(%string, %level)
{
   $NM::terrainSizeIndex = %level;
   NewMission::setValues();
}

function varSelect::onSelect(%string, %level)
{
   $NM::varIndex = %level;
   NewMission::setValues();
}

function varValue::onAction()
{
   %val = Control::getValue(varValue);
   %mi = $NM::missionIndex;
   eval("$" @ $missionTypes[%mi, type] @ "::" @ $missionTypes[%mi, varName, $NM::varIndex] @ " = " @ %val @ ";" );
}

//-----------------------------------------------------------------------

function NewMission::create()
{
   focusClient();
   disconnect();
   
   NewMission::createServer();

   %mi = $NM::missionIndex;
   %si = $NM::skyIndex;
   %wi = $NM::worldIndex;
   %ti = $NM::terrainIndex;
   %missionName = String::convertSpaces($NM::missionName);
   %worldName = $worldTypes[%wi, type];
   %timeOfDay = $worldTypes[%wi, time, %si];
   %worldVolName = strcat(%worldName, "World", ".vol");
   %terrVolName = strcat(%worldName, "Terrain", ".vol");
   %terrFileName = strcat(%missionName, ".dtf");
   %dmlName = strcat(%worldName, ".dml");
   %palName = strcat(%worldName, ".", %timeOfDay, ".ppl");
   %tedFileName = strcat(%missionName, ".ted");
   %dscName = "base\\missions\\" @ %missionName @ ".dsc";
   %missionName = strcat("base\\missions\\", %missionName, ".mis");
   %rulesName = strcat(%worldName, ".rules.dat");
   %gridName = strcat(%worldName, ".grid.dat");

   // Default groups
   newObject("MissionGroup", SimGroup);
   newObject("Volumes", SimGroup);
   newObject("World", SimGroup);
   newObject("Landscape", SimGroup);
   newObject("Lights", SimGroup);
   addToSet("MissionGroup", "Volumes", "World", "Landscape", "Lights" );

   // Default volumes
   newObject("World", SimVolume , %worldVolName);
   newObject("WorldTerrain", SimVolume , %terrVolName);
   newObject("InterfaceVol", SimVolume, "interface.vol");
   newObject("Entities", SimVolume, "entities.vol");
   addToSet("MissionGroup\\Volumes", "InterfaceVol", "Entities", "World", "WorldTerrain");

   // Register the world's DML volumes (Contained in %terrVolName)
   exec("createWorldVolumes.cs");

   // World palette
   newObject("Palette", SimPalette, %palName, true);
   addToSet("MissionGroup\\World", "Palette");

   // add the sky
   %skydmlName = $worldTypes[%wi, skydml, %si];
   if(%skydmlname == "starfield")
   {
      newObject(Stars, StarField);
      addToSet("MissionGroup\\Landscape", Stars);
      %skydmlName = "";
   }
      
   newObject(Sky, Sky, 0, 0, 0, %skydmlName, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
   addToSet("MissionGroup\\Landscape", Sky);

   // Default sun light
   if(%timeOfDay == Night)
      newObject("Sun", Planet, 0, 0, 60, T, F, 0.4, 0.4, 0.4, 0.15, 0.15, 0.15);
   else 
      newObject("Sun", Planet, 0, 0, 60, T, F, 0.7, 0.7, 0.7, 0.3, 0.3, 0.3);
   addToSet("MissionGroup\\Landscape", "Sun");

   // create the teams
   newObject("Teams", SimGroup);
   addToSet(MissionGroup, Teams);
   for(%i = 0; %i < $NM::numTeams; %i++)
   {
      newObject("Team" @ %i, TeamGroup);
      addToSet("MissionGroup\\Teams", "Team" @ %i);

      // Default team setup      
      newObject("DropPoints", SimGroup);
	   newObject("Start", SimGroup);
	   newObject("Random", SimGroup);
	   addToSet("DropPoints", "Start", "Random");
	   addToSet("MissionGroup\\Teams\\team" @ %i, DropPoints);
   }

   // create the mission....
   flushExportText();
   eval(Mission:: @ $missionTypes[$NM::missionIndex, type] @ "::create($NM::numTeams);");
   eval( "%i = $" @ $missionTypes[$NM::missionIndex, type] @ "::" @ "cdTrack;");
   addExportText("$cdTrack = " @ %i @ ";");
   eval( "%i = $" @ $missionTypes[$NM::missionIndex, type] @ "::" @ "cdMode;");
   addExportText("$cdPlayMode = " @ %i @ ";");

   // get the size
   %ts = $terrainSizes[$NM::terrainSizeIndex, size];

   // ground scale (1 << n ) meters per block
   %gs = 3;

   // width in meters
   %tw = %ts * (1 << %gs);

   // create the simterrain obj
   %terrain = newObject("Terrain", SimTerrain, Create, %terrFileName, 3, %gs, %ts, 0, 0, -(3 * %tw / 2), -(3 * %tw / 2), 0, 0, 0, 0);
	newObject("MissionCenter", MissionCenterPos, -(%tw / 2), -(%tw / 2), %tw, %tw);

   // set the detail (perspective distance, screensize)   
   setTerrainDetail(Terrain, 100, $terrainTypes[%ti, screenSize]);
   setTerrainVisibility(Terrain, $terrainTypes[%ti, visDistance], $terrainTypes[%ti, hazeDistance]);
   SetTerrainContainer(Terrain,"0 0 -20.0",0,10000);  //  (Terrain,Gravity,Drag,Height)

   // apply the terrains
   LS::Create(1);
   LS::Textures(%dmlName, %gridName);

   // execute the terrain file and apply the rules
   exec(%worldname @ ".terrain.cs");
   eval(Terrain:: @ %worldname @ "::setRules();");
   
   // process the terrain commands for this type
   LS::flushCommands();
   eval(Terrain:: @ $terrainTypes[%ti, type] @ "::create(" @ %ts @ ");");
   %terrain.terrainType = $terrainTypes[%ti, type];
   LS::parseCommands();
   
   LS::ApplyLandscape();
   LS::ApplyTextures();

   // save off the terrain   
   saveTerrain(Terrain, "base\\missions\\" @ %tedFileName);
   
   // reevaluate the path
   $ConsoleWorld::DefaultSearchPath = $ConsoleWorld::DefaultSearchPath;
   
   newObject("TedFile", SimVolume, "missions\\" @ %tedFileName);
   addToSet("MissionGroup\\Volumes", "TedFile");
   addToSet("MissionGroup\\Landscape", "Terrain");
   addToSet("MissionGroup\\World", "MissionCenter");

   // fill out the mission description
   $MDESC::Type         = $missionTypes[%mi, description];
   $MDESC::Text         = "<s3><f1>Mission Type: <f0> " @ $MDESC::Type @
      "\n<f1>Planet: <f0> UberPrime" @
      "\n<f1>Weather: <f0> Not Too Bad" @
      "\n<f1>Time of Day: <f0> " @ %timeOfDay @
      "\n<f1># of Teams: <f0> " @ $NM::numTeams @
      "\n\n<f1>Briefing:<f0> The greater the difficulty, the more the glory in surmounting it.";
   export("$MDESC::*", %dscName);

   // save the mission - toss the old stuff
	exportObjectToScript(MissionGroup, %missionName, true);
   flushExportText();
   
   // reevaluate the path
   $ConsoleWorld::DefaultSearchPath = $ConsoleWorld::DefaultSearchPath;
   MissionList::build();
   
   NewMission::exit($NM::missionName);
}

//-----------------------------------------------------------------------

function NewMission::exit(%mission)
{
   focusClient();

   // get rid of the dialog and do mouse things
   if(!$NM::cursorOn)
      cursorOff(MainWindow);
   
   // check if hit cancel
   if(%mission != "")
      schedule("NewMission::doCreate(\"" @ %mission @ "\");", 1); 
   else
      GuiPopDialog(MainWindow, 0);
}

//-----------------------------------------------------------------------
// this needs to be scheduled because the create is called through
// a buttons onaction (the button gets destroyed by this function)
function NewMission::doCreate(%mission)
{
   $ConnectedToServer = false;
   deleteServer();
   purgeResources();
   deleteObject(ConsoleScheduler);
   newObject(ConsoleScheduler, SimConsoleScheduler);
   exec(editor);  // pre-load the editor
   createServer(%mission, false);
}

//-----------------------------------------------------------------------

function NewMission::reset()
{
   // initialize all the tables...
   exec(missiontypes);
   exec(worlds);
   exec(terrains);

   // set the defaults 'NM for NewMission'
   if($NewMissionName != "")
   {
      $NM::missionName = $NewMissionName;
      $NewMissionName = "";
   }
   else
      $NM::missionName        = "NewMission";
   $NM::missionIndex       = 0;
   $NM::numTeams           = $missionTypes[$NM::missionIndex, minTeams];
   $NM::worldIndex         = 0;
   $NM::skyIndex           = 0;
   $NM::terrainIndex       = 0;
   $NM::terrainSizeIndex   = 0;
   $NM::terrainSeed        = 0;
   $NM::varIndex           = 0;

   NewMission::setValues();

   // get the mouse setup
   $NM::cursorOn = isCursorOn(MainWindow);
   if(!$NM::cursorOn)
      cursorOn(MainWindow);
}

//-----------------------------------------------------------------------
// initialization

GuiPushDialog(MainWindow, "gui\\newmission.gui");
NewMission::reset();
   
// end initialization
//-----------------------------------------------------------------------

// grabbed from server.cs
function NewMission::createServer()
{
   deleteServer();
   purgeResources();
   newServer();
   focusServer();

   newObject(serverDelegate, FearCSDelegate, true, "LOOPBACK", $Server::Port);

   exec(admin);
   exec(Marker);
   exec(Trigger);
   exec(NSound);
   exec(BaseExpData);
   exec(BaseDebrisData);
   exec(BaseProjData);
   exec(ArmorData);
   exec(Mission);
   exec(Item);
   exec(Player);
   exec(Vehicle);
   exec(Turret);
   exec(Beacon);
   exec(StaticShape);
   exec(Station);
   exec(Moveable);
   exec(Sensor);
   exec(Mine);
   exec(AI);
   exec(InteriorLight);
   preloadServerDataBlocks();
}
