//----------------------------------------------------------------------------
// Client side convience scripts
//
function buy(%desc)
{
	%type = getItemType(%desc);
	if (%type != -1) {
		remoteEval(2048,buyItem,%type);
	}
	else {
		echo("Unknown item \"" @ %desc @ "\"");
	}
}

function markFavorites()
{
	// Currently done by the shell
}

function remoteSetInfoLine(%mgr, %lineNum, %text)
{
   if(%mgr != 2048)
      return;

	if (%lineNum == 1)
	{
		if (%text == "") Control::setVisible(InfoCtrlBox, FALSE);
		else Control::setVisible(InfoCtrlBox, TRUE);
	}

   Control::setText("InfoCtrlLine" @ %lineNum, %text);
}


function buyFavorites()
{
	// This function is invoked by the shell.
	for (%i = 0; %i < 20; %i = %i + 1) {
		if ($pref::itemFavorite[%i] != "") {
			%type = getItemType($pref::itemFavorite[%i]);
			if (%type != -1) {
				%list = %list @ "," @ %type;
			}
		}
	}
	if (%list != "") {
		eval("remoteEval(2048,buyFavorites" @ %list @ ");");
	}
}	

function sell(%desc)
{
	%type = getItemType(%desc);
	if (%type != -1) {
		remoteEval(2048,sellItem,%type);
	}
	else {
		echo("Unknown item \"" @ %desc @ "\"");
	}
}

function use(%desc)
{
	%type = getItemType(%desc);
	if (%type != -1) {
		// The client useItem function will make sure the use is
		// sequenced correctly with trigger events.  The remoteEval
		// works fine but may be delivered out of order.
		// remoteEval(2048,useItem,%type);
		useItem(%type);
	}
	else {
		echo("Unknown item \"" @ %desc @ "\"");
	}
}

function drop(%desc)
{
	%type = getItemType(%desc);
	if (%type != -1) {
		remoteEval(2048,dropItem,%type);
	}
	else {
		echo("Unknown item \"" @ %desc @ "\"");
	}
}

function throwStart()
{
	$throwStartTime = getSimTime();
}

function throwRelease(%desc)
{
	%type = getItemType(%desc);
	if (%type != -1) {
		%delta = getSimTime() - $throwStartTime;
		if (%delta > 1)
			%delta = 100;
		else
			%delta = floor(%delta * 100);
		remoteEval(2048,throwItem,%type,%delta);
	}
	else {
		echo("Unknown item \"" @ %desc @ "\"");
	}
}

function deploy(%desc)
{
	%type = getItemType(%desc);
	if (%type != -1) {
		remoteEval(2048,deployItem,%type);
	}
	else {
		echo("Unknown item \"" @ %desc @ "\"");
	}
}

function nextWeapon()
{
	// Up to the server right now
	remoteEval(2048,nextWeapon);
}	

function prevWeapon()
{
	// Up to the server right now
	remoteEval(2048,prevWeapon);
}

function commandAck()
{
	// Placed here to avoid binding problems with gui.
	remoteEval(2048,CStatus,1,"Command Acknowledged~wacknow");
}

function commandDeclined()
{
	// Placed here to avoid binding problems with gui.
	remoteEval(2048,CStatus,0,"Unable to complete objective~wobjxcmp");
}

function CommandCompleted()
{
	// Placed here to avoid binding problems with gui.
	remoteEval(2048,CStatus,0,"Objective Completed~wobjcomp");
}

function targetClient()
{
   if($lastClientMessage)
   {
      if(Client::getTeam(getManagerId()) == Client::getTeam($lastClientMessage))
         %cmd = "Escort " @ Client::getName($lastClientMessage) @ ".~wescfr";
      else
         %cmd = "Attack " @ Client::getName($lastClientMessage) @ ".~wattway";
      remoteEval(2048, "IssueTargCommand", 0, %cmd, $lastClientMessage - 2048, getManagerId());
   }
}

function onClientMessage(%client, %msg)
{
   if(%client)
      $lastClientMessage = %client;

   // filter messages here
   return true;
}

function onTeamAdd(%team, %name)
{

}

function onClientJoin(%client)
{

}

function onClientDrop(%client)
{

}

function onClientChangeTeam(%client, %team)
{

}

function clearCenterPrint(%id)
{
   if(%id == $centerPrintId)
      Client::centerPrint("", 0);
}

