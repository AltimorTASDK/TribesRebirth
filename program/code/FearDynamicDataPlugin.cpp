//----------------------------------------------------------------------------
// Description:   FEAR-specific dynamic data plugin 
//
// $Workfile$     FearDynamicDataPlugin.cpp
// $Revision$     1.0
// $Author  $     Robert Mobbs (robertm@dynamix.com)
// $Modtime $     04/11/98
//
// Copyright (c) 1998 Dynamix Incorporated.  All rights reserved
//----------------------------------------------------------------------------

#include <feardynamicdataplugin.h>

#include <shapebase.h>
#include <staticbase.h>
#include <item.h>
#include <player.h>
#include <moveablebase.h>
#include <vehicle.h>

#include <datablockmanager.h>
#include <simpartdebris.h>
#include <sensor.h>
#include <vehicle.h>
#include <flier.h>
#include <tank.h>
#include <projectile.h>
#include <projRocketDumb.h>
#include <turret.h>
#include <projBullet.h>
#include <projGrenade.h>
#include <projLaser.h>
#include <projTargetLaser.h>
#include <projSeekMissile.h>
#include <projLightning.h>
#include <projRepair.h>
#include <ts_material.h>
#include <consoleInternal.h>
#include <explosion.h>
#include <moveable.h>
#include <debris.h>
#include <sound.h>
#include <marker.h>
#include <mine.h>
#include <interiorshape.h>
#include <simResource.h>
#include <trigger.h>
#include <car.h>
#include <fearguiircjoincontrol.h>

//----------------------------------------------------------------------------

static const char *getDataTypeString(void *dptr);
static const char *getDataTypeString(void *dptr);
static const char *getDataTypeInt(void *dptr);
static const char *getDataTypeFloat(void *dptr);
static const char *getDataTypeBool(void *dptr);
static const char *getDataTypeAnimData(void *dptr);
static const char *getDataTypePoint3F(void *dptr);
static const char *getDataTypeColorF(void *dptr);
static const char *getDataTypeBox3F(void *dptr);
static const char *getDataTypeTMat3F(void *dptr);
static const char *getDataTypeItemBlock(void *dptr);
static const char *getDataTypeItemImageBlock(void *dptr);
static const char *getDataTypeProjectileBlock(void *dptr);
static const char *getDataTypeSequenceSound(void *dptr);
static const char *getDataTypeNormalVector(void *dptr);
static const char *getDataTypeSoundProfileBlock(void *dptr);
static const char *getDataTypeSoundBlock(void *dptr);
static const char *getDataTypeExplosionBlock(void *dptr);
static const char *getDataTypeDebrisBlock(void *dptr);
static const char *getDataTypeStaticDamage(void *dptr);
static const char *getDataTypeDamageSkinBlock(void *dptr);
static const char *getDataTypeLightAnimParam(void *dptr);
static const char *getDataTypeGrenadeDist(void *dptr);
static void setDataTypeString(void *dptr, int argc, const char **argv);
static void setDataTypeCaseString(void *dptr, int argc, const char **argv);
static void setDataTypeInt(void *dptr, int argc, const char **argv);
static void setDataTypeFloat(void *dptr, int argc, const char **argv);
static void setDataTypeBool(void *dptr, int argc, const char **argv);
static void setDataTypeAnimData(void *dptr, int argc, const char **argv);
static void setDataTypePoint3F(void *dptr, int argc, const char **argv);
static void setDataTypeColorF(void *dptr, int argc, const char **argv);
static void setDataTypeBox3F(void *dptr, int argc, const char **argv);
static void setDataTypeTMat3F(void *dptr, int argc, const char **argv);
static void setDataTypeItemBlock(void *dptr, int argc, const char **argv);
static void setDataTypeItemImageBlock(void *dptr, int argc, const char **argv);
static void setDataTypeProjectileBlock(void *dptr, int argc, const char **argv);
static void setDataTypeSequenceSound(void *dptr, int argc, const char **argv);
static void setDataTypeSoundFlags(void *dptr, int argc, const char **argv);
static void setDataTypeNormalVector(void *dptr, int argc, const char **argv);
static void setDataTypeSoundProfileBlock(void *dptr, int argc, const char **argv);
static void setDataTypeSoundBlock(void *dptr, int argc, const char **argv);
static void setDataTypeExplosionBlock(void *dptr, int argc, const char **argv);
static void setDataTypeDebrisBlock(void *dptr, int argc, const char **argv);
static void setDataTypeStaticDamage(void *dptr, int argc, const char **argv);
static void setDataTypeDamageSkinBlock(void *dptr, int argc, const char **argv);
static void setDataTypeLightAnimParam(void *dptr, int argc, const char **argv);
static void setDataTypeGrenadeDist(void *dptr, int argc, const char **argv);

const char *matTypeNames[] = 
{
   "Default",   
   "Concrete",  
   "Carpet",    
   "Metal",     
   "Glass",     
   "Plastic",   
   "Wood",      
   "Marble",    
   "Snow",      
   "Ice",       
   "Sand",      
   "Mud",       
   "Stone",     
   "SoftEarth", 
   "PackedEarth",
   NULL,
};

class ResourceTypeMissionScript: public ResourceType
{
  public:   
   ResourceTypeMissionScript(const char *ext = ".mis") :
	ResourceType( ResourceType::typeof(ext) )  { }
   void* construct(StreamIO *stream,int);
   void destruct(void *p);
};

static ResourceTypeMissionScript			_rescon_mis(".mis");
static ResourceTypeMissionScript			_rescon_dsc(".dsc");
static ResourceTypeMissionScript			_rescon_mac(".mac");

void* ResourceTypeMissionScript::construct(StreamIO *stream, int size)
{
   ConsoleScript *ret = new ConsoleScript;
   ret->data = new BYTE[size+1];
   stream->read(size, ret->data);
   ret->data[size] = 0; // null terminate the buffer
   ret->size = size;
   return (void*) ret;
}

void ResourceTypeMissionScript::destruct(void *p)
{
   delete (ConsoleScript *) p;
}


static const char *setMaterialProperty(CMDConsole* /*c*/, int, int argc, const char ** argv)
{
   int index = 0;
   if(argc != 4)
      return "False";

   while(matTypeNames[index] && stricmp(matTypeNames[index], argv[1]))
      index++;
   if(!matTypeNames[index])
      return "False";

   TS::DefaultMaterialProps* pDefaults = TS::DefaultMaterialProps::getDefaultProps();
   TS::DefaultMaterialProps::MaterialProps* pProps = pDefaults->m_typeProps;
   pProps[index].friction = atof(argv[2]);
   pProps[index].elasticity = atof(argv[3]);
   return "True";
}

//----------------------------------------------------------------------------

#define	Offset(x,cls) (int)&(((cls *)0)->x)

//----------------------------------------------------------------------------

FearDynamicDataPlugin::FearDynamicDataPlugin()
{
   classes = NULL;
   curClass = NULL;
}

FearDynamicDataPlugin::~FearDynamicDataPlugin()
{
   while(classes)
   {
      DataBlockClass *next = classes->nextClass;
      delete [] classes->fieldTable;
      delete classes;
      classes = next;
   }
   classes = NULL;
   delete curClass;
   
   // remove the export lines
   for(int i = 0; i < objectExportText.size(); i++)
      delete [] objectExportText[i];
}

// init is called when the plugin is added to the game
void FearDynamicDataPlugin::init()
{
   Parent::init();
   console->printf("FearDynamicDataPlugin");
   Console->addDataManager(this);
   console->addCommand(0, "setMaterialProperty", setMaterialProperty);
   console->addCommand(0, "exportObjectToScript", this);
   console->addCommand(1, "setInstantGroup", this);
   console->addCommand(2, "flushExportText", this); 
   console->addCommand(3, "addExportText", this);

   dataFieldManager.registerType(TypeString, sizeof(const char *), getDataTypeString, setDataTypeString);
   dataFieldManager.registerType(TypeCaseString, sizeof(const char *), getDataTypeString, setDataTypeCaseString);
   dataFieldManager.registerType(TypeInt, sizeof(int), getDataTypeInt,  setDataTypeInt);
   dataFieldManager.registerType(TypeFloat, sizeof(float), getDataTypeFloat,  setDataTypeFloat);
   dataFieldManager.registerType(TypeBool, sizeof(bool), getDataTypeBool,  setDataTypeBool);
   dataFieldManager.registerType(TypePoint3F, sizeof(Point3F), getDataTypePoint3F,  setDataTypePoint3F);
   dataFieldManager.registerType(TypeTMat3F, sizeof(TMat3F), getDataTypeTMat3F,  setDataTypeTMat3F);
   dataFieldManager.registerType(TypeBox3F, sizeof(Box3F), getDataTypeBox3F,  setDataTypeBox3F);
   dataFieldManager.registerType(TypeColorF, sizeof(ColorF), getDataTypeColorF,  setDataTypeColorF);
   dataFieldManager.registerType(TypeAnimData, sizeof(Player::AnimData), getDataTypeAnimData, setDataTypeAnimData);
   dataFieldManager.registerType(TypeItemBlock,sizeof(int), getDataTypeItemBlock, setDataTypeItemBlock);
   dataFieldManager.registerType(TypeItemImageBlock,sizeof(int), getDataTypeItemImageBlock, setDataTypeItemImageBlock);
   dataFieldManager.registerType(TypeProjectileBlock,sizeof(ProjectileDataType), getDataTypeProjectileBlock, setDataTypeProjectileBlock);
   dataFieldManager.registerType(TypeSequenceSound,sizeof(ShapeBase::ShapeBaseData::SequenceSound), getDataTypeSequenceSound, setDataTypeSequenceSound);
   dataFieldManager.registerType(TypeSoundFlags,sizeof(int), getDataTypeInt, setDataTypeSoundFlags);
   dataFieldManager.registerType(TypeNormalVector,sizeof(Point3F), getDataTypeNormalVector, setDataTypeNormalVector);
   dataFieldManager.registerType(TypeSoundProfileBlock,sizeof(int), getDataTypeSoundProfileBlock, setDataTypeSoundProfileBlock);
   dataFieldManager.registerType(TypeSoundBlock,sizeof(int), getDataTypeSoundBlock, setDataTypeSoundBlock);
   dataFieldManager.registerType(TypeExplosionBlock, sizeof(Int32), getDataTypeExplosionBlock, setDataTypeExplosionBlock);
   dataFieldManager.registerType(TypeDebrisBlock, sizeof(Int32), getDataTypeDebrisBlock, setDataTypeDebrisBlock);
   dataFieldManager.registerType(TypeStaticDamage, sizeof(StaticBase::StaticBaseData::Damage), getDataTypeStaticDamage, setDataTypeStaticDamage);
   dataFieldManager.registerType(TypeDamageSkinBlock, sizeof(Int32), getDataTypeDamageSkinBlock, setDataTypeDamageSkinBlock);
   dataFieldManager.registerType(TypeLightAnimParam,      sizeof(Vector<InteriorShape::LightAnimParam>), getDataTypeLightAnimParam, setDataTypeLightAnimParam);
   dataFieldManager.registerType(TypeGrenadeDist,      sizeof(float), getDataTypeGrenadeDist, setDataTypeGrenadeDist);

   // register all the persist classes
   Persistent::AbstractTaggedClass::initPersistFields();

   registerClassBegin("DamageSkinData", NULL, DataBlockManager::DamageSkinDataType);
	registerData ("bmpName",          TypeString,        Offset(bmpName, ShapeBase::DamageSkinData), ShapeBase::DamageSkinData::BmpCount);
   registerClassEnd();

   registerClassBegin("GameBase", NULL, -1);
	registerData ("className",          TypeString,        Offset(className, GameBase::GameBaseData));
   registerData ("visibleToSensor",     TypeBool,          Offset(visibleToSensor, GameBase::GameBaseData));
	registerData ("mapIcon",            TypeString,        Offset(mapIcon, GameBase::GameBaseData));
	registerData ("mapFilter",        	TypeInt,           Offset(mapFilter, GameBase::GameBaseData));
	registerData ("description",        TypeCaseString,    Offset(description, GameBase::GameBaseData));
   registerClassEnd();

   registerClassBegin("ShapeBase", "GameBase", -1);
	registerData ("damageSkinData",     TypeDamageSkinBlock,Offset(damageSkinId, ShapeBase::ShapeBaseData));
	registerData ("shapeFile",          TypeString,        Offset(fileName, ShapeBase::ShapeBaseData));
   registerData ("shieldShapeName",		TypeString,	       Offset(shieldShapeName, ShapeBase::ShapeBaseData));
	registerData ("shadowDetailMask",   TypeInt,           Offset(shadowDetailMask, ShapeBase::ShapeBaseData));
	registerData ("explosionId",        TypeExplosionBlock,Offset(explosionId, ShapeBase::ShapeBaseData));
	registerData ("debrisId",        	TypeDebrisBlock,   Offset(debrisId, ShapeBase::ShapeBaseData));
	registerData ("sequenceSound",      TypeSequenceSound, Offset(sequenceSound, ShapeBase::ShapeBaseData),ShapeBase::MaxSequenceIndex);
   registerData ("maxEnergy",	         TypeFloat,         Offset(maxEnergy, ShapeBase::ShapeBaseData));
   registerData ("maxDamage",				TypeFloat,         Offset(maxDamage, ShapeBase::ShapeBaseData));
   registerData ("repairRate",			TypeFloat,         Offset(repairRate, ShapeBase::ShapeBaseData));
   registerData ("isPerspective",      TypeBool,          Offset(isPerspective, ShapeBase::ShapeBaseData));
   registerClassEnd();

   registerClassBegin("StaticBase", "ShapeBase", -1);
   registerData ("sfxAmbient",      TypeSoundBlock,Offset(sfxAmbient, StaticBase::StaticBaseData));
   registerData ("damageLevel",     TypeStaticDamage,  Offset(damageLevel, StaticBase::StaticBaseData));
   registerData ("isTranslucent",   TypeBool, Offset(isTranslucent, StaticBase::StaticBaseData));
   registerData ("triggerRadius",	TypeFloat, Offset(outerRadius, StaticBase::StaticBaseData));
   registerData ("disableCollision", TypeBool, Offset(disableCollision, StaticBase::StaticBaseData));
   registerClassEnd();

   registerClassBegin("CarData", "StaticBase", DataBlockManager::CarDataType);
   registerClassEnd();

   registerClassBegin("TriggerData", "GameBase", DataBlockManager::TriggerDataType);
   registerClassEnd();

   registerClassBegin("VehicleData", "StaticBase", DataBlockManager::VehicleDataType);
   registerData ("mass",      		TypeFloat,	Offset(mass, Vehicle::VehicleData));
   registerData ("drag",      		TypeFloat,	Offset(drag, Vehicle::VehicleData));
   registerData ("density",   		TypeFloat,	Offset(density, Vehicle::VehicleData));
   registerData ("maxSpeed",			TypeFloat,	Offset(maxSpeed, Vehicle::VehicleData));
   registerData ("minSpeed",			TypeFloat,	Offset(minSpeed, Vehicle::VehicleData));
   registerData ("maxDamage",			TypeFloat,	Offset(maxDamage, Vehicle::VehicleData));
   registerData ("reloadDelay",		TypeFloat,	Offset(reloadDelay, Vehicle::VehicleData));
	registerData ("projectileType",  TypeProjectileBlock, Offset(projectile, Vehicle::VehicleData));
   registerData ("fireSound",			TypeSoundBlock, Offset(fireSound, Vehicle::VehicleData));
   registerData ("damageSound",		TypeSoundBlock, Offset(damageSound, Vehicle::VehicleData));
	registerData ("mountSound",		TypeSoundBlock, Offset (mountSound, Vehicle::VehicleData));
	registerData ("dismountSound",	TypeSoundBlock, Offset (dismountSound, Vehicle::VehicleData));
	registerData ("idleSound",			TypeSoundBlock, Offset (idleSound, Vehicle::VehicleData));
	registerData ("moveSound",			TypeSoundBlock, Offset (moveSound, Vehicle::VehicleData));
   registerData ("ramDamage",			TypeFloat, 	Offset(ramDamage, Vehicle::VehicleData));
   registerData ("ramDamageType",	TypeInt,		Offset(ramDamageType, Vehicle::VehicleData));
	registerData ("visibleDriver",	TypeBool,	Offset (visibleDriver, Vehicle::VehicleData));
   registerData ("driverPose",		TypeInt,		Offset(driverPose, Vehicle::VehicleData));
   registerClassEnd();

   registerClassBegin("TankData", "VehicleData", DataBlockManager::TankDataType);
   registerData ("maxGrade",		TypeFloat,		Offset(maxGrade, Tank::TankData));
	registerData ("suspension",	TypePoint3F,	Offset(suspension, Tank::TankData), 4);
   registerClassEnd();
   
   registerClassBegin("FlierData", "VehicleData", DataBlockManager::FlierDataType);
   registerData ("maxBank",				TypeFloat, Offset(maxBank, Flier::FlierData));
   registerData ("maxPitch",				TypeFloat, Offset(maxPitch, Flier::FlierData));
   registerData ("lift",					TypeFloat, Offset(lift, Flier::FlierData));
   registerData ("maxAlt",					TypeFloat, Offset(maxAlt, Flier::FlierData));
   registerData ("maxVertical",			TypeFloat, Offset(maxVertical, Flier::FlierData));
   registerData ("accel",					TypeFloat, Offset(accel, Flier::FlierData));
   registerData ("groundDamageScale",	TypeFloat, Offset(groundDamageScale, Flier::FlierData));
   registerClassEnd();

   registerClassBegin("PlayerData", "ShapeBase", DataBlockManager::PlayerDataType);
   registerData ("flameShapeName",  TypeString,    Offset(flameShapeName, Player::PlayerData));
   registerData ("canCrouch", TypeBool,     Offset(canCrouch, Player::PlayerData));
   registerData ("animData",        TypeAnimData,  Offset(animData, Player::PlayerData), Player::NUM_ANIMS);
   registerData ("maxJetSideForceFactor", TypeFloat,     Offset(maxJetSideForceFactor, Player::PlayerData));
   registerData ("maxJetForwardVelocity", TypeFloat,     Offset(maxJetForwardVelocity, Player::PlayerData));
   registerData ("minJetEnergy", TypeFloat,     Offset(minJetEnergy, Player::PlayerData));
   registerData ("jetEnergyDrain", TypeFloat,     Offset(jetEnergyDrain, Player::PlayerData));
   registerData ("jumpImpulse", TypeFloat,     Offset(jumpImpulse, Player::PlayerData));
   registerData ("jumpSurfaceMinDot", TypeFloat,     Offset(jumpSurfaceMinDot, Player::PlayerData));
   registerData ("groundForce", TypeFloat,     Offset(groundForce, Player::PlayerData));
   registerData ("groundTraction", TypeFloat,     Offset(groundTraction, Player::PlayerData));
   registerData ("maxForwardSpeed", TypeFloat,     Offset(maxForwardSpeed, Player::PlayerData));
   registerData ("maxBackwardSpeed",TypeFloat,     Offset(maxBackwardSpeed, Player::PlayerData));
   registerData ("maxSideSpeed",    TypeFloat,     Offset(maxSideSpeed, Player::PlayerData));
   registerData ("mass",            TypeFloat,     Offset(mass, Player::PlayerData));
   registerData ("drag",            TypeFloat,     Offset(drag, Player::PlayerData));
   registerData ("density",         TypeFloat,     Offset(density, Player::PlayerData));
   registerData ("jetForce",        TypeFloat,     Offset(jetForce, Player::PlayerData));
   registerData ("rFootSounds",     TypeSoundBlock,       Offset(rFootSounds, Player::PlayerData), Player::NUM_FOOTSOUNDS);
   registerData ("lFootSounds",     TypeSoundBlock,       Offset(lFootSounds, Player::PlayerData), Player::NUM_FOOTSOUNDS);
   registerData ("footPrints",      TypeInt,              Offset(footPrints,  Player::PlayerData), 2);
   registerData ("jetSound",        TypeSoundBlock,       Offset(jetSound, Player::PlayerData));
   registerData ("boxWidth",        TypeFloat,     Offset(boxWidth, Player::PlayerData));
   registerData ("boxDepth",        TypeFloat,     Offset(boxDepth, Player::PlayerData));
   registerData ("boxNormalHeight", TypeFloat,     Offset(boxNormalHeight, Player::PlayerData));
   registerData ("boxCrouchHeight", TypeFloat,     Offset(boxCrouchHeight, Player::PlayerData));
   registerData ("boxNormalHeadPercentage",  TypeFloat,     Offset(boxNormalHeadPercentage,   Player::PlayerData));
   registerData ("boxNormalTorsoPercentage", TypeFloat,     Offset(boxNormalTorsoPercentage,  Player::PlayerData));
   registerData ("boxCrouchHeadPercentage",  TypeFloat,     Offset(boxCrouchHeadPercentage,   Player::PlayerData));
   registerData ("boxCrouchTorsoPercentage", TypeFloat,     Offset(boxCrouchTorsoPercentage,  Player::PlayerData));
   registerData ("boxHeadLeftPercentage",    TypeFloat,     Offset(boxHeadLeftPercentage,     Player::PlayerData));
   registerData ("boxHeadRightPercentage",   TypeFloat,     Offset(boxHeadRightPercentage,    Player::PlayerData));
   registerData ("boxHeadBackPercentage",    TypeFloat,     Offset(boxHeadBackPercentage, Player::PlayerData));
   registerData ("boxHeadFrontPercentage",   TypeFloat,     Offset(boxHeadFrontPercentage,    Player::PlayerData));
   registerData ("minDamageSpeed",	TypeFloat,		Offset(minDamageSpeed, Player::PlayerData));
   registerData ("damageScale",		TypeFloat,		Offset(damageScale, Player::PlayerData));
   registerClassEnd();

   registerClassBegin("StaticShapeData", "StaticBase", DataBlockManager::StaticShapeDataType);
   registerClassEnd();

   registerClassBegin("MarkerData", "GameBase", DataBlockManager::MarkerDataType);
	registerData ("shapeFile",          TypeString, Offset(fileName, Marker::MarkerData));
   registerClassEnd();

   registerClassBegin("ItemData", "StaticBase", DataBlockManager::ItemDataType);
	registerData ("imageType",			TypeItemImageBlock,Offset(imageId, Item::ItemData));
	registerData ("price",           TypeInt,       Offset(price, Item::ItemData));
	registerData ("mass",            TypeFloat,     Offset(mass, Item::ItemData));
	registerData ("density",         TypeFloat,     Offset(density, Item::ItemData));
	registerData ("drag",            TypeFloat,     Offset(drag, Item::ItemData));
	registerData ("friction",        TypeFloat,     Offset(friction, Item::ItemData));
	registerData ("elasticity",      TypeFloat,     Offset(elasticity, Item::ItemData));
	registerData ("hudIcon",         TypeCaseString,Offset(hudIcon, Item::ItemData));
	registerData ("heading",         TypeCaseString,Offset(typeString, Item::ItemData));
	registerData ("showInventory",   TypeBool,      Offset(showInventory, Item::ItemData));
	registerData ("showWeaponBar",   TypeBool,      Offset(showWeaponBar, Item::ItemData));
	registerData ("hiliteOnActive",  TypeBool,      Offset(hiliteOnActive, Item::ItemData));
	registerData ("lightType",       TypeInt,       Offset(lightType, Item::ItemData));
	registerData ("lightTime",       TypeFloat,     Offset(lightTime, Item::ItemData));
	registerData ("lightRadius",     TypeFloat,     Offset(lightRadius, Item::ItemData));
	registerData ("lightColor",      TypePoint3F,   Offset(lightColor, Item::ItemData));
   registerClassEnd();
	
	registerClassBegin ("MineData", "ItemData",		DataBlockManager::MineDataType);
	registerData ("kickBackStrength",TypeFloat,		Offset (kickBackStrength, Mine::MineData));
	registerData ("explosionRadius",	TypeFloat,		Offset (explosionRadius, Mine::MineData));
	registerData ("damageValue",		TypeFloat,		Offset (damageValue, Mine::MineData));
	registerData ("damageType",		TypeInt,			Offset (damageType, Mine::MineData));
   registerClassEnd();
	
	registerClassBegin("ItemImageData", NULL, DataBlockManager::ItemImageDataType);
	registerData ("shapeFile",       TypeString,    Offset(shapeFile, Player::ItemImageData));
	registerData ("mountPoint",      TypeInt,       Offset(mountPoint, Player::ItemImageData));
	registerData ("mountOffset",     TypePoint3F,   Offset(mountOffset, Player::ItemImageData));
	registerData ("mountRotation",   TypePoint3F,   Offset(mountRotation, Player::ItemImageData));
	registerData ("firstPerson",     TypeBool,      Offset(firstPerson, Player::ItemImageData));
	registerData ("accuFire",			TypeBool,      Offset(accuFire, Player::ItemImageData));
	registerData ("weaponType",      TypeInt,       Offset(weaponType, Player::ItemImageData));
	registerData ("ammoType",        TypeItemBlock, Offset(ammoType, Player::ItemImageData));
	registerData ("activateTime",    TypeFloat,     Offset(activateTime, Player::ItemImageData));
	registerData ("fireTime",        TypeFloat,     Offset(fireTime, Player::ItemImageData));
	registerData ("reloadTime",      TypeFloat,     Offset(reloadTime, Player::ItemImageData));
	registerData ("spinUpTime",      TypeFloat,     Offset(spinUpTime, Player::ItemImageData));
	registerData ("spinDownTime",    TypeFloat,     Offset(spinDownTime, Player::ItemImageData));
	registerData ("projectileType",  TypeProjectileBlock, Offset(projectile, Player::ItemImageData));
	registerData ("minEnergy",       TypeFloat,     Offset(minEnergy, Player::ItemImageData));
	registerData ("maxEnergy",       TypeFloat,     Offset(maxEnergy, Player::ItemImageData));
	registerData ("mass",            TypeFloat,     Offset(mass, Player::ItemImageData));
	registerData ("lightType",       TypeInt,       Offset(lightType, Player::ItemImageData));
	registerData ("lightTime",       TypeFloat,     Offset(lightTime, Player::ItemImageData));
	registerData ("lightRadius",     TypeFloat,     Offset(lightRadius, Player::ItemImageData));
	registerData ("lightColor",      TypePoint3F,   Offset(lightColor, Player::ItemImageData));
	registerData ("sfxActivate",     TypeSoundBlock,Offset(sfxActivateTag, Player::ItemImageData));
	registerData ("sfxFire",         TypeSoundBlock,Offset(sfxFireTag, Player::ItemImageData));
	registerData ("sfxReady",        TypeSoundBlock,Offset(sfxReadyTag, Player::ItemImageData));
	registerData ("sfxReload",       TypeSoundBlock,Offset(sfxReloadTag, Player::ItemImageData));
	registerData ("sfxSpinUp",       TypeSoundBlock,Offset(sfxSpinUpTag, Player::ItemImageData));
	registerData ("sfxSpinDown",     TypeSoundBlock,Offset(sfxSpinDownTag, Player::ItemImageData));
   registerClassEnd();

   registerClassBegin("MoveableData", "StaticBase", DataBlockManager::MoveableDataType);
   registerData("displace",        TypeBool,       Offset(displace, Moveable::MoveableData));
   registerData("blockForward",    TypeBool,       Offset(blockForward, Moveable::MoveableData));
   registerData("blockBackward",   TypeBool,       Offset(blockBackward, Moveable::MoveableData));
   registerData("sfxStart",        TypeSoundBlock, Offset(sfxStart, Moveable::MoveableData));
   registerData("sfxStop",         TypeSoundBlock, Offset(sfxStop, Moveable::MoveableData));
   registerData("sfxRun",          TypeSoundBlock, Offset(sfxRun, Moveable::MoveableData));
   registerData("sfxBlocked",      TypeSoundBlock, Offset(sfxBlocked, Moveable::MoveableData));
   registerData("speed",           TypeFloat,      Offset(speed, Moveable::MoveableData));
   registerData("side",            TypeString,     Offset(side, Moveable::MoveableData));
   registerClassEnd();
      
   registerClassBegin("SensorData", "StaticBase", DataBlockManager::SensorDataType);
   registerData("castLOS",          TypeBool,     Offset(fCastLOS, Sensor::SensorData));
   registerData("range",            TypeInt,      Offset(iRange, Sensor::SensorData));
   registerData("dopplerVelocity",  TypeInt,      Offset(iDopplerVelocity, Sensor::SensorData));
   registerData("supression",       TypeBool,     Offset(supression, Sensor::SensorData));
   registerData("supressable",      TypeBool,     Offset(supressable, Sensor::SensorData));
   registerData("pinger",      		TypeBool,     Offset(pinger, Sensor::SensorData));
   registerClassEnd();

   registerClassBegin("TurretData", "SensorData", DataBlockManager::TurretDataType);
   registerData ("gunRange",			TypeFloat,	Offset(gunRange, Turret::TurretData));
   registerData ("speed",				TypeFloat,	Offset(speed, Turret::TurretData));
   registerData ("speedModifier",	TypeFloat, Offset(speedModifier, Turret::TurretData));
   registerData ("reloadDelay",		TypeFloat,	Offset(reloadDelay, Turret::TurretData));
	registerData ("projectileType",  TypeProjectileBlock, Offset(projectile, Turret::TurretData));
   registerData ("minGunEnergy",		TypeFloat,	Offset(minGunEnergy, Turret::TurretData));
   registerData ("maxGunEnergy",		TypeFloat,	Offset(maxGunEnergy, Turret::TurretData));
   registerData ("fireSound",			TypeSoundBlock, Offset(fireSound, Turret::TurretData));
   registerData ("activationSound",	TypeSoundBlock, Offset(activationSound, Turret::TurretData));
   registerData ("deactivateSound",	TypeSoundBlock, Offset(deactivateSound, Turret::TurretData));
   registerData ("whirSound",			TypeSoundBlock, Offset(whirSound, Turret::TurretData));
	registerData ("FOV", 				TypeFloat,	Offset(FOV, Turret::TurretData));
	registerData ("deflection", 		TypeFloat,	Offset(deflection, Turret::TurretData));
	registerData ("targetableFovRatio", TypeFloat, Offset(targetableFovRatio, Turret::TurretData));
   registerData ("isSustained",     TypeBool,   Offset(isSustained, Turret::TurretData));
   registerData ("firingTimeMS",     TypeInt,   Offset(firingTime, Turret::TurretData));
   registerData ("energyRate",       TypeFloat, Offset(energyRate, Turret::TurretData));
   registerClassEnd();

   //------------------------------------------------------------------------------   
   //-------------------------------------- Projectile data blocks...
   //
   registerClassBegin("BulletData", NULL, DataBlockManager::BulletDataType);
   registerData("bulletShapeName",        TypeString,         Offset(bulletShapeName,        Bullet::BulletData));
   registerData("explosionTag",           TypeExplosionBlock, Offset(explosionTag,           Bullet::BulletData));
   registerData("expRandCycle",           TypeInt,            Offset(expRandCycle,           Bullet::BulletData));
//   registerData("collisionRadius",        TypeFloat,          Offset(collisionRadius,        Bullet::BulletData));
   registerData("mass",                   TypeFloat,          Offset(mass,                   Bullet::BulletData));

   registerData("damageClass",            TypeInt,            Offset(damageClass,            Bullet::BulletData));
   registerData("damageValue",            TypeFloat,          Offset(damageValue,            Bullet::BulletData));
   registerData("damageType",             TypeInt,            Offset(damageType,             Bullet::BulletData));
   registerData("explosionRadius",        TypeFloat,          Offset(explosionRadius,        Bullet::BulletData));

   registerData("aimDeflection",          TypeFloat,          Offset(aimDeflection,          Bullet::BulletData));
   registerData("muzzleVelocity",         TypeFloat,          Offset(muzzleVelocity,         Bullet::BulletData));
   registerData("totalTime",              TypeFloat,          Offset(totalTime,              Bullet::BulletData));
   registerData("liveTime",               TypeFloat,          Offset(liveTime,               Bullet::BulletData));
   registerData("lightRange",             TypeFloat,          Offset(lightRange,             Bullet::BulletData));
   registerData("lightColor",             TypeColorF,         Offset(lightColor,             Bullet::BulletData));
   registerData("inheritedVelocityScale", TypeFloat,          Offset(inheritedVelocityScale, Bullet::BulletData));
   registerData("isVisible",              TypeBool,           Offset(projSpecialBool,        Bullet::BulletData));
   registerData("rotationPeriod",         TypeFloat,          Offset(rotationPeriod,         Bullet::BulletData));
   registerData("bulletHoleIndex",        TypeInt,            Offset(bulletHoleIndex,        Bullet::BulletData));

   registerData("tracerPercentage",       TypeFloat,          Offset(projSpecialTime,        Bullet::BulletData));
   registerData("tracerLength",           TypeFloat,          Offset(tracerLength,           Bullet::BulletData));

   registerData("soundId",                TypeSoundBlock,     Offset(soundId,                Bullet::BulletData));
   registerClassEnd();

   registerClassBegin("GrenadeData", NULL, DataBlockManager::GrenadeDataType);
   registerData("bulletShapeName",        TypeString,         Offset(bulletShapeName,        Grenade::GrenadeData));
   registerData("explosionTag",           TypeExplosionBlock, Offset(explosionTag,           Grenade::GrenadeData));
   registerData("collideWithOwner",       TypeBool,           Offset(collideWithOwner,       Grenade::GrenadeData));
   registerData("ownerGraceMS",           TypeInt,            Offset(ownerGraceMS,           Grenade::GrenadeData));
   registerData("collisionRadius",        TypeFloat,          Offset(collisionRadius,        Grenade::GrenadeData));
   registerData("mass",                   TypeFloat,          Offset(mass,                   Grenade::GrenadeData));
   registerData("damageClass",            TypeInt,            Offset(damageClass,            Grenade::GrenadeData));
   registerData("damageValue",            TypeFloat,          Offset(damageValue,            Grenade::GrenadeData));
   registerData("damageType",             TypeInt,            Offset(damageType,             Grenade::GrenadeData));
   registerData("explosionRadius",        TypeFloat,          Offset(explosionRadius,        Grenade::GrenadeData));
   registerData("kickBackStrength",       TypeFloat,          Offset(kickBackStrength,       Grenade::GrenadeData));
   registerData("maxLevelFlightDist",     TypeGrenadeDist,    Offset(muzzleVelocity,         Grenade::GrenadeData));
   registerData("totalTime",              TypeFloat,          Offset(totalTime,              Grenade::GrenadeData));
   registerData("liveTime",               TypeFloat,          Offset(liveTime,               Grenade::GrenadeData));
   registerData("projSpecialTime",        TypeFloat,          Offset(projSpecialTime,        Grenade::GrenadeData));
   registerData("lightRange",             TypeFloat,          Offset(lightRange,             Grenade::GrenadeData));
   registerData("lightColor",             TypeColorF,         Offset(lightColor,             Grenade::GrenadeData));
   registerData("inheritedVelocityScale", TypeFloat,          Offset(inheritedVelocityScale, Grenade::GrenadeData));
   registerData("elasticity",             TypeFloat,          Offset(elasticity,             Grenade::GrenadeData));
   registerData("smokeName",              TypeString,         Offset(pSmokeName,             Grenade::GrenadeData));
   registerData("soundId",                TypeSoundBlock,     Offset(soundId,                Grenade::GrenadeData));
   registerClassEnd();

   registerClassBegin("LaserData", NULL, DataBlockManager::LaserDataType);
   registerData("laserBitmapName",        TypeString,  Offset(pBitmapName,       LaserProjectile::LaserData));
   registerData("hitName",                TypeString,  Offset(pHitShapeName,     LaserProjectile::LaserData));
   registerData("damageConversion",       TypeFloat,   Offset(damageValue,       LaserProjectile::LaserData));
   registerData("baseDamageType",         TypeInt,     Offset(damageType,        LaserProjectile::LaserData));
   registerData("beamTime",               TypeFloat,   Offset(liveTime,          LaserProjectile::LaserData));
   registerData("lightRange",             TypeFloat,   Offset(lightRange,        LaserProjectile::LaserData));
   registerData("lightColor",             TypeColorF,  Offset(lightColor,        LaserProjectile::LaserData));
   registerData("detachFromShooter",      TypeBool,    Offset(detachFromShooter, LaserProjectile::LaserData));
   registerData("hitSoundId",                TypeSoundBlock, Offset(soundId,     LaserProjectile::LaserData));
   registerClassEnd();

   registerClassBegin("TargetLaserData", NULL, DataBlockManager::TargetLaserDataType);
   registerData("laserBitmapName",        TypeString,  Offset(pBitmapName,   TargetLaser::TargetLaserData));
   registerData("lightRange",             TypeFloat,   Offset(lightRange,    TargetLaser::TargetLaserData));
   registerData("lightColor",             TypeColorF,  Offset(lightColor,    TargetLaser::TargetLaserData));
   registerData("hitShapeName",           TypeString,  Offset(pHitShapeName, TargetLaser::TargetLaserData));
   registerClassEnd();

   registerClassBegin("LightningData", NULL, DataBlockManager::LightningDataType);
   registerData("bitmapName",             TypeString,  Offset(pBitmapName,      Lightning::LightningData));
   registerData("damageType",             TypeInt,     Offset(damageType,       Lightning::LightningData));
   registerData("boltLength",             TypeFloat,   Offset(boltLength,       Lightning::LightningData));
   registerData("coneAngle",              TypeFloat,   Offset(coneAngle,        Lightning::LightningData));
   registerData("damagePerSec",           TypeFloat,   Offset(damagePerSec,     Lightning::LightningData));
   registerData("energyDrainPerSec",      TypeFloat,   Offset(energyDrainPerSec, Lightning::LightningData));
   registerData("segmentDivisions",       TypeInt,     Offset(segmentDivisions, Lightning::LightningData));
   registerData("numSegments",            TypeInt,     Offset(numSegments,      Lightning::LightningData));
   registerData("beamWidth",              TypeFloat,   Offset(beamWidth,        Lightning::LightningData));

   registerData("updateTime",             TypeInt,     Offset(updateTime,       Lightning::LightningData));
   registerData("skipPercent",            TypeFloat,   Offset(skipPercent,      Lightning::LightningData));
   registerData("displaceBias",           TypeFloat,   Offset(displaceBias,     Lightning::LightningData));

   registerData("lightRange",             TypeFloat,   Offset(lightRange,       Lightning::LightningData));
   registerData("lightColor",             TypeColorF,  Offset(lightColor,       Lightning::LightningData));
   registerData("soundId",                TypeSoundBlock,     Offset(soundId,                Bullet::BulletData));
   registerClassEnd();

   registerClassBegin("RepairEffectData", NULL, DataBlockManager::RepairEffectDataType);
   registerData("bitmapName",             TypeString,  Offset(pBitmapName,      RepairEffect::RepairEffectData));
   registerData("boltLength",             TypeFloat,   Offset(boltLength,       RepairEffect::RepairEffectData));
   registerData("segmentDivisions",       TypeInt,     Offset(segmentDivisions, RepairEffect::RepairEffectData));
   registerData("beamWidth",              TypeFloat,   Offset(beamWidth,        RepairEffect::RepairEffectData));
   registerData("updateTime",             TypeInt,     Offset(updateTime,       RepairEffect::RepairEffectData));
   registerData("skipPercent",            TypeFloat,   Offset(skipPercent,      RepairEffect::RepairEffectData));
   registerData("displaceBias",           TypeFloat,   Offset(displaceBias,     RepairEffect::RepairEffectData));
   registerData("lightRange",             TypeFloat,   Offset(lightRange,       RepairEffect::RepairEffectData));
   registerData("lightColor",             TypeColorF,  Offset(lightColor,       RepairEffect::RepairEffectData));
   registerClassEnd();

   registerClassBegin("RocketData", NULL, DataBlockManager::RocketDataType);
   registerData("bulletShapeName",        TypeString,         Offset(bulletShapeName,        RocketDumb::RocketDumbData));
   registerData("explosionTag",           TypeExplosionBlock, Offset(explosionTag,           RocketDumb::RocketDumbData));
   registerData("collideWithOwner",       TypeBool,           Offset(collideWithOwner,       RocketDumb::RocketDumbData));
   registerData("ownerGraceMS",           TypeInt,            Offset(ownerGraceMS,           RocketDumb::RocketDumbData));
   registerData("collisionRadius",        TypeFloat,          Offset(collisionRadius,        RocketDumb::RocketDumbData));
   registerData("mass",                   TypeFloat,          Offset(mass,                   RocketDumb::RocketDumbData));

   registerData("damageClass",            TypeInt,            Offset(damageClass,            RocketDumb::RocketDumbData));
   registerData("damageValue",            TypeFloat,          Offset(damageValue,            RocketDumb::RocketDumbData));
   registerData("damageType",             TypeInt,            Offset(damageType,             RocketDumb::RocketDumbData));
   registerData("explosionRadius",        TypeFloat,          Offset(explosionRadius,        RocketDumb::RocketDumbData));

   registerData("kickBackStrength",       TypeFloat,          Offset(kickBackStrength,       RocketDumb::RocketDumbData));
   registerData("muzzleVelocity",         TypeFloat,          Offset(muzzleVelocity,         RocketDumb::RocketDumbData));
   registerData("terminalVelocity",       TypeFloat,          Offset(terminalVelocity,       RocketDumb::RocketDumbData));
   registerData("acceleration",           TypeFloat,          Offset(acceleration,           RocketDumb::RocketDumbData));
   registerData("totalTime",              TypeFloat,          Offset(totalTime,              RocketDumb::RocketDumbData));
   registerData("liveTime",               TypeFloat,          Offset(liveTime,               RocketDumb::RocketDumbData));
   registerData("lightRange",             TypeFloat,          Offset(lightRange,             RocketDumb::RocketDumbData));
   registerData("lightColor",             TypeColorF,         Offset(lightColor,             RocketDumb::RocketDumbData));
   registerData("inheritedVelocityScale", TypeFloat,          Offset(inheritedVelocityScale, RocketDumb::RocketDumbData));
   registerData("trailType",              TypeInt,            Offset(trailType,              RocketDumb::RocketDumbData));
   registerData("trailString",            TypeString,         Offset(trailString,            RocketDumb::RocketDumbData));
   registerData("trailLength",            TypeFloat,          Offset(trailLength,            RocketDumb::RocketDumbData));
   registerData("trailWidth",             TypeFloat,          Offset(trailWidth,             RocketDumb::RocketDumbData));
   registerData("smokeDist",              TypeFloat,          Offset(projSpecialTime,        RocketDumb::RocketDumbData));
   registerData("soundId",                TypeSoundBlock,     Offset(soundId,                RocketDumb::RocketDumbData));
   registerClassEnd();

   registerClassBegin("SeekingMissileData", NULL, DataBlockManager::SeekingMissileDataType);
   registerData("bulletShapeName",         TypeString,         Offset(bulletShapeName,         SeekingMissile::SeekingMissileData));
   registerData("explosionTag",            TypeExplosionBlock, Offset(explosionTag,            SeekingMissile::SeekingMissileData));
   registerData("collideWithOwner",        TypeBool,           Offset(collideWithOwner,        SeekingMissile::SeekingMissileData));
   registerData("ownerGraceMS",            TypeInt,            Offset(ownerGraceMS,            SeekingMissile::SeekingMissileData));
   registerData("collisionRadius",         TypeFloat,          Offset(collisionRadius,         SeekingMissile::SeekingMissileData));
   registerData("mass",                    TypeFloat,          Offset(mass,                    SeekingMissile::SeekingMissileData));

   registerData("damageClass",             TypeInt,            Offset(damageClass,             SeekingMissile::SeekingMissileData));
   registerData("damageValue",             TypeFloat,          Offset(damageValue,             SeekingMissile::SeekingMissileData));
   registerData("damageType",              TypeInt,            Offset(damageType,              SeekingMissile::SeekingMissileData));
   registerData("explosionRadius",         TypeFloat,          Offset(explosionRadius,         SeekingMissile::SeekingMissileData));
   registerData("kickBackStrength",        TypeFloat,          Offset(kickBackStrength,        SeekingMissile::SeekingMissileData));

   registerData("muzzleVelocity",          TypeFloat,          Offset(muzzleVelocity,          SeekingMissile::SeekingMissileData));
   registerData("totalTime",               TypeFloat,          Offset(totalTime,               SeekingMissile::SeekingMissileData));
   registerData("liveTime",                TypeFloat,          Offset(liveTime,                SeekingMissile::SeekingMissileData));
   registerData("seekingTurningRadius",    TypeFloat,          Offset(seekingTurningRadius,    SeekingMissile::SeekingMissileData));
   registerData("nonSeekingTurningRadius", TypeFloat,          Offset(nonSeekingTurningRadius, SeekingMissile::SeekingMissileData));
   registerData("proximityDist",           TypeFloat,          Offset(proximityDist,           SeekingMissile::SeekingMissileData));

   registerData("smokeDist",              TypeFloat,          Offset(projSpecialTime,        SeekingMissile::SeekingMissileData));
   registerData("lightRange",             TypeFloat,          Offset(lightRange,             SeekingMissile::SeekingMissileData));
   registerData("lightColor",             TypeColorF,         Offset(lightColor,             SeekingMissile::SeekingMissileData));
   registerData("inheritedVelocityScale", TypeFloat,          Offset(inheritedVelocityScale, SeekingMissile::SeekingMissileData));
   registerData("soundId",                TypeSoundBlock,     Offset(soundId,                SeekingMissile::SeekingMissileData));
   registerClassEnd();

   registerClassBegin("ExplosionData", NULL, DataBlockManager::ExplosionDataType);
   registerData("shapeName",     TypeString,     Offset(pShapeName,    Explosion::ExplosionData));
   registerData("soundId",       TypeSoundBlock, Offset(soundId,       Explosion::ExplosionData));
   registerData("faceCamera",    TypeBool,       Offset(faceCamera,    Explosion::ExplosionData));
   registerData("randomSpin",    TypeBool,       Offset(randomSpin,    Explosion::ExplosionData));
   registerData("hasLight",      TypeBool,       Offset(hasLight,      Explosion::ExplosionData));
   registerData("lightRange",    TypeFloat,      Offset(lightRange,    Explosion::ExplosionData));
   registerData("timeZero",      TypeFloat,      Offset(timeZero,      Explosion::ExplosionData));
   registerData("timeOne",       TypeFloat,      Offset(timeOne,       Explosion::ExplosionData));
   registerData("colors",        TypeColorF,     Offset(colors,        Explosion::ExplosionData), 3);
   registerData("radFactors",    TypeFloat,      Offset(radFactors,    Explosion::ExplosionData), 3);
   registerData("shiftPosition", TypeBool,       Offset(shiftPosition, Explosion::ExplosionData));
   registerData("timeScale",     TypeFloat,      Offset(timeScale,     Explosion::ExplosionData));
   registerClassEnd();

   registerClassBegin("SoundProfileData", NULL, DataBlockManager::SoundProfileDataType);
   registerData("flags",               TypeSoundFlags,   Offset(profile.flags, SoundProfileData));
   registerData("baseVolume",          TypeFloat,        Offset(profile.baseVolume, SoundProfileData));
   registerData("minDistance",         TypeFloat,        Offset(profile.minDistance, SoundProfileData));
   registerData("maxDistance",         TypeFloat,        Offset(profile.maxDistance, SoundProfileData));
   registerData("coneInsideAngle",     TypeFloat,        Offset(profile.coneInsideAngle, SoundProfileData));
   registerData("coneOutsideAngle",    TypeFloat,        Offset(profile.coneOutsideAngle, SoundProfileData));
   registerData("coneVector",          TypeNormalVector, Offset(profile.coneVector, SoundProfileData));
   registerData("coneOutsideVolume",   TypeFloat,        Offset(profile.coneOutsideVolume, SoundProfileData));
   registerClassEnd();

   registerClassBegin("SoundData", NULL, DataBlockManager::SoundDataType);
   registerData("wavFileName", TypeString, Offset(wavFileName, SoundData));
   registerData("profile", TypeSoundProfileBlock, Offset(profileIndex, SoundData));
   registerData("priority", TypeFloat, Offset(priority, SoundData));
   registerClassEnd();

   registerClassBegin("DebrisData", NULL, DataBlockManager::DebrisDataType);
   registerData("type",                  TypeInt,            Offset(type,                  Debris::DebrisData));
   registerData("pShapeName",            TypeString,         Offset(pShapeName,            Debris::DebrisData));
   registerData("imageType",             TypeInt,            Offset(imageType,             Debris::DebrisData));
   registerData("imageSortValue",        TypeInt,            Offset(imageSortValue,        Debris::DebrisData));
   registerData("mass",                  TypeFloat,          Offset(mass,                  Debris::DebrisData));
   registerData("elasticity",            TypeFloat,          Offset(elasticity,            Debris::DebrisData));
   registerData("friction",              TypeFloat,          Offset(friction,              Debris::DebrisData));
   registerData("center",                TypeFloat,          Offset(center,                Debris::DebrisData), 3);
   registerData("collisionMask",         TypeInt,            Offset(collisionMask,         Debris::DebrisData));
   registerData("knockMask",             TypeInt,            Offset(knockMask,             Debris::DebrisData));
   registerData("animationSequence",     TypeInt,            Offset(animationSequence,     Debris::DebrisData));
   registerData("animRestartTime",       TypeFloat,          Offset(animRestartTime,       Debris::DebrisData));
   registerData("minTimeout",            TypeFloat,          Offset(minTimeout,            Debris::DebrisData));
   registerData("maxTimeout",            TypeFloat,          Offset(maxTimeout,            Debris::DebrisData));
   registerData("explodeOnBounce",       TypeFloat,          Offset(explodeOnBounce,       Debris::DebrisData));
   registerData("damage",                TypeFloat,          Offset(damage,                Debris::DebrisData));
   registerData("damageThreshold",       TypeFloat,          Offset(damageThreshold,       Debris::DebrisData));
   registerData("spawnedDebrisMask",     TypeInt,            Offset(spawnedDebrisMask,     Debris::DebrisData));
   registerData("spawnedDebrisStrength", TypeFloat,          Offset(spawnedDebrisStrength, Debris::DebrisData));
   registerData("spawnedDebrisRadius",   TypeFloat,          Offset(spawnedDebrisRadius,   Debris::DebrisData));
   registerData("spawnedExplosionID",    TypeExplosionBlock, Offset(spawnedExplosionID,    Debris::DebrisData));
   registerData("p",                     TypeFloat,          Offset(p,                     Debris::DebrisData));
   registerData("explodeOnRest",         TypeInt,            Offset(explodeOnRest,         Debris::DebrisData));
   registerData("collisionDetail",       TypeInt,            Offset(collisionDetail,       Debris::DebrisData));
   registerClassEnd();

   registerClassBegin("IRCChannel", NULL, DataBlockManager::IRCChannelType);
   registerData("name",  TypeString, Offset(name,  FearGui::FGIRCJoinControl::Channel));
   registerData("topic", TypeString, Offset(topic, FearGui::FGIRCJoinControl::Channel));
   registerClassEnd();
}

//----------------------------------------------------------------------------

void FearDynamicDataPlugin::registerClassBegin(const char *className, const char *superClassName, int dbmType)
{
   AssertFatal(tempFields.size() == 0 && !curClass, "Must call registerClassEnd before beginning a new one.");
   curClass = new DataBlockClass;
   curClass->className = stringTable.insert(className);
   curClass->dbmClass = dbmType;
   if(superClassName)
   {
      superClassName = stringTable.insert(superClassName);
      DataBlockClass *walk = classes;
      while(walk)
      {
         if(walk->className == superClassName)
            break;
         walk = walk->nextClass;
      }
      AssertFatal(walk != NULL, "Invalid superclass!");
      int i;
      for(i = 0; i < walk->fieldTableSize; i++)
      {
         if(walk->fieldTable[i].fieldName)
            tempFields.push_back(walk->fieldTable[i]);
      }
   }
}

void FearDynamicDataPlugin::registerClassEnd()
{
   AssertFatal(curClass != NULL, "registerClassBegin must be called first!");
   
   curClass->nextClass = classes;
   classes = curClass;
   
   int size = tempFields.size() * 2 + 1;
   curClass->fieldTable = new Field[size];
   int i;
   curClass->fieldTableSize = size;
   for(i = 0; i < size; i++)
      curClass->fieldTable[i].fieldName = NULL;
   for(i = 0; i < tempFields.size(); i++) {
      int startIndex = (((DWORD) tempFields[i].fieldName) >> 2) % size;
      while(curClass->fieldTable[startIndex].fieldName) {
         startIndex++;
         if(startIndex >= size)
            startIndex = 0;
      }
      curClass->fieldTable[startIndex] = tempFields[i];
   }
   curClass = NULL;
   tempFields.setSize(0);
}

//----------------------------------------------------------------------------

void FearDynamicDataPlugin::registerData(const char *fieldName, int type, int offset, int elementCount)
{
   Field temp;
   temp.fieldName = stringTable.insert(fieldName);
   temp.type = type;
   temp.elementCount = elementCount;
   temp.offset = offset;
   tempFields.push_back(temp);
}

//----------------------------------------------------------------------------

static char returnBuffer[4096];

//----------------------------------------------------------------------------

static const char *getDataTypeString(void *dptr)
{
   return *((const char **)(dptr));
}

static void setDataTypeString(void *dptr, int argc, const char **argv)
{
   if(argc == 1)
      *((const char **) dptr) = stringTable.insert(argv[0]);
   else
      Console->printf("Cannot set multiple args to a single string.");
}

static void setDataTypeCaseString(void *dptr, int argc, const char **argv)
{
   if(argc == 1)
      *((const char **) dptr) = stringTable.insert(argv[0], true);
   else
      Console->printf("Cannot set multiple args to a single string.");
}

//----------------------------------------------------------------------------

static const char *getDataTypeInt(void *dptr)
{
   sprintf(returnBuffer, "%d", *((int *) dptr) );
   return returnBuffer;
}

static void setDataTypeInt(void *dptr, int argc, const char **argv)
{
   if(argc == 1)
      *((int *) dptr) = atoi(argv[0]);
   else
      Console->printf("Cannot set multiple args to a single int.");
}

static const char *getDataTypeFloat(void *dptr)
{
   sprintf(returnBuffer, "%g", *((float *) dptr) );
   return returnBuffer;
}

static void setDataTypeFloat(void *dptr, int argc, const char **argv)
{
   if(argc == 1)
      *((float *) dptr) = atof(argv[0]);
   else
      Console->printf("Cannot set multiple args to a single float.");
}

static const char *getDataTypeBool(void *dptr)
{
   return *((bool *) dptr) ? "True" : "False";
}

static void setDataTypeBool(void *dptr, int argc, const char **argv)
{
   if(argc == 1)
      *((bool *) dptr) = CMDConsole::getBool(argv[0]);
   else
      Console->printf("Cannot set multiple args to a single float.");
}

//----------------------------------------------------------------------------

static const char *getDataTypePoint3F(void *dptr)
{
   Point3F *pt = (Point3F *) dptr;
   sprintf(returnBuffer, "%g %g %g", pt->x, pt->y, pt->z);
   return returnBuffer;
}

static void setDataTypePoint3F(void *dptr, int argc, const char **argv)
{
   if(argc == 1)
      sscanf(argv[0], "%f %f %f", &((Point3F *) dptr)->x, &((Point3F *) dptr)->y, &((Point3F *) dptr)->z);
   else if(argc == 3)
      *((Point3F *) dptr) = Point3F(atof(argv[0]), atof(argv[1]), atof(argv[2]));
   else
      Console->printf("Point3F must be set as { x, y, z } or \"x y z\"");
}

//----------------------------------------------------------------------------

static const char *getDataTypeTMat3F(void *dptr)
{
   TMat3F *m = (TMat3F *) dptr;
   sprintf(returnBuffer, "%g %g %g %g %g %g %g %g %g %g %g %g",
      m->m[0][0], m->m[0][1], m->m[0][2], m->m[1][0], m->m[1][1], m->m[1][2],
      m->m[2][0], m->m[2][1], m->m[2][2], m->p.x, m->p.y, m->p.z);
   return returnBuffer;
}

static void setDataTypeTMat3F(void *dptr, int argc, const char **argv)
{
   if(argc == 1)
   {
      TMat3F *m = (TMat3F *) dptr;
      sscanf(argv[0], "%f %f %f %f %f %f %f %f %f %f %f %f", 
         &m->m[0][0], &m->m[0][1], &m->m[0][2], &m->m[1][0], &m->m[1][1], &m->m[1][2],
      &m->m[2][0], &m->m[2][1], &m->m[2][2], &m->p.x, &m->p.y, &m->p.z);
      TMat3F mt;
      mt.identity();
      m->flags = 0;
      if(memcmp(mt.m, m->m, sizeof(mt.m)))
         m->flags |= RMat3F::Matrix_HasRotation;
      if(memcmp(mt.p, m->p, sizeof(mt.p)))
         m->flags |= RMat3F::Matrix_HasTranslation;
   }
}

//----------------------------------------------------------------------------

static const char *getDataTypeBox3F(void *dptr)
{
   Box3F *b = (Box3F *) dptr;
   sprintf(returnBuffer, "%g %g %g %g %g %g",
      b->fMin.x, b->fMin.y, b->fMin.z, b->fMax.x, b->fMax.y, b->fMax.z);
   return returnBuffer;
}

static void setDataTypeBox3F(void *dptr, int argc, const char **argv)
{
   if(argc == 1)
   {
      Box3F *b = (Box3F *) dptr;
      sscanf(argv[0], "%f %f %f %f %f %f",
         &b->fMin.x, &b->fMin.y, &b->fMin.z, 
         &b->fMax.x, &b->fMax.y, &b->fMax.z);
   }
}

//----------------------------------------------------------------------------

static const char *getDataTypeNormalVector(void *dptr)
{
   Point3F *pt = (Point3F *) dptr;
   sprintf(returnBuffer, "%g %g %g", pt->x, pt->y, pt->z);
   return returnBuffer;
}

static void setDataTypeNormalVector(void *dptr, int argc, const char **argv)
{
   Point3F *pt = (Point3F *) dptr;
   if(argc == 1)
   {
      sscanf(argv[0], "%f %f %f", &pt->x, &pt->y, &pt->z);
      pt->normalize();
   }
   else if(argc == 3)
   {
      *(pt) = Point3F(atof(argv[0]), atof(argv[1]), atof(argv[2]));
      pt->normalize();
   }
   else
      Console->printf("Normal Vector must be set as { x, y, z } or \"x y z\"");
}

//----------------------------------------------------------------------------

static const char *getDataTypeColorF(void *dptr)
{
   ColorF *pt = (ColorF *) dptr;
   sprintf(returnBuffer, "%g %g %g", pt->red, pt->green, pt->blue);
   return returnBuffer;
}

static void setDataTypeColorF(void *dptr, int argc, const char **argv)
{
   if(argc == 3)
      *((ColorF *) dptr) = ColorF(atof(argv[0]), atof(argv[1]), atof(argv[2]));
   else
      Console->printf("ColorF must be set as { red, green, blue }");
}


//----------------------------------------------------------------------------

static const char *getDataTypeAnimData(void *dptr)
{
   Player::AnimData *ad = (Player::AnimData *) dptr;
   return ad->name;
}

static void setDataTypeAnimData(void *dptr, int argc, const char **argv)
{
   Player::AnimData *ad = (Player::AnimData *) dptr;
   if(argc == 8)
   {
      ad->name = stringTable.insert(argv[0]);
      if(!stricmp(argv[1], "none"))
         ad->soundTag = -1;
      else
         sg.dbm->lookupDataBlock(argv[1],DataBlockManager::SoundDataType,ad->soundTag);

      ad->direction = atoi(argv[2]);
		int flags = 0;
		flags |= CMDConsole::getBool(argv[3]);
		flags |= CMDConsole::getBool(argv[4]) << 1;
		flags |= CMDConsole::getBool(argv[5]) << 2;
		flags |= CMDConsole::getBool(argv[6]) << 3;

      ad->viewFlags = (Player::AnimData::ViewFlags) flags;

		ad->priority = atoi(argv[7]);

   }
   else
      Console->printf("Animdata set as { Name, soundTag, direction, viewFlags, priority }");
}


//----------------------------------------------------------------------------

static const char *getDataTypeItemBlock(void *dptr)
{
	if (*(int*)dptr == -1)
		return "";
	return sg.dbm->lookupBlockName(*(int*)dptr,DataBlockManager::ItemDataType);
}

static void setDataTypeItemBlock(void *dptr, int argc, const char **argv)
{
	if (argc == 1) {
		int typeId;
	   if (sg.dbm->lookupDataBlock(argv[0],DataBlockManager::ItemDataType,typeId))
			*(int*)dptr = typeId;
		else {
			*(int*)dptr = -1;
			Console->printf("Item data block \"%s\" undefined",argv[0]);
		}
   }
	else
      Console->printf("Cannot set multiple args to a Item data block name.");
}

//----------------------------------------------------------------------------

static const char *getDataTypeItemImageBlock(void *dptr)
{
	if (*(int*)dptr == -1)
		return "";
	return sg.dbm->lookupBlockName(*(int*)dptr,DataBlockManager::ItemImageDataType);
}

static void setDataTypeItemImageBlock(void *dptr, int argc, const char **argv)
{
	if (argc == 1) {
		int typeId;
	   if (sg.dbm->lookupDataBlock(argv[0],DataBlockManager::ItemImageDataType,typeId))
			*(int*)dptr = typeId;
		else {
			*(int*)dptr = -1;
			Console->printf("ItemImage data block \"%s\" is undefined",argv[0]);
		}
   }
	else
      Console->printf("Cannot set multiple args to a ItemImage data block name.");
}

//----------------------------------------------------------------------------

static const char *getDataTypeSoundProfileBlock(void *dptr)
{
	if (*(int*)dptr == -1)
		return "";
	return sg.dbm->lookupBlockName(*(int*)dptr,DataBlockManager::SoundProfileDataType);
}

static void setDataTypeSoundProfileBlock(void *dptr, int argc, const char **argv)
{
	if (argc == 1) {
		int typeId;
	   if (sg.dbm->lookupDataBlock(argv[0],DataBlockManager::SoundProfileDataType,typeId))
			*(int*)dptr = typeId;
		else {
			*(int*)dptr = -1;
			Console->printf("Sound profile data block \"%s\" is undefined",argv[0]);
		}
   }
	else
      Console->printf("Cannot set multiple args to a sound profile data block name.");
}

//----------------------------------------------------------------------------

static const char *getDataTypeSoundBlock(void *dptr)
{
	if (*(int*)dptr == -1)
		return "";
	return sg.dbm->lookupBlockName(*(int*)dptr,DataBlockManager::SoundDataType);
}

static void setDataTypeSoundBlock(void *dptr, int argc, const char **argv)
{
	if (argc == 1) {
		int typeId;
	   if (sg.dbm->lookupDataBlock(argv[0],DataBlockManager::SoundDataType,typeId))
			*(int*)dptr = typeId;
		else {
			*(int*)dptr = -1;
			Console->printf("Sound data block \"%s\" is undefined",argv[0]);
		}
   }
	else
      Console->printf("Cannot set multiple args to a sound data block name.");
}

