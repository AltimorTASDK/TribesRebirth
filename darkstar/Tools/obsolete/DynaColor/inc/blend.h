/*---------------------------------------------------------------------------
/ blend.h
/
/ functions to apply blending effects to the 24-bit data image
/--------------------------------------------------------------------------*/

#ifndef __BLEND_H
#define __BLEND_H

#include "dy_types.h"
#include "bmp.h"

typedef struct {
   u8 r;
   u8 g;
   u8 b;
   f32 start_intensity;  
} blend_color;


void blend_uniform(blend_color *color);

#endif //__BLEND_H