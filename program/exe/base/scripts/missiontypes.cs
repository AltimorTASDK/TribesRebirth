// -------------------------------------------------
// declare the mission types here - 'cdTrack' is needed for
// all mission types
// -------------------------------------------------
$missionTypes[0, type]        = "CandH";
$missionTypes[0, description] = "Capture and Hold";
$missionTypes[0, minTeams]    = 2;
$missionTypes[0, maxTeams]    = 8;
$missionTypes[0, varName, 0]  = numObjectives;
$missionTypes[0, varDesc, 0]  = "Number of objectives";
$missionTypes[0, varVal, 0]   = 1;
$missionTypes[0, varName, 1]  = scoreLimit;
$missionTypes[0, varDesc, 1]  = "Score limit";
$missionTypes[0, varVal, 1]   = 100;
$missionTypes[0, varName, 2]  = cdTrack;
$missionTypes[0, varDesc, 2]  = "CD Track";
$missionTypes[0, varVal, 2]   = 2;
$missionTypes[0, varName, 3]  = cdMode;
$missionTypes[0, varDesc, 3]  = "CD Play Mode";
$missionTypes[0, varVal, 3]   = 1;
 
$missionTypes[1, type]        = "CTF";
$missionTypes[1, description] = "Capture the Flag";
$missionTypes[1, minTeams]    = 2;
$missionTypes[1, maxTeams]    = 8;
$missionTypes[1, varName, 0]  = winCaps;
$missionTypes[1, varDesc, 0]  = "Flag caps to win";
$missionTypes[1, varVal, 0]   = 5;
$missionTypes[1, varName, 1]  = cdTrack;
$missionTypes[1, varDesc, 1]  = "CD Track";
$missionTypes[1, varVal, 1]   = 2;
$missionTypes[1, varName, 2]  = cdMode;
$missionTypes[1, varDesc, 2]  = "CD Play Mode";
$missionTypes[1, varVal, 2]   = 1;

$missionTypes[2, type]        = "DM";
$missionTypes[2, description] = "Death match";
$missionTypes[2, minTeams]    = 1;
$missionTypes[2, maxTeams]    = 1;
$missionTypes[2, varName, 0]  = fragLimit;
$missionTypes[2, varDesc, 0]  = "Frag limit";
$missionTypes[2, varVal, 0]   = 40;
$missionTypes[2, varName, 1]  = cdTrack;
$missionTypes[2, varDesc, 1]  = "CD Track";
$missionTypes[2, varVal, 1]   = 2;
$missionTypes[2, varName, 2]  = cdMode;
$missionTypes[2, varDesc, 2]  = "CD Play Mode";
$missionTypes[2, varVal, 2]   = 1;

$missionTypes[3, type]        = "DandD";
$missionTypes[3, description] = "Defend and Destroy";
$missionTypes[3, minTeams]    = 2;
$missionTypes[3, maxTeams]    = 8;
$missionTypes[3, varName, 0]  = scoreLimit;
$missionTypes[3, varDesc, 0]  = "Score limit";
$missionTypes[3, varVal, 0]   = 20;
$missionTypes[3, varName, 1]  = cdTrack;
$missionTypes[3, varDesc, 1]  = "CD Track";
$missionTypes[3, varVal, 1]   = 2;
$missionTypes[3, varName, 2]  = cdMode;
$missionTypes[3, varDesc, 2]  = "CD Play Mode";
$missionTypes[3, varVal, 2]   = 1;

$missionTypes[4, type]        = "FandR";
$missionTypes[4, description] = "Find and Retrieve";
$missionTypes[4, minTeams]    = 2;
$missionTypes[4, maxTeams]    = 8;
$missionTypes[4, varName, 0]  = numFlags;
$missionTypes[4, varDesc, 0]  = "Number of flags";
$missionTypes[4, varVal, 0]   = 1;
$missionTypes[4, varName, 1]  = cdTrack;
$missionTypes[4, varDesc, 1]  = "CD Track";
$missionTypes[4, varVal, 1]   = 2;
$missionTypes[4, varName, 2]  = cdMode;
$missionTypes[4, varDesc, 2]  = "CD Play Mode";
$missionTypes[4, varVal, 2]   = 1;

