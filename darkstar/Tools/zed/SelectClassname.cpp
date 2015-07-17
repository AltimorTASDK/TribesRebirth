// SelectClassname.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "SelectClassname.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectClassname dialog

char* EndEnt[] = { "", "" };
char* Ent1[] =  { "classname", "info_intermission", "origin", "1 1 1", ""};
char* Ent2[] =  { "classname", "info_player_start", "origin", "1 1 1", ""};
char* Ent3[] =  { "classname", "info_player_deathmatch", "origin", "1 1 1", ""};
char* Ent4[] =  { "classname", "info_player_coop", "origin", "1 1 1", ""};
char* Ent5[] =  { "classname", "info_player_start2", "origin", "1 1 1", ""};
char* Ent6[] =  { "classname", "info_teleport_destination", "origin", "1 1 1", ""};

char* Ent7[] =  { "classname", "air_bubbles", "origin", "1 1 1", ""};
char* Ent8[] =  { "classname", "ambient_drip", "origin", "1 1 1", ""};
char* Ent9[] =  { "classname", "ambient_drone", "origin", "1 1 1", ""};
char* Ent10[] =  { "classname", "ambient_comp_hum", "origin", "1 1 1", ""};
char* Ent11[] =  { "classname", "ambient_swamp1", "origin", "1 1 1", ""};
char* Ent12[] =  { "classname", "ambient_swamp2", "origin", "1 1 1", ""};

char* Ent13[] =  { "classname", "func_door", "angle", "0", "speed", "100", "sounds", "1", "wait", "4", ""};
char* Ent14[] =  { "classname", "func_door_secret", "angle", "0", "speed", "10", "sounds", "1", "wait", "4", ""};
char* Ent15[] =  { "classname", "func_wall", "angle", "0", "speed", "100", "sounds", "1", "wait", "4", ""};
char* Ent16[] =  { "classname", "func_button", "angle", "0", "speed", "100", "sounds", "1", "wait", "4", ""};
char* Ent17[] =  { "classname", "func_train", "angle", "0", "speed", "100", "sounds", "1", "wait", "4", ""};
char* Ent18[] =  { "classname", "func_plat", "angle", "-1", "speed", "100", "sounds", "1", "wait", "4", "height", "128", ""};
char* Ent19[] =  { "classname", "func_dm_only", "angle", "0", "speed", "100", "sounds", "1", "wait", "4", ""};

char* Ent20[] =  { "classname", "item_cells", "origin", "1 1 1", ""};
char* Ent21[] =  { "classname", "item_rockets", "origin", "1 1 1", ""};
char* Ent22[] =  { "classname", "item_shells", "origin", "1 1 1", ""};
char* Ent23[] =  { "classname", "item_spikes", "origin", "1 1 1", ""};
char* Ent24[] =  { "classname", "item_health", "origin", "1 1 1", ""};
char* Ent25[] =  { "classname", "item_artifact_envirosuit", "origin", "1 1 1", ""};
char* Ent26[] =  { "classname", "item_artifact_super_damage", "origin", "1 1 1", ""};
char* Ent27[] =  { "classname", "item_artifact_invulnerability", "origin", "1 1 1", ""};
char* Ent28[] =  { "classname", "item_artifact_invisibility", "origin", "1 1 1", ""};
char* Ent29[] =  { "classname", "item_armorInv", "origin", "1 1 1", ""};
char* Ent30[] =  { "classname", "item_armor2", "origin", "1 1 1", ""};
char* Ent31[] =  { "classname", "item_armor1", "origin", "1 1 1", ""};
char* Ent32[] =  { "classname", "item_key1", "origin", "1 1 1", ""};
char* Ent33[] =  { "classname", "item_key2", "origin", "1 1 1", ""};
char* Ent34[] =  { "classname", "item_sigil", "origin", "1 1 1", ""};

//char* Ent35[] =  { "classname", "light", "origin", "1 1 1", "light", "300", ""};
char* Ent35[] =  { "classname", "light", "pos", "1 1 1", "color", "0.7 0.7 0.7", "distance", "0.0 0.0007 0.0", ""};
char* Ent36[] =  { "classname", "light_torch_small_walltorch", "origin", "1 1 1", "light", "300", ""};
char* Ent37[] =  { "classname", "light_flame_large_yellow", "origin", "1 1 1", "light", "300", ""};
char* Ent38[] =  { "classname", "light_flame_small_yellow", "origin", "1 1 1", "light", "300", ""};
char* Ent39[] =  { "classname", "light_flame_small_white", "origin", "1 1 1", "light", "300", ""};
char* Ent40[] =  { "classname", "light_fluoro", "origin", "1 1 1", "light", "300", ""};
char* Ent41[] =  { "classname", "light_fluorospark", "origin", "1 1 1", "light", "300", ""};


char* Ent42[] =  { "classname", "monster_army", "origin", "1 1 1", ""};
char* Ent43[] =  { "classname", "monster_dog", "origin", "1 1 1", ""};
char* Ent44[] =  { "classname", "monster_ogre", "origin", "1 1 1", ""};
char* Ent45[] =  { "classname", "monster_knight", "origin", "1 1 1", ""};
char* Ent46[] =  { "classname", "monster_zombie", "origin", "1 1 1", ""};
char* Ent47[] =  { "classname", "monster_wizard", "origin", "1 1 1", ""};
char* Ent48[] =  { "classname", "monster_demon1", "origin", "1 1 1", ""};
char* Ent49[] =  { "classname", "monster_shambler", "origin", "1 1 1", ""};
char* Ent50[] =  { "classname", "monster_boss", "origin", "1 1 1", ""};

char* Ent51[] =  { "classname", "misc_fireball", "origin", "1 1 1", ""};
char* Ent52[] =  { "classname", "misc_explobox", "origin", "1 1 1", ""};

char* Ent53[] =  { "classname", "path_corner", "origin", "1 1 1", "target", "", "targetname", "", ""};

char* Ent54[] =  { "classname", "trap_spikeshooter", "origin", "1 1 1",  ""};
char* Ent55[] =  { "classname", "trap_shooter", "origin", "1 1 1",  ""};

char* Ent56[] =  { "classname", "trigger_teleport", "style", "32", "target", "", "wait", "", "delay", "", ""};
char* Ent57[] =  { "classname", "trigger_changelevel", "style", "32", "wait", "", "delay", "", ""};
char* Ent58[] =  { "classname", "trigger_setskill", "style", "32", "wait", "", "delay", "", ""};
char* Ent59[] =  { "classname", "trigger_counter", "style", "32", "target", "", "wait", "", "delay", "", ""};
char* Ent60[] =  { "classname", "trigger_once", "style", "32", "target", "", "wait", "", "delay", "", ""};
char* Ent61[] =  { "classname", "trigger_multiple", "style", "32", "target", "", "wait", "", "delay", "", ""};
char* Ent62[] =  { "classname", "trigger_onlyregistered", "style", "32", "target", "", "wait", "", "delay", "", ""};
char* Ent63[] =  { "classname", "trigger_secret", "style", "32", "target", "", "wait", "", "delay", "", ""};
char* Ent64[] =  { "classname", "trigger_monsterjump", "style", "32", "target", "", "wait", "", "delay", "", ""};
char* Ent65[] =  { "classname", "trigger_relay", "style", "32", "target", "", "wait", "", "delay", "", ""};


char* Ent66[] =  { "classname", "weapon_supershotgun", "origin", "1 1 1", ""};
char* Ent67[] =  { "classname", "weapon_nailgun", "origin", "1 1 1", ""};
char* Ent68[] =  { "classname", "weapon_supernailgun", "origin", "1 1 1", ""};
char* Ent69[] =  { "classname", "weapon_grenadelauncher", "origin", "1 1 1", ""};
char* Ent70[] =  { "classname", "weapon_rocketlauncher", "origin", "1 1 1", ""};
char* Ent71[] =  { "classname", "weapon_lightning", "origin", "1 1 1", ""};


char **CSelectClassname::ClassnameList[] = {

	Ent1,	Ent2,	Ent3,
	Ent4,	Ent5,	Ent6,
	Ent7,	Ent8,	Ent9,
	Ent10,	Ent11,	Ent12,
	Ent13,	Ent14,	Ent15,
	Ent16,	Ent17,	Ent18, Ent19,

	Ent20,	Ent21,	Ent22,
	Ent23,	Ent24,	Ent25,
	Ent26,	Ent27,	Ent28, Ent29,
	Ent30,	Ent31,	Ent32, Ent33, Ent34,

	Ent35,	Ent36,	Ent37, Ent38, Ent39, Ent40, Ent41,

	Ent42,	Ent43,	Ent44, Ent45, Ent46, Ent47, Ent48, Ent49, Ent50,

	Ent51,	Ent52,	Ent53, Ent54, Ent55,

	Ent56,	Ent57,	Ent58, Ent59, Ent60,
	Ent61,	Ent62,	Ent63, Ent64, Ent65,

	Ent66,	Ent67,	Ent68, Ent69, Ent70, Ent71,

	EndEnt
};




CSelectClassname::CSelectClassname(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectClassname::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectClassname)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	ClassnameString = NULL;
}


void CSelectClassname::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectClassname)
	DDX_Control(pDX, IDC_LIST1, m_ListBox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectClassname, CDialog)
	//{{AFX_MSG_MAP(CSelectClassname)
	ON_LBN_SELCHANGE(IDC_LIST1, OnSelchangeList1)
	ON_LBN_DBLCLK(IDC_LIST1, OnDblclkList1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectClassname message handlers

char** CSelectClassname::GetClassname()
{

	DoModal();
	return ClassnameString;
}



BOOL CSelectClassname::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	int CurrentString = 0;
	int DataNumber;
	// add the stuff to our clistbox
	while(ClassnameList[CurrentString][0][0] != 0 ) {
		DataNumber = m_ListBox.AddString(ClassnameList[CurrentString][1] );
		m_ListBox.SetItemData(DataNumber, (DWORD)ClassnameList[CurrentString] );
		CurrentString++;
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectClassname::OnSelchangeList1() 
{
	int Sel = m_ListBox.GetCurSel();
	ClassnameString = (char**)m_ListBox.GetItemData(Sel);
}

void CSelectClassname::OnDblclkList1() 
{
	EndDialog(IDOK);	
}









