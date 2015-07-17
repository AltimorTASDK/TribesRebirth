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

#ifndef _M_MAT3_H_    
#define _M_MAT3_H_

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

class RMat3F
   {
public:
   enum 
      {
      Matrix_HasRotation = 1,
      Matrix_HasTranslation = 2,
      Matrix_HasScale = 4,
      };

   Int32 flags;
   float  m[3][3];

	RMat3F();
	RMat3F( Bool ident );
	RMat3F( EulerF const & e );
	RMat3F( AngAxisF const & aa );
	RMat3F( QuatF const & q );
	
   RMat3F& set( EulerF const & euler );
   RMat3F& set( AngAxisF const & aa );
   RMat3F& set( QuatF const & quat );

   void getColumn(int index, Point3F *column) const;
	void getRow(int index, Point3F *row) const;
	void setColumn(int index, Point3F &column);
	void setRow(int index, Point3F &row);

	void pitch( float angle );
	void yaw( float angle );
	void roll( float angle );
	void normalize(void);
	RMat3F& transpose( void );
	RMat3F& inverse( void );
   float determinant( void ) const;
   RMat3F& adjoint( RMat3F * pDest ) const;
	RMat3F& identity( void );
	EulerF*  angles( EulerF* rotAngles ) const;
   float trace( void ) const;
   RMat3F& preScale( Point3F const & p );
   RMat3F& postScale( Point3F const & p );
   RMat3F& preRotateX( float rotAngle );
   RMat3F& postRotateX( float s, float c );
   RMat3F& postRotateX( float rotAngle );
   RMat3F& preRotateY( float rotAngle );
   RMat3F& postRotateY( float s, float c );
   RMat3F& postRotateY( float rotAngle );
   RMat3F& preRotateZ( float rotAngle );
   RMat3F& postRotateZ( float s, float c );
   RMat3F& postRotateZ( float rotAngle );
   };

//---------------------------------------------------------------------------
// TMat classes:

class TMat3F : public RMat3F
   {
public:
	Point3F   p;

	TMat3F();
	TMat3F( Bool ident );
	TMat3F( EulerF const & e, Point3F const & p );
	TMat3F( AngAxisF const & aa, Point3F const & p );
	TMat3F( QuatF const & q, Point3F const & p );

   TMat3F& set( EulerF const & euler, Point3F const & p );
   TMat3F& set( AngAxisF const & aa, Point3F const & p );
   TMat3F& set( QuatF const & quat, Point3F const & p );

	TMat3F& inverse( void );
	TMat3F& identity( void );
   TMat3F& preScale( Point3F const & p );
   TMat3F& postScale( Point3F const & p );
   TMat3F& preTranslate( Point3F const & p );
   TMat3F& postTranslate( Point3F const & p );
   TMat3F& preRotateX( float rotAngle );
   TMat3F& postRotateX( float rotAngle );
   TMat3F& preRotateY( float rotAngle );
   TMat3F& postRotateY( float rotAngle );
   TMat3F& preRotateZ( float rotAngle );
   TMat3F& postRotateZ( float rotAngle );
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

//---------------------------------------------------------------------------
//
// - RMat3F
//

inline RMat3F::RMat3F()
   {
   // Beware: no initialization is done!
   }

inline RMat3F::RMat3F( Bool ident )
   {
   if( ident )
      identity();
   }

inline RMat3F::RMat3F( EulerF const & euler )
   {
   set( euler );
   }

inline RMat3F::RMat3F( AngAxisF const & aa )
   {
   set( aa );
   }

inline RMat3F::RMat3F( QuatF const & quat )
   {
   set( quat );
   }

inline RMat3F& RMat3F::set( EulerF const & e )
   {
   return *e.makeMatrix( this );
   }

inline RMat3F& RMat3F::set( AngAxisF const & a )
   {
   return *a.makeMatrix( this );
   }

inline RMat3F& RMat3F::set( QuatF const & q )
   {
   return *q.makeMatrix( this );
   }

inline EulerF* RMat3F::angles( EulerF* pe ) const
   {
   pe->set( *this );
   return pe;
   }

inline RMat3F& RMat3F::transpose( void )
   {
   float temp;

   temp = m[0][1];
   m[0][1] = m[1][0];
   m[1][0] = temp;

   temp = m[0][2];
   m[0][2] = m[2][0];
   m[2][0] = temp;

   temp = m[1][2];
   m[1][2] = m[2][1];
   m[2][1] = temp;

   return *this;
   }

inline float RMat3F::determinant( void ) const
   {
   return 
      m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) +
      m[1][0] * (m[0][2] * m[2][1] - m[0][1] * m[2][2]) +
      m[2][0] * (m[0][1] * m[1][2] - m[0][2] * m[1][1]) ;
   }

inline RMat3F& RMat3F::adjoint( RMat3F * pDest ) const
{
   pDest->m[0][0] = m[1][1] * m[2][2] - m[1][2] * m[2][1];
   pDest->m[0][1] = m[2][1] * m[0][2] - m[2][2] * m[0][1];
   pDest->m[0][2] = m[0][1] * m[1][2] - m[0][2] * m[1][1];

   pDest->m[1][0] = m[1][2] * m[2][0] - m[1][0] * m[2][2];
   pDest->m[1][1] = m[2][2] * m[0][0] - m[2][0] * m[0][2];
   pDest->m[1][2] = m[0][2] * m[1][0] - m[0][0] * m[1][2];

   pDest->m[2][0] = m[1][0] * m[2][1] - m[1][1] * m[2][0];
   pDest->m[2][1] = m[2][0] * m[0][1] - m[2][1] * m[0][0];
   pDest->m[2][2] = m[0][0] * m[1][1] - m[0][1] * m[1][0];
   return *pDest;
}

inline RMat3F& RMat3F::inverse( void )
   {
   if( flags & Matrix_HasScale )
      {
      RMat3F adj;
      adjoint( &adj );
      float det = determinant();
      if( !IsZero( det ) )
         {
         float invDet = float(1.0) / det;
         m[0][0] = adj.m[0][0] * invDet;
         m[0][1] = adj.m[0][1] * invDet;
         m[0][2] = adj.m[0][2] * invDet;
         m[1][0] = adj.m[1][0] * invDet;
         m[1][1] = adj.m[1][1] * invDet;
         m[1][2] = adj.m[1][2] * invDet;
         m[2][0] = adj.m[2][0] * invDet;
         m[2][1] = adj.m[2][1] * invDet;
         m[2][2] = adj.m[2][2] * invDet;
         }
      else
         {
         AssertWarn( 0, "non-singular matrix, no inverse" );
         }
      }
   else
      transpose();

   return *this;
   }

inline void RMat3F::normalize()
   {
	Point3F c1, c2, c3;
	getColumn(0, &c1);
	getColumn(1, &c2);
	m_cross(c1, c2, &c3);
	m_cross(c3, c1, &c2);
	c1.normalize(float(1.0));
	c2.normalize(float(1.0));
	c3.normalize(float(1.0));
	setColumn(0, c1);
	setColumn(1, c2);
	setColumn(2, c3);
   flags &= ~Matrix_HasScale;
   }

inline RMat3F& RMat3F::identity( void )
   {
	m[0][1] = m[0][2] = m[1][0] = m[2][0] = m[1][2] = m[2][1] = float(0.0);
	m[0][0] = m[1][1] = m[2][2] = float(1.0);
   flags &= ~(Matrix_HasRotation | Matrix_HasScale);
	return *this;
   }


inline float RMat3F::trace( void ) const
   {
   return m[0][0] + m[1][1] + m[2][2];
   }

inline RMat3F& RMat3F::preScale( Point3F const & pt )
   {
   m[0][0] *= pt.x;
   m[0][1] *= pt.x;
   m[0][2] *= pt.x;

   m[1][0] *= pt.y;
   m[1][1] *= pt.y;
   m[1][2] *= pt.y;

   m[2][0] *= pt.z;
   m[2][1] *= pt.z;
   m[2][2] *= pt.z;

   flags |= Matrix_HasScale;
	return *this;
   }

inline RMat3F& RMat3F::postScale( Point3F const & pt )
   {
   m[0][0] *= pt.x;
   m[0][1] *= pt.y;
   m[0][2] *= pt.z;

   m[1][0] *= pt.x;
   m[1][1] *= pt.y;
   m[1][2] *= pt.z;

   m[2][0] *= pt.x;
   m[2][1] *= pt.y;
   m[2][2] *= pt.z;

   flags |= Matrix_HasScale;
	return *this;
   }

inline RMat3F& RMat3F::preRotateX( float rotAngle )
   {
   float c,s,t;
   m_sincos( rotAngle, &s, &c );
   t = m[1][0];
   m[1][0] = t * c    + m[2][0] * s;
   m[2][0] = t * (-s) + m[2][0] * c;

   t = m[1][1];
   m[1][1] = t * c    + m[2][1] * s;
   m[2][1] = t * (-s) + m[2][1] * c;

   t = m[1][2];
   m[1][2] = t * c    + m[2][2] * s;
   m[2][2] = t * (-s) + m[2][2] * c;

   flags |= Matrix_HasRotation;
   return *this;
   }

inline RMat3F& RMat3F::postRotateX( float s, float c )
   {
   float t;
   t = m[0][1];
   m[0][1] = t * c - m[0][2] * s;
   m[0][2] = t * s + m[0][2] * c;

   t = m[1][1];
   m[1][1] = t * c - m[1][2] * s;
   m[1][2] = t * s + m[1][2] * c;

   t = m[2][1];
   m[2][1] = t * c - m[2][2] * s;
   m[2][2] = t * s + m[2][2] * c;

   flags |= Matrix_HasRotation;
   return *this;
   }

inline RMat3F& RMat3F::postRotateX( float rotAngle )
   {
   float c,s;
   m_sincos( rotAngle, &s, &c );
   return postRotateX( s, c );
   }

inline RMat3F& RMat3F::preRotateY( float rotAngle )
   {
   float c,s,t;
   m_sincos( rotAngle, &s, &c );
   t = m[0][0];
   m[0][0] = t * c - m[2][0] * s;
   m[2][0] = t * s + m[2][0] * c;

   t = m[0][1];
   m[0][1] = t * c - m[2][1] * s;
   m[2][1] = t * s + m[2][1] * c;

   t = m[0][2];
   m[0][2] = t * c - m[2][2] * s;
   m[2][2] = t * s + m[2][2] * c;

   flags |= Matrix_HasRotation;
   return *this;
   }

inline RMat3F& RMat3F::postRotateY( float s, float c )
   {
   float t;
   t = m[0][0];
   m[0][0] = t * c    + m[0][2] * s;
   m[0][2] = t * (-s) + m[0][2] * c;

   t = m[1][0];
   m[1][0] = t * c    + m[1][2] * s;
   m[1][2] = t * (-s) + m[1][2] * c;

   t = m[2][0];
   m[2][0] = t * c    + m[2][2] * s;
   m[2][2] = t * (-s) + m[2][2] * c;

   flags |= Matrix_HasRotation;
   return *this;
   }

inline RMat3F& RMat3F::postRotateY( float rotAngle )
   {
   float c,s;
   m_sincos( rotAngle, &s, &c );
   return postRotateY( s, c );
   }

inline RMat3F& RMat3F::preRotateZ( float rotAngle )
   {
   float c,s,t;
   m_sincos( rotAngle, &s, &c );
   t = m[0][0];
   m[0][0] = t * c    + m[1][0] * s;
   m[1][0] = t * (-s) + m[1][0] * c;

   t = m[0][1];
   m[0][1] = t * c    + m[1][1] * s;
   m[1][1] = t * (-s) + m[1][1] * c;

   t = m[0][2];
   m[0][2] = t * c    + m[1][2] * s;
   m[1][2] = t * (-s) + m[1][2] * c;

   flags |= Matrix_HasRotation;
   return *this;
   }

inline RMat3F& RMat3F::postRotateZ( float s, float c )
   {
   float t;
   t = m[0][0];
   m[0][0] = t * c - m[0][1] * s;
   m[0][1] = t * s + m[0][1] * c;

   t = m[1][0];
   m[1][0] = t * c - m[1][1] * s;
   m[1][1] = t * s + m[1][1] * c;

   t = m[2][0];
   m[2][0] = t * c - m[2][1] * s;
   m[2][1] = t * s + m[2][1] * c;

   flags |= Matrix_HasRotation;
   return *this;
   }

inline RMat3F& RMat3F::postRotateZ( float rotAngle )
   {
   float c,s;
   m_sincos( rotAngle, &s, &c );
   return postRotateZ( s, c );
   }

inline void RMat3F::getColumn(int index, Point3F *column) const
	{
	column->x = m[0][index];	
	column->y = m[1][index];	
	column->z = m[2][index];
	};

inline void RMat3F::getRow(int index, Point3F *row) const
	{
	row->x = m[index][0];	
	row->y = m[index][1];	
   row->z = m[index][2];
	};

inline void RMat3F::setColumn(int index, Point3F &column)
	{
	m[0][index] = column.x;
	m[1][index] = column.y;
	m[2][index] = column.z;
	};

inline void RMat3F::setRow(int index, Point3F &row)
	{
	m[index][0] = row.x;
	m[index][1] = row.y;
	m[index][2] = row.z;
	};

//---------------------------------------------------------------------------
//
// - TMat3
//

inline TMat3F::TMat3F()
   {
   // Beware: no initialization is done!
   }

inline TMat3F::TMat3F( Bool ident )
   {
   if( ident )
      identity();
   }

inline TMat3F::TMat3F( EulerF const & euler, Point3F const & p )
   {
   set( euler, p );
   }

inline TMat3F::TMat3F( AngAxisF const & aa, Point3F const & p )
   {
   set( aa, p );
   }

inline TMat3F::TMat3F( QuatF const & quat, Point3F const & p )
   {
   set( quat, p );
   }

inline TMat3F& TMat3F::
   set( EulerF const & e, Point3F const & t )
   {
	p = t;
	RMat3F::set( e );

   flags |= Matrix_HasTranslation;
	return *this;
   }

inline TMat3F& TMat3F::
   set( AngAxisF const & aa, Point3F const & t )
   {
	p = t;
	RMat3F::set( aa );

   flags |= Matrix_HasTranslation;
	return *this;
   }

inline TMat3F& TMat3F::
   set( QuatF const & q, Point3F const & t )
   {
	p = t;
	RMat3F::set( q );

   flags |= Matrix_HasTranslation;
	return *this;
   }

inline TMat3F& TMat3F::inverse( void )
   {
	RMat3F::inverse();
//   if( flags & Matrix_HasTranslation )
//     {
   	p.neg();
	   Point3F p2 = p;
      m_mul( p2, *((RMat3F*)this), &p );
//      }
	return *this;
   }

inline TMat3F& TMat3F::identity( void )
   {
	RMat3F::identity();
   p.set();
   flags &= ~Matrix_HasTranslation;
	return *this;
   }

inline TMat3F& TMat3F::preScale( Point3F const & pt )
   {
	RMat3F::preScale( pt );
	return *this;
   }

inline TMat3F& TMat3F::postScale( Point3F const & pt )
   {
	RMat3F::postScale( pt );
   p.x *= pt.x;
   p.y *= pt.y;
   p.z *= pt.z;
	return *this;
   }

inline TMat3F& TMat3F::preTranslate( Point3F const & pt )
   {
   Point3F temp;
   m_mul( pt, *(RMat3F*)this, &temp );
   p += temp;
   flags |= Matrix_HasTranslation;
	return *this;
   }

inline TMat3F& TMat3F::postTranslate( Point3F const & pt )
   {
   p += pt;
   flags |= Matrix_HasTranslation;
	return *this;
   }

inline TMat3F& TMat3F::preRotateX( float rotAngle )
   {
   RMat3F::preRotateX( rotAngle );
	return *this;
   }

inline TMat3F& TMat3F::postRotateX( float rotAngle )
   {
   float c,s,t;
   m_sincos( rotAngle, &s, &c );
   RMat3F::postRotateX( s, c );
   t = p.y;
   p.y = t * c - p.z * s;
   p.z = t * s + p.z * c;
	return *this;
   }

inline TMat3F& TMat3F::preRotateY( float rotAngle )
   {
   RMat3F::preRotateY( rotAngle );
	return *this;
   }

inline TMat3F& TMat3F::postRotateY( float rotAngle )
   {
   float c,s,t;
   m_sincos( rotAngle, &s, &c );
   RMat3F::postRotateY( s, c );
   t = p.x;
   p.x = t * c + p.z * s;
   p.z = t * (-s) + p.z * c;
	return *this;
   }

inline TMat3F& TMat3F::preRotateZ( float rotAngle )
   {
   RMat3F::preRotateZ( rotAngle );
	return *this;
   }

inline TMat3F& TMat3F::postRotateZ( float rotAngle )
   {
   float c,s,t;
   m_sincos( rotAngle, &s, &c );
   RMat3F::postRotateZ( s, c );
   t = p.x;
   p.x = t * c - p.y * s;
   p.y = t * s + p.y * c;
	return *this;
   }

#endif
