//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _PNUNQUANTIZEDFILE_H_
#define _PNUNQUANTIZEDFILE_H_

//Includes
#include <base.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class PNPopularityTable;

// Mostly an interface class.  Concrete classes will implement all important
//  functionality...
//
class PNUnquantizedImage {
   friend class PNUQImageFactory;
   
   char* m_pImageName;
   
   UInt32 m_width;
   UInt32 m_height;

  protected:
   void setImageName(const char* in_pImageName);
   void setImageDimensions(const UInt32 in_width, const UInt32 in_height);

  public:
   PNUnquantizedImage();
   virtual ~PNUnquantizedImage();

   const char* getImageName() const { return m_pImageName; }
   UInt32 getWidth()  const { return m_width; }
   UInt32 getHeight() const { return m_height; }

   virtual const PALETTEENTRY* getFinalPixels() const = 0;

   virtual void registerColors(PNPopularityTable& in_rPopTable,
                               const UInt32       in_weight) = 0;
   virtual bool loadFile(const char* in_pFileName) = 0;
};

inline void
PNUnquantizedImage::setImageDimensions(const UInt32 in_w, const UInt32 in_h)
{
   m_width  = in_w;
   m_height = in_h;
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_PNUNQUANTIZEDFILE_H_
