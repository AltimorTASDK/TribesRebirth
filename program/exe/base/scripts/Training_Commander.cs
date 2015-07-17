//Commander Training 
//-------------------------

exec("game.cs");
exec("training_AI.cs");
$Train::missionType = "COMMAND";
$pref::mapFilter = 15;
$pref::mapNames = true;

function Game::initialMissionDrop(%clientId)
{
   GameBase::setTeam(%clientId, 0);
   Client::setGuiMode(%clientId, $GuiModePlay);
   Game::playerSpawn(%clientId, false);
   Training::displayBitmap(0);
   command::initTargets(%clientId);
   Training::setupAI( %clientId );
}


function command::initTargets(%cl)
{
  $panelsDes = 0;
  $numCompleted = 0;
  $TrainMissionComplete = false;
  %group = nameToId("MissionGroup\\Teams\\Team1\\base");
  %numObj = Group::objectCount(%group);
  $numTargets = 0;
  %lineNum = 7;
  $init = true;
  
  %set = newObject(ObjectivesSet, SimSet);
  addToSet(MissionCleanup, ObjectivesSet);
  
  for(%i = 0; %i < %numObj; %i++)
  {
    %Obj = Group::getObject(%group, %i);
	
    if(%Obj.target)
	{
      %Obj.destroyed = false;
      $targets[%i] = %Obj;
	  $completed[%i] = false;
	  %Obj.lineNum = %lineNum++;
	  $numTargets++;
	  %Obj.position = GameBase::getPosition(%Obj);
	  addToSet(%set, %Obj);
	  Command::checkMissionObjectives(%cl, %Obj);
	}
  }
  Command::intro(%cl);
}

function Command::intro(%clientId)
{
   bottomprint(%clientId, "<jc><f1>Training Mission 4 - Commander and Target laser Training", 5);
   schedule("messageAll(0, \"~wshell_click.wav\");", 0);
   schedule("firstObjective1(" @ %clientId @ ");", 5);
   schedule("command::setWayPoint(" @ %clientId @ ");", 5);
}

function firstObjective1(%clientId)
{
   schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>Here are your orders.\", 5);", 0);
   schedule("messageAll(0, \"~wshell_click.wav\");", 0);
   schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>The enemy base has been located by your scouts, and a mortar attack has been issued to clear the way for offensive troops.  It is going to be your job to set up targets for the two heavy armor troops that are under your command.\", 15);", 5);
   schedule("messageAll(0, \"~wshell_click.wav\");", 5);
   firstObjective2(%clientId);
}  
  
function firstObjective2(%clientId)
{  
   schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>Before you do anything, you will need to be briefed on using your base inventory machine so that you can get equipped for your mission.\", 15);", 20);
   schedule("messageAll(0, \"~wshell_click.wav\");", 20);
   schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>In front of you is an inventory station. Walk up to it to bring up the inventory screen. The list on the right are things that you may purchase. Things in your personal inventory are displayed on the left side of the screen.\", 15);", 35);
   schedule("messageAll(0, \"~wshell_click.wav\");", 35);
   firstObjective3(%clientId);
}  
  
function firstObjective3(%clientId)
{ 
   schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>You are going to need a targeting laser for this mission to target objects for your mortar men. To purchase a targeting laser scroll down on the buy menu until you come to the weapons section. Choose the targeting laser by double clicking on it.\", 15);", 50);
   schedule("messageAll(0, \"~wshell_click.wav\");", 50);
   firstObjective4(%clientId);
}
  
  
function firstObjective4(%clientId)
{ 
   schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>Once you have the targeting laser, hit the inventory key(I), exit your base,  and we will move on to the commander screen briefing.\", 15);", 65);
   schedule("messageAll(0, \"~wshell_click.wav\");", 65);
   schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>To view your commander screen, press the C key. You should see a map of the mission area. This is a top down view of everything in the area.\", 15);", 80);
   schedule("messageAll(0, \"~wshell_click.wav\");", 80);
   firstObjective5(%clientId);
}  
  
function firstObjective5(%clientId)
{ 
   schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>You are represented by a solid green triangle somewhere near the building labeled Home Base. It should be within your senor range circle. Use the zoom button at the bottom right to zoom in on your base.\", 151);", 95);
   schedule("messageAll(0, \"~wshell_click.wav\");", 95);
   schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>To command your men, click on them either from the list of players on the top right, or by clicking on them on the map. When they are selected, a box will appear around them.\", 15);", 110);
   schedule("messageAll(0, \"~wshell_click.wav\");", 110);
   firstObjective6(%clientId);
}  
  
function firstObjective6(%clientId)
{ 
   schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>To give them a waypoint just choose a command from the bottom right menu.  For example, choose one of your troops and issue him to attack a waypoint by pressing the A key and then clicking on the map where you want him to go.\", 15);", 125);
   schedule("messageAll(0, \"~wshell_click.wav\");", 125);
   secondObjective1(%clientId); 
}

function secondObjective1(%clientId)
{   
   schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>Your men will not fire until they have a target, however. This is where you come in. You are going to target objects for them to attack with the targeting laser.\", 15);", 140);
   schedule("messageAll(0, \"~wshell_click.wav\");", 140);
   schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>On the Commander screen you will see location A and location B.\", 20);", 155);
   schedule("messageAll(0, \"~wshell_click.wav\");", 155);
   schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>Once they are in position, it will be time for you to advance to a position near the waypoint given to you. Once you get there you can start lighting up targets.\", 15);", 170);
   schedule("messageAll(0, \"~wshell_click.wav\");", 170);
   schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>To target things, just aim at the object with your crosshair and then pull the trigger. You must hold the fire button down to keep the beam targeting the object.\", 15);", 185);
   schedule("messageAll(0, \"~wshell_click.wav\");", 185);
   schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>You should see the mortars take out the targets. If you notice that the mortars are landing long or short, adjust where you fire the beam. Use your zoom(E) to help target objects.\", 15);", 200);
   schedule("messageAll(0, \"~wshell_click.wav\");", 200);
   secondObjective2(%clientId);
}

function secondObjective2(%clientId)
{
  schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>There are 6 targets that need to be destroyed. 3 Solar panels, 1 Sensor, and 2 Turrets.  Your first target will be the enemy's solar panels that supply their power. They are just West of thier base.\", 15);", 215);
  schedule("messageAll(0, \"~wshell_click.wav\");", 215);
  schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>Taking out their power first will negate all the shields for concurrent targets. Even if they have some kind of backup system, the sensors should be down long enough to take them out.\", 15);", 230);
  schedule("messageAll(0, \"~wshell_click.wav\");", 230);
  schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>Next, take out the enemy sensor.  If the enemy does repair power to their base, they will be blind to our infiltration.\", 15);", 245);
  schedule("messageAll(0, \"~wshell_click.wav\");", 245);
  schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>Now we need to take out the turrets that guard the entrance. They should be easy to destroy provided the enemy has not restored power as of yet.\", 15);", 260);
  schedule("messageAll(0, \"~wshell_click.wav\");", 260);
  schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>The best approch is to send your men to location A for the solar panels and the main Sensor, and then move them to location B for the 2 turrets.\", 15);", 275);
  schedule("messageAll(0, \"~wshell_click.wav\");", 275);
  schedule("bottomprint(" @ %clientId @ ", \"<jc><f1>Your waypoint has been set to the first target.  Good luck.\", 10);", 290);
  schedule("messageAll(0, \"~wshell_click.wav\");", 290);
}  

function checkTargetsDestroyed(%this)
{
   %cl = Client::getFirst();
   Command::checkMissionObjectives(%cl, %this);
   
   if(!$TrainMissionComplete && $panelsDes > 2)
      Command::setWayPoint(%cl, %this);
}

function StaticShape::objectiveDestroyed(%this)
{
  dbecho(2,"shape destroyed");
  $numCompleted++;
  
  for(%i = 0; %i < $numTargets; %i++)
  {
    if(%this == $targets[%i])
	{
	  $panelsDes++;
	  $completed[%i] = true;
	  %cl = Client::getFirst(); 
	  %this.destroyed = true;
	  if($panelsDes == 3)
	  {
	     bottomprint(%cl, "<f1><jc>You have destroyed the solar panels", 5);
         messageAll(0, "~wshell_click.wav");
		 Ai::soundHelper( 2051, 2049, cheer3 );
	  }
	}
  }
  checkTargetsDestroyed(%this);
}


