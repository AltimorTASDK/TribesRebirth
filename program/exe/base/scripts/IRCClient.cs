//------------------------------------
//
// GUI screen functions
//
//------------------------------------

//
// Create the default IRC channels
//
$IRC::ChannelName [0] = "#Tribes";
$IRC::ChannelTopic[0] = "General TRIBES discussion";

$IRC::ChannelName [1] = "#Tribes_Forum";
$IRC::ChannelTopic[1] = "";

$IRC::ChannelName [2] = "#Tribes_Beginner";
$IRC::ChannelTopic[2] = "";

$IRC::ChannelName [3] = "#Tribes_Expert";
$IRC::ChannelTopic[3] = "";

$IRC::ChannelName [4] = "#Tribes_Strategy";
$IRC::ChannelTopic[4] = "";

$IRC::ChannelName [5] = "#Tribes_Stories";
$IRC::ChannelTopic[5] = "";

$IRC::ChannelName [6] = "#Tribes_Opponents";
$IRC::ChannelTopic[6] = "";

$IRC::ChannelName [7] = "#Tribes";
$IRC::ChannelTopic[7] = "";


//------------------------------------
// LOGIN GUI
function IRCLoginGui::onOpen()
{
	//initialize the vars
	$IRCConnected = FALSE;
	$IRCJoinedRoom = FALSE;
	if ($IRC::ListenInSim == "")
	{
		$IRC::ListenInSim = "FALSE";
	}
	if ($IRC::lastServer == "")
	{
		$IRC::lastServer = -1;
	}
	if ($IRC::currentServer == "" || $IRC::currentServer > $IRC::lastServer)
	{
		if ($IRC::lastServer < 0)
		{
			$IRC::currentServer = -1;
		}
		else
		{
			$IRC::currentServer = 0;
		}
	}

	//clear the combo
	FGCombo::clear(IDCTG_IRC_SERVER_COMBO);

	//first add all the servers to the server list combo box
   for(%i = 0; $IRC::description[%i] != ""; %i++)
	{
		FGCombo::addEntry(IDCTG_IRC_SERVER_COMBO, $IRC::description[%i], %i);
	}

	//now set the console vars
	if ($IRC::currentServer >= 0)
	{
		FGCombo::setSelected(IDCTG_IRC_SERVER_COMBO, $IRC::currentServer);

		$IRC::server		= $IRC::server[$IRC::currentServer];
		$IRC::serverPort	= $IRC::serverPort[$IRC::currentServer];
		$IRC::room			= $IRC::room[$IRC::currentServer];
	}

	//verify the IRC connect button state
	IRCLoginGui::verifyConnect();
}

function IRCLoginGui::onClose()
{
   export("IRC::*", "config\\IRCServers.cs", False);
}

function IRCLoginGui::verifyConnect()
{
	if ($IRC::server != "" && $IRC::nickname != "")
	{
		Control::setActive(IDCTG_IRC_CONNECT, TRUE);
	}
	else
	{
		Control::setActive(IDCTG_IRC_CONNECT, FALSE);
	}
}

function IRCLoginGui::Select()
{
	IRCLoginGui::Read(FGCombo::getSelected(IDCTG_IRC_SERVER_COMBO));
}

function IRCLoginGui::read(%cfgNum)
{
	if (%cfgNum >= 0 && %cfgNum <= $IRC::lastServer)
	{
		$IRC::currentServer = %cfgNum;

		//copy the values into the current set used by IRCClient.cpp
		$IRC::server		= $IRC::server[%cfgNum];
		$IRC::serverPort	= $IRC::serverPort[%cfgNum];
		$IRC::room			= $IRC::room[%cfgNum];
	}

	//set the connect button
	if ($IRC::server != "" && $IRC::serverPort != "" && $IRC::nickname != "")
	{
		Control::setActive(IDCTG_IRC_CONNECT, TRUE);
	}
	else
	{
		Control::setActive(IDCTG_IRC_CONNECT, FALSE);
	}
}

function IRCLogin::AutoConnect()
{
	if ((! $IRCConnected) && $IRC::ConnectOnStartup && $IRC::nickname != "" && $IRC::server != "")
	{
		//set the real name
		if ($PCFG::RealName)
		{
			$IRC::realName = $PCFG::RealName;
		}
		if ($IRC::realName == "")
		{
			$IRC::realName = $IRC::nickname;
		}

		//connect to the IRC server
		ircConnect($IRC::server, $IRC::serverPort);

		//set the var
		$IRCConnected = TRUE;
	}
}

function ConnectToIRCServer()
{
	if ($IRC::nickname != "" && $IRC::server != "")
	{
		//validate the port
		if ($IRC::serverPort <= 0) $IRC::serverPort = 6667;

		//set the real name
		if ($PCFG::RealName)
		{
			$IRC::realName = $PCFG::RealName;
		}
		if ($IRC::realName == "")
		{
			$IRC::realName = $IRC::nickname;
		}

		//connect to the IRC server
		ircConnect($IRC::server, $IRC::serverPort);

		//set the var
		$IRCConnected = TRUE;

		//load the content ctrl
		GuiLoadContentCtrl(MainWindow, "gui\\IRCChat.gui");
	}
}

