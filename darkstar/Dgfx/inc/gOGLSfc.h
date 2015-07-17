//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GGLSFC_H_
#define _GGLSFC_H_

//Includes
#include <windows.h>
#include <gl\gl.h>

#include "g_surfac.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

namespace OpenGL {

struct DGLVertex4F;
struct DGLColor4F;
struct DGLTexCoord4F;
struct DGLHazeCoordF;

//class TriCache;
class HandleCache;

//-------------------------------------- Note that many members of this class are marked
//                                        as protected to guard against the necessity of
//                                        deriving classes to support specific
//                                        implementations
//
class Surface : public GFXSurface {
   // Internal interfaces...
  public:
   class TextureCache;
   friend class TextureCache;

   // WGL variables
  private:
   HWND  m_hWnd;
   HDC   m_hDC;

   HGLRC m_hGLRC;
   bool  m_glrcMadeCurrent;

   int   m_oldPixelFormat;
   PIXELFORMATDESCRIPTOR m_oldPixelFormatDescriptor;

   RECT  m_oldWindowRect;
   bool  m_windowStyleChanged;

   char* m_pVendorString;
   char* m_pRendererString;
   char* m_pVersionString;
   char* m_pExtensionsString;

   // These variables are for the use of functions in gOGLFn.cpp ONLY
  public:
   // dGFX renderstate information
   //
   ColorF m_fillColor;
   int    m_fillColorIndex;
   DWORD  m_currPaletteIndex;
   ColorF m_hazeColor;

   ColorF m_constantShadeColor;
   float  m_constantHaze;
   float  m_constantAlpha;

   bool   m_transparencyEnabled;
   bool   m_perspectiveCorrect;

   float  m_nearClipPlane;
   float  m_farClipPlane;

   GFXFillMode    m_fillMode;
   GFXShadeSource m_shadeSource;
   GFXHazeSource  m_hazeSource;
   GFXAlphaSource m_alphaSource;
   GFXZBufferMode m_zTest;

  protected:
   static int sm_maxNumVertices;
   int        m_currVertexIndex;

  protected:
   float  m_alphaBlend;
   ColorF m_alphaColor;
   float  m_flareBlend;
   ColorF m_flareColor;
   void setSurfaceAlphaBlend(ColorF *color, float alpha);

  public:
   void setSurfaceFlareBlend(ColorF* color, float alpha);

   DGLVertex4F*   m_pVertexArray;
   DGLTexCoord4F* m_pTexCoord0Array;
   DGLHazeCoordF* m_pHazeStoreArray;
   DGLColor4F*    m_pColorArray;

   bool m_colorEnabled;
   bool m_disableBilinear;

   bool m_in3dmode;

  protected:
   TextureCache*  m_pTextureCache;
   HandleCache*   m_pHandleCache;
//   TriCache*      m_pTriCache;

  public:
   DGLVertex4F*   getCurrentVertex();
   DGLColor4F*    getCurrentColor();
   DGLTexCoord4F* getCurrentTexCoord0();
   DGLHazeCoordF* getCurrentHazeCoord();
   void incCurrVertexIndex();
   int  getCurrVertexIndex();
   void clearCurrVertexIndex();

   // Protected pure virtuals from GFXSurface
  protected:
   Bool _setPalette(GFXPalette* lpPalette,
                    Int32       in_start,
                    Int32       in_count,
                    Bool        in_rsvPal);
   void _setGamma();
	void _lock(const GFXLockMode in_lockMode);
	void _unlock();

   // Prevent direct creation of a Surface
  protected:
   Surface();
   bool choosePixelFormat(int&                   out_rPixelFormat,
                          PIXELFORMATDESCRIPTOR& out_rPFD,
                          HWND                   io_hWnd,
                          HDC                    io_hDC);
  public:
   ~Surface();
 
   // Only OGLDevice should call these...
  public:
   static bool init(HWND);
   static void shutdown();

   // Public pure virtuals from GFXSurface
  public:
 	void flip();
	void drawSurface(GFXSurface*    lpSourceSurface,
                    const RectI*   in_subRegion,
	                 const Point2I* in_at);


  public:
   void setFogDists(float, float);

   // Surface Creation functions
  public:
   static Surface* create(HWND        io_clientWnd,
                          const Int32 in_width,
                          const Int32 in_height);
   // misc
  public:
   TextureCache* getTextureCache() { return m_pTextureCache; }
   HandleCache*  getHandleCache()  { return m_pHandleCache;  }
//   TriCache*     getTriCache()     { return m_pTriCache;     }

   void processMessage(int argc, const char* argv[]);
	void drawSurfaceToBitmap(GFXBitmap *bmp);

   DWORD getCaps();

   // user? preference functions...
  public:
   static GLenum getPrefMinFilter(const bool in_mipMap);
   static GLenum getPrefMagFilter();
   static int    getMaxTextureDetailLevel();
};


//-------------------------------------- Vertex array structs and helper inlines...
struct DGLVertex4F {
   GLfloat x;
   GLfloat y;
   GLfloat z;
   GLfloat w;
};

struct DGLColor4F {
   GLfloat r;
   GLfloat g;
   GLfloat b;
   GLfloat a;
};

struct DGLTexCoord4F {
   GLfloat s;
   GLfloat t;
   GLfloat r;
   GLfloat q;
};

struct DGLHazeCoordF {
   GLfloat h;
};

inline DGLVertex4F*
Surface::getCurrentVertex()
{
   return &m_pVertexArray[m_currVertexIndex];
}

inline DGLColor4F*
Surface::getCurrentColor()
{
   return &m_pColorArray[m_currVertexIndex];
}

inline DGLTexCoord4F*
Surface::getCurrentTexCoord0()
{
   return &m_pTexCoord0Array[m_currVertexIndex];
}

inline DGLHazeCoordF*
Surface::getCurrentHazeCoord()
{
   return &m_pHazeStoreArray[m_currVertexIndex];
}

inline void
Surface::incCurrVertexIndex()
{
   m_currVertexIndex++;
   AssertFatal(m_currVertexIndex < sm_maxNumVertices, "Vertex count exceeded max");
}

inline int
Surface::getCurrVertexIndex()
{
   return m_currVertexIndex;
}

inline void
Surface::clearCurrVertexIndex()
{
   m_currVertexIndex = 0;
}


inline GLenum
Surface::getPrefMinFilter(const bool in_mipMap)
{
   if (in_mipMap == false)
      return GL_LINEAR;
   else
      return GL_NEAREST_MIPMAP_LINEAR;
}

inline GLenum
Surface::getPrefMagFilter()
{
   return GL_LINEAR;
}

inline int
Surface::getMaxTextureDetailLevel()
{
   return 1;
}

//------------------------------------------------------------------------------
const char* translateOpenGLError(const GLenum in_glError);

} // namespace OpenGL

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GGLSFC_H_
