// MaterialProperties.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "itrbasiclighting.h"
#include "itrshapeedit.h"
#include "MaterialProperties.h"
#include "g_bitmap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMaterialProperties dialog


int CMaterialProperties::m_materialCount = 0;
ITRBasicLighting::MaterialPropList CMaterialProperties::m_propertyInfo;

CMaterialProperties::CMaterialProperties(CWnd* pParent /*=NULL*/)
	: CDialog(CMaterialProperties::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMaterialProperties)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
   m_materialIndex = 0;
   m_pFrame = NULL;
   m_stretchit = 0;
   
   // need an init flag because child windows are updated prior to initdialog
   m_init = FALSE;
}


void CMaterialProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMaterialProperties)
	DDX_Control(pDX, IDC_MP_COLORPICK, m_colorPick);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMaterialProperties, CDialog)
	//{{AFX_MSG_MAP(CMaterialProperties)
	ON_BN_CLICKED(IDC_MP_COLORPICK, OnMpColorpick)
	ON_BN_CLICKED(IDC_MP_FLATCOLORRADIO, OnMpFlatcolorradio)
	ON_BN_CLICKED(IDC_MP_FLATEMISSIONRADIO, OnMpFlatemissionradio)
	ON_BN_CLICKED(IDC_MP_MODIFYCHECK, OnMpModifycheck)
	ON_BN_CLICKED(IDC_MP_STRETCHCHECK, OnMpStretchcheck)
	ON_WM_PAINT()
	ON_NOTIFY(UDN_DELTAPOS, IDC_MP_SPINMAT, OnDeltaposMpSpinmat)
	ON_EN_CHANGE(IDC_MP_MATINDEX, OnChangeMpMatindex)
	ON_EN_CHANGE(IDC_MP_COLOREDIT, OnChangeMpColoredit)
	ON_EN_CHANGE(IDC_MP_INTENSITYEDIT, OnChangeMpIntensityedit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMaterialProperties message handlers

void CMaterialProperties::OnMpStretchcheck() 
{
   m_stretchit ^= 1;
   m_pFrame->stretchit = m_stretchit;
   OnPaint();
}

#define CMATERIALPROPTYPE           "CMaterialProperties"
#define CMATERIALPROPNUMMATERIALS   "NumMaterials"
#define CMATERIALPROPMODIFIED       "Modified"
#define CMATERIALPROPEMISSIONTYPE   "EmissionType"
#define CMATERIALPROPCOLOR          "Color"
#define CMATERIALPROPINTENSITY      "Intensity"
#define CMATERIALPROPMODIFIEDPHONG  "ModifiedPhong"
#define CMATERIALPROPPHONGEXP       "PhongExp"
#define CENDMATERIALPROPTYPE        "End CMaterialProperties"

void CMaterialProperties::Serialize( CThredParser & Parser )
{
	Parser.SetEnd( CENDMATERIALPROPTYPE );

   CString String;
   
	if(Parser.IsStoring ) 
   {
 		// we want to write our our transaction type
		Parser.WriteString( CMATERIALPROPTYPE );
      
      Parser.WriteInt( CMATERIALPROPNUMMATERIALS, m_materialCount );
      
      for( int i = 0; i < m_materialCount; i++ )
      {
         Parser.WriteBool( CMATERIALPROPMODIFIED, m_propertyInfo[i].modified );
         if( m_propertyInfo[i].emissionType == ITRBasicLighting::MaterialProp::flatColor )
            Parser.WriteInt( CMATERIALPROPEMISSIONTYPE, 0 );
         if( m_propertyInfo[i].emissionType == ITRBasicLighting::MaterialProp::flatEmission )
            Parser.WriteInt( CMATERIALPROPEMISSIONTYPE, 1 );
         
         String.Format( "%f %f %f", m_propertyInfo[i].color.red, 
            m_propertyInfo[i].color.green, m_propertyInfo[i].color.blue );
         Parser.WriteString( CMATERIALPROPCOLOR, String );
         String.Format( "%f", m_propertyInfo[i].intensity );
         Parser.WriteString( CMATERIALPROPINTENSITY, String );
         
//         Parser.WriteBool( CMATERIALPROPMODIFIEDPHONG, m_propertyInfo[i].modifiedPhong );
//         String.Format( "%f", m_propertyInfo[i].phongExp );
//         Parser.WriteString( CMATERIALPROPPHONGEXP, String );
      }
               
		// we want to write our our transaction type
		Parser.WriteString( CENDMATERIALPROPTYPE );
   }
   else
   {
      // get the key
      if( !Parser.GetKey( CMATERIALPROPTYPE ) )
         return;

      // get the number of materials to load info for
      Parser.GetKeyInt( CMATERIALPROPNUMMATERIALS, m_materialCount );
      
      m_propertyInfo.clear();
      m_propertyInfo.compact();
      m_propertyInfo.setSize( m_materialCount );
      
      int intVal;
      
      // for msvc (*&#$
      ITRBasicLighting::MaterialProp tmpMaterialProp;
      
      for( int i = 0; i < m_materialCount; i++ )
      {
         memcpy( &m_propertyInfo[i], &tmpMaterialProp, sizeof( ITRBasicLighting::MaterialProp ) );
         
         Parser.GetKeyBool( CMATERIALPROPMODIFIED, m_propertyInfo[i].modified );
         Parser.GetKeyInt( CMATERIALPROPEMISSIONTYPE, intVal );
         
         if( intVal == 0 )
            m_propertyInfo[i].emissionType = ITRBasicLighting::MaterialProp::flatColor;
         if( intVal == 1 )
            m_propertyInfo[i].emissionType = ITRBasicLighting::MaterialProp::flatEmission;
            
         Parser.GetKeyString( CMATERIALPROPCOLOR, String );
         sscanf( LPCTSTR( String ), "%f %f %f",
            &m_propertyInfo[i].color.red,         
            &m_propertyInfo[i].color.green,         
            &m_propertyInfo[i].color.blue );
         Parser.GetKeyFloat( CMATERIALPROPINTENSITY, m_propertyInfo[i].intensity );
         
//         Parser.GetKeyBool( CMATERIALPROPMODIFIEDPHONG, m_propertyInfo[i].modifiedPhong );
//         Parser.GetKeyFloat( CMATERIALPROPPHONGEXP, m_propertyInfo[i].phongExp );
      }
      
		Parser.GotoEnd();
   }
}

BOOL CMaterialProperties::OnInitDialog() 
{
	CDialog::OnInitDialog();

   int oldCount = m_propertyInfo.size();
   
   m_propertyInfo.setSize( m_materialCount );
   
   // for msvc (*&#$
   ITRBasicLighting::MaterialProp tmpMaterialProp;
   
   // set the defaults
   for( int i = oldCount; i < m_materialCount; i++ )
   {
      // msvc #$##$
      memcpy( &m_propertyInfo[ i ], &tmpMaterialProp, sizeof( ITRBasicLighting::MaterialProp ) );
      
      m_propertyInfo[ i ].modified = FALSE;
      m_propertyInfo[ i ].emissionType = ITRBasicLighting::MaterialProp::flatColor;
      m_propertyInfo[ i ].color.red = 1.0;
      m_propertyInfo[ i ].color.blue = 1.0;
      m_propertyInfo[ i ].color.green = 1.0;
      m_propertyInfo[ i ].intensity = 1.0;
      m_propertyInfo[ i ].modifiedPhong = FALSE;
      m_propertyInfo[ i ].phongExp = 0.0;
      m_propertyInfo[ i ].pBitmap = NULL;
      m_propertyInfo[ i ].pEmissionMask = NULL;
   }
   
	CWnd * tmpWnd = GetDlgItem( IDC_MP_CANVAS );
	RECT rect;
	tmpWnd->GetClientRect( &rect );
   
	m_pFrame = new CMatCanvas;
	m_pFrame->Create( tmpWnd, rect, 0x1234 );

   m_pFrame->stretchit = m_stretchit;

	if( m_materialList )
   {
		m_materialList->setDetailLevel( 0 );
		m_pFrame->m_pMaterial = &m_materialList->getMaterial( m_materialIndex );
	}

	// Set up the spin controls
	((CSpinButtonCtrl*)GetDlgItem(IDC_MP_SPINMAT))->SetRange(0, m_materialCount - 1 );
	((CSpinButtonCtrl*)GetDlgItem(IDC_MP_SPINMAT))->SetPos( m_materialIndex );

   m_init = TRUE;

   if( m_pFrame )
      UpdateMaterialInfo();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMaterialProperties::OnPaint() 
{
	
	// Set the palette for the frames
	if( !m_pFrame )
		return;

	m_pFrame->m_pSurface->setPalette( m_palette, false );

	// Redraw the textures
	if( m_materialList )
	{
		m_materialList->setDetailLevel(0);
		m_pFrame->m_pMaterial = &m_materialList->getMaterial( m_materialIndex );
		m_pFrame->Refresh();
	}

	CPaintDC dc( this );
}

void CMaterialProperties::OnCancel() 
{
	delete m_pFrame;
	
	CDialog::OnCancel();
}

void CMaterialProperties::OnDeltaposMpSpinmat(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	int newval = pNMUpDown->iPos + pNMUpDown->iDelta;

	if( ( newval >= 0 ) && ( newval < m_materialCount ) )
	{
		char str_val[10];
		sprintf( str_val, "%d", newval );
		((CEdit*)GetDlgItem( IDC_MP_MATINDEX ))->SetSel(0, -1, TRUE);
		((CEdit*)GetDlgItem( IDC_MP_MATINDEX ))->ReplaceSel((LPCTSTR)str_val, FALSE);
		m_materialIndex = newval;
		OnPaint();
	}

	*pResult = 0;
}

void CMaterialProperties::OnChangeMpMatindex() 
{
   char  str_val[10];
   int   tmp;

   ((CEdit*)GetDlgItem(IDC_MP_MATINDEX))->GetLine(0, (LPTSTR)str_val);

   tmp = atoi(str_val);

   if( ( tmp >= 0 ) && ( tmp < m_materialCount ) )
   {
      m_materialIndex = tmp;
      UpdateMaterialInfo();
   }
}

void CMaterialProperties::UpdateMaterialInfo()
{
   if( !m_init )
      return;
      
   char  buf[ 128 ];

   if( m_pFrame )
   {
      if ((m_pFrame->m_pMaterial->fParams.fFlags & TSMaterial::MatFlags ) 
         == TSMaterial::MatTexture)
      {
         const GFXBitmap*  bmp = m_pFrame->m_pMaterial->getTextureMap();
         sprintf(buf, "(%d x %d)", bmp->width, bmp->height);
         ((CEdit*)GetDlgItem(IDC_MP_VIEWSIZE))->SetSel(0, -1, FALSE );
         ((CEdit*)GetDlgItem(IDC_MP_VIEWSIZE))->ReplaceSel( buf );
      } 
      else 
      {
         sprintf(buf, "Null Texture");
         ((CEdit*)GetDlgItem(IDC_MP_VIEWSIZE))->SetSel(0, -1, FALSE );
         ((CEdit*)GetDlgItem(IDC_MP_VIEWSIZE))->ReplaceSel( buf );
      }
   }

   // go through and fill the info in
   
   // do the color info
   sprintf( buf, "%f %f %f",
      m_propertyInfo[ m_materialIndex ].color.red,
      m_propertyInfo[ m_materialIndex ].color.green,
      m_propertyInfo[ m_materialIndex ].color.blue );
   ((CEdit*)GetDlgItem(IDC_MP_COLOREDIT))->SetSel(0, -1, FALSE );
   ((CEdit*)GetDlgItem(IDC_MP_COLOREDIT))->ReplaceSel( buf );
   
   // do the intensity
   sprintf( buf, "%f", m_propertyInfo[ m_materialIndex ].intensity );
   ((CEdit*)GetDlgItem(IDC_MP_INTENSITYEDIT))->SetSel(0, -1, FALSE );
   ((CEdit*)GetDlgItem(IDC_MP_INTENSITYEDIT))->ReplaceSel( buf );
   
   // check the flags
   if( m_propertyInfo[ m_materialIndex ].emissionType ==
	   ITRBasicLighting::MaterialProp::flatColor )
   {
      ((CButton*)GetDlgItem(IDC_MP_FLATCOLORRADIO))->SetCheck( TRUE );
      ((CButton*)GetDlgItem(IDC_MP_FLATEMISSIONRADIO))->SetCheck( FALSE );
   }
   else
   {
      ((CButton*)GetDlgItem(IDC_MP_FLATCOLORRADIO))->SetCheck( FALSE );
      ((CButton*)GetDlgItem(IDC_MP_FLATEMISSIONRADIO))->SetCheck( TRUE );
   }
   
   bool bModified = m_propertyInfo[ m_materialIndex ].modified;
   
   // check if modified or not
   ((CButton*)GetDlgItem(IDC_MP_MODIFYCHECK))->SetCheck( bModified );

   // check for null texture
   if( m_pFrame && !( m_pFrame->m_pMaterial->fParams.fFlags & 
      TSMaterial::MatFlags ) )
   {
      GetDlgItem( IDC_MP_MODIFYCHECK )->EnableWindow( FALSE );
      bModified = FALSE;
   }
   else
      GetDlgItem( IDC_MP_MODIFYCHECK )->EnableWindow( TRUE );
      
   
   EnableControls( bModified );
}

void CMaterialProperties::EnableControls( bool bModified )
{
   // activate/deactivate the windows
   GetDlgItem( IDC_MP_COLOREDIT )->EnableWindow( bModified );
   GetDlgItem( IDC_MP_INTENSITYEDIT )->EnableWindow( bModified );
   GetDlgItem( IDC_MP_FLATCOLORRADIO )->EnableWindow( bModified );
   GetDlgItem( IDC_MP_FLATEMISSIONRADIO )->EnableWindow( bModified );
   GetDlgItem( IDC_MP_COLORPICK )->EnableWindow( bModified );

   RedrawWindow();
}

void CMaterialProperties::OnMpFlatcolorradio() 
{
   if( ( ( CButton* )GetDlgItem( IDC_MP_FLATCOLORRADIO ) )->GetCheck() )
      m_propertyInfo[ m_materialIndex ].emissionType =
	  ITRBasicLighting::MaterialProp::flatColor;
}

void CMaterialProperties::OnMpFlatemissionradio() 
{
   if( ( ( CButton* )GetDlgItem( IDC_MP_FLATEMISSIONRADIO ) )->GetCheck() )
      m_propertyInfo[ m_materialIndex ].emissionType =
	  ITRBasicLighting::MaterialProp::flatEmission;
}

void CMaterialProperties::OnMpModifycheck() 
{
   bool bModified;
   bModified = ( ( CButton * )GetDlgItem( IDC_MP_MODIFYCHECK ) )->GetCheck();
   
   m_propertyInfo[ m_materialIndex ].modified = bModified;
   
   EnableControls( bModified );
}


void CMaterialProperties::OnChangeMpColoredit() 
{
   CString text;
   
   GetDlgItem( IDC_MP_COLOREDIT )->GetWindowText( text );
   
   sscanf( text, "%f %f %f", 
      &m_propertyInfo[ m_materialIndex ].color.red,
      &m_propertyInfo[ m_materialIndex ].color.green,
      &m_propertyInfo[ m_materialIndex ].color.blue );

   m_colorPick.SetColor( m_propertyInfo[ m_materialIndex ].color );
}

void CMaterialProperties::OnChangeMpIntensityedit() 
{
   CString text;
   
   GetDlgItem( IDC_MP_INTENSITYEDIT )->GetWindowText( text );
   
   sscanf( text, "%f", &m_propertyInfo[ m_materialIndex ].intensity );
}

void CMaterialProperties::OnMpColorpick() 
{
   CColorDialog  Dialog;
   COLORREF       Color;
   
   Dialog.m_cc.lStructSize = sizeof(CHOOSECOLOR);
   Dialog.m_cc.Flags = CC_FULLOPEN | CC_ENABLEHOOK;

   // create the color to set in the dialog
   Color = 0;
   Color |= ( ( 0xff & int( m_propertyInfo[ m_materialIndex ].color.blue * 255 ) ) << 16 );
   Color |= ( ( 0xff & int( m_propertyInfo[ m_materialIndex ].color.green  * 255 ) ) << 8 );
   Color |= ( ( 0xff & int( m_propertyInfo[ m_materialIndex ].color.red * 255 ) ) );
   Dialog.m_cc.rgbResult = Color;
   Dialog.m_cc.Flags |= CC_RGBINIT;
   
   // create a color dialog
   if( Dialog.DoModal() == IDOK )
   {
      Color = Dialog.GetColor();
      
      float blue, green, red;

      red = float( Color & 0xFF) / 255.F;
      green = float( ( Color >> 8 ) & 0xFF ) / 255.F;
      blue = float( ( Color >> 16 ) & 0xFF ) / 255.F;

      m_propertyInfo[ m_materialIndex ].color.blue = blue;
      m_propertyInfo[ m_materialIndex ].color.red = red;
      m_propertyInfo[ m_materialIndex ].color.green = green;
      
      CString text;
      text.Format( "%f %f %f", red, green, blue );
      GetDlgItem( IDC_MP_COLOREDIT )->SetWindowText( text );
   }
}

// initialize the material properties list with x count of materials
void CMaterialProperties::Initialize( int MatCount )
{
   m_materialCount = MatCount;
   m_propertyInfo.clear();
   m_propertyInfo.compact();
   m_propertyInfo.setSize( m_materialCount );
   
   // for msvc (*&#$
   ITRBasicLighting::MaterialProp tmpMaterialProp;
   
   // set the defaults
   for( int i = 0; i < m_materialCount; i++ )
   {
      // msvc #$##$
      memcpy( &m_propertyInfo[ i ], &tmpMaterialProp, sizeof( ITRBasicLighting::MaterialProp ) );
      
      m_propertyInfo[ i ].modified = FALSE;
      m_propertyInfo[ i ].emissionType = ITRBasicLighting::MaterialProp::flatColor;
      m_propertyInfo[ i ].color.red = 1.0;
      m_propertyInfo[ i ].color.blue = 1.0;
      m_propertyInfo[ i ].color.green = 1.0;
      m_propertyInfo[ i ].intensity = 1.0;
      m_propertyInfo[ i ].modifiedPhong = FALSE;
      m_propertyInfo[ i ].phongExp = 0.0;
      m_propertyInfo[ i ].pBitmap = NULL;
      m_propertyInfo[ i ].pEmissionMask = NULL;
   }
}

//-------------------------------------------------------
// the color button stuff below
//-------------------------------------------------------

void CColorButton::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
   COLORREF       Color;
   
   // create the color to set in the dialog
   Color = 0;
   Color |= ( ( 0xff & int( m_color.blue * 255 ) ) << 16 );
   Color |= ( ( 0xff & int( m_color.green * 255 ) ) << 8 );
   Color |= ( ( 0xff & int( m_color.red * 255 ) ) );
   
	// get a cdc
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC );

	// now draw a solid rectangle
	pDC->FillSolidRect(&lpDrawItemStruct->rcItem, Color );

	// if we have the focus
	if(lpDrawItemStruct->itemState & ODS_FOCUS ) 
   {
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

void CColorButton::SetColor( const ITRBasicLighting::Color& color )
{
   m_color.red = color.red;
   m_color.blue = color.blue;
   m_color.green = color.green;
   
   RedrawWindow();
}

