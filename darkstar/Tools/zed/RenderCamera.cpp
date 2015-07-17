// RenderCamera.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>
#include "THRED.h"
#include "RenderCamera.h"
#include "ts_vertex.h"
#include "ts_PointArray.h"
#include "gfxmetrics.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/************* CAMERA/ENGINE design

	We are going to have a class within the camera
which is a span buffer. The camera will have a mode.
If the camera needs the span buffer in it's current mode
then it will init it during onsize commands to it's
new size (if that's needed).  Then in our renderer
we pass a pointer to our camera, get the pointer to
the span buffer and send the spans to it.  Then we
tell the span buffer to render all the spans to the
screen.  So Obviously the first thing we need it the span
buffer class.  The reason this goes in the camera is
that each view needs it's own span buffer.

*/


/////////////////////////////////////////////////////////////////////////////
// CRenderCamera

CRenderCamera::CRenderCamera()
{
	// set our default position
	mCameraPosition.X = DEFAULT_CAMERA_X;
	mCameraPosition.Y = DEFAULT_CAMERA_Y;
	mCameraPosition.Z = DEFAULT_CAMERA_Z;

	mCameraDirection.X = DEFAULT_CAMERA_YAW;
	mCameraDirection.Y = DEFAULT_CAMERA_PITCH;
	mCameraDirection.Z = DEFAULT_CAMERA_ROLL;

	// DPW - Needed for TS compatibility
	//for (int i = 0; i < MAX_POLYGON_VERTS; i++)
	//{
	//	poly_point_2d[i] = new Point2I(0,0);
	//}
}

CRenderCamera::~CRenderCamera()
{
	// DPW - Clean up after ourselves
	//for (int i = 0; i < MAX_POLYGON_VERTS; i++)
	//	delete poly_point_2d[i];
}



void CRenderCamera::SetupCamera(CRect& ClippingRect)//, RGBQUAD PaletteQuad[256])
{
	SetupCamera(ClippingRect.left, ClippingRect.right, 
		ClippingRect.top, ClippingRect.bottom);//, PaletteQuad);  	 
}

//===================================================================
// This function sets up the matrix that the camera owns
// that that the apply matrix function of the matrix
// will work to properly rotate the points into view.
//===================================================================
void CRenderCamera::SetupMatrix()
{
	// clear the matrix
	mMatrix.ClearMatrix();

	// set the translation
	mMatrix.SetTranslation(-mCameraPosition.X, -mCameraPosition.Y, -mCameraPosition.Z );

	// set the rotation matrix
	mMatrix.CreateRotationMatrix(-mCameraDirection.X, -mCameraDirection.Y, -mCameraDirection.Z);
	mRotateOnlyMatrix = mMatrix;

}

//===================================================================
// This funtion will take a point and do the perspective transform
// on it.  You end up with the X and Y values representing
// the values that should be on screen for this current
// view port.
//===================================================================
void CRenderCamera::DoPerspectiveTransform(ThredPoint& ThePoint )
{
	// apply the transform 
	mMatrix.ApplyMatrix(ThePoint );

	if(ThePoint.Z > 0 ) {
		// now we have to do our perspective divides
		ThePoint.X = ((mXScale * ThePoint.X)
			/ ThePoint.Z) + mXOrigin;

		ThePoint.Y = ((mYScale * ThePoint.Y)
				/ ThePoint.Z) + mYOrigin;
	} 
	else {
		// off the viewport... this way they won't draw
		ThePoint.X = -32;
		ThePoint.Y = -32;
	}
}

// This function will rotate the camera by the following
// in 360 degree system.  This is stupid and simple.
void CRenderCamera::RotateCamera(double Yaw, double Pitch, double Roll)
{
	mCameraDirection.X += Yaw;
	mCameraDirection.Y += Pitch;
	mCameraDirection.Z += Roll;
}

void CRenderCamera::RotateCameraWithLimits(double Yaw, double Pitch, double Roll)
{
	// no limit on Yaw.
	mCameraDirection.X += Yaw;

	// limit our pitch to +- 90 degress
	mCameraDirection.Y += Pitch;
	if(mCameraDirection.Y > (M_TOTAL_DEGREES/4) )
		mCameraDirection.Y = (M_TOTAL_DEGREES/4);
	if(mCameraDirection.Y < (-M_TOTAL_DEGREES/4) )
		mCameraDirection.Y = (-M_TOTAL_DEGREES/4);

	// no limit on roll (we don't even roll!!)
	mCameraDirection.Z += Roll;
}

#define DEFAULT_MOVEMENT_AMOUNT ((double)4)
//============================================================
// This function moves the camera in different ways
// depending upon the axis passed. Good for normal
// movement of the camera.
//============================================================
void CRenderCamera::MoveCamera(double X, double Y, double Z)
{
				   
	// just some temp crap to get radians
	double Yaw, PerpYaw;
	Yaw		=   ((mCameraDirection.X * (double)M_PI_VALUE) / (double)(M_TOTAL_DEGREES/2));
	PerpYaw =   (((mCameraDirection.X+M_TOTAL_DEGREES/4) * (double)M_PI_VALUE) / (double)(M_TOTAL_DEGREES/2));

	// do the Z movement
	mCameraPosition.X -= sin(Yaw) * Z * DEFAULT_MOVEMENT_AMOUNT;
	mCameraPosition.Z += cos(Yaw) * Z * DEFAULT_MOVEMENT_AMOUNT;

	// do the Y movement
	mCameraPosition.Y += Y * DEFAULT_MOVEMENT_AMOUNT;

	// do the X movement
	mCameraPosition.X -= sin(PerpYaw) * X * DEFAULT_MOVEMENT_AMOUNT;
	mCameraPosition.Z += cos(PerpYaw) * X * DEFAULT_MOVEMENT_AMOUNT;

}

//=================================================================
// This function sets up all of our camera constants etc.
//=================================================================
void CRenderCamera::SetupCamera(int ClipMinX, int ClipMaxX, int ClipMinY, int ClipMaxY)//, RGBQUAD PaletteQuad[256])
{
	// make sure that our size is nice
	ClipMaxX = (ClipMaxX + 3) & ~3;

	// Assign the clipping coordinates
	mClip.left = ClipMinX;
	mClip.right = ClipMaxX;
	mClip.top = ClipMinY;
	mClip.bottom = ClipMaxY;

	// Get the render view width/height
	mViewWidth = ClipMaxX - ClipMinX;
	mViewHeight = ClipMaxY - ClipMinY;

	// Get the origin of the view
	mXOrigin = ClipMinX + (mViewWidth / 2);
	mYOrigin = ClipMinY + (mViewHeight / 2);

	// Set the angle of vision in RADIANS
	//mAngleOfVision = (((AngleYouWant) * (double)M_PI_VALUE) / (double)(M_TOTAL_DEGREES/2));
	mAngleOfVision = (((M_TOTAL_DEGREES/4) * (double)M_PI_VALUE) / (double)(M_TOTAL_DEGREES/2));

	// Setup the scaling factors
	mXScale = (mViewWidth/2) * cos(mAngleOfVision/2) / sin(mAngleOfVision/2);
	//mYScale = -(mViewHeight/2) * cos(mAngleOfVision/2) / sin(mAngleOfVision/2);
	mYScale = -mXScale;
}

void CRenderCamera::MoveCameraAbsolute(double X, double Y, double Z)
{
	mCameraPosition.X += X;//*2;
	mCameraPosition.Y += Y;//*2;
	mCameraPosition.Z += Z;//*2;
}

void CRenderCamera::PositionCamera(double X, double Y, double Z)
{
	mCameraPosition.X = X;
	mCameraPosition.Y = Y;
	mCameraPosition.Z = Z;
}

void CRenderCamera::DrawWireframePolygon(ThredPolygon* Polygon, COLORREF LineColour, COLORREF PointColour, TSRenderContext* rc, bool show_cornerpoints, bool show_midpoints)
{
	// some points
	ThredPoint    TranPoint;
	//ThredPolygon  Poly;
	int           CurrentVert;
	//TMat3F        imat;
    GFXSurface*   pBuffer = rc->getSurface();

	// DPW - Needed for TS compatibility
	int			            tmp_loc, i;
	RectI		            vertex_rect;
	//Point2I	                **poly_2d = &(poly_point_2d[0]);
	Point3F	                in_point;
    TS::VertexIndexPair     pairs[100];
    Point2F                 texture_indices[100];
    Point2I                 start, end;

    // DPW - Needed for hither clipping
    int     next_avail, next_one;
    int     n;

    TSCamera* ts_cam = rc->getCamera();
    rc->getPointArray()->useTextures(texture_indices);

	switch(mViewType )
    {
	  case ID_VIEW_SOLIDRENDER:
	  case ID_VIEW_TEXTUREVIEW:
	  case ID_VIEW_3DWIREFRAME:

        next_avail = 0;

        // Loop thru all the vertices...
        for (i = 0; i < Polygon->NumberOfPoints; i++)
        {
          rc->getPointArray()->reset();
          next_one = (i+1)%Polygon->NumberOfPoints;
          n = 0;

          in_point.x = Polygon->Points[i].X-1.F;
          in_point.y = Polygon->Points[i].Z;//+1.F;
          in_point.z = Polygon->Points[i].Y;//-1.F;
          pairs[n].fVertexIndex = rc->getPointArray()->addPoint(in_point);
          pairs[n].fTextureIndex = n;
          texture_indices[n].x = 0.0;
          texture_indices[n].y = 0.0;
          n++;

          in_point.x = Polygon->Points[next_one].X-1.F;
          in_point.y = Polygon->Points[next_one].Z;//+1.F;
          in_point.z = Polygon->Points[next_one].Y;//+1.F;
          pairs[n].fVertexIndex = rc->getPointArray()->addPoint(in_point);
          pairs[n].fTextureIndex = n;
          texture_indices[n].x = 0.0;
          texture_indices[n].y = 0.0;
          n++;

          in_point.x = Polygon->Points[next_one].X+1.F;
          in_point.y = Polygon->Points[next_one].Z;//-1.F;
          in_point.z = Polygon->Points[next_one].Y;//+1.F;
          pairs[n].fVertexIndex = rc->getPointArray()->addPoint(in_point);
          pairs[n].fTextureIndex = n;
          texture_indices[n].x = 0.0;
          texture_indices[n].y = 0.0;
          n++;

          in_point.x = Polygon->Points[i].X+1.F;
          in_point.y = Polygon->Points[i].Z;//-1.F;
          in_point.z = Polygon->Points[i].Y;//+1.F;
          pairs[n].fVertexIndex = rc->getPointArray()->addPoint(in_point);
          pairs[n].fTextureIndex = n;
          texture_indices[n].x = 0.0;
          texture_indices[n].y = 0.0;
          n++;

          // 2
          in_point.x = Polygon->Points[i].X;//-1.F;
          in_point.y = Polygon->Points[i].Z-1.F;
          in_point.z = Polygon->Points[i].Y;//+1.F;
          pairs[n].fVertexIndex = rc->getPointArray()->addPoint(in_point);
          pairs[n].fTextureIndex = n;
          texture_indices[n].x = 0.0;
          texture_indices[n].y = 0.0;
          n++;

          in_point.x = Polygon->Points[next_one].X;//-1.F;
          in_point.y = Polygon->Points[next_one].Z-1.F;
          in_point.z = Polygon->Points[next_one].Y;//+1.F;
          pairs[n].fVertexIndex = rc->getPointArray()->addPoint(in_point);
          pairs[n].fTextureIndex = n;
          texture_indices[n].x = 0.0;
          texture_indices[n].y = 0.0;
          n++;

          in_point.x = Polygon->Points[next_one].X;//+1.F;
          in_point.y = Polygon->Points[next_one].Z+1.F;
          in_point.z = Polygon->Points[next_one].Y;//+1.F;
          pairs[n].fVertexIndex = rc->getPointArray()->addPoint(in_point);
          pairs[n].fTextureIndex = n;
          texture_indices[n].x = 0.0;
          texture_indices[n].y = 0.0;
          n++;

          in_point.x = Polygon->Points[i].X;//+1.F;
          in_point.y = Polygon->Points[i].Z+1.F;
          in_point.z = Polygon->Points[i].Y;//+1.F;
          pairs[n].fVertexIndex = rc->getPointArray()->addPoint(in_point);
          pairs[n].fTextureIndex = n;
          texture_indices[n].x = 0.0;
          texture_indices[n].y = 0.0;
          n++;

          // 3
          in_point.x = Polygon->Points[i].X;//+1.F;
          in_point.y = Polygon->Points[i].Z;//-1.F;
          in_point.z = Polygon->Points[i].Y-1.F;
          pairs[n].fVertexIndex = rc->getPointArray()->addPoint(in_point);
          pairs[n].fTextureIndex = n;
          texture_indices[n].x = 0.0;
          texture_indices[n].y = 0.0;
          n++;

          in_point.x = Polygon->Points[next_one].X;//+1.F;
          in_point.y = Polygon->Points[next_one].Z;//-1.F;
          in_point.z = Polygon->Points[next_one].Y-1.F;
          pairs[n].fVertexIndex = rc->getPointArray()->addPoint(in_point);
          pairs[n].fTextureIndex = n;
          texture_indices[n].x = 0.0;
          texture_indices[n].y = 0.0;
          n++;

          in_point.x = Polygon->Points[next_one].X;//-1.F;
          in_point.y = Polygon->Points[next_one].Z;//+1.F;
          in_point.z = Polygon->Points[next_one].Y+1.F;
          pairs[n].fVertexIndex = rc->getPointArray()->addPoint(in_point);
          pairs[n].fTextureIndex = n;
          texture_indices[n].x = 0.0;
          texture_indices[n].y = 0.0;
          n++;

          in_point.x = Polygon->Points[i].X;//-1.F;
          in_point.y = Polygon->Points[i].Z;//+1.F;
          in_point.z = Polygon->Points[i].Y+1.F;
          pairs[n].fVertexIndex = rc->getPointArray()->addPoint(in_point);
          pairs[n].fTextureIndex = n;
          texture_indices[n].x = 0.0;
          texture_indices[n].y = 0.0;
          n++;
          pBuffer->setFillColor(LineColour);
          rc->getPointArray()->drawPoly(12, pairs, 0);
        }
		break;

	case ID_VIEW_TOPVIEW:
		// Draw poly from top
		for(CurrentVert = 0; CurrentVert < Polygon->NumberOfPoints; CurrentVert++ )
		{
            int next_vertex = (CurrentVert + 1) % Polygon->NumberOfPoints;

            TranPoint = Polygon->Points[CurrentVert];
            start.x = (int)((TranPoint.X - mCameraPosition.X) * mZoomFactor) + mXOrigin;
            start.y = (int)((mCameraPosition.Z - TranPoint.Z) * mZoomFactor) + mYOrigin;

            TranPoint = Polygon->Points[next_vertex];
            end.x = (int)((TranPoint.X - mCameraPosition.X) * mZoomFactor) + mXOrigin;
            end.y = (int)((mCameraPosition.Z - TranPoint.Z) * mZoomFactor) + mYOrigin;

            pBuffer->drawLine2d (&start, &end, LineColour);
        }

        pBuffer->setFillColor(PointColour);

		// Draw the vertex points
		for(CurrentVert = 0; CurrentVert < Polygon->NumberOfPoints; CurrentVert++ )
		{
          int next_vertex = (CurrentVert + 1) % Polygon->NumberOfPoints;

          if (show_cornerpoints)
          {
			TranPoint = Polygon->Points[CurrentVert];

			tmp_loc = (int)((TranPoint.X - mCameraPosition.X) * mZoomFactor) + mXOrigin-2;
			vertex_rect.upperL.x = (tmp_loc < 2) ? 0 : tmp_loc;
			tmp_loc = (int)((mCameraPosition.Z - TranPoint.Z) * mZoomFactor) + mYOrigin-2;
			vertex_rect.upperL.y = (tmp_loc < 2) ? 0 : tmp_loc;

			tmp_loc = (int)((TranPoint.X - mCameraPosition.X) * mZoomFactor) + mXOrigin+2;
			vertex_rect.lowerR.x = (tmp_loc > (mClip.BottomRight().x-2)) ? (mClip.BottomRight().x-2) : tmp_loc;
			tmp_loc = (int)((mCameraPosition.Z - TranPoint.Z) * mZoomFactor) + mYOrigin+2;
			vertex_rect.lowerR.y = (tmp_loc > (mClip.BottomRight().y-2)) ? (mClip.BottomRight().y-2) : tmp_loc;

			pBuffer->drawRect2d_f (&vertex_rect, PointColour);
          }

          // Draw center point along the edge...
          if (show_midpoints)
          {
			  TranPoint.X = (Polygon->Points[CurrentVert].X + Polygon->Points[next_vertex].X) / 2;
			  TranPoint.Y = (Polygon->Points[CurrentVert].Y + Polygon->Points[next_vertex].Y) / 2;
			  TranPoint.Z = (Polygon->Points[CurrentVert].Z + Polygon->Points[next_vertex].Z) / 2;

			  tmp_loc = (int)((TranPoint.X - mCameraPosition.X) * mZoomFactor) + mXOrigin-2;
			  vertex_rect.upperL.x = (tmp_loc < 2) ? 0 : tmp_loc;
			  tmp_loc = (int)((mCameraPosition.Z - TranPoint.Z) * mZoomFactor) + mYOrigin-2;
			  vertex_rect.upperL.y = (tmp_loc < 2) ? 0 : tmp_loc;

			  tmp_loc = (int)((TranPoint.X - mCameraPosition.X) * mZoomFactor) + mXOrigin+2;
			  vertex_rect.lowerR.x = (tmp_loc > (mClip.BottomRight().x-2)) ? (mClip.BottomRight().x-2) : tmp_loc;
			  tmp_loc = (int)((mCameraPosition.Z - TranPoint.Z) * mZoomFactor) + mYOrigin+2;
			  vertex_rect.lowerR.y = (tmp_loc > (mClip.BottomRight().y-2)) ? (mClip.BottomRight().y-2) : tmp_loc;

			  pBuffer->drawRect2d_f (&vertex_rect, PointColour);
          }
		}
		break;
	case ID_VIEW_FRONTVIEW:
		// Draw poly from front
		for(CurrentVert = 0; CurrentVert < Polygon->NumberOfPoints; CurrentVert++ )
		{
            int next_vertex = (CurrentVert + 1) % Polygon->NumberOfPoints;

            TranPoint = Polygon->Points[CurrentVert];
            start.x = (int)((TranPoint.X - mCameraPosition.X) * mZoomFactor) + mXOrigin;
            start.y = (int)((mCameraPosition.Y - TranPoint.Y) * mZoomFactor) + mYOrigin;

            TranPoint = Polygon->Points[next_vertex];
            end.x = (int)((TranPoint.X - mCameraPosition.X) * mZoomFactor) + mXOrigin;
            end.y = (int)((mCameraPosition.Y - TranPoint.Y) * mZoomFactor) + mYOrigin;

            pBuffer->drawLine2d (&start, &end, LineColour);
		}

        pBuffer->setFillColor(PointColour);

		// Draw the vertex points
		for(CurrentVert = 0; CurrentVert < Polygon->NumberOfPoints; CurrentVert++ )
		{
          int next_vertex = (CurrentVert + 1) % Polygon->NumberOfPoints;

          if (show_cornerpoints)
          {
            TranPoint = Polygon->Points[CurrentVert];

			tmp_loc = (int)((TranPoint.X - mCameraPosition.X) * mZoomFactor) + mXOrigin-2;
			vertex_rect.upperL.x = (tmp_loc < 2) ? 0 : tmp_loc;
			tmp_loc = (int)((mCameraPosition.Y - TranPoint.Y) * mZoomFactor) + mYOrigin-2;
			vertex_rect.upperL.y = (tmp_loc < 2) ? 0 : tmp_loc;

			tmp_loc = (int)((TranPoint.X - mCameraPosition.X) * mZoomFactor) + mXOrigin+2;
			vertex_rect.lowerR.x = (tmp_loc > (mClip.BottomRight().x-2)) ? (mClip.BottomRight().x-2) : tmp_loc;
			tmp_loc = (int)((mCameraPosition.Y - TranPoint.Y) * mZoomFactor) + mYOrigin+2;
			vertex_rect.lowerR.y = (tmp_loc > (mClip.BottomRight().y-2)) ? (mClip.BottomRight().y-2) : tmp_loc;

			pBuffer->drawRect2d_f (&vertex_rect, PointColour);
          }

          if (show_midpoints)
          {
            // Draw center point along the edge...
			TranPoint.X = (Polygon->Points[CurrentVert].X + Polygon->Points[next_vertex].X) / 2;
			TranPoint.Y = (Polygon->Points[CurrentVert].Y + Polygon->Points[next_vertex].Y) / 2;
			TranPoint.Z = (Polygon->Points[CurrentVert].Z + Polygon->Points[next_vertex].Z) / 2;

			tmp_loc = (int)((TranPoint.X - mCameraPosition.X) * mZoomFactor) + mXOrigin-2;
			vertex_rect.upperL.x = (tmp_loc < 2) ? 0 : tmp_loc;
			tmp_loc = (int)((mCameraPosition.Y - TranPoint.Y) * mZoomFactor) + mYOrigin-2;
			vertex_rect.upperL.y = (tmp_loc < 2) ? 0 : tmp_loc;

			tmp_loc = (int)((TranPoint.X - mCameraPosition.X) * mZoomFactor) + mXOrigin+2;
			vertex_rect.lowerR.x = (tmp_loc > (mClip.BottomRight().x-2)) ? (mClip.BottomRight().x-2) : tmp_loc;
			tmp_loc = (int)((mCameraPosition.Y - TranPoint.Y) * mZoomFactor) + mYOrigin+2;
			vertex_rect.lowerR.y = (tmp_loc > (mClip.BottomRight().y-2)) ? (mClip.BottomRight().y-2) : tmp_loc;

			pBuffer->drawRect2d_f (&vertex_rect, PointColour);
          }
		}
		break;
	case ID_VIEW_SIDEVIEW:
		// Draw poly from side
		for(CurrentVert = 0; CurrentVert < Polygon->NumberOfPoints; CurrentVert++ )
		{
            int next_vertex = (CurrentVert + 1) % Polygon->NumberOfPoints;

            TranPoint = Polygon->Points[CurrentVert];
            start.x = (int)((TranPoint.Z - mCameraPosition.Z) * mZoomFactor) + mXOrigin;
            start.y = (int)((mCameraPosition.Y - TranPoint.Y) * mZoomFactor) + mYOrigin;

            TranPoint = Polygon->Points[next_vertex];
            end.x = (int)((TranPoint.Z - mCameraPosition.Z) * mZoomFactor) + mXOrigin;
            end.y = (int)((mCameraPosition.Y - TranPoint.Y) * mZoomFactor) + mYOrigin;

            pBuffer->drawLine2d (&start, &end, LineColour);
		}

        pBuffer->setFillColor(PointColour);

		// Draw the vertex points
		for(CurrentVert = 0; CurrentVert < Polygon->NumberOfPoints; CurrentVert++ )
		{
          int next_vertex = (CurrentVert + 1) % Polygon->NumberOfPoints;

          if (show_cornerpoints)
          {
			TranPoint = Polygon->Points[CurrentVert];

			tmp_loc = (int)((TranPoint.Z - mCameraPosition.Z) * mZoomFactor) + mXOrigin-2;
			vertex_rect.upperL.x = (tmp_loc < 2) ? 0 : tmp_loc;
			tmp_loc = (int)((mCameraPosition.Y - TranPoint.Y) * mZoomFactor) + mYOrigin-2;
			vertex_rect.upperL.y = (tmp_loc < 2) ? 0 : tmp_loc;

			tmp_loc = (int)((TranPoint.Z - mCameraPosition.Z) * mZoomFactor) + mXOrigin+2;
			vertex_rect.lowerR.x = (tmp_loc > (mClip.BottomRight().x-2)) ? (mClip.BottomRight().x-2) : tmp_loc;
			tmp_loc = (int)((mCameraPosition.Y - TranPoint.Y) * mZoomFactor) + mYOrigin+2;
			vertex_rect.lowerR.y = (tmp_loc > (mClip.BottomRight().y-2)) ? (mClip.BottomRight().y-2) : tmp_loc;

			pBuffer->drawRect2d_f (&vertex_rect, PointColour);
          }

          if (show_midpoints)
          {
            // Draw center point along the edge...
			TranPoint.X = (Polygon->Points[CurrentVert].X + Polygon->Points[next_vertex].X) / 2;
			TranPoint.Y = (Polygon->Points[CurrentVert].Y + Polygon->Points[next_vertex].Y) / 2;
			TranPoint.Z = (Polygon->Points[CurrentVert].Z + Polygon->Points[next_vertex].Z) / 2;

			tmp_loc = (int)((TranPoint.Z - mCameraPosition.Z) * mZoomFactor) + mXOrigin-2;
			vertex_rect.upperL.x = (tmp_loc < 2) ? 0 : tmp_loc;
			tmp_loc = (int)((mCameraPosition.Y - TranPoint.Y) * mZoomFactor) + mYOrigin-2;
			vertex_rect.upperL.y = (tmp_loc < 2) ? 0 : tmp_loc;

			tmp_loc = (int)((TranPoint.Z - mCameraPosition.Z) * mZoomFactor) + mXOrigin+2;
			vertex_rect.lowerR.x = (tmp_loc > (mClip.BottomRight().x-2)) ? (mClip.BottomRight().x-2) : tmp_loc;
			tmp_loc = (int)((mCameraPosition.Y - TranPoint.Y) * mZoomFactor) + mYOrigin+2;
			vertex_rect.lowerR.y = (tmp_loc > (mClip.BottomRight().y-2)) ? (mClip.BottomRight().y-2) : tmp_loc;

			pBuffer->drawRect2d_f (&vertex_rect, PointColour);
          }
		}
		break;
	}
}


void CRenderCamera::SetViewType (int ViewType)
{
  mViewType = ViewType;
}

void CRenderCamera::SetViewSpecifics(CDC* pDC, int ViewType, double ZoomFactor, double GridSize)
{
	m_pDC = pDC;
	mViewType = ViewType;
	mZoomFactor = ZoomFactor;
	mGridSize = GridSize;
}


void CRenderCamera::TranslateToScreen(CPoint& Point, ThredPoint TranPoint, TSRenderContext* rc)
{
    TSCamera* ts_cam = rc->getCamera();
	TMat3F imat;
    TS::TransformedVertex	outpoint;
	Point3F	inpoint (TranPoint.X, TranPoint.Z, TranPoint.Y);

	switch(mViewType ) {
	case ID_VIEW_SOLIDRENDER:
	case ID_VIEW_TEXTUREVIEW:
	case ID_VIEW_3DWIREFRAME:
			ts_cam->transformProject(inpoint, &outpoint);
			Point.x = outpoint.fPoint.x;
			Point.y = outpoint.fPoint.y;
		return;
	case ID_VIEW_TOPVIEW:
			Point.x = (int)((TranPoint.X - mCameraPosition.X) * mZoomFactor) + mXOrigin;
			Point.y = (int)((mCameraPosition.Z - TranPoint.Z) * mZoomFactor) + mYOrigin;
		return;
	case ID_VIEW_FRONTVIEW:
			Point.x = (int)((TranPoint.X - mCameraPosition.X) * mZoomFactor) + mXOrigin;
			Point.y = (int)((mCameraPosition.Y - TranPoint.Y) * mZoomFactor) + mYOrigin;
		break;
	case ID_VIEW_SIDEVIEW:
			Point.x = (int)((TranPoint.Z - mCameraPosition.Z) * mZoomFactor) + mXOrigin;
			Point.y = (int)((mCameraPosition.Y - TranPoint.Y) * mZoomFactor) + mYOrigin;
		break;
	}
}

void CRenderCamera::DrawDiamond( ThredPoint & origin, float offset, COLORREF Colour, TSRenderContext* rc )
{
   Point3F pnt;
   pnt.x = origin.X;
   pnt.y = origin.Z;
   pnt.z = origin.Y;
   
   GFXSurface * surface;
   TSPointArray * pointArray;

   surface = rc->getSurface();
   surface->setHazeSource(GFX_HAZE_NONE);
   surface->setShadeSource(GFX_SHADE_NONE);
   surface->setAlphaSource(GFX_ALPHA_NONE);
   surface->setFillMode(GFX_FILL_CONSTANT);
   surface->setTexturePerspective(FALSE);
   surface->setTransparency(FALSE);
   surface->setFillColor( Colour );

   pointArray = rc->getPointArray();
   pointArray->reset();
   pointArray->useIntensities(false);
   pointArray->useTextures(false);
   pointArray->useHazes(false);
   pointArray->useBackFaceTest(false);
   pointArray->setVisibility( TS::ClipMask );

   // draw a diamond
   TS::VertexIndexPair pairs[3];
   
   Point3F a( pnt.x - offset, pnt.y, pnt.z );
   Point3F b( pnt.x, pnt.y - offset, pnt.z );
   Point3F c( pnt.x + offset, pnt.y, pnt.z );
   Point3F d( pnt.x, pnt.y + offset, pnt.z );
   Point3F e( pnt.x, pnt.y, pnt.z - offset );
   Point3F f( pnt.x, pnt.y, pnt.z + offset );
   
   // check for emission of too many pollies
   if( GFXMetrics.emittedPolys >= (MAX_POLYS-9) )
      return;
   
   // go through and draw all eight sides
   pairs[0].fVertexIndex = pointArray->addPoint(a);
   pairs[1].fVertexIndex = pointArray->addPoint(b);
   pairs[2].fVertexIndex = pointArray->addPoint(e);
   pointArray->drawPoly( 3, pairs, 0 );

   pairs[0].fVertexIndex = pointArray->addPoint(b);
   pairs[1].fVertexIndex = pointArray->addPoint(c);
   pairs[2].fVertexIndex = pointArray->addPoint(e);
   pointArray->drawPoly( 3, pairs, 0 );

   pairs[0].fVertexIndex = pointArray->addPoint(c);
   pairs[1].fVertexIndex = pointArray->addPoint(d);
   pairs[2].fVertexIndex = pointArray->addPoint(e);
   pointArray->drawPoly( 3, pairs, 0 );

   pairs[0].fVertexIndex = pointArray->addPoint(d);
   pairs[1].fVertexIndex = pointArray->addPoint(a);
   pairs[2].fVertexIndex = pointArray->addPoint(e);
   pointArray->drawPoly( 3, pairs, 0 );

   pairs[0].fVertexIndex = pointArray->addPoint(f);
   pairs[1].fVertexIndex = pointArray->addPoint(b);
   pairs[2].fVertexIndex = pointArray->addPoint(a);
   pointArray->drawPoly( 3, pairs, 0 );

   pairs[0].fVertexIndex = pointArray->addPoint(f);
   pairs[1].fVertexIndex = pointArray->addPoint(c);
   pairs[2].fVertexIndex = pointArray->addPoint(b);
   pointArray->drawPoly( 3, pairs, 0 );

   pairs[0].fVertexIndex = pointArray->addPoint(f);
   pairs[1].fVertexIndex = pointArray->addPoint(d);
   pairs[2].fVertexIndex = pointArray->addPoint(c);
   pointArray->drawPoly( 3, pairs, 0 );

   pairs[0].fVertexIndex = pointArray->addPoint(f);
   pairs[1].fVertexIndex = pointArray->addPoint(a);
   pairs[2].fVertexIndex = pointArray->addPoint(d);
   pointArray->drawPoly( 3, pairs, 0 );
}

void CRenderCamera::DrawTri( ThredPoint & origin, float offset, COLORREF Colour, TSRenderContext* rc )
{
   Point3F pnt;
   pnt.x = origin.X;
   pnt.y = origin.Z;
   pnt.z = origin.Y;
   
   GFXSurface * surface;
   TSPointArray * pointArray;

   surface = rc->getSurface();
   surface->setHazeSource(GFX_HAZE_NONE);
   surface->setShadeSource(GFX_SHADE_NONE);
   surface->setAlphaSource(GFX_ALPHA_NONE);
   surface->setFillMode(GFX_FILL_CONSTANT);
   surface->setTexturePerspective(FALSE);
   surface->setTransparency(FALSE);
   surface->setFillColor( Colour );

   pointArray = rc->getPointArray();
   pointArray->reset();
   pointArray->useIntensities(false);
   pointArray->useTextures(false);
   pointArray->useHazes(false);
   pointArray->useBackFaceTest(false);
   pointArray->setVisibility( TS::ClipMask );

   // draw a diamond
   TS::VertexIndexPair pairs[3];
   
   Point3F a( pnt.x + offset, pnt.y, pnt.z - (offset / 2) );
   Point3F b( pnt.x + (offset * -0.5f) , pnt.y + (offset * 0.866025), pnt.z - (offset / 2 ) );
   Point3F c( pnt.x + (offset * -0.5f) , pnt.y + (offset * -0.866025), pnt.z - (offset / 2 ) );
   Point3F d( pnt.x, pnt.y, pnt.z + (offset / 2) );
   
   // check for emission of too many pollies
   if( GFXMetrics.emittedPolys >= (MAX_POLYS-5) )
      return;
   
   // go through and draw all eight sides
   pairs[0].fVertexIndex = pointArray->addPoint(d);
   pairs[1].fVertexIndex = pointArray->addPoint(b);
   pairs[2].fVertexIndex = pointArray->addPoint(a);
   pointArray->drawPoly( 3, pairs, 0 );

   pairs[0].fVertexIndex = pointArray->addPoint(d);
   pairs[1].fVertexIndex = pointArray->addPoint(c);
   pairs[2].fVertexIndex = pointArray->addPoint(b);
   pointArray->drawPoly( 3, pairs, 0 );

   pairs[0].fVertexIndex = pointArray->addPoint(d);
   pairs[1].fVertexIndex = pointArray->addPoint(a);
   pairs[2].fVertexIndex = pointArray->addPoint(c);
   pointArray->drawPoly( 3, pairs, 0 );

   pairs[0].fVertexIndex = pointArray->addPoint(c);
   pairs[1].fVertexIndex = pointArray->addPoint(a);
   pairs[2].fVertexIndex = pointArray->addPoint(b);
   pointArray->drawPoly( 3, pairs, 0 );
}

void CRenderCamera::DrawCone( ThredPoint & origin, ThredPoint & direction, float length, double angle, COLORREF Colour, TSRenderContext* rc )
{
   GFXSurface * surface;
   TSPointArray * pointArray;

   surface = rc->getSurface();
   surface->setHazeSource(GFX_HAZE_NONE);
   surface->setShadeSource(GFX_SHADE_NONE);
   surface->setAlphaSource(GFX_ALPHA_NONE);
   surface->setFillMode(GFX_FILL_CONSTANT);
   surface->setTexturePerspective(FALSE);
   surface->setTransparency(FALSE);
   surface->setFillColor( Colour );

   pointArray = rc->getPointArray();
   pointArray->reset();
   pointArray->useIntensities(false);
   pointArray->useTextures(false);
   pointArray->useHazes(false);
   pointArray->useBackFaceTest(false);
   pointArray->setVisibility( TS::ClipMask );
   
   const int numSides = 10;
   
   Point3F pnts[ numSides + 1 ];
   
   double seg = ( 2 * M_PI_VALUE ) / numSides;

   double radius = sin( angle ) * length;
      
   // translate each point
   for( int i = 0; i < numSides; i++ )
   {
      // get theta
      double theta = seg * i;
      
      pnts[i].x = origin.X + ( radius * cos( theta ) );
      pnts[i].y = origin.Y + ( radius * sin( theta ) );
      pnts[i].z = origin.Z + length;
   }
   
   // set the last point
   pnts[numSides].x = origin.X;
   pnts[numSides].y = origin.Y;
   pnts[numSides].z = origin.Z;
   
   TS::VertexIndexPair pairs[numSides];

   // draw all the sides
   for( int j = 0; j < ( numSides - 1 ); j++ )
   {
      pairs[0].fVertexIndex = pointArray->addPoint( pnts[j] );
      pairs[1].fVertexIndex = pointArray->addPoint( pnts[j+1] );
      pairs[2].fVertexIndex = pointArray->addPoint( pnts[numSides] );
      pointArray->drawPoly( 3, pairs, 0 );
   }
   
   // draw the last one
   pairs[0].fVertexIndex = pointArray->addPoint( pnts[ numSides - 1 ] );
   pairs[1].fVertexIndex = pointArray->addPoint( pnts[ 0 ] );
   pairs[2].fVertexIndex = pointArray->addPoint( pnts[numSides] );
   pointArray->drawPoly( 3, pairs, 0 );
   
   // draw the top
   for( int k = 0; k < numSides; k++ )
      pairs[k].fVertexIndex = pointArray->addPoint( pnts[numSides - k - 1] );
   pointArray->drawPoly( numSides, pairs, 0 );
}


void CRenderCamera::DrawPoint(CPoint& Point, COLORREF Colour, TSRenderContext* rc)
{
	// DPW - Needed for TS compatibility
	RectI	vertex_rect;

	vertex_rect.upperL.x = (Point.x < 2) ? 0: Point.x-2;
	vertex_rect.upperL.y = (Point.y < 2) ? 0: Point.y-2;
	vertex_rect.lowerR.x = (Point.x > (mClip.BottomRight().x-2)) ? (mClip.BottomRight().x-2) : Point.x+2;
	vertex_rect.lowerR.y = (Point.y > (mClip.BottomRight().y-2)) ? (mClip.BottomRight().y-2) : Point.y+2;

    rc->getSurface()->setFillColor(Colour);
	rc->getSurface()->drawRect2d_f(&vertex_rect, Colour);
}

void CRenderCamera::DrawOrthoGrid(int gridtype, int line_grid_color, int dot_grid_color, int origin_color, TSRenderContext* rc)
{
  ThredPoint  Position;
  double      StartX, StartY;
  Point2I     line_start, line_end;
  
  if (gridtype == USE_NO_GRID)
      return;

  switch(mViewType)
  {
	case ID_VIEW_3DWIREFRAME:
	case ID_VIEW_SOLIDRENDER:
	case ID_VIEW_TEXTUREVIEW:
		return;
	case ID_VIEW_TOPVIEW:
		Position = mCameraPosition;
		Position.Y = Position.Z;  // DPW
		break;
	case ID_VIEW_FRONTVIEW:
		Position = mCameraPosition;
		// make the Y the Z
		Position.Z = Position.Y;
		break;
	case ID_VIEW_SIDEVIEW:
		Position = mCameraPosition;
		// make the X the Z
		Position.X = Position.Z;
		Position.Z = Position.Y;
		break;
  }

  GFXSurface* pBuffer = rc->getSurface();
  
  // Get our client rectangle man.
  CRect rect;
  (m_pDC->GetWindow())->GetClientRect(&rect);

  // Draw the line grid...
  if (gridtype == USE_LINE_GRID)
  {
    StartX = floor(Position.X / mGridSize) * mGridSize;
    
    line_start.x = line_end.x = ((StartX - Position.X) * mZoomFactor) + mXOrigin;
	line_start.y = 0;
	line_end.y = rect.bottom;

    while (line_start.x < rect.right)
    {
	  pBuffer->drawLine2d (&line_start, &line_end, line_grid_color);

      StartX += mGridSize;

      line_start.x = line_end.x = ((StartX - Position.X) * mZoomFactor) + mXOrigin;
    }

    StartX = floor(Position.X / mGridSize) * mGridSize;
    
    line_start.x = line_end.x = ((StartX - Position.X) * mZoomFactor) + mXOrigin;
	line_start.y = 0;
	line_end.y = rect.bottom;

    while (line_start.x > rect.left)
    {
	  pBuffer->drawLine2d (&line_start, &line_end, line_grid_color);

      StartX -= mGridSize;

      line_start.x = line_end.x = ((StartX - Position.X) * mZoomFactor) + mXOrigin;
    }

    StartY = floor(Position.Y / mGridSize) * mGridSize;
    
    line_start.y = line_end.y = ((Position.Y-StartY) * mZoomFactor) + mYOrigin;
	line_start.x = 0;
	line_end.x = rect.right;

    while (line_start.y < rect.bottom)
    {
	  pBuffer->drawLine2d (&line_start, &line_end, line_grid_color);

      StartY -= mGridSize;

      line_start.y = line_end.y = ((Position.Y-StartY) * mZoomFactor) + mYOrigin;
    }

    StartY = floor(Position.Y / mGridSize) * mGridSize;
    
    line_start.y = line_end.y = ((Position.Y-StartY) * mZoomFactor) + mYOrigin;
	line_start.x = 0;
	line_end.x = rect.right;

    while (line_start.y > rect.top)
    {
	  pBuffer->drawLine2d (&line_start, &line_end, line_grid_color);

      StartY += mGridSize;

      line_start.y = line_end.y = ((Position.Y-StartY) * mZoomFactor) + mYOrigin;
    }
  }

  if (gridtype == USE_DOT_GRID)
  {
    StartX = floor(Position.X / mGridSize) * mGridSize;

    line_start.x = line_end.x = ((StartX - Position.X) * mZoomFactor) + mXOrigin;

    while (line_start.x < rect.right)
    {
      StartY = floor(Position.Y / mGridSize) * mGridSize;
      line_start.y = ((Position.Y - StartY) * mZoomFactor) + mYOrigin;

      while (line_start.y > rect.top)
      {
	    pBuffer->drawPoint2d (&line_start, dot_grid_color);

        StartY += mGridSize;
        line_start.y = ((Position.Y - StartY) * mZoomFactor) + mYOrigin;
      }

      StartY = floor(Position.Y / mGridSize) * mGridSize;
      line_start.y = ((Position.Y - StartY) * mZoomFactor) + mYOrigin;
      while (line_start.y < rect.bottom)
      {
	    pBuffer->drawPoint2d (&line_start, dot_grid_color);

        StartY -= mGridSize;
        line_start.y = ((Position.Y - StartY) * mZoomFactor) + mYOrigin;
      }

      StartX += mGridSize;

      line_start.x = line_end.x = ((StartX - Position.X) * mZoomFactor) + mXOrigin;
    }

    StartX = floor(Position.X / mGridSize) * mGridSize;
    
    line_start.x = line_end.x = ((StartX - Position.X) * mZoomFactor) + mXOrigin;

    while (line_start.x > rect.left)
    {
      StartY = floor(Position.Y / mGridSize) * mGridSize;
      line_start.y = ((Position.Y - StartY) * mZoomFactor) + mYOrigin;

      while (line_start.y > rect.top)
      {
	    pBuffer->drawPoint2d (&line_start, dot_grid_color);

        StartY += mGridSize;
        line_start.y = ((Position.Y - StartY) * mZoomFactor) + mYOrigin;
      }

      StartY = floor(Position.Y / mGridSize) * mGridSize;
      line_start.y = ((Position.Y - StartY) * mZoomFactor) + mYOrigin;
      while (line_start.y < rect.bottom)
      {
	    pBuffer->drawPoint2d (&line_start, dot_grid_color);

        StartY -= mGridSize;
        line_start.y = ((Position.Y - StartY) * mZoomFactor) + mYOrigin;
      }

      StartX -= mGridSize;

      line_start.x = line_end.x = ((StartX - Position.X) * mZoomFactor) + mXOrigin;
    }
  }

  // Draw the origin
  line_start.x = line_end.x = (-Position.X * mZoomFactor) + mXOrigin;
  line_start.y = 0;
  line_end.y = rect.bottom;
  pBuffer->drawLine2d (&line_start, &line_end, origin_color);

  line_start.y = line_end.y = (Position.Y * mZoomFactor) + mYOrigin;
  line_start.x = 0;
  line_end.x = rect.right;
  pBuffer->drawLine2d (&line_start, &line_end, origin_color);
}

#if 0
void CRenderCamera::DrawOrthoGrid(int gridtype, int line_grid_color, int dot_grid_color, int origin_color, TSRenderContext* rc)
{
	// DPW - Needed for TS compatibility
	Point2I	line_start, line_end;
    double curpos;
    int origindrawn;

	ThredPoint Position;
    double StartX, StartY, Step;
    double CurrentX, CurrentY;

    if (gridtype == USE_NO_GRID)
      return;

    GFXSurface* pBuffer = rc->getSurface();

	// We want to set our position depening upon our viewtype
	switch(mViewType) {
	case ID_VIEW_3DWIREFRAME:
	case ID_VIEW_TEXTUREVIEW:
	case ID_VIEW_SOLIDRENDER:
		return;
	case ID_VIEW_TOPVIEW:
		Position = mCameraPosition;
		Position.Y = Position.Z;  // DPW
		break;
	case ID_VIEW_FRONTVIEW:
		Position = mCameraPosition;
		// make the Y the Z
		Position.Z = Position.Y;
		break;
	case ID_VIEW_SIDEVIEW:
		Position = mCameraPosition;
		// make the X the Z
		Position.X = Position.Z;
		Position.Z = Position.Y;
		break;
	}

    // Get our client rectangle man.
	CRect rect;
	(m_pDC->GetWindow())->GetClientRect(&rect);

	if (gridtype == USE_DOT_GRID)
	{
#if 0
        origindrawn = 0;

        if ((Position.X < 0.0) && (Position.X > -mGridSize))
          curpos = mGridSize;
        else
          curpos = Position.X + mGridSize;
#endif

		StartX = mZoomFactor * (double)(mGridSize - ((int)Position.X % (int)mGridSize));
		Step   = mZoomFactor * mGridSize;

		// Draw points on grid - right of center
		for (CurrentX = (double)(rect.right/2)+StartX; CurrentX < rect.right; CurrentX += Step, curpos += mGridSize)
		{
			line_start.x = (int)(CurrentX);

			// Draw points on grid - below center
			StartY = mZoomFactor * (double)((int)Position.Y % (int)mGridSize);
			for (CurrentY = (double)(rect.bottom/2)+StartY; CurrentY < rect.right; CurrentY += Step)
			{
				line_start.y = (int)(CurrentY);
				pBuffer->drawPoint2d (&line_start, dot_grid_color);
			}

			// Draw points on grid - above center
			StartY = mZoomFactor * (double)(mGridSize - ((int)Position.Y % (int)mGridSize));
			for (CurrentY = (double)(rect.bottom/2)-StartY; CurrentY > 0; CurrentY -= Step)
			{
				line_start.y = (int)(CurrentY);
				pBuffer->drawPoint2d (&line_start, dot_grid_color);
			}
		}

		// Draw points on grid - left of center
		StartX = mZoomFactor * (double)((int)Position.X % (int)mGridSize);

		for (CurrentX = (double)(rect.right/2)-StartX; CurrentX > 0; CurrentX -= Step)
		{
			line_start.x = (int)(CurrentX);

			// Draw points on grid - below center
			StartY = mZoomFactor * (double)((int)Position.Y % (int)mGridSize);
			for (CurrentY = (double)(rect.bottom/2)+StartY; CurrentY < rect.right; CurrentY += Step)
			{
				line_start.y = (int)(CurrentY);
				pBuffer->drawPoint2d (&line_start, dot_grid_color);
			}

			// Draw points on grid - above center
			StartY = mZoomFactor * (double)(mGridSize - ((int)Position.Y % (int)mGridSize));
			for (CurrentY = (double)(rect.bottom/2)-StartY; CurrentY > 0; CurrentY -= Step)
			{
				line_start.y = (int)(CurrentY);
				pBuffer->drawPoint2d (&line_start, dot_grid_color);
			}
		}
	}

    if (gridtype == USE_ONLY_ORIGIN)
    {
      origindrawn = 0;

      if ((Position.X < 0.0) && (Position.X > -mGridSize))
        curpos = mGridSize;
      else
        curpos = Position.X + mGridSize;

		StartX = mZoomFactor * (double)(mGridSize - ((int)Position.X % (int)mGridSize));
		Step   = mZoomFactor * mGridSize;

		// Draw vertical lines on grid - right of center
		for (CurrentX = (double)(rect.right/2)+StartX; CurrentX < rect.right; CurrentX += Step, curpos += mGridSize)
		{
		    // DPW - Needed for TS compatibility
			line_start.x = (int)(CurrentX);
			line_start.y = 0;
			line_end.x = (int)(CurrentX);
			line_end.y = rect.bottom;

            if ((!origindrawn) &&
                ((((curpos) < 0.0) && ((curpos) > -mGridSize)) ||
                (((curpos) >= 0.0) && ((curpos) < mGridSize))))
            {
			  pBuffer->drawLine2d (&line_start, &line_end, origin_color);
              origindrawn = 1;
            }
		}

        if ((Position.X < 0.0) && (Position.X > -mGridSize))
          curpos = Position.X + mGridSize;
        else
          curpos = Position.X;

		StartX = mZoomFactor * (double)((int)Position.X % (int)mGridSize);

		// Draw vertical lines on grid - left of center
		for (CurrentX = (double)(rect.right/2)-StartX; CurrentX > 0; CurrentX -= Step, curpos -= mGridSize)
		{
		    // DPW - Needed for TS compatibility
			line_start.x = (int)(CurrentX);
			line_start.y = 0;
			line_end.x = (int)(CurrentX);
			line_end.y = rect.bottom;

            if ((curpos >= mGridSize) || (curpos < 0.0))
            {
			  //pBuffer->drawLine2d (&line_start, &line_end, line_grid_color);
            }
            else
            {
              if (!origindrawn)
              {
			    pBuffer->drawLine2d (&line_start, &line_end, origin_color);
              }
            }
		}

        origindrawn = 0;

        if ((Position.Y < 0.0) && (Position.Y > -mGridSize))
          curpos = mGridSize;
        else
          curpos = Position.Y + mGridSize;

		StartY = mZoomFactor * (double)(mGridSize - ((int)Position.Y % (int)mGridSize));

		// Draw horizontal lines on grid - above center
		for (CurrentY = (double)(rect.bottom/2)-StartY; CurrentY > 0; CurrentY -= Step, curpos += mGridSize)
		{
		    // DPW - Needed for TS compatibility
			line_start.x = 0;
			line_start.y = (int)(CurrentY);
			line_end.x = rect.right;
			line_end.y = (int)(CurrentY);

            if ((!origindrawn) &&
                ((((curpos) < 0.0) && ((curpos) > -mGridSize)) ||
                (((curpos) >= 0.0) && ((curpos) < mGridSize))))
            {
			  pBuffer->drawLine2d (&line_start, &line_end, origin_color);
              origindrawn = 1;
            }
		}

        if ((Position.Y < 0.0) && (Position.Y > -mGridSize))
          curpos = Position.Y + mGridSize;
        else
          curpos = Position.Y;

		StartY = mZoomFactor * (double)((int)Position.Y % (int)mGridSize);

        // Draw horizontal lines on grid - below center
		for (CurrentY = (double)(rect.bottom/2)+StartY; CurrentY < rect.right; CurrentY += Step, curpos -= mGridSize)
		{
		    // DPW - Needed for TS compatibility
			line_start.x = 0;
			line_start.y = (int)(CurrentY);
			line_end.x = rect.right;
			line_end.y = (int)(CurrentY);

            if ((curpos >= mGridSize) || (curpos < 0.0))
            {
			  //pBuffer->drawLine2d (&line_start, &line_end, line_grid_color);
            }
            else
            {
              if (!origindrawn)
              {
			    pBuffer->drawLine2d (&line_start, &line_end, origin_color);
              }
            }
		}
	}

	if (gridtype == USE_LINE_GRID)
	{
      origindrawn = 0;

      if ((Position.X < 0.0) && (Position.X > -mGridSize))
        curpos = mGridSize;
      else
        curpos = Position.X + mGridSize;

		StartX = mZoomFactor * (double)(mGridSize - ((int)Position.X % (int)mGridSize));
		Step   = mZoomFactor * mGridSize;

		// Draw vertical lines on grid - right of center
		for (CurrentX = (double)(rect.right/2)+StartX; CurrentX < rect.right; CurrentX += Step, curpos += mGridSize)
		{
		    // DPW - Needed for TS compatibility
			line_start.x = (int)(CurrentX);
			line_start.y = 0;
			line_end.x = (int)(CurrentX);
			line_end.y = rect.bottom;

            if ((!origindrawn) &&
                ((((curpos) < 0.0) && ((curpos) > -mGridSize)) ||
                (((curpos) >= 0.0) && ((curpos) < mGridSize))))
            {
			  pBuffer->drawLine2d (&line_start, &line_end, origin_color);
              origindrawn = 1;
            }
            else
            {
			  pBuffer->drawLine2d (&line_start, &line_end, line_grid_color);
            }
		}

        if ((Position.X < 0.0) && (Position.X > -mGridSize))
          curpos = Position.X + mGridSize;
        else
          curpos = Position.X;

		StartX = mZoomFactor * (double)((int)Position.X % (int)mGridSize);

		// Draw vertical lines on grid - left of center
		for (CurrentX = (double)(rect.right/2)-StartX; CurrentX > 0; CurrentX -= Step, curpos -= mGridSize)
		{
		    // DPW - Needed for TS compatibility
			line_start.x = (int)(CurrentX);
			line_start.y = 0;
			line_end.x = (int)(CurrentX);
			line_end.y = rect.bottom;

            if ((curpos >= mGridSize) || (curpos < 0.0))
            {
			  pBuffer->drawLine2d (&line_start, &line_end, line_grid_color);
            }
            else
            {
              if (!origindrawn)
              {
			    pBuffer->drawLine2d (&line_start, &line_end, origin_color);
              }
            }
		}

        origindrawn = 0;

        if ((Position.Y < 0.0) && (Position.Y > -mGridSize))
          curpos = mGridSize;
        else
          curpos = Position.Y + mGridSize;

		StartY = mZoomFactor * (double)(mGridSize - ((int)Position.Y % (int)mGridSize));

		// Draw horizontal lines on grid - above center
		for (CurrentY = (double)(rect.bottom/2)-StartY; CurrentY > 0; CurrentY -= Step, curpos += mGridSize)
		{
		    // DPW - Needed for TS compatibility
			line_start.x = 0;
			line_start.y = (int)(CurrentY);
			line_end.x = rect.right;
			line_end.y = (int)(CurrentY);

            if ((!origindrawn) &&
                ((((curpos) < 0.0) && ((curpos) > -mGridSize)) ||
                (((curpos) >= 0.0) && ((curpos) < mGridSize))))
            {
			  pBuffer->drawLine2d (&line_start, &line_end, origin_color);
              origindrawn = 1;
            }
            else
            {
			  pBuffer->drawLine2d (&line_start, &line_end, line_grid_color);
            }
		}

        if ((Position.Y < 0.0) && (Position.Y > -mGridSize))
          curpos = Position.Y + mGridSize;
        else
          curpos = Position.Y;

		StartY = mZoomFactor * (double)((int)Position.Y % (int)mGridSize);

        // Draw horizontal lines on grid - below center
		for (CurrentY = (double)(rect.bottom/2)+StartY; CurrentY < rect.right; CurrentY += Step, curpos -= mGridSize)
		{
		    // DPW - Needed for TS compatibility
			line_start.x = 0;
			line_start.y = (int)(CurrentY);
			line_end.x = rect.right;
			line_end.y = (int)(CurrentY);

            if ((curpos >= mGridSize) || (curpos < 0.0))
            {
			  pBuffer->drawLine2d (&line_start, &line_end, line_grid_color);
            }
            else
            {
              if (!origindrawn)
              {
			    pBuffer->drawLine2d (&line_start, &line_end, origin_color);
              }
            }
		}
	}
}
#endif

//==============================================================
// This function does the inverse transform from screen space
// to world space.
//==============================================================
void CRenderCamera::TranslateToWorld(CPoint Point, ThredPoint& TranPoint)
{
	// one of them is going to be zero
	TranPoint.X =
	TranPoint.Y =
	TranPoint.Z = 0;

	switch(mViewType ) {
	case ID_VIEW_SOLIDRENDER:
	case ID_VIEW_TEXTUREVIEW:
	case ID_VIEW_3DWIREFRAME:
		//DoPerspectiveTransform(TranPoint );
		return;
	case ID_VIEW_TOPVIEW:
			Point.x -= mXOrigin;
			TranPoint.X = (double)Point.x / mZoomFactor;
			TranPoint.X += mCameraPosition.X;

			Point.y -= mYOrigin;
			TranPoint.Z = (double)Point.y / mZoomFactor;
			TranPoint.Z = mCameraPosition.Z - TranPoint.Z;
		return;
	case ID_VIEW_FRONTVIEW:
			Point.x -= mXOrigin;
			TranPoint.X = (double)Point.x / mZoomFactor;
			TranPoint.X += mCameraPosition.X;

			Point.y -= mYOrigin;
			TranPoint.Y = (double)Point.y / mZoomFactor;
			TranPoint.Y = mCameraPosition.Y - TranPoint.Y;
		return;
	case ID_VIEW_SIDEVIEW:
			Point.x -= mXOrigin;
			TranPoint.Z = (double)Point.x / mZoomFactor;
			TranPoint.Z += mCameraPosition.Z;

			Point.y -= mYOrigin;
			TranPoint.Y = (double)Point.y / mZoomFactor;
			TranPoint.Y = mCameraPosition.Y - TranPoint.Y;
		return;
	}
}

void CRenderCamera::DrawRect(TSRenderContext* rc, CRect* srect, int color)
{
      RectI rectangle;

      if (srect->Width() < 0)
      {
        rectangle.lowerR.x = srect->TopLeft().x;
        rectangle.upperL.x = rectangle.lowerR.x + srect->Width();
      } else {
        rectangle.upperL.x = srect->TopLeft().x;
        rectangle.lowerR.x = rectangle.upperL.x + srect->Width();
      }

      if (srect->Height() < 0)
      {
        rectangle.lowerR.y = srect->TopLeft().y;
        rectangle.upperL.y = rectangle.lowerR.y + srect->Height();
      } else {
        rectangle.upperL.y = srect->TopLeft().y;
        rectangle.lowerR.y = rectangle.upperL.y + srect->Height();
      }

      //rc->getSurface()->setFillColor(color);
      rc->getSurface()->drawRect2d(&rectangle, color);
}

void CRenderCamera::DrawLine(Point2I* start, Point2I* end, int width, COLORREF Colour, TSRenderContext* rc)
{
	Point2I	newstart, newend;
    GFXSurface* pBuffer = rc->getSurface();

	if (width < 1)
		return;

	if (width > 1)
	{
		int half = width/2;

		newstart.x = start->x;
		newstart.y = start->y;
		newend.x = end->x;
		newend.y = end->y;

		if (abs(start->y - end->y) > abs(start->x - end->x))
		{
			// Verticalish line
			newstart.x -= half;
			newend.x -= half;

			for (int i = 0; i < width; i++)
			{
				pBuffer->drawLine2d (&newstart, &newend, Colour);
				newstart.x++;
				newend.x++;
			}
		} else {
			// Horizontalish line
			newstart.y -= half;
			newend.y -= half;

			for (int i = 0; i < width; i++)
			{
				pBuffer->drawLine2d (&newstart, &newend, Colour);
				newstart.y++;
				newend.y++;
			}
		}
	} else {
		pBuffer->drawLine2d (start, end, Colour);
	}
}

void CRenderCamera::DrawCircle( Point2I & center, int radius, COLORREF Colour, int numSegs, TSRenderContext* rc )
{
   // should draw something somewhat circle-like
   if( radius < 4 || numSegs < 3 )
      return;
       
   Point2F projPnt( radius, 0.f );
   Point2I start( center.x + radius, center.y );
   
   double rotAmt = 2 * M_PI / numSegs;
   double cosAmt = cos( rotAmt );
   double sinAmt = sin( rotAmt );
   
   // draw this as line segments.. the circle routine does not clip
   for( int i = 0; i < numSegs; i++ )
   {
      // rotate the projected point 1/32 of a circle
      Point2F tmp( projPnt );
      projPnt.x = tmp.x * cosAmt - tmp.y * sinAmt;
      projPnt.y = tmp.x * sinAmt + tmp.y * cosAmt;
      
      Point2I end( center.x + (int)projPnt.x, center.y + (int)projPnt.y );
      DrawLine( &start, &end, 1, Colour, rc );
      start = end;
   }
}

// DPW - a polygon splitting algorithm
bool CRenderCamera::Splitme (Point3F* point1, Point3F* point2, TSRenderContext* rc)
{
#if 0
  	Point3F camPos = ts_cam->getLoc();
	Point3F viewDirNormal, viewDirPt;
    Point3F vector1, vector2, vector3;
    double   t;
    double   denom, numer, dist1, dist2;

   	// get the hither plane info
	const TMat3F &mat = ts_cam->getTotalTransform();

	// This row of the matrix gives us a vector that lies along the camera's Y axis
	viewDirNormal.set(mat.m_0_1, mat.m_1_1, mat.m_2_1);
	viewDirNormal.normalize(); // should already be normalized, but...
  
	viewDirPt = viewDirNormal;
//    viewDirPt *= 5.0;
    // add in the camera position
	viewDirPt += ts_cam->getLoc();

    // Calculate distance from vertex1 to the plane point
    vector1.x = point1->x - viewDirPt.x;
    vector1.y = point1->y - viewDirPt.y;
    vector1.z = point1->z - viewDirPt.z;
    dist1 = numer = m_dot(viewDirNormal, vector1);

    // Calculate distance from vertex2 to the plane point
    vector2.x = point2->x - viewDirPt.x;
    vector2.y = point2->y - viewDirPt.y;
    vector2.z = point2->z - viewDirPt.z;
    dist2 = m_dot(viewDirNormal, vector2);

    // Calculate projection of the line onto the hither plane normal
    vector3.x = point2->x - point1->x;
    vector3.y = point2->y - point1->y;
    vector3.z = point2->z - point1->z;
    denom = m_dot(viewDirNormal, vector3);
    denom = -denom;

    // Calculate the intersection's parametric value
    if (denom != 0.0)
    {
      t = numer / denom;
    } else {
      if (dist1 <= 0.0)
        return false;
      else
        return true;
    }
      
    // ---------------------------
    if (dist1 <= 0.0)
    {
      // This point is behind the hither plane & invalid to draw as-is
      if ((dist2 > 0.0) && (t <= 1.0) && (t >= 0.0) )
      {
        // Gotta move dist1's vertex onto the hither plane
        point1->x += (t * vector3.x);
        point1->y += (t * vector3.y);
        point1->z += (t * vector3.z);

        // clipped dist1 -> dist2 is visible
        return true;

      } else {
        // Okay, fine, be that way!!!   ...Nothing's visible
        return false;
      }
    } else {
      // This point is in front of the hither plane & valid to draw as-is
      if ((dist2 <= 0.0) && (t <= 1.0) && (t >= 0.0) )
      {
        // Gotta move dist2's vertex onto the hither plane
        point2->x = point1->x + (t * vector3.x);
        point2->y = point1->y + (t * vector3.y);
        point2->z = point1->z + (t * vector3.z);
      }
        
      // Okay, dist1 -> (un)clipped dist2 is visible
      return true;
    }
    // ---------------------------
#endif
    return false;
}

