//Training_welcome.cs
//-------------------------------------
exec("game.cs");
exec("Training_AI.cs");

//Globals
//////////////////////
$Train::missionType = "WELCOME";

//--------------------------------
//TrainingWelcome::start()
//--------------------------------
function TrainingWelcome::Start(%playerId)
{
   //this mess will give us sensor ping
   %group = "MissionGroup\\teams\\team1\\sensor";  
   %sensor = Group::getObject(%group, 0);  
   %sensor.origPos = "\"808.601 -196.23 99.1342\"";
   %sensor.pingPos = "\"-16.8637 -28.1507 155.878\"";
   %position = "-16.8635 -28.1505 155.876";
   
   %x = getWord(%position, 0);
   %y = getWord(%position, 1);
   GameBase::startFadeOut(%sensor);
   
   //display the proper bitmap in our objective screen
   Training::displayBitmap(0);
   
   //this needs to be redone. Patch?
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>This mission will provide instruction on basic player movement and your heads-up display.\", 10);", 5);
   schedule("messageAll(0, \"~wshell_click.wav\");", 5);
   
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>Let's get started.....\", 5);", 10);
   schedule("messageAll(0, \"~wshell_click.wav\");", 10);
   
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>We will start with the heads-up display, or HUD.\", 10);", 15);
   schedule("messageAll(0, \"~wshell_click.wav\");", 15); 
   
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>In the upper left hand corner of the screen there are two meters.  The green meter is your armor meter, the blue is your energy meter.\", 20);", 25);
   schedule("messageAll(0, \"~wshell_click.wav\");", 25);
   schedule("flashIcon(\"healthHud\", 20, 0.5);", 25);
   schedule("flashIcon(\"jetPackHud\", 20, 0.5);", 25);
   
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>The green meter displays the current status of your armor. When it falls to zero, you will die. The blue meter displays your current energy reserves. When it falls to zero, your jump jets and all equipment requiring energy will not function.\", y);", 45);
   schedule("messageAll(0, \"~wshell_click.wav\");", 45);
   schedule("flashIcon(\"healthHud\", 10, 0.5);", 45);
   schedule("flashIcon(\"jetPackHud\", 10, 0.5);", 55);
   
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>In the upper right-hand corner of the screen is your compass. This is a versatile feature for setting and finding waypoints in the vast landscapes of the game.\", 20);", 65);
   schedule("messageAll(0, \"~wshell_click.wav\");", 65);
   schedule("flashIcon(\"compassHud\", 20, 0.5);", 65);
   
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>When you have been assigned a waypoint, a solid triangle will appear in the middle of your compass to guide you to your destination. The distance to the waypoint is displayed in meters.\", 20);", 85);
   schedule("messageAll(0, \"~wshell_click.wav\");", 85);
   schedule("issueCommand(" @ %playerId @ ", " @ %playerId @ ", 0, \"Waypoint set to tower switch.\", " @ %x @ ", " @ %y @ ");", 85);
   
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>The red line on the compass shows you which direction you are heading. To get to the waypoint, line the triangle up with the red line and move in that direction.\", 20);", 105);
   schedule("messageAll(0, \"~wshell_click.wav\");", 105);
   
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>At the bottom left of your screen is your Weapon/Pack Bar, displaying the weapons and pack that you presently possess.\", 20);", 125);
   schedule("messageAll(0, \"~wshell_click.wav\");", 125);
   
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>  The number next to the weapon icons displays how much ammo remains for that weapon. If the weapon uses energy for ammo, the infinity symbol replaces the number.\", 20);", 145);
   schedule("messageAll(0, \"~wshell_click.wav\");", 145);
   schedule("flashIcon(\"weaponHud\", 20, 0.5);", 125);
   
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>The weapon you are currently using will be highlighted  Press 1 to bring up your blaster, your most basic weapon.\", 20);", 165);
   schedule("messageAll(0, \"~wshell_click.wav\");", 165);
   
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>At the top of the screen is the chat display. Messages from teammates and mission updates will be displayed here.\", 20);", 185);
   schedule("messageAll(0, \"~wshell_click.wav\");", 185);
   schedule("flashIcon(\"chatDisplayHud\", 20, 0.5);", 185);
   
   schedule("control::setVisible(\"sensorHUD\", true);", 204);
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>The small radar dish icon in the upper right hand corner of the screen is the Sensor icon. When you are being scanned by enemy sensors, it will flash red.\", 20);", 205);
   schedule("messageAll(0, \"~wshell_click.wav\");", 205);
   schedule("GameBase::setPosition(" @ %sensor @ ", " @ %sensor.pingPos @ ");", 205);
   schedule("flashIcon(\"sensorHUD\", 20, 0.5);", 204);
  
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>This concludes the HUD Tutorial. If you would like to adjust the position of any of the icons that are part of your HUD, in other missions you may use the K key to drag them to other locations, and toggle them on or off.\", 20);", 225);
   schedule("messageAll(0, \"~wshell_click.wav\");", 225);
   schedule("GameBase::setPosition(" @ %sensor @ ", " @ %sensor.origPos @ ");", 225);
   
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>Using your jump jets is one of the most important things to learn. The default key for jetting is the right mouse button. You can climb to a fairly high altitude, but you will take damage when trying to land from a great height.\", 20);", 245);
   schedule("messageAll(0, \"~wshell_click.wav\");", 245);
   
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>To run forward, use the W key. To sidestep left, use the A key.  To sidestep right, use the D key. To jump, use the space bar.\", 20);", 265);
   schedule("messageAll(0, \"~wshell_click.wav\");", 265);
   
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>Familiarize yourself with the movement keys. When you are ready, jet your way up to the highest platform arranged around your base and hit the switch to end this introduction.\", 20);", 285);
   schedule("messageAll(0, \"~wshell_click.wav\");", 285);															   
}