function remoteITXT(%manager, %msg)
{
   if(%manager == 2048)
      Control::setValue(EnergyDisplayText, %msg);
}

function remoteCP(%manager, %msg, %timeout)
{
   if(%manager == 2048)
   {
      $centerPrintId++;
      if(%timeout)
         schedule("clearCenterPrint(" @ $centerPrintId @ ");", %timeout);
      Client::centerPrint(%msg, 0);
   }
}

function remoteBP(%manager, %msg, %timeout)
{
   if(%manager == 2048)
   {
      $centerPrintId++;
      if(%timeout)
         schedule("clearCenterPrint(" @ $centerPrintId @ ");", %timeout);
      Client::centerPrint(%msg, 1);
   }
}

function remoteTP(%manager, %msg, %timeout)
{
   if(%manager == 2048)
   {
      $centerPrintId++;
      if(%timeout)
         schedule("clearCenterPrint(" @ $centerPrintId @ ");", %timeout);
      Client::centerPrint(%msg, 2);
   }
}

function kill()
{
	remoteEval(2048,kill);
}	

function giveall()
{
	remoteEval(2048,giveall);
}

// Fear aliases
function setTeam(%team)
{
   remoteEval(2048, setTeam, %team);
}

function say(%channel, %message)
{
   remoteEval(2048, say, %channel, %message);
}

function mute(%playerName)
{
   remoteEval(2048, mute, 1, getClientByName(%playerName));
}

function unmute(%playerName)
{
   remoteEval(2048, mute, 0, getClientByName(%playerName));
}

function show()
{
   // redefine end frame script
   function Game::EndFrame()
   {
   }
   $Console::LastLineTimeout = 0;
   $Console::updateMetrics = false;
}
show();

function showTime()
{
   function Game::EndFrame()
   {
      echo("C: " @ getIntegerTime(false) @ "   S: " @ getIntegerTime(true));
   }
   $Console::LastLineTimeout = 1000;
   $Console::updateMetrics = false;
}

function showFPS()
{
   function Game::EndFrame()
   {
	   echo($ConsoleWorld::FrameRate);
   }
   $Console::LastLineTimeout = 1000;
   $Console::updateMetrics = false;
}

function showGfxOGL()
{
   function Game::EndFrame()
   {
	   echo($ConsoleWorld::FrameRate  @
	   		" T: "   @ $OpenGL::TexDL @
	   		" L: "   @ $OpenGL::LMDL  @
	   		" LB: "  @ $OpenGL::LMB   @
	   		" E: "   @ $OpenGL::ET);
   }
   $Console::LastLineTimeout = 1000;
}

function showGfxSW()
{
   function Game::EndFrame()
   {
      echo($ConsoleWorld::FrameRate 
       @ " P:" @ $GFXMetrics::EmittedPolys
       @ ", " @ $GFXMetrics::RenderedPolys
       @ " TSU:", $GFXMetrics::textureSpaceUsed);
   }
   $Console::LastLineTimeout = 1000;
   $Console::updateMetrics = true;
}

function messageAndAnimate(%animSeq,%wav)
{ 
	remoteEval(2048,playAnimWav,%animSeq,%wav);
}

function remotePlayAnimWav(%cl, %anim, %wav)
{
   remotePlayAnim(%cl, %anim);
   playVoice(%cl, %wav);
}

function remoteLMSG(%cl, %wav)
{
   playVoice(%cl, %wav);
}

function localMessage(%wav)
{
   remoteEval(2048, LMSG, %wav);
}

function changeLevel(%newLevel)
{
   remoteEval(2048, changeLevel, %newLevel);
}

function setArmor(%armorType)
{
   remoteEval(2048, setArmor, %armorType);
}

function voteYes()
{
   remoteEval(2048, VoteYes);
}

function voteNo()
{
   remoteEval(2048, VoteNo);
}

//editing functions

function winMouse()
{
   inputDeactivate(mouse0);
   windowsMouseEnable(MainWindow);
}

function dirMouse()
{
   inputActivate(mouse0);
   windowsMouseDisable(MainWindow);
}

function editGui()
{
   winMouse();
   GuiInspect(MainWindow);
   GuiToolbar(MainWindow);
}

function tree()
{
	simTreeCreate(tree, MainWindow);
	simTreeAddSet(tree, manager);
}

function toggleMouse()
{
   if($flagMouseDirect = !$flagMouseDirect)
   {
      dirMouse();
   }
   else
   {
      winMouse();
   }
}

function remoteSetTime(%server, %time)
{
   if(%server == 2048)
      setHudTimer(%time);
}

function SAD(%password)
{
   remoteEval(2048, AdminPassword, %password);
}

function SADSetPassword(%password)
{
   remoteEval(2048, SetPassword, %password);
}

function ADSetTimeLimit(%time)
{
   remoteEval(2048, SetTimeLimit, %time);
}

function ADSetTeamInfo(%team, %teamName, %skinBase)
{
   remoteEval(2048, SetTeamInfo, %team, %teamName, %skinBase);
}

function remoteSVInfo(%server, %version, %hostname, %mod, %info, %favKey)
{
   if(%server == 2048)
   {
      $ServerVersion = %version;
      $ServerName = %hostname;
      $modList = %mod;
      $ServerMod = $modList;
      $ServerInfo = %info;
      $ServerFavoritesKey = %favKey;
      EvalSearchPath();
   }
}


function remoteMODInfo(%server, %modString)
{
   if(%server == 2048)
   {
      $ServerModInfo = %modString;
      // if it is not "", 
      // show it on the loading gui
      if($ServerModInfo != "")
      {
         Control::setValue(ModTextString, "<jc><f1>NOTICE: This server has been modified\n<f0>" @ %modString);
      }
   }
}

function remoteFileURL(%server, %url)
{
   if(%server == 2048)
   {
      $ServerFileURL = %url;
      // if a disconnect occurs, pop this string up instead of error message
   }
}

function remoteMInfo(%server, %missionName)
{
   if(%server == 2048)
   {
      %file = "missions\\" @ %missionName @ ".dsc";
      $MDESC::Type = "";
      $MDESC::Text = "";
      if(File::findFirst(%file) != "")
         exec(%file);
      $ServerMission = %missionName;
      $ServerText = $MDESC::Text;
      $ServerMissionType = $MDESC::Type;
		if(isObject(LobbyGui))
			LobbyGui::onOpen();  // update lobby screen text.
   }
}

function remoteMissionChangeNotify(%serverManagerId, %nextMission)
{
   if(%serverManagerId == 2048)
   {
      cls();
      echo("Server mission complete - changing to mission: ", %nextMission);
      echo("Flushing Texture Cache");
      flushTextureCache();
      schedule("purgeResources(true);", 3);
   }
}

function dataGotBlock(%blockName, %pctDone)
{
   if(%pctDone < 0.1)
      %text = "Initializing Personal Digital Assistant...";
   else if(%pctDone < 0.2)
      %text = "Establishing uplink with satellite network...";
   else if(%pctDone < 0.3)
      %text = "Downloading navigational and topographical data...";
   else if(%pctDone < 0.4)
      %text = "Charging armor energy cell...";
   else if(%pctDone < 0.5)
      %text = "Pingback satellite uplink check...";
   else if(%pctDone < 0.6)
      %text = "Beginning primary weapons system check...";
   else if(%pctDone < 0.7)
      %text = "Beginning secondary weapons system check...";
   else if(%pctDone < 0.8)
      %text = "Downloading tactical information from tribal database...";
   else
      %text = "Starting armor power-up sequence...";

   //Control::setText(ProgressText, "Loading " @ %blockName @ " data...");
   Control::setValue(ProgressText, "<jc><f1>" @ %text);
   Control::setValue(ProgressSlider, %pctDone * 0.75);
}

function dataFinished()
{
   // called on client when all the dynamic data has
   // finished transfer.

	if ($cdMusic && !$pref::userCDOverride)
	{
		rbSetPlayMode (CD, 0);
		rbStop (CD);
	}
   Control::setValue(ProgressText, "<jc><f0>Get ready to rock n' roll!");
   Control::setValue(ProgressSlider, 0.9);

   $dataFinished = true;
   remoteEval(2048, dataFinished);

   echo("Flushing Texture Cache");
   flushTextureCache();
}

function onClientGhostAlwaysDone()
{
   // preload the commander gui if it's not already loaded
   if(!isObject("commandGui"))
      loadObject("commandGui", "gui\\command.gui");

   %temp = $pref::terrainTextureDetail;
   $pref::terrainTextureDetail = 1;
   rebuildCommandMap();
   $pref::terrainTextureDetail = %temp;
   flushTextureCache();
   purgeResources(true);
   remoteEval(2048, "CGADone");

//   echo("Registering Static Textures");
//   RegisterStaticTextures(MainWindow);
}

function remoteSetMusic (%player, %track, %mode)
{
	if(%player == 2048)
   {
	   $cdPlayMode = %mode;
	   $cdTrack = %track;
	   if (!$pref::userCDOverride)
	   {
		   if ($cdTrack == 0)
			{
				rbSetPlayMode (CD, 0);
				rbStop (CD);
			}
		   else
			   if ($cdTrack != "")
				{
					rbSetPlayMode (CD, 0);
					rbStop (CD);
					rbSetPlayMode (CD, $cdPlayMode);
					if ($pref::cdMusic)
						rbPlay (CD, $cdTrack);
				}
	   }
   }
}

function onConnectionError(%client, %manager, %errorString)
{
   if(%manager == 2048)
   {
   }
   else
   {
      Quickstart();
      GuiPushDialog(MainWindow, "gui\\MessageDialog.gui");
      $errorString = "Connection to server error:\n" @ %errorString;
		schedule("Control::setValue(MessageDialogTextFormat, $errorString);", 0);
   }
}

function onConnection(%message)
{
   echo("Connection ", %message);
   $dataFinished = false;
   if(%message == "Accepted")
   {
      resetSimTime();
		//execute the custom script
		if ($PCFG::Script != "")
		{
			exec($PCFG::Script);
		}

      resetPlayDelegate();
      remoteEval(2048, "SetCLInfo", $PCFG::SkinBase, $PCFG::RealName, $PCFG::EMail, $PCFG::Tribe, $PCFG::URL, $PCFG::Info, $pref::autoWaypoint, $pref::noEnterInvStation, $pref::messageMask);

		if ($Pref::PlayGameMode == "JOIN")
		{
			cursorOn(MainWindow);
	      GuiLoadContentCtrl(MainWindow, "gui\\Loading.gui");
			renderCanvas(MainWindow);
		}

   }
   else if(%message == "Rejected")
   {
		Quickstart();
      $errorString = "Connection to server rejected:\n" @ $errorString;
      GuiPushDialog(MainWindow, "gui\\MessageDialog.gui");
		schedule("Control::setValue(MessageDialogTextFormat, $errorString);", 0);
   }
   else
   {
      //startMainMenuScreen();
		Quickstart();

      if(%message == "Dropped")
      {
         if($errorString == "")
            $errorString = "Connection to server lost:\nServer went down.";
         else
            $errorString = "Connection to server lost:\n" @ $errorString;

         GuiPushDialog(MainWindow, "gui\\MessageDialog.gui");
		   schedule("Control::setValue(MessageDialogTextFormat, $errorString);", 0);
      }
      else if(%message == "TimedOut")
      {
         $errorString = "Connection to server timed out.";
         GuiPushDialog(MainWindow, "gui\\MessageDialog.gui");
		   schedule("Control::setValue(MessageDialogTextFormat, $errorString);", 0);
      }
   }
}

function onPlaybackFinished()
{
   // called when a recording is done with playback.
	cursorOn(MainWindow);
   GuiLoadContentCtrl(MainWindow, "gui\\Recordings.gui");
}

function setupRecorderFile(%fileName)
{
	if(%fileName != "" && %fileName != "False") {
		if(isFile("recordings\\" @ %fileName)) 
			echo("Warning- " @ %fileName @ " File Already Exists");
		$recorderFileName = "recordings\\" @ %fileName;
		if($recorderFileName != "False") {
			echo("File is named: ",%fileName);
			return "True";
		}	
 	}
	else {
		for(%i = 0; %i < 500; %i++) {
			if(!isFile("recordings\\recording" @ %i @ ".rec")) {
			  $recorderFileName = "recordings\\recording" @ %i @ ".rec";
				if($recorderFileName != "False") {
					echo("File is named: recording",%i,".rec");
					return "True";
				}	
			}		
		}
	}
	echo("Couldn't setup File");
	return "False";	
}

function EnterLobbyMode()
{
   schedule("ELM();", 0);
}

function ELM()
{
   if($playingDemo)
   {
      setCursor(MainWindow, "Cur_Arrow.bmp");
      disconnect();
      startMainMenuScreen();
      GuiLoadContentCtrl(MainWindow, "gui\\Recordings.gui");
   }
   else
   {
	   $InLobbyMode = true;
      GuiLoadContentCtrl(MainWindow, "gui\\Lobby.gui");
      CursorOn(MainWindow);
   }
}

