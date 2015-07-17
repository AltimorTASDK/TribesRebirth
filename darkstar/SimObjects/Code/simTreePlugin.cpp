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

#include "simConsolePlugin.h"
#include "simTreePlugin.h"
#include "simTreeView.h"
#include "simCanvas.h"
#include "simGame.h"

//------------------------------------------------------------------------------

void SimTreePlugin::init()
{
   console->printf("simTreePlugin");
   console->addCommand(SimTreeCreate,     "simTreeCreate",     this);
   console->addCommand(SimTreeAddSet,   "simTreeAddSet",   this);
   console->addCommand(SimTreeRegBitmaps, "simTreeRegBitmaps", this);
   console->addCommand(SimTreeRegClass,   "simTreeRegClass",   this);
   console->addCommand(SimTreeRegScript,  "simTreeRegScript",  this);
}

//------------------------------------------------------------------------------

const char *SimTreePlugin::consoleCallback(CMDConsole*,int id,int argc,const char *argv[])
{
   switch( id )
   {
      case SimTreeCreate:
         if ( argc==3 || argc==4 )
         {
            SimCanvas *sc = dynamic_cast<SimCanvas*>(manager->findObject(argv[2]));
            if (!sc)
            {
               console->printf("simTreeCreate: can't find parent");
               return 0;
            }
            GWWindow *parent = sc;

            SimTreeView *stv = new SimTreeView();
            if ( stv )
            {
					if ( argc==4 )
					{
   					if (!stricmp(argv[3],"server"))
   						stv->targetManager = SimGame::get()->getManager(SimGame::SERVER);
   					else
   						stv->targetManager = SimGame::get()->getManager(SimGame::CLIENT);
					}
               else
   					stv->targetManager = manager;

				   manager->addObject(stv);
					manager->assignName(stv,argv[1]);
               stv->createWin( parent, argv[1], RectI(0,0,300,400) );

               console->evaluate( avar("simTreeRegBitmaps( %s, \"Node.bmp\", \"NodeOpen.bmp\");",argv[1]), false );
               console->evaluate( avar("simTreeRegBitmaps( %s, \"FolderClosed.bmp\", \"FolderOpen.bmp\");",argv[1]), false );
               console->evaluate( avar("simTreeRegBitmaps( %s, \"SetClosed.bmp\", \"SetOpen.bmp\");",argv[1]), false );
               console->evaluate( avar("simTreeRegBitmaps( %s, \"DropPt.bmp\", \"DropPtOpen.bmp\");",argv[1]), false );
               console->evaluate( avar("simTreeRegClass( %s, SimSet,   4, 5);",argv[1]), false );
               console->evaluate( avar("simTreeRegClass( %s, SimGroup, 2, 3);",argv[1]), false );
               console->evaluate( avar("simTreeRegClass( %s, Locked,   6, 7);",argv[1]), false );
            }
         }
         else
            console->printf( "simTreeCreate: TreeName parentWindowName [Server|Client]" );
         break;

      case SimTreeAddSet:
         if ( argc>=3 )
         {
            if ( SimObject* obj = manager->findObject(argv[1]) )
            {
               SimTreeView *stv=dynamic_cast<SimTreeView*>(obj);
               SimSet *set;
               for ( int i=2; i<argc; i++ )
               {
                  if ((obj = stv->targetManager->findObject(argv[i])) != NULL)
                  {
                     set = dynamic_cast<SimSet*>(obj);
                     stv->addSet( set );
                  }
                  else if ( !strcmpi(argv[i],"manager") )
                     stv->addSet( stv->targetManager );
                  else
                  {
                     console->printf( "simTreeAddSet: can't find SimSet" );
                     return 0;
                  }
               }
            }
         }
         else
            console->printf( "simTreeAddSet: TreeName TargetSimSet [TargetSimSet...]" );
         break;

      case SimTreeRegBitmaps:
         if ( argc>2 )
         {
            if ( SimObject* obj = manager->findObject(argv[1]) )
            {
               SimTreeView *stv=dynamic_cast<SimTreeView*>(obj);
               Resource<GFXPalette> pal = SimGame::get()->getWorld(SimGame::CLIENT)->getPalette();
               for ( int i=2; i<argc; i++ )
               {
                  Resource<GFXBitmap> bitmap;
         	      bitmap = SimResource::get(manager)->load(argv[i]);
                  if ( (bool)bitmap )
                     stv->registerBitmap( bitmap, pal );
                  else
                     console->printf( avar("simTreeRegBitmaps: bitmap not found %s",argv[i]) );
               }
            }
         }
         else
            console->printf( "simTreeRegBitmaps: TreeName bitmap1.bmp [bitmapn.bmp] ..." );
         break;

      case SimTreeRegClass:
         if ( argc == 5 )
         {
            if ( SimObject* obj = manager->findObject(argv[1]) )
            {
               SimTreeView *stv=dynamic_cast<SimTreeView*>(obj);
               if ( stv )
                  stv->registerClass( argv[2], atoi(argv[3]), atoi(argv[4]) );
            }
         }
         break;

      case SimTreeRegScript:
         if ( argc > 1 )
         {
            if ( SimObject* obj = manager->findObject(argv[1]) )
            {
               SimTreeView *stv=dynamic_cast<SimTreeView*>(obj);
               stv->registerScript( argv[2], argv[3] );
            }
         }
         else
         {
            console->printf( "simTreeRegScript: TreeName menuText scriptName" );
            console->printf( "simTreeRegScript: TreeName seperator" );
         }
         break;

   }
   return 0;
}