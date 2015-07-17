//----------------------------------------------------------------------------

//	$Workfile:   grdrange.h  $
//	$Version$
//	$Revision:   1.2  $
//	$Date:   31 Oct 1995 17:15:52  $

//  VO:timg

//----------------------------------------------------------------------------

#ifndef _GRDRANGE_H_
#define _GRDRANGE_H_

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//----------------------------------------------------------------------------

template <class T>
struct GridRange
{
	T fMin,fMax;
	GridRange() {}
	GridRange(const T& _min,const T& _max): fMin(_min), fMax(_max) {}
	bool inside(const T& a) const { return a >= fMin && a <= fMax; }
	bool outside(const T& a) const { return !inside(a); }
	int length() const { return fMax - fMin; }
	int width() const { return length(); }
	GridRange<T>& operator+=(const T& off) {
		fMin += off; fMax += off;
		return *this;
	}
	GridRange<T>& set(const T& a,const T& b) {
		fMin = a; fMax = b;
		return *this;
	}
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif


#endif
