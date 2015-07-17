//Training_retrieval.cs

$Train::missionType = "F&R";
exec("game.cs");
exec(Training_AI);

function Retrieval::init(%clientId)
{
   bottomprint(%clientId, "<jc><f1>Training Mission 7 - Find and Retrieve", 5);
   schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>This mission will introduce you to the Find and Retrieve game scenario.\", 5);", 5);
   schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>Bring up The Objectives screen for a gameplay description.\", 5);", 10);
   schedule("messageAll(0, \"~wshell_click.wav\");", 0);
   schedule("messageAll(0, \"~wshell_click.wav\");", 5);
   schedule("messageAll(0, \"~wshell_click.wav\");", 10);
   %player = Client::getOwnedObject(%clientId);
   %player.hasFlag = false;
   $flagDropped = false;
   %player.flag	= "";
   $flagReturnTime = 30;
   %lineNum = 13;
   $capsToWin = 0;
   $captures = 0;
   $ourBase = "-125.978 -91.6377 172.689";
   $flagBase = "247.425 -284.296 211.108";
   
   %group = nameToID("MissionGroup\\Flags");
    
   %numObj = Group::objectCount(%group);
   
   for(%j = 0; %j < %numObj; %j++)
   { 
      %tempObj = Group::getObject(%group, %j);
      
      if(GameBase::getDataName(%tempObj) == flag)
      {
        %tempObj.lineNum = %lineNum++;
        %tempObj.isMounted = false;
        %tempObj.lost = false;
		%tempObj.conveyed = false;
        %tempObj.originalPos = GameBase::getPosition(%tempObj);
        $capsToWin++;
		retrieval::checkMissionObjectives(%clientId, %tempObj);
      }
   }
   schedule("retrieval::setWayPoint(" @ %tempObj @ ", " @ %clientId @ ", " @ true @ ");", 15);
}

function Game::initialMissionDrop(%clientId)
{
   GameBase::setTeam(%clientId, 0);
   Client::setGuiMode(%clientId, $GuiModePlay);
   Game::playerSpawn(%clientId, false);
   Retrieval::init(%clientId);
   Training::setupAI( %clientId );
}

///////////////////////////////////////
//Flag functions
///////////////////////////////////////
function FlagStand::onCollision(%this, %object)
{
   
   %standTeam = GameBase::getTeam(%this);
   %playerTeam = GameBase::getTeam(%object);
   %playerClient = Player::getClient(%object);
   
   
   if(getObjectType(%object) != "Player" || %playerTeam != %standTeam || %this.hasFlag)
      return;

   if(%object.hasFlag)
   {   
      // if we're here, we're carrying a flag, we've hit 
      // our flag stand, it doesn't have a flag.
      
      %flag = %object.flag;
      Item::hide(%object.flag, false);
      GameBase::setPosition(%object.flag, GameBase::getPosition(%this));
	   
      Player::setItemCount(%object, "Flag", 0);
      %this.hasFlag = true;
	  %object.hasFlag = false;
	  %object.flag.isMounted = true;
	  $captures++;
	  
	  if($captures != 3)
	  {  
	     bottomprint(%playerClient, "<jc><f1>You conveyed the " @ %object.flag.objective @ " to your base.", 5);
	  	 Training::moreAI($captures);
	  }
	  messageAll(%playerClient, "~wflagcapture.wav");
	  %object.flag = "";
	  %flag.conveyed = true;
	  
	  retrieval::checkMissionObjectives(%playerClient, %flag);
	  if($captures != 3)
	     retrieval::setWayPoint(%flag, %playerClient, false);
   }
}

function Flag::onCollision(%this, %object)
{
   if(getObjectType(%object) != "Player" || %object.hasFlag || %this.isMounted)
      return;

   %playerClient = Player::getclient(%object);
   %flagTeam = GameBase::getTeam(%this);
   
   Player::setItemCount(%object, "Flag", 1); 
   Player::mountItem(%object, "Flag", $FlagSlot, %flagTeam);
   Item::hide(%this, true);
   %object.flag = %this;
   %object.hasFlag = true;    
   
   if(!$flagDropped)
     bottomprint(%playerClient, "<jc><f1>You took the " @ %this.objective @ " !", 5);
   else
   {  
     bottomprint(%playerClient, "<jc><f1>You recovered the " @ %this.objective @ " !", 5);
	 %this.lost = "false";
   }
   messageall(%playerClient, "~wflag1.wav");
   retrieval::checkMissionObjectives(%playerClient, %this);
   retrieval::setWayPoint(%this, %playerClient, false);
}

function Flag::onDrop(%player, %type)
{
   %playerTeam = GameBase::getTeam(%player);
   %playerClient = Player::getClient(%player);
   %dropClientName = Client::getName(Player::getClient(%player));
   %flag = %player.flag;
   %player.flag = "";
   %player.hasFlag = false;
   %flag.lost = true;
   
   bottomprint(%playerClient, "<jc><f1>You dropped the " @ %flag.objective @ " !", 5);
   GameBase::throw(%flag, %player, 10, false);
   Item::hide(%flag, false);
   Player::setItemCount(%player, "Flag", 0);
   
   schedule("Retrieval::checkFlagReturn(" @ %flag @ ", " @ %playerClient @ ");", $flagReturnTime);
   retrieval::checkMissionObjectives(%playerClient, %flag);
   $flagDropped = true;
   retrieval::setWayPoint(%flag, %playerClient, false);
}

function Retrieval::checkFlagReturn(%this, %clientId)
{
   if(%this.lost)
   {
      bottomprint(%clientId, "<f1><jc>" @ %this.objective @ " was returned to its base.", 5);
      dbecho(2,"moving flag to " @ %this.originalPos);
      GameBase::startFadeOut(%this);
      GameBase::setPosition(%this, %this.originalPos);
	  GameBase::startFadeIn(%this);
      $flagDropped = false;
	  %this.lost = false;
	  retrieval::checkMissionObjectives(%clientId, %this);
	  retrieval::setWayPoint(%this, %clientId, false);
   }
}

function retrieval::setWayPoint(%flag, %cl, %init)
{
   %player = Client::getOwnedObject(%cl);
   %flagPos = GameBase::getPosition(%flag);
   %flagx = getWord(%flagpos, 0);
   %flagy = getWord(%flagpos, 1);
   
   %homebasePos = $ourBase;
   %homebasex = getWord(%homebasePos, 0);
   %homebasey = getWord(%homebasePos, 1);
   
   %flagbasePos = $flagBase;
   %flagbasex = getWord(%flagbasePos, 0);
   %flagbasey = getWord(%flagbasePos, 1);  

   
   if(%flag.lost)
   {
      schedule("bottomprint(" @ %cl @ ", \"<jc><f1>Waypoint set to " @ %flag.objective @ "\", 5);", 5);
	  issueCommand(%cl, %cl, 0, "Waypoint set to " @ %flag.objective, %flagx, %flagy);
   }
   else	if(%flag.conveyed || %init)
   {
      schedule("bottomprint(" @ %cl @ ", \"<jc><f1>Waypoint set to next objective\", 5);", 5);
	  issueCommand(%cl, %cl, 0, "Waypoint set to next objective", %flagbasex, %flagbasey);
   }
   else if(%player.hasFlag)
   {
      schedule("bottomprint(" @ %cl @ ", \"<jc><f1>Waypoint set back to your base\", 5);", 5);
	  issueCommand(%cl, %cl, 0, "Waypoint set to your base", %homebasex, %homebasey);
   }
}

function retrieval::checkMissionObjectives(%cl, %flag)
{
  if(retrieval::MissionObjectives( Client::getTeam(%cl), %cl, %flag ))
   {
      retrieval::MissionComplete( %cl );
   }
}

function retrieval::MissionObjectives( %teamId, %cl, %this )
{
   %enemyTeam = $enemyTeam[%teamId];
   %teamName = getTeamName(%teamId); 
   %player = Client::getOwnedObject(%cl);
   %enemyTeamName = getTeamName(%enemyTeam);
   %capsNeeded = $capstowin - $captures;
  
   if(%capsNeeded == 0)
   {
     return "True";
   }
   
   Training::displayBitmap(0);
   
   Team::setObjective(%teamId, 1, "<f5><jl>GamePlay Description:");
   Team::setObjective(0, 2, "<f1><jl>   In a Find and Retrieve mission, your team must find objectives placed in a");  
   Team::setObjective(0, 3, "<f1><jl>   mission and bring them back to your base. This is similar to CTF, but the");
   Team::setObjective(0, 4, "<f1><jl>   objectives are not owned by any one team in the beginning.  For every");
   Team::setObjective(0, 5, "<f1><jl>   objective you return to your base, your team scores points.");
   Team::setObjective(%teamId, 6, "\n");
   Team::setObjective(%teamId, 7, "<f5><jl>Mission Completion:"); 
   Team::setObjective(%teamId, 8, "<f1>   -Convey all objective flags to your base");
   Team::setObjective(%teamId, 9, "\n");
   Team::setObjective(%teamId, 10, "<f5>Mission Information:");
   Team::setObjective(%teamId, 11, "<f1>   -Mission Name: Find and Retrieve Training"); 
   Team::setObjective(%teamId, 12, "\n");
   Team::setObjective(%teamId, 13, "<f5><jl>Mission Objectives:");

   if(%this.conveyed)
   {
      %status = "<Bflag_atbase.bmp><F1>\n    " @ %this.objective @ " has been conveyed";
      
   }
   else
   {
      
      if(%this.lost)
      {
         %status = "<Bflag_notatbase.bmp><f1>\n    " @ %this.objective @ " was dropped in the field";
      }
      else if($captures == 0 && !%player.hasFlag)
      {
		 %status = "<Bflag_neutral.bmp><f1>\n    " @ %this.objective @ " has not been conveyed";
      }
      else if(%player.hasFlag)
      {
         %status = "<Bflag_atbase.bmp><f1>\n    Convey " @ %this.objective @ " to your base";
      }
	  else
	     %status = "<Bflag_neutral.bmp><f1>\n    " @ %this.objective @ " has not been conveyed";
   }
   
   if(%this.lineNum == 14)
      Team::setObjective(%teamId, %this.lineNum, %status);
   else if(%this.lineNum == 15)
      Team::setObjective(%teamId, %this.lineNum, %status);
   else if(%this.lineNum == 16)
      Team::setObjective(%teamId, %this.lineNum, %status);

   return "False";
}

function Retrieval::missionComplete(%cl)
{
  
  schedule("Client::setGuiMode(" @ %cl @ ", " @ $GuiModeObjectives @ ");", 8);
  missionSummary();
  remoteEval(2049, TrainingEndMission);
}

function remoteTrainingEndMission()
{
   schedule("EndGame();", 16);
}

function missionSummary()
{
   %time = getSimTime() - $MatchStartTime;
   
   Training::displayBitmap(0);
   
   Team::setObjective(0, 1, "<f5><jl>Mission Completion:");
   Team::setObjective(0, 2, "<f1><t>   -Completed:");
   Team::setObjective(0, 3, "\n");
   Team::setObjective(0, 4, "<f5><jl>Mission Information:");
   Team::setObjective(0, 5, "<f1><t>   -Mission Name: Find and Retrieve Training");
   Team::setObjective(0, 6, "\n");
   
   Team::setObjective(0, 7, "<f5><j1>Mission Summary:");
   
   Team::setObjective(0, 8, "<f1><t>   -Enemy Kills: " @ "<f1>" @ $AIKilled @ " out of " @ $numGuards);
   Team::setObjective(0, 9, "<f1><t>   -Total Mission Time: " @ "<f1>" @ Time::getMinutes(%time) @ " Minutes " @ Time::getSeconds(%time) @ " Seconds");
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


function Training::MoreAI(%wave)
{
  if(%wave == 1)
    %group = nameToId("MissionGroup\\AIwave1");
  else if(%wave == 2)
    %group = nameToId("MissionGroup\\AIwave2");

  if(%group == -1)
     dbecho(2,"No AI exists...");  
   
  else 
  {  
     %clientId = Client::getFirst(); 
     %AIname = "guard" @ $numGuards + 1;
     createAI(%AIname, %group, larmor, $AI_Names[ floor(getRandom() * 10) ]);
     %aiId = AI::getId( %AIname );
     GameBase::setTeam(%aiId, 1);
     AI::setVar( %AIname,  iq,  70 );
     AI::setVar( %AIname,  attackMode, 0);
     AI::DirectiveTarget(%AIname, %clientId);
  }
  AI::callWithId(%AIname, Player::setItemCount, blaster, 1);
  AI::callWithId(%AIname, Player::mountItem, blaster, 0);
  AI::SetVar(%AIname, triggerPct, 0.04 );
  $numGuards++;
  
}

function remoteScoresOn(%clientId)
{
}

function remoteScoresOff(%clientId)
{
}