//----------------------------------------
//TowerSwitch::onCollision()
//---------------------------------------
function TowerSwitch::onCollision(%this, %object)
{
	%playerId = Player::getClient(%object);
	messageAll(0, "~wCapturedTower.wav");
	bottomprint(%playerId, "<f1><jc>You have completed the introduction!", 8);
	messageAll(0, "~wshell_click.wav");
	schedule("Training::missionComplete(" @ %playerId @ ");", 8);
}

//-----------------------------------------
//Game::playerSpawned()
//-----------------------------------------
function Game::playerSpawned(%pl, %clientId, %armor, %respawn)
{
   Player::setItemCount(%clientId,$ArmorName[%armor],1);
   Player::setItemCount(%clientId,Blaster,1);
   Player::setItemCount(%clientId,Chaingun,1);
   Player::setItemCount(%clientId,DiscLauncher,1);
   Player::setItemCount(%clientId,Plasmagun,1);
   Player::setItemCount(%clientId,Mortar,1);
   Player::setItemCount(%clientId,grenadelauncher,1); 
   Player::setItemCount(%clientId,Energyrifle,1);
   Player::setItemCount(%clientId,DiscLauncher,1);
   Player::setItemCount(%clientId,laserrifle,1);
}

//----------------------------------------
//Game::initialMissionDrop()
//---------------------------------------
function Game::initialMissionDrop(%clientId)
{
	GameBase::setTeam(%clientId, 0);
	Client::setGuiMode(%clientId, $GuiModePlay);
	Game::playerSpawn(%clientId, false);

	schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>Training Mission 1 - Introduction and basic player movement.\", 5);", 0);
	schedule("messageAll(0, \"~wshell_click.wav\");", 0);
	schedule("welcome::checkIcons();", 3);
	ObjectiveScreen();
	TrainingWelcome::Start(%clientId);
}

//----------------------------------------
//ObjectiveScreen()
//----------------------------------------
function ObjectiveScreen()
{
   %time = getSimTime() - $MatchStartTime;
   
   Training::displayBitmap(0);
   Team::setObjective(0, 1, "<f5><jl>Mission Completion:");
   Team::setObjective(0, 2, "<f1>   -Collision with control switch");
   Team::setObjective(0, 3, "\n");
   Team::setObjective(0, 4, "<f5><jl>Mission Information:");
   Team::setObjective(0, 5, "<f1>   -Mission Name: Introduction to TRIBES");
   Team::setObjective(0, 6, "\n");
   
   Team::setObjective(0, 7, "<f5><j1>Mission Objectives:");
   Team::setObjective(0, 8, "<f1>   -Learn the main game screen icons and basic player movement.");
   Team::setObjective(0, 9, "\n");
   
   Team::setObjective(0, 10, "\n");
   Team::setObjective(0, 11, "\n");
   Team::setObjective(0, 12, "\n");
   Team::setObjective(0, 13, "\n");
   Team::setObjective(0, 14, "\n");
}


//------------------------------------------
//flashIcon()
//------------------------------------------
function flashIcon(%icon, %time, %increment)
{
	%off = "true";
	for(%i = 0; %i <= %time; %i = %i + %increment)
	{
		if(%off)
		{   
			schedule("control::setVisible(" @ %icon @ ", " @ %off @ ");", %i);
			%off = "false";
		}
		else
		{
			schedule("control::setVisible(" @ %icon @ ", " @ %off @ ");", %i);
			%off = "true";
		}
	}
}

//------------------------------------
//missionSummary()
//------------------------------------
function missionSummary()
{
   %time = getSimTime() - $MatchStartTime;
   Training::displayBitmap(0);
   Team::setObjective(0, 1, "<f5><jl>Mission Completion:");
   Team::setObjective(0, 2, "<f1>   -Completed:");
   Team::setObjective(0, 3, "\n");
   
   Team::setObjective(0, 4, "<f5><jl>Mission Information:");
   Team::setObjective(0, 5, "<f1>   -Mission Name: Introduction to TRIBES");
   Team::setObjective(0, 6, "\n");
   
   Team::setObjective(0, 7, "<f5><j1>Mission Objectives:");
   Team::setObjective(0, 8, "<f1>   -Total Mission Time: " @ "<f1>" @ Time::getMinutes(%time) @ " Minutes " @ Time::getSeconds(%time) @ " Seconds");
   Team::setObjective(0, 9, "\n");
   
   Team::setObjective(0, 10, "\n");
   Team::setObjective(0, 11, "\n");
   Team::setObjective(0, 12, "\n");
   Team::setObjective(0, 13, "\n");
   Team::setObjective(0, 14, "\n");
}

//------------------------------------
//Training::missionComplete()
//------------------------------------
function Training::missionComplete(%cl)
{
	schedule("Client::setGuiMode(" @ %cl @ ", " @ $GuiModeObjectives @ ");", 0);
	missionSummary();
	remoteEval(2049, TrainingEndMission);
}

//------------------------------------
//remoteTrainingEndMission()
//------------------------------------
function remoteTrainingEndMission()
{
   schedule("EndGame();", 8);
}

//------------------------------
//Welcome::CheckIcon()
//------------------------------
function Welcome::CheckIcons()
{
   $AI_SaveHealth = Control::getVisible("healthHud");
   $AI_SaveJett = Control::getVisible("jetPackHud");
   $AI_SaveWeapons = Control::getVisible("weaponHud");
   $AI_SaveCompass = Control::getVisible("compassHud");
   $AI_SaveChat = Control::getVisible("chatDisplayHud");
   $AI_SavePing = Control::getVisible("sensorHUD");
}

//do nothing functions
function remoteScoresOn(%clientId)
{
}

function remoteScoresOff(%clientId)
{
}