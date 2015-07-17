//================================================================
//   
//	$Workfile:   m_trig.h  $
//	$Revision:   1.8  $
// $Version: 1.6.2 $
//	
// DESCRIPTION:
//    Trigonometry related types and functions.
//   
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _M_TRIG_H_
#define _M_TRIG_H_

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

#ifndef M_2PI
#define M_2PI       (2*3.14159265358979323846)
#endif

//----------------------------------------------------------------------------
// Basic Trig Functions
// Float versions:

#ifndef M_NO_FLOAT

typedef RealF   AngleF;

// assembly functions for float trig:

extern "C" {
void __fastcall m_AngleF_sincos( const AngleF &, RealF *, RealF * );
const AngleF __fastcall m_AngleF_reduce( const AngleF & );
};

extern AngleF DLLAPI m_reduce( const AngleF a );

inline RealF	m_sin( const AngleF a )
{
   return (RealF) sin( a );
}

inline RealF	m_cos( const AngleF a )
{
   return (RealF) cos( a );
}

inline void m_sincos( const AngleF a, RealF *psin, RealF *pcos )
{
   m_AngleF_sincos( a, psin, pcos );
}

inline AngleF	m_asin( const RealF val )
{
   AssertFatal(val >= -1.0f && val <= 1.0f, "asin domain error");
	return (AngleF) asin( val );
}

inline AngleF	m_acos( const RealF val )
{
   AssertFatal(val >= -1.0f && val <= 1.0f, "acos domain error");
	return  (AngleF) acos( val );
}

inline AngleF	m_atan( const RealF x, const RealF y )
{
   AssertFatal( (y != 0.0f) || (x != 0.0f), "X and Y are both zero in atan");
	return (AngleF) atan2( y, x );
}

inline RealF	m_ptox( const RealF r, const AngleF a )
{
	return (RealF) (r * cos( a ));
}

inline RealF	m_ptoy( const RealF r, const AngleF a )
{
	return (RealF) (r * sin( a ));
}

#endif // M_NO_FLOAT

//----------------------------------------------------------------------------
// Basic Trig Functions
// Double versions:

#ifndef M_NO_DOUBLE

typedef RealD   AngleD;

// assembly functions for double trig:

extern "C" {
const AngleD __fastcall m_AngleD_reduce( const AngleD & );
void __fastcall m_AngleD_sincos( const AngleD &, RealD *, RealD * );
};

extern AngleD DLLAPI m_reduce( const AngleD a );

inline RealD	m_sin( const AngleD a )
{
   return (RealD) sin( a );
}

inline RealD	m_cos( const AngleD a )
{
   return (RealD) cos( a );
}

inline void m_sincos( const AngleD a, RealD *psin, RealD *pcos )
{
   m_AngleD_sincos( a, psin, pcos );
}

inline AngleD	m_asin( const RealD val )
{
	return (AngleD) asin( val );
}

inline AngleD	m_acos( const RealD val )
{
	return  (AngleD) acos( val );
}

inline AngleD	m_atan( const RealD x, const RealD y )
{
	return (AngleD) atan2( y, x );
}

inline RealD	m_ptox( const RealD r, const AngleD a )
{
	return (RealD) (r * cos( a ));
}

inline RealD	m_ptoy( const RealD r, const AngleD a )
{
	return (RealD) (r * sin( a ));
}

#endif // M_NO_DOUBLE
//----------------------------------------------------------------------------

#endif








