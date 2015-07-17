// check which version should be loaded..
if( $ME::Loaded == "true" )
{
	newObject( editCamera, EditCamera, "editor.sae" );
	ME::Create( MainWindow );

	// add the types... 
	$ME::SimDefaultObject			= 1 << 0;
	$ME::SimTerrain					= 1 << 1;
	$ME::SimInteriorObject			= 1 << 2;
	$ME::SimCameraObject			   = 1 << 3;
	$ME::SimMissionObject			= 1 << 4;
	$ME::SimShapeObject				= 1 << 5;
	$ME::SimContainerObject			= 1 << 6;
	$ME::SimPlayerObjectType		= 1 << 7;
	$ME::SimProjectileObject		= 1 << 8;
	$ME::SimVehicleObjectType 		= 1 << 9;

	// fear specific
	$ME::FearItemObjectType			= 1 << 31;
	$ME::FearPlayerObjectType    	= 1 << 30;
	$ME::FearTeleportObjectType  	= 1 << 29;
	$ME::FearCorpseObjectType    	= 1 << 28;
	$ME::StationObjectType       	= 1 << 27;
	$ME::FearMineObjectType      	= 1 << 26;
	$ME::FearMoveableObjectType  	= 1 << 25;
	$ME::FearVehicleObjectType   	= 1 << 24;
	$ME::StaticObjectType        	= 1 << 23;
	$ME::MoveableBaseObjectType  	= 1 << 22;
	$ME::ItemObjectType          	= 1 << 21;
	$ME::MarkerObjectType        	= 1 << 20;
	$ME::AIObjectType            	= 1 << 19;

	// all but SimTerrain and SimContainerObject and SimDefaultObject
	ME::SetGrabMask( ~( $ME::SimTerrain | $ME::SimContainerObject | $ME::SimDefaultObject ) );

	// place masks
	ME::SetDefaultPlaceMask( $ME::SimTerrain | $ME::SimInteriorObject );

	$ME::camera = editCamera;

	// modifiers
	$ME::Mod1 = false;		// control
	$ME::Mod2 = false;		// shift
	$ME::Mod3 = false;		// alt

	// sensitivity
	$ME::MoveSensitivity 	= 0.2;
	$ME::RotateSensitivity 	= 0.02;
}
else
{
	$_Me = ME;
    $_MeCanvas = strcat( $_Me, Canvas );
    $_MeCamera = strcat( $_Me, MoveObj );
    
    exec( "move.cs" );
    move();
    
    newMissionObject( editCamera, FlyingCamera, "gui\\edit.gui", "move.sae" );
    MissionEditor( MainWindow );
    $MED::camera = editCamera;
    
    $MED::mouseRotateSens = 50.0;
    $MED::mouseMoveSens = 1.0;
    $MED::useOldMovementMethod = true;
    winMouse();
    
    missionSetAutoSaveInterval(0);
    missionSetAutoSaveName( "AUTO." @ $missionName @ ".mis" );
    
    $_Me = "";
    $_MeCanvas = "";
    $_MeCamera = "";
}
