//------------------------------------
//
// OPTIONS screen functions
//
//------------------------------------

function OptionsGui::onOpen()
{
   //the IRC options page
   IRCOptions::init();

   OptionsMovement::init();
   OptionsVideo::init();
   OptionsGraphics::init();
   OptionsNetwork::init();
   OptionsSound::init();
}

function OptionsGui::onClose()
{
   //the IRC options page
   IRCOptions::Shutdown();

   OptionsNetwork::shutdown();
}

//---  OPTIONS VIDEO  ------------------------------------------------

function OptionsVideo::init()
{
   //validate the prefs
   OptionsVideo::validate();

   //set the values in the controls
   OptionsVideoMode::update();
   FGCombo::setSelected(IDCTG_OPTS_VID_FULLSCRN, FGCombo::findEntry(IDCTG_OPTS_VID_FULLSCRN, $pref::VideoFullScreenDriver));
   FGCombo::setSelected(IDCTG_OPTS_VID_WINDOW, FGCombo::findEntry(IDCTG_OPTS_VID_WINDOW, $pref::VideoWindowedDriver));
   FGCombo::setSelected(IDCTG_OPTS_VID_RES, FGCombo::findEntry(IDCTG_OPTS_VID_RES, $pref::VideoFullScreenRes));

	//the openGL part
	FGCombo::clear(OptionsVideo::OpenGLCombo);
	FGCombo::addEntry(OptionsVideo::OpenGLCombo, "TNT/TNT2", 0);
	FGCombo::addEntry(OptionsVideo::OpenGLCombo, "i740", 1);
//	FGCombo::addEntry(OptionsVideo::OpenGLCombo, "Rage 128", 2);
//	FGCombo::addEntry(OptionsVideo::OpenGLCombo, "G200/G400", 3);
	if ($pref::VideoOpenGLMode == "TNT/TNT2")
	{
		FGCombo::setSelected(OptionsVideo::OpenGLCombo, 0);
	}
	else if ($pref::VideoOpenGLMode == "i740")
	{
		FGCombo::setSelected(OptionsVideo::OpenGLCombo, 1);
	}
	else if ($pref::VideoOpenGLMode == "Rage 128")
	{
		FGCombo::setSelected(OptionsVideo::OpenGLCombo, 2);
	}
	else
	{
		FGCombo::setSelected(OptionsVideo::OpenGLCombo, 3);
	}

   %temp = getWord(Control::getValue(IDCTG_OPTS_VID_FULLSCRN), 0); 
	if (%temp == "OpenGL")
	{
		Control::setActive(OptionsVideo::OpenGLCombo, true);
		OptionsVideo::OpenGLsetup();
	}
	else Control::setActive(OptionsVideo::OpenGLCombo, false);

   $pref::VideoGamma = $pref::Display::gammaValue / 2;
   if ($pref::VideoGamma == 0) $pref::VideoGamma = 0.5;
   Control::setValue(IDCTG_OPTS_VID_GAMMA, $pref::VideoGamma);
}

function OptionsVideo::OpenGLsetup()
{
   %temp = getWord(Control::getValue(IDCTG_OPTS_VID_FULLSCRN), 0); 
	if (%temp == "OpenGL")
	{
		Control::setActive(OptionsVideo::OpenGLCombo, true);
		%temp = FGCombo::getSelected(OptionsVideo::OpenGLCombo);
		if (%temp == 0)
		{
			$pref::VideoOpenGLMode = "TNT/TNT2";
			OptionsVideo::setUpTNT();
		}
		else if (%temp == 1)
		{
			$pref::VideoOpenGLMode = "i740";
			OptionsVideo::setUpi740();
		}
		else
		{
			$pref::VideoOpenGLMode = "TNT/TNT2";
			OptionsVideo::setUpTNT();
		}
//		else if (%temp == 2)
//		{
//			$pref::VideoOpenGLMode = "Rage 128";
//			OptionsVideo::setUpi740();
//		}
//		else
//		{
//			$pref::VideoOpenGLMode = "G200/G400";
//			OptionsVideo::setUpG200G400();
//		}
	}
	else
	{
		Control::setActive(OptionsVideo::OpenGLCombo, false);
	}
}

function OptionsVideo::validate()
{
   if ($pref::VideoFullScreen == "") $pref::VideoFullScreen = "FALSE";
   if ($pref::VideoFullScreenDriver == "")
   {
      if (isGfxDriver(MainWindow, "Glide")) $pref::VideoFullScreenDriver = "Glide";
      else $pref::VideoFullScreenDriver = "Software";
   }
   if ($pref::VideoWindowedDriver == "") $pref::VideoWindowedDriver = "Software";
   if ($pref::VideoFullScreenRes == "") $pref::VideoFullScreenRes = "640x480";
   if ($pref::VideoGamma == "") $pref::VideoGamma = "0.5";
}

function OptionsVideo::onApply()
{
   //store the prefs...
   $pref::VideoFullScreen       = Control::getValue(IDCTG_OPTS_VID_MODE_FS);
   $pref::VideoFullScreenDriver = getWord(Control::getValue(IDCTG_OPTS_VID_FULLSCRN), 0); 
   $pref::VideoWindowedDriver   = getWord(Control::getValue(IDCTG_OPTS_VID_WINDOW), 0);
   $pref::VideoFullScreenRes    = Control::getValue(IDCTG_OPTS_VID_RES);
	$pref::VideoOpenGLMode 		  = getWord(Control::getValue(OptionsVideo::OpenGLCombo), 0); 

   //validate the prefs
   OptionsVideo::validate();

   //apply the prefs
   OptionsVideo::apply();
}

$firstTimeThroughApply = true;

function OptionsVideo::apply()
{
	//check the openGL setup
	OptionsVideo::OpenGLsetup();

   setWindowedDevice(MainWindow, $pref::VideoWindowedDriver);
   %temp = setFullscreenDevice(MainWindow, $pref::VideoFullScreenDriver);

   if (String::ICompare(%temp, "false") == 0) {
      %temp = setFullscreenDevice(MainWindow, "Software");
      if (String::ICompare(%temp, "false") != 0)
         $pref::VideoFullScreenDriver = "Software";
   }

   %fs = false;
   if (String::ICompare($pref::VideoFullScreen, "TRUE") == 0) {
      // set fs mode
      setFullscreenMode(MainWindow, "true");
      %fs = true;
   }
   else {
      // set windowed mode
      setFullscreenMode(MainWindow, "false");

      if ($firstTimeThroughApply == true) {
         if (GUI::isIn8BitMode() == "true") {
            GUI::issue8BitWarning();
         }
         $firstTimeThroughApply = false;
      }
   }

   if ($pref::VideoFullScreenRes != "") {
      setFSResolution(MainWindow, $pref::VideoFullScreenRes);
   }
   if (%fs == true) {
      if (isVirtualFS(MainWindow) == true) {
         echo("Virtual FS");
         $useLowRes3D = true;
      } else {
         $useLowRes3D = false;
      }
   }

   flushTextureCache();
}

function OptionsVideoMode::update()
{
   Control::setValue(IDCTG_OPTS_VID_MODE_FS, $pref::VideoFullScreen);
   if (! String::ICompare($pref::VideoFullScreen, "FALSE"))
   {
      Control::setValue(IDCTG_OPTS_VID_MODE_WIN, "TRUE");
   }
   else
   {
      Control::setValue(IDCTG_OPTS_VID_MODE_WIN, "FALSE");
   }
}

function OptionsVideo::setGamma()
{
   $pref::VideoGamma = Control::getValue(IDCTG_OPTS_VID_GAMMA);
   setGamma(MainWindow, 0.05 + (1.95 * $pref::VideoGamma));
   flushTextureCache();
   resetUpdateRegion(MainWindow);
}

function OptionsVideo::setUpTNT()
{
	echo("setting up TNT/TNT2");
   $pref::OpenGL::NoPackedTextures   = false;
   $pref::OpenGL::NoPalettedTextures = true;

   flushTextureCache();
}

function OptionsVideo::setUpi740()
{
	echo("setting up i740");
   $pref::OpenGL::NoPackedTextures   = true;
   $pref::OpenGL::NoPalettedTextures = true;

   flushTextureCache();
}

function OptionsVideo::setUpRage128()
{
	echo("setting up Rage128");
   $pref::OpenGL::NoPackedTextures   = true;
   $pref::OpenGL::NoPalettedTextures = true;

   flushTextureCache();
}

function OptionsVideo::setUpG200G400()
{
	echo("setting up G200/G400");
   $pref::OpenGL::NoPackedTextures   = true;
   $pref::OpenGL::NoPalettedTextures = true;

   flushTextureCache();
}

//---  OPTIONS MOVEMENT  ------------------------------------------------

$minMouseSensitivity = 0.0002;
$maxMouseSensitivity = 0.004;

function OptionsMovement::init()
{
   %sensitivity = Client::getMouseSensitivity("playMap.sae");
   %percentage = (%sensitivity - $minMouseSensitivity) / ($maxMouseSensitivity - $minMouseSensitivity);
   Control::setValue(IDCTG_OPTS_CFG_MOUSE_SENSITIVE, %percentage);
   Control::setValue(IDCTG_OPTS_CFG_MOUSE_X_FLIP, Client::getMouseXAxisFlip("playMap.sae"));
   Control::setValue(IDCTG_OPTS_CFG_MOUSE_Y_FLIP, Client::getMouseYAxisFlip("playMap.sae"));

   if ($pref::freeLookAlwaysOn == "")
   {
      $pref::freeLookAlwaysOn = "TRUE";
      $pref::freeLook = "TRUE";
   }
   Control::setValue(OptionsMovement::freeLook, $pref::freeLookAlwaysOn);
}

function OptionsMovement::updateMouse()
{
   editActionMap("playMap.sae");

   //calculate the sensitivity
   %percentage = Control::getValue(IDCTG_OPTS_CFG_MOUSE_SENSITIVE);
   %sensitivity = $minMouseSensitivity + (%percentage * ($maxMouseSensitivity - $minMouseSensitivity));

   if (String::ICompare(Control::getValue(IDCTG_OPTS_CFG_MOUSE_X_FLIP), "TRUE") == 0)
   {
      bindAction(mouse0, xaxis0, TO, IDACTION_YAW, Scale, %sensitivity);
   }
   else
   {
      bindAction(mouse0, xaxis0, TO, IDACTION_YAW, Flip, Scale, %sensitivity);
   }

   if (String::ICompare(Control::getValue(IDCTG_OPTS_CFG_MOUSE_Y_FLIP), "TRUE") == 0)
   {
      bindAction(mouse0, yaxis0, TO, IDACTION_PITCH, Scale, %sensitivity);
   }
   else
   {
      bindAction(mouse0, yaxis0, TO, IDACTION_PITCH, Flip, Scale, %sensitivity);
   }
}

function setFreeLook(%value)
{
   if (String::ICompare($pref::freeLookAlwaysOn, "FALSE") == 0)
   {
      echo("DdbEBUG:  freelook: " @ %value);
      $pref::freeLook = %value;
   }
   else                                             
   {
      $pref::freeLook = TRUE;
   }
}

//---  OPTIONS GRAPHICS  ------------------------------------------------

function OptionsGraphics::init()
{
   FGSlider::setDiscretePositions(OptionsTextureDetail, 0);
   FGSlider::setDiscretePositions(OptionsTerrainDetail, 0);
   FGSlider::setDiscretePositions(OptionsLightingDetail, 0);
   FGSlider::setDiscretePositions(OptionsDecalDetail, 0);
   FGSlider::setDiscretePositions(OptionsShapeDetail, 0);
   FGSlider::setDiscretePositions(OptionsDamageDetail, 3);
   Control::setValue(OptionsTextureDetail, $pref::TerrainTextureDetail);
   Control::setValue(OptionsTerrainDetail, $pref::TerrainDetail);
   Control::setValue(OptionsLightingDetail, $pref::dynamicLightDistance/200);
   Control::setValue(OptionsDecalDetail, $pref::NumDecals/50);
   Control::setValue(OptionsShapeDetail, ($pref::staticShapeDetail - 0.2)/0.8);
   Control::setValue(OptionsDamageDetail, $pref::damageSkinDetail);
}

function OptionsGraphics::setLowDetail()
{
   Control::setValue(OptionsTextureDetail, 0.0);
   Control::setValue(OptionsTerrainDetail, 0.0);
   Control::setValue(OptionsLightingDetail, 0.0);
   Control::setValue(OptionsDecalDetail, 0.0);
   Control::setValue(OptionsShapeDetail, 0.0);
   Control::setValue(OptionsDamageDetail, 0);
   
   OptionsTextureDetail::onAction();
   OptionsTerrainDetail::onAction();
   OptionsLightingDetail::onAction();
   OptionsDecalDetail::onAction();
   OptionsShapeDetail::onAction();
   OptionsDamageDetail::onAction();
}

function OptionsGraphics::setMediumDetail()
{
   Control::setValue(OptionsTextureDetail, 0.5);
   Control::setValue(OptionsTerrainDetail, 0.5);
   Control::setValue(OptionsLightingDetail, 0.5);
   Control::setValue(OptionsDecalDetail, 0.5);
   Control::setValue(OptionsShapeDetail, 0.5);
   Control::setValue(OptionsDamageDetail, 1);

   OptionsTextureDetail::onAction();
   OptionsTerrainDetail::onAction();
   OptionsLightingDetail::onAction();
   OptionsDecalDetail::onAction();
   OptionsShapeDetail::onAction();
   OptionsDamageDetail::onAction();
}

function OptionsGraphics::setHighDetail()
{
   Control::setValue(OptionsTextureDetail, 1.0);
   Control::setValue(OptionsTerrainDetail, 1.0);
   Control::setValue(OptionsLightingDetail, 1.0);
   Control::setValue(OptionsDecalDetail, 1.0);
   Control::setValue(OptionsShapeDetail, 1.0);
   Control::setValue(OptionsDamageDetail, 2);

   OptionsTextureDetail::onAction();
   OptionsTerrainDetail::onAction();
   OptionsLightingDetail::onAction();
   OptionsDecalDetail::onAction();
   OptionsShapeDetail::onAction();
   OptionsDamageDetail::onAction();
}

function OptionsTextureDetail::onAction()
{
   %val = Control::getValue(OptionsTextureDetail);
   $pref::TerrainTextureDetail = %val;
   $pref::InteriorTextureDetail = floor(((%val)* -8)+ 8);
   $pref::skyDetail = floor(%val * 1.99);
   $pref::starDetail = floor(%val * 1.99);
   $pref::mipcap = floor(%val * 4.99) + 5;
   flushTextureCache();
}                             

function OptionsTerrainDetail::onAction()
{
   $pref::TerrainDetail =  Control::getValue(OptionsTerrainDetail);
   $pref::TerrainPerspectiveDistance = Control::getValue(OptionsTerrainDetail);
   $pref::TerrainVisibleDistance = (Control::getValue(OptionsTerrainDetail) * 1300) + 200 ;
   $pref::WeatherPrecipitation = floor(Control::getValue(OptionsTerrainDetail) * 1.4);
}

function OptionsLightingDetail::onAction()
{
   $pref::dynamicLightDistance = Control::getValue(OptionsLightingDetail) * 200;
   $pref::interiorLightUpdateMS = ((1.0 - Control::getValue(OptionsLightingDetail)) * 984) + 16;
}

function OptionsDecalDetail::onAction()
{
   $pref::NumDecals = Control::getValue(OptionsDecalDetail) * 50;
   $pref::maxNumSmokePuffs = (Control::getValue(OptionsDecalDetail) * 240) + 10;
}

function OptionsShapeDetail::onAction()
{
   %value = Control::getValue(OptionsShapeDetail);

   $pref::staticShapeDetail = (Control::getValue(OptionsShapeDetail) * 0.8) + 0.2;
   $pref::vehicleShapeDetail = (Control::getValue(OptionsShapeDetail) * 0.8) + 0.2;
   $pref::playerShapeDetail = (Control::getValue(OptionsShapeDetail) * 0.8) + 0.2;
   $pref::animateWithTransitions = %value > 0.9;
}

function OptionsDamageDetail::onAction()
{
   $pref::damageSkinDetail = Control::getValue(OptionsDamageDetail);
}


//---  OPTIONS NETWORK  ------------------------------------------------

function OptionsNetwork::init()
{
   //make sure the fields are valid
   OptionsNetwork::validate();

   //initalize the gui
	Control::setValue(NetworkAutoRefresh, $pref::AutoRefresh);
	Control::setValue(NetworkHostPublic, $Server::HostPublicGame);
   Control::setValue(NetworkPortNumber, $server::port);
   Control::setValue(NetworkPacketRate, $pref::packetRate);
   Control::setValue(NetworkPacketSize, $pref::packetSize);
   OptionsNetwork::setPacketFrame();

   //setup the master server list
   FGMasterList::clear(OptionsNetworkMasterList);

   //popuplate the list
   $MasterCount = 0;
   while($Server::MasterAddressN[$MasterCount] != "")
   {
      OptionsNetwork::addMaster($Server::MasterName[$MasterCount], $Server::MasterAddressN[$MasterCount], $MasterCount);
      $MasterCount++;
   }

   if ($Server::CurrentMaster == "") $Server::CurrentMaster = 0;
   if ($MasterCount > 0)
   {
      if ($Server::CurrentMaster >= $MasterCount) $Server::CurrentMaster = 0;
      Control::setActive(OptionsNetworkRemoveMaster, true);
      FGMasterList::setSelected(OptionsNetworkMasterList, $Server::CurrentMaster);
   }
   else
   {
      Control::setActive(OptionsNetworkRemoveMaster, true);
   }
}

function OptionsNetwork::addMaster(%name, %addr, %id)
{
   if (%name == "") %name = "Tribes Master";
   if (%addr != "")
   {
      FGMasterList::addEntry(OptionsNetworkMasterList, %name, %addr, %id);
      $Server::MasterName[%id] = %name;
      $Server::MasterAddressN[%id] = %addr;
   }
}

function OptionsNetwork::deleteSelectedMaster()
{
   if ($Server::CurrentMaster >= 0)
   {
      //replace the current entry with the last
      if ($Server::CurrentMaster < $MasterCount - 1)
      {
         $Server::MasterName[$Server::CurrentMaster] = $Server::MasterName[$MasterCount - 1];
         $Server::MasterAddressN[$Server::CurrentMaster] = $Server::MasterAddressN[$MasterCount - 1];
      }
      $Server::MasterName[$MasterCount - 1] = "";
      $Server::MasterAddressN[$MasterCount - 1] = "";

      FGMasterlist::clear(OptionsNetworkMasterList);

      //popuplate the list
      $MasterCount = 0;
      while($Server::MasterAddressN[$MasterCount] != "")
      {
         OptionsNetwork::addMaster($Server::MasterName[$MasterCount], $Server::MasterAddressN[$MasterCount], $MasterCount);
         $MasterCount++;
      }
   }
   if ($MasterCount > 0)
   {
      if ($Server::CurrentMaster >= $MasterCount) $Server::CurrentMaster = 0;
      Control::setActive(OptionsNetworkRemoveMaster, TRUE);
      FGMasterList::setSelected(OptionsNetworkMasterList, $Server::CurrentMaster);
		selectNewMaster();
   }
   else
   {
      Control::setActive(OptionsNetworkRemoveMaster, FALSE);
   }
}

function OptionsNetwork::verifyMaster()
{
   %name = Control::getValue(OptionsNetworkMasterName);
   %addr = Control::getValue(OptionsNetworkMasterAddress);
   if (%name != "" && %addr != "")
   {
      Control::setActive(DialogReturnButton, true);
   }
   else
   {
      Control::setActive(DialogReturnButton, false);
   }
}

function OptionsNetwork::newMaster()
{
   %name = Control::getValue(OptionsNetworkMasterName);
   %addr = Control::getValue(OptionsNetworkMasterAddress);
   OptionsNetwork::addMaster(%name, %addr, $MasterCount);

   $Server::CurrentMaster = $MasterCount;
   FGMasterList::setSelected(OptionsNetworkMasterList, $Server::CurrentMaster);
   Control::setActive(OptionsNetworkRemoveMaster, true);

   $MasterCount++;

   GuiPopDialog(MainWindow, 0);
	selectNewMaster();
}

function OptionsNetwork::warnPublicServer()
{
   GuiPushDialog(MainWindow, "gui\\hostwarn.gui");
}

function OptionsGui::HostPublicOK()
{
   $Server::HostPublicGame = true;
   GuiPopDialog(MainWindow, 1);
}

function OptionsNetwork::HostPublicCancel()
{
   $Server::HostPublicGame = false;
   Control::setValue(NetworkHostPublic, "FALSE");
   GuiPopDialog(MainWindow, 1);
}

function OptionsNetwork::shutdown()
{
   OptionsNetwork::validate();
}

function OptionsNetwork::setPacketFrame()
{
   %value = 1.0 - (($pref::packetFrame - 32) / (128 - 32));
   Control::setValue(NetworkPacketFrame, %value);
}

function NetworkPacketFrame::onAction()
{
   %value = 1.0 - Control::getValue(NetworkPacketFrame);
   $pref::packetFrame = 32 + (%value * (128 - 32));
}

function OptionsNetwork::validate()
{
	if ($pref::AutoRefresh == "") $pref::AutoRefresh = TRUE;
	if ($Server::HostPublicGame == "") $Server::HostPublicGame = FALSE;

   if ($pref::packetRate < 1) $pref::packetRate = 10;
   if ($pref::packetRate > 30) $pref::packetRate = 30;
   Control::setValue(NetworkPacketRate, $pref::packetRate);

   if ($pref::packetSize < 100) $pref::packetSize = 200;
   if ($pref::packetSize > 500) $pref::packetSize = 500;
   Control::setValue(NetworkPacketSize, $pref::packetSize);

   if ($pref::packetFrame < 32) $pref::packetFrame = 96;
   if ($pref::packetFrame > 128) $pref::packetFrame = 128;
   OptionsNetwork::setPacketFrame();
}


function OptionsNetwork::setT1Defaults()
{
   $pref::packetRate = 15;
   $pref::packetSize = 400;
   $pref::packetFrame = 32;
   Control::setValue(NetworkPacketRate, $pref::packetRate);
   Control::setValue(NetworkPacketSize, $pref::packetSize);
   OptionsNetwork::setPacketFrame();
}

function OptionsNetwork::setISDNDefaults()
{
   $pref::packetRate = 15;
   $pref::packetSize = 300;
   $pref::packetFrame = 32;
   Control::setValue(NetworkPacketRate, $pref::packetRate);
   Control::setValue(NetworkPacketSize, $pref::packetSize);
   OptionsNetwork::setPacketFrame();
}

function OptionsNetwork::setModemDefaults()
{
   $pref::packetRate = 10;
   $pref::packetSize = 200;
   $pref::packetFrame = 96;
   Control::setValue(NetworkPacketRate, $pref::packetRate);
   Control::setValue(NetworkPacketSize, $pref::packetSize);
   OptionsNetwork::setPacketFrame();
}

//---  OPTIONS SOUND  ------------------------------------------------

function OptionsSound::init()
{
   sfxClose();

   %SFX_QUERY_DSOUND     = 1; 
   %SFX_QUERY_DSOUND3D   = 2;
   %SFX_QUERY_AUREAL3D   = 3;
   %SFX_QUERY_LIVE3D     = 4;

   FGSlider::setDiscretePositions(OptionsSoundCDVolume, 0);
   FGSlider::setDiscretePositions(OptionsSound2DVolume, 0);
   FGSlider::setDiscretePositions(OptionsSound3DVolume, 0);
   Control::setValue(OptionsSoundCDVolume, $pref::cdVolume);
   Control::setValue(OptionsSound2DVolume, $pref::sfx2dVolume);
   Control::setValue(OptionsSound3DVolume, $pref::sfx3dVolume);

   //setup the 3d sound driver combo
   FGCombo::clear(OptionsSound3DDriver);
   if (sfxQuery(%SFX_QUERY_DSOUND)) FGCombo::addEntry(OptionsSound3DDriver, "Direct Sound", %SFX_QUERY_DSOUND);
   //if (sfxQuery(%SFX_QUERY_DSOUND3D)) FGCombo::addEntry(OptionsSound3DDriver, "Direct Sound 3D", %SFX_QUERY_DSOUND3D);
   if (sfxQuery(%SFX_QUERY_AUREAL3D)) FGCombo::addEntry(OptionsSound3DDriver, "Aureal Sound 3D", %SFX_QUERY_AUREAL3D);
   //if (sfxQuery(%SFX_QUERY_LIVE3D)) FGCombo::addEntry(OptionsSound3DDriver, "Sound Blaster Live 3D", %SFX_QUERY_LIVE3D);

   if ($pref::useDSound) FGCombo::setSelected(OptionsSound3DDriver, %SFX_QUERY_DSOUND);
   //else if ($pref::useDSound3D) FGCombo::setSelected(OptionsSound3DDriver, %SFX_QUERY_DSOUND3D);
   else if ($pref::useAureal3D) FGCombo::setSelected(OptionsSound3DDriver, %SFX_QUERY_AUREAL3D);
   //else if ($pref::useLive3D) FGCombo::setSelected(OptionsSound3DDriver, %SFX_QUERY_LIVE3D);
   else
   {
      $pref::useDSound = FALSE;
      $pref::useDSound3D = FALSE;
      $pref::useAureal3D = FALSE;
      $pref::useLive3D = FALSE;

      FGCombo::setSelected(OptionsSound3DDriver, %SFX_QUERY_DSOUND);
      //FGCombo::setSelected(OptionsSound3DDriver, %SFX_QUERY_DSOUND3D);
      FGCombo::setSelected(OptionsSound3DDriver, %SFX_QUERY_AUREAL3D);
      //FGCombo::setSelected(OptionsSound3DDriver, %SFX_QUERY_LIVE3D);

      %entry = FGCombo::getSelected(OptionsSound3DDriver);
      if (%entry == %SFX_QUERY_DSOUND) $pref::useDSound = TRUE;
      //else if (%entry == %SFX_QUERY_DSOUND3D) $pref::useDSound3D = TRUE;
      else if (%entry == %SFX_QUERY_AUREAL3D) $pref::useAureal3D = TRUE;
      //else if (%entry == %SFX_QUERY_LIVE3D) $pref::useLive3D = TRUE;

   }

   //open sound
   sfxOpen();

   //intialize a var
   if ($pref::playVoices == "") $pref::playVoices = true;
   Control::setValue(OptionsSoundPlayVoices, $pref::playVoices);
}

function OptionsSound3DDriver::onAction()
{
   %SFX_QUERY_DSOUND     = 1; 
   %SFX_QUERY_DSOUND3D   = 2;
   %SFX_QUERY_AUREAL3D   = 3;
   %SFX_QUERY_LIVE3D     = 4;

   sfxClose();

   $pref::useDSound = FALSE;
   $pref::useDSound3D = FALSE;
   $pref::useAureal3D = FALSE;
   $pref::useLive3D = FALSE;

   %entry = FGCombo::getSelected(OptionsSound3DDriver);
   echo("Survey says: " @ %entry);
   if (%entry == %SFX_QUERY_DSOUND) $pref::useDSound = TRUE;
   //else if (%entry == %SFX_QUERY_DSOUND3D) $pref::useDSound3D = TRUE;
   //else if (%entry == %SFX_QUERY_AUREAL3D) $pref::useAureal3D = TRUE;
   //else if (%entry == %SFX_QUERY_LIVE3D) $pref::useLive3D = TRUE;

   sfxOpen();
}

function OptionsSoundCDVolume::onAction()
{
   $pref::CDVolume = Control::getValue(OptionsSoundCDVolume);
   rbSetVolume(CD, 1 - $pref::CDVolume);
}                             

function OptionsSoundCDMusic::onAction()
{
   if ($pref::CDMusic)
   {
      echo("Music is ON");
      $userCDOverride = False;
      newRedBook (CD, MainWindow);
      if ($cdTrack != "" && $cdTrack != 0)
         {
            rbSetPlayMode (CD, $cdPlayMode);
            rbPlay (CD, $cdTrack);
         }
   }
   else
   {
      echo("Music is OFF");
      rbSetPlayMode (CD, 0);
      rbStop (CD);
      deleteObject (CD);
   }
}

function OptionsSound2DVolume::onAction()
{
   $pref::sfx2dVolume = 0.01 + (0.99 * Control::getValue(OptionsSound2DVolume));
}                             

function OptionsSound3DVolume::onAction()
{
   $pref::sfx3dVolume = 0.01 + (0.99 * Control::getValue(OptionsSound3DVolume));
}                             


