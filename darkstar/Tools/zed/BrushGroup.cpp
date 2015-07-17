// BrushGroup.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "BrushGroup.h"
#include "thredparser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NO_GROUP (-1)
/////////////////////////////////////////////////////////////////////////////
// CBrushGroup

CBrushGroup::CBrushGroup()
{
	mBrushPosition.X = mBrushPosition.Y = mBrushPosition.Z = 128;
	mBrushRotation.X = mBrushRotation.Y = mBrushRotation.Z = 0;
	mBrushScale.X = mBrushScale.Y = mBrushScale.Z = 1;
	mBrushShear.X = mBrushShear.Y = mBrushShear.Z = 0;
	mRenderBrushPosition = mBrushPosition;
	mRenderBrushRotation = mBrushRotation;

	// no name for now
	mName = "Unnamed Group";

	// set our colour to some default (white?)
	mColour = RGB(255,0,0);

	// we are visible
	mVisible = 1;
}


CBrushGroup& CBrushGroup::operator=(CBrushGroup &Group )
{
	mName = Group.mName;
	mColour = Group.mColour;
	mVisible = Group.mVisible;	

	mBrushPosition = Group.mBrushPosition;
	mBrushRotation = Group.mBrushRotation;
	mBrushScale = Group.mBrushScale ;
	mBrushShear = Group.mBrushShear;
	mRenderBrushPosition = Group.mRenderBrushPosition;
	mRenderBrushRotation = Group.mRenderBrushRotation;


	return *this;
}


// get our name
const char * CBrushGroup::GetName()
{
	return (LPCTSTR)mName;
}


#define CBGROUPTYPE "CBGroup"
#define CBGROUPVISIBLE "gVis"
#define CBGROUPNAME "gName"
#define CBGROUPCOLOUR "gCol"
#define CENDBGROUPTYPE "End CBGroup"
void CBrushGroup::Serialize(CThredParser& Parser)
{
	//CThredParser Parser(&ar, CENDBGROUPTYPE);

	Parser.SetEnd(CENDBGROUPTYPE);

	if(Parser.IsStoring ) {
		// we want to write our our transaction type
		Parser.WriteString(CBGROUPTYPE);

		// write our visibility
		Parser.WriteInt(CBGROUPVISIBLE, mVisible);

		// write out name
		Parser.WriteString(CBGROUPNAME, mName);

		// write out color
		Parser.WriteInt(CBGROUPCOLOUR, mColour);

		// we want to write our our transaction type
		Parser.WriteString(CENDBGROUPTYPE);
	}
	else {
		// we want to write our our transaction type
		if(!Parser.GetKey(CBGROUPTYPE) )
			return;

		// write our visibility
		Parser.GetKeyInt(CBGROUPVISIBLE, mVisible);

		// write out name
		Parser.GetKeyString(CBGROUPNAME, mName);

		// write out name
		int InColour;
		Parser.GetKeyInt(CBGROUPCOLOUR, InColour);
		mColour = (COLORREF)InColour;

		// get the end of this one
// DPW - 10/23 -		Parser.GetKey(CENDBGROUPTYPE);

		// go to the end of this type
		Parser.GotoEnd();
	}

}


void CBrushGroup::RotateBrush(double Yaw, double Pitch, double Roll)
{
	if(Globals.mAxis & AXIS_Y )
		mRenderBrushRotation.X += Yaw;
	if(Globals.mAxis & AXIS_X )
		mRenderBrushRotation.Y += Pitch;
	if(Globals.mAxis & AXIS_Z )
		mRenderBrushRotation.Z += Roll;
}

// It seems like the axis's might be busted
// so this is reversed for now till I figure
// out what's going down.
void CBrushGroup::MoveBrush(double X, double Y, double Z)
{
	// change the stuff
	if(Globals.mAxis & AXIS_X ) {
		mRenderBrushPosition.X -= X;
	}
	if(Globals.mAxis & AXIS_Y ) {
		mRenderBrushPosition.Y -= Y;
	}
	if(Globals.mAxis & AXIS_Z ) {
		mRenderBrushPosition.Z -= Z;
	}
}


#define BRUSH_SCALE_MUL	(double)0.005
#define BRUSH_SCALE_LOW_LIMIT	(double)0.1

void CBrushGroup::ScaleBrush(double X, double Y, double Z)
{
	// Do the Change
	if(Globals.mAxis & AXIS_X )
		mBrushScale.X -= X * BRUSH_SCALE_MUL;
	if(Globals.mAxis & AXIS_Y )
		mBrushScale.Y -= Y * BRUSH_SCALE_MUL;
	if(Globals.mAxis & AXIS_Z )
		mBrushScale.Z -= Z * BRUSH_SCALE_MUL;

	// Check the bounds
	if(mBrushScale.X < BRUSH_SCALE_LOW_LIMIT )
		mBrushScale.X = BRUSH_SCALE_LOW_LIMIT;
	if(mBrushScale.Y < BRUSH_SCALE_LOW_LIMIT )
		mBrushScale.Y = BRUSH_SCALE_LOW_LIMIT;
	if(mBrushScale.Z < BRUSH_SCALE_LOW_LIMIT )
		mBrushScale.Z = BRUSH_SCALE_LOW_LIMIT;
}


void CBrushGroup::ResetBrush()
{
	//mBrushPosition.X = mBrushPosition.Y = mBrushPosition.Z = 0;
	mBrushRotation.X = mBrushRotation.Y = mBrushRotation.Z = 0;
	mBrushScale.X = mBrushScale.Y = mBrushScale.Z = 1;
	mBrushShear.X = mBrushShear.Y = mBrushShear.Z = 0;
	mRenderBrushPosition = mBrushPosition;
	mRenderBrushRotation = mBrushRotation;

}


#define BRUSH_SHEAR_MUL	(double)0.005
void CBrushGroup::ShearBrush(double X, double Y, double Z)
{
	// Do the Change
	if(Globals.mAxis & AXIS_X )
		mBrushShear.X += X * BRUSH_SHEAR_MUL;
	if(Globals.mAxis & AXIS_Y )
		mBrushShear.Y += Y * BRUSH_SHEAR_MUL;
	if(Globals.mAxis & AXIS_Z )
		mBrushShear.Z += Z * BRUSH_SHEAR_MUL;
}

// we are done moving the brush
void CBrushGroup::DoneMoveBrush(double GridSize)
{
	// round up the suckers
	if(Globals.mAxis & AXIS_X )
		mRenderBrushPosition.X = RoundFloat((mRenderBrushPosition.X/GridSize) ) * GridSize;
	if(Globals.mAxis & AXIS_Y )
		mRenderBrushPosition.Y = RoundFloat((mRenderBrushPosition.Y/GridSize) ) * GridSize;
	if(Globals.mAxis & AXIS_Z )
		mRenderBrushPosition.Z = RoundFloat((mRenderBrushPosition.Z/GridSize) ) * GridSize;

	// if we are pusing the shit key we want to move only on greatest
	// on the axis we have the most movement on
	
	if(GetAsyncKeyState(VK_SHIFT) & 0x8000) {
		double XDis, YDis, ZDis;
		XDis = fabs(mRenderBrushPosition.X - mBrushPosition.X);
		YDis = fabs(mRenderBrushPosition.Y - mBrushPosition.Y);
		ZDis = fabs(mRenderBrushPosition.Z - mBrushPosition.Z);
		if(XDis > YDis ) {
			if(XDis > ZDis ) 
				mBrushPosition.X = mRenderBrushPosition.X;
			else
				mBrushPosition.Z = mRenderBrushPosition.Z;
		}
		else {
			if(YDis > ZDis )
				mBrushPosition.Y = mRenderBrushPosition.Y;
			else
				mBrushPosition.Z = mRenderBrushPosition.Z;
		}
		mRenderBrushPosition = mBrushPosition;
	}
	else mBrushPosition = mRenderBrushPosition;
}

void CBrushGroup::DoneRotateBrush(double RotateSnap)
{
	// round up the suckers
	if(Globals.mAxis & AXIS_X )
		mRenderBrushRotation.X = RoundFloat((mRenderBrushRotation.X/RotateSnap) ) * RotateSnap;
	if(Globals.mAxis & AXIS_Y )
		mRenderBrushRotation.Y = RoundFloat((mRenderBrushRotation.Y/RotateSnap) ) * RotateSnap;
	if(Globals.mAxis & AXIS_Z )
		mRenderBrushRotation.Z = RoundFloat((mRenderBrushRotation.Z/RotateSnap) ) * RotateSnap;
	
	// if we are pushing the shift key we want to move only on greatest
	// on the axis we have the most movement on
	
	if(GetAsyncKeyState(VK_SHIFT) & 0x8000)
    {
		double XDis, YDis, ZDis;
		XDis = fabs(mRenderBrushRotation.X - mBrushRotation.X);
		YDis = fabs(mRenderBrushRotation.Y - mBrushRotation.Y);
		ZDis = fabs(mRenderBrushRotation.Z - mBrushRotation.Z);
		if(XDis > YDis ) {
			if(XDis > ZDis ) 
				mBrushRotation.X = mRenderBrushRotation.X;
			else
				mBrushRotation.Z = mRenderBrushRotation.Z;
		}
		else {
			if(YDis > ZDis )
				mBrushRotation.Y = mRenderBrushRotation.Y;
			else
				mBrushRotation.Z = mRenderBrushRotation.Z;
		}
		mRenderBrushRotation = mBrushRotation;
	}
	else mBrushRotation = mRenderBrushRotation;
}



//==============================================================
// This function will setup our matrix so that we can
// do the stuff that we need to do.  If you know what
// I mean.
//==============================================================
CTransformMatrix& CBrushGroup::GetGroupMatrix()
{
	// clear the matrix
	mBrushMatrix.ClearMatrix();

	// the position of the group.
	mBrushMatrix.SetTranslation(mRenderBrushPosition.X, mRenderBrushPosition.Y, mRenderBrushPosition.Z);

	// Create the rotation matrix
	mBrushMatrix.CreateRotationMatrix(mRenderBrushRotation.X, mRenderBrushRotation.Y, mRenderBrushRotation.Z );

	// set the scale of the matrix
	mBrushMatrix.SetScale(mBrushScale.X, mBrushScale.Y, mBrushScale.Z);

	// set the shear
	mBrushMatrix.SetShear(mBrushShear.X, mBrushShear.Y, mBrushShear.Z);

	return mBrushMatrix;
}


// Set the origin of the group.
// DPW - 10/23 - void CBrushGroup::SetGroupOrigin(CThredBrush* mBrushList)
void CBrushGroup::SetGroupOrigin(CThredBrush* mBrushList, int mCurrentGroup)
{
	int BrushCount =0;
	ThredPoint AveragePoint(0,0,0);
	ThredPoint p;	// DPW - 10/23

	// go through all of the selected brushes and get there
	// average points so we can make a center point for our group.
	CThredBrush* Current = mBrushList;
	while(Current ) {
		// check if brush is selected
#if 0	// DPW - 10/23
		if(Current->mFlags & BRUSH_SELECTED ) {
			AveragePoint = AveragePoint + Current->GetAveragePoint();
#endif	// DPW - 10/23
		if((mCurrentGroup == -1) && (Current->mFlags & BRUSH_SELECTED) 
			|| mCurrentGroup == Current->mBrushGroup) {
			p = Current->GetAveragePoint();
			AveragePoint = AveragePoint + p;
			BrushCount++;
		}
		Current = Current->GetNextBrush();
	}

	// get the count
	if(BrushCount ) {
		// divide by our count
		AveragePoint.X /= BrushCount;
		AveragePoint.Y /= BrushCount;
		AveragePoint.Z /= BrushCount;
	}

	// now set the origin of this group
	mBrushPosition = AveragePoint;
	mRenderBrushPosition = AveragePoint;
}


