// LightEmitterProp.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "LightEmitterProp.h"
#include "light.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLightEmitterProp property page

IMPLEMENT_DYNCREATE(CLightEmitterProp, CPropertyPage)

CLightEmitterProp::CLightEmitterProp() : CPropertyPage(CLightEmitterProp::IDD)
{
	//{{AFX_DATA_INIT(CLightEmitterProp)
	m_posString = _T("");
	m_distance3 = 0.0f;
	m_distance2 = 0.0f;
	m_distance1 = 0.0f;
	m_angleString = _T("");
	m_directionString = _T("");
	//}}AFX_DATA_INIT
}

CLightEmitterProp::~CLightEmitterProp()
{
}

void CLightEmitterProp::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLightEmitterProp)
	DDX_Text(pDX, IDC_LD_POSITIONEDIT, m_posString);
	DDX_Text(pDX, IDC_LD_DISTANCE3EDIT, m_distance3);
	DDX_Text(pDX, IDC_LD_DISTANCE2EDIT, m_distance2);
	DDX_Text(pDX, IDC_LD_DISTANCE1EDIT, m_distance1);
	DDX_Text(pDX, IDC_LD_ANGLEEDIT, m_angleString);
	DDX_Text(pDX, IDC_LD_DIRECTIONEDIT, m_directionString);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLightEmitterProp, CPropertyPage)
	//{{AFX_MSG_MAP(CLightEmitterProp)
	ON_BN_CLICKED(IDC_LD_LINEARRADIO, OnLdLinearradio)
	ON_BN_CLICKED(IDC_LD_POINTRADIO, OnLdPointradio)
	ON_BN_CLICKED(IDC_LD_QUADRATICRADIO, OnLdQuadraticradio)
	ON_BN_CLICKED(IDC_LD_SPOTRADIO, OnLdSpotradio)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLightEmitterProp message handlers

BOOL CLightEmitterProp::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
   // set the buttons
   if( m_falloff == CLightEmitter::Linear )
      ( ( CButton * )GetDlgItem( IDC_LD_LINEARRADIO ) )->SetCheck(1);
   else
      ( ( CButton * )GetDlgItem( IDC_LD_QUADRATICRADIO ) )->SetCheck(1);
      
   if( m_lightType == CLightEmitter::PointLight )
      ( ( CButton * )GetDlgItem( IDC_LD_POINTRADIO ) )->SetCheck(1);
   else
      ( ( CButton * )GetDlgItem( IDC_LD_SPOTRADIO ) )->SetCheck(1);
	
   // add an apply button....
   
	return TRUE;
}

void CLightEmitterProp::OnLdLinearradio() 
{
   m_falloff = CLightEmitter::Linear;
}

void CLightEmitterProp::OnLdPointradio() 
{
	m_lightType = CLightEmitter::PointLight;
}

void CLightEmitterProp::OnLdQuadraticradio() 
{
   m_falloff = CLightEmitter::Distance;
}

void CLightEmitterProp::OnLdSpotradio() 
{
   m_lightType = CLightEmitter::SpotLight;
}
