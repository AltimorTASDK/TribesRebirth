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

namespace PowerSGL {

//-------------------------------------- Note that many members of this class are marked
//                                        as protected to guard against the necessity of
//                                        deriving classes to support specific
//                                        implementations
//
class Surface : public GFXSurface {
  private:
   // dGFX renderstate information
   //
   ColorF m_fillColor;

   ColorF m_constantShadeColor;
   float  m_constantHaze;
   float  m_constantAlpha;

   bool   m_transparencyEnabled;
   bool   m_perspectiveCorrect;

   GFXFillMode    m_fillMode;
   GFXShadeSource m_shadeSource;
   GFXHazeSource  m_hazeSource;
   GFXAlphaSource m_alphaSource;

   // Protected pure virtuals from GFXSurface
   //
  protected:
   Bool _setPalette(GFXPalette *lpPalette, Int32 in_start, Int32 in_count,
                            Bool in_rsvPal);
   void _setGamma();
	void _lock(const GFXLockMode in_lockMode);
	void _unlock();

   // Prevent direct creation of a Surface
  protected:
   Surface();
  public:
   ~Surface();
   
   // Public pure virtuals from GFXSurface
  public:
 	void flip();
	void drawSurface(GFXSurface *lpSourceSurface, const RectI *in_subRegion,
	                         const Point2I *in_at);

   // Surface Creation functions
  public:
   static Surface* create(HWND        io_clientWnd,
                                  const Int32 in_width,
                                  const Int32 in_height);
   // misc
  public:
   void processMessage(int argc, const char* argv[]);
};

//------------------------------------------------------------------------------

}; // namespace PowerSGL

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GGLSFC_H_
