//----------------------------------------------------------------------------

// Basic collision structures used by all the libraries.


//----------------------------------------------------------------------------

#ifndef _M_COLLISION_H_
#define _M_COLLISION_H_

#include <m_point.h>
#include <m_mat3.h>
#include <tRVector.h>

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

struct DLLAPI CollisionSurface
{
	// Intersection point & time.
	// Most intersection objects don't return
	// very accurate position values.
	float time;
	Point3F position;

	// Plane of intersection
	Point3F normal;
	float distance;

	// Data for recovering more info.
	// The material value is used to store a 
	// pointer to the TS::Material on the surface.
	// The other two fields are used differently
	// depending on the geometry engine.
	int material;
	int surface;
	int part;

	//
	void transform(TMat3F& mat);
};


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class CollisionSurfaceList : public RVector<CollisionSurface>
{
#if 0
	enum Constants {
		MaxSurfaces = 50,
	};
	int count;
	CollisionSurface surfaces[MaxSurfaces];
#endif

public:
	TMat3F tWorld;	// Into world space
	TMat3F tLocal;	// Into local space
	Point3F stepVector;
	bool contained;
	void transform(TMat3F& mat);

#if 0

	// Basic "vector" methods
	CollisionSurfaceList() { count = 0; stepHeight.set(0,0,0); contained = false; }
	typedef CollisionSurface* iterator;
	CollisionSurface* begin()  { return &surfaces[0]; }
	CollisionSurface* end()  { return &surfaces[count]; }
	int size() const { return count; }
	CollisionSurface& operator[](int index)  { return surfaces[index]; }
	const CollisionSurface& operator[](int index) const { return surfaces[index]; }
	bool empty() const { return size() == 0; }
	void clear()  { count = 0; }
	void push_back(CollisionSurface& s) {
		if (count < MaxSurfaces)
			surfaces[count++] = s;
	}
	void increment()  {
		if (count < MaxSurfaces)
			count++;
	}
	void decrement()   { if (count) count--; }
	CollisionSurface& last()  { return surfaces[count - 1]; }
   
#endif   
};


#endif
