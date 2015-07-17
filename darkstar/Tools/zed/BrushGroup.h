// BrushGroup.h : header file
//

#ifndef _BRUSHGROUP_H_
#define _BRUSHGROUP_H_

#include "thredparser.h"
#include "transformmatrix.h"
#include "thredbrush.h"

/////////////////////////////////////////////////////////////////////////////
// CBrushGroup window

// a brushgroup is basically a list of brush
#define NO_GROUP (-1)

class CBrushGroup
{
// Construction
public:
	ThredPoint& GetGroupOrigin() { return mBrushPosition; }
// DPW 10/23 -	void SetGroupOrigin(CThredBrush* mBrushList);
	void SetGroupOrigin(CThredBrush* mBrushList, int mCurrentGroup = -1);
	void Serialize(CThredParser& Parser);
	COLORREF GetColour() { return mColour; }
	void SetColour(COLORREF Colour) { mColour = Colour; }
	const char * GetName();
	void SetName(CString& Name ) { mName = Name; }
	// functions
	CBrushGroup();
    CBrushGroup &operator=(CBrushGroup &Group );  // Right side is the argument.
	int IsVisible() { return mVisible; }
	void SetVisible(int Visible = 1 ) { mVisible = Visible; }

	void DoneRotateBrush(double RotateSnap);
	void DoneMoveBrush(double GridSize);
	void ShearBrush(double X, double Y, double Z);
	void ResetBrush();
	void ScaleBrush(double X, double Y, double Z);
	void MoveBrush(double X, double Y, double Z);
	void RotateBrush(double Yaw, double Pitch, double Roll);
	CTransformMatrix& GetGroupMatrix();

	// are we visible
	int mVisible;

	// name of the brushgroup
	CString mName;

	// colour of the group
	COLORREF mColour;

	ThredPoint mBrushPosition;
	ThredPoint mRenderBrushPosition;
	ThredPoint mBrushRotation;
	ThredPoint mRenderBrushRotation;
	ThredPoint mBrushScale;
	ThredPoint mBrushShear;

	CTransformMatrix mBrushMatrix;

};

/////////////////////////////////////////////////////////////////////////////
typedef CArray<CBrushGroup, CBrushGroup&> CBrushGroupArray;

#endif

