//----------------------------------------------------------------------------
//
// TexTable.cpp
//
// This is a console application to create a terrain texture table for use by
// the Terrains texture applicator.  This tool examines the available textures
// and there properties and creates a table of appropriate textures given
// the types of textures existing in the GridFiles material map matrix.
//
// You pass it a terrain data file (ex. mars.terrain.dat) created from
// a compiled "mars.terrain.cpp" file and stripped using LStrip.exe.  It
// creates two files "mars.grid.dat", the texture table, and "mars.dml", a  
// material list of textures.
//
//----------------------------------------------------------------------------
// 2/1/99:  changed the MAXFILE to 128 bytes, this should allow for
// JohnF    a full 256 textures while keeping the size <64k.  Currently,
//          having a data segment of >64k causes objstrip to fail - this
//          will happen when the terrain has around 235-240+ textures.
//          Uses this formula to detect older versions:
//             Filesize = 8 + (32 * numTerrainTypes) + (276 * numTextures)
//----------------------------------------------------------------------------

#include <types.h>
#include <grd.h>

#define MAXDESC         32    // also defined in datTerrain.h
#define MAXFILE         128   // also defined in datTerrain.h
#define PREV_MAXFILE    256   // for older terrains

#define  TT_VERSION     3

//----------------------------------------------------------------------------

struct   Pick
{
   BYTE  id;
   BYTE  flag;
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

struct TexData
{
   BYTE     id;
   BYTE     flag;
   BYTE     percentage;
   Block    block;
};

//----------------------------------------------------------------------------

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

//----------------------------------------------------------------------------

class TexGrid
{
   int      numBaseTypes;           // number of base types
   int      numBaseTextures;        // number of original textures
   int      numTextureCombos;       // number of all texture combinations
   int      numPicks;               // number of entries in pick table
   Block    *blocks;                // array of block data for each base texture
   TexData  *texData;               // array of texture combinataion information 
   DWORD    *texCombos;             // n*n*n*n matrix of valid combinations (in bit-field format)
   DWORD    *pickOffs;              // table of offsets into pickList
   BYTE     *pickList;              // collection of picks
   BYTE     *pickPtr;
   TSMaterialList *matList;         // ptr to generated material list
   char     *typList;               // ptr to list of type descriptions 
   int      dataVersion;            // version of the terrain.dat file           
      
public:
   TexGrid();
   ~TexGrid();
   bool  load( const char *_filename );
   void  getDataVersion(FileRStream & stream);
   bool  save( const char *_filename );
   bool  saveMaterialList( const char *_filename );
   void  generateCombos();
   void  generatePickList();
   DWORD validCombos( BYTE _c0, BYTE _c1, BYTE _c2, BYTE _c3 );
   DWORD findPicks( BYTE _c0, BYTE _c1, BYTE _c2, BYTE _c3 );
};

//############################################################################

int main(int argc, char *argv[], char *)
{
   bool result = false;
   if ( argc == 4 )
   {
      TexGrid txt;
      result = txt.load( argv[1] );
      AssertFatal( result, avar("TexTable: Unable to load %s.", argv[1]));

      txt.generateCombos();
      txt.generatePickList();

      result = txt.save( argv[2] );
      AssertFatal( result, avar("TexTable: Unable to write file %s.", argv[2]));

      result = txt.saveMaterialList( argv[3] );
      AssertFatal( result, avar("TexTable: Unable to write file %s.", argv[3]));
   }

   if ( !result )
   {
      printf( "TexTable V2.1 usage:\n" );
      printf( "   TexTable mars.terrain.dat mars.Grid.dat mars.dml\n" );
      printf( "   mars.terrain.dat is passed in\n" );
      printf( "   mars.grid.dat and mars.dml are created\n" );
   }

   return (!result);
}

//############################################################################

TexGrid::TexGrid()
{
   numBaseTypes      = 0;
   numBaseTextures   = 0;
   numTextureCombos  = 0;
   numPicks          = 0;
   dataVersion       = 0;

   texData           = NULL;
   texCombos         = NULL;
   pickOffs          = NULL;
   pickList          = NULL;
   matList           = NULL;
}

TexGrid::~TexGrid()
{
}

//----------------------------------------------------------------------------

Block arrange( Block _block, Block _blockOrder )
{
   Block block;
   block.corner.ul = 0xff&(_block.block>>(_blockOrder.corner.ul<<3));
   block.corner.ur = 0xff&(_block.block>>(_blockOrder.corner.ur<<3));
   block.corner.lr = 0xff&(_block.block>>(_blockOrder.corner.lr<<3));
   block.corner.ll = 0xff&(_block.block>>(_blockOrder.corner.ll<<3));
   return ( block );
}

//----------------------------------------------------------------------------
// get's the version for the in-data file
void TexGrid::getDataVersion(FileRStream & stream)
{
   int numTypes;
   int numTextures;
   int fileSize;
   
   // read in the num types and textures, size... then reset
   stream.read(&numTypes);
   stream.read(&numTextures);
   fileSize = stream.getSize();
   stream.setPosition(0);
   
   // Uses this formula to detect older versions:
   //    Filesize = 8 + (32 * numTerrainTypes) + (276 * numTextures)
   if(fileSize == (8 + (32 * numTypes) + (276 * numTextures)))
   {
      printf( "   -- detected old terrain.dat file (MAXFILE of 256 bytes)\n");
      dataVersion = 0;
   }
   else
      dataVersion = 1;
}

//----------------------------------------------------------------------------

bool  TexGrid::load( const char *_filename )
{
   TSMaterial  mat;
   bool        result=true;
   HANDLE      fileHandle;
   fileHandle = CreateFile(_filename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
   if ( fileHandle == (HANDLE)-1 ) return false;

   mat.fParams.fAlpha = 0;
   mat.fParams.fFlags = TSMaterial::MatTexture | TSMaterial::ShadingNone;

   FileRStream frs(fileHandle);
   getDataVersion(frs);
   
   result &= frs.read( &numBaseTypes );
   result &= frs.read( &numBaseTextures );
   typList = new char[ numBaseTypes*MAXDESC ];
   result &= frs.read( numBaseTypes*MAXDESC, (BYTE*)typList );
   matList = new TSMaterialList( numBaseTextures );
   texData = new TexData[ numBaseTextures * 8 ];
   blocks  = new Block[ numBaseTextures ];
   TexData *pTexData = texData;
   for ( int i=0; result && i<numBaseTextures; i++ )
   {
      char  buff[1024];
      Block block;
      BYTE  states;
      BYTE  percentage;

      // check if older version.. currently only two versions
      result &= frs.read((dataVersion == 0) ? PREV_MAXFILE : MAXFILE, buff );    // texture name
      strcpy( mat.fParams.fMapFile, buff );

      result &= frs.read( sizeof(Block),(BYTE*)&block );  // read block definitions for this textre
      result &= frs.read( &states );         // read valid flip/rot states for this texture
      result &= frs.read( &percentage);      // read percentage pick for this sub-texture 

		// Structure allignment.
		UInt16 fill;
		result &= frs.read( &fill );

		result &= frs.read( &mat.fParams.fType );
		result &= frs.read( &mat.fParams.fElasticity );
		result &= frs.read( &mat.fParams.fFriction );
      matList->setMaterial(i,mat);

      blocks[i] = block;                     // save original block data
      for ( int j=0; j<8; j++ )
      {
         if ( states & 1<<j )       // if flip/rot state supported for this texture
         {
            pTexData->id          = i;
            pTexData->flag        = 1<<j;
            pTexData->percentage  = percentage;
            pTexData->block       = arrange( block, xyr[j].block );
            pTexData++;
            numTextureCombos++;
         }
      }
   }

   frs.close();
   CloseHandle(fileHandle);
   return ( result );
}   

//----------------------------------------------------------------------------

void TexGrid::generateCombos( )
{
   int   numBTW = numBaseTypes + 1;
   texCombos = new DWORD[ numBTW * numBTW * numBTW * numBTW ];
   DWORD *pTC = texCombos;

   for ( int c0=0; c0<numBTW; c0++ )
      for ( int c1=0; c1<numBTW; c1++ )
         for ( int c2=0; c2<numBTW; c2++ )
            for ( int c3=0; c3<numBTW; c3++ )
               *pTC++ = validCombos( c0, c1, c2, c3 );
}

//----------------------------------------------------------------------------

void TexGrid::generatePickList()
{
   int   numBTW = numBaseTypes + 1;
   pickOffs = new DWORD[ numBTW * numBTW * numBTW * numBTW + 1 ];
   pickList = new BYTE[ numBTW * numBTW * numBTW * numBTW * 8 ];
   pickPtr = pickList;
   DWORD *pPickOffs = pickOffs;
   *pPickOffs++ = 0;

   for ( int c0=0; c0<numBTW; c0++ )
      for ( int c1=0; c1<numBTW; c1++ )
         for ( int c2=0; c2<numBTW; c2++ )
            for ( int c3=0; c3<numBTW; c3++ )
               // generate table of flip states and materials for each
               // combination and return the offset to the end of the table
               *pPickOffs++ = findPicks( c0, c1, c2, c3 );
}

//----------------------------------------------------------------------------

DWORD TexGrid::validCombos( BYTE _c0, BYTE _c1, BYTE _c2, BYTE _c3 )
{
   DWORD  combinations=0;
   TexData *pTexData = texData;

   for ( int i=0; i<numTextureCombos; i++ )
   {
      bool result = true;
      if ( _c0 && ((_c0-1)!=pTexData->block.corner.ul) ) result = false;
      if ( _c1 && ((_c1-1)!=pTexData->block.corner.ur) ) result = false;
      if ( _c2 && ((_c2-1)!=pTexData->block.corner.lr) ) result = false;
      if ( _c3 && ((_c3-1)!=pTexData->block.corner.ll) ) result = false;
      if ( result )
      {
         if ( !_c0 ) combinations |= 1<<pTexData->block.corner.ul;
         if ( !_c1 ) combinations |= 1<<pTexData->block.corner.ur;
         if ( !_c2 ) combinations |= 1<<pTexData->block.corner.lr;
         if ( !_c3 ) combinations |= 1<<pTexData->block.corner.ll;
      }
      pTexData++;
   }
   return ( combinations );
}

//----------------------------------------------------------------------------

DWORD TexGrid::findPicks( BYTE _c0, BYTE _c1, BYTE _c2, BYTE _c3 )
{
   Block    check= {_c0-1,_c1-1,_c2-1,_c3-1};
   TexData  *pTexData = texData;
   BYTE     textureIDs[256];
   BYTE     texturePct[256];
   BYTE     states = 0;
   BYTE     numTextures = 0;
   BYTE     numFlipStates = 0;

   for ( int i=0; i<numTextureCombos; i++ )
   {
      if ( pTexData->block.block == check.block )
      {
         states |= pTexData->flag;

         bool found = false;
         for ( int j=0; !found && j<numTextures; j++ )
            if ( pTexData->id == textureIDs[j] )
               found = true;

         // don't add if already in list
         if ( !found )
         {
            textureIDs[ numTextures ] = pTexData->id;
            texturePct[ numTextures ] = pTexData->percentage;
            numTextures++;
         }
      }
      pTexData++;
   }

   if ( numTextures )   // store to memory
   {
      for ( int i=0; i<8; i++ )                       // get # of flip states
         if ( states&(1<<i) ) numFlipStates++;

      *pickPtr++ = numFlipStates;                     // store # of flip states
      *pickPtr++ = numTextures;                       // stpre # of textures

      for ( int i=0; i<8; i++ )
         if ( states&(1<<i) ) *pickPtr++ = xyr[i].flag;  // store flip flags

      for ( int i=0; i<numTextures; i++ )             // store texture ids
      {
         *pickPtr++ = textureIDs[i];
         *pickPtr++ = texturePct[i];
      }
   }
   return( pickPtr - pickList );
}

//----------------------------------------------------------------------------

bool  TexGrid::save( const char *_filename )
{
   bool     result=true;
   HANDLE   fileHandle;
   fileHandle = CreateFile(_filename,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
   if ( fileHandle == (HANDLE)-1 ) return false;

   FileWStream fws(fileHandle);

   int   numBTW = numBaseTypes + 1;

   int version = TT_VERSION;
   result &= fws.write( version );
   result &= fws.write( numBaseTypes );
   result &= fws.write( numBaseTextures );
   result &= fws.write( numPicks );
   result &= fws.write( numBaseTypes*MAXDESC, typList );
   for ( int i=0; result && i<numBaseTextures; i++ )
      result &= fws.write( blocks[i].block );
   result &= fws.write( sizeof(DWORD)*numBTW*numBTW*numBTW*numBTW, (BYTE*)texCombos );
   result &= fws.write( sizeof(DWORD)*(numBTW*numBTW*numBTW*numBTW+1), (BYTE*)pickOffs );
   result &= fws.write( (pickPtr-pickList), (BYTE*)pickList );

   fws.close();
   CloseHandle(fileHandle);
   return ( result );
}

//----------------------------------------------------------------------------

bool TexGrid::saveMaterialList( const char *_filename )
{
   bool     result;
   HANDLE   fileHandle;
   fileHandle = CreateFile(_filename,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
   if ( fileHandle == (HANDLE)-1 ) return false;

   FileWStream fws(fileHandle);

   result = matList->store( fws );

   fws.close();
   CloseHandle(fileHandle);
   return ( !result );
}   