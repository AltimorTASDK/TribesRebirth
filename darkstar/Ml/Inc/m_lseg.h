//================================================================
//   
// $Workfile:   m_lseg.h  $
// $Revision:   1.5  $
// $Version$
//	
// DESCRIPTION:
//    Line segment classes: 2D and 3D.
//	
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _M_LSEG_H_
#define _M_LSEG_H_

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//----------------------------------------------------------------------------
// Line Segment Classes

class LineSeg2F
{
public:
	Point2F	start, end;

	inline LineSeg2F();
	inline LineSeg2F( const Point2F&, const Point2F& );

   inline Point2F * getVector( Point2F * ) const;

	inline float	len( void ) const;
	inline float	lenf( void ) const;

	inline float	len_x( void ) const;
	inline float	len_y( void ) const;

   inline void	reverse( void );
};

class LineSeg3F
{
public:
   Point3F start, end;

	inline LineSeg3F();
	inline LineSeg3F( const Point3F&, const Point3F& );

   inline Point3F * getVector( Point3F * ) const;

	inline float   len( void ) const;
	inline float   lenf( void ) const;

   inline float   len_x( void ) const;
	inline float   len_y( void ) const;
	inline float   len_z( void ) const;

	inline void	reverse( void );
};

//----------------------------------------------------------------------------
// LineSeg2F

inline LineSeg2F::LineSeg2F()
{}

inline LineSeg2F::LineSeg2F( const Point2F& s, const Point2F& e )
{
	start = s;
	end = e;
}

inline Point2F* LineSeg2F::getVector( Point2F *p ) const
{
   *p = end;
   *p -= start;
   return p;
}

inline float LineSeg2F::len( void ) const
{
   return m_hyp( float(start.x - end.x), 
                 float(start.y - end.y) );
}

inline float LineSeg2F::lenf( void ) const
{
   return m_hypf( float(start.x - end.x), 
                  float(start.y - end.y) );
}

inline float LineSeg2F::len_x( void ) const
{
   return float(end.x - start.x);
}

inline float LineSeg2F::len_y( void ) const
{
   return float(end.y - start.y);
}

inline void LineSeg2F::reverse( void )
{
	Point2F temp = start;
	start = end;
	end = temp;
}

//----------------------------------------------------------------------------
// LineSeg3F

inline LineSeg3F::LineSeg3F()
{}

inline LineSeg3F::LineSeg3F( const Point3F& s, const Point3F& e )
{
	start = s;
	end = e;
}

inline Point3F* LineSeg3F::getVector( Point3F *p ) const
{
   *p = end;
   *p -= start;
   return p;
}

inline float LineSeg3F::len( void ) const
{
   return m_hyp( float(start.x - end.x), 
                 float(start.y - end.y),
                 float(start.z - end.z) );
}

inline float LineSeg3F::lenf( void ) const
{
   return m_hypf( float(start.x - end.x), 
                  float(start.y - end.y),
                  float(start.z - end.z) );
}

inline float LineSeg3F::len_x( void ) const
{
   return float(end.x - start.x);
}

inline float LineSeg3F::len_y( void ) const
{
   return float(end.y - start.y);
}

inline float LineSeg3F::len_z( void ) const
{
   return float(end.z - start.z);
}

inline void LineSeg3F::reverse( void )
{
	Point3F temp = start;
	start = end;
	end = temp;
}


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif

