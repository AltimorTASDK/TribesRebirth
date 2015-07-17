// GUI Screens
IDGUI_MAIN_MENU                 = 00100100, "gui\\MainMenu.gui";
IDGUI_LOAD_GAME				     = 00100101, "gui\\LoadGame.gui";
IDGUI_MULTI_PLAYER              = 00100102, "gui\\MultiPlayer.gui";
IDGUI_JOIN_GAME	        	     = 00100103, "gui\\JoinGame.gui";
IDGUI_ADDRESS		        	     = 00100104, "gui\\Address.gui";
IDGUI_CREATE_SERVER             = 00100105, "gui\\CreateServer.gui";
IDGUI_MULTI_SETUP               = 00100106, "gui\\MultiSetup.gui";
IDGUI_OPTIONS		        	     = 00100107, "gui\\Options.gui";
IDGUI_VIDEO		        		     = 00100108, "gui\\Video.gui";
IDGUI_SOUND		        		     = 00100109, "gui\\Sound.gui";
IDGUI_CONTROLS		        	     = 00100110, "gui\\Controls.gui";
IDGUI_PERFORMANCE	        	     = 00100111, "gui\\Performance.gui";
IDGUI_GAME_SHELL	        	     = 00100112, "gui\\GameShell.gui";
IDGUI_RECORDINGS	        	     = 00100113, "gui\\Recordings.gui";
IDGUI_FLYINGCAM                 = 00100114, "gui\\flyingcam.gui";
IDGUI_PLAYER_SETUP  			     = 00100115, "gui\\PlayerSetup.gui";
IDGUI_CONNECT  			     	  = 00100116, "gui\\Connect.gui";
IDGUI_IRC_CHAT  			     	  = 00100117, "gui\\IRCChat.gui";
IDGUI_TEMP_OPTIONS  			     = 00100118, "gui\\TempOptions.gui";

IDGUI_TEMP	        			     = 00100998, "gui\\Temp.gui";

//DLG tags
IDDLG_ADD_ADDRESS                = 00110003, "gui\\NewAddr.gui";
IDDLG_EDIT_ADDRESS               = 00110004, "gui\\EditAddr.gui";
IDDLG_ADD_CONDITION              = 00110010, "gui\\NewCond.gui";
IDDLG_ADD_FILTER        	     = 00110011, "gui\\NewFilter.gui";
IDDLG_EDIT_FILTER        	     = 00110012, "gui\\EditFilter.gui";
IDDLG_ADD_PLAYER        	     = 00110013, "gui\\NewPlayer.gui";
IDDLG_SERVER_INFO        	     = 00110014, "gui\\ServerInfo.gui";
IDDLG_MASTER_MOTD        	     = 00110015, "gui\\MasterMOTD.gui";
IDDLG_ADD_IRC_SERVER      	     = 00110016, "gui\\NewIRCServer.gui";
IDDLG_IRC_AWAY                   = 00110017, "gui\\IRCAway.gui";
IDDLG_IRC_CHANNEL_PROPERTIES     = 00110018, "gui\\IRCChannelProperties.dlg";
IDDLG_IRC_KICK                   = 00110019, "gui\\IRCKick.dlg";
IDDLG_IRC_BAN                    = 00110020, "gui\\IRCBan.dlg";
IDDLG_IRC_INVITE                 = 00110021, "gui\\IRCInvite.dlg";
IDDLG_IRC_INVITED                = 00110022, "gui\\IRCInvited.dlg";
IDDLG_IRC_CHANNELS               = 00110023, "gui\\IRCChannels.dlg";
IDDLG_IRC_OPTIONS                = 00110024, "gui\\IRCOptions.dlg";
IDDLG_TEMP			             = 00110998, "gui\\TempDlg.gui";

// GUI Executed console commands
IDCMD_TEST                      = 00120001, "echo \"hello world\"";
IDCMD_QUIT                      = 00120002, "quit";
IDCMD_WEBSITE                   = 00120003, "HTMLOpen(\"http://www.tribesplayers.com/\");";
IDCMD_NEW_SINGLE_PLAYER_GAME    = 00120004, "NewSinglePlayerGame";
IDCMD_JOIN_GAME                 = 00120005, "JoinGame";
IDCMD_NEW_SERVER                = 00120006, "CreateServer";
IDCMD_MISSION_LUSH              = 00120007, "MissionCreateLush";
IDCMD_MISSION_LUSH2             = 00120008, "MissionCreateLush2";
IDCMD_D3DTEST                   = 00120009, "winMouse;d3dStart;listDevices";
IDCMD_NEW_SERVER_LOCAL          = 00120010, "NewLocalServer";
IDCMD_NEW_SERVER_DEDICATED      = 00120011, "NewDedicatedServer";
IDCMD_REFRESH                   = 00120012, "rebuildServerList();";
IDCMD_QUERY_DETAIL              = 00120013, "queryServer";

// GUI Data strings 130-139

IDSTR_000   					     = 00132000, "OK";
IDSTR_001   					     = 00132001, "DONE";
IDSTR_002   					     = 00132002, "CANCEL";
IDSTR_003   					     = 00132003, "ADD";
IDSTR_004   					     = 00132004, "NEW";
IDSTR_005   					     = 00132005, "New";
IDSTR_006   					     = 00132006, "EDIT";
IDSTR_007   					     = 00132007, "Edit";
IDSTR_008   					     = 00132008, "REMOVE";
IDSTR_009   					     = 00132009, "N/A FOR ALPHA";

IDSTR_020   					     = 00132020, "QUICK START";
IDSTR_021   					     = 00132021, "PLAY GAME";
IDSTR_022   					     = 00132022, "DEMOS";
IDSTR_023   					     = 00132023, "OPTIONS";
IDSTR_024   					     = 00132024, "WEB SITE";
IDSTR_025   					     = 00132025, "QUIT";

IDSTR_030   					     = 00132030, "HOST GAME";
IDSTR_031   					     = 00132031, "JOIN GAME";
IDSTR_032   					     = 00132032, "CHAT";
IDSTR_033   					     = 00132033, "PLAYER";

IDSTR_040   					     = 00132040, "Server Name";
IDCTG_PORT_NUMBER				     = 00132041, "Port Number";
IDCTG_MAX_PLAYERS				     = 00132042, "Max Players";
IDSTR_043   					     = 00132043, "Password";
IDSTR_044   					     = 00132044, "Mission";
IDCTG_SCORE_LIMIT				     = 00132045, "Score Limit";
IDSTR_TIME_LIMIT				     = 00132046, "Time Limit";
IDSTR_047   					     = 00132047, "Dedicated";
IDSTR_048   					     = 00132048, "This server requires a password:";

IDSTR_050   					     = 00132050, "Filter";
IDSTR_051   					     = 00132051, "Server Name";
IDSTR_052   					     = 00132052, "Mission";
IDSTR_053   					     = 00132053, "Ping";
IDSTR_054   					     = 00132054, "#Plyrs/Max";
IDSTR_055   					     = 00132055, "CPU";
IDSTR_056   					     = 00132056, "Address";
IDSTR_057   					     = 00132057, "Info";
IDSTR_058   					     = 00132058, "Refresh";

IDSTR_060   					     = 00132060, "Player";
IDSTR_061   					     = 00132061, "Control File";
IDSTR_062   					     = 00132062, "Real Name";
IDSTR_063   					     = 00132063, "Location";
IDSTR_064   					     = 00132064, "EMail";
IDSTR_065   					     = 00132065, "Other Info";
IDSTR_066   					     = 00132066, "Tribe";
IDSTR_067   					     = 00132067, "Tribe URL";
IDSTR_068   					     = 00132068, "Chat Room";
IDSTR_069   					     = 00132069, "Tribe Server";
IDSTR_070   					     = 00132070, "Gender";
IDSTR_071   					     = 00132071, "Markings";
IDSTR_072   					     = 00132072, "Custom script";

IDSTR_080   					     = 00132080, "SERVER SOURCES";
IDSTR_081   					     = 00132081, "FILTER SETUP";
IDSTR_082   					     = 00132082, "COLUMN DISPLAY";

IDSTR_083   					     = 00132083, "TYPE";
IDSTR_084   					     = 00132084, "ACTIVE?";

IDSTR_090   					     = 00132090, "VARIABLE";
IDSTR_091   					     = 00132091, "CONDITION";
IDSTR_092   					     = 00132092, "VALUE";

IDSTR_100   					     = 00132100, "Address";
IDSTR_101   					     = 00132101, "Condition";
IDSTR_102   					     = 00132102, "Playback";
IDSTR_103   					     = 00132103, "PLAYBACK";
IDSTR_104   					     = 00132104, "LOADING...";

IDSTR_COMMANDER_VERSION		     = 00132105, "Commander Version?";

IDSTR_KEY   					     = 00132106, "KEY";
IDSTR_ITEM   					     = 00132107, "ITEM";
IDSTR_QUANTITY					     = 00132108, "#";

IDSTR_INV_ARMOR				     = 00132109, "ARMOR";
IDSTR_INV_PACK					     = 00132110, "PACK";
IDSTR_INV_SENSOR				     = 00132111, "SENSOR";
IDSTR_INV_WEAPON_1			     = 00132112, "WEAPON 1";
IDSTR_INV_WEAPON_2			     = 00132113, "WEAPON 2";
IDSTR_INV_WEAPON_3			     = 00132114, "WEAPON 3";
IDSTR_INV_WEAPON_4			     = 00132115, "WEAPON 4";

IDSTR_TEAM_ENERGY				     = 00132116, "TEAM ENERGY";
IDSTR_INV_COST					     = 00132117, "COST";
IDSTR_INV_CUR_ITEM			     = 00132118, "CURRENT INVENTORY";
IDSTR_INV_AVAIL_ITEM			     = 00132119, "ITEMS AVAILABLE";

IDSTR_120   					     = 00132120, "NEXT >>";
IDSTR_121   					     = 00132121, "<< BACK";

IDSTR_122   					     = 00132122, "Select Game Type";
IDSTR_123   					     = 00132123, "Choose Connection For";
IDSTR_124   					     = 00132124, "Multiplayer Game";
IDSTR_125   					     = 00132125, "Single Player Training";
IDSTR_126   					     = 00132126, "Host Multiplayer Game";
IDSTR_127   					     = 00132127, "Join Multiplayer Game";
IDSTR_128   					     = 00132128, "Internet";
IDSTR_129   					     = 00132129, "Local Area Network";
IDSTR_130   					     = 00132130, "Modem";
IDSTR_131   					     = 00132131, "Serial Connection";

IDSTR_ACTIVE_ROOMS			     = 00132132, "ACTIVE ROOMS";
IDSTR_NICKNAMES   				  = 00132133, "NAMES";

IDSTR_134   					     = 00132134, "Graphics Detail";
IDSTR_135   					     = 00132135, "Sound";
IDSTR_136   					     = 00132136, "Video";
IDSTR_137	  					     = 00132137, "Networking";
IDSTR_138   					     = 00132138, "Controls";
IDSTR_139   					     = 00132139, "Chat";
IDSTR_140   					     = 00132140, "Server";
IDSTR_141   					     = 00132141, "Join Display";
IDSTR_142   					     = 00132142, "Movement";
IDSTR_143   					     = 00132143, "Equipment";
IDSTR_144   					     = 00132144, "TCP/IP";
IDSTR_145   					     = 00132145, "IPX";
IDSTR_146   					     = 00132146, "Modem";
IDSTR_147   					     = 00132147, "Filters";
IDSTR_148   					     = 00132148, "Columns";

IDSTR_149   					     = 00132149, "Nickname";
IDSTR_150   					     = 00132150, "Alt Nick";

IDSTR_151   					     = 00132151, "Blaster Rifle";
IDSTR_152   					     = 00132152, "Plasma Rifle";
IDSTR_153   					     = 00132153, "Disc Launcher";
IDSTR_154   					     = 00132154, "Chain Gun";
IDSTR_155   					     = 00132155, "Grenade Launcher";
IDSTR_156   					     = 00132156, "Sniper Rifle";
IDSTR_157   					     = 00132157, "Laser Target";
IDSTR_158   					     = 00132158, "Mortar";
IDSTR_159   					     = 00132159, "Energy Rifle";
IDSTR_160   					     = 00132160, "? Weapon";

IDSTR_161   					     = 00132161, "Fire Weapon";
IDSTR_162   					     = 00132162, "Next Weapon";
IDSTR_163   					     = 00132163, "Prev Weapon";
IDSTR_164   					     = 00132164, "Drop Item";
IDSTR_165   					     = 00132165, "Drop Ammo";
IDSTR_166   					     = 00132166, "Use Pack";
IDSTR_167   					     = 00132167, "Deploy Mine";
IDSTR_168   					     = 00132168, "Throw Grenade";
IDSTR_169   					     = 00132169, "Team Chat";
IDSTR_170   					     = 00132170, "Chat";
IDSTR_171   					     = 00132171, "Scores";
IDSTR_172   					     = 00132172, "Inventory";
IDSTR_173   					     = 00132173, "Objectives";
IDSTR_174   					     = 00132174, "Commander Mode";
IDSTR_175   					     = 00132175, "Game View";
IDSTR_176   					     = 00132176, "Map View";
IDSTR_177   					     = 00132177, "Objectives";
IDSTR_178   					     = 00132178, "Inventory";

