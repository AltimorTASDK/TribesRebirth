//================================================================
//   
//	$Workfile:   m_temp.cpp  $
//	$Revision:   1.2  $
// $Version: 1.6.2 $
//	$Date:   10 Nov 1995 11:36:42  $
// $Log:   R:\darkstar\develop\core\ml\vcs\m_temp.cpv  $
//	
//================================================================

#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)

#pragma option -Jg

#include "ml.h"

template const int &min(const int &, const int &);
template const int &max(const int &, const int &);
template const long &min(const long &, const long &);
template const long &max(const long &, const long &);
template const float &min(const float &, const float &);
template const float &max(const float &, const float &);

#if 0
template class Point2F;
template class Point2D;
template class Point2I;

template class Point3F;
template class Point3D;
template class Point3I;

template class Point4F;
template class Point4D;
template class Point4I;


template class RMat2F;
template class RMat2D;

template class DLLAPI RMat3F;
template class DLLAPI RMat3D;

template class TMat2F;
template class TMat2D;

template class TMat3F;
template class TMat3D;

template class DLLAPI EulerF;
template class DLLAPI EulerD;

template class AngAxisF;
template class AngAxisD;

template class DLLAPI QuatF;
template class DLLAPI QuatD;

template class Box2F;
template class Box2D;
template class Box2I;

template class Box3F;
template class Box3D;
template class Box3I;

template class RectF;
template class RectD;
template class RectI;

template class CircleF;
template class CircleD;
template class CircleI;

template class SphereF;
template class SphereD;
template class SphereI;

template class PtList2F;
template class PtList2D;
template class PtList2I;

template class PtList3F;
template class PtList3D;
template class PtList3I;

template class Poly2F;
template class Poly2D;
template class Poly2I;

template class Poly3F;
template class Poly3D;
template class Poly3I;

template class PlaneF;
template class PlaneD;
template class PlaneI;

#endif
#endif

