// Register objects to be used with the mission editor in this file
// Format is:
//  MissionRegObject MenuCategory Description ConsoleScript...
//
function MissionRegDis( %MenuGroup, %ItemName )
{
	%DisName = strcat( %ItemName, ".dis" );
	MissionRegObject( %MenuGroup, %ItemName, MissionCreateObject, %ItemName, InteriorShape, %DisName );
}

function MissionRegItem( %MenuGroup, %Desc, %ItemName, %count )
{
	MissionRegObject( %MenuGroup, %Desc, MissionCreateObject, %ItemName, Item, %ItemName, %count, false, true);
}	

// 
//
//

MissionRegObject( Mission, Group, MissionCreateObject, "Group", SimGroup );
MissionRegObject( Mission, Set, MissionCreateObject, "Set", SimSet );
MissionRegObject( Mission, Path, MissionCreateObject, "Path", SimPath );
MissionRegObject( Mission, Marker, MissionCreateObject, "Marker", Marker, PathMarker );
MissionRegObject( Mission, "Map Marker", MissionCreateObject, "Map Marker", Marker, MapMarker );
MissionRegObject( Mission, "Drop Point", MissionCreateObject, "Drop Point", Marker, DropPointMarker );
MissionRegObject( Mission, Trigger, MissionCreateObject,GroupTrigger, Trigger, GroupTrigger );
MissionRegObject( Mission, Volume, ME::onCreateVolume);
MissionRegObject( Mission, "Point Light", MissionCreateObject, PointLight, SimLight, Point, 10, 1, 1, 1, 0, 0, 0);

//
//
//

MissionRegObject( Sky, Sky, MissionCreateObject, sky, Sky, ".5", ".5", ".5" );
MissionRegObject( Sky, Planet, MissionCreateObject, planet, Planet );
MissionRegObject( Sky, Starfield, MissionCreateObject, stars, StarField );
MissionRegObject( Sky, Snow, MissionCreateObject, snow, Snowfall, 1, 0, 0, 0 );
MissionRegObject( Sky, Rain, MissionCreateObject, rain, Snowfall, 1, 0, 0, 1 );

//
//
//

MissionRegObject( Objectives, "Flag", ME::AddFlag);
MissionRegObject( Objectives, "Flag Stand", MissionCreateObject, Stand, StaticShape, FlagStand );
MissionRegObject( Objectives, "Tower Switch", ME::AddTowerSwitch);

//
//
//

MissionRegObject( Stations, "Inventory", MissionCreateObject, InventoryStation, StaticShape, InventoryStation );
MissionRegObject( Stations, "Ammo", MissionCreateObject, AmmoStation, StaticShape, AmmoStation );
MissionRegObject( Stations, "Command", MissionCreateObject, CommandStation, StaticShape, CommandStation );
MissionRegObject( Stations, "Vehicle", MissionCreateObject, VehicleStation, StaticShape, VehicleStation );
MissionRegObject( Stations, "Vehicle Pad", MissionCreateObject, VehiclePad, StaticShape, VehiclePad);

//
//
//

MissionRegObject( Generators, "Generator", MissionCreateObject, Generator, StaticShape, Generator );
MissionRegObject( Generators, "Portable Generator", MissionCreateObject, PortGen, StaticShape, PortGenerator );
MissionRegObject( Generators, "Solar Panel", MissionCreateObject, Solar, StaticShape, SolarPanel );

//
//
//

MissionRegObject( Turrets, "Plasma Turret",  MissionCreateObject, plasmaTurret, Turret, plasmaTurret );
MissionRegObject( Turrets, "Indoor Turret",  MissionCreateObject, indoorTurret, Turret, indoorTurret );
MissionRegObject( Turrets, "Mortar Turret",  MissionCreateObject, mortarTurret, Turret, mortarTurret );
MissionRegObject( Turrets, "Rocket Turret",  MissionCreateObject, rocketTurret, Turret, rocketTurret );
MissionRegObject( Turrets, "ELF Turret",     MissionCreateObject, ELFTurret,    Turret, ELFTurret );

//
//
//
MissionRegObject( Panels, "Square", MissionCreateObject, SquarePanel, StaticShape, SquarePanel );
MissionRegObject( Panels, "Vertical", MissionCreateObject, VerticalPanel, StaticShape, VerticalPanel );
MissionRegObject( Panels, "Blue", MissionCreateObject, BluePanel, StaticShape, BluePanel );
MissionRegObject( Panels, "Yellow", MissionCreateObject, YellowPanel, StaticShape, YellowPanel );
MissionRegObject( Panels, "Set", MissionCreateObject, SetPanel, StaticShape, SetPanel );
MissionRegObject( Panels, "Vertical2", MissionCreateObject, "VertPanel2", StaticShape, VerticalPanelB );
MissionRegObject( Panels, "Display1", MissionCreateObject, "Display1", StaticShape, DisplayPanelOne );
MissionRegObject( Panels, "Display2", MissionCreateObject, "Display2", StaticShape, DisplayPanelTwo );
MissionRegObject( Panels, "Display3", MissionCreateObject, "Display3", StaticShape, DisplayPanelThree );
MissionRegObject( Panels, "DisplayH1", MissionCreateObject, "DisplayH1", StaticShape, HOnePanel );
MissionRegObject( Panels, "DisplayH2", MissionCreateObject, "DisplayH2", StaticShape, HTwoPanel );
MissionRegObject( Panels, "DisplayS1", MissionCreateObject, "DisplayS1", StaticShape, SOnePanel );
MissionRegObject( Panels, "DisplayS2", MissionCreateObject, "DisplayS2", StaticShape, STwoPanel );
MissionRegObject( Panels, "DisplayV1", MissionCreateObject, "DisplayV1", StaticShape, VOnePanel );
MissionRegObject( Panels, "DisplayV2", MissionCreateObject, "DisplayV2", StaticShape, VTwoPanel );

//
//
//

MissionRegItem( Weapons, "Chaingun", Chaingun, 1);
MissionRegItem( Weapons, "Plasma Gun", PlasmaGun, 1);
MissionRegItem( Weapons, "Grenade Launcher", GrenadeLauncher, 1);
MissionRegItem( Weapons, "Disc Launcher", DiscLauncher, 1);
MissionRegItem( Weapons, "Laser Rifle", LaserRifle, 1);
MissionRegItem( Weapons, "Energy Rifle", EnergyRifle, 1);

//
//
//
MissionRegItem( Ammo, "Bullets", BulletAmmo, 30);
MissionRegItem( Ammo, "Plasma Bolts", PlasmaAmmo, 10);
MissionRegItem( Ammo, "Grenades", GrenadeAmmo, 5);
MissionRegItem( Ammo, "Discs", DiscAmmo, 5);
MissionRegItem( Ammo, "Anti-personnel Mines", MineAmmo, 2 );
MissionRegItem( Ammo, "Hand Grenades", Grenade, 5);

//
//
//

MissionRegItem( Packs, "Energy Pack", EnergyPack, 1);
MissionRegItem( Packs, "Repair Pack", RepairPack, 1);
MissionRegItem( Packs, "Shield Pack", ShieldPack, 1);
MissionRegItem( Packs, "Sensor Jammer Pack", SensorJammerPack, 1);
MissionRegItem( Packs, "Ammo Pack", AmmoPack, 1);
MissionRegItem( Packs, "Repair Kit (inventory)", RepairKit, 1);
MissionRegItem( Packs, "Repair Patch (instant)", RepairPatch, 1);


//
//
//

MissionRegObject( Comms, "Small Antenna", MissionCreateObject, AntennaSmall, StaticShape, SmallAntenna );
MissionRegObject( Comms, "Medium Antenna", MissionCreateObject, AntennaMed, StaticShape, MediumAntenna );
MissionRegObject( Comms, "Large Antenna", MissionCreateObject, AntennaLarge, StaticShape, LargeAntenna );
MissionRegObject( Comms, "Antenna Array", MissionCreateObject, AntennaArray, StaticShape, ArrayAntenna );
MissionRegObject( Comms, "Antenna Rod", MissionCreateObject, AntennaRod, StaticShape, RodAntenna );
MissionRegObject( Comms, "Force Beacon", MissionCreateObject, Beacon, StaticShape, ForceBeacon );

//
//
//

MissionRegObject( Sensors, "Remote Pulse Sensor", MissionCreateObject, RmtPulse, Sensor, DeployablePulseSensor );
MissionRegObject( Sensors, "Pulse Sensor", MissionCreateObject, Sensor, Sensor, PulseSensor );
MissionRegObject( Sensors, "Medium Pulse Sensor", MissionCreateObject, MedPulse, Sensor, MediumPulseSensor );
MissionRegObject( Sensors, "Portable Motion Sensor", MissionCreateObject, MotionSens, Sensor, DeployableMotionSensor );

//
//
//

MissionRegObject( Base, "Air Filter", MissionCreateObject, filter, StaticShape, AirFilter );
MissionRegObject( Base, "Cargo Crate", MissionCreateObject, MagCargo, StaticShape, CargoCrate );
MissionRegObject( Base, "Liquid Cylinder", MissionCreateObject, LiquidCyl, StaticShape, CargoBarrel );

//
//
//

MissionRegObject( Doors, "AngleCut-T",  MissionCreateObject, DoorOneTop,  Moveable,  DoorOneTop );
MissionRegObject( Doors, "AngleCut-B", MissionCreateObject, DoorOneBottom, Moveable,  DoorOneBottom );
MissionRegObject( Doors, "RedSpider-L",  MissionCreateObject, DoorOneLeft,  Moveable,  DoorOneLeft );
MissionRegObject( Doors, "RedSpider-R", MissionCreateObject, DoorOneRight, Moveable,  DoorOneRight );
MissionRegObject( Doors, "GreyStaff-L",  MissionCreateObject, DoorTwoLeft,  Moveable,  DoorTwoLeft );
MissionRegObject( Doors, "GreyStaff-R", MissionCreateObject, DoorTwoRight, Moveable,  DoorTwoRight );
MissionRegObject( Doors, "GoldLogo-L",  MissionCreateObject, DoorThreeLeft,  Moveable,  DoorThreeLeft );
MissionRegObject( Doors, "GoldLogo-R", MissionCreateObject, DoorThreeRight, Moveable,  DoorThreeRight );
MissionRegObject( Doors, "GoldMartini-L",   MissionCreateObject, DoorFourLeft,Moveable,  DoorFourLeft );
MissionRegObject( Doors, "GoldMartini-R",  MissionCreateObject, DoorFourRight,  Moveable,  DoorFourRight );
MissionRegObject( Doors, "Red&Blue-L", MissionCreateObject, DoorSixLeft, Moveable,  DoorSixLeft );
MissionRegObject( Doors, "Red&Blue-R", MissionCreateObject, DoorSixRight, Moveable,  DoorSixRight );
MissionRegObject( Doors, "Single", MissionCreateObject, DoorFive, Moveable,  DoorFive );
MissionRegObject( Doors, "ForceField", MissionCreateObject, DoorForceField, Moveable,  DoorForceField );
MissionRegObject( Doors, "ForceField3x4", MissionCreateObject, DoorForceField, Moveable,  DoorForceField3x4 );
MissionRegObject( Doors, "ForceField4x17", MissionCreateObject, DoorForceField, Moveable,  DoorForceField4x17 );
MissionRegObject( Doors, "ForceField4x14", MissionCreateObject, DoorForceField, Moveable,  DoorForceField4x14 );
MissionRegObject( Doors, "ForceField4x8", MissionCreateObject, DoorForceField, Moveable,  DoorForceField4x8 );
MissionRegObject( Doors, "ForceField5x5", MissionCreateObject, DoorForceField, Moveable,  DoorForceField5x5 );
MissionRegObject( Doors, "Diagonal", MissionCreateObject, DoorDiagonal, Moveable,  DoorDiagonal );

//
//
//

MissionRegObject( Environment, "Tree 1", MissionCreateObject, TreeOne, StaticShape, TreeShape );
MissionRegObject( Environment, "Tree 2", MissionCreateObject, TreeTwo, StaticShape, TreeShapeTwo );
MissionRegObject( Environment, "Cactus1", MissionCreateObject, Cactus1, StaticShape, Cactus1 );
MissionRegObject( Environment, "Cactus2", MissionCreateObject, Cactus2, StaticShape, Cactus2 );
MissionRegObject( Environment, "Cactus3", MissionCreateObject, Cactus3, StaticShape, Cactus3 );
MissionRegObject( Environment, "Plant", MissionCreateObject, PlantTwo, StaticShape, PlantTwo );
MissionRegObject( Environment, "MUD - Grass Vent", MissionCreateObject, GrassVent, StaticShape, SteamOnGrass );
MissionRegObject( Environment, "MUD - Mud Vent", MissionCreateObject, MudVent, StaticShape, SteamOnMud );
MissionRegObject( Environment, "MUD - Grass Vent2", MissionCreateObject, GrassVent, StaticShape, SteamOnGrass2 );
MissionRegObject( Environment, "MUD - Mud Vent2", MissionCreateObject, MudVent, StaticShape, SteamOnMud2 );

//
//
//

MissionRegObject( Misc, "Forcefield", MissionCreateObject, Forcefield, StaticShape, ForceField );
MissionRegObject( Misc, "Electric Beam", MissionCreateObject, ElectrBeam, StaticShape, ElectricalBeam );
MissionRegObject( Misc, "Bigger Elec. Beam", MissionCreateObject, BigElectroBeam, StaticShape, ElectricalBeamBig );
//
//
//

MissionRegObject( Platforms, "Elev:4x4", MissionCreateObject, elevator_4x4,  Moveable, elevator4x4 );
MissionRegObject( Platforms, "Elev:4x5", MissionCreateObject, elevator_4x5,  Moveable, elevator4x5 );
MissionRegObject( Platforms, "Elev:5x5", MissionCreateObject, elevator_5x5,  Moveable, elevator5x5 );
MissionRegObject( Platforms, "Elev:6x4", MissionCreateObject, elevator_6x4, Moveable, elevator6x4 );
MissionRegObject( Platforms, "Elev:6x4_thin", MissionCreateObject, elevator_6x4thin, Moveable, elevator6x4thin );
MissionRegObject( Platforms, "Elev:6x5", MissionCreateObject, elevator_6x5, Moveable, elevator6x5 );
MissionRegObject( Platforms, "Elev:6x6_thin", MissionCreateObject, elevator_6x6thin, Moveable, elevator6x6thin );
MissionRegObject( Platforms, "Elev:6x6", MissionCreateObject, elevator_6x6,  Moveable, elevator6x6 );
MissionRegObject( Platforms, "Elev:6x6_Octa", MissionCreateObject, elevator_6x6_octagon,  Moveable, elevator6x6Octa );
MissionRegObject( Platforms, "Elev:8x4", MissionCreateObject, elevator_8x4,Moveable, elevator8x4 );
MissionRegObject( Platforms, "Elev:8x6", MissionCreateObject, elevator_8x6, Moveable, elevator8x6 );
MissionRegObject( Platforms, "Elev:8x8", MissionCreateObject, elevator_8x8, Moveable, elevator8x8 );
MissionRegObject( Platforms, "Elev:9x9", MissionCreateObject, elevator_9x9, Moveable, elevator9x9 );
MissionRegObject( Platforms, "Elev:16x16_Octa", MissionCreateObject, elevator16x16_octo, Moveable, elevator16x16Octa);

// door groupings
MissionRegObject(DoorMacros, DoubleDoor1, ME::PasteFile, "doubledoor1.cs");
MissionRegObject(DoorMacros, DoubleDoor2, ME::PasteFile, "doubledoor2.cs");
MissionRegObject(DoorMacros, DoubleDoor3, ME::PasteFile, "doubledoor3.cs");
MissionRegObject(DoorMacros, DoubleDoor4, ME::PasteFile, "doubledoor4.cs");
MissionRegObject(DoorMacros, DoubleDoor5, ME::PasteFile, "doubledoor5.cs");
MissionRegObject(DoorMacros, DoubleDoor6, ME::PasteFile, "doubledoor6.cs");
MissionRegObject(DoorMacros, ForceField, ME::PasteFile, "forcefield.cs");
MissionRegObject(DoorMacros, ForceField3x4, ME::PasteFile, "forcefield3x4.cs");
MissionRegObject(DoorMacros, Forcefield4x8, ME::PasteFile, "Forcefield4x8.cs");
MissionRegObject(DoorMacros, Forcefield4x14, ME::PasteFile, "Forcefield4x14.cs");
MissionRegObject(DoorMacros, Forcefield4x17, ME::PasteFile, "Forcefield4x17.cs");
MissionRegObject(DoorMacros, Forcefield5x5, ME::PasteFile, "Forcefield5x5.cs");
MissionRegObject(DoorMacros, SingleDoorRotate, ME::PasteFile, "SingleDoorRotate.cs");
MissionRegObject(DoorMacros, SingleDoorSlide, ME::PasteFile, "SingleDoorSlide.cs");
MissionRegObject(DoorMacros, SingleDoorUp, ME::PasteFile, "SingleDoorUp.cs");

// elevator groupings
MissionRegObject(ElevatorMacros, Elevator4x4, ME::PasteFile, "Elevator4x4.cs");
MissionRegObject(ElevatorMacros, Elevator4x5, ME::PasteFile, "Elevator4x5.cs");
MissionRegObject(ElevatorMacros, Elevator5x5, ME::PasteFile, "Elevator5x5.cs");
MissionRegObject(ElevatorMacros, Elevator6x4, ME::PasteFile, "Elevator6x4.cs");
MissionRegObject(ElevatorMacros, Elevator6x4t, ME::PasteFile, "Elevator6x4t.cs");
MissionRegObject(ElevatorMacros, Elevator6x5, ME::PasteFile, "Elevator6x5.cs");
MissionRegObject(ElevatorMacros, Elevator6x6, ME::PasteFile, "Elevator6x6.cs");
MissionRegObject(ElevatorMacros, Elevator6x6octa, ME::PasteFile, "Elevator6x6octa.cs");
MissionRegObject(ElevatorMacros, Elevator6x6t, ME::PasteFile, "Elevator6x6t.cs");
MissionRegObject(ElevatorMacros, Elevator8x4, ME::PasteFile, "Elevator8x4.cs");
MissionRegObject(ElevatorMacros, Elevator8x6, ME::PasteFile, "Elevator8x6.cs");
MissionRegObject(ElevatorMacros, Elevator8x8, ME::PasteFile, "Elevator8x8.cs");
MissionRegObject(ElevatorMacros, Elevator9x9, ME::PasteFile, "Elevator9x9.cs");
MissionRegObject(ElevatorMacros, Elevator16x16, ME::PasteFile, "Elevator16x16.cs");

exec("registerworld.cs");
exec("registerUserObjects.cs");
