// TransformMatrix.h : header file
//

#ifndef _TRANSFORMMATRIX_H_
#define _TRANSFORMMATRIX_H_

/////////////////////////////////////////////////////////////////////////////
// CTransformMatrix window
#include "thredprimitives.h"


class CTransformMatrix
{
public:
	// here is our actual matrix
	double	M[4][4];

	// constructor
	void ClearMatrix();

	// here are our functions to set different
	// aspects of the matrix
	void SetYaw(double Angle);
	void SetPitch(double Angle);
	void SetRoll(double Angle);
	void SetScale(double XScale, double YScale, double ZScale);
	void SetShear(double XShear, double YShear, double ZShear);

    void SetXYShear(double XShear);
    void SetXZShear(double XShear);
    void SetYXShear(double YShear);
    void SetYZShear(double YShear);
    void SetZXShear(double ZShear);
    void SetZYShear(double ZShear);

    // our code to multiply this matrix
	// by another one and store
	// into this one
	void MultiplyMatrix(CTransformMatrix& Matrix);
	void SetTranslation(double CameraX, double CameraY, double CameraZ);
	void ApplyMatrix(ThredPoint& ApplyPoint);
	void CreateRotationMatrix(double Yaw, double Pitch, double Roll);

};

/////////////////////////////////////////////////////////////////////////////

#endif
