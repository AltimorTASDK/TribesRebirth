//---------------------------------------------------------------------------

//	$Workfile:   ts_mesh.h  $
//	$Revision:   2.8  $
// $Version$
//	$Date:   15 Sep 1995 10:27:12  $
//	$Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
//	

//---------------------------------------------------------------------------

#include <g_pal.h>

#include <ts_Res.h>
#include "ts_Material.h"

namespace TS
{
static ResourceTypeTSMaterialList _resdml(".dml");


//---------------------------------------------------------------------------

// static members used by material renderers ... not for general use
Point3F Material::scale;
Point3F Material::origin;
int Material::firstVert = 0;
const PackedVertex * Material::pVerts = NULL;
GFXSurface * Material::srf = NULL;
PointArray * Material::pa = NULL;
const ShapeInstance * Material::si = NULL;

// Renderers for the material types

// code: matrenderXYZ where
//       X is shading type (S, F, or N for smooth, flat, or none)
//       Y is material type (T, R, or P for texture, RGB, or palette)
//       Z is texture type (U, A, or nothing for translUcent, transpArent, or neither)

// the order they are listed is roughly in order of frequency
// non-translucent, non-transparent
extern void matrenderST(const Material * mat, const char * pFace);
extern void matrenderFT(const Material * mat, const char * pFace);
extern void matrenderNR(const Material * mat, const char * pFace);
extern void matrenderFR(const Material * mat, const char * pFace);
extern void matrenderNT(const Material * mat, const char * pFace);
extern void matrenderSR(const Material * mat, const char * pFace);
extern void matrenderNP(const Material * mat, const char * pFace);
extern void matrenderFP(const Material * mat, const char * pFace);
extern void matrenderSP(const Material * mat, const char * pFace);

// translucent
extern void matrenderNTU(const Material * mat, const char * pFace);

// transparent
extern void matrenderNTA(const Material * mat, const char * pFace);

//---------------------------------------------------------------------------
  
Material::Material()
{
   fTextureMap = 0;
   fnRender = NULL;
   memset( &fParams, 0, sizeof( fParams ) );
	fParams.fElasticity = 1.0f;
	fParams.fFriction = 1.0f;
   fParams.fUseDefaultProps = true;
}

Material::Material( const Material & mat)
{
   fTextureMap = 0;
   fParams     = mat.fParams;
}

Material::~Material()
{
}

Material & Material::operator=( const Material & mat )
{
   fParams = mat.fParams;
   return *this;
}

int Material::operator==( const Material & mat )
{
   return ( (fParams.fFlags == mat.fParams.fFlags) &&
            (fParams.fIndex == mat.fParams.fIndex) &&
            IsEqual( fParams.fAlpha, mat.fParams.fAlpha ) &&
            (fParams.fRGB == mat.fParams.fRGB) &&
            !strcmp( fParams.fMapFile, mat.fParams.fMapFile ) );
}

void Material::load( ResourceManager & rm, Bool block )
{
   block;
   switch( fParams.fFlags & MatFlags )
   {
      case MatNull:
      case MatPalette:
         break;
      case MatRGB:
#if 1
//  				AssertWarn(0,"TS::Material::lock: Could not resolve RGB color");
			fParams.fIndex = 0;
#else            
       if ( rc.getSurface()->getPalette() )
          fParams.fIndex = rc.getSurface()->getPalette()->
              GetNearestColor( *(PALETTEENTRY*)(&fParams.fRGB) );
#endif
          break;
       case MatTexture:
          fTextureMap = rm.load( fParams.fMapFile );
          AssertFatal( !block || (GFXBitmap*)fTextureMap, 
             avar("TS::Material::load: texture '%s'load failed",fParams.fMapFile ) );
          break;
       default:
          AssertFatal( 1, "TS::Material::load: invalid material type" );
          break;
   }
   resolveRender();
}

void Material::unload()
{
   switch( fParams.fFlags & MatFlags )
      {
      case MatNull:
      case MatPalette:
      case MatRGB:
         break;
      case MatTexture:
         fTextureMap.unlock();
         break;
      default:
         AssertFatal( 1, "TS::Material::unload: invalid material type" );
         break;
      }
   fnRender = NULL;
}

Bool Material::isReady() const
{
   if( ((fParams.fFlags & MatFlags) == MatTexture) && 
       !(GFXBitmap const *)fTextureMap )
      return FALSE;
   return TRUE;
}

void Material::resolveRender()
{
   switch (fParams.fFlags)
   {
      // non-translucent, non-transparent
      case ShadingSmooth | MatTexture:
         fnRender = matrenderST;
         break;
      case ShadingFlat   | MatTexture:
         fnRender = matrenderFT;
         break;
      case ShadingNone   | MatRGB    :
         fnRender = matrenderNR;
         break;
      case ShadingFlat   | MatRGB    :
         fnRender = matrenderFR;
         break;
      case ShadingNone   | MatTexture:
         fnRender = matrenderNT;
         break;
      case ShadingSmooth | MatRGB    :
         fnRender = matrenderSR;
         break;
      case ShadingNone   | MatPalette:
         fnRender = matrenderNP;
         break;
      case ShadingFlat   | MatPalette:
         fnRender = matrenderFP;
         break;
      case ShadingSmooth | MatPalette:
         fnRender = matrenderSP;
         break;

      // translucent
      case ShadingSmooth | MatTexture | TextureTranslucent:
      case ShadingNone   | MatTexture | TextureTranslucent:
      case ShadingFlat   | MatTexture | TextureTranslucent:
         fnRender = matrenderNTU;
         break;

      // transparent -- only have shadingNone
      case ShadingSmooth | MatTexture | TextureTransparent:
      case ShadingNone   | MatTexture | TextureTransparent:
      case ShadingFlat   | MatTexture | TextureTransparent:
         fnRender = matrenderNTA;
         break;

      default:
         // f000 is apparently used as a place holder for the sky...or something like that.
         AssertFatal(fParams.fFlags==0xf000,
            "TSMaterial::resolveRender: unrecognized material type");
   }
}

void Material::prepareRender(const PackedVertex* _verts, int _first,
                             const Point3F & _scale,const Point3F & _origin,
                             GFXSurface * _srf, PointArray * _pa,
                             const ShapeInstance * _si)
{
   // set up statics
   pVerts = _verts;
   firstVert = _first;
   scale = _scale;
   origin = _origin;
   srf = _srf;
   pa = _pa;
   si = _si;
}

//---------------------------------------------------------------------------

Bool Material::read( StreamIO & sio, int version )
{
   AssertWarn(version != 1, "Old version of material list... reconvert");
   if ( version == 1 ) {
      fParams.fType = DefaultType;
      return sio.read( sizeof( fParams ) - 
      	(MapFilenameMaxV2-MapFilenameMaxV1), (char *)&fParams );
   } else if ( version == 2) {
      fParams.fType = DefaultType;
      fParams.fUseDefaultProps = true;
      return sio.read((sizeof( fParams ) - sizeof(fParams.fType) -
                       sizeof(fParams.fElasticity) - sizeof(fParams.fFriction) - sizeof(fParams.fUseDefaultProps)),
				          (char *)&fParams );
   } else if (version == 3) {
      fParams.fUseDefaultProps = true;
      return sio.read(sizeof(fParams) - sizeof(fParams.fUseDefaultProps),
                      (char *)&fParams );
   } else {
      return sio.read( sizeof( fParams ), (char *)&fParams );
   }
}

Bool Material::write( StreamIO & sio )
{
   return sio.write( sizeof( fParams ) - sizeof(fParams.fUseDefaultProps), (char *)&fParams );
}

RealF
Material::getElasticity() const
{
   if (fParams.fUseDefaultProps != 0) {
      const DefaultMaterialProps* pDefaults = DefaultMaterialProps::getDefaultProps();
      return pDefaults->m_typeProps[fParams.fType].elasticity;
   } else {
      return fParams.fElasticity;
   }
}

RealF
Material::getFriction() const
{
   if (fParams.fUseDefaultProps != 0) {
      const DefaultMaterialProps* pDefaults = DefaultMaterialProps::getDefaultProps();
      return pDefaults->m_typeProps[fParams.fType].friction;
   } else {
      return fParams.fFriction;
   }
}

void
Material::setElasticity(const RealF in_elasticity)
{
   fParams.fElasticity = in_elasticity;
}

void
Material::setFriction(const RealF in_friction)
{
   fParams.fFriction = in_friction;
}

//---------------------------------------------------------------------------

MaterialList::MaterialList()
{
   fLoaded = false;
   fRGBMapped = false;
   setSize( 0, 0 );
}

MaterialList::MaterialList( int nMats, int nDetLevels )
{
   fLoaded = false;
   fRGBMapped = false;
   setSize( nMats, nDetLevels );
}

MaterialList::MaterialList( MaterialList const &ml )
{
   *this = ml;
}

MaterialList::~MaterialList()
{
   if (fLoaded)
      unload();
}

MaterialList &  MaterialList::operator=( const MaterialList & ml )
{
   fLoaded = false;
   fRGBMapped = false;
   setSize( ml.getMaterialsCount(), ml.getDetailsCount() );
   setDetailLevel( ml.getDetailLevel() );

   for( int m = 0; m < fnDetails * fnMaterials; m++ )
      fMaterials[m] = ml.fMaterials[m];
   return (*this);      
}


//---------------------------------------------------------------------------

void MaterialList::setSize( int nMats, int nDetLevels )
{
   // Note: setSize clears all materials!

   AssertFatal( !fLoaded, "TS::MaterialList::setSize: must call unload first" );
   fnMaterials    = nMats;
   fnDetails      = nDetLevels;
   fMaterials.setSize( fnDetails * fnMaterials );
	for (int i = fnMaterials * fnDetails - 1; i >= 0; i--)
		new(&fMaterials[i]) Material;
   fDetailLevel   = 0;
   fOffset        = 0;
}

//---------------------------------------------------------------------------

void MaterialList::lock( ResourceManager & rm, Bool block )
{
   AssertWarn(0, "TS::MaterialList::lock obsolete, use load() instead");
   load(rm, block);
}

void MaterialList::unlock()
{
   AssertWarn(0, "TS::MaterialList::unlock obsolete, you do not need to unlock");
}

