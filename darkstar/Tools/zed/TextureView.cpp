// TextureView.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "TextureView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////
// CTextureListBox
BEGIN_MESSAGE_MAP(CTextureListBox, CListBox)
	//{{AFX_MSG_MAP(CTextureListBox)
   ON_WM_DRAWITEM()
   ON_WM_KEYDOWN()
   ON_WM_GETDLGCODE()
	//}}AFX_MSG_MAP                    
END_MESSAGE_MAP()

CTextureListBox::CTextureListBox( class CTextureView * TextureView )
{
   m_TextureView = TextureView;
}


void CTextureListBox::MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct )
{
   lpMeasureItemStruct->itemWidth = m_TextureView->m_ItemWidth;
   lpMeasureItemStruct->itemHeight = m_TextureView->m_ItemHeight;
}
   

void CTextureListBox::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	// Set the palette for the frames
	if( !m_TextureView->m_pFrame )
		return;

	m_TextureView->m_pFrame->m_pSurface->setPalette( m_TextureView->m_palette, false );

   if( m_TextureView->IsFloating() )
   {
      m_TextureView->m_pFrame->SetWindowPos( NULL,
         lpDrawItemStruct->rcItem.left + 2, lpDrawItemStruct->rcItem.top + 2,
         lpDrawItemStruct->rcItem.right - lpDrawItemStruct->rcItem.left - 4,
         lpDrawItemStruct->rcItem.bottom - lpDrawItemStruct->rcItem.top - 4, SWP_NOREDRAW );
   }
   else
   {
      m_TextureView->m_pFrame->SetWindowPos( NULL,
         lpDrawItemStruct->rcItem.left + 2, lpDrawItemStruct->rcItem.top + 2 + m_TextureView->CaptionSize,
         lpDrawItemStruct->rcItem.right - lpDrawItemStruct->rcItem.left - 4,
         lpDrawItemStruct->rcItem.bottom - lpDrawItemStruct->rcItem.top - 4, SWP_NOREDRAW );
   }
      
	// Redraw the textures
	if( m_TextureView->m_materialList )
	{
      if(lpDrawItemStruct->itemState & ODS_SELECTED)
      {
         HBRUSH hBrush = CreateSolidBrush( 0x000000ff );
         FillRect( lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem, hBrush );
      }
      else
      {
         HBRUSH hBrush = CreateSolidBrush( 0x00ffffff );
         FillRect( lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem, hBrush );
      }
	
   	m_TextureView->m_materialList->setDetailLevel(0);
		m_TextureView->m_pFrame->m_pMaterial = &m_TextureView->
         m_materialList->getMaterial( lpDrawItemStruct->itemID );
		m_TextureView->m_pFrame->Refresh();
      
      // draw a number in the corner ( black rect/ black text on white backdrop )
      char buffer[20];
      sprintf( buffer, "%d", lpDrawItemStruct->itemID );
      
      // get the size of the box/string
      SIZE size;
      GetTextExtentPoint( lpDrawItemStruct->hDC, buffer, strlen( buffer ), &size );
      size.cx += 4;
      size.cy += 4;
      
      RECT rect;
      rect.top = lpDrawItemStruct->rcItem.bottom - size.cy - 2;
      rect.left = lpDrawItemStruct->rcItem.left + 2;
      rect.bottom = rect.top + size.cy;
      rect.right = rect.left + size.cx;

      HBRUSH hBrush = CreateSolidBrush( 0x00000000 );
      FillRect( lpDrawItemStruct->hDC, &rect, hBrush );

      DrawText( lpDrawItemStruct->hDC, buffer, strlen( buffer), &rect, 
         DT_CENTER | DT_VCENTER | DT_SINGLELINE );
	}
}

void CTextureListBox::OnDrawItem( int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct )
{
}

UINT CTextureListBox::OnGetDlgCode()
{
   return DLGC_WANTALLKEYS;
}

CTextureListBox::~CTextureListBox()
{
}

// key down
void CTextureListBox::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
   const int sizeAmount = 10;
   const int minHeight = 100;
   const int minWidth = 50;

   // process this click
   CMainFrame * pFrame = ( CMainFrame * )AfxGetMainWnd();
   
   
   // check for ctrl ( size window by x );
   if( GetKeyState( VK_CONTROL ) & 0x8000 )
   {
      // call recalclayout twice because it saves code prob's
      switch( nChar )
      {
         case VK_LEFT:
            if( m_TextureView->CurrentSize.cx > minWidth )
               m_TextureView->CurrentSize.cx -= sizeAmount;
            break;
         case VK_RIGHT:
            m_TextureView->CurrentSize.cx += sizeAmount;
            break;
         case VK_UP:
            if( m_TextureView->CurrentSize.cy > minHeight )
               m_TextureView->CurrentSize.cy -= sizeAmount;
            break;
         case VK_DOWN:
            m_TextureView->CurrentSize.cy += sizeAmount;
            break;
      }
      
      if( ( nChar == VK_LEFT ) || ( nChar == VK_RIGHT ) || 
         ( nChar == VK_UP ) || ( nChar == VK_DOWN ) )
      {
         pFrame->RecalcLayout();
         pFrame->RecalcLayout();
            
	      TSMaterialList * tmp_materialList;
         GFXPalette * tmp_palette;
         tmp_materialList = m_TextureView->m_materialList;
         tmp_palette = m_TextureView->m_palette;
         int top = GetTopIndex();
         
         m_TextureView->Detatch();
         m_TextureView->Attach( tmp_materialList, tmp_palette );
         SetTopIndex( top );
         m_TextureView->RedrawWindow();
         m_TextureView->RedrawWindow();
         return;
      }
         
   }
   CListBox::OnKeyDown( nChar, nRepCnt, nFlags );
}

/////////////////////////////////////////////////////////////////////////////
// CTextureView

CTextureView::CTextureView() : 
   m_materialIndex( 0 ), m_materialCount( 0 ), m_palette( NULL ),
   m_materialList( NULL ), m_pFrame( NULL ), m_ListWnd( this ),
   m_ItemWidth( 64 ), m_ItemHeight( 64 )
{
}

CTextureView::~CTextureView()
{
   if( m_pFrame )
      delete m_pFrame;
}


BEGIN_MESSAGE_MAP(CTextureView, CControlBar)
	//{{AFX_MSG_MAP(CTextureView)
	ON_WM_PAINT()
   ON_WM_WINDOWPOSCHANGED()
   ON_NOTIFY( LBN_DBLCLK, 0x1230, OnDblClk )
	//}}AFX_MSG_MAP                    
END_MESSAGE_MAP()

int CTextureView::getMaterialIndex()
{
   return(m_ListWnd.GetCurSel());
}

void CTextureView::OnDblClk(NMHDR * pNotify, LRESULT * result)
{
	int Id = m_ListWnd.GetCurSel();
   
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
   if( !pFrame ) return;
   
   CMaterialProperties materialDialog;
   CTHREDDoc* pDoc;

	pDoc = (CTHREDDoc*)pFrame->MDIGetActive()->GetActiveDocument();

	if( pDoc )
	{
		materialDialog.m_materialCount = pDoc->GetMaterialList()->getMaterialsCount();
		materialDialog.m_materialList = pDoc->GetMaterialList();
		materialDialog.m_palette = pDoc->GetPalette();
      materialDialog.m_materialIndex = Id;

		materialDialog.DoModal();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTextureView message handlers

CSize CTextureView::CalcDynamicLayout( int nLength, DWORD dwMode )
{
   CRect rect;
   CSize size;
   
   GetClientRect( rect );

   // check for floating
   if( IsFloating() )
   {  
      CWnd * pParent;
                                
      // get the parent window
      pParent = GetParent();
      
      // set the window dimensions for the tree
      size.cy -= CaptionSize;   
      m_ListWnd.SetWindowPos( NULL, 0, 0, rect.right, rect.bottom, SWP_NOZORDER );
      
      EnableDocking( CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT );
   }
   else
      m_ListWnd.SetWindowPos( NULL, 0, CaptionSize, rect.right, rect.bottom - 
         CaptionSize, SWP_NOZORDER );
   
   size.cx = CurrentSize.cx;
   size.cy = CurrentSize.cy;
   
   return( size );
}


// attach a material list
void CTextureView::Attach( TSMaterialList * pMaterialList, GFXPalette * pPalette )
{
   if( m_pFrame )
      Detatch();
      
   // create a small rect for this canvas
   CRect rect;
   GetClientRect( rect );
   rect.right -=4;
   rect.bottom = rect.right;
   
	m_pFrame = new CMatCanvas;
	m_pFrame->Create( this, rect, 0x4321 );

   m_materialList = pMaterialList;
   m_palette = pPalette;
   m_materialIndex = 0;
   m_materialCount = 0;
   
   // remove all the items
   m_ListWnd.ResetContent();
   
   m_ItemWidth = rect.right;
   m_ItemHeight = m_ItemWidth;
   
	if( m_materialList )
   {
      m_materialCount = m_materialList->getMaterialsCount();
		m_materialList->setDetailLevel( 0 );
		m_pFrame->m_pMaterial = &m_materialList->getMaterial( m_materialIndex );
      m_pFrame->stretchit = TRUE;
	}
   
   // go through and add this many items to the list box
   for( int i = 0; i < m_materialCount; i ++ )
      m_ListWnd.AddString( "" );
}

void CTextureView::Detatch()
{
   if( m_pFrame )
   {
      delete( m_pFrame );
      m_pFrame = NULL;
   }
   
   // remove all the items
   m_ListWnd.ResetContent();
}


void CTextureView::OnUpdateCmdUI( CFrameWnd * pFrame, BOOL fDisable )
{
}

void CTextureView::OnPaint() 
{
   CPaintDC dc(this);
   
   // draw the caption which is used in non-float mode to grab the thing
   if( !IsFloating() )
   {
      CRect rect;
      rect.left = 0;
      rect.top = 0;
      rect.right = CurrentSize.cx;
      rect.bottom = CaptionSize;

      CBrush brush;
      brush.CreateSolidBrush( 0x00000000 );
      dc.FillRect( &rect, &brush );  
      dc.SetTextColor( 0x00ffffff );
      
      COLORREF color;
      color = dc.GetBkColor();
      dc.SetBkColor( 0x00000000 );

      CString Title;
      GetWindowText( Title );
      dc.DrawText( Title, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER );
      dc.SetBkColor( color );
   }
   
   CControlBar::OnPaint();
}

void CTextureView::OnWindowPosChanged( WINDOWPOS* lpwndpos )
{
   // check for floating
   if( IsFloating() )
   {
   }
}

