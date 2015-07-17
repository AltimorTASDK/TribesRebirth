// AsgnText.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "AsgnText.h"
#include "ThredBrush.h"
#include "THREDDoc.h"
#include "g_bitmap.h"
#include "itrgeometry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static Point2F  txcenter;
static int      txcount;

// change these if we get some bits for texture scale values taken/given
const int numScaleBits = ITRGeometry::Surface::Constants::textureScaleBits;
const int maxScale = 6;
const int scaleSignMask = 1 << ( numScaleBits - 1 );
const int scaleValueMask = scaleSignMask - 1;

/////////////////////////////////////////////////////////////////////////////
// AssignTexture dialog


AssignTexture::AssignTexture(CWnd* pParent /*=NULL*/)
	: CDialog(AssignTexture::IDD, pParent)
{
	//{{AFX_DATA_INIT(AssignTexture)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

    stretchit = 0;
	material_index1 = 0;
	material_count = 0;
	m_pFrame1 = NULL;
    rotation = 0.0;
    r_delta = 0.F;
    last_rotation = (double)(180*MAX_TICS_PER_DEGREE);
    lastrot = 0;

    deltau = 0.0;
//    u_val = 0.0;
    deltav = 0.0;
//    v_val = 0.0;
    uval = 0.0;
    vval = 0.0;
    doneBsp = false;
}

AssignTexture::~AssignTexture()
{
  if (m_pFrame1)
  {
    delete m_pFrame1;
  }
}


void AssignTexture::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AssignTexture)
	DDX_Control(pDX, IDC_MAT_SPIN, m_spinControl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(AssignTexture, CDialog)
	//{{AFX_MSG_MAP(AssignTexture)
	ON_WM_PAINT()
	ON_EN_CHANGE(IDC_MAT_INDEX, OnChangeMatIndex)
	ON_NOTIFY(UDN_DELTAPOS, IDC_MAT_SPIN, OnDeltaposMatSpin)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_UPBUTTON, OnUpbutton)
	ON_BN_CLICKED(IDC_DOWNBUTTON, OnDownbutton)
	ON_BN_CLICKED(IDC_RADIO10_0, OnRadio100)
	ON_BN_CLICKED(IDC_RADIO2_5, OnRadio25)
	ON_BN_CLICKED(IDC_RADIO5_0, OnRadio50)
	ON_BN_CLICKED(IDC_FLIPHORIZ, OnFliphoriz)
	ON_BN_CLICKED(IDC_FLIPVERT, OnFlipvert)
	ON_BN_CLICKED(IDC_STRETCHCHECK2, OnStretchcheck2)
	ON_BN_CLICKED(IDC_RADIO1_0, OnRadio10)
	ON_BN_CLICKED(IDC_AMBIENTCHECK, OnAmbientcheck)
	ON_BN_CLICKED(IDC_CLEARAMBIENTBUT, OnClearambientbut)
	ON_BN_CLICKED(IDC_SETAMBIENTBUT, OnSetambientbut)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// AssignTexture message handlers

BOOL AssignTexture::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class
    cs.x += 200;
	cs.lpszClass = AfxRegisterWndClass(CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT,
										::LoadCursor(NULL,IDC_ARROW));
    
	return CDialog::PreCreateWindow(cs);
}

void AssignTexture::OnPaint() 
{
	if (!m_pFrame1)
		return;

	// Set the palette for the frames
	m_pFrame1->m_pSurface->setPalette(palette, false);

	// Redraw the textures
	if (material_list)
	{
		material_list->setDetailLevel(0);
		m_pFrame1->m_pMaterial = &material_list->getMaterial(material_index1);
		m_pFrame1->Refresh();
	}
	
	CPaintDC dc( this );
	// Do not call CDialog::OnPaint() for painting messages
}


int AssignTexture::GetIndex()
{
  return material_index1;
}


void AssignTexture::SetIndex(int index)
{
  temp_index = index;
  material_index1 = index;
}

int AssignTexture::m_lastX = -1;
int AssignTexture::m_lastY = -1;

BOOL AssignTexture::OnInitDialog() 
{
    CSliderCtrl*  slider;
    char  mybuf[80];


	CDialog::OnInitDialog();
	
    CWnd *temp = GetDlgItem(IDC_MAT_CANVAS);
	RECT rect;
	temp->GetClientRect(&rect);
    material_index1 = temp_index;
	m_pFrame1 = new CMatCanvas;
	m_pFrame1->Create(temp, rect, IDC_FRAME3);

	if (material_list)
	{
		material_list->setDetailLevel(0);
		m_pFrame1->m_pMaterial = &material_list->getMaterial(material_index1);
	}

	// Set up the spin controls
	((CSpinButtonCtrl*)GetDlgItem(IDC_MAT_SPIN))->SetRange(0, material_count-1);
	((CSpinButtonCtrl*)GetDlgItem(IDC_MAT_SPIN))->SetPos(material_index1);

    if (m_pFrame1)
    {
      char  buf[32];
      if ((m_pFrame1->m_pMaterial->fParams.fFlags & TSMaterial::MatFlags) == TSMaterial::MatTexture)
      {
        const GFXBitmap*  bmp = m_pFrame1->m_pMaterial->getTextureMap();
        sprintf(buf, "(%d x %d)", bmp->width, bmp->height);
	    ((CEdit*)GetDlgItem(IDC_TSIZEREADOUT))->SetSel(0, -1, FALSE);
        ((CEdit*)GetDlgItem(IDC_TSIZEREADOUT))->ReplaceSel(buf);
      } else {
        sprintf(buf, "Null Texture");
	    ((CEdit*)GetDlgItem(IDC_TSIZEREADOUT))->SetSel(0, -1, FALSE);
        ((CEdit*)GetDlgItem(IDC_TSIZEREADOUT))->ReplaceSel(buf);
      }
    }

    // Set focus to the edit control & preset selection
	((CEdit*)GetDlgItem(IDC_MAT_INDEX))->SetSel(0, -1, FALSE);
	((CEdit*)GetDlgItem(IDC_MAT_INDEX))->SetFocus();

    // Set up vertical offset values
	slider = (CSliderCtrl*)GetDlgItem(IDC_VERTICALSLIDER);
	slider->SetRange(0, 255, TRUE);
	slider->SetPos(node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.y);

    // Set up horizontal offset values
	slider = (CSliderCtrl*)GetDlgItem(IDC_HORIZONTALSLIDER);
	slider->SetRange(0, 255, TRUE);
	slider->SetPos(node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.x);

//   for (int i = 0; i < node->Polygon.NumberOfPoints; i++)      //
//   {
//      CString string;
//      string.Format( "[%d] - %f, %f %f\n", i,
//         node->Polygon.Points[i].X,
//         node->Polygon.Points[i].Y,
//         node->Polygon.Points[i].Z );
//      OutputDebugString( string );
//   }

    deltau = deltav = 0.0;
    lastu = node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.x;
    lastv = node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.y;

    u_val = node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.x;
    v_val = node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.y;
    rotation = 0.0;

    double printval = 360.0 - node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureRotate;
    if (printval == 360.0)
    {
      printval = 0.0;
    }

    sprintf(&(mybuf[0]), "%4.1f", printval);
    ((CEdit*)GetDlgItem(IDC_TROT_SHOW))->ReplaceSel((LPCTSTR)&(mybuf[0]), TRUE);

    rotate_amount = 5.0;
    ((CButton*)GetDlgItem(IDC_RADIO5_0))->SetCheck(1);

    WINDOWPLACEMENT wndpl;
    int swidth;
    
//    swidth = GetSystemMetrics(SM_CXFULLSCREEN);
//    GetWindowPlacement(&wndpl);
//    wndpl.rcNormalPosition.left += ((swidth/2));
//    wndpl.rcNormalPosition.right += ((swidth/2));
//    MoveWindow(&(wndpl.rcNormalPosition));

   last_scaleshift = node->mBrushPtr->mPolygons[node->mPolygonNumber].mTextureScaleShift;

   CString Title;
	slider = (CSliderCtrl*)GetDlgItem( IDC_TEX_SHIFT );
   
	slider->SetRange( 0, 2 * maxScale, TRUE );
   
   // grab the lower bits
   int pos = last_scaleshift & scaleValueMask;

   if( pos > maxScale )
      pos = maxScale;
   
   // make sure we dont have -0 ( shouldnt hurt but what is -0 anyways? )
   if( !pos )
      last_scaleshift = 0;
      
   if( last_scaleshift & scaleSignMask )
   {
	   slider->SetPos( maxScale - pos );
      Title.Format( "- %d", pos );      
   }
   else
   {
	   slider->SetPos( maxScale + pos );
      if( pos )
         Title.Format( "+ %d", pos );      
      else
         Title.Format( "%d", pos );      
   }
   GetDlgItem( IDC_TEX_SHIFT_DISPLAY )->SetWindowText( Title );

   if( ( m_lastX != -1 ) && ( m_lastY != -1 ) )
      SetWindowPos( NULL, m_lastX, m_lastY, 0, 0, SWP_NOSIZE | SWP_NOZORDER );

   // set the ambient applycheck   
   ((CButton*)GetDlgItem(IDC_AMBIENTCHECK))->SetCheck( 
      node->mBrushPtr->mPolygons[node->mPolygonNumber].mApplyAmbient );

	return FALSE; // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void AssignTexture::OnOK() 
{
    //ClearHandles(topnode, node->mBrushPtr, node->mPolygonNumber);
	
   // get the x/y positions
   CRect rect;
   GetWindowRect( rect );
   if( ( rect.left >= 0 ) && ( rect.top >= 0 ) )
   {
      m_lastX = rect.left;
      m_lastY = rect.top;
   }
   
	CDialog::OnOK();
}

void AssignTexture::OnCancel() 
{
   node->mBrushPtr->mPolygons[node->mPolygonNumber].mTextureScaleShift = last_scaleshift;
   node->mBrushPtr->mTransformedPolygons[node->mPolygonNumber].mTextureScaleShift = last_scaleshift;
   
    // Restore texture index
    TextureTheNodes(topnode, node->mBrushPtr, node->mPolygonNumber, temp_index);

    // Restore texture u,v values
    node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.x = u_val;
    ShiftNodesU(topnode, node->mBrushPtr, node->mPolygonNumber, u_val);

    node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.y = v_val;
    ShiftNodesV(topnode, node->mBrushPtr, node->mPolygonNumber, v_val);

    // Restore texture rotation
    Point2F center(0.0, 0.0);// = node->mBrushPtr->GetCenterTextureOffsets(node->mPolygonNumber);
    node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureRotate += (-rotation);

    if (node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureRotate >= 360.F)
      node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureRotate -= 360.F;

    if (node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureRotate < 0.F)
      node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureRotate += 360.F;

    RotateNodes(topnode, node->mBrushPtr, node->mPolygonNumber, center, -rotation);
    node->mBrushPtr->RotateFaceTexture(node->mPolygonNumber, -rotation);

    //ClearHandles(topnode, node->mBrushPtr, node->mPolygonNumber);

   // get the x/y positions
   CRect rect;
   GetWindowRect( rect );
   if( ( rect.left >= 0 ) && ( rect.top >= 0 ) )
   {
      m_lastX = rect.left;
      m_lastY = rect.top;
   }

    CDialog::OnCancel();
}

void AssignTexture::OnChangeMatIndex() 
{
	char	str_val[10];
	int		tmp;

	((CEdit*)GetDlgItem(IDC_MAT_INDEX))->GetLine(0, (LPTSTR)str_val);

	tmp = atoi(str_val);

	if ((tmp >= 0) && (tmp < material_count))
	{
		material_index1 = tmp;

		OnPaint();

        if (m_pFrame1)
        {
          char  buf[32];

          if ((m_pFrame1->m_pMaterial->fParams.fFlags & TSMaterial::MatFlags) == TSMaterial::MatTexture)
          {
            const GFXBitmap*  bmp = m_pFrame1->m_pMaterial->getTextureMap();
            sprintf(buf, "(%d x %d)", bmp->width, bmp->height);
	        ((CEdit*)GetDlgItem(IDC_TSIZEREADOUT))->SetSel(0, -1, FALSE);
            ((CEdit*)GetDlgItem(IDC_TSIZEREADOUT))->ReplaceSel(buf);
          } else {
            sprintf(buf, "Null Texture");
	        ((CEdit*)GetDlgItem(IDC_TSIZEREADOUT))->SetSel(0, -1, FALSE);
            ((CEdit*)GetDlgItem(IDC_TSIZEREADOUT))->ReplaceSel(buf);
          }
        }

        TextureTheNodes(topnode, node->mBrushPtr, node->mPolygonNumber, material_index1);
        parentview->RedrawWindow();
	}
}

void AssignTexture::OnDeltaposMatSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	int newval = pNMUpDown->iPos + pNMUpDown->iDelta;

	if ((newval >= 0) && (newval < material_count))
	{
		char str_val[10];
		sprintf(str_val, "%d", newval);
		((CEdit*)GetDlgItem(IDC_MAT_INDEX))->SetSel(0, -1, TRUE);

		((CEdit*)GetDlgItem(IDC_MAT_INDEX))->ReplaceSel((LPCTSTR)str_val, FALSE);

		material_index1 = newval;

		OnPaint();
	}

	*pResult = 0;
}


void AssignTexture::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	UINT	new_pos;
   CString Title;

	int which_slider = pScrollBar->GetDlgCtrlID();

	if ((nSBCode == SB_THUMBPOSITION) || (nSBCode == SB_THUMBTRACK))
	{
		switch (which_slider)
		{
		case IDC_HORIZONTALSLIDER:
            deltau = (double)nPos - lastu;
            lastu = nPos;
//            u_val += deltau;

            uval = (double)nPos / 100.0;
            
            node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.x = (double)nPos;
//            node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.x += ((double)deltau/100.0) * 256;
//            if (node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.x < 0)
//              node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.x += 256;

            ShiftNodesU(topnode, node->mBrushPtr, node->mPolygonNumber, (double)nPos);//(deltau/100.0));
            
            //node->mBrushPtr->ShiftFaceTextureU(node->mPolygonNumber, (double)nPos / 100.0);//deltau/100.0);
//            node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.x = ((double)nPos/100.0) * 256;
            parentview->RedrawWindow();
			break;

		case IDC_VERTICALSLIDER:
            deltav = (double)nPos - lastv;
            lastv = nPos;
//            v_val += deltav;
            vval = (double)nPos / 100.0;
            
            node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.y = (double)nPos;
//            node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.y += ((double)deltav/100.0) * 256;
//            if (node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.y < 0)
//              node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.y += 256;

            ShiftNodesV(topnode, node->mBrushPtr, node->mPolygonNumber, (double)nPos);//(deltav/100.0));
            
            //node->mBrushPtr->ShiftFaceTextureV(node->mPolygonNumber, (double)nPos / 100.0);//deltav/100.0);
//            node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.y = ((double)nPos/100.0) * 256;
            parentview->RedrawWindow();
			break;
         
      case IDC_TEX_SHIFT:
			new_pos = ((CSliderCtrl*)GetDlgItem(IDC_TEX_SHIFT))->GetPos();

         // positive
         if( new_pos >= maxScale )
         {
            new_pos -= maxScale;
            node->mBrushPtr->mPolygons[node->mPolygonNumber].mTextureScaleShift = new_pos & scaleValueMask;
            node->mBrushPtr->mTransformedPolygons[node->mPolygonNumber].mTextureScaleShift = new_pos & scaleValueMask;
            if( new_pos )
               Title.Format( "+ %d", new_pos );      
            else
               Title.Format( "%d", new_pos );      
         }
         else // neg
         {
            new_pos = maxScale - new_pos;
            node->mBrushPtr->mPolygons[node->mPolygonNumber].mTextureScaleShift = 
               ( new_pos & scaleValueMask ) | scaleSignMask;
            node->mBrushPtr->mTransformedPolygons[node->mPolygonNumber].mTextureScaleShift = 
               ( new_pos & scaleValueMask ) | scaleSignMask;
            Title.Format( "- %d", new_pos );      
         }
         GetDlgItem( IDC_TEX_SHIFT_DISPLAY )->SetWindowText( Title );
         parentview->RedrawWindow();
         break;
         
		default:
			break;
		}
	} else {
		// Handle non-sliding event
		switch (which_slider)
		{
		case IDC_HORIZONTALSLIDER:
			new_pos = ((CSliderCtrl*)GetDlgItem(IDC_HORIZONTALSLIDER))->GetPos();

            deltau = (double)new_pos - lastu;
            lastu = new_pos;
//            u_val += deltau;
            uval = (double)new_pos / 100.0;

            node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.x = (double)new_pos;
//            node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.x += ((double)deltau/100.0) * 256;
//            if (node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.x < 0)
//              node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.x += 256;

            ShiftNodesU(topnode, node->mBrushPtr, node->mPolygonNumber, (double)new_pos);//(deltau/100.0));
            
            //node->mBrushPtr->ShiftFaceTextureU(node->mPolygonNumber, (double)new_pos / 100.0);//deltau/100.0);
//            node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.x = ((double)new_pos/100.0) * 256;
            parentview->RedrawWindow();
			break;

		case IDC_VERTICALSLIDER:
			new_pos = ((CSliderCtrl*)GetDlgItem(IDC_VERTICALSLIDER))->GetPos();

            deltav = (double)new_pos - lastv;
            lastv = new_pos;
//            v_val += deltav;
            vval = (double)new_pos / 100.0;
            
            node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.y = (double)new_pos;
//            node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.y += ((double)deltav/100.0) * 256;
//            if (node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.y < 0)
//              node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.y += 256;

            ShiftNodesV(topnode, node->mBrushPtr, node->mPolygonNumber, (double)new_pos);//(deltav/100.0));
            
            //node->mBrushPtr->ShiftFaceTextureV(node->mPolygonNumber, (double)new_pos / 100.0);//deltav/100.0);
//            node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureShift.y = ((double)new_pos/100.0) * 256;
            parentview->RedrawWindow();
			break;

      case IDC_TEX_SHIFT:
			new_pos = ((CSliderCtrl*)GetDlgItem(IDC_TEX_SHIFT))->GetPos();

         //positive         
         if( new_pos >= maxScale )
         {
            new_pos -= maxScale;
            node->mBrushPtr->mPolygons[node->mPolygonNumber].mTextureScaleShift = new_pos & scaleValueMask;
            node->mBrushPtr->mTransformedPolygons[node->mPolygonNumber].mTextureScaleShift = new_pos & scaleValueMask;
            if( new_pos )
               Title.Format( "+ %d", new_pos );      
            else
               Title.Format( "%d", new_pos );      
         }
         else // negative
         {
            new_pos = maxScale - new_pos;
            node->mBrushPtr->mPolygons[node->mPolygonNumber].mTextureScaleShift = ( new_pos & scaleValueMask ) | scaleSignMask;
            node->mBrushPtr->mTransformedPolygons[node->mPolygonNumber].mTextureScaleShift = ( new_pos & scaleValueMask ) | scaleSignMask;
            Title.Format( "- %d", new_pos );      
         }
         GetDlgItem( IDC_TEX_SHIFT_DISPLAY )->SetWindowText( Title );
         parentview->RedrawWindow();
         break;
         
		default:
			break;
		}
	}
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void AssignTexture::ClearHandles(CBspNode* curr_node, CThredBrush* brush, int which_poly)
{
  CBspNode* tempnode;

  if (!curr_node)
    return;

  if ((curr_node->mBrushPtr == brush) && (curr_node->mPolygonNumber == which_poly))
  {
    tempnode = curr_node;

    while (tempnode)
    {
      tempnode = tempnode->CoplanarChild;
    }
  }

  if (curr_node->FrontChild)
  {
    ClearHandles(curr_node->FrontChild, brush, which_poly);
  }

  if (curr_node->BackChild)
  {
    ClearHandles(curr_node->BackChild, brush, which_poly);
  }
}

void AssignTexture::RotateNodes(CBspNode* curr_node, CThredBrush* brush, int which_poly, Point2F center, double val)
{
  CBspNode* tempnode;

  if (!curr_node)
    return;

  if ((curr_node->mBrushPtr == brush) && (curr_node->mPolygonNumber == which_poly))
  {
    tempnode = curr_node;

    while (tempnode)
    {
      tempnode->normalizeTexture(&(tempnode->Polygon), tempnode->mBrushPtr);//, center);

      // Restore U/V shifting
      tempnode->Polygon.mTextureOffset.x =
        (tempnode->Polygon.mTextureOffset.x + tempnode->mTextureShift.x) % 256;
      tempnode->Polygon.mTextureOffset.y =
        (tempnode->Polygon.mTextureOffset.y + tempnode->mTextureShift.y) % 256;

      tempnode->Polygon.texture_handle = CConstructiveBsp::GetNextCacheKey(tempnode);
      //GFXLightMap* templm = pDoc->pWorldBsp->CopyLightMap(tempnode->Polygon.lm);
      //tempnode->Polygon.lm = templm;
      pDoc->pWorldBsp->AssignLightMap(tempnode, theApp.lightval);

      tempnode = tempnode->CoplanarChild;
    }
  }
    if (curr_node->FrontChild)
      RotateNodes(curr_node->FrontChild, brush, which_poly, center, val);

    if (curr_node->BackChild)
      RotateNodes(curr_node->BackChild, brush, which_poly, center, val);
}

void AssignTexture::ShiftNodesU(CBspNode* curr_node, CThredBrush* brush, int which_poly, double val)
{
  CBspNode* tempnode;

  if (!curr_node)
    return;

  if ((curr_node->mBrushPtr == brush) && (curr_node->mPolygonNumber == which_poly))
  {
    tempnode = curr_node;

    while (tempnode)
    {
      tempnode->mBrushPtr->mTextureID[tempnode->mPolygonNumber].TextureShift.x = val;
      tempnode->ShiftFaceTextureU(val);
      tempnode->Polygon.texture_handle = CConstructiveBsp::GetNextCacheKey(tempnode);
      //GFXLightMap* templm = pDoc->pWorldBsp->CopyLightMap(tempnode->Polygon.lm);
      //tempnode->Polygon.lm = templm;
      pDoc->pWorldBsp->AssignLightMap(tempnode, theApp.lightval);

      tempnode = tempnode->CoplanarChild;
    }
  }

  if (curr_node->FrontChild)
  {
    ShiftNodesU(curr_node->FrontChild, brush, which_poly, val);
  }

  if (curr_node->BackChild)
  {
    ShiftNodesU(curr_node->BackChild, brush, which_poly, val);
  }
}

void AssignTexture::ShiftNodesV(CBspNode* curr_node, CThredBrush* brush, int which_poly, double val)
{
  CBspNode* tempnode;

  if (!curr_node)
    return;

  if ((curr_node->mBrushPtr == brush) && (curr_node->mPolygonNumber == which_poly))
  {
    tempnode = curr_node;

    while (tempnode)
    {
      tempnode->mBrushPtr->mTextureID[tempnode->mPolygonNumber].TextureShift.y = val;
      tempnode->ShiftFaceTextureV(val);
      tempnode->Polygon.texture_handle = CConstructiveBsp::GetNextCacheKey(tempnode);
      //GFXLightMap* templm = pDoc->pWorldBsp->CopyLightMap(tempnode->Polygon.lm);
      //tempnode->Polygon.lm = templm;
      pDoc->pWorldBsp->AssignLightMap(tempnode, theApp.lightval);

      tempnode = tempnode->CoplanarChild;
    }
  }

  if (curr_node->FrontChild)
  {
    ShiftNodesV(curr_node->FrontChild, brush, which_poly, val);
  }

  if (curr_node->BackChild)
  {
    ShiftNodesV(curr_node->BackChild, brush, which_poly, val);
  }
}

void AssignTexture::OnUpbutton() 
{
    Point2F center(0.0, 0.0);
    char  mybuf[80];

    // Rotate by rotate_amount degrees
    node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureRotate += rotate_amount;

    if (node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureRotate >= 360.F)
      node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureRotate -= 360.F;

    if (node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureRotate < 0.F)
      node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureRotate += 360.F;

    //center = node->mBrushPtr->GetCenterTextureOffsets(node->mPolygonNumber);
    txcenter.x = 0.0;
    txcenter.y = 0.0;
    txcount = 0;
    FindTextureCenter(topnode, node->mBrushPtr, node->mPolygonNumber);
    if (txcount)
    {
      center.x = txcenter.x / ((double)txcount);
      center.y = txcenter.y / ((double)txcount);

      center.x += (double)node->Polygon.mTextureOffset.x;
      center.x = ((int)center.x) % 256;
      center.y += (double)node->Polygon.mTextureOffset.y;
      center.y = ((int)center.y) % 256;
    }

    RotateNodes(topnode, node->mBrushPtr, node->mPolygonNumber, center, rotate_amount);
    rotation += rotate_amount;

    double printval = 360.0 - node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureRotate;
    if (printval == 360.0)
    {
      printval = 0.0;
    }

    sprintf(&(mybuf[0]), "%4.1f", printval);
	((CEdit*)GetDlgItem(IDC_TROT_SHOW))->SetSel(0, -1, FALSE);
    ((CEdit*)GetDlgItem(IDC_TROT_SHOW))->ReplaceSel((LPCTSTR)&(mybuf[0]), TRUE);

    parentview->RedrawWindow();
}

void AssignTexture::OnDownbutton()
{
    Point2F center;
    char  mybuf[80];

    // Rotate by -rotate_amount degrees
    node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureRotate += -rotate_amount;

    if (node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureRotate >= 360.F)
      node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureRotate -= 360.F;

    if (node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureRotate < 0.F)
      node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureRotate += 360.F;

    //center = node->mBrushPtr->GetCenterTextureOffsets(node->mPolygonNumber);
    txcenter.x = 0.0;
    txcenter.y = 0.0;
    txcount = 0;
    //FindTextureCenter(topnode, node->mBrushPtr, node->mPolygonNumber);
    if (txcount)
    {
      center.x = txcenter.x / ((double)txcount);
      center.y = txcenter.y / ((double)txcount);
    }

    RotateNodes(topnode, node->mBrushPtr, node->mPolygonNumber, center, -rotate_amount);
    rotation -= rotate_amount;

    double printval = 360.0 - node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureRotate;
    if (printval == 360.0)
    {
      printval = 0.0;
    }

    sprintf(&(mybuf[0]), "%4.1f", printval);
	((CEdit*)GetDlgItem(IDC_TROT_SHOW))->SetSel(0, -1, FALSE);
    ((CEdit*)GetDlgItem(IDC_TROT_SHOW))->ReplaceSel((LPCTSTR)&(mybuf[0]), TRUE);

    parentview->RedrawWindow();
}

void AssignTexture::TextureTheNodes(CBspNode* curr_node, CThredBrush* brush, int which_poly, int which_material)
{
  CBspNode* tempnode;

  if ( ((material_list->getMaterial(which_material).fParams.fFlags & TSMaterial::MatFlags) != TSMaterial::MatTexture) &&
       ((material_list->getMaterial(which_material).fParams.fFlags & TSMaterial::MatFlags) != TSMaterial::MatNull) )
    return;

  if (!curr_node)
    return;

  if (curr_node->mBrushPtr == brush)
  {
    if(curr_node->mPolygonNumber == which_poly)
    {
      tempnode = curr_node;

      while (tempnode)
      {
        tempnode->mBrushPtr->mTextureID[tempnode->mPolygonNumber].TextureID = which_material;
        tempnode->Polygon.texture_handle = CConstructiveBsp::GetNextCacheKey(tempnode);
        //GFXLightMap* templm = pDoc->pWorldBsp->CopyLightMap(tempnode->Polygon.lm);
        //tempnode->Polygon.lm = templm;
        pDoc->pWorldBsp->AssignLightMap(tempnode, theApp.lightval);

        tempnode = tempnode->CoplanarChild;
      }
    }
  }

  if (curr_node->FrontChild)
  {
    TextureTheNodes(curr_node->FrontChild, brush, which_poly, which_material);
  }

  if (curr_node->BackChild)
  {
    TextureTheNodes(curr_node->BackChild, brush, which_poly, which_material);
  }  
}

void AssignTexture::OnRadio100() 
{
  rotate_amount = 10.0;

  ((CButton*)GetDlgItem(IDC_RADIO1_0))->SetCheck(0);
  ((CButton*)GetDlgItem(IDC_RADIO2_5))->SetCheck(0);
  ((CButton*)GetDlgItem(IDC_RADIO5_0))->SetCheck(0);
}

void AssignTexture::OnRadio25() 
{
  rotate_amount = 2.5;

  ((CButton*)GetDlgItem(IDC_RADIO1_0))->SetCheck(0);
  ((CButton*)GetDlgItem(IDC_RADIO10_0))->SetCheck(0);
  ((CButton*)GetDlgItem(IDC_RADIO5_0))->SetCheck(0);
}

void AssignTexture::OnRadio50() 
{
  rotate_amount = 5.0;

  ((CButton*)GetDlgItem(IDC_RADIO1_0))->SetCheck(0);
  ((CButton*)GetDlgItem(IDC_RADIO2_5))->SetCheck(0);
  ((CButton*)GetDlgItem(IDC_RADIO10_0))->SetCheck(0);
}

void AssignTexture::OnFliphoriz() 
{
  Point2F center(0.0, 0.0);
  node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureFlipH ^= 1;
  
  txcenter.x = 0.0;
  txcenter.y = 0.0;
  txcount = 0;
  //FindTextureCenter(topnode, node->mBrushPtr, node->mPolygonNumber);
  if (txcount)
  {
    center.x = txcenter.x / ((double)txcount);
    center.y = txcenter.y / ((double)txcount);
  }

  FlipNodesH(topnode, node->mBrushPtr, node->mPolygonNumber, center);
  parentview->RedrawWindow();
}

void AssignTexture::FlipNodesH(CBspNode* curr_node, CThredBrush* brush, int which_poly, Point2F center)
{
  CBspNode* tempnode;

  if (!curr_node)
    return;

  if ((curr_node->mBrushPtr == brush) && (curr_node->mPolygonNumber == which_poly))
  {
    tempnode = curr_node;

    while (tempnode)
    {
      //tempnode->FlipFaceTextureH();
      tempnode->normalizeTexture(&tempnode->Polygon, brush);//, center);

      tempnode->Polygon.mTextureOffset.x =
        (tempnode->Polygon.mTextureOffset.x + tempnode->mTextureShift.x) % 256;
      tempnode->Polygon.mTextureOffset.y =
        (tempnode->Polygon.mTextureOffset.y + tempnode->mTextureShift.y) % 256;      

      tempnode->Polygon.texture_handle = CConstructiveBsp::GetNextCacheKey(tempnode);
      //GFXLightMap* templm = pDoc->pWorldBsp->CopyLightMap(tempnode->Polygon.lm);
      //tempnode->Polygon.lm = templm;
      pDoc->pWorldBsp->AssignLightMap(tempnode, theApp.lightval);

      tempnode = tempnode->CoplanarChild;
    }
  }

  if (curr_node->FrontChild)
  {
    FlipNodesH(curr_node->FrontChild, brush, which_poly, center);
  }

  if (curr_node->BackChild)
  {
    FlipNodesH(curr_node->BackChild, brush, which_poly, center);
  }
}

void AssignTexture::FlipNodesV(CBspNode* curr_node, CThredBrush* brush, int which_poly, Point2F center)
{
  CBspNode* tempnode;

  if (!curr_node)
    return;

  if ((curr_node->mBrushPtr == brush) && (curr_node->mPolygonNumber == which_poly))
  {
    tempnode = curr_node;

    while (tempnode)
    {
      //tempnode->FlipFaceTextureV();
      tempnode->normalizeTexture(&tempnode->Polygon, brush);//, center);

      tempnode->Polygon.mTextureOffset.x =
        (tempnode->Polygon.mTextureOffset.x + tempnode->mTextureShift.x) % 256;
      tempnode->Polygon.mTextureOffset.y =
        (tempnode->Polygon.mTextureOffset.y + tempnode->mTextureShift.y) % 256;      

      tempnode->Polygon.texture_handle = CConstructiveBsp::GetNextCacheKey(tempnode);
      //GFXLightMap* templm = pDoc->pWorldBsp->CopyLightMap(tempnode->Polygon.lm);
      //tempnode->Polygon.lm = templm;
      pDoc->pWorldBsp->AssignLightMap(tempnode, theApp.lightval);

      tempnode = tempnode->CoplanarChild;
    }
  }

  if (curr_node->FrontChild)
  {
    FlipNodesV(curr_node->FrontChild, brush, which_poly, center);
  }

  if (curr_node->BackChild)
  {
    FlipNodesV(curr_node->BackChild, brush, which_poly, center);
  }
}

void AssignTexture::OnFlipvert() 
{
  Point2F center(0.0, 0.0);

  node->mBrushPtr->mTextureID[node->mPolygonNumber].TextureFlipV ^= 1;
  
  txcenter.x = 0.0;
  txcenter.y = 0.0;
  txcount = 0;
  //FindTextureCenter(topnode, node->mBrushPtr, node->mPolygonNumber);
  if (txcount)
  {
    center.x = txcenter.x / ((double)txcount);
    center.y = txcenter.y / ((double)txcount);
  }

  FlipNodesV(topnode, node->mBrushPtr, node->mPolygonNumber, center);
  parentview->RedrawWindow();
}

void AssignTexture::OnStretchcheck2() 
{
  stretchit ^= 1;

  m_pFrame1->stretchit = stretchit;

  OnPaint();
}

void AssignTexture::FindTextureCenter(CBspNode* curr_node, CThredBrush* brush, int which_poly)
{
  CBspNode* tempnode;

  if (!curr_node)
    return;

  if ((curr_node->mBrushPtr == brush) && (curr_node->mPolygonNumber == which_poly))
  {
    tempnode = curr_node;

    while (tempnode)
    {
      for (int i = 0; i < tempnode->Polygon.NumberOfPoints; i++)
      {
        txcenter.x += tempnode->Polygon.mTextureUV[i].x;
        txcenter.y += tempnode->Polygon.mTextureUV[i].y;
        txcount++;
      }

      tempnode = tempnode->CoplanarChild;
    }
  }
    if (curr_node->FrontChild)
      FindTextureCenter(curr_node->FrontChild, brush, which_poly);

    if (curr_node->BackChild)
      FindTextureCenter(curr_node->BackChild, brush, which_poly);
}


void AssignTexture::OnRadio10() 
{
  rotate_amount = 0.5;

  ((CButton*)GetDlgItem(IDC_RADIO2_5))->SetCheck(0);
  ((CButton*)GetDlgItem(IDC_RADIO5_0))->SetCheck(0);
  ((CButton*)GetDlgItem(IDC_RADIO10_0))->SetCheck(0);
}


void AssignTexture::OnAmbientcheck() 
{
  node->mBrushPtr->mPolygons[node->mPolygonNumber].mApplyAmbient ^= 1;
}

void AssignTexture::OnClearambientbut() 
{
   // need to do a quickbsp at least once
   if( !doneBsp )
   {
      pDoc->pWorldBsp->RebuildOptimal( NULL, 0, FALSE );
      doneBsp = true;
   }
   
   // go through all the brushes and clear out the flag
   CThredBrush * brush = pDoc->mBrushList;
   while( brush )
   {
      for( int i = 0; i < brush->mNumberOfPolygons; i++ )
         brush->mPolygons[i].mApplyAmbient = false;
      brush = brush->GetNextBrush();
   }
   ((CButton*)GetDlgItem(IDC_AMBIENTCHECK))->SetCheck( false );
}

void AssignTexture::OnSetambientbut() 
{
   // need to do a quickbsp at least once
   if( !doneBsp )
   {
      pDoc->pWorldBsp->RebuildOptimal( NULL, 0, FALSE );
      doneBsp = true;
   }
   
   // go through all the brushes and clear out the flag
   CThredBrush * brush = pDoc->mBrushList;
   while( brush )
   {
      for( int i = 0; i < brush->mNumberOfPolygons; i++ )
         brush->mPolygons[i].mApplyAmbient = true;
      brush = brush->GetNextBrush();
   }
   ((CButton*)GetDlgItem(IDC_AMBIENTCHECK))->SetCheck( true );
}
