// THREDView.cpp : implementation of the CTHREDView class
//

#include "stdafx.h"
#include "THRED.h"

#include "THREDDoc.h"
#include "THREDView.h"
#include "MainFrm.h"
#include "AsgnText.h"
#include "splitter.h"
#include "fn_table.h"
#include "fn_all.h"
#include "gfxmetrics.h"
#include "g_cds.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static bool GotARasterList = FALSE;

//static int  busydrawing = 0;

#define DEFAULT_3D_X  0
#define DEFAULT_3D_Y  0
#define DEFAULT_3D_Z  -500

/////////////////////////////////////////////////////////////////////////////
// CTHREDView

IMPLEMENT_DYNCREATE(CTHREDView, CView)

BEGIN_MESSAGE_MAP(CTHREDView, CView)
	//{{AFX_MSG_MAP(CTHREDView)
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
   ON_WM_LBUTTONDBLCLK()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_TOOLS_BRUSH_RESETBRUSH, OnToolsBrushResetbrush)
	ON_COMMAND(ID_TOOLS_BRUSH_SHOWBRUSH, OnToolsBrushShowbrush)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_BRUSH_SHOWBRUSH, OnUpdateToolsBrushShowbrush)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_TOOLS_ENTITY, OnToolsEntity)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_ENTITY, OnUpdateToolsEntity)
	ON_UPDATE_COMMAND_UI(ID_BRUSH_GROUPS_MAKENEWGROUP, OnUpdateBrushGroupsMakenewgroup)
	ON_UPDATE_COMMAND_UI(ID_BRUSH_GROUPS_ADDTOGROUP, OnUpdateBrushGroupsAddtogroup)
	ON_COMMAND(ID_BRUSH_GROUPS_SELECT, OnBrushGroupsSelect)
	ON_UPDATE_COMMAND_UI(ID_BRUSH_GROUPS_SELECT, OnUpdateBrushGroupsSelect)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_TOOLS_BRUSH_MOVESELECTEDBRUSHES, OnToolsBrushMoveselectedbrushes)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_BRUSH_MOVESELECTEDBRUSHES, OnUpdateToolsBrushMoveselectedbrushes)
	ON_COMMAND(ID_TEXTURE_EDIT_FACE, OnTextureEditFace)
	ON_UPDATE_COMMAND_UI(ID_TEXTURE_EDIT_FACE, OnUpdateTextureEditFace)
	ON_COMMAND(ID_EDIT_MODE_BUTTON, OnEditModeButton)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MODE_BUTTON, OnUpdateEditModeButton)
	ON_WM_KEYUP()
	ON_COMMAND(ID_VIEW_CENTERCAMERAATORIGIN, OnViewCentercameraatorigin)
	ON_COMMAND(ID_TOOLS_CENTERCAM, OnToolsCentercam)
	ON_COMMAND(ID_VIEW_ZOOMIN, OnViewZoomin)
	ON_COMMAND(ID_VIEW_ZOOMOUT, OnViewZoomout)
	ON_WM_CREATE()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
   ON_WM_MOUSEWHEEL()
   ON_COMMAND( ID_VIEWBUTTON, OnViewButton )
	ON_COMMAND_RANGE(ID_VIEW_3DWIREFRAME, ID_VIEW_TEXTUREVIEW, OnViewType)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_3DWIREFRAME, ID_VIEW_TEXTUREVIEW, OnViewTypeCmdUi)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTHREDView construction/destruction

CTHREDView::CTHREDView()
{
//    busydrawing = 0;
   pFrame = NULL;

	// TODO: add construction code here
	//mViewAxis = (AXIS_X|AXIS_Y|AXIS_Z);
	mViewType = ID_VIEW_SOLIDRENDER;

	// DPW - Changed to zoom in closer initially
	mZoomFactor = 0.4;

	// DPW - Added for TS compatibility
	m_pTSCamera  = NULL;
	pBuffer = NULL;
	nearPlane = 1.0f;
	farPlane = 1.0E4f;

   m_renderContext = NULL;

   ShiftIsDown = FALSE;
   CtrlIsDown = FALSE;
   AltIsDown = FALSE;
   TextureEditing = FALSE;
    
   //rclip_table.fnDraw3DBegin = GFXSort3dBegin;
   rclip_table.fnDraw3DEnd = GFXSort3dEnd;
   rclip_table.fnEmitPoly = GFXSortEmitPoly;

   
   // DPW - for doing the drag rectangle for brush selection
   selectRect.SetRect(0, 0, 0, 0);
   showSelectRect = FALSE;
}

CTHREDView::~CTHREDView()
{
   if (m_renderContext)
   {
      delete m_renderContext;
      m_renderContext = NULL;
   }

   if (m_pTSCamera)
   {
      delete m_pTSCamera;
      m_pTSCamera = NULL;
   }

   if (pBuffer)
   {
      if (GotARasterList)
      {
         //pBuffer->disposeRasterList();
         GotARasterList = FALSE;
      }
      delete pBuffer;
      pBuffer = NULL;
   }
}

BOOL CTHREDView::PreCreateWindow(CREATESTRUCT& cs)
{
   cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW,
      theApp.LoadStandardCursor(IDC_ARROW), (HBRUSH)GetStockObject(BLACK_BRUSH)); 

   return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CTHREDView drawing

void CTHREDView::OnDraw(CDC* pDC)
{
   // check if we should be rendering at all ( never through commandline )
   if( theApp.mExportRun )
      return;
      
   // Busy drawing this view already?
   if (m_renderContext->isLocked())
      return;

   CTHREDDoc* pDoc = GetDocument();
   
   // set the info for this view
   if( pFrame->pCurrentDetail != pDoc->pActiveDetail )
   {
      // swap out the active info temp.
      pDoc->pActiveDetail->GetInfo();  
      pFrame->pCurrentDetail->SetInfo();
   }
   
   ASSERT_VALID(pDoc);
   RectI tmprect;

   // make sure the camera is ready to translate our stuff.
   mCamera.SetupMatrix();

   // make sure our camera knows wassup.
   mCamera.SetViewSpecifics(pDC, mViewType, mZoomFactor, pDoc->mGridSize);

   pBuffer->setHWND(m_hWnd);
   m_renderContext->setSurface(pBuffer);
   m_renderContext->setPointArray(&DefaultPointArray);
   m_renderContext->setCamera(m_pTSCamera);
   m_renderContext->setLights(&m_GSceneLights);

   pBuffer->getClientRect(&tmprect);

   pBuffer->setHazeSource(GFX_HAZE_NONE);
   pBuffer->setShadeSource(GFX_SHADE_NONE);
   pBuffer->setAlphaSource(GFX_ALPHA_NONE);
   pBuffer->setTransparency(FALSE);
   pDoc->GetMaterialList()->setDetailLevel(0);

   DefaultPointArray.reset();
   DefaultPointArray.useIntensities(FALSE);
   DefaultPointArray.setVisibility(TS::ClipMask);

   m_renderContext->lock();
   pBuffer->setZTest(GFX_ZWRITE);
   pBuffer->draw3DBegin();

   // Clear out the surface...
   pBuffer->clear(pDoc->background_color);
   pBuffer->setFillMode(GFX_FILL_CONSTANT);
   pBuffer->setFillColor(pDoc->background_color);

   float width = (float) pBuffer->getWidth();
   float height = (float) pBuffer->getHeight() - 1;

   pBuffer->addVertex(&Point3F(0, 0, 0), NULL, NULL, 0);
   pBuffer->addVertex(&Point3F(width, 0, 0), NULL, NULL, 0);
   pBuffer->addVertex(&Point3F(width, height, 0), NULL, NULL, 0);
   pBuffer->addVertex(&Point3F(0, height, 0), NULL, NULL, 0);

   pBuffer->emitPoly();

   pBuffer->setZTest(GFX_ZTEST_AND_WRITE);
   
   // figure out how to draw this world
   switch(mViewType)
   {
      case ID_VIEW_SOLIDRENDER:
         //pBuffer->setZTest(1);
         pBuffer->setFillMode(GFX_FILL_CONSTANT);
         pBuffer->setTexturePerspective(FALSE);
         DefaultPointArray.useTextures(false);

         pDoc->RenderSolidWorld(&mCamera, m_renderContext);
         break;
      case ID_VIEW_TEXTUREVIEW:
         //pBuffer->setZTest(1);
         pBuffer->setFillMode(GFX_FILL_TWOPASS);
         DefaultPointArray.useTextures(TRUE);
         pBuffer->setTexturePerspective(TRUE);
         pDoc->RenderWorld(&mCamera, m_renderContext);
         break;
      case ID_VIEW_3DWIREFRAME:
         //pBuffer->setZTest(1);
         pBuffer->setFillMode(GFX_FILL_CONSTANT);
         pBuffer->setTexturePerspective(FALSE);
         DefaultPointArray.useTextures(false);
         pDoc->RenderOrthoView(mViewType, &mCamera, m_pTSCamera, m_renderContext, FALSE, NULL);
         break;
      case ID_VIEW_TOPVIEW:
         pBuffer->setZTest(0);
         pBuffer->setFillMode(GFX_FILL_CONSTANT);
         pDoc->RenderOrthoView(mViewType, &mCamera, m_pTSCamera, m_renderContext, showSelectRect, &selectRect);
         break;
      case ID_VIEW_SIDEVIEW:
         pBuffer->setZTest(0);
         pBuffer->setFillMode(GFX_FILL_CONSTANT);
         pDoc->RenderOrthoView(mViewType, &mCamera, m_pTSCamera, m_renderContext, showSelectRect, &selectRect);
         break;
      case ID_VIEW_FRONTVIEW:
         pBuffer->setZTest(0);
         pBuffer->setFillMode(GFX_FILL_CONSTANT);
         pDoc->RenderOrthoView(mViewType, &mCamera, m_pTSCamera, m_renderContext, showSelectRect, &selectRect);
         break;
   }

   // draw the entities
   if(pDoc->mShowEntities)
   {
      pBuffer->setFillMode(GFX_FILL_TWOPASS);
      DefaultPointArray.useTextures(TRUE);
      pBuffer->setTexturePerspective(TRUE);
      pDoc->RenderEntities(&mCamera, mViewType, mZoomFactor, m_renderContext);
   }

   // draw the lights
   if( pDoc->mShowLights )
   {
      pBuffer->setFillMode(GFX_FILL_TWOPASS);
      DefaultPointArray.useTextures(TRUE);
      pBuffer->setTexturePerspective(TRUE);
      pDoc->RenderLights( &mCamera, mViewType, mZoomFactor, 
      m_renderContext);
   }

//#if 0//DPW -------------------------------------------------
   // Draw the brush
   if(pDoc->mShowBrush)
   {
      pBuffer->setFillMode(GFX_FILL_CONSTANT);
      pBuffer->setTexturePerspective(FALSE);
      DefaultPointArray.useTextures(false);
      pDoc->RenderBrush(mCamera, pDC, mViewType, mZoomFactor, m_renderContext, true);
   }
//#endif//DPW -------------------------------------------------

   // Draw the world & unlock the render context...
   if ((mViewType == ID_VIEW_TOPVIEW) ||
      (mViewType == ID_VIEW_SIDEVIEW) ||
      (mViewType == ID_VIEW_FRONTVIEW))
   {
      pBuffer->draw3DEnd();
      pBuffer->update();

      pDoc->SetSurfaceInfo(pBuffer);
      //pBuffer->setZTest(0);

      m_renderContext->unlock();
   } 
   else 
   {
      pBuffer->draw3DEnd();
      pBuffer->update();

      //pBuffer->setZTest(0);

      m_renderContext->unlock();

      // Write out # polys emitted to status bar...
      ((CMainFrame*)AfxGetMainWnd())->UpdateRenderInfo();
   }
#if 0
   //DPW ------------------------------------------------
   if ((mViewType == ID_VIEW_SOLIDRENDER) ||
      (mViewType == ID_VIEW_TEXTUREVIEW) ||
      (mViewType == ID_VIEW_3DWIREFRAME))
   {
      m_renderContext->lock();
      pBuffer->setZTest(2);
      pBuffer->draw3DBegin();

      // Draw the brush
      if(pDoc->mShowBrush)
      {
         pBuffer->setFillMode(GFX_FILL_CONSTANT);
         pBuffer->setTexturePerspective(FALSE);
         DefaultPointArray.useTextures(FALSE);
         pDoc->RenderBrush(mCamera, pDC, mViewType, mZoomFactor, m_renderContext, true);
      }

      pBuffer->draw3DEnd();
      pBuffer->update();                                            
      m_renderContext->unlock();
   }
   //DPW ------------------------------------------------
#endif

   ViewButton.RedrawWindow();
   
   // check if the active info needs to be updated
   if( pFrame->pCurrentDetail != pDoc->pActiveDetail )
      pDoc->pActiveDetail->SetInfo();  
}

/////////////////////////////////////////////////////////////////////////////
// CTHREDView diagnostics

#ifdef _DEBUG
void CTHREDView::AssertValid() const
{
   CView::AssertValid();
}

void CTHREDView::Dump(CDumpContext& dc) const
{
   CView::Dump(dc);
}

CTHREDDoc* CTHREDView::GetDocument() // non-debug version is inline
{
   ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTHREDDoc)));
   return (CTHREDDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTHREDView message handlers

void CTHREDView::OnSize(UINT nType, int cx, int cy) 
{
   // call our oldself
   CView::OnSize(nType, cx, cy);

   // check the size of the window  ( windows can make the with/height 0 )
   if( ( cx <= 0 ) || ( cy <= 0 ) )
      return;

   // make sure that our camera knows our current size
   mCamera.SetupCamera(0, cx, 0, cy);

   if (!m_renderContext)
      return;

   // Reallocate a GFXSurface for the new viewport size...
   ResizeBuffer(cx, cy);

   // Resize the TS camera:
   if (!m_pTSCamera)
      return;

   double max_dim = (cx > cy) ? (double)cx : (double)cy;

   m_pTSCamera->setWorldViewport(RectF(Point2F(-cx/max_dim, cy/max_dim),
      Point2F(cx/max_dim, -cy/max_dim)));

   m_pTSCamera->setScreenViewport(RectI(Point2I(0,0),Point2I(cx,cy)));
   m_pTSCamera->setNearDist(nearPlane);
   m_pTSCamera->setFarDist(farPlane);
}

void CTHREDView::ResizeBuffer(int cx, int cy)
{
   Point2I temp(0,0);

   CTHREDDoc* pDoc = GetDocument();
   ASSERT_VALID(pDoc);

   // GFXBuffers don't resize, so drop the one we have & get a new one...
   if (pBuffer)
   {
      delete pBuffer;
   }

   // Create a new GFXBuffer & assign its palette
   GFXCDSSurface::create(pBuffer, TRUE, cx, cy, m_hWnd, &temp);

   if (!GotARasterList)
   {
      pBuffer->createRasterList(3000);
      GotARasterList = TRUE;
   }

   pBuffer->setPalette(pDoc->GetPalette(), FALSE);
}

void CTHREDView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
   CTHREDDoc* pDoc = GetDocument();
   
   // check if should set the current detail info
   if( bActivate )
   {
      if( pDoc->DetailExist( pFrame->pCurrentDetail ) )
      {
         if( pFrame->pCurrentDetail != pDoc->pActiveDetail )
            pFrame->pCurrentDetail->SetActive();
      }
      else
         pFrame->pCurrentDetail = pDoc->pActiveDetail;
   }

   // set the titles for the windows
   pFrame->pCurrentDetail->UpdateTitles();
   
   if(bActivate )
      Globals.mAxis = GetDocument()->mViewAxis;

   // change the view button
   ViewButton.EnableWindow( bActivate );

   // set our title
   SetTitle();

   CView::OnActivateView(bActivate, pActivateView, pDeactiveView);

   // make sure the bar is updated for our doc.
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
   pFrame->LoadComboBox();

   if (bActivate)
   {
      if (((CTHREDDoc*)GetDocument())->ShouldGotoFourViews())
      {
         ((CMainFrame*)AfxGetMainWnd())->Show4Views();
      }
   }
}

void CTHREDView::OnToolsBrushShowbrush() 
{
   CTHREDDoc* pDoc = GetDocument();
   ASSERT_VALID(pDoc);

   // toggle brush
   pDoc->mShowBrush ^= 1;

   // redraw the screen
   pDoc->UpdateAllViews(NULL);
}

void CTHREDView::OnUpdateToolsBrushShowbrush(CCmdUI* pCmdUI) 
{
   CTHREDDoc* pDoc = GetDocument();
   ASSERT_VALID(pDoc);

   if(pDoc->mShowBrush )
      pCmdUI->SetCheck();
   else
      pCmdUI->SetCheck(0);
}

void CTHREDView::OnToolsBrushResetbrush() 
{
   CTHREDDoc* pDoc = GetDocument();
   ASSERT_VALID(pDoc);

   // Set the brush back to camera position
   pDoc->ResetBrush(mCamera.mCameraPosition);
   pDoc->UpdateAllViews(NULL);
}

BOOL CTHREDView::OnEraseBkgnd(CDC* pDC) 
{
   // if we are not solid erase it
   if(mViewType != ID_VIEW_SOLIDRENDER && mViewType != ID_VIEW_TEXTUREVIEW) 
   {
      return CView::OnEraseBkgnd(pDC);
   } 
   else 
   {
      // otherwise.... say that we erased it..
      return 1;
   }
}

// This is the range handler for the types of view that we have
// make sure when we add more view types that we update this.
void CTHREDView::OnViewType(UINT nID)
{
   CTHREDDoc* pDoc = GetDocument();

   mViewType = nID;
   SetTitle();

   RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);

}

void CTHREDView::OnViewTypeCmdUi(CCmdUI* pCmdUI)
{
   if(mViewType == pCmdUI->m_nID )
      pCmdUI->SetCheck();
   else
      pCmdUI->SetCheck(0);
}

void CTHREDView::SetTitle()
{
   switch(mViewType ) {
      case ID_VIEW_3DWIREFRAME:
         ViewButton.SetWindowText("3D Wireframe");
         break;
      case ID_VIEW_SOLIDRENDER:
         ViewButton.SetWindowText("Solid Render");
         break;
      case ID_VIEW_TEXTUREVIEW:
         ViewButton.SetWindowText("Texture View");
         break;
      case ID_VIEW_TOPVIEW:
         ViewButton.SetWindowText("Top View");
         break;
      case ID_VIEW_FRONTVIEW:
         ViewButton.SetWindowText("Front View");
         break;
      case ID_VIEW_SIDEVIEW:
         ViewButton.SetWindowText("Side View");
         break;
   }
}

// This is just a wrapper function if we ever want to
// move this to the document.  Right now we each camera
// can have a differnet tool selected.
int CTHREDView::GetTool(void)
{
   CTHREDDoc* pDoc = GetDocument();
   ASSERT_VALID(pDoc);
   return pDoc->mCurrentTool;
}

// This is just a wrapper function if we ever want to
// move this to the document.  Right now we each camera
// can have a differnet tool selected.
void CTHREDView::SetTool(int Tool)
{
   CTHREDDoc* pDoc = GetDocument();
   ASSERT_VALID(pDoc);
   pDoc->mCurrentTool = Tool;
}

void CTHREDView::OnToolsEntity() 
{
//   SetTool(ID_TOOLS_ENTITY );	
}

void CTHREDView::OnUpdateToolsEntity(CCmdUI* pCmdUI) 
{
   if(GetTool() == ID_TOOLS_ENTITY )
      pCmdUI->SetCheck();
   else
      pCmdUI->SetCheck(0);
}

void CTHREDView::OnBrushGroupsSelect() 
{
   SetTool(ID_BRUSH_GROUPS_SELECT);
}

void CTHREDView::OnUpdateBrushGroupsSelect(CCmdUI* pCmdUI) 
{
   if(GetTool() == ID_BRUSH_GROUPS_SELECT )
      pCmdUI->SetCheck();
   else
      pCmdUI->SetCheck(0);
}

#define ROTATION_MOVEMENT_MUL	(double)6
#define ZOOM_CHANGE	(double)0.0008
void CTHREDView::OnMouseMove(UINT nFlags, CPoint point) 
{
   ThredPoint  temppoint;
   int DeltaX, DeltaY;
   CPoint CursorPos;
   ThredPoint  WorldPoint;
   Point3F resize_delta (0.F, 0.F, 0.F);
   Point3F min_bound (0.F, 0.F, 0.F);
   Point3F max_bound (0.F, 0.F, 0.F);

   // get our document
   CTHREDDoc* pDoc = GetDocument();
   ASSERT_VALID(pDoc);

   if(this != GetCapture())
      return;

    if(GetAsyncKeyState(VK_MENU) & 0x8000)
    {
      AltIsDown = TRUE;
    } else {
      AltIsDown = FALSE;
    }

    if (nFlags & MK_SHIFT)
    {
      ShiftIsDown = TRUE;
    } else {
      ShiftIsDown = FALSE;
    }

    if (nFlags & MK_CONTROL)
    {
      CtrlIsDown = TRUE;
    } else {
      CtrlIsDown = FALSE;
    }

   // Calculate our deltas in case we need them
   GetCursorPos(&CursorPos);
   DeltaX = mStartPoint.x - CursorPos.x;
   DeltaY = mStartPoint.y - CursorPos.y;

   // stuff for moving around the brush
   if(GetTool() == ID_TOOLS_ENTITY) 
   {
      if(this == GetCapture() ) 
      {
         // if we have something then DO something.
         if(DeltaX || DeltaY ) 
         {
            switch(mViewType ) 
            {
               case ID_VIEW_3DWIREFRAME:
               case ID_VIEW_SOLIDRENDER:
               case ID_VIEW_TEXTUREVIEW:
                  // Draw the damned brush
                  if((nFlags & MK_LBUTTON) && (nFlags & MK_RBUTTON) ) 
                  {
                     pDoc->MoveEntity(DeltaX,-DeltaY,0);
                     pDoc->MoveSelectedLights(DeltaX,-DeltaY,0);
                  }
                  else if(nFlags & MK_LBUTTON ) 
                  {
                     pDoc->MoveEntity(DeltaX,0,-DeltaY);
                     pDoc->MoveSelectedLights(DeltaX,0,-DeltaY);
                  }
                  break;
               case ID_VIEW_TOPVIEW:
                  // Draw the damned brush
                  if(nFlags & MK_LBUTTON ) 
                  {
                     pDoc->MoveEntity(DeltaX,0,-DeltaY);
                     pDoc->MoveSelectedLights(DeltaX,0,-DeltaY);
                  }
                  break;
               case ID_VIEW_FRONTVIEW:
                  // Draw the damned brush
                  if(nFlags & MK_LBUTTON ) 
                  {
                     pDoc->MoveEntity(DeltaX,-DeltaY,0);
                     pDoc->MoveSelectedLights(DeltaX,-DeltaY,0);
                  }
                  break;
               case ID_VIEW_SIDEVIEW:
                  // Draw the damned brush
                  if(nFlags & MK_LBUTTON ) 
                  {
                     pDoc->MoveEntity(0,-DeltaY,DeltaX);
                     pDoc->MoveSelectedLights(0,-DeltaY,DeltaX);
                  }
                  break;
            }
            
            RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
         }
         
         // put our cursor back at the beginning
         SetCursorPos(mStartPoint.x, mStartPoint.y);
      }
   }

   // Do our grabpoint editing
   if (GetTool() == ID_EDIT_MODE_BUTTON)
   {
      ThredPoint  wp;

      mCamera.TranslateToWorld(point, wp);
      pDoc->UpdateBrushInfoPOS(wp);

      // Which button are we holding down?
      if((nFlags & MK_LBUTTON) && (!(nFlags & MK_RBUTTON)))
      {
         if((mViewType == ID_VIEW_TEXTUREVIEW) || (mViewType == ID_VIEW_3DWIREFRAME) || (mViewType == ID_VIEW_SOLIDRENDER))
         {
            // Are we editing the texture on a polygon?
            if(TextureEditing)
            {
               Invalidate(FALSE);
               UpdateWindow();

               if (!(m_renderContext->isLocked()))
               {
                  // Which polygon are we selecting?
                  m_renderContext->setSurface(pBuffer);

                  m_renderContext->lock();
                  m_renderContext->getSurface()->draw3DBegin();

                  pDoc->SelectAPolygon(point, &mCamera, m_renderContext);

                  if (pDoc->pWorldBsp->APolyIsSelected())
                  {
                     CBspNode* node = pDoc->pWorldBsp->GetSelectedNode();
                     // Calculate normal to determine which way to render selection polygon...
                     ThredPolygon*  the_poly = &(node->mBrushPtr->mTransformedPolygons[node->mPolygonNumber]);
                     Point3F       vector1, vector2, normal;

                     vector1.x = the_poly->Points[2].X - the_poly->Points[1].X;
                     vector1.y = the_poly->Points[2].Y - the_poly->Points[1].Y;
                     vector1.z = the_poly->Points[2].Z - the_poly->Points[1].Z;

                     vector2.x = the_poly->Points[1].X - the_poly->Points[0].X;
                     vector2.y = the_poly->Points[1].Y - the_poly->Points[0].Y;
                     vector2.z = the_poly->Points[1].Z - the_poly->Points[0].Z;

                     m_cross(vector1, vector2, &normal);

                     vector1.x = the_poly->Points[1].X - mCamera.mCameraPosition.X;
                     vector1.x = the_poly->Points[1].Y - mCamera.mCameraPosition.Y;
                     vector1.x = the_poly->Points[1].Z - mCamera.mCameraPosition.Z;

                     double dotprod = m_dot(normal, vector1);

                     DrawSolid(&(node->mBrushPtr->mTransformedPolygons[node->mPolygonNumber]), 4, m_renderContext, node->mBrushPtr->mTransactionType);
                  }
                  
                  m_renderContext->getSurface()->draw3DEnd();
                  m_renderContext->getSurface()->update();
                  m_renderContext->unlock();
               }
            } 
            else 
            {
               if(DeltaX || DeltaY)
               {
                  mCamera.RotateCameraWithLimits(DeltaX*ROTATION_MOVEMENT_MUL, 0, 0);
                  mCamera.MoveCamera(0,0,DeltaY);

                  // DPW - incorporate TSCamera stuff
                  m_pTSCamera->setTWC(TMat3F(
                     EulerF((mCamera.mCameraDirection.Y*ANGLE_CONVERSION),
                        (mCamera.mCameraDirection.Z*ANGLE_CONVERSION),
                        (mCamera.mCameraDirection.X*ANGLE_CONVERSION)),
                     Point3F(mCamera.mCameraPosition.X,
                        mCamera.mCameraPosition.Z,
                        mCamera.mCameraPosition.Y)).inverse());

                  RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                  SetCursorPos(mStartPoint.x, mStartPoint.y);
               }
            }
         } 
         else 
         {
            if ((showSelectRect) && (pDoc->mCurrentEntity == -1))
            {
               selectRect.SetRect(selectRect.TopLeft().x, selectRect.TopLeft().y, point.x, point.y);

               RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
               mLastCursorPos = CursorPos;
            } 
            else 
            {
               // Only the left button is pressed...
               switch (m_LeftOp)
               {
                  case BBOX_PICK_NONE:
                     DeltaX = CursorPos.x - mLastCursorPos.x;
                     DeltaY = CursorPos.y - mLastCursorPos.y;
                     if(DeltaX || DeltaY ) 
                     {
                        switch (mViewType)
                        {
                           case ID_VIEW_TOPVIEW:
                              pDoc->MoveEntity(-DeltaX/mZoomFactor, 0, DeltaY/mZoomFactor);
                              pDoc->MoveSelectedLights(-DeltaX/mZoomFactor, 0, DeltaY/mZoomFactor);
                              break;
                           case ID_VIEW_SIDEVIEW:
                              pDoc->MoveEntity(0, DeltaY/mZoomFactor, -DeltaX/mZoomFactor);
                              pDoc->MoveSelectedLights(0, DeltaY/mZoomFactor, -DeltaX/mZoomFactor);
                              break;
                           case ID_VIEW_FRONTVIEW:
                              pDoc->MoveEntity(-DeltaX/mZoomFactor, DeltaY/mZoomFactor, 0);
                              pDoc->MoveSelectedLights(-DeltaX/mZoomFactor, DeltaY/mZoomFactor, 0);
                              break;
                        }
                        RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                        mLastCursorPos = CursorPos;
                     }
                     break;
                     
                  case BBOX_PICK_BRUSH_CENTER:
                  
                     if(this == GetCapture())
                     {
                        DeltaX = CursorPos.x - mLastCursorPos.x;
                        DeltaY = CursorPos.y - mLastCursorPos.y;

                        // if we have something then DO something.
                        if(DeltaX || DeltaY)
                        {
                           switch(mViewType)
                           {
                              case ID_VIEW_TOPVIEW:
                                 if (pDoc->edit_state == ES_ROTATE)
                                 {
                                    //pDoc->MoveBrushCenter(-DeltaX/mZoomFactor,0,DeltaY/mZoomFactor);
                                    pDoc->MoveEditBoxCenter(-DeltaX/mZoomFactor,0,DeltaY/mZoomFactor);
                                    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                                 }
                                 break;
                              case ID_VIEW_SIDEVIEW:
                                 if (pDoc->edit_state == ES_ROTATE)
                                 {
                                    //pDoc->MoveBrushCenter(0,DeltaY/mZoomFactor,-DeltaX/mZoomFactor);
                                    pDoc->MoveEditBoxCenter(0,DeltaY/mZoomFactor,-DeltaX/mZoomFactor);
                                    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                                 }
                                 break;
                              case ID_VIEW_FRONTVIEW:
                                 if (pDoc->edit_state == ES_ROTATE)
                                 {
                                    //pDoc->MoveBrushCenter(-DeltaX/mZoomFactor,DeltaY/mZoomFactor,0);
                                    pDoc->MoveEditBoxCenter(-DeltaX/mZoomFactor,DeltaY/mZoomFactor,0);
                                    RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                                 }
                                 break;
                           }
                           mLastCursorPos = CursorPos;
                        }
                     }
                     break;
                     
                  case BBOX_PICK_CENTER:
                     // Gotta move the brush
                     if(this == GetCapture())
                     {
                        DeltaX = CursorPos.x - mLastCursorPos.x;
                        DeltaY = CursorPos.y - mLastCursorPos.y;

                        // if we have something then DO something.
                        if(DeltaX || DeltaY)
                        {
                           switch(mViewType)
                           {
                              case ID_VIEW_TOPVIEW:
                                 if((pDoc->mViewAxis & AXIS_X) && (pDoc->mViewAxis & AXIS_Z))
                                 {
                                    pDoc->MoveCurrentBrushes(-DeltaX/mZoomFactor,0,DeltaY/mZoomFactor);
                                 }

                                 if((pDoc->mViewAxis & AXIS_X) && !(pDoc->mViewAxis & AXIS_Z))
                                 {
                                    pDoc->MoveCurrentBrushes(-DeltaX/mZoomFactor,0,0);
                                 }

                                 if(!(pDoc->mViewAxis & AXIS_X) && (pDoc->mViewAxis & AXIS_Z))
                                 {
                                    pDoc->MoveCurrentBrushes(0,0,DeltaY/mZoomFactor);
                                 }

                                 pDoc->BuildEditBox(TRUE);
                                 RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                                 break;
                              case ID_VIEW_SIDEVIEW:
                                 if((pDoc->mViewAxis & AXIS_Y) && (pDoc->mViewAxis & AXIS_Z))
                                 {
                                    pDoc->MoveCurrentBrushes(0,DeltaY/mZoomFactor,-DeltaX/mZoomFactor);
                                 }

                                 if((pDoc->mViewAxis & AXIS_Y) && !(pDoc->mViewAxis & AXIS_Z))
                                 {
                                    pDoc->MoveCurrentBrushes(0,DeltaY/mZoomFactor,0);
                                 }

                                 if(!(pDoc->mViewAxis & AXIS_Y) && (pDoc->mViewAxis & AXIS_Z))
                                 {
                                    pDoc->MoveCurrentBrushes(0,0,-DeltaX/mZoomFactor);
                                 }

                                 pDoc->BuildEditBox(TRUE);
                                 RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                                 break;
                              case ID_VIEW_FRONTVIEW:
                                 if((pDoc->mViewAxis & AXIS_X) && (pDoc->mViewAxis & AXIS_Y))
                                 {
                                    pDoc->MoveCurrentBrushes(-DeltaX/mZoomFactor,DeltaY/mZoomFactor,0);
                                 }

                                 if((pDoc->mViewAxis & AXIS_X) && !(pDoc->mViewAxis & AXIS_Y))
                                 {
                                    pDoc->MoveCurrentBrushes(-DeltaX/mZoomFactor,0,0);
                                 }

                                 if(!(pDoc->mViewAxis & AXIS_X) && (pDoc->mViewAxis & AXIS_Y))
                                 {
                                    pDoc->MoveCurrentBrushes(0,DeltaY/mZoomFactor,0);
                                 }

                                 pDoc->BuildEditBox(TRUE);
                                 RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                                 break;
                           }
                           mLastCursorPos = CursorPos;
                        }
                     }
                     break;
                     
               case BBOX_PICK_TOP:
                  DeltaX = CursorPos.x - mLastCursorPos.x;
                  DeltaY = CursorPos.y - mLastCursorPos.y;

                  if((this == GetCapture()) && (DeltaX || DeltaY))
                  {
                     if (pDoc->edit_state == ES_RESIZE_MOVE)
                     {
                        // Resize in upward direction only
                        switch (mViewType)
                        {
                           case ID_VIEW_TOPVIEW:
                              if(pDoc->mViewAxis & AXIS_Z)
                              {
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;
               
                                 if ((pDoc->EditBox.mLargest.Z - min_bound.z - DeltaY) < 1)
                                 {
                                    resize_delta.z = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.z = -DeltaY/mZoomFactor;
                                 }
                              }
                              break;
                           case ID_VIEW_SIDEVIEW:
                              if(pDoc->mViewAxis & AXIS_Y)
                              {
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;

                                 if ((pDoc->EditBox.mLargest.Y - min_bound.y - DeltaY) < 1)
                                 {
                                    resize_delta.y = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.y = -DeltaY/mZoomFactor;
                                 }
                              }
                              break;
                           case ID_VIEW_FRONTVIEW:
                              if(pDoc->mViewAxis & AXIS_Y)
                              {
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;

                                 if ((pDoc->EditBox.mLargest.Y - min_bound.y - DeltaY) < 1)
                                 {
                                    resize_delta.y = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.y = -DeltaY/mZoomFactor;
                                 }
                              }
                              break;
                        }
            
                        pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                     }
      
                     if (pDoc->edit_state == ES_SHEAR)
                     {
                        double  height;

                        switch (mViewType)
                        {
                           case ID_VIEW_TOPVIEW:
                              if(pDoc->mViewAxis & AXIS_X)
                              {
                                 height = pDoc->EditBox.mLargest.Z - pDoc->EditBox.mSmallest.Z;
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;

                                 resize_delta.x = (DeltaX/mZoomFactor)/(height/2.0);
                  
                                 pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, XZ_SHEAR);
                              }
                              break;
                           case ID_VIEW_SIDEVIEW:
                              if(pDoc->mViewAxis & AXIS_Z)
                              {
                                 height = pDoc->EditBox.mLargest.Y - pDoc->EditBox.mSmallest.Y;
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;
               
                                 resize_delta.z = (DeltaX/mZoomFactor)/(height/2.0);

                                 pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, ZY_SHEAR);
                              }
                              break;
                           case ID_VIEW_FRONTVIEW:
                              if(pDoc->mViewAxis & AXIS_X)
                              {
                                 height = pDoc->EditBox.mLargest.Y - pDoc->EditBox.mSmallest.Y;
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;

                                 resize_delta.x = (DeltaX/mZoomFactor)/(height/2.0);

                                 pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, XY_SHEAR);
                              }
                              break;
                        }
                     }

                     pDoc->BuildEditBox(FALSE);
                     pDoc->UpdateBrushInfoSIZE(pDoc->EditBox.mSize);
                     RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                     mLastCursorPos = CursorPos;
                  }
                  break;
                  
               case BBOX_PICK_RIGHT:
                  DeltaX = CursorPos.x - mLastCursorPos.x;
                  DeltaY = CursorPos.y - mLastCursorPos.y;

                  if((this == GetCapture()) && (DeltaX || DeltaY))
                  {
                     if (pDoc->edit_state == ES_RESIZE_MOVE)
                     {
                        // Resize in right direction only
                        switch (mViewType)
                        {
                           case ID_VIEW_TOPVIEW:
                              if(pDoc->mViewAxis & AXIS_X)
                              {
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;

                                 if ((pDoc->EditBox.mLargest.X - min_bound.x + DeltaX) < 1)
                                 {
                                    resize_delta.x = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.x = DeltaX/mZoomFactor;
                                 }
                              }
                              break;
                           case ID_VIEW_SIDEVIEW:
                              if(pDoc->mViewAxis & AXIS_Z)
                              {
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;

                                 if ((pDoc->EditBox.mLargest.Z - min_bound.z + DeltaX) < 1)
                                 {
                                    resize_delta.z = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.z = DeltaX/mZoomFactor;
                                 }
                              }
                              break;
                           case ID_VIEW_FRONTVIEW:
                              if(pDoc->mViewAxis & AXIS_X)
                              {
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;

                                 if ((pDoc->EditBox.mLargest.X - min_bound.x + DeltaX) < 1)
                                 {
                                    resize_delta.x = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.x = DeltaX/mZoomFactor;
                                 }
                              }
                              break;
                        }

                        pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                     }

                     if (pDoc->edit_state == ES_SHEAR)
                     {
                        double  width;

                        switch (mViewType)
                        {
                           case ID_VIEW_TOPVIEW:
                              if(pDoc->mViewAxis & AXIS_Z)
                              {
                                 width = pDoc->EditBox.mLargest.X - pDoc->EditBox.mSmallest.X;
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;

                                 resize_delta.z = (-DeltaY/mZoomFactor)/(width/2.0);

                                 pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, ZX_SHEAR);
                              }
                              break;
                           case ID_VIEW_SIDEVIEW:
                              if(pDoc->mViewAxis & AXIS_Y)
                              {
                                 width = pDoc->EditBox.mLargest.Z - pDoc->EditBox.mSmallest.Z;
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;

                                 resize_delta.y = (-DeltaY/mZoomFactor)/(width/2.0);

                                 pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, YZ_SHEAR);
                              }
                              break;
                           case ID_VIEW_FRONTVIEW:
                              if(pDoc->mViewAxis & AXIS_Y)
                              {
                                 width = pDoc->EditBox.mLargest.X - pDoc->EditBox.mSmallest.X;
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;

                                 resize_delta.y = (-DeltaY/mZoomFactor)/(width/2.0);

                                 pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, YX_SHEAR);
                              }
                              break;
                        }
                     }

                     pDoc->BuildEditBox(FALSE);
                     pDoc->UpdateBrushInfoSIZE(pDoc->EditBox.mSize);
                     RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                     mLastCursorPos = CursorPos;
                  }
                  break;
               
               case BBOX_PICK_LEFT:
                  DeltaX = CursorPos.x - mLastCursorPos.x;
                  DeltaY = CursorPos.y - mLastCursorPos.y;

                  if((this == GetCapture()) && (DeltaX || DeltaY))
                  {
                     if (pDoc->edit_state == ES_RESIZE_MOVE)
                     {
                        // Resize in left direction only
                        switch (mViewType)
                        {
                           case ID_VIEW_TOPVIEW:
                              if(pDoc->mViewAxis & AXIS_X)
                              {
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 if ((min_bound.x - pDoc->EditBox.mSmallest.X - DeltaX) < 1)
                                 {
                                    resize_delta.x = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.x = -DeltaX/mZoomFactor;
                                 }
                              }
                              break;
                           case ID_VIEW_SIDEVIEW:
                              if(pDoc->mViewAxis & AXIS_Z)
                              {
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 if ((min_bound.z - pDoc->EditBox.mSmallest.Z - DeltaX) < 1)
                                 {
                                    resize_delta.z = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.z = -DeltaX/mZoomFactor;
                                 }
                              }
                              break;
                           case ID_VIEW_FRONTVIEW:
                              if(pDoc->mViewAxis & AXIS_X)
                              {
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 if ((min_bound.x - pDoc->EditBox.mSmallest.X - DeltaX) < 1)
                                 {
                                    resize_delta.x = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.x = -DeltaX/mZoomFactor;
                                 }
                              }
                              break;
                        }

                        pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                     }

                     if (pDoc->edit_state == ES_SHEAR)
                     {
                        double  width;

                        switch (mViewType)
                        {
                           case ID_VIEW_TOPVIEW:
                              if(pDoc->mViewAxis & AXIS_Z)
                              {
                                 width = pDoc->EditBox.mLargest.X - pDoc->EditBox.mSmallest.X;
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 resize_delta.z = (DeltaY/mZoomFactor)/(width/2.0);

                                 pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, ZX_SHEAR);
                              }
                              break;
                           case ID_VIEW_SIDEVIEW:
                              if(pDoc->mViewAxis & AXIS_Y)
                              {
                                 width = pDoc->EditBox.mLargest.Z - pDoc->EditBox.mSmallest.Z;
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 resize_delta.y = (DeltaY/mZoomFactor)/(width/2.0);

                                 pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, YZ_SHEAR);
                              }
                              break;
                           case ID_VIEW_FRONTVIEW:
                              if(pDoc->mViewAxis & AXIS_Y)
                              {
                                 width = pDoc->EditBox.mLargest.X - pDoc->EditBox.mSmallest.X;
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 resize_delta.y = (DeltaY/mZoomFactor)/(width/2.0);
   
                                 pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, YX_SHEAR);
                              }
                              break;
                        }
                     }

                     pDoc->BuildEditBox(FALSE);
                     pDoc->UpdateBrushInfoSIZE(pDoc->EditBox.mSize);
                     RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                     mLastCursorPos = CursorPos;
                  }
                  
                  break;
               
               case BBOX_PICK_BOTTOM:
                  DeltaX = CursorPos.x - mLastCursorPos.x;
                  DeltaY = CursorPos.y - mLastCursorPos.y;

                  if((this == GetCapture()) && (DeltaX || DeltaY))
                  {
                     if (pDoc->edit_state == ES_RESIZE_MOVE)
                     {
                        // Resize in downward direction only
                        switch (mViewType)
                        {
                           case ID_VIEW_TOPVIEW:
                              if(pDoc->mViewAxis & AXIS_Z)
                              {
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 if ((min_bound.z - pDoc->EditBox.mSmallest.Z + DeltaY) < 1)
                                 {
                                    resize_delta.z = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.z = DeltaY/mZoomFactor;
                                 }
                              }
                              break;
                           case ID_VIEW_SIDEVIEW:
                              if(pDoc->mViewAxis & AXIS_Y)
                              {
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 if ((min_bound.y - pDoc->EditBox.mSmallest.Y + DeltaY) < 1)
                                 {
                                    resize_delta.y = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.y = DeltaY/mZoomFactor;
                                 }
                              }
                              break;
                           case ID_VIEW_FRONTVIEW:
                              if(pDoc->mViewAxis & AXIS_Y)
                              {
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 if ((min_bound.y - pDoc->EditBox.mSmallest.Y + DeltaY) < 1)
                                 {
                                    resize_delta.y = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.y = DeltaY/mZoomFactor;
                                 }
                              }
                              break;
                        }

                        pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                     }

                     if (pDoc->edit_state == ES_SHEAR)
                     {
                        double  height;

                        switch (mViewType)
                        {
                           case ID_VIEW_TOPVIEW:
                              if(pDoc->mViewAxis & AXIS_X)
                              {
                                 height = pDoc->EditBox.mLargest.Z - pDoc->EditBox.mSmallest.Z;
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 resize_delta.x = (-DeltaX/mZoomFactor)/(height/2.0);

                                 pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, XZ_SHEAR);
                              }
                              break;
                           case ID_VIEW_SIDEVIEW:
                              if(pDoc->mViewAxis & AXIS_Z)
                              {
                                 height = pDoc->EditBox.mLargest.Y - pDoc->EditBox.mSmallest.Y;
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 resize_delta.z = (-DeltaX/mZoomFactor)/(height/2.0);

                                 pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, ZY_SHEAR);
                              }
                              break;
                           case ID_VIEW_FRONTVIEW:
                              if(pDoc->mViewAxis & AXIS_X)
                              {
                                 height = pDoc->EditBox.mLargest.Y - pDoc->EditBox.mSmallest.Y;
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 resize_delta.x = (-DeltaX/mZoomFactor)/(height/2.0);
               
                                 pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, XY_SHEAR);
                              }
                              break;
                        }
                     }

                     pDoc->BuildEditBox(FALSE);
                     pDoc->UpdateBrushInfoSIZE(pDoc->EditBox.mSize);
                     RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                     mLastCursorPos = CursorPos;
                  }
                  break;
               
               
               case BBOX_PICK_TOP_LEFT:
               
                  DeltaX = CursorPos.x - mLastCursorPos.x;
                  DeltaY = CursorPos.y - mLastCursorPos.y;

                  if((this == GetCapture()) && (DeltaX || DeltaY))
                  {
                     if(pDoc->edit_state == ES_RESIZE_MOVE)
                     {
                        // Resize in left & upward direction
                        switch (mViewType)
                        {
                           case ID_VIEW_TOPVIEW:
                              if(pDoc->mViewAxis & AXIS_Z)
                              {
                                 // Go in upward direction
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;

                                 if ((pDoc->EditBox.mLargest.Z - min_bound.z - DeltaY) < 1)
                                 {
                                    resize_delta.z = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.z = -DeltaY/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }

                              resize_delta.z = 0.F;

                              if(pDoc->mViewAxis & AXIS_X)
                              {
                                 // Now go in left direction
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 if ((min_bound.x - pDoc->EditBox.mSmallest.X - DeltaX) < 1)
                                 {
                                    resize_delta.x = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.x = -DeltaX/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }
                              break;
                           case ID_VIEW_SIDEVIEW:
                              if(pDoc->mViewAxis & AXIS_Y)
                              {
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;

                                 if((pDoc->EditBox.mLargest.Y - min_bound.y - DeltaY) < 1)
                                 {
                                    resize_delta.y = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.y = -DeltaY/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }

                              resize_delta.y = 0.F;

                              if(pDoc->mViewAxis & AXIS_Z)
                              {
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 if ((min_bound.z - pDoc->EditBox.mSmallest.Z - DeltaX) < 1)
                                 {
                                    resize_delta.z = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.z = -DeltaX/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }
                              break;
                           case ID_VIEW_FRONTVIEW:
                              if(pDoc->mViewAxis & AXIS_Y)
                              {
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;

                                 if ((pDoc->EditBox.mLargest.Y - min_bound.y - DeltaY) < 1)
                                 {
                                    resize_delta.y = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.y = -DeltaY/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }

                              resize_delta.y = 0.F;

                              if(pDoc->mViewAxis & AXIS_X)
                              {
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 if ((min_bound.x - pDoc->EditBox.mSmallest.X - DeltaX) < 1)
                                 {
                                    resize_delta.x = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.x = -DeltaX/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }
                              break;
                        }// end switch
                     }

                     if(pDoc->edit_state == ES_ROTATE)
                     {
                        DoBrushRotate(point, DeltaX, DeltaY);
                     }

                     // Preserve the box's center in case we've moved it...
                     temppoint.X = pDoc->EditBox.mCenter.X;
                     temppoint.Y = pDoc->EditBox.mCenter.Y;
                     temppoint.Z = pDoc->EditBox.mCenter.Z;

                     pDoc->BuildEditBox(FALSE);

                     // Restore the center we've preserved
                     pDoc->EditBox.mCenter.X = temppoint.X;
                     pDoc->EditBox.mCenter.Y = temppoint.Y;
                     pDoc->EditBox.mCenter.Z = temppoint.Z;

                     if (pDoc->edit_state !=ES_ROTATE)
                     {
                        pDoc->UpdateBrushInfoSIZE(pDoc->EditBox.mSize);
                     } 
                     else 
                     {
                     }
                     RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                     mLastCursorPos = CursorPos;
                  }
                  break;
               
               case BBOX_PICK_TOP_RIGHT:
                  DeltaX = CursorPos.x - mLastCursorPos.x;
                  DeltaY = CursorPos.y - mLastCursorPos.y;

                  if((this == GetCapture()) && (DeltaX || DeltaY))
                  {
                     if (pDoc->edit_state == ES_RESIZE_MOVE)
                     {
                        // Resize in upward & right directions
                        switch (mViewType)
                        {
                           case ID_VIEW_TOPVIEW:
                              if(pDoc->mViewAxis & AXIS_Z)
                              {
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;
      
                                 if ((pDoc->EditBox.mLargest.Z - min_bound.z - DeltaY) < 1)
                                 {
                                    resize_delta.z = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.z = -DeltaY/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }

                              resize_delta.z = 0.F;

                              if(pDoc->mViewAxis & AXIS_X)
                              {
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;
   
                                 if ((pDoc->EditBox.mLargest.X - min_bound.x + DeltaX) < 1)
                                 {
                                    resize_delta.x = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.x = DeltaX/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }
                              break;
                           case ID_VIEW_SIDEVIEW:
                              if(pDoc->mViewAxis & AXIS_Y)
                              {
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;

                                 if ((pDoc->EditBox.mLargest.Y - min_bound.y - DeltaY) < 1)
                                 {
                                    resize_delta.y = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.y = -DeltaY/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }

                              resize_delta.y = 0.F;

                              if(pDoc->mViewAxis & AXIS_Z)
                              {
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;

                                 if ((pDoc->EditBox.mLargest.Z - min_bound.z + DeltaX) < 1)
                                 {
                                    resize_delta.z = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.z = DeltaX/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }
                              break;
                           case ID_VIEW_FRONTVIEW:
                              if(pDoc->mViewAxis & AXIS_Y)
                              {
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;

                                 if ((pDoc->EditBox.mLargest.Y - min_bound.y - DeltaY) < 1)
                                 {
                                    resize_delta.y = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.y = -DeltaY/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }

                              resize_delta.y = 0.F;

                              if(pDoc->mViewAxis & AXIS_X)
                              {
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;

                                 if ((pDoc->EditBox.mLargest.X - min_bound.x + DeltaX) < 1)
                                 {
                                    resize_delta.x = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.x = DeltaX/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }
                              break;
                        }
                     }

                     if(pDoc->edit_state == ES_ROTATE)
                     {
                        DoBrushRotate(point, DeltaX, DeltaY);
                     }
                        // Preserve the box's center in case we've moved it...
                        temppoint.X = pDoc->EditBox.mCenter.X;
                        temppoint.Y = pDoc->EditBox.mCenter.Y;
                        temppoint.Z = pDoc->EditBox.mCenter.Z;

                        pDoc->BuildEditBox(FALSE);

                        // Restore the center's we've preserved
                        pDoc->EditBox.mCenter.X = temppoint.X;
                        pDoc->EditBox.mCenter.Y = temppoint.Y;
                        pDoc->EditBox.mCenter.Z = temppoint.Z;

                        if (pDoc->edit_state !=ES_ROTATE)
                        {
                           pDoc->UpdateBrushInfoSIZE(pDoc->EditBox.mSize);
                        } 
                        else 
                        {
                        }

                        RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                        mLastCursorPos = CursorPos;
                     }
                     break;
               
               case BBOX_PICK_BOTTOM_LEFT:
                  DeltaX = CursorPos.x - mLastCursorPos.x;
                  DeltaY = CursorPos.y - mLastCursorPos.y;

                  if((this == GetCapture()) && (DeltaX || DeltaY))
                  {
                     if (pDoc->edit_state == ES_RESIZE_MOVE)
                     {
                        // Resize in downward & left directions
                        switch (mViewType)
                        {
                           case ID_VIEW_TOPVIEW:
                              if(pDoc->mViewAxis & AXIS_Z)
                              {
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 if ((min_bound.z - pDoc->EditBox.mSmallest.Z + DeltaY) < 1)
                                 {
                                    resize_delta.z = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.z = DeltaY/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }

                              resize_delta.z = 0.F;

                              if(pDoc->mViewAxis & AXIS_X)
                              {
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 if ((min_bound.x - pDoc->EditBox.mSmallest.X - DeltaX) < 1)
                                 {
                                    resize_delta.x = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.x = -DeltaX/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }
                              break;
                           case ID_VIEW_SIDEVIEW:
                              if(pDoc->mViewAxis & AXIS_Y)
                              {
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 if ((min_bound.y - pDoc->EditBox.mSmallest.Y + DeltaY) < 1)
                                 {
                                    resize_delta.y = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.y = DeltaY/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }
      
                              resize_delta.y = 0.F;
            
                              if(pDoc->mViewAxis & AXIS_Z)
                              {
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 if ((min_bound.z - pDoc->EditBox.mSmallest.Z - DeltaX) < 1)
                                 {
                                    resize_delta.z = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.z = -DeltaX/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }
                              break;
                        case ID_VIEW_FRONTVIEW:
                           if(pDoc->mViewAxis & AXIS_Y)
                           {
                              min_bound.x = pDoc->EditBox.mLargest.X;
                              min_bound.y = pDoc->EditBox.mLargest.Y;
                              min_bound.z = pDoc->EditBox.mLargest.Z;

                              if ((min_bound.y - pDoc->EditBox.mSmallest.Y + DeltaY) < 1)
                              {
                                 resize_delta.y = 0.F;
                              } 
                              else 
                              {
                                 resize_delta.y = DeltaY/mZoomFactor;
                              }

                              pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                           }

                           resize_delta.y = 0.F;

                           if(pDoc->mViewAxis & AXIS_X)
                           {
                              min_bound.x = pDoc->EditBox.mLargest.X;
                              min_bound.y = pDoc->EditBox.mLargest.Y;
                              min_bound.z = pDoc->EditBox.mLargest.Z;

                              if ((min_bound.x - pDoc->EditBox.mSmallest.X - DeltaX) < 1)
                              {
                                 resize_delta.x = 0.F;
                              } 
                              else 
                              {
                                 resize_delta.x = -DeltaX/mZoomFactor;
                              }
               
                              pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                           }
                           break;
                     }
                  }

                  if(pDoc->edit_state == ES_ROTATE)
                  {
                     DoBrushRotate(point, DeltaX, DeltaY);
                  }
                     
                  // Preserve the box's center in case we've moved it...
                  temppoint.X = pDoc->EditBox.mCenter.X;
                  temppoint.Y = pDoc->EditBox.mCenter.Y;
                  temppoint.Z = pDoc->EditBox.mCenter.Z;

                  pDoc->BuildEditBox(FALSE);

                  // Restore the center's we've preserved
                  pDoc->EditBox.mCenter.X = temppoint.X;
                  pDoc->EditBox.mCenter.Y = temppoint.Y;
                  pDoc->EditBox.mCenter.Z = temppoint.Z;

                  if (pDoc->edit_state !=ES_ROTATE)
                  {
                     pDoc->UpdateBrushInfoSIZE(pDoc->EditBox.mSize);
                  } 
                  else 
                  {
                  }

                  RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                  mLastCursorPos = CursorPos;
                  }
               
                  break;
               
               case BBOX_PICK_BOTTOM_RIGHT:
                  DeltaX = CursorPos.x - mLastCursorPos.x;
                  DeltaY = CursorPos.y - mLastCursorPos.y;

                  if((this == GetCapture()) && (DeltaX || DeltaY)) 
                  {
                     if (pDoc->edit_state == ES_RESIZE_MOVE)
                     {
                        // Resize in downward & right directions
                        switch (mViewType)
                        {
                           case ID_VIEW_TOPVIEW:
                              if(pDoc->mViewAxis & AXIS_Z)
                              {
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 if ((min_bound.z - pDoc->EditBox.mSmallest.Z + DeltaY) < 1)
                                 {
                                    resize_delta.z = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.z = DeltaY/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }

                              resize_delta.z = 0.F;

                              if(pDoc->mViewAxis & AXIS_X)
                              {
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;

                                 if ((pDoc->EditBox.mLargest.X - min_bound.x + DeltaX) < 1)
                                 {
                                    resize_delta.x = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.x = DeltaX/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }
                              break;
                           case ID_VIEW_SIDEVIEW:
                              if(pDoc->mViewAxis & AXIS_Y)
                              {
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 if ((min_bound.y - pDoc->EditBox.mSmallest.Y + DeltaY) < 1)
                                 {
                                    resize_delta.y = 0.F;
                                 } else {
                                    resize_delta.y = DeltaY/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }

                              resize_delta.y = 0.F;

                              if(pDoc->mViewAxis & AXIS_Z)
                              {
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;

                                 if ((pDoc->EditBox.mLargest.Z - min_bound.z + DeltaX) < 1)
                                 {
                                    resize_delta.z = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.z = DeltaX/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }
                              break;
                           case ID_VIEW_FRONTVIEW:
                              if(pDoc->mViewAxis & AXIS_Y)
                              {
                                 min_bound.x = pDoc->EditBox.mLargest.X;
                                 min_bound.y = pDoc->EditBox.mLargest.Y;
                                 min_bound.z = pDoc->EditBox.mLargest.Z;

                                 if ((min_bound.y - pDoc->EditBox.mSmallest.Y + DeltaY) < 1)
                                 {
                                    resize_delta.y = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.y = DeltaY/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }

                              resize_delta.y = 0.F;

                              if(pDoc->mViewAxis & AXIS_X)
                              {
                                 min_bound.x = pDoc->EditBox.mSmallest.X;
                                 min_bound.y = pDoc->EditBox.mSmallest.Y;
                                 min_bound.z = pDoc->EditBox.mSmallest.Z;

                                 if ((pDoc->EditBox.mLargest.X - min_bound.x + DeltaX) < 1)
                                 {
                                    resize_delta.x = 0.F;
                                 } 
                                 else 
                                 {
                                    resize_delta.x = DeltaX/mZoomFactor;
                                 }

                                 pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                              }
                              break;
                           }
                        }

                        if(pDoc->edit_state == ES_ROTATE)
                        {
                           DoBrushRotate(point, DeltaX, DeltaY);
                        }
                        // Preserve the box's center in case we've moved it...
                        temppoint.X = pDoc->EditBox.mCenter.X;
                        temppoint.Y = pDoc->EditBox.mCenter.Y;
                        temppoint.Z = pDoc->EditBox.mCenter.Z;

                        pDoc->BuildEditBox(FALSE);

                        // Restore the center's we've preserved
                        pDoc->EditBox.mCenter.X = temppoint.X;
                        pDoc->EditBox.mCenter.Y = temppoint.Y;
                        pDoc->EditBox.mCenter.Z = temppoint.Z;

                        if (pDoc->edit_state !=ES_ROTATE)
                        {
                           pDoc->UpdateBrushInfoSIZE(pDoc->EditBox.mSize);
                        } 
                        else 
                        {
                        }

                        RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                        mLastCursorPos = CursorPos;
                     }
                     break;
               }
            }
         }
      }

      if((nFlags & MK_RBUTTON) && (!(nFlags & MK_LBUTTON)))
      {
         // Only the right button is pressed...
         if(this == GetCapture())
         {
            // if we have something then DO something.
            if(DeltaX || DeltaY )
            {
               switch(mViewType)
               {

                  case ID_VIEW_TEXTUREVIEW:
                     if(ShiftIsDown)
                     {
                        if (pDoc->pWorldBsp->APolyIsSelected())
                        {
                           CBspNode* node = pDoc->pWorldBsp->GetSelectedNode();

                           // move the texture
                           AssignTexture assign;
                           assign.node = node;
                           assign.topnode = pDoc->pWorldBsp->TreeTop;
                           assign.pDoc = pDoc;
                           
                           Point2I delta;
                           delta.x = mLastCursorPos.x - CursorPos.x;
                           delta.y = mLastCursorPos.y - CursorPos.y;
                           Point2I texShift;
                           texShift.x = node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.x + delta.x;
                           texShift.y = node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.y + delta.y;
                           
                           if(!CtrlIsDown)
                           {
                              assign.ShiftNodesU(pDoc->pWorldBsp->TreeTop,
                                 node->mBrushPtr, node->mPolygonNumber, texShift.x);
                              node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.x = (texShift.x & 0x7f);
                           }
                           if(!AltIsDown)
                           {
                              assign.ShiftNodesV(pDoc->pWorldBsp->TreeTop,
                                 node->mBrushPtr, node->mPolygonNumber, texShift.y);
                              node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.y = (texShift.y & 0x7f);
                           }
                           RedrawWindow();
                         }
                         
                        mLastCursorPos = CursorPos;
                        break;
                     }
                  case ID_VIEW_3DWIREFRAME:
                  case ID_VIEW_SOLIDRENDER:
                     mCamera.RotateCameraWithLimits(DeltaX*ROTATION_MOVEMENT_MUL, DeltaY*ROTATION_MOVEMENT_MUL, 0);

                     m_pTSCamera->setTWC(TMat3F(
                     EulerF((mCamera.mCameraDirection.Y*ANGLE_CONVERSION),
                        (mCamera.mCameraDirection.Z*ANGLE_CONVERSION),
                        (mCamera.mCameraDirection.X*ANGLE_CONVERSION)),
                        Point3F(mCamera.mCameraPosition.X,
                           mCamera.mCameraPosition.Z,
                           mCamera.mCameraPosition.Y)).inverse());
                     RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                     SetCursorPos(mStartPoint.x, mStartPoint.y);
                     break;
                  case ID_VIEW_TOPVIEW:
                     DeltaX = CursorPos.x - mLastCursorPos.x;
                     DeltaY = CursorPos.y - mLastCursorPos.y;

                     if (ShiftIsDown)
                     {
                        mZoomFactor += (ZOOM_CHANGE * (DeltaY+DeltaX));

                        if(mZoomFactor < .05)
                        {
                           mZoomFactor = .05;
                        }
                     } 
                     else 
                     {
                        mCamera.MoveCameraAbsolute(-DeltaX/mZoomFactor, 0, DeltaY/mZoomFactor);
                     }
                     mLastCursorPos = CursorPos;
                     RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                     break;
                  case ID_VIEW_FRONTVIEW:
                     DeltaX = CursorPos.x - mLastCursorPos.x;
                     DeltaY = CursorPos.y - mLastCursorPos.y;

                     if (ShiftIsDown)
                     {
                        mZoomFactor += (ZOOM_CHANGE * (DeltaY+DeltaX));
         
                        if(mZoomFactor < .05)
                        {
                           mZoomFactor = .05;
                         }
                     } 
                     else 
                     {
                        mCamera.MoveCameraAbsolute(-DeltaX/mZoomFactor, DeltaY/mZoomFactor, 0);
                     }
                     mLastCursorPos = CursorPos;
                     RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                     break;
                  case ID_VIEW_SIDEVIEW:
                     DeltaX = CursorPos.x - mLastCursorPos.x;
                     DeltaY = CursorPos.y - mLastCursorPos.y;

                     if(ShiftIsDown)
                     {
                        mZoomFactor += (ZOOM_CHANGE * (DeltaY+DeltaX));

                        if(mZoomFactor < .05)
                        {
                           mZoomFactor = .05;
                        }
                     } 
                     else 
                     {
                        mCamera.MoveCameraAbsolute(0, DeltaY/mZoomFactor, -DeltaX/mZoomFactor);
                     }
                     mLastCursorPos = CursorPos;
                     RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                     break;
               }
            }
         }     
      }

      if((nFlags & MK_RBUTTON) && ((nFlags & MK_LBUTTON)))
      {
         // Both the right & left buttons are pressed...
         if(this == GetCapture())
         {
            // if we have something then DO something.
            if(DeltaX || DeltaY )
            {
               switch(mViewType)
               {
                  case ID_VIEW_3DWIREFRAME:
                  case ID_VIEW_SOLIDRENDER:
                  case ID_VIEW_TEXTUREVIEW:
                     mCamera.MoveCamera(DeltaX, DeltaY, 0);

                     // DPW - incorporate TSCamera stuff
                     m_pTSCamera->setTWC(TMat3F(
                        EulerF((mCamera.mCameraDirection.Y*ANGLE_CONVERSION),
                           (mCamera.mCameraDirection.Z*ANGLE_CONVERSION),
                           (mCamera.mCameraDirection.X*ANGLE_CONVERSION)),
                           Point3F(mCamera.mCameraPosition.X,
                              mCamera.mCameraPosition.Z,
                              mCamera.mCameraPosition.Y)).inverse());

                     RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
                     SetCursorPos(mStartPoint.x, mStartPoint.y);
               }
            }
         }
      } // End both right & left buttons depressed
   } // End processing for edit mode
}

void CTHREDView::DoBrushRotate(CPoint point, int DeltaX, int DeltaY)
{
   ThredPoint  temppoint;
   CTHREDDoc*  pDoc = GetDocument();

   switch(mViewType)
   {
      case ID_VIEW_TOPVIEW:
         mCamera.TranslateToWorld(point, temppoint);
         if (temppoint.Z > pDoc->EditBox.mCenter.Z)
         {
            if (temppoint.X > pDoc->EditBox.mCenter.X)
            {
               pDoc->RotateCurrentBrushes((-DeltaX*ROTATION_MOVEMENT_MUL)-(DeltaY*ROTATION_MOVEMENT_MUL), 0, 0);
            } 
            else 
            {
               pDoc->RotateCurrentBrushes((-DeltaX*ROTATION_MOVEMENT_MUL)+(DeltaY*ROTATION_MOVEMENT_MUL), 0, 0);
            }
         } 
         else 
         {
            if (temppoint.X > pDoc->EditBox.mCenter.X)
            {
               pDoc->RotateCurrentBrushes((DeltaX*ROTATION_MOVEMENT_MUL)-(DeltaY*ROTATION_MOVEMENT_MUL), 0, 0);
            } 
            else 
            {
               pDoc->RotateCurrentBrushes((DeltaX*ROTATION_MOVEMENT_MUL)+(DeltaY*ROTATION_MOVEMENT_MUL), 0, 0);
            }
         }
         break;
      case ID_VIEW_SIDEVIEW:
         mCamera.TranslateToWorld(point, temppoint);
         if (temppoint.Y > pDoc->EditBox.mCenter.Y)
         {
            if (temppoint.Z > pDoc->EditBox.mCenter.Z)
            {
               pDoc->RotateCurrentBrushes(0, (-DeltaX*ROTATION_MOVEMENT_MUL)-(DeltaY*ROTATION_MOVEMENT_MUL), 0);
            } 
            else 
            {
               pDoc->RotateCurrentBrushes(0, (-DeltaX*ROTATION_MOVEMENT_MUL)+(DeltaY*ROTATION_MOVEMENT_MUL), 0);
            }
         } 
         else 
         {
            if (temppoint.Z > pDoc->EditBox.mCenter.Z)
            {
               pDoc->RotateCurrentBrushes(0, (DeltaX*ROTATION_MOVEMENT_MUL)-(DeltaY*ROTATION_MOVEMENT_MUL), 0);
            } 
            else 
            {
               pDoc->RotateCurrentBrushes(0, (DeltaX*ROTATION_MOVEMENT_MUL)+(DeltaY*ROTATION_MOVEMENT_MUL), 0);
            }
         }
         break;
      case ID_VIEW_FRONTVIEW:
         mCamera.TranslateToWorld(point, temppoint);
         if (temppoint.Y > pDoc->EditBox.mCenter.Y)
         {
            if (temppoint.X > pDoc->EditBox.mCenter.X)
            {
               pDoc->RotateCurrentBrushes(0, 0, (DeltaX*ROTATION_MOVEMENT_MUL)+(DeltaY*ROTATION_MOVEMENT_MUL));
            } 
            else 
            {
               pDoc->RotateCurrentBrushes(0, 0, (DeltaX*ROTATION_MOVEMENT_MUL)-(DeltaY*ROTATION_MOVEMENT_MUL));
            }
         } 
         else 
         {
            if (temppoint.X > pDoc->EditBox.mCenter.X)
            {
               pDoc->RotateCurrentBrushes(0, 0, (-DeltaX*ROTATION_MOVEMENT_MUL)+(DeltaY*ROTATION_MOVEMENT_MUL));
            } 
            else 
            {
               pDoc->RotateCurrentBrushes(0, 0, (-DeltaX*ROTATION_MOVEMENT_MUL)-(DeltaY*ROTATION_MOVEMENT_MUL));
            }
         }
         break;
   }// end switch
}

void CTHREDView::OnRButtonDown(UINT nFlags, CPoint point) 
{
   // get the doc
   CTHREDDoc* pDoc = GetDocument();
   ASSERT_VALID(pDoc);

   if (nFlags & MK_SHIFT)
   {
      ShiftIsDown = TRUE;
   } 
   else 
   {
      ShiftIsDown = FALSE;
   }

   // if we right clicked
   if(GetTool() == ID_TOOLS_ENTITY ) 
   {
      // get a new entity
      pDoc->SelectEntity(point, mCamera, TRUE );
      pDoc->SelectLight(point, mCamera, TRUE );
   }

   if(!(nFlags & MK_LBUTTON) ) 
   {
      // lock to our window
      SetCapture();

      // hide the view button
      ViewButton.ShowWindow( SW_HIDE );

      if(GetTool() == ID_BRUSH_GROUPS_SELECT )
      {
         mStartPoint = mCurrentPoint = point;
         // get a DC
         CClientDC Dc(this);

         // draw our rect twice... once using the
         // old rect and once using the new
         CRect NewRect(mStartPoint, point);
         NewRect.NormalizeRect();
         CSize Size(2,2);
         Dc.DrawDragRect(&NewRect, Size, NULL, Size);
      } 
      else 
      {
         if (GetTool() == ID_EDIT_MODE_BUTTON)
         {
            // Determine if you're within bounding box of current brush, and
            // If you are, determine what kind of operation you should do based
            // on the relative position of the pointer within the brush bounds
            GetCursorPos(&mStartPoint);
            mLastCursorPos = mStartPoint;
            //ShowCursor(FALSE);

            //m_RightOp = GetDocument()->InCurrentBrush(GetDocument()->mCurrentBrush, point, mViewType, &mCamera);
            m_RightOp = BBOX_PICK_NONE;
            m_LeftOp = BBOX_PICK_NONE;
            
            if(ShiftIsDown)
            {
               m_renderContext->lock();
               pDoc->SelectAPolygon(point, &mCamera, m_renderContext);
               m_renderContext->unlock();
            }
         }	
         else 
         {
            // get our point
            GetCursorPos(&mStartPoint);

            // hide our mouse cursor
            ShowCursor(FALSE);
         }
      }
   }
}


// This function should check if we have a current selection!!!
void CTHREDView::OnUpdateBrushGroupsMakenewgroup(CCmdUI* pCmdUI) 
{
   // get the doc
   CTHREDDoc* pDoc = GetDocument();
   ASSERT_VALID(pDoc);

   if(pDoc->BrushesSelected() )
      pCmdUI->Enable();
   else
      pCmdUI->Enable(0);
}
void CTHREDView::OnUpdateBrushGroupsAddtogroup(CCmdUI* pCmdUI) 
{
   // get the doc
   CTHREDDoc* pDoc = GetDocument();
   ASSERT_VALID(pDoc);

   if(pDoc->BrushesSelected() )
      pCmdUI->Enable();
   else
      pCmdUI->Enable(0);
}

void CTHREDView::OnInitialUpdate() 
{
	// DPW - Added for TS compatibility
	try
	{
      // Create the render context for this view...
      // Note:  I'll initialize all the render context values at render time...
      m_renderContext = new TSRenderContext;

		// Create the perspective camera
		RECT wsize;
		GetClientRect(&wsize);

		RectI screenView(Point2I(0,0),Point2I(wsize.right-1,wsize.bottom-1));


      double max_dim = (wsize.right > wsize.bottom) ? (double)wsize.right : (double)wsize.bottom;
		RectF worldView(Point2F(-wsize.right/max_dim, wsize.bottom/max_dim),
		   Point2F(wsize.right/max_dim, -wsize.bottom/max_dim));

      ResizeBuffer(wsize.right-1, wsize.bottom-1);

		m_pTSCamera = new TSPerspectiveCamera(screenView, worldView, nearPlane, farPlane);
	
      m_pTSCamera->setTWC(TMat3F(
                EulerF((mCamera.mCameraDirection.Y*ANGLE_CONVERSION),
                       (mCamera.mCameraDirection.Z*ANGLE_CONVERSION),
                       (mCamera.mCameraDirection.X*ANGLE_CONVERSION)),
                Point3F(mCamera.mCameraPosition.X,
                        mCamera.mCameraPosition.Z,
                        mCamera.mCameraPosition.Y)).inverse());

      if ((mViewType != ID_VIEW_SOLIDRENDER) &&
        (mViewType != ID_VIEW_TEXTUREVIEW) &&
        (mViewType != ID_VIEW_3DWIREFRAME))
      {
         OnViewCentercameraatorigin();
      }
   }

	catch(AssertException TSExcept)
	{
		CString strMessage;
		strMessage.Format("TS AssertException\n%s %d: %s",
						TSExcept.fFile, TSExcept.fLine, TSExcept.fMessage);
		AfxMessageBox(strMessage, MB_OK, 0);
	}

	CView::OnInitialUpdate();
	
   // set to full size
   if( pFrame ) pFrame->ShowWindow( SW_SHOWMAXIMIZED );
	SetTitle();
}

void CTHREDView::OnLButtonDown(UINT nFlags, CPoint point) 
{
   CTHREDDoc*    pDoc = GetDocument();
   CThredBrush*  tempbrush;
   int           status;

   if (nFlags & MK_SHIFT)
   {
      ShiftIsDown = TRUE;
   } 
   else 
   {
      ShiftIsDown = FALSE;
   }

   if (nFlags & MK_CONTROL)
   {
      CtrlIsDown = TRUE;
   } 
   else 
   {
      CtrlIsDown = FALSE;
   }

   if(!(nFlags & MK_RBUTTON) )
   {
      // lock to our window
      SetCapture();

      // store info on the selected items some info
      if( Globals.mLockTexturePositions )
      {
         tempbrush = pDoc->mBrushList;
         while( tempbrush )
         {
            if( tempbrush->mFlags & BRUSH_SELECTED )
               tempbrush->BeginTextureChange();
      
            tempbrush = tempbrush->GetNextBrush();
         }
      }
      
      // hide the view button
      ViewButton.ShowWindow( SW_HIDE );

      if(GetTool() == ID_TEXTURE_EDIT_FACE )
      {
         m_renderContext->setSurface(pBuffer);

         if (!(m_renderContext->isLocked()))
         {
            m_renderContext->lock();
            m_renderContext->getSurface()->draw3DBegin();

            pDoc->SelectAPolygon(point, &mCamera, m_renderContext);

            if (pDoc->pWorldBsp->APolyIsSelected())
            {
               CBspNode* node = pDoc->pWorldBsp->GetSelectedNode();
               DrawSolid(&(node->mBrushPtr->mTransformedPolygons[node->mPolygonNumber]), 4, m_renderContext, node->mBrushPtr->mTransactionType);
            }

            m_renderContext->getSurface()->draw3DEnd();
            m_renderContext->getSurface()->update();
            m_renderContext->unlock();
         }
      } 
      else 
      {
         if(GetTool() == ID_BRUSH_GROUPS_SELECT )
         {
            mStartPoint = mCurrentPoint = point;
            // get a DC
            CClientDC Dc(this);

            // draw our rect twice... once using the
            // old rect and once using the new
            CRect NewRect(mStartPoint, point);
            NewRect.NormalizeRect();
            CSize Size(2,2);
            Dc.DrawDragRect(&NewRect, Size, NULL, Size);
         } 
         else 
         {
            if(GetTool() == ID_BRUSH_ADJUSTMENTMODE )
            {
               ;     // do nothing here for now...
            } 
            else 
            {
               if(GetTool() == ID_TOOLS_BRUSH_MOVEROTATEBRUSH)
               {
                  // get our point
                  GetCursorPos(&mStartPoint);

                  ShowCursor(FALSE);

               } 
               else 
               {
                  if (GetTool() == ID_EDIT_MODE_BUTTON)
                  {
                     if ((mViewType == ID_VIEW_SIDEVIEW) ||
                     (mViewType == ID_VIEW_FRONTVIEW) ||
                     (mViewType == ID_VIEW_TOPVIEW))
                     {
                        // Determine if you're within bounding box of current brush, and
                        // If you are, determine what kind of operation you should do based
                        // on the relative position of the pointer within the brush bounds
                        GetCursorPos(&mStartPoint);
                        mLastCursorPos = mStartPoint;
                        //ShowCursor(FALSE);

                        ////////////////////////////////////////////////////////
                        m_RightOp = BBOX_PICK_NONE;
                        m_LeftOp = BBOX_PICK_NONE;

                        CLightEmitter * pEmitter = NULL;
                        CEntity   *  pEntity = NULL;

                        // select the light/entity
                        pEntity = pDoc->SelectEntity( point, mCamera, FALSE );
                        pEmitter = pDoc->SelectLight( point, mCamera, FALSE );

                        // check if the point is in an entity or a light
                        if( pEntity || pEmitter )
                        {
                           // check if shift is down
                           if( ShiftIsDown )
                           {
                              // work with entity first, then light if no
                              // entity is selected
                              if( pEntity )
                              {
                                 // select it
                                 pDoc->SetCurrentEntity( pEntity );
                              }
                              else if( pEmitter )
                              {
                                 // select it
                                 pDoc->SetCurrentLight( pEmitter );
                              }
                              return;
                           }
                           else
                           {
                              // check if already selected - entity first
                              if( pEntity )
                              {
                                 if( !( pEntity->mFlags & ENTITY_SELECTED ) )
                                 {
                                    // clear the lights
                                    pDoc->ResetSelectedLights();

                                    // select the entity with full resets
                                    pDoc->SelectEntity( point, mCamera, TRUE );
                                    if( pDoc->mCurrentBrush )
                                    {
                                       pDoc->mCurrentBrush->mFlags &= ~BRUSH_SELECTED;
                                       pDoc->mCurrentBrush = NULL;
                                    }
                                 }
                              }
                              else if( pEmitter )
                              {
                                 if( !( pEmitter->getSelect() ) )
                                 {
                                    // clear the entities
                                    pDoc->ResetSelectedEntities();

                                    // select the light  with full resets
                                    pDoc->SelectLight( point, mCamera, TRUE );
                                    if( pDoc->mCurrentBrush )
                                    {
                                       pDoc->mCurrentBrush->mFlags &= ~BRUSH_SELECTED;
                                       pDoc->mCurrentBrush = NULL;
                                    }
                                 }
                              }
                              
                              if( pDoc->mCurrentBrush == &(pDoc->mRegularBrush ) )
                              {
                                 pDoc->mCurrentBrush->mFlags &= ~BRUSH_SELECTED;
                                 pDoc->mCurrentBrush = NULL;
                              }
                                 
                              return;
                           }
                        }
                        else
                        {
                           // check if should reset the ptr
                           if( !pDoc->mCurrentBrush )
                           {
                              // reset the entities/lights
                              pDoc->ResetSelectedEntities();
                              pDoc->ResetSelectedLights();
                           
                              pDoc->mCurrentBrush = &(pDoc->mRegularBrush );
//                              return;
                           }
                        }

                        int editstatus = pDoc->InCurrentBrush(&(pDoc->EditBox), point, mViewType, &mCamera);

                        switch(editstatus)
                        {
                           case BBOX_PICK_TOP_LEFT:
                           case BBOX_PICK_TOP_RIGHT:
                           case BBOX_PICK_BOTTOM_LEFT:
                           case BBOX_PICK_BOTTOM_RIGHT:
                              if ((pDoc->edit_state == ES_RESIZE_MOVE) || (pDoc->edit_state == ES_ROTATE))
                              {
                                 // Do the editing function
                                 m_LeftOp = editstatus;

                                 // Save away undo info
                                 pDoc->StoreUndoInfo();
                              }
                              break;
                           case BBOX_PICK_TOP:
                           case BBOX_PICK_LEFT:
                           case BBOX_PICK_RIGHT:
                           case BBOX_PICK_BOTTOM:
                              if ((pDoc->edit_state == ES_RESIZE_MOVE) || (pDoc->edit_state == ES_SHEAR))
                              {
                                 // Go ahead and do the edit function
                                 m_LeftOp = editstatus;

                                 // Save away undo info
                                 pDoc->StoreUndoInfo();
                              }
                              break;
                           case BBOX_PICK_BRUSH_CENTER:
                              if (pDoc->edit_state == ES_ROTATE)
                              {
                                 // Go ahead and do the edit function
                                 m_LeftOp = editstatus;
                              } else {
                                 m_LeftOp = BBOX_PICK_CENTER;
                              }
                              break;
                           case BBOX_PICK_CENTER:
                              // Init to none found...
                              m_LeftOp = BBOX_PICK_CENTER;
                              status = BBOX_PICK_NONE;

                              tempbrush = pDoc->mBrushList;

                              if ( (pDoc->mCurrentBrush == &(pDoc->mRegularBrush)) &&
                                 (((pDoc->InCurrentBrush(&(pDoc->mCurrentBrush->mBoundingBox), point, mViewType, &mCamera)) == BBOX_PICK_BRUSH_CENTER) ||
                                 ((pDoc->InCurrentBrush(&(pDoc->mCurrentBrush->mBoundingBox), point, mViewType, &mCamera)) == BBOX_PICK_CENTER)) )
                              {
                                 //m_LeftOp = BBOX_PICK_CENTER;

                                 // Save off undo info...
                                 pDoc->StoreUndoInfo();

                                 tempbrush = NULL;
                              } else
                              {
                                 m_LeftOp = BBOX_PICK_NONE;
                              }

                              // Are you inside any unselected brushes?
                              while (tempbrush)
                              {
                                 if (tempbrush->mFlags & BRUSH_SELECTED)
                                 {
                                    // In a selected brush
                                    if (ShiftIsDown)
                                    {
                                       status = pDoc->InCurrentBrush(&(tempbrush->mBoundingBox), point, mViewType, &mCamera);

                                       if (status == BBOX_PICK_CENTER)
                                       {
                                          // Okay to deselect this brush (our shift is down)
                                          pDoc->SelectABrushIndependent(point, mCamera);

                                          // Get out of the loop
                                          m_LeftOp = BBOX_PICK_NONE;
                                          tempbrush = NULL;
                                       } 
                                       else 
                                       {
                                          tempbrush = tempbrush->GetNextBrush();
                                          //tempbrush = NULL;
                                       }
                                    } 
                                    else 
                                    {
                                       // Don't look at already-selected brushes if shift isn't down
                                       //dpw tempbrush = tempbrush->GetNextBrush();
                                       m_LeftOp = BBOX_PICK_CENTER;

                                       // Save away undo info
                                       pDoc->StoreUndoInfo();

                                       tempbrush = NULL;
                                    }
                                 } 
                                 else 
                                 {
                                    // This brush is not yet selected, so check whether we're inside it...
                                    status = pDoc->InCurrentBrush(&(tempbrush->mBoundingBox), point, mViewType, &mCamera);

                                    if (status == BBOX_PICK_CENTER)
                                    {
                                       // We're inside a previously-unselected brush
                                       if (ShiftIsDown)
                                       {
                                          pDoc->SelectABrushIndependent(point, mCamera);

                                          // Get out of the loop
                                          m_LeftOp = BBOX_PICK_NONE;
                                          tempbrush = NULL;
                                       } 
                                       else 
                                       {
                                          //dpw pDoc->SelectABrush(point, mCamera);

                                          // Deselect all other selected brushes...There can be only one...

                                          // Get out of the loop
                                          //dpw m_LeftOp = BBOX_PICK_NONE;
                                          m_LeftOp = BBOX_PICK_CENTER;

                                          // Save away undo info
                                          pDoc->StoreUndoInfo();

                                          tempbrush = NULL;
                                       }
                                    } 
                                    else 
                                    {
                                       tempbrush = tempbrush->GetNextBrush();
                                    }
                                 }
                              }// end while
                              
                              break;
                              
                           case BBOX_PICK_NONE:

                              if (ShiftIsDown)
                              {
                                 // If we are inside a brush we can select/deselect it
                                 pDoc->SelectABrushIndependent(point, mCamera);

                                 // Draw the bounding box selection rectangle
                                 selectRect.SetRect(point.x, point.y, point.x+1, point.y+1);
                                 showSelectRect = TRUE;
                              } 
                              else 
                              {
                                 tempbrush = pDoc->mBrushList;

                                 while (tempbrush)
                                 {
                                    if ((status = pDoc->InCurrentBrush(&(tempbrush->mBoundingBox), point, mViewType, &mCamera)) == BBOX_PICK_CENTER)
                                    {
                                       if (tempbrush->mFlags & BRUSH_SELECTED)
                                       {
                                          // Inside a brush & it's already selected, so toggle resize/rot. mode, etc.
                                       } 
                                       else 
                                       {
                                          //if (tempbrush->mBrushGroup == NO_GROUP)
                                          //{
                                          // tempbrush is the non-grouped brush you've selected, so select it...
                                          pDoc->SelectABrush(point, mCamera);
                                          //} else {
                                          // This is a grouped brush, select all brushes in group
                                          //}
                                       }
                                       // Get us outta this loop
                                       tempbrush = NULL;
                                    } 
                                    else 
                                    {
                                       tempbrush = tempbrush->GetNextBrush();
                                    }

                                 }// end while

                                 if( status == BBOX_PICK_NONE )
                                 {
                                    // Deselect any selected brushes, and make the regular brush current
                                    pDoc->mCurrentBrush = &(pDoc->mRegularBrush);
                                    pDoc->mAdjustmentMode = FALSE;

                                    // Loop through and deselect everyone...
                                    tempbrush = pDoc->mBrushList;
                                    while(tempbrush)
                                    {
                                       tempbrush->mFlags &= ~BRUSH_SELECTED;
                                       tempbrush->NextSelectedBrush = NULL;
                                       tempbrush->PrevSelectedBrush = NULL;

                                       tempbrush = tempbrush->GetNextBrush();
                                    }

                                    // clear the lights
                                    pDoc->ResetSelectedLights();
                                    
                                    // clear the entities
                                    pDoc->ResetSelectedEntities();
                                    
                                    // Draw the bounding box selection rectangle
                                    selectRect.SetRect(point.x, point.y, point.x+1, point.y+1);
                                    showSelectRect = TRUE;
                                 }
                              }
                              break;
                        } // end switch
                        
                        if( pDoc->mCurrentBrush == &(pDoc->mRegularBrush ) )
                        {
                           // Enable +,- toolbar buttons
                           CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

                           pFrame->m_wndBrushToolBar.GetToolBarCtrl().SetState(ID_BRUSH_ADDTOWORLD, TBSTATE_ENABLED);
                           pFrame->m_wndBrushToolBar.GetToolBarCtrl().SetState(ID_BRUSH_SUBTRACTFROMWORLD, TBSTATE_ENABLED);
                        } 
                        else 
                        {
                           // Disable +,- toolbar buttons
                           CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
                           int         nindex = pFrame->m_wndBrushToolBar.CommandToIndex(ID_BRUSH_ADDTOWORLD);

                           pFrame->m_wndBrushToolBar.GetToolBarCtrl().Indeterminate(ID_BRUSH_ADDTOWORLD, TRUE);
                           pFrame->m_wndBrushToolBar.GetToolBarCtrl().Indeterminate(ID_BRUSH_SUBTRACTFROMWORLD, TRUE);
                           //pFrame->m_wndBrushToolBar.GetToolBarCtrl().EnableButton(ID_BRUSH_ADDTOWORLD, FALSE);
                        }
                     } 
                     else 
                     {
                        if (CtrlIsDown||ShiftIsDown)
                        {
                           TextureEditing = TRUE;
                           m_renderContext->setSurface(pBuffer);

                           if (!(m_renderContext->isLocked()))
                           {
                              m_renderContext->lock();
                              m_renderContext->getSurface()->draw3DBegin();

                              pDoc->SelectAPolygon(point, &mCamera, m_renderContext);

                              if (pDoc->pWorldBsp->APolyIsSelected())
                              {
                                 CBspNode* node = pDoc->pWorldBsp->GetSelectedNode();
                                 DrawSolid(&(node->mBrushPtr->mTransformedPolygons[node->mPolygonNumber]), 4, m_renderContext, node->mBrushPtr->mTransactionType);
                              }

                              m_renderContext->getSurface()->draw3DEnd();
                              m_renderContext->getSurface()->update();
                              m_renderContext->unlock();
                              
                           }
                        } 
                        else 
                        {
                           GetCursorPos(&mStartPoint);
                        }
                     }
                  ////////////////////////////////////////////////////////

                  } 
                  else 
                  {
                     if (GetTool() == ID_TOOLS_SELECTNMOVE)
                     {
                        pDoc->SelectABrush(point, mCamera);

                        // get our point
                        GetCursorPos(&mStartPoint);

                        // hide our mouse cursor
                        ShowCursor(FALSE);
                     } 
                     else 
                     {
                        // get our point
                        GetCursorPos(&mStartPoint);

                        // hide our mouse cursor
                        ShowCursor(FALSE);
                     }
                  }
               }
            }
         }
      }
   }
}

// handle double click
void CTHREDView::OnLButtonDblClk( UINT nFlags, CPoint point )
{
   CTHREDDoc*    pDoc = GetDocument();
    
   // check if shift or cntrl or right button are down is down
   if( ( nFlags & MK_SHIFT ) || ( nFlags & MK_CONTROL ) || ( nFlags & MK_RBUTTON ) )
      return;

   if( GetTool() == ID_EDIT_MODE_BUTTON )
   {
      if( ( mViewType == ID_VIEW_SIDEVIEW ) ||
         ( mViewType == ID_VIEW_FRONTVIEW ) ||
         ( mViewType == ID_VIEW_TOPVIEW ) )
      {
         if( pDoc->SelectEntity( point, mCamera, FALSE ) )
            pDoc->OnEntitiesEditor();
         else 
         {
            CLightEmitter * pEmitter = pDoc->SelectLight( point, mCamera, false );
            if( pEmitter )
               pEmitter->inspect(true,true);
         }
      }
   }
}


void CTHREDView::OnLButtonUp(UINT nFlags, CPoint point) 
{
    double  width, height, snapamt;
    Point3F resize_delta(0.0, 0.0, 0.0);
    Point3F min_bound(0.0, 0.0, 0.0);

    if(GetAsyncKeyState(VK_MENU) & 0x8000)
    {
      AltIsDown = TRUE;
    } else {
      AltIsDown = FALSE;
    }

    if (nFlags & MK_SHIFT)
    {
      ShiftIsDown = TRUE;
    } else {
      ShiftIsDown = FALSE;
    }

    if (nFlags & MK_CONTROL)
    {
      CtrlIsDown = TRUE;
    } else {
      CtrlIsDown = FALSE;
    }

	if(!(nFlags & MK_RBUTTON) ) {
		// get the doc
		CTHREDDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);

		// release our capture
		if(this == GetCapture() )
      {
			ReleaseCapture();
         
         // hide the view button
         ViewButton.ShowWindow( SW_SHOW );
      }
      else
         return;
#if 0
		// make sure we tell we are done moving
		if(GetTool() == ID_TOOLS_BRUSH_MOVEROTATEBRUSH || GetTool() == ID_TOOLS_BRUSH_MOVESELECTEDBRUSHES) {
			pDoc->DoneMoveBrush();
			pDoc->DoneRotateBrush();
			pDoc->DoneMoveEntity();
			pDoc->DoneMoveLights( false, 0 );
		}

        if (GetTool() == ID_TOOLS_SELECTNMOVE)
        {
			pDoc->DoneMoveBrush();
        }

		if(GetTool() == ID_TOOLS_BRUSH_SCALEBRUSH )
			pDoc->DoneScaleBrush();

		if(GetTool() == ID_TOOLS_BRUSH_SHEARBRUSH )
			pDoc->DoneShearBrush();
#endif
		// make sure we tell we are done moving
  		pDoc->DoneMoveEntity();
  		pDoc->DoneMoveLights( false, 0 );

        if(GetTool() == ID_TEXTURE_EDIT_FACE )
        {
 			// Done selecting a face...So pull up a dialog box to select a texture
			AssignTexture	material_dialog;

			if (pDoc)
			{
              if (pDoc->pWorldBsp->APolyIsSelected())	// Check for valid selection too
              {
                CBspNode* node = pDoc->pWorldBsp->GetSelectedNode();

				// DPW - Added new dialog to view material list
				material_dialog.material_count = pDoc->GetMaterialList()->getMaterialsCount();
				material_dialog.material_list = pDoc->GetMaterialList();
				material_dialog.palette = pDoc->GetPalette();

                material_dialog.parentview = this;
                material_dialog.bsptree = pDoc->pWorldBsp;
                material_dialog.node = node;
                material_dialog.topnode = pDoc->pWorldBsp->TreeTop;
                material_dialog.pDoc = pDoc;

                if (node->mBrushPtr)
                {
                  material_dialog.SetIndex(node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureID);
                }
               material_dialog.DoModal();

				//if (material_dialog.DoModal() == IDOK)
                //{
                //}
              }
			}

        } else {
		  if(GetTool() == ID_BRUSH_GROUPS_SELECT ) {
			CRect NewRect(mStartPoint, point);
			NewRect.NormalizeRect();

			// select stuff
			pDoc->SelectBrushes(NewRect, mCamera);

			// update all the views
			pDoc->UpdateAllViews(NULL);

	  	  } else {
            if(GetTool() == ID_BRUSH_ADJUSTMENTMODE )
            {
              if ((mViewType == ID_VIEW_SOLIDRENDER) ||
                  (mViewType == ID_VIEW_TEXTUREVIEW) ||
                  (mViewType == ID_VIEW_3DWIREFRAME))
              {
                m_renderContext->setSurface(pBuffer);
                m_renderContext->setPointArray(&DefaultPointArray);
                m_renderContext->setCamera(m_pTSCamera);
                m_renderContext->setLights(&m_GSceneLights);

                m_renderContext->lock();
                m_renderContext->getSurface()->draw3DBegin();

                pDoc->SelectAPolygon(point, &mCamera, m_renderContext);

                if (pDoc->pWorldBsp->APolyIsSelected())
                {
                  CBspNode* node = pDoc->pWorldBsp->GetSelectedNode();

                  pDoc->ForceBrushSelection(node->mBrushPtr);
                }

                m_renderContext->getSurface()->draw3DEnd();
                m_renderContext->unlock();
              } else {
			    pDoc->SelectABrush(point, mCamera);
              }

			  // update all the views
			  pDoc->UpdateAllViews(NULL);
		    } else {

			  // show our mouse cursor
			  ShowCursor(TRUE);

			  // update all the views
			  pDoc->UpdateAllViews(NULL);
            }
          }
		}

        if (GetTool() == ID_EDIT_MODE_BUTTON)
        {
          if (TextureEditing)
          {
			// Done selecting a face...So pull up a dialog box to select a texture
			AssignTexture	material_dialog;

			if (pDoc)
			{
              if (pDoc->pWorldBsp->APolyIsSelected())	// Check for valid selection too
              {
                CBspNode* node = pDoc->pWorldBsp->GetSelectedNode();

				// DPW - Added new dialog to view material list
				material_dialog.material_count = pDoc->GetMaterialList()->getMaterialsCount();
				material_dialog.material_list = pDoc->GetMaterialList();
				material_dialog.palette = pDoc->GetPalette();

                material_dialog.parentview = this;
                material_dialog.bsptree = pDoc->pWorldBsp;
                material_dialog.node = node;
                material_dialog.topnode = pDoc->pWorldBsp->TreeTop;
                material_dialog.pDoc = pDoc;

                if (node->mBrushPtr)
                {
                  material_dialog.SetIndex(node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureID);
                }


               // assign it
               if(ShiftIsDown)
               {
                  CMainFrame * pFrame = (CMainFrame*)AfxGetMainWnd();
                  int index = pFrame->m_wndTextureView.getMaterialIndex();
                  if(index != -1)
                  {
                     material_dialog.TextureTheNodes(pDoc->pWorldBsp->TreeTop,
                        node->mBrushPtr, node->mPolygonNumber, index);
                  }
                  pDoc->SetModifiedFlag();
               }
               else if (material_dialog.DoModal() == IDOK)
                {
                  	pDoc->SetModifiedFlag();
                }
              }
			}

            TextureEditing = FALSE;
          }

          if (showSelectRect)
          {
            int x1, y1, x2, y2;
            int tlx, tly, brx, bry;

            x1 = selectRect.TopLeft().x;
            y1 = selectRect.TopLeft().y;
            x2 = selectRect.BottomRight().x;
            y2 = selectRect.BottomRight().y;

            if (x1 > x2)
            {
              tlx = x2;
              brx = x1;
            } else {
              tlx = x1;
              brx = x2;
            }

            if (y1 > y2)
            {
              tly = y2;
              bry = y1;
            } else {
              tly = y1;
              bry = y2;
            }

            //selectRect.SetRect(selectRect.TopLeft().x, selectRect.TopLeft().y, point.x, point.y);
            selectRect.SetRect(tlx, tly, brx, bry);

            // Select all the brushes within the bounds of the selection rectangle
            pDoc->SelectBrushes(selectRect, mCamera, TRUE);

            // Turn off the selection rectangle
            showSelectRect = FALSE;

            if (pDoc->mCurrentBrush == &(pDoc->mRegularBrush))
            {
               // Enable +,- toolbar buttons
               CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
                        
               pFrame->m_wndBrushToolBar.GetToolBarCtrl().SetState(ID_BRUSH_ADDTOWORLD, TBSTATE_ENABLED);
               pFrame->m_wndBrushToolBar.GetToolBarCtrl().SetState(ID_BRUSH_SUBTRACTFROMWORLD, TBSTATE_ENABLED);
            } else {
               // Disable +,- toolbar buttons
               CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
                        
               pFrame->m_wndBrushToolBar.GetToolBarCtrl().Indeterminate(ID_BRUSH_ADDTOWORLD, TRUE);
               pFrame->m_wndBrushToolBar.GetToolBarCtrl().Indeterminate(ID_BRUSH_SUBTRACTFROMWORLD, TRUE);
            }
          }

          if ((mViewType == ID_VIEW_SIDEVIEW) ||
              (mViewType == ID_VIEW_FRONTVIEW) ||
              (mViewType == ID_VIEW_TOPVIEW))
          {
            switch (m_LeftOp)
            {
            case BBOX_PICK_NONE:
              break;
            case BBOX_PICK_CENTER:
              // If the shift key is down we are trying to select a single brush
              //if (ShiftIsDown)
              //{
                // Trying to pick a single brush...
                //pDoc->SelectABrushIndependent(point, mCamera);
              //} else {
                pDoc->DoneMoveCurrentBrushes(mViewType);
                pDoc->BuildEditBox(TRUE);
              //}
              break;
            case BBOX_PICK_BRUSH_CENTER:
              // Snap moving brush center point
              //pDoc->DoneMoveBrushCenter(mCamera.mGridSize);
              pDoc->DoneMoveEditBoxCenter();
              pDoc->SetCurrentBrushesCenters();
              break;
            case BBOX_PICK_RIGHT:
              switch(mViewType)
              {
                case ID_VIEW_TOPVIEW:
                  if (pDoc->edit_state == ES_RESIZE_MOVE)
                  {
                    width = (pDoc->EditBox.mLargest.X - pDoc->EditBox.mSmallest.X);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((width / mCamera.mGridSize) + 0.5);
                      //  resize_delta.x = ((snapamt * mCamera.mGridSize) - width);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mLargest.X / mCamera.mGridSize) + 0.5);
                        resize_delta.x = (snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.X;
                      //}
                      min_bound.x = pDoc->EditBox.mSmallest.X;
                      min_bound.y = pDoc->EditBox.mSmallest.Y;
                      min_bound.z = pDoc->EditBox.mSmallest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                    }
                    //pDoc->mCurrentBrush->DoneResize(pDoc->mGridSize);
                  }

                  if ((pDoc->edit_state == ES_SHEAR) && (pDoc->mUseGrid))
                  {
                    width = pDoc->EditBox.mLargest.X - pDoc->EditBox.mSmallest.X;

                    snapamt = floor((pDoc->EditBox.mLargest.Z / mCamera.mGridSize) + 0.5);
                    resize_delta.z = ((snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.Z) / width;
                    min_bound.x = pDoc->EditBox.mSmallest.X;
                    min_bound.y = pDoc->EditBox.mSmallest.Y;
                    min_bound.z = pDoc->EditBox.mSmallest.Z;
                
                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, ZX_SHEAR);

                    snapamt = floor((pDoc->EditBox.mSmallest.Z / mCamera.mGridSize) + 0.5);
                    resize_delta.z = ((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.Z) / width;
                    min_bound.x = pDoc->EditBox.mSmallest.X;
                    min_bound.y = pDoc->EditBox.mSmallest.Y;
                    min_bound.z = pDoc->EditBox.mSmallest.Z;
                
                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, ZX_SHEAR);
                  }

                  pDoc->BuildEditBox(FALSE);
                  break;
                case ID_VIEW_FRONTVIEW:
                  if (pDoc->edit_state == ES_RESIZE_MOVE)
                  {
                    width = (pDoc->EditBox.mLargest.X - pDoc->EditBox.mSmallest.X);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((width / mCamera.mGridSize) + 0.5);
                      //  resize_delta.x = ((snapamt * mCamera.mGridSize) - width);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mLargest.X / mCamera.mGridSize) + 0.5);
                        resize_delta.x = (snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.X;
                      //}
                      min_bound.x = pDoc->EditBox.mSmallest.X;
                      min_bound.y = pDoc->EditBox.mSmallest.Y;
                      min_bound.z = pDoc->EditBox.mSmallest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                    }
                    //pDoc->mCurrentBrush->DoneResize(pDoc->mGridSize);
                  }

                  if ((pDoc->edit_state == ES_SHEAR) && (pDoc->mUseGrid))
                  {
                    width = pDoc->EditBox.mLargest.X - pDoc->EditBox.mSmallest.X;

                    snapamt = floor((pDoc->EditBox.mLargest.Y / mCamera.mGridSize) + 0.5);
                    resize_delta.y = ((snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.Y) / width;
                    min_bound.x = pDoc->EditBox.mSmallest.X;
                    min_bound.y = pDoc->EditBox.mSmallest.Y;
                    min_bound.z = pDoc->EditBox.mSmallest.Z;
                
                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, YX_SHEAR);

                    snapamt = floor((pDoc->EditBox.mSmallest.Y / mCamera.mGridSize) + 0.5);
                    resize_delta.y = ((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.Y) / width;
                    min_bound.x = pDoc->EditBox.mSmallest.X;
                    min_bound.y = pDoc->EditBox.mSmallest.Y;
                    min_bound.z = pDoc->EditBox.mSmallest.Z;
                
                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, YX_SHEAR);
                  }

                  pDoc->BuildEditBox(FALSE);
                  break;
                case ID_VIEW_SIDEVIEW:
                  if (pDoc->edit_state == ES_RESIZE_MOVE)
                  {
                    width = (pDoc->EditBox.mLargest.Z - pDoc->EditBox.mSmallest.Z);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((width / mCamera.mGridSize) + 0.5);
                      //  resize_delta.z = ((snapamt * mCamera.mGridSize) - width);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mLargest.Z / mCamera.mGridSize) + 0.5);
                        resize_delta.z = (snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.Z;
                      //}
                      min_bound.x = pDoc->EditBox.mSmallest.X;
                      min_bound.y = pDoc->EditBox.mSmallest.Y;
                      min_bound.z = pDoc->EditBox.mSmallest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                    }
                    //pDoc->mCurrentBrush->DoneResize(pDoc->mGridSize);
                  }

                  if ((pDoc->edit_state == ES_SHEAR) && (pDoc->mUseGrid))
                  {
                    width = pDoc->EditBox.mLargest.Z - pDoc->EditBox.mSmallest.Z;

                    snapamt = floor((pDoc->EditBox.mLargest.Y / mCamera.mGridSize) + 0.5);
                    resize_delta.y = ((snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.Y) / width;
                    min_bound.x = pDoc->EditBox.mSmallest.X;
                    min_bound.y = pDoc->EditBox.mSmallest.Y;
                    min_bound.z = pDoc->EditBox.mSmallest.Z;
                
                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, YZ_SHEAR);

                    snapamt = floor((pDoc->EditBox.mSmallest.Y / mCamera.mGridSize) + 0.5);
                    resize_delta.y = ((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.Y) / width;
                    min_bound.x = pDoc->EditBox.mSmallest.X;
                    min_bound.y = pDoc->EditBox.mSmallest.Y;
                    min_bound.z = pDoc->EditBox.mSmallest.Z;
                
                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, YZ_SHEAR);
                  }

                  pDoc->BuildEditBox(FALSE);
                  break;
              }
              pDoc->UpdateBrushInfoSIZE(pDoc->EditBox.mSize);
              break;
            case BBOX_PICK_LEFT:
              switch(mViewType)
              {
                case ID_VIEW_TOPVIEW:
                  if (pDoc->edit_state == ES_RESIZE_MOVE)
                  {
                    width = (pDoc->EditBox.mLargest.X - pDoc->EditBox.mSmallest.X);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((width / mCamera.mGridSize) + 0.5);
                      //  resize_delta.x = ((snapamt * mCamera.mGridSize) - width);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mSmallest.X / mCamera.mGridSize) + 0.5);
                        resize_delta.x = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.X);
                      //}
                      min_bound.x = pDoc->EditBox.mLargest.X;
                      min_bound.y = pDoc->EditBox.mLargest.Y;
                      min_bound.z = pDoc->EditBox.mLargest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                    }
                    //pDoc->mCurrentBrush->DoneResize(pDoc->mGridSize);
                  }

                  if ((pDoc->edit_state == ES_SHEAR) && (pDoc->mUseGrid))
                  {
                    width = pDoc->EditBox.mLargest.X - pDoc->EditBox.mSmallest.X;

                    snapamt = floor((pDoc->EditBox.mLargest.Z / mCamera.mGridSize) + 0.5);
                    resize_delta.z = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.Z) / width;
                    min_bound.x = pDoc->EditBox.mLargest.X;
                    min_bound.y = pDoc->EditBox.mLargest.Y;
                    min_bound.z = pDoc->EditBox.mLargest.Z;
                
                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, ZX_SHEAR);

                    snapamt = floor((pDoc->EditBox.mSmallest.Z / mCamera.mGridSize) + 0.5);
                    resize_delta.z = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.Z) / width;
                    min_bound.x = pDoc->EditBox.mLargest.X;
                    min_bound.y = pDoc->EditBox.mLargest.Y;
                    min_bound.z = pDoc->EditBox.mLargest.Z;
                
                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, ZX_SHEAR);
                  }

                  pDoc->BuildEditBox(FALSE);
                  break;
                case ID_VIEW_FRONTVIEW:
                  if (pDoc->edit_state == ES_RESIZE_MOVE)
                  {
                    width = (pDoc->EditBox.mLargest.X - pDoc->EditBox.mSmallest.X);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((width / mCamera.mGridSize) + 0.5);
                      //  resize_delta.x = ((snapamt * mCamera.mGridSize) - width);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mSmallest.X / mCamera.mGridSize) + 0.5);
                        resize_delta.x = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.X);
                      //}
                      min_bound.x = pDoc->EditBox.mLargest.X;
                      min_bound.y = pDoc->EditBox.mLargest.Y;
                      min_bound.z = pDoc->EditBox.mLargest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                    }
                    //pDoc->mCurrentBrush->DoneResize(pDoc->mGridSize);
                  }

                  if ((pDoc->edit_state == ES_SHEAR) && (pDoc->mUseGrid))
                  {
                    width = pDoc->EditBox.mLargest.X - pDoc->EditBox.mSmallest.X;

                    snapamt = floor((pDoc->EditBox.mLargest.Y / mCamera.mGridSize) + 0.5);
                    resize_delta.y = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.Y) / width;
                    min_bound.x = pDoc->EditBox.mLargest.X;
                    min_bound.y = pDoc->EditBox.mLargest.Y;
                    min_bound.z = pDoc->EditBox.mLargest.Z;
                
                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, YX_SHEAR);

                    snapamt = floor((pDoc->EditBox.mSmallest.Y / mCamera.mGridSize) + 0.5);
                    resize_delta.y = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.Y) / width;
                    min_bound.x = pDoc->EditBox.mLargest.X;
                    min_bound.y = pDoc->EditBox.mLargest.Y;
                    min_bound.z = pDoc->EditBox.mLargest.Z;
                
                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, YX_SHEAR);
                  }

                  pDoc->BuildEditBox(FALSE);
                  break;
                case ID_VIEW_SIDEVIEW:
                  if (pDoc->edit_state == ES_RESIZE_MOVE)
                  {
                    width = (pDoc->EditBox.mLargest.Z - pDoc->EditBox.mSmallest.Z);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((width / mCamera.mGridSize) + 0.5);
                      //  resize_delta.z = ((snapamt * mCamera.mGridSize) - width);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mSmallest.Z / mCamera.mGridSize) + 0.5);
                        resize_delta.z = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.Z);
                      //}
                      min_bound.x = pDoc->EditBox.mLargest.X;
                      min_bound.y = pDoc->EditBox.mLargest.Y;
                      min_bound.z = pDoc->EditBox.mLargest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                    }
                    //pDoc->mCurrentBrush->DoneResize(pDoc->mGridSize);
                  }

                  if ((pDoc->edit_state == ES_SHEAR) && (pDoc->mUseGrid))
                  {
                    width = pDoc->EditBox.mLargest.Z - pDoc->EditBox.mSmallest.Z;

                    snapamt = floor((pDoc->EditBox.mLargest.Y / mCamera.mGridSize) + 0.5);
                    resize_delta.y = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.Y) / width;
                    min_bound.x = pDoc->EditBox.mLargest.X;
                    min_bound.y = pDoc->EditBox.mLargest.Y;
                    min_bound.z = pDoc->EditBox.mLargest.Z;
                
                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, YZ_SHEAR);

                    snapamt = floor((pDoc->EditBox.mSmallest.Y / mCamera.mGridSize) + 0.5);
                    resize_delta.y = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.Y) / width;
                    min_bound.x = pDoc->EditBox.mLargest.X;
                    min_bound.y = pDoc->EditBox.mLargest.Y;
                    min_bound.z = pDoc->EditBox.mLargest.Z;
                
                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, YZ_SHEAR);
                  }

                  pDoc->BuildEditBox(FALSE);
                  break;
              }
              pDoc->UpdateBrushInfoSIZE(pDoc->EditBox.mSize);
              break;
            case BBOX_PICK_TOP:
              switch(mViewType)
              {
                case ID_VIEW_TOPVIEW:
                  if (pDoc->edit_state == ES_RESIZE_MOVE)
                  {
                    height = (pDoc->EditBox.mLargest.Z - pDoc->EditBox.mSmallest.Z);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((height / mCamera.mGridSize) + 0.5);
                      //  resize_delta.z = ((snapamt * mCamera.mGridSize) - height);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mLargest.Z / mCamera.mGridSize) + 0.5);
                        resize_delta.z = (snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.Z;
                      //}
                      min_bound.x = pDoc->EditBox.mSmallest.X;
                      min_bound.y = pDoc->EditBox.mSmallest.Y;
                      min_bound.z = pDoc->EditBox.mSmallest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                    }
                    //pDoc->mCurrentBrush->DoneResize(pDoc->mGridSize);
                  }

                  if ((pDoc->edit_state == ES_SHEAR) && (pDoc->mUseGrid))
                  {
                    height = pDoc->EditBox.mLargest.Z - pDoc->EditBox.mSmallest.Z;

                    min_bound.x = pDoc->EditBox.mSmallest.X;
                    min_bound.y = pDoc->EditBox.mSmallest.Y;
                    min_bound.z = pDoc->EditBox.mSmallest.Z;

                    snapamt = floor((pDoc->EditBox.mLargest.X / mCamera.mGridSize) + 0.5);
                    resize_delta.x = ((snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.X) / height;

                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, XZ_SHEAR);

                    min_bound.x = pDoc->EditBox.mSmallest.X;
                    min_bound.y = pDoc->EditBox.mSmallest.Y;
                    min_bound.z = pDoc->EditBox.mSmallest.Z;

                    snapamt = floor((pDoc->EditBox.mSmallest.X / mCamera.mGridSize) + 0.5);
                    resize_delta.x = ((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.X) / height;

                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, XZ_SHEAR);
                  }

                  pDoc->BuildEditBox(FALSE);
                  break;
                case ID_VIEW_FRONTVIEW:
                  if (pDoc->edit_state == ES_RESIZE_MOVE)
                  {
                    height = (pDoc->EditBox.mLargest.Y - pDoc->EditBox.mSmallest.Y);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((height / mCamera.mGridSize) + 0.5);
                      //  resize_delta.y = ((snapamt * mCamera.mGridSize) - height);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mLargest.Y / mCamera.mGridSize) + 0.5);
                        resize_delta.y = (snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.Y;
                      //}
                      min_bound.x = pDoc->EditBox.mSmallest.X;
                      min_bound.y = pDoc->EditBox.mSmallest.Y;
                      min_bound.z = pDoc->EditBox.mSmallest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                    }
                    //pDoc->mCurrentBrush->DoneResize(pDoc->mGridSize);
                  }

                  if ((pDoc->edit_state == ES_SHEAR) && (pDoc->mUseGrid))
                  {
                    height = pDoc->EditBox.mLargest.Y - pDoc->EditBox.mSmallest.Y;

                    min_bound.x = pDoc->EditBox.mSmallest.X;
                    min_bound.y = pDoc->EditBox.mSmallest.Y;
                    min_bound.z = pDoc->EditBox.mSmallest.Z;

                    snapamt = floor((pDoc->EditBox.mLargest.X / mCamera.mGridSize) + 0.5);
                    resize_delta.x = ((snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.X) / height;

                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, XY_SHEAR);

                    min_bound.x = pDoc->EditBox.mSmallest.X;
                    min_bound.y = pDoc->EditBox.mSmallest.Y;
                    min_bound.z = pDoc->EditBox.mSmallest.Z;

                    snapamt = floor((pDoc->EditBox.mSmallest.X / mCamera.mGridSize) + 0.5);
                    resize_delta.x = ((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.X) / height;

                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, XY_SHEAR);
                  }

                  pDoc->BuildEditBox(FALSE);
                  break;
                case ID_VIEW_SIDEVIEW:
                  if (pDoc->edit_state == ES_RESIZE_MOVE)
                  {
                    height = (pDoc->EditBox.mLargest.Y - pDoc->EditBox.mSmallest.Y);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((height / mCamera.mGridSize) + 0.5);
                      //  resize_delta.y = ((snapamt * mCamera.mGridSize) - height);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mLargest.Y / mCamera.mGridSize) + 0.5);
                        resize_delta.y = (snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.Y;
                      //}
                      min_bound.x = pDoc->EditBox.mSmallest.X;
                      min_bound.y = pDoc->EditBox.mSmallest.Y;
                      min_bound.z = pDoc->EditBox.mSmallest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                    }
                    //pDoc->mCurrentBrush->DoneResize(pDoc->mGridSize);
                  }

                  if ((pDoc->edit_state == ES_SHEAR) && (pDoc->mUseGrid))
                  {
                    height = pDoc->EditBox.mLargest.Y - pDoc->EditBox.mSmallest.Y;

                    min_bound.x = pDoc->EditBox.mSmallest.X;
                    min_bound.y = pDoc->EditBox.mSmallest.Y;
                    min_bound.z = pDoc->EditBox.mSmallest.Z;

                    snapamt = floor((pDoc->EditBox.mLargest.Z / mCamera.mGridSize) + 0.5);
                    resize_delta.z = ((snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.Z) / height;

                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, ZY_SHEAR);

                    min_bound.x = pDoc->EditBox.mSmallest.X;
                    min_bound.y = pDoc->EditBox.mSmallest.Y;
                    min_bound.z = pDoc->EditBox.mSmallest.Z;

                    snapamt = floor((pDoc->EditBox.mSmallest.Z / mCamera.mGridSize) + 0.5);
                    resize_delta.z = ((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.Z) / height;

                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, ZY_SHEAR);
                  }

                  pDoc->BuildEditBox(FALSE);
                  break;
              }
              pDoc->UpdateBrushInfoSIZE(pDoc->EditBox.mSize);
              break;
            case BBOX_PICK_BOTTOM:
              switch(mViewType)
              {
                case ID_VIEW_TOPVIEW:
                  if (pDoc->edit_state == ES_RESIZE_MOVE)
                  {
                    height = (pDoc->EditBox.mLargest.Z - pDoc->EditBox.mSmallest.Z);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((height / mCamera.mGridSize) + 0.5);
                      //  resize_delta.z = ((snapamt * mCamera.mGridSize) - height);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mSmallest.Z / mCamera.mGridSize) + 0.5);
                        resize_delta.z = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.Z);
                      //}
                      min_bound.x = pDoc->EditBox.mLargest.X;
                      min_bound.y = pDoc->EditBox.mLargest.Y;
                      min_bound.z = pDoc->EditBox.mLargest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                    }
                    //pDoc->mCurrentBrush->DoneResize(pDoc->mGridSize);
                  }

                  if ((pDoc->edit_state == ES_SHEAR) && (pDoc->mUseGrid))
                  {
                    height = pDoc->EditBox.mLargest.Z - pDoc->EditBox.mSmallest.Z;

                    min_bound.x = pDoc->EditBox.mLargest.X;
                    min_bound.y = pDoc->EditBox.mLargest.Y;
                    min_bound.z = pDoc->EditBox.mLargest.Z;

                    snapamt = floor((pDoc->EditBox.mLargest.X / mCamera.mGridSize) + 0.5);
                    resize_delta.x = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.X) / height;

                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, XZ_SHEAR);

                    min_bound.x = pDoc->EditBox.mLargest.X;
                    min_bound.y = pDoc->EditBox.mLargest.Y;
                    min_bound.z = pDoc->EditBox.mLargest.Z;

                    snapamt = floor((pDoc->EditBox.mSmallest.X / mCamera.mGridSize) + 0.5);
                    resize_delta.x = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.X) / height;

                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, XZ_SHEAR);
                  }

                  pDoc->BuildEditBox(FALSE);
                  break;
                case ID_VIEW_FRONTVIEW:
                  if (pDoc->edit_state == ES_RESIZE_MOVE)
                  {
                    height = (pDoc->EditBox.mLargest.Y - pDoc->EditBox.mSmallest.Y);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((height / mCamera.mGridSize) + 0.5);
                      //  resize_delta.y = ((snapamt * mCamera.mGridSize) - height);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mSmallest.Y / mCamera.mGridSize) + 0.5);
                        resize_delta.y = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.Y);
                      //}
                      min_bound.x = pDoc->EditBox.mLargest.X;
                      min_bound.y = pDoc->EditBox.mLargest.Y;
                      min_bound.z = pDoc->EditBox.mLargest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                    }
                    //pDoc->mCurrentBrush->DoneResize(pDoc->mGridSize);
                  }

                  if ((pDoc->edit_state == ES_SHEAR) && (pDoc->mUseGrid))
                  {
                    height = pDoc->EditBox.mLargest.Y - pDoc->EditBox.mSmallest.Y;

                    min_bound.x = pDoc->EditBox.mLargest.X;
                    min_bound.y = pDoc->EditBox.mLargest.Y;
                    min_bound.z = pDoc->EditBox.mLargest.Z;

                    snapamt = floor((pDoc->EditBox.mLargest.X / mCamera.mGridSize) + 0.5);
                    resize_delta.x = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.X) / height;

                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, XY_SHEAR);

                    min_bound.x = pDoc->EditBox.mLargest.X;
                    min_bound.y = pDoc->EditBox.mLargest.Y;
                    min_bound.z = pDoc->EditBox.mLargest.Z;

                    snapamt = floor((pDoc->EditBox.mSmallest.X / mCamera.mGridSize) + 0.5);
                    resize_delta.x = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.X) / height;

                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, XY_SHEAR);
                  }

                  pDoc->BuildEditBox(FALSE);
                  break;
                case ID_VIEW_SIDEVIEW:
                  if (pDoc->edit_state == ES_RESIZE_MOVE)
                  {
                    height = (pDoc->EditBox.mLargest.Y - pDoc->EditBox.mSmallest.Y);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((height / mCamera.mGridSize) + 0.5);
                      //  resize_delta.y = ((snapamt * mCamera.mGridSize) - height);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mSmallest.Y / mCamera.mGridSize) + 0.5);
                        resize_delta.y = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.Y);
                      //}
                      min_bound.x = pDoc->EditBox.mLargest.X;
                      min_bound.y = pDoc->EditBox.mLargest.Y;
                      min_bound.z = pDoc->EditBox.mLargest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                    }
                    //pDoc->mCurrentBrush->DoneResize(pDoc->mGridSize);
                  }

                  if ((pDoc->edit_state == ES_SHEAR) && (pDoc->mUseGrid))
                  {
                    height = pDoc->EditBox.mLargest.Y - pDoc->EditBox.mSmallest.Y;

                    min_bound.x = pDoc->EditBox.mLargest.X;
                    min_bound.y = pDoc->EditBox.mLargest.Y;
                    min_bound.z = pDoc->EditBox.mLargest.Z;

                    snapamt = floor((pDoc->EditBox.mLargest.Z / mCamera.mGridSize) + 0.5);
                    resize_delta.z = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.Z) / height;

                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, ZY_SHEAR);

                    min_bound.x = pDoc->EditBox.mLargest.X;
                    min_bound.y = pDoc->EditBox.mLargest.Y;
                    min_bound.z = pDoc->EditBox.mLargest.Z;

                    snapamt = floor((pDoc->EditBox.mSmallest.Z / mCamera.mGridSize) + 0.5);
                    resize_delta.z = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.Z) / height;

                    pDoc->ShearCurrentBrushes(&min_bound, &resize_delta, ZY_SHEAR);
                  }

                  pDoc->BuildEditBox(FALSE);
                  break;
              }
              pDoc->UpdateBrushInfoSIZE(pDoc->EditBox.mSize);
              break;
            case BBOX_PICK_TOP_LEFT:
              if (pDoc->edit_state == ES_ROTATE)
              {
                pDoc->DoneRotateCurrentBrushes();
                pDoc->BuildEditBox(FALSE);
              }

              if (pDoc->edit_state == ES_RESIZE_MOVE)
              {
                switch(mViewType)
                {
                  case ID_VIEW_TOPVIEW:
                    height = (pDoc->EditBox.mLargest.Z - pDoc->EditBox.mSmallest.Z);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((height / mCamera.mGridSize) + 0.5);
                      //  resize_delta.z = ((snapamt * mCamera.mGridSize) - height);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mLargest.Z / mCamera.mGridSize) + 0.5);
                        resize_delta.z = (snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.Z;
                      //}
                      min_bound.x = pDoc->EditBox.mSmallest.X;
                      min_bound.y = pDoc->EditBox.mSmallest.Y;
                      min_bound.z = pDoc->EditBox.mSmallest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      resize_delta.z = 0.0;

                      width = (pDoc->EditBox.mLargest.X - pDoc->EditBox.mSmallest.X);

                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((width / mCamera.mGridSize) + 0.5);
                      //  resize_delta.x = ((snapamt * mCamera.mGridSize) - width);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mSmallest.X / mCamera.mGridSize) + 0.5);
                        resize_delta.x = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.X);
                      //}
                      min_bound.x = pDoc->EditBox.mLargest.X;
                      min_bound.y = pDoc->EditBox.mLargest.Y;
                      min_bound.z = pDoc->EditBox.mLargest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      pDoc->BuildEditBox(FALSE);
                    }
                    break;
                  case ID_VIEW_FRONTVIEW:
                    height = (pDoc->EditBox.mLargest.Y - pDoc->EditBox.mSmallest.Y);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((height / mCamera.mGridSize) + 0.5);
                      //  resize_delta.y = ((snapamt * mCamera.mGridSize) - height);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mLargest.Y / mCamera.mGridSize) + 0.5);
                        resize_delta.y = (snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.Y;
                      //}
                      min_bound.x = pDoc->EditBox.mSmallest.X;
                      min_bound.y = pDoc->EditBox.mSmallest.Y;
                      min_bound.z = pDoc->EditBox.mSmallest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      resize_delta.y = 0.0;

                      width = (pDoc->EditBox.mLargest.X - pDoc->EditBox.mSmallest.X);
                    
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((width / mCamera.mGridSize) + 0.5);
                      //  resize_delta.x = ((snapamt * mCamera.mGridSize) - width);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mSmallest.X / mCamera.mGridSize) + 0.5);
                        resize_delta.x = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.X);
                      //}
                      min_bound.x = pDoc->EditBox.mLargest.X;
                      min_bound.y = pDoc->EditBox.mLargest.Y;
                      min_bound.z = pDoc->EditBox.mLargest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      pDoc->BuildEditBox(FALSE);
                    }
                    break;
                  case ID_VIEW_SIDEVIEW:
                    height = (pDoc->EditBox.mLargest.Y - pDoc->EditBox.mSmallest.Y);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((height / mCamera.mGridSize) + 0.5);
                      //  resize_delta.y = ((snapamt * mCamera.mGridSize) - height);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mLargest.Y / mCamera.mGridSize) + 0.5);
                        resize_delta.y = (snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.Y;
                      //}
                      min_bound.x = pDoc->EditBox.mSmallest.X;
                      min_bound.y = pDoc->EditBox.mSmallest.Y;
                      min_bound.z = pDoc->EditBox.mSmallest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      resize_delta.y = 0.0;

                      width = (pDoc->EditBox.mLargest.Z - pDoc->EditBox.mSmallest.Z);

                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((width / mCamera.mGridSize) + 0.5);
                      //  resize_delta.z = ((snapamt * mCamera.mGridSize) - width);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mSmallest.Z / mCamera.mGridSize) + 0.5);
                        resize_delta.z = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.Z);
                      //}
                      min_bound.x = pDoc->EditBox.mLargest.X;
                      min_bound.y = pDoc->EditBox.mLargest.Y;
                      min_bound.z = pDoc->EditBox.mLargest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      pDoc->BuildEditBox(FALSE);
                    }
                    break;
                }
                pDoc->mCurrentBrush->DoneResize(pDoc->mGridSize);
                pDoc->UpdateBrushInfoSIZE(pDoc->EditBox.mSize);
              }
              break;
            case BBOX_PICK_TOP_RIGHT:
              if (pDoc->edit_state == ES_ROTATE)
              {
                pDoc->DoneRotateCurrentBrushes();
                pDoc->BuildEditBox(FALSE);
              }

              if (pDoc->edit_state == ES_RESIZE_MOVE)
              {
                switch(mViewType)
                {
                  case ID_VIEW_TOPVIEW:
                    height = (pDoc->EditBox.mLargest.Z - pDoc->EditBox.mSmallest.Z);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((height / mCamera.mGridSize) + 0.5);
                      //  resize_delta.z = ((snapamt * mCamera.mGridSize) - height);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mLargest.Z / mCamera.mGridSize) + 0.5);
                        resize_delta.z = (snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.Z;
                      //}
                      min_bound.x = pDoc->EditBox.mSmallest.X;
                      min_bound.y = pDoc->EditBox.mSmallest.Y;
                      min_bound.z = pDoc->EditBox.mSmallest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      resize_delta.z = 0.0;

                      width = (pDoc->EditBox.mLargest.X - pDoc->EditBox.mSmallest.X);

                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((width / mCamera.mGridSize) + 0.5);
                      //  resize_delta.x = ((snapamt * mCamera.mGridSize) - width);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mLargest.X / mCamera.mGridSize) + 0.5);
                        resize_delta.x = (snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.X;
                      //}
                      min_bound.x = pDoc->EditBox.mSmallest.X;
                      min_bound.y = pDoc->EditBox.mSmallest.Y;
                      min_bound.z = pDoc->EditBox.mSmallest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      pDoc->BuildEditBox(FALSE);
                    }
                    break;
                  case ID_VIEW_FRONTVIEW:
                    height = (pDoc->EditBox.mLargest.Y - pDoc->EditBox.mSmallest.Y);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((height / mCamera.mGridSize) + 0.5);
                      //  resize_delta.y = ((snapamt * mCamera.mGridSize) - height);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mLargest.Y / mCamera.mGridSize) + 0.5);
                        resize_delta.y = (snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.Y;
                      //}
                      min_bound.x = pDoc->EditBox.mSmallest.X;
                      min_bound.y = pDoc->EditBox.mSmallest.Y;
                      min_bound.z = pDoc->EditBox.mSmallest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      resize_delta.y = 0.0;

                      width = (pDoc->EditBox.mLargest.X - pDoc->EditBox.mSmallest.X);

                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((width / mCamera.mGridSize) + 0.5);
                      //  resize_delta.x = ((snapamt * mCamera.mGridSize) - width);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mLargest.X / mCamera.mGridSize) + 0.5);
                        resize_delta.x = (snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.X;
                      //}
                      min_bound.x = pDoc->EditBox.mSmallest.X;
                      min_bound.y = pDoc->EditBox.mSmallest.Y;
                      min_bound.z = pDoc->EditBox.mSmallest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      pDoc->BuildEditBox(FALSE);
                    }
                    break;
                  case ID_VIEW_SIDEVIEW:
                    height = (pDoc->EditBox.mLargest.Y - pDoc->EditBox.mSmallest.Y);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((height / mCamera.mGridSize) + 0.5);
                      //  resize_delta.y = ((snapamt * mCamera.mGridSize) - height);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mLargest.Y / mCamera.mGridSize) + 0.5);
                        resize_delta.y = (snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.Y;
                      //}
                      min_bound.x = pDoc->EditBox.mSmallest.X;
                      min_bound.y = pDoc->EditBox.mSmallest.Y;
                      min_bound.z = pDoc->EditBox.mSmallest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      resize_delta.y = 0.0;

                      width = (pDoc->EditBox.mLargest.Z - pDoc->EditBox.mSmallest.Z);

                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((width / mCamera.mGridSize) + 0.5);
                      //  resize_delta.z = ((snapamt * mCamera.mGridSize) - width);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mLargest.Z / mCamera.mGridSize) + 0.5);
                        resize_delta.z = (snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.Z;
                      //}
                      min_bound.x = pDoc->EditBox.mSmallest.X;
                      min_bound.y = pDoc->EditBox.mSmallest.Y;
                      min_bound.z = pDoc->EditBox.mSmallest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      pDoc->BuildEditBox(FALSE);
                    }
                    break;
                  }
                pDoc->mCurrentBrush->DoneResize(pDoc->mGridSize);
                pDoc->UpdateBrushInfoSIZE(pDoc->EditBox.mSize);
              }
              break;
            case BBOX_PICK_BOTTOM_LEFT:
              if (pDoc->edit_state == ES_ROTATE)
              {
                pDoc->DoneRotateCurrentBrushes();
                pDoc->BuildEditBox(FALSE);
              }

              if (pDoc->edit_state == ES_RESIZE_MOVE)
              {
                switch(mViewType)
                {
                  case ID_VIEW_TOPVIEW:
                    height = (pDoc->EditBox.mLargest.Z - pDoc->EditBox.mSmallest.Z);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((height / mCamera.mGridSize) + 0.5);
                      //  resize_delta.z = ((snapamt * mCamera.mGridSize) - height);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mSmallest.Z / mCamera.mGridSize) + 0.5);
                        resize_delta.z = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.Z);
                      //}
                      min_bound.x = pDoc->EditBox.mLargest.X;
                      min_bound.y = pDoc->EditBox.mLargest.Y;
                      min_bound.z = pDoc->EditBox.mLargest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      resize_delta.z = 0.0;

                      width = (pDoc->EditBox.mLargest.X - pDoc->EditBox.mSmallest.X);

                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((width / mCamera.mGridSize) + 0.5);
                      //  resize_delta.x = ((snapamt * mCamera.mGridSize) - width);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mSmallest.X / mCamera.mGridSize) + 0.5);
                        resize_delta.x = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.X);
                      //}
                      min_bound.x = pDoc->EditBox.mLargest.X;
                      min_bound.y = pDoc->EditBox.mLargest.Y;
                      min_bound.z = pDoc->EditBox.mLargest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      pDoc->BuildEditBox(FALSE);
                    }
                    break;
                  case ID_VIEW_FRONTVIEW:
                    height = (pDoc->EditBox.mLargest.Y - pDoc->EditBox.mSmallest.Y);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((height / mCamera.mGridSize) + 0.5);
                      //  resize_delta.y = ((snapamt * mCamera.mGridSize) - height);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mSmallest.Y / mCamera.mGridSize) + 0.5);
                        resize_delta.y = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.Y);
                      //}
                      min_bound.x = pDoc->EditBox.mLargest.X;
                      min_bound.y = pDoc->EditBox.mLargest.Y;
                      min_bound.z = pDoc->EditBox.mLargest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      resize_delta.y = 0.0;

                      width = (pDoc->EditBox.mLargest.X - pDoc->EditBox.mSmallest.X);

                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((width / mCamera.mGridSize) + 0.5);
                      //  resize_delta.x = ((snapamt * mCamera.mGridSize) - width);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mSmallest.X / mCamera.mGridSize) + 0.5);
                        resize_delta.x = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.X);
                      //}
                      min_bound.x = pDoc->EditBox.mLargest.X;
                      min_bound.y = pDoc->EditBox.mLargest.Y;
                      min_bound.z = pDoc->EditBox.mLargest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      pDoc->BuildEditBox(FALSE);
                    }
                    break;
                  case ID_VIEW_SIDEVIEW:
                    height = (pDoc->EditBox.mLargest.Y - pDoc->EditBox.mSmallest.Y);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((height / mCamera.mGridSize) + 0.5);
                      //  resize_delta.y = ((snapamt * mCamera.mGridSize) - height);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mSmallest.Y / mCamera.mGridSize) + 0.5);
                        resize_delta.y = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.Y);
                      //}
                      min_bound.x = pDoc->EditBox.mLargest.X;
                      min_bound.y = pDoc->EditBox.mLargest.Y;
                      min_bound.z = pDoc->EditBox.mLargest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      resize_delta.y = 0.0;

                      width = (pDoc->EditBox.mLargest.Z - pDoc->EditBox.mSmallest.Z);

                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((width / mCamera.mGridSize) + 0.5);
                      //  resize_delta.z = ((snapamt * mCamera.mGridSize) - width);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mSmallest.Z / mCamera.mGridSize) + 0.5);
                        resize_delta.z = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.Z);
                      //}
                        min_bound.x = pDoc->EditBox.mLargest.X;
                        min_bound.y = pDoc->EditBox.mLargest.Y;
                        min_bound.z = pDoc->EditBox.mLargest.Z;
                        pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                        pDoc->BuildEditBox(FALSE);
                    }
                    break;
                }
                pDoc->mCurrentBrush->DoneResize(pDoc->mGridSize);
                pDoc->UpdateBrushInfoSIZE(pDoc->EditBox.mSize);
              }
              break;
            case BBOX_PICK_BOTTOM_RIGHT:
              if (pDoc->edit_state == ES_ROTATE)
              {
                pDoc->DoneRotateCurrentBrushes();
                pDoc->BuildEditBox(FALSE);
              }

              if (pDoc->edit_state == ES_RESIZE_MOVE)
              {
                switch(mViewType)
                {
                  case ID_VIEW_TOPVIEW:
                    height = (pDoc->EditBox.mLargest.Z - pDoc->EditBox.mSmallest.Z);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((height / mCamera.mGridSize) + 0.5);
                      //  resize_delta.z = ((snapamt * mCamera.mGridSize) - height);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mSmallest.Z / mCamera.mGridSize) + 0.5);
                        resize_delta.z = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.Z);
                      //}
                      min_bound.x = pDoc->EditBox.mLargest.X;
                      min_bound.y = pDoc->EditBox.mLargest.Y;
                      min_bound.z = pDoc->EditBox.mLargest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      resize_delta.z = 0.0;

                      width = (pDoc->EditBox.mLargest.X - pDoc->EditBox.mSmallest.X);

                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((width / mCamera.mGridSize) + 0.5);
                      //  resize_delta.x = ((snapamt * mCamera.mGridSize) - width);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mLargest.X / mCamera.mGridSize) + 0.5);
                        resize_delta.x = (snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.X;
                      //}
                      min_bound.x = pDoc->EditBox.mSmallest.X;
                      min_bound.y = pDoc->EditBox.mSmallest.Y;
                      min_bound.z = pDoc->EditBox.mSmallest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      pDoc->BuildEditBox(FALSE);
                    }
                    break;
                  case ID_VIEW_FRONTVIEW:
                    height = (pDoc->EditBox.mLargest.Y - pDoc->EditBox.mSmallest.Y);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((height / mCamera.mGridSize) + 0.5);
                      //  resize_delta.y = ((snapamt * mCamera.mGridSize) - height);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mSmallest.Y / mCamera.mGridSize) + 0.5);
                        resize_delta.y = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.Y);
                      //}
                      min_bound.x = pDoc->EditBox.mLargest.X;
                      min_bound.y = pDoc->EditBox.mLargest.Y;
                      min_bound.z = pDoc->EditBox.mLargest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      resize_delta.y = 0.0;

                      width = (pDoc->EditBox.mLargest.X - pDoc->EditBox.mSmallest.X);

                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((width / mCamera.mGridSize) + 0.5);
                      //  resize_delta.x = ((snapamt * mCamera.mGridSize) - width);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mLargest.X / mCamera.mGridSize) + 0.5);
                        resize_delta.x = (snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.X;
                      //}
                      min_bound.x = pDoc->EditBox.mSmallest.X;
                      min_bound.y = pDoc->EditBox.mSmallest.Y;
                      min_bound.z = pDoc->EditBox.mSmallest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      pDoc->BuildEditBox(FALSE);
                    }
                    break;
                  case ID_VIEW_SIDEVIEW:
                    height = (pDoc->EditBox.mLargest.Y - pDoc->EditBox.mSmallest.Y);

                    if (pDoc->mUseGrid)
                    {
                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((height / mCamera.mGridSize) + 0.5);
                      //  resize_delta.y = ((snapamt * mCamera.mGridSize) - height);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mSmallest.Y / mCamera.mGridSize) + 0.5);
                        resize_delta.y = -((snapamt * mCamera.mGridSize) - pDoc->EditBox.mSmallest.Y);
                      //}
                      min_bound.x = pDoc->EditBox.mLargest.X;
                      min_bound.y = pDoc->EditBox.mLargest.Y;
                      min_bound.z = pDoc->EditBox.mLargest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      resize_delta.y = 0.0;

                      width = (pDoc->EditBox.mLargest.Z - pDoc->EditBox.mSmallest.Z);

                      //if (pDoc->mUseRotationSnap)
                      //{
                      //  snapamt = floor((width / mCamera.mGridSize) + 0.5);
                      //  resize_delta.z = ((snapamt * mCamera.mGridSize) - width);
                      //} else {
                        snapamt = floor((pDoc->EditBox.mLargest.Z / mCamera.mGridSize) + 0.5);
                        resize_delta.z = (snapamt * mCamera.mGridSize) - pDoc->EditBox.mLargest.Z;
                      //}
                      min_bound.x = pDoc->EditBox.mSmallest.X;
                      min_bound.y = pDoc->EditBox.mSmallest.Y;
                      min_bound.z = pDoc->EditBox.mSmallest.Z;
                      pDoc->ResizeCurrentBrushes(&min_bound, &resize_delta);
                      pDoc->BuildEditBox(FALSE);
                    }
                    break;
                }
                pDoc->mCurrentBrush->DoneResize(pDoc->mGridSize);
                pDoc->UpdateBrushInfoSIZE(pDoc->EditBox.mSize);
              }
              break;
            }
          } else {
            if (AltIsDown)
            {
              // Try to select the brush...
              m_renderContext->setSurface(pBuffer);

              if (!(m_renderContext->isLocked()))
              {
                // Deselect any selected brushes, and make the regular brush current
                pDoc->mCurrentBrush = &(pDoc->mRegularBrush);
                pDoc->mAdjustmentMode = FALSE;

                // Loop through and deselect everyone...
                CThredBrush* tempbrush = pDoc->mBrushList;
                while(tempbrush)
                {
                  tempbrush->mFlags &= ~BRUSH_SELECTED;
                  tempbrush->NextSelectedBrush = NULL;
                  tempbrush->PrevSelectedBrush = NULL;

                  tempbrush = tempbrush->GetNextBrush();
                }

                pDoc->BuildEditBox(TRUE);

                m_renderContext->lock();
                m_renderContext->getSurface()->draw3DBegin();

                pDoc->SelectAPolygon(point, &mCamera, m_renderContext);

                if (pDoc->pWorldBsp->APolyIsSelected())
                {
                  CBspNode* node = pDoc->pWorldBsp->GetSelectedNode();

                  pDoc->ForceBrushSelection(node->mBrushPtr);
                }

                m_renderContext->getSurface()->draw3DEnd();
                m_renderContext->getSurface()->update();
                m_renderContext->unlock();
              }
            }
          }
          m_LeftOp = BBOX_PICK_NONE;
        }

		pDoc->UpdateAllViews(NULL);
	}
}

