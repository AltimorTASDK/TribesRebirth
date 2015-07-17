/*---------------------------------------------------------------------------
/ blend.c
/
/ functions to apply blending effects to the 24-bit data image
/--------------------------------------------------------------------------*/


#include "globs.h"
#include "blend.h"

#define BLEND_COLOR_EQ(r,g,b, color)  ( (r == (u8)color.r) && (g == (u8)color.g) && (b == (u8)color.b) )

/*---------------------------------------------------------------------------
/ blend_uniform()
/
/ Blends the specified color onto the bitmap data pointed to by the
/ global variable Data.
/
/ For every EVEN vertical scanline of the bitmap, the blending color
/ is applied with DECREASING intensity from left to right.
/
/ For every ODD vertical scanline of the bitmap, the blending color
/ is applied with INCREASING intensity from left to right.
/--------------------------------------------------------------------------*/
void blend_uniform(blend_color *color)
{
   u8 *data;
   u32 half_width;
   u32 height;
   u32 padded_width;
   u8 r, g, b;
   s32 dr, dg, db; /* delta colors */
   u32 x,y;
   f32 left_intensity;
   f32 right_intensity;
   f32 deltaIntensity;


   // all globals are mapped to local variables
   // for debugging
   // 
   // re-map of globals start
   data = Data;      
   half_width = Width / 2;
   height = Height;     
   padded_width = PaddedWidth; 
   // re-map of globals end

   left_intensity = color->start_intensity;
   right_intensity = 0.0f;
   deltaIntensity = left_intensity / (float)half_width;

   for (y = 0; y < height; y++) {
      for (x = 0; x < half_width; x++) {
		   // do the left side first
         // get original color
         b = *data;
         g = *(data + 1);
         r = *(data + 2);

         if ((! (IgnoreColorOn && BLEND_COLOR_EQ(r,g,b, IgnoreColor) ) )
               && (! (zeroColorOn && BLEND_COLOR_EQ(r,g,b, zeroColor) ) ))
         {
            // modify the color 
            dr = (s32)color->r - (s32)r;
            dg = (s32)color->g - (s32)g;
            db = (s32)color->b - (s32)b;

            dr = (s32)r + (s32)((f32)dr * left_intensity);
            dg = (s32)g + (s32)((f32)dg * left_intensity);
            db = (s32)b + (s32)((f32)db * left_intensity);


            r = (dr < 0) ? 0 : ((dr > 255) ? 255 : (u8)dr);
            g = (dg < 0) ? 0 : ((dg > 255) ? 255 : (u8)dg);
            b = (db < 0) ? 0 : ((db > 255) ? 255 : (u8)db);
         }
         // write the new color back
         *data++ = b;
         *data++ = g;
         *data++ = r;

		   // do the right side next
         // get original color
         b = *data;
         g = *(data + 1);
         r = *(data + 2);

         if ((! (IgnoreColorOn && BLEND_COLOR_EQ(r,g,b, IgnoreColor) ) )
               && (! (zeroColorOn && BLEND_COLOR_EQ(r,g,b, zeroColor) ) ))
         {

            // modify the color 
            dr = (s32)color->r - (s32)r;
             dg = (s32)color->g - (s32)g;
             db = (s32)color->b - (s32)b;

            dr = (s32)r + (s32)((f32)dr * right_intensity);
            dg = (s32)g + (s32)((f32)dg * right_intensity);
            db = (s32)b + (s32)((f32)db * right_intensity);
      
            r = (dr < 0) ? 0 : ((dr > 255) ? 255 : (u8)dr);
            g = (dg < 0) ? 0 : ((dg > 255) ? 255 : (u8)dg);
            b = (db < 0) ? 0 : ((db > 255) ? 255 : (u8)db);
         }
         // write the new color back
         *data++ = b;
         *data++ = g;
         *data++ = r;
 
         left_intensity -= deltaIntensity;
		   right_intensity += deltaIntensity;
         if (left_intensity < 0.0f) 
            left_intensity = 0.0f;
		   if (right_intensity > 1.0f)
		      right_intensity = 1.0f;
      }
      data += (padded_width - (half_width * 3 * 2));
      left_intensity = color->start_intensity;
      right_intensity = 0.0f;
   } 
}
