//----------------------------------------------------------------------------
//
// mapViewPlugin.cpp
//
//----------------------------------------------------------------------------

#include <sim.h>
#include "simMapCanvas.h"
#include "mapViewPlugin.h"

//----------------------------------------------------------------------------

void MapViewPlugin::init()
{
	console->printf("MapViewPlugin");
	console->addCommand(LoadMap, "loadMap", this);
   console->addCommand(LoadMapPalette, "loadMapPalette", this);
}

//----------------------------------------------------------------------------

void MapViewPlugin::startFrame()
{
}

void MapViewPlugin::endFrame()
{
}

//----------------------------------------------------------------------------

int MapViewPlugin::consoleCallback(CMDConsole *cp, int id,int argc,const char *argv[])
{
	switch(id) 
	{
      case LoadMapPalette:
			if (argc > 1) 
			{
				// Blocking resource load
				Resource<GFXPalette> np =
					SimResource::get(manager)->load(argv[1],true);
				if (bool(np)) 
               palette = np;
				else
					cp->printf("loadMapPalette: Could not load palette");
			}
         else
			   cp->printf("loadMapPalette paletteFilename");
			break;

      case LoadMap:
         if (!palette)
            cp->printf("loadMap: must loadMapPalette first");
         else if (argc > 1)
         {               
            SimMapWindow *window = new SimMapWindow;
            SimMapCanvas *canvas = new SimMapCanvas;
            if (window->create("Map",
         			WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
         			Point2I(CW_USEDEFAULT,0),
                  Point2I(512, 512),
         			NULL,0)) 
         	{
               canvas->setMapWindow(window);
   	   		canvas->setPalette(palette);
   		   	manager->addObject(canvas);
   				manager->assignName(canvas, argv[1]);

               //SimTSViewport *viewport = new SimTSViewport;
               //manager->addObject(viewport);
               //viewport->setObject(canvas);

               canvas->addObject(viewport);
               canvas->show(SW_SHOW);  // incase it isn't visible yet
    
            }
         }
         else
            cp->printf("loadMap name");
         break;
	}
	return 0;
}

