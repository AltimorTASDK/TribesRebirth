//############################################################################
//##   
//##  $Workfile:   LS_Dlg.cpp  $
//##  $Version$
//##  $Revision:   1.00  $
//##    
//##  DESCRIPTION:
//##        A dialog class to bring up a LandScape editor in your application
//##        
//##  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
//## 
//############################################################################

#include <g_bitmap.h>
#include "LSMapper.h"
#include "console.h"
#include <process.h>

const char * LSMapper::ruleExt = "PLR";
const int LSMapper::ruleVersion = 2;

//############################################################################

LSMapper::LSMapper()
{
   ruleList.clear();
   texCombos    = NULL;
   pickOffs     = NULL;
   pickList     = NULL;
   pLS          = NULL;
   terrainId    = NULL;
   matCornerMap = NULL;
   rotationMap  = NULL;
   texCorners   = NULL;
   heightMap    = NULL;
   flags.clear();
}

//--------------------------------------------------------------------------- 

LSMapper::~LSMapper()
{
   delete pLS;
}

//--------------------------------------------------------------------------- 
// if the entireGrid flag is set.  The entire terrain is treated as a unique
// piece and each GrdBlock is given a unique LandScape and Texture treatment.
// if the entireGrid flag is clear.  It is assumed that the terrain is made
// from a single tiled GrdBlock and only that GrdBlock is textured.

bool LSMapper::create( SimTerrain *pTerrain )
{
   terrainId = pTerrain->getId();

   int gbWidth = terrain()->getGridFile()->getBlock( Point2I(0,0) )->getSize().x;
   int   blocks;
   switch( pTerrain->getGridFile()->getBlockPattern() )
   {
      case GridFile::OneBlockMapsToAll:
         blocks = 1;
         break;
      case GridFile::EachBlockUnique:
         blocks = pTerrain->getGridFile()->getSize().x;
         break;
      case GridFile::MosaicBlockPattern1:
         blocks = 4;
         break;
   }

   pLS = new LandScape( gbWidth * blocks );
   AssertFatal( pLS, "LSMapper::create:  Not enough memory" );
   if ( pLS )
   {
      ResourceManager *rm = SimResource::get(manager);
      pLS->setRM(rm);
   }

   // attempt to load default rules & textures, if these fail then the 
   // user is must specify rules through the plugin
   
   setRules("rules.dat");
   setTextures("terrain.dml", "grid.dat");

   return ( pLS != NULL );
}   

//------------------------------------------------------------------------------

bool LSMapper::setScript( const char *_landscapeFile )
{
   char  buff[4096];
   Int32 numBytes;
   FileRStream frs( _landscapeFile );
   if ( frs.read( &numBytes ) )
      if ( numBytes && frs.read( numBytes, (void*)buff ) )
      {
         pLS->parseScript( buff );
         return true;
      }
   return false;
}   

//--------------------------------------------------------------------------- 

const char * LSMapper::getExtension( const char * file )
{
   char * pos = const_cast< char *>( file );
   pos = pos + strlen( pos );
   
   while( pos > file )
   {
      if( *pos == '.' )
         return( pos + 1 );
      pos--;
   }
   
   return( file );
}

//--------------------------------------------------------------------------- 

bool LSMapper::setRules( const char *_ruleFile )
{
   AssertFatal( _ruleFile, "LSMapper::setRules:  Filename must not be NULL" );
      
   bool retVal = true;
   ruleList.clear();
   
   // check for loading of old files ( just check the extension.. .PLR for new stuff ) 
   //    - need to do this better somehow....
   if( stricmp( getExtension( _ruleFile ), ruleExt ) )
   {
      char * p = (char*)SimResource::get(manager)->lock( _ruleFile );
      if( !p )
         return( false );
         
      // get the number of rules and offset the pointer to start of rule info
      int numRules = *(int*)p;
      p += sizeof( int );
      
      // grab the rules then add to the vector
      RuleInfo * rules = ( RuleInfo * )p;
      for(;numRules;numRules--)
         ruleList.push_back( rules[numRules-1] );
   }
   else
   {
      // load in the rules with the new method
      retVal = loadRules( _ruleFile, ruleList );
   }
   
   flags.set( RULES_LOADED, retVal );
   return( retVal );
}

//--------------------------------------------------------------------------- 

bool LSMapper::setRules( const Vector< RuleInfo > & rules )
{
   // check if setting own stuff
   if( &rules == &ruleList )
      return( true );
      
   // clear out the list and copy in the new one
   ruleList.clear();
   ruleList = rules;
   flags.set( RULES_LOADED, true );
   
   return( true );
}

// --------------------------------------------------------------
// tack on the \n here for fscanf simplicity
#define LS_RULE_VERSION          "VERSION\n"
#define LS_RULE_NUMRULES         "NUMRULES\n"

// ---------------------------------------------------------------------------
// load in some rules - groups vector for missioneditor stuff
bool LSMapper::loadRules( const char * fileName, Vector< RuleInfo > & rules ) 
{
   char buff[256];
   
   FILE * fp = fopen( fileName, "r" );
   
   // check if open'd the file
   if( fp == NULL )
      return( false );
      
   int version;
   int numRules = 0;
   
   // grab version
   do {
      if( !fgets( buff, 256, fp ) )
      {
         fclose(fp);
         return( false );
      }
   } while( stricmp( buff, LS_RULE_VERSION ) );
   fscanf( fp, "%d\n", &version );
   
   // change for support of multiple versions
   AssertFatal( version == ruleVersion, "LSMapper: incorrect rule file version" );
   
   // get the number of rules
   do {
      if( !fgets( buff, 256, fp ) )
      {
         fclose(fp);
         return( false );
      }
   } while( stricmp( buff, LS_RULE_NUMRULES ) );
   fscanf( fp, "%d\n", &numRules );
   
   // clear out the array
   rules.clear();

   // go through the rules
   for(;numRules;numRules--)
   {
      // read in the rule
      RuleInfo rule;
      fscanf( fp, "%d,%f,%f,%f,%f,%f,%d,%f,%f,%f,%f,%f,%d\n",
         &rule.groupNum,
         &rule.Altitude.min,
         &rule.Altitude.max,
         &rule.Altitude.mean,
         &rule.Altitude.sdev,
         &rule.AltWeight,
         &rule.adjHeights,
         &rule.Slope.min,
         &rule.Slope.max,
         &rule.Slope.mean,
         &rule.Slope.sdev,
         &rule.SlopeWeight,
         &rule.adjSlopes );
      
      // add to the list
      rules.push_back( rule );   
   }
   
   fclose(fp);
   return( true );
}

// ---------------------------------------------------------------------------
// save off the rules
bool LSMapper::saveRules( const char * fileName, const Vector< RuleInfo > & rules )
{
   AssertISV(ResourceManager::sm_pManager == NULL ||
             ResourceManager::sm_pManager->isValidWriteFileName(fileName) == true,
             avar("Attempted write to file: %s.\n"
                  "File is not in a writable directory.", fileName));

   FILE * fp = fopen( fileName, "w" );
   
   if( fp == NULL )
      return( false );
      
   // write the version info - newline in the define
   fprintf( fp, "%s", LS_RULE_VERSION );
   fprintf( fp, "%d\n", ruleVersion );
   
   // write a little help text
   fprintf( fp, "# Rule: typeL, alt(min,max,mean,sdev)F, altweightF, adjheightsL, "
      "slope(min,max,mean,sdev)F, slopeweightF, adjslopesL\n" );
   
   // write out the rule info - newline in define
   fprintf( fp, "%s", LS_RULE_NUMRULES );
   fprintf( fp, "%d\n", rules.size() );
   
   // go through and write out each rule
   for( int i = 0; i < rules.size(); i++ )
   {
      fprintf( fp, "%d,%f,%f,%f,%f,%f,%d,%f,%f,%f,%f,%f,%d\n",
         rules[i].groupNum,
         rules[i].Altitude.min,
         rules[i].Altitude.max,
         rules[i].Altitude.mean,
         rules[i].Altitude.sdev,
         rules[i].AltWeight,
         rules[i].adjHeights,
         rules[i].Slope.min,
         rules[i].Slope.max,
         rules[i].Slope.mean,
         rules[i].Slope.sdev,
         rules[i].SlopeWeight,
         rules[i].adjSlopes );
   }
   
   fclose(fp);
   return( true );
}


//--------------------------------------------------------------------------- 

bool LSMapper::setTextures( const char *_materialFile, const char *_gridFile )
{
   matList = SimResource::get(manager)->load( _materialFile, true);
   if(!bool(matList))
      return false;

   matList->load( *SimResource::get(manager), true );
   terrain()->getGridFile()->setMaterialList( matList ); // we own matList
   terrain()->getGridFile()->setMaterialListName( _materialFile ); // we own matList

   //--------------------------------------------------------------------------- 
   char *p = (char*)SimResource::get(manager)->lock( _gridFile );
   if(!p)
      return false;

   int gridVersion = *(int*)p;   p += sizeof(int);
   AssertFatal( gridVersion>=2, avar("LSMapper::setTextures: %s is not version #2",_gridFile) );
   AssertWarn(  gridVersion>=3, avar("LSMapper::setTextures: Upgrade %s to version #3.  Get new TexTable.exe",_gridFile) );

   numBaseTypes    = *(int*)p;   p += sizeof(int);
   numBaseTextures = *(int*)p;   p += sizeof(int);
   numPicks        = *(int*)p;   p += sizeof(int);
   if ( gridVersion > 2 )
      typeText     = (char*)p;   p += numBaseTypes*MAXDESC;
   texCorners      = (Block*)p;  p += sizeof(Block)*numBaseTextures;

   int   numBTW = numBaseTypes + 1;

   texCombos = (DWORD*)p;  p += sizeof(DWORD)*numBTW*numBTW*numBTW*numBTW;
   pickOffs  = (DWORD*)p;  p += sizeof(DWORD)*(numBTW*numBTW*numBTW*numBTW+1);
   pickList  = (unsigned char *) p;

   flags.set( TEXTS_LOADED );
   return ( true );
}

//------------------------------------------------------------------------------

bool LSMapper::applyTerrainStamp( const Point2I &gbPt, Point3F origin, LS_ROT_90S rot, LSTerrainStamp *stamp)
{
   Point2I hf_origin;

   worldToTileCoords(origin, hf_origin);
   
   if (stamp->flatList.size())
   {
//      pLS->flatten(hf_origin, rot, &(stamp->flatList), origin.z);
      pLS->flatten(hf_origin, rot, &(stamp->flatList) ); // use avg to determine flaten height
      pLS->clamp(hf_origin, rot, &(stamp->flatList), stamp->clamp_max_detail);
   }

   if (stamp->texList.size())
      seedTexByTile( gbPt, hf_origin, rot, stamp );

   return true; 
}   

//--------------------------------------------------------------------------- 

char *LSMapper::getTypeDesc( bool first )
{
   static int index;
   index = first? 0:index;

   if ( index < numBaseTypes )
      return ( &typeText[ index++*MAXDESC ] );
   else
      return NULL;
}   

//---------------------------------------------------------------------------

void  LSMapper::seedTexByTile( const Point2I &gbPt, const Point2I &origin, LS_ROT_90S rot, LSTerrainStamp *stamp )
{
   GridBlock *pGB = terrain()->getGridFile()->getBlock( gbPt ); // get grid block
   int width = pGB->getSize().x;
   GridBlock::Material* pMat = pGB->getMaterialMap();
   pGB->setDirtyFlag();

   Vector<LSTerrainStamp::TextureType>::iterator iter = stamp->texList.begin();
   for ( ; iter!=stamp->texList.end(); iter++ )
   {
      Point2I pt = iter->tile;               // get relative offset
      rotateCoord( pt, rot );                // rotate if necessary
      pt += origin;                          // get absolute offset

      Int32 flags = iter->flags;
   
      if ( !( (flags >> GridBlock::Material::EmptyShift) &
      	GridBlock::Material::EmptyMask) )
      {
         // adjust rotation flags of current tile
         // following structure converts rotation to flip state flags, where "rotation"
         // is 0,1,2,3 for 0, 90, 180, 270 degree CCW rotation
         static Int32 rotTorot[] = 
         { 0,
              GridBlock::Material::Rotate,
              GridBlock::Material::FlipX | GridBlock::Material::FlipY,
              GridBlock::Material::Rotate | GridBlock::Material::FlipX | GridBlock::Material::FlipY
         };

         // EOR performs composition for FlipX and FlipY...
         flags &= GridBlock::Material::FlipX | GridBlock::Material::FlipY;
         flags ^= rotTorot[rot];
         // ...but for Rotate, R*R = FlipX | FlipY, so we have to be a little more careful
         if (flags&GridBlock::Material::Rotate && iter->flags&GridBlock::Material::Rotate)
            flags ^= GridBlock::Material::RotateMask;
         else
            flags |= iter->flags & GridBlock::Material::Rotate;
         flags |= iter->flags & ~GridBlock::Material::RotateMask;
  
         // seed material map
         pMat[pt.y*width + pt.x].index = iter->textureID; // store texture index
      }
      else
         pMat[pt.y*width + pt.x].index = 0xff;     // hole, no material

      pMat[pt.y*width + pt.x].flags = flags;     // store flip states

      // seed corner and rotation map if texture specified
      if ( !( (flags >> GridBlock::Material::EmptyShift) &
	      	GridBlock::Material::EmptyMask) )
         seedCorners( pGB->getSize().x, pt, iter->textureID, flags );
   }   
}   

//---------------------------------------------------------------------------
// given a point on a grid block, and a texture index with flip states,
// seed the corner and rotation maps

Block arrange( Block _block, Block _blockOrder )
{
   Block block;
   block.corner.ul = 0xff&(_block.block>>(_blockOrder.corner.ul<<3));
   block.corner.ur = 0xff&(_block.block>>(_blockOrder.corner.ur<<3));
   block.corner.lr = 0xff&(_block.block>>(_blockOrder.corner.lr<<3));
   block.corner.ll = 0xff&(_block.block>>(_blockOrder.corner.ll<<3));
   return ( block );
}