IDSTR_179   					     = 00132179, "Turn Left";
IDSTR_180   					     = 00132180, "Forward";
IDSTR_181   					     = 00132181, "Turn Right";
IDSTR_182   					     = 00132182, "Strafe Left";
IDSTR_183   					     = 00132183, "Backward";
IDSTR_184   					     = 00132184, "Strafe Right";
IDSTR_185   					     = 00132185, "Jetpack";
IDSTR_186   					     = 00132186, "Jump";
IDSTR_187   					     = 00132187, "Toggle POV";
IDSTR_188   					     = 00132188, "Zoom";
IDSTR_189   					     = 00132189, "Zoom Level +";
IDSTR_190   					     = 00132190, "Zoom Level -";
IDSTR_191   					     = 00132191, "Commander";
IDSTR_192   					     = 00132192, "Team Chat";
IDSTR_193   					     = 00132193, "Global Chat";
IDSTR_194   					     = 00132194, "Look Up";
IDSTR_195   					     = 00132195, "Look Down";
IDSTR_196   					     = 00132196, "Free Look?";
IDSTR_197   					     = 00132197, "Crosshair?";
IDSTR_198   					     = 00132198, "**new temp";
IDSTR_199   					     = 00132199, "**new temp";

IDSTR_RECORDINGS				     = 00132200, "RECORDINGS";


IDSTR_DONE			   		     = 00139900, "DONE";
IDSTR_CANCEL		   		     = 00139901, "CANCEL";
IDSTR_MISSION                   = 00139902, "MISSION:";

// main menu
IDSTR_MM			   			     = 00131100, "BACK TO MAIN MENU";
IDSTR_MM_NEW_GAME   			     = 00131101, "NEW GAME";
IDSTR_MM_CREATE_GAME	   	     = 00131102, "CREATE GAME";
IDSTR_MM_JOIN_GAME			     = 00131103, "JOIN GAME";
IDSTR_MM_OPTIONS                = 00131104, "OPTIONS";
IDSTR_MM_WEBSITE                = 00131105, "WEB SITE";
IDSTR_MM_CREDITS                = 00131106, "CREDITS";
IDSTR_MM_QUIT                   = 00131107, "QUIT";
IDSTR_MM_TRAINING               = 00131108, "TRAINING";
IDSTR_MM_MULTIPLAYER            = 00131109, "MULTIPLAYER";
IDSTR_MM_DEMOS			           = 00131110, "DEMOS";

//load menu

// multi-player menu
IDSTR_MP			                 = 00130100, "BACK TO MULTIPLAYER MENU";
IDSTR_MP_JOIN_GAME              = 00130101, "JOIN GAME";
IDSTR_MP_CREATE_SERVER          = 00130102, "***old CREATE SERVER";
IDSTR_MP_SETUP			           = 00130103, "MULTIPLAYER SETUP";

//join game menu
IDSTR_JG				    		     = 00130200, "BACK TO JOIN GAME MENU";
IDSTR_JG_ADDRESS		    	     = 00130201, "ADDRESS BOOK";
IDSTR_JG_SERVERS		    	     = 00130210, "SERVERS";
IDSTR_JG_SERVER_NAME	    	     = 00130211, "CHAT";
IDSTR_JG_SERVER_PING	    	     = 00130212, "PLAYER";
IDSTR_JG_SERVER_REFRESH  	     = 00130215, "REFRESH";
IDSTR_JG_JOIN				        = 00130225, "JOIN GAME";

//Address Book menu
IDSTR_AD				              = 00130300, "BACK TO ADDRESS BOOK MENU";
IDSTR_AD_LIST			           = 00130301, "ADDRESS LIST";
IDSTR_AD_NAME			           = 00130302, "NAME";
IDSTR_AD_ADDRESS			        = 00130303, "ADDRESS";
IDSTR_AD_TYPE				        = 00130304, "TYPE";
IDSTR_AD_PERFORMANCE		        = 00130305, "PERFORMANCE";
IDSTR_AD_PASSWORD			        = 00130306, "PASSWORD";
IDSTR_AD_NEW				        = 00130310, "NEW";
IDSTR_AD_DELETE			        = 00130311, "DELETE";

//Create Server menu
IDSTR_CS						        = 00130400, "BACK TO CREATE SERVER MENU";
IDSTR_CS_SERVER_NAME		        = 00130401, "SERVER NAME";
IDSTR_CS_PORT_NUMBER		        = 00130402, "PORT NUMBER";
IDSTR_CS_MAX_PLAYERS		        = 00130403, "MAX PLAYERS";
IDSTR_CS_MISSION       	        = 00130404, "MISSION";
IDSTR_CS_FRAG_LIMIT             = 00130405, "SCORE LIMIT";
IDSTR_CS_TIME_LIMIT             = 00130406, "TIME LIMIT";
IDSTR_CS_DEDICATED              = 00130407, "DEDICATED";
IDSTR_CS_CREATE_SERVER          = 00130408, "***old CREATE SERVER";

//Server Options Menu
IDSTR_SO					           = 00130500, "BACK TO SERVER OPTIONS MENU";
IDSTR_SO_MISSION			        = 00130501, "MISSION";
IDSTR_SO_MAX_PLAYERS		        = 00130502, "MAX PLAYERS";
IDSTR_SO_TEAM_NAMES		        = 00130503, "TEAM NAMES";
IDSTR_SO_TEAM_FRAGS		        = 00130504, "TEAM FRAGS";
IDSTR_SO_PASSWORD			        = 00130505, "PASSWORD";

//Multiplayer Setup Menu
IDSTR_MS						        = 00130600, "BACK TO MULTIPLAYER SETUP MENU";
IDSTR_MS_NAME				        = 00130601, "PLAYER";
IDSTR_MS_TEAM_NAME		        = 00130602, "CONTROL FILE";
IDSTR_MS_REAL_NAME		        = 00130603, "REAL NAME";
IDSTR_MS_LOCATION			        = 00130604, "LOCATION";
IDSTR_MS_EMAIL				        = 00130605, "EMAIL";
IDSTR_MS_INFO				        = 00130606, "INFO";
IDSTR_MS_TRIBE_NAME		        = 00130607, "TRIBE NAME";
IDSTR_MS_TRIBE_URL		        = 00130608, "TRIBE URL";
IDSTR_MS_TRIBE_ROOM		        = 00130609, "TRIBE ROOM";
IDSTR_MS_TRIBE_SERVER	        = 00130610, "TRIBE SERVER";
IDSTR_MS_GENDER			        = 00130611, "GENDER";
IDSTR_MS_MARKINGS			        = 00130612, "MARKINGS";
IDSTR_MS_MALE				        = 00130613, "MALE";
IDSTR_MS_FEMALE			        = 00130614, "FEMALE";
IDSTR_MS_NA					        = 00130615, "N/A FOR ALPHA";
IDSTR_MS_TEMPORARY		        = 00130616, "TEMPORARY";

//Options Menu
IDSTR_OP					           = 00130700, "BACK TO OPTIONS MENU";
IDSTR_OP_VIDEO				        = 00130701, "VIDEO";
IDSTR_OP_SOUND				        = 00130702, "SOUND";
IDSTR_OP_CONTROLS			        = 00130703, "CONTROLS";
IDSTR_OP_PERFORMANCE		        = 00130704, "PERFORMANCE";

IDSTR_PLAYBACK			           = 00130710, "PLAYBACK";

//Video Options Menu
IDSTR_VO					           = 00130800, "BACK TO VIDEO OPTIONS MENU";
IDSTR_VO_DRIVERS			        = 00130801, "DRIVER LIST";
IDSTR_VO_RESOLUTION		        = 00130802, "RESOLUTION";
IDSTR_VO_FULL_SCREEN		        = 00130803, "FULL SCREEN";
IDSTR_VO_GAMMA				        = 00130804, "GAMMA";
IDSTR_VO_APPLY				        = 00130805, "APPLY";

//Sound Options Menu
IDSTR_SD					           = 00130900, "BACK TO SOUND OPTIONS MENU";
IDSTR_SD_MUSIC_VOL		        = 00130901, "MUSIC VOLUME";
IDSTR_SD_SOUNDFX_VOL		        = 00130902, "SOUND FX VOLUME";

//Performance Menu
IDSTR_PF					           = 00131000, "BACK TO PERFORMANCE MENU";
IDSTR_PF_TERRAIN			        = 00131001, "TERRAIN DETAIL";
IDSTR_PF_TEXTURE			        = 00131002, "TEXTURE DETAIL";
IDSTR_PF_SHAPE				        = 00131003, "SHAPE DETAIL";
IDSTR_PF_LIGHTING			        = 00131004, "LIGHTING DETAIL";
IDSTR_PF_SHADOWS			        = 00131005, "SHADOWS";
IDSTR_PF_EXPLOSIONS		        = 00131006, "EXPLOSIONS";

//Game Shell
IDSTR_PAGE_1				        = 00131201, "PAGE 1";
IDSTR_PAGE_2				        = 00131202, "PAGE 2";
IDSTR_PAGE_3				        = 00131203, "PAGE 3";
IDSTR_PAGE_4				        = 00131204, "PAGE 4";
IDSTR_ABORT_MISSION		        = 00131205, "Are you sure you want to abort the mission? (Y/N)";
IDSTR_OBSERVERMODE              = 00131206, "OBSERVER MODE";

//Server Info column headers
IDSTR_SI_NAME				        = 00131301, "Server Name";
IDSTR_SI_MISSION			        = 00131302, "Mission Name";
IDSTR_SI_PING				        = 00131303, "Ping";
IDSTR_SI_PLAYERSAVAIL	        = 00131304, "#Players/Avail";
IDSTR_SI_CPU				        = 00131305, "CPU";
IDSTR_SI_ADDRESS			        = 00131306, "Address";
IDSTR_SI_STATUS			        = 00131307, "Status";
IDSTR_SI_CONNECTION		        = 00131308, "Connection";
IDSTR_SI_FAVORITES		        = 00131309, "Favorites";
IDSTR_SI_MASTER_SERVER		     = 00131310, "Master Server";
IDSTR_SI_MTYPE                  = 00131311, "Mission Type";
IDSTR_SI_MOD                    = 00131312, "Server Type";

IDSTR_SD_PLAYER			        = 00131350, "PLAYER";
IDSTR_SD_TEAM				        = 00131351, "TEAM";
IDSTR_SD_SCORE				        = 00131352, "SCORE";
IDSTR_SD_PING				        = 00131353, "PING";

//Address book column headers
IDSTR_AI_SOURCES			        = 00131401, "SERVER SOURCES";
IDSTR_AI_FILTER			        = 00131402, "FILTER SETUP";
IDSTR_AI_COLUMNS			        = 00131403, "COLUMN DISPLAY";
IDSTR_AI_ADD				        = 00131404, "ADD";
IDSTR_AI_EDIT				        = 00131405, "EDIT";
IDSTR_AI_REMOVE			        = 00131406, "REMOVE";
IDSTR_AI_TYPE				        = 00131410, "TYPE";
IDSTR_AI_ACTIVE			        = 00131411, "ACTIVE?";

//Column Display headers
IDSTR_CI_COLUMN_NAME		        = 00131501, "NAME";
IDSTR_CI_DESC				        = 00131502, "DESCRIPTION";
IDSTR_CI_ACTIVE			        = 00131503, "DISP";

//Column Display dscriptions
IDSTR_CI_FAVORITES				  = 00131519, "Your favorite server indicator.";
IDSTR_CI_CONNECTION				  = 00131517, "Quality of connection indicator.";
IDSTR_CI_STATUS				     = 00131516, "Dedicated and password indicators.";
IDSTR_CI_NAME				        = 00131510, "Text name of the server.";
IDSTR_CI_MISSION			        = 00131511, "Current Mission being run.";
IDSTR_CI_PING				        = 00131512, "Packet time from you to the server.";
IDSTR_CI_PLAYERSAVAIL	        = 00131513, "The Max/Active number of players.";
IDSTR_CI_CPU				        = 00131514, "CPU speed of the server.";
IDSTR_CI_ADDRESS			        = 00131515, "Internet address of the server.";
IDSTR_CI_VERSION			        = 00131518, "The version of TRIBES on the server.";
IDSTR_CI_MTYPE 			        = 00131520, "The type of the current mission.";
IDSTR_CI_MOD   			        = 00131521, "The modification name of the server.";

IDSTR_FT_VAR				        = 00131600, "Variable";
IDSTR_FT_COND				        = 00131601, "Condition";
IDSTR_FT_VALUE				        = 00131602, "Value";
IDSTR_FT_FILTER			        	= 00131603, "FILTER";

IDSTR_FT_VAR_PING			        = 00131610, "Ping";
IDSTR_FT_VAR_PLAYERS		        = 00131611, "# Players";
IDSTR_FT_VAR_MISSION		        = 00131612, "Mission";
IDSTR_FT_VAR_SERVER		        = 00131613, "Server Name";
IDSTR_FT_VAR_SERVER_TYPE        = 00131614, "Server Type";
IDSTR_FT_VAR_MISSION_TYPE       = 00131615, "MissionType";
IDSTR_FT_VAR_IS_FAVORITE	     = 00131616, "Favorite";
IDSTR_FT_VAR_IS_DEDICATED       = 00131617, "Dedicated";

IDSTR_FT_COND_LT			        = 00131630, "Is Less Than";
IDSTR_FT_COND_GT			        = 00131631, "Is Greater Than";
IDSTR_FT_COND_EQ			        = 00131632, "Is Equal To";
IDSTR_FT_COND_NE			        = 00131633, "Is Not Equal To";
IDSTR_FT_COND_HAS			        = 00131634, "Contains";
IDSTR_FT_COND_HAS_NOT	        = 00131635, "Does Not Contain";
IDSTR_FT_COND_TRUE		        = 00131636, "Is True";
IDSTR_FT_COND_FALSE		        = 00131637, "Is False";

IDSTR_VID_WINDOWED		        = 00131700, "WINDOWED MODE";
IDSTR_VID_FULLSCRN_MODE	        = 00131701, "FULL SCREEN MODE";
IDSTR_VID_FULLSCRN_RES	        = 00131702, "FULL SCREEN RESOLUTION";
IDSTR_VID_FULLSC_SHELL	        = 00131703, "SHELL IN FULL SCREEN";
IDSTR_VID_FULLSC_GAME	        = 00131704, "GAME IN FULL SCREEN";

IDSTR_SRVR_INFO_NAME			     = 00131800, "SERVER NAME";
IDSTR_SRVR_INFO_ADDRESS		     = 00131801, "IP Address";
IDSTR_SRVR_INFO_PING			     = 00131802, "Ping";
IDSTR_SRVR_INFO_VERSION		     = 00131803, "Version";
IDSTR_SRVR_INFO_GAME			     = 00131804, "Game";
IDSTR_SRVR_INFO_MISSION		     = 00131805, "Mission";
IDSTR_SRVR_INFO_PASSWORD	     = 00131806, "Password?";
IDSTR_SRVR_INFO_LIMIT_SC	     = 00131807, "***OBSOLETE";
IDSTR_SRVR_INFO_LIMIT_TM	     = 00131808, "***OBSOLETE";
IDSTR_SRVR_INFO_DEDICATED	     = 00131809, "Dedicated?";
IDSTR_KICK_USER                  = 00131810, "Kick User ...";
IDSTR_BAN_UNBAN_USER             = 00131811, "Ban / UnBan User ...";
IDSTR_AWAY_FROM_KEYBOARD         = 00131812, "Away from Keyboard ...";
IDSTR_IGNORE_USER                = 00131813, "Ignore User";
IDSTR_OPERATOR                   = 00131814, "Operator";
IDSTR_SPEAKER                    = 00131815, "Speaker";
IDSTR_SPECTATOR                  = 00131816, "Spectator";
IDSTR_PRIVATE_CHAT               = 00131817, "Private Chat";
IDSTR_PING_USER                  = 00131818, "Ping User";
IDSTR_WHO_IS_THIS                = 00131819, "Who is This";
IDSTR_LEAVE_ROOM                 = 00131820, "Leave Room";
IDSTR_PROPERTIES                 = 00131821, "Properties ...";
IDSTR_INVITE_USER                = 00131822, "Invite User ...";
IDSTR_INVITED_EXTRA              = 00131823, "You can accept or decline the invitation by using the buttons below";
IDSTR_IRC_CHANNEL_NAME           = 00131824, "NAME";
IDSTR_IRC_CHANNEL_TOPIC          = 00131825, "TOPIC";
IDSTR_PASSWORD_REQUIRED          = 00131826, "This server requires a password.  Please enter it below.";
IDSTR_CHANPROPS_OPER_BANNER      = 00131827, "As a channel operator, you can set the following properties";
IDSTR_CHANPROPS_SPKR_BANNER      = 00131828, "As a channel speaker, you can only set the channel topic";
IDSTR_CHANPROPS_SPEC_BANNER      = 00131829, "As a channel spectator, you can only set the channel topic";
IDSTR_CHANPROPS_NOT_OPER_BANNER  = 00131830, "You are not a channel operator, and cannot set any properties";

IDSTR_CURRENT_COMMAND		     = 00131900, "Current Command:";
IDSTR_RADIO_SET				     = 00131901, "Radio Set:";
IDSTR_MISSION_OBJECTIVES	     = 00131910, "Mission Objectives:";
IDSTR_MIS_OBJ_LINE_1			     = 00131911, "Get the girl, grab the money, and run!";

// control tags - fit in the misc range 140-149
IDCTG_SESSION                   = 00140001, "";
IDCTG_SERVER_JOIN_CANCEL        = 00140002, "";
IDCTG_PLAYER_TS                 = 00140003, "";
IDCTG_MISSION_SELECT_LIST       = 00140004, "";
IDCTG_EDIT_TS                   = 00140005, "";
IDCTG_SERVER_JOIN               = 00140006, "";
IDCTG_SERVER_DEDICATED          = 00140007, "";
IDCTG_NEW_ADDRESS				     = 00140008, "";
IDCTG_NEW_ADDR_NAME			     = 00140009, "";
IDCTG_NEW_ADDR_ADDRESS	        = 00140010, "";
IDCTG_ADDRESS_BOOK	    	     = 00140011, "";
IDCTG_EDIT_ADDRESS			     = 00140012, "";
IDCTG_EDIT_ADDR_NAME			     = 00140013, "";
IDCTG_EDIT_ADDR_ADDRESS	        = 00140014, "";
IDCTG_REMOVE_ADDR			        = 00140015, "";
IDCTG_SERVER_SELECT_LIST  	     = 00140016, "";
IDCTG_SERVER_REFRESH_LIST 	     = 00140017, "";
IDCTG_SERVER_QUERY		 	     = 00140018, "";
IDCTG_SERVER_JOIN_PASSWORD	     = 00140019, "";
IDCTG_SERVER_REFRESH_PROGRESS	  = 00140024, "";
IDCTG_SERVER_REFRESH_TEXT	  	  = 00140025, "";
IDCTG_SERVER_REFRESH_CANCEL 	  = 00140026, "";
IDCTG_SERVER_REFRESH_BOX 	  	  = 00140027, "";
IDCTG_SERVER_JOIN_INTERNET	  	  = 00140040, "";
IDCTG_SERVER_JOIN_LAN	  	  	  = 00140041, "";
IDCTG_SERVER_REFRESH_VISIBLE 	  = 00140042, "";
IDCTG_SERVER_INFO_REFRESH	 	  = 00140043, "";
IDCTG_SERVER_INFO_FAVORITE 	  = 00140044, "";
IDCTG_SERVER_INFO_NEVER_PING 	  = 00140045, "";

IDCTG_QUICK_START	     			  = 00140020, "";
IDCTG_PLAY_GAME	     			  = 00140021, "";
IDCTG_SHELL_NEXT	     			  = 00140022, "";
IDCTG_SHELL_BACK	     			  = 00140023, "";

IDCTG_NEW_CONDITION		 	     = 00140030, "";
IDCTG_REMOVE_CONDITION		     = 00140031, "";
IDCTG_NEW_FILTER		 		     = 00140032, "";
IDCTG_EDIT_FILTER		 		     = 00140033, "";
IDCTG_REMOVE_FILTER		 	     = 00140034, "";
IDCTG_CONDITION_LIST		 	     = 00140035, "";
IDCTG_FILTER_LIST		 		     = 00140036, "";
IDCTG_CONDITION_BOX		 	     = 00140037, "";
IDCTG_NEW_FILTER_TEXT 	     	  = 00140038, "";
IDCTG_NEW_CONDITION_TEXT 	     = 00140039, "";

IDCTG_FILTER_VAR		 		     = 00140050, "";
IDCTG_FILTER_COND 			     = 00140051, "";
IDCTG_FILTER_CHANGE_VAR		     = 00140052, "";

IDCTG_NEW_PLAYER				     = 00140060, "";
IDCTG_REMOVE_PLAYER			     = 00140061, "";
IDCTG_PLAYER_LIST				     = 00140062, "";

IDCTG_HUD_CLOCK		 	        = 00140100, "Clock";
IDCTG_HUD_COMPASS		 	        = 00140101, "Compass";
IDCTG_HUD_WEAPON		 	        = 00140102, "Weapons";
IDCTG_HUD_CROSSHAIR	 	        = 00140103, "Crosshair";
IDCTG_HUD_HEALTH		 	        = 00140104, "Armor";
IDCTG_GS_PAGE_DISABLE	        = 00140105, "";
IDCTG_GS_PAGE_1		 	        = 00140106, "";
IDCTG_GS_PAGE_2		 	        = 00140107, "";
IDCTG_GS_PAGE_3		 	        = 00140108, "";
IDCTG_GS_PAGE_4		 	        = 00140109, "";
IDCTG_GAME_SHELL		 	        = 00140110, "";
IDCTG_HUD_JETPACK		 	        = 00140111, "Energy";
IDCTG_HUD_CHAT			 	        = 00140112, "Chat";
IDCTG_HUD_CHAT_DISPLAY 	        = 00140113, "Chat Display";
IDCTG_PLAY_GUI			 	        = 00140114, "";
IDCTG_GS_RADIO_1		 	        = 00140115, "";
IDCTG_GS_RADIO_2		 	        = 00140116, "";
IDCTG_GS_RADIO_3		 	        = 00140117, "";
IDCTG_GS_RADIO_4		 	        = 00140118, "";
IDCTG_HUD_INVENTORY	 	        = 00140119, "";
IDCTG_SERVER_CREATE             = 00140120, "";
IDCTG_COMMAND_GUI 	 	        = 00140200, "";
IDCTG_COMMAND_TS   	 	        = 00140201, "";
IDCTG_COMMAND_PLAYER_LIST       = 00140202, "";
IDCTG_HUD_CMD_DISPLAY 	        = 00140203, "";
IDCTG_CMD_PANEL_MAP	 	        = 00140204, "";
IDCTG_CMD_PANEL_OBJ	 	        = 00140205, "";
IDCTG_COMMAND_TURRET_LIST       = 00140206, "";
IDCTG_COMMAND_ITEM_LIST	        = 00140207, "";
IDCTG_COMMAND_TSOBSERVE         = 00140208, "";
IDCTG_CONTROL_OBSERVE           = 00140209, "";
IDCTG_HUD_SENSOR 	        		  = 00140219, "Sensor Ping";
IDCTG_HUD_LIST 	        		  = 00140220, "";
IDCTG_HUD_LOGO 	        		  = 00140221, "Tribes Logo";
IDCTG_HUD_SNIPER_CROSS 	        = 00140222, "Sniper Cross";

IDCTG_CURRENT_COMMAND	        = 00140210, "";
IDCTG_ZOOM_MODE			        = 00140211, "";
IDCTG_CMD_PANEL_MODE		        = 00140212, "";
IDCTG_SET_COMMANDER		        = 00140213, "";
IDCTG_UNSET_COMMANDER	        = 00140214, "";
IDCTG_CMD_OVERLAY_MODE	        = 00140215, "";
IDCTG_CENTER_MODE			        = 00140216, "";
IDCTG_MUTE			        		  = 00140217, "";
IDCTG_CANCEL_CENTER_MODE     	  = 00140218, "";

IDCTG_CMDO_ZOOM			        = 00160220, "";
IDCTG_CMDO_CENTER			        = 00160221, "";
IDCTG_CMDO_EXTRA			        = 00160222, "";
IDCTG_CMDO_ITEMS			        = 00160223, "";
IDCTG_CMDO_OBJECTS		        = 00160224, "";
IDCTG_CMDO_PLAYERS		        = 00160225, "";
IDCTG_CMDO_RADAR			        = 00160226, "";
IDCTG_CMDO_TURRETS		        = 00160227, "";

IDCTG_CMDP_ATTACK			        = 00160228, "";
IDCTG_CMDP_DEFEND			        = 00160229, "";
IDCTG_CMDP_ESCORT			        = 00160230, "";
IDCTG_CMDP_GO_TO			        = 00160231, "";
IDCTG_CMDP_HEALTH			        = 00160232, "";
IDCTG_CMDP_RECON			        = 00160233, "";
IDCTG_CMDP_REPAIR			        = 00160234, "";

IDCTG_CMD_MAP				        = 00160235, "";
IDCTG_CMD_OBJECTIVES		        = 00160236, "";
IDCTG_CMD_HUD_MANAGER	        = 00160237, "";
IDCTG_CMD_PLAYER_STATS	        = 00160238, "";

IDCTG_CMDP_PLAYER			        = 00160239, "";
IDCTG_CMDP_TURRETS		        = 00160240, "";
IDCTG_CMDP_ITEMS			        = 00160241, "";
IDCTG_CMDP_ALL				        = 00160242, "";

IDCTG_CMD_TAKE_CONTROL	        = 00160243, "";

IDCTG_PLYR_CFG			 	        = 00140300, "";
IDCTG_PLYR_CFG_CTRLS	 	        = 00140301, "";
IDCTG_PLYR_CFG_NAME	 	        = 00140302, "";
IDCTG_PLYR_CFG_LOC	 	        = 00140303, "";
IDCTG_PLYR_CFG_EMAIL	 	        = 00140304, "";
IDCTG_PLYR_CFG_INFO	 	        = 00140305, "";
IDCTG_PLYR_CFG_TRIBE		        = 00140306, "";
IDCTG_PLYR_CFG_URL	 	        = 00140307, "";
IDCTG_PLYR_CFG_ROOM	 	        = 00140308, "";
IDCTG_PLYR_CFG_SERVER	        = 00140309, "";
IDCTG_PLYR_CFG_GENDER_M	        = 00140310, "";
IDCTG_PLYR_CFG_SKIN	 	        = 00140311, "";
IDCTG_PLYR_CFG_GENDER_F	        = 00140312, "";
IDCTG_PLYR_CFG_VOICE	        	  = 00140313, "";
IDCTG_PLYR_CFG_VOICE_TEST	     = 00140314, "";
IDCTG_PLYR_CFG_COMBO	     		  = 00140315, "";
IDCTG_PLYR_CFG_LIST	     		  = 00140316, "";
IDCTG_PLYR_CFG_SCRIPT	     	  = 00140317, "";
IDCTG_PLYR_CFG_TEXT_EDIT     	  = 00140318, "";
IDCTG_PLYR_CFG_CANCEL     	  	  = 00140319, "";

IDCTG_PLYR_CFG_READ	 	        = 00140320, "";
IDCTG_PLYR_CFG_WRITE	 	        = 00140321, "";

IDCTG_OPTS_VID_WINDOW	        = 00140400, "";
IDCTG_OPTS_VID_FULLSCRN	        = 00140401, "";
IDCTG_OPTS_VID_RES	 	        = 00140402, "";
IDCTG_OPTS_VID_GAMMA		        = 00140403, "";
IDCTG_OPTS_VID_APPLY		        = 00140404, "";
IDCTG_OPTS_VID_MODE_WIN	        = 00140405, "";
IDCTG_OPTS_VID_MODE_FS	        = 00140406, "";

IDCTG_OPTS_CTRL_CFG_SELECT      = 00140410, "";
IDCTG_OPTS_CTRL_CFG_CANCEL      = 00140413, "";
IDCTG_OPTS_CTRL_CFG_DONE        = 00140414, "";
IDCTG_OPTS_CTRL_CFG_NAME_1      = 00140415, "";
IDCTG_OPTS_CTRL_CFG_MAP_1       = 00140416, "";
IDCTG_OPTS_CTRL_CFG_NAME_2      = 00140417, "";
IDCTG_OPTS_CTRL_CFG_MAP_2       = 00140418, "";
IDCTG_OPTS_CTRL_CFG_NAME_3      = 00140419, "";
IDCTG_OPTS_CTRL_CFG_MAP_3       = 00140420, "";
IDCTG_OPTS_CTRL_CFG_NAME_4      = 00140421, "";
IDCTG_OPTS_CTRL_CFG_MAP_4       = 00140422, "";

IDCTG_OPTS_CFG_MOUSE_X_FLIP     = 00140423, "";
IDCTG_OPTS_CFG_MOUSE_Y_FLIP     = 00140424, "";
IDCTG_OPTS_CFG_MOUSE_SENSITIVE  = 00140425, "";

IDCTG_OPTS_CTRL_CFG_DEFAULTS	  = 00140426, ""; 
IDCTG_OPTS_CTRL_CFG_REVERT	  	  = 00140427, ""; 

IDCTG_SRVR_INFO_NAME			     = 00140500, "";
IDCTG_SRVR_INFO_ADDRESS		     = 00140501, "";
IDCTG_SRVR_INFO_PING			     = 00140502, "";
IDCTG_SRVR_INFO_VERSION		     = 00140503, "";
IDCTG_SRVR_INFO_GAME			     = 00140504, "";
IDCTG_SRVR_INFO_MISSION		     = 00140505, "";
IDCTG_SRVR_INFO_PASSWORD	     = 00140506, "";
IDCTG_SRVR_INFO_LIMIT_SC	     = 00140507, "";
IDCTG_SRVR_INFO_LIMIT_TM	     = 00140508, "";
IDCTG_SRVR_INFO_DEDICATED	     = 00140509, "";
IDCTG_SRVR_INFO_LIST	     		  = 00140510, "";

IDCTG_RECORDINGS_LIST		     = 00140550, "";
IDCTG_RECORDINGS_PLAYBACK	     = 00140551, "";
IDCTG_RECORDINGS_TIMER		     = 00140552, "";
IDCTG_RECORDINGS_THUMB		     = 00140553, "";
IDCTG_REMOVE_SELECTED_DEMO      = 00140554, "";
IDCTG_RENAME_SELECTED_DEMO      = 00140555, "";
IDCTG_RENAME_DEMO_DONE      	  = 00140556, "";
IDCTG_RENAME_DEMO_CANCEL     	  = 00140557, "";

IDCTG_INV_ARMOR				     = 00140560, "";
IDCTG_INV_PACK					     = 00140561, "";
IDCTG_INV_SENSOR				     = 00140562, "";
IDCTG_INV_WEAPON_1			     = 00140563, "";
IDCTG_INV_WEAPON_2			     = 00140564, "";
IDCTG_INV_WEAPON_3			     = 00140565, "";
IDCTG_INV_WEAPON_4			     = 00140566, "";

IDCTG_INV_USE					     = 00140567, "";
IDCTG_INV_DROP					     = 00140568, "";
IDCTG_INV_BUY					     = 00140569, "";
IDCTG_INV_SELL					     = 00140570, "";

IDCTG_INV_TEAM_ENERGY		     = 00140571, "";

IDCTG_INV_ITEM_SHAPE_VIEW	     = 00140572, "";

IDCTG_INV_CURRENT				     = 00140580, "";
IDCTG_INV_AVAILABLE			     = 00140581, "";
IDCTG_INV_SHOPPING_CHANGED      = 00140582, "";
IDCTG_INV_SHOPPING_SET_FAVS     = 00140583, "";
IDCTG_INV_SHOPPING_BUY_FAVS     = 00140584, "";

IDCTG_CMD_BOX_0				     = 00140600, "";
IDCTG_CMD_BOX_1				     = 00140601, "";
IDCTG_CMD_BOX_2				     = 00140602, "";
IDCTG_CMD_BOX_3				     = 00140603, "";
IDCTG_CMD_BOX_4				     = 00140604, "";
IDCTG_CMD_BOX_5				     = 00140605, "";
IDCTG_CMD_BOX_6				     = 00140606, "";

IDCTG_CMD_SCROLL_0			     = 00140610, "";
IDCTG_CMD_SCROLL_1			     = 00140611, "";
IDCTG_CMD_SCROLL_2			     = 00140612, "";
IDCTG_CMD_SCROLL_3			     = 00140613, "";
IDCTG_CMD_SCROLL_4			     = 00140614, "";

IDCTG_CMD_TEXT_0				     = 00140615, "";
IDCTG_CMD_TEXT_1				     = 00140616, "";
IDCTG_CMD_TEXT_2				     = 00140617, "";
IDCTG_CMD_TEXT_3				     = 00140618, "";
IDCTG_CMD_TEXT_4				     = 00140619, "";

IDCTG_CMD_RADIO_0				     = 00140620, "";
IDCTG_CMD_RADIO_1				     = 00140621, "";
IDCTG_CMD_RADIO_2				     = 00140622, "";
IDCTG_CMD_RADIO_3				     = 00140623, "";
IDCTG_CMD_RADIO_4				     = 00140624, "";

IDCTG_IRC_COMMAND_LINE			  = 00140650, "";	
IDCTG_IRC_CLIENT			  		  = 00140651, "";	
IDCTG_IRC_NICK_LIST			  	  = 00140652, "";	
IDCTG_IRC_ROOM_LIST			  	  = 00140653, "";	
IDCTG_IRC_JOIN_ROOM			  	  = 00140654, "";	
IDCTG_IRC_EXIT_ROOM			  	  = 00140655, "";	

IDCTG_IRC_SERVER_COMBO	  	  	  = 00140656, "";	
IDCTG_IRC_SERVER_TEXT	  	  	  = 00140657, "";	
IDCTG_IRC_PORT_TEXT	  	  	  	  = 00140658, "";	
IDCTG_IRC_ROOM_TEXT	  	  	  	  = 00140659, "";	
IDCTG_IRC_NICK_TEXT	  	  	  	  = 00140640, "";	
IDCTG_IRC_ALT_NICK_TEXT	  	  	  = 00140641, "";	

IDCTG_IRC_CONNECT	  	  	  		  = 00140642, "";	
IDCTG_IRC_DISCONNECT	  	  	  	  = 00140643, "";	
IDCTG_IRC_CONNECT_CANCEL  		  = 00140649, "";	
IDCTG_IRC_CONNECT_DONE  		  = 00140625, "";	

IDCTG_IRC_NEW_SERVER	  	  	  	  = 00140644, "";	
IDCTG_IRC_REMOVE_SERVER	  	  	  = 00140645, "";	

IDCTG_IRC_SERVER_DESC	  	  	  = 00140646, "";	
IDCTG_IRC_SERVER_DONE	  	  	  = 00140647, "";	
IDCTG_IRC_SERVER_CANCEL	  	  	  = 00140648, "";	

IDCTG_IRC_TOPIC	  	  	  		  = 00140626, "";	
IDCTG_IRC_OPTIONS_COMBO	  		  = 00140627, "";	
IDCTG_IRC_PEOPLE_LIST             = 00140628, "";

IDCTG_MENU_ROOT_PAGE  			  = 00140660, "";	
IDCTG_MENU_SERVER_COLUMNS  	  = 00140661, "";	
IDCTG_MENU_SERVER_FILTERS  	  = 00140662, "";	
IDCTG_MENU_PAGE_03  	  			  = 00140663, "";	
IDCTG_MENU_PAGE_04  	  			  = 00140664, "";	
IDCTG_MENU_PAGE_05  	  			  = 00140665, "";	
IDCTG_MENU_PAGE_06  	  			  = 00140666, "";	
IDCTG_MENU_PAGE_07  	  			  = 00140667, "";	
IDCTG_MENU_PAGE_08  	  			  = 00140668, "";	
IDCTG_MENU_PAGE_09  	  			  = 00140669, "";	
IDCTG_MENU_PAGE_10  	  			  = 00140670, "";	
IDCTG_MENU_PAGE_11  	  			  = 00140671, "";	
IDCTG_MENU_PAGE_12  	  			  = 00140672, "";	
IDCTG_MENU_PAGE_13  	  			  = 00140673, "";	
IDCTG_MENU_PAGE_14  	  			  = 00140674, "";	
IDCTG_MENU_PAGE_15  	  			  = 00140675, "";	
IDCTG_MENU_PAGE_16  	  			  = 00140676, "";	
IDCTG_MENU_PAGE_17  	  			  = 00140677, "";	
IDCTG_MENU_PAGE_18  	  			  = 00140678, "";	
IDCTG_MENU_PAGE_19  	  			  = 00140679, "";	

IDCTG_CFG_DETECT_KEYBOARD		  = 00140680, "";	
IDCTG_CFG_DETECT_MOUSE		  	  = 00140681, "";	

IDCTG_MASTER_MOTD		 		      = 00140900, "";

IDCTG_DIALOG			 		     = 00141000, "";

IDCTG_OBSERVER_CUROBJ           = 00141100, "";
IDCTG_OBSERVER_CURMOD           = 00141101, "";

IDCTG_IRCDLG_BAN_BTN_BAN        = 00141110, "";
IDCTG_IRCDLG_BAN_BTN_UNBAN      = 00141111, "";
IDCTG_IRCDLG_BAN_EDT_BAN        = 00141112, "";
IDCTG_IRCDLG_BAN_LBX_BAN        = 00141113, "";
IDCTG_IRCDLG_BAN_BANNER         = 00141114, "";

IDCTG_IRCDLG_INVITED_BTN_IGNORE = 00141120, "";

IDCTG_IRCDLG_JOIN_LBX_CHANNELS  = 00141125, "";
IDCTG_IRCDLG_JOIN_EDT_CHANNEL   = 00141126, "";
IDCTG_IRCDLG_JOIN_EDT_PASSWORD  = 00141127, "";
IDCTG_IRCDLG_JOIN_CHK_LISTALL   = 00141128, "";

IDCTG_IRCDLG_PROPS_CHK_HIDDEN           = 00141130, "";
IDCTG_IRCDLG_PROPS_CHK_PRIVATE          = 00141131, "";
IDCTG_IRCDLG_PROPS_EDT_TOPIC            = 00141132, "";
IDCTG_IRCDLG_PROPS_CHK_SET_USER_LIMIT   = 00141133, "";
IDCTG_IRCDLG_PROPS_EDT_USER_LIMIT       = 00141134, "";
IDCTG_IRCDLG_PROPS_CHK_SET_PASSWORD     = 00141135, "";
IDCTG_IRCDLG_PROPS_EDT_PASSWORD         = 00141136, "";
IDCTG_IRCDLG_PROPS_CHK_MODERATED        = 00141137, "";
IDCTG_IRCDLG_PROPS_CHK_SET_TOPIC_ANYONE = 00141138, "";
IDCTG_IRCDLG_PROPS_CHK_INVITE_ONLY      = 00141139, "";
IDCTG_IRCDLG_PROPS_WRP_BANNER           = 00141140, "";

// GUI resources

// fonts 150-159
IDFNT_HILITE          	        = 00150001, "hilite.pft";
IDFNT_LOLITE              	     = 00150002, "lolite.pft";
IDFNT_CONSOLE					     = 00150003, "console.pft";
IDFNT_YELLOW                    = 00150004, "yellow.pft";
IDFNT_TINY	                    = 00150005, "if_small.pft";
IDFNT_W_8	                    = 00150006, "if_w_8.pft";
IDFNT_W_10	                    = 00150007, "if_w_10.pft";
IDFNT_W_12B	                    = 00150008, "if_w_12B.pft";
IDFNT_MR_18B	                 = 00150011, "if_mr_18b.pft";
IDFNT_DR_18B	                 = 00150012, "if_dr_18b.pft";

//kludge to use the new yellow font without recreating every window
//00150009 IDFNT_MR_14B	                 'if_mr_14b.pft'
//00150010 IDFNT_DR_14B	                 'if_dr_14b.pft'
//00150013 IDFNT_MR_12B	                 'if_mr_12b.pft'
//00150014 IDFNT_DR_12B	                 'if_dr_12b.pft'
//00150015 IDFNT_MR_10B	                 'if_mr_10b.pft'
//00150016 IDFNT_DR_10B	                 'if_dr_10b.pft'
//00150017 IDFNT_R_10B	                 'if_r_10b.pft'
//00150018 IDFNT_R_12B	                 'if_r_12b.pft'

IDFNT_MR_14B	                 = 00150009, "if_y_14b.pft";
IDFNT_DR_14B	                 = 00150010, "if_y_14b.pft";
IDFNT_MR_12B	                 = 00150013, "if_y_12b.pft";
IDFNT_DR_12B	                 = 00150014, "if_y_12b.pft";
IDFNT_MR_10B	                 = 00150015, "if_y_10b.pft";
IDFNT_DR_10B	                 = 00150016, "if_y_10b.pft";
IDFNT_R_10B	            	     = 00150017, "if_w_10.pft";
IDFNT_R_12B	            	     = 00150018, "if_w_12b.pft";

IDFNT_MR_36B            	     = 00150019, "if_mr_36b.pft";
IDFNT_GR_8            		     = 00150020, "if_gr_8.pft";
IDFNT_R_8            		     = 00150021, "if_r_8.pft";
IDFNT_DR_8            		     = 00150022, "if_dr_8.pft";
IDFNT_GR_10            		     = 00150023, "if_gr_10.pft";
IDFNT_GR_12            		     = 00150024, "if_gr_12.pft";
IDFNT_GR_10B            	     = 00150025, "if_gr_10b.pft";
IDFNT_GR_12B            	     = 00150026, "if_gr_12b.pft";

IDFNT_Y_10B            		     = 00150027, "if_gr_10b.pft";
IDFNT_Y_12B            		     = 00150028, "if_gr_12b.pft";

IDFNT_8_STATIC            	     = 00150100, "if_gr_8b.pft";
IDFNT_10_STATIC                 = 00150101, "sf_grey200_10b.pft";
IDFNT_12_STATIC                 = 00150102, "if_gr_12b.pft";
IDFNT_14_STATIC                 = 00150103, "if_gr_14b.pft";
IDFNT_9_STATIC            	     = 00150104, "sf_grey200_9b.pft";
IDFNT_6_STATIC            	     = 00150105, "sf_grey200_6.pft";
IDFNT_7_STATIC            	     = 00150106, "sf_white_7.pft";

IDFNT_8_STANDARD                = 00150110, "sf_orange214_8.pft";
IDFNT_10_STANDARD               = 00150111, "sf_orange214_10.pft";
IDFNT_12_STANDARD               = 00150112, "if_y_12b.pft";
IDFNT_14_STANDARD               = 00150113, "if_y_14b.pft";
IDFNT_9_STANDARD                = 00150114, "sf_yellow200_9b.pft";
IDFNT_6_STANDARD                = 00150115, "sf_yellow200_6.pft";
IDFNT_7_STANDARD                = 00150116, "sf_orange214_7.pft";

IDFNT_8_HILITE            	     = 00150120, "sf_orange255_8.pft";
IDFNT_10_HILITE                 = 00150121, "sf_orange255_10.pft";
IDFNT_12_HILITE                 = 00150122, "if_y_12b.pft";
IDFNT_14_HILITE                 = 00150123, "if_y_14b.pft";
IDFNT_9_HILITE            	     = 00150124, "sf_white_9b.pft";
IDFNT_6_HILITE            	     = 00150125, "sf_white_6.pft";
IDFNT_7_HILITE            	     = 00150126, "sf_orange255_7.pft";

IDFNT_8_DISABLED                = 00150130, "if_gr_8b.pft";
IDFNT_10_DISABLED               = 00150131, "sf_grey100_10b.pft";
IDFNT_12_DISABLED               = 00150132, "if_gr_12b.pft";
IDFNT_14_DISABLED               = 00150133, "if_gr_14b.pft";
IDFNT_9_DISABLED                = 00150134, "sf_grey100_9b.pft";
IDFNT_6_DISABLED                = 00150135, "sf_grey100_6.pft";
IDFNT_7_DISABLED                = 00150136, "sf_grey100_7.pft";

IDFNT_8_SELECTED                = 00150160, "sf_white_9.pft";
IDFNT_10_SELECTED               = 00150161, "sf_white_10.pft";
IDFNT_12_SELECTED               = 00150162, "if_y_12b.pft";
IDFNT_14_SELECTED               = 00150163, "if_y_14b.pft";
IDFNT_9_SELECTED                = 00150164, "sf_white_9b.pft";
IDFNT_6_SELECTED                = 00150165, "sf_white_6.pft";
IDFNT_7_SELECTED                = 00150166, "sf_green_7.pft";

IDFNT_7_BLACK           	     = 00150170, "sf_black_7.pft";
IDFNT_8_BLACK           	     = 00150171, "sf_black_8.pft";
IDFNT_9_BLACK           	     = 00150172, "sf_black_9b.pft";
IDFNT_10_BLACK          	     = 00150173, "sf_black_10.pft";

IDFNT_HUD_6_STANDARD            = 00150140, "if_g_6.pft";
IDFNT_HUD_6_HILITE              = 00150141, "if_w_6.pft";
IDFNT_HUD_6_DISABLED            = 00150142, "if_dg_6.pft";
IDFNT_HUD_6_SPECIAL       	     = 00150143, "if_r_6.pft";
IDFNT_HUD_6_OTHER       	     = 00150144, "if_gr_6.pft";

IDFNT_HUD_10_STANDARD           = 00150150, "if_g_10b.pft";
IDFNT_HUD_10_HILITE             = 00150151, "if_w_10b.pft";
IDFNT_HUD_10_DISABLED           = 00150152, "if_dg_10b.pft";
IDFNT_HUD_10_SPECIAL            = 00150153, "if_r_10b.pft";
IDFNT_HUD_10_OTHER       	     = 00150154, "if_gr_10b.pft";

IDFNT_HUD_8_STANDARD            = 00150180, "if_g_8b.pft";
IDFNT_HUD_8_HILITE              = 00150181, "if_w_8b.pft";
IDFNT_HUD_8_DISABLED            = 00150182, "if_dg_8b.pft";
IDFNT_HUD_8_SPECIAL             = 00150183, "if_r_8b.pft";

// background bitmaps
IDBMP_BG1                 	     = 00160001, "BrightLogo.bmp";
IDBMP_BG2                 	     = 00160002, "Connecting.bmp";
IDBMP_BG3                 	     = 00160003, "Loading.bmp";
IDBMP_BG4                 	     = 00160004, "Dedicated.bmp";
IDBMP_BG5                 	     = 00160005, "Background0.bmp";
IDBMP_BG6                 	     = 00160006, "Background1.bmp";
IDBMP_BG7                 	     = 00160007, "Background2.bmp";
//IDBMP_SKY_DESERT_1			     = 00160500, "planet1.bmp";
//IDBMP_SKY_DESERT_2			     = 00160501, "planet2.bmp";
//IDBMP_SKY_DESERT_3			     = 00160502, "planet3.bmp";
//IDBMP_SKY_DESERT_4			     = 00160503, "planet4.bmp";
IDBMP_BG_DIVIDE1          	     = 00160504, "horz_divide1.bmp";
IDBMP_BG_DIVIDE2          	     = 00160505, "horz_divide2.bmp";
IDBMP_BG_DIALOG          	     = 00160506, "TAB_JoinRight.bmp";
IDBMP_SKIN1          		     = 00160507, "TempSkin.bmp";
IDBMP_SKY_DESERT_N256		     = 00160508, "niteplanet_256.bmp";
IDBMP_SKY_DESERT_N64		        = 00160509, "niteplanet_64.bmp";
//IDBMP_SKY_DESERT_N1		        = 00160510, "niteplanet1.bmp";
//IDBMP_SKY_DESERT_N2		        = 00160511, "niteplanet2.bmp";
//IDBMP_SKY_DESERT_D1		        = 00160512, "dayplanet1.bmp";
//IDBMP_SKY_DESERT_D2		        = 00160513, "dayplanet2.bmp";
//IDBMP_SKY_DESERT_D64		        = 00160514, "dayplanet_64.bmp";
//IDBMP_SKY_DESERT_D256	        = 00160515, "dayplanet_256.bmp";
IDBMP_SKY_ICE_GREY1		        = 00160516, "greyplanet1.bmp";
IDBMP_SKY_ICE_GREY2		        = 00160517, "greyplanet2.bmp";
IDBMP_SKY_ICE_GREY256	        = 00160518, "greyplanet_256.bmp";
IDBMP_SKY_ICE_GREY64		        = 00160519, "greyplanet_64.bmp";
//IDBMP_SKY_DESERT_TANSUN	        = 00160520, "deserttansun.bmp";
//IDBMP_SKY_LUSH_BLUESUN		     = 00160521, "bsun.bmp";
IDBMP_SKY_LUSH_DAYSUN		     = 00160522, "lushdaysun.bmp";
//IDBMP_SKY_ICE_DAY1			     = 00160523, "iceniteplanet1.bmp";
//IDBMP_SKY_ICE_DAY2			     = 00160524, "iceniteplanet2.bmp";
//IDBMP_SKY_ICE_DAY64		        = 00160525, "iceniteplanet_64.bmp";
//IDBMP_SKY_ICE_DAY256		        = 00160526, "iceniteplanet_256.bmp";
//IDBMP_SKY_LUSHCLOUD		        = 00160527, "bcloud.bmp";
IDBMP_SKY_ICEHAZYSUN				= 00160528, "iceskysun.bmp";

//00160530 IDBMP_E3_JOINUS				     'Join_Us_Small_Alpha.bmp'
IDBMP_E3_JOINUS				     = 00160530, "joinus3.bmp";
IDBMP_E3_REDFLAG				     = 00160531, "REDFLAG.bmp";
IDBMP_E3_BLUEFLAG				     = 00160532, "BLUEFLAG.bmp";


//cursors
IDBMP_CURSOR_DEFAULT            = 00160550, "CUR_Arrow.bmp, 0, 0";
IDBMP_CURSOR_HAND               = 00160551, "CUR_Hand.bmp, 8, 2";
IDBMP_CURSOR_OPENHAND           = 00160552, "CUR_OpenHand.bmp, 8, 2";
IDBMP_CURSOR_GRAB               = 00160553, "CUR_Grab.bmp, 8, 11";
IDBMP_CURSOR_IBEAM              = 00160554, "CUR_IBeam.bmp, 4, 10";
IDBMP_CURSOR_HADJUST            = 00160555, "CUR_HAdjust.bmp, 10, 8";
IDBMP_CURSOR_WAYPOINT           = 00160556, "CUR_WayPoint.bmp, 11, 13";
IDBMP_CURSOR_ZOOM      		     = 00160557, "CUR_Zoom.bmp, 7, 10";
IDBMP_CURSOR_WAYPOINT_ARROW     = 00160558, "CUR_WayPointArrow.bmp, 6, 8";
IDBMP_CURSOR_WAYPOINT_WAIT	     = 00160559, "CUR_WayPointWait.bmp, 5, 5";
IDBMP_CURSOR_CENTERING	     	  = 00160560, "CUR_Centering.bmp, 11, 13";
IDBMP_CURSOR_ROTATE             = 00160561, "cur_rotate.bmp, 11, 18";

IDBMP_BEG_SWITCH				     = 00160600, "";

IDBMP_BIGFEAR_ON				     = 00160601, "i_bigfear";
IDBMP_BIGFEAR_OFF				     = 00160602, "i_bigfear";
IDBMP_BACKPACK_ON				     = 00160603, "i_backpack";
IDBMP_BACKPACK_OFF			     = 00160604, "i_backpack";
IDBMP_WEAPON_ON				     = 00160605, "i_weapon";
IDBMP_WEAPON_OFF				     = 00160606, "i_weapon";
IDBMP_CLOCK_ON					     = 00160607, "i_clock";
IDBMP_CLOCK_OFF				     = 00160608, "i_clock";
IDBMP_COMPASS_ON				     = 00160609, "i_compass";
IDBMP_COMPASS_OFF				     = 00160610, "i_compass";
IDBMP_FEAR_ON					     = 00160611, "i_fear";
IDBMP_FEAR_OFF					     = 00160612, "i_fear";
IDBMP_RETICLE_ON				     = 00160613, "i_reticle";
IDBMP_RETICLE_OFF				     = 00160614, "i_reticle";
IDBMP_SCROLL_ON				     = 00160615, "i_scroll";
IDBMP_SCROLL_OFF				     = 00160616, "i_scroll";
IDBMP_SCROLLEND_ON			     = 00160617, "i_scrollend";
IDBMP_SCROLLEND_OFF			     = 00160618, "i_scrollend";

IDBMP_ZOOM_ON					     = 00160619, "MO_zoom";
IDBMP_ZOOM_OFF					     = 00160620, "MO_zoom";
IDBMP_CENTER_ON				     = 00160621, "MO_center";
IDBMP_CENTER_OFF				     = 00160622, "MO_center";
IDBMP_EXTRA_ON					     = 00160623, "MO_extra";
IDBMP_EXTRA_OFF				     = 00160624, "MO_extra";
IDBMP_ITEMS_ON					     = 00160625, "MO_items";
IDBMP_ITEMS_OFF				     = 00160626, "MO_items";
IDBMP_OBJECTS_ON				     = 00160627, "MO_obj";
IDBMP_OBJECTS_OFF				     = 00160628, "MO_obj";
IDBMP_PLAYERS_ON				     = 00160629, "MO_players";
IDBMP_PLAYERS_OFF				     = 00160644, "MO_players";
IDBMP_RADAR_ON					     = 00160645, "MO_radar";
IDBMP_RADAR_OFF				     = 00160646, "MO_radar";
IDBMP_TURRETS_ON				     = 00160647, "MO_turret";
IDBMP_TURRETS_OFF				     = 00160648, "MO_turret";

IDBMP_ATTACK				     	  = 00160630, "I_attack";
IDBMP_ATTACK_OFF				     = 00160631, "I_attack";
IDBMP_DEFEND_ON				     = 00160632, "I_defend";
IDBMP_DEFEND_OFF				     = 00160633, "I_defend";
IDBMP_ESCORT_ON				     = 00160634, "I_escort";
IDBMP_ESCORT_OFF				     = 00160635, "I_escort";
IDBMP_GO_TO_ON					     = 00160636, "I_go_to";
IDBMP_GO_TO_OFF				     = 00160637, "I_go_to";
IDBMP_GET_ON					     = 00160638, "I_get";
IDBMP_GET_OFF					     = 00160639, "I_get";
IDBMP_RECON_ON					     = 00160640, "I_recon";
IDBMP_RECON_OFF				     = 00160641, "I_recon";
IDBMP_REPAIR_ON				     = 00160642, "I_repair";
IDBMP_REPAIR_OFF				     = 00160643, "I_repair";
IDBMP_DEPLOY_ON				     = 00160649, "I_deploy";
IDBMP_DEPLOY_OFF				     = 00160650, "I_deploy";

IDBMP_REPORT_ON				     = 00160651, "I_report";
IDBMP_REPORT_OFF				     = 00160652, "I_report";
IDBMP_UNREPORT_ON				     = 00160653, "I_un_report";
IDBMP_UNREPORT_OFF			     = 00160654, "I_un_report";

IDBMP_CLOSE_ON					     = 00160655, "I_ok";
IDBMP_CLOSE_OFF				     = 00160656, "I_ok";

IDBMP_SHELL_RADIO_ON				  = 00160657, "RD_Check";
IDBMP_SHELL_RADIO_OFF			  = 00160658, "RD_Check";

IDBMP_MAP_ON					     = 00160660, "CMD_Map";
IDBMP_MAP_OFF					     = 00160661, "CMD_Map";
IDBMP_OBJECTIVES_ON			     = 00160662, "CMD_Objectives";
IDBMP_OBJECTIVES_OFF			     = 00160663, "CMD_Objectives";
IDBMP_HUD_MANAGER_ON			     = 00160664, "CMD_HudManager";
IDBMP_HUD_MANAGER_OFF		     = 00160665, "CMD_HudManager";
IDBMP_PLAYER_STATS_ON		     = 00160666, "CMD_PlayerStats";
IDBMP_PLAYER_STATS_OFF		     = 00160667, "CMD_PlayerStats";

IDBMP_CMDP_PLAYER_ON			     = 00160670, "CMDP_Players";
IDBMP_CMDP_PLAYER_OFF		     = 00160671, "CMDP_Players";
IDBMP_CMDP_TURRETS_ON		     = 00160672, "CMDP_Turrets";
IDBMP_CMDP_TURRETS_OFF		     = 00160673, "CMDP_Turrets";
IDBMP_CMDP_ITEMS_ON			     = 00160674, "CMDP_Items";
IDBMP_CMDP_ITEMS_OFF			     = 00160675, "CMDP_Items";
IDBMP_CMDP_ALL_ON				     = 00160676, "CMDP_All";
IDBMP_CMDP_ALL_OFF			     = 00160677, "CMDP_All";

IDBMP_LR_CMDP_PLAYER_ON		     = 00160680, "LR_CMDP_Players";
IDBMP_LR_CMDP_PLAYER_OFF	     = 00160681, "LR_CMDP_Players";
IDBMP_LR_CMDP_TURRETS_ON	     = 00160682, "LR_CMDP_Turrets";
IDBMP_LR_CMDP_TURRETS_OFF	     = 00160683, "LR_CMDP_Turrets";
IDBMP_LR_CMDP_ITEMS_ON		     = 00160684, "LR_CMDP_Items";
IDBMP_LR_CMDP_ITEMS_OFF		     = 00160685, "LR_CMDP_Items";
IDBMP_LR_CMDP_ALL_ON			     = 00160686, "LR_CMDP_All";
IDBMP_LR_CMDP_ALL_OFF		     = 00160687, "LR_CMDP_All";

IDBMP_LR_ATTACK_ON			     = 00160688, "LR_I_attack";
IDBMP_LR_ATTACK_OFF			     = 00160689, "LR_I_attack";
IDBMP_LR_DEFEND_ON			     = 00160690, "LR_I_defend";
IDBMP_LR_DEFEND_OFF			     = 00160691, "LR_I_defend";
IDBMP_LR_ESCORT_ON			     = 00160692, "LR_I_escort";
IDBMP_LR_ESCORT_OFF			     = 00160693, "LR_I_escort";
IDBMP_LR_GO_TO_ON				     = 00160694, "LR_I_go_to";
IDBMP_LR_GO_TO_OFF			     = 00160695, "LR_I_go_to";
IDBMP_LR_GET_ON				     = 00160696, "LR_I_get";
IDBMP_LR_GET_OFF				     = 00160697, "LR_I_get";
IDBMP_LR_RECON_ON				     = 00160698, "LR_I_recon";
IDBMP_LR_RECON_OFF			     = 00160699, "LR_I_recon";
IDBMP_LR_REPAIR_ON			     = 00160801, "LR_I_repair";
IDBMP_LR_REPAIR_OFF			     = 00160802, "LR_I_repair";
IDBMP_LR_DEPLOY_ON			     = 00160803, "LR_I_deploy";
IDBMP_LR_DEPLOY_OFF			     = 00160804, "LR_I_deploy";

IDBMP_LR_REPORT_ON			     = 00160805, "LR_I_report";
IDBMP_LR_REPORT_OFF			     = 00160806, "LR_I_report";
IDBMP_LR_UNREPORT_ON			     = 00160807, "LR_I_un_report";
IDBMP_LR_UNREPORT_OFF		     = 00160808, "LR_I_un_report";

IDBMP_LR_CLOSE_ON				     = 00160809, "LR_I_ok";
IDBMP_LR_CLOSE_OFF			     = 00160810, "LR_I_ok";

IDBMP_LR_CENTER_ON			     = 00160812, "LR_MO_center";
IDBMP_LR_CENTER_OFF			     = 00160813, "LR_MO_center";
IDBMP_LR_EXTRA_ON				     = 00160814, "LR_MO_extra";
IDBMP_LR_EXTRA_OFF			     = 00160815, "LR_MO_extra";
IDBMP_LR_ITEMS_ON				     = 00160816, "LR_MO_items";
IDBMP_LR_ITEMS_OFF			     = 00160817, "LR_MO_items";
IDBMP_LR_OBJECTS_ON			     = 00160818, "LR_MO_obj";
IDBMP_LR_OBJECTS_OFF			     = 00160819, "LR_MO_obj";
IDBMP_LR_PLAYERS_ON			     = 00160820, "LR_MO_players";
IDBMP_LR_PLAYERS_OFF			     = 00160821, "LR_MO_players";
IDBMP_LR_RADAR_ON				     = 00160822, "LR_MO_radar";
IDBMP_LR_RADAR_OFF			     = 00160823, "LR_MO_radar";
IDBMP_LR_TURRETS_ON			     = 00160824, "LR_MO_turret";
IDBMP_LR_TURRETS_OFF			     = 00160825, "LR_MO_turret";
IDBMP_LR_ZOOM_ON				     = 00160826, "LR_MO_zoom";
IDBMP_LR_ZOOM_OFF				     = 00160827, "LR_MO_zoom";

IDBMP_HUD_CHAIN_ON			     = 00160700, "i_chain_on.bmp";
IDBMP_HUD_DISC_ON				     = 00160701, "i_disk_on.bmp";
IDBMP_HUD_GRENADE_ON			     = 00160702, "i_grenade_on.bmp";
IDBMP_HUD_PLASMA_ON			     = 00160703, "i_plasma_on.bmp";
IDBMP_HUD_PULSE_ON			     = 00160704, "i_pulse_on.bmp";
IDBMP_HUD_SNIPER_ON			     = 00160705, "i_sniper_on.bmp";
IDBMP_HUD_CHAIN_OFF			     = 00160720, "i_chain_off.bmp";
IDBMP_HUD_DISC_OFF			     = 00160721, "i_disk_off.bmp";
IDBMP_HUD_GRENADE_OFF		     = 00160722, "i_grenade_off.bmp";
IDBMP_HUD_PLASMA_OFF			     = 00160723, "i_plasma_off.bmp";
IDBMP_HUD_PULSE_OFF			     = 00160724, "i_pulse_off.bmp";
IDBMP_HUD_SNIPER_OFF			     = 00160725, "i_sniper_off.bmp";

IDBMP_HUD_LR_CHAIN_ON			  = 00160750, "lr_chain_on.bmp";
IDBMP_HUD_LR_DISC_ON				  = 00160751, "lr_disk_on.bmp";
IDBMP_HUD_LR_GRENADE_ON			  = 00160752, "lr_grenade_on.bmp";
IDBMP_HUD_LR_PLASMA_ON			  = 00160753, "lr_plasma_on.bmp";
IDBMP_HUD_LR_PULSE_ON			  = 00160754, "lr_pulse_on.bmp";
IDBMP_HUD_LR_SNIPER_ON			  = 00160755, "lr_sniper_on.bmp";
IDBMP_HUD_LR_CHAIN_OFF			  = 00160760, "lr_chain_off.bmp";
IDBMP_HUD_LR_DISC_OFF			  = 00160761, "lr_disk_off.bmp";
IDBMP_HUD_LR_GRENADE_OFF		  = 00160762, "lr_grenade_off.bmp";
IDBMP_HUD_LR_PLASMA_OFF			  = 00160763, "lr_plasma_off.bmp";
IDBMP_HUD_LR_PULSE_OFF			  = 00160764, "lr_pulse_off.bmp";
IDBMP_HUD_LR_SNIPER_OFF			  = 00160765, "lr_sniper_off.bmp";

IDBMP_SHELL_BORDER_PLAY_ON	  	  = 00160900, "SBB_Play";
IDBMP_SHELL_BORDER_PLAY_OFF	  = 00160901, "SBB_Play";
IDBMP_SHELL_BORDER_CHAT_ON	  	  = 00160902, "SBB_Chat";
IDBMP_SHELL_BORDER_CHAT_OFF	  = 00160903, "SBB_Chat";
IDBMP_SHELL_BORDER_OPTIONS_ON	  = 00160904, "SBB_Options";
IDBMP_SHELL_BORDER_OPTIONS_OFF  = 00160905, "SBB_Options";
IDBMP_SHELL_BORDER_CANCEL_ON	  = 00160906, "SBB_Cancel";
IDBMP_SHELL_BORDER_CANCEL_OFF   = 00160907, "SBB_Cancel";

IDBMP_MENU_CIRC_NORMAL          = 00160920, "circ_n.bmp";
IDBMP_MENU_CIRC_SELECTED        = 00160921, "circ_s.bmp";
IDBMP_MENU_CIRC_DISABLED        = 00160922, "circ_d.bmp";


IDBMP_END_SWITCH        	     = 00160999, "";

// bitmap arrays
IDPBA_BOX1                	     = 00170001, "gui\fearbox1.pba";
IDPBA_SCROLL2_SHELL        	  = 00170002, "ShellScrollCtrl.PBA";
IDPBA_BUTTON        			     = 00170003, "ButtonCtrl.PBA";
IDPBA_BOX_STANDARD	           = 00170004, "BoxCtrl.PBA";
IDPBA_SCROLL_HUD    			     = 00170005, "HudScrollCtrl.PBA";
IDPBA_SHELL_TAB    			     = 00170006, "ShellTabCtrl.PBA";
IDPBA_SHELL_COLUMNS 			     = 00170007, "ShellCHeaderCtrl.PBA";
IDPBA_POPUP 					     = 00170008, "ShellPopUpCtrl.PBA";
IDPBA_BOX_DIALOG	      	     = 00170009, "DlgBoxCtrl.PBA";
IDPBA_BOX_WINDOW	      	     = 00170010, "WinBoxCtrl.PBA";
IDPBA_TEXT_EDIT	      	     = 00170011, "ShellTextCtrl.PBA";
IDPBA_COMMAND_VIEW	           = 00170012, "CmdViewCtrl.PBA";
IDPBA_COMMAND_LIST	           = 00170013, "CmdListCtrl.PBA";
IDPBA_LR_COMMAND_VIEW           = 00170014, "LRCmdViewCtrl.PBA";
IDPBA_LR_COMMAND_LIST           = 00170015, "LRCmdListCtrl.PBA";
IDPBA_OPENING           		  = 00170016, "Opening.PBA";
IDPBA_WHERE_AM_I          		  = 00170017, "CmdWhereAmI.PBA";
IDPBA_SHELL_BORDER          	  = 00170018, "ShellBorderCtrl.PBA";
IDPBA_SCROLL_SHELL       	     = 00170019, "ShellGreenScrollCtrl.PBA";
IDPBA_SLIDER_SHELL       	     = 00170020, "ShellSliderCtrl.PBA";

// palettes
IDPAL_SHELL            		     = 00180001, "Shell.ppl";

// sky DMLs
//IDDML_SKY    	                 = 00190001, "sky.dml";
IDDML_LITESKY					     = 00190002, "litesky.dml";
//IDDML_ELUSHSKY					     = 00190003, "e_lushsky.dml";
//IDDML_DESERTTANSKY			     = 00190004, "tansky.dml";
IDDML_ICEGREYSKY				     = 00190005, "greysky.dml";
//IDDML_LUSHBLUESKY				     = 00190006, "bsky.dml";
//IDDML_DESERTBLUESKY			     = 00190007, "desbluesky.dml";
//IDDML_ICEBLUESKY				     = 00190008, "icebluesky.dml";
//IDDML_LUSHPURPLESKY			     = 00190009, "sunsetsky.dml";
//IDDML_LUSHDARKSKY				     = 00190010, "darkskycrash.dml";
IDDML_DESERTSUNSETSKY				= 00190011, "nitesky.dml";

// Mission Editing Strings
IDITG_REGEN_DELAY				     = 00200001, "Regen Delay:";
IDITG_DROPPOINT_NAME			     = 00200002, "Drop Point:";
IDITG_TEAM_NUMBER				     = 00200003, "Team:";
IDITG_COUNT					        = 00200004, "Quantity:";
IDITG_ENCUMBERANCE			     = 00200005, "Encumberance:";
IDITG_WEIGHT					     = 00200006, "Weight:";
IDITG_EXCLUSIVE				     = 00200007, "Exclusive:";
IDITG_AUTO						     = 00200008, "Auto use:";
IDITG_FTS_SHAPECOLLFACES        = 00200009, "Shape uses Face collision:";
IDITG_FTS_HULKCOLLFACES         = 00200010, "Hulk uses Face collision:";
IDITG_FTS_ISSHAPETRANSPARENT    = 00200011, "Shape is Transparent (or Trans.):";
IDITG_DROPPTS_RANDOM            = 00200012, "Random Selection:";
IDITG_TURRET_SPEED				  = 00200013, "Turret speed:";
IDITG_TURRET_RANGE				  = 00200014, "Visual range:";
IDITG_FTS_DISABLECOLLISIONS     = 00200015, "Disable Collisions:";
IDITG_TRIGGER_ID					  = 00200016, "Trigger ID:";
IDITG_NAME                      = 00200017, "Name:";
IDITG_DATA                      = 00200018, "Script Data:";
IDITG_BMP_ROOT_TAG          	  = 00200100, "BMP root name:";
IDITG_OFF_BMP_TAG               = 00200101, "OBSOLETE";
IDITG_TARGET_CONTROL_TAG        = 00200102, "Target Ctrl ID:";

IDITG_IS_A_SWITCH               = 00200103, "Is an ON/OFF switch:";
IDITG_LR_OFF_BMP_TAG            = 00200104, "OBSOLETE";

IDITG_CHAT_DISPLAY_LINES        = 00200110, "# of Lines:";
IDITG_MENU_FIXED_SIZE	        = 00200120, "Max Extent:";

IDITG_SNOW_INTENSITY            = 00200130, "Snowfall Intensity";
IDITG_SNOW_WIND                 = 00200131, "Snowfall wind direction";

IDITG_TAB_PAGE_CTRL             = 00200140, "Tab Page Control:";
IDITG_TAB_SET	                 = 00200141, "Tab On:";
IDITG_TAB_ROW_COUNT             = 00200142, "Number Of Rows:";

IDITG_INV_SHOW_BUY			     = 00200143, "Show available?";

IDITG_LR_POSITION			        = 00200144, "Lo-Res Position:";
IDITG_LR_EXTENT			        = 00200145, "Lo-Res Extent:";

IDITG_BORDER_WIDTH			     = 00200146, "Border Width:";

IDITG_OPT_EXPANDED			     = 00200150, "Expanded:";

IDITG_OBJECT_NAME		     		  = 00200151, "Menu Object name:";

IDITG_MIRROR_CONS_VAR     		  = 00200152, "Mirror console var?";

IDITG_ACTION_MAP  			  	  = 00200160, "Action Map:";
IDITG_ACTION_OR_COMMAND   		  = 00200161, "Action or Command?";
IDITG_ACTION_MAKE  			  	  = 00200162, "Make Action:";
IDITG_ACTION_BREAK 			  	  = 00200163, "Break Action:";
IDITG_ACTION_MAKE_VALUE		  	  = 00200164, "Make Value:";
IDITG_ACTION_BREAK_VALUE	  	  = 00200165, "Break Value:";
IDITG_ACTION_FLAGS	   		  = 00200166, "Action";
IDITG_DEVICE   		  			  = 00200167, "Device:";
IDITG_DEVICE_KEYBOARD  			  = 00200168, "Keyboard";
IDITG_DEVICE_MOUSE	  			  = 00200169, "Mouse";
IDITG_DEVICE_JOYSTICK  			  = 00200170, "Joystick";
IDITG_DEVICE_OTHER  			  	  = 00200171, "Other";

//--------------Smacker Tags
IDITG_SMACK_TAG               = 00200172, "Smacker Tag:";
IDITG_LOOP                    = 00200173, "Loop:";
IDITG_PRELOAD                 = 00200174, "Preload:";
IDITG_PALSTART                = 00200175, "Place at pal index:";
IDITG_PALCOLORS               = 00200176, "Colors used:";
IDITG_MOVIE_DONE_TAG          = 00200177, "Movie Done Tag:";
IDITG_STRETCH                 = 00200178, "Stretch to Fit:";

IDSMK_DYNLOGO                 = 00200200, "DynLogoF.SMK";

//--------------------------------------Item related strings
//
//00210001 IDFILE_BULLET					  'bullet.dat'
//00210002 IDDTS_PR_BULLET				     'bullet.dts'
IDDTS_WP_SHOTGUN				     = 00210003, "arm";
IDDTS_WP_PLASMA				     = 00210004, "plasma";
IDDTS_AM_SHOTGUN				     = 00210005, "ammo1";
IDDTS_PL_LARMOR				     = 00210006, "larmor";
IDDTS_OB_TELEPORT				     = 00210007, "teleporter";
IDNAME_WP_SHOTGUN				     = 00210008, "Energy Glove";
IDNAME_WP_PLASMA				     = 00210009, "Plasma";
IDNAME_AM_SHOTGUN				     = 00210010, "Chaingun Shells";
IDNAME_AM_PLASMA				     = 00210011, "Plasma Rounds";
IDDTS_AM_PLASMA				     = 00210012, "plasammo";
IDDTS_PL_LFEMALE 				     = 00210013, "lfemale";
//00210014 IDDTS_PR_PLASMA				     'plasmabolt.dts'

IDDTS_PR_TURRET				     = 00210015, "fusionbolt.dts";
//00210016 IDFILE_ROCKET					  'rocket.dat'
IDNAME_WP_ROCKET				     = 00210017, "Rocket Launcher";
IDDTS_WP_ROCKET				     = 00210018, "disc";
IDNAME_AM_ROCKET				     = 00210019, "Explosive Disks";
IDDTS_AM_ROCKET				     = 00210020, "discammo";
IDDTS_OB_FIRSTAID				     = 00210022, "medpack";
IDDTS_OB_HEALING				     = 00210023, "medpack";
IDNAME_OB_FIRSTAID			     = 00210024, "First Aid";
IDNAME_OB_HEALING				     = 00210025, "Healing Kit";
IDDTS_OB_FLAG					     = 00210028, "flag";
IDNAME_OB_FLAG					     = 00210029, "Flag";
IDDTS_PL_HARMOR				     = 00210030, "harmor";
IDNAME_WP_CHAINGUN 			     = 00210031, "Chaingun";
IDDTS_WP_CHAINGUN				     = 00210032, "chaingun";
IDNAME_WP_GRENADE				     = 00210033, "Grenade Launcher";
IDNAME_AM_GRENADE				     = 00210034, "Grenades";
IDDTS_WP_GRENADE				     = 00210035, "grenadel";
IDDTS_AM_GRENADE				     = 00210036, "grenammo";
IDNAME_WP_SNIPER				     = 00210037, "Sniper Rifle";
IDDTS_WP_SNIPER				     = 00210038, "sniper";
IDNAME_AM_SNIPER				     = 00210039, "Sniper Bullets";
IDDTS_AM_SNIPER				     = 00210040, "snipammo";
IDNAME_PACK_MEDICAL			     = 00210042, "Medical Pack";
IDNAME_PACK_FUEL				     = 00210043, "Fuel Pack";
IDNAME_PACK_AMMO				     = 00210044, "Super Ammo Pack";
IDDTS_PACK_FUEL				     = 00210045, "jetpack";
IDDTS_PACK_SHIELD				     = 00210046, "shieldpack";
IDNAME_POWER_CRYSTAL			     = 00210047, "Power Crystal";
IDDTS_PACK_MEDICAL			     = 00210049, "armorpack";
IDNAME_PACK_SHIELD  			     = 00210052, "Shield Pack";
IDDTS_POWER_CRYSTALL			     = 00210053, "cryst_l";
IDNAME_WP_MINE					     = 00210054, "Anti-personel mine";
IDDTS_WP_MINE					     = 00210055, "mine";
IDNAME_WP_MINE2				     = 00210056, "Anti-vehicle mine";
IDDTS_WP_MINE2					     = 00210057, "mine2";
IDDTS_WP_MINEBOX				     = 00210058, "mineammo";
IDDTS_FLIER                     = 00210059, "flyer";
IDDTS_FM_PLATFORM               = 00210060, "platform";
IDDTS_FM_DOOR1L                 = 00210061, "door_left";
IDDTS_FM_DOOR1R                 = 00210062, "door_right";
IDDTS_FM_ELEVPAD2               = 00210063, "elevpad2";
IDDTS_FM_ELEVPAD3               = 00210064, "elevpad3";
IDDTS_FM_W64ELEVPAD             = 00210065, "w64elevpad";
IDDTS_FM_DOOR_TOP               = 00210066, "door_top";
IDDTS_FM_DOOR_BOT               = 00210067, "door_bot";
IDDTS_PACK_AMMO				     = 00210068, "ammopack";

//------------------------------------------------------------------------------
//-------------------------------------- Explosions
//
IDDTS_EXP_1					        = 00210100, "bluex.dts";
IDDTS_EXP_2					        = 00210101, "plasmaex.dts";
IDDTS_EXP_3					        = 00210102, "chainspk.dts";
IDDTS_EXP_4					        = 00210103, "fiery.dts";
IDDTS_EXP_5					        = 00210104, "shockwave.dts";
IDDTS_EXP_6					        = 00210105, "enex.dts";
IDDTS_EXP_7					        = 00210106, "tumult.dts";
IDDTS_EXP_8					        = 00210107, "fusionex.dts";

//-------------------------------------- General Explosions
IDFEAR_EXPLOSIONS_START		     = 00210200, "";
IDEXP_ROCKET					     = 00210201, "";
IDEXP_PLASMA					     = 00210202, "";
IDEXP_GRENADE					     = 00210205, "";
IDEXP_SHOCKWAVE				     = 00210206, "";
IDEXP_ENERGY					     = 00210207, "";
IDEXP_DEBRIS					     = 00210208, "";
IDEXP_TURRET					     = 00210209, "";

//-------------------------------------- Sniper randomized explosions
IDEXP_BULLETSNIPER_RICO_1		  = 00210210, "";
IDEXP_BULLETSNIPER_RICO_2		  = 00210211, "";
IDEXP_BULLETSNIPER_RICO_3		  = 00210212, "";

//-------------------------------------- BulletNormal randomized explosions
IDEXP_BULLET_RICO_1			     = 00210230, "";
IDEXP_BULLET_RICO_2			     = 00210231, "";
IDEXP_BULLET_RICO_3			     = 00210232, "";


//-------------------------------------- Shape Explosions
IDEXP_TSSHAPE_TEMP1			     = 00210300, "";
IDEXP_TSSHAPE_TEMP2			     = 00210301, "";
IDEXP_ITRSHAPE_TEMP1			     = 00210302, "";
IDEXP_ITRSHAPE_TEMP2			     = 00210303, "";

//-------------------------------------- shapes...
//IDDTS_GENERATOR			        = 00210500, "generator";
IDDTS_POWERSTATION  		        = 00210501, "enerpad";
IDDTS_AMMOSTATION	  		        = 00210502, "ammopad";
IDDTS_ITEMSTATION	  		        = 00210503, "mainpad";
//IDDTS_RADAR	  				        = 00210504, "radar";
IDDTS_TURRET	  			        = 00210505, "hellfiregun";
//IDDTS_AIR					        = 00210506, "air";
//IDDTS_ANTARRAY				        = 00210507, "anten_lava";
//IDDTS_ANTSMALL				        = 00210508, "anten_small";
//IDDTS_ANTMED				        = 00210509, "anten_med";
//IDDTS_ANTLARGE				        = 00210510, "anten_lrg";
//IDDTS_CDRCHAIR				        = 00210511, "chair";
//IDDTS_FBEACON				        = 00210512, "force";
IDDTS_MEDSUP				        = 00210513, "medical";
//IDDTS_HOVER					        = 00210514, "bridge";
IDDTS_BIGSAT				        = 00210515, "sat_big";
IDDTS_SATUP				           = 00210516, "sat_up";
IDDTS_SQPANEL				        = 00210517, "teleport_square";
IDDTS_VERTPANEL			        = 00210518, "teleport_vertical";
//IDDTS_MAGCARGO				        = 00210519, "magcargo";
//IDDTS_LIQCYL				        = 00210520, "liqcyl";
IDDTS_BLUEPANEL			        = 00210521, "panel_blue";
IDDTS_SETPANEL				        = 00210522, "panel_set";
IDDTS_VERTPANEL2			        = 00210523, "panel_vertical";
IDDTS_YELLOWPANEL			        = 00210524, "panel_yellow";
IDDTS_GAPC					        = 00210525, "gapc";
IDDTS_COMMANDTERM			        = 00210526, "command";
IDDTS_DISPLAYONE			        = 00210527, "display_one";
IDDTS_DISPLAYTWO			        = 00210528, "display_two";
IDDTS_DISPLAYTHREE			     = 00210529, "display_three";
IDDTS_ASSAULTAPC			        = 00210530, "assaultapc";
//IDDTS_ANTENROD				        = 00210531, "anten_rod";
IDDTS_CLOAKPACK			        = 00210532, "cloakpack";
IDDTS_RMTSENSOR			        = 00210533, "rmtsensor";
IDDTS_REPRPACK				        = 00210534, "reprpack";
IDDTS_SOLAR				           = 00210535, "solar";
IDDTS_PLATFORM1			        = 00210536, "platform1";
IDDTS_PLATFORM2			        = 00210537, "platform2";
IDDTS_PLATFORM3			        = 00210538, "platform3";
IDDTS_PLATFORM4			        = 00210539, "platform4";
IDDTS_DOOR1LEFT			        = 00210540, "newdoor1_l";
IDDTS_DOOR1RIGHT			        = 00210541, "newdoor1_r";
IDDTS_DOOR2LEFT			        = 00210542, "newdoor2_l";
IDDTS_DOOR2RIGHT			        = 00210543, "newdoor2_r";
IDDTS_DOOR3LEFT			        = 00210544, "newdoor3_l";
IDDTS_DOOR3RIGHT			        = 00210545, "newdoor3_r";
IDDTS_DOOR4LEFT			        = 00210546, "newdoor4_l";
IDDTS_DOOR5				           = 00210547, "newdoor5";
IDDTS_DOOR6LEFT			        = 00210548, "newdoor6_l";
IDDTS_DOOR6RIGHT			        = 00210549, "newdoor6_r";
IDDTS_CHAINTURRET			        = 00210550, "chainturret";
IDDTS_INDOORGUN			        = 00210551, "indoorgun";
IDDTS_DOOR4RIGHT			        = 00210552, "newdoor4_r";
IDDTS_MISLTURRET			        = 00210553, "missileturret";
IDDTS_PLATFORM5			        = 00210554, "platform5";
IDDTS_AMMOUNIT				        = 00210555, "ammounit";
IDDTS_CMDPNL				        = 00210556, "cmdpnl";
IDDTS_MINE_FLRG			        = 00210557, "mine_flrg";
IDDTS_TELEPORT				        = 00210558, "teleport";
IDDTS_DISPH1						= 00210559, "dsply_h1";
IDDTS_DISPH2						= 00210560, "dsply_h2";
IDDTS_DISPS1						= 00210561, "dsply_s1";
IDDTS_DISPS2						= 00210562, "dsply_s2";
IDDTS_DISPV1						= 00210563, "dsply_v1";
IDDTS_DISPV2						= 00210564, "dsply_v2";
IDDTS_ICECRYST						= 00210565, "crystal_ice";

////-------------------------------------- Actions
IDACTION_FIRE1				        = 00220001;
IDACTION_ROLL				        = 00220002;
IDACTION_PITCH				        = 00220003;
IDACTION_YAW				        = 00220004;
IDACTION_STOP				        = 00220005;
IDACTION_SETSPEED			        = 00220006;
IDACTION_MOVELEFT			        = 00220007;
IDACTION_MOVERIGHT		        = 00220008;
IDACTION_MOVEBACK			        = 00220009;
IDACTION_MOVEFORWARD		        = 00220010;
IDACTION_MOVEUP			        = 00220011;
IDACTION_MOVEDOWN			        = 00220012;
IDACTION_SHOTGUN			        = 00220013;
IDACTION_PLASMA			        = 00220014;
IDACTION_ROCKET			        = 00220015;
IDACTION_GRENADE			        = 00220016;
IDACTION_VIEW				        = 00220017;
IDACTION_FIRE2				        = 00220018;
IDACTION_BREAK1			        = 00220019;
IDACTION_CHAINGUN			        = 00220020;
IDACTION_SNIPER			        = 00220021;
IDACTION_TOGGLE			        = 00220100;
IDACTION_TURN_ON			        = 00220101;
IDACTION_TURN_OFF			        = 00220102;
IDACTION_EMPTY				        = 00220103;
IDACTION_RUN				        = 00220104;
IDACTION_STRAFE			        = 00220105;
IDACTION_USEITEM			        = 00220106;
IDACTION_JET				        = 00220107;
IDACTION_SEND_VALUE		        = 00220108;
IDACTION_WAVE				        = 00220109;
IDACTION_CROUCH			        = 00220110;
IDACTION_STAND				        = 00220111;
IDACTION_TOGGLE_CROUCH	        = 00220112;
IDACTION_SNIPER_FOV		        = 00220113;
IDACTION_INC_SNIPER_FOV	        = 00220114;
IDACTION_USE_PACK			        = 00220115;
IDACTION_MINE				        = 00220116;
IDACTION_CHAT					     = 00220200;
IDACTION_CHAT_DISP_SIZE		     = 00220201;
IDACTION_CHAT_DISP_PAGE		     = 00220202;
IDACTION_MENU_PAGE			     = 00220203;
IDACTION_ESCAPE_PRESSED		     = 00220204;
IDACTION_PLAY_MODE			     = 00220205;
IDACTION_COMMAND_MODE		     = 00220206;
IDACTION_ZOOM_IN                = 00220207;
IDACTION_ZOOM_OUT               = 00220208;
IDACTION_ISSUE_COMMAND          = 00220209;
IDACTION_ZOOM_MODE_ON           = 00220210;
IDACTION_ZOOM_MODE_OFF          = 00220211;
IDACTION_OBSERVER_NEXT          = 00220212;
IDACTION_OBSERVER_PREV          = 00220213;
IDACTION_OBSERVER_TOGGLE        = 00220214;
IDACTION_CMD_ACKNOWLEGED	     = 00220215;
IDACTION_CMD_UNABLE			     = 00220216;
IDACTION_CMD_DONE				     = 00220217;
IDACTION_CMD_CANCEL			     = 00220218;
IDACTION_CENTER_MODE_ON         = 00220219;
IDACTION_CENTER_MODE_OFF        = 00220220;
IDACTION_LOOKUP                 = 00220221;
IDACTION_LOOKDOWN               = 00220222;
IDACTION_TURNLEFT               = 00220223;
IDACTION_TURNRIGHT              = 00220224;
IDACTION_PITCHSPEED             = 00220225;
IDACTION_YAWSPEED               = 00220226;
IDACTION_ME_MOD1                = 00220227;
IDACTION_ME_MOD2                = 00220228;
IDACTION_ME_MOD3                = 00220229;
IDACTION_CENTERVIEW					= 00220230;

//------------Weapon death messages
IDMSG_DEATH_SHOTGUN1            = 00220300, "%V zigged when he should have zagged";
IDMSG_DEATH_PLASMA1             = 00220305, "%V got a plasma transfusion from %K";
IDMSG_DEATH_ROCKET1             = 00220310, "%V asked to see %Ks pocket rocket";
IDMSG_DEATH_CHAINGUN1           = 00220315, "%V was cut in half by %Ks chain gun";
IDMSG_DEATH_GRENADE1            = 00220320, "%V tried to catch %Ks grenade";
IDMSG_DEATH_SNIPER1             = 00220325, "%K took %V on a snipe hunt";
IDMSG_DEATH_MINE1               = 00220330, "%V should have looked before he leapt";
IDMSG_DEATH_MINEBOX1            = 00220335, "%V was in the wrong place at the wrong time";

//--- IRC stuff
IDIRC_MENUOPT_KICK               = 00220340, "";
IDIRC_MENUOPT_BAN                = 00220341, "";
IDIRC_MENUOPT_PRIVATE_CHAT       = 00220342, "";
IDIRC_MENUOPT_PING_USER          = 00220343, "";
IDIRC_MENUOPT_WHOIS_USER         = 00220344, "";
IDIRC_MENUOPT_AWAY               = 00220345, "";
IDIRC_MENUOPT_IGNORE             = 00220346, "";
IDIRC_MENUOPT_OPER               = 00220347, "";
IDIRC_MENUOPT_SPKR               = 00220348, "";
IDIRC_MENUOPT_SPEC               = 00220349, "";
IDIRC_MENUOPT_LEAVE              = 00220350, "";
IDIRC_MENUOPT_CHANNEL_PROPERTIES = 00220351, "";
IDIRC_MENUOPT_INVITE             = 00220352, "";
IDIRC_BTN_JOIN                   = 00220353, "";
IDIRC_CTL_VIEW                   = 00220354, "";

//-------------FEARMovingShape stuff
IDITG_FM_SHAPE                  = 00220400, "Shape:";
IDITG_FM_COLLISIONFORWARD       = 00220401, "Collision Forward:";
IDITG_FM_AUTOFORWARD            = 00220402, "Auto Forward:";
IDITG_FM_AUTOBACKWARD           = 00220403, "Auto Backward:";
IDITG_FM_TIMESCALE              = 00220404, "Time Scale:";
IDITG_FM_FORWARDDELAY           = 00220405, "Forward Delay:";
IDITG_FM_BACKWARDDELAY          = 00220406, "Backward Delay:";
IDITG_FM_PATHID                 = 00220407, "Path ID:";

//-------------Commander's commands
IDCMDR_BASE                     = 00220500, "";
IDCMDR_ATTACK                   = 00220501, "Attack enemy targets                     ~wCom_AttackWayPoint.wav";
IDCMDR_DEFEND                   = 00220502, "Defend friendlies at waypoint            ~wCom_DefendWay.wav";
IDCMDR_REPAIR                   = 00220503, "Repair device at waypoint                ~wCom_RepairItem.wav";
IDCMDR_GOTO                     = 00220504, "Go to waypoint                           ~wCom_GotoWayPoint.wav";
IDCMDR_RECON                    = 00220505, "Recon at waypoint and report             ~wCom_GoOffense.wav";
IDCMDR_ESCORT                   = 00220506, "Escort friendlies to waypoint            ~wCom_EscortFlag.wav";
IDCMDR_DEPLOY                   = 00220507, "Deploy at waypoint                       ~wCom_DeployMines.wav";
IDCMDR_GET               	     = 00220508, "Get objective at waypoint                ~wCom_GetEnemyFlag.wav";
IDCMDR_TEMP1              	     = 00220509, "Defend our base                          ~wCom_DefendBase.wav";
IDCMDR_TEMP2              	     = 00220510, "Secure our flag                          ~wCom_ReturnFlag.wav";
IDCMDR_BELAY              	     = 00220511, "Belay that order";

//the help tags are in help.strings.cs - do not use 220600 - 220999
//IDHELP_BASE                    = 00220600, "";
//IDHELP_END                     = 00220999, "";
