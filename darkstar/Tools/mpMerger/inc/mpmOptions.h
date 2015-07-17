//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _MPMOPTIONS_H_
#define _MPMOPTIONS_H_

//Includes
#include <base.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class MPMFileList;

class MPMOptions {
   // File access members.  Note: this class assumes file is small enough to read into
   //  buffer...
  private:
   char*    m_pFileName;

   UInt32   m_fileSize;
   char*    m_pFileBuffer;
   bool     m_tokenizationBegun;

   // Extracted options
  private:
   UInt32 m_masterChooseFirst;
   UInt32 m_masterChooseLast;
   UInt32 m_masterRenderFirst;
   UInt32 m_masterRenderLast;

   ColorF m_shadeColor;
   float  m_shadePercentage;
   ColorF m_hazeColor;
   float  m_hazePercentage;

   UInt32 m_numShadeLevels;
   UInt32 m_numHazeLevels;

   UInt32 m_numShadeBiasLevels;
   UInt32 m_numHazeBiasLevels;
   float  m_shadeHazeBiasFactor;

   float  m_defaultColorWeighting;

   char*  m_pOutputPalName;
   char*  m_pBasePalName;

   // File access functions...
  protected:
   bool isAttached() const { return (m_pFileBuffer != NULL); }
   bool getNextLine(char out_pLineBuffer[], const UInt32 in_bufferSize);

   // Parsing functions...
  public:
   bool masterChooseRange(const char*);
   bool masterRenderRange(const char*);
   bool shadeColor(const char*);
   bool hazeColor(const char*);
   bool shadeHazeBiasLevels(const char*);
   bool shadeHazeBiasFactor(const char*);
   bool shadeHazeLevels(const char*);
   bool palOutputName(const char*);
   bool basePalName(const char*);

   bool paletteListStart(const char*);

  protected:
   bool parseLine(const char* in_pLine);  // returns false if no more options are to be
                                          //  found in the file, i.e., bitmapListStart()
                                          //  or endOptions() is found
  public:
   MPMOptions();
   ~MPMOptions();

   bool validateOptions() const;

   bool attachResponseFile(const char* in_pFileName);
   void processResponseFile(MPMFileList& out_rFileList);
   void unattachResponseFile();

   // Access functions...
  public:
   void getMasterRenderRange(UInt32& out_first, UInt32& out_last) const {
      out_first = m_masterRenderFirst;
      out_last  = m_masterRenderLast;
   }
   void getMasterChooseRange(UInt32& out_first, UInt32& out_last) const {
      out_first = m_masterChooseFirst;
      out_last  = m_masterChooseLast;
   }

   ColorF getShadeColor() const { return m_shadeColor; }
   ColorF getHazeColor()  const { return m_hazeColor; }
   float  getShadePercentage() const { return m_shadePercentage; }
   float  getHazePercentage()  const { return m_hazePercentage; }

   UInt32 getNumShadeLevels() const { return m_numShadeLevels; }
   UInt32 getNumHazeLevels()  const { return m_numHazeLevels; }

   void   getShadeHazeBiasLevels(UInt32& out_shadeBias, UInt32& out_hazeBias) const {
      out_shadeBias = m_numShadeBiasLevels;
      out_hazeBias  = m_numHazeBiasLevels;
   }
   float  getShadeHazeBiasFactor() const { return m_shadeHazeBiasFactor; }

   bool        hasBasePalette() const   { return (m_pBasePalName != NULL); }
   const char* getBasePalName() const   { return m_pBasePalName; }
   const char* getOutputPalName() const { return m_pOutputPalName; }
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_PNOPTIONS_H_
