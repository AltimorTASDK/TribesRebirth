//Weapons-FixedTarget Training
//--------------------------------------
//all these functions do is set up the scene for each weapon.
//upon killing each target assigned, the next weapon lesson will resume.
exec("game.cs");
exec("Training_AI.cs");

//Globals
//////////////
$Train::missionType = "WEAPONS";
$counter = 0;
$AIkilled = 0;

//-------------------------------------
//fixedTargetsBlaster()
//-------------------------------------
function fixedTargetsBlaster(%playerId)
{   
   //weapon discription
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>This mission will introduce you to the many weapons that are available in TRIBES.\", 10);", 5);
   schedule("messageAll(0, \"~wshell_click.wav\");", 5);
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>Ready?\", 10);", 15);
   schedule("messageAll(0, \"~wshell_click.wav\");", 15);
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>Let's start with the blaster.  This is an energy weapon which all armor classes can carry.  It is a good close- to medium-range weapon that feeds off of your energy supply.\", 10);", 25);
   schedule("messageAll(0, \"~wshell_click.wav\");", 25);
   
   //mount weapons
   schedule("Player::setItemCount(" @ %playerId @ ",Blaster, 1);", 25);
   schedule("Player::mountItem(" @ %playerId @ ", Blaster, 0);", 25);
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>Time to do some shooting!  Somewhere around you a player is trying to infiltrate your base.  Eliminate him with the blaster.\", 10);", 35);
   schedule("messageAll(0, \"~wshell_click.wav\");", 35);
   
   //drone setup
   %name = "Nailz";
   %group = "\"MissionGroup\\\\AI\\\\guard1\"";
   schedule("initAI(" @ %name @ ", " @ %group @ ", marmor);", 35);
   schedule("setTarget(" @ %playerId @ ", " @ %name @ ");", 35);
   $counter++;
}

//----------------------------------
//fixedTargetsPlasma()
//----------------------------------
function fixedTargetsPlasma(%playerId)
{   
   //discription of weapon
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>Next is the plasma gun.\", 10);", 10);
   schedule("messageAll(0, \"~wshell_click.wav\");", 10);
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>The plasma gun has about the same range as the blaster, but far more punch.  This weapon is a good choice against slower heavy armors because of its splash damage radius. \", 10);", 20);
   schedule("messageAll(0, \"~wshell_click.wav\");", 20);
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>Let's take out a heavy armor and see how you like it.\", 10);",30);
   schedule("messageAll(0, \"~wshell_click.wav\");", 30);
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>Once again, an enemy tribesman has infiltrated your base, and it is up to you to dispose of the threat.\", 10);", 40);
   schedule("messageAll(0, \"~wshell_click.wav\");", 40);
   
   //mount weapon
   schedule("Player::setItemCount(" @ %playerId @ ",Blaster, 0);", 12);
   schedule("Player::setItemCount(" @ %playerId @ ",Plasmagun, 1);", 12);
   schedule("Player::setItemCount(" @ %playerId @ ",PlasmaAmmo, 100);", 45);
   schedule("Player::mountItem(" @ %playerId @ ", Plasmagun, 0);", 12);
   
   //drone details
   %name = "SymLink";
   %group = "\"MissionGroup\\\\AI\\\\guard2\"";
   schedule("initAI(" @ %name @ " ," @  %group @ " , harmor);", 45);
   schedule("setTarget(" @ %playerId @ ", " @ %name @ ");", 45);
   $counter++;
}

//--------------------------------------
//fixedTargetsChain()
//--------------------------------------
function fixedTargetsChain(%playerId)
{   
   //weapon discription
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>Next, the chain gun.  Capable of churning out explosive flechettes in rapid order, it can chew up even heavy armors in short order.\", 10);", 10);
   schedule("messageAll(0, \"~wshell_click.wav\");", 10);
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>It can be carried by all armor classes.  Deadly at short range, it loses much of its effectiveness at greater distances.\", 10);", 20);
   schedule("messageAll(0, \"~wshell_click.wav\");", 20);
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>Let's fire a few rounds and get an idea of the range of the chain gun. Try and take out the target that is crossing your path below your base tower.\", 10);", 30);
   schedule("messageAll(0, \"~wshell_click.wav\");", 30);

   //weapon mounting
   schedule("Player::setItemCount(" @ %playerId @ ",Chaingun, 1);", 12);
   schedule("Player::setItemCount(" @ %playerId @ ",bulletAmmo, 10000);", 35);
   schedule("Player::setItemCount(" @ %playerId @ ",disclauncher, 0);", 12);
   schedule("Player::mountItem(" @ %playerId @ ", Chaingun, 0);", 12);
   
   //drone stuff
   %name = "Slacker";
   %group = "\"MissionGroup\\\\AI\\\\guard4\"";
   schedule("initAI(" @ %name @ " ," @  %group @ " , marmor);", 35);
   schedule("setTarget(" @ %playerId @ ", " @ %name @ ");", 35);
   $counter++;
}
    
//-----------------------------------
//fixedTargetsdisc()
//-----------------------------------
function fixedTargetsdisc(%playerId)
{   
   //discribe the weapon
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>The next weapon available to you is the disk launcher.\", 10);", 10);
   schedule("messageAll(0, \"~wshell_click.wav\");", 10);
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>This is definitely a weapon that you want to master.  Little else in tribal arsenals can match the damage dealt by its magnetic spinfusor disks. This weapon also has a wide splash radius, so aim at the feet of your enemy for maximum effectivness.\", 10);", 20);
   schedule("messageAll(0, \"~wshell_click.wav\");", 20);
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>The best way to get a feel for this weapon is to shoot some discs.  See if you can find an enemy tribesman wandering around your tower, and take 'em out!\", 10);", 30);
   schedule("messageAll(0, \"~wshell_click.wav\");", 30);
   
   //mount the correct weapon
   schedule("Player::setItemCount(" @ %playerId @ ",Disclauncher, 1);", 12);
   schedule("Player::setItemCount(" @ %playerId @ ",Plasmagun, 0);", 12);
   schedule("Player::setItemCount(" @ %playerId @ ",DiscAmmo, 100);", 32);
   schedule("Player::mountItem(" @ %playerId @ ", Disclauncher, 0);", 12);
   
   //create a drone for them to shoot 
   %name = "\"Cornboy!\"";
   %group = "\"MissionGroup\\\\AI\\\\guard3\"";
   schedule("initAI(" @ %name @ " ," @  %group @ " , harmor);", 35);
   schedule("setTarget(" @ %playerId @ ", " @ %name @ ");", 35);
   $counter++;
}

//-----------------------------------
//fixedTargetsSniper
//-----------------------------------
function fixedTargetsSniper(%playerId)
{   
   //discribe weapon
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>For long-range sniping, nothing beats the laser rifle.  A powerful energy-based weapon, it can hit a target kilometers away!\", 10);", 5);
   schedule("messageAll(0, \"~wshell_click.wav\");", 5);
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>Because of its high energy consumption, you must also have an energy pack to use this weapon and it can only be used by light armors.\", 10);", 15);
   schedule("messageAll(0, \"~wshell_click.wav\");", 10);
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>The zoom feature enhances the value of the laser rifle immensely. The default zoom key is E. To change the magnification, press the Z key.  You can change the magnification to two, five, ten or even twenty times normal vision!\", 10);", 25);
   schedule("messageAll(0, \"~wshell_click.wav\");", 25);
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>Since a head shot deals more damage than a shot to other parts of the body, being able to use the zoom feature quickly and accurately can make the difference between killing or being killed.\", 10);", 35);
   schedule("messageAll(0, \"~wshell_click.wav\");", 35);
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>Zoom in on an enemy tribesmen and take him out. Use your compass to locate the enemy to the north of you, then take him our with the laser rifle.\", 10);", 45);
   schedule("messageAll(0, \"~wshell_click.wav\");", 45);

   //mount weapon
   schedule("Player::setItemCount(" @ %playerId @ ",chaingun, 0);", 8);
   schedule("Player::setItemCount(" @ %playerId @ ",laserrifle, 1);", 8);
   schedule("Player::setItemCount(" @ %playerId @ ",EnergyPack, 1);", 8);
   schedule("Player::mountItem(" @ %playerId @ ",EnergyPack, 1);", 8);
   schedule("Player::mountItem(" @ %playerId @ ", laserrifle, 0);", 8);

   //setup target to shoot
   %group = "\"MissionGroup\\\\AI\\\\guard5\"";
   %name = "\"GotM*lk?\"";
   schedule("initAI(" @ %name @ " ," @  %group @ " , marmor);", 45);
   schedule("setTarget(" @ %playerId @ ", " @ %name @ ");", 45);
   $counter++;

}

//---------------------------------------
//fixedTargetsGrenade()
//---------------------------------------
function fixedTargetsGrenade(%playerId)
{   
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>On to a more explosive weapon!  Let's take a look at the grenade launcher.  It can be carried by all armor classes.\", 10);", 5);
   schedule("messageAll(0, \"~wshell_click.wav\");", 5);
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>Grenades launched from the launcher have a timed fuse, exploding a few seconds after impact.  To clear out the inside of a bunker, just lob a couple of grenades insde and watch the enemy come running out.\", 10);", 15);
   schedule("messageAll(0, \"~wshell_click.wav\");", 15);
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>An enemy tribesman is thinking of trying to hide in a bunker.  Time to teach him the error of his ways.\", 10);", 25);
   schedule("messageAll(0, \"~wshell_click.wav\");", 25);
   schedule("bottomprint(" @ %playerId @ ", \"<f1><jc>Watch the bunker at the bottom of your base tower, and fire a couple of grenades when you see the enemy tribesman run inside.\", 10);", 35);
   schedule("messageAll(0, \"~wshell_click.wav\");", 35);

   //mount weapon
   schedule("Player::setItemCount(" @ %playerId @ ",grenadeAmmo, 50);", 40);
   schedule("Player::setItemCount(" @ %playerId @ ",laserrifle, 0);", 8);
   schedule("Player::setItemCount(" @ %playerId @ ",grenadelauncher, 1);", 8);
   schedule("Player::mountItem(" @ %playerId @ ", grenadelauncher, 0);", 8);

   //setup target to shoot
   %group1 = "\"MissionGroup\\\\AI\\\\guard6\"";
   %name1 = "Skeet";
   schedule("initAI(" @ %name1 @ " ," @  %group1 @ " , harmor);", 40);
   schedule("setTarget(" @ %playerId @ ", " @ %name1 @ ");", 40);
   $counter++;
}

//-------------------------------
//weaponsStart()
//-------------------------------
function weaponsStart(%clientId)
{
  fixedTargetsBlaster(%clientId);
}

//-------------------------------
//initAi()
//-------------------------------
function initAI(%name, %group, %armor)
{
  createAI(%name, %group, %armor, %name);
}

//-------------------------------
//Game:playerSpawned()
//-------------------------------
function Game::playerSpawned(%pl, %clientId, %armor)
{
   replenishAmmo();
} 

//--------------------------------
//AI::onDroneKilled()
//--------------------------------
function AI::onDroneKilled(%aiName)
{
  %clientId = 2049;
  //kill thy vehicle creater!
  if(%aiName == "Nailz")
  {
    fixedTargetsPlasma(%clientId);
	bottomprint(%clientId, "<f1><jc>Good Job!", 5);
	$AIkilled++;
	ObjectiveScreen();
  }
  //symlink? hmmm enjoy it cuz it won't happen often. Killing him that is.
  else if(%aiName == "SymLink")
  {
    fixedTargetsDisc(%clientId);
	bottomprint(%clientId, "<f1><jc>Good Job!", 5);
	$AIkilled++;
	ObjectiveScreen();
  }
  //kill thy mission building God!
  else if(%aiName == "Cornboy!")
  {
    fixedTargetsChain(%clientId);
	bottomprint(%clientId, "<f1><jc>Good Job!", 5);
	$AIkilled++;
	ObjectiveScreen();
  }
  //kill thy master of the engine.
  else if(%aiName == "Slacker")
  {
    fixedTargetsSniper(%clientId);
	bottomprint(%clientId, "<f1><jc>Good Job!", 5);
	$AIkilled++;
	ObjectiveScreen();
  }
  //one word, 9 lives! Make every shot count.
  else if(%aiName == "GotM*lk?")
  {
    fixedTargetsGrenade(%clientId);
	bottomprint(%clientId, "<f1><jc>Good Job!", 5);
	$AIkilled++;
	ObjectiveScreen();
  }
  //kill thy compiler of the Gods!
  else if(%aiName == "Skeet")
  {
    endMission(%clientId);
	bottomprint(%clientId, "<f1><jc>Good Job!", 5);
	$AIkilled++;
  }
}  

//----------------------------------
//endMission()
//----------------------------------
function endMission(%clientId)
{
  %name = Client::getName(%clientId);
  %String = "\"<f1><jc>Way to go " @ %name @ ", you have completed the weapons introduction mission!\"";
  schedule("bottomprint(" @ %clientId @ ", " @ %String @ " , 10);", 5);
  schedule("messageAll(0, \"~wshell_click.wav\");", 5); 
  schedule("Training::MissionComplete(" @ %clientId @ ");", 15);
}

//----------------------------------
//StayInBounds()
//----------------------------------
function StayInBounds(%clientId)
{ 
	%player = Client::getOwnedObject(%clientId);
	%flash = 0.4;
	Player::setDamageFlash(%player, %flash);
	%pos = "203.811 -532.056 176.938";
	messageAll(0, "You must Stay inside your base!" @ "~wshieldhit.wav");
	GameBase::setPosition(%ClientId, %pos);
} 
   
//------------------------------------------
//GroupTrigger::onLeave()
//------------------------------------------
function GroupTrigger::onLeave(%this, %object)
{ 
   %clId = Client::getFirst();
   //The trigger stuff is not completely reliable, so we'll try our own 
   //hard-coded rough check here as well:
   %pos = Gamebase::getPosition( %clId );

   %x = getWord(%pos, 0 );
   %y = getWord(%pos, 1 );
   %z = getWord(%pos, 2 );

   // Seems to work pretty well.  You can get on top of the platform now.  I increased
   // the Z so you can fly pretty high (it will cause the call to the else{} below.
   //
   // This has the "side effect" that there now IS a way to leave the platform, but you 
   // have to try, and I think it qualifies as an easter egg rather than a bug.  
   if( %x < 193 || %x > 212 || %y < -539 || %y > -521 || %z < 175 || %z > 250 )
   {
      dbecho(2,"Out of Bounds!");
      StayInBounds(%clId);
   }
   else
   {
      dbecho(2,"onLeave() erroneously called in Weapons Training");
   }
} 

//----------------------------------
//setTarget()
//----------------------------------
function setTarget(%playerId, %name)
{
	%id = AI::getId(%name);
	%targIdx = (%id - 2048);
	issueTargCommand(%playerId, %playerId, 1,"Waypoint set to target", %targIdx);
}				

//----------------------------------
//missionSummary()
//----------------------------------
function missionSummary()
{
   %time = getSimTime();
   
   Training::displayBitmap(0);
   Team::setObjective(0, 1, "<f5><jl>Mission Completion:");
   Team::setObjective(0, 2, "<f1>   -Completed:");
   Team::setObjective(0, 3, "\n");
   Team::setObjective(0, 4, "<f5><jl>Mission Information:");
   Team::setObjective(0, 5, "<f1>   -Mission Name: Introduction to Weapons");
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

//--------------------------------------
//Training::missionComplete()
//--------------------------------------
function Training::missionComplete(%cl)
{
  Client::setGuiMode(%cl, $GuiModeObjectives);
  missionSummary();
  remoteEval(2049, TrainingEndMission);
}

//-------------------------------------
//remoteTrainingEndMission()
//-------------------------------------
function remoteTrainingEndMission()
{
   schedule("EndGame();", 8);
}

//------------------------------------
//replenishAmmo()
//------------------------------------
function replenishAmmo()
{
  %playerId = 2049;
  
  if($counter == 2)
    Player::setItemCount(%playerId, PlasmaAmmo, 100);						  
  else if($counter == 4)
    Player::setItemCount(%playerId, bulletAmmo, 100);
  else if($counter == 3)
    Player::setItemCount(%playerId, discAmmo, 100);
  else if($counter == 5)
    Player::setItemCount(%playerId ,grenadeAmmo, 100);
  schedule("replenishAmmo();", 30);
}

//--------------------------------------------
//Game::initialMissionDrop()
//--------------------------------------------
function Game::initialMissionDrop(%clientId)
{
   GameBase::setTeam(%clientId, 0);
   Client::setGuiMode(%clientId, $GuiModePlay);
   Game::playerSpawn(%clientId, false);

   schedule("bottomprint(" @ %clientId @ ", \"<f1><jc>Training Mission 2 - Weapon Introduction.\", 5);", 0);
   schedule("messageAll(0, \"~wshell_click.wav\");", 0); 
   schedule("weaponsStart(" @ %clientId @ ");", 5);
	Training::displayBitmap(0);
	objectiveScreen();
}

//------------------------------------------------
//ObjectiveScreen()
//------------------------------------------------
function ObjectiveScreen()
{
   %time = getSimTime() - $MatchStartTime;
   Training::displayBitmap(0);
   Team::setObjective(0, 1, "<f5><jl>Mission Completion:");
   Team::setObjective(0, 2, "<f1>   -Firing range: Weapons training in progress");
   Team::setObjective(0, 3, "\n");
   Team::setObjective(0, 4, "<f5><jl>Mission Information:");
   Team::setObjective(0, 5, "<f1>   -Mission Name: Introduction to Weapons");
   Team::setObjective(0, 6, "\n");
   
   Team::setObjective(0, 7, "<f5><j1>Mission Summary:");
   Team::setObjective(0, 8, "<f1>   -Total Mission Time: " @ "<f1>" @ Time::getMinutes(%time) @ " Minutes " @ Time::getSeconds(%time) @ " Seconds");
   Team::setObjective(0, 9, "<f1>   -Total kills: " @ $AIkilled @ " out of 6");
   
   Team::setObjective(0, 10, "\n");
   Team::setObjective(0, 11, "\n");
   Team::setObjective(0, 12, "\n");
   Team::setObjective(0, 13, "\n");
   Team::setObjective(0, 14, "\n");
}

//----------------------------------------
//Player::onDamage()
//----------------------------------------
function Player::onDamage(%this,%type,%value,%pos,%vec,%mom,%vertPos,%quadrant,%object)
{
  if(Player::getClient(%this) != Client::getFirst())	
  {	
	if (Player::isExposed(%this)) {
      %damagedClient = Player::getClient(%this);
      %shooterClient = %object;

		Player::applyImpulse(%this,%mom);
		if($teamplay && %damagedClient != %shooterClient && Client::getTeam(%damagedClient) == Client::getTeam(%shooterClient) ) {
			if (%shooterClient != -1) {
				%curTime = getSimTime();
			   if ((%curTime - %this.DamageTime > 3.5 || %this.LastHarm != %shooterClient) && %damagedClient != %shooterClient && $Server::TeamDamageScale > 0) {
					Client::sendMessage(%damagedClient,0,"You took Friendly Fire from " @ Client::getName(%shooterClient) @ "!");
					Client::sendMessage(%shooterClient,0,"You just harmed Teammate " @ Client::getName(%damagedClient) @ "!");
					%this.LastHarm = %shooterClient;
					%this.DamageStamp = %curTime;
				}
			}
			%friendFire = $Server::TeamDamageScale;
		}
		else 
			%friendFire = 1.0;	

		if (!Player::isDead(%this)) {
			%armor = Player::getArmor(%this);
			//More damage applyed to head shots
			if(%vertPos == "head" && %type == $LaserDamageType) {
				if(%armor == "harmor") { 
					if(%quadrant == "middle_back" || %quadrant == "middle_front" || %quadrant == "middle_middle") {
						%value += (%value * 0.3);
					}
				}
				else {
					%value += (%value * 0.3);
				}
			}
			//If Shield Pack is on
			if (%type != -1 && %this.shieldStrength) {
				%energy = GameBase::getEnergy(%this);
				%strength = %this.shieldStrength;
				if (%type == $ShrapnelDamageType || %type == $MortarDamageType)
					%strength *= 0.75;
				%absorb = %energy * %strength;
				if (%value < %absorb) {
					GameBase::setEnergy(%this,%energy - ((%value / %strength)*%friendFire));
					%thisPos = getBoxCenter(%this);
					%offsetZ =((getWord(%pos,2))-(getWord(%thisPos,2)));
					GameBase::activateShield(%this,%vec,%offsetZ);
					%value = 0;
				}
				else {
					GameBase::setEnergy(%this,0);
					%value = %value - %absorb;
				}
			}
  			if (%value) {
				%value = $DamageScale[%armor, %type] * %value * %friendFire;
            %dlevel = GameBase::getDamageLevel(%this) + %value;
            %spillOver = %dlevel - %armor.maxDamage;
				GameBase::setDamageLevel(%this,%dlevel);
				%flash = Player::getDamageFlash(%this) + %value * 2;
				if (%flash > 0.75) 
					%flash = 0.75;
				Player::setDamageFlash(%this,%flash);
				//If player not dead then play a random hurt sound
				if(!Player::isDead(%this)) { 
					if(%damagedClient.lastDamage < getSimTime()) {
						%sound = radnomItems(3,injure1,injure2,injure3);
						playVoice(%damagedClient,%sound);
						%damagedClient.lastdamage = getSimTime() + 1.5;
					}
				}
				else {
               if(%spillOver > 0.5 && (%type== $ExplosionDamageType || %type == $ShrapnelDamageType || %type== $MortarDamageType|| %type == $MissileDamageType)) {
		 				Player::trigger(%this, $WeaponSlot, false);
						%weaponType = Player::getMountedItem(%this,$WeaponSlot);
						if(%weaponType != -1)
							Player::dropItem(%this,%weaponType);
                	Player::blowUp(%this);
					}
					else
					{
						if ((%value > 0.40 && (%type== $ExplosionDamageType || %type == $ShrapnelDamageType || %type== $MortarDamageType || %type == $MissileDamageType )) || (Player::getLastContactCount(%this) > 6) ) {
					  		if(%quadrant == "front_left" || %quadrant == "front_right") 
								%curDie = $PlayerAnim::DieBlownBack;
							else
								%curDie = $PlayerAnim::DieForward;
						}
						else if( Player::isCrouching(%this) ) 
							%curDie = $PlayerAnim::Crouching;							
						else if(%vertPos=="head") {
							if(%quadrant == "front_left" ||	%quadrant == "front_right"	) 
								%curDie = radnomItems(2, $PlayerAnim::DieHead, $PlayerAnim::DieBack);
						  	else 
								%curDie = radnomItems(2, $PlayerAnim::DieHead, $PlayerAnim::DieForward);
						}
						else if (%vertPos == "torso") {
							if(%quadrant == "front_left" ) 
								%curDie = radnomItems(3, $PlayerAnim::DieLeftSide, $PlayerAnim::DieChest, $PlayerAnim::DieForwardKneel);
							else if(%quadrant == "front_right") 
								%curDie = radnomItems(3, $PlayerAnim::DieChest, $PlayerAnim::DieRightSide, $PlayerAnim::DieSpin);
							else if(%quadrant == "back_left" ) 
								%curDie = radnomItems(4, $PlayerAnim::DieLeftSide, $PlayerAnim::DieGrabBack, $PlayerAnim::DieForward, $PlayerAnim::DieForwardKneel);
							else if(%quadrant == "back_right") 
								%curDie = radnomItems(4, $PlayerAnim::DieGrabBack, $PlayerAnim::DieRightSide, $PlayerAnim::DieForward, $PlayerAnim::DieForwardKneel);
						}
						else if (%vertPos == "legs") {
							if(%quadrant == "front_left" ||	%quadrant == "back_left") 
								%curDie = $PlayerAnim::DieLegLeft;
							if(%quadrant == "front_right" ||	%quadrant == "back_right") 
								%curDie = $PlayerAnim::DieLegRight;
						}
						Player::setAnimation(%this, %curDie);
					}
					if(%type == $ImpactDamageType && %object.clLastMount != "")  
						%shooterClient = %object.clLastMount;
				   	Client::onKilled(%damagedClient,%shooterClient, %type);
				}
			}
		}
 	 }
  }
}


//-------------------------------------
//Ammo::onDrop() -don't be an idiot!
//-------------------------------------
function Ammo::onDrop(%player,%item)
{
	if($matchStarted) {
		%count = Player::getItemCount(%player,%item);
		%delta = $SellAmmo[%item];
		if(%count <= %delta) { 
			if( %item == "BulletAmmo" || (Player::getMountedItem(%player,$WeaponSlot)).imageType.ammoType != %item)
				%delta = %count;
			else 
				%delta = %count - 1;

		}
		if(%delta > 0) {
			%obj = newObject("","Item",%item,%delta,false);
      	schedule("Item::Pop(" @ %obj @ ");", $ItemPopTime, %obj);

      	addToSet("MissionCleanup", %obj);
			GameBase::throw(%obj,%player,20,false);
			messageAll(0, "~wmale2.wdsgst2.wav");
			Player::decItemCount(%player,%item,%delta);
		}
	}
}	


//do nothing functions
function remoteScoresOn(%clientId)
{
}

function remoteScoresOff(%clientId)
{
}

function Item::onDrop(%player, %item)
{
}
