
#------------- Sound/Music
if(!$dedicated)
{
newSfx();
sfxSetFormat(22050, 16, 1);
#sfxSetMaxBuffers 32
newObject(soundVol, SimVolume, "sound.vol");

#
# Sound Profiles
#
# optional modifiers
#     SFX_IS_STREAMING
#     SFX_IS_LOOPING

sfxAdd2Dprofile(IDPRF_2D,				0.0											);
sfxAdd2Dprofile(IDPRF_2D_LOOP,		0.0,						SFX_IS_LOOPING	);
sfxAdd3Dprofile(IDPRF_NEAR,			0.0,	1.0,	100.0							);
sfxAdd3Dprofile(IDPRF_MEDIUM,			0.0,	5.0,	500.0							);
sfxAdd3Dprofile(IDPRF_FAR,				0.0,	10.0,	1000.0						);
sfxAdd3Dprofile(IDPRF_MEDIUM_LOOP,	0.0,	5.0,	500.0,	SFX_IS_LOOPING	);
sfxAdd3Dprofile(IDPRF_NEAR_LOOP, 	0.0,	1.0,	100.0, 	SFX_IS_LOOPING	);
sfxAdd3Dprofile(IDPRF_FOOTS,			0.0,	0.0,	30.0							);
#
# 3D sounds
# 
sfxAddPair	(IDSFX_BXPLO1,			IDPRF_FAR,			"BXplo1.wav");
sfxAddPair	(IDSFX_BXPLO2,			IDPRF_FAR,			"BXplo2.wav");
sfxAddPair	(IDSFX_BXPLO3,			IDPRF_FAR,			"BXplo3.wav");
sfxAddPair	(IDSFX_BXPLO4,			IDPRF_FAR,			"BXplo4.wav");
sfxAddPair	(IDSFX_DISCLOOP,		IDPRF_MEDIUM_LOOP,"Discloop.wav");
sfxAddPair	(IDSFX_SHELL1,			IDPRF_FAR,			"Dropshl1.wav");
sfxAddPair	(IDSFX_SHELL2,			IDPRF_FAR,			"Dropshl2.wav");
sfxAddPair	(IDSFX_DRYFIRE,		IDPRF_NEAR,			"Dryfire1.wav");
sfxAddPair	(IDSFX_EXPLO3,			IDPRF_FAR,			"EXPLO3.wav");
sfxAddPair	(IDSFX_EXPLO4,			IDPRF_FAR,			"Explo4.wav");
sfxAddPair	(IDSFX_FIRE,			IDPRF_MEDIUM,		"fire.wav");
sfxAddPair	(IDSFX_FLAG1,			IDPRF_NEAR,			"Flag1.wav");
sfxAddPair	(IDSFX_FLAG2,			IDPRF_NEAR,			"Flag2.wav");
sfxAddPair	(IDSFX_GRENADE,		IDPRF_MEDIUM,		"Grenade.wav");
sfxAddPair	(IDSFX_GRAZED,			IDPRF_NEAR,			"Injure1.wav");
sfxAddPair	(IDSFX_INJURED,		IDPRF_NEAR,			"Injure2.wav");
sfxAddPair	(IDSFX_NAILED,			IDPRF_NEAR,			"Injure3.wav");
sfxAddPair	(IDSFX_LANDHURT,		IDPRF_NEAR,			"Landhurt.wav");
sfxAddPair	(IDSFX_LAND_ON_GROUND,	IDPRF_NEAR,		"Land_On_Ground.wav");
sfxAddPair	(IDSFX_AUTOGUNCOOL,		IDPRF_NEAR,		"Machgun2.wav");
sfxAddPair	(IDSFX_AUTOGUN,		IDPRF_MEDIUM_LOOP,"machinegun.wav");
sfxAddPair	(IDSFX_PU_AMMO,		IDPRF_NEAR,			"Pku_ammo.wav");
sfxAddPair	(IDSFX_PU_HEALTH,		IDPRF_NEAR,			"Pku_hlth.wav");
sfxAddPair	(IDSFX_PU_WEAPON,		IDPRF_NEAR,			"Pku_weap.wav");
sfxAddPair	(IDSFX_PU_PACK,		IDPRF_NEAR,			"Pku_pack.wav");
sfxAddPair	(IDSFX_PLASMA1,		IDPRF_MEDIUM,		"Plasma1.wav");
sfxAddPair	(IDSFX_PLASMA2,		IDPRF_MEDIUM,		"Plasma2.wav");
sfxAddPair	(IDSFX_PLASMA3,		IDPRF_MEDIUM_LOOP,"Plasma3.wav");
sfxAddPair	(IDSFX_PLAYER_DEATH,	IDPRF_MEDIUM,		"player_death.wav");
sfxAddPair	(IDSFX_PLAYER_HIT,	IDPRF_NEAR,			"player_hit.wav");
sfxAddPair	(IDSFX_PLAYER_RES,	IDPRF_MEDIUM,		"reborn.wav");
sfxAddPair	(IDSFX_RICOCHE1,		IDPRF_NEAR,			"Ricoche1.wav");
sfxAddPair	(IDSFX_RICOCHE2,		IDPRF_NEAR,			"Ricoche2.wav");
sfxAddPair	(IDSFX_RICOCHE3,		IDPRF_NEAR,			"Ricoche3.wav");
sfxAddPair	(IDSFX_RIFLE1,			IDPRF_MEDIUM,		"rifle1.wav");
sfxAddPair	(IDSFX_RIFLE2,			IDPRF_MEDIUM,		"rifle2.wav");
sfxAddPair	(IDSFX_ROCKET,			IDPRF_MEDIUM,		"rocket2.wav");
sfxAddPair	(IDSFX_ROCKETA,		IDPRF_NEAR,			"rocket.wav");
sfxAddPair	(IDSFX_ROCKET3,		IDPRF_NEAR,			"rocket3.wav");
sfxAddPair	(IDSFX_SNIPER,			IDPRF_MEDIUM,		"sniper.wav");
sfxAddPair	(IDSFX_SNIPER2,		IDPRF_NEAR_LOOP,	"Sniper2.wav");
sfxAddPair	(IDSFX_JET,				IDPRF_MEDIUM_LOOP,"thrust.wav");
sfxAddPair	(IDSFX_EMPTY,			IDPRF_NEAR,			"weapon_empty.wav");
sfxAddPair	(IDSFX_ENERGYDONATE,	IDPRF_FAR,			"energy.wav");
sfxAddPair	(IDSFX_SHIELDHIT,		IDPRF_MEDIUM,		"shieldhit.wav");
sfxAddPair	(IDSFX_GENERATOR,		IDPRF_NEAR_LOOP,	"generator.wav");
sfxAddPair	(IDSFX_TURRET_SHOOT,	IDPRF_MEDIUM,		"turret.wav");
sfxAddPair	(IDSFX_ENERGYEXP,	IDPRF_MEDIUM,		"energyexp.wav");
sfxAddPair	(IDSFX_ROCKEXP,		IDPRF_MEDIUM,		"rockexp.wav");
sfxAddPair	(IDSFX_SHOCKEXP,	IDPRF_MEDIUM,		"shockexp.wav");
sfxAddPair	(IDSFX_TURRETEXP,	IDPRF_MEDIUM,		"turretexp.wav");
sfxAddPair	(IDSFX_FLIER_SHOOT, IDPRF_MEDIUM,		"flierrocket.wav");

sfxAddPair	(IDSFX_LFOOTLSOFT,	IDPRF_FOOTS,		"lfootlsoft.wav");
sfxAddPair	(IDSFX_LFOOTRSOFT,	IDPRF_FOOTS,		"lfootrsoft.wav");
sfxAddPair	(IDSFX_LFOOTLHARD,	IDPRF_FOOTS,		"lfootlsoft.wav");
sfxAddPair	(IDSFX_LFOOTRHARD,	IDPRF_FOOTS,		"lfootrsoft.wav");
sfxAddPair	(IDSFX_LFOOTLSNOW,	IDPRF_FOOTS,		"lfootlsoft.wav");
sfxAddPair	(IDSFX_LFOOTRSNOW,	IDPRF_FOOTS,		"lfootrsoft.wav");
sfxAddPair	(IDSFX_MFOOTLSOFT,	IDPRF_FOOTS,		"mfootlsoft.wav");
sfxAddPair	(IDSFX_MFOOTRSOFT,	IDPRF_FOOTS,		"mfootrsoft.wav");
sfxAddPair	(IDSFX_MFOOTLHARD,	IDPRF_FOOTS,		"mfootlsoft.wav");
sfxAddPair	(IDSFX_MFOOTRHARD,	IDPRF_FOOTS,		"mfootrsoft.wav");
sfxAddPair	(IDSFX_MFOOTLSNOW,	IDPRF_FOOTS,		"mfootlsoft.wav");
sfxAddPair	(IDSFX_MFOOTRSNOW,	IDPRF_FOOTS,		"mfootrsoft.wav");
sfxAddPair	(IDSFX_HFOOTLSOFT,	IDPRF_FOOTS,		"hfootlsoft.wav");
sfxAddPair	(IDSFX_HFOOTRSOFT,	IDPRF_FOOTS,		"hfootrsoft.wav");
sfxAddPair	(IDSFX_HFOOTLHARD,	IDPRF_FOOTS,		"hfootlsoft.wav");
sfxAddPair	(IDSFX_HFOOTRHARD,	IDPRF_FOOTS,		"hfootrsoft.wav");
sfxAddPair	(IDSFX_HFOOTLSNOW,	IDPRF_FOOTS,		"hfootlsoft.wav");
sfxAddPair	(IDSFX_HFOOTRSNOW,	IDPRF_FOOTS,		"hfootrsoft.wav");

sfxAddPair	(IDSFX_GRENADE_DEFAULT,	IDPRF_NEAR,		"thump1.wav");
sfxAddPair	(IDSFX_GRENADE_STONE,	IDPRF_NEAR,		"grenadeMetal.wav");
sfxAddPair	(IDSFX_GRENADE_METAL,	IDPRF_NEAR,		"grenadeStone.wav");

#
# Non-positioned sounds
#
sfxAddPair	(IDSFX_SNOW				,IDPRF_2D_LOOP		,"Snow.wav");
sfxAddPair	(IDSFX_RAIN				,IDPRF_2D_LOOP		,"rain.wav");
sfxAddPair	(IDSFX_TELEPORT		,IDPRF_2D	 		,"teleport1.wav");
sfxAddPair	(IDSFX_RIFLE_RELOAD	,IDPRF_2D	 		,"Rifle_Reload.wav");
sfxAddPair	(IDSFX_WEAPON2			,IDPRF_2D	 		,"Weapon2.wav");
sfxAddPair	(IDSFX_WEAPON3			,IDPRF_2D	 		,"Weapon3.wav");
sfxAddPair	(IDSFX_WEAPON4			,IDPRF_2D	 		,"Weapon4.wav");
sfxAddPair	(IDSFX_WEAPON5			,IDPRF_2D	 		,"Weapon5.wav");
sfxAddPair	(IDSFX_DISCLOAD	 	,IDPRF_2D 			,"Discload.wav");
sfxAddPair	(IDSFX_FLAGCAPTURE 	,IDPRF_2D 			,"flagcapture.wav");
sfxAddPair	(IDSFX_FLAGRETURN		,IDPRF_2D	 		,"flagreturn.wav");

#
# Doors and Elevators
#
sfxAddPair (IDSFX_DOOR1    ,IDPRF_NEAR		  ,"Door1.wav");
sfxAddPair (IDSFX_DOOR2    ,IDPRF_NEAR		  ,"Door2.wav");
sfxAddPair (IDSFX_DOOR3    ,IDPRF_NEAR		  ,"Door3.wav");
sfxAddPair (IDSFX_DOOR4    ,IDPRF_NEAR		  ,"Door4.wav");
sfxAddPair (IDSFX_DOOR5    ,IDPRF_NEAR		  ,"Door5.wav");
}
