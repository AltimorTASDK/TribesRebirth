//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _PNOPTIONS_H_
#define _PNOPTIONS_H_

//Includes
#include <base.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class PNFileList;

class PNOptions {
   // File access members.  Note: this class assumes file is small enough to read into
   //  buffer...
  private:
   char*    m_pFileName;

   UInt32   m_fileSize;
   char*    m_pFileBuffer;
   bool     m_tokenizationBegun;

   // Publically available enumerations...
  public:
   enum ColorSpace {
      ColorSpaceUndetermined,
      ColorSpaceAlpha,
      ColorSpaceAdditive,
      ColorSpaceSubtractive,
      ColorSpaceRGB,
   };

   // Extracted options
  private:
   ColorSpace  m_colorSpace;

   bool        m_renderingBitmaps;
   bool        m_overrideRenderRange;
   bool        m_choosingColors;
   Int32       m_chooseFirst;
   Int32       m_chooseLast;
   Int32       m_renderFirst;
   Int32       m_renderLast;

   Int32       m_numRBits;
   Int32       m_numGBits;
   Int32       m_numBBits;
   Int32       m_numABits;

   char*       m_pOutPath;

   char*       m_pOutputPalName;
   char*       m_pBasePalName;

   ColorF      m_zeroColor;

   DWORD       m_paletteKey;

   bool        m_extrudingMipLevels;
   bool        m_stripQuantizationInfo;

   bool        m_areDithering;
   UInt32      m_ditherTolerance;

   bool        m_standAlonePalette;
   bool        m_asPhoenix;

   bool        m_maintainEdges;

   // File access functions...
  protected:
   bool isAttached() const { return (m_pFileBuffer != NULL); }
   bool getNextLine(char out_pLineBuffer[], const UInt32 in_bufferSize);

   // Parsing functions...
  public:
   bool quantizeAlpha(const char*);
   bool quantizeAdditive(const char*);
   bool quantizeSubtractive(const char*);
   bool quantizeRGB(const char*);
   bool chooseRange(const char*);
   bool renderRange(const char*);
   bool noRender(const char*);
   bool outputPath(const char*);
   bool palOutputName(const char*);
   bool basePalName(const char*);
   bool zeroColor(const char*);
   bool quantizeBitsRGB(const char*);
   bool quantizeBitsAlpha(const char*);
   bool paletteKey(const char*);
   bool stripQuantizationInfo(const char*);
   bool extrudeMipLevels(const char*);
   bool enableDithering(const char*);
   bool ditherTolerance(const char*);
   bool standAlonePalette(const char*);
   bool enforcePhoenixSave(const char*);
   bool maintainEdges(const char*);

   bool bitmapListStart(const char*);

  protected:
   bool parseLine(const char* in_pLine);  // returns false if no more options are to be
                                          //  found in the file, i.e., bitmapListStart()
                                          //  or endOptions() is found
  public:
   PNOptions();
   ~PNOptions();

   bool validateOptions() const;

   bool attachResponseFile(const char* in_pFileName);
   void processResponseFile(PNFileList& out_rFileList);
   void unattachResponseFile();

   // Accessor functions for properties...
  public:
   ColorSpace getColorSpace() const { return m_colorSpace; }

   bool areChoosingColors()   const { return m_choosingColors; }
   void getChooseRange(UInt32& out_rRangeFirst, UInt32& out_rRangeLast) const {
      out_rRangeFirst = m_chooseFirst;
      out_rRangeLast  = m_chooseLast;
   }
   bool areRenderingBitmaps() const {
      if (m_overrideRenderRange == false) {
         return m_renderingBitmaps;
      } else {
         return false;
      }
   }
   void getRenderRange(UInt32& out_rRangeFirst, UInt32& out_rRangeLast) const;

   bool        hasBasePalette() const   { return (m_pBasePalName != NULL); }
   const char* getBasePalName() const   { return m_pBasePalName; }
   const char* getOutputPalName() const { return m_pOutputPalName; }

   const char* getOutputPath() const { return m_pOutPath; }

   int getRBits() const { return m_numRBits; }
   int getGBits() const { return m_numGBits; }
   int getBBits() const { return m_numBBits; }
   int getABits() const { return m_numABits; }

   bool getZeroColor(ColorF& out_rColor) const;

   UInt32 getPaletteKey() const { return m_paletteKey; }

   bool getStripQuantizationInfo() const { return m_stripQuantizationInfo; }
   bool getExtrudingMipLevels() const    { return m_extrudingMipLevels; }

   bool areDithering() const         { return m_areDithering; }
   UInt32 getDitherTolerance() const { return m_ditherTolerance; }

   bool isStandAlonePalette() const { return m_standAlonePalette; }
   bool getForcePhoenix() const { return m_asPhoenix; }

   bool areMaintainingEdges() const { return m_maintainEdges; }
};

inline bool
PNOptions::getZeroColor(ColorF& out_rColor) const
{
   if (m_zeroColor.red < 0.0f)
      return false;

   out_rColor = m_zeroColor;
   return true;
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_PNOPTIONS_H_
