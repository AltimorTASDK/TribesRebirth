//------------------------------------------------------------------------------
// Description : Prototypes for the LUV conversion functions.
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#ifndef _LUVCONV_H_
#define _LUVCONV_H_

struct COLOR
{
   union { double R; double L; double X; };
   union { double G; double U; double Y; };
   union { double B; double V; double Z; };
   int index;
};

void convertRGBtoLUV(COLOR *rgbArray, unsigned long count);
void convertLUVtoRGB(COLOR *luvArray, int count);
void rgbToLuvConv(COLOR *rgb);
void luvToRgbConv(COLOR *luv);


#endif //_LUVCONV_H_
