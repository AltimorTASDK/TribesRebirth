//================================================================
//   
// $Workfile:   g_rect.cpp  $
// $Version$
// $Revision:   1.4  $
//   
// DESCRIPTION:
//   
// (c) Copyright 1995, Dynamix Inc.   All rights reserved.
//================================================================

#include <ml.h>

//================================================================

static Point2I    pt[4];
static Point2X   ptf[4];
static Point2I*   rectangle_vertex[4] = { &pt[0], &pt[1], &pt[2], &pt[3] };
static Point2X*  rectangle_texvert[4]= { &ptf[0], &ptf[1], &ptf[2], &ptf[3] };
static Fix16     rectangle_shade[4];

//================================================================

static Point3I    pt3[4];
static Point3X   pt3f[4];
static Point3I*   rectangle3_vertex[4] = { &pt3[0], &pt3[1], &pt3[2], &pt3[3] };
static Point3X*  rectangle3_texvert[4]= { &pt3f[0], &pt3f[1], &pt3f[2], &pt3f[3] };

//================================================================
// NAME
//   rect_rect_to_point2
//   
// DESCRIPTION
// ARGUMENTS 
// RETURNS 
// NOTES 
//   
//================================================================
void rect_rect_to_point2( const RectI *in_rect, Point2I **&out_point )
{
   Point2I *pt;
   out_point= rectangle_vertex;
   pt       = *rectangle_vertex;

   *pt++ = in_rect->upperL;

   pt->x = in_rect->lowerR.x;
   pt->y = in_rect->upperL.y;
   pt++;

   *pt++ = in_rect->lowerR;

   pt->x = in_rect->upperL.x;
   pt->y = in_rect->lowerR.y;
}

//================================================================
// NAME
//   rect_rect_to_point3
//   
// DESCRIPTION
// ARGUMENTS 
// RETURNS 
// NOTES 
//   
//================================================================
void rect_rect_to_point3( const RectI *in_rect, Int32 in_z, Point3I **&out_point )
{
   Point3I *pt;
   out_point= rectangle3_vertex;
   pt       = *rectangle3_vertex;

   pt->x = in_rect->upperL.x;
   pt->y = in_rect->upperL.y;
   pt->z = in_z;
   pt++;

   pt->x = in_rect->lowerR.x;
   pt->y = in_rect->upperL.y;
   pt->z = in_z;
   pt++;

   pt->x = in_rect->lowerR.x;
   pt->y = in_rect->lowerR.y;
   pt->z = in_z;
   pt++;

   pt->x = in_rect->upperL.x;
   pt->y = in_rect->lowerR.y;
   pt->z = in_z;
}

//================================================================
// NAME
//   rect_rect_to_point2f
//   
// DESCRIPTION
// ARGUMENTS 
// RETURNS 
// NOTES 
//   
//================================================================
void rect_rect_to_point2f( const RectI *in_rect, Point2X **&out_point )
{
   Point2I *pt;
   out_point= rectangle_texvert;
   pt       = (Point2I*)(*rectangle_texvert);

   *pt++   = in_rect->upperL;

   pt->x = in_rect->lowerR.x;
   pt->y = in_rect->upperL.y;
   pt++;

   *pt++ = in_rect->lowerR;

   pt->x = in_rect->upperL.x;
   pt->y = in_rect->lowerR.y;
}


//================================================================
// NAME
//   rect_rect_to_point2f
//   
// DESCRIPTION
// ARGUMENTS 
// RETURNS 
// NOTES 
//   
//================================================================
void rect_rect_to_point3f( const RectI *in_rect, Point3X **&out_point )
{
   Point3X *pt;
   out_point= rectangle3_texvert;
   pt       = *rectangle3_texvert;

   pt->x = in_rect->upperL.x;
   pt->y = in_rect->upperL.y;
   pt++;

   pt->x = in_rect->lowerR.x;
   pt->y = in_rect->upperL.y;
   pt++;

   pt->x = in_rect->lowerR.x;
   pt->y = in_rect->lowerR.y;
   pt++;

   pt->x = in_rect->upperL.x;
   pt->y = in_rect->lowerR.y;
}

//================================================================
// NAME
//   rect_shade_to_fix16
//   
// DESCRIPTION
// ARGUMENTS 
// RETURNS 
// NOTES 
//   
//================================================================
void rect_rect_to_fix16( const RectI *in_shade, Fix16 *&out_point )
{
   out_point = rectangle_shade;
   out_point[0] = in_shade->upperL.x;
   out_point[1] = in_shade->upperL.y;
   out_point[2] = in_shade->lowerR.x;
   out_point[3] = in_shade->lowerR.y;
}
