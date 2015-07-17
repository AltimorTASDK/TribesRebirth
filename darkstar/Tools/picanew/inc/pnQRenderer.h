//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _PNQRENDERER_H_
#define _PNQRENDERER_H_

//Includes
#include <base.h>
#include <tString.h>
#include "colorMatcher.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class PNOptions;
class PNUnquantizedImage;
class GFXPalette;
class PNQuantizedRenderer;

class PNQuantizedRenderer {
   GFXPalette* m_pPalette;

   String     m_outputDir;

   UInt32     m_renderFirst;
   UInt32     m_renderLast;
   UInt32     m_paletteKey;

   bool       m_inAlphaMode;
   bool       m_inAdditiveMode;
   bool       m_inSubtractiveMode;
   bool       m_transparentRender;

   bool       m_extrudingMipLevels;

   bool       m_areDithering;
   UInt32     m_ditherTolerance;

   ColorMatcher* m_pMatchObject;

   bool          m_areZeroing;
   bool          m_areEdgeMaintaining;
   PALETTEENTRY  m_zeroColor;

   bool m_forcePhoenix;

   bool extrudeBitmapMipLevels(GFXBitmap* io_pBitmap);

   void renderUQImageDithered(const PNUnquantizedImage* in_pRawImage);

  public:
   PNQuantizedRenderer(const PNOptions& in_rOptions,
                       GFXPalette*      in_pPal);
   ~PNQuantizedRenderer();

   void renderUQImage(const PNUnquantizedImage* in_pRawImage);

   const UInt32* getChosenFreqs() const { return m_pMatchObject->getChosenFreqs(); }
};


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_PNQRENDERER_H_