function DisconnectFromIRCServer()
{
	//disconnect
	ircDisconnect();

	//set the var
	$IRCConnected = FALSE;
	$IRCJoinedRoom = FALSE;

	//load the content ctrl
	GuiLoadContentCtrl(MainWindow, "gui\\IRCLogin.gui");
}

//--------------------------------------------------------------------------------------------------
// OPTIONS GUI

function IRCOptions::Clear()
{
	Control::setValue(IDCTG_IRC_SERVER_TEXT, "");
	Control::setValue(IDCTG_IRC_PORT_TEXT, "");
	Control::setValue(IDCTG_IRC_ROOM_TEXT, "");

	$IRC::server		= "";
	$IRC::serverPort	= "";
	$IRC::room			= ""; 
}

function IRCOptions::init()
{
	//init some vars
	if ($IRC::lastServer == "")
	{
		$IRC::lastServer = -1;
	}
	if ($IRC::currentServer == "" || $IRC::currentServer > $IRC::lastServer)
	{
		if ($IRC::lastServer < 0)
		{
			$IRC::currentServer = -1;
		}
		else
		{
			$IRC::currentServer = 0;
		}
	}
	if ($IRC::DisconnectInSim == "")
	{
		$IRC::DisconnectInSim = true;
	}

	if ($IRC::BroadcastIP == "")
	{
		$IRC::BroadcastIP = true;
	}

	if ($IRC::MsgHistory == "")
	{
		$IRC::MsgHistory = 500;
	}
	Control::setValue(OptionsIRCHistory, $IRC::MsgHistory);

	if ($IRC::UseLogFile == "") $IRC::UseLogFile = FALSE;
	Control::setValue(OptionsIRCCaptureToLogfile, $IRC::UseLogFile);

	if ($IRC::LogFile == "") $IRC::LogFile = "IRCLog.txt";
	Control::setValue(OptionsIRCLogfile, $IRC::LogFile);

	//set the other options
	Control::setValue("OptionsIRCDisconnectInSim", $IRC::DisconnectInSim);
	Control::setValue("OptionsIRCBroadcast", $IRC::BroadcastIP);

	//clear all the config controls to start
	IRCOptions::Clear();

	//clear the combo
	FGCombo::clear(IDCTG_IRC_OPTIONS_COMBO);

	//first add all the players to the player list combo box
   for(%i = 0; $IRC::description[%i] != ""; %i++)
	{
		FGCombo::addEntry(IDCTG_IRC_OPTIONS_COMBO, $IRC::description[%i], %i);
	}

	if ($IRC::currentServer >= 0)
	{
		IRCOptions::Read($IRC::currentServer);
		Control::setActive(IDCTG_IRC_REMOVE_SERVER, TRUE);

		//also activate the entry fields
		Control::setActive(IDCTG_IRC_SERVER_TEXT, true);
		Control::setActive(IDCTG_IRC_PORT_TEXT, true);
		Control::setActive(IDCTG_IRC_ROOM_TEXT, true);
	}
	else
	{
		//set the button
		Control::setActive(IDCTG_IRC_REMOVE_SERVER, FALSE);

		//also grey out the entry fields
		Control::setActive(IDCTG_IRC_SERVER_TEXT, false);
		Control::setActive(IDCTG_IRC_PORT_TEXT, false);
		Control::setActive(IDCTG_IRC_ROOM_TEXT, false);
	}
}

function IRCOptionsMsgHistory::validate()
{
	if ($IRC::MsgHistory < 50)
	{
		$IRC::MsgHistory = 50;
	}
	else if ($IRC::MsgHistory > 500)
	{
		$IRC::MsgHistory = 500;
	}
	Control::setValue(OptionsIRCHistory, $IRC::MsgHistory);
}

function IRCOptions::Shutdown()
{
	IRCOptions::Write();
   export("IRC::*", "config\\IRCServers.cs", False);
}

function IRCOptions::Select()
{
	IRCOptions::Write();
	IRCOptions::Read(FGCombo::getSelected(IDCTG_IRC_OPTIONS_COMBO));
}

function IRCOptions::New()
{
	//push the dialog
   GuiPushDialog(MainWindow, "gui\\NewIRCServer.gui");
}

function IRCOptions::Read(%cfgNum)
{
	if (%cfgNum >= 0 && %cfgNum <= $IRC::lastServer)
	{
		//load the values into the gui controls		
		Control::setValue(IDCTG_IRC_SERVER_TEXT,		$IRC::server[%cfgNum]);
		Control::setValue(IDCTG_IRC_PORT_TEXT,			$IRC::serverPort[%cfgNum]);
		Control::setValue(IDCTG_IRC_ROOM_TEXT,			$IRC::room[%cfgNum]);

		//copy the values into the current set used by IRCClient.cpp
		$IRC::server		= $IRC::server[%cfgNum];
		$IRC::serverPort	= $IRC::serverPort[%cfgNum];
		$IRC::room			= $IRC::room[%cfgNum];
		
		//set the selected server in the combo box
		$IRC::currentServer = %cfgNum;
		FGCombo::setSelected(IDCTG_IRC_OPTIONS_COMBO, $IRC::currentServer);

		//set the buttons
		Control::setActive(IDCTG_IRC_REMOVE_SERVER, TRUE);
	}
}
	
function IRCOptions::Write()
{
	if ($IRC::CurrentPlayer >= 0 && $IRC::CurrentPlayer <= $IRC::lastServer)
	{
		//get the values from the gui controls
		$IRC::server[$IRC::currentServer]		= Control::getValue(IDCTG_IRC_SERVER_TEXT);
		$IRC::serverPort[$IRC::currentServer]	= Control::getValue(IDCTG_IRC_PORT_TEXT);
		$IRC::room[$IRC::currentServer]			= Control::getValue(IDCTG_IRC_ROOM_TEXT);

		//update the set of vars used by IRCClient.cpp
		$IRC::server		= $IRC::server[$IRC::currentServer];
		$IRC::serverPort	= $IRC::serverPort[$IRC::currentServer];
		$IRC::room			= $IRC::room[$IRC::currentServer];
	}
}
	
function IRCOptions::Add(%newServerDescription)
{
	IRCOptions::Write();
	if (%newServerDescription != "")
	{
		//add it to the end
		$IRC::lastServer = $IRC::lastServer + 1;

		//create the vars
		$IRC::description[$IRC::lastServer]	= %newServerDescription;
		$IRC::server[$IRC::lastServer]		= "";
		$IRC::serverPort[$IRC::lastServer]	= "";
		$IRC::room[$IRC::lastServer]			= "";
		
		//add it to the list
		FGCombo::addEntry(IDCTG_IRC_OPTIONS_COMBO, $IRC::description[$IRC::lastServer], $IRC::lastServer);

		//set the button
		Control::setActive(IDCTG_IRC_REMOVE_SERVER, true);

		//activate the entry fields
		Control::setActive(IDCTG_IRC_SERVER_TEXT, true);
		Control::setActive(IDCTG_IRC_PORT_TEXT, true);
		Control::setActive(IDCTG_IRC_ROOM_TEXT, true);

		//set the added player as the current
		IRCOptions::Read($IRC::lastServer);
		IRCOptions::Write();


	}
}

function IRCOptions::Remove()
{
	if ($IRC::currentServer >= 0 && $IRC::currentServer <= $IRC::lastServer)
	{
		//copy the last player entries over top of the one being deleted
		if ($IRC::currentServer < $IRC::lastServer)
		{
			$IRC::description[$IRC::currentServer]	= $IRC::description[$IRC::lastServer];
			$IRC::server[$IRC::currentServer]		= $IRC::server[$IRC::lastServer];
			$IRC::serverPort[$IRC::currentServer]	= $IRC::serverPort[$IRC::lastServer];
			$IRC::room[$IRC::currentServer]			= $IRC::room[$IRC::lastServer];

			//delete it from the combo box
			FGCombo::deleteEntry(IDCTG_IRC_OPTIONS_COMBO, $IRC::currentServer);

			//delete the last entry, and re-add it with the new ID number
			FGCombo::deleteEntry(IDCTG_IRC_OPTIONS_COMBO, $IRC::lastServer);
			FGCombo::addEntry(IDCTG_IRC_OPTIONS_COMBO, $IRC::description[$IRC::currentServer], $IRC::currentServer);
		}
		else
		{
			//delete it from the combo box
			FGCombo::deleteEntry(IDCTG_IRC_OPTIONS_COMBO, $IRC::currentServer);
		}
		
		//erase the last player config
		$IRC::description[$IRC::lastServer]	= "";
		$IRC::server[$IRC::lastServer]		= "";
		$IRC::serverPort[$IRC::lastServer]	= "";
		$IRC::room[$IRC::lastServer]			= "";
		
		//decriment the last player index
		$IRC::lastServer = $IRC::lastServer - 1;

		//now set the current server to be the first
		if ($IRC::lastServer >= 0)
		{
			IRCOptions::Read(0);
		}
		else
		{
			IRCOptions::Clear();

			//set the button
			Control::setActive(IDCTG_IRC_REMOVE_SERVER, FALSE);

			//also grey out the entry fields
			Control::setActive(IDCTG_IRC_SERVER_TEXT, false);
			Control::setActive(IDCTG_IRC_PORT_TEXT, false);
			Control::setActive(IDCTG_IRC_ROOM_TEXT, false);
		}
	}
}




