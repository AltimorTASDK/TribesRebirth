//================================================================
//   
// $Workfile:   m_box.h  $
// $Revision:   1.6  $
// $Version$
//	
// DESCRIPTION:
//    Box classes: 2D and 3D.
//	
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _M_BOX_H_
#define _M_BOX_H_

#include "m_point.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//----------------------------------------------------------------------------
// Box Classes

class Box2F
{
public:
   Point2F    fMin;
   Point2F    fMax;

   //== Constructors and Assignment
	Box2F();
	Box2F( const Point2F& in_min, const Point2F& in_max );
	Box2F( const Point2F* in_min, const Point2F* in_max );
	Box2F( const Box2F *in_box) ;
	Box2F( float in_min_x, float in_min_y, float in_max_x, float in_max_y )
		{ fMin(in_min_x, in_min_y); fMax(in_max_x, in_max_y); }

   Box2F& operator= (const Box2F *in_box);

   //== Member access:
   Point2F & upperL();
   Point2F & lowerR();
   float & left();
   float & top();
   float & right();
   float & bottom();


   //== Arithmetic that modifies this object
   bool contains(const Point2F &in_pt) const;
   void intersect(const Box2F &in_box);

   Box2F& operator() (const Box2F *in_rect);
   Box2F& operator() (const Point2F *in_min, const Point2F *in_max);
   Box2F& operator() (float in_min_x, float in_min_y, 
      float in_max_x, float in_max_y);
   Box2F& operator-= (const Point2F *in_pt);
   Box2F& operator-= (const float in_val);
   Box2F& operator+= (const Point2F *in_pt);
   Box2F& operator+= (const float in_val);
   Box2F& operator*= (const Point2F *in_pt);
   Box2F& operator*= (const float in_val);
   Box2F& operator/= (const Point2F *in_pt);
   Box2F& operator/= (const float in_val);

	float 	len_x( void ) const;
	float 	len_y( void ) const;
};

class Box3F
{
public:
   Point3F    fMin, fMax;

   //== Constructors and Assignment
	Box3F();
	Box3F( const Point3F& in_min, const Point3F& in_max );
	Box3F( const Point3F* in_min, const Point3F* in_max );
   Box3F( const Box3F *in_box) ;

   bool contains(const Point3F &in_pt) const;
   void intersect(const Box3F &in_box);
   bool overlaps(const Box3F &in_box);

   Box3F& operator= (const Box3F *in_box);

   //== Arithmetic that modifies this object
   Box3F& operator() (const Box3F *in_rect);
   Box3F& operator() (const Point3F *in_min, const Point3F *in_max);
   Box3F& operator-= (const Point3F *in_pt);
   Box3F& operator-= (const float in_val);
   Box3F& operator+= (const Point3F *in_pt);
   Box3F& operator+= (const float in_val);
   Box3F& operator*= (const Point3F *in_pt);
   Box3F& operator*= (const float in_val);
   Box3F& operator/= (const Point3F *in_pt);
   Box3F& operator/= (const float in_val);

	float 	len_x( void ) const;
	float 	len_y( void ) const;
	float 	len_z( void ) const;
};

//----------------------------------------------------------------------------
// Box2F

inline Box2F::Box2F()
{}

inline Box2F::Box2F( const Point2F& in_min, const Point2F& in_max )
{
   fMin = in_min;
   fMin.setMin( in_max );
   fMax = in_max;
   fMax.setMax( in_min );
}

inline Box2F::Box2F(const Point2F *in_min, const Point2F *in_max)
{
   fMin = *in_min;
   fMax = *in_max;
}

inline Box2F::Box2F(const Box2F *in_box)
{
   *this = *in_box;
}

inline Point2F & Box2F::upperL()
{
   return fMin;
}

inline Point2F & Box2F::lowerR()
{
   return fMax;
}

inline float & Box2F::left()
{
   return fMin.x;
}

inline float & Box2F::top()
{
   return fMin.y;
}

inline float & Box2F::right()
{
   return fMax.x;
}

inline float & Box2F::bottom()
{
   return fMax.y;
}

inline bool Box2F::contains(const Point2F &pt) const
{
   return pt.x >= fMin.x && pt.x < fMax.x && pt.y >= fMin.y && pt.y < fMax.y;
}

inline void Box2F::intersect(const Box2F  &b2)
{
   if(b2.fMin.x > fMin.x)
      fMin.x = b2.fMin.x;
   if(b2.fMin.y > fMin.y)
      fMin.y = b2.fMin.y;
   if(b2.fMax.x < fMax.x)
      fMax.x = b2.fMax.x;
   if(b2.fMax.y < fMax.y)
      fMax.y = b2.fMax.y;
}

inline Box2F& Box2F::
   operator= (const Box2F *in_box)
{
   memcpy((void *)this, (void *)in_box, sizeof(Box2F));
   return *this;
}

inline Box2F& Box2F::
   operator() (const Box2F *in_box)
{
   *this = *in_box;
   return *this;
}

inline Box2F& Box2F::
   operator() (const Point2F *in_ul, const Point2F *in_lr)
{
   fMin = *in_ul;
   fMax = *in_lr;
   return *this;
}

inline Box2F& Box2F::
   operator() (float in_min_x, float in_min_y, float in_max_x, float in_max_y)
{
   fMin(in_min_x, in_min_y);
   fMax(in_max_x, in_max_y);
   return *this;
}

inline Box2F& Box2F::
   operator-= (const Point2F *in_pt)
{
   fMin -= *in_pt;
   fMax -= *in_pt;
   return *this;
}

inline Box2F& Box2F::
   operator-= (const float in_val)
{
   fMin -= in_val;
   fMax -= in_val;
   return *this;
}

inline Box2F& Box2F::
   operator+= (const Point2F *in_pt)
{
   fMin += *in_pt;
   fMax += *in_pt;
   return *this;
}

inline Box2F& Box2F::
   operator+= (const float in_val)
{
   fMin += in_val;
   fMax += in_val;
   return *this;
}

inline Box2F& Box2F::
   operator*= (const Point2F *in_pt)
{
   fMin.x *= in_pt->x;
   fMin.y *= in_pt->y;
   fMax.x *= in_pt->x;
   fMax.y *= in_pt->y;
   return *this;
}

inline Box2F& Box2F::
   operator*= (const float in_val)
{
   fMin.x *= in_val;
   fMin.y *= in_val;
   fMax.x *= in_val;
   fMax.y *= in_val;
   return *this;
}

inline Box2F& Box2F::
   operator/= (const Point2F *in_pt)
{
   fMin.x /= in_pt->x;
   fMin.y /= in_pt->y;
   fMax.x /= in_pt->x;
   fMax.y /= in_pt->y;
   return *this;
}

inline Box2F& Box2F::
   operator/= (const float in_val)
{
   fMin.x /= in_val;
   fMin.y /= in_val;
   fMax.x /= in_val;
   fMax.y /= in_val;
   return *this;
}

inline float Box2F::len_x( void ) const
{
	return fMax.x - fMin.x;
}

inline float Box2F::len_y( void ) const
{
	return fMax.y - fMin.y;
}

//----------------------------------------------------------------------------
// Box3F

inline Box3F::Box3F()
{}

inline Box3F::Box3F( const Point3F& in_min, const Point3F& in_max )
{
   fMin = in_min;
   fMin.setMin( in_max );
   fMax = in_max;
   fMax.setMax( in_min );
}

inline Box3F::Box3F(const Point3F *in_min, const Point3F *in_max)
{
   fMin = *in_min;
   fMax = *in_max;
}

inline Box3F::Box3F(const Box3F *in_box)
{
   *this = *in_box;
}

inline Box3F& Box3F::
   operator= (const Box3F *in_box)
{
   memcpy((void *)this, (void *)in_box, sizeof(Box3F));
   return *this;
}

inline Box3F& Box3F::
   operator() (const Box3F *in_box)
{
   *this = *in_box;
   return *this;
}

inline Box3F& Box3F::
   operator() (const Point3F *in_ul, const Point3F *in_lr)
{
   fMin = *in_ul;
   fMax = *in_lr;
   return *this;
}

inline Box3F& Box3F::
   operator-= (const Point3F *in_pt)
{
   fMin -= *in_pt;
   fMax -= *in_pt;
   return *this;
}

inline Box3F& Box3F::
   operator-= (const float in_val)
{
   fMin -= in_val;
   fMax -= in_val;
   return *this;
}

inline Box3F& Box3F::
   operator+= (const Point3F *in_pt)
{
   fMin += *in_pt;
   fMax += *in_pt;
   return *this;
}

inline Box3F& Box3F::
   operator+= (const float in_val)
{
   fMin += in_val;
   fMax += in_val;
   return *this;
}

inline Box3F& Box3F::
   operator*= (const Point3F *in_pt)
{
   fMin.x *= in_pt->x;
   fMin.y *= in_pt->y;
   fMax.x *= in_pt->x;
   fMax.y *= in_pt->y;
   return *this;
}

inline Box3F& Box3F::
   operator*= (const float in_val)
{
   fMin.x *= in_val;
   fMin.y *= in_val;
   fMax.x *= in_val;
   fMax.y *= in_val;
   return *this;
}

inline Box3F& Box3F::
   operator/= (const Point3F *in_pt)
{
   fMin.x /= in_pt->x;
   fMin.y /= in_pt->y;
   fMax.x /= in_pt->x;
   fMax.y /= in_pt->y;
   return *this;
}

inline Box3F& Box3F::
   operator/= (const float in_val)
{
   fMin.x /= in_val;
   fMin.y /= in_val;
   fMax.x /= in_val;
   fMax.y /= in_val;
   return *this;
}

inline float Box3F::len_x( void ) const
{
	return fMax.x - fMin.x;
}

inline float Box3F::len_y( void ) const
{
	return fMax.y - fMin.y;
}

inline float Box3F::len_z( void ) const
{
	return fMax.z - fMin.z;
}

inline bool Box3F::contains(const Point3F &pt) const
{
   return pt.x >= fMin.x && pt.x < fMax.x &&
          pt.y >= fMin.y && pt.y < fMax.y &&
          pt.z >= fMin.z && pt.z < fMax.z;
}

inline void Box3F::intersect(const Box3F &b2)
{
   if(b2.fMin.x > fMin.x)
      fMin.x = b2.fMin.x;
   if(b2.fMax.x < fMax.x)
      fMax.x = b2.fMax.x;
      
   if(b2.fMin.y > fMin.y)
      fMin.y = b2.fMin.y;
   if(b2.fMax.y < fMax.y)
      fMax.y = b2.fMax.y;
      
   if(b2.fMin.z > fMin.z)
      fMin.z = b2.fMin.z;
   if(b2.fMax.z < fMax.z)
      fMax.z = b2.fMax.z;
}

inline bool Box3F::overlaps(const Box3F &b2)
{
   if ( b2.fMin.x > fMax.x || b2.fMin.y > fMax.y || b2.fMin.z > fMax.z )
      return false;
   if ( fMin.x > b2.fMax.x || fMin.y > b2.fMax.y || fMin.z > b2.fMax.z )
      return false;
   return true;
}

//----------------------------------------------------------------------------
// Box Classes

class Box2I
{
public:
   Point2I    fMin;
   Point2I    fMax;

   //== Constructors and Assignment
	Box2I();
	Box2I( const Point2I& in_min, const Point2I& in_max );
	Box2I( const Point2I* in_min, const Point2I* in_max );
	Box2I( const Box2I *in_box) ;
	Box2I( Int32 in_min_x, Int32 in_min_y, Int32 in_max_x, Int32 in_max_y )
		{ fMin(in_min_x, in_min_y); fMax(in_max_x, in_max_y); }

   Box2I& operator= (const Box2I *in_box);

   //== Member access:
   Point2I & upperL();
   Point2I & lowerR();
   Int32 & left();
   Int32 & top();
   Int32 & right();
   Int32 & bottom();


   //== Arithmetic that modifies this object
   bool contains(const Point2I &in_pt) const;
   void intersect(const Box2I &in_box);
   bool overlaps(const Box2I &in_box);

   Box2I& operator() (const Box2I *in_rect);
   Box2I& operator() (const Point2I *in_min, const Point2I *in_max);
   Box2I& operator() (Int32 in_min_x, Int32 in_min_y, 
      Int32 in_max_x, Int32 in_max_y);
   Box2I& operator-= (const Point2I *in_pt);
   Box2I& operator-= (const Int32 in_val);
   Box2I& operator+= (const Point2I *in_pt);
   Box2I& operator+= (const Int32 in_val);
   Box2I& operator*= (const Point2I *in_pt);
   Box2I& operator*= (const Int32 in_val);
   Box2I& operator/= (const Point2I *in_pt);
   Box2I& operator/= (const Int32 in_val);

	Int32 	len_x( void ) const;
	Int32 	len_y( void ) const;
};

class Box3I
{
public:
   Point3I    fMin, fMax;

   //== Constructors and Assignment
	Box3I();
	Box3I( const Point3I& in_min, const Point3I& in_max );
	Box3I( const Point3I* in_min, const Point3I* in_max );
   Box3I( const Box3I *in_box) ;

   bool contains(const Point3I &in_pt) const;

   Box3I& operator= (const Box3I *in_box);

   //== Arithmetic that modifies this object
   Box3I& operator() (const Box3I *in_rect);
   Box3I& operator() (const Point3I *in_min, const Point3I *in_max);
   Box3I& operator-= (const Point3I *in_pt);
   Box3I& operator-= (const Int32 in_val);
   Box3I& operator+= (const Point3I *in_pt);
   Box3I& operator+= (const Int32 in_val);
   Box3I& operator*= (const Point3I *in_pt);
   Box3I& operator*= (const Int32 in_val);
   Box3I& operator/= (const Point3I *in_pt);
   Box3I& operator/= (const Int32 in_val);

	Int32 	len_x( void ) const;
	Int32 	len_y( void ) const;
	Int32 	len_z( void ) const;
};

//----------------------------------------------------------------------------
// Box2I

inline Box2I::Box2I()
{}

inline Box2I::Box2I( const Point2I& in_min, const Point2I& in_max )
{
   fMin = in_min;
   fMin.setMin( in_max );
   fMax = in_max;
   fMax.setMax( in_min );
}

inline Box2I::Box2I(const Point2I *in_min, const Point2I *in_max)
{
   fMin = *in_min;
   fMax = *in_max;
}

inline Box2I::Box2I(const Box2I *in_box)
{
   *this = *in_box;
}

inline Point2I & Box2I::upperL()
{
   return fMin;
}

inline Point2I & Box2I::lowerR()
{
   return fMax;
}

inline Int32 & Box2I::left()
{
   return fMin.x;
}

inline Int32 & Box2I::top()
{
   return fMin.y;
}

inline Int32 & Box2I::right()
{
   return fMax.x;
}

inline Int32 & Box2I::bottom()
{
   return fMax.y;
}

inline bool Box2I::contains(const Point2I &pt) const
{
   return pt.x >= fMin.x && pt.x < fMax.x && pt.y >= fMin.y && pt.y < fMax.y;
}

inline void Box2I::intersect(const Box2I  &b2)
{
   if(b2.fMin.x > fMin.x)
      fMin.x = b2.fMin.x;
   if(b2.fMin.y > fMin.y)
      fMin.y = b2.fMin.y;
   if(b2.fMax.x < fMax.x)
      fMax.x = b2.fMax.x;
   if(b2.fMax.y < fMax.y)
      fMax.y = b2.fMax.y;
}

inline bool Box2I::overlaps(const Box2I  &b2)
{
   return ! ( b2.fMin.x > fMax.x || b2.fMin.y > fMax.y 
               || fMin.x > b2.fMax.x || fMin.y > b2.fMax.y );
}

inline Box2I& Box2I::
   operator= (const Box2I *in_box)
{
   memcpy((void *)this, (void *)in_box, sizeof(Box2I));
   return *this;
}

inline Box2I& Box2I::
   operator() (const Box2I *in_box)
{
   *this = *in_box;
   return *this;
}

inline Box2I& Box2I::
   operator() (const Point2I *in_ul, const Point2I *in_lr)
{
   fMin = *in_ul;
   fMax = *in_lr;
   return *this;
}

inline Box2I& Box2I::
   operator() (Int32 in_min_x, Int32 in_min_y, Int32 in_max_x, Int32 in_max_y)
{
   fMin(in_min_x, in_min_y);
   fMax(in_max_x, in_max_y);
   return *this;
}

inline Box2I& Box2I::
   operator-= (const Point2I *in_pt)
{
   fMin -= *in_pt;
   fMax -= *in_pt;
   return *this;
}

inline Box2I& Box2I::
   operator-= (const Int32 in_val)
{
   fMin -= in_val;
   fMax -= in_val;
   return *this;
}

inline Box2I& Box2I::
   operator+= (const Point2I *in_pt)
{
   fMin += *in_pt;
   fMax += *in_pt;
   return *this;
}

inline Box2I& Box2I::
   operator+= (const Int32 in_val)
{
   fMin += in_val;
   fMax += in_val;
   return *this;
}

inline Box2I& Box2I::
   operator*= (const Point2I *in_pt)
{
   fMin.x *= in_pt->x;
   fMin.y *= in_pt->y;
   fMax.x *= in_pt->x;
   fMax.y *= in_pt->y;
   return *this;
}

inline Box2I& Box2I::
   operator*= (const Int32 in_val)
{
   fMin.x *= in_val;
   fMin.y *= in_val;
   fMax.x *= in_val;
   fMax.y *= in_val;
   return *this;
}

inline Box2I& Box2I::
   operator/= (const Point2I *in_pt)
{
   fMin.x /= in_pt->x;
   fMin.y /= in_pt->y;
   fMax.x /= in_pt->x;
   fMax.y /= in_pt->y;
   return *this;
}

inline Box2I& Box2I::
   operator/= (const Int32 in_val)
{
   fMin.x /= in_val;
   fMin.y /= in_val;
   fMax.x /= in_val;
   fMax.y /= in_val;
   return *this;
}

inline Int32 Box2I::len_x( void ) const
{
	return fMax.x - fMin.x;
}

inline Int32 Box2I::len_y( void ) const
{
	return fMax.y - fMin.y;
}

//----------------------------------------------------------------------------
// Box3I

inline Box3I::Box3I()
{}

inline Box3I::Box3I( const Point3I& in_min, const Point3I& in_max )
{
   fMin = in_min;
   fMin.setMin( in_max );
   fMax = in_max;
   fMax.setMax( in_min );
}

inline Box3I::Box3I(const Point3I *in_min, const Point3I *in_max)
{
   fMin = *in_min;
   fMax = *in_max;
}

inline Box3I::Box3I(const Box3I *in_box)
{
   *this = *in_box;
}

inline Box3I& Box3I::
   operator= (const Box3I *in_box)
{
   memcpy((void *)this, (void *)in_box, sizeof(Box3I));
   return *this;
}

inline Box3I& Box3I::
   operator() (const Box3I *in_box)
{
   *this = *in_box;
   return *this;
}

inline Box3I& Box3I::
   operator() (const Point3I *in_ul, const Point3I *in_lr)
{
   fMin = *in_ul;
   fMax = *in_lr;
   return *this;
}

inline Box3I& Box3I::
   operator-= (const Point3I *in_pt)
{
   fMin -= *in_pt;
   fMax -= *in_pt;
   return *this;
}

inline Box3I& Box3I::
   operator-= (const Int32 in_val)
{
   fMin -= in_val;
   fMax -= in_val;
   return *this;
}

inline Box3I& Box3I::
   operator+= (const Point3I *in_pt)
{
   fMin += *in_pt;
   fMax += *in_pt;
   return *this;
}

inline Box3I& Box3I::
   operator+= (const Int32 in_val)
{
   fMin += in_val;
   fMax += in_val;
   return *this;
}

inline Box3I& Box3I::
   operator*= (const Point3I *in_pt)
{
   fMin.x *= in_pt->x;
   fMin.y *= in_pt->y;
   fMax.x *= in_pt->x;
   fMax.y *= in_pt->y;
   return *this;
}

inline Box3I& Box3I::
   operator*= (const Int32 in_val)
{
   fMin.x *= in_val;
   fMin.y *= in_val;
   fMax.x *= in_val;
   fMax.y *= in_val;
   return *this;
}

inline Box3I& Box3I::
   operator/= (const Point3I *in_pt)
{
   fMin.x /= in_pt->x;
   fMin.y /= in_pt->y;
   fMax.x /= in_pt->x;
   fMax.y /= in_pt->y;
   return *this;
}

inline Box3I& Box3I::
   operator/= (const Int32 in_val)
{
   fMin.x /= in_val;
   fMin.y /= in_val;
   fMax.x /= in_val;
   fMax.y /= in_val;
   return *this;
}

inline Int32 Box3I::len_x( void ) const
{
	return fMax.x - fMin.x;
}

inline Int32 Box3I::len_y( void ) const
{
	return fMax.y - fMin.y;
}

inline Int32 Box3I::len_z( void ) const
{
	return fMax.z - fMin.z;
}

inline bool Box3I::contains(const Point3I &pt) const
{
   return pt.x >= fMin.x && pt.x < fMax.x &&
          pt.y >= fMin.y && pt.y < fMax.y &&
          pt.z >= fMin.z && pt.z < fMax.z;
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif


