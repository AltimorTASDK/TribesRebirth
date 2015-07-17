//----------------------------------------------------------------------------
//   
//  $Workfile:   LSMapper.cpp  $
//  $Version$
//  $Revision:   1.00  $
//    
//  DESCRIPTION:
//        Class for applying LandScapes and Textures to trerrain
//        
//  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
// 
//----------------------------------------------------------------------------

#ifndef _LSMAPPER_H_
#define _LSMAPPER_H_

#include "LS.h"
#include "LSStampInst.h"
#include <sim.h>
#include <SimTerrain.h>
#include <bitset.h>
#include "ts_materialtypes.h"

//--------------------------------------------------------------------------- 

#define MAXDESC   32    // also defined in datTerrain.h and TexTable.cpp

struct   Distribution
{
   float          min;
   float          max;
   float          mean;
   float          sdev;
};

struct RuleInfo
{
   int            groupNum;      // what texture group does this rule apply to?
   Distribution   Altitude;
   float          AltWeight;
   int            adjHeights;
   Distribution   Slope;
   float          SlopeWeight;
   int            adjSlopes;
};

struct LSMaterial
{
   BYTE           texIndx;       // which base texture are we using
   BYTE           flag;          // what are its TSMaterial flag settings
   short          sides[8];      // what are its sides
   LSMaterial( BYTE _flag, short *_sides );
   LSMaterial( int _texIndx, BYTE _flag ) { texIndx=_texIndx, flag=_flag; }
};

union   Block
{
   struct
   {
      BYTE  ul;
      BYTE  ur;
      BYTE  lr;
      BYTE  ll;
   }corner;
   DWORD block;
};

//########################################################################### 

class LSMapper : public SimObject
{
private:
   typedef     SimObject Parent;

   LandScape   *pLS;                // ptr to a LandScape generator object
   int         terrainId;           // terrain associated with this object

   // Texture Mapping members
   int         numRules;
   int         numBaseTypes;
   int         numBaseTextures;     
   int         numPicks;
   Vector< RuleInfo > ruleList;
   DWORD       *texCombos;          // n*n*n*n matrix of valid combinations (in bit-field format)
   DWORD       *pickOffs;           // table of offsets into pickList
   BYTE        *pickList;           // collection of picks
   Random      rand;
   BYTE        *matCornerMap;       // corner map
   DWORD       *rotationMap;        // encoded texture type map
   Block       *texCorners;         // array of texture corner definitions
   float       *heightMap;          // array of corner heights
   char        *typeText;           // ptr to array of type descriptions
   GridBlock::Material *materialMap;   // array of GB materials for entire terrain
   Resource<TSMaterialList> matList;

public:
   BitSet32    flags;
   static const char * ruleExt;
   static const int ruleVersion;
   
   enum
   {
      RULES_LOADED   = (1<<0),
      TEXTS_LOADED   = (1<<1),
   };

   #define MAXDESC   32
   #define MAXFILE   128
   
   class TerrainType {
      public:
         UInt8    tag;
         char     description[MAXDESC];
   };

   class TerrainTexture {
      public:
         char     filename[MAXFILE];
         UInt32   sides;
         UInt8    rotations;
         UInt8    percentage;
         int      type;
         float    elasticity;
         float    friction;
   };

   typedef Vector<Point2I>          PointList;

private:

   Vector<TerrainType>              terrainTypeList;
   Vector<TerrainTexture>           textureList;

   struct ScriptCommand
   {
      char  execString[256];
   };
   Vector<ScriptCommand>            commandList; 
  
public:

   // new methods
   void  flushTextures();
   bool  addTerrainType(UInt8 tag, const char * description);
   int   getMaterialType(const char * name);
   bool  addTerrainTexture(const char * fileName, UInt32 sides, UInt8 rotations,
      UInt8 percentage, const char * type, float elasticity, float friction);
   bool  createGridFile(const char * gridFile, const char * matlist);
   void  flushRules(){ruleList.clear();}
   void  addRule(const RuleInfo & rule){ruleList.push_back(rule);}
   bool  getTerrainTypeIndex(const UInt8 tag, int & index);
   void  flushCommands(){commandList.clear();}
   bool  addCommand(const char * str);
   void  parseCommands();
      
private:
   SimTerrain  *terrain();

   // simobject methods
   void onRemove();

   // Texture Mapping Methods
   void  rotateCoord(Point2I &vert, LS_ROT_90S rotation);
   void  worldToHFCoords( Point3F world_pt, Point2I &hf_pt);
   void  worldToTileCoords( Point3F world_pt, Point2I &tile_pt);
   bool  applyTerrainStamp(const Point2I &gbPt, Point3F origin, LS_ROT_90S rot, LSTerrainStamp *stamp );
   void  seedCorners( int width, Point2I &pt, BYTE index, BYTE flags );
   void  seedCorner( int width, Point2I &pt, BYTE type );
   void  seedTexByTile( const Point2I &gbPt, const Point2I &origin, LS_ROT_90S rot, LSTerrainStamp *stamp );
   BYTE  pickType( int _scale, int mtxStride, int _types, int _index );
   void  setHeightField( const Point2I &gbPt );
   void  seedTextureApplicator( int blkStart, int numBlks );
   void  seedLocalTextureApplicator( const RectI &rec );
   void  applyTextures( int mtxStride );
   void  storeTextures( int blkStart, int numBlks );
   void  storeLocalTextures( const RectI &rec );
   void  resetTextures( GridBlock *pGB );
   Point2I& worldToGrid( Point2F& pt );
   GridBlock::Material *getMaterial( Point2I& pt );
   float getHeight( Point2I& pt );

public:
   LSMapper();
   ~LSMapper();
   bool create( SimTerrain *pTerrain );

   // terrain rule functions
   const char * getExtension( const char * file );
   bool setRules( const char * fileName );
   bool setRules( const Vector< RuleInfo > & rules );
   bool loadRules( const char * fileName, Vector< RuleInfo > & rules );
   bool saveRules( const char * fileName, const Vector< RuleInfo > & rules );
   Vector< RuleInfo > * getRuleList(){ return( &ruleList ); }
   
   bool setTextures( const char *_materialFile, const char *_gridFile );
   bool setScript( const char *_landscapeFile );
   bool applyLandScape();  // to entire terrain
   bool applyTextures( PointList* pl=0, BYTE type=0 );   // to entire terrain
   bool applyLocalTextures( const RectI &rec );   // to region of terrain
   bool forceCornerType( Point2I &pt, BYTE type );
   LandScape   *getLandScape() { return pLS; }
   char *getTypeDesc( bool first=false );
   int   getNumBaseTypes(){ return( numBaseTypes ); }
};

#endif   // _LSMAPPER_H_
