//Training_Defend_Destroy.cs

exec("game.cs");
exec("Training_AI.cs");
$Train::missionType = "D&D";

function Game::initialMissionDrop(%clientId)
{
   GameBase::setTeam(%clientId, 0);
   Client::setGuiMode(%clientId, $GuiModePlay);
   Game::playerSpawn(%clientId, false);
   Training::displayBitmap(0);
   Defend::initTargets(%clientId);
   Training::setupAI( %clientId );
}

function Defend::initTargets(%cl)
{
   $numCompleted = 0;
   $TrainMissionComplete = false;
   %group = nameToId("MissionGroup\\Teams\\Team1\\base");
   %numObj = Group::objectCount(%group);
   $numTargets = 0;
   %lineNum = 14;
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
         Defend::checkMissionObjectives(%cl, %Obj);
      }
   }
   Defend::introduction(%cl);
}

function Defend::introduction(%cl)
{
   schedule("bottomprint(" @ %cl @ ", \"<f1><jc>Training Mission 8 - Defend and Destroy.\", 5);", 0);
   schedule("messageAll(0, \"~wshell_click.wav\");", 0);
   schedule("bottomprint(" @ %cl @ ", \"<f1><jc>This is your indoctrination to a Defend and Destroy mission.\", 5);", 5);
   schedule("messageAll(0, \"~wshell_click.wav\");", 5);
   schedule("bottomprint(" @ %cl @ ", \"<f1><jc>Press O to bring up the Objectives screen for a gameplay description and a list of targets.\", 5);", 10);
   schedule("messageAll(0, \"~wshell_click.wav\");", 10);
   Defend::setWayPoint(%cl);
   $init = false;
}  

function checkTargetsDestroyed(%this)
{
   %cl = Client::getFirst();
   Defend::checkMissionObjectives(%cl, %this);
   
   if(!$TrainMissionComplete)
      Defend::setWayPoint(%cl, %this);
   
}

function StaticShape::objectiveDestroyed(%this)
{
   for(%i = 0; %i < $numTargets; %i++)
   {
      if(%this == $targets[%i])
      {
         $numCompleted++;
         $completed[%i] = true;
         %cl = Client::getFirst(); 
         %this.destroyed = true;
         bottomprint(%cl, "<f1><jc>You have destroyed " @ %this.objective, 5);
         messageAll(0, "~wshell_click.wav");
      }
   }
   checkTargetsDestroyed(%this);
}

function Defend::setWayPoint(%cl)
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
      %delay = 15;
   else 
      %delay = 5;
   
   schedule("bottomprint(" @ %cl @ ", \"<jc><f1>Waypoint set to " @ %target.objective @ "\", 5);", %delay);
   schedule("messageAll(0, \"~wshell_click.wav\");", %deley);
   issueCommand(%cl, %cl, 0, "Waypoint set to next objective", %x, %y);
   $init = false;
}


function Defend::checkMissionObjectives(%cl, %shape)
{
   if(Defend::MissionObjectives( Client::getTeam(%cl), %cl, %shape ))
   {
      Defend::MissionComplete( %cl );
   }
}

function Defend::MissionObjectives( %teamId, %cl, %this )
{
   %teamName = getTeamName(%teamId); 
   %player = Client::getOwnedObject(%cl);
   %enemyTeamName = getTeamName(%enemyTeam);
  
   if($numCompleted == $numTargets)
   {
     $TrainMissionComplete = true;
     return "True";
   }
   
   Team::setObjective(%teamId, 1, "<f5><jl>GamePlay Description:");
   Team::setObjective(0, 2, "<f1><jl>    In a Defend and Destroy mission, both teams have objectives");  
   Team::setObjective(0, 3, "<f1><jl>    that must be destroyed, such as a generator or other object.");
   Team::setObjective(0, 4, "<f1><jl>    Each team must successfully defend their base while attempting");
   Team::setObjective(0, 5, "<f1><jl>    to infiltrate their enemy's base and destroying their targets. Points are given");
   Team::setObjective(0, 6, "<f1><jl>    for each enemy objective destroyed and for protecting your objectives.");
   Team::setObjective(%teamId, 7, "\n");
   Team::setObjective(%teamId, 8, "<f5><jl>Mission Completion:");
   Team::setObjective(%teamId, 9, "<f1>   -Destroy all enemy generators");
   Team::setObjective(%teamId, 10, "\n");
   Team::setObjective(%teamId, 11, "<f5>Mission Information:");
   Team::setObjective(%teamId, 12, "<f1>   -Mission Name: Defend and Destroy Training");
   Team::setObjective(%teamId, 13, "\n"); 
   Team::setObjective(%teamId, 14, "<f5><jl>Mission Obejectives:");
   
   if(%this.destroyed)
   {
      %status = "<f1><Bitem_ok.bmp>\n    You successfully destroyed the " @ %this.objective;
   }
   else
   {
      %status = "<f1><Bitem_damaged.bmp>\n    You have not destroyed the " @ %this.objective;
   }

   if(%this.lineNum == 15)
      Team::setObjective(%teamId, %this.lineNum, %status);
   else if(%this.lineNum == 16)
      Team::setObjective(%teamId, %this.lineNum, %status);
   else if(%this.lineNum == 17)
      Team::setObjective(%teamId, %this.lineNum, %status);

   
    
   return "False";
}

function missionSummary()
{
   %time = getSimTime() - $MatchStartTime;
   
   Training::displayBitmap(0);
   Team::setObjective(0, 1, "<f5><jl>Mission Completion:");
   Team::setObjective(0, 2, "<f1><t>   -Completed:");
   Team::setObjective(0, 3, "\n");
   Team::setObjective(0, 4, "<f5><jl>Mission Information:");
   Team::setObjective(0, 5, "<f1><t>   -Mission Name: Defend and Destroy Training");
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


function Defend::missionComplete(%cl)
{
  
  schedule("Client::setGuiMode(" @ %cl @ ", " @ $GuiModeObjectives @ ");", 8);
  missionSummary();
  remoteEval(2049, TrainingEndMission);
}

function remoteTrainingEndMission()
{
   schedule("EndGame();", 16);
}


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