// -------------------------------------------------
// create all the variables needed
// -------------------------------------------------

for(%i = 0; $missionTypes[%i, type] != ""; %i++)
   for(%j = 0; $missionTypes[%i, varName, %j] != ""; %j++)
      eval("$" @ $missionTypes[%i, type] @ "::" @ $missionTypes[%i, varName, %j] @ "=" @ $missionTypes[%i, varVal, %j] @ ";");

// -------------------------------------------------
// create functions - add stuff to the mission
// -------------------------------------------------

function Mission::CandH::create(%numTeams)
{
   // add in the objectives
   for(%i = 0; %i < $CandH::numObjectives; %i++)
   {
      %tower = newObject("Tower" @ %i, SimGroup);
      addToSet("MissionGroup", %tower);
      %switch = newObject("Switch" @ %i, StaticShape, TowerSwitch);
      %switch.scoreValue = 12;
      GameBase::setMapName(%switch, "Switch " @ (%i + 1));
      addToSet(%tower, %switch);
   }
  
   // add lines to the mission file...
   addExportText("$teamScoreLimit = " @ $CandH::scoreLimit  @ ";");
   addExportText("exec(objectives);");
   addExportText("$Game::missionType = \"C&H\";");
}

function Mission::CTF::create(%numTeams)
{
   // add a flag
   for(%i = 0; %i < %numTeams; %i++)
   {
      // add a 'base' group for the flag
      %base = newObject(Base, SimGroup);
      addToSet("MissionGroup\\Teams\\team" @ %i, %base);

      // add the flag
      %flag = newObject(Flag @ %i, Item, Flag, 1, false);
      %flag.scoreValue = 1;
      GameBase::setMapName(%flag, "Flag " @ (%i + 1));
      addToSet(%base, %flag);
   }
   
   // add lines to the mission file...
   addExportText("$teamScoreLimit = " @ $CTF::winCaps @ ";");
   addExportText("exec(objectives);");
   addExportText("$Game::missionType = \"CTF\";");
}

function Mission::DM::create(%numTeams)
{
   // add lines to the mission file...
   addExportText("$DMScoreLimit = " @ $DM::fragLimit @ ";");
   addExportText("exec(dm);");
   addExportText("$Game::missionType = \"DM\";");
}

function Mission::DandD::create(%numTeams)
{
   // add lines to the mission file...
   addExportText("$teamScoreLimit = " @ $DandD::scoreLimit @ ";");
   addExportText("exec(objectives);");
   addExportText("$Game::missionType = \"D&D\";");
}

function Mission::FandR::create(%numTeams)
{
   // add flagstands for the teams
   for(%i = 0; %i < %numTeams; %i++)
   {
      // add a 'base' group for the stands
      %base = newObject(Base, SimGroup);
      addToSet("Missiongroup\\Teams\\team" @ %i, %base);
         
      // add all the stands
      for(%j = 0; %j < $FandR::numFlags; %j++)
      {
         %stand = newObject("Stand" @ %j, StaticShape, FlagStand);
         GameBase::setMapName(%stand, "Stand " @ (%j + 1));
         addToSet(%base, %stand);
      }
   }

   // add the flags   
   for(%i = 0; %i < $FandR::numFlags; %i++)
   {
      %flag = newObject(Flag @ %i, Item, Flag, 1, false);
      %flag.scoreValue = 2;
      GameBase::setMapName(%flag, "Flag " @ (%i + 1));
      addToSet(MissionGroup, %flag);
   }
   
   // add lines to the mission file...
   addExportText("$teamScoreLimit = " @ ($FandR::numFlags * 2) @ ";");
   addExportText("exec(objectives);");
   addExportText("$Game::missionType = \"F&R\";");
}