function Sensor::onDestroyed(%this)
{
	%this.shieldStrength = 0;
	GameBase::setRechargeRate(%this,0);
	Sensor::onDeactivate(%this);
	sensor::objectiveDestroyed(%this);
	%sensorName = GameBase::getDataName(%this);
	if(%sensorName == "DeployableSensorJammer") 
   	$TeamItemCount[GameBase::getTeam(%this) @ "DeployableSensorJammerPack"]--;
	else if(%sensorName == "DeployableMotionSensor") 
   	$TeamItemCount[GameBase::getTeam(%this) @ "MotionSensorPack"]--;
	else if(%sensorName == "DeployablePulseSensor") 
   	$TeamItemCount[GameBase::getTeam(%this) @ "PulseSensorPack"]--;
	calcRadiusDamage(%this, $DebrisDamageType, 2.5, 0.05, 25, 13, 2, 0.40, 
		0.1, 250, 100);
}


function sensor::objectiveDestroyed(%this)
{
    
  dbecho(2, "sensor destroyed");
  $numCompleted++;
  for(%i = 0; %i < $numTargets; %i++)
  {
    if(%this == $targets[%i])
	{
	  $completed[%i] = true;
	  %cl = Client::getFirst(); 
	  %this.destroyed = true;
	  bottomprint(%cl, "<f1><jc>You have destroyed " @ %this.objective, 5);
      Ai::soundHelper( 2051, 2049, cheer4 );
	  
	}
  }
  checkTargetsDestroyed(%this);
}

function Turret::onDestroyed(%this)
{
	%this.shieldStrength = 0;
	GameBase::setRechargeRate(%this,0);
	Turret::onDeactivate(%this);
	Turret::objectiveDestroyed(%this);
	calcRadiusDamage(%this, $DebrisDamageType, 2.5, 0.05, 25, 9, 3, 0.40, 
		0.1, 200, 100); 
}



function turret::objectiveDestroyed(%this)
{
    
  dbecho(2,"turret destroyed");
  $numCompleted++;
  for(%i = 0; %i < $numTargets; %i++)
  {
    if(%this == $targets[%i])
	{
	  $completed[%i] = true;
	  %cl = Client::getFirst(); 
	  %this.destroyed = true;
	  bottomprint(%cl, "<f1><jc>You have destroyed " @ %this.objective, 5);
      Ai::soundHelper( 2050, 2049, cheer2 );
	  
	}
  }
  checkTargetsDestroyed(%this);
}


function Command::setWayPoint(%cl)
{
   %nextTarget = 0;
   %group = nameToID("MissionCleanup/ObjectivesSet");
   %target = Group::getObject(%group, %nextTarget);

   while(%target.destroyed)
   {   
      %nextTarget++;
      %target = Group::getObject(%group, %nextTarget);
   } 
   
   %x = getWord(%target.position, 0);
   %y = getWord(%target.position, 1);

   if($init)
      %delay = 10;
   else 
      %delay = 5;
   
   if(!%target.init)
       schedule("bottomprint(" @ %cl @ ", \"<jc><f1>Waypoint set to " @ %target.objective @ "\", 5);", 5);
   issueCommand(%cl, %cl, 0, "Waypoint set to next objective", %x, %y);
   $init = false;
}

function Training::setupAI(%clientId)
{
  %group = "MissionGroup\\AI";
  $numGuards = Group::objectCount(%group);
    
  
  if( %group == -1 || $numGuards == 0 )
    dbecho(2,"No AI exists...");  
  
  else 
  {  
    for(%guard = 1; %guard <= $numGuards; %guard++)
    {
      %AIname = "guard" @ %guard;
      createAI(%AIname, %group @ "\\guard" @ %guard, harmor, $AI_Names[floor(getRandom() * 15)]);
 	  GameBase::setTeam(AI::getId( %AIname ), 0);
 	  AI::setVar( %AIname,  iq,  100 );
 	  AI::setVar( %AIname,  attackmode, 1 );
 	}
    
	AI::DirectiveTargetLaser( "Guard1", %clientId );
	AI::DirectiveTargetLaser( "Guard2", %clientId );
	AI::callWithId("Guard1", Player::setItemCount, Mortar, 1);
	AI::callWithId("Guard1", Player::setItemCount, MortarAmmo, 10000);
	AI::callWithId("Guard1", Player::mountItem, mortar, 0);
	AI::callWithId("Guard2", Player::setItemCount, Mortar, 1);
	AI::callWithId("Guard2", Player::setItemCount, MortarAmmo, 10000);
	AI::callWithId("Guard2", Player::mountItem, mortar, 0);
	AI::SetVar( "*", triggerPct, 100 );
  }
}


function command::checkMissionObjectives(%cl, %shape)
{
  if(command::MissionObjectives( Client::getTeam(%cl), %cl, %shape ))
   {
      bottomprint(%cl, "<f1><jc>You have completed the commander and targeting laser training!", 8);
      schedule("training::MissionComplete(" @ %cl @ " );", 8);
   }
}

function Command::MissionObjectives( %teamId, %cl, %this )
{
   %teamName = getTeamName(%teamId); 
   %player = Client::getOwnedObject(%cl);

   %enemyTeamName = getTeamName(%enemyTeam);
  
   if($numCompleted == $numTargets)
   {
     $TrainMissionComplete = true;
     return "True";
   }
   
   Team::setObjective(%teamId, 1, "<f5><jl>Mission Completion:");
   Team::setObjective(%teamId, 2, "<f1>   -Destroy all enemy targets that are assigned");
   Team::setObjective(%teamId, 3, "\n");
   Team::setObjective(%teamId, 4, "<f5>Mission Information:");
   Team::setObjective(%teamId, 5, "<f1>   -Mission Name: Commander and Targeting laser Training");
   Team::setObjective(%teamId, 6, "\n"); 
   Team::setObjective(%teamId, 7, "<f5><jl>Mission Obejectives:");
   
   if(%this.destroyed)
   {
      %status = "<f1><Bitem_ok.bmp>\n   -You successfully destroyed " @ %this.objective;
   }
   else
   {
      %status = "<f1><Bitem_damaged.bmp>\n   -You have not destroyed " @ %this.objective;
   }

   if(%this.lineNum == 8)
      Team::setObjective(%teamId, %this.lineNum, %status);
   else if(%this.lineNum == 9)
      Team::setObjective(%teamId, %this.lineNum, %status);
   else if(%this.lineNum == 10)
      Team::setObjective(%teamId, %this.lineNum, %status);
   else if(%this.lineNum == 11)
      Team::setObjective(%teamId, %this.lineNum, %status);
   else if(%this.lineNum == 12)
      Team::setObjective(%teamId, %this.lineNum, %status);
   else if(%this.lineNum == 13)
      Team::setObjective(%teamId, %this.lineNum, %status);

   return "False";
}

function missionSummary()
{
   %time = getSimTime() - $MatchStartTime;
   
   Training::displayBitmap(0);
   Team::setObjective(0, 1, "<f5><jl>Mission Completion:");
   Team::setObjective(0, 2, "<f1>   -Completed:");
   Team::setObjective(0, 3, "\n");
   Team::setObjective(0, 4, "<f5><jl>Mission Information:");
   Team::setObjective(0, 5, "<f1>   -Mission Name: Commander and Target laser Training");
   Team::setObjective(0, 6, "\n");
   
   Team::setObjective(0, 7, "<f5><j1>Mission Summary:");
   
   Team::setObjective(0, 8, "<f1>   -Total Mission Time: " @ "<f1>" @ Time::getMinutes(%time) @ " Minutes " @ Time::getSeconds(%time) @ " Seconds");
   Team::setObjective(0, 9, "<f1>   -Total enemy kills: " @ $AIkilled);
   Team::setObjective(0, 10, "\n");
   Team::setObjective(0, 11, "\n");
   Team::setObjective(0, 12, "\n");
   Team::setObjective(0, 13, "\n");
   Team::setObjective(0, 14, "\n");
}

function Training::missionComplete(%cl)
{
  schedule("Client::setGuiMode(" @ %cl @ ", " @ $GuiModeObjectives @ ");", 0);
  missionSummary();
  remoteEval(2049, TrainingEndMission);
}

function remoteTrainingEndMission()
{
   schedule("EndGame();", 8);
}


//these just disable certain behaviors
function StaticShape::objectiveDisabled(%this)
{
}

function StaticShape::objectiveEnabled(%this)
{
}

function remoteScoresOn(%clientId)
{
}

function remoteScoresOff(%clientId)
{
}


function Flag::onCollision(%this, %object)
{
   dbecho( 2, "Flag collision called for " @ %this @ " and " @ %object );
}