//----------------------------------------------------------------------------

static void setDataTypeSoundFlags(void *dptr, int argc, const char **argv)
{
   int flags = 0;
   int i;
   for(i = 0; i < argc; i++)
   {
      int num = atoi(argv[i]);
      if(num)
         flags |= num;
      else if(!stricmp(argv[i], "SFX_IS_LOOPING"))
         flags |= SFX_IS_LOOPING;
      else if(!stricmp(argv[i], "SFX_IS_STREAMING"))
         flags |= SFX_IS_STREAMING;
      else if(!stricmp(argv[i], "SFX_IS_PITCHING"))
         flags |= SFX_IS_PITCHING;
      else if(!stricmp(argv[i], "SFX_IS_SOFTWARE_3D"))
         flags |= SFX_IS_SOFTWARE_3D;
      else if(!stricmp(argv[i], "SFX_IS_HARDWARE_3D"))
         flags |= SFX_IS_HARDWARE_3D;
   }
	*(int*)dptr = flags;
}

//----------------------------------------------------------------------------

int ProjectileGroups[8] = {
	DataBlockManager::BulletDataType,
	DataBlockManager::GrenadeDataType,
	DataBlockManager::RocketDataType,
	DataBlockManager::LaserDataType,
	DataBlockManager::TargetLaserDataType,
	DataBlockManager::SeekingMissileDataType,
	DataBlockManager::LightningDataType,
	DataBlockManager::RepairEffectDataType
};

static const char *getDataTypeProjectileBlock(void *dptr)
{
	ProjectileDataType* data = (ProjectileDataType*) dptr;
	return sg.dbm->lookupBlockName(data->type,data->dataType);
}

static void setDataTypeProjectileBlock(void *dptr, int argc, const char **argv)
{
	ProjectileDataType* data = (ProjectileDataType*) dptr;
	if (argc == 1) {
		int typeId;
		for (int i = 0; i < 8; i++)
		   if (sg.dbm->lookupDataBlock(argv[0],ProjectileGroups[i],typeId)) {
				data->type = ProjectileGroups[i];
				data->dataType = typeId;
				return;
			}
		data->type = -1;
		data->dataType = -1;
		Console->printf("Projectile data block \"%s\" is undefined",argv[0]);
   }
	else
      Console->printf("Cannot set multiple args to a Projectile data block name.");
}


//----------------------------------------------------------------------------


static const char *getDataTypeSequenceSound(void *dptr)
{
   ShapeBase::ShapeBaseData::SequenceSound *ad = 
   	(ShapeBase::ShapeBaseData::SequenceSound *) dptr;

   sprintf(returnBuffer, "%s %s", ad->sequence, 
   	sg.dbm->lookupBlockName(ad->sound,DataBlockManager::SoundDataType));
   return returnBuffer;
}

static void setDataTypeSequenceSound(void *dptr, int argc, const char **argv)
{
   ShapeBase::ShapeBaseData::SequenceSound *ad = 
   	(ShapeBase::ShapeBaseData::SequenceSound *) dptr;
   if(argc == 2)
   {
      ad->sequence = stringTable.insert(argv[0]);
		int typeId;
	   if (sg.dbm->lookupDataBlock(argv[1],DataBlockManager::SoundDataType,typeId))
	      ad->sound = typeId;
		else {
	      ad->sequence = 0;
	      ad->sound = -1;
			Console->printf("Sound data block \"%s\" is undefined",argv[0]);
		}
   }
   else
      Console->printf("Sequence sound is set as { sequenceName, soundTag }");
}

//----------------------------------------------------------------------------

static const char *getDataTypeExplosionBlock(void *dptr)
{
	if (*(int*)dptr == -1)
		return "";
	return sg.dbm->lookupBlockName(*(int*)dptr, DataBlockManager::ExplosionDataType);
}

static void setDataTypeExplosionBlock(void *dptr, int argc, const char **argv)
{
	if (argc == 1) {
		int typeId;
	   if (sg.dbm->lookupDataBlock(argv[0], DataBlockManager::ExplosionDataType, typeId))
			*(Int32*)dptr = typeId;
		else {
			*(int*)dptr = -1;
			Console->printf("Explosion data block \"%s\" is undefined",argv[0]);
		}
   }
	else
      Console->printf("Cannot set multiple args to a sound data block name.");
}

//----------------------------------------------------------------------------

static const char *getDataTypeDamageSkinBlock(void *dptr)
{
	if (*(int*)dptr == -1)
		return "";
	return sg.dbm->lookupBlockName(*(int*)dptr, DataBlockManager::DamageSkinDataType);
}

static void setDataTypeDamageSkinBlock(void *dptr, int argc, const char **argv)
{
	if (argc == 1) {
		int typeId;
	   if (sg.dbm->lookupDataBlock(argv[0], DataBlockManager::DamageSkinDataType, typeId))
			*(Int32*)dptr = typeId;
		else {
			*(int*)dptr = -1;
			Console->printf("Damage skin data block \"%s\" is undefined",argv[0]);
		}
   }
	else
      Console->printf("Cannot set multiple args to a damage skin data block name.");
}

//----------------------------------------------------------------------------

static const char *getDataTypeDebrisBlock(void *dptr)
{
	if (*(int*)dptr == -1)
		return "";
	return sg.dbm->lookupBlockName(*(int*)dptr, DataBlockManager::DebrisDataType);
}

static void setDataTypeDebrisBlock(void *dptr, int argc, const char **argv)
{
	if (argc == 1) {
		int typeId;
	   if (sg.dbm->lookupDataBlock(argv[0], DataBlockManager::DebrisDataType, typeId))
			*(Int32*)dptr = typeId;
		else {
			*(int*)dptr = -1;
			Console->printf("Debris data block \"%s\" is undefined",argv[0]);
		}
   }
	else
      Console->printf("Cannot set multiple args to a sound data block name.");
}

//----------------------------------------------------------------------------

static const char *getDataTypeStaticDamage(void *dptr)
{
	StaticBase::StaticBaseData::Damage* dp = 
		(StaticBase::StaticBaseData::Damage*) dptr;
   sprintf(returnBuffer, "%g %g", dp->disabled, dp->destroyed);
   return returnBuffer;
}

static void setDataTypeStaticDamage(void *dptr, int argc, const char **argv)
{
	StaticBase::StaticBaseData::Damage* dp = 
		(StaticBase::StaticBaseData::Damage*) dptr;
   if(argc == 2) {
		dp->disabled = atof(argv[0]);
		dp->destroyed = atof(argv[1]);
   }
   else
      Console->printf("Damage level must be set as { disable, destroyed }");
}

static const char*
getDataTypeLightAnimParam(void* dptr)
{
   Vector<InteriorShape::LightAnimParam>* pLParam =
      (Vector<InteriorShape::LightAnimParam>*)dptr;

   int curr    = 0;
   int advance = sprintf(returnBuffer, "%d ", pLParam->size());

   curr += advance;
   for (int i = 0; i < pLParam->size(); i++) {
      advance = sprintf(&returnBuffer[curr], "%f %d ", (*pLParam)[i].time, (*pLParam)[i].active ? 1 : 0);
      curr += advance;
   }

   Console->printf("return string: %s", returnBuffer);
   return returnBuffer;
}

static void
setDataTypeLightAnimParam(void* dptr, int /*argc*/, const char** /*argv*/)
{
   Vector<InteriorShape::LightAnimParam>* pLParam =
      (Vector<InteriorShape::LightAnimParam>*)dptr;


}

//----------------------------------------------------------------------------

#define DefaultTerrainGravity 20.0f

static const char *getDataTypeGrenadeDist(void *dptr)
{
   float vel = *((float *) dptr);
   float d = (vel*vel) / DefaultTerrainGravity;

   sprintf(returnBuffer, "%g", d );
   return returnBuffer;
}

static void setDataTypeGrenadeDist(void *dptr, int argc, const char **argv)
{
   
   if(argc == 1) {
      float d = atof(argv[0]);
      float vel = sqrt((d < 0 ? -d : d) * DefaultTerrainGravity);

      *((float *) dptr) = vel;
   } else {
      Console->printf("Cannot set multiple args to a single float.");
   }
}

//----------------------------------------------------------------------------

void FearDynamicDataPlugin::declareDataBlock(StringTableEntry className, StringTableEntry objectName)
{
   DataBlockManager *dbm = (DataBlockManager *) manager->findObject(DataBlockManagerId);

   if(dbm->lookupDataBlock(objectName))
   {
      Console->printf("warning: data block %s already declared", objectName);
      return;
   }

   DataBlockClass *temp = classes;
   while(temp) {
      if(temp->className == className)
         break;
      temp = temp->nextClass;
   }
   if(!temp) {
      Console->printf("Unknown class %s", className);
      return;
   }

   dbm->createBlock(objectName, temp->dbmClass, (void *) temp);
}

const char *FearDynamicDataPlugin::getDataField(StringTableEntry objectName, StringTableEntry slotName, const char *array)
{
   SimObject *obj = manager->findObject(objectName);
   if(obj)
   {
      if(!array)
         return obj->getFieldValue(slotName);
      else
      {
         static char buf[256];
         strcpy(buf, slotName);
         strcat(buf, array);
         return obj->getFieldValue(stringTable.insert(buf));
      }
   }
   int array1 = array ? atoi(array) : -1;
   DataBlockManager *dbm = (DataBlockManager *) manager->findObject(DataBlockManagerId);
   GameBase::GameBaseData *dat = dbm->lookupDataBlock(objectName);
   if(!dat)
      return "False";
   DataBlockClass *cls = (DataBlockClass *) dat->pluginClass;
   if(!cls)
      return "False";
   int startIndex = (((DWORD) slotName) >> 2) % cls->fieldTableSize;
   while(cls->fieldTable[startIndex].fieldName != slotName)
   {
      if(!cls->fieldTable[startIndex].fieldName) {
         Console->printf("No such field: %s in block type %s", slotName, objectName);
         return "False"; // no such field
      }
      startIndex++;
      if(startIndex >= cls->fieldTableSize)
         startIndex = 0;
   }
   Field &fld = cls->fieldTable[startIndex];
   if(array1 == -1 && fld.elementCount == 1)
      return dataFieldManager.getData(fld.type, (void *) (DWORD(dat) + fld.offset), 0);
   if(array1 >= 0 && array1 < fld.elementCount)
      return dataFieldManager.getData(fld.type, (void *) (DWORD(dat) + fld.offset), array1);// + typeSizes[fld.type] * array1));
   return "False";
}

static SimObject *curObject = NULL;
static SimGroup *curInstantGroup;

void FearDynamicDataPlugin::setDataField(StringTableEntry objectName, StringTableEntry slotName, const char *array, int argc, const char **argv)
{
   void *dt;
   Field *fld;
   
   const char *field;
   if(!array)
      field = slotName;
   else
   {
      char buf[256];
      strcpy(buf, slotName);
      strcat(buf, array);
      field = stringTable.insert(buf);
   }

   SimObject *obj = NULL;

   if(objectName)
   {
      // see if it's a game base
      SimObject *obj = manager->findObject(objectName);
      if(obj)
      {
         obj->setFieldValue(field, argv[0]);
         return;
      }
      DataBlockManager *dbm = (DataBlockManager *) manager->findObject(DataBlockManagerId);
      GameBase::GameBaseData *dat = dbm->lookupDataBlock(objectName);
      if(!dat)
         return;
      DataBlockClass *cls = (DataBlockClass *) dat->pluginClass;
      if(!cls)
         return;
      int startIndex = (((DWORD) slotName) >> 2) % cls->fieldTableSize;
      while(cls->fieldTable[startIndex].fieldName != slotName)
      {
         if(!cls->fieldTable[startIndex].fieldName)
            return; // no such field
         startIndex++;
         if(startIndex >= cls->fieldTableSize)
            startIndex = 0;
      }
      fld = &cls->fieldTable[startIndex];
      dt = (void *) dat;
   }
   else
   {
      int numFields;
      if(!curObject)
         return;

      curObject->getFieldList(numFields, fld);
      int i;
      for(i = 0; i < numFields; i++,fld++)
         if(fld->fieldName == slotName)
            break;
      if(i == numFields)
      {
         // it's a special field
         curObject->setFieldValue(field, argv[0]);
         return;
      }
      dt = (void *) curObject;
   }
   int array1 = array ? atoi(array) : -1;

   if(array1 == -1 && fld->elementCount == 1)
      dataFieldManager.setData(fld->type, (void *) (DWORD(dt) + fld->offset), 0, argc, argv);
   else if(array1 >= 0 && array1 < fld->elementCount && fld->elementCount > 1)
      dataFieldManager.setData(fld->type, (void *) (DWORD(dt) + fld->offset), array1, argc, argv);
   else if(array1 == -1 && fld->elementCount == argc)
   {
      int i;
      for(i = 0; i < argc;i++)
         dataFieldManager.setData(fld->type, (void *) (DWORD(dt) + fld->offset), i, 1, argv + i);
   }
   else
      Console->printf("setDataField: array set access out of range.");
}

static char baseName[256];

void FearDynamicDataPlugin::beginObject(StringTableEntry className, const char *objectName)
{
	Persistent::Base* obj = reinterpret_cast<Persistent::Base*>
		(Persistent::create(className));
	if (obj) {
		SimObject* simObj = dynamic_cast<SimObject*>(obj);
		if (simObj) {
         if(curObject)
         {
            SimGroup * curGroup = dynamic_cast<SimGroup *>(curObject);
            curGroup->addObject(simObj);
            if(objectName)
               curGroup->assignName(simObj, objectName);
         }
         else
            strcpy( baseName, objectName ? objectName : "" );
         curObject = simObj;
      }
      else
      {
         AssertFatal(0, avar("%s/%s is not a SimObject", className, objectName ? objectName : "NULL"));
         delete obj;
      }
   }
   else
   {
      AssertFatal(0, avar("%s/%s is not a Persist Object", className, objectName ? objectName : "NULL"));
   }
}

void FearDynamicDataPlugin::endObject()
{
   SimObject *nextCurObject = curObject->getGroup();
   if(!nextCurObject)
   {
      if(curInstantGroup)
         curInstantGroup->addObject(curObject, baseName);
      else
         manager->addObject(curObject, baseName);
   }
   curObject = nextCurObject;
}

static bool isIdent(const char *string)
{
   char c;
   while((c = *string++) != 0)
   {
      if(!(isalnum(c) || c == ':' || c == '+'))
         return false;
   }
   return true;
}

void FearDynamicDataPlugin::exportObject(SimObject *obj, StreamIO &sio, bool onlySelected, int tabin )
{
   // check if only selected objects are to be exported
   if( onlySelected && !obj->isSelected() )
      return;
      
   // write out an object...
   SimSet *grp = dynamic_cast<SimSet *>(obj);

   char lineBuf[1024];
   char fdata[2048];

   int i;
   for(i = 0; i < tabin; i++)
      lineBuf[i] = '\t';
   lineBuf[i] = 0;
   strcat(lineBuf, "instant ");
   strcat(lineBuf, obj->getClassName());
   
   if(obj->getName())
      sprintf(lineBuf + tabin, "instant %s \"%s\"", obj->getClassName(), obj->getName());
   else
      sprintf(lineBuf + tabin, "instant %s", obj->getClassName());
   Field *fields;
   int numFields;
   obj->getFieldList(numFields, fields);
   SimObject::CMDFieldEntry *walk = obj->getDynamicFieldList();

   if(walk || numFields || (grp && grp->size()))
   {
      strcat(lineBuf, " {\r\n");
      sio.write(strlen(lineBuf), lineBuf);
      tabin++;

      int j;
      for(j = 0; j < numFields; j++)
      {
         int k;
         for(k = 0; k < fields[j].elementCount; k++)
         {
            for(i = 0; i < tabin; i++)
               lineBuf[i] = '\t';
            const char *dstr = dataFieldManager.getData(fields[j].type, (void *) (DWORD(obj) + fields[j].offset), k);
            if(!dstr)
               dstr = "";
            expandEscape(fdata, dstr);

            if(fields[j].elementCount == 1)
               sprintf(lineBuf + tabin, "%s = \"%s\";\r\n", fields[j].fieldName, 
                  fdata);
            else
               sprintf(lineBuf + tabin, "%s[%d] = \"%s\";\r\n", fields[j].fieldName, k, 
                  fdata);

            sio.write(strlen(lineBuf), lineBuf);
         }
      }

      // write out the game base's dictionary:
      for(;walk; walk = walk->nextEntry)
      {
         // see if we already wrote out this field:
         for(j = 0; j < numFields; j++)
            if(fields[j].fieldName == walk->slotName)
               break;
         if(j != numFields)
            continue;

         for(i = 0; i < tabin; i++)
            lineBuf[i] = '\t';
         expandEscape(fdata, walk->value);

         sprintf(lineBuf + tabin, "%s = \"%s\";\r\n", walk->slotName, fdata);
         sio.write(strlen(lineBuf), lineBuf);
      }
      
      if(grp && grp->size())
      {
         for(j = 0; j < grp->size(); j++)
            exportObject((*grp)[j], sio, onlySelected, tabin);
      }
      tabin--;
      for(i = 0; i < tabin; i++)
         lineBuf[i] = '\t';
      lineBuf[i] = '}';
      lineBuf[i+1] = 0;
   }
   strcat(lineBuf, ";\r\n");
   sio.write(strlen(lineBuf), lineBuf);
}

const char *FearDynamicDataPlugin::consoleCallback(CMDConsole*,int id,int argc,const char *argv[])
{
   switch(id)
   {
      case 0: {
         // argv paramaters
         // ---------------
         // 0 - 'exportObjectToScript'
         // 1 - object name or the objects id
         // 2 - complete filename to export (includes base path)
         // 3 - (opt) flag if wanting to clear current script out 
         // 4 - (opt) flag if wanting to append
         // 5 - (opt) flag for exporting only selected objects
         if(argc < 3)
            return "False";

         SimObject *obj = manager->findObject(argv[1]);
         
         // if these change, then update them in missionlighting.cpp as well
         const char *header = "//--- export object begin ---//\r\n";
         const char *footer = "//--- export object end ---//\r\n";

         if(obj)
         {
            bool append = false;
            bool onlySelected = false;
            bool tossScript = false;
            
            if(argc >= 4)
               tossScript = (stricmp(argv[3], "true") == 0);
            if( argc >= 5 )
               append = ( stricmp( argv[4], "true" ) == 0 );
            if( argc >= 6 )
               onlySelected = ( stricmp( argv[5], "true" ) == 0 );

            FileWStream * st = NULL;
            if(tossScript)
            {
               st = new FileWStream(argv[2]);
               st->write(strlen(header), header);
               exportObject(obj, *st, onlySelected);
               st->write(strlen(footer), footer);
            }
            else
            {
               BYTE *headerStart = NULL;
               BYTE *footerEnd = NULL;

               // generate the name for load (hack off the base dir)
               char * loadName = strchr(const_cast<char*>(argv[2]), '\\');
               loadName ? loadName++ : loadName = const_cast<char*>(argv[2]);
               Resource<ConsoleScript> cscript = SimResource::get(manager)->load(loadName);
         
               st = new FileWStream(argv[2]);
               
               if(bool(cscript) && !append)
               {
                  headerStart = cscript->data;
                  while(*headerStart && strncmp((const char *)headerStart, header, strlen(header)))
                     headerStart++;

                  footerEnd = headerStart;
                  while(*footerEnd && strncmp((const char *)footerEnd, footer, strlen(footer)))
                     footerEnd++;
               }
               if( bool(cscript) && append )
                  st->write( cscript->size, cscript->data );
               if(headerStart && *headerStart)
                  st->write(DWORD(headerStart) - DWORD(cscript->data), (void *) cscript->data);

               st->write(strlen(header), header);
               exportObject(obj, *st, onlySelected);
               st->write(strlen(footer), footer);
            
               if(footerEnd && *footerEnd)
               {
                  footerEnd += strlen(footer);
                  st->write(cscript->size - (DWORD(footerEnd) - DWORD(cscript->data)), footerEnd);
               }
               
               if(bool(cscript))
                  cscript.purge();
            }
            
            // append any extra text to the script
            char lineBuf[1024];
            for(int i = 0; i < objectExportText.size(); i++)
            {
               sprintf(lineBuf, "%s\r\n", objectExportText[i]);
               st->write(strlen(lineBuf), lineBuf);   
            }
            
            delete st;
            
            return "True";
         }
         return "False";
      }
      case 1: {
         if(argc != 2)
            return "False";

         SimObject *obj;
         obj = manager->findObject(argv[1]);
         if(!obj)
            return "False";
         SimGroup *grp = dynamic_cast<SimGroup*>(obj);
         if(!grp)
            return "False";
         curInstantGroup = grp;
         return "True";
      }
      
      // clear out all the text lines
      case 2:
      {
         // remove the export lines
         for(int i = 0; i < objectExportText.size(); i++)
            delete [] objectExportText[i];
         objectExportText.clear();
         return "True";
      }
      
      // add a text line to the vector for the end of an exporting obj
      case 3:
      {
         if(argc != 2)
            return("False");
         objectExportText.push_back(strnew(argv[1]));
         return("True");
      }
   }
   return "False";
}
