// ITRLightOpt.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "ITRLightOpt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ITRLightOpt property page

IMPLEMENT_DYNCREATE(ITRLightOpt, CPropertyPage)

ITRLightOpt::ITRLightOpt() : CPropertyPage(ITRLightOpt::IDD)
{
	//{{AFX_DATA_INIT(ITRLightOpt)
	m_geometryScale = 0.0f;
	m_lightScale = 0;
	m_emissionQuantumNumber = 0;
	m_useMaterialProperties = FALSE;
	m_useNormals = FALSE;
   m_ambientLight = "";
	m_ambientOutsidePolys = FALSE;
	//}}AFX_DATA_INIT
   
}

ITRLightOpt::~ITRLightOpt()
{
}

void ITRLightOpt::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ITRLightOpt)
	DDX_Text(pDX, IDC_LIGHT_GEOMETRYSCALE, m_geometryScale);
	DDX_Text(pDX, IDC_LIGHT_LIGHTSCALE, m_lightScale);
	DDX_Text(pDX, IDC_LIGHT_QUANTUM, m_emissionQuantumNumber);
	DDX_Check(pDX, IDC_LIGHT_USEMATERIAL, m_useMaterialProperties);
	DDX_Check(pDX, IDC_LIGHT_USENORMALS, m_useNormals);
	DDX_Text(pDX, IDC_AMBIENTLIGHT, m_ambientLight);
	DDX_Check(pDX, IDC_AMBIENT_OUTSIDE, m_ambientOutsidePolys);
	//}}AFX_DATA_MAP
   
   if( pDX->m_bSaveAndValidate )
   {
      // grab the three color componants
      char * buf = new char[ strlen( m_ambientLight ) + 1 ];
      strcpy( buf, m_ambientLight );
      
      float r, g, b;
      sscanf( buf, "%f, %f, %f", &r, &g, &b );
      
      // check the values
      if( r < 0.0f ) r = 0.0f; if( r > 1.0f ) r = 1.0f;
      if( g < 0.0f ) g = 0.0f; if( g > 1.0f ) g = 1.0f;
      if( b < 0.0f ) b = 0.0f; if( b > 1.0f ) b = 1.0f;
      
      // set it
      m_ambientLightIntensity.set( r, g, b );
      
      delete [] buf;
   }
   else
   {
      // set the string
      m_ambientLight.Format( "%f, %f, %f", m_ambientLightIntensity.red,
         m_ambientLightIntensity.green, m_ambientLightIntensity.blue );
      GetDlgItem( IDC_AMBIENTLIGHT )->SetWindowText( m_ambientLight );
   }
}


BEGIN_MESSAGE_MAP(ITRLightOpt, CPropertyPage)
	//{{AFX_MSG_MAP(ITRLightOpt)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
