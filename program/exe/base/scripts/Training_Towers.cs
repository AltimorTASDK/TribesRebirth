//Towers Training script file
//-----------------------------
//Handles all automations for towers training mission

exec("game.cs");
exec("Training_AI.cs");
$enemyTeam[0] = 1;
$enemyTeam[1] = 0;
$missionOver = false;
$numCaps = 0;
$AIkilled = 0;
$Train::missionType = "C&H";

function TowersTraining::init()
{       
  $trainingType = 0;
  $currentWay = 1;
  $numSwitches = 0;     
  $numToComplete = 0;
  
   
  for(%i = 1; (%group = nameToID("MissionGroup\\Towers\\Tower" @ %i)) != -1; %i++)
  {    
    
    %numObj = Group::objectCount(%group);
    for(%j = 0; %j < %numObj; %j++)
    { 
      %tempObj = Group::getObject(%group, %j);
      GameBase::setTeam(%tempObj, 1);
      if(GameBase::getDataName(%tempObj) == TowerSwitch)
      {
        
        $SwitchObject[%i] = %tempObj;
		$SwitchObject[%i].objectiveComplete = false;
        $SwitchPosition[%i] = GameBase::getPosition($SwitchObject[%i]);
		$TowerControlled[%i] = "false";
		$numSwitches++;
		$numToComplete++;
      }
	}
  }
  %startGroup =  nameToID("MissionGroup\\Teams\\Team0\\DropPoints\\Start");
  $startPoint = Group::getObject(%startGroup, 0);
  
  for(%k = 1; %k <= 3; %k++)
  {
    %group = nameToID("MissionGroup\\Towers\\Tower" @ %k @ "\\DropPoints");
    $spawnPoint[%k] = Group::getObject(%group, 0);
	
  }
  towers::teamMissionObjectives(0, false);
}


function TowerSwitch::onCollision(%this, %object)
{
   $numCaps++;
   if(getObjectType(%object) != "Player")
      return;

   if(Player::isDead(%object))
      return;

   if(Player::getClient(%object) != Client::getFirst())
      return;

   %playerTeam = GameBase::getTeam(%object);
   %otherTeam = GameBase::getTeam(%this);
   
   if(%otherTeam == %playerTeam)
      return;
   
   for(%k = 1; %k <= $numSwitches; %k++)
   {
     if($SwitchObject[%k] == %this)
     { 
        %switchNum = %k;
     }
   }    
   if(%playerTeam == 0 )
   {
     if($SwitchObject[$currentWay] != %this)
     {	  
     	TowersTraining::wrongObjective(%this, %object);
     	return;
     }
   
     if($currentWay == 1)
	   $towerTime1 =  getSimTime();
	 else if($currentWay == 2)
	   $towerTime2 = getSimTime();
	 else
	   $towerTime3 = getSimTime();
     
     %this.trainingObjectiveComplete = true;
     %playerClient = Player::getClient(%object);
     %touchClientName = Client::getName(%playerClient);
   	 %group = "MissionGroup\\Towers\\tower" @ $currentWay;
     
     //set all objects to players team.
     for(%i = 0; (%obj = Group::getObject(%group, %i)) != -1; %i++)
     {  
       GameBase::setTeam(%obj, %playerTeam);
     }  
	 messageAll(0, "~wCapturedTower.wav");
	 
	 if($numToComplete == 1)
	   %time = 0;
	 else 
	   %time = 5;
	 
	 schedule("bottomprint(" @ %playerClient @ ", \"<f1><jc>Objective " @ $currentWay @ " has been accomplished!\", 5);", %time);
     ObjectiveMission::objectiveChanged(%this, false);
	 towers::teamMissionObjectives(%switchNum, false);
	     
   }
   else if(%playerTeam == 1)
   {
     bottomprint(%playerClient, "<f1><jc>The enemy has taken one of your towers!", 5);
     schedule("bottomprint(" @ %playerClient @ ", \"<f1><jc>You must go back and claim the tower before you can win the mission!\", 5);", 5);
	 messageAll(0, "~wCapturedTower.wav");
	 
	 //set all objects to opposite team , Ai's Team.
     %group = nameToId("MissionGroup\\Towers\\tower" @ %switchNum);
     for(%i = 0; (%obj = Group::getObject(%group, %i)) != -1; %i++)
     {  
       
       GameBase::setTeam(%obj, %playerTeam);
     }  
      
     ObjectiveMission::objectiveChanged(%this, true);
	 towers::teamMissionObjectives(%switchNum, true);
   }
}

//bottomprints to all players on teamId
function Teambottomprint(%teamId, %string)
{
  for(%i = 0; %i < getNumClients(); %i++)
  {
    %player = Client::getFirst();
    if(GameBase::getTeam(%player) == %teamId)
	{ 
      bottomprint(%player, %string, 5);
    }
    %player = Client::getNext(%player);
  }
}

function remoteScoresOn(%clientId)
{
}

function remoteScoresOff(%clientId)
{
}
 
function Game::playerSpawn(%clientId, %respawn)
{
   if($currentWay == 1)
     %spawnMarker = $startPoint;
   else
     %spawnMarker = $spawnPoint[$currentWay - 1]; 
   
   %clientId.guiLock = "";
   if(%spawnMarker == -1)
   {
      %spawnPos = "0 0 300";
      %spawnRot = "0 0 0";
   }
   else
   {
      %spawnPos = GameBase::getPosition(%spawnMarker);
      %spawnRot = GameBase::getRotation(%spawnMarker);
   }

	if(!String::ICompare(Client::getGender(%clientId), "Male"))
      %armor = "larmor";
   else
      %armor = "lfemale";

   %pl = spawnPlayer(%armor, %spawnPos, %spawnRot);
    
   GameBase::setTeam(%pl, Client::getTeam(%clientId));                                          
   Client::setOwnedObject(%clientId, %pl);                                                      
   Game::playerSpawned(%pl, %clientId, %armor, %respawn);                                       
   Client::setControlObject(%clientId, %pl);                                                 
}

function Towers::teamMissionObjectives(%towNum, %taken)
{
   //check which to replace
   if(%taken)
     %string = "Tower " @ %towNum @ " has not been captured.";
   else
     %string = "Tower " @ %towNum @ " has been captured.";
   
   if($missionOver)
   {
     Training::MissionComplete( 2049 );
	 return;
   }  
   
   //title screen
   Team::setObjective(0, 1, "<f5><jl>GamePlay Description:");
   Team::setObjective(0, 2, "<f1><jl>    The Objective of a Capture and Hold mission are to find and to take control");  
   Team::setObjective(0, 3, "<f1><jl>    of the objects placed in a mission. Once taken, any equipment associated with");
   Team::setObjective(0, 4, "<f1><jl>    the object, such as turrets, stations, ect., come under your control as well.");
   Team::setObjective(0, 5, "<f1><jl>    the longer you hold a location, the more points you recieve. In this training");
   Team::setObjective(0, 6, "<f1><jl>    mission, all you will have to do is take control of all 3 towers.");
   Team::setObjective(0, 7, "\n");
   Team::setObjective(0, 8, "<f5><jl>Mission Completion:");
   Team::setObjective(0, 9, "<f1>   -To own all 3 towers.");
   Team::setObjective(0, 10, "\n");
   Team::setObjective(0, 11, "<f5><jl>Mission Information:");
   Team::setObjective(0, 12, "<f1>   -Mission Name: Capture and Hold Training.");
   Team::setObjective(0, 13, "\n");
   Team::setObjective(0, 14, "<f5><jl>Mission Objectives:");
   
   //replace string
   if(%towNum == 1)
   {  
	  Team::setObjective(0, 15, "<f1><Btower_teamcontrol.bmp>\n   " @ %string);
   }
   else if(%towNum == 2) 
   {
      Team::setObjective(0, 16, "<f1><Btower_teamcontrol.bmp>\n   " @ %string);
   } 						  
   else if(%towNum == 3) 
   {
      Team::setObjective(0, 17, "<f1><Btower_teamcontrol.bmp>\n   " @ %string);
   }
   //initial setup
   else
   {
      Team::setObjective(0, 15, "<f1><Btower_enemycontrol.bmp>\n   Tower 1 has not been captured.");
   	  Team::setObjective(0, 16, "<f1><Btower_enemycontrol.bmp>\n   Tower 2 has not been captured.");
	  Team::setObjective(0, 17, "<f1><Btower_enemycontrol.bmp>\n   Tower 3 has not been captured.");
   }
}

function missionSummary()
{
   %time = getSimTime() - $MatchStartTime;
   
   Training::displayBitmap(0);
   
   Team::setObjective(0, 1, "<f5><jl>Mission Completion:");
   Team::setObjective(0, 2, "<f1>   -Completed:");
   Team::setObjective(0, 3, "\n");
   Team::setObjective(0, 4, "<f5><jl>Mission Information:");
   Team::setObjective(0, 5, "<f1>   -Mission Name: Capture and Hold Training");
   Team::setObjective(0, 6, "\n");
   
   Team::setObjective(0, 7, "<f5><j1>Mission Summary:");
   
   Team::setObjective(0, 8, "<f1>   -Enemy Kills: " @ "<f1>" @ $AIKilled @ " out of " @ $numGuards);
   Team::setObjective(0, 9, "<f1>   -Total Mission Time: " @ "<f1>" @ Time::getMinutes(%time) @ " Minutes " @ Time::getSeconds(%time) @ " Seconds");
   Team::setObjective(0, 10, "");
   Team::setObjective(0, 11, "");
   Team::setObjective(0, 12, "");
   Team::setObjective(0, 13, "");
   Team::setObjective(0, 14, "");
   Team::setObjective(0, 15, "");
   Team::setObjective(0, 16, "");
   Team::setObjective(0, 17, "");
   Team::setObjective(0, 18, "");
   Team::setObjective(0, 19, "");
   Team::setObjective(0, 20, "");
}


function TowersTraining::setWayPoint(%clientid)
{
   //setup initial waypoint information
   %currentPos = $switchPosition[$currentWay];
   %currentx = getWord(%currentPos, 0);
   %currenty = getWord(%currentPos, 1);
   bottomprint(%clientId, "<f1><jc>Your Waypoint has been set to tower " @ $currentWay @ ".", 5);    
   
   issueCommand(%clientId, %clientId, 0, "Waypoint set to tower " @ $currentWay @ ".", %currentx, %currenty);
}       

function TowersTraining::objectiveComplete(%this, %taken)
{
  for(%k = 1; %k <= $numSwitches; %k++)
  {
    if(%this == $SwitchObject[%k])
    {  
      if(%taken)
	  {
        $TowerControlled[%k] = "false";
		$numToComplete++;
	  }
	  else
	  {
	    $TowerControlled[%k] = "true";
      	$numToComplete--;
      }
    }
  }
  if($numToComplete != 0)
  {
    if(%this == $SwitchObject[$currentWay] && !%taken)
    {
      if($currentWay != 3)
	  {
        $currentWay++;
        %playerId = Client::getFirst();
        %newPos = GameBase::getPosition($SwitchObject[$currentWay]);
        %currentx = getWord(%newPos, 0);
        %currenty = getWord(%newPos, 1);
        bottomprint(%playerId, "<f1><jc>Your Waypoint has been set to tower " @ $currentWay @ ".", 5);
        issueCommand(%playerId, %playerId, 0, "Goto waypoint " @ $currentWay, %currentx, %currenty);
	  }
	  //else
	    //setupLastWayPoint();
	}
    else
    {
      dbecho(2,"Captured secondary target, waypoint still set to primary target.");
	}
  } 
  else
  {
    $missionOver = true;
   
  }
}

function setUpLastWayPoint()
{
  $currentWay = 1;
  %playerId = Client::getFirst();
  %newPos = GameBase::getPosition($SwitchObject[$currentWay]);
  %currentx = getWord(%newPos, 0);
  %currenty = getWord(%newPos, 1);
  bottomprint(%playerId, "<f1><jc>Your Waypoint has been set back to tower " @ $currentWay @ " for recapture.", 5);
  issueCommand(%playerId, %playerId, 0, "Goto waypoint " @ $currentWay, %currentx, %currenty);
}

function Game::initialMissionDrop(%clientId)
{
    GameBase::setTeam(%clientId, 0);
    Client::setGuiMode(%clientId, $GuiModePlay);
    Game::playerSpawn(%clientId, false);
	//Team::clearObjectives(0);
	Training::displayBitmap(0);
    Training::setupAI(%clientId);
    Training::startMission(%clientId);    
}


function TowersTraining::wrongObjective(%this, %object)
{
  %cl = client::getFirst();
  bottomprint( %cl, "<f1><jc>You must take the tower that has been set by your commander first.", 5);
}


function Training::missionComplete(%cl)
{
  
  schedule("Client::setGuiMode(" @ %cl @ ", " @ $GuiModeObjectives @ ");", 8);
  missionSummary();
  remoteEval(2049, TrainingEndMission);
}

function remoteTrainingEndMission()
{
   schedule("EndGame();", 16);
}

function Training::startMission(%playerId)
{
  schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>Training Mission 6 - Capture and Hold.\", 5);", 0);
  schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>This mission will introduce you to the capture and hold game scenario.\", 5);", 5);
  schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>Bring up The Objectives screen for a gameplay description.\", 5);", 10);
  schedule("messageAll(0, \"~wshell_click.wav\");", 0);
  schedule("messageAll(0, \"~wshell_click.wav\");", 5);
  schedule("messageAll(0, \"~wshell_click.wav\");", 10);
  schedule("TowersTraining::setWayPoint(" @ %playerId @ ");", 15);
}

function Player::enterMissionArea(%this)
{
}

function Player::leaveMissionArea(%this)
{
}

//init all info
TowersTraining::init();
towers::teamMissionObjectives(0);