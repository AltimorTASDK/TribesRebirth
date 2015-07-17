//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#include <sim.h>

#include "simConsolePlugin.h"
#include "terrainPlugin.h"
#include "simTerrain.h"
#include <SimSetIterator.h>


//----------------------------------------------------------------------------

void TerrainPlugin::init()
{
	console->printf("TerrainPlugin");
	console->addCommand(NewTerrain,"newTerrain",this);
	console->addCommand(NewTerrainParam,"newTerrainParam",this);
	console->addCommand(LoadTerrain,"loadTerrain",this);
   console->addCommand(SaveTerrain,"saveTerrain",this);
	console->addCommand(SetTerrainVisibility,"setTerrainVisibility",this);
	console->addCommand(SetTerrainDetail,"setTerrainDetail",this);
	console->addCommand(SetTerrainContainer,"setTerrainContainer",this);
	console->addCommand(LightTerrain,"lightTerrain",this);
#ifdef DEBUG
   console->addCommand(ShowTerrain, "showTerrain", this);
#endif
}

//----------------------------------------------------------------------------

const char *TerrainPlugin::consoleCallback(CMDConsole*,int id,int argc,const char *argv[])
{
	switch(id)
	{
      case ShowTerrain:
      {
         bool on = false;
         if (argc > 1 && (argv[1][0] == '1' || argv[1][0] == 't' || argv[1][0] == 't'))
            on = true;
         for ( SimSetIterator obj(manager); *obj; ++obj) 
         {
            SimTerrain *terrain = dynamic_cast<SimTerrain *>(*obj);
            if (terrain) {
               terrain->setSuspendRender(!on);
               break;
            }
         }            
         break;
      }   
      case SaveTerrain:
         if(argc == 3)
         {
            SimObject *obj = manager->findObject(argv[1]);
            SimTerrain *terr = NULL;
            if(obj)
               terr = dynamic_cast<SimTerrain *>(obj);
            if(terr)
            {
               if(terr->save(argv[2]))
                  break;
            }
         }
			console->printf("saveTerrain: objectName volumeName");
         break;
		case SetTerrainVisibility:
			if (argc == 4) {
				SimObject* obj = manager->findObject(argv[1]);
				SimTerrain* terrain = obj? static_cast<SimTerrain*>(obj): 0;
				if (terrain) {
					SimTerrain::Visibility tvis;
					tvis.visibleDist = atof(argv[2]);
					tvis.hazeDist = atof(argv[3]);
					terrain->setVisibility(&tvis);
				}
				else
					console->printf("setTerrainVisibility: "
						"object not found or not a SimTerrain object");
			}
			else
				console->printf("setTerrainVisibility: objectName visibleDist hazeDist");
			break;

		case SetTerrainDetail:
			if (argc == 4) {
				SimObject* obj = manager->findObject(argv[1]);
				SimTerrain* terrain = obj? static_cast<SimTerrain*>(obj): 0;
				if (terrain) {
					SimTerrain::Detail tdet;
					tdet.perspectiveDist = atof(argv[2]);
					tdet.screenSize = atof(argv[3]);
					terrain->setDetail(&tdet);
				}
				else
					console->printf("setTerrainDetail: "
						"object not found or not a SimTerrain object");
			}
			else
				console->printf("setTerrainDetail: objectName perspectiveDist screenSquareSize");
			break;

		case SetTerrainContainer:
			if (argc == 5) {
				SimObject* obj = manager->findObject(argv[1]);
				SimTerrain* terrain = obj? static_cast<SimTerrain*>(obj): 0;
				if (terrain) {
					Point3F gravity;
					SimTerrain::ContainerProperties tcont;
				   sscanf(argv[2], "%f %f %f", &gravity.x, &gravity.y, &gravity.z);
					tcont.gravity = gravity;
					tcont.drag = atof(argv[3]);
					tcont.height = atof(argv[4]);
					terrain->setContainerProperties(tcont);
				}
				else
					console->printf("setTerrainContainer: "
						"object not found or not a SimTerrain object");
			}
			else
				console->printf("setTerrainContainer NEEDS: objectName Gravity Drag Height");
			break;

		case LightTerrain:
			if (argc == 6 || argc==2) {
				SimObject* obj = manager->findObject(argv[1]);
				SimTerrain* terrain = obj? static_cast<SimTerrain*>(obj): 0;
				if (terrain && argc == 6)
				{
					Box2F area;
					area.fMin.x = atof(argv[2]);
					area.fMin.y = atof(argv[3]);
					area.fMax.x = atof(argv[4]);
					area.fMax.y = atof(argv[5]);
					terrain->buildLightMap(area);
				}
            else if ( terrain && argc == 2 )
               terrain->buildLightMap();
				else 
					console->printf("lightTerrain: "
						"object not found or not a SimTerrain object");
			}
			else
			{
				console->printf("lightTerrain: objectName [minx miny maxx maxy]");
			}
			break;
	}
	return 0;
}

//----------------------------------------------------------------------------

void TerrainPlugin::endFrame()
{
   if(CMDConsole::updateMetrics)
   {
	   // Update console variables.
	   console->setIntVariable("GridMetrics::VisibleBlocks",
		   GridMetrics.render.blocks);
	   console->setIntVariable("GridMetrics::TransformedVertices",
		   GridMetrics.render.vertices);
	   console->setIntVariable("GridMetrics::VisibleSquares",
		   GridMetrics.render.squares);
	   console->setIntVariable("GridMetrics::RenderedPolys",
		   GridMetrics.render.polys);
	   console->setIntVariable("GridMetrics::BuiltTextures",
		   GridMetrics.render.textureBuild);
	   console->setIntVariable("GridMetrics::RegisteredTextures",
		   GridMetrics.render.textureRegister);
	   console->setIntVariable("GridMetrics::BuiltLightMaps",
		   GridMetrics.render.lightMapBuild);
   }
}