 void MaterialList::load( ResourceManager & rm, Bool block )
{
   if ( fLoaded )
      return;

   for( int m = 0; m < fnDetails * fnMaterials; m++ )
         fMaterials[m].load( rm, block );
   fRGBMapped = false;
   fLoaded = true;
}

void MaterialList::unload()
{
   if (fLoaded)
      for( int m = 0; m < fnDetails * fnMaterials; m++ )
         fMaterials[m].unload();
   fLoaded = false;
}

//---------------------------------------------------------------------------
// mapping of Mat RGB's to Palette indices
bool MaterialList::RGB_NeedsRemap(GFXPalette *pal)
{
   if (! pal)
      return false;
      
   if ( !fRGBMapped || (pal->getPal_ID() != curPal_ID))
      return true;

   return false;
}

RGBCache MaterialList::rgbCache;

void MaterialList::mapRGBtoPal(GFXPalette *pal)
{
   if (RGB_NeedsRemap(pal))
      {
      //mapping RGB value to the current palette 
      for( int m = 0; m < fnDetails * fnMaterials; m++ )
         {     
         if ( (fMaterials[m].fParams.fFlags & Material::MatFlags) == Material::MatRGB )
            {
               RGB &rgb = fMaterials[m].fParams.fRGB;
               fMaterials[m].fParams.fIndex = rgbCache.getPalIndex(pal, rgb);
            }
         }
      curPal_ID = pal->getPal_ID();
      fRGBMapped = true;
      }
}


//---------------------------------------------------------------------------

IMPLEMENT_PERSISTENT(TS::MaterialList);

int MaterialList::version()
{
//   return 4; 
   return 3; // be old version so old show tool can work with newly exported shapes
}

#pragma argsused

Persistent::Base::Error MaterialList::read( StreamIO & sio, int version, int user )
{
   AssertFatal( !fLoaded, "TS::MaterialList::read: must call unlock first" );

   sio.read( &fnDetails );
   sio.read( &fnMaterials );
   setSize( fnMaterials, fnDetails ); 

   for( int m = 0; m < fnDetails * fnMaterials; m++ )
      fMaterials[m].read( sio, version );

   return Ok;      
}

#pragma argsused

Persistent::Base::Error MaterialList::write( StreamIO & sio, int version, int user )
{
   sio.write( fnDetails );
   sio.write( fnMaterials );

   for( int m = 0; m < fnDetails * fnMaterials; m++ )
      fMaterials[m].write( sio );

   return Ok;      
}

//---------------------------------------------------------------------------   
// RGB to pal index cache
RGBCache::RGBCache()
{
   cache.reserve(RGBCacheSize);
   curPal_ID = 0;
   nextFree= -1; // only -1 when cache has never been used
}

bool RGBCache::needsRemap(GFXPalette *pal)
{
      if ( (curPal_ID != pal->getPal_ID()) || (nextFree == -1) )
         return true;

      return false;
}

Int32 RGBCache::getPalIndex(GFXPalette *pal, RGB &rgb)
{
   Int32 index;

   // flush the cache if the palette has changed
   if (needsRemap(pal))
      {
      cache.clear();
      nextFree = 0;
      curPal_ID = pal->getPal_ID();
      }

   // check if the rgb is in the cache
   index = -1;
   for (int i = 0; i < cache.size(); i++)
      {
      if (cache[i].rgb == rgb)
         {
         index = cache[i].index;
         if (i == nextFree)
            {
            // clock's alogrithm
            nextFree++;
            if (nextFree >= RGBCacheSize)
               nextFree = 0; 
            }  
         return index;
         }
      }
   
   // if the rgb isn't in the cache, map it to the palette and
   // insert it into the cache.
   if (index == -1)
      {
      index = pal->GetNearestColor( (PALETTEENTRY &)rgb );
      if (cache.size() >= RGBCacheSize)
         {
         cache[nextFree].rgb = rgb;
         cache[nextFree].index = index;
         }
      else
         {
         RGBCacheEntry rgbEntry;
         rgbEntry.rgb = rgb;
         rgbEntry.index = index;
         cache.push_back(rgbEntry);
         }
      nextFree++;
      if (nextFree >= RGBCacheSize)
         nextFree = 0;
      }   

   return index;
}

//------------------------------------------------------------------------------
//--------------------------------------
// Default Properties table...
//--------------------------------------
//
DefaultMaterialProps DefaultMaterialProps::sm_defaultMaterialProps;

DefaultMaterialProps::DefaultMaterialProps()
{
   for (int i = 0; i < NumSurfaceTypes; i++) {
      m_typeProps[i].friction   = 1.0f;
      m_typeProps[i].elasticity = 1.0f;
   }
}

DefaultMaterialProps*
DefaultMaterialProps::getDefaultProps()
{
   return &sm_defaultMaterialProps;
}

   //---------------------------------------------------------------------------
} // namespace TS
 