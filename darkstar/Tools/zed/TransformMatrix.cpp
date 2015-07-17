// TransformMatrix.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>
#include "THRED.h"
#include "TransformMatrix.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//=========================================================================*/
// Sets the matrix to NOTHING so that we can multiply
// into it without a problem.
//=========================================================================*/
void CTransformMatrix::ClearMatrix()
{

	// build the actual matrix
	// first row
	M[0][0] = 1;
	M[0][1] = 0;
	M[0][2] = 0;
	M[0][3] = 0;

	// second row
	M[1][0] = 0;
	M[1][1] = 1;
	M[1][2] = 0;
	M[1][3] = 0;

	// third row
	M[2][0] = 0;
	M[2][1] = 0;
	M[2][2] = 1;
	M[2][3] = 0;

	// fourth row
	M[3][0] = 0;
	M[3][1] = 0;
	M[3][2] = 0;
	M[3][3] = 1;
}


//=========================================================================*/
// MultiplyMatrix   
//                                                               
// DESCRIPTION: Multiplies two Matrixes together                 
//                                                               
// PARAMETERS:                                                   
// Result - resultant matrix                                     
// FirstMatrix - The first one to mul by                         
// SecondMatrix - The other one stupid!                          
//                                                               
// RETURNS: Nothing                                              
//                                                               
// AFFECTS: The resultant matrix                                 
//                                                               
//=========================================================================
void CTransformMatrix::MultiplyMatrix(CTransformMatrix& Matrix)
{
	int i, j, k;
	CTransformMatrix FirstMatrix;
	CTransformMatrix SecondMatrix;

	// copy our matrix into the first matrix
	FirstMatrix = Matrix;
	SecondMatrix = *this;

	// do the multiply
	for(i = 0; i < 4; i++)	{
		for(j = 0; j < 4; j++) {
			M[i][j]=0;
			for(k = 0; k < 4; k++ ) {
				M[i][j] += FirstMatrix.M[i][k] * SecondMatrix.M[k][j];
			}
		}
	}
}


//=========================================================================
// SetYaw
//       
// DESCRIPTION: This function creates a matrix to rotate a point by the   
// heading of the point.  The heading is a rotate on the Y axis in the    
// grayrain and hopefully LW coordinate system.  Give the angle in        
// degrees (360) as in the LWScene file.                                  
//                                                                        
// PARAMETERS:                                                            
// Angle - Angle in 360deg units to rotate by                             
//                                                                        
// RETURNS: Nothing                                    
//                                                     
// AFFECTS: The matrix
//                                                     
//                                                     
//  ROTATION ON Y AXIS -                               
//	cos(ya)  0  -sin(ya)  0                            
//		0     1      0     0                           
//	sin(ya)  0   cos(ya)  0                            
//		0     0      0     1                           
//                                                     
//=========================================================================
void CTransformMatrix::SetYaw(double Angle)
{
	CTransformMatrix Heading;
	double Radians;

	// get the radians for the passed angle
	Radians = ((Angle * (double)M_PI_VALUE) / (double)(M_TOTAL_DEGREES/2));


	// build the actual matrix
	// first row
	Heading.M[0][0] = cos(Radians);
	Heading.M[0][1] = 0;
	Heading.M[0][2] = -sin(Radians);
	Heading.M[0][3] = 0;

	// second row
	Heading.M[1][0] = 0;
	Heading.M[1][1] = 1;
	Heading.M[1][2] = 0;
	Heading.M[1][3] = 0;

	// third row
	Heading.M[2][0] = sin(Radians);
	Heading.M[2][1] = 0;
	Heading.M[2][2] = cos(Radians);
	Heading.M[2][3] = 0;

	// fourth row
	Heading.M[3][0] = 0;
	Heading.M[3][1] = 0;
	Heading.M[3][2] = 0;
	Heading.M[3][3] = 1;

	// add in this matrix
	MultiplyMatrix(Heading );
}



//=========================================================================
// SetPitch
//                                                                        
// DESCRIPTION: This function creates a matrix to rotate a point by the   
// heading of the point.  The pitch is a rotate on the X axis in the      
// grayrain and hopefully LW coordinate system.  Give the angle in        
// degrees (360) as in the LWScene file.                                  
//                                                                        
// PARAMETERS:                                                            
// Angle - Angle in 360deg units to rotate by                             
//                                                                        
// RETURNS: Nothing                                                       
//                                                                        
// AFFECTS: The matrix                                              
//                                                                        
//                                                                        
//  ROTATION ON X AXIS -                                                  
//	 1     0       0        0                                             
//	 0  cos(xa) sin(xa)     0                                             
//	 0 -sin(xa) cos(xa)     0                                             
//	 0     0       0        1                                             
//=========================================================================
void CTransformMatrix::SetPitch(double Angle)
{
	CTransformMatrix Pitch;
	double Radians;

	// get the radians for the passed angle
	Radians = ((Angle * (double)M_PI_VALUE) / (double)(M_TOTAL_DEGREES/2));


	// build the actual matrix
	// first row
	Pitch.M[0][0] = 1;
	Pitch.M[0][1] = 0;
	Pitch.M[0][2] = 0;
	Pitch.M[0][3] = 0;

	// second row
	Pitch.M[1][0] = 0;
	Pitch.M[1][1] = cos(Radians);
	Pitch.M[1][2] = sin(Radians);
	Pitch.M[1][3] = 0;

	// third row
	Pitch.M[2][0] = 0;
	Pitch.M[2][1] = -sin(Radians);
	Pitch.M[2][2] = cos(Radians);
	Pitch.M[2][3] = 0;

	// fourth row
	Pitch.M[3][0] = 0;
	Pitch.M[3][1] = 0;
	Pitch.M[3][2] = 0;
	Pitch.M[3][3] = 1;

	// add in this matrix
	MultiplyMatrix(Pitch );

}





//=========================================================================
// mtxCreateBankMatrix                                                    
//                                                                        
// DESCRIPTION: This function creates a matrix to rotate a point by the   
// heading of the point.  The bank is a rotate on the Z axis in the       
// grayrain and hopefully LW coordinate system.  Give the angle in        
// degrees (360) as in the LWScene file.                                  
//                                                                        
// PARAMETERS:                                                            
// Heading - The matrix to put the heading info in.                       
// Angle - Angle in 360deg units to rotate by                             
//                                                                        
// RETURNS: Nothing                                                       
//                                                                        
// AFFECTS: The bank matrix                                               
//                                                                        
//                                                                        
//  ROTATION ON Z AXIS -                                                  
//	 cos(za)	sin(za) 0 0                                               
//	-sin(za) cos(za) 0 0                                                  
//		0       0     1 0                                                 
//		0       0     0 1                                                 
//=========================================================================
void CTransformMatrix::SetRoll(double Angle)
{
	CTransformMatrix Bank;
	double Radians;

	// get the radians for the passed angle
	Radians = ((Angle * (double)M_PI_VALUE) / (double)(M_TOTAL_DEGREES/2));


	// build the actual matrix
	// first row
	Bank.M[0][0] = cos(Radians);
	Bank.M[0][1] = sin(Radians);
	Bank.M[0][2] = 0;
	Bank.M[0][3] = 0;

	// second row
	Bank.M[1][0] = -sin(Radians);
	Bank.M[1][1] = cos(Radians);
	Bank.M[1][2] = 0;
	Bank.M[1][3] = 0;

	// third row
	Bank.M[2][0] = 0;
	Bank.M[2][1] = 0;
	Bank.M[2][2] = 1;
	Bank.M[2][3] = 0;

	// fourth row
	Bank.M[3][0] = 0;
	Bank.M[3][1] = 0;
	Bank.M[3][2] = 0;
	Bank.M[3][3] = 1;

	// add in this matrix
	MultiplyMatrix(Bank );
}




//=========================================================================
// SetScale
//                                                                       
// DESCRIPTION: This function will create a matrix appropriate for       
// scaling an object a set amount on each axis.                          
//                                                                       
// PARAMETERS:                                                           
// XScale, YScale, ZScale - the three scales for each axis               
//                                                                       
// RETURNS: Nothing                                                      
//                                                                       
// AFFECTS: Scale matrix                                                 
//                                                                       
//  Scale Matrix                                                         
//	XS  0  0  0                                                          
//	0  YS  0  0                                                          
//	0   0 ZS  0                                                          
//	0   0  0  1                                                          
//=========================================================================
void CTransformMatrix::SetScale(double XScale, double YScale, double ZScale)
{
	CTransformMatrix Scale;

	// build the actual matrix
	// first row
	Scale.M[0][0] = XScale;
	Scale.M[0][1] = 0;
	Scale.M[0][2] = 0;
	Scale.M[0][3] = 0;

	// second row
	Scale.M[1][0] = 0;
	Scale.M[1][1] = YScale;
	Scale.M[1][2] = 0;
	Scale.M[1][3] = 0;

	// third row
	Scale.M[2][0] = 0;
	Scale.M[2][1] = 0;
	Scale.M[2][2] = ZScale;
	Scale.M[2][3] = 0;

	// fourth row
	Scale.M[3][0] = 0;
	Scale.M[3][1] = 0;
	Scale.M[3][2] = 0;
	Scale.M[3][3] = 1;

	// add in this matrix
	MultiplyMatrix(Scale );
}



//=========================================================================
// Create a full rotation matrix.
//=========================================================================
void CTransformMatrix::CreateRotationMatrix(double Yaw, double Pitch, double Roll)
{
	SetYaw(Yaw );
	SetPitch(Pitch );
	SetRoll(Roll );
}


//=========================================================================
// Apply our translation to the point.
//=========================================================================
void CTransformMatrix::ApplyMatrix(ThredPoint& ApplyPoint)
{
	double OriginalX, OriginalY, OriginalZ;

	// translate the point first
	OriginalX = ApplyPoint.X;
	OriginalY = ApplyPoint.Y;
	OriginalZ = ApplyPoint.Z;

	// NOTE
	// now do the rotation.  Note that later
	// on when the translation is done in the matrix
	// we have to multiply out more of the matrix.
	ApplyPoint.X = 
		 (M[0][0] * OriginalX
		+ M[0][1] * OriginalY
		+ M[0][2] * OriginalZ
		+ M[0][3]);

	ApplyPoint.Y = 
		 (M[1][0] * OriginalX
		+ M[1][1] * OriginalY
		+ M[1][2] * OriginalZ
		+ M[1][3]);

	ApplyPoint.Z = 
		 (M[2][0] * OriginalX
		+ M[2][1] * OriginalY
		+ M[2][2] * OriginalZ
		+ M[2][3]);

}



//=========================================================================
// This function will set these temporary variables that we have
// for translation.  Later on this will be put in the matrix
// but it has to be figured out.
//  Translation Matrix                                                         
//	1   0  0  X                                                          
//	0   1  0  Y                                                          
//	0   0  1  Z                                                          
//	0   0  0  1                                                          
//=========================================================================
void CTransformMatrix::SetTranslation(double CameraX, double CameraY, double CameraZ)
{
	CTransformMatrix Translation;

	// build the actual matrix
	// first row
	Translation.M[0][0] = 1;
	Translation.M[0][1] = 0;
	Translation.M[0][2] = 0;
	Translation.M[0][3] = CameraX;

	// second row
	Translation.M[1][0] = 0;
	Translation.M[1][1] = 1;
	Translation.M[1][2] = 0;
	Translation.M[1][3] = CameraY;

	// third row
	Translation.M[2][0] = 0;
	Translation.M[2][1] = 0;
	Translation.M[2][2] = 1;
	Translation.M[2][3] = CameraZ;

	// fourth row
	Translation.M[3][0] = 0;
	Translation.M[3][1] = 0;
	Translation.M[3][2] = 0;
	Translation.M[3][3] = 1;

	// add in this matrix
	MultiplyMatrix(Translation );
}



//=========================================================================
// SetShear
//                                                                       
// DESCRIPTION: This function will create a matrix appropriate for       
// shearing an object a set amount on each axis.                          
//                                                                       
// PARAMETERS:                                                           
// XShear, YShear, ZShear - the three scales for each axis               
//                                                                       
// RETURNS: Nothing                                                      
//                                                                       
// AFFECTS: Shear matrix                                                 
//                                                                       
//  Shear Matrix                                                         
//	1   0 XS  0                                                          
//	0   1 YS  0                                                          
//	0  ZS  1  0                                                          
//	0   0  0  1                                                          
//=========================================================================
void CTransformMatrix::SetShear(double XShear, double YShear, double ZShear)
{
	CTransformMatrix Shear;

	// build the actual matrix
	// first row
	Shear.M[0][0] = 1;
	Shear.M[0][1] = 0;
	Shear.M[0][2] = XShear;
	Shear.M[0][3] = 0;

	// second row
	Shear.M[1][0] = 0;
	Shear.M[1][1] = 1;
	Shear.M[1][2] = YShear;
	Shear.M[1][3] = 0;

	// third row
	Shear.M[2][0] = 0;
	Shear.M[2][1] = ZShear;
	Shear.M[2][2] = 1;
	Shear.M[2][3] = 0;

	// fourth row
	Shear.M[3][0] = 0;
	Shear.M[3][1] = 0;
	Shear.M[3][2] = 0;
	Shear.M[3][3] = 1;

	// add in this matrix
	MultiplyMatrix(Shear );
}

void CTransformMatrix::SetXYShear(double XShear)
{
	CTransformMatrix Shear;

	// build the actual matrix
	// first row
	Shear.M[0][0] = 1;
	Shear.M[0][1] = XShear;
	Shear.M[0][2] = 0;
	Shear.M[0][3] = 0;

	// second row
	Shear.M[1][0] = 0;
	Shear.M[1][1] = 1;
	Shear.M[1][2] = 0;
	Shear.M[1][3] = 0;

	// third row
	Shear.M[2][0] = 0;
	Shear.M[2][1] = 0;
	Shear.M[2][2] = 1;
	Shear.M[2][3] = 0;

	// fourth row
	Shear.M[3][0] = 0;
	Shear.M[3][1] = 0;
	Shear.M[3][2] = 0;
	Shear.M[3][3] = 1;

	// add in this matrix
	MultiplyMatrix(Shear);
}

void CTransformMatrix::SetXZShear(double XShear)
{
	CTransformMatrix Shear;

	// build the actual matrix
	// first row
	Shear.M[0][0] = 1;
	Shear.M[0][1] = 0;
	Shear.M[0][2] = XShear;
	Shear.M[0][3] = 0;

	// second row
	Shear.M[1][0] = 0;
	Shear.M[1][1] = 1;
	Shear.M[1][2] = 0;
	Shear.M[1][3] = 0;

	// third row
	Shear.M[2][0] = 0;
	Shear.M[2][1] = 0;
	Shear.M[2][2] = 1;
	Shear.M[2][3] = 0;

	// fourth row
	Shear.M[3][0] = 0;
	Shear.M[3][1] = 0;
	Shear.M[3][2] = 0;
	Shear.M[3][3] = 1;

	// add in this matrix
	MultiplyMatrix(Shear);
}

void CTransformMatrix::SetYXShear(double YShear)
{
	CTransformMatrix Shear;

	// build the actual matrix
	// first row
	Shear.M[0][0] = 1;
	Shear.M[0][1] = 0;
	Shear.M[0][2] = 0;
	Shear.M[0][3] = 0;

	// second row
	Shear.M[1][0] = YShear;
	Shear.M[1][1] = 1;
	Shear.M[1][2] = 0;
	Shear.M[1][3] = 0;

	// third row
	Shear.M[2][0] = 0;
	Shear.M[2][1] = 0;
	Shear.M[2][2] = 1;
	Shear.M[2][3] = 0;

	// fourth row
	Shear.M[3][0] = 0;
	Shear.M[3][1] = 0;
	Shear.M[3][2] = 0;
	Shear.M[3][3] = 1;

	// add in this matrix
	MultiplyMatrix(Shear);
}

void CTransformMatrix::SetYZShear(double YShear)
{
	CTransformMatrix Shear;

	// build the actual matrix
	// first row
	Shear.M[0][0] = 1;
	Shear.M[0][1] = 0;
	Shear.M[0][2] = 0;
	Shear.M[0][3] = 0;

	// second row
	Shear.M[1][0] = 0;
	Shear.M[1][1] = 1;
	Shear.M[1][2] = YShear;
	Shear.M[1][3] = 0;

	// third row
	Shear.M[2][0] = 0;
	Shear.M[2][1] = 0;
	Shear.M[2][2] = 1;
	Shear.M[2][3] = 0;

	// fourth row
	Shear.M[3][0] = 0;
	Shear.M[3][1] = 0;
	Shear.M[3][2] = 0;
	Shear.M[3][3] = 1;

	// add in this matrix
	MultiplyMatrix(Shear);
}

void CTransformMatrix::SetZXShear(double ZShear)
{
	CTransformMatrix Shear;

	// build the actual matrix
	// first row
	Shear.M[0][0] = 1;
	Shear.M[0][1] = 0;
	Shear.M[0][2] = 0;
	Shear.M[0][3] = 0;

	// second row
	Shear.M[1][0] = 0;
	Shear.M[1][1] = 1;
	Shear.M[1][2] = 0;
	Shear.M[1][3] = 0;

	// third row
	Shear.M[2][0] = ZShear;
	Shear.M[2][1] = 0;
	Shear.M[2][2] = 1;
	Shear.M[2][3] = 0;

	// fourth row
	Shear.M[3][0] = 0;
	Shear.M[3][1] = 0;
	Shear.M[3][2] = 0;
	Shear.M[3][3] = 1;

	// add in this matrix
	MultiplyMatrix(Shear);
}

void CTransformMatrix::SetZYShear(double ZShear)
{
	CTransformMatrix Shear;

	// build the actual matrix
	// first row
	Shear.M[0][0] = 1;
	Shear.M[0][1] = 0;
	Shear.M[0][2] = 0;
	Shear.M[0][3] = 0;

	// second row
	Shear.M[1][0] = 0;
	Shear.M[1][1] = 1;
	Shear.M[1][2] = 0;
	Shear.M[1][3] = 0;

	// third row
	Shear.M[2][0] = 0;
	Shear.M[2][1] = ZShear;
	Shear.M[2][2] = 1;
	Shear.M[2][3] = 0;

	// fourth row
	Shear.M[3][0] = 0;
	Shear.M[3][1] = 0;
	Shear.M[3][2] = 0;
	Shear.M[3][3] = 1;

	// add in this matrix
	MultiplyMatrix(Shear);
}
