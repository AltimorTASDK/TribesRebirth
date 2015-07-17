// ThredBrush.cpp : implementation file
//

//==============================================================
// BRUSH NOTES:
// We should probably make functions to add polygons etc
// to the brush so that they can be built up by other functions
// which create polygons.  E.g. have a function which makes
// a circle and then call the brush with each thing and let
// the brush allocate it's own memory.  This will allow
// us to free that shit at the end of the brush.  We should
// also have a serialize function within the brush so that
// it can automatically save itself to a file or whatever.
//==============================================================

#include "stdafx.h"
#include <math.h>
#include "THRED.h"
#include "ThredBrush.h"
#include "BrushAttributesDialog.h"
#include "thredparser.h"
#include "threddoc.h"

#include "CreateBoxDialog.h"
#include "CreateSpheroidDialog.h"
//#include "CreateTriDialog.h"
#include "CreateCylDialog.h"
#include "itrgeometry.h"
#include "CreateStaircaseDialog.h"

// DPW
#include "ts_camera.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//==============================================================
// This function is the regular constructor which will just
// zero most things out.
//==============================================================
CThredBrush::CThredBrush()
{
	mMatrix.ClearMatrix();
	mRenderMatrix.ClearMatrix();

	// set a bunch of crap to zero
	mFlags = 0;
	mTransactionType = 0;
	mNumberOfTransformedPolygons = 0;
	mTransformedPolysValid = 0;
	mTransformedPolygons = NULL;
	mNumberOfPolygons = 0;
	mPolygons = NULL;
    mTextureID = NULL;
	mBrushPosition[0].X = mBrushPosition[0].Y = mBrushPosition[0].Z = 0;
	mBrushRotation[0].X = mBrushRotation[0].Y = mBrushRotation[0].Z = 0;
	mBrushScale[0].X = mBrushScale[0].Y = mBrushScale[0].Z = 1;
	mBrushShear[0].X = mBrushShear[0].Y = mBrushShear[0].Z = 0;

	mBrushPosition[1].X = mBrushPosition[1].Y = mBrushPosition[1].Z = 0;
	mBrushRotation[1].X = mBrushRotation[1].Y = mBrushRotation[1].Z = 0;
	mBrushScale[1].X = mBrushScale[1].Y = mBrushScale[1].Z = 1;
	mBrushShear[1].X = mBrushShear[1].Y = mBrushShear[1].Z = 0;
	NextBrush = PrevBrush = NULL;
	mRenderBrushPosition[0] = mBrushPosition[0];
	mRenderBrushRotation[0] = mBrushRotation[0];
	mRenderBrushPosition[1] = mBrushPosition[1];
	mRenderBrushRotation[1] = mBrushRotation[1];
	pDoc = NULL;

    // DPW
    mBrushCenter.X = 0.0;
    mBrushCenter.Y = 0.0;
    mBrushCenter.Z = 0.0;

    mBrushRealCenter.X = 0.0;
    mBrushRealCenter.Y = 0.0;
    mBrushRealCenter.Z = 0.0;

	// our undo arrays
	//mBrushScaleData.SetSize(0, 20);
	//mBrushPositionData.SetSize(0, 20);
	//mBrushRotationData.SetSize(0,20);
	//mBrushShearData.SetSize(0,20);
	//mCurrentUndoPosition = -1;

	// set the id of our brush
	mBrushId = -1;

	// set our name to nothing
	mName = "0";

	// we currently have no group
	mBrushGroup = NO_GROUP;

	// DPW - Needed for TS compatibility
	mBrushColour = 255;

	// we are active
	mFlags |= BRUSH_ACTIVE;

    BrushType = UNKNOWN_BRUSH;

    // Used to link together selected brushes....
    NextSelectedBrush = NULL;
    PrevSelectedBrush = NULL;

    mTotalMatrix.ClearMatrix();

    volumestate = 0;
    m_ShowVolPolys = 0;
}


//==============================================================
// This is the assignment operator for our brush.  Please note that
// this function DOESN'T COPY EVERYTHING.  Also make sure
// that we keep this thing updated!!!!!
//==============================================================
CThredBrush& CThredBrush::operator=(CThredBrush& Brush )
{
	// copy over our name
   mName = Brush.mName;
   mFlags = Brush.mFlags;
   mBrushGroup = Brush.mBrushGroup;
   mEntityId = Brush.mEntityId;
   mMatrix = Brush.mMatrix;
   mRenderMatrix = Brush.mRenderMatrix;
   NextSelectedBrush = Brush.NextSelectedBrush;
   PrevSelectedBrush = Brush.PrevSelectedBrush;
   volumestate = Brush.volumestate;
   m_ShowVolPolys = Brush.m_ShowVolPolys;
   mBoundingBox = Brush.mBoundingBox;

   // get our positional info
   mBrushPosition[0] = Brush.mBrushPosition[0];
   mBrushRotation[0] = Brush.mBrushRotation[0];
   mBrushScale[0] = Brush.mBrushScale[0];
   mBrushShear[0] = Brush.mBrushShear[0];
   mRenderBrushPosition[0] = mBrushPosition[0];
   mRenderBrushRotation[0] = mBrushRotation[0];

   mBrushPosition[1] = Brush.mBrushPosition[1];
   mBrushRotation[1] = Brush.mBrushRotation[1];
   mBrushScale[1] = Brush.mBrushScale[1];
   mBrushShear[1] = Brush.mBrushShear[1];
   mRenderBrushPosition[1] = mBrushPosition[1];
   mRenderBrushRotation[1] = mBrushRotation[1];

   mTotalMatrix = Brush.mTotalMatrix;

   // get the document
   pDoc = Brush.pDoc;

   // save our transaction type
   mTransactionType = Brush.mTransactionType;

   // allocate ourselves some room for polys
   AllocateBrushPolygons(Brush.mNumberOfPolygons );
   AllocateBrushPolyTextures(Brush.mNumberOfPolygons );

   // copy over the gons
   for(int CurrentPoly = 0; CurrentPoly < mNumberOfPolygons; CurrentPoly++)
   {
      mPolygons[CurrentPoly] = Brush.mPolygons[CurrentPoly];
      mTextureID[CurrentPoly].TextureID = Brush.mTextureID[CurrentPoly].TextureID;
      mTextureID[CurrentPoly].TextureShift.x = Brush.mTextureID[CurrentPoly].TextureShift.x;
      mTextureID[CurrentPoly].TextureShift.y = Brush.mTextureID[CurrentPoly].TextureShift.y;
      mTextureID[CurrentPoly].TextureFlipH = Brush.mTextureID[CurrentPoly].TextureFlipH;
      mTextureID[CurrentPoly].TextureFlipV = Brush.mTextureID[CurrentPoly].TextureFlipV;
      mTextureID[CurrentPoly].TextureRotate = Brush.mTextureID[CurrentPoly].TextureRotate;
      mTextureID[CurrentPoly].TextureScale = Brush.mTextureID[CurrentPoly].TextureScale;
   }

   // copy over our undo data.  All of these arrays
   // should be the same size
   //mBrushScaleData.SetSize(0,20);
   //mBrushPositionData.SetSize(0,20);
   //mBrushRotationData.SetSize(0,20);
   //mBrushShearData.SetSize(0,20);
   //if(Brush.mBrushScaleData.GetUpperBound() >= 0 )
   //	for(Index = 0; Index <= Brush.mBrushScaleData.GetUpperBound(); Index++ ) {
   //		mBrushScaleData.SetAtGrow(Index, Brush.mBrushScaleData[Index]);
   //		mBrushPositionData.SetAtGrow(Index, Brush.mBrushPositionData[Index]);
   //		mBrushRotationData.SetAtGrow(Index, Brush.mBrushRotationData[Index]);
   //		mBrushShearData.SetAtGrow(Index, Brush.mBrushShearData[Index]);
   //}
   //mCurrentUndoPosition = Brush.mCurrentUndoPosition;

   BrushType = Brush.BrushType;

   switch (Brush.BrushType)
   {
      case UNKNOWN_BRUSH:
         break;
      case BOX_BRUSH:
         BrushSpecifics.box.m_YSize = Brush.BrushSpecifics.box.m_YSize;
         BrushSpecifics.box.m_Solid = Brush.BrushSpecifics.box.m_Solid;
         BrushSpecifics.box.m_Thickness = Brush.BrushSpecifics.box.m_Thickness;
         BrushSpecifics.box.m_XSizeBot = Brush.BrushSpecifics.box.m_XSizeBot;
         BrushSpecifics.box.m_XSizeTop = Brush.BrushSpecifics.box.m_XSizeTop;
         BrushSpecifics.box.m_ZSizeBot = Brush.BrushSpecifics.box.m_ZSizeBot;
         BrushSpecifics.box.m_ZSizeTop = Brush.BrushSpecifics.box.m_ZSizeTop;
         break;
      case TRI_BRUSH:
         BrushSpecifics.triangle.m_Solid = Brush.BrushSpecifics.triangle.m_Solid;
         BrushSpecifics.triangle.m_Thickness = Brush.BrushSpecifics.triangle.m_Thickness;
         BrushSpecifics.triangle.m_XSizeBot = Brush.BrushSpecifics.triangle.m_XSizeBot;
         BrushSpecifics.triangle.m_YSize = Brush.BrushSpecifics.triangle.m_YSize;
         BrushSpecifics.triangle.m_ZSizeBot = Brush.BrushSpecifics.triangle.m_ZSizeBot;
         BrushSpecifics.triangle.m_ZSizeTop = Brush.BrushSpecifics.triangle.m_ZSizeTop;
         break;
      case CYL_BRUSH:
         BrushSpecifics.cylinder.m_BotXOffset = Brush.BrushSpecifics.cylinder.m_BotXOffset;
         BrushSpecifics.cylinder.m_BotXSize = Brush.BrushSpecifics.cylinder.m_BotXSize;
         BrushSpecifics.cylinder.m_BotZOffset = Brush.BrushSpecifics.cylinder.m_BotZOffset;
         BrushSpecifics.cylinder.m_BotZSize = Brush.BrushSpecifics.cylinder.m_BotZSize;
         BrushSpecifics.cylinder.m_Solid = Brush.BrushSpecifics.cylinder.m_Solid;
         BrushSpecifics.cylinder.m_Thickness = Brush.BrushSpecifics.cylinder.m_Thickness;
         BrushSpecifics.cylinder.m_TopXOffset = Brush.BrushSpecifics.cylinder.m_TopXOffset;
         BrushSpecifics.cylinder.m_TopXSize = Brush.BrushSpecifics.cylinder.m_TopXSize;
         BrushSpecifics.cylinder.m_TopZOffset = Brush.BrushSpecifics.cylinder.m_TopZOffset;
         BrushSpecifics.cylinder.m_TopZSize = Brush.BrushSpecifics.cylinder.m_TopZSize;
         BrushSpecifics.cylinder.m_VerticalStripes = Brush.BrushSpecifics.cylinder.m_VerticalStripes;
         BrushSpecifics.cylinder.m_YSize = Brush.BrushSpecifics.cylinder.m_YSize;
         BrushSpecifics.cylinder.m_Ring = Brush.BrushSpecifics.cylinder.m_Ring;
         break;
      case SPHERE_BRUSH:
         BrushSpecifics.sphere.m_HorizontalBands = Brush.BrushSpecifics.sphere.m_HorizontalBands;
         BrushSpecifics.sphere.m_VerticalBands = Brush.BrushSpecifics.sphere.m_VerticalBands;
         BrushSpecifics.sphere.m_XSize = Brush.BrushSpecifics.sphere.m_XSize;
         BrushSpecifics.sphere.m_YSize = Brush.BrushSpecifics.sphere.m_YSize;
         BrushSpecifics.sphere.m_ZSize = Brush.BrushSpecifics.sphere.m_ZSize;
         BrushSpecifics.sphere.m_Solid = Brush.BrushSpecifics.sphere.m_Solid;
         BrushSpecifics.sphere.m_Thickness = Brush.BrushSpecifics.sphere.m_Thickness;
         BrushSpecifics.sphere.m_HalfSphere = Brush.BrushSpecifics.sphere.m_HalfSphere;
         break;
      case STAIR_BRUSH:
         BrushSpecifics.staircase.m_Height = Brush.BrushSpecifics.staircase.m_Height;
         BrushSpecifics.staircase.m_Length = Brush.BrushSpecifics.staircase.m_Length;
         BrushSpecifics.staircase.m_NumberOfStairs = Brush.BrushSpecifics.staircase.m_NumberOfStairs;
         BrushSpecifics.staircase.m_Width = Brush.BrushSpecifics.staircase.m_Width;
         BrushSpecifics.staircase.m_MakeRamp = Brush.BrushSpecifics.staircase.m_MakeRamp;
         break;
         
   }

   return *this;
}


CThredBrush::~CThredBrush()
{
	if(mTransformedPolygons)
    {
		delete []mTransformedPolygons;
        mTransformedPolygons = NULL;
    }

	if(mPolygons) 
	{
		delete []mPolygons;
        mPolygons = NULL;
	}

    if (mTextureID)
    {
        delete []mTextureID;
        mTextureID = NULL;
    }
}

//==============================================================
// this function doesn't setup the camera matrix
// if you are calling this make sure it is setup
//==============================================================
void CThredBrush::ShowOrigin(CRenderCamera& Camera, COLORREF Colour, TSRenderContext* rc)
{
	ThredPoint TranPoint;
	CPoint Point;

	// get our location
	TranPoint.X = mBrushCenter.X;
	TranPoint.Y = mBrushCenter.Y;
	TranPoint.Z = mBrushCenter.Z;

	// figure out where it is.
	//mBrushMatrix.ApplyMatrix(TranPoint);

	// translate it to the screen
	Camera.TranslateToScreen(Point, TranPoint, rc);

	// draw the point
	Camera.DrawPoint(Point, Colour, rc);
}

//==============================================================
// This function will go through all of the verts of all of
// the polygons in the brush and transform them so that
// we can get the proper world coordinates out of them.
//==============================================================
void CThredBrush::TransformBrushPolygons()
{
    Point3F tmpmin(+1.0E20F, +1.0E20F, +1.0E20F);
    Point3F tmpmax(-1.0E20F, -1.0E20F, -1.0E20F);

	// they are already good man.. don't mess with a good thing
	//if(mTransformedPolysValid )
	//	return;

	// get our matrix ready
	SetupBrushMatrix();

	// check our array and allocate if necessary
	if(mTransformedPolygons )
    {
		// if we are allocated with a different number
		// then delete and re-allocate
		if(mNumberOfTransformedPolygons != mNumberOfPolygons )
        {
			delete []mTransformedPolygons;
			mTransformedPolygons = new ThredPolygon[mNumberOfPolygons];
			mNumberOfTransformedPolygons = mNumberOfPolygons;
		}
	} else {
		// else allocate some man.
		mTransformedPolygons = new ThredPolygon[mNumberOfPolygons];
		mNumberOfTransformedPolygons = mNumberOfPolygons;
	}
	
	// clear our bounding box
	mBoundingBox.ClearBox();

	// This is basically a loop where we go through
	// each polygon and transform its points to
	// the proper world coordinates.
	for(int CurrentPoly = 0; CurrentPoly < mNumberOfPolygons; CurrentPoly++)
    {

		ThredPolygon* pPoly = &mPolygons[CurrentPoly];
		ThredPolygon* pNewPoly = &mTransformedPolygons[CurrentPoly];
		pNewPoly->NumberOfPoints = pPoly->NumberOfPoints;
      pNewPoly->mTextureScaleShift = pPoly->mTextureScaleShift;
      pNewPoly->mApplyAmbient = pPoly->mApplyAmbient;
		_ASSERTE(pPoly->NumberOfPoints >= 3);

		// go through and transform
		for(int CurrentVert = 0; CurrentVert < pPoly->NumberOfPoints; CurrentVert++)
        {
			ThredPoint TranPoint;
			TranPoint = pPoly->Points[CurrentVert];

			mTotalMatrix.ApplyMatrix(TranPoint);

			// SAVE THE TRANSFORMED
			pNewPoly->Points[CurrentVert].X = TranPoint.X;
			pNewPoly->Points[CurrentVert].Y = TranPoint.Y;
			pNewPoly->Points[CurrentVert].Z = TranPoint.Z;

         // apply some snapping to the points
//         pNewPoly->Points[CurrentVert].Snap( 1.0 );

			// make our bounding box bigger
			mBoundingBox.AddPoint(pNewPoly->Points[CurrentVert] );

            // DPW
            pNewPoly->mTextureUV[CurrentVert].x = pPoly->mTextureUV[CurrentVert].x;
            pNewPoly->mTextureUV[CurrentVert].y = pPoly->mTextureUV[CurrentVert].y;

            // DPW - Determine dimensions of MY bounding box...
            if (pNewPoly->Points[CurrentVert].X < tmpmin.x)
              tmpmin.x = pNewPoly->Points[CurrentVert].X;
            if (pNewPoly->Points[CurrentVert].Y < tmpmin.y)
              tmpmin.y = pNewPoly->Points[CurrentVert].Y;
            if (pNewPoly->Points[CurrentVert].Z < tmpmin.z)
              tmpmin.z = pNewPoly->Points[CurrentVert].Z;

            if (pNewPoly->Points[CurrentVert].X > tmpmax.x)
              tmpmax.x = pNewPoly->Points[CurrentVert].X;
            if (pNewPoly->Points[CurrentVert].Y > tmpmax.y)
              tmpmax.y = pNewPoly->Points[CurrentVert].Y;
            if (pNewPoly->Points[CurrentVert].Z > tmpmax.z)
              tmpmax.z = pNewPoly->Points[CurrentVert].Z;
        }

        // DPW
        pNewPoly->mTextureOffset.x = pPoly->mTextureOffset.x;
        pNewPoly->mTextureOffset.y = pPoly->mTextureOffset.y;

        pNewPoly->mTextureSize.x = pPoly->mTextureSize.x;
        pNewPoly->mTextureSize.y = pPoly->mTextureSize.y;
	}

	// we just did it tell them it's valid man
	mTransformedPolysValid = TRUE;

    // DPW - Build MY bounding box...
    SetBBoxValues (tmpmin, tmpmax);
}


void CThredBrush::SetBBoxValues(Point3F min, Point3F max)
{
    for (int j = 0; j < 6; j++)
    {
      BBox[j].NumberOfPoints = 4;
    }

    // Front
    BBox[0].Points[0].X = min.x;
    BBox[0].Points[0].Y = max.y;
    BBox[0].Points[0].Z = min.z;

    BBox[0].Points[1].X = max.x;
    BBox[0].Points[1].Y = max.y;
    BBox[0].Points[1].Z = min.z;

    BBox[0].Points[2].X = max.x;
    BBox[0].Points[2].Y = min.y;
    BBox[0].Points[2].Z = min.z;

    BBox[0].Points[3].X = min.x;
    BBox[0].Points[3].Y = min.y;
    BBox[0].Points[3].Z = min.z;

    // Back
    BBox[1].Points[0].X = max.x;
    BBox[1].Points[0].Y = max.y;
    BBox[1].Points[0].Z = max.z;

    BBox[1].Points[1].X = min.x;
    BBox[1].Points[1].Y = max.y;
    BBox[1].Points[1].Z = max.z;

    BBox[1].Points[2].X = min.x;
    BBox[1].Points[2].Y = min.y;
    BBox[1].Points[2].Z = max.z;

    BBox[1].Points[3].X = max.x;
    BBox[1].Points[3].Y = min.y;
    BBox[1].Points[3].Z = max.z;

    // Top
    BBox[2].Points[0].X = min.x;
    BBox[2].Points[0].Y = max.y;
    BBox[2].Points[0].Z = min.z;

    BBox[2].Points[1].X = min.x;
    BBox[2].Points[1].Y = max.y;
    BBox[2].Points[1].Z = max.z;

    BBox[2].Points[2].X = max.x;
    BBox[2].Points[2].Y = max.y;
    BBox[2].Points[2].Z = max.z;

    BBox[2].Points[3].X = max.x;
    BBox[2].Points[3].Y = max.y;
    BBox[2].Points[3].Z = min.z;

    // Bottom
    BBox[3].Points[0].X = min.x;
    BBox[3].Points[0].Y = min.y;
    BBox[3].Points[0].Z = min.z;

    BBox[3].Points[1].X = max.x;
    BBox[3].Points[1].Y = min.y;
    BBox[3].Points[1].Z = min.z;

    BBox[3].Points[2].X = max.x;
    BBox[3].Points[2].Y = min.y;
    BBox[3].Points[2].Z = max.z;

    BBox[3].Points[3].X = min.x;
    BBox[3].Points[3].Y = min.y;
    BBox[3].Points[3].Z = max.z;

    // Right
    BBox[4].Points[0].X = max.x;
    BBox[4].Points[0].Y = max.y;
    BBox[4].Points[0].Z = min.z;

    BBox[4].Points[1].X = max.x;
    BBox[4].Points[1].Y = max.y;
    BBox[4].Points[1].Z = max.z;

    BBox[4].Points[2].X = max.x;
    BBox[4].Points[2].Y = min.y;
    BBox[4].Points[2].Z = max.z;

    BBox[4].Points[3].X = max.x;
    BBox[4].Points[3].Y = min.y;
    BBox[4].Points[3].Z = min.z;

    // Left
    BBox[5].Points[0].X = min.x;
    BBox[5].Points[0].Y = max.y;
    BBox[5].Points[0].Z = max.z;

    BBox[5].Points[1].X = min.x;
    BBox[5].Points[1].Y = max.y;
    BBox[5].Points[1].Z = min.z;

    BBox[5].Points[2].X = min.x;
    BBox[5].Points[2].Y = min.y;
    BBox[5].Points[2].Z = min.z;

    BBox[5].Points[3].X = min.x;
    BBox[5].Points[3].Y = min.y;
    BBox[5].Points[3].Z = max.z;
}



//==============================================================
// This function will create bsp for the brush using
// the transformed coordinates of the bloody thing.
//==============================================================
void CThredBrush::CreateBrushBsp()
{
	TransformBrushPolygons();

	// if our transformed polys aren't valid assert
	_ASSERTE(mTransformedPolysValid );

	// clear the bsp
	mBrushBsp.ClearBsp();

	// go through the list of gons in the brush
	// and send them into the bsp
	for(int CurrentGon = 0; CurrentGon < mNumberOfPolygons; CurrentGon++ ) {
		CBspNode AddPolygon;
		AddPolygon.AssignPolygon(&mTransformedPolygons[CurrentGon]);
        AddPolygon.mBrushPtr = this;
		mBrushBsp.AddPolygonToTree(&AddPolygon);
	}
}

//===========================================================
// this function gets rid of all the polygons in the brush
//===========================================================
void CThredBrush::DeleteBrushPolygons()
{
	// delete the polygons
	if(mPolygons ) 
		delete []mPolygons;

	// set em to none.
	mNumberOfPolygons = 0;
	mPolygons = NULL;

}

//===========================================================
// this function gets rid of all the polygon texture ids in the brush
//===========================================================
void CThredBrush::DeleteBrushPolyTextures()
{
	// delete the polygon texture ids
	if(mTextureID ) 
		delete []mTextureID;

	// set it to none.
	mTextureID = NULL;
}

void CThredBrush::AllocateBrushPolygons(int NumberOfPolygons)
{
	mTransformedPolysValid = FALSE;

	// why reallocate when you don't have to?
	// we will keep the list at it's currentsize
	if(mNumberOfPolygons < NumberOfPolygons ) {
		// get rid of the gons
		DeleteBrushPolygons();

		// allocate some room for new gons
		mPolygons = new ThredPolygon[NumberOfPolygons];
	}
   
	mNumberOfPolygons = NumberOfPolygons;
}


void CThredBrush::AllocateBrushPolyTextures (int NumberOfPolygons)
{
    if (NumberOfPolygons == 0)
      return;

    // This function will check for a null pointer so don't worry about it here...
    DeleteBrushPolyTextures();

	// allocate some room for new gons
	mTextureID = new Texinfo[NumberOfPolygons];

    for (int i = 0; i < NumberOfPolygons; i++)
    {
      mTextureID[i].TextureID = 0;
      mTextureID[i].TextureShift.x = 0;
      mTextureID[i].TextureShift.y = 0;
      mTextureID[i].TextureFlipH = 0;
      mTextureID[i].TextureFlipV = 0;
      mTextureID[i].TextureRotate = 0.F;
      mTextureID[i].TextureScale = theApp.m_texturescale;
    }
}

void CThredBrush::ResetBrush(ThredPoint newpos, int MoveType)
{
    mBrushPosition[MoveType].X = newpos.X;
    mBrushPosition[MoveType].Y = newpos.Y;
    mBrushPosition[MoveType].Z = newpos.Z;

	mBrushRotation[MoveType].X = mBrushRotation[MoveType].Y = mBrushRotation[MoveType].Z = 0;
	mBrushScale[MoveType].X = mBrushScale[MoveType].Y = mBrushScale[MoveType].Z = 1;
	mBrushShear[MoveType].X = mBrushShear[MoveType].Y = mBrushShear[MoveType].Z = 0;
	mRenderBrushPosition[MoveType] = mBrushPosition[MoveType];
	mRenderBrushRotation[MoveType] = mBrushRotation[MoveType];

    // Reset the total transform matrix
    mTotalMatrix.ClearMatrix();
    //mTotalMatrix.SetTranslation(newpos.X - mBrushPosition[MoveType].X,
    //                            newpos.Y - mBrushPosition[MoveType].Y,
    //                            newpos.Z - mBrushPosition[MoveType].Z);
    mTotalMatrix.SetTranslation(newpos.X,
                                newpos.Y,
                                newpos.Z);

	mTransformedPolysValid = FALSE;
}

//==============================================================
// This function brings up a dialog which we can use
// to adjust many of the attributes of the brush.
//==============================================================
void CThredBrush::BrushAttributesDialog(CTHREDDoc* doc, int MoveType)
{
	CBrushAttributesDialog Dialog;

	// set all of our vars
	Dialog.m_Position_X = mBrushPosition[MoveType].X; 
	Dialog.m_Position_Y = mBrushPosition[MoveType].Y;
	Dialog.m_Position_Z = mBrushPosition[MoveType].Z;
	Dialog.m_Rotation_Yaw = (mBrushRotation[MoveType].X / M_TOTAL_DEGREES)*360;
	Dialog.m_Rotation_Pitch = (mBrushRotation[MoveType].Y / M_TOTAL_DEGREES)*360;
	Dialog.m_Rotation_Roll = (mBrushRotation[MoveType].Z / M_TOTAL_DEGREES)*360;
	Dialog.m_Scale_X = mBrushScale[MoveType].X;
	Dialog.m_Scale_Y = mBrushScale[MoveType].Y;
	Dialog.m_Scale_Z = mBrushScale[MoveType].Z;
	Dialog.m_XShear = mBrushShear[MoveType].X;
	Dialog.m_YShear = mBrushShear[MoveType].Y;
	Dialog.m_ZShear = mBrushShear[MoveType].Z;
	Dialog.m_Name = mName;

	Dialog.material_count = doc->GetMaterialList()->getMaterialsCount();
	Dialog.material_list = doc->GetMaterialList();
	Dialog.palette = doc->GetPalette();

    // DPW
    Dialog.m_BBoxX = mBoundingBox.mLargest.X - mBoundingBox.mSmallest.X;
    Dialog.m_BBoxY = mBoundingBox.mLargest.Y - mBoundingBox.mSmallest.Y;
    Dialog.m_BBoxZ = mBoundingBox.mLargest.Z - mBoundingBox.mSmallest.Z;

    if (mTransactionType == BRUSH_ADD_TO_WORLD)
    {
      Dialog.m_VolumeState = volumestate;
      Dialog.m_ShowVolPolys = (BOOL)m_ShowVolPolys;
      if (volumestate)
      {
        Dialog.m_Additive = FALSE;
        Dialog.m_Subtractive = FALSE;
        Dialog.m_Volume = TRUE;
      } else {
        Dialog.m_Additive = TRUE;
        Dialog.m_Subtractive = FALSE;
        Dialog.m_Volume = FALSE;
      }
    } else {
      Dialog.m_VolumeState = 0;
      Dialog.m_Additive = FALSE;
      Dialog.m_Subtractive = TRUE;
      Dialog.m_Volume = FALSE;
    }

    // Fill in the states into the dropdown
    //Dialog.m_VolumeDescriptions[0].Format("%2d - %s", 0, "Normal Volume");
    for (int x = 0; x < TOTAL_VOLUME_STATES; x++)
    {
      Dialog.m_VolumeDescriptions[x].Format("%2d - %s", x, doc->m_VolumeDescription[x]);
    }

	// Do the dialog
	if(Dialog.DoModal() != IDCANCEL )
    {
		// Get all of our vars
		mBrushPosition[MoveType].X = Dialog.m_Position_X;
		mBrushPosition[MoveType].Y = Dialog.m_Position_Y;
		mBrushPosition[MoveType].Z = Dialog.m_Position_Z;
		mBrushRotation[MoveType].X = (Dialog.m_Rotation_Yaw / 360)*M_TOTAL_DEGREES;
		mBrushRotation[MoveType].Y = (Dialog.m_Rotation_Pitch / 360)*M_TOTAL_DEGREES;
		mBrushRotation[MoveType].Z = (Dialog.m_Rotation_Roll / 360)*M_TOTAL_DEGREES;
		mBrushScale[MoveType].X = Dialog.m_Scale_X;
		mBrushScale[MoveType].Y = Dialog.m_Scale_Y;
		mBrushScale[MoveType].Z = Dialog.m_Scale_Z;
		mBrushShear[MoveType].X = Dialog.m_XShear;
		mBrushShear[MoveType].Y = Dialog.m_YShear;
		mBrushShear[MoveType].Z = Dialog.m_ZShear;

        // DPW - only update face texture ids if name was changed
        if (atoi(mName) != Dialog.material_index1)
        {
          mName.Format("%d", Dialog.material_index1);

          for (int i = 0; i < mNumberOfPolygons; i++)
          {
            mTextureID[i].TextureID = atoi(mName);
            mTextureID[i].TextureShift.x = 0;
            mTextureID[i].TextureShift.y = 0;
            mTextureID[i].TextureRotate = 0.F;
            mTextureID[i].TextureFlipH = 0;
            mTextureID[i].TextureFlipV = 0;
            mTextureID[i].TextureScale = theApp.m_texturescale;
          }
        }

		mRenderBrushPosition[MoveType] = mBrushPosition[MoveType];
		mRenderBrushRotation[MoveType] = mBrushRotation[MoveType];
        m_ShowVolPolys = Dialog.m_ShowVolPolys;

        if (Dialog.m_Additive)
        {
          mTransactionType = BRUSH_ADD_TO_WORLD;
          volumestate = 0;
        } else {
          if (Dialog.m_Subtractive)
          {
            mTransactionType = BRUSH_SUBTRACT_FROM_WORLD;
            volumestate = 0;
          } else {
            mTransactionType = BRUSH_ADD_TO_WORLD;
            volumestate = Dialog.m_VolumeState;
          }
        }
	}
	mTransformedPolysValid = FALSE;
}

//==============================================================
// UNDO stuff
//==============================================================
//int CThredBrush::CanUndo()
//{
//		return 0;
//}

// if our current undo position is less than our
// upper bound then we can redo
//int CThredBrush::CanRedo()
//{
//	return 0;
//}

void CThredBrush::RenderBrushStandard(CRenderCamera& Camera, int color, int color_select, TSRenderContext* rc)
{
	// When rendering a brush we want to give
	// the colour of the brush and the colour of it's
	// points
	RenderBrush(Camera, color, color, color_select, 0, rc, false);
}



//==============================================================
// This function will create bsp for the brush using
// the transformed coordinates of the bloody thing.
//==============================================================
void CThredBrush::CreateQuickBrushBsp(CBspNode* Polygon)
{
	_ASSERTE(mTransformedPolysValid);

	// clear the bsp (but we don't delete the nodes!!)
	mBrushBsp.TreeTop = NULL;
	mBrushBsp.InitializeQuickBspNodes(mNumberOfPolygons);

	// go through the list of gons in the brush
	// and send them into the bsp
	for(int CurrentGon = 0; CurrentGon < mNumberOfPolygons; CurrentGon++ ) {
		ThredPolygon AddPolygon;
		AddPolygon = mTransformedPolygons[CurrentGon];

		mBrushBsp.AddQuickPolygonToTree(&AddPolygon, Polygon, this);
	}
}


//==============================================================
// This function renders the brush to the device context
// and camera that is passed to it.
//==============================================================
void CThredBrush::RenderBrush(CRenderCamera& Camera, COLORREF LineColour, COLORREF PointColour, COLORREF SelectColour, COLORREF BBoxColour, TSRenderContext* rc, bool showbb)
{
    int i;

    // return if not active.
	if(!(mFlags & BRUSH_ACTIVE))
		return;

	// Transform the polygons
    TransformBrushPolygons();

	// if we are selected
	if(mFlags & BRUSH_SELECTED)
    {
		LineColour = SelectColour;
		PointColour = PointColour;
	}

	for(i = 0; i < mNumberOfPolygons; i++)
    {
        Camera.DrawWireframePolygon(&mTransformedPolygons[i], LineColour, PointColour, rc, FALSE, FALSE);
    }
}	

//==============================================================
// This function will check if the passed point is on one
// of the brush lines (to see whether we should select the
// brush or not).
//==============================================================
int CThredBrush::CheckPoint(ThredPoint Point)
{
	TransformBrushPolygons();
	return mBoundingBox.CheckPointIntersection(Point);
}

// set the name
void CThredBrush::SetName(CString& Name)
{
	mName = Name;
}

//=================================================================
// This function iterates through the list of brushes looking
// for the next brush that belongs to this entity.  If it doesn't
// find it it returns null.
//=================================================================
// some defines for different brush flags
//#define BRUSH_ACTIVE
//#define	BRUSH_SELECTED
//#define BRUSH_ENTITY
CThredBrush*  CThredBrush::NextEntityBrush(int EntityNumber)
{
	CThredBrush* CurrentBrush = this->NextBrush;

	// go through through the brushes looking for ones
	// that match the flags
	while(CurrentBrush ) {
		if(CurrentBrush->mFlags & BRUSH_ENTITY )
			if(CurrentBrush->mEntityId == EntityNumber)
				return CurrentBrush;

		CurrentBrush = CurrentBrush->NextBrush;
	}

	// nuttin' honey
	return NULL;
}

//=================================================================
// This function returns the next active brush in the list.
// The flag you pass in defines what types of brushes it
// will take.  basically the brush has to match the
// passed in flags
//=================================================================
CThredBrush* CThredBrush::NextActiveBrush()
{
	CThredBrush* CurrentBrush = this->NextBrush;

	// go through through the brushes looking for ones
	// that match the flags
	while(CurrentBrush ) {
		if(CurrentBrush->mFlags & BRUSH_ACTIVE )
			return CurrentBrush;
		CurrentBrush = CurrentBrush->NextBrush;
	}

	// nuttin' honey
	return NULL;
}

//=================================================================
// goes through and gets the next brush that is active whether it
// be the current etc
//=================================================================
CThredBrush* CThredBrush::GetActiveBrush()
{
	CThredBrush* CurrentBrush = this;

	// go through through the brushes looking for ones
	// that match the flags
	while(CurrentBrush ) {
		if(!(CurrentBrush->mFlags & BRUSH_ENTITY) ) {
			if(CurrentBrush->mFlags & BRUSH_ACTIVE )
				return CurrentBrush;
		}
		CurrentBrush = CurrentBrush->NextBrush;
	}

	// nuttin' honey
	return NULL;
}

void CThredBrush::RemoveBrush()
{
	// get rid of the brush in the order
	if(GetNextBrush())
    {
		NextBrush->PrevBrush = PrevBrush;
    }

	if(PrevBrush)
    {
		PrevBrush->NextBrush = NextBrush;
    }

    if(PrevSelectedBrush)
    {
      PrevSelectedBrush->NextSelectedBrush = NextSelectedBrush;
    }

    if(NextSelectedBrush)
    {
      NextSelectedBrush->PrevSelectedBrush = PrevBrush;
    }
}

CThredBrush* CThredBrush::GetBrushById(int BrushId )
{
	CThredBrush* CurrentBrush = this;

	while(CurrentBrush ) {

		// check if correct
		if(CurrentBrush->mBrushId == BrushId )
			return CurrentBrush;

		// on to next brush
		CurrentBrush = CurrentBrush->GetNextBrush();
	}

	return NULL;
}

// all of the group is reset here.
void CThredBrush::ResetGroupItems()
{
	mBrushRotation[1].X = mBrushRotation[1].Y = mBrushRotation[1].Z = 0;
	mBrushScale[1].X = mBrushScale[1].Y = mBrushScale[1].Z = 1;
	mBrushShear[1].X = mBrushShear[1].Y = mBrushShear[1].Z = 0;
	mRenderBrushRotation[1] = mBrushRotation[1];
}

/*************************************************
  1.  Make it so that when something is done to a brush it is stored in a matrix.
  This matrix is the one that is used to transform a brushes points after they have
  been transformed around its "local" coordinates.

  Things that are done on "local" coordinates are 1) rotation 2) scale 3) shear.

  So when a brush is moved it is done to its "secondary" matrix.  To figure
  out at which point a brush is at all you do is:
  1) make a matrix that represents it's local transformations
  2) concatanate that with a matrix that represents all of its "other"
  movements.  
  3) transform each point.
*****************************************************/

//==============================================================
// This function will setup our matrix so that we can
// do the stuff that we need to do.  If you know what
// I mean.
//==============================================================
void CThredBrush::SetupBrushMatrix()
{
    CTransformMatrix  ToMatrix, FromMatrix, TempMatrix;

	// clear the matrix
	mBrushMatrix.ClearMatrix();

	/*************  BRUSH *************/
	// set the shear
	mBrushMatrix.SetShear(mBrushShear[0].X, mBrushShear[0].Y, mBrushShear[0].Z);

	// set the scale of the matrix
	mBrushMatrix.SetScale(mBrushScale[0].X, mBrushScale[0].Y, mBrushScale[0].Z);

	// Create the rotation matrix
    ToMatrix.ClearMatrix();
    FromMatrix.ClearMatrix();
    TempMatrix.ClearMatrix();
    ////////////////////////////////////
#if 0
    ToMatrix.SetTranslation(-mBrushCenter.X,
                            -mBrushCenter.Y,
                            -mBrushCenter.Z);
    FromMatrix.SetTranslation(mBrushCenter.X,
                              mBrushCenter.Y,
                              mBrushCenter.Z);
#endif
//#if 0
	mBrushMatrix.CreateRotationMatrix(mRenderBrushRotation[0].X, mRenderBrushRotation[0].Y, mRenderBrushRotation[0].Z);
//#endif
#if 0
	TempMatrix.CreateRotationMatrix(mRenderBrushRotation[0].X, mRenderBrushRotation[0].Y, mRenderBrushRotation[0].Z);

    mBrushMatrix.MultiplyMatrix(ToMatrix);
    mBrushMatrix.MultiplyMatrix(TempMatrix);
    mBrushMatrix.MultiplyMatrix(FromMatrix);
#endif
    ///////////////////////////////////

	// ALL of the predone stuff
	mBrushMatrix.MultiplyMatrix(mRenderMatrix);

	// the CURRENT group operation that we are doing
	mBrushMatrix.MultiplyMatrix(mMatrix);
}

void CThredBrush::SetBrushCenter(ThredPoint center)
{
  mBrushCenter.X = center.X;
  mBrushCenter.Y = center.Y;
  mBrushCenter.Z = center.Z;
}

// set the position of the group within the world
void CThredBrush::SetGroupPosition(ThredPoint Position)
{
	ResetGroupItems();
	// set the position of the group in relation to it's parent.
	mBrushPosition[1] = Position;
	mRenderBrushPosition[1] = Position;

}

// It seems like the axes might be busted

// so this is reversed for now till I figure
// out what's going down.
void CThredBrush::MoveBrush(double X, double Y, double Z, CTHREDDoc* pDoc, int MoveType)
{
    //Point3F     brush_size;

    //brush_size.x = (mBoundingBox.mLargest.X - mBoundingBox.mSmallest.X);
    //brush_size.y = (mBoundingBox.mLargest.Y - mBoundingBox.mSmallest.Y);
    //brush_size.z = (mBoundingBox.mLargest.Z - mBoundingBox.mSmallest.Z);

	MoveType = 0;

	// change the stuff
	if(Globals.mAxis & AXIS_X ) {
		mRenderBrushPosition[MoveType].X -= X;
	}
	if(Globals.mAxis & AXIS_Y ) {
		mRenderBrushPosition[MoveType].Y -= Y;
	}
	if(Globals.mAxis & AXIS_Z ) {
		mRenderBrushPosition[MoveType].Z -= Z;
	}

	//double XDis = (mRenderBrushPosition[MoveType].X - mBrushPosition[MoveType].X);
	//double YDis = (mRenderBrushPosition[MoveType].Y - mBrushPosition[MoveType].Y);
	//double ZDis = (mRenderBrushPosition[MoveType].Z - mBrushPosition[MoveType].Z);
	double XDis = -X;
	double YDis = -Y;
	double ZDis = -Z;
	mMatrix.ClearMatrix();
	mMatrix.SetTranslation(XDis, YDis, ZDis);

	mTransformedPolysValid = FALSE;

    ///////////////////////////////////
    mTotalMatrix.MultiplyMatrix(mMatrix);
    ///////////////////////////////////

    //pDoc->UpdateBrushInfo(brush_size, mRenderBrushPosition[0]);
}

void CThredBrush::DoneMoveBrushCenter(double GridSize)
{
  mBrushCenter.X = RoundFloat((mBrushCenter.X / GridSize)) * GridSize;
  mBrushCenter.Y = RoundFloat((mBrushCenter.Y / GridSize)) * GridSize;
  mBrushCenter.Z = RoundFloat((mBrushCenter.Z / GridSize)) * GridSize;
}

void CThredBrush::MoveBrushCenter(double X, double Y, double Z, CTHREDDoc* pDoc, int MoveType)
{
    Point3F     brush_size;
    ThredPoint  oldcenter;

    oldcenter.X = mBrushCenter.X;
    oldcenter.Y = mBrushCenter.Y;
    oldcenter.Z = mBrushCenter.Z;

	// change the stuff
	if(Globals.mAxis & AXIS_X ) {
		mBrushCenter.X -= X;
	}
	if(Globals.mAxis & AXIS_Y ) {
		mBrushCenter.Y -= Y;
	}
	if(Globals.mAxis & AXIS_Z ) {
		mBrushCenter.Z -= Z;
	}

	//double XDis = (mBrushCenter.X - oldcenter.X);
	//double YDis = (mBrushCenter.Y - oldcenter.Y);
	//double ZDis = (mBrushCenter.Z - oldcenter.Z);
	//mMatrix.ClearMatrix();
	//mMatrix.SetTranslation(XDis, YDis, ZDis);

	mTransformedPolysValid = FALSE;

    //pDoc->UpdateBrushInfo(brush_size, mRenderBrushPosition[0]);
}


// we are done moving the brush
void CThredBrush::DoneMoveBrush(double GridSize, CTHREDDoc* pDoc, int MoveType)
{
	double            XDis, YDis, ZDis;
    ThredPoint        oldloc;
	CTransformMatrix  TempMatrix;

	MoveType = 0;

    oldloc = mRenderBrushPosition[MoveType];

	// round up the suckers
	if(Globals.mAxis & AXIS_X)
		mRenderBrushPosition[MoveType].X = RoundFloat((mRenderBrushPosition[MoveType].X/GridSize)) * GridSize;
	if(Globals.mAxis & AXIS_Y)
		mRenderBrushPosition[MoveType].Y = RoundFloat((mRenderBrushPosition[MoveType].Y/GridSize)) * GridSize;
	if(Globals.mAxis & AXIS_Z)
		mRenderBrushPosition[MoveType].Z = RoundFloat((mRenderBrushPosition[MoveType].Z/GridSize)) * GridSize;

	// if we are pushing the shift key we want to move only on greatest
	// on the axis we have the most movement on
	
	//if(GetAsyncKeyState(VK_SHIFT) & 0x8000)
    //{
	//	XDis = (mRenderBrushPosition[MoveType].X - mBrushPosition[MoveType].X);
	//	YDis = (mRenderBrushPosition[MoveType].Y - mBrushPosition[MoveType].Y);
	//	ZDis = (mRenderBrushPosition[MoveType].Z - mBrushPosition[MoveType].Z);

	//	if(XDis > YDis )
    //    {
	//		if(XDis > ZDis ) 
	//			mBrushPosition[MoveType].X = mRenderBrushPosition[MoveType].X;
	//		else
	//			mBrushPosition[MoveType].Z = mRenderBrushPosition[MoveType].Z;
	//	} else {
	//		if(YDis > ZDis )
	//			mBrushPosition[MoveType].Y = mRenderBrushPosition[MoveType].Y;
	//		else
	//			mBrushPosition[MoveType].Z = mRenderBrushPosition[MoveType].Z;
	//	}
		//{
	//		XDis = (mRenderBrushPosition[MoveType].X - mBrushPosition[MoveType].X);
	//		YDis = (mRenderBrushPosition[MoveType].Y - mBrushPosition[MoveType].Y);
	//		ZDis = (mRenderBrushPosition[MoveType].Z - mBrushPosition[MoveType].Z);
	//		TempMatrix.ClearMatrix();
	//		TempMatrix.SetTranslation(XDis, YDis, ZDis);
	//		mRenderMatrix.MultiplyMatrix(TempMatrix );
		//}
	//	mRenderBrushPosition[MoveType] = mBrushPosition[MoveType];
	//}
	//else 
	//{
			//XDis = (mRenderBrushPosition[MoveType].X - mBrushPosition[MoveType].X);
			//YDis = (mRenderBrushPosition[MoveType].Y - mBrushPosition[MoveType].Y);
			//ZDis = (mRenderBrushPosition[MoveType].Z - mBrushPosition[MoveType].Z);
			XDis = (mRenderBrushPosition[MoveType].X - oldloc.X);
			YDis = (mRenderBrushPosition[MoveType].Y - oldloc.Y);
			ZDis = (mRenderBrushPosition[MoveType].Z - oldloc.Z);
			TempMatrix.ClearMatrix();
			//TempMatrix.SetTranslation(XDis, YDis, ZDis);
			TempMatrix.SetTranslation((XDis), (YDis), (ZDis));
			//mRenderMatrix.MultiplyMatrix(TempMatrix );
    ////////////////////////////////////
    mTotalMatrix.MultiplyMatrix(TempMatrix);
    ////////////////////////////////////

		mBrushPosition[MoveType] = mRenderBrushPosition[MoveType];
	//}
	mMatrix.ClearMatrix();
	mTransformedPolysValid = FALSE;

    CalculateBrushCenter(); // DPW
    //pDoc->UpdateBrushInfo(brush_size, mRenderBrushPosition[0]);
}

ThredPoint CThredBrush::GetAveragePoint() 
{
	ThredPoint TranPoint;
	CPoint Point;

	SetupBrushMatrix();

	// get our location
	TranPoint.X = 0;
	TranPoint.Y = 0;
	TranPoint.Z = 0;

	// figure out where it is.
	mBrushMatrix.ApplyMatrix(TranPoint );

	return TranPoint;
}

void CThredBrush::DoneRotateBrush(double RotateSnap, int MoveType)
{
	double XDis, YDis, ZDis;
    ThredPoint  oldangle;

    oldangle = mRenderBrushRotation[MoveType];

	// round up the suckers
	//if(Globals.mAxis & AXIS_X )
		mRenderBrushRotation[MoveType].X = RoundFloat((mRenderBrushRotation[MoveType].X/RotateSnap) ) * RotateSnap;
	//if(Globals.mAxis & AXIS_Y )
		mRenderBrushRotation[MoveType].Y = RoundFloat((mRenderBrushRotation[MoveType].Y/RotateSnap) ) * RotateSnap;
	//if(Globals.mAxis & AXIS_Z )
		mRenderBrushRotation[MoveType].Z = RoundFloat((mRenderBrushRotation[MoveType].Z/RotateSnap) ) * RotateSnap;

	// if we are pushing the shift key we want to move only on greatest
	// on the axis we have the most movement on

	if(MoveType )
	{
		//XDis = (mRenderBrushRotation[MoveType].X - mBrushRotation[MoveType].X);
		//YDis = (mRenderBrushRotation[MoveType].Y - mBrushRotation[MoveType].Y);
		//ZDis = (mRenderBrushRotation[MoveType].Z - mBrushRotation[MoveType].Z);
		XDis = (mRenderBrushRotation[MoveType].X - oldangle.X);
		YDis = (mRenderBrushRotation[MoveType].Y - oldangle.Y);
		ZDis = (mRenderBrushRotation[MoveType].Z - oldangle.Z);

		// Create all the matrices we need
		CTransformMatrix TempMatrix;
		CTransformMatrix ToMatrix, FromMatrix;

		ToMatrix.ClearMatrix();
		FromMatrix.ClearMatrix();
		TempMatrix.ClearMatrix();

		ToMatrix.SetTranslation(-mRenderBrushPosition[MoveType].X, 
			-mRenderBrushPosition[MoveType].Y, -mRenderBrushPosition[MoveType].Z);

		FromMatrix.SetTranslation(mRenderBrushPosition[MoveType].X, 
			mRenderBrushPosition[MoveType].Y, mRenderBrushPosition[MoveType].Z);

		TempMatrix.CreateRotationMatrix(XDis, YDis, ZDis );

		// go through and concatenate the matrices we need
		mRenderMatrix.MultiplyMatrix(ToMatrix );
		mRenderMatrix.MultiplyMatrix(TempMatrix );
		mRenderMatrix.MultiplyMatrix(FromMatrix );

        ////////////////////////////////////////
        mTotalMatrix.MultiplyMatrix(ToMatrix);
        mTotalMatrix.MultiplyMatrix(TempMatrix);
        mTotalMatrix.MultiplyMatrix(FromMatrix);
        ////////////////////////////////////////

		mMatrix.ClearMatrix();
		//ResetGroupItems();
	}
	else
	{
		if(GetAsyncKeyState(VK_SHIFT) & 0x8000) 
		{
			XDis = fabs(mRenderBrushRotation[MoveType].X - mBrushRotation[MoveType].X);
			YDis = fabs(mRenderBrushRotation[MoveType].Y - mBrushRotation[MoveType].Y);
			ZDis = fabs(mRenderBrushRotation[MoveType].Z - mBrushRotation[MoveType].Z);

			if(XDis > YDis ) {
				if(XDis > ZDis ) 
					mBrushRotation[MoveType].X = mRenderBrushRotation[MoveType].X;
				else
					mBrushRotation[MoveType].Z = mRenderBrushRotation[MoveType].Z;
			}
			else {
				if(YDis > ZDis )
					mBrushRotation[MoveType].Y = mRenderBrushRotation[MoveType].Y;
				else
					mBrushRotation[MoveType].Z = mRenderBrushRotation[MoveType].Z;
			}
			mRenderBrushRotation[MoveType] = mBrushRotation[MoveType];
		}
		else
			mBrushRotation[MoveType] = mRenderBrushRotation[MoveType];
	}

    CalculateBrushCenter(); // DPW

	mTransformedPolysValid = FALSE;
}

void CThredBrush::RotateBrushAroundPoint(double Yaw, double Pitch, double Roll, ThredPoint point)
{
	//if(Globals.mAxis & AXIS_Y)
		mRenderBrushRotation[1].X += Yaw;
	//if(Globals.mAxis & AXIS_X)
		mRenderBrushRotation[1].Y += Pitch;
	//if(Globals.mAxis & AXIS_Z)
		mRenderBrushRotation[1].Z += Roll;

	//double XDis = (mRenderBrushRotation[1].X - mBrushRotation[1].X);
	//double YDis = (mRenderBrushRotation[1].Y - mBrushRotation[1].Y);
	//double ZDis = (mRenderBrushRotation[1].Z - mBrushRotation[1].Z);
	double XDis = Yaw;
	double YDis = Pitch;
	double ZDis = Roll;

    mRenderBrushPosition[1].X = point.X;
    mRenderBrushPosition[1].Y = point.Y;
    mRenderBrushPosition[1].Z = point.Z;

	// Create all the matrices we need
	CTransformMatrix TempMatrix;
	CTransformMatrix ToMatrix, FromMatrix;

	ToMatrix.ClearMatrix();
	FromMatrix.ClearMatrix();
	TempMatrix.ClearMatrix();

	ToMatrix.SetTranslation(-point.X, -point.Y, -point.Z);

	FromMatrix.SetTranslation(point.X, point.Y, point.Z);

	TempMatrix.CreateRotationMatrix(XDis, YDis, ZDis);

	// go through and concatenate the matrices
	mMatrix.ClearMatrix();
	mMatrix.MultiplyMatrix(ToMatrix);
	mMatrix.MultiplyMatrix(TempMatrix);
	mMatrix.MultiplyMatrix(FromMatrix);

    ///////////////////////////////////
    mTotalMatrix.MultiplyMatrix(mMatrix);
    ///////////////////////////////////

	mTransformedPolysValid = FALSE;
}

void CThredBrush::RotateBrush(double Yaw, double Pitch, double Roll, int MoveType)
{
	//if(Globals.mAxis & AXIS_Y)
		mRenderBrushRotation[MoveType].X += Yaw;
	//if(Globals.mAxis & AXIS_X)
		mRenderBrushRotation[MoveType].Y += Pitch;
	//if(Globals.mAxis & AXIS_Z)
		mRenderBrushRotation[MoveType].Z += Roll;

	// if are are moving in a group
	if(MoveType)
	{
		double XDis = (mRenderBrushRotation[MoveType].X - mBrushRotation[MoveType].X);
		double YDis = (mRenderBrushRotation[MoveType].Y - mBrushRotation[MoveType].Y);
		double ZDis = (mRenderBrushRotation[MoveType].Z - mBrushRotation[MoveType].Z);

		// Create all the matrices we need
		CTransformMatrix TempMatrix;
		CTransformMatrix ToMatrix, FromMatrix;

		ToMatrix.ClearMatrix();
		FromMatrix.ClearMatrix();
		TempMatrix.ClearMatrix();

		ToMatrix.SetTranslation(-mRenderBrushPosition[MoveType].X, 
			-mRenderBrushPosition[MoveType].Y, -mRenderBrushPosition[MoveType].Z);

		FromMatrix.SetTranslation(mRenderBrushPosition[MoveType].X, 
			mRenderBrushPosition[MoveType].Y, mRenderBrushPosition[MoveType].Z);

		TempMatrix.CreateRotationMatrix(XDis, YDis, ZDis);

		// go through and concatenate the matrices
		mMatrix.ClearMatrix();
		mMatrix.MultiplyMatrix(ToMatrix);
		mMatrix.MultiplyMatrix(TempMatrix);
		mMatrix.MultiplyMatrix(FromMatrix);
	}

	mTransformedPolysValid = FALSE;
}

#define BRUSH_SCALE_MUL	(double)0.005
#define BRUSH_SCALE_LOW_LIMIT	(double)0.1

void CThredBrush::ScaleBrush(double X, double Y, double Z, int MoveType)
{
	// Do the Change
	if(Globals.mAxis & AXIS_X)
		mBrushScale[MoveType].X -= X * BRUSH_SCALE_MUL;
	if(Globals.mAxis & AXIS_Y)
		mBrushScale[MoveType].Y -= Y * BRUSH_SCALE_MUL;
	if(Globals.mAxis & AXIS_Z)
		mBrushScale[MoveType].Z -= Z * BRUSH_SCALE_MUL;

	// Check the bounds
	if(mBrushScale[MoveType].X < BRUSH_SCALE_LOW_LIMIT )
		mBrushScale[MoveType].X = BRUSH_SCALE_LOW_LIMIT;
	if(mBrushScale[MoveType].Y < BRUSH_SCALE_LOW_LIMIT )
		mBrushScale[MoveType].Y = BRUSH_SCALE_LOW_LIMIT;
	if(mBrushScale[MoveType].Z < BRUSH_SCALE_LOW_LIMIT )
		mBrushScale[MoveType].Z = BRUSH_SCALE_LOW_LIMIT;

	mTransformedPolysValid = FALSE;

	// if are are moving in a group
	if(MoveType )
	{
		// Create all the matricise we need
		CTransformMatrix TempMatrix;
		CTransformMatrix ToMatrix, FromMatrix;

		ToMatrix.ClearMatrix();
		FromMatrix.ClearMatrix();
		TempMatrix.ClearMatrix();

		ToMatrix.SetTranslation(-mRenderBrushPosition[MoveType].X, 
			-mRenderBrushPosition[MoveType].Y, -mRenderBrushPosition[MoveType].Z);

		FromMatrix.SetTranslation(mRenderBrushPosition[MoveType].X, 
			mRenderBrushPosition[MoveType].Y, mRenderBrushPosition[MoveType].Z);

		// set the scale of the matrix
		TempMatrix.SetScale(mBrushScale[MoveType].X, mBrushScale[MoveType].Y, mBrushScale[MoveType].Z);

		// go through and concatenate the matrices we need
		mMatrix.ClearMatrix();
		mMatrix.MultiplyMatrix(ToMatrix );
		mMatrix.MultiplyMatrix(TempMatrix );
		mMatrix.MultiplyMatrix(FromMatrix );
	}

}

void CThredBrush::DoneScaleBrush(int MoveType)
{
	mTransformedPolysValid = FALSE;
	// if are are moving in a group
	if(MoveType )
	{
		// Create all the matricise we need
		CTransformMatrix TempMatrix;
		CTransformMatrix ToMatrix, FromMatrix;

		ToMatrix.ClearMatrix();
		FromMatrix.ClearMatrix();
		TempMatrix.ClearMatrix();

		ToMatrix.SetTranslation(-mRenderBrushPosition[MoveType].X, 
			-mRenderBrushPosition[MoveType].Y, -mRenderBrushPosition[MoveType].Z);

		FromMatrix.SetTranslation(mRenderBrushPosition[MoveType].X, 
			mRenderBrushPosition[MoveType].Y, mRenderBrushPosition[MoveType].Z);

		// set the scale of the matrix
		TempMatrix.SetScale(mBrushScale[MoveType].X, mBrushScale[MoveType].Y, mBrushScale[MoveType].Z);

		// go through and concatenate the matrices we need
		mMatrix.ClearMatrix();
		mRenderMatrix.MultiplyMatrix(ToMatrix );
		mRenderMatrix.MultiplyMatrix(TempMatrix );
		mRenderMatrix.MultiplyMatrix(FromMatrix );
		ResetGroupItems();
	}
}

#define BRUSH_SHEAR_MUL	(double)0.005
void CThredBrush::ShearBrush(double X, double Y, double Z, int MoveType)
{
	// Do the Change
	if(Globals.mAxis & AXIS_X)
		mBrushShear[MoveType].X += X * BRUSH_SHEAR_MUL;
	if(Globals.mAxis & AXIS_Y)
		mBrushShear[MoveType].Y += Y * BRUSH_SHEAR_MUL;
	if(Globals.mAxis & AXIS_Z)
		mBrushShear[MoveType].Z += Z * BRUSH_SHEAR_MUL;
	mTransformedPolysValid = FALSE;

	// if are are moving in a group
	if(MoveType)
	{
		// Create all the matricise we need
		CTransformMatrix TempMatrix;
		CTransformMatrix ToMatrix, FromMatrix;

		ToMatrix.ClearMatrix();
		FromMatrix.ClearMatrix();
		TempMatrix.ClearMatrix();

		ToMatrix.SetTranslation(-mRenderBrushPosition[MoveType].X, 
			-mRenderBrushPosition[MoveType].Y, -mRenderBrushPosition[MoveType].Z);

		FromMatrix.SetTranslation(mRenderBrushPosition[MoveType].X, 
			mRenderBrushPosition[MoveType].Y, mRenderBrushPosition[MoveType].Z);

		// set the scale of the matrix
		TempMatrix.SetShear(mBrushShear[MoveType].X, mBrushShear[MoveType].Y, mBrushShear[MoveType].Z);

		// go through and concatenate the matrices we need
		mMatrix.ClearMatrix();
		mMatrix.MultiplyMatrix(ToMatrix);
		mMatrix.MultiplyMatrix(TempMatrix);
		mMatrix.MultiplyMatrix(FromMatrix);
	}
}

void CThredBrush::DoneShearBrush(int MoveType )
{
	mTransformedPolysValid = FALSE;

	// if are are moving in a group
	if(MoveType )
	{
		// Create all the matricise we need
		CTransformMatrix TempMatrix;
		CTransformMatrix ToMatrix, FromMatrix;

		ToMatrix.ClearMatrix();
		FromMatrix.ClearMatrix();
		TempMatrix.ClearMatrix();

		ToMatrix.SetTranslation(-mRenderBrushPosition[MoveType].X, 
			-mRenderBrushPosition[MoveType].Y, -mRenderBrushPosition[MoveType].Z);

		FromMatrix.SetTranslation(mRenderBrushPosition[MoveType].X, 
			mRenderBrushPosition[MoveType].Y, mRenderBrushPosition[MoveType].Z);

		// set the scale of the matrix
		TempMatrix.SetShear(mBrushShear[MoveType].X, mBrushShear[MoveType].Y, mBrushShear[MoveType].Z);

		// go through and concatenate the matrices we need
		mMatrix.ClearMatrix();
		mRenderMatrix.MultiplyMatrix(ToMatrix );
		mRenderMatrix.MultiplyMatrix(TempMatrix );
		mRenderMatrix.MultiplyMatrix(FromMatrix );
		ResetGroupItems();
	}
}

#define CBRUSHTYPE            "CThredBrush"
#define CBRUSHTRANSACTIONTYPE "TranT"
#define CBRUSHDESCRIPTION     "BType"
#define CBRUSHNAME            "Name"
#define CBRUSHGROUP	          "BGroup"
#define CBRUSHID              "BBid"
#define CBRUSHMATRIX          "BMatrix"
#define CTOTALMATRIX          "TMatrix"
#define CBRUSHSELECTED        "BFlags"
#define CBRUSHENTITY          "BEnt"
#define CBRUSHPOSITION        "Pos"
#define CBRUSHCENTER          "Center"
#define CBRUSHROTATION        "Rot"
#define CBRUSHSCALE           "Scl"
#define CBRUSHSHEAR           "Shr"
#define CFACETEXTUREFLIPH     "TFlipH"
#define CFACETEXTUREFLIPV     "TFlipV"
#define CFACETEXTUREID        "TxtID"
#define CFACETEXTURESHIFTX    "TShftx"
#define CFACETEXTURESHIFTY    "TShfty"
#define CFACETEXTUREROTATE    "TRotate"
#define CFACETEXTURESCALE     "TScale"
#define CBRUSHNUMBEROFPOLYS   "NumfBPolys"
#define CBRUSHXSIZE           "XS"
#define CBRUSHXSIZEBOT        "XSBot"
#define CBRUSHXSIZETOP        "XSTop"
#define CBRUSHYSIZE           "YS"
#define CBRUSHZSIZE           "ZS"
#define CBRUSHZSIZEBOT        "ZSBot"
#define CBRUSHZSIZETOP        "ZSTop"
#define CBRUSHTHICKNESS       "Thknes"
#define CBRUSHSOLID           "Sol"
#define CBRUSHXOFFSETBOT      "XOBot"
#define CBRUSHXOFFSETTOP      "XOTop"
#define CBRUSHZOFFSETBOT      "ZOBot"
#define CBRUSHZOFFSETTOP      "ZOTop"
#define CBRUSHVSTRIPES        "VStripes"
#define CBRUSHHBANDS          "HBands"
#define CBRUSHVBANDS          "VBands"
#define CBRUSHHEIGHT          "Height"
#define CBRUSHWIDTH           "Width"
#define CBRUSHLENGTH          "Length"
#define CBRUSHNUMSTAIRS       "NStairs"
#define CBRUSHMAKERAMP        "Ramp"
#define CVOLSTATE             "VState"
#define CSHOWVOL              "ShowVol"
#define CBRUSHHALFSPHERE      "HalfSphere"
#define CBRUSHRING            "Ring"
#define ENDBRUSHTYPE          "End CThredBrush"
void CThredBrush::Serialize(CThredParser& Parser, int maxTextureID )
{
    char  tmpbuf[32];
    CString tmpbuf1;
    int texture_id; // DPW
    int texture_shiftx, texture_shifty; // DPW
    double texture_rotate, texture_scale;

	//CThredParser Parser(&ar, ENDBRUSHTYPE);
	int Current, tmpint;
	int NumberOfPolygons;
	Parser.SetEnd(ENDBRUSHTYPE);

	// check if we are storing or loading
	if(Parser.IsStoring)
    {
		// we want to write our our transaction type
		Parser.WriteString(CBRUSHTYPE);
		Parser.WriteInt(CBRUSHTRANSACTIONTYPE, mTransactionType );

        // Write out which type of brush we have (tri, box, etc.)
        Parser.WriteInt(CBRUSHDESCRIPTION, (int)BrushType);

        // Depending on BrushType, write out the correct information
        switch (BrushType)
        {
        case BOX_BRUSH:
            sprintf(tmpbuf, "%f", BrushSpecifics.box.m_XSizeBot);
            Parser.WriteString (CBRUSHXSIZEBOT, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.box.m_XSizeTop);
            Parser.WriteString (CBRUSHXSIZETOP, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.box.m_YSize);
            Parser.WriteString (CBRUSHYSIZE, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.box.m_ZSizeBot);
            Parser.WriteString (CBRUSHZSIZEBOT, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.box.m_ZSizeTop);
            Parser.WriteString (CBRUSHZSIZETOP, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.box.m_Thickness);
            Parser.WriteString (CBRUSHTHICKNESS, tmpbuf);
            Parser.WriteInt(CBRUSHSOLID, BrushSpecifics.box.m_Solid);
         break;
        case TRI_BRUSH:
            sprintf(tmpbuf, "%f", BrushSpecifics.triangle.m_XSizeBot);
            Parser.WriteString (CBRUSHXSIZEBOT, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.triangle.m_YSize);
            Parser.WriteString (CBRUSHYSIZE, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.triangle.m_ZSizeBot);
            Parser.WriteString (CBRUSHZSIZEBOT, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.triangle.m_ZSizeTop);
            Parser.WriteString (CBRUSHZSIZETOP, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.triangle.m_Thickness);
            Parser.WriteString (CBRUSHTHICKNESS, tmpbuf);
            Parser.WriteInt(CBRUSHSOLID, BrushSpecifics.triangle.m_Solid);
          break;
        case CYL_BRUSH:
            sprintf(tmpbuf, "%f", BrushSpecifics.cylinder.m_BotXOffset);
            Parser.WriteString (CBRUSHXOFFSETBOT, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.cylinder.m_BotXSize);
            Parser.WriteString (CBRUSHXSIZEBOT, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.cylinder.m_TopXOffset);
            Parser.WriteString (CBRUSHXOFFSETTOP, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.cylinder.m_TopXSize);
            Parser.WriteString (CBRUSHXSIZETOP, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.cylinder.m_YSize);
            Parser.WriteString (CBRUSHYSIZE, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.cylinder.m_BotZOffset);
            Parser.WriteString (CBRUSHZOFFSETBOT, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.cylinder.m_BotZSize);
            Parser.WriteString (CBRUSHZSIZEBOT, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.cylinder.m_TopZOffset);
            Parser.WriteString (CBRUSHZOFFSETTOP, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.cylinder.m_TopZSize);
            Parser.WriteString (CBRUSHZSIZETOP, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.cylinder.m_VerticalStripes);
            Parser.WriteString (CBRUSHVSTRIPES, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.cylinder.m_Thickness);
            Parser.WriteString (CBRUSHTHICKNESS, tmpbuf);
            Parser.WriteInt(CBRUSHSOLID, BrushSpecifics.cylinder.m_Solid);
            Parser.WriteInt(CBRUSHRING, BrushSpecifics.cylinder.m_Ring);
          break;
        case SPHERE_BRUSH:
            sprintf(tmpbuf, "%f", BrushSpecifics.sphere.m_XSize);
            Parser.WriteString (CBRUSHXSIZE, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.sphere.m_YSize);
            Parser.WriteString (CBRUSHYSIZE, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.sphere.m_ZSize);
            Parser.WriteString (CBRUSHZSIZE, tmpbuf);
            Parser.WriteInt(CBRUSHHBANDS, BrushSpecifics.sphere.m_HorizontalBands);
            Parser.WriteInt(CBRUSHVBANDS, BrushSpecifics.sphere.m_VerticalBands);
            sprintf(tmpbuf, "%f", BrushSpecifics.sphere.m_Thickness);
            Parser.WriteString (CBRUSHTHICKNESS, tmpbuf);
            Parser.WriteInt(CBRUSHSOLID, BrushSpecifics.sphere.m_Solid);
            Parser.WriteInt(CBRUSHHALFSPHERE, BrushSpecifics.sphere.m_HalfSphere);
          break;
        case STAIR_BRUSH:
            sprintf(tmpbuf, "%f", BrushSpecifics.staircase.m_Height);
            Parser.WriteString (CBRUSHHEIGHT, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.staircase.m_Width);
            Parser.WriteString (CBRUSHWIDTH, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.staircase.m_Length);
            Parser.WriteString (CBRUSHLENGTH, tmpbuf);
            sprintf(tmpbuf, "%f", BrushSpecifics.staircase.m_NumberOfStairs);
            Parser.WriteString (CBRUSHNUMSTAIRS, tmpbuf);
            Parser.WriteInt(CBRUSHMAKERAMP, (int)BrushSpecifics.staircase.m_MakeRamp);
          break;
        }

		// write out the name
		Parser.WriteString(CBRUSHNAME, mName);

		// some info.
		Parser.WriteInt(CBRUSHGROUP, mBrushGroup);
		Parser.WriteInt(CBRUSHSELECTED, mFlags);
		Parser.WriteInt(CBRUSHENTITY, mEntityId);
		Parser.WriteInt(CBRUSHID, mBrushId);
        Parser.WriteLong(CVOLSTATE, volumestate);
        Parser.WriteInt(CSHOWVOL, m_ShowVolPolys);

		// write out our point
		Parser.WritePoint(CBRUSHPOSITION, mBrushPosition[INDEX_BRUSH]);
		Parser.WritePoint(CBRUSHCENTER, mBrushCenter);
		Parser.WritePoint(CBRUSHROTATION, mBrushRotation[INDEX_BRUSH]);
		Parser.WritePoint(CBRUSHSCALE, mBrushScale[INDEX_BRUSH]);
		Parser.WritePoint(CBRUSHSHEAR, mBrushShear[INDEX_BRUSH]);

		// save our polygons
		Parser.WriteInt(CBRUSHNUMBEROFPOLYS, mNumberOfPolygons);

		// save 'em
		for(Current = 0; Current < mNumberOfPolygons; Current++)
        {
			Parser.WritePolygon(mPolygons[Current]);
		}

        // DPW - save out the face texture names
        for (Current = 0; Current < mNumberOfPolygons; Current++)
        {
            Parser.WriteInt (CFACETEXTUREID, mTextureID[Current].TextureID);
            Parser.WriteInt (CFACETEXTURESHIFTX, mTextureID[Current].TextureShift.x);
            Parser.WriteInt (CFACETEXTURESHIFTY, mTextureID[Current].TextureShift.y);
            Parser.WriteInt (CFACETEXTUREFLIPH, mTextureID[Current].TextureFlipH);
            Parser.WriteInt (CFACETEXTUREFLIPV, mTextureID[Current].TextureFlipV);

            sprintf(tmpbuf, "%f", mTextureID[Current].TextureRotate);
            Parser.WriteString (CFACETEXTUREROTATE, tmpbuf);
            sprintf(tmpbuf, "%f", mTextureID[Current].TextureScale);
            Parser.WriteString (CFACETEXTURESCALE, tmpbuf);
        }

		// save the matrix we need
		//Parser.WriteMatrix(CBRUSHMATRIX, mRenderMatrix);
		Parser.WriteMatrix(CTOTALMATRIX, mTotalMatrix);

		Parser.WriteString(ENDBRUSHTYPE);
	} else // reading
   {
		// Getting a brush
		if(!Parser.GetKey(CBRUSHTYPE))
        {
			AfxThrowArchiveException(CArchiveException::generic, NULL);
			return;
		}

		// get our transaction type
		Parser.GetKeyInt(CBRUSHTRANSACTIONTYPE, mTransactionType );

        // get which type of brush we have (tri, box, etc.)
        if (!Parser.GetKeyInt(CBRUSHDESCRIPTION, tmpint))
        {
          tmpint = 0;
        }

        BrushType = (enum TypesOBrush)tmpint;

        // Depending on BrushType, get the correct information
        switch (BrushType)
        {
        case BOX_BRUSH:
       		if (!Parser.GetKeyString(CBRUSHXSIZEBOT, tmpbuf1))
              BrushSpecifics.box.m_XSizeBot = 128.0;
            else
              BrushSpecifics.box.m_XSizeBot = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHXSIZETOP, tmpbuf1))
              BrushSpecifics.box.m_XSizeTop = 128.0;
            else
              BrushSpecifics.box.m_XSizeTop = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHYSIZE, tmpbuf1))
              BrushSpecifics.box.m_YSize = 128.0;
            else
              BrushSpecifics.box.m_YSize = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHZSIZEBOT, tmpbuf1))
              BrushSpecifics.box.m_ZSizeBot = 128.0;
            else
              BrushSpecifics.box.m_ZSizeBot = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHZSIZETOP, tmpbuf1))
              BrushSpecifics.box.m_ZSizeTop = 128.0;
            else
              BrushSpecifics.box.m_ZSizeTop = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHTHICKNESS, tmpbuf1))
              BrushSpecifics.box.m_Thickness = 16.0;
            else
              BrushSpecifics.box.m_Thickness = atof((LPCTSTR)tmpbuf1);

            if (!Parser.GetKeyInt(CBRUSHSOLID, BrushSpecifics.box.m_Solid))
              BrushSpecifics.box.m_Solid = 0;
#if 0
          {
          CCreateBoxDialog  boxdialog;

          boxdialog.m_Solid = BrushSpecifics.box.m_Solid;
          boxdialog.m_Thickness = BrushSpecifics.box.m_Thickness;
          boxdialog.m_XSizeBot = BrushSpecifics.box.m_XSizeBot;
          boxdialog.m_XSizeTop = BrushSpecifics.box.m_XSizeTop;
          boxdialog.m_YSize = BrushSpecifics.box.m_YSize;
          boxdialog.m_ZSizeBot = BrushSpecifics.box.m_ZSizeBot;
          boxdialog.m_ZSizeTop = BrushSpecifics.box.m_ZSizeTop;

          boxdialog.CreateBox(this);
          }
#endif
         break;
        case TRI_BRUSH:
       		if (!Parser.GetKeyString(CBRUSHXSIZEBOT, tmpbuf1))
              BrushSpecifics.triangle.m_XSizeBot = 128.0;
            else
              BrushSpecifics.triangle.m_XSizeBot = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHYSIZE, tmpbuf1))
              BrushSpecifics.triangle.m_YSize = 128.0;
            else
              BrushSpecifics.triangle.m_YSize = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHZSIZEBOT, tmpbuf1))
              BrushSpecifics.triangle.m_ZSizeBot = 128.0;
            else
              BrushSpecifics.triangle.m_ZSizeBot = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHZSIZETOP, tmpbuf1))
              BrushSpecifics.triangle.m_ZSizeTop = 128.0;
            else
              BrushSpecifics.triangle.m_ZSizeTop = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHTHICKNESS, tmpbuf1))
              BrushSpecifics.triangle.m_Thickness = 16.0;
            else
              BrushSpecifics.triangle.m_Thickness = atof((LPCTSTR)tmpbuf1);

            if (!Parser.GetKeyInt(CBRUSHSOLID, BrushSpecifics.triangle.m_Solid))
              BrushSpecifics.triangle.m_Solid = 0;
              
            // fix for problem with this not being set on operator=
            if( BrushSpecifics.triangle.m_Solid & 0xfffffffe )
               BrushSpecifics.triangle.m_Solid = 0;
#if 0
          {
          CCreateTriDialog  tridialog;

          tridialog.m_Solid = BrushSpecifics.triangle.m_Solid;
          tridialog.m_Thickness = BrushSpecifics.triangle.m_Thickness;
          tridialog.m_XSizeBot = BrushSpecifics.triangle.m_XSizeBot;
          tridialog.m_YSize = BrushSpecifics.triangle.m_YSize;
          tridialog.m_ZSizeBot = BrushSpecifics.triangle.m_ZSizeBot;
          tridialog.m_ZSizeTop = BrushSpecifics.triangle.m_ZSizeTop;

          tridialog.CreateTriangle(this);
          }
#endif
          break;
        case CYL_BRUSH:
       		if (!Parser.GetKeyString(CBRUSHXOFFSETBOT, tmpbuf1))
              BrushSpecifics.cylinder.m_BotXOffset = 0.0;
            else
              BrushSpecifics.cylinder.m_BotXOffset = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHXSIZEBOT, tmpbuf1))
              BrushSpecifics.cylinder.m_BotXSize = 128.0;
            else
              BrushSpecifics.cylinder.m_BotXSize = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHXOFFSETTOP, tmpbuf1))
              BrushSpecifics.cylinder.m_TopXOffset = 0.0;
            else
              BrushSpecifics.cylinder.m_TopXOffset = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHXSIZETOP, tmpbuf1))
              BrushSpecifics.cylinder.m_TopXSize = 128.0;
            else
              BrushSpecifics.cylinder.m_TopXSize = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHYSIZE, tmpbuf1))
              BrushSpecifics.cylinder.m_YSize = 512.0;
            else
              BrushSpecifics.cylinder.m_YSize = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHZOFFSETBOT, tmpbuf1))
              BrushSpecifics.cylinder.m_BotZOffset = 0.0;
            else
              BrushSpecifics.cylinder.m_BotZOffset = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHZSIZEBOT, tmpbuf1))
              BrushSpecifics.cylinder.m_BotZSize = 128.0;
            else
              BrushSpecifics.cylinder.m_BotZSize = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHZOFFSETTOP, tmpbuf1))
              BrushSpecifics.cylinder.m_TopZOffset = 0.0;
            else
              BrushSpecifics.cylinder.m_TopZOffset = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHZSIZETOP, tmpbuf1))
              BrushSpecifics.cylinder.m_TopZSize = 128.0;
            else
              BrushSpecifics.cylinder.m_TopZSize = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHVSTRIPES, tmpbuf1))
              BrushSpecifics.cylinder.m_VerticalStripes = 6.0;
            else
              BrushSpecifics.cylinder.m_VerticalStripes = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHTHICKNESS, tmpbuf1))
              BrushSpecifics.cylinder.m_Thickness = 16.0;
            else
              BrushSpecifics.cylinder.m_Thickness = atof((LPCTSTR)tmpbuf1);

            if (!Parser.GetKeyInt(CBRUSHSOLID, BrushSpecifics.cylinder.m_Solid))
              BrushSpecifics.cylinder.m_Solid = 0;

            if (!Parser.GetKeyInt(CBRUSHRING, BrushSpecifics.cylinder.m_Ring))
              BrushSpecifics.cylinder.m_Ring = 0;
#if 0
          {
          CCreateCylDialog  cyldialog;

          cyldialog.m_BotXOffset = BrushSpecifics.cylinder.m_BotXOffset;
          cyldialog.m_BotXSize = BrushSpecifics.cylinder.m_BotXSize;
          cyldialog.m_BotZOffset = BrushSpecifics.cylinder.m_BotZOffset;
          cyldialog.m_BotZSize = BrushSpecifics.cylinder.m_BotZSize;
          cyldialog.m_Ring = BrushSpecifics.cylinder.m_Ring;
          cyldialog.m_Solid = BrushSpecifics.cylinder.m_Solid;
          cyldialog.m_Thickness = BrushSpecifics.cylinder.m_Thickness;
          cyldialog.m_TopXOffset = BrushSpecifics.cylinder.m_TopXOffset;
          cyldialog.m_TopXSize = BrushSpecifics.cylinder.m_TopXSize;
          cyldialog.m_TopZOffset = BrushSpecifics.cylinder.m_TopZOffset;
          cyldialog.m_TopZSize = BrushSpecifics.cylinder.m_TopZSize;
          cyldialog.m_VerticalStripes = BrushSpecifics.cylinder.m_VerticalStripes;
          cyldialog.m_YSize = BrushSpecifics.cylinder.m_YSize;

          cyldialog.CreateCyl(this);
          }
#endif
          break;
        case SPHERE_BRUSH:
       		if (!Parser.GetKeyString(CBRUSHXSIZE, tmpbuf1))
              BrushSpecifics.sphere.m_XSize = 256.0;
            else
              BrushSpecifics.sphere.m_XSize = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHYSIZE, tmpbuf1))
              BrushSpecifics.sphere.m_YSize = 256.0;
            else
              BrushSpecifics.sphere.m_YSize = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHZSIZE, tmpbuf1))
              BrushSpecifics.sphere.m_ZSize = 256.0;
            else
              BrushSpecifics.sphere.m_ZSize = atof((LPCTSTR)tmpbuf1);

            if (!Parser.GetKeyInt(CBRUSHHBANDS, BrushSpecifics.sphere.m_HorizontalBands))
              BrushSpecifics.sphere.m_HorizontalBands = 4;

            if (!Parser.GetKeyInt(CBRUSHVBANDS, BrushSpecifics.sphere.m_VerticalBands))
              BrushSpecifics.sphere.m_VerticalBands = 8;

            if (!Parser.GetKeyString(CBRUSHTHICKNESS, tmpbuf1))
              BrushSpecifics.sphere.m_Thickness = 16.0;
            else
              BrushSpecifics.sphere.m_Thickness = atof((LPCTSTR)tmpbuf1);

            if (!Parser.GetKeyInt(CBRUSHSOLID, BrushSpecifics.sphere.m_Solid))
              BrushSpecifics.sphere.m_Solid = 0;

            if (!Parser.GetKeyInt(CBRUSHHALFSPHERE, BrushSpecifics.sphere.m_HalfSphere))
              BrushSpecifics.sphere.m_HalfSphere = 0;
#if 0
          {
          CCreateSpheroidDialog spheredialog;

          spheredialog.m_HalfSphere = BrushSpecifics.sphere.m_HalfSphere;
          spheredialog.m_HorizontalBands = BrushSpecifics.sphere.m_HorizontalBands;
          spheredialog.m_Solid = BrushSpecifics.sphere.m_Solid;
          spheredialog.m_Thickness = BrushSpecifics.sphere.m_Thickness;
          spheredialog.m_VerticalBands = BrushSpecifics.sphere.m_VerticalBands;
          spheredialog.m_XSize = BrushSpecifics.sphere.m_XSize;
          spheredialog.m_YSize = BrushSpecifics.sphere.m_YSize;
          spheredialog.m_ZSize = BrushSpecifics.sphere.m_ZSize;

          spheredialog.CreateSpheroid(this);
          }
#endif
          break;
        case STAIR_BRUSH:
       		if (!Parser.GetKeyString(CBRUSHHEIGHT, tmpbuf1))
              BrushSpecifics.staircase.m_Height = 128.0;
            else
              BrushSpecifics.staircase.m_Height = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHWIDTH, tmpbuf1))
              BrushSpecifics.staircase.m_Width = 64.0;
            else
              BrushSpecifics.staircase.m_Width = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHLENGTH, tmpbuf1))
              BrushSpecifics.staircase.m_Length = 128.0;
            else
              BrushSpecifics.staircase.m_Length = atof((LPCTSTR)tmpbuf1);

       		if (!Parser.GetKeyString(CBRUSHNUMSTAIRS, tmpbuf1))
              BrushSpecifics.staircase.m_NumberOfStairs = 8.0;
            else
              BrushSpecifics.staircase.m_NumberOfStairs = atof((LPCTSTR)tmpbuf1);

            if (!Parser.GetKeyInt(CBRUSHMAKERAMP, tmpint))
            {
              tmpint = 0;
            }

            BrushSpecifics.staircase.m_MakeRamp = (BOOL)tmpint;
#if 0
          {
          CCreateStaircaseDialog  stairdialog;

          stairdialog.m_Height = BrushSpecifics.staircase.m_Height;
          stairdialog.m_Length = BrushSpecifics.staircase.m_Length;
          stairdialog.m_MakeRamp = BrushSpecifics.staircase.m_MakeRamp;
          stairdialog.m_NumberOfStairs = BrushSpecifics.staircase.m_NumberOfStairs;
          stairdialog.m_Width = BrushSpecifics.staircase.m_Width;

          stairdialog.MakeBrush(this);
          }
#endif
          break;
        }

		// read in the name
		Parser.GetKeyString(CBRUSHNAME, mName);

		// some info.
		Parser.GetKeyInt(CBRUSHGROUP, mBrushGroup);
		Parser.GetKeyInt(CBRUSHSELECTED, mFlags);
		Parser.GetKeyInt(CBRUSHENTITY, mEntityId);

		// if we can't get the brushid set it to a default of -1
		if(!Parser.GetKeyInt(CBRUSHID, mBrushId))
        {
			mBrushId = -1;
        }

        if(!Parser.GetKeyULong(CVOLSTATE, volumestate))
        {
          volumestate = 0;
        }

        if(!Parser.GetKeyInt(CSHOWVOL, m_ShowVolPolys))
        {
          m_ShowVolPolys = 0;
        }

		// get our position rotation etc.
		// if any are missing it doesn't really matter.
		Parser.GetKeyPoint(CBRUSHPOSITION, mBrushPosition[INDEX_BRUSH]);
        if (!Parser.GetKeyPoint(CBRUSHCENTER, mBrushCenter))
        {
          mBrushCenter = mBrushPosition[0];
        }

		Parser.GetKeyPoint(CBRUSHROTATION, mBrushRotation[INDEX_BRUSH]);
		Parser.GetKeyPoint(CBRUSHSCALE, mBrushScale[INDEX_BRUSH]);
		Parser.GetKeyPoint(CBRUSHSHEAR, mBrushShear[INDEX_BRUSH]);
		mRenderBrushPosition[INDEX_BRUSH] = mBrushPosition[INDEX_BRUSH];
		mRenderBrushRotation[INDEX_BRUSH] = mBrushRotation[INDEX_BRUSH];

		// get our number of polygons
		// get our polygons
		Parser.GetKeyInt(CBRUSHNUMBEROFPOLYS, NumberOfPolygons);

		// allocate room for them
		AllocateBrushPolygons(NumberOfPolygons );

       // DPW - allocate room for the polygon texture names
        AllocateBrushPolyTextures (NumberOfPolygons );
		
 		// read our polygons
		for(Current = 0; Current < mNumberOfPolygons; Current++ ) {
			Parser.ReadPolygon(mPolygons[Current]);
		}

        // TEST----------------------------
        switch (BrushType)
        {
        case BOX_BRUSH:
          {
          CCreateBoxDialog  boxdialog;

          boxdialog.m_Solid = BrushSpecifics.box.m_Solid;
          boxdialog.m_Thickness = BrushSpecifics.box.m_Thickness;
          boxdialog.m_XSizeBot = BrushSpecifics.box.m_XSizeBot;
          boxdialog.m_XSizeTop = BrushSpecifics.box.m_XSizeTop;
          boxdialog.m_YSize = BrushSpecifics.box.m_YSize;
          boxdialog.m_ZSizeBot = BrushSpecifics.box.m_ZSizeBot;
          boxdialog.m_ZSizeTop = BrushSpecifics.box.m_ZSizeTop;

          boxdialog.CreateBox(this);
          }
          break;
        case TRI_BRUSH:
          {
          CCreateTriDialog  tridialog;

          tridialog.m_Solid = BrushSpecifics.triangle.m_Solid;
          tridialog.m_Thickness = BrushSpecifics.triangle.m_Thickness;
          tridialog.m_XSizeBot = BrushSpecifics.triangle.m_XSizeBot;
          tridialog.m_YSize = BrushSpecifics.triangle.m_YSize;
          tridialog.m_ZSizeBot = BrushSpecifics.triangle.m_ZSizeBot;
          tridialog.m_ZSizeTop = BrushSpecifics.triangle.m_ZSizeTop;

          tridialog.CreateTriangle(this);
          }
         break;
        case SPHERE_BRUSH:
          {
          CCreateSpheroidDialog spheredialog;

          spheredialog.m_HalfSphere = BrushSpecifics.sphere.m_HalfSphere;
          spheredialog.m_HorizontalBands = BrushSpecifics.sphere.m_HorizontalBands;
          spheredialog.m_Solid = BrushSpecifics.sphere.m_Solid;
          spheredialog.m_Thickness = BrushSpecifics.sphere.m_Thickness;
          spheredialog.m_VerticalBands = BrushSpecifics.sphere.m_VerticalBands;
          spheredialog.m_XSize = BrushSpecifics.sphere.m_XSize;
          spheredialog.m_YSize = BrushSpecifics.sphere.m_YSize;
          spheredialog.m_ZSize = BrushSpecifics.sphere.m_ZSize;

          spheredialog.CreateSpheroid(this);
          }
          break;
        case STAIR_BRUSH:
          {
          CCreateStaircaseDialog  stairdialog;

          stairdialog.m_Height = BrushSpecifics.staircase.m_Height;
          stairdialog.m_Length = BrushSpecifics.staircase.m_Length;
          stairdialog.m_MakeRamp = BrushSpecifics.staircase.m_MakeRamp;
          stairdialog.m_NumberOfStairs = BrushSpecifics.staircase.m_NumberOfStairs;
          stairdialog.m_Width = BrushSpecifics.staircase.m_Width;

          stairdialog.MakeBrush(this);
          }
        break;
        case CYL_BRUSH:
          {
          CCreateCylDialog  cyldialog;

          cyldialog.m_BotXOffset = BrushSpecifics.cylinder.m_BotXOffset;
          cyldialog.m_BotXSize = BrushSpecifics.cylinder.m_BotXSize;
          cyldialog.m_BotZOffset = BrushSpecifics.cylinder.m_BotZOffset;
          cyldialog.m_BotZSize = BrushSpecifics.cylinder.m_BotZSize;
          cyldialog.m_Ring = BrushSpecifics.cylinder.m_Ring;
          cyldialog.m_Solid = BrushSpecifics.cylinder.m_Solid;
          cyldialog.m_Thickness = BrushSpecifics.cylinder.m_Thickness;
          cyldialog.m_TopXOffset = BrushSpecifics.cylinder.m_TopXOffset;
          cyldialog.m_TopXSize = BrushSpecifics.cylinder.m_TopXSize;
          cyldialog.m_TopZOffset = BrushSpecifics.cylinder.m_TopZOffset;
          cyldialog.m_TopZSize = BrushSpecifics.cylinder.m_TopZSize;
          cyldialog.m_VerticalStripes = BrushSpecifics.cylinder.m_VerticalStripes;
          cyldialog.m_YSize = BrushSpecifics.cylinder.m_YSize;

          cyldialog.CreateCyl(this);
          }
          break;
        }
        // TEST----------------------------

        // DPW - read in the face texture names if they're there, if not default to 0
        for (Current = 0; Current < mNumberOfPolygons; Current++ )
        {
            if (!Parser.GetKeyInt (CFACETEXTUREID, texture_id) )
                texture_id = 0;
   
            if( texture_id > maxTextureID )
            {
//               AfxMessageBox(LPCTSTR("Texture set to null" ) );
               texture_id = 0;
            }

            // Store it into our array
            mTextureID[Current].TextureID = texture_id;

            if (!Parser.GetKeyInt (CFACETEXTURESHIFTX, texture_shiftx) )
                texture_shiftx = 0;

            // Store it into our array
            mTextureID[Current].TextureShift.x = texture_shiftx;

            if (!Parser.GetKeyInt (CFACETEXTURESHIFTY, texture_shifty) )
                texture_shifty = 0;

            // Store it into our array
            mTextureID[Current].TextureShift.y = texture_shifty;

            int texture_flip;
            if (!Parser.GetKeyInt (CFACETEXTUREFLIPH, texture_flip))
              texture_flip = 0;

            mTextureID[Current].TextureFlipH = texture_flip;

            if (!Parser.GetKeyInt (CFACETEXTUREFLIPV, texture_flip))
              texture_flip = 0;

            mTextureID[Current].TextureFlipV = texture_flip;

            // Get Texture Rotation value
       		if (!Parser.GetKeyString(CFACETEXTUREROTATE, tmpbuf1))
              texture_rotate = 0.F;
            else
              texture_rotate = atof((LPCTSTR)tmpbuf1);

            mTextureID[Current].TextureRotate = texture_rotate;

            // Get Texture Scaling value
       		if (!Parser.GetKeyString(CFACETEXTURESCALE, tmpbuf1))
              texture_scale = 1.F;
            else
              texture_scale = atof((LPCTSTR)tmpbuf1);

            mTextureID[Current].TextureScale = texture_scale;
        }
	
		// read the matrix (if we have it)
		if(!Parser.ReadMatrix(CBRUSHMATRIX, mRenderMatrix) )
		{
			// set the position
			mRenderMatrix.SetTranslation(mRenderBrushPosition[INDEX_BRUSH].X, 
				mRenderBrushPosition[INDEX_BRUSH].Y, mRenderBrushPosition[INDEX_BRUSH].Z);
		}
	
		// read the total matrix (if we have it)
		if(!Parser.ReadMatrix(CTOTALMATRIX, mTotalMatrix) )
		{
			// Create the total matrix for this brush
            SetupBrushMatrix();
            
            mTotalMatrix = mBrushMatrix;
		}
#if 0
        // TEST----------------------------
        switch (BrushType)
        {
        case BOX_BRUSH:
          {
          CCreateBoxDialog  boxdialog;

          boxdialog.m_Solid = BrushSpecifics.box.m_Solid;
          boxdialog.m_Thickness = BrushSpecifics.box.m_Thickness;
          boxdialog.m_XSizeBot = BrushSpecifics.box.m_XSizeBot;
          boxdialog.m_XSizeTop = BrushSpecifics.box.m_XSizeTop;
          boxdialog.m_YSize = BrushSpecifics.box.m_YSize;
          boxdialog.m_ZSizeBot = BrushSpecifics.box.m_ZSizeBot;
          boxdialog.m_ZSizeTop = BrushSpecifics.box.m_ZSizeTop;

          boxdialog.CreateBox(this);
          }
          break;
        case TRI_BRUSH:
          {
          CCreateTriDialog  tridialog;

          tridialog.m_Solid = BrushSpecifics.triangle.m_Solid;
          tridialog.m_Thickness = BrushSpecifics.triangle.m_Thickness;
          tridialog.m_XSizeBot = BrushSpecifics.triangle.m_XSizeBot;
          tridialog.m_YSize = BrushSpecifics.triangle.m_YSize;
          tridialog.m_ZSizeBot = BrushSpecifics.triangle.m_ZSizeBot;
          tridialog.m_ZSizeTop = BrushSpecifics.triangle.m_ZSizeTop;

          tridialog.CreateTriangle(this);
          }
         break;
        case SPHERE_BRUSH:
          {
          CCreateSpheroidDialog spheredialog;

          spheredialog.m_HalfSphere = BrushSpecifics.sphere.m_HalfSphere;
          spheredialog.m_HorizontalBands = BrushSpecifics.sphere.m_HorizontalBands;
          spheredialog.m_Solid = BrushSpecifics.sphere.m_Solid;
          spheredialog.m_Thickness = BrushSpecifics.sphere.m_Thickness;
          spheredialog.m_VerticalBands = BrushSpecifics.sphere.m_VerticalBands;
          spheredialog.m_XSize = BrushSpecifics.sphere.m_XSize;
          spheredialog.m_YSize = BrushSpecifics.sphere.m_YSize;
          s spheredialog.m_ZSize = BrushSpecifics.sphere.m_ZSize;

          spheredialog.CreateSpheroid(this);
          }
          break;
        case STAIR_BRUSH:
          {
          CCreateStaircaseDialog  stairdialog;

          stairdialog.m_Height = BrushSpecifics.staircase.m_Height;
          stairdialog.m_Length = BrushSpecifics.staircase.m_Length;
          stairdialog.m_MakeRamp = BrushSpecifics.staircase.m_MakeRamp;
          stairdialog.m_NumberOfStairs = BrushSpecifics.staircase.m_NumberOfStairs;
          stairdialog.m_Width = BrushSpecifics.staircase.m_Width;

          stairdialog.MakeBrush(this);
          }
        break;
        case CYL_BRUSH:
          {
          CCreateCylDialog  cyldialog;

          cyldialog.m_BotXOffset = BrushSpecifics.cylinder.m_BotXOffset;
          cyldialog.m_BotXSize = BrushSpecifics.cylinder.m_BotXSize;
          cyldialog.m_BotZOffset = BrushSpecifics.cylinder.m_BotZOffset;
          cyldialog.m_BotZSize = BrushSpecifics.cylinder.m_BotZSize;
          cyldialog.m_Ring = BrushSpecifics.cylinder.m_Ring;
          cyldialog.m_Solid = BrushSpecifics.cylinder.m_Solid;
          cyldialog.m_Thickness = BrushSpecifics.cylinder.m_Thickness;
          cyldialog.m_TopXOffset = BrushSpecifics.cylinder.m_TopXOffset;
          cyldialog.m_TopXSize = BrushSpecifics.cylinder.m_TopXSize;
          cyldialog.m_TopZOffset = BrushSpecifics.cylinder.m_TopZOffset;
          cyldialog.m_TopZSize = BrushSpecifics.cylinder.m_TopZSize;
          cyldialog.m_VerticalStripes = BrushSpecifics.cylinder.m_VerticalStripes;
          cyldialog.m_YSize = BrushSpecifics.cylinder.m_YSize;

          cyldialog.CreateCyl(this);
          }
          break;
        }
        // TEST----------------------------
#endif
		Parser.GotoEnd();
	}
}

Point2F CThredBrush::GetCenterTextureOffsets(int which_face)
{
  Point2F center;

  center.x = 0.0;
  center.y = 0.0;

  // Find center of rotation
  for (int i = 0; i < mPolygons[which_face].NumberOfPoints; i++)
  {
    center.x += mPolygons[which_face].mTextureUV[i].x;
    center.y += mPolygons[which_face].mTextureUV[i].y;
  }

  center.x /= (float)(mPolygons[which_face].NumberOfPoints);
  center.y /= (float)(mPolygons[which_face].NumberOfPoints);

  return center;
}

void CThredBrush::RotateFaceTexture(int which_face, double angle)
{
  double  rotation;
  double  tmp_x, tmp_y;
  Point2F center;

  // angle is in degrees, so convert to radians...
  rotation = (angle / 180.0)*M_PI_VALUE;

  // Find center of rotation
  center = GetCenterTextureOffsets(which_face);

  for (int i = 0; i < mPolygons[which_face].NumberOfPoints; i++)
  {
    tmp_x = mPolygons[which_face].mTextureUV[i].x - center.x;
    tmp_y = mPolygons[which_face].mTextureUV[i].y - center.y;

    mPolygons[which_face].mTextureUV[i].x = center.x +
        (tmp_x * cos(rotation)) +
        (tmp_y * sin(rotation));

    mPolygons[which_face].mTextureUV[i].y = center.y + 
        (tmp_x * -sin(rotation)) +
        (tmp_y * cos(rotation));
  }

  NormalizeTexture (which_face);
}

void CThredBrush::ShiftFaceTextureU(int which_face, double offset)
{
  double  shift = 0.0;

  mTextureID[which_face].TextureShift.x = int(offset * 256.0);
#if 0
  for (int i = 0; i < mPolygons[which_face].NumberOfPoints; i++)
    if ((mPolygons[which_face].mTextureUV[i].x + (offset*256)) < 0.0)
      shift = 256.0;

  for (i = 0; i < mPolygons[which_face].NumberOfPoints; i++)
  {
    mPolygons[which_face].mTextureUV[i].x += ((offset*256) + shift);

    if (mPolygons[which_face].mTextureUV[i].x < 0.0)
      mPolygons[which_face].mTextureUV[i].x = 0.0;
  }
#endif
}


void CThredBrush::ShiftFaceTextureV(int which_face, double offset)
{
  double  shift = 0.0;

  mTextureID[which_face].TextureShift.y = int(offset * 256.0);
#if 0
  for (int i = 0; i < mPolygons[which_face].NumberOfPoints; i++)
    if ((mPolygons[which_face].mTextureUV[i].y + (offset*256)) < 0.0)
      shift = 256.0;

  for (i = 0; i < mPolygons[which_face].NumberOfPoints; i++)
  {
    mPolygons[which_face].mTextureUV[i].y += ((offset*256) + shift);

    if (mPolygons[which_face].mTextureUV[i].y < 0.0)
      mPolygons[which_face].mTextureUV[i].y = 0.0;
  }
#endif
}


#define BOX_YZ            0
#define BOX_XY            1
#define BOX_XZ            2
#define TEXTURE_SCALE     1.F //0.001F
#define MAX_TEXTURE_COOR  256
#define MAX_TEXTURE_MASK  (MAX_TEXTURE_COOR-1)
void CThredBrush::BoxMap()
{
  int     i, j;
  int     mapping;

  if (mTransformedPolysValid)
  {
    for (i = 0; i < mNumberOfPolygons; i++)
    {
      // Calculate Normal of this polygon
      Point3F normal;
      Point3F vect1, vect2;
      Point2F tmin(+1.0E20f, +1.0E20f);
      Point2F tmax(-1.0E20F, -1.0E20F);
      int     numpoints = mTransformedPolygons[i].NumberOfPoints;

      vect1.x = mTransformedPolygons[i].Points[1].X - mTransformedPolygons[i].Points[0].X;
      vect1.y = mTransformedPolygons[i].Points[1].Z - mTransformedPolygons[i].Points[0].Z;
      vect1.z = mTransformedPolygons[i].Points[1].Y - mTransformedPolygons[i].Points[0].Y;

      vect2.x = mTransformedPolygons[i].Points[numpoints-1].X - mTransformedPolygons[i].Points[0].X;
      vect2.y = mTransformedPolygons[i].Points[numpoints-1].Z - mTransformedPolygons[i].Points[0].Z;
      vect2.z = mTransformedPolygons[i].Points[numpoints-1].Y - mTransformedPolygons[i].Points[0].Y;

      m_cross(vect1, vect2, &normal);

      // Select the proper box face
      if (fabs(normal.x) > fabs(normal.y))
        mapping = ((fabs(normal.x) > fabs(normal.z)) ? BOX_YZ : BOX_XY);
      else
        mapping = ((fabs(normal.y) > fabs(normal.z)) ? BOX_XZ : BOX_XY);

      // Map the vertices to the box face
      for (j = 0; j < numpoints; j++)
      {
        switch (mapping)
        {
          // DPW - note that the y values should be (-) for the new 3space stuff
        case BOX_YZ:
          mPolygons[i].mTextureUV[j].x = mTransformedPolygons[i].Points[j].Z;   // y in 3space
          mPolygons[i].mTextureUV[j].y = -mTransformedPolygons[i].Points[j].Y;  // -z in 3space
          break;
        case BOX_XY:
          mPolygons[i].mTextureUV[j].x = mTransformedPolygons[i].Points[j].X;   // x in 3space
          mPolygons[i].mTextureUV[j].y = -mTransformedPolygons[i].Points[j].Z;  // -y in 3space
          break;
        case BOX_XZ:
          mPolygons[i].mTextureUV[j].x = mTransformedPolygons[i].Points[j].X;   // x in 3space
          mPolygons[i].mTextureUV[j].y = -mTransformedPolygons[i].Points[j].Y;  // -z in 3space
          break;
        } // end switch

        mPolygons[i].mTextureUV[j].x *= mTextureID[i].TextureScale;//theApp.m_texturescale;
        mPolygons[i].mTextureUV[j].y *= mTextureID[i].TextureScale;//theApp.m_texturescale;

        if (mPolygons[i].mTextureUV[j].x < tmin.x)
          tmin.x = mPolygons[i].mTextureUV[j].x;

        if (mPolygons[i].mTextureUV[j].y < tmin.y)
          tmin.y = mPolygons[i].mTextureUV[j].y;

        if (mPolygons[i].mTextureUV[j].x > tmax.x)
          tmax.x = mPolygons[i].mTextureUV[j].x;

        if (mPolygons[i].mTextureUV[j].y > tmax.y)
          tmax.y = mPolygons[i].mTextureUV[j].y;
      } // end loop thru vertices

      // Min/Max must be whole numbers
      tmin.x = floor(tmin.x); tmin.y = floor(tmin.y);
      tmax.x = ceil(tmax.x);  tmax.y = ceil(tmax.y);

      mPolygons[i].mTextureSize.x = int(tmax.x - tmin.x);
      mPolygons[i].mTextureSize.y = int(tmax.y - tmin.y);

      if ((mPolygons[i].mTextureSize.x < 1) || (mPolygons[i].mTextureSize.y < 1))
        return;

      // Calculate texture offsets
      mPolygons[i].mTextureOffset.x = 0;//int(tmin.x) & MAX_TEXTURE_MASK;
      mPolygons[i].mTextureOffset.y = 0;//int(tmin.y) & MAX_TEXTURE_MASK;

      // Translate coordinates so they fit on the texture
      for (j = 0; j < numpoints; j++)
      {
        mPolygons[i].mTextureUV[j].x -= tmin.x;
        mPolygons[i].mTextureUV[j].y -= tmin.y;
      }
    } // end loop thru polygons
  }
}


void CThredBrush::NormalizeTexture(int which_poly)
{
  int     j;
  Point2F tmin(+1.0E20f, +1.0E20f);
  Point2F tmax(-1.0E20F, -1.0E20F);

  int numpoints = mTransformedPolygons[which_poly].NumberOfPoints;

  for (j = 0; j < numpoints; j++)
  {
      if (mPolygons[which_poly].mTextureUV[j].x < tmin.x)
        tmin.x = mPolygons[which_poly].mTextureUV[j].x;

      if (mPolygons[which_poly].mTextureUV[j].y < tmin.y)
        tmin.y = mPolygons[which_poly].mTextureUV[j].y;

      if (mPolygons[which_poly].mTextureUV[j].x > tmax.x)
        tmax.x = mPolygons[which_poly].mTextureUV[j].x;

      if (mPolygons[which_poly].mTextureUV[j].y > tmax.y)
        tmax.y = mPolygons[which_poly].mTextureUV[j].y;
  } // end loop thru vertices

  // Min/Max must be whole numbers
  tmin.x = floor(tmin.x); tmin.y = floor(tmin.y);
  tmax.x = ceil(tmax.x);  tmax.y = ceil(tmax.y);

  mPolygons[which_poly].mTextureSize.x = int(tmax.x - tmin.x);
  mPolygons[which_poly].mTextureSize.y = int(tmax.y - tmin.y);

  if ((mPolygons[which_poly].mTextureSize.x < 1) || (mPolygons[which_poly].mTextureSize.y < 1))
    return;

  // Calculate texture offsets
  mPolygons[which_poly].mTextureOffset.x = int(tmin.x) & MAX_TEXTURE_MASK;
  mPolygons[which_poly].mTextureOffset.y = int(tmin.y) & MAX_TEXTURE_MASK;

  // Translate coordinates so they fit on the texture
  for (j = 0; j < numpoints; j++)
  {
      mPolygons[which_poly].mTextureUV[j].x -= tmin.x;
      mPolygons[which_poly].mTextureUV[j].y -= tmin.y;
  }
}

void CThredBrush::Resize(Point3F* minbound, Point3F* delta, CTHREDDoc* pDoc, ThredBox* BoundingBox)
{
  CTransformMatrix  TempMatrix;
  CTransformMatrix  ToMatrix, FromMatrix;
  ThredPoint        scaleamount;
  double            oldsize, newsize;

  scaleamount.X = 1.0;
  scaleamount.Y = 1.0;
  scaleamount.Z = 1.0;

  if (delta->x != 0.0)
  {
    oldsize = BoundingBox->mLargest.X - BoundingBox->mSmallest.X;
    newsize = floor(((oldsize + delta->x) * 10.0) + 0.5) / 10.0;

    if (pDoc->mUseGrid)
    {
      if (newsize < pDoc->mGridSize)
      {
        newsize = pDoc->mGridSize;
      }
    } else {
      if (newsize < 2.0)
      {
        newsize = 2.0;
      }
    }

    scaleamount.X = newsize / oldsize;
  }

  if (delta->y != 0.0)
  {
    oldsize = BoundingBox->mLargest.Y - BoundingBox->mSmallest.Y;
    newsize = floor(((oldsize + delta->y) * 10.0) + 0.5) / 10.0;

    if (pDoc->mUseGrid)
    {
      if (newsize < pDoc->mGridSize)
      {
        newsize = pDoc->mGridSize;
      }
    } else {
      if (newsize < 2.0)
      {
        newsize = 2.0;
      }
    }

    scaleamount.Y = newsize / oldsize;
  }

  if (delta->z != 0.0)
  {
    oldsize = BoundingBox->mLargest.Z - BoundingBox->mSmallest.Z;
    newsize = floor(((oldsize + delta->z) * 10.0) + 0.5) / 10.0;

    if (pDoc->mUseGrid)
    {
      if (newsize < pDoc->mGridSize)
      {
        newsize = pDoc->mGridSize;
      }
    } else {
      if (newsize < 2.0)
      {
        newsize = 2.0;
      }
    }

    scaleamount.Z = newsize / oldsize;
  }
  ToMatrix.ClearMatrix();
  FromMatrix.ClearMatrix();
  TempMatrix.ClearMatrix();

  // Set up transformation
  ToMatrix.SetTranslation(-minbound->x, -minbound->y, -minbound->z);
  FromMatrix.SetTranslation(minbound->x, minbound->y, minbound->z);
  TempMatrix.SetScale(scaleamount.X, scaleamount.Y, scaleamount.Z);

  ////////////////////////////////////////
  mTotalMatrix.MultiplyMatrix(ToMatrix);
  mTotalMatrix.MultiplyMatrix(TempMatrix);
  mTotalMatrix.MultiplyMatrix(FromMatrix);
  ////////////////////////////////////////

  CalculateBrushCenter(); // DPW

  mTransformedPolysValid = FALSE;
}


void CThredBrush::Shear(Point3F* minbound, Point3F* delta, CTHREDDoc* pDoc, ThredBox* BoundingBox, ShearType wrt)
{
  CTransformMatrix  TempMatrix;
  CTransformMatrix  ToMatrix, FromMatrix;
  Point3F           shearval;

  shearval.x = delta->x;//floor((delta->x * 10.0) + 0.5) / 10.0;
  shearval.y = delta->y;//floor((delta->y * 10.0) + 0.5) / 10.0;
  shearval.z = delta->z;//floor((delta->z * 10.0) + 0.5) / 10.0;

  ToMatrix.ClearMatrix();
  FromMatrix.ClearMatrix();
  TempMatrix.ClearMatrix();

  // Set up transformation
  ToMatrix.SetTranslation(-minbound->x, -minbound->y, -minbound->z);
  FromMatrix.SetTranslation(minbound->x, minbound->y, minbound->z);

  switch (wrt)
  {
  case XY_SHEAR:
    TempMatrix.SetXYShear(shearval.x);
    break;
  case XZ_SHEAR:
    TempMatrix.SetXZShear(shearval.x);
    break;
  case YX_SHEAR:
    TempMatrix.SetYXShear(shearval.y);
    break;
  case YZ_SHEAR:
    TempMatrix.SetYZShear(shearval.y);
    break;
  case ZX_SHEAR:
    TempMatrix.SetZXShear(shearval.z);
    break;
  case ZY_SHEAR:
    TempMatrix.SetZYShear(shearval.z);
    break;
  }

  ////////////////////////////////////////
  mTotalMatrix.MultiplyMatrix(ToMatrix);
  mTotalMatrix.MultiplyMatrix(TempMatrix);
  mTotalMatrix.MultiplyMatrix(FromMatrix);
  ////////////////////////////////////////

  CalculateBrushCenter(); // DPW

  mTransformedPolysValid = FALSE;
}

void CThredBrush::DoneResize(double GridSize)
{
#if 0
  int     curr_poly, curr_vertex;
  int     total_vertices = 0;
  Point3F summed_values(0.0, 0.0, 0.0);

  for (curr_poly = 0; curr_poly < mNumberOfTransformedPolygons; curr_poly++)
  {
    for (curr_vertex = 0; curr_vertex < mTransformedPolygons[curr_poly].NumberOfPoints; curr_vertex++)
    {
      summed_values.x += mPolygons[curr_poly].Points[curr_vertex].X;
      summed_values.y += mPolygons[curr_poly].Points[curr_vertex].Y;
      summed_values.z += mPolygons[curr_poly].Points[curr_vertex].Z;
      total_vertices++;
    }
  }

  mBrushCenter.X = summed_values.x / total_vertices;
  mBrushCenter.Y = summed_values.y / total_vertices;
  mBrushCenter.Z = summed_values.z / total_vertices;

  //mBrushPosition[0].X -= mBrushCenter.X;
  //mBrushPosition[0].Y -= mBrushCenter.Y;
  //mBrushPosition[0].Z -= mBrushCenter.Z;
#endif

  mTransformedPolysValid = FALSE;
}

Point3F CThredBrush::GetBrushFaceCenter(int whichface)
{
  int     curr_vertex;
  int     total_vertices = 0;
  Point3F summed_values(0.0, 0.0, 0.0);
  

  for (curr_vertex = 0; curr_vertex < mTransformedPolygons[whichface].NumberOfPoints; curr_vertex++)
  {
    summed_values.x += mTransformedPolygons[whichface].Points[curr_vertex].X;
    summed_values.y += mTransformedPolygons[whichface].Points[curr_vertex].Y;
    summed_values.z += mTransformedPolygons[whichface].Points[curr_vertex].Z;
    total_vertices++;
  }

  summed_values.x = summed_values.x / (double)total_vertices;
  summed_values.y = summed_values.y / (double)total_vertices;
  summed_values.z = summed_values.z / (double)total_vertices;
  return summed_values;
}

void CThredBrush::CalculateBrushCenter()
{
  int     curr_poly, curr_vertex;
  int     total_vertices = 0;
  Point3F summed_values(0.0, 0.0, 0.0);

  for (curr_poly = 0; curr_poly < mNumberOfTransformedPolygons; curr_poly++)
  {
    for (curr_vertex = 0; curr_vertex < mTransformedPolygons[curr_poly].NumberOfPoints; curr_vertex++)
    {
      summed_values.x += mPolygons[curr_poly].Points[curr_vertex].X;
      summed_values.y += mPolygons[curr_poly].Points[curr_vertex].Y;
      summed_values.z += mPolygons[curr_poly].Points[curr_vertex].Z;
      total_vertices++;
    }
  }

  mBrushRealCenter.X = (summed_values.x / total_vertices) + mBrushPosition[0].X;
  mBrushRealCenter.Y = (summed_values.y / total_vertices) + mBrushPosition[0].Y;
  mBrushRealCenter.Z = (summed_values.z / total_vertices) + mBrushPosition[0].Z;
}


void CThredBrush::BeginTextureChange( CThredBrush::BrushTextureChange type )
{
   int     i, j;
   int     mapping;

   for (i = 0; i < mNumberOfPolygons; i++)
   {
      // Calculate Normal of this polygon
      Point3F normal;
      Point3F vect1, vect2;
      Point2F tmin(+1.0E20f, +1.0E20f);
      Point2F tmax(-1.0E20F, -1.0E20F);
      int numpoints = mTransformedPolygons[i].NumberOfPoints;

      vect1.x = mTransformedPolygons[i].Points[1].X - mTransformedPolygons[i].Points[0].X;
      vect1.y = mTransformedPolygons[i].Points[1].Z - mTransformedPolygons[i].Points[0].Z;
      vect1.z = mTransformedPolygons[i].Points[1].Y - mTransformedPolygons[i].Points[0].Y;

      vect2.x = mTransformedPolygons[i].Points[numpoints-1].X - mTransformedPolygons[i].Points[0].X;
      vect2.y = mTransformedPolygons[i].Points[numpoints-1].Z - mTransformedPolygons[i].Points[0].Z;
      vect2.z = mTransformedPolygons[i].Points[numpoints-1].Y - mTransformedPolygons[i].Points[0].Y;

      m_cross(vect1, vect2, &normal);

      // Select the proper box face
      if (fabs(normal.x) > fabs(normal.y))
         mapping = ((fabs(normal.x) > fabs(normal.z)) ? BOX_YZ : BOX_XY);
      else
         mapping = ((fabs(normal.y) > fabs(normal.z)) ? BOX_XZ : BOX_XY);

      // Map the vertices to the box face
      for (j = 0; j < numpoints; j++)
      {
         switch (mapping)
         {
            // DPW - note that the y values should be (-) for the new 3space stuff
            case BOX_YZ:
               mPolygons[i].mTextureUV[j].x = mTransformedPolygons[i].Points[j].Z;   // y in 3space
               mPolygons[i].mTextureUV[j].y = -mTransformedPolygons[i].Points[j].Y;  // -z in 3space
               break;
            case BOX_XY:
               mPolygons[i].mTextureUV[j].x = mTransformedPolygons[i].Points[j].X;   // x in 3space
               mPolygons[i].mTextureUV[j].y = -mTransformedPolygons[i].Points[j].Z;  // -y in 3space
               break;
            case BOX_XZ:
               mPolygons[i].mTextureUV[j].x = mTransformedPolygons[i].Points[j].X;   // x in 3space
               mPolygons[i].mTextureUV[j].y = -mTransformedPolygons[i].Points[j].Y;  // -z in 3space
               break;
         } 

         // adjust the texture scale
         float TextureScale;
         
         int bits = ITRGeometry::Surface::Constants::textureScaleBits;
         
         int signMask = 1 << ( bits - 1 );
         int valueMask = signMask - 1;
         
         if( mTransformedPolygons[i].mTextureScaleShift & signMask )
         {
            TextureScale = mTextureID[i].TextureScale / 
               ( 1 << ( mTransformedPolygons[i].mTextureScaleShift & valueMask ) );
         }
         else
         {
            TextureScale = mTextureID[i].TextureScale *
               ( 1 << ( mTransformedPolygons[i].mTextureScaleShift & valueMask ) );
         }
         
         mPolygons[i].mTextureUV[j].x *= TextureScale;
         mPolygons[i].mTextureUV[j].y *= TextureScale;

         if (mPolygons[i].mTextureUV[j].x < tmin.x)
            tmin.x = mPolygons[i].mTextureUV[j].x;

         if (mPolygons[i].mTextureUV[j].y < tmin.y)
            tmin.y = mPolygons[i].mTextureUV[j].y;

         if (mPolygons[i].mTextureUV[j].x > tmax.x)
            tmax.x = mPolygons[i].mTextureUV[j].x;

         if (mPolygons[i].mTextureUV[j].y > tmax.y)
            tmax.y = mPolygons[i].mTextureUV[j].y;
      }

      // Min/Max must be whole numbers
      tmin.x = floor(tmin.x); tmin.y = floor(tmin.y);
      tmax.x = ceil(tmax.x);  tmax.y = ceil(tmax.y);

      mPolygons[i].mTextureSize.x = int(tmax.x - tmin.x);
      mPolygons[i].mTextureSize.y = int(tmax.y - tmin.y);

      if( ( mPolygons[i].mTextureSize.x < 1) || ( mPolygons[i].mTextureSize.y < 1) )
         return;

      // Calculate texture offsets
      mPolygons[i].mTextureOffset.x = 0;
      mPolygons[i].mTextureOffset.y = 0;

      // do the rotation
      Point2F tmpPnt;
      float angle;
      tmpPnt.x = mPolygons[i].mTextureUV[1].x - mPolygons[i].mTextureUV[0].x;
      tmpPnt.y = mPolygons[i].mTextureUV[1].y - mPolygons[i].mTextureUV[0].y;

      angle = 0.0;
      
      if( !( ( tmpPnt.x == 0.0 ) && ( tmpPnt.y == 0.0 ) ) )
      {
         angle = acos( fabs( tmpPnt.y / tmpPnt.len() ) );

         // convert to degrees
         angle = ( 180 * angle ) / M_PI_VALUE;
         if( angle > 360.0 )
            angle -= 360.0;
      }
         
      // store this pseudo angle
      mTextureID[i].LastTextureRotate = angle;
      
      // store the pseudo shift value
      mTextureID[i].LastTextureShift.x = 256 - ( int(tmin.x) & MAX_TEXTURE_MASK );
      mTextureID[i].LastTextureShift.y = 256 - ( int(tmin.y) & MAX_TEXTURE_MASK );

      // Translate coordinates so they fit on the texture
      for (j = 0; j < numpoints; j++)
      {
         mPolygons[i].mTextureUV[j].x -= tmin.x;
         mPolygons[i].mTextureUV[j].y -= tmin.y;
      }
   }
}

void CThredBrush::EndTextureChange( CThredBrush::BrushTextureChange type )
{
   int     i, j;
   int     mapping;

   if( type == TEX_ROTATE )
      return;
   for (i = 0; i < mNumberOfPolygons; i++)
   {
      // Calculate Normal of this polygon
      Point3F normal;
      Point3F vect1, vect2;
      Point2F tmin(+1.0E20f, +1.0E20f);
      Point2F tmax(-1.0E20F, -1.0E20F);
      int numpoints = mTransformedPolygons[i].NumberOfPoints;

      vect1.x = mTransformedPolygons[i].Points[1].X - mTransformedPolygons[i].Points[0].X;
      vect1.y = mTransformedPolygons[i].Points[1].Z - mTransformedPolygons[i].Points[0].Z;
      vect1.z = mTransformedPolygons[i].Points[1].Y - mTransformedPolygons[i].Points[0].Y;

      vect2.x = mTransformedPolygons[i].Points[numpoints-1].X - mTransformedPolygons[i].Points[0].X;
      vect2.y = mTransformedPolygons[i].Points[numpoints-1].Z - mTransformedPolygons[i].Points[0].Z;
      vect2.z = mTransformedPolygons[i].Points[numpoints-1].Y - mTransformedPolygons[i].Points[0].Y;

      m_cross(vect1, vect2, &normal);

      // Select the proper box face
      if (fabs(normal.x) > fabs(normal.y))
         mapping = ((fabs(normal.x) > fabs(normal.z)) ? BOX_YZ : BOX_XY);
      else
         mapping = ((fabs(normal.y) > fabs(normal.z)) ? BOX_XZ : BOX_XY);

      // Map the vertices to the box face
      for (j = 0; j < numpoints; j++)
      {
         switch (mapping)
         {
            // DPW - note that the y values should be (-) for the new 3space stuff
            case BOX_YZ:
               mPolygons[i].mTextureUV[j].x = mTransformedPolygons[i].Points[j].Z;   // y in 3space
               mPolygons[i].mTextureUV[j].y = -mTransformedPolygons[i].Points[j].Y;  // -z in 3space
               break;
            case BOX_XY:
               mPolygons[i].mTextureUV[j].x = mTransformedPolygons[i].Points[j].X;   // x in 3space
               mPolygons[i].mTextureUV[j].y = -mTransformedPolygons[i].Points[j].Z;  // -y in 3space
               break;
            case BOX_XZ:
               mPolygons[i].mTextureUV[j].x = mTransformedPolygons[i].Points[j].X;   // x in 3space
               mPolygons[i].mTextureUV[j].y = -mTransformedPolygons[i].Points[j].Y;  // -z in 3space
               break;
         } 

         // adjust the texture scale
         float TextureScale;
         int bits = ITRGeometry::Surface::Constants::textureScaleBits;
         int signMask = 1 << ( bits - 1 );
         int valueMask = signMask - 1;
         
         if( mTransformedPolygons[i].mTextureScaleShift & signMask )
         {
            TextureScale = mTextureID[i].TextureScale / 
               ( 1 << ( mTransformedPolygons[i].mTextureScaleShift & valueMask ) );
         }
         else
         {
            TextureScale = mTextureID[i].TextureScale *
               ( 1 << ( mTransformedPolygons[i].mTextureScaleShift & valueMask ) );
         }
         
         mPolygons[i].mTextureUV[j].x *= TextureScale;
         mPolygons[i].mTextureUV[j].y *= TextureScale;

         if (mPolygons[i].mTextureUV[j].x < tmin.x)
            tmin.x = mPolygons[i].mTextureUV[j].x;

         if (mPolygons[i].mTextureUV[j].y < tmin.y)
            tmin.y = mPolygons[i].mTextureUV[j].y;

         if (mPolygons[i].mTextureUV[j].x > tmax.x)
            tmax.x = mPolygons[i].mTextureUV[j].x;

         if (mPolygons[i].mTextureUV[j].y > tmax.y)
            tmax.y = mPolygons[i].mTextureUV[j].y;
      }

      // Min/Max must be whole numbers
      tmin.x = floor(tmin.x); tmin.y = floor(tmin.y);
      tmax.x = ceil(tmax.x);  tmax.y = ceil(tmax.y);

      mPolygons[i].mTextureSize.x = int(tmax.x - tmin.x);
      mPolygons[i].mTextureSize.y = int(tmax.y - tmin.y);

      if( ( mPolygons[i].mTextureSize.x < 1) || ( mPolygons[i].mTextureSize.y < 1) )
         return;

      // Calculate texture offsets
      mPolygons[i].mTextureOffset.x = 0;
      mPolygons[i].mTextureOffset.y = 0;

      if( type == TEX_ROTATE )
      {
         // do the rotation
         Point2F tmpPnt;
         float angle;
         tmpPnt.x = mPolygons[i].mTextureUV[1].x - mPolygons[i].mTextureUV[0].x;
         tmpPnt.y = mPolygons[i].mTextureUV[1].y - mPolygons[i].mTextureUV[0].y;

         angle = 0.0;
         
         if( !( ( tmpPnt.x == 0.0 ) && ( tmpPnt.y == 0.0 ) ) )
         {
            angle = acos( fabs( tmpPnt.y / tmpPnt.len() ) );

            // convert to degrees
            angle = ( 180 * angle ) / M_PI_VALUE;
            if( angle > 360.0 )
               angle -= 360.0;

            angle = angle - mTextureID[i].LastTextureRotate;
         }

         mTextureID[i].TextureRotate += angle;
         if( mTextureID[i].TextureRotate >= 360.F )
            mTextureID[i].TextureRotate -= 360.F;
         if( mTextureID[i].TextureRotate <= 0.f )
            mTextureID[i].TextureRotate += 360.f;
      }
         
      // Translate coordinates so they fit on the texture
      if( type == TEX_MOVE )
      {
         Point2I tmpPnt;
         
         // get the shift values
         tmpPnt.x = 256 - ( int(tmin.x) & MAX_TEXTURE_MASK );
         tmpPnt.y = 256 - ( int(tmin.y) & MAX_TEXTURE_MASK );
         
         tmpPnt.x -= mTextureID[i].LastTextureShift.x;
         tmpPnt.y -= mTextureID[i].LastTextureShift.y;
         
         mTextureID[i].TextureShift.x += tmpPnt.x;
         mTextureID[i].TextureShift.y += tmpPnt.y;

         // adjust
         if( mTextureID[i].TextureShift.x < 0 )
            mTextureID[i].TextureShift.x = 256 - ( abs( mTextureID[i].TextureShift.x ) & MAX_TEXTURE_MASK );
         else
            mTextureID[i].TextureShift.x &= MAX_TEXTURE_MASK;
         if( mTextureID[i].TextureShift.y < 0 )
            mTextureID[i].TextureShift.y = 256 - ( abs( mTextureID[i].TextureShift.y ) & MAX_TEXTURE_MASK );
         else
            mTextureID[i].TextureShift.y &= MAX_TEXTURE_MASK;
            
//         if( mTextureID[i].TextureShift.x >= 256 )
//            mTextureID[i].TextureShift.x -= 256;
//         if( mTextureID[i].TextureShift.x < 0 )
//            mTextureID[i].TextureShift.x += 256;
//         if( mTextureID[i].TextureShift.y >= 256 )
//            mTextureID[i].TextureShift.y -= 256;
//         if( mTextureID[i].TextureShift.y < 0 )
//            mTextureID[i].TextureShift.y += 256;
      }
      
      for (j = 0; j < numpoints; j++)
      {
         mPolygons[i].mTextureUV[j].x -= tmin.x;
         mPolygons[i].mTextureUV[j].y -= tmin.y;
      }
   }
}