void CTHREDView::OnRButtonUp(UINT nFlags, CPoint point) 
{
    if (nFlags & MK_SHIFT)
    {
      ShiftIsDown = TRUE;
    } else {
      ShiftIsDown = FALSE;
    }

    if(!(nFlags & MK_LBUTTON))
    {
		// get the doc
		CTHREDDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);

		// release our capture
		if(this == GetCapture() )
      {
			ReleaseCapture();
         
         // hide the view button
         ViewButton.ShowWindow( SW_SHOW );
      }
      else
         return;   

		// make sure we tell we are done moving
		if(GetTool() == ID_TOOLS_BRUSH_MOVEROTATEBRUSH || GetTool() == ID_TOOLS_BRUSH_MOVESELECTEDBRUSHES) {
			pDoc->DoneMoveEntity();
			pDoc->DoneMoveLights( false, 0 );
			pDoc->DoneMoveBrush();
			pDoc->DoneRotateBrush();
		}

		if(GetTool() == ID_TOOLS_BRUSH_SCALEBRUSH )
			pDoc->DoneScaleBrush();

		if(GetTool() == ID_TOOLS_BRUSH_SHEARBRUSH )
			pDoc->DoneShearBrush();

		if(GetTool() == ID_BRUSH_GROUPS_SELECT ) {
			CRect NewRect(mStartPoint, point);
			NewRect.NormalizeRect();

			// select stuff
			pDoc->SelectBrushes(NewRect, mCamera, FALSE );

			// update all the views
			pDoc->UpdateAllViews(NULL);
		}
		else {

			// set cursor
//dpw			SetCursorPos(mStartPoint.x, mStartPoint.y);

			// show our mouse cursor
			ShowCursor(TRUE);

			// update all the views
			pDoc->UpdateAllViews(NULL);
		}

        if (GetTool() == ID_EDIT_MODE_BUTTON)
        {
          switch (m_RightOp)
          {
            case BBOX_PICK_CENTER:
              //pDoc->DoneRotateBrush();
              break;
            case BBOX_PICK_TOP:
            case BBOX_PICK_RIGHT:
            case BBOX_PICK_LEFT:
            case BBOX_PICK_BOTTOM:
            case BBOX_PICK_TOP_LEFT:
            case BBOX_PICK_TOP_RIGHT:
            case BBOX_PICK_BOTTOM_LEFT:
            case BBOX_PICK_BOTTOM_RIGHT:
              break;
          }
          m_RightOp = BBOX_PICK_NONE;
        }

	}
}

void CTHREDView::OnToolsBrushMoveselectedbrushes() 
{
	SetTool(ID_TOOLS_BRUSH_MOVESELECTEDBRUSHES);
}
void CTHREDView::OnUpdateToolsBrushMoveselectedbrushes(CCmdUI* pCmdUI) 
{
	if(GetTool() == ID_TOOLS_BRUSH_MOVESELECTEDBRUSHES )
		pCmdUI->SetCheck();
	else
		pCmdUI->SetCheck(0);
}


//===========================================================
// This function is a quick shortcut key that switches
// between some brushes.
//===========================================================
void CTHREDView::OnMButtonUp(UINT nFlags, CPoint point) 
{
	CView::OnMButtonUp( nFlags, point );

//	CTHREDDoc* pDoc = GetDocument();
//	ASSERT_VALID(pDoc);
//
//	// if other buttons are down get outta here
//	if((nFlags & MK_LBUTTON) || (nFlags & MK_RBUTTON) )
//		return;
//
//	// a quick shortcut to get to the camera
//	// and from the camera to the movement
//	if(GetTool() != ID_TOOLS_CAMERA ) {
//		pDoc->mOldTool = GetTool();
//		SetTool(ID_TOOLS_CAMERA );
//	}
//	else
//		SetTool(pDoc->mOldTool);
}

