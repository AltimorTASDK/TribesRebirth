//Training_Vehicle.cs
//////////////////////////
exec("game.cs");
exec("training_AI.cs");

//Globals
//////////////////////////
$Train::missionType = "VEHICLE";
$currentLeg = 0;
//vehicle buy limits
$TeamItemMax[ScoutVehicle] = 50;
$TeamItemMax[HAPCVehicle] = 50;
$TeamItemMax[LAPCVehicle] = 50;
//crash respawn warps
$spawn[1] = "-38.9365 43.7014 114.682";
$spawn[2] = "-522.794 768.53 135.786";
$spawn[3] = "-1487.25 1201.25 115.386";


//---------------------------
//vehicle::leg1()
//---------------------------
function vehicle::leg1(%clientId)
{
	schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>In front of you is a vehicle station that is used for buying vehicles. When you step up to it, you get the Vehicle Inventory screen.\", 10);", 0);
	schedule("messageAll(0, \"~wshell_click.wav\");", 0);
	schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>Step up to the panel and the Inventory screen will apear in front of you.\", 10);", 10);
	schedule("messageAll(0, \"~wshell_click.wav\");", 10);
	schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>The right side of the menu is the available inventory you can purchase. Select the Scout from the right side menu.\", 10);", 20);
	schedule("messageAll(0, \"~wshell_click.wav\");", 20);
	schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>Click on the BUY button to purchase the Scout.  To return to the play screen, click on the X button in the upper right hand corner of the screen or hit the inventory(I) key.\", 10);", 30);
	schedule("messageAll(0, \"~wshell_click.wav\");", 30);
	schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>Once the scout is purchased, a scout vehicle will appear on the vehicle pad that is at the front of the vehicle bay. Once you buy the scout, a waypoint to the next station will be assigned to you.\", 10);", 40);
	schedule("messageAll(0, \"~wshell_click.wav\");", 40);
	schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>Jump into the Scout to pilot it.  Press the forward key to move forward.  Press the jetpack key to hover upwards. You can also use the move backward key as a brake to help stop the vehicle\", 10);", 50); 
	schedule("messageAll(0, \"~wshell_click.wav\");", 50);
	schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>Once you have arrived at the next station, walk up to the vehicle buying station there for further instructions.\", 10);", 60);
	schedule("messageAll(0, \"~wshell_click.wav\");", 60);
	schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>For an added challenge, try and maneuver through the pylons between here and the waypoint.\", 10);", 70);
	schedule("messageAll(0, \"~wshell_click.wav\");", 70);

	//limit vehicle inventory
	$VehicleInvList["ScoutVehicle"] = 1;
	$VehicleInvList["LAPCVehicle"] = 0;
	$VehicleInvList["HAPCVehicle"] = 0;
}

//-----------------------------
//vehicle::leg2()
//-----------------------------
function vehicle::leg2(%clientId)
{
	schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>How was the flight? Did you enjoy the view?\", 5);", 0);
	schedule("messageAll(0, \"~wshell_click.wav\");", 0);
	schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>Next on the list is the Light Personnel Carrier, or LPC.\", 5);", 5);
	schedule("messageAll(0, \"~wshell_click.wav\");", 5);
	schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>The LPC is a small two-man transport. It's not as fast as the Scout but a little tougher.\", 10);", 10);
	schedule("messageAll(0, \"~wshell_click.wav\");", 10);
	schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>Go ahead and purchase one and take it for a spin. As before, you must pilot the LPC to a waypoint on the map and land there.\", 10);", 20);
	schedule("messageAll(0, \"~wshell_click.wav\");", 20);

   //limit vehicle inventory	
	$VehicleInvList["ScoutVehicle"] = 0;
	$VehicleInvList["LAPCVehicle"] = 1;
	$VehicleInvList["HAPCVehicle"] = 0;
}

//--------------------------------
//vehicle::leg3()
//--------------------------------
function vehicle::leg3(%clientId)
{
  schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>On to the last vehicle, the Heavy Personnel Carrier, or HPC.\", 5);", 0);
  schedule("messageAll(0, \"~wshell_click.wav\");", 0);
  schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>Besides the driver, the HPC holds up to four tribesmen.\", 5);", 5);
  schedule("messageAll(0, \"~wshell_click.wav\");", 5);
  schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>The HPC is far slower than the LPC, but can take much more punishment.\", 10);", 10);
  schedule("messageAll(0, \"~wshell_click.wav\");", 10);
  schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>This vehicle is great for ambushes deep into enemy territory, with four Heavy Armors lobbing mortar shells from on high, making it a gunship of sorts.\", 10);", 20);
  schedule("messageAll(0, \"~wshell_click.wav\");", 20);
  schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>Just like before, pilot the HPC to the assigned waypoint. You must land on directly on the last building and exit your vehicle to end the mission.\", 10);", 30);
  schedule("messageAll(0, \"~wshell_click.wav\");", 30);
  
  //limit vehicle inventory
  $VehicleInvList["ScoutVehicle"] = 0;
  $VehicleInvList["LAPCVehicle"] = 0;
  $VehicleInvList["HAPCVehicle"] = 1;
}

//-------------------------------------
//vehicle::setWayPoint()
//-------------------------------------
function vehicle::setWayPoint(%clientid)
{
	dbecho(2,"setting up waypoints...");
	%group = nameToId("MissionGroup\\destinations");

	//flow controlled by triggers
	if($currentLeg == 0)
	{
		%obj = Group::getObject(%group, 0);
		$currentLeg++;
		ObjectiveScreen();
	}
	else if($currentLeg == 1)
	{
		%obj = Group::getObject(%group, 1);
		$currentLeg++;
		objectiveScreen();
	}
	else if($currentLeg == 2)
	{
		%obj = Group::getObject(%group, 2);
		$currentLeg++;
		objectiveScreen();
	}
	
	//find the position of the next station
	%position = GameBase::getPosition(%obj);
	%x = getWord(%position, 0);
	%y = getWord(%position, 1);
	//setway point
	issueCommand(%clientId, %clientId, 0, "Waypoint set to next vehicle station.", %x, %y);
}       


//-----------------------------------------
//GroupTrigger::onEnter()
//-----------------------------------------
function GroupTrigger::onEnter(%this, %object)
{
	if(!%this.secret) 
	{  
		objectiveScreen();
		if(getObjectType(%object) != "Player")
			return;

		%cl = Player::getClient(%object);

		if(%this.num == "Trigger1" && $currentLeg == 1)
		{
			%group = nameToId("MissionCleanup");
			for(%i = 0; (%obj = Group::getObject(%group, %i)) != -1; %i++)
			{
				dbecho(2,%obj);
				if(getObjectType(%obj) == "Flier")
					deleteObject(%obj);
			}
			Vehicle::setWayPoint(%cl);
			vehicle::leg2(%cl);
		}
		else if(%this.num == "Trigger2" && $currentLeg == 2)
		{
			%group = nameToId("MissionCleanup");
			for(%i = 0; (%obj = Group::getObject(%group, %i)) != -1; %i++)
			{
				if(getObjectType(%obj) == "Flier")
				{
					deleteObject(%obj);
				}
			}
			Vehicle::setWayPoint(%cl);
			vehicle::leg3(%cl);
		}
		else if(%this.num == "Trigger3" && $currentLeg == 3)
		{  
			bottomprint(%cl, "<jc><f2>This concludes your introduction to vehicles.", 8);
			schedule("Training::missionComplete(" @ %cl @ ");", 8);
		}
	}
	else
		griffonTown(%this, 400);
}	

//------------------------------
//Game::initialMissionDrop()
//------------------------------
function Game::initialMissionDrop(%clientId)
{
	GameBase::setTeam(%clientId, 0);
	Client::setGuiMode(%clientId, $GuiModePlay);
	Game::playerSpawn(%clientId, false);

	schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>Training Mission 3 - Introduction to vehicles\", 5);", 0);
	schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>In this training excercise, you will fly the 3 different vehicles to particular destinations assigned to you by waypoints.\", 10);", 5);
	schedule("messageAll(0, \"~wshell_click.wav\");", 0);
	schedule("messageAll(0, \"~wshell_click.wav\");", 5);
	schedule("vehicle::leg1(" @ %clientId @ ");", 15);
	$VehicleInvList["ScoutVehicle"] = 1;
	$VehicleInvList["LAPCVehicle"] = 0;
	$VehicleInvList["HAPCVehicle"] = 0;
	schedule("vehicle::setWayPoint(" @ %clientId @ ");", 0);
}

//--------------------------------
//Vehicle::onDestroyed()
//--------------------------------
function Vehicle::onDestroyed (%this,%mom)
{
	%clientId = "2049";
	$TeamItemCount[GameBase::getTeam(%this) @ $VehicleToItem[GameBase::getDataName(%this)]]--;
   %cl = GameBase::getControlClient(%this);
	%pl = Client::getOwnedObject(%cl);
	if(%pl != -1) {
	   Player::setMountObject(%pl, -1, 0);
   	Client::setControlObject(%cl, %pl);
	}
	calcRadiusDamage(%this, $DebrisDamageType, 2.5, 0.05, 25, 13, 2, 0.55, 
		0.1, 225, 100);
		
   bottomprint(%clientId, "<f1><jc>Lets try that again.", 5); 
	%position = $spawn[$currentLeg];
	GameBase::setPosition( 2049, %position);
}

//----------------------------------
//ObjectiveScreen()
//----------------------------------
function ObjectiveScreen()
{
   %time = getSimTime() - $MatchStartTime;
   
   Training::displayBitmap(0);
   Team::setObjective(0, 1, "<f5><jl>Mission Completion:");
   Team::setObjective(0, 2, "<f1>   -Travel to all vehicle stations that are assigned");
   Team::setObjective(0, 3, "\n");
   Team::setObjective(0, 4, "<f5><jl>Mission Information:");
   Team::setObjective(0, 5, "<f1>   -Mission Name: Introduction to vehicles");
   Team::setObjective(0, 6, "\n");
   
   Team::setObjective(0, 7, "<f5><j1>Mission Objectives:");
   Team::setObjective(0, 8, "<f1>   -Current station waypoint: " @ $currentLeg);
   
   Team::setObjective(0, 9, "\n");
   Team::setObjective(0, 10, "\n");
   Team::setObjective(0, 11, "\n");
   Team::setObjective(0, 12, "\n");
   Team::setObjective(0, 13, "\n");
}

//------------------------------
//MissionSummary()
//------------------------------
function missionSummary()
{
	%time = getSimTime() - $MatchStartTime;

	Training::displayBitmap(0);
	Team::setObjective(0, 1, "<f5><jl>Mission Completion:");
	Team::setObjective(0, 2, "<f1>   -Completed:");
	Team::setObjective(0, 3, "\n");
	Team::setObjective(0, 4, "<f5><jl>Mission Information:");
	Team::setObjective(0, 5, "<f1>   -Mission Name: Introduction to vehicles");
	Team::setObjective(0, 6, "\n");

	Team::setObjective(0, 7, "<f5><j1>Mission Summary:");

	Team::setObjective(0, 8, "<f1>   -Total Mission Time: " @ "<f1>" @ Time::getMinutes(%time) @ " Minutes " @ Time::getSeconds(%time) @ " Seconds");
	Team::setObjective(0, 9, "\n");
	Team::setObjective(0, 10, "\n");
	Team::setObjective(0, 11, "\n");
	Team::setObjective(0, 12, "\n");
	Team::setObjective(0, 13, "\n");
	Team::setObjective(0, 14, "\n");
}


//---------------------------------------
//griffonTown() -can you find this place?
//---------------------------------------
function griffonTown(%this, %time)
{
   dbecho(2,"time = " @  getSimTime() - $MatchStartTime);
   if(getSimTime() - $MatchStartTime > %time)
   {   
      %positionIn = "-2847.21 -2254.32 -16.137";
      %positionOut = "-38.9356 43.7005 114.673";
      if(%this.in)
      {
         GameBase::setPosition( 2049, %positionIn);
      	messageAll(0, "~wshieldhit.wav");
         centerprint(2049, "<f1><jc>Welcome to GriffonTown.  Jett, Skeet, and Sne/\\ker's home away from home!", 10);
      	schedule("centerprint(2049, \"<f1><jc>Make yourself at home, and to get back to your primary goal, jump into the tunnel that is in front of you.\", 10);", 10);
      }
      else
      {
         GameBase::setPosition( 2049, %positionOut);
      	messageAll(0, "~wshieldhit.wav");
      }
   }
   else
      //please leave me alone until I am done!
      messageAll(0, "I'm busy......");

}

//-----------------------------
//StaticShape::onDamage()
//-----------------------------
function StaticShape::onDamage(%this,%type,%value,%pos,%vec,%mom,%object)
{
	%shape = GameBase::getDataName(%this);
	if(%shape.className != Vehicle)
	   return;
	
	%damageLevel = GameBase::getDamageLevel(%this);
	%TDS= 1;
	if(GameBase::getTeam(%this) == GameBase::getTeam(%object))
		%TDS = $Server::TeamDamageScale;
	GameBase::setDamageLevel(%this,%damageLevel + %value * %TDS);
   %this.lastDamageObject = %object;
   %this.lastDamageTeam = GameBase::getTeam(%object);
}

//----------------------------
//Training::MissionComplete
//----------------------------
function Training::missionComplete(%cl)
{
  Client::setGuiMode(%cl, $GuiModeObjectives);
  missionSummary();
  remoteEval(2049, TrainingEndMission);
}

//---------------------------
//TrainEndMission()
//---------------------------
function remoteTrainingEndMission()
{
   schedule("EndGame();", 8);
}

//do nothing functions
function Player::onDamage(%this,%type,%value,%pos,%vec,%mom,%vertPos,%quadrant,%object)
{
}

function remoteScoresOn(%clientId)
{
}

function remoteScoresOff(%clientId)
{
}

