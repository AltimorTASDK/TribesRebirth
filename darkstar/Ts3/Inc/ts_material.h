//---------------------------------------------------------------------------

//	$Workfile:   ts_mesh.h  $
//	$Revision:   2.8  $
// $Version$
//	$Date:   15 Sep 1995 10:27:12  $
//	$Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
//	

//---------------------------------------------------------------------------

#ifndef _TS_MATERIAL_H_
#define _TS_MATERIAL_H_

#include <ml.h>
#include <streams.h>
#include <persist.h>

class GFXPalette;
class GFXBitmap;
class GFXSurface;

namespace TS
{

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class ShapeInstance;
class PointArray;
class PackedVertex;
class Material;

typedef void (*MatRenderer)(const Material *, const char * pFace);

//---------------------------------------------------------------------------
// Our own RGB type:

// We need something which has the same size as a UInt32.

class RGB
{
  public:
   UInt8    fRed;
   UInt8    fGreen;
   UInt8    fBlue;
   UInt8    fFlags;

   int operator==( RGB const & c )
   {
      return (*((Int32*)this)) == (*((Int32*)&c));
   }
};

//---------------------------------------------------------------------------
// Cache to map betwen RGB values and palette indices:
class RGBCache 
{
  private:
   enum RGBCacheConst {
      RGBCacheSize = 30,
   };

   struct RGBCacheEntry {
      RGB      rgb;     // rgb value
      Int32    index;   // index into palette
   };

   Vector<RGBCacheEntry>      cache;
   Int32                      curPal_ID;  // the palette the cache is currently mapped to.

   Int32                      nextFree;   // index into the next free entry in the cache
                                          // using clock algorithm

  public:
   RGBCache();
   bool               needsRemap(GFXPalette *pal);   
   Int32              getPalIndex(GFXPalette *pal, RGB &rgb);
};

//---------------------------------------------------------------------------
// Material:

class Material
{

  public:
   enum Constants
      {
      MapFilenameMaxV1 = 16,
      MapFilenameMaxV2 = 32,
      MapFilenameMax = MapFilenameMaxV2,
      };

   //----------------------------------------------------
   // I/O Methods:
  public:
   Bool  read( StreamIO &,  int version );
   Bool  write( StreamIO &);
   //
   //----------------------------------------------------

  public:
  // public statics used by material renderers ... not for general use
  static Point3F scale;
  static Point3F origin;
  static int firstVert;
  static const PackedVertex * pVerts;
  static GFXSurface * srf;
  static PointArray * pa;
  static const ShapeInstance * si;
  
  static void prepareRender(const PackedVertex*,int,const Point3F &,const Point3F &, 
                            GFXSurface *, PointArray*, const ShapeInstance *);

  void resolveRender();

  public:
   enum MatType {
      MatFlags          = 0x0f,
      MatNull           = 0x00,
      MatPalette        = 0x01,
      MatRGB            = 0x02,
      MatTexture        = 0x03,
   };
   enum ShadingType {
      ShadingFlags      = 0xf00,
      ShadingNone       = 0x100,
      ShadingFlat       = 0x200,
      ShadingSmooth     = 0x400,
   };

   enum TextureType {
      TextureFlags       = 0xf000,
      TextureTransparent = 0x1000,
      TextureTranslucent = 0x2000,
   };

   enum SurfaceType {
      DefaultType       = 0x0,

      ConcreteType      = 0x1,
      CarpetType        = 0x2,
      MetalType         = 0x3,
      GlassType         = 0x4,
      PlasticType       = 0x5,
      WoodType          = 0x6,
      MarbleType        = 0x7,
      SnowType          = 0x8,
      IceType           = 0x9,
      SandType          = 0xA,
      MudType           = 0xB,
      StoneType         = 0xC,
      SoftEarthType     = 0xD,
      PackedEarthType   = 0xE,
   };


  private:
   // run-time only data members:
   Resource<GFXBitmap> fTextureMap;
   
   MatRenderer fnRender;

  public:

   // persistent data members:
   class Params
   {
     public:
      Int32       fFlags;
      RealF       fAlpha;
      Int32       fIndex;        // for palette types
      RGB         fRGB;          // for RGB types
      char        fMapFile[MapFilenameMaxV2];
		// Stuffed here at the end for backwards
		// compatibility on fileIO
		Int32			fType;

		RealF			fElasticity;         // use get/set Elasticity/Friction() on material
		RealF			fFriction;
      UInt32      fUseDefaultProps;    // actually just a bool, but read routines
                                       // require this to be the same under all
                                       // compilers...
   };

   Params          fParams;

   Material &      operator=( const Material & mat );
   int             operator==( const Material & mat );

   void            load( ResourceManager &rm, Bool block );
   void            unload();
   Bool            isReady() const;
   
   const GFXBitmap *  getTextureMap() const;

   void render(const char * pFace) const { fnRender(this,pFace); }

   Material();
   Material( const Material & );
   ~Material();

   RealF getElasticity() const;
   RealF getFriction() const;
   void  setElasticity(RealF);
   void  setFriction(RealF);
};


//---------------------------------------------------------------------------

class MaterialList : public Persistent::VersionedBase
{
   //----------------------------------------------------
   // Persistent Methods:
  public:
   DECLARE_PERSISTENT(TS::MaterialList);
   int                     version();
   Persistent::Base::Error read( StreamIO &, int version, int user = 0 );
   Persistent::Base::Error write( StreamIO &, int version, int user = 0 );
  private:
   //
   //----------------------------------------------------

   //----------------------------------------------------
   // instance data:
  private:
   // run-time only data members:
   bool	                     fLoaded;
   int                        fOffset;
   int                        fDetailLevel;

   // persistent data members:
  private:
   Vector<Material>           fMaterials;
   Int32                      fnDetails;
   Int32                      fnMaterials;

   // mapping of RGB to pal index:
  private:
   Int32                      curPal_ID;  // the palette the MatRGB's are currently mapped to.
   bool                       fRGBMapped; // has Material::fParams::fIndex been set for MatRGB types
   //
   //----------------------------------------------------

   //----------------------------------------------------
   // RGB to Palette index cache data:
  private:
   static RGBCache            rgbCache;  
   //
   //----------------------------------------------------

   //----------------------------------------------------
   // instance methods:
  public:
   int                getDetailsCount() const;
   int                getMaterialsCount() const;

   int                getDetailLevel() const;
   void               setDetailLevel( int detLevel );

   MaterialList &     operator=( const MaterialList & ml );

   const Material &   operator[]( int index ) const;
   const Material &   getMaterial( int index ) const;
   const Material &   getMaterial( int detail, int index ) const;
   Material &         operator[]( int index );
   Material &         getMaterial( int index );
   void               setMaterial( int index, Material const &mat );

   void               setSize( int nMats, int nDetLevels );

   void               lock( ResourceManager & rm, Bool block );
   void               load( ResourceManager & rm, Bool block );
   void               unlock();
   void               unload();
   bool               isLocked() const;
   bool               isLoaded() const;

   bool               RGB_NeedsRemap(GFXPalette *pal);
   void               mapRGBtoPal(GFXPalette *pal);

   MaterialList();
   MaterialList( int nMats, int nDetLevels = 1 );
   MaterialList( const MaterialList & );
   ~MaterialList();

   //
   //----------------------------------------------------
};


class DefaultMaterialProps
{
   static DefaultMaterialProps sm_defaultMaterialProps;

  public:
   enum {
      NumSurfaceTypes = 0xF
   };

   struct MaterialProps {
      RealF friction;
      RealF elasticity;
   };

   MaterialProps m_typeProps[NumSurfaceTypes];

  private:
   DefaultMaterialProps();
  public:
   ~DefaultMaterialProps() {}
   static DefaultMaterialProps* getDefaultProps();
};


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

//--------------------------------------------------------------------------

inline const GFXBitmap*
Material::getTextureMap() const
{
   AssertFatal( (GFXBitmap const *)fTextureMap != NULL,
                avar("TS::Material::getTextureMap: no bitmap! (%s)", fParams.fMapFile ));
   return (GFXBitmap const *)fTextureMap;
}

//--------------------------------------------------------------------------

inline bool
MaterialList::isLocked() const
{
   AssertWarn(0, "TS::MaterialList::isLocked obsolete, use isLoaded() instead");
	return isLoaded();
}

inline bool
MaterialList::isLoaded() const
{
	return fLoaded;
}

inline int
MaterialList::getMaterialsCount() const
{
   return fnMaterials;
}

inline int
MaterialList::getDetailsCount() const
{
   return fnDetails;
}

inline int
MaterialList::getDetailLevel() const
{
   return fDetailLevel;
}

inline void
MaterialList::setDetailLevel( int detLevel )
{
   AssertFatal( detLevel >= 0 && detLevel < fnDetails, 
      "TS::MaterialList::setDetailLevel: Detail level out of range." );
   fDetailLevel = detLevel;
   fOffset = fDetailLevel * fnMaterials;
}

inline const Material&
MaterialList::getMaterial( int detail, int index ) const
{
   AssertFatal( detail >= 0 && detail < fnDetails, 
      "TS::MaterialList::getMaterial: Material index out of range." );
   AssertFatal( index >= 0 && index < fnMaterials, 
      "TS::MaterialList::getMaterial: Material index out of range." );
   
   return fMaterials[detail * fnMaterials + index];
}

inline const Material&
MaterialList::getMaterial( int index ) const
{
   AssertFatal( index >= 0 && index < fnMaterials, 
      "TS::MaterialList::getMaterial: Material index out of range." );
   
   return fMaterials[fOffset + index];
}

inline Material &
MaterialList::getMaterial( int index )
{
   AssertFatal( index >= 0 && index < fnMaterials, 
      "TS::MaterialList::getMaterial: Material index out of range." );
   
   return fMaterials[fOffset + index];
}

inline const Material&
MaterialList::operator[]( int index ) const
{
   return getMaterial( index );
}

inline Material& MaterialList::operator[]( int index )
{
   return getMaterial( index );
}

inline void MaterialList::setMaterial( int index, Material const &mat )
{
   AssertFatal( index >= 0 && index < fnMaterials, 
      "TS::MaterialList::getMaterial: Material index out of range." );
   
   fMaterials[fOffset + index] = mat;

   // remap if new material is an RGB
   if (mat.fParams.fFlags & Material::MatRGB)
      fRGBMapped = false;
}

} // namespace TS

//

typedef TS::Material     TSMaterial;
typedef TS::MaterialList TSMaterialList;


#endif // _TS_MATERIAL_H_