struct 
{
   int      flag;
   Block    block;
}xyr[8] =
{
   GridBlock::Material::Plain,                                 { 0,1,2,3 },
   GridBlock::Material::Rotate,                                { 3,0,1,2 },
   GridBlock::Material::FlipX,                                 { 1,0,3,2 },
   GridBlock::Material::FlipX  | GridBlock::Material::Rotate,  { 2,1,0,3 },
   GridBlock::Material::FlipY,                                 { 3,2,1,0 },
   GridBlock::Material::FlipY  | GridBlock::Material::Rotate,  { 0,3,2,1 },
   GridBlock::Material::FlipX  | GridBlock::Material::FlipY,   { 2,3,0,1 },
   GridBlock::Material::Rotate | GridBlock::Material::FlipX | GridBlock::Material::FlipY, { 1,2,3,0 },
};

//------------------------------------------------------------------------------

// This utility function stores the texture type in a corner.  It 
// replaces the corner if it is already set.
DWORD   reconstructTile( DWORD tile, BYTE type, int corner, int base )
{
   DWORD    c[4];
   int t1,t2,t3;
   t1 = base+1;
   t2 = t1*t1;
   t3 = t1*t1*t1;

   c[3] = tile / t3;   tile -= c[3]*t3;
   c[2] = tile / t2;   tile -= c[2]*t2;
   c[1] = tile / t1;   tile -= c[1]*t1;
   c[0] = tile;

   c[ corner ] = type;

   tile = c[0] + c[1]*t1 + c[2]*t2 + c[3]*t3;

   return ( tile );
}   

//------------------------------------------------------------------------------

void LSMapper::seedCorners( int width, Point2I &pt, BYTE index, BYTE flags )
{
   flags &= 3; // strip off non-orientation bits
   // get DWORD descrbing texture corners into a DWORD
   AssertFatal( (index<numBaseTextures)&&(flags<8), "seedCorners indecies out of range" );

   Block corners = arrange((Block)texCorners[index], xyr[flags].block);
   
   DWORD *tileA,*tileB,*tileC,*tileD;
   int wmask = width-1;

   // seed corner 0
   BYTE type = corners.corner.ll + 1;
   matCornerMap[ pt.y*width + pt.x ] = type;
   // seed rotation map
   tileA = &rotationMap[ pt.y*width + ((pt.x-1)&wmask) ];
   tileB = &rotationMap[ pt.y*width + pt.x ];
   tileC = &rotationMap[ ((pt.y-1)&wmask)*width + ((pt.x-1)&wmask) ];
   tileD = &rotationMap[ ((pt.y-1)&wmask)*width + pt.x ];
   *tileA = reconstructTile( *tileA, type, 1, numBaseTypes );
   *tileB = reconstructTile( *tileB, type, 0, numBaseTypes );
   *tileC = reconstructTile( *tileC, type, 2, numBaseTypes );
   *tileD = reconstructTile( *tileD, type, 3, numBaseTypes );

   // seed corner 1
   type = corners.corner.lr + 1;
   matCornerMap[ pt.y*width + ((pt.x+1)&wmask) ] = type;
   // seed rotation map
   tileA = &rotationMap[ pt.y*width + pt.x ];
   tileB = &rotationMap[ pt.y*width + ((pt.x+1)&wmask) ];
   tileC = &rotationMap[ ((pt.y-1)&wmask)*width + pt.x ];
   tileD = &rotationMap[ ((pt.y-1)&wmask)*width + ((pt.x+1)&wmask) ];
   *tileA = reconstructTile( *tileA, type, 1, numBaseTypes );
   *tileB = reconstructTile( *tileB, type, 0, numBaseTypes );
   *tileC = reconstructTile( *tileC, type, 2, numBaseTypes );
   *tileD = reconstructTile( *tileD, type, 3, numBaseTypes );

   // seed corner 2
   type = corners.corner.ul + 1;
   matCornerMap[ ((pt.y+1)&wmask)*width + pt.x ] = type;
   // seed rotation map
   tileA = &rotationMap[ ((pt.y+1)&wmask)*width + ((pt.x-1)&wmask) ];
   tileB = &rotationMap[ ((pt.y+1)&wmask)*width + pt.x ];
   tileC = &rotationMap[ pt.y*width + ((pt.x-1)&wmask) ];
   tileD = &rotationMap[ pt.y*width + pt.x ];
   *tileA = reconstructTile( *tileA, type, 1, numBaseTypes );
   *tileB = reconstructTile( *tileB, type, 0, numBaseTypes );
   *tileC = reconstructTile( *tileC, type, 2, numBaseTypes );
   *tileD = reconstructTile( *tileD, type, 3, numBaseTypes );

   // seed corner 3
   type = corners.corner.ur + 1;
   matCornerMap[ ((pt.y+1)&wmask)*width + ((pt.x+1)&wmask) ] = type;
   // seed rotation map
   tileA = &rotationMap[ ((pt.y+1)&wmask)*width + pt.x ];
   tileB = &rotationMap[ ((pt.y+1)&wmask)*width + ((pt.x+1)&wmask) ];
   tileC = &rotationMap[ pt.y*width + pt.x ];
   tileD = &rotationMap[ pt.y*width + ((pt.x+1)&wmask) ];
   *tileA = reconstructTile( *tileA, type, 1, numBaseTypes );
   *tileB = reconstructTile( *tileB, type, 0, numBaseTypes );
   *tileC = reconstructTile( *tileC, type, 2, numBaseTypes );
   *tileD = reconstructTile( *tileD, type, 3, numBaseTypes );
}   

//---------------------------------------------------------------------------

void LSMapper::seedCorner( int width, Point2I &pt, BYTE type )
{
   AssertFatal( (type<numBaseTypes), avar("seedCorner texture type out of range: %i",type) );
   type++;  // types in rotationMap are type+1
   DWORD *tileA,*tileB,*tileC,*tileD;
   int wmask = width-1;

   matCornerMap[ pt.y*width + pt.x ] = type;
   // seed rotation map
   tileA = &rotationMap[ pt.y*width + ((pt.x-1)&wmask) ];
   tileB = &rotationMap[ pt.y*width + pt.x ];
   tileC = &rotationMap[ ((pt.y-1)&wmask)*width + ((pt.x-1)&wmask) ];
   tileD = &rotationMap[ ((pt.y-1)&wmask)*width + pt.x ];
   *tileA = reconstructTile( *tileA, type, 1, numBaseTypes );
   *tileB = reconstructTile( *tileB, type, 0, numBaseTypes );
   *tileC = reconstructTile( *tileC, type, 2, numBaseTypes );
   *tileD = reconstructTile( *tileD, type, 3, numBaseTypes );
}   

//---------------------------------------------------------------------------

void LSMapper::setHeightField( const Point2I &gbPt )
{
   GridBlock *pGB = terrain()->getGridFile()->getBlock( gbPt ); 
   int width = pGB->getHeightMapWidth();

   pLS->setGrdHeightMap( (float*)pGB->getHeightMap(), Box2I(gbPt.x*width,gbPt.y*width,gbPt.x*width+width,gbPt.y*width+width) );
   pGB->updateHeightRange();
}   

//--------------------------------------------------------------------------- 
// getCornerType for debugging only, returns corner type for a tile
BYTE   getCornerType( DWORD tile, int corner, int base )
{
   DWORD    c[4];
   int t1,t2,t3;
   t1 = base+1;
   t2 = t1*t1;
   t3 = t1*t1*t1;

   c[3] = tile / t3;   tile -= c[3]*t3;
   c[2] = tile / t2;   tile -= c[2]*t2;
   c[1] = tile / t1;   tile -= c[1]*t1;
   c[0] = tile;

   return ( c[ corner ] );
}   

//--------------------------------------------------------------------------- 
// mtxStride - width of the MaterialMap and HeightFields to texture

void LSMapper::applyTextures( int mtxStride )
{
//   LSStampInstance::applyStamps( this, gBlockPt, manager );

   DWORD *tileA,*tileB,*tileC,*tileD;
   int width,area,wmask;
   int t1,t2,t3;
   int scale = terrain()->getGridFile()->getScale();

   width = mtxStride;
   area = width*width;
   wmask = width-1;
   t1 = numBaseTypes+1;
   t2 = t1*t1;
   t3 = t1*t1*t1;

   // pick corner types

   BYTE *pCnr=matCornerMap;
   GridBlock::Material* pMat = materialMap;

   tileB = rotationMap;
   tileA = &rotationMap[ wmask ];
   tileC = &rotationMap[ area-1 ];
   tileD = &rotationMap[ area-width ];


   int   index=0;
   while( index<area )
   {
      DWORD combinations;
      BYTE  type;
      if ( !*pCnr )  // don't pick if already set by user
      {
         combinations=0xffffffff;
         combinations &= texCombos[ *tileA ];
         combinations &= texCombos[ *tileB ];
         combinations &= texCombos[ *tileC ];
         combinations &= texCombos[ *tileD ];
         if ( combinations )
         {
            type = pickType( scale, mtxStride, combinations, index );
            *tileA += type*t1;
            *tileB += type;
            *tileC += type*t2;
            *tileD += type*t3;
         }
         else
         {
            pMat[index].index = 0;
            type = 0;
            *tileA += type*t1;
            *tileB += type;
            *tileC += type*t2;
            *tileD += type*t3;
            AssertMessage( combinations, avar("LSMapper::Missing Texture [%i]( %i,%i,%i,%i )",
                  index,
                  getCornerType( *tileA, 2, numBaseTypes ),
                  getCornerType( *tileB, 3, numBaseTypes ),
                  getCornerType( *tileC, 1, numBaseTypes ),
                  getCornerType( *tileD, 0, numBaseTypes ) ) );
         }
      }

      pCnr++; index++;
      tileA = tileB;
      tileC = tileD;
      tileB++;
      tileD++;

      for ( int x=1; x<width; x++ )
      {
         if ( !*pCnr )  // don't pick if already set by user
         {
            combinations=0xffffffff;
            combinations &= texCombos[ *tileA ];
            combinations &= texCombos[ *tileB ];
            combinations &= texCombos[ *tileC ];
            combinations &= texCombos[ *tileD ];
            if ( combinations )
            {
               type = pickType( scale, mtxStride, combinations, index );
               *tileA += type*t1;
               *tileB += type;
               *tileC += type*t2;
               *tileD += type*t3;
            }
            else
            {
               pMat[index].index = 0;
               type = 0;
               *tileA += type*t1;
               *tileB += type;
               *tileC += type*t2;
               *tileD += type*t3;
               AssertMessage( combinations, avar("LSMapper::Missing Texture [%i]( %i,%i,%i,%i )",
                     index,
                     getCornerType( *tileA, 2, numBaseTypes ),
                     getCornerType( *tileB, 3, numBaseTypes ),
                     getCornerType( *tileC, 1, numBaseTypes ),
                     getCornerType( *tileD, 0, numBaseTypes ) ) );
            }
         }
         pCnr++; index++; tileA++; tileB++; tileC++; tileD++;
      }
      tileC = tileA;
      if( index == width ) // special case on first row
         tileD = rotationMap;
      if( index > area-width ) // special case on last row
         tileB = rotationMap;
      tileA = tileB+wmask;
   }

   // pick flip state for each texture
   DWORD *pRMap = rotationMap;
   pMat = materialMap;
   static BYTE  *pPtr;
   for ( index=0; index<area; index++ )
   {
      if ( !(pMat->flags & GridBlock::Material::Edit) )  // don't change a texture that is already set by user
      {
         DWORD block = *pRMap;
         pPtr = pickList + pickOffs[block];
         int numFlips = (BYTE)*pPtr;       // get number of valid flip states
         int numTexts = (BYTE)*(pPtr+1);   // get number of valid textures

         // first pick flip state
         int flipPick = 0;
         if ( numFlips > 1 )
            flipPick = rand.getInt( 0, (numFlips-1) );
         pMat->flags &= ~GridBlock::Material::RotateMask;
         pMat->flags |= (BYTE)*(pPtr+2+flipPick);

         // now pick material
         pPtr += 2+numFlips;
         if ( numTexts > 1 ) 
         {
            int percent = rand.getInt( 0, 99 );
            while( (percent -= (BYTE)*(pPtr+1)) > 0 )
               pPtr += 2;
         }
         pMat->index = (BYTE)*(pPtr);
      }
      pMat++;
      pRMap++;
   }

// ES3 specific
//   // Let the navigation system regenerate the nav grid
//   SimMessageEvent::post(
//      manager->findObject(AppNavigationId), AppTerrainGenerationMessage);
}

//--------------------------------------------------------------------------- 

// quick substitute for normalProbability function (in LS_Math.cpp).  Computes
// prob. that a number will lie between val and mean assuming said number is
// drawn from a normal distribution with mean "mean" and standard deviation based
// on "sd" parameter (but not actually = to that parameter).  Crude approx. only.
// See that function for more details.
float prob( float minVal, float maxVal, float mean, float sd, float val)
{

   if (val<=mean)
   {
      if (val<=minVal)
         return 0.5f;
      val = (mean-val)/(mean-minVal); // val = 0..1 ~ mean..min
   }
   else
   {
      if (val>=maxVal)
         return 0.5f;
      val = (val-mean)/(maxVal-mean); // val = 0..1 ~ mean..max
   }
   // return 0 at 0, .18f at sd, and .5f at 1 -- linearly interpolate in between
   // .18f was chosen because w/ the normal dist. ~35% of dist. is +/- 1 sd from mean
   if (val<=sd)
      return 0.18f* val/sd;
   else
      return 0.18f + (val-sd)/(1.0f-sd) * 0.32f;
}

//------------------------------------------------------------------------------

//#define  h(x,y)   _pGB->getHeight(0,Point2I((x)&wmask,(y)&wmask))->height

#define  h(x,y)   heightMap[((y)&wmask)*stride + ((x)&wmask)]

BYTE LSMapper::pickType( int _scale, int stride, int _types, int _index  )
{
   float highScore = -1;
   BYTE  type=0;

   int   wmask = stride-1;

   int   x = _index&wmask;
   int   y = _index/stride;

   // h7 h8 h9
   // h4 h5 h6
   // h1 h2 h3
   // where h5 is the grid cell for which we are picking the type

   float h1 = h(x-1,y-1);
   float h2 = h(x  ,y-1);
   float h3 = h(x+1,y-1);
   float h4 = h(x-1,y);
   float h5 = h(x  ,y);
   float h6 = h(x+1,y);
   float h7 = h(x-1,y+1);
   float h8 = h(x  ,y+1);
   float h9 = h(x+1,y+1);

   float minH = min( min(h1,h2), min(h3, min(h4,h5) ) );
         minH = min( minH, min( min(h6,h7), min(h8,h9) ) );

   float maxH = max( max(h1,h2), max(h3, max(h4,h5) ) );
         maxH = max( maxH, max( max(h6,h7), max(h8,h9) ) );
   
   float _alt = (maxH+minH)*0.5f;
//   float _slope = ( (maxH-_alt) / (1<<_scale) + (_alt-minH) / (1<<_scale) ) * 0.5f;
   float _slope = (maxH-minH) * .5f / (1<<_scale);

   for ( int i=0; i<ruleList.size(); i++ )
   {
      RuleInfo & rule = ruleList[i];
      if ( (1<<rule.groupNum) & _types )
      {
         float score;
         if ( ( rule.adjHeights && (_alt<rule.Altitude.min || _alt>rule.Altitude.max) ) ||
            ( rule.adjSlopes && (_slope<rule.Slope.min || _slope>rule.Slope.max) ) )
             score=0;
         else
         {
//            score = rule.AltWeight*(1-2*normalProbability( rule.Altitude.min, rule.Altitude.max, rule.Altitude.mean, rule.Altitude.sdev, _alt ));
//            score += rule.SlopeWeight*(1-2*normalProbability( rule.Slope.min, rule.Slope.max, rule.Slope.mean, rule.Slope.sdev, _slope ));
            score = rule.AltWeight*(1-2*prob( rule.Altitude.min, rule.Altitude.max, rule.Altitude.mean, rule.Altitude.sdev, _alt ));
            score += rule.SlopeWeight*(1-2*prob( rule.Slope.min, rule.Slope.max, rule.Slope.mean, rule.Altitude.sdev, _slope ));
         }
         if ( score > highScore )
         {
            highScore = score;
            type = rule.groupNum+1;
         }
      }
   }
   return ( type );
}

//------------------------------------------------------------------------------

void LSMapper::worldToHFCoords( Point3F world_pt, Point2I &hf_pt)
{
   Int32 length = ( terrain()->getGridFile()->getBlock(Point2I(0,0)) )->getHeightMapWidth();
   Int32 scale = terrain()->getGridFile()->getScale();

   Point3F preTrans_world_pt;
   m_mul(world_pt, terrain()->getInvTransform(), &preTrans_world_pt);
   
   hf_pt.x = (Int32)(preTrans_world_pt.x);
   hf_pt.y = (Int32)(preTrans_world_pt.y);
   hf_pt.x = hf_pt.x >> scale;
   hf_pt.y = hf_pt.y >> scale;
   hf_pt.x = (hf_pt.x + length) % length;
   hf_pt.y = (hf_pt.y + length) % length;
}   

//------------------------------------------------------------------------------

void LSMapper::worldToTileCoords( Point3F world_pt, Point2I &tile_pt)
{
   Int32 length = ( terrain()->getGridFile()->getBlock(Point2I(0,0)) )->getSize().x;
   Int32 scale = terrain()->getGridFile()->getScale();

   Point3F preTrans_world_pt;
   m_mul(world_pt, terrain()->getInvTransform(), &preTrans_world_pt);
   
   tile_pt.x = (Int32)(preTrans_world_pt.x);
   tile_pt.y = (Int32)(preTrans_world_pt.y);
   tile_pt.x = tile_pt.x >> scale;
   tile_pt.y = tile_pt.y >> scale;
   tile_pt.x = (tile_pt.x + length) % length;
   tile_pt.y = (tile_pt.y + length) % length;
} 

//--------------------------------------------------------------------------- 

void LSMapper::onRemove()
{
   matList.unlock();

   Parent::onRemove();
}


//------------------------------------------------------------------------------

void LSMapper::resetTextures( GridBlock *pGB )
{
   // init material and flags to -1
   int gbWidth = pGB->getSize().x;
   int size = gbWidth * gbWidth;
   GridBlock::Material* pMat = pGB->getMaterialMap();
   pGB->setDirtyFlag();

   memset( matCornerMap, 0, size );
   memset( rotationMap, 0, size<<2 );
   
   Point2I pt( 0, 0 );
   for ( pt.y = 0; pt.y < gbWidth; pt.y++ )
      for ( pt.x = 0; pt.x < gbWidth; pt.x++, pMat++ )
         if ( pMat->flags & GridBlock::Material::Corner )
         {
            BYTE flag = pMat->flags & GridBlock::Material::RotateMask;
            Block corners = arrange((Block)texCorners[pMat->index], xyr[flag].block);
            BYTE type = corners.corner.ll;
            seedCorner( pGB->getSize().x,pt,type );
            pMat->flags = GridBlock::Material::Corner;
            pMat->index = 0xff;
         }
         else if ( pMat->flags & GridBlock::Material::Edit )
            seedCorners( pGB->getSize().x, pt, pMat->index, pMat->flags );
         else
         {
            pMat->index = 0xff;
            pMat->flags = 0;
         }
}   

//------------------------------------------------------------------------------

void LSMapper::rotateCoord(Point2I &vert, LS_ROT_90S rotation)
{
    Int32 tmp;

    switch (rotation)
    {
        case CCW_0:
            return;

        case CCW_90:
            tmp = vert.x;
            vert.x = -vert.y;
            vert.y = tmp;
            return;

        case CCW_180:
            vert.x = -vert.x;
            vert.y = -vert.y;
            return;

        case CCW_270:
            tmp = vert.x;
            vert.x = vert.y;
            vert.y = -tmp;
            return;
    }
}   

//----------------------------------------------------------------------------

SimTerrain *LSMapper::terrain()
{
   SimObject *obj = manager->findObject( terrainId );
   if ( obj )
      return ( dynamic_cast<SimTerrain*>(obj) );
   else
      return (NULL);
}   

//----------------------------------------------------------------------------

bool LSMapper::applyLandScape()
{
   GridFile *pGF = terrain()->getGridFile(); 
   GridBlock *pGB = pGF->getBlock(Point2I(0,0));
   int width = pGB->getHeightMapWidth() - 1;

   pLS->flipY();  // turns image upside down because grid(0,0) is on bottom

   switch( terrain()->getGridFile()->getBlockPattern() )
   {
      case GridFile::OneBlockMapsToAll:
         pLS->setGrdHeightMap( (float*)pGB->getHeightMap(), Box2I(0,0,width+1,width+1) );
         pGB->updateHeightRange();
         break;

      case GridFile::EachBlockUnique:
         {
         // check to see if gridfile same size as stack's bitmap
         if ( width*(pGF->getSize().x) != pLS->getSize() )
         {
            AssertWarn( 0, avar("GridFile size:%i, does not match Lanscape size:%i",(width)*pGF->getSize().x,pLS->getSize()) );
            return false;
         }
         for (int y=0; y<pGF->getSize().y; y++)
            for ( int x=0; x<pGF->getSize().x; x++)
            {
               pGB = pGF->getBlock(Point2I(x,y));
               pLS->setGrdHeightMap( (float*)pGB->getHeightMap(), 
                  Box2I(x*width,y*width,x*width+width+1,y*width+width+1) );
               pGB->updateHeightRange();
            }
         }
         break;

      case GridFile::MosaicBlockPattern1:
         {
         // check to see if landscape is correct size
         if ( width*4 != pLS->getSize() )
         {
            AssertWarn( 0, avar("GridFile size:%i, does not match Lanscape size:%i",(width)*pGF->getSize().x,pLS->getSize()) );
            return false;
         }
         GridBlockList *blockList = terrain()->getGridFile()->getBlockList();
         for ( int i=0; i<16; i++)
         {
            GridBlock   *pCGB;      // center grid block
            int y = i>>2;
            int x = i&3;
            pGB  = blockList->findBlock( i );
            pCGB = blockList->findBlock( 16+i );
            pLS->setGrdHeightMap( (float*)pGB->getHeightMap(), 
               Box2I(x*width,y*width,x*width+width+1,y*width+width+1) );
            pLS->setGrdHeightMap( (float*)pCGB->getHeightMap(), 
               Box2I(x*width,y*width,x*width+width+1,y*width+width+1) );
            pGB->updateHeightRange();
            pCGB->updateHeightRange();
         }
         }
         break;

      default:
         AssertFatal(0,avar("LSMapper::applyLandscape:  Unsupported Block Pattern: %i",terrain()->getGridFile()->getBlockPattern()) );
         break;
   }

   pLS->flipY();  // turns image back to normal

   pGF->updateHeightRange();
   terrain()->updateBoundingBox();
   return true;
}   

//----------------------------------------------------------------------------
// The following function allows you to retexture an nxn area of terrain
// The rectangular area must be a power of 2, i.e.( 2,4,8,...)
// This function is slow for large areas, so if area is most of terrain,
// just use the regular applyTextures() method
// NOTE:  might behave badly along edges of the terrain

bool LSMapper::applyLocalTextures( const RectI &rec )
{
   int area = (rec.len_x()+1) * (rec.len_y()+1);

   matCornerMap = new BYTE[ area ];
   rotationMap  = new DWORD[ area ];
   heightMap    = new float[ area ];
   materialMap  = new GridBlock::Material[ area ];

   seedLocalTextureApplicator( rec );
   applyTextures( rec.len_x()+1 );
   storeLocalTextures( rec );

   // something to rebuild light map for a region???
   // terrain()->buildLightMap()

   delete [] matCornerMap; matCornerMap = NULL;
   delete [] rotationMap;  rotationMap  = NULL;
   delete [] heightMap;    heightMap    = NULL;
   delete [] materialMap;  materialMap  = NULL;

   return true;
}   

// NOTE:  might behave badly along edges of the terrain
void LSMapper::seedLocalTextureApplicator( const RectI &rec )
{
   int area = (rec.len_x()+1) * (rec.len_y()+1);
   memset( matCornerMap, 0, area );
   memset( rotationMap, 0, area<<2 );
   memset( heightMap, 0, area<<2 );
   memset( materialMap, 0, sizeof(GridBlock::Material)*area );

   GridBlock::Material *pMM = materialMap;
   float               *pHM = heightMap;

   for ( int y=rec.upperL.y; y<=rec.lowerR.y; y++ )
      for ( int x=rec.upperL.x; x<=rec.lowerR.x; x++ )
      {
         Point2I pos(x,y);
         *pMM = *(getMaterial( pos ));
         *pHM = getHeight( pos );

         Point2I lpos = pos;
         lpos -= rec.upperL;
//         if ( pMM->flags & GridBlock::Material::Edit )
            seedCorners( rec.len_x()+1, lpos, pMM->index , pMM->flags );
//         else
//            pMM->index = pMM->flags = 0xff;

         pMM->index = pMM->flags = 0xff;

         pMM++;
         pHM++;
      }
}   

// NOTE:  might behave badly along edges of the terrain
void LSMapper::storeLocalTextures( const RectI &rec )
{
   GridBlock::Material *pMM = materialMap;
   for ( int y=rec.upperL.y; y<=rec.lowerR.y; y++ )
      for ( int x=rec.upperL.x; x<=rec.lowerR.x; x++ )
      {
         GridBlock::Material *mat = getMaterial( Point2I(x,y) );
         mat->index = pMM->index;
         mat->flags = pMM->flags;
         pMM++;
      }
}   

//----------------------------------------------------------------------------
// The following function allows you to force a corner on the terrain
// to a particular type and re-apply textures around that corner
// NOTE:  might behave badly along edges of the terrain

bool LSMapper::forceCornerType( Point2I &pt, BYTE type )
{
   RectI rec( pt.x-2, pt.y-2, pt.x+1, pt.y+1 );

   int area = 16;  // a 4x4 region

   matCornerMap = new BYTE[ area ];
   rotationMap  = new DWORD[ area ];
   heightMap    = new float[ area ];
   materialMap  = new GridBlock::Material[ area ];

   seedLocalTextureApplicator( rec );

   // something here to stuff the corner in the rotation map
   seedCorner( rec.len_x()+1, Point2I(2,2), type );

   // then re-apply textures around it and store it back into terrain
   applyTextures( rec.len_x()+1 );
   storeLocalTextures( rec );

   // something to rebuild light map for a region???
   // terrain()->buildLightMap()

   delete [] matCornerMap; matCornerMap = NULL;
   delete [] rotationMap;  rotationMap  = NULL;
   delete [] heightMap;    heightMap    = NULL;
   delete [] materialMap;  materialMap  = NULL;

   return true;
}   

//----------------------------------------------------------------------------

bool LSMapper::applyTextures( PointList *pl, BYTE type )
{
   if ( !flags.test(RULES_LOADED|TEXTS_LOADED) )
      return false;

   GridBlockList *blockList = terrain()->getGridFile()->getBlockList();
   int gbWidth = (blockList->findBlock(0))->getSize().x;
   int numBlocks; // number of grid blocks along the side to texture
   RectI recPts1;  // valid points boundaries 
   RectI recPts2;  // special valid point boundaries for mosaic pattern
	Point2I ptMask(-1,-1);
   switch( terrain()->getGridFile()->getBlockPattern() )
   {
      case GridFile::OneBlockMapsToAll:
         {
            numBlocks = 1;
            recPts1( 0,0,gbWidth,gbWidth );
				ptMask.x = ptMask.y = gbWidth - 1;
         }   
         break;
      case GridFile::EachBlockUnique:
         {
            numBlocks = terrain()->getGridFile()->getSize().x;
            recPts1( 0,0,gbWidth*numBlocks,gbWidth*numBlocks );
         }
         break;
      case GridFile::MosaicBlockPattern1:
         {
            numBlocks = 4;    // pattern has a 4x4 grid block pattern
            recPts1( 0,0,gbWidth*numBlocks,gbWidth*numBlocks );

            int gfSize = terrain()->getGridFile()->getSize().x;
            int offset = (gfSize-4)>>1;
            recPts2.upperL( offset*gbWidth,offset*gbWidth );
            recPts2.lowerR = recPts2.upperL;
            recPts2.lowerR += recPts1.lowerR;
         }
         break;
   }

   int tWidth = gbWidth * numBlocks;
   int area = tWidth * tWidth;

   matCornerMap = new BYTE[ area ];
   rotationMap  = new DWORD[ area ];
   heightMap    = new float[ area ];
   materialMap  = new GridBlock::Material[ area ];
   AssertFatal( matCornerMap && rotationMap && heightMap && materialMap, "ApplyTextures: not enough memory");

   seedTextureApplicator( 0, numBlocks );
   if ( pl )
      for ( PointList::iterator itr=pl->begin(); itr!=pl->end(); itr++ ) {
			Point2I ptt;
			ptt.x = (*itr).x & ptMask.x;
			ptt.y = (*itr).y & ptMask.y;
         if ( ptt.x>=recPts1.upperL.x && ptt.x<recPts1.lowerR.x && 
              ptt.y>=recPts1.upperL.y && ptt.y<recPts1.lowerR.y )
            seedCorner( tWidth, ptt, type );
      }

   applyTextures( tWidth );
   storeTextures( 0, numBlocks );

   // mosaic pattern1 has an additional 4x4 block starting at offset 16
   if ( terrain()->getGridFile()->getBlockPattern() == GridFile::MosaicBlockPattern1 )
   {
      seedTextureApplicator( 16, numBlocks );  // offset to gridblock[16]
      if ( pl )
         for ( PointList::iterator itr=pl->begin(); itr!=pl->end(); itr++ ) {
				Point2I ptt;
				ptt.x = (*itr).x & ptMask.x;
				ptt.y = (*itr).y & ptMask.y;
            if ( ptt.x>=recPts2.upperL.x && ptt.x<recPts2.lowerR.x && 
                 ptt.y>=recPts2.upperL.y && ptt.y<recPts2.lowerR.y )
                 {
                    ptt -= recPts2.upperL;
                    seedCorner( tWidth, ptt, type );
                 }
         }

      applyTextures( tWidth );
      storeTextures( 16, numBlocks );
   }

#if 0
   terrain()->buildLightMap();
#endif

   delete [] matCornerMap; matCornerMap = NULL;
   delete [] rotationMap;  rotationMap  = NULL;
   delete [] heightMap;    heightMap    = NULL;
   delete [] materialMap;  materialMap  = NULL;

	Console->evaluate( "flushTextureCache();", false );
   return true;
}   

//----------------------------------------------------------------------------
// blkStart is the index of the grid block to texture first.
// numBlks is the number of grid blocks per side of the area we are
// texturing.  It must be a power of 2.

void LSMapper::seedTextureApplicator( int blkStart, int numBlks )
{
   Point2I pt( 0, 0 );
   GridBlockList *blockList = terrain()->getGridFile()->getBlockList();
   int gbWidth = (blockList->findBlock(blkStart))->getSize().x;
   int tWidth = gbWidth * numBlks;
   int area = tWidth * tWidth;

   memset( matCornerMap, 0, area );
   memset( rotationMap, 0, area<<2 );
   memset( heightMap, 0, area<<2 );
   memset( materialMap, 0, sizeof(GridBlock::Material)*area );

   GridBlock::Material *pMMap = materialMap;
   float *pHMap = heightMap;

   for ( int b=0; b<numBlks*numBlks; b++ )
   {
      // for each grid block in terrain
      GridBlock *pGB = blockList->findBlock(b+blkStart);
      pGB->setDirtyFlag();
      GridBlock::Material *pGBMMap = pGB->getMaterialMap();
      GridBlock::Height   *pGBHMap = pGB->getHeightMap();
      for ( int i=0; i<gbWidth; i++ )
      {
         // for each row in grid block
         int y = b/numBlks;
         int x = b&(numBlks-1);
         pt.y = y*gbWidth+i;
         int rowOff = (pt.y)*tWidth;
         int colOff = x*gbWidth;
         for ( pt.x=colOff; pt.x<colOff+gbWidth; pt.x++ )
         {
            // for each element in grid block
            if ( pGBMMap->flags & GridBlock::Material::Corner )
            {
               BYTE flag = pGBMMap->flags & GridBlock::Material::RotateMask;
               Block corners = arrange((Block)texCorners[pGBMMap->index], xyr[flag].block);
               BYTE type = corners.corner.ll;
               seedCorner( tWidth,pt,type );
               pGBMMap->flags = GridBlock::Material::Corner;
               pGBMMap->index = 0xff;
            }
            else if ( pGBMMap->flags & GridBlock::Material::Edit )
               seedCorners( tWidth, pt, pGBMMap->index, pGBMMap->flags );
            else
               pGBMMap->index = 0xff;

            pMMap[ rowOff + pt.x ] = *pGBMMap;
            pHMap[ rowOff + pt.x ] = pGBHMap->height;

            pGBMMap++;
            pGBHMap++;
         }
         pGBHMap++;  // because in grdBlock, hmap widths are 1<<scale + 1
      }
   }
}   

//----------------------------------------------------------------------------
// blkStart is the index of the grid block to texture first.
// numBlks is the number of grid blocks per side of the area we are
// texturing.  It must be a power of 2.

void LSMapper::storeTextures( int blkStart, int numBlks )
{
   GridBlockList *blockList = terrain()->getGridFile()->getBlockList();
   int gbWidth = (blockList->findBlock(blkStart))->getSize().x;
   int tWidth = gbWidth * numBlks;

   GridBlock::Material *pMMap = materialMap;

   for ( int b=0; b<numBlks*numBlks; b++ )
   {
      GridBlock *pGB = blockList->findBlock(b+blkStart);
      GridBlock::Material *pGBMMap = pGB->getMaterialMap();
      for ( int y=0; y<gbWidth; y++ )
      {
         int rowOff = ((b/numBlks)*gbWidth+y)*tWidth;
         int colOff = (b&(numBlks-1))*gbWidth;
         for ( int x=0; x<gbWidth; x++ )
         {
            *pGBMMap = pMMap[ rowOff + colOff + x ];
            pGBMMap++;
         }
      }
   }
}

//----------------------------------------------------------------------------

GridBlock::Material *LSMapper::getMaterial( Point2I& pt )
{
   Point2I bpos = pt;
   GridFile *gf = terrain()->getGridFile();
	bpos >>= gf->getScale();
   GridBlock *gb = gf->getBlock( bpos );
   Point2I mpos = pt;
   bpos <<= gf->getScale();
   mpos -= bpos;  // mask off to get position relative to grid block
   GridBlock::Material *pmat = gb->getMaterialMap();
   int index = mpos.y * gb->getMaterialMapWidth() + mpos.x;
   return ( &pmat[index] );
}   

//----------------------------------------------------------------------------

float LSMapper::getHeight( Point2I& pt )
{
   Point2I bpos = pt;
   GridFile *gf = terrain()->getGridFile();
	bpos >>= gf->getScale();
   GridBlock *gb = gf->getBlock( bpos );
   Point2I mpos = pt;
   bpos <<= gf->getScale();
   mpos -= bpos;  // mask off to get position relative to grid block
   float *ph = (float*)gb->getHeightMap();
   int index = mpos.y * gb->getHeightMapWidth() + mpos.x;
   return ( ph[index] );
}   

//----------------------------------------------------------------------------

Point2I& LSMapper::worldToGrid( Point2F& pt )
{
   static Point2I pos( pt.x, pt.y );
   SimTerrain::Context context = terrain()->getContext();
   pos.x += context.pos.x;
   pos.y += context.pos.y;
   GridFile *gf = terrain()->getGridFile();
   pos /= ( 1<<gf->getScale() );
   return pos;
}   

//----------------------------------------------------------------------------

void LSMapper::flushTextures()
{
   terrainTypeList.clear();
   textureList.clear();   
}

//----------------------------------------------------------------------------

bool LSMapper::addTerrainType(UInt8 tag, const char * description)
{
   TerrainType info;
   info.tag = tag;
   strcpy(info.description, description);
   terrainTypeList.push_back(info);
   return(true);
}

//----------------------------------------------------------------------------

int LSMapper::getMaterialType(const char * name)
{
   // return the id
   if(!stricmp(name, "Concrete"))
      return(TS3_ConcreteType);
   if(!stricmp(name, "Carpet"))
      return(TS3_CarpetType);
   if(!stricmp(name, "Metal"))
      return(TS3_MetalType);
   if(!stricmp(name, "Glass"))
      return(TS3_GlassType);
   if(!stricmp(name, "Plastic"))
      return(TS3_PlasticType);
   if(!stricmp(name, "Wood"))
      return(TS3_WoodType);
   if(!stricmp(name, "Marble"))
      return(TS3_MarbleType);
   if(!stricmp(name, "Snow"))
      return(TS3_SnowType);
   if(!stricmp(name, "Ice"))
      return(TS3_IceType);
   if(!stricmp(name, "Sand"))
      return(TS3_SandType);
   if(!stricmp(name, "Mud"))
      return(TS3_MudType);
   if(!stricmp(name, "Stone"))
      return(TS3_StoneType);
   if(!stricmp(name, "SoftEarth"))
      return(TS3_SoftEarthType);
   if(!stricmp(name, "PackedEarth"))
      return(TS3_PackedEarthType);

   return(TS3_DefaultType);
}

//----------------------------------------------------------------------------

bool LSMapper::getTerrainTypeIndex(const UInt8 tag, int & index)
{
   for(int j = 0; j < terrainTypeList.size(); j++)
   {
      if(tag == terrainTypeList[j].tag)
      {
         index = j;
         return(true);
      }
   }
   return(false);
}

//----------------------------------------------------------------------------

bool LSMapper::addTerrainTexture(const char * fileName, UInt32 sides, UInt8 rotations,
   UInt8 percentage, const char * type, float elasticity, float friction)
{
   TerrainTexture info;
   strcpy(info.filename, fileName);

   info.sides = 0;
   // convert the side's into indexes into terrain type
   for(int i = 0; i < 4; i++)
   {
      int j;
      for(j = 0; j < terrainTypeList.size(); j++)
      {
         if(terrainTypeList[j].tag == (UInt8)(sides & 0xff))
         {
            info.sides |= (UInt32)(j & 0xff);
            if(i != 3)
               info.sides <<= 8;
            break;
         }
      }
      
      if(j == terrainTypeList.size())
         return(false);
         
      sides >>= 8;
   }
   
   info.rotations = rotations;
   info.percentage = percentage;
   info.type = getMaterialType(type);
   info.elasticity = elasticity;
   info.friction = friction;
   
   textureList.push_back(info);
   return(true);
}

//----------------------------------------------------------------------------

bool LSMapper::createGridFile(const char * gridFile, const char * matlist)
{
   // create the terrain.dat file
   // ---------------------------
   // numBaseTypes      <int>
   // numBaseTextures   <int>
   // descriptions      [baseTypes * MAXDESC]
   // 
   // for each texture...
   //    textureName       [MAXFILE] - 128 bytes
   //    block             <UInt32> [ul,ur,lr,ll]
   //    rotate            <UInt8>
   //    percentage        <UInt8>
   //    fill              <Int16> [structure fill]
   //    type              <UInt32>
   //    elasticity        <float>
   //    friction          <float>

   AssertISV(ResourceManager::sm_pManager == NULL ||
             ResourceManager::sm_pManager->isValidWriteFileName(gridFile) == true,
             avar("Attempted write to file: %s.\n"
                  "File is not in a writable directory.", gridFile));

   int numTypes = terrainTypeList.size();
   int numTextures = textureList.size();
   
   // dump out the dat file for textable
   FileWStream file("temp\\terrain.dat");
   file.write(numTypes);
   file.write(numTextures);
   int i;
   for(i = 0; i < numTypes; i++)
      file.write(MAXDESC, terrainTypeList[i].description);
   
   // dump the textures
   for(i = 0; i < numTextures; i++)
   {
      file.write(MAXFILE, textureList[i].filename);
      file.write(textureList[i].sides);
      file.write(textureList[i].rotations);
      file.write(textureList[i].percentage);
      file.write((UInt16)0);
      file.write(textureList[i].type);
      file.write(textureList[i].elasticity);
      file.write(textureList[i].friction);
   }
   file.close();
   
   char * argv[5];
   argv[0] = "textable.exe";
   argv[1] = "temp\\terrain.dat";
   argv[2] = const_cast<char*>(gridFile);
   argv[3] = const_cast<char*>(matlist);
   argv[4] = 0;
   spawnvp(P_WAIT, "textable.exe", argv);
   
   return(true);
}

// add a command to the command list
bool LSMapper::addCommand(const char * str)
{
   if(!str || (strlen(str) >= 255))
      return(false);

   ScriptCommand command;
   strcpy(command.execString, str);
   commandList.push_back(command);
   return(true);
}

//--------------------------------------------------------------------------- 

void LSMapper::parseCommands()
{
   pLS->clear();
   for(int i = 0; i < commandList.size(); i++)
      pLS->parseScript(commandList[i].execString, false);
}

