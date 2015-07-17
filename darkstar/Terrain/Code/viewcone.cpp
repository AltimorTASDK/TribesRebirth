//---------------------------------------------------------------------------

//	$Workfile:   viewcone.cpp  $
//	$Version$
//	$Revision:   1.6  $
//	$Date:   09 Jan 1996 17:29:44  $

//  VO:timg

//---------------------------------------------------------------------------

#include <string.h>

#include <base.h>
#include <ml.h>
#include <ts.h>

#include "viewcone.h"

//---------------------------------------------------------------------------

static void poly_copyTranslate(Poly3F& spoly,Point3F& cp,Poly3F* dpoly);
static void poly_copyUnTranslate(Poly3F& spoly,Point3F& cp,Poly3F* dpoly);


//---------------------------------------------------------------------------

ViewCone::ViewCone()
{
	expand = 0.0f;
	distance = 0.0f;
	clipMask = VC_CLIP_ALL;
};

ViewCone::ViewCone(float dst,float exp)
{
	distance = dst;
	expand = exp;
	clipMask = VC_CLIP_ALL;
}

ViewCone::ViewCone(const TSCamera& cam,const Point3F& camPos,
	float dist,float expnd)
{
	distance = dist;
	expand = expnd;
	clipMask = VC_CLIP_ALL;
	set(cam,camPos,dist,expand);
}


//---------------------------------------------------------------------------
// Build the six planes that make up the viewing cone.
//
void ViewCone::set(const TSCamera& cam,
	const Point3F& camPos,float dist,float exp)
{
	Point3F cp;
	Vector3F vec,rvec,zvec,xvec,yvec;
	float dxl,dxr,dy,dzt,dzb;

	AssertFatal(dist > cam.getNearDist(),
		"ViewCone::set: Back plane must be further than focal plane.");

	// Save the distance and expansion and camera
	// for later.
	//
	TMat3F camTCW = cam.getTOC();
	camTCW.inverse();
	distance = dist;
	expand = exp;

	// Cannot use TCW.p because it's been rotated around a couple
	// of times by now and is badly mangled.
	//
	camera = camPos;

	// Project the viewport on to the back plane.
	//
	const RectF& vp = cam.getWorldViewport();
	float cx = (vp.lowerR.x - vp.upperL.x) / 2.0f;
	float cy = (vp.upperL.y - vp.lowerR.y) / 2.0f;
	float div = 1 / cam.getNearDist();
	dxr = dxl = float(cx * distance * div);
	dzb = dzt = float(cy * distance * div);
	dy = distance;

	// Transform the expansion vectors into world space.
	// One vector along each axis.
	//
	vec.x = vec.y = 0.0f;
	vec.z = expand;
	m_mul(vec,*(RMat3F*)&camTCW,&zvec);

	vec.z = 0.0f;
	vec.x = expand;
	m_mul(vec,*(RMat3F*)&camTCW,&xvec);

	vec.x = 0.0f;
	vec.y = -expand;
	m_mul(vec,*(RMat3F*)&camTCW,&yvec);

	// Create the size planes that represent the cone.
	// The positive faces of the planes are facing the
	// inside of the cone.

	// The planes are formed by creating normals to the planes in
	// camera space, transforming them to world space, and then
	// building the plane equation using the transformed normal and 
	// the expansion vectors.  The planes are actually built around
	// the origin, not the camera. The polygons that are split against
	// the planes are translated to the planes space, clipped, then
	// translated back.  This is done to avoid overflow in the integer
	// plane math (which we don't use anymore, Doh!)

	// Top plane.
	//
	vec.x = 0.0f;
	vec.y = dzt;
	vec.z = -dy;
	m_mul(vec,*(RMat3F*)&camTCW,&rvec);
	plane[VC_TOP_PLANE].set(zvec,rvec);

	// Bottom plane.
	//
	zvec.neg();
	vec.y = dzb;
	vec.z = dy;
	m_mul(vec,*(RMat3F*)&camTCW,&rvec);
	plane[VC_BOTTOM_PLANE].set(zvec,rvec);

	// Right plane.
	//
	vec.x = -dy;
	vec.y = dxr;
	vec.z = 0.0f;
	m_mul(vec,*(RMat3F*)&camTCW,&rvec);
	plane[VC_RIGHT_PLANE].set(xvec,rvec);

	// Left plane.
	//
	xvec.neg();
	vec.y = dxl;
	vec.x = dy;
	m_mul(vec,*(RMat3F*)&camTCW,&rvec);
	plane[VC_LEFT_PLANE].set(xvec,rvec);

	// Front plane
	//
	vec.x = vec.z = 0.0f;
	vec.y = dy;
	m_mul(vec,*(RMat3F*)&camTCW,&rvec);
	plane[VC_FRONT_PLANE].set(yvec,rvec);

	// Back plane.
	// This plane is not expanded.
	//
	vec = rvec;
	rvec.neg();
	plane[VC_BACK_PLANE].set(vec,rvec);
}


//---------------------------------------------------------------------------
// Use by following functions.
//
inline void swap(Poly3F*& a,Poly3F*& b)
{
	Poly3F* c = a;
	a = b;
	b = c;
}


//---------------------------------------------------------------------------
// Clip the polygon against the viewing cone.
// Return true if the resulting polygon is not empty.
//
Bool ViewCone::clip(Poly3F& poly,Poly3F* pout)
{
	Point3F pa1[VC_MAX_NUMPOINTS];
	Point3F pa2[VC_MAX_NUMPOINTS];
	Poly3F tp1,tp2,*src,*dst;

	tp1.points = pa1;
	tp2.points = pa2;

	// Clip the Poly against the six viewing planes.
	// The polygon is first translated into plane space,
	// then the result translated back to world space.
	//
	poly_copyTranslate(poly,camera,pout);
	src = pout; dst = &tp1;

	if (clipMask & VC_CLIP_LEFT)
		plane[VC_LEFT_PLANE].split(*src,dst,&tp2),
		swap(src,dst);
	if (clipMask & VC_CLIP_RIGHT)
		plane[VC_RIGHT_PLANE].split(*src,dst,&tp2),
		swap(src,dst);
	if (clipMask & VC_CLIP_TOP)
		plane[VC_TOP_PLANE].split(*src,dst,&tp2),
		swap(src,dst);
	if (clipMask & VC_CLIP_BOTTOM)
		plane[VC_BOTTOM_PLANE].split(*src,dst,&tp2),
		swap(src,dst);
	if (clipMask & VC_CLIP_BACK)
		plane[VC_BACK_PLANE].split(*src,dst,&tp2),
		swap(src,dst);
	if (clipMask & VC_CLIP_FRONT)
		plane[VC_FRONT_PLANE].split(*src,dst,&tp2),
		src = dst;

	poly_copyUnTranslate(*src,camera,pout);
	return pout->numPoints != 0;
}


//---------------------------------------------------------------------------
// Clip the polygon against the viewing cone.
// Return true if the resulting polygon is not empty.
// The polygon is assumed to represent some extent of ground.  The min
// and max arguments should be set to the min and max height of all points
// or features on the ground.
//
Bool ViewCone::clipGround(Poly3F& poly,float min,float max,Poly3F* pout)
{
	Point3F pa1[VC_MAX_NUMPOINTS];
	Point3F pa2[VC_MAX_NUMPOINTS];
	Poly3F	tp1,tp2,*src,*dst;
	PlaneF tplane[VC_TOTAL_PLANES];

	tp1.points = pa1;
	tp2.points = pa2;

	// Copy and shift all the planes.
	//
	for (int i = 0; i < VC_TOTAL_PLANES; ++i) {
		tplane[i] = plane[i];

		// Positive C, shift it down (if max is positive).
		// Negative C, shift it up (if min is positive).
		//
		register PlaneF* p = &tplane[i];
		if (!p->isVertical())
			if (p->z > 0)
				p->shiftZ(-max);
			else
				p->shiftZ(min);
	}

	// Clip the Poly against the shifted planes.
	// The polygon is first translated into plane space,
	// then the result translated back to world space.
	//
	poly_copyTranslate(poly,camera,pout);
	src = pout; dst = &tp1;

	if (clipMask & VC_CLIP_LEFT)
		tplane[VC_LEFT_PLANE].split(*src,dst,&tp2),
		swap(src,dst);
	if (clipMask & VC_CLIP_RIGHT)
		tplane[VC_RIGHT_PLANE].split(*src,dst,&tp2),
		swap(src,dst);
	if (clipMask & VC_CLIP_TOP)
		tplane[VC_TOP_PLANE].split(*src,dst,&tp2),
		swap(src,dst);
	if (clipMask & VC_CLIP_BOTTOM)
		tplane[VC_BOTTOM_PLANE].split(*src,dst,&tp2),
		swap(src,dst);
	if (clipMask & VC_CLIP_BACK)
		tplane[VC_BACK_PLANE].split(*src,dst,&tp2),
		swap(src,dst);
	if (clipMask & VC_CLIP_FRONT)
		tplane[VC_FRONT_PLANE].split(*src,dst,&tp2),
		src = dst;

	poly_copyUnTranslate(*src,camera,pout);
	return pout->numPoints != 0;
}


//---------------------------------------------------------------------------
// Copy the polygon and translate to camera space.

static void poly_copyTranslate(Poly3F& spoly,Point3F& cp,Poly3F* dpoly)
{
	Point3F* sp = spoly.points;
	Point3F* dp = dpoly->points;

	for (int i = dpoly->numPoints = spoly.numPoints; i > 0; --i,++dp,++sp) {
		dp->x = sp->x - cp.x;
		dp->y = sp->y - cp.y;
		dp->z = sp->z - cp.z;
	}
}


//---------------------------------------------------------------------------
// Copy the polygon and translate to world space.

static void poly_copyUnTranslate(Poly3F& spoly,Point3F& cp,Poly3F* dpoly)
{
	Point3F* sp = spoly.points;
	Point3F* dp = dpoly->points;

	for (int i = dpoly->numPoints = spoly.numPoints; i > 0; --i,++dp,++sp) {
		dp->x = sp->x + cp.x;
		dp->y = sp->y + cp.y;
		dp->z = sp->z + cp.z;
	}
}
