exec("game.cs");
$MaxNumKills = 15;
//Deathmatch mission script -Aug. 25 1998

//calc scores upon fraging
function Game::clientKilled(%playerId, %killerId)
{
   if($teamplay)
   {
      if(%killerId == -1 || %playerId == -1)
      {
         return;
      }
      %kteam = Client::getTeam(%killerId);
      %pteam = Client::getTeam(%playerId);
      
      if(%kteam == %pteam)
         $teamScore[%kteam] = $teamScore[%kteam] - 1;
      else
         $teamScore[%kteam] = $teamScore[%kteam] - 1;
      
      DMTEAM::checkMissionObjectives();
   }
   else
      DM::checkMissionObjectives(%killerId);
   
}

function Game::playerSpawned(%pl, %clientId, %armor)
{	  
   // use this client's skin preference
   Client::setSkin(%clientId, $Client::info[%clientId, 0]);

   //spawn with only blaster and health kit
   Player::setItemCount(%clientId,$ArmorName[%armor],1);
   Player::setItemCount(%clientId,Blaster,1);
   Player::setItemCount(%clientId,RepairKit,1);
	Player::useItem(%pl,Blaster);
   
   if($teamplay)
   {
      DMTEAM::checkMissionObjectives();
      DMTEAM::echoScores();
   }
}

//Player has a total of 10 seconds per life allowed outside designated mission area.
//After a player expends this 10 sec, the player is remotely killed.
//-lesson to be learned= stay in the mission area!
function Player::leaveMissionArea(%player)
{
   %cl = Player::getClient(%player);
	Client::sendMessage(%cl,1,"You have left the mission area.");
	%player.outArea=1;
	alertPlayer(%player, 3);
}

//checking for timeout of dieSeqCount
function Player::checkLMATimeout(%player, %seqCount)
{
   echo("checking player timeout " @ %player @ " " @ %seqCount);
   if(%player.dieSeqCount == %seqCount)
      remoteKill(Player::getClient(%player));
}

//called if player leaves mission area
function Player::enterMissionArea(%player)
{
   %player.outArea="";
   %player.dieSeqCount = 0;
   %player.timeLeft = %player.timeLeft - (getSimTime() - %player.leaveTime);
}
  
function alertPlayer(%player, %count)
{
	if(%player.outArea == 1) {
		%clientId = Player::getClient(%player);
	  	Client::sendMessage(%clientId,1,"~wLeftMissionArea.wav");
		if(%count > 1)
		   schedule("alertPlayer(" @ %player @ ", " @ %count - 1 @ ");",1.5,%clientId);
		else 
	   	schedule("leaveMissionAreaDamage(" @ %clientId @ ");",1,%clientId);
	}
}

function leaveMissionAreaDamage(%client)
{
	%player = Client::getOwnedObject(%client);
	if(%player.outArea == 1) {
		if(!Player::isDead(%player)) {
		  	Player::setDamageFlash(%client,0.1);
			GameBase::setDamageLevel(%player,GameBase::getDamageLevel(%player) + 0.05);
	   	schedule("leaveMissionAreaDamage(" @ %client @ ");",1);
		}
		else { 
			playNextAnim(%client);	
			Client::onKilled(%client, %client);
		}
	}
}

function Game::checkTimeLimit()
{
   // if no timeLimit set or timeLimit set to 0,
   // just reschedule the check for a minute hence
   $timeLimitReached = false;

   if(!$Server::timeLimit)
   {
      schedule("Game::checkTimeLimit();", 60);
      return;
   }
   %curTimeLeft = ($Server::timeLimit * 60) + $missionStartTime - getSimTime();
   if(%curTimeLeft <= 0)
   {
      $timeLimitReached = true;
      $timeReached = 1;
		DM::missionObjectives();
		Server::nextMission();
	}
   else
   {
      schedule("Game::checkTimeLimit();", 20);
      UpdateClientTimes(%curTimeLeft);
   }
}

function Vote::changeMission()
{
	$timeLimitReached = true;
   $timeReached = 1;
	DM::missionObjectives();
}
  
//---------------------------------------------------------------------------------------
//
//Free for all Deathmatch function definitions
//
//---------------------------------------------------------------------------------------
function DM::checkMissionObjectives(%playerId) 
{
   if(DM::missionObjectives(%playerId)) 
      schedule("nextMission();", 0);
	if($DMScoreLimit > 0)
		if((Player::getClient(%playerId)).scoreKills >= $DMScoreLimit) {
	      $timeLimitReached = true;
   	   $timeReached = 1;
			DM::missionObjectives();
			Server::nextMission();
		}
}

function DM::missionObjectives()
{
	%numClients = getNumClients();
	for(%i = 0 ; %i < %numClients ; %i++) 
		%clientList[%i] = getClientByIndex(%i);
	%doIt = 1;
	while(%doIt == 1) {
		%doIt = "";
		for(%i= 0 ; %i < %numClients; %i++) {
			if((%clientList[%i]).ratio < (%clientList[%i+1]).ratio) {
				%hold = %clientList[%i];
				%clientList[%i] = %clientList[%i+1];
				%clientList[%i+1]	= %hold;
				%doIt=1;
			}
		}
	}
   if(!$Server::timeLimit)
      %str = "<f1>   - No time limit on the game.";
   else if($timeLimitReached)
      %str = "<f1>   - Time limit reached.";
   else
      %str = "<f1>   - Time remaining: " @ floor($Server::timeLimit - (getSimTime() - $missionStartTime) / 60) @ " minutes.";
	for(%l = -1; %l < 1 ; %l++) {		
		%lineNum = 0;
		if($timeReached == "") {
	 	  	Team::setObjective(%l, %lineNum, "<jc><B0,0:deathmatch1.bmp><B0,0:deathmatch2.bmp>");
	  		Team::setObjective(%l, %lineNum++, "<f5>Mission Information:");
			Team::setObjective(%l, %lineNum++, "<f1>   - Mission Name: " @ $missionName); 
	      Team::setObjective(%l, %lineNum++, %str);
	      Team::setObjective(%l, %lineNum++, " ");
	 	  	Team::setObjective(%l, %lineNum++, "<f5>Mission Objectives:");
	 	  	Team::setObjective(%l, %lineNum++, "<f1>   -Kill all other players!");
	 	  	Team::setObjective(%l, %lineNum++, "<f1>   -Stay alive!");
	 	  	Team::setObjective(%l, %lineNum++, "<f1>   -To have the highest Efficiency!");
	 	  	Team::setObjective(%l, %lineNum++, "<f1>   	 -Efficiency is calculated once (Kills + Deaths) is greater than 4");
	 	  	Team::setObjective(%l, %lineNum++, " ");
	 	  	Team::setObjective(%l, %lineNum++, "<f1>Remember to stay within the mission area, which is defined by the extents of your commander screen map."	@ 
	 	                                 " If you go outside of the mission area you will have 3 seconds to get back into the mission area, or you'll start taking damage!");
	 	  	Team::setObjective(%l, %lineNum++, " ");
	 	  	Team::setObjective(%l, %lineNum++, " ");
		  	Team::setObjective(%l, %lineNum++, "<f5>TOP PLAYERS ARE: " );
	 	  	Team::setObjective(%l, %lineNum++, " ");
	 	  	Team::setObjective(%l, %lineNum++, "<f1>Player Name<L30>Kills<L50>Deaths<L70>Efficiency");
	   }
	   else {
			Team::setObjective(%l, %lineNum++, "<f5>Mission Summary:");
	 	  	Team::setObjective(%l, %lineNum++, " " );
	 	  	Team::setObjective(%l, %lineNum++, "<f1>     - The Best Player(s): " );
			%i=0;
			%TopRatio="";
			while(%i < %numClients && %clientList[%i].ratio != 0 && (%TopRatio == "" || (%TopRatio ==  (%clientList[%i+1]).ratio && %TopRatio != 0) )) {
	 	  		Team::setObjective(%l, %lineNum++, "<L14><f5><Bskull_big.bmp>\n" @ Client::getName(%clientList[%i]) @ "<f1> with a ratio of <f5>" @ (%clientList[%i]).ratio @ ".0%");
				%TopRatio = (%clientList[%i]).ratio;
				%i++;
			}
			if(%i == 0) {
	 	  		Team::setObjective(%l, %lineNum++, "<L14><f1>NONE with a ratio greater than 0.0%");
			}
			Team::setObjective(%l, %lineNum++, " ");
	 	  	Team::setObjective(%l, %lineNum++, " ");
			Team::setObjective(%l, %lineNum++, "<f5>TOP PLAYERS ARE: " );
	 	  	Team::setObjective(%l, %lineNum++, " ");
			Team::setObjective(%l, %lineNum++, "<f1>Player Name<L30>Kills<L50>Deaths<L70>Efficiency");
		}
	   //print out top 5 scores
		%index = 0;
		while(%index < %numClients && %clientList[%index].ratio != 0 && (%index < 5 || (%clientList[%index].ratio == %lastRatio && %lastRatio != 0))) {
	  		%client = getClientByIndex(%count);
	  	   Team::setObjective(%l, %lineNum++,"<Bskull_small.bmp>" @ Client::getName(%clientList[%index]) @ " <L31>" @ (%clientList[%index]).scoreKills @ "<L53>" @ (%clientList[%index]).scoreDeaths @ "<L72>" @ (%clientList[%index]).ratio @ ".0%");
			%lastRatio = (%clientList[%index]).ratio;
			%index++;
		}  
		for(%s = %lineNum+1; %s < 30 ;%s++)
			Team::setObjective(%l, %s, " ");
	}
	$timeReached="";
}

//-----------------------------------------------------------------
//
//Team Deathmatch Function definitions
// 
//-----------------------------------------------------------------
function DMTEAM::echoScores()
{   
	 %score = getTeamName(0) @ ": " @ $teamScore[0];
	 
	 for(%i = 1; %i < $numTeams; %i = %i + 1)
        %score = %score @ ", " @ getTeamName(%i) @ ": " @ $teamScore[%i];
     
     MessageAll(0, %score);
     schedule("DMTEAM::echoScores();", 40);
}

function DMTEAM::checkMissionObjectives()
{
   for(%p = 0; %p < $numTeams; %p = %p + 1) 
   {
	  if(DMTEAM::teamMissionObjectives(%p))
	     schedule("Server::nextMission();", 0);
   }
}

function DMTEAM::teamMissionObjectives(%teamId)
{
   %numHighs = 0;
   %teamName = getTeamName(%teamId);
   %teamScore = $teamScore[%teamId];
   %highScore = 0;
  
   for(%t = 0; %t < $numTeams; %t = %t + 1)
   {
	   if(%teamScore > $teamScore[%t]) 
	   {
	      %highScore = %teamScore;
		  %numHighs = %numHighs + 1;
	   }
	   else if($teamScore[%t] > %highScore)
	   {
		  %highScore = $teamScore[%t];
		  %numHighs = %numHighs + 1;
	   }
   }

   if(%highScore == $ScoreLimit)
   {
       for(%r = 0; %r < $numTeams; %r = %r + 1)
       {
		  if(%teamScore == %highScore)
             Team::setObjective(%teamId, 2, "~f0Your team is victorious!");
          else if((%teamScore == %highScore) && (%numHighs > 1)) 
             Team::setObjective(%teamId, 2, "~f0Your team ended up tied for the lead!");
          else
             Team::setObjective(%teamId, 2, "~f0Your team lost!"); 
       }

       Team::setObjective(%teamId, 3, "\n");
       //print out all team scores
	   %lnum = 4;
	   for(%q = 0; %q < $numTeams; %q = %q + 1) 
	   {
       	  Team::setObjective(%teamId, %lnum, getTeamName(%q) @ ": " @ $teamScore[%q]);
		  %lnum = %lnum + 1;
       }
       return "True"; //change mission
   }
  
   Team::setObjective(%teamId, 3, "\n");
   Team::setObjective(%teamId, 4, "Mission Status:"); 													    
   
   
   if((%teamScore == %highScore) && (%highScore == 0))
      Team::setObjective(%teamId, 5, "~f0All teams tied at 0.");
   else if((%teamScore == %highScore) && (%numHighs > 1)) 
      Team::setObjective(%teamId, 5, "~f0Your team is Tied for the lead!");
   else	if((%teamScore == %highScore) && (%numHighs == 1))
      Team::setObjective(%teamId, 5, "~f0Your team is winning.");
   else
      Team::setObjective(%teamId, 5, "~f0Your team is losing.");
   
   
   Team::setObjective(%teamId, 6, "\n");
   Team::setObjective(%teamId, 7, "You must:");
   Team::setObjective(%teamId, 8, "-Kill all players on all other teams.");
   Team::setObjective(%teamId, 9, "-Stay alive!");
   Team::setObjective(%teamId, 10, "\n");
   Team::setObjective(%teamId, 11, "Remember to stay within the mission area, which is defined by the extents of your commander screen map."	@
                                  " If you go outside of the mission area you will have 10 seconds to get back into the mission area, or you will be killed!");
     							  
   Team::setObjective(%teamId, 12, "\n");
       //print out team scores
	   %lnum = 14;
	   Team::setObjective(%teamId, 13, "Team Scores");
	   for(%g = 0; %g < $numTeams; %g = %g + 1) 
	   {
       	  Team::setObjective(%teamId, %lnum, getTeamName(%g) @ ": " @ $teamScore[%g]);
		  %lnum = %lnum + 1;
       }

   return "False";
}												
function getEfficiencyRatio(%clientId)
{
	if((%clientId.scoreKills + %clientId.scoreDeaths) > 4) {
		%ratio = floor((%clientId.scoreKills/(%clientId.scoreKills + %clientId.scoreDeaths))*100);		
		return %ratio;
	}
	return "0";
}

function Game::refreshClientScore(%clientId)
{
	%clientId.ratio = getEfficiencyRatio(%clientId);
   if($teamplay)
      Client::setScore(%clientId, "%n\t%t\t" @ %clientId.score  @ "\t%p\t%l", %clientId.score);
   else
      Client::setScore(%clientId, "%n\t " @ %clientId.scoreKills @ "\t  " @ %clientId.scoreDeaths @ "\t  " @ %clientId.ratio @ ".0%\t%p\t %l", %clientId.ratio);
	DM::missionObjectives();
}


function Mission::init()
{
   setClientScoreHeading("Player Name\t\x78Team\t\xC8Score");

   $numTeams = getNumTeams();
   for(%i = 0; %i < $numTeams; %i++)
      $teamScore[%i] = 0;

   if($teamplay = !($numTeams == 1))
   {
      setTeamScoreHeading("Team Name\t\xC8Score");
      setClientScoreHeading("Player Name\t\x78Team\t\xC8Score");
   }
   else
   {
      $SensorNetworkEnabled = false;
      setTeamScoreHeading("");
      setClientScoreHeading("Player Name\t\x55Kills\t\x75Deaths\t\xA5Efficiency\t\xE3Ping\t\xFFPL");
   }
   $dieSeqCount = 0;
   //setup ai if any
   AI::setupAI();
	DM::missionObjectives();
}
