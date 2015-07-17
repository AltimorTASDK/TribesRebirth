//----------------------------------------------------------------------------
//
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//----------------------------------------------------------------------------

#include <windows.h>
#include <sim.h>

#include "simConsolePlugin.h"
#include "LSPlugin.h"
#include "LSEditor.h"
#include "simCanvas.h"

int atoi_ex(const char * str);
bool boolify(const char * val);

//----------------------------------------------------------------------------

void LSPlugin::init()
{
   console->printf("LSPlugin");
   console->addCommand(LSCreate,          "LS::Create",            this);
   console->addCommand(LSRules,           "LS::Rules",             this);
   console->addCommand(LSTextures,        "LS::Textures",          this);
   console->addCommand(LSScript,          "LS::Script",            this);
   console->addCommand(LSApplyTextures,   "LS::ApplyTextures",     this);
   console->addCommand(LSApplyLandScape,  "LS::ApplyLandScape",    this);
   console->addCommand(LSEdit  ,          "LS::Editor",            this);
   
   // new console functions for allowing more console access
   console->addCommand(flushTextures,     "LS::flushTextures",     this);
   console->addCommand(addTerrainTexture, "LS::addTerrainTexture", this);
   console->addCommand(addTerrainType,    "LS::addTerrainType",    this);
   console->addCommand(createGridFile,    "LS::createGridFile",    this);
   console->addCommand(flushRules,        "LS::flushRules",        this);
   console->addCommand(addRule,           "LS::addRule",           this);
   console->addCommand(flushCommands,     "LS::flushCommands",     this);
   console->addCommand(addCommand,        "LS::addCommand",        this);
   console->addCommand(parseCommands,     "LS::parseCommands",     this);
}

//----------------------------------------------------------------------------

const char *LSPlugin::consoleCallback(CMDConsole*,int id,int argc,const char *argv[])
{
   LSEditor *pLSE = NULL;
   LSMapper *pLSM = NULL;

   if ( SimObject* obj = manager->findObject("LandScapeMapper") )
      pLSM = dynamic_cast<LSMapper*>(obj);

   if ( SimObject* obj = manager->findObject("LandScapeEditor") )
      pLSE = dynamic_cast<LSEditor*>(obj);

   // chek that the landscape mapper exists
   if((id != LSCreate) && !pLSM)
   {
      console->printf(avar("%s: must call LSCreate first", argv[0]));
      return(0);
   }
   
   // pLSM is valid for all but LSCreate
   switch(id)
   {
      case flushTextures:
         pLSM->flushTextures();
         break;
         
      case addTerrainType:
      {
         if(argc!=3)
         {
            console->printf(avar("%s: typeTag, description", argv[0]));
            return(0);
         }
         
         if(!pLSM->addTerrainType(toupper(argv[1][0]), argv[2]))
            console->printf(avar("%s: failed to add terrain type", argv[0]));
         
         break;
      }
         
      case addTerrainTexture:
      {
         if(argc != 8)
         {
            console->printf(avar("%s: file, sides, rotate, percent, type, elasticity, friction", argv[0]));
            return(0);
         }
         
         if(strlen(argv[2]) < 4)
         {
            console->printf(avar("%s: sides is the four character terrain type id's", argv[0]));
            return(0);
         }
            
         UInt32 sides = FOURCC(toupper(argv[2][0]), toupper(argv[2][1]), 
            toupper(argv[2][2]), toupper(argv[2][3]));
         if(!pLSM->addTerrainTexture(argv[1], sides, (UInt8)(atoi_ex(argv[3])),
            (UInt8)(atoi(argv[4])), argv[5], atof(argv[6]), atof(argv[7])))
            console->printf(avar("%s: failed to add terrain texture", argv[0]));
                        
         break;
      }
        
      case createGridFile:
      {
         if(argc != 3)
         {
            console->printf(avar("%s: gridfile, matlist", argv[0]));
            return(0);
         }
         
         if(!pLSM->createGridFile(argv[1], argv[2]))
            console->printf(avar("%s: failed to create grid file", argv[0]));
            
         break;
      }
      
      case flushRules:
      {
         pLSM->flushRules();
         break;
      }
      
      case addRule:
      {
         // check the num of arguments
         if(argc != 14)
         {
            console->printf(avar("%s: group, altMin, altMax, altMean, altSdev, altWeight, adjWeight, slopeMin, slopeMax, slopeMean, slopeSdev, slopeWeight, adjSlopes", argv[0]));
            return(0);
         }
         
         // get the group figured out: 
         //   needs to be a terrain type that has this group's tag
         RuleInfo rule;
         if(!pLSM->getTerrainTypeIndex((UInt8)toupper(argv[1][0]), rule.groupNum))
         {
            console->printf(avar("%s: cannot add rule to a nonexisting terrain type '%s'", argv[0], argv[1]));
            return(0);
         }
         
         // get the rest of the arguments
         rule.Altitude.min = atof(argv[2]);
         rule.Altitude.max = atof(argv[3]);
         rule.Altitude.mean = atof(argv[4]);
         rule.Altitude.sdev = atof(argv[5]);
         rule.AltWeight = atof(argv[6]);
         rule.adjHeights = boolify(argv[7]) ? 1 : 0;
         
         rule.Slope.min = atof(argv[8]);
         rule.Slope.max = atof(argv[9]);
         rule.Slope.mean = atof(argv[10]);
         rule.Slope.sdev = atof(argv[11]);
         rule.SlopeWeight = atof(argv[12]);
         rule.adjSlopes = boolify(argv[13]) ? 1 : 0;
         
         pLSM->addRule(rule);
         
         break;
      }
      
      case flushCommands:
      {
         pLSM->flushCommands();
         break;
      }
      
      case addCommand:
      {
         if(argc != 2)
         {
            console->printf(avar("%s: command string", argv[0]));
            return(0);
         }
         pLSM->addCommand(argv[1]);
         break;
      }
      
      case parseCommands:
      {
         pLSM->parseCommands();
         break;
      }
         
      case LSCreate:
         if ( pLSM )
         {
            console->printf("LSCreate: LandScapeMapper object already exists");
            break;
         }
			{
            SimTerrain *terrain = NULL;
            SimObject *obj = manager->findObject( SimTerrainId );
            if ( obj )
               terrain = dynamic_cast<SimTerrain*>(obj);
            if ( !terrain )
            {
               console->printf("LSCreate: Could not find object with SimTerrainId");
               break;
            }

				pLSM = new LSMapper;
			   manager->addObject(pLSM);
				manager->assignName(pLSM,"LandScapeMapper");
            pLSM->create( terrain );
 			}
			break;

      case LSRules:
         if ( argc == 2 )
         {
            if ( !pLSM->setRules( argv[1] ) )
               console->printf(avar("LSRules: Could not load rules %s",argv[1]));
         }
         else
            console->printf("LSRules terrain.rules.dat");
         break;

      case LSTextures:
         if ( argc == 3 )
         {
            if ( !pLSM->setTextures( argv[1], argv[2] ) )
               console->printf(avar("LSRules: Could not load texture files %s, %s",argv[1],argv[2]));
         }
         else
            console->printf("LSRules terrain.dml terrain.grid.dat ");
         break;

      case LSScript:
         if ( argc == 2 )
         {
            if ( !pLSM->setScript( argv[1] ) )
               console->printf("LSScript: execScript(%s) failed", argv[1]);
         }
         else 
            console->printf("LSScript: script.pls");
         break;

      case LSApplyTextures:
         if ( !pLSM->applyTextures() )
            console->printf("LSApplyTextures: error!  Are rules.dat, grid.dat and terrain.dml loaded?");
         break;

      case LSApplyLandScape:
         if ( !pLSM->applyLandScape() )
            console->printf("LSApplyLandScape: error!  is script.pls loaded?");
         break;

      case LSEdit:
         if ( pLSE )
         {
            console->printf("LSDlg: LSEditor object already exists");
            break;
         }
         if ( argc == 2 )
         {
            SimCanvas *sc = dynamic_cast<SimCanvas*>(manager->findObject(argv[1]));
            if (!sc)
            {
               console->printf("LSDlg: %s SimCanvas not found",argv[1]);
               break;
            }
            pLSE = new LSEditor();
            if ( !pLSE->create( sc, pLSM, pLSM->getLandScape(), false ) )
            {
               console->printf("LSDlg: Trouble initializing LandScape Editor");
               break;
            }
			   manager->addObject(pLSE);
				manager->assignName(pLSE,"LandScapeEditor");
         }
         else
            console->printf("LSDlg: SimCanvas");
         break;
   }
   return 0;
}

// helper function to convert normal and hex int's
int atoi_ex(const char * str)
{
   // check for hex
   if((str[0] == '0') && (tolower(str[1]) == 'x'))
   {
      int value = 0;
      sscanf("%x", str, &value);
      return(value);
   }
   return(atoi(str));
}

bool boolify(const char * val)
{
   if(!stricmp(val, "0") || !stricmp(val, "false") || !stricmp(val, "f"))
      return(false);
   return(true);
}
