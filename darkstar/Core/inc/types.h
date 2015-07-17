//----------------------------------------------------------------
//
// $Workfile:   types.h  $
// $Version: 1.0.a1 $
// $Revision:   1.20  $
//
// DESCRIPTION:
//
// (c) Copyright 1995, Dynamix Inc.   All rights reserved.
//----------------------------------------------------------------

#ifndef _TYPES_H_
#define _TYPES_H_

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

#pragma warning( disable : 4068 )   // Force MSVC++ to ignore unknow pragmas!

// msvc warnings about exports
#pragma warning( disable : 4251 )
#pragma warning( disable : 4275 )
// msvc warnings about conversions to bool
#pragma warning( disable : 4800 )
// msvc warnings about precision loss
#pragma warning( disable : 4244 )



#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>

//---------------------------------------------------------------------------
// calling conventions: (don't use fastcall by default for debug)
//
// _USERENTRY is defined by borland for user supplied callback functions
// so we define it here for VC++ as well.
// is used for all functions except constructors and destructors
// is used for all constructurs and destructors
//

//#ifndef NODLLAPI
//#define DLLAPI       __declspec(dllexport)
//#else
#define DLLAPI
//#endif

#ifdef _MSC_VER
#ifndef _USERENTRY
#define _USERENTRY   __cdecl
#endif
#endif


//---------------------------------------------------------------------------

#ifndef NULL
#define NULL 0L
#endif

//---------------------------------------------------------------------------
// Standardized Types
//               Object                          Range
typedef           char Int8;   //         -128 to 127          8-bit char
typedef          short Int16;  //       -32768 to 32767       16-bit signed int
typedef           long Int32;  //  -2147483684 to 214483647   32-bit signed long
typedef          float Flt32;  //    3.4x10-38 to 3.4x1038    32-bit real
typedef         double Flt64;  //   1.7x10-308 to 1.7x10308   64-bit real
typedef unsigned  char UInt8;  //            0 to 255          8-bit unsigned byte
typedef unsigned short UInt16; //            0 to 65535       16-bit unsigned word
typedef unsigned  long UInt32; //            0 to 4294967295  32-bit unsigned dword
typedef           long Fix16;  // -32767.65536 to 32767.65536 15.16  signed fixed float
typedef const char *StringTableEntry;

typedef struct {
   Int32    lo;
   Int32    hi;
} Int64;

//---------------------------------------------------------------------------
// quotient+remainter type

typedef struct {
	Int32		quot;
	Int32		rem;
} div_t32;

//---------------------------------------------------------------------------
// Bool type

// Does your compiler support the bool type?
// So far, only Borland 5.0 does.

#if ((defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)) || (defined(_MSC_VER) && (_MSC_VER >= 1100)))
   typedef  bool  Bool;
#else
#if defined(_MSC_VER) && (_MSC_VER <= 1020)
// MSVC 4.2 reserves bool,true,false but doesn't define them
// this disables the resulting warnings
#pragma warning( disable : 4237 )
   typedef         int Bool;   // Boolean
   typedef         int bool;
   const Bool false = 0;            // TRUE and FALSE
   const Bool true  = 1;
#else
   typedef  bool Bool;
#endif
#endif

//---------------------------------------------------------------------------

const Bool YES   = TRUE;       // YES and NO
const Bool NO    = FALSE;

const Bool ON    = TRUE;		 // ON as OFF
const Bool OFF   = FALSE;

//---------------------------------------------------------------------------
// Real, Fixed point type.


#define FIX_SHIFT 16
#define FIX_ONE   (1 << FIX_SHIFT)  // a fixed point value of 1.0

typedef Fix16  RealX;   // sort of real...

#define RealX_One FIX_ONE
#define RealX_Half (FIX_ONE>>1)
#define RealX_Zero 0

inline Bool IsEqual( RealX a, RealX b )   // same as IsEqual(Int32,Int32)
{
   return a == b;
}

inline Bool IsZero( RealX a )    // same as IsZero(Int32)
{
   return !a;
}

//---------------------------------------------------------------------------
// Real, Float type.

typedef float  RealF;

#define EQUAL_F   ((RealF)0.000001)

#define RealF_One ((RealF)1.0)
#define RealF_Half ((RealF)0.5)
#define RealF_Zero ((RealF)0.0)
#define RealF_Pi  ((RealF)3.14159265358979323846)
#define RealF_2Pi  ((RealF)(2*3.14159265358979323846))

inline Bool IsZero( RealF a, RealF equal = EQUAL_F)
{
   return a*a < equal*equal;
}

inline Bool IsEqual( RealF a, RealF b , RealF equal = EQUAL_F)
{
   return IsZero(a-b,equal);
}

//---------------------------------------------------------------------------
// Real, Double type.

typedef double RealD;

#define EQUAL_D   ((RealD)0.000001)

#define RealD_One ((RealD)1.0)
#define RealD_Half ((RealD)0.5)
#define RealD_Zero ((RealD)0.0)
#define RealD_Pi  ((RealD)3.14159265358979323846)
#define RealD_2Pi  ((RealD)(2*3.14159265358979323846))

inline Bool IsZero( RealD a, RealD equal = EQUAL_D)
{
   return a*a < equal*equal;
}

inline Bool IsEqual( RealD a, RealD b , RealD equal = EQUAL_D)
{
   return IsZero(a-b,equal);
}

//---------------------------------------------------------------------------
// Min and Max values for integer types:
// (two sets for back compatibility reasons)

const int		M_MIN_INT =		1 << ((sizeof(int) << 3) - 1);
const int		M_MAX_INT =		~(1 << ((sizeof(int) << 3) - 1));

const Int8		M_MAX_INT8 =	0x7f;
const Int8		M_MIN_INT8 =	(Int8) 0x80;
const Int16		M_MAX_INT16 =	0x7fff;
const Int16		M_MIN_INT16	=	(Int16) 0x8000;
const Int32		M_MAX_INT32	=	0x7fffffff;
const Int32		M_MIN_INT32	=	0x80000000;

const UInt8		M_MAX_UINT8	=	0xff;
const UInt16	M_MAX_UINT16 =	0xffff;
const UInt32	M_MAX_UINT32 =	0xffffffff;


const Int8   INT8_MAX  = 127;
const Int8   INT8_MIN  =-128;
const UInt8  UINT8_MAX = 255;

const Int16	 INT16_MAX = 32767;
const Int16  INT16_MIN =-32768;
const UInt16 UINT16_MAX= 0xFFFF;

const Int32	 INT32_MAX = ((Int32)0x7FFFFFFF);
const Int32	 INT32_MIN = ((Int32)0x80000000);
const UInt32 UINT32_MAX= 0xFFFFFFFF;

//---------------------------------------------------------------------------

#define FOURCC( ch0, ch1, ch2, ch3 )                                \
		( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |    \
		( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )

#define SWAP(a, b) { (a) ^= (b); (b) ^= (a); (a) ^= (b); }


#define FIX16_SHIFT        (16)
#define FIX16_TO_INT(i)    (((i)<0)?(-((-(i))>>FIX16_SHIFT)):((i)>>FIX16_SHIFT))
#define INT_TO_FIX16(i)		((i)<<FIX16_SHIFT)


// Windows Values used by us.

#define INVALID_HANDLE_VALUE (HANDLE)-1

struct ColorX
{
   RealX red;
   RealX green;
   RealX blue;

   ColorX & interpolate( ColorX const &c1, 
      ColorX const &c2, RealF t1 )
      {
      float t2 = 1.0f - t1;
      red   = RealX(c1.red * t2   + c2.red * t1);
      green = RealX(c1.green * t2 + c2.green * t1);
      blue  = RealX(c1.blue * t2  + c2.blue * t1);
      return *this;
      }
};

struct ColorF
{
   RealF red;
   RealF green;
   RealF blue;

   void set(RealF r = 0.0f, RealF g = 0.0f, RealF b = 0.0f) 
   { 
      red = r; green = g; blue = b; 
   }
   ColorF() {}
   ColorF(RealF r, RealF g, RealF b) { set(r, g, b); }

   ColorF& interpolate(const ColorF& c1,
                       const ColorF& c2,
                       RealF         t1)
   {
      float t2 = 1.0f - t1;
      red   = (c1.red   * t2) + (c2.red   * t1);
      green = (c1.green * t2) + (c2.green * t1);
      blue  = (c1.blue  * t2) + (c2.blue  * t1);
      return *this;
   }

   ColorF& operator+=(const ColorF &c) 
   {
      red += c.red;
      green += c.green;
      blue += c.blue;
      return *this;
   }
   ColorF operator+(const ColorF &c) const
   {
      return ColorF(red   + c.red,
                    green + c.green,
                    blue  + c.blue);
   }


   ColorF& operator*=(const float in_mul) {
      red   *= in_mul;
      green *= in_mul;
      blue  *= in_mul;
      
      return *this;
   }
   ColorF operator*(const float in_mul) const {
      return ColorF(red   * in_mul,
                    green * in_mul,
                    blue  * in_mul);
   }
};

struct GFXColorInfoF
{
   float haze;
   ColorF color;
   float alpha;
};


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif
