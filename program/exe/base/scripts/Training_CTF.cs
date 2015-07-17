//Training_CTF.cs
//----------------------
//
//Handles all automation of the Capture the Flag Training mission
//----------------------------------------------------------------

$Train::missionType = "CTF";
exec("game.cs");
exec("Training_AI.cs");


function ctf::init()
{
  
  %group0 = "MissionGroup\\teams\\team0\\flag";
  %group1 = "MissionGroup\\teams\\team1\\flag";
  
  $flagObject[0] = Group::getObject(%group0, 0);
  $flagObject[1] = Group::getObject(%group1, 0);

  $flagAtHome[0] = true;
  $flagAtHome[1] = true;
  $enemyTeam[0] = 1;
  $enemyTeam[1] = 0;

  $enemyFlagName[0] = "Flag1";
  $enemyFlagName[1] = "Flag0";

  $flagCarrier[0] = -1;
  $flagCarrier[1] = -1;

  $flagPosition[0] = GameBase::getPosition($flagObject[0]);
  $flagPosition[1] = GameBase::getPosition($flagObject[1]);

  dbecho(2, "flagObject 0 pos:" @ $flagPosition[0]);
  dbecho(2, "flagObject 1 pos:" @ $flagPosition[1]);

  $lastTeamSpawn[0] = -1;
  $lastTeamSpawn[1] = -1;

  $captures = 0;
  $capstowin = 3;
  $pickupSequence[0] = 0;
  $pickupSequence[1] = 0;
  $flagReturnTime = 45;
  $flagDropped = false;

}

function TeamMessages(%mtype, %team1, %message1, %team2, %message2)
{
   %numPlayers = getNumClients();
   for(%i = 0; %i < %numPlayers; %i++)
   {
      %id = getClientByIndex(%i);
      if(Client::getTeam(%id) == %team1)
      {
         Client::sendMessage(%id, %mtype, %message1);
      }
      else if(Client::getTeam(%id) == %team2)
      {
         Client::sendMessage(%id, %mtype, %message2);
      }
   }
}

function CTFTraining::setWayPoint(%this, %init)
{
   %flagTeam = GameBase::getTeam(%this);
   //setup waypoint information
   if(GameBase::getTeam(%this) == 1)
     %currentPos = GameBase::getPosition($flagObject[0]);
   else
     %currentPos = GameBase::getPosition($flagObject[1]);
   
     %currentx = getWord(%currentPos, 0);
     %currenty = getWord(%currentPos, 1);
     %clFirst = Client::getFirst();    
   
   if(!$flagDropped)
   {
     if(!%init)
     {
       
       if($flagAtHome[1])
       {  
         for(%cl = Client::getFirst(); %cl != -1; %cl = Client::getNext(%cl))
         {
           %this.trainingObjectiveComplete = "";
           issueCommand(%clFirst, %cl, 0, "Make your way to the enemy's base.", %currentx, %currenty);
           schedule("bottomprint( " @ %cl @ ", \"<jc><f1>WayPoint set to enemy base.\", 5);", 5);

     	   }
       }
       else
       {  
         %this.trainingObjectiveComplete = "";
         for(%cl = Client::getFirst(); %cl != -1; %cl = Client::getNext(%cl))
     	   {
     	     issueCommand(%clFirst, %cl, 0, "Return enemy flag to your base.", %currentx, %currenty);
     	     schedule("bottomprint(" @ %cl @ ", \"<jc><f1>Bring enemy flag back to your base for a capture.\", 5);", 5);
     	   }
     	 }
     }
     else
     {  
         issueCommand(%clFirst, %clFirst, 0, "Make your way to the enemy's base.", %currentx, %currenty);
         schedule("bottomprint(" @ %clFirst @ ", \"<jc><f1>WayPoint set to enemy base.\", 5);", 15);
     }
   }
   else
   {  
     issueCommand(%clFirst, %clFirst, 0, "Make your way to the enemy's flag.", %currentx, %currenty);
     schedule("bottomprint(" @ %clFirst @ ", \"<jc><f1>WayPoint set to dropped flag.\", 5);", 5);
   }
}       
  

function Flag::onDrop(%player, %type)
{
   %playerTeam = GameBase::getTeam(%player);
   %flagTeam = $enemyTeam[%playerTeam];
   %playerClient = Player::getClient(%player);
   %dropClientName = Client::getName(Player::getClient(%player));
   
   
   bottomprint(%playerClient, "<f1><jc>You dropped the " @ getTeamName(%flagTeam) @ " flag!", 5);
   

   GameBase::throw($flagObject[%flagTeam], %player, 10, false);
   Item::hide($flagObject[%flagTeam], false);
   Player::setItemCount(%player, "Flag", 0);
   
   $flagCarrier[%flagTeam] = -1;
   schedule("CTF::checkFlagReturn(" @ %flagTeam @ ", " @ $pickupSequence[%flagTeam] @ ");", $flagReturnTime);
   CTF::checkMissionObjectives();
   $flagDropped = true;
   
   CTFTraining::setWayPoint($flagObject[0]);

}
function CTF::checkMissionObjectives()
{
   if(CTF::teamMissionObjectives(0))
   {
      Training::MissionComplete( 2049 );
   }
}


function CTF::checkFlagReturn(%team, %sequenceNum)
{
   dbecho(2, "checking for flag return: ", %team, ", ", %sequenceNum);
   if($pickupSequence[%team] == %sequenceNum)
   {
      messageAll(0, "The enemy flag was returned to thier base.~wflagreturn.wav");
      GameBase::startFadeOut($flagObject[%team]);
      GameBase::setPosition($flagObject[%team], $flagPosition[%team]);
      GameBase::startFadeIn($flagObject[%team]);
      $flagAtHome[%team] = true;
      CTF::checkMissionObjectives();
	  $flagDropped = false;
	  CTFTraining::setWayPoint($flagObject[0], true);
   }
}

function Flag::onCollision(%this, %object)
{
   if(Player::getClient(%object) != 2049)
   	  return;
   
   if(getObjectType(%object) == "Player")
   {
      %name = Item::getItemData(%this);
      %playerTeam = GameBase::getTeam(%object);
      %flagTeam = GameBase::getTeam(%this);
      %touchClientName = Client::getName(Player::getClient(%object));
      %enemyTeam = $enemyTeam[%playerTeam];

      dbecho(2, %name, " ", %playerTeam, " ", %flagTeam, " ", %touchClientName, " ", %enemyTeam);
      dbecho(2, $flagObject[%flagTeam], " ", $flagPosition[%flagTeam]);
      %playerClient = Player::getClient(%object);

      if(%flagTeam == %playerTeam)
      {
         // player is touching his own flag...
         if(!$flagAtHome[%playerTeam])
         {
            // the flag isn't home! so return it.
            GameBase::setPosition(%this, $flagPosition[%flagTeam]);
            $flagAtHome[%playerTeam] = true;
            bottomprint(%playerClient, "<jc><f1>You returned the " @ getTeamName(%playerTeam) @ " flag!" @ "~wflagreturn.wav", 5);
            CTF::checkMissionObjectives();
            $pickupSequence[%flagTeam] = $pickupSequence[%flagTeam] + 1;
            
         }
         else
         {
            // it's at home - see if we have the enemy flag!
            
            if(Player::getItemCount(%playerClient, "Flag") > 0)
            {
               bottomprint(%playerClient, "<jc><f1>You captured the " @ getTeamName(%enemyTeam) @ " flag!", 5);
               messageAll(%playerClient, "~wflagcapture.wav");
               $flagAtHome[%enemyTeam] = true;
               $flagCarrier[%enemyTeam] = -1;
               GameBase::setPosition($flagObject[%enemyTeam], $flagPosition[%enemyTeam]);
               Item::hide($flagObject[%enemyTeam], false);
               Player::setItemCount(%object, "Flag", 0);
               $captures++;
               CTF::checkMissionObjectives();
			   CTFTraining::setWayPoint(%this);
			   moreAI($captures);
			}
         }
      }
	  else
      {
         // it's the enemy's flag! woohoo!
         Player::setItemCount(%object, %name, 1);
         Player::mountItem(%object, %name, $FlagSlot, %flagTeam);
         Item::hide($flagObject[%enemyTeam], true);
         $flagAtHome[%enemyTeam] = false;
         
         if(!$flagDropped)
           bottomprint(%playerClient, "<jc><f1>You took the " @ getTeamName(%enemyTeam) @ " flag!", 5);
		 else
		   bottomprint(%playerClient, "<jc><f1>You recovered the " @ getTeamName(%enemyTeam) @ " flag!", 5);
         
         messageall(%playerClient, "~wflag1.wav");
         
         $flagDropped = false;
         $flagCarrier[%enemyTeam] = %object;
         CTF::checkMissionObjectives();
         $pickupSequence[%flagTeam] = $pickupSequence[%flagTeam] + 1;
		 CTFTraining::setWayPoint(%this);
      }
   }
}

function CTF::teamMissionObjectives(%teamId)
{
   %enemyTeam = $enemyTeam[%teamId];
   %teamName = getTeamName(%teamId);
     
   %enemyTeamName = getTeamName(%enemyTeam);
   %flagCarrier = $flagCarrier[%teamId];
   %enemyFlagCarrier = $flagCarrier[%enemyTeam];

 
   %capsNeeded = $capstowin - $captures;
  
   if(%capsNeeded == 0)
   {
     return "True";
   }
   Team::setObjective(%teamId, 1, "<f5><jl>GamePlay Description:");
   Team::setObjective(0, 2, "<f1><jl>    The objectives of a Capture the Flag mission are to find the enemy flag, take");  
   Team::setObjective(0, 3, "<f1><jl>    it from their base, bring it back to your base and touch it to your flag.");
   Team::setObjective(%teamId, 4, "\n");
   Team::setObjective(%teamId, 5, "<f5><jl>Mission Completion:");
   Team::setObjective(%teamId, 6, "<f1>   -3 flag captures");
   Team::setObjective(%teamId, 7, "\n");
   Team::setObjective(%teamId, 8, "<f5><jl>Mission Information:");
   Team::setObjective(%teamId, 9, "<f1>   -Mission Name: Capture the Flag Training");
   Team::setObjective(%teamId, 10, "<f1>   -Total Captures: " @ $captures);
   Team::setObjective(%teamId, 12, "\n");
   Team::setObjective(%teamId, 13, "<f5>Mission Objectives:");
   
   if($flagAtHome[%enemyTeam])
   {
      Team::setObjective(%teamId, 14,"<Bflag_enemycaptured.bmp>\n<f1>   Infiltrate the " @ %enemyTeamName @ " base and take their flag.");
   }
   else
   {
      if(%enemyFlagCarrier == -1)
      {
         Team::setObjective(%teamId, 14,"<Bflag_notatbase.bmp>\n<f1>   Enemy flag was dropped in the field.");
      }
      else
      {
         Team::setObjective(%teamId, 14,"<Bflag_atbase.bmp>\n<f1>   Return enemy flag to base for a capture.");
      }
   }
   
   
   
   return "False";
}

function Player::leaveMissionArea(%this)
{
   // if a guy leaves the area, warp the flag back to its base
   %flag = Player::getMountedItem(%this, $FlagSlot);
   if(%flag != -1)
   {
      %playerClient = Player::getClient(%this);
      bottomprint(%playerClient, "<f1><jc>You left the mission area while carrying the " @ getTeamName(1) @ " flag!", 5);
      messageAll(0, "~wshell_click.wav");
      messageAll(0, "The enemy flag was returned to thier base.~wflagreturn.wav");
      Item::hide($flagObject[1], false);
      Player::setItemCount(%this, "Flag", 0);
      $flagCarrier[0] = -1;
      $flagAtHome[1] = true;
      GameBase::setPosition($flagObject[1], $flagPosition[1]);
      ctf::checkMissionObjectives();
      CTFTraining::setWayPoint($flagObject[0]);   
   }   
}

function Game::initialMissionDrop(%clientId)
{
  GameBase::setTeam(%clientId, 0);
  Client::setGuiMode(%clientId, $GuiModePlay);
  Game::playerSpawn(%clientId, false);
  ctf::init(%clientId);
  schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>Training Mission 5 - Capture the Flag.\", 5);", 0);
  schedule("messageAll(0, \"~wshell_click.wav\");", 0);
  schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>This mission will introduce you to the capture the flag game scenario.\", 5);", 5);
  schedule("messageAll(0, \"~wshell_click.wav\");", 5);
  schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>Bring up The Objectives screen for a gameplay description.\", 5);", 10);
  schedule("messageAll(0, \"~wshell_click.wav\");", 10);
  Training::displayBitmap(0);
  CTFTraining::setWayPoint($flagObject[0], true);
  Training::setupAI(%clientId);
      
}
																									  
function missionSummary()
{
   %time = getSimTime() - $MatchStartTime;
   
   Training::displayBitmap(0);
   
   Team::setObjective(0, 1, "<f5><jl>Mission Completion:");
   Team::setObjective(0, 2, "<f1><t>   -Completed:");
   Team::setObjective(0, 3, "\n");
   Team::setObjective(0, 4, "<f5><jl>Mission Information:");
   Team::setObjective(0, 5, "<f1><t>   -Mission Name: Capture the Flag Training");
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

function moreAI(%wave)
{
  if(%wave == 1)
    %group = nameToId("MissionGroup\\AIwave1");
  else if(%wave == 2)
    %group = nameToId("MissionGroup\\AIwave2");

  if(%group == -1)
     dbecho(2, "No AI exists...");  
   
  else 
  {  
     %clientId = Client::getFirst(); 
     %AIname = "guard" @ $numGuards + 1;
     createAI(%AIname, %group, larmor, $AI_Names[floor(getRandom() * 15)]);
     %aiId = AI::getId( %AIname );
     GameBase::setTeam(%aiId, 1);
     AI::setVar( %AIname,  iq,  60 );
     AI::setVar( %AIname,  attackMode, 0);
     AI::DirectiveTarget(%AIname, %clientId);
     
  }
	
  AI::callWithId(%AIname, Player::setItemCount, disclauncher, 1);
  AI::callWithId(%AIname, Player::setItemCount, discammo, 1000);
  AI::callWithId(%AIname, Player::mountItem, disclauncher, 0);
  //AI::SetVar(%AIname, triggerPct, 0.04 );
  $numGuards++;
  
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


function remoteScoresOn(%clientId)
{
}

function remoteScoresOff(%clientId)
{
}


// set up initial objectives screen stuff.
ctf::init();
CTF::checkMissionObjectives();