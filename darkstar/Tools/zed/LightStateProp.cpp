// LightStateProp.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "light.h"
#include "LightStateProp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLightStateProp property page

IMPLEMENT_DYNCREATE(CLightStateProp, CPropertyPage)

CLightStateProp::CLightStateProp() : CPropertyPage(CLightStateProp::IDD)
{
	//{{AFX_DATA_INIT(CLightStateProp)
	m_duration = 0.0f;
	m_colorString = _T("");
	//}}AFX_DATA_INIT
   m_color.set( 0.f, 0.f, 0.f );
}

CLightStateProp::~CLightStateProp()
{
}

void CLightStateProp::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLightStateProp)
   DDX_Control(pDX, IDC_LD_COLORPICK, m_colorButton );
	DDX_Text(pDX, IDC_EDIT1, m_duration);
	DDX_Text(pDX, IDC_LD_COLOREDIT, m_colorString);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLightStateProp, CPropertyPage)
	//{{AFX_MSG_MAP(CLightStateProp)
	ON_BN_CLICKED(IDC_LD_COLORPICK, OnLdColorpick)
	ON_EN_CHANGE(IDC_LD_COLOREDIT, OnChangeLdColoredit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLightStateProp message handlers

void CLightStateProp::OnLdColorpick() 
{
   CColorDialog  Dialog;
   COLORREF       Color;
   
   Dialog.m_cc.lStructSize = sizeof(CHOOSECOLOR);
   Dialog.m_cc.Flags = CC_FULLOPEN | CC_ENABLEHOOK;

   // create the color to set in the dialog
   Color = 0;
   Color |= ( ( 0xff & int( m_color.blue * 255 ) ) << 16 );
   Color |= ( ( 0xff & int( m_color.green * 255 ) ) << 8 );
   Color |= ( ( 0xff & int( m_color.red * 255 ) ) );
   Dialog.m_cc.rgbResult = Color;
   Dialog.m_cc.Flags |= CC_RGBINIT;
   
   // create a color dialog
   if( Dialog.DoModal() == IDOK)
   {
      Color = Dialog.GetColor();
      
      float blue, green, red;

      red = float( Color & 0xFF) / 255.F;
      green = float( ( Color >> 8 ) & 0xFF ) / 255.F;
      blue = float( ( Color >> 16 ) & 0xFF ) / 255.F;

      // set the member data fields
      m_color.blue = blue;
      m_color.red = red;
      m_color.green = green;
   
      m_colorString.Format( "%f, %f, %f", red, green, blue );

      // set the string
      GetDlgItem( IDC_LD_COLOREDIT )->SetWindowText( m_colorString );
   }

   // redraw the little button
   m_colorButton.RedrawWindow();
}

BOOL CLightStateProp::OnInitDialog() 
{
   // set the ptr for the button
   m_colorButton.SetDialog( this );
   
   // set the color string
   m_colorString.Format( "%f, %f, %f", m_color.red,
      m_color.green, m_color.blue );
      
	CPropertyPage::OnInitDialog();
	
	return TRUE;  
}

// ----------------------------------------------------------

void CMyColorButton::SetDialog( CLightStateProp * pDialog )
{
   m_dialog = pDialog;
}

// ----------------------------------------------------------

void CMyColorButton::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
   COLORREF       Color;
   
   ColorF col = m_dialog->m_color;

   // create the color to set in the dialog
   Color = 0;
   Color |= ( ( 0xff & int( col.blue * 255 ) ) << 16 );
   Color |= ( ( 0xff & int( col.green * 255 ) ) << 8 );
   Color |= ( ( 0xff & int( col.red * 255 ) ) );
   
	// get a cdc
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC );

	// now draw a solid rectangle
	pDC->FillSolidRect(&lpDrawItemStruct->rcItem, Color );

	// if we have the focus
	if(lpDrawItemStruct->itemState & ODS_FOCUS ) {
		// get a null brush
		CBrush *NullBrush = CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)), *OldBrush;

		// select the brush
		OldBrush = pDC->SelectObject(NullBrush );

		// draw a cute rectangle around it
		pDC->Rectangle(&lpDrawItemStruct->rcItem);

		// get old
		pDC->SelectObject(OldBrush );
	}
}

//----------------------------------------------------------

void CLightStateProp::OnChangeLdColoredit() 
{
   // get the text      
   GetDlgItem( IDC_LD_COLOREDIT )->GetWindowText( m_colorString );
   
   sscanf( LPCTSTR( m_colorString ), "%f, %f, %f",
      &m_color.red, &m_color.green, &m_color.blue );

   // redraw the button
   m_colorButton.RedrawWindow();
}
