//[TAG]
//============================================================================================
// Tags for main program.
//
// Some of the tags used by Main is predefined in darkstar.strings.ttag
//============================================================================================

//--------------------------------------------------------------------------------------------
// SimGui Region reserves tags 100,000 - 199,999
//
// Commented out SimTags are predefined in darkstar.strings.ttag
//--------------------------------------------------------------------------------------------
//00100001 IDRES_BEG_GUI              '-- Gui RESOURCE group reserves tags 100,001-109,999 --'
IDGUI_START                = 00100002, "start.gui";
IDGUI_GFXTEST              = 00100003, "gfxtest.gui";
IDGUI_RRTEST               = 00100004, "rrTest.gui";
//00109999 IDRES_END_GUI              '-- Gui RESOURCE group reserves tags 100,001-109,999 --' 

//00120000 IDRES_BEG_CMD              '-- CMD RESOURCE group reserves tags 120,000-129,999 --'
IDCMD_TEST                 = 00120001, "echo \"hello world\"";
IDCMD_QUIT                 = 00120002, "quit";
IDCMD_WEBSITE              = 00120003, "shellOpen http://www.dynamix.com/futurewars";
IDCMD_SINGLEPLAYER         = 00120004, "runSinglePlayerGame";
IDCMD_MULTISTART           = 00120005, "startMultiPlayerGame";
IDCMD_MULTIJOIN            = 00120006, "joinMultiPlayerGame";
IDCMD_REFRESH              = 00120007, "refreshServers";
//00129999 IDRES_END_CMD              '-- CMD RESOURCE group reserves tags 120,000-129,999 --' 

//00130000 IDDAT_BEG_GUI_STR          '-- GUI STR DATA group reserves tags 130,000-139,999 --'
IDSTR_SINGLEPLAYER         = 00130001, "Single Player";
IDSTR_MULTISTART           = 00130002, "Start MultiPlayer";
IDSTR_MULTIJOIN            = 00130003, "Join MultiPlayer";
IDSTR_REFRESH              = 00130004, "Refresh Servers";
IDSTR_WEBSITE              = 00130006, "ES3 Web Site";
IDSTR_QUIT                 = 00130007, "Quit";
//00139999 IDDAT_END_GUI_STR          '-- GUI STR DATA group reserves tags 130,000-139,999 --'

IDCTG_NAME                 = 00140015, "";
IDCTG_SERVER_SELECT_LIST   = 00140016, "";
IDCTG_SERVER_REFRESH_LIST  = 00140017, "";

//00150000 IDRES_BEG_GUI_FNT          '-- GUI FONT RESOURCE group reserves tags 150,000-159,999 --' 
IDFNT_CONSOLE              = 00150001, "console.pft";
IDFNT_CONTROL              = 00150002, "control.pft";
//00159999 IDRES_END_GUI_FNT          '-- GUI FONT RESOURCE group reserves tags 150,000-159,999 --' 

//00160000 IDRES_BEG_GUI_BMP          '-- GUI BMP RESOURCE group reserves tags 160,000-169,999 --'
IDBMP_DARKSTAR             = 00160001, "darkstar.bmp";
IDBMP_RENDTEST             = 00160002, "rendTest.bmp";
IDBMP_ALPHATEST            = 00160003, "alphaTest.bmp";
//00169999 IDRES_END_GUI_BMP          '-- GUI BMP RESOURCE group reserves tags 160,000-169,999 --'

IDPBA_BOX                  = 00170001, "box.pba";

//00180000 IDRES_BEG_GUI_PAL          '-- GUI PAL RESOURCE group reserves tags 180,000-189,999 --'
IDPAL_MAIN                 = 00180001, "main.pal";
IDPAL_MAIN2                = 00180002, "main2.ppl";
IDPAL_TEST                 = 00180003, "test.ppl";
IDPAL_BRUCE                = 00180004, "world.pal";
//00189999 IDRES_END_GUI_PAL          '-- GUI PAL RESOURCE group reserves tags 180,000-189,999 --'

//--------------------------------------------------------------------------------------------
// Polygon control tags
//
//--------------------------------------------------------------------------------------------
IDDAT_PC_FONTTAG        = 00200000, "Font Tag:";
IDDAT_PC_TEXTURETAG     = 00200001, "Texture Tag:";
IDDAT_PC_SHOWCYCLES     = 00200002, "Show EmitPoly Cycles:";
IDDAT_PC_TRANSPARENT    = 00200003, "Render Transparent:";
IDDAT_PC_TEXTURE        = 00200004, "Render Texture";
IDDAT_PC_PERSPECTIVE    = 00200005, "Render Perspective";
IDDAT_PC_SHADE          = 00200006, "Render Shaded\",       shade)";
IDDAT_PC_HAZE           = 00200007, "Render Hazed";
IDDAT_PC_INTERP         = 00200008, "Interpolate (Shade|Haze)";
IDDAT_PC_TRIANGLE       = 00200009, "Triangle";

// Actions
IDACTION_FIRE1			=	00220001, "";
IDACTION_ROLL			=	00220002, "";
IDACTION_PITCH			=	00220003, "";
IDACTION_YAW			=	00220004, "";
IDACTION_PAUSE			=	00220005, "";
IDACTION_SETSPEED		=	00220006, "";
IDACTION_MOVELEFT		=	00220007, "";
IDACTION_MOVERIGHT		=   00220008, "";
IDACTION_MOVEBACK		=	00220009, "";
IDACTION_MOVEFORWARD	=	00220010, "";
IDACTION_MOVEUP		  	=   00220011, "";
IDACTION_MOVEDOWN		=	00220012, "";
IDACTION_SHOTGUN		=	00220013, "";
IDACTION_PLASMA			=   00220014, "";
IDACTION_ROCKET			=   00220015, "";
IDACTION_VIEW			=	00220016, "";
IDACTION_FIRE2			=	00220017, "";
IDACTION_MPITCH	  		=   00220018, "";
IDACTION_MYAW			=	00220019, "";

//--------------------------------------------------------------------------------------------
// Funct control tags
//
//--------------------------------------------------------------------------------------------
IDITG_CHANGE_STRING     = 00300000, "Change string to Hello World:";
IDITG_BACKGROUND_COLOR  = 00300001, "Background color:";
IDITG_PRESET_TXT        = 00300002, "Change text to:";
IDDAT_BEG_PRESET_TXT    = 00300003, "start of preset text group";
IDSTR_FOO               = 00300004, "foo";
IDSTR_BAT               = 00300005, "bat";
IDSTR_BAR               = 00300006, "bar";
IDDAT_END_PRESET_TXT    = 00300010, "end of preset text group";
IDSTR_RED               = 00300011, "red";
IDSTR_BLUE              = 00300012, "blue";
IDSTR_GREEN             = 00300013, "green";
IDITG_DESCRIPTION       = 00300014, "Description only";

//[END]
