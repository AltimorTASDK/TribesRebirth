//============================================================================================
// DarkStar Region reserves tags 0 - 99,999
// 
// *IMPORTANT, PLEASE READ*
//     Do not define SimTag id zero.  Darkstar relies on the fact 
//     that SimTag id zero is never defined.
//
//     The SimTags are grouped into 3 sections: REGION, RESOURCE, and DATA.
//     A REGION holds one more more RESOURCE and DATA sections.
//     Tags in a DATA group will be translated during a foriegn language conversion.
//     Tags in the RESOURCE group will NOT.
//
//============================================================================================
IDRGN_BEG_DARKSTAR         = 00000001, "- DarkStar REGION reserves tags 0 - 99,999 -";

//--------------------------------------------------------------------------------------------
// System Group reserves tags 2 - 19,999
//
// Used for SimTags specific to darkstar
//--------------------------------------------------------------------------------------------
IDRES_BEG_SYSTEM           = 00000002, "-- System RESOURCE group reserves tags 2 - 19,999 --";
IDSYS_SIBLING_DISABLE      = 00000003, "Sibling disable";
IDRES_END_SYSTEM           = 00019999, "-- System RESROUCE group reserves tags 2 - 19,999 --";

//--------------------------------------------------------------------------------------------
// Inspect Group reserves tags 20,000 - 39,999
//
// Used for desciption labels in the Inspect window
//--------------------------------------------------------------------------------------------
IDDAT_BEG_INSP             = 00020000, "-- Inspect DATA group reserves tags 20,000 - 39,999 --";
// these tags have migrated to editor.strings.ttag
IDDAT_END_INSP             = 00039999, "-- Inspect DATA group reserves tags 20,000 - 39,999 --";

IDRGN_END_DARKSTAR         = 00099999, "- DarkStar REGION reserves tags 0 - 99,999 -";

//============================================================================================
// SimGui Region reserves tags 100,000 - 199,999
//
// Defines ranges that must followed when setting user defined GUI tags.  The SimGui
// engine operates using these ranges.
//============================================================================================
IDRGN_BEG_SIMGUI           = 00100000, "- SimGui REGION reserves tags 100,000 - 199,999 -";

//--------------------------------------------------------------------------------------------
// Gui Group reserves SimTags 100,001 - 109,999
//
// Used to list .gui's 
//--------------------------------------------------------------------------------------------
IDRES_BEG_GUI              = 00100001, "-- Gui RESOURCE group reserves tags 100,001-109,999 --";
IDRES_END_GUI              = 00109999, "-- Gui RESOURCE group reserves tags 100,001-109,999 --";

//--------------------------------------------------------------------------------------------
// DLG Group reserves SimTags 110,000 - 119,999
//
// Used to list .dlgs (dialogs)
//--------------------------------------------------------------------------------------------
IDRES_BEG_DLG              = 00110000, "-- Dlg RESOURCE group reserves tags 110,000-119,999 --";
IDDLG_OK                   = 00110001, "Reserved dlg command";
IDDLG_CANCEL               = 00110002, "Reserved dlg command";
IDRES_END_DLG              = 00119999, "-- Dlg RESOURCE group reserves tags 110,000-119,999 --";

//--------------------------------------------------------------------------------------------
// CMD Group reserves SimTags 120,000 - 129,999
//
// Used to list console commands. (ie, IDCMD_)
//--------------------------------------------------------------------------------------------
IDRES_BEG_CMD              = 00120000, "-- CMD RESOURCE group reserves tags 120,000-129,999 --";
IDRES_END_CMD              = 00129999, "-- CMD RESOURCE group reserves tags 120,000-129,999 --'";

IDDAT_BEG_GUI_STR          = 00130000, "-- GUI STR DATA group reserves tags 130,000-139,999 --";
IDSTR_STRING_DEFAULT       = 00139998, "TestString";
IDDAT_END_GUI_STR          = 00139999, "-- GUI STR DATA group reserves tags 130,000-139,999 --";

IDRES_BEG_GUI_MSC          = 00140000, "-- GUI Misc. group reserves tags 140,000-149,999 --";
IDRES_END_GUI_MSC          = 00149999, "-- GUI Misc. group reserves tags 140,000-149,999 --";

IDRES_BEG_GUI_FNT          = 00150000, "-- GUI FONT RESOURCE group reserves tags 150,000-159,999 --";
IDFNT_EDITOR               = 00159996, "mefont.pft";
IDFNT_EDITOR_HILITE        = 00159997, "mefonthl.pft";
IDFNT_FONT_DEFAULT         = 00159998, "darkfont.pft";
IDRES_END_GUI_FNT          = 00159999, "-- GUI FONT RESOURCE group reserves tags 150,000-159,999 --";

IDRES_BEG_GUI_BMP          = 00160000, "-- GUI BMP RESOURCE group reserves tags 160,000-169,999 --";
IDBMP_BITMAP_DEFAULT       = 00169998, "darkbmp.bmp";
IDRES_END_GUI_BMP          = 00169999, "-- GUI BMP RESOURCE group reserves tags 160,000-169,999 --";

IDRES_BEG_GUI_PBA          = 00170000, "-- GUI PBA RESOURCE group reserves tags 170,000-179,999 --";
IDPBA_BOX_DEFAULT          = 00179997, "darkbox.pba";
IDPBA_SCROLL_DEFAULT       = 00179998, "darkscroll.pba";
IDRES_END_GUI_PBA          = 00179999, "-- GUI PBA RESOURCE group reserves tags 170,000-179,999 --";

IDRES_BEG_GUI_PAL          = 00180000, "-- GUI PAL RESOURCE group reserves tags 180,000-189,999 --";
IDPAL_PAL_DEFAULT          = 00189998, "darkpal.pal";
IDRES_END_GUI_PAL          = 00189999, "-- GUI PAL RESOURCE group reserves tags 180,000-189,999 --";

IDRGN_END_SIMGUI           = 00199999, "- SimGui REGION reserves tags 100,000 - 199,999 -";



//============================================================================================
// SimObjects Region reserves tags 1.15M - 1.2M
//
// Whatever additional definitions are required by SimObjects
//============================================================================================
IDRGN_BEG_SIMOBJ           = 01150000, "- SimObject REGION reserves tags 1.15M - 1.2M -";


IDSIMOBJ_BEG_USERWAV       = 01160000, "- User-Defined WavDefIds reserve 1.16M to 1.161M -";
IDSIMOBJ_END_USERWAV       = 01160999, "- User-Defined WavDefIds reserve 1.16M to 1.161M -";


IDRGN_END_SIMOBJ           = 01199999, "- SimObject REGION reserves tags 1.15M - 1.2M -";

