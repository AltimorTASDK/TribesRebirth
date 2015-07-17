//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//±
//± Description
//±
//± $Workfile$
//± $Revision$
//± $Author  $
//± $Modtime $
//±
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

//#include <ptCore.h>
//#include <ptBuild.h>
#include <luvConv.h>
#include <math.h>


static double rgbConvMatrix[3][3] = { 3.240479, -1.537150, -0.498535,
                                    -0.969256,  1.875992,  0.041556,
                                     0.055648, -0.204043,  1.057311 };

static double xyzConvMatrix[3][3] = { 0.412453,  0.357580,  0.180423,
                                     0.212671,  0.715160,  0.072169,
                                     0.019334,  0.119193,  0.950227 };

static COLOR white =
{
   0.412453 + 0.357580 + 0.180423,
   0.212671 + 0.715160 + 0.072169,
   0.019334 + 0.119193 + 0.950227,
};



//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//± NAME
//±   void convertRGBtoLUV
//±
//± DESCRIPTION
//±               Converts the input rgbPixelInt's to the CIE L*U*V* format
//±         a more perceptually uniform color space...
//±
//± ARGUMENTS
//±            in_rgbArray    input pixels
//±            in_numPixels   number of above
//±            out_luvArray   output luv Array
//±
//± RETURNS
//±            output pixels in out_luvArray
//±
//± NOTES
//±            - Assumes memory for out_luvArray has already been allocated
//±               by the calling function.
//±
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
void convertRGBtoLUV(COLOR *rgbArray, unsigned long count)
{
   for (unsigned int i = 0; i < count; i++, rgbArray++)
   {
      rgbArray->R /= 256.0f;
      rgbArray->G /= 256.0f;
      rgbArray->B /= 256.0f;
      rgbToLuvConv( rgbArray );
   }
}


//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//± NAME
//±   void convertLUVtoRGB
//±
//± DESCRIPTION
//±            The reverse of the previous function, with the exception that
//±      this function returns the pixels as COLOR's, so that the
//±      calling function can use the output with maximum precision.
//±   
//± ARGUMENTS 
//±            in_luvArray    input pixels
//±            in_numPixels   number of above
//±            out_rgbArray   output luv Array
//±   
//± RETURNS 
//±            output pixels in out_rgbArray
//±   
//± NOTES 
//±            - Assumes memory for out_rgbArray has already been allocated
//±               by the calling function.
//±   
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
void convertLUVtoRGB(COLOR *luvArray, int count)
{
   for (int i = 0; i < count; i++, luvArray++) 
      luvToRgbConv( luvArray );
}



//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//± NAME 
//±   void rgbToLuvConv
//±   
//± NOTES 
//±   
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
void rgbToLuvConv( COLOR *rgb )
{
   COLOR xyz;

   double Lstar;
   double Ustar;
   double Vstar;

   double uPrime, vPrime;
   double unPrime, vnPrime;


   xyz.X = (rgb->R * xyzConvMatrix[0][0]) +
           (rgb->G * xyzConvMatrix[0][1]) +
           (rgb->B * xyzConvMatrix[0][2]);

   xyz.Y = (rgb->R * xyzConvMatrix[1][0]) +
           (rgb->G * xyzConvMatrix[1][1]) +
           (rgb->B * xyzConvMatrix[1][2]);

   xyz.Z = (rgb->R * xyzConvMatrix[2][0]) +
           (rgb->G * xyzConvMatrix[2][1]) +
           (rgb->B * xyzConvMatrix[2][2]);

   double yOverYn = xyz.Y / white.Y;

   if (yOverYn < 0.008856) 
      Lstar = 903.3 * yOverYn;
   else // L* is a cube root proportional function...
      Lstar = (116.0 * pow(yOverYn, 1./3.)) - 16.0;

   if (Lstar != 0.0) 
   {
        uPrime = (4.0 * xyz.X) / (xyz.X + 15.0 * xyz.Y + 3.0 * xyz.Z);
        vPrime = (9.0 * xyz.Y) / (xyz.X + 15.0 * xyz.Y + 3.0 * xyz.Z);
        unPrime= (4.0 * white.X) / (white.X + 15.0 * white.Y + 3.0 * white.Z);
        vnPrime = (9 * white.Y) / (white.X + 15.0 * white.Y +  3.0 * white.Z);
        Ustar = 13.0 * Lstar * (uPrime - unPrime);
        Vstar = 13.0 * Lstar * (vPrime - vnPrime);
    } 
    else 
    {
        Ustar = 0.0;
        Vstar = 0.0;
    }

   // Copy the values into the output pixel...
   //
   rgb->L = Lstar;
   rgb->U = Ustar;
   rgb->V = Vstar;
}



//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//± NAME 
//±   void rgbToLuvConv
//±   
//± NOTES 
//±   
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
void luvToRgbConv( COLOR *luv )
{
   double Lstar;

   double uPrime, vPrime;
   double unPrime, vnPrime;
      
   double temp;

   double X, Y, Z;

   unPrime = (4.0 * white.X) / ( white.X + 15.0 * white.Y + 3.0 * white.Z);
   vnPrime = (9.0 * white.Y) / ( white.X + 15.0 * white.Y + 3.0 * white.Z);
   Lstar = luv->X;

   // Recover Y first...
   //
   if (Lstar < (903.3 * .008856)) 
      Y = (Lstar / 903.3) * white.Y;
   else 
      Y = pow(((Lstar + 16.0) / 116.0), 3.0) * white.Y;

   // Now retrieve uPrime and vPrime...
   //
   if (Lstar == 0.0) 
   {
      //   If we are at this point, we know that the pixel is black.
      // and can exit now to prevent div by zero checks later...
      //
      //uPrime = 0.0;
      //vPrime = 0.0;

      luv->R = 0.0;
      luv->G = 0.0;
      luv->B = 0.0;

      return;
   } 
   else 
   {
      uPrime = (luv->Y / (13.0 * Lstar)) + unPrime;
      vPrime = (luv->Z / (13.0 * Lstar)) + vnPrime;
   }

   temp = vPrime / (9.0 * Y);

   // Now recover X...
   //
   X = uPrime / (4. * temp);

   // And Z...
   //
   temp = 1.0/temp;
   temp -= X;
   temp -= (15.0 * Y);
   temp /= 3.0;
   Z = temp;

   // Now transform from XYZ to RGB...
   //
   luv->R = (X * rgbConvMatrix[0][0]) +
            (Y * rgbConvMatrix[0][1]) +
            (Z * rgbConvMatrix[0][2]);

   luv->G = (X * rgbConvMatrix[1][0]) +
            (Y * rgbConvMatrix[1][1]) +
            (Z * rgbConvMatrix[1][2]);

   luv->B = (X * rgbConvMatrix[2][0]) +
            (Y * rgbConvMatrix[2][1]) +
            (Z * rgbConvMatrix[2][2]);
}
