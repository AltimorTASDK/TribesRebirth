//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#include <ml.h>
#include "m_collision.h"


//----------------------------------------------------------------------------

void CollisionSurface::transform(TMat3F& mat)
{
	Point3F tmp = position;
	m_mul(tmp,mat,&position);
	tmp = normal;
	m_mul(tmp,static_cast<RMat3F&>(mat),&normal);
}


//----------------------------------------------------------------------------

void CollisionSurfaceList::transform(TMat3F& mat)
{
#if 1
	for (iterator itr = begin(); itr != end(); itr++)
		(*itr).transform(mat);
#else      
	for (int i = 0; i < count; i++)
		surfaces[i].transform(mat);
#endif      
}	

