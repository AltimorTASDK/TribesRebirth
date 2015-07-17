//------------------------------------
//
// Game/network functions
//
//------------------------------------

function selectNewMaster()
{
   translateMasters();
}

function checkMasterTranslation()
{
   for(%i = 0; %i < $Server::numMasters; %i++)
   {
      %mstr = DNet::getResolvedMaster(%i);
      if(%mstr != "")
         $Server::XLMasterN[%i] = %mstr;
      $inet::master[%i+1] = $Server::XLMasterN[%i];
   }
}

function translateMasters()
{
   for(%i = 0; (%word = getWord($Server::MasterAddressN[$Server::CurrentMaster], %i)) != -1; %i++)
      %mlist[%i] = %word;

   $Server::numMasters = DNet::resolveMasters(%mlist0, %mlist1, %mlist2, %mlist3, %mlist4, %mlist5, %mlist6, %mlist7, %mlist8, %mlist9);
}

function QuickStart()
{
	$ConnectedToServer = FALSE;
   setCursor(MainWindow, "Cur_Arrow.bmp");
	cursorOn(MainWindow);
	$QuickStart = TRUE;
	if ($PlayingDemo)
	{
		$PlayingDemo = FALSE;
		GuiLoadContentCtrl(MainWindow, "gui\\Recordings.gui");
	}
	else if ($PCFG::Name == "")
	   GuiLoadContentCtrl(MainWindow, "gui\\PlayerSetup.gui");
	else if ($pref::PlayGameMode == "")
	   GuiLoadContentCtrl(MainWindow, "gui\\Connect.gui");
	else if ($Pref::PlayGameMode == "JOIN")
	   GuiLoadContentCtrl(MainWindow, "gui\\JoinGame.gui");
	else if ($pref::PlayGameMode == "TRAIN")
	   GuiLoadContentCtrl(MainWindow, "gui\\training.gui");
	else
	   GuiLoadContentCtrl(MainWindow, "gui\\CreateServer.gui");
}

function PlayerSetupNext()
{
	if ($QuickStart == "TRUE")
		QuickStart();
	else
	{
		if ($PCFG::Name == "")
			OpenNewPlayerDialog();
		else
		   GuiLoadContentCtrl(MainWindow, "gui\\Connect.gui");
	}
}

function JoinGame()
{
	if ($IRC::DisconnectInSim == "")
		$IRC::DisconnectInSim = true;

	//disconnect from IRC if req'd
	if ($IRC::DisconnectInSim == true)
	{
		ircDisconnect();
		$IRCConnected = FALSE;
		$IRCJoinedRoom = FALSE;
	}
	//load the "connecting" gui
	GuiLoadContentCtrl(MainWindow, "gui\\Connecting.gui");

   $SinglePlayer = False;
   purgeResources();
   connect($Server::Address);
}

function EndGame()
{
	$ConnectedToServer = FALSE;
   setCursor(MainWindow, "Cur_Arrow.bmp");
   disconnect();
   if ($SinglePlayer)
	   TrainingEndGameCallback();

	if ($pref::userCDOverride == False)
	{
		rbSetPlayMode (CD, 0);
		rbStop (CD);
		$cdTrack = "";
	}
	$recordDemo = FALSE;
   startMainMenuScreen();
   deleteServer();
	QuickStart();
   deleteObject(ConsoleScheduler);
   newObject(ConsoleScheduler, SimConsoleScheduler);
   if($quitOnDisconnect)
      schedule("quit();", 0);
}

function Canvas::handleEscapeKey()
{
	if ($ConnectedToServer && $InLobbyMode)
	{
		Client::exitLobbyMode();
		return "true";
	}
	return "false";
}

function startMainMenuScreen()
{
   GuiLoadContentCtrl(MainWindow, "gui\\MainMenu.gui");

	if ($pref::userCDOverride == False)
	{
		rbSetPlayMode (CD, 0);
		rbStop (CD);
		$cdPlayMode = 1;
		$cdTrack = 2;
		if ($pref::cdMusic)
			rbPlay (CD, $cdTrack);
	}
	else if ($Override == "")
	{
		$Override = True;
		rbStop (CD);
		rbSetPlayMode (CD, 2);
		rbPlay (CD);
	}
   setCursor(MainWindow, "Cur_Arrow.bmp");
   cursorOn(MainWindow);
}

function loadPlayGui()
{
	if (File::FindFirst("play.gui") != "")
		GuiLoadContentCtrl(MainWindow, "play.gui");
	else
		GuiLoadContentCtrl(MainWindow, "gui\\play.gui");
}

function switchToChat()
{
	if ($IRCConnected == "TRUE")
		GuiLoadContentCtrl(MainWindow, "gui\\IRCChat.gui");
	else
		GuiLoadContentCtrl(MainWindow, "gui\\IRCLogin.gui");
}

//-----------------------------------------------
//
// onExit() is called when the program terminates
//
//-----------------------------------------------

function onExit()
{
   if(isObject(playGui))
      storeObject(playGui, "config\\play.gui");

   saveActionMap("config\\config.cs", "actionMap.sae", "playMap.sae", "pdaMap.sae");

	//update the video mode - since it can be changed with alt-enter
	$pref::VideoFullScreen = isFullScreenMode(MainWindow);

   checkMasterTranslation();
	echo("exporting pref::* to prefs.cs");
   export("pref::*", "config\\ClientPrefs.cs", False);
   export("Server::*", "config\\ServerPrefs.cs", False);
   export("pref::lastMission", "config\\ServerPrefs.cs", True);
   BanList::export("config\\banlist.cs");
}

//-----------------------------------------------
//
// onQuit() is called when the user clicks the
// in-shell close box
//
//-----------------------------------------------

function onQuit()
{
	GuiPushDialog(MainWindow, "gui\\Exit.gui");
}

function JoinGameGui::onOpen()
{
   checkMasterTranslation();
	//update the record button
	if (! $recordDemo) $recorderFileName = "";
	Control::setValue(RecordDemo, $recordDemo);
}

function CreateServerGui::onOpen()
{
   TextList::clear(MissionTextList);
   FGCombo::clear(MissionTypePopup);
   for(%i = 0; %i < $MLIST::TypeCount; %i++)
      if($MLIST::Type[%i] != "Training")
         FGCombo::addEntry(MissionTypePopup, $MLIST::Type[%i], %i);

   if($pref::LastMissionType == "" || $pref::LastMissionType >= $MLIST::TypeCount)
      $pref::LastMissionType = 0;
   FGCombo::setSelected(MissionTypePopup, $pref::LastMissionType);
   MissionTypePopup::buildList();

	//verify the max number of players as well...
	verifyMaxPlayers();
	
	//sort the mission text list
	FGTextList::sort(MissionTextList);

	//now set the selected
   MissionTextList::onAction();

	//update the record button
	if (! $recordDemo) $recorderFileName = "";
	Control::setValue(RecordDemo, $recordDemo);
}

function enableHostGame()
{
	%valid = TRUE;

	//check the max number of players
	if ($Server::MaxPlayers < 1 || $Server::MaxPlayers > 128)
	{
		%valid = FALSE;
	}

	//check the server name
	if (String::empty($Server::HostName))
	{
		%valid = FALSE;
	}

	//set the button state
	Control::setActive("CreateServerNow", %valid);
}

function verifyServerName()
{
	if (String::Empty(Control::getValue("CreateServerHostName")))
	{
		Control::setValue("CreateServerHostName", "TRIBES Server");
	}
}

function verifyMaxPlayers()
{
	if (Control::getValue(IDCTG_MAX_PLAYERS) < 1) Control::setValue(IDCTG_MAX_PLAYERS, 8);
	else if (Control::getValue(IDCTG_MAX_PLAYERS) > 32) Control::setValue(IDCTG_MAX_PLAYERS, 32);
	enableHostGame();
}

function TrainingGui::onOpen()
{
   TextList::clear(TrainingMissionTextList);
   for(%i = 0; %i < $MLIST::TypeCount; %i++)
      if($MLIST::Type[%i] == "Training")
         break;

   for(%j = 0; (%mis = getWord($MLIST::MissionList[%i], %j)) != -1; %j++)
      TextList::addLine(TrainingMissionTextList, $MLIST::EName[%mis]);
   
	//sort the training mission text list
	FGTextList::sort(TrainingMissionTextList);

   Control::setValue(TrainingMissionTextList, $pref::LastTrainingMission);
   TrainingMissionTextList::onAction();

   %prev = getWord($MLIST::MissionList[%i], 0);
   %ml = $MLIST::MissionList[%i] @ %prev;
   %prevName = $MLIST::EName[%prev];

	//update the record button
	if (! $recordDemo) $recorderFileName = "";
	Control::setValue(RecordDemo, $recordDemo);
}

function MissionTypePopup::buildList()
{
   %sel = FGCombo::getSelected(MissionTypePopup);
   TextList::clear(MissionTextList);

	//see if it gets added
	%found = FALSE;
   for(%i = 0; (%mis = getWord($MLIST::MissionList[%sel], %i)) != -1; %i++)
	{
      TextList::addLine(MissionTextList, $MLIST::EName[%mis]);
		if ($MLIST::EName[%mis] == $pref::LastMission) %found = TRUE;
	}

	//if the last mission selected was not added, choose the first in the list
	if (%found == FALSE)
	{
	   $pref::lastMission = $MLIST::EName[getWord($MLIST::MissionList[%sel], 0)];
	}

	//sort the mission text list
	FGTextList::sort(MissionTextList);

   Control::setValue(MissionTextList, $pref::LastMission);
   $pref::lastMissionType = %sel;
   MissionTextList::onAction();
}

function MissionTextList::onAction()
{
   $pref::lastMission = Control::getValue(MissionTextList);

   // find the mission number...
   for(%i = 0; %i < $MLIST::Count; %i++)
      if($MLIST::EName[%i] == $pref::lastMission)
         break;

   Control::setValue(MissionDescText, $MLIST::EText[%i]);

}

function TrainingMissionTextList::onAction()
{
   $pref::lastTrainingMission = Control::getValue(TrainingMissionTextList);

   // find the mission number...
   for(%i = 0; %i < $MLIST::Count; %i++)
      if($MLIST::EName[%i] == $pref::lastTrainingMission)
         break;

   Control::setValue(MissionDescText, $MLIST::EText[%i]);

}
//--------------------------------------------------------------------------------------------------
function JoinGameGui::onOpen()
{
	//now clear the list, and re-enter it
	FGCombo::clear(JoinGameBuddyCombo);

	%i = 0;
	while ($pref::buddyList[%i] != "")
	{
	   FGCombo::addEntry(JoinGameBuddyCombo, $pref::buddyList[%i], %i);
		%i++;
	}

	Control::setValue(JoinGameBuddyList, $pref::buddyList);
}

function JGBuddyCombo::select()
{
   %sel = FGCombo::getSelected(JoinGameBuddyCombo);
	$pref::buddyList = $pref::buddyList[%sel];
	Control::setValue(JoinGameBuddyList, $pref::buddyList[%sel]);

	//push the selected to the top of the combo
	for (%i = %sel; %i > 0; %i--)
	{
		$pref::buddyList[%i] = $pref::buddyList[%i - 1];
	}
	$pref::buddyList[0] = $pref::buddyList;

	//now recreate the combo
	JoinGameGui::onOpen();

	Server::ResortList(IDCTG_SERVER_SELECT_LIST);
}

function updateBuddyList()
{
	$pref::buddyList = Control::getValue(JoinGameBuddyList);

	//if we're searching for a new buddy, push it at the front of the list, and bump the rest down
	if ($pref::buddyList != $pref::buddyList[0] && $pref::buddyList != "")
	{
		for (%i = 15; %i > 0; %i--)
		{
			$pref::buddyList[%i] = $pref::buddyList[%i - 1];
		}
		$pref::buddyList[0] = $pref::buddyList;

		//now recreate the combo
		JoinGameGui::onOpen();
	}

	Server::ResortList(IDCTG_SERVER_SELECT_LIST);
}

function JGNewServer::verify()
{
	%name = Control::getValue(JoinGameNewServerName);
	%address = Control::getValue(JoinGameNewServerAddress);

	if (%name != "" && %address != "")
	{
		Control::setActive(DialogReturnButton, TRUE);
	}
	else
	{
		Control::setActive(DialogReturnButton, FALSE);
	}
}

function JGNewServer()
{
	%name = Control::getValue(JoinGameNewServerName);
	%address = Control::getValue(JoinGameNewServerAddress);
	echo("DEBUG: " @ %name @ " " @ %address);

	addGameServer(%address, %name, "N/A", "-1", "TRUE");
	GuiPopDialog(MainWindow, 0);
}

//--------------------------------------------------------------------------------------------------

function RecordingsGui::onOpen()
{
	$RecordingsCount = 0;
   EvalSearchPath();
	TextList::clear(RecordingsTextList);
   %rec = File::FindFirst("*.rec");
   while(%rec != "")
   {
      %demo = File::getBase(%rec);
      TextList::addLine(RecordingsTextList, %demo);
      %rec = File::FindNext("*.rec");

		$Recording[$RecordingsCount] = %demo;
		$RecordingsCount++;
   }
	RecordingsGui::verify();
	FGTextList::sort(RecordingsTextList);
	Control::setValue(RecordingsContinuous, $pref::RecordingsContinuous);
}

function RecordingsGui::verify()
{
	if (Control::getValue(RecordingsTextList) == "")
	{
		%selected = FALSE;
	}
	else
	{
		%selected = TRUE;
	}
	Control::setActive(RecordingsRemove, %selected);
	Control::setActive(RecordingsRename, %selected);
	Control::setActive(RecordingsPlayDemo, %selected);
}

function RecordingsGui::PlayDemo()
{
	%fileName = "recordings\\" @ Control::getValue(RecordingsTextList) @ ".rec";
	if (isFile(%fileName))
	{
		$PlayingDemo = TRUE;
	   playDemo("recordings\\" @ Control::getValue(RecordingsTextList) @ ".rec");
		cursorOn(MainWindow);
	   GuiLoadContentCtrl(MainWindow, "gui\\Loading.gui");
		renderCanvas(MainWindow);
	}
	else
	{
		RecordingsGui::onOpen();
	}
}

function RecordingsGui::removeSelectedDemo()
{
	if (Control::getValue(RecordingsTextList) != "")
	{
		%srcFile = "recordings\\" @ Control::getValue(RecordingsTextList) @ ".rec";

		//delete the file
		File::delete(%srcFile);

		//now repopulate the list
		RecordingsGui::onOpen();
	}
}

function RecordingsGui::renameSelectedDemo()
{
	//first copy the old
	%newName = Control::getValue(RenameDemoText);
	if (Control::getValue(RecordingsTextList) != "" && %newName != "")
	{
		%srcFile = "recordings\\" @ Control::getValue(RecordingsTextList) @ ".rec";
		if (String::findSubStr(%newName, ".rec") >= 1)
		{
			%destFile = "recordings\\" @ %newName;
		}
		else
		{
			%destFile = "recordings\\" @ %newName @ ".rec";
		}

		//copy the file
		if (File::copy(%srcFile, %destFile))
		{
			//delete the old
			File::delete(%srcFile);

			//now repopulate the list
			RecordingsGui::onOpen();

			//reselect the new name
			TextList::setSelected(RecordingsTextList, %newName);
			RecordingsGui::verify();
			GuiPopDialog(MainWindow, 0);
		}
		else
		{
			GuiPopDialog(MainWindow, 0);
			GuiPushDialog(MainWindow, "gui\\MessageDialog.gui");
			Control::setValue(MessageDialogTextFormat, "Unable to save recording as <f1>" @ %newName @ ".<f0>  Please ensure the new name is a valid file name.");
		}
	}
}

function RenameDemoText::onAction()
{
	if (! String::empty(Control::getValue(RenameDemoText)))
	{
		Control::setActive(IDCTG_RENAME_DEMO_DONE, TRUE);
	}
	else
	{
		Control::setActive(IDCTG_RENAME_DEMO_DONE, FALSE);
	}
}

function RecordingsGui::playRandom()
{
	if ($RecordingsCount > 0 && $pref::RecordingsContinuous)
	{
		%randomNum = floor(getRandom() * ($RecordingsCount - 0.01));
		Control::setValue(RecordingsTextList, $Recording[%randomNum]);
		RecordingsGui::PlayDemo();
	}
}

//--------------------------------------------------------------------------------------------------

// button names are favbutton1...favbutton5, markfavoritesButton, buyFavoritesButton

function BuyList::onSelect(%sel)
{
   ShapeView::setItem(ItemView, %sel);
   //echo(%sel);
}

function BuyList::onDoubleClick(%sel)
{
   remoteEval(2048, "buyItem", %sel);
   //echo("BuyD " @ %sel);
}

function InventoryList::onSelect(%sel)
{
   ShapeView::setItem(ItemView, %sel);
}

function InventoryList::onDoubleClick(%sel)
{
   remoteEval(2048, "sellItem", %sel);
   //echo("InvD " @ %sel);
}

function CmdInventoryGui::sellSelected()
{
   %sel = Control::getValue(InventoryList);
   if(%sel != -1)
      remoteEval(2048, "sellItem", %sel);
}

function CmdInventoryGui::dropSelected()
{
   %sel = Control::getValue(InventoryList);
   if(%sel != -1)
      remoteEval(2048, "dropItem", %sel);
}

function CmdInventoryGui::useSelected()
{
   %sel = Control::getValue(InventoryList);
   if(%sel != -1)
      remoteEval(2048, "useItem", %sel);
}

function CmdInventoryGui::buySelected()
{
   %sel = Control::getValue(BuyList);
   if(%sel != -1)
      remoteEval(2048, "buyItem", %sel);
}

// ts control's name is ItemView

$curFavorites = -1;

function CmdInventoryGui::onOpen()
{
   if($curFavorites == -1)
   {
      CmdInventoryGui::favoritesSel(1);
      Control::performClick("FavButton1");
   }
}

function CmdInventoryGui::favoritesSel(%favList)
{
   $curFavorites = %favList;
   CmdInventory::setFavorites($pref::favoriteList[$curFavorites @ $ServerFavoritesKey]);
}

function CmdInventoryGui::markFavorites()
{
   if($curFavorites != -1)
   {
      $pref::favoriteList[$curFavorites @ $ServerFavoritesKey] = CmdInventory::getVisibleSet(InventoryList);
      CmdInventory::setFavorites($pref::favoriteList[$curFavorites @ $ServerFavoritesKey]);
   }
}

function CmdInventoryGui::buyFavorites(%fav)
{
   if(%fav != "")
      $curFavorites = %fav;
   if($curFavorites != -1)
   {
		//hilite the favs button
		if ($curFavorites >= 1 && $curFavorites <= 5)
		{
			for (%i = 1; %i <= 5; %i++)
			{
				%btnName = "FavButton" @ %i;
		      Control::setValue(%btnName, FALSE);
			}
			%btnName = "FavButton" @ $curFavorites;
	      Control::setValue(%btnName, TRUE);

			//and perform the click
			CmdInventoryGui::favoritesSel($curFavorites);
		}

      %fav = $pref::favoriteList[$curFavorites @ $ServerFavoritesKey];
      %first = getWord(%fav, 0);
      if(%first == -1)
         return;

      %cmd = "remoteEval(2048, buyFavorites, " @ %first;
      for(%i = 1; (%word = getWord(%fav, %i)) != -1; %i++)
         %cmd = %cmd @ ", " @ %word;
      %cmd = %cmd @ ");";
      eval(%cmd);
   }
}

function ConnectGui::ChooseGame()
{
   if($pref::playGameMode == "JOIN")
      GuiLoadContentCtrl(MainWindow, "gui\\joingame.gui");
   else if($pref::playGameMode == "TRAIN")
      GuiLoadContentCtrl(MainWindow, "gui\\training.gui");
   else
      GuiLoadContentCtrl(MainWindow, "gui\\createServer.gui");
}

function LoadingGui::onOpen()
{
   Control::setValue(ProgressSlider, 0.0);
}

//--------------------------------------------------------------------------------------------------
function userCD ()
{
 	$cdPlayMode = 2;
	$pref::cdMusic = False;
	$pref::userCDOverride = True;
	rbStop (CD);
	rbSetPlayMode (CD, $cdPlayMode);
	rbPlay (CD, 1);
}

function gameCD ()
{
	$pref::userCDOverride = False;
	$pref::cdMusic = True;
	rbStop (CD);
	$cdPlayMode = 0;
	rbSetPlayMode (CD, $cdPlayMode);
	rbPlay (CD, 2);
}

//--------------------------------------------------------------------------------------------------
function LobbyGui::onOpen()
{
	Control::setValue(LobbyServerName, $ServerName);
	Control::setValue(LobbyServerType, $ServerMod);
	Control::setValue(LobbyMissionName, $ServerMission);
	Control::setValue(LobbyMissionType, $ServerMissionType);
	Control::setValue(LobbyVersion, $ServerVersion);
	Control::setValue(LobbyAddress, $Server::Address);

   Control::setValue(LobbyServerText, $ServerInfo);
   Control::setValue(LobbyMissionDesc, $ServerText);
}