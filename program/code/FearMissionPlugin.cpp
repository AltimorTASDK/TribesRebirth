#include "sim.h"
#include "simEvDcl.h"
#include "simConsolePlugin.h"
#include "simGame.h"
#include "fearmissionplugin.h"
#include "FearMissionEditor.h"
#include "simterrain.h"
#include <process.h>

bool isWriteable(const char * file);
bool boolify(const char * str);

void FearMissionPlugin::init()
{
   console->printf( "FearMissionPlugin" );
   
   // add the commands
   console->addCommand( Create,              "ME::Create",                 this );
   console->addCommand( RegisterType,        "ME::RegisterType",           this );
   console->addCommand( SetGrabMask,         "ME::SetGrabMask",            this );
   console->addCommand( SetPlaceMask,        "ME::SetPlaceMask",           this );
   console->addCommand( SetDefaultPlaceMask, "ME::SetDefaultPlaceMask",    this );
   console->addCommand( GetConsoleOptions,   "ME::GetConsoleOptions",      this );
   console->addCommand( ObjectToCamera,      "ME::ObjectToCamera",         this );   
   console->addCommand( CameraToObject,      "ME::CameraToObject",         this );
   console->addCommand( ObjectToSC,          "ME::ObjectToSC",             this );
   console->addCommand( CreateObject,        "MissionCreateObject",        this );
   
   console->addCommand( onSelected,          "ME::onSelected",             this );
   console->addCommand( onUnselected,        "ME::onUnselected",           this );
   console->addCommand( onSelectionCleared,  "ME::onSelectionCleared",     this );
   console->addCommand( onUseTerrainGrid,    "ME::onUseTerrainGrid",       this );

   console->addCommand( DeleteSelection,     "ME::DeleteSelection",        this );   
   console->addCommand( CopySelection,       "ME::CopySelection",          this );
   console->addCommand( CutSelection,        "ME::CutSelection",           this );
   console->addCommand( PasteSelection,      "ME::PasteSelection",         this );
   console->addCommand( PasteFile,           "ME::PasteFile",              this );
   console->addCommand( DuplicateSelection,  "ME::DuplicateSelection",     this );
   console->addCommand( CreateGroup,         "ME::CreateGroup",            this );
   console->addCommand( DropSelection,       "ME::DropSelection",          this );
   
   console->addCommand( PlaceBookmark,       "ME::PlaceBookmark",          this );
   console->addCommand( GotoBookmark,        "ME::GotoBookmark",           this );

   console->addCommand( Undo,                "ME::Undo",                   this );
   console->addCommand( Redo,                "ME::Redo",                   this );
   console->addCommand( Save,                "ME::Save",                   this );
   console->addCommand( MissionLight,        "ME::MissionLight",           this );
   console->addCommand( RebuildCommandMap,   "ME::RebuildCommandMap",      this );
}

//----------------------------------------------------------------

static char ME_Success[] = "true";
static char ME_Failure[] = "false";

const char * FearMissionPlugin::consoleCallback( CMDConsole *, int id, int argc, const char * argv[] )
{
   SimManager * clientManager = SimGame::get()->getManager( SimGame::CLIENT );
   SimManager * serverManager = SimGame::get()->getManager( SimGame::SERVER );
   
   FearMissionEditor * me = NULL;
   
   // get the missioneditor
   if( SimObject * obj = clientManager->findObject( "MissionEditor" ) )
      me = dynamic_cast<FearMissionEditor *>( obj );
   
   // check for the editor object
   if(id != Create)
   {
      if(!me)
      {
         console->printf( "%s: Must call ME::Create first", argv[0] );
         return(ME_Failure);
      }
   }
   
   // look at the command
   switch( id )
   {
      case Create:
      {
         if( argc > 1 )
         {
            // check for the missioneditor
            if(me)
               me->deleteObject();
            
            // grab the canvas
            SimGui::Canvas * sc = dynamic_cast< SimGui::Canvas * >( manager->findObject( argv[1] ) );
            if(!sc)
            {
               console->printf( "%s: can't find parent canvas [%s]", argv[0], argv[1] );
               break;
            }
         
            me = new FearMissionEditor();
            clientManager->addObject(me, "MissionEditor");
            clientManager->assignId(me, SimMissionEditorId);
            
            // set the manager and canvas for the missioneditor
            me->setManager(serverManager);
            me->setCanvas(sc);

            // need to get rid of the missioneditor on deletion of server, so
            // add it to an object that is always in the server...
            SimObject * notifyObj = serverManager->findObject("ConsoleScheduler");
            if(notifyObj)
               me->deleteNotify(notifyObj);
            
            // init the missionEditor
            if(!me->init())
               console->printf( "%s: failed to initialize MissionEditor", argv[0] );
               
            return(ME_Success);
         }
         else
            console->printf( "%s: simCanvasName", argv[0] );
            
         break;
      }
      
      case RegisterType:
      {
         if( argc >= 3 )
         {
            // check for bitmap names
            if( !me->addTypeInfo( argv[1], atoi(argv[2]),
               ( argc > 3 ) ? argv[3] : NULL,
               ( argc > 4 ) ? argv[4] : NULL ) )
               console->printf( "%s: failed to add type [%s]", argv[0], argv[1] );
            else
               return(ME_Success);
         }
         else
            console->printf( "%s: ClassDescription, BitPos, <BmpSelected>, <BmpUnselected>", argv[0] );

         break;
      }
      
      case SetGrabMask:
      {
         if( argc == 2 )
         {
            me->setGrabMask( atoi( argv[1] ) );   
            return(ME_Success);
         }
         else
            console->printf( "%s: grab mask", argv[0] );
            
         break;
      }
      
      case SetPlaceMask:
      {
         if( argc >= 2 )
         {
            if( argc == 2 )
            {
               me->removeTypeInfo( argv[1] );
               return(ME_Success);
            }
            else
            {
               FearMissionEditor::TypeInfo * dest = me->getTypeInfo( argv[1] );
               if( !dest )
               {
                  console->printf( "%s: improper type [%s]", argv[0], argv[1] );
                  break;
               }
               dest->mPlaceMask = atoi( argv[2] );
               return(ME_Success);
            }
         }
         else
            console->printf( "%s: ObjectType <mask>", argv[0] );
            
         break;
      }
      
      case SetDefaultPlaceMask:
      {
         if( argc == 2 )
         {
            me->setDefPlaceMask( atoi( argv[1] ) );   
            return(ME_Success);
         }
         else
            console->printf( "%s: placement mask", argv[0] );
            
         break;
      }
      
      case GetConsoleOptions:
      {
         me->getConsoleOptions();
         return(ME_Success);
      }

      case ObjectToCamera:
      case CameraToObject:
      case ObjectToSC:
      {
         int objId = atoi( console->getVariable( "ME::InspectObject" ) );
         SimObject * obj = me->getObject( static_cast< UINT >( objId ) );

         if( !obj )
         {
            console->printf( "%s: failed to get inspect object", argv[0] );
            break;         
         }

         switch( id )
         {
            case ObjectToCamera:
            {
               if( obj->isLocked() )
               {
                  console->printf("%s: object is locked", argv[0]);
                  break;
               }
                  
               TMat3F objMat;
               me->saveInfo( obj );
               me->getTransform( obj, objMat );
               objMat.p.set( 0.f, 0.f, 0.f );
               me->setTransform( obj, objMat );
               me->onDrop( obj, FearMissionEditor::DropAtCamera );
               break;
            }
            case CameraToObject:
            {
               SimObject * cam = me->getCamera();
               if( !cam )
               {
                  console->printf( "%s: failed to get camera", argv[0] );
                  break;
               }
               
               TMat3F camMat;
               TMat3F objMat;
               
               me->getTransform( obj, objMat );
               me->getTransform( cam, camMat );
               me->saveInfo( cam );
               
               // move her
               camMat.p = objMat.p;
               camMat.flags |= TMat3F::Matrix_HasTranslation;
               if( !me->setTransform( cam, camMat ) )
               {
                  console->printf( "%s: failed to move camera", argv[0] );
                  break;
               }
               
               return(ME_Success);
            }
            case ObjectToSC:
            {
               if( obj->isLocked() )
               {
                  console->printf("%s: object is locked", argv[0]);
                  break;
               }

               TMat3F objMat;
               me->saveInfo( obj );
               me->getTransform( obj, objMat );
               objMat.p.set( 0.f, 0.f, 0.f );
               me->setTransform( obj, objMat );
               me->onDrop( obj, FearMissionEditor::DropToScreenCenter );
               return(ME_Success);
            }
         }
         
         break;
      }
 
      case CreateObject:
      {
         serverManager->lock();
            
         if ( argc >= 3 )
         {
            // find a name to use
            char strname[255];
            int count = 1;
            do 
            {
               sprintf(strname, "%s%d", argv[1], count);
               count++;
            }
            while( serverManager->findObject( strname ) );
            
            // must have mission editor for this command
            if( me )
            {
   				Persistent::Base * obj = reinterpret_cast< Persistent::Base* >
	   				( Persistent::create( argv[2] ) );
               SimObject *simObj;
               if( obj )
               {
                  simObj = dynamic_cast<SimObject *>( obj );
                  if(simObj)
                  {
                     if(!serverManager->addObject( simObj ))
                     {
                        console->printf("MissionCreateObject: failed on addObject");
                        break;
                     }
                     
                     serverManager->assignName( simObj, strname );
                     
                     if( !simObj->processArguments( argc - 3, argv + 3 ) )
                     {
                        simObj->deleteObject();
                        console->printf("MissionCreateObject: failed argument list");
                     }
                     else
                     {
                        // add to the mission editor
                        if( !me->addObject( simObj ) )
                           simObj->deleteObject();

                        // grab the centroid for 'DropToSelectedObj'
                        me->mSelection.calcCentroid();
                                                      
                        // make this the only selection
                        console->evaluatef( "MissionObjectList::ClearSelection();" );
                        console->evaluatef( "MissionObjectList::SelectObject( %d, %d );", 
                           me->getWorldId( simObj ), simObj->getId() );

                        // move the item where we want it...
                        me->onDrop( simObj, me->mDropType );

                        // re-select to get inspect...   
                        console->evaluatef( "MissionObjectList::Inspect( %d, %d );", 
                           me->getWorldId( simObj ), simObj->getId() );
                        
                        // check if should drop to ground
                        if( me->mFlags.test( FearMissionEditor::DropToGround ) )
                           me->dropObjectDown( simObj );

                        serverManager->unlock();
                        return(avar("%d", simObj->getId()));
                     }
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
         return("0");
      }
      
      case onSelected:
      {
         // ignores worldid...
         if( argc == 3 )
         {
            me->mSelection.addObject( me->getObject( atoi( argv[2] ) ) );
            return(ME_Success);
         }
         else
            console->printf( "%s: WorldId, ObjId", argv[0] );
            
         break;
      }

      case onUnselected:
      {
         // ignores worldid...
         if( argc == 3 )
         {
            me->mSelection.removeObject( me->getObject( atoi( argv[2] ) ) );
            return(ME_Success);
         }
         else
            console->printf( "%s: WorldId, ObjId", argv[0] );
            
         break;
      }
      
      case onSelectionCleared:
      {
         me->mSelection.clearSet();
         return(ME_Success);
      }
      
      case onUseTerrainGrid:
      {
         if( me->getManager() )
         {
            SimObject * obj = me->getManager()->findObject( SimTerrainId );
            if( obj )
            {
               SimTerrain * stObj = dynamic_cast< SimTerrain * >( obj );
               if( stObj )
               {
                  float granularity = 1 << stObj->getGridFile()->getScale();
                  console->setVariable( "ME::XGridSnap", avar( "%5.3f", granularity ) );
                  console->setVariable( "ME::YGridSnap", avar( "%5.3f", granularity ) );
                  return(ME_Success);
               }
            }
            console->printf( "%s: failed to get SimTerrain object", argv[0] );
         }
         else
            console->printf( "%s: failed to get manager", argv[0] );
            
         break;
      }

      case DeleteSelection:
      {
         me->deleteSelection();
         return(ME_Success);
      }
         
      case CopySelection:
      {
         me->copySelection();
         return(ME_Success);
      }
      
      case PasteFile:
      {
         if(argc != 2)
            break;
            
         me->pasteFile(argv[1]);
         return(ME_Success);
      }
         
      case CutSelection:
      {
         me->copySelection();
         me->deleteSelection();
         return(ME_Success);
      }
         
      case PasteSelection:
      {
         me->pasteSelection();
         return(ME_Success);
      }
         
      case DuplicateSelection:
      {
         me->copySelection();
         me->pasteSelection();
         return(ME_Success);
      }

      case DropSelection:
      {  
         me->dropSelection();
         return(ME_Success);
      }      
      case CreateGroup:
      {
         SimGroup * sg = new SimGroup;
         serverManager->addObject( sg );
         serverManager->assignName( sg, "SimGroup" );
         if( !me->addObject( sg ) )
            delete sg;
         else
            return(ME_Success);
            
         break;
      }

      case PlaceBookmark:
      {
         if( argc == 2 )
         {
            int num = atoi( argv[1] );
            if( num >= 0 && num <= 9 )
            {
               me->placeBookmark( num );
               return(ME_Success);
            }
         }
         console->printf( "%s: Invalid bookmark [%d] use [0-9]", argv[0], atoi( argv[1] ) );
         break;
      }
      
      case GotoBookmark:
      {
         if( argc == 2 )
         {
            int num = atoi( argv[1] );
            if( num >= 0 && num <= 9 )
            {
               me->gotoBookmark( num );
               return(ME_Success);
            }
         }

         console->printf( "%s: Invalid bookmark [%d] use [0-9]", argv[0], atoi( argv[1] ) );
         
         break;
      }

      case Undo:
      {
         me->restoreInfo();
         return(ME_Success);
      }
         
      case Redo:
         break;
         
      case Save:
      {
         char * missionName = const_cast<char*>(console->getVariable( "$missionFile" ));
         if( missionName && strlen( missionName ) )
         {
            // remove any base\\...
            char * baseName = strrchr(missionName, '\\');
            baseName ? baseName++ : baseName = missionName;

            // get the full path of the mission file            
            ResourceManager * rm = SimResource::get(serverManager);
            char fileBuf[MAX_FILE];
            rm->getFullPath(baseName, fileBuf);

            // check the files
            if(!isWriteable(console->getVariable("$Ted::currFile")) ||
               !isWriteable(fileBuf))
            {
               console->printf("Failed to save mission - terrain and mission files need write access");
               break;
            }
               
            // save ted
            console->evaluate( "Ted::save($Ted::currFile);", false );
            if(!console->getBoolVariable("$TED::success"))
               break;
               
            // save med
            SimObject * obj = serverManager->findObject( "MissionGroup" );
            if( obj )
            {
               console->executef( 3, "saveMission", avar( "%d", obj->getId() ), fileBuf );
               return(ME_Success);
            }
            else
               console->printf( "%s: failed to save mission '%s'", argv[0], missionName );
         }
         else
            console->printf( "%s: missionName not found", argv[0] );
       
         break;
      }
      
      case MissionLight:
      {
         bool quick = false;
         if(argc == 2)
            quick = boolify(argv[1]);
            
         // save the mission
         if(stricmp(console->evaluate("ME::Save();", false), "true"))
            break;
            
         // make sure non-fullscreen
         console->evaluate("setFullscreenMode(mainWindow, false);", false);

         // get the mission name, remove the base\\...
         char * missionName = const_cast<char*>(console->getVariable("$missionFile"));
         char * baseName = strrchr(missionName, '\\');
         baseName ? baseName++ : baseName = missionName;

         ResourceManager * rm = SimResource::get(serverManager);
         char fileBuf[MAX_FILE];
         rm->getFullPath(baseName, fileBuf);
         
         AssertISV(ResourceManager::sm_pManager == NULL ||
            ResourceManager::sm_pManager->isValidWriteFileName(fileBuf) == true,
            avar("Attempted write to file: %s.\n"
                  "File is not in a writable directory.", fileBuf));
         
         // spawn missionlighting on mission
         char * argv[4];
         argv[0] = "missionlighting.exe";
         argv[1] = fileBuf;
         argv[2] = quick ? "-q" : 0;
         argv[3] = 0;
         spawnvp(P_WAIT, "missionlighting.exe", argv);
         
         console->evaluate("ME::ReloadMission();, 3);", false);
         break;
  
      }
      
      case RebuildCommandMap:
      {
         if(argc != 1)
            break;
            
         me->rebuildCommandMap();
         return(ME_Success);
      }
   }
   
   return("false");
}

//-----------------------------------------------------------------
// helper functions
bool isWriteable(const char * file)
{
   // returns 0xffffffff if not a file
   DWORD fa = GetFileAttributes(file);
   if(fa == 0xffffffff || !(fa & FILE_ATTRIBUTE_READONLY))
      return(true);
   return(false);
}

bool boolify(const char * str)
{
   if(!str)
      return(false);
      
   if(!stricmp(str, "true") || !stricmp(str, "1"))
      return(true);
      
   return(false);
}