void CTHREDView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// get the document
	CTHREDDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// go through and implement some helper functions
	switch(nChar )
    {
      case VK_TAB:
        if (ShiftIsDown)
        {
          pDoc->GotoPrevEditState();
		  pDoc->UpdateAllViews(NULL);
        } else {
          pDoc->GotoNextEditState();
		  pDoc->UpdateAllViews(NULL);
        }
        break;
      case VK_RIGHT:
        pDoc->GotoNextBrush();
        break;
      case VK_LEFT:
        pDoc->GotoPrevBrush();
        break;
	  case VK_DELETE:
		pDoc->DeleteCurrentThing();
		break;
	  case VK_ADD:
		pDoc->ChangeGridSize(1);
		break;
	  case VK_SUBTRACT:
		pDoc->ChangeGridSize(0);
		break;
	  case VK_SPACE:
		if(GetAsyncKeyState(VK_LBUTTON) & 0x8000)
			break;
		if(GetAsyncKeyState(VK_RBUTTON) & 0x8000 )
			break;

		// a quick shortcut to get to the camera
		// and from the camera to the movement
		if(GetTool() != ID_TOOLS_CAMERA ) {
			pDoc->mOldTool = GetTool();
			SetTool(ID_TOOLS_CAMERA );
		}
		else
			SetTool(pDoc->mOldTool);
		break;
	}
}

void CTHREDView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
   // check for tilde
   if( nChar == 0xC0 )
   {
      // check for ctrl
      if( GetKeyState( VK_CONTROL ) & 0x8000 )
      {
         // check direction
         if( GetKeyState( VK_SHIFT ) & 0x8000 )
            pFrame->RotateViews( FALSE );
         else
            pFrame->RotateViews( TRUE );
      }
   }
   
   // check for escape which will clear the current document selections
   if( nChar == VK_ESCAPE )
   {
      CTHREDDoc* pDoc = GetDocument();
      pDoc->ClearSelections();
      
      // Draw the bounding box selection rectangle
      showSelectRect = false;
      
      pDoc->BuildEditBox(false);
      pDoc->UpdateAllViews(NULL);
   }
   
   // check for the ctrl(shift)tilde combos
	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CTHREDView::PostNcDestroy() 
{
	CView::PostNcDestroy();
}

void CTHREDView::OnTextureEditFace() 
{
	SetTool(ID_TEXTURE_EDIT_FACE);
}

void CTHREDView::OnUpdateTextureEditFace(CCmdUI* pCmdUI) 
{
	if(GetTool() == ID_TEXTURE_EDIT_FACE)
		pCmdUI->SetCheck();
	else
		pCmdUI->SetCheck(0);
}

void CTHREDView::OnEditModeButton() 
{
    m_LeftOp = m_RightOp = BBOX_PICK_NONE;
	SetTool(ID_EDIT_MODE_BUTTON);
}

void CTHREDView::OnUpdateEditModeButton(CCmdUI* pCmdUI) 
{
	if(GetTool() == ID_EDIT_MODE_BUTTON)
		pCmdUI->SetCheck();
	else
		pCmdUI->SetCheck(0);
}


// Assumes surface has been set in render context & render context has been
// locked & will be unlocked outside this routine
void CTHREDView::DrawSolid(ThredPolygon* poly, int color, TSRenderContext* rc, int vertex_dir)
{
    TS::VertexIndexPair         pairs[100];            // Move these into class later...
    Point2F                     texture_indices[100];  // Move these into class later...
    int                         i;
	Point3F	                    in_point;

      rc->setPointArray(&DefaultPointArray);
      rc->setCamera(m_pTSCamera);
      rc->setLights(&m_GSceneLights);

      rc->getSurface()->setHazeSource(GFX_HAZE_NONE);
      rc->getSurface()->setShadeSource(GFX_SHADE_NONE);
      rc->getSurface()->setAlphaSource(GFX_ALPHA_NONE);
      rc->getSurface()->setTransparency(FALSE);
      GetDocument()->GetMaterialList()->setDetailLevel(0);

      rc->getPointArray()->reset();
      rc->getPointArray()->useTextures(texture_indices);
      rc->getPointArray()->useIntensities(FALSE);
      rc->getPointArray()->setVisibility(TS::ClipMask);

      rc->getSurface()->setFillMode(GFX_FILL_CONSTANT);
      rc->getSurface()->setTexturePerspective(FALSE);
      rc->getPointArray()->useTextures(false);

//      rc->getCamera()->pushTransform((TMat3F)(TRUE));
      rc->getSurface()->setFillColor(color);

      if (!vertex_dir)  // Added brush
      {
        // Loop thru all the vertices...
        for (i = 0; i < poly->NumberOfPoints; i++)
        {
          in_point.x = poly->Points[i].X;
          in_point.y = poly->Points[i].Z;
          in_point.z = poly->Points[i].Y;

          pairs[i].fVertexIndex = rc->getPointArray()->addPoint(in_point);
          pairs[i].fTextureIndex = i;

          texture_indices[i].x = 0;
          texture_indices[i].y = 0;
        }
      } else {
        // Loop thru all the vertices...
        for (i = poly->NumberOfPoints-1; i >= 0; i--)
        {
          in_point.x = poly->Points[i].X;
          in_point.y = poly->Points[i].Z;
          in_point.z = poly->Points[i].Y;

          pairs[i].fVertexIndex = rc->getPointArray()->addPoint(in_point);
          pairs[i].fTextureIndex = i;

          texture_indices[i].x = 0;
          texture_indices[i].y = 0;
        }
      }

      rc->getPointArray()->drawPoly(poly->NumberOfPoints, pairs, 0);
//      rc->getCamera()->popTransform();
}

void CTHREDView::SetPalette(GFXPalette* Pal)
{
  pBuffer->setPalette(Pal, FALSE);
}


void CTHREDView::OnViewCentercameraatorigin() 
{
	CTHREDDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// Set the camera back to center.
    mCamera.PositionCamera(0.0, 0.0, 0.0);

    if ((mViewType == ID_VIEW_SOLIDRENDER) ||
        (mViewType == ID_VIEW_TEXTUREVIEW) ||
        (mViewType == ID_VIEW_3DWIREFRAME))
    {
      // update the 3space camera too
      m_pTSCamera->setTWC(TMat3F(
                         EulerF((mCamera.mCameraDirection.Y*ANGLE_CONVERSION),
                                (mCamera.mCameraDirection.Z*ANGLE_CONVERSION),
                                (mCamera.mCameraDirection.X*ANGLE_CONVERSION)),
                         Point3F(mCamera.mCameraPosition.X,
                                 mCamera.mCameraPosition.Z,
                                 mCamera.mCameraPosition.Y)).inverse());
    }

	pDoc->UpdateAllViews(NULL);
}

void CTHREDView::OnToolsCentercam() 
{
  OnViewCentercameraatorigin();
}

void CTHREDView::OnViewZoomin() 
{
   ViewZoomin();
}

void CTHREDView::OnViewZoomout() 
{
   ViewZoomout();
}

int CTHREDView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
   // create the window selection thingy
   CDC * pDC = this->GetDC();
   CRect rect;
   rect.left = 0; rect.top = 0;
   CSize size;
   CString string;
   
   // get the size for the button as the height of a menu item and
   // the width of the longest 
   rect.bottom = GetSystemMetrics( SM_CYMENU );
   rect.right = 0;
   
   string = "Solid Render"; size = pDC->GetTextExtent( string );
   if( size.cx > rect.right ) rect.right = size.cx;
   string = "3D Wireframe"; size = pDC->GetTextExtent( string );
   if( size.cx > rect.right ) rect.right = size.cx;
   string = "Top View"; size = pDC->GetTextExtent( string );
   if( size.cx > rect.right ) rect.right = size.cx;
   string = "Side View"; size = pDC->GetTextExtent( string );
   if( size.cx > rect.right ) rect.right = size.cx;
   string = "Front View"; size = pDC->GetTextExtent( string );
   if( size.cx > rect.right ) rect.right = size.cx;
   string = "Texture View"; size = pDC->GetTextExtent( string );
   if( size.cx > rect.right ) rect.right = size.cx;
   
   // add some edge
   rect.right = rect.right + GetSystemMetrics( SM_CXDLGFRAME ) * 4;
   
   ViewButton.Create( "", WS_CHILD | WS_VISIBLE | WS_DISABLED | 
      BS_PUSHBUTTON, rect, this, ID_VIEWBUTTON );
	
	return 0;
}

void CTHREDView::OnViewButton( void )
{
   CMenu menu;
   CPoint point;
      	
   GetCursorPos( &point );
   menu.CreatePopupMenu();
   menu.AppendMenu( MF_STRING, ID_VIEW_3DWIREFRAME, "3D Wireframe");
   menu.AppendMenu( MF_STRING, ID_VIEW_SOLIDRENDER, "Solid Render");
   menu.AppendMenu( MF_STRING, ID_VIEW_TEXTUREVIEW, "Texture View");
   menu.AppendMenu( MF_STRING, ID_VIEW_TOPVIEW, "Top View" );
   menu.AppendMenu( MF_STRING, ID_VIEW_FRONTVIEW, "Front View" );
   menu.AppendMenu( MF_STRING, ID_VIEW_SIDEVIEW, "Side View" );
   menu.CheckMenuItem( mViewType, MF_BYCOMMAND | MF_CHECKED );
   
   int value = menu.TrackPopupMenu( TPM_RETURNCMD | TPM_CENTERALIGN | 
      TPM_LEFTBUTTON, point.x,  point.y, this, NULL );
      
   if( !value )
      return;
      
   // set the view type
   OnViewType( value );
}

HBRUSH CTHREDView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	
   return CView::OnCtlColor( pDC, pWnd, nCtlColor );
   
   // make sure a button
   if( nCtlColor == CTLCOLOR_BTN )
   {
   }
}

BOOL CTHREDView::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
   // check for zoom 
   if( zDelta < 0 )
      ViewZoomout(); 
   else
      ViewZoomin();
   
   // zoom
   return( TRUE );
}

void CTHREDView::ViewZoomin()
{
  mZoomFactor *= 1 + (ZOOM_CHANGE * 100);
  RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
}

void CTHREDView::ViewZoomout()
{
  mZoomFactor *= 1 + (ZOOM_CHANGE * -100);

  if(mZoomFactor < .1)
  {
    mZoomFactor = .1;
  }

  RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
}

