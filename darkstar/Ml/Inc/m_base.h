//================================================================
//   
// $Workfile:   m_base.h  $
// $Revision:   1.11  $
// $Version$
//	$Date:   14 Dec 1995 18:58:42  $
//	
// DESCRIPTION:
//    Some basic arithmetic functions.
//	
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================
#include <base.h>
 
#ifndef _M_BASE_H_
#define _M_BASE_H_

//----------------------------------------------------------------------------
// ASM functions
//----------------------------------------------------------------------------

extern "C" {
   void     __fastcall m_RealF_to_RealX( const RealF &, RealX * );
   void     __fastcall m_RealF_to_Int32( const RealF &, Int32 * );
   Int32	   __fastcall m_mulf16( const Fix16, const Int32 );
   Fix16	   __fastcall m_mulff16( const Fix16 a, const Fix16 b );
   Fix16    __fastcall m_divff16( const Fix16 numer, const Fix16 denom );
   Int32	   __fastcall m_mulf16R( const Fix16, const Int32 );
   Fix16	   __fastcall m_mulff16R( const Fix16 a, const Fix16 b );
   Fix16    __cdecl m_divff16R( const Fix16 numer, const Fix16 denom );


	Int64		__cdecl m_mul32( const Int32, const Int32 );
	div_t32	__cdecl m_div32( const Int64 & numer, const Int32 denom );
	Int32 	__cdecl m_muldiv32(Int32,Int32,Int32);
	Int32 	__cdecl m_muludiv32(Int32,Int32,UInt32);

	Int32    __cdecl m_sqrt_( const Int32 );

	float    __cdecl m_sqrt_RealF( const float a);
   float	   __cdecl m_sqrtf_RealF( const float a );
   float	   __cdecl m_invsqrtf_RealF( const float a );
   float	   __cdecl m_log2_RealF( const float a );
   float	   __cdecl m_log2f_RealF( const float a );
   float	   __cdecl m_pow2f_RealF( const float a );
   float	   __cdecl m_fog_RealF( const float w, const float d );

	Int32    __cdecl m_log2_( const Int32 );

	Int32		__cdecl m_bsr_( const Int32 );
	Int32		__cdecl m_bsf_( const Int32 );

	Int32		__cdecl m_map32( const Int32 dval, const Int32 dmin, const Int32 dmax,
					const Int32 rmin, const Int32 rmax);

	Int16		__cdecl m_fpuGetControlState();
	void		__cdecl m_fpuSetControlState(Int16 state);

};

//----------------------------------------------------------------------------
// ASM functions
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Basic type multiply and divide

inline Int64 m_mul( const Int32 a, const Int32 b )
{
	return m_mul32( a, b );
}

inline div_t32	m_div( const Int64 & numer, const Int32 denom )
{
	return m_div32( numer, denom );
}

inline int m_muldiv( const int numer, const int multp, const int denom )
{
   // Uses a 64bit intermediate value (numer * multp) / denom;
	return m_muldiv32(numer,multp,denom);
}

inline Int32 m_muldiv( const Int32 numer, const Int32 multp, const Int32 denom )
{
   // Uses a 64bit intermediate value (numer * multp) / denom;
	return m_muldiv32(numer,multp,denom);
}

inline Int32 m_muludiv( const Int32 numer, const Int32 multp, const UInt32 denom )
{
   // Uses a 64bit intermediate value (numer * multp) / denom;
	return m_muludiv32(numer,multp,denom);
}

inline Int32 m_mulf16( const Fix16 frac, const int val )
{
   return m_mulf16( frac, (Int32)val );
}

inline Int32 m_mulf16( const Fix16 frac, const unsigned int val )
{
   return m_mulf16( frac, (Int32)val );
}

#ifndef M_NO_FLOAT
inline RealF m_muldiv( const RealF numer, const RealF multp, const RealF denom )
{
   return (RealF)(numer * multp / denom);
}

inline RealF m_mulf16( const Fix16 frac, const RealF val )
{
   return (RealF)(val * frac / 65536.0);
}
#endif

#ifndef M_NO_DOUBLE
inline RealD m_muldiv( const RealD numer, const RealD multp, const RealD denom )
{
   return (RealD)(numer * multp / denom);
}

inline RealD m_mulf16( const Fix16 frac, const RealD val )
{
   return (RealD)(val * frac / 65536.0);
}
#endif

//----------------------------------------------------------------------------
// Square root, log 

#ifndef M_NO_FLOAT

inline RealF m_abs( const RealF val )
{
   return RealF(fabs(val));
}

inline RealF m_sqr( const RealF val )
{
   return (RealF)( val*val );
}

inline RealF m_sqrt( const RealF val )
{
   AssertFatal( val >= RealF(0), "m_sqrt: domain error" );
   return (RealF) m_sqrt_RealF( val );
}

inline RealF m_sqrtf( const RealF val )
{
   AssertFatal( val >= RealF(0), "m_sqrtf: domain error" );
   return m_sqrtf_RealF( val );
}

inline RealF m_log2( const RealF val )
{
   AssertFatal( val >= RealF(0), "m_log2: domain error" );
   return m_log2_RealF( val );
}

inline int asInt(float f) 
{
   return *(int*)&f;
}
inline int asFloat(int i) 
{
   return *(float*)&i;
}

inline RealF m_log2f( const RealF val )
{
   AssertFatal( val >= RealF(0), "m_log2f: domain error" );
   return m_log2f_RealF( val );
}

inline RealF m_pow2( const RealF val )
{
   return (RealF)(pow(2.0, val));
}

inline RealF m_pow2f( const RealF val )
{
   return m_pow2f_RealF( val );
}

inline RealF m_invsqrt( const RealF val )
{
   AssertFatal( val >= RealF(0), "m_invsqrt: domain error" );
   return (RealF) 1.0/m_sqrt_RealF( val );
}

inline RealF m_invsqrtf( const RealF val )
{
   AssertFatal( val >= RealF(0), "m_invsqrtf: domain error" );
   return m_invsqrtf_RealF( val );
}

inline RealF m_fog( const RealF w, const RealF density )
{
   return m_fog_RealF( w, density );
}

#endif

#ifndef M_NO_DOUBLE

inline RealD m_abs( const RealD val )
{
   return fabs(val);
}

inline RealD m_sqr( const RealD val )
{
   return (RealD)( val*val );
}

inline RealD m_sqrt( const RealD val )
{
   AssertFatal( val >= RealD(0), "m_sqrt: domain error" );
   return (RealD) sqrt( val );
}

inline RealD m_sqrtf( const RealD val )
{
   AssertFatal( val >= RealD(0), "m_sqrtf: domain error" );
   return (RealD) sqrt( val );
}

inline RealD m_invsqrt( const RealD val )
{
   AssertFatal( val >= RealF(0), "m_invsqrt: domain error" );
   return (RealD) 1.0/sqrt(val);
}

inline RealD m_invsqrtf( const RealD val )
{
   AssertFatal( val >= RealF(0), "m_invsqrtf: domain error" );
   return (RealD) 1.0/sqrt(val);
}

inline RealD m_log2( const RealD val )
{
   AssertFatal( val >= RealD(0), "m_log2: domain error" );
   return (RealD)(log( val ) / log( 2.0 ));
}

#endif

inline Int32 m_abs( const Int32 val )
{
   return ( val >= 0 ) ? val : -val;
}

inline Int32 m_sqr( const Int32 val )
{
   return ( val*val );
}

inline Int32	m_sqrt( const Int32 val )
{
   AssertFatal( val >= 0, "m_sqrt: domain error" );
	return m_sqrt_( val );
}

inline Int32	m_log2( const Int32 val )
{
   AssertFatal( val >= 0, "m_log2: domain error" );
	return m_log2_( val );
}


//----------------------------------------------------------------------------
// hypotenuse functions:

#ifndef M_NO_FLOAT

inline RealF	m_hyp( const RealF a, const RealF b )
{
   return m_sqrt( a*a + b*b );
}

inline RealF	m_hyp( const RealF a, const RealF b, const RealF c )
{
   return m_sqrt( a*a + b*b + c*c );
}

inline RealF	m_hypf( const RealF a, const RealF b )
{
   return m_hyp( a, b );
}

inline RealF	m_hypf( const RealF a, const RealF b, const RealF c )
{
   return m_hyp( a, b, c );
}

inline RealF	m_hypvf( const RealF a, const RealF b )
{
   return m_hypf( a, b );
}

inline RealF	m_hypvf( const RealF a, const RealF b, const RealF c )
{
   return m_hypf( a, b, c );
}

#endif   // M_NO_FLOAT

#ifndef M_NO_DOUBLE

inline RealD	m_hyp( const RealD a, const RealD b )
{
   return m_sqrt( a*a + b*b );
}

inline RealD	m_hyp( const RealD a, const RealD b, const RealD c )
{
   return m_sqrt( a*a + b*b + c*c );
}

inline RealD	m_hypf( const RealD a, const RealD b )
{
   return m_hyp( a, b );
}

inline RealD	m_hypf( const RealD a, const RealD b, const RealD c )
{
   return m_hyp( a, b, c );
}

inline RealD	m_hypvf( const RealD a, const RealD b )
{
   return m_hypf( a, b );
}

inline RealD	m_hypvf( const RealD a, const RealD b, const RealD c )
{
   return m_hypf( a, b, c );
}

#endif   // M_NO_DOUBLE

inline Int32	m_hyp( const Int32 a, const Int32 b )
{
   return m_sqrt( a*a + b*b );
}

inline Int32	m_hyp( const Int32 a, const Int32 b, const Int32 c )
{
   return m_sqrt( a*a + b*b + c*c );
}

inline Int32	m_hypf( const Int32 x, const Int32 y )
{
	// error: -0,+12%	(never under estimates)
	
	Int32 ax = abs( x );
	Int32 ay = abs( y );

	if( ax > ay )
		return ax + ay/2;
	else
		return ay + ax/2;
}

inline Int32	m_hypf( const Int32 x, const Int32 y, const Int32 z )
{
	// error: -+8%
	
	Int32 ax = abs( x );
	Int32 ay = abs( y );
	Int32 az = abs( z );


	// sort so that ax >= ay >= az
   Int32 temp;
	if( ax < ay )
      {
      temp = ax;
      ax = ay;
      ay = temp;
      }
	if( ax < az )
      {
      temp = ax;
      ax = az;
      az = temp;
      }
	if( ay < az )
      {
      temp = ay;
      ay = az;
      az = temp;
      }
	return ax + ay*11/32 + az/4;
}

inline Int32	m_hypvf( const Int32 x, const Int32 y )
{
	// Simply return the length of the largest:
	
	Int32 ax = abs( x );
	Int32 ay = abs( y );

	if( ax > ay )
		return ax;
	else
		return ay;
}

inline Int32	m_hypvf( const Int32 x, const Int32 y, const Int32 z )
{
	// Simply return the length of the largest:
	
	Int32 ax = abs( x );
	Int32 ay = abs( y );
	Int32 az = abs( z );

	if( ax > ay )
		{
		if( ax > az )
			return ax;
		else
			return az;
		}
	else
		{
		if( ay > az )
			return ay;
		else
			return az;
		}
}

//----------------------------------------------------------------------------
// Range check & limit functions:

inline Bool		m_check_range( const RealF val, const RealF range )
{
	return ( val <= range ) && ( val >= -range );
}

inline RealF	m_limit_range( const RealF val, const RealF range )
{
	if( val >= range )
		return range;
	if( val <= -range )
		return -range;
	return val;
}

inline Bool		m_check_range( const RealD val, const RealD range )
{
	return ( val <= range ) && ( val >= -range );
}

inline RealD	m_limit_range( const RealD val, const RealD range )
{
	if( val >= range )
		return range;
	if( val <= -range )
		return -range;
	return val;
}

inline Bool		m_check_range( const Int32 val, const Int32 range )
{
	return ( val <= range ) && ( val >= -range );
}

inline Int32	m_limit_range( const Int32 val, const Int32 range )
{
	if( val >= range )
		return range;
	if( val <= -range )
		return -range;
	return val;
}

//----------------------------------------------------------------------------
// Bit scanning functions

inline Int32	m_bsr( const Int32 val )
{
   AssertFatal( val >= 0, "m_bsr: domain error" );
	return m_bsr_( val );
}

inline Int32	m_bsf( const Int32 val )
{
   AssertFatal( val >= 0, "m_bsf: domain error" );
	return m_bsf_( val );
}

//----------------------------------------------------------------------------
// Mapping functions:

inline Int32	m_map( const Int32 dval, const Int32 dmin, const Int32 dmax,
				const Int32 rmin, const Int32 rmax);

inline Int32	m_map( const Int32 dval, const Int32 dmin, const Int32 dmax,
				const Int32 rmin, const Int32 rmax)
{
	return m_map32( dval, dmin, dmax, rmin, rmax );
}

//----------------------------------------------------------------------------
// float to Fix routines 

// For unsigned floats, assumed have min 0.0 and max 1.0
// unsigned float to unsigned FixPoint.  We subtract TWO from
// 1<<bits so that 0.5f has an exact bit representation
inline int UNIT_TO_UFIX(float f, int bits)
{
   return (int)(f * float((1<<bits)-2) + 0.5);
}   

// unsigned float to unsigned FixPoint
inline float UFIX_TO_UNIT(int f, int bits)
{
   return float(f)/float((1<<bits)-2);
}   

// For signed floats, assumed have min -1.0 and max 1.0
// You loose one bit of precesion storing the sign bit
// signed float to signed FixPoint
inline int UNIT_TO_SFIX(float f, int bits)
{
   return ((int) (f * float(1 << (bits-1) ) + 0.5));
}   

// signed float to signed FixPoint
inline float SFIX_TO_UNIT(int f, int bits)
{
   if ( f & (1<<bits))
      f |= ~((1<<bits)-1);
   return float(f)/float(1 << (bits-1));
}   





#endif
