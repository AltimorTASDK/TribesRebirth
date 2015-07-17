//---------------------------------------------------------------------------

//	$Workfile:   viewcone.h  $
//	$Version$
//	$Revision:   1.3  $
//	$Date:   31 Oct 1995 17:15:52  $

//  VO:timg

//---------------------------------------------------------------------------

#ifndef _VIEW_CONE_H
#define _VIEW_CONE_H

#include <ml.h>


#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//---------------------------------------------------------------------------
// VC_MAX_NUMPOINTS represents the size of the vertex arrays used for
// temporary polygons.  This number must be larger than the largest
// polygon passed to the clipping functions since clipping may increase
// the number of vertices in a polygon.
//
#define VC_MAX_NUMPOINTS	20


//---------------------------------------------------------------------------

enum ViewConePlanes
{
	VC_FRONT_PLANE,
	VC_BACK_PLANE,
	VC_TOP_PLANE,
	VC_BOTTOM_PLANE,
	VC_LEFT_PLANE,
	VC_RIGHT_PLANE,
	VC_TOTAL_PLANES,
};

enum ViewConeClip
{
	VC_CLIP_FRONT	= 1 << 1,
	VC_CLIP_BACK	= 1 << 2,
	VC_CLIP_TOP		= 1 << 3,
	VC_CLIP_BOTTOM	= 1 << 4,
	VC_CLIP_LEFT	= 1 << 5,
	VC_CLIP_RIGHT	= 1 << 6,
	VC_CLIP_ALL		= ~0,
};


//---------------------------------------------------------------------------

class ViewCone
{
	float expand;							// Cone is expanded by this value.
	float distance;						// Distance to back plane.
	Int32	clipMask;						// Mask of planes to clip against.

	Point3F	camera;						// Camera position.
	PlaneF plane[VC_TOTAL_PLANES];	// The six plane equations.

public:
	ViewCone();
	ViewCone(float dist,float expand);
	ViewCone(const TSCamera& cam,const Point3F& camPos,
			float dist,float expand);

	void	set(const TSCamera& cam,const Point3F& camPos);
	void	set(const TSCamera& cam,const Point3F& camPos,
				float dist,float expand);
	float getDistance() const;
	float getExpansion() const;

	void	setClipMask(Int32 m);
	Int32 getClipMask() const;

	Bool	clip(Poly3F& poly,Poly3F* pout);
	Bool	clipGround(Poly3F& poly,float min,float max,Poly3F* pout);
};


//---------------------------------------------------------------------------

inline void ViewCone::set(const TSCamera& cam,const Point3F& camPos)
{
	set(cam,camPos,distance,expand);
}

inline float ViewCone::getDistance() const
{
	return distance;
}

inline float ViewCone::getExpansion() const
{
	return expand;
}

inline void ViewCone::setClipMask(Int32 m)
{
	clipMask = m;
}

inline Int32 ViewCone::getClipMask() const
{
	return clipMask;
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif

