//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include "pnUnquantizedImage.h"

PNUnquantizedImage::PNUnquantizedImage()
 : m_pImageName(NULL),
   m_width(0),
   m_height(0)
{

}

PNUnquantizedImage::~PNUnquantizedImage()
{
   delete [] m_pImageName;
   m_pImageName = NULL;
}

void
PNUnquantizedImage::setImageName(const char* in_pImageName)
{
   AssertFatal(in_pImageName != NULL, "Error, no name");

   delete [] m_pImageName;
   m_pImageName = new char[strlen(in_pImageName) + 1];
   strcpy(m_pImageName, in_pImageName);
}
