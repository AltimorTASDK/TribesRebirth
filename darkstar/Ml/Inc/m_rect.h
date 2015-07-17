//================================================================
//   
// $Workfile:   m_rect.h  $
// $Revision:   1.4  $
// $Version$
//	
// DESCRIPTION:
//    Rectangle classes.
//	
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _M_RECT_H_
#define _M_RECT_H_

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//----------------------------------------------------------------------------
// RectI Classes

class RectI
{
public:
   Point2I   upperL;
   Point2I   lowerR;

   //== Constructors and Assignment
   RectI()   {}
   RectI(const RectI *in_rect);
   RectI(const Point2I *ul, const Point2I *lr);
   RectI(const Point2I &ul, const Point2I &lr);
   RectI(Int32 in_u, Int32 in_ul, Int32 in_l, Int32 in_lr);

   RectI& operator= (const RectI *in_rect);

   //== Member access:
   Int32 & left();
   Int32 & top();
   Int32 & right();
   Int32 & bottom();

   //== Arithmetic that modifies this object
   RectI& operator() (const RectI *in_rect);
   RectI& operator() (const Point2I *in_ul, const Point2I *in_lr);
   RectI& operator() (Int32 in_ulx, Int32 in_uly, Int32 in_lrx, Int32 in_lry);
   RectI& operator-= (const Point2I *in_pt);
   RectI& operator-= (const Point2I &in_pt);
   RectI& operator-= (const Int32 in_val);
   RectI& operator+= (const Point2I *in_pt);
   RectI& operator+= (const Point2I &in_pt);
   RectI& operator+= (const Int32 in_val);
   RectI& operator*= (const Point2I *in_pt);
   RectI& operator*= (const Point2I &in_pt);
   RectI& operator*= (const Int32 in_val);
   RectI& operator/= (const Point2I *in_pt);
   RectI& operator/= (const Point2I &in_pt);
   RectI& operator/= (const Int32 in_val);

	inline Int32	len_x( void ) const;
	inline Int32	len_y( void ) const;

   bool isValidRect() const;
};

//---------------------------------------------------------------------------

inline RectI::RectI(const RectI *in_rect)
{
   *this = *in_rect;
}

inline RectI::RectI(const Point2I *in_ul, const Point2I *in_lr)
{
   upperL = *in_ul;
   lowerR = *in_lr;
}

inline RectI::RectI(const Point2I &in_ul, const Point2I &in_lr)
{
   upperL = in_ul;
   lowerR = in_lr;
}

inline RectI::RectI(Int32 in_ulx, Int32 in_uly, Int32 in_lrx, Int32 in_lry)
{
   upperL(in_ulx, in_uly);
   lowerR(in_lrx, in_lry);
}

inline RectI& RectI::operator= (const RectI *in_rect)
{
   memcpy((void *)this, (void *)in_rect, sizeof(RectI));
   return *this;
}

inline Int32 & RectI::left()
{
   return upperL.x;
}

inline Int32 & RectI::top()
{
   return upperL.y;
}

inline Int32 & RectI::right()
{
   return lowerR.x;
}

inline Int32 & RectI::bottom()
{
   return lowerR.y;
}


inline RectI& RectI::operator() (const RectI *in_rect)
{
   *this = *in_rect;
   return *this;
}

inline RectI& RectI::operator() (const Point2I *in_ul, const Point2I *in_lr)
{
   upperL = *in_ul;
   lowerR = *in_lr;
   return *this;
}

inline RectI& RectI::operator() (Int32 in_ulx, Int32 in_uly, Int32 in_lrx, Int32 in_lry)
{
   upperL(in_ulx, in_uly);
   lowerR(in_lrx, in_lry);
   return *this;
}

inline RectI& RectI::operator-= (const Point2I *in_pt)
{
   upperL -= *in_pt;
   lowerR -= *in_pt;
   return *this;
}

inline RectI& RectI::operator-= (const Point2I &in_pt)
{
   upperL -= in_pt;
   lowerR -= in_pt;
   return *this;
}

inline RectI& RectI::operator-= (const Int32 in_val)
{
   upperL -= in_val;
   lowerR -= in_val;
   return *this;
}

inline RectI& RectI::operator+= (const Point2I *in_pt)
{
   upperL += *in_pt;
   lowerR += *in_pt;
   return *this;
}

inline RectI& RectI::operator+= (const Point2I &in_pt)
{
   upperL += in_pt;
   lowerR += in_pt;
   return *this;
}

inline RectI& RectI::operator+= (const Int32 in_val)
{
   upperL += in_val;
   lowerR += in_val;
   return *this;
}

inline RectI& RectI::operator*= (const Point2I *in_pt)
{
   upperL.x *= in_pt->x;
   upperL.y *= in_pt->y;
   lowerR.x *= in_pt->x;
   lowerR.y *= in_pt->y;
   return *this;
}

inline RectI& RectI::operator*= (const Point2I &in_pt)
{
   upperL.x *= in_pt.x;
   upperL.y *= in_pt.y;
   lowerR.x *= in_pt.x;
   lowerR.y *= in_pt.y;
   return *this;
}

inline RectI& RectI::operator*= (const Int32 in_val)
{
   upperL.x *= in_val;
   upperL.y *= in_val;
   lowerR.x *= in_val;
   lowerR.y *= in_val;
   return *this;
}

inline RectI& RectI::operator/= (const Point2I *in_pt)
{
   upperL.x /= in_pt->x;
   upperL.y /= in_pt->y;
   lowerR.x /= in_pt->x;
   lowerR.y /= in_pt->y;
   return *this;
}

inline RectI& RectI::operator/= (const Point2I &in_pt)
{
   upperL.x /= in_pt.x;
   upperL.y /= in_pt.y;
   lowerR.x /= in_pt.x;
   lowerR.y /= in_pt.y;
   return *this;
}

inline RectI& RectI::operator/= (const Int32 in_val)
{
   upperL.x /= in_val;
   upperL.y /= in_val;
   lowerR.x /= in_val;
   lowerR.y /= in_val;
   return *this;
}

inline Int32	RectI::len_x( void ) const
{
	return lowerR.x - upperL.x;
}

inline Int32	RectI::len_y( void ) const
{
	return lowerR.y - upperL.y;
}

inline bool
RectI::isValidRect() const
{
   return (upperL.x <= lowerR.x) &&
          (upperL.y <= lowerR.y);
}

//----------------------------------------------------------------------------
// RectF Classes

class RectF
{
public:
   Point2F   upperL;
   Point2F   lowerR;

   //== Constructors and Assignment
   RectF()   {}
   RectF(const RectF *in_rect);
   RectF(const Point2F *ul, const Point2F *lr);
   RectF(const Point2F &ul, const Point2F &lr);
   RectF(float in_u, float in_ul, float in_l, float in_lr);

   RectF& operator= (const RectF *in_rect);

   //== Member access:
   float & left();
   float & top();
   float & right();
   float & bottom();

   //== Arithmetic that modifies this object
   RectF& operator() (const RectF *in_rect);
   RectF& operator() (const Point2F *in_ul, const Point2F *in_lr);
   RectF& operator() (float in_ulx, float in_uly, float in_lrx, float in_lry);
   RectF& operator-= (const Point2F *in_pt);
   RectF& operator-= (const Point2F &in_pt);
   RectF& operator-= (const float in_val);
   RectF& operator+= (const Point2F *in_pt);
   RectF& operator+= (const Point2F &in_pt);
   RectF& operator+= (const float in_val);
   RectF& operator*= (const Point2F *in_pt);
   RectF& operator*= (const Point2F &in_pt);
   RectF& operator*= (const float in_val);
   RectF& operator/= (const Point2F *in_pt);
   RectF& operator/= (const Point2F &in_pt);
   RectF& operator/= (const float in_val);

	inline float	len_x( void ) const;
	inline float	len_y( void ) const;
};

//---------------------------------------------------------------------------

inline RectF::RectF(const RectF *in_rect)
{
   *this = *in_rect;
}

inline RectF::RectF(const Point2F *in_ul, const Point2F *in_lr)
{
   upperL = *in_ul;
   lowerR = *in_lr;
}

inline RectF::RectF(const Point2F &in_ul, const Point2F &in_lr)
{
   upperL = in_ul;
   lowerR = in_lr;
}

inline RectF::RectF(float in_ulx, float in_uly, float in_lrx, float in_lry)
{
   upperL(in_ulx, in_uly);
   lowerR(in_lrx, in_lry);
}

inline RectF& RectF::operator= (const RectF *in_rect)
{
   memcpy((void *)this, (void *)in_rect, sizeof(RectF));
   return *this;
}

inline float & RectF::left()
{
   return upperL.x;
}

inline float & RectF::top()
{
   return upperL.y;
}

inline float & RectF::right()
{
   return lowerR.x;
}

inline float & RectF::bottom()
{
   return lowerR.y;
}


inline RectF& RectF::operator() (const RectF *in_rect)
{
   *this = *in_rect;
   return *this;
}

inline RectF& RectF::operator() (const Point2F *in_ul, const Point2F *in_lr)
{
   upperL = *in_ul;
   lowerR = *in_lr;
   return *this;
}

inline RectF& RectF::operator() (float in_ulx, float in_uly, float in_lrx, float in_lry)
{
   upperL(in_ulx, in_uly);
   lowerR(in_lrx, in_lry);
   return *this;
}

inline RectF& RectF::operator-= (const Point2F *in_pt)
{
   upperL -= *in_pt;
   lowerR -= *in_pt;
   return *this;
}

inline RectF& RectF::operator-= (const Point2F &in_pt)
{
   upperL -= in_pt;
   lowerR -= in_pt;
   return *this;
}

inline RectF& RectF::operator-= (const float in_val)
{
   upperL -= in_val;
   lowerR -= in_val;
   return *this;
}

inline RectF& RectF::operator+= (const Point2F *in_pt)
{
   upperL += *in_pt;
   lowerR += *in_pt;
   return *this;
}

inline RectF& RectF::operator+= (const Point2F &in_pt)
{
   upperL += in_pt;
   lowerR += in_pt;
   return *this;
}

inline RectF& RectF::operator+= (const float in_val)
{
   upperL += in_val;
   lowerR += in_val;
   return *this;
}

inline RectF& RectF::operator*= (const Point2F *in_pt)
{
   upperL.x *= in_pt->x;
   upperL.y *= in_pt->y;
   lowerR.x *= in_pt->x;
   lowerR.y *= in_pt->y;
   return *this;
}

inline RectF& RectF::operator*= (const Point2F &in_pt)
{
   upperL.x *= in_pt.x;
   upperL.y *= in_pt.y;
   lowerR.x *= in_pt.x;
   lowerR.y *= in_pt.y;
   return *this;
}

inline RectF& RectF::operator*= (const float in_val)
{
   upperL.x *= in_val;
   upperL.y *= in_val;
   lowerR.x *= in_val;
   lowerR.y *= in_val;
   return *this;
}

inline RectF& RectF::operator/= (const Point2F *in_pt)
{
   upperL.x /= in_pt->x;
   upperL.y /= in_pt->y;
   lowerR.x /= in_pt->x;
   lowerR.y /= in_pt->y;
   return *this;
}

inline RectF& RectF::operator/= (const Point2F &in_pt)
{
   upperL.x /= in_pt.x;
   upperL.y /= in_pt.y;
   lowerR.x /= in_pt.x;
   lowerR.y /= in_pt.y;
   return *this;
}

inline RectF& RectF::operator/= (const float in_val)
{
   upperL.x /= in_val;
   upperL.y /= in_val;
   lowerR.x /= in_val;
   lowerR.y /= in_val;
   return *this;
}

inline float	RectF::len_x( void ) const
{
	return lowerR.x - upperL.x;
}

inline float	RectF::len_y( void ) const
{
	return lowerR.y - upperL.y;
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif



