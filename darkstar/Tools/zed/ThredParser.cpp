// ThredParser.cpp : implementation file
//

//********************************************************
//TODO:  make our constructor so it takes the current
// type of object being loaded... make it so we don't 
// read past the end of that current object type...
// this will make it easier to have things skipped.

#include "stdafx.h"
#include "thred.h"
#include "ThredParser.h"
#include "itrgeometry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CThredParser
const char* CThredParser::WhiteSpace = " \t";


// we want to write out zee polygon
#define CPOLYGONSTART "CThredPoly"
#define CPOLYGONVERTCOUNT "NumVrts"
#define CPOLYGONVERT "V"
#define CPOLYTEXSHIFT   "TexShift"
#define CPOLYAPPLYAMBIENT "ApplyAmbient"
#define CPOLYGONTEXTVERT "TV"
#define CPOLYGONEND "End CThredPoly"
void CThredParser::WritePolygon(ThredPolygon& Poly)
{
	Init();
	// write out our start
	WriteString(CPOLYGONSTART);

	// write out the number of verts
	WriteInt(CPOLYGONVERTCOUNT, Poly.NumberOfPoints );

   // write the texture scale as an int
   int tmp;
   int bits = ITRGeometry::Surface::Constants::textureScaleBits;
   int signMask = 1 << ( bits - 1 );
   int valueMask = signMask - 1;
   
   if( Poly.mTextureScaleShift & signMask )
      tmp = -( Poly.mTextureScaleShift & valueMask );
   else
      tmp = ( Poly.mTextureScaleShift & valueMask );
   
   WriteInt(CPOLYTEXSHIFT, tmp );
   
   // write out the ambient flag
   WriteBool( CPOLYAPPLYAMBIENT, Poly.mApplyAmbient );

	// write out the verts
	for(int Current = 0; Current < Poly.NumberOfPoints; Current++ )
		WritePoint(CPOLYGONVERT, Poly.Points[Current]);

	// write out the texture verts
	for(Current = 0; Current < Poly.NumberOfPoints; Current++ )
		WritePoint2F(CPOLYGONTEXTVERT, Poly.mTextureUV[Current]);

	// write our end string
	WriteString(CPOLYGONEND);
}



int CThredParser::ReadPolygon(ThredPolygon& Poly)
{
	// we need this
	if(!GetKey(CPOLYGONSTART) ) {
		AfxThrowArchiveException(CArchiveException::generic, NULL);
		return 0;
	}

	// get our transaction type
	GetKeyInt(CPOLYGONVERTCOUNT, Poly.NumberOfPoints );

   int tmp = 0;
   GetKeyInt(CPOLYTEXSHIFT, tmp );
   
   // saving as an integer and not as a bit pattern now - version <= 1.70 had 
   // 6bits and version 1.74 has 5 bits
   CTHREDApp * App = ( CTHREDApp * )AfxGetApp();
   if( ( App->m_fileVersion >> 16 ) == 1 )
   {
      int ver = App->m_fileVersion & 0x0000ffff;

      // 5 bits      
      if( ver == 74 )
      {
         if( tmp & ( 1 << 4 ) )
            tmp = -( tmp & ( ( 1 << 4 ) - 1 ) );
         else
            tmp = ( tmp & ( ( 1 << 4 ) - 1 ) );
      }
      
      // 6 bits
      if( ver < 74 )
      {
         if( tmp & ( 1 << 5 ) )
            tmp = -( tmp & ( ( 1 << 5 ) - 1 ) );
         else
            tmp = ( tmp & ( ( 1 << 5 ) - 1 ) );
      }
   }

   int bits = ITRGeometry::Surface::Constants::textureScaleBits;
   int signMask = 1 << ( bits - 1 );
   int valueMask = signMask - 1;
   
   // convert tmp to a useable mask - check the range
   if( abs( tmp ) > valueMask )
      tmp = 0;
      
   if( tmp < 0 )
   {
      tmp = abs( tmp );
      Poly.mTextureScaleShift = ( tmp & valueMask ) | signMask;
   }
   else
      Poly.mTextureScaleShift = ( tmp & valueMask );

   // get the ambient flag
   Poly.mApplyAmbient = true;
   GetKeyBool( CPOLYAPPLYAMBIENT, Poly.mApplyAmbient );
   
	// read in the verts
	for(int Current = 0; Current < Poly.NumberOfPoints; Current++ )
		GetKeyPoint(CPOLYGONVERT, Poly.Points[Current]);

	// read in the texture verts
	for(Current = 0; Current < Poly.NumberOfPoints; Current++ )
    {
		GetKeyPoint2F(CPOLYGONTEXTVERT, Poly.mTextureUV[Current]);
    }

	return 1;
}

int CThredParser::ReadMatrix(const char *Key, CTransformMatrix& Matrix )
{
	const char *IntString;

	// clear the matrix in case we don't find it
	Matrix.ClearMatrix();

	// try to get our key line
	if(!GetKey(Key) )
		return 0;

	// we know have our key line in OutString
	IntString = (OutString.FindOneOf(WhiteSpace )) + (LPCTSTR)OutString;

	// now read the int
   if(theApp.m_fileVersion < makeVersion(1,93))
   {
	   sscanf(IntString, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf ", 
		   &Matrix.M[0][0],
		   &Matrix.M[0][1],
		   &Matrix.M[0][2],
		   &Matrix.M[0][3],

		   &Matrix.M[1][0],
		   &Matrix.M[1][1],
		   &Matrix.M[1][2],
		   &Matrix.M[1][3],

		   &Matrix.M[2][0],
		   &Matrix.M[2][1],
		   &Matrix.M[2][2],
		   &Matrix.M[2][3],

		   &Matrix.M[3][0],
		   &Matrix.M[3][1],
		   &Matrix.M[3][2],
		   &Matrix.M[3][3] );
   }
   else
   {
	   sscanf(IntString, "%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x ", 
         &Matrix.M[0][0], ((int *)&Matrix.M[0][0]) + 1,
         &Matrix.M[0][1], ((int *)&Matrix.M[0][1]) + 1,
         &Matrix.M[0][2], ((int *)&Matrix.M[0][2]) + 1,
         &Matrix.M[0][3], ((int *)&Matrix.M[0][3]) + 1,

         &Matrix.M[1][0], ((int *)&Matrix.M[1][0]) + 1,
         &Matrix.M[1][1], ((int *)&Matrix.M[1][1]) + 1,
         &Matrix.M[1][2], ((int *)&Matrix.M[1][2]) + 1,
         &Matrix.M[1][3], ((int *)&Matrix.M[1][3]) + 1,

         &Matrix.M[2][0], ((int *)&Matrix.M[2][0]) + 1,
         &Matrix.M[2][1], ((int *)&Matrix.M[2][1]) + 1,
         &Matrix.M[2][2], ((int *)&Matrix.M[2][2]) + 1,
         &Matrix.M[2][3], ((int *)&Matrix.M[2][3]) + 1,

         &Matrix.M[3][0], ((int *)&Matrix.M[3][0]) + 1,
         &Matrix.M[3][1], ((int *)&Matrix.M[3][1]) + 1,
         &Matrix.M[3][2], ((int *)&Matrix.M[3][2]) + 1,
         &Matrix.M[3][3], ((int *)&Matrix.M[3][3]) + 1 );
                                                         
   }

	return 1;
}

int CThredParser::WriteMatrix(const char *String, CTransformMatrix& Matrix )
{
	OutString.Format("%s %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\r\n",
		String, 
      *((int *)&Matrix.M[0][0]), *(((int *)&Matrix.M[0][0]) + 1),
      *((int *)&Matrix.M[0][1]), *(((int *)&Matrix.M[0][1]) + 1),
      *((int *)&Matrix.M[0][2]), *(((int *)&Matrix.M[0][2]) + 1),
      *((int *)&Matrix.M[0][3]), *(((int *)&Matrix.M[0][3]) + 1),

      *((int *)&Matrix.M[1][0]), *(((int *)&Matrix.M[1][0]) + 1),
      *((int *)&Matrix.M[1][1]), *(((int *)&Matrix.M[1][1]) + 1),
      *((int *)&Matrix.M[1][2]), *(((int *)&Matrix.M[1][2]) + 1),
      *((int *)&Matrix.M[1][3]), *(((int *)&Matrix.M[1][3]) + 1),

      *((int *)&Matrix.M[2][0]), *(((int *)&Matrix.M[2][0]) + 1),
      *((int *)&Matrix.M[2][1]), *(((int *)&Matrix.M[2][1]) + 1),
      *((int *)&Matrix.M[2][2]), *(((int *)&Matrix.M[2][2]) + 1),
      *((int *)&Matrix.M[2][3]), *(((int *)&Matrix.M[2][3]) + 1),

      *((int *)&Matrix.M[3][0]), *(((int *)&Matrix.M[3][0]) + 1),
      *((int *)&Matrix.M[3][1]), *(((int *)&Matrix.M[3][1]) + 1),
      *((int *)&Matrix.M[3][2]), *(((int *)&Matrix.M[3][2]) + 1),
      *((int *)&Matrix.M[3][3]), *(((int *)&Matrix.M[3][3]) + 1));

	ar->WriteString(OutString);

	return 1;
}

