//================================================================
//   
// $Workfile:   m_mat.h  $
// $Revision:   1.5  $
// $Version$
//	
// DESCRIPTION:
//    RMat and TMat classes.
//	
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _M_MAT2_H_    
#define _M_MAT2_H_

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class EulerF;
class AngAxisF;
class QuatF;

//---------------------------------------------------------------------------
// Notes:
//
// - matrix element naming convention: m[row][col]
//
// - TMat does rotation before translation
//
// - Beware of meddling with matrix members directly

//---------------------------------------------------------------------------
// RMat classes:

class RMat2F
   {
public:
   enum 
      {
      Matrix_HasRotation = 1,
      Matrix_HasTranslation = 2,
      Matrix_HasScale = 4,
      };

   Int32 flags;
   float  m[2][2];

	RMat2F();

	RMat2F& set( const float rotAngle );
	RMat2F& transpose( void );
	RMat2F& inverse( void );
	RMat2F& identity( void );
   float trace( void ) const;
   RMat2F& preScale( Point2F const & p );
   RMat2F& postScale( Point2F const & p );
   RMat2F& preRotate( float rotAngle );
   RMat2F& postRotate( float s, float c );
   RMat2F& postRotate( float rotAngle );
   };

//---------------------------------------------------------------------------
// TMat classes:

class TMat2F : public RMat2F
   {
public:
	Point2F  p;

	TMat2F();
	TMat2F( Bool ident );

	TMat2F& set( const float rotAngle, const Point2F& );
	TMat2F& inverse( void );
	TMat2F& identity( void );
   TMat2F& preScale( Point2F const & p );
   TMat2F& postScale( Point2F const & p );
   TMat2F& preTranslate( Point2F const & p );
   TMat2F& postTranslate( Point2F const & p );
   TMat2F& preRotate( float rotAngle );
   TMat2F& postRotate( float rotAngle );
   };

//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif


//---------------------------------------------------------------------------
// Implementation:

//#include <m_dot.h>

//---------------------------------------------------------------------------
//
// - RMat2F
//

inline RMat2F::RMat2F()
   {
   // Beware: no initialization is done!
   }

inline RMat2F& RMat2F::set( const float rotAngle )
   {
//;  cos(z)   sin(z)
//;  -sin(z)  cos(z)
   m[0][0] = m[1][1] = m_cos( rotAngle );
   m[0][1] = m_sin( rotAngle );
   m[1][0] = - m[0][1];

   flags &= ~Matrix_HasScale;
   flags |= Matrix_HasRotation;

   return *this;
   }

inline RMat2F& RMat2F::transpose( void )
   {
   float temp = m[0][1];
   m[0][1] = m[1][0];
   m[1][0] = temp;
   return *this;
   }

inline RMat2F& RMat2F::inverse( void )
   {
   if( flags & Matrix_HasScale )
      {
      float sx = m[0][0] * m[0][0] + m[0][1] * m[0][1];
      float sy = m[1][0] * m[1][0] + m[1][1] * m[1][1];
      m[0][0] /= sx;
      m[1][1] /= sy;
      float temp = m[0][1] / sy;
      m[0][1] = m[1][0] / sx;
      m[1][0] = temp;
      }
   else
      transpose();
   return *this;
   }

inline RMat2F& RMat2F::identity( void )
   {
   m[0][1] = m[1][0] = float(0.0);
	m[0][0] = m[1][1] = float(1.0);
   flags &= ~(Matrix_HasRotation | Matrix_HasScale);
	return *this;
   }

inline float RMat2F::trace( void ) const
   {
   return m[0][0] + m[1][1];
   }

inline RMat2F& RMat2F::preScale( Point2F const & pt )
   {
   m[0][0] *= pt.x;
   m[0][1] *= pt.x;
   m[1][0] *= pt.y;
   m[1][1] *= pt.y;
   flags |= Matrix_HasScale;
	return *this;
   }

inline RMat2F& RMat2F::postScale( Point2F const & pt )
   {
   m[0][0] *= pt.x;
   m[0][1] *= pt.y;
   m[1][0] *= pt.x;
   m[1][1] *= pt.y;
   flags |= Matrix_HasScale;
	return *this;
   }

inline RMat2F& RMat2F::preRotate( float rotAngle )
   {
   float c,s,t;
   m_sincos( rotAngle, &s, &c );
   t = m[0][0];
   m[0][0] = t * c + m[1][0] * s;
   m[1][0] = t * (-s) + m[1][0] * c;
   t = m[0][1];
   m[0][1] = t * c + m[1][1] * s;
   m[1][1] = t * (-s) + m[1][1] * c;
   flags |= Matrix_HasRotation;
   return *this;
   }

inline RMat2F& RMat2F::postRotate( float s, float c )
   {
   float t;
   t = m[0][0];
   m[0][0] = t * c - m[0][1] * s;
   m[0][1] = t * s + m[0][1] * c;
   t = m[1][0];
   m[1][0] = t * c - m[1][1] * s;
   m[1][1] = t * s + m[1][1] * c;
   flags |= Matrix_HasRotation;
   return *this;
   }

inline RMat2F& RMat2F::postRotate( float rotAngle )
   {
   float c,s;
   m_sincos( rotAngle, &s, &c );
   return postRotate( s, c );
   }


//---------------------------------------------------------------------------
//
// - TMat2F
//

inline TMat2F::TMat2F()
   {
   // Beware: no initialization is done!
   }

inline TMat2F::TMat2F( Bool ident )
   {
   if( ident )
      identity();
   }

inline TMat2F& TMat2F::
   set( const float rotAngle, const Point2F& t )
   {
	p = t;
	RMat2F::set( rotAngle );
   flags |= Matrix_HasTranslation;
	return *this;
   }

inline TMat2F& TMat2F::inverse( void )
   {
	RMat2F::inverse();
	p.neg();
	Point2F p2 = p;
   m_mul( p2, *((RMat2F*)this), &p );
	return *this;
   }

inline TMat2F& TMat2F::identity( void )
   {
   RMat2F::identity();
   p.set();
   flags &= ~Matrix_HasTranslation;
	return *this;
   }

inline TMat2F& TMat2F::preScale( Point2F const & pt )
   {
	RMat2F::preScale( pt );
	return *this;
   }

inline TMat2F& TMat2F::postScale( Point2F const & pt )
   {
	RMat2F::postScale( pt );
   p.x *= pt.x;
   p.y *= pt.y;
	return *this;
   }

inline TMat2F& TMat2F::preTranslate( Point2F const & pt )
   {
   Point2F temp;
   m_mul( pt, *(RMat2F*)this, &temp );
   p += temp;
   flags |= Matrix_HasTranslation;
	return *this;
   }

inline TMat2F& TMat2F::postTranslate( Point2F const & pt )
   {
   p += pt;
   flags |= Matrix_HasTranslation;
	return *this;
   }
inline TMat2F& TMat2F::preRotate( float rotAngle )
   {
   RMat2F::preRotate( rotAngle );
   return *this;
   }

inline TMat2F& TMat2F::postRotate( float rotAngle )
   {
   float c,s,t;
   m_sincos( rotAngle, &s, &c );
   RMat2F::postRotate( s, c );
   t = p.x;
   p.x = t * c + p.y * s;
   p.y = t * s + p.y * c;
	return *this;
   }

//---------------------------------------------------------------------------
#endif
