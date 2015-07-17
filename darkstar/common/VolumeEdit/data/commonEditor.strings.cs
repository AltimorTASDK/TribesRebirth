//============================================================================================
// Common Editor strings
// Reserved: 1 million to 1.05 million

IDRGN_BEG_COMEDIT			           = 01000000; // "-- Common Editors reserve 1 to 1.05 million --";

//--------------------------------------------------------------------------------------------
// Shape Filenames
// Reserves 1 million to 1.01 million
//--------------------------------------------------------------------------------------------
IDCOMEDIT_BEG_SHAPES				     = 01000001; // "-- Shape Filenames --";

IDCOMEDIT_SHP_25M_ARROW            = 01000003, "arrow25.dts";
IDCOMEDIT_SHP_50M_ARROW            = 01000004, "arrow50.dts";

IDCOMEDIT_SHP_2M_CUBE			     = 01000005, "cube2.dts";
IDCOMEDIT_SHP_4M_CUBE			     = 01000006, "cube4.dts";
IDCOMEDIT_SHP_8M_CUBE			     = 01000007, "cube8.dts";

IDCOMEDIT_SHP_2M_PYRM			     = 01000008, "pyrm2.dts";
IDCOMEDIT_SHP_4M_PYRM			     = 01000009, "pyrm4.dts";
IDCOMEDIT_SHP_8M_PYRM			     = 01000010, "pyrm8.dts";

IDCOMEDIT_SHP_5M_ARROW_RED         = 01000011, "arrow5_r.dts";
IDCOMEDIT_SHP_5M_ARROW_GREEN       = 01000012, "arrow5_g.dts";
IDCOMEDIT_SHP_5M_ARROW_BLUE        = 01000013, "arrow5_b.dts";
IDCOMEDIT_SHP_5M_ARROW_YELLOW      = 01000014, "arrow5_y.dts";


IDCOMEDIT_END_SHAPES               = 01009999; // "-- Shape Filenames --";



//--------------------------------------------------------------------------------------------
// Inspect Window Tags
// Reserves 1.01 million to 1.02 million
//--------------------------------------------------------------------------------------------
IDCOMEDIT_BEG_ITG			      	  = 01010000; //"-- Inspect Window Tags --";

// Object Class Name (used 
IDCOMEDIT_ITG_CLASSNAME            = 01010001, "Script Class:";

// SimTrigger
IDCOMEDIT_ITG_INITIAL			     = 01010002, "Initally Enabled:";
IDCOMEDIT_ITG_CONTACT_RATE         = 01010003, "Contact Rate (secs):";
IDCOMEDIT_ITG_DIMENSIONS           = 01010004, "Dimensions:";
IDCOMEDIT_ITG_SPHERE               = 01010005, "Is Sphere:";
IDCOMEDIT_ITG_PLAYER               = 01010006, "Collide w/ Players:";
IDCOMEDIT_ITG_PROJECTILE           = 01010007, "Collide w/ Projectiles:";
IDCOMEDIT_ITG_VEHICLE              = 01010008, "Collide w/ Vehicles:";

// SimSoundSource
IDCOMEDIT_ITG_SOUND                = 01010015, "Sound ID:";
IDCOMEDIT_ITG_CHANNEL              = 01010016, "Channel:";

IDCOMEDIT_ITG_FORCE                = 01010020, "Force:";

// SimShape
IDITG_SSP_DAMAGEDISTRIBUTION       = 01010100, "Damage Distribution:";
IDITG_SSP_DAMAGEFORWARDATTEN       = 01010101, "Damage Forward Attenuation:";
IDITG_SSP_DESTRUCTEVENT            = 01010102, "Destruction Event:";
IDITG_SSP_DESTRUCTIONDAMAGE        = 01010103, "Destruction Damage:";
IDITG_SSP_SUSTAINABLEDAMAGE        = 01010104, "Sustainable Damage:";
IDITG_SSP_CURRENTDAMAGE            = 01010105, "Current Damage:";
IDITG_SSP_INDESTRUCTIBLE           = 01010106, "Indestructible:";
IDITG_SSP_DESTROYED                = 01010107, "Is Destroyed:";
IDITG_SSP_EXPLOSIONTAG             = 01010108, "Explosion Tag:";
IDITG_SSP_TESTSPAWNEXP             = 01010109, "Test Spawn Explosion";
IDITG_SSP_LOCALLYCONSERVEDAMAGE    = 01010110, "Locally conserve damage:";
IDITG_SSP_GLOBALLYCONSERVEDAMAGE   = 01010111, "Globally conserve damage:";

// SimShapeGroupRep
IDITG_SSGR_OBJECTREF               = 01010200, "Referenced Object:";

// SimTSShape
IDITG_STSS_SEQUENCE0_AUTOSTART     = 01010300, "(Sequence01) AutoStart:";
IDITG_STSS_SEQUENCE0_LOOPING       = 01010301, "Loop:";
IDITG_STSS_SEQUENCE0_DURATION      = 01010302, "Duration:";
IDITG_STSS_SEQUENCE1_AUTOSTART     = 01010303, "(Sequence02) AutoStart:";
IDITG_STSS_SEQUENCE1_LOOPING       = 01010304, "Loop:";
IDITG_STSS_SEQUENCE1_DURATION      = 01010305, "Duration:";
IDITG_STSS_SEQUENCE2_AUTOSTART     = 01010306, "(Sequence03) AutoStart:";
IDITG_STSS_SEQUENCE2_LOOPING       = 01010307, "Loop:";
IDITG_STSS_SEQUENCE2_DURATION      = 01010308, "Duration:";
IDITG_STSS_SEQUENCE3_AUTOSTART     = 01010309, "(Sequence04) AutoStart:";
IDITG_STSS_SEQUENCE3_LOOPING       = 01010310, "Loop:";
IDITG_STSS_SEQUENCE3_DURATION      = 01010311, "Duration:";
IDITG_STSS_SEQUENCE4_AUTOSTART     = 01010312, "(Sequence05) AutoStart:";
IDITG_STSS_SEQUENCE4_LOOPING       = 01010313, "Loop:";
IDITG_STSS_SEQUENCE4_DURATION      = 01010314, "Duration:";
IDITG_STSS_SEQUENCE5_AUTOSTART     = 01010315, "(Sequence06) AutoStart:";
IDITG_STSS_SEQUENCE5_LOOPING       = 01010316, "Loop:";
IDITG_STSS_SEQUENCE5_DURATION      = 01010317, "Duration:";
IDITG_STSS_SEQUENCE6_AUTOSTART     = 01010318, "(Sequence07) AutoStart:";
IDITG_STSS_SEQUENCE6_LOOPING       = 01010319, "Loop:";
IDITG_STSS_SEQUENCE6_DURATION      = 01010320, "Duration:";
IDITG_STSS_SEQUENCE7_AUTOSTART     = 01010321, "(Sequence08) AutoStart:";
IDITG_STSS_SEQUENCE7_LOOPING       = 01010322, "Loop:";
IDITG_STSS_SEQUENCE7_DURATION      = 01010323, "Duration:";
IDITG_STSS_SHAPEFILENAME           = 01010324, "Shape File Name:";
IDITG_STSS_HULKFILENAME            = 01010325, "Hulk File Name:";

// SimInteriorShape
IDITG_SIS_ISCONTAINER              = 01010400, "Is Container:";
IDITG_SIS_INITIALSTATE             = 01010401, "Initial State:";
IDITG_SIS_DESTROYEDSTATE           = 01010402, "Destroyed State:";
IDITG_SIS_LIGHTSTATE               = 01010403, "Light State:";
IDITG_SIS_ACTIVEINIT               = 01010404, "Active on Init:";
IDITG_SIS_ANIMTIME                 = 01010405, "Animation Time:";
IDITG_SIS_MAXLIGHTUPDATEPERSEC     = 01010406, "Maximum Light Update Hz:";
IDITG_SIS_DISABLECOLLISIONS        = 01010407, "Disable Collisions:";

// simVolumetric
IDITG_SV_POSITION                  = 01010500, "Box Position:";
IDITG_SV_DIMENSIONS                = 01010501, "Box Dimensions:";
IDITG_SV_TEXTURE_SCALE             = 01010502, "Texture Scale:";
IDITG_SV_TRANS_LEVEL               = 01010503, "Translucency Level:";
IDITG_SV_ANIM_SCALE                = 01010504, "Anim Cycles / Sec:";
IDITG_SV_TOGGLES                   = 01010505, "Flags (upper=ON):";
IDITG_SV_TOGGLE_STR                = 01010506, "Top Down Left Right Front Back:";
IDITG_SV_TOGGLE_PARSE_UPPER        = 01010507, "TDLRFB:";
IDITG_SV_TOGGLE_PARSE_LOWER        = 01010508, "tdlrfb:";
IDITG_SV_BOX_EXTRA                 = 01010509, "Box Extra:";
IDITG_SV_CURRENT_VEL               = 01010510, "Vel. of Current:";
IDITG_SV_CURRENT_DRAG              = 01010511, "Drag of Current:";
IDITG_SV_DENSITY                   = 01010512, "Density:";
IDITG_SV_DAMAGE                    = 01010513, "Damage / Sec:";
IDITG_SV_DML_FNAME                 = 01010514, "Material List:";
IDITG_SV_OBJECT_TYPE               = 01010515, "Object Type:";
IDITG_SV_CONTAINER_DRAG            = 01010516, "Container Drag:";
IDITG_SV_BOX_DIVISIONS             = 01010517, "Number of Divisions:";
        // volume types:
IDITG_SV_TYPE_WATER                = 01010520, "Water";
IDITG_SV_TYPE_AIR                  = 01010521, "Air Current";
IDITG_SV_TYPE_ACID                 = 01010522, "Acid";


IDCOMEDIT_END_ITG				        = 01019999; // "-- Inspect Window Tags --";

IDRGN_END_COMEDIT			           = 01049999; // "-- Common Editors reserve 1 to 1.05 million --";

