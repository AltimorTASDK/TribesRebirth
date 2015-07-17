//================================================================
//   
// $Workfile:   m_dist.h  $
// $Revision:   1.1  $
// $Version$
//	$Date:   23 Oct 1995 13:22:56  $
//	
// DESCRIPTION:
//    Overloaded Function: Value m_dist( Point&, Point& )
//	
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _M_DIST_H_
#define _M_DIST_H_

//---------------------------------------------------------------------------
// Float Versions:
// 2D Distance Functions

inline RealF	m_dist( const Point2F& a, const Point2F& b )
{
	Point2F temp = a;
	temp -= b;
	return (RealF) temp.len();
}

inline RealF	m_distf( const Point2F& a, const Point2F& b )
{
	Point2F temp = a;
	temp -= b;
	return (RealF) temp.lenf();
}

inline RealF	m_distvf( const Point2F& a, const Point2F& b )
{
	Point2F temp = a;
	temp -= b;
	return (RealF) temp.lenvf();
}

inline Bool	m_check_dist( const Point2F& a, const Point2F& b, const RealF d )
{
	return m_dist( a, b ) <= d;
}

inline Bool	m_check_distf( const Point2F& a, const Point2F& b, const RealF d )
{
	return m_distf( a, b ) <= d;
}

inline Bool	m_check_distvf( const Point2F& a, const Point2F& b, const RealF d )
{
	return m_distvf( a, b ) <= d;
}

//---------------------------------------------------------------------------
// Float Versions:
// 3D Distance Functions

inline RealF	m_dist( const Point3F& a, const Point3F& b )
{
	Point3F temp = a;
	temp -= b;
	return (RealF) temp.len();
}

inline RealF	m_distf( const Point3F& a, const Point3F& b )
{
	Point3F temp = a;
	temp -= b;
	return (RealF) temp.lenf();
}

inline RealF	m_distvf( const Point3F& a, const Point3F& b )
{
	Point3F temp = a;
	temp -= b;
	return (RealF) temp.lenvf();
}

inline Bool	m_check_dist( const Point3F& a, const Point3F& b, const RealF d )
{
	return m_dist( a, b ) <= d;
}

inline Bool	m_check_distf( const Point3F& a, const Point3F& b, const RealF d )
{
	return m_distf( a, b ) <= d;
}

inline Bool	m_check_distvf( const Point3F& a, const Point3F& b, const RealF d )
{
	return m_distvf( a, b ) <= d;
}

#if 0
//---------------------------------------------------------------------------
// Double Versions:
// 2D Distance Functions

inline RealD	m_dist( const Point2D& a, const Point2D& b )
{
	Point2D temp = a;
	temp -= b;
	return (RealD) temp.len();
}

inline RealD	m_distf( const Point2D& a, const Point2D& b )
{
	Point2D temp = a;
	temp -= b;
	return (RealD) temp.lenf();
}

inline RealD	m_distvf( const Point2D& a, const Point2D& b )
{
	Point2D temp = a;
	temp -= b;
	return (RealD) temp.lenvf();
}

inline Bool	m_check_dist( const Point2D& a, const Point2D& b, const RealD d )
{
	return m_dist( a, b ) <= d;
}

inline Bool	m_check_distf( const Point2D& a, const Point2D& b, const RealD d )
{
	return m_distf( a, b ) <= d;
}

inline Bool	m_check_distvf( const Point2D& a, const Point2D& b, const RealD d )
{
	return m_distvf( a, b ) <= d;
}

//---------------------------------------------------------------------------
// Double Versions:
// 3D Distance Functions

inline RealD	m_dist( const Point3D& a, const Point3D& b )
{
	Point3D temp = a;
	temp -= b;
	return (RealD) temp.len();
}

inline RealD	m_distf( const Point3D& a, const Point3D& b )
{
	Point3D temp = a;
	temp -= b;
	return (RealD) temp.lenf();
}

inline RealD	m_distvf( const Point3D& a, const Point3D& b )
{
	Point3D temp = a;
	temp -= b;
	return (RealD) temp.lenvf();
}

inline Bool	m_check_dist( const Point3D& a, const Point3D& b, const RealD d )
{
	return m_dist( a, b ) <= d;
}

inline Bool	m_check_distf( const Point3D& a, const Point3D& b, const RealD d )
{
	return m_distf( a, b ) <= d;
}

inline Bool	m_check_distvf( const Point3D& a, const Point3D& b, const RealD d )
{
	return m_distvf( a, b ) <= d;
}
#endif

//---------------------------------------------------------------------------
// Integer Versions:
// 2D Distance Functions

inline Int32	m_dist( const Point2I& a, const Point2I& b )
{
	Point2I temp = a;
	temp -= b;
	return (Int32) temp.len();
}

inline Int32	m_distf( const Point2I& a, const Point2I& b )
{
	Point2I temp = a;
	temp -= b;
	return (Int32) temp.lenf();
}

inline Int32	m_distvf( const Point2I& a, const Point2I& b )
{
	Point2I temp = a;
	temp -= b;
	return (Int32) temp.lenvf();
}

inline Bool	m_check_dist( const Point2I& a, const Point2I& b, const Int32 d )
{
	return m_dist( a, b ) <= d;
}

inline Bool	m_check_distf( const Point2I& a, const Point2I& b, const Int32 d )
{
	return m_distf( a, b ) <= d;
}

inline Bool	m_check_distvf( const Point2I& a, const Point2I& b, const Int32 d )
{
	return m_distvf( a, b ) <= d;
}

//---------------------------------------------------------------------------
// Integer Versions:
// 3D Distance Functions

inline Int32	m_dist( const Point3I& a, const Point3I& b )
{
	Point3I temp = a;
	temp -= b;
	return (Int32) temp.len();
}

inline Int32	m_distf( const Point3I& a, const Point3I& b )
{
	Point3I temp = a;
	temp -= b;
	return (Int32) temp.lenf();
}

inline Int32	m_distvf( const Point3I& a, const Point3I& b )
{
	Point3I temp = a;
	temp -= b;
	return (Int32) temp.lenvf();
}

inline Bool	m_check_dist( const Point3I& a, const Point3I& b, const Int32 d )
{
	return m_dist( a, b ) <= d;
}

inline Bool	m_check_distf( const Point3I& a, const Point3I& b, const Int32 d )
{
	return m_distf( a, b ) <= d;
}

inline Bool	m_check_distvf( const Point3I& a, const Point3I& b, const Int32 d )
{
	return m_distvf( a, b ) <= d;
}


#endif


