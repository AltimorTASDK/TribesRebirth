//================================================================
//	
// Initial Author: Rick Overman 
//	
// Description 
//	
// $Workfile:   g_surfac.h  $
// $Revision:   1.6  $
// $Author  $
// $Modtime $
//
//================================================================

#ifndef _G_SURFACE_H_
#define _G_SURFACE_H_

#include <base.h>
#include "d_defs.h"
#include "d_funcs.h"
#include "g_raster.h"
#include <windows.h>
#include <windowsx.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class    GFXPalette;
class    GFXZBuffer;
class    GFXFont;
class    GFXBitmap;

struct   GFXEdgeList;
struct   RGBFormat;
struct   FunctionTable;
struct   GFXLightMap;

class GFXSurface
{
public:
   static GFXRasterList  rasterList;      //rasterization buffer

	FunctionTable* functionTable;
   DWORD          flags;         //clipping, expansion, etc
   RectI          clipRect;      //clipping region for rect clip fn's
   RectI          lastRect;      //
   float          gamma;         // gamma correction value
	GFXPalette 		*pPalette;
	BYTE	         *pSurfaceBits;	//ONLY valid when locked
	Int32			   surfaceStride;	//ONLY valid when locked
	Int32			   surfaceWidth;
	Int32			   surfaceHeight;
	Int32          surfaceBitDepth; // number of bits per pixel
	GFXSurface 		*next;			//linked flipping surfaces
	Int32          pages;			//number of flipping surfaces
	BOOL   			isPrimary;
	GFX_SURFACE_TYPE surfaceType;
   Int32          popCount;      // number of times to populate this
                                 // front page to the back surfaces
   Int32          lockCount;
   Int32          dcLockCount;
   HDC            hDC;

   bool           m_draw3DBegun;
   
  protected:
	GFXSurface* findPrimary();
   virtual Bool _setPalette(GFXPalette *lpPalette, Int32 in_start, Int32 in_count, Bool in_rsvPal = true) = 0;
   virtual void _setGamma()=0;
	virtual void _lock(const GFXLockMode in_lockMode)=0;
	virtual void _unlock()=0;
   
public:
	static Bool createRasterList(Int32 lEntries);
	static void disposeRasterList();
	static GFXRasterList* getRasterList();

	GFXSurface();
	virtual ~GFXSurface();

   virtual bool supportsDoubleLock() const;
   void lock(const GFXLockMode in_lockMode = GFX_NORMALLOCK);
   void unlock();
	inline FunctionTable* getFunctionTable() { return functionTable; }
	inline void setFunctionTable(FunctionTable *ft) { functionTable = ft; }
	virtual Bool getRGBFormat(RGBFormat *rgb) { return FALSE; }
	virtual void setHWND( HWND hWnd ) {};
 	virtual void flip() = 0;
	virtual void update(const RectI *rect = NULL);
   virtual void easyFlip();
   
	virtual void drawSurface(GFXSurface *lpSourceSurface, const RectI *in_subRegion, const Point2I *in_at) = 0;
   virtual DWORD getCaps();
   virtual void drawSurfaceToBitmap(GFXBitmap *bmp);

   bool testCap(int cap);
   void setPopulateBackBuffers(BOOL popFlag);

   float getGamma();
   void  animateGamma( float in_gamma );
   void  setGamma( float in_gamma );
   Bool  setPalette(GFXPalette *lpPalette, Int32 in_start, Int32 in_count, Bool in_rsvPal = true);
	Bool  setPalette(GFXPalette *lpPalette, Bool in_rsvPal = true);
	BOOL  isLocked();
   BOOL  isDCLocked();

   //------------------------------ Data Access Members
	GFX_SURFACE_TYPE getType();
	BYTE* getAddress();
	BYTE* getAddress(const Point2I &in_pt);
	BYTE* getAddress(Int32 x, Int32 y);
	Int32 getStride();
	Int32 getWidth();
	Int32 getHeight();
   Int32 getBitDepth();
   Int32 getPageCount() { return pages; }
	
	HDC    getDC();
	RectI* getClipRect();
   void   setClipRect( const RectI *in_rect );
	RectI* getLastRect();
   void   setLastRect( const RectI *in_rect );

	GFXPalette* getPalette();

   DWORD getFlags();
   void  setFlags(DWORD in_flags);
   GFX_DMFlag getFlag(GFX_DMFlag in_flag);
   void setFlag(GFX_DMFlag in_flag);
   void clearFlag(GFX_DMFlag in_flag);
   void toggleFlag(GFX_DMFlag in_flag);
   virtual void getBitmap(GFXBitmap *bmp);
   virtual void setSurfaceAlphaBlend(ColorF *color, float alpha);
   
   void getClientRect(RectI *io_rect);


   //------------------------------ Drawing Members
   void clear(DWORD in_color);

   void drawPoint2d  (const Point2I *in_pt, DWORD in_color);
   void drawPoint3d  (const Point2I *in_pt, float in_w, DWORD in_color);

   void drawLine2d   (const Point2I *in_st, const Point2I *in_en, DWORD in_color);

   void drawRect2d   (const RectI *in_rect, DWORD in_color);
   void drawRect2d_f (const RectI *in_rect, DWORD in_color);
   void drawRect3d_f (const RectI *in_rect, float in_w, DWORD in_color);

   void drawBitmap2d (const GFXBitmap *in_pBM, const Point2I *in_at, GFXFlipFlag in_flip=GFX_FLIP_NONE);
   void drawBitmap2d (const GFXBitmap *in_pBM, const RectI *in_subRegion, const Point2I *in_at, GFXFlipFlag in_flip=GFX_FLIP_NONE);
   void drawBitmap2d (const GFXBitmap *in_pBM, const Point2I *in_at, const Point2I *in_stretch, GFXFlipFlag in_flip=GFX_FLIP_NONE);
   void drawBitmap2d (const GFXBitmap *in_pBM, const Point2I *in_at, const Point2I *in_stretch, Int32 in_angle);
   
   Int32 drawText_p(GFXFont *in_font, const Point2I *in_atPt, const void *in_string);
   Int32 drawText_r(GFXFont *io_font, const RectI *in_atRec,  const void *in_string);

   void addVertex(const Point3F *, DWORD);
   void addVertex(const Point3F *, const Point2F *, DWORD);
   void addVertex(const Point3F *, const GFXColorInfoF *, DWORD);
   void addVertex(const Point3F *, const Point2F *, const GFXColorInfoF *, DWORD);
   void emitPoly();
   void draw3DBegin();
   void draw3DEnd();

   void setShadeSource(GFXShadeSource ss);
   void setHazeSource(GFXHazeSource hs);
   GFXHazeSource getHazeSource();
   void setAlphaSource(GFXAlphaSource as);
   void setFillColor(const ColorF *c);
   void setFillColor(Int32 index, DWORD paletteIndex = 0xFFFFFFFF);
   void setHazeColor(const ColorF *c);
   void setHazeColor(Int32 index);
   void setConstantShade(const ColorF *c);
   void setConstantShade(float c);
   void setConstantHaze(float h);
   float getConstantHaze();
   void setConstantAlpha(float h);
   void setTransparency(Bool transFlag);
   void setTextureMap(const GFXBitmap *pTextureMap);
   void setTextureWrap(Bool wrapEnable);
   void setFillMode(GFXFillMode fm);
   void setTexturePerspective(Bool perspTex);
	void setZTest(int zTestEnable);
   void setZMode(bool wBuffer); // true by default (false means zbuffer for ortho cam)
	void clearZBuffer();

	void registerTexture(GFXTextureHandle,
		int sizeX, int sizeY, int offsetX, int offsetY, int lightScale,
		GFXLightMap *map, const GFXBitmap *tex, int mipLevel);
	void registerTexture(GFXTextureHandle,
		GFXCacheCallback cb, int csizeX, int csizeY,
      int lightScale, GFXLightMap *map);
	Bool setTextureHandle(GFXTextureHandle);
	GFXLightMap * allocateLightMap(int pixelSize);
	GFXLightMap * handleGetLightMap();
	void handleSetLightMap(int lightScale, GFXLightMap *map);
	void handleSetTextureMap(const GFXBitmap *);
	void handleSetMipLevel(int mipLevel);
	void handleSetTextureSize(int newSize);
	void flushTextureCache();
   void flushTexture(const GFXBitmap* in_pTexture, const bool in_reload = false);

   void setClipPlanes(const float in_nearDist, const float in_farDist);
};


inline DWORD GFXSurface::getFlags() {  return flags; }
inline void GFXSurface::setFlags(DWORD in_flags) { flags = in_flags; }
inline GFX_DMFlag GFXSurface::getFlag(GFX_DMFlag in_flag) { return (flags & in_flag); }
inline void GFXSurface::setFlag(GFX_DMFlag in_flag) { flags |= in_flag; }
inline void GFXSurface::clearFlag(GFX_DMFlag in_flag) { flags &= (~in_flag); }
inline void GFXSurface::toggleFlag(GFX_DMFlag in_flag) { flags ^= in_flag; }

inline bool GFXSurface::testCap(int cap)
{
   return ((1 << cap) & getCaps()) != 0;
}

inline HDC GFXSurface::getDC()
{
   return hDC;
}

inline float GFXSurface::getGamma()
{
   return gamma;
}   

inline void GFXSurface::lock(const GFXLockMode in_lockMode)
{
   if(!(lockCount++)) {
      _lock(in_lockMode);
   }
}

inline void GFXSurface::unlock()
{
   AssertWarn(lockCount, "GFXSurface::unlock: cannot unlock an unlocked surface.");
   if(!(--lockCount))
      _unlock();
}

inline void GFXSurface::setPopulateBackBuffers(BOOL popFlag)
{
   if(popFlag)
      popCount = pages - 1;
   else
      popCount = 0;
}

inline BOOL GFXSurface::isDCLocked()
{
   return dcLockCount != 0;
}

inline BOOL GFXSurface::isLocked() 
{ 
	return lockCount != 0; 
}

inline BYTE* GFXSurface::getAddress()
{
	AssertFatal(pSurfaceBits, "GFXSurface::getAddress: Surface must be locked first.");	
	return ( pSurfaceBits );
}

inline BYTE* GFXSurface::getAddress(Int32 x, Int32 y)
{
	AssertFatal(pSurfaceBits, "GFXSurface::getAddress: Surface must be locked first.");	
	return ( pSurfaceBits + (y*surfaceStride) + x * (surfaceBitDepth >> 3) );
}

inline BYTE* GFXSurface::getAddress(const Point2I &in_pt)
{
	return getAddress( in_pt.x, in_pt.y );
}

inline Int32 GFXSurface::getStride()
{
	AssertFatal(isLocked(), "GFXSurface::getStride: Surface must be locked first.");	
	return ( surfaceStride );
}

inline Int32 GFXSurface::getBitDepth()
{
   return ( surfaceBitDepth );
}

inline Int32 GFXSurface::getWidth()
{
	return ( surfaceWidth );
}
inline Int32 GFXSurface::getHeight()
{
	return ( surfaceHeight );
}

inline Bool GFXSurface::setPalette(GFXPalette *lpPalette, Int32 in_start, Int32 in_count, Bool in_rsvPal)
{
	return _setPalette(lpPalette, in_start, in_count, in_rsvPal);
}

inline Bool GFXSurface::setPalette(GFXPalette *lpPalette, Bool in_rsvPal)
{
	return _setPalette(lpPalette, 0, 256, in_rsvPal);
}

inline void GFXSurface::animateGamma(float in_gamma)
{
   float save_gamma = gamma;
   gamma = in_gamma;
   _setGamma();
   gamma = save_gamma;
}

inline void GFXSurface::setGamma( float in_gamma )
{
   gamma = in_gamma;
   _setGamma();
}   

inline GFXPalette* GFXSurface::getPalette() 
{ 
	return pPalette; 
}

inline GFXRasterList* GFXSurface::getRasterList()
{
	return ( &rasterList );
}

inline GFX_SURFACE_TYPE GFXSurface::getType()
{
	return ( surfaceType );
}


//------------------------------------------------------------------------
//------------------------------ INLINES ---------------------------------
//------------------------------------------------------------------------

inline void GFXSurface::getClientRect(RectI *io_rect)
{
    io_rect->upperL(0,0);
    io_rect->lowerR(surfaceWidth-1, surfaceHeight-1);
}

inline RectI* GFXSurface::getLastRect( )
{
   return ( &lastRect );
}

inline void GFXSurface::setLastRect( const RectI *in_rect )
{
	lastRect = *in_rect;
}


inline RectI* GFXSurface::getClipRect( )
{
   return ( &clipRect );
}

inline void GFXSurface::setClipRect( const RectI *in_rect )
{
	clipRect = *in_rect;
}


//------------------------------------------------------------------------
// CLEAR
//
inline void GFXSurface::clear( DWORD in_color )
{
   getFunctionTable()->fnClear(this, in_color);
}

inline void GFXSurface::draw3DBegin()
{
   AssertFatal(m_draw3DBegun == false, "Error, already drawing 3d");
   m_draw3DBegun = true;
	getFunctionTable()->fnDraw3DBegin(this);
}

inline void GFXSurface::draw3DEnd()
{
   AssertFatal(m_draw3DBegun == true, "Error, not drawing 3d");
   m_draw3DBegun = false;
	getFunctionTable()->fnDraw3DEnd(this);
}

//------------------------------------------------------------------------
// POINT DRAW
//
inline void GFXSurface::drawPoint2d( const Point2I *in_pt, DWORD in_color )
{
   getFunctionTable()->fnDrawPoint(this, in_pt, 0, in_color);
}

inline void GFXSurface::drawPoint3d( const Point2I *in_pt, float in_w, DWORD in_color )
{
   getFunctionTable()->fnDrawPoint(this, in_pt, in_w, in_color);
}


//------------------------------------------------------------------------
// LINE DRAW 2D
//
inline void GFXSurface::drawLine2d( const Point2I *in_st, const Point2I *in_en, DWORD in_color )
{
   getFunctionTable()->fnDrawLine2d( this, in_st, in_en, in_color );
}


//------------------------------------------------------------------------
// CIRCLE DRAW
//

//------------------------------------------------------------------------
// RECTANGLE DRAW 2D
//
inline void GFXSurface::drawRect2d(const RectI *in_rect, DWORD in_color)
{
   getFunctionTable()->fnDrawRect2d(this, in_rect, in_color);
}
inline void GFXSurface::drawRect2d_f(const RectI *in_rect, DWORD in_color)
{
   getFunctionTable()->fnDrawRect_f(this, in_rect, 1.0, in_color);
}

inline void GFXSurface::drawRect3d_f(const RectI *in_rect, float in_w, DWORD in_color)
{
   getFunctionTable()->fnDrawRect_f(this, in_rect, in_w, in_color);
}

//------------------------------------------------------------------------
// BITMAP DRAW 2D
//
inline void GFXSurface::drawBitmap2d(const GFXBitmap *in_pBM, const Point2I *in_at, GFXFlipFlag in_flip)
{
   getFunctionTable()->fnDrawBitmap2d_f(this, in_pBM, in_at, in_flip );
}

inline void GFXSurface::drawBitmap2d(const GFXBitmap *in_pBM, const RectI *in_subRegion, const Point2I *in_at, GFXFlipFlag in_flip)
{
   getFunctionTable()->fnDrawBitmap2d_rf(this, in_pBM, in_subRegion, in_at, in_flip );
}

inline void GFXSurface::drawBitmap2d(const GFXBitmap *in_pBM, const Point2I *in_at, const Point2I *in_stretch, GFXFlipFlag in_flip)
{
   getFunctionTable()->fnDrawBitmap2d_sf(this, in_pBM, in_at, in_stretch, in_flip );
}

inline void GFXSurface::drawBitmap2d(const GFXBitmap *in_pBM, const Point2I *in_at, const Point2I *in_stretch, Int32 in_angle)
{
   getFunctionTable()->fnDrawBitmap2d_af(this, in_pBM, in_at, in_stretch, in_angle);
}


//------------------------------------------------------------------------
// TEXT FUNCTIONS
//
inline Int32 GFXSurface::drawText_p( GFXFont *in_font, const Point2I *in_atPt, const void *in_string )
{
   return getFunctionTable()->fnDrawText_p( this, in_font, in_atPt, in_string );
}

inline Int32 GFXSurface::drawText_r( GFXFont *io_font, const RectI *in_atRec, const void *in_string )
{
   return getFunctionTable()->fnDrawText_r( this, io_font, in_atRec, in_string );
}

//------------------------------------------------------------------------
// STATE POLY FUNCTIONS
//

inline void GFXSurface::setZTest(int zTestEnable)
{
	getFunctionTable()->fnSetZTest(this, zTestEnable);
}

inline void GFXSurface::setZMode(bool wBuffer)
{
	getFunctionTable()->fnSetZMode(this, wBuffer);
}

inline void GFXSurface::clearZBuffer()
{
	getFunctionTable()->fnClearZBuffer(this);
}

inline void GFXSurface::addVertex(const Point3F *v, DWORD key=0)
{
	getFunctionTable()->fnAddVertexV(this, v, key);
}
                                                     
inline void GFXSurface::addVertex(const Point3F *v, const Point2F *t, DWORD key=0)
{
	getFunctionTable()->fnAddVertexVT(this, v, t, key);
}

inline void GFXSurface::addVertex(const Point3F *v, const GFXColorInfoF *c, DWORD key=0)
{
	getFunctionTable()->fnAddVertexVC(this, v, c, key);
}

inline void GFXSurface::addVertex(const Point3F *v, const Point2F *t, const GFXColorInfoF *c, DWORD key=0)
{
	getFunctionTable()->fnAddVertexVTC(this, v, t, c, key);
}

inline void GFXSurface::emitPoly()
{
	getFunctionTable()->fnEmitPoly(this);
}

inline void GFXSurface::setShadeSource(GFXShadeSource ss)
{
	getFunctionTable()->fnSetShadeSource(this, ss);
}

inline void GFXSurface::setHazeSource(GFXHazeSource hs)
{
	getFunctionTable()->fnSetHazeSource(this, hs);
}

inline GFXHazeSource GFXSurface::getHazeSource()
{
   return getFunctionTable()->fnGetHazeSource(this);
}

inline float GFXSurface::getConstantHaze()
{
   return getFunctionTable()->fnGetConstantHaze(this);
}

inline void GFXSurface::setAlphaSource(GFXAlphaSource as)
{
	getFunctionTable()->fnSetAlphaSource(this, as);
}

inline void GFXSurface::setFillColor(const ColorF *c)
{
	getFunctionTable()->fnSetFillColorCF(this, c);
}

inline void GFXSurface::setFillColor(Int32 index, DWORD paletteIndex)
{
	getFunctionTable()->fnSetFillColorI(this, index, paletteIndex);
}

inline void GFXSurface::setHazeColor(const ColorF *c)
{
	getFunctionTable()->fnSetHazeColorCF(this, c);
}

inline void GFXSurface::setHazeColor(Int32 index)
{
	getFunctionTable()->fnSetHazeColorI(this, index);
}

inline void GFXSurface::setConstantShade(const ColorF *c)
{
	getFunctionTable()->fnSetConstantShadeCF(this, c);
}

inline void GFXSurface::setConstantShade(float c)
{
	getFunctionTable()->fnSetConstantShadeF(this, c);
}

inline void GFXSurface::setConstantHaze(float h)
{
	getFunctionTable()->fnSetConstantHaze(this, h);
}

inline void GFXSurface::setConstantAlpha(float h)
{
	getFunctionTable()->fnSetConstantAlpha(this, h);
}

inline void GFXSurface::setTransparency(Bool transFlag)
{
	getFunctionTable()->fnSetTransparency(this, transFlag);
}

inline void GFXSurface::setTextureMap(const GFXBitmap *pTextureMap)
{
	getFunctionTable()->fnSetTextureMap(this, pTextureMap);
}

inline void GFXSurface::setTextureWrap(Bool wrapEnable)
{
	getFunctionTable()->fnSetTextureWrap(this, wrapEnable);
}

inline void GFXSurface::setFillMode(GFXFillMode fm)
{
	getFunctionTable()->fnSetFillMode(this, fm);
}

inline void GFXSurface::setTexturePerspective(Bool perspTex)
{
	getFunctionTable()->fnSetTexturePerspective(this, perspTex);
}

inline void GFXSurface::registerTexture(GFXTextureHandle th,
		int sizeX, int sizeY, int offsetX, int offsetY, int lightScale,
		GFXLightMap *map, const GFXBitmap *tex, int mipLevel)
{
	getFunctionTable()->fnRegisterTexture(this, th, sizeX, sizeY, 
		offsetX, offsetY, lightScale, map, tex, mipLevel);
}

inline void GFXSurface::registerTexture(GFXTextureHandle tex,
		GFXCacheCallback cb, int csizeX, 
		int csizeY, int lightScale, GFXLightMap *map)
{
	getFunctionTable()->fnRegisterTextureCB(this, tex, cb, csizeX,
		csizeY, lightScale, map);
}

inline Bool GFXSurface::setTextureHandle(GFXTextureHandle tex)
{
	return getFunctionTable()->fnSetTextureHandle(this, tex);
}

inline GFXLightMap * GFXSurface::allocateLightMap(int pixelSize)
{
	return getFunctionTable()->fnAllocateLightMap(this, pixelSize);
}

inline GFXLightMap * GFXSurface::handleGetLightMap()
{
	return getFunctionTable()->fnHandleGetLightMap(this);
}

inline void GFXSurface::handleSetLightMap(int lightScale, GFXLightMap *map)
{
	getFunctionTable()->fnHandleSetLightMap(this, lightScale, map);
}

inline void GFXSurface::handleSetTextureMap(const GFXBitmap *bmp)
{
	getFunctionTable()->fnHandleSetTextureMap(this, bmp);
}

inline void GFXSurface::handleSetMipLevel(int mipLevel)
{
	getFunctionTable()->fnHandleSetMipLevel(this, mipLevel);
}

inline void GFXSurface::handleSetTextureSize(int newSize)
{
	getFunctionTable()->fnHandleSetTextureSize(this, newSize);
}

inline void GFXSurface::flushTextureCache()
{
	getFunctionTable()->fnFlushTextureCache(this);
}

inline void 
GFXSurface::flushTexture(const GFXBitmap* in_pTexture, const bool in_reload)
{
   getFunctionTable()->fnFlushTexture(this, in_pTexture, in_reload);
}

inline void
GFXSurface::setClipPlanes(const float in_nearDist,
                          const float in_farDist)
{
   getFunctionTable()->fnSetClipPlanes(this, in_nearDist, in_farDist);
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_G_SURFACE_H_
