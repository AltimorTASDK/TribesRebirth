//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//               (c) Copyright 1996, Dynamix Inc.   All rights reserved.
//
//------------------------------------------------------------------------------

#include <sim.h>
#include <simEvDcl.h>

#include "simConsolePlugin.h"
#include "MissionPlugin.h"
#include "MissionEditor.h"
#include "simGame.h"

//------------------------------------------------------------------------------

void MissionPlugin::init()
{
   console->printf("MissionPlugin");
   
   // console commands
   console->addCommand(newMissionEditor,        "MissionEditor",            this);
   console->addCommand(MissionRegType,          "MissionRegType",           this);
   console->addCommand(MissionRegObject,        "MissionRegObject",         this);
   console->addCommand(MissionRegTerrain,       "MissionRegTerrain",        this);
   console->addCommand(MissionCreateObject,     "MissionCreateObject",      this);
   console->addCommand(MissionAddObject,        "MissionAddObject",         this);
   console->addCommand(MissionLoadObjects,      "MissionLoadObjects",       this);
   console->addCommand(newMissionGroup,         "newMissionGroup",          this);
   console->addCommand(addMissionButton,        "addMissionButton",         this);
   console->addCommand(removeMissionButton,     "removeMissionButton",      this);
   console->addCommand(removeMissionButtons,    "removeMissionButtons",     this);
   console->addCommand(setMissionButtonChecked, "setMissionButtonChecked",  this);
   console->addCommand(setMissionButtonEnabled, "setMissionButtonEnabled",  this);
   console->addCommand(isMissionButtonChecked,  "isMissionButtonChecked",   this);
   console->addCommand(isMissionButtonEnabled,  "isMissionButtonEnabled",   this);
   console->addCommand(setAutoSaveInterval,     "missionSetAutoSaveInterval", this);
   console->addCommand(setAutoSaveName,         "missionSetAutoSaveName",   this);
   console->addCommand(saveObjectPersist,       "missionSaveObjectPersist", this);
   console->addCommand(loadObjectPersist,       "missionLoadObjectPersist", this);
   console->addCommand(undoMoveRotate,          "missionUndoMoveRotate",    this);
}

// ok... kinda kludge like but needed....
const char * MissionPlugin::getConsoleCommandString( int argc, const char * argv[], int offset )
{
   static char buff[ 1024 ];
   buff[0] = '\0';
   
   // create the console string for this baby
   int len = 0;
   for( int i = offset; i < argc; i++ )
      len += ( strlen( argv[i] ) + 8 );
      
   if( len >= sizeof( buff ) )
      return( NULL );
   
   sprintf( buff, "%s(", argv[offset] );
   
   // walk through and generate the args
   for( int j = ( offset + 1 ); j < argc; j++ )
   {
      strcat( buff, "\"" );
      strcat( buff, argv[j] );
      strcat( buff, "\"" );
      if( argc > ( j + 1 ) )
         strcat( buff, "," );
   }
   strcat( buff, ");" );
   return( buff );
}

//------------------------------------------------------------------------------

const char *MissionPlugin::consoleCallback(CMDConsole*,int id,int argc,const char *argv[])
{
   argc;
   argv;

   MissionEditor *me=NULL;
   SimManager *clientManager = SimGame::get()->getManager(SimGame::CLIENT);
   SimManager *serverManager = SimGame::get()->getManager(SimGame::SERVER);
   if ( !serverManager ) 
   {
      console->printf("Mission Editor only runs on Server");
      return "False";
   }

   if ( SimObject* obj = clientManager->findObject("MissionEditor") )
      me = dynamic_cast<MissionEditor*>(obj);

   switch( id )
   {
      case newMissionEditor:
         if ( argc > 1 )
         {
            // check to see if they specified a valid canvas 
            SimCanvas *sc = dynamic_cast<SimCanvas*>(manager->findObject(argv[1]));
            if (!sc)
            {
               console->printf("%s: can't find parent canvas", argv[0]);
               return 0;
            }
            GWWindow *parent = sc;

            // create a mission editor treeView window
            me = new MissionEditor();
            if ( me )
            {
   				me->targetManager = serverManager;

               Resource<GFXPalette> pal = SimGame::get()->getWorld(SimGame::CLIENT)->getPalette();

				   clientManager->addObject(me,"MissionEditor");
				   clientManager->assignId(me,SimMissionEditorId );

               // grab the missino name
               if( !me->create( parent, console->getVariable("missionName"), RectI(50,50,400,300) ) )
               {
                  console->printf("%s: failed to create window", argv[0]);
                  break;
               }

               console->evaluate( "simTreeRegBitmaps( MissionEditor, \"Node.bmp\", \"NodeOpen.bmp\");", false );
               console->evaluate( "simTreeRegBitmaps( MissionEditor, \"FolderClosed.bmp\", \"FolderOpen.bmp\");", false );
               console->evaluate( "simTreeRegBitmaps( MissionEditor, \"SetClosed.bmp\", \"SetOpen.bmp\");", false );
               console->evaluate( "simTreeRegBitmaps( MissionEditor, \"DropPt.bmp\", \"DropPtOpen.bmp\");", false );
               console->evaluate( "simTreeRegClass( MissionEditor, SimSet, 4, 5);", false );
               console->evaluate( "simTreeRegClass( MissionEditor, SimGroup, 2, 3);", false );
               console->evaluate( "simTreeRegClass( MissionEditor, Locked, 6, 7);", false );

               // these types are found in SimObjectTypes.h
               console->evaluate( "MissionRegType( SimDefaultObject,     0);", false );
               console->evaluate( "MissionRegType( SimTerrain,           1);", false );
               console->evaluate( "MissionRegType( SimInteriorObject,    2);", false );
               console->evaluate( "MissionRegType( SimCameraObject,      3);", false );
               console->evaluate( "MissionRegType( SimMissionObject,     4);", false );
               console->evaluate( "MissionRegType( SimShapeObject,       5);", false );
               console->evaluate( "MissionRegType( SimContainerObject,   6);", false );
               console->evaluate( "MissionRegType( SimPlayerObjectType,  7);", false );
               console->evaluate( "MissionRegType( SimProjectileObject,  8);", false );
               console->evaluate( "MissionRegType( SimVehicleObjectType, 9);", false );

               // execute the RegisterObjects.cs script
               console->evaluate( "exec( \"RegisterObjects.cs\" );", false );
            }
         }
         else
            console->printf("%s: simCanvasName", argv[0]);
         break;

      case newMissionGroup:
         {
            SimObject *obj = serverManager->findObject( SimMissionGroupId );
            if ( obj )
               console->printf("newMissionGroup MissionGroup already exists");
            else
            {
               SimSet *set = new SimSet(true);   // owns objects
               serverManager->addObject( set, "MissionGroup" );
               serverManager->assignId( set, SimMissionGroupId );
            }
         }
         break;

      case MissionRegType:
         if ( argc == 3 )
         {
            if ( me )  // must have mission editor for this command
            {
               MissionEditor::MaskTypes *mt =  new MissionEditor::MaskTypes;
               mt->description = strnew( argv[1] );
               mt->mask = (1<<(atoi(argv[2])));
               me->maskTypes.push_back( mt );
            }
            else
               console->printf("MissionRegType:  Must call MissionCreate first");
         }
         else
         {
            console->printf("MissionRegType ClassDescription scaleFactor");
         }
         break;

      case MissionRegObject:
         if ( argc > 1 )
         {
            if ( me )  // must have mission editor for this command
               me->registerObject( argv[1], argv[2], getConsoleCommandString( argc, argv, 3 ) );
            else
               console->printf("MissionRegObject:  Must call MissionCreate first");
         }
         else
         {
            console->printf("MissionRegObject menuCategory menuText consoleScript");
            console->printf("MissionRegObject menuCategory seperator");
            console->printf("MissionRegObject seperator");
         }
         break;

      case MissionRegTerrain:
         if ( argc > 1 )
         {
            if ( me )  // must have mission editor for this command
               me->registerTerrain( argv[1], argv[2], getConsoleCommandString( argc, argv, 3 ) );
            else
               console->printf("MissionRegTerrain:  Must call MissionCreate first");
         }
         else
         {
            console->printf("MissionRegTerrain menuCategory menuText consoleScript");
            console->printf("MissionRegTerrain menuCategory seperator");
            console->printf("MissionRegTerrain seperator");
         }
         break;

      case MissionLoadObjects:
         if ( argc == 2 )
         {
            Persistent::Base::Error err;
            SimObject *obj = (SimObject*)Persistent::Base::fileLoad( argv[1], &err );
            SimSet *missionSet = dynamic_cast<SimSet*>(obj);
            serverManager->addObject( missionSet );
            serverManager->assignName( missionSet, "MissionGroup" );
            missionSet->setOwnObjects(true);
         }
         else
            console->printf("MissionLoadObjects fileName.mis");
         break;

      //------------------------------------------------------------------------------
      // MissionCreateObject uses the Persistent class to instantiate a new
      // object of the class type given.
      // format is: MissionCreateObject type_name class_name arglist
      //
      // type_name is a string describing the actual object, so for example
      // MissionCreateObject bunker SimInterior bunker.dig
      // it will name the first object bunker1, the second bunker2, the third bunker3, etc
      //
      // the arglist is passed directly to the object's processArguments function

      case MissionCreateObject:
         serverManager->lock();
         if ( argc >= 3 )
         {
            char strname[255];
            int count = 1;
            do 
            {
               sprintf(strname, "%s%d", argv[1], count);
               count++;
            }
            while(serverManager->findObject(strname));
            
            if ( me )  // must have mission editor for this command
            {
   				Persistent::Base* obj = reinterpret_cast<Persistent::Base*>
	   				(Persistent::create(argv[2]));
               SimObject *simObj;
               if ( obj )
               {
                  simObj = dynamic_cast<SimObject *>(obj);
                  if(simObj)
                  {
                     serverManager->addObject(simObj);
                     serverManager->assignName(simObj, strname);
                     
                     if(!simObj->processArguments(argc - 3, argv + 3))
                     {
                        simObj->deleteObject();
                        console->printf("MissionCreateObject: failed argument list");
                     }
                     else
                        if (!me->addObject( simObj ))
                           simObj->deleteObject();
                  }
               }
               else
                  console->printf(avar("MissionCreateObject: Unable to instantiate object of class type: %s",argv[2]));
            }
            else
               console->printf("MissionCreateObject:  Must call MissionCreate first");
         }
         else
            console->printf("MissionCreateObject objtypename className arglist");
         serverManager->unlock();
         break;

      //------------------------------------------------------------------------------
      // MissionAddObject adds an existing object to the Mission Editor.
      // The Mission Editor adds the object the MissionGroup, and adds a
      // representation of the object to the currently focused SimSet in
      // the MissionEditor.

      case MissionAddObject:
         if ( argc == 2 )
         {
            if ( me )  // must have mission editor for this command
            {
               SimObject *obj = serverManager->findObject(argv[1]);
               if ( obj )
                  me->addObject( obj );
               else
                  console->printf(avar("MissionAddObject: Unable to find object: %s",argv[1]));
            }
            else
               console->printf("MissionAddObject:  Must call MissionCreate first");
         }
         else
            console->printf("MissionAddObject objName");
         break;

      //------------------------------------------------------------------------------
      // add's a button to the toolbar and associates a command to it.  With one command
      // passed, this callback will make it a normal button and call it on 
      // 
      case addMissionButton:
      {
         // check args
         if( argc != 2 && argc != 5 && argc != 6  )
         {
            console->printf( "addMissionButton buttonName bitmap helpString commandA commandB" );
            break;
         }
         
         // check for object
         if( !me )
         {
            console->printf( "addMissionButton: must call MissionCreate first" );
            break;
         }
         
         // call it - two arguments for adding gap
         bool b;
         if( argc == 2 )
            b = me->addButton( argv[1], NULL, NULL, NULL, NULL );
         else
            b = me->addButton( argv[1], argv[2], argv[3], argv[4], ( argc == 5 ) ? NULL : argv[5] );
            
         // check for failure
         if( !b )
            console->printf( "addMissionButton: failed to add button '%s'", argv[1] );
            
         break;
      }
      
      case removeMissionButton:
      {
         // check the number of args
         if( argc != 2 )
         {
            // give a usage string
            console->printf( "removeMissionButton [buttonName]" );
            break;
         }
         
         // check for object
         if( !me )
         {
            console->printf( "removeMissionButton: must call MissionCreate first" );
            break;
         }

         // remove the button
         if( !me->removeButton( argv[1] ) )
            console->printf( "removeMissionButton: failed to remove button '%s'", argv[1] );
            
         break;
      }
      
      case removeMissionButtons:
      {
         // check for missioneditor
         if( !me )
         {
            console->printf( "removeMissionButton: must call MissionCreate first" );
            break;
         }
         
         // remove the buttons
         if( !me->removeButtons() )
            console->printf( "removeMissionButtons: failed to remove button(s)" );
            
         break;
      }
      
      case setMissionButtonChecked:
      {
         // check params
         if( argc < 2 )
         {
            // give a usage string
            console->printf( "setMissionButtonChecked [name] [true|false]" );
            break;
         }
         
         // check for missioneditor
         if( !me )
         {
            console->printf( "setMissionButtonChecked: must call MissionCreate first" );
            break;
         }
         
         // call into the mission editor
         if( !me->setButtonChecked( argv[1], ( argc == 2 ) ? true : 
            ( stricmp( argv[2], "true" ) ? false : true ) ) )
            console->printf( "setMissionButtonChecked: failed to set state for button '%s'", argv[1] );
            
         break;
      }

      case setMissionButtonEnabled:
      {
         // check params
         if( argc < 2 )
         {
            // give a usage string
            console->printf( "setMissionButtonEnabled [name] [true|false]" );
            break;
         }
         
         // check for missioneditor
         if( !me )
         {
            console->printf( "setMissionButtonEnabled: must call MissionCreate first" );
            break;
         }
         
         // call into the mission editor
         if( !me->setButtonEnabled( argv[1], ( argc == 2 ) ? true : 
            ( stricmp( argv[2], "true" ) ? false : true ) ) )
            console->printf( "setMissionButtonEnabled: failed to set state for button '%s'", argv[1] );
            
         break;
      }
      
      case isMissionButtonChecked:
      {
         // check params
         if( argc < 2 )
         {
            // give a usage string
            console->printf( "isMissionButtonChecked [name]" );
            break;
         }
         
         // check for missioneditor
         if( !me )
         {
            console->printf( "isMissionButtonChecked: must call MissionCreate first" );
            break;
         }
         
         // check 
         return( me->isButtonChecked( argv[1] ) ? "True" : "False" );
      }
      
      case isMissionButtonEnabled:
      {
         // check params
         if( argc < 2 )
         {
            // give a usage string
            console->printf( "isMissionButtonEnabled [name]" );
            break;
         }
         
         // check for missioneditor
         if( !me )
         {
            console->printf( "isMissionButtonEnabled: must call MissionCreate first" );
            break;
         }
         
         // call it
         return( me->isButtonEnabled( argv[1] ) ? "True" : "False" );
      }
      
      case setAutoSaveInterval:
      {
         // check params
         if( argc < 2 )
         {
            // give a usage string
            console->printf( "missionSetAutoSaveInterval: < minutes > - 0 to turn off" );
            break;
         }
         
         // check for missioneditor
         if( !me )
         {
            console->printf( "missionSetAutoSaveInterval: must call MissionCreate first" );
            break;
         }
         
         // call it
         return( me->setAutoSaveInterval( atoi( argv[1] ) ) ? "True" : "False" );
      }
      
      case saveObjectPersist:
      {
         // check params
         if( argc < 3 )
         {
            // give a usage string
            console->printf( "missionSaveObjectPersist: objName, fileName" );
            break;
         }
         
         // check for missioneditor
         if( !me )
         {
            console->printf( "missionSaveObjectPersist: must call MissionCreate first" );
            break;
         }
         
         // have the missioneditor do the persist        
         return( me->objectSavePersist( argv[1], argv[2] ) ? "True" : "False" );
      }

      case loadObjectPersist:
      {
         // check params
         if( argc < 3 )
         {
            // give a usage string
            console->printf( "missionLoadObjectPersist: parentobj filename" );
            break;
         }
         
         // check for missioneditor
         if( !me )
         {
            console->printf( "missionLoadObjectPersist: must call MissionCreate first" );
            break;
         }
         
         // call it
         return( me->objectLoadPersist( argv[1], argv[2] ) ? "True" : "False" );
      } 

      case setAutoSaveName:
      {
         // check params
         if( argc < 2 )
         {
            // give a usage string
            console->printf( "missionSetAutoSaveName: < autosave.mis >" );
            break;
         }
         
         // check for missioneditor
         if( !me )
         {
            console->printf( "missionSetAutoSaveName: must call MissionCreate first" );
            break;
         }
         
         // call it
         return( me->setAutoSaveName( argv[1] ) ? "True" : "False" );
      }
      
      case undoMoveRotate:
      {
         // check for missioneditor
         if( !me )
         {
            console->printf( "missionUndoMoveRotate: must call MissionCreate first" );
            break;
         }
         return( me->undoMoveRotate() ? "True" : "False" );
      }
      
   }
   return 0;
}