//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _LUVCONV_H_
#define _LUVCONV_H_

void convertRGBtoLUV(COLOR *rgbArray, Int32 count);
void convertLUVtoRGB(COLOR *luvArray, int count);
void rgbToLuvConv(COLOR *rgb);
void luvToRgbConv(COLOR *luv);


#endif //_LUVCONV_H_
