// ITRBuildOpt.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "ITRBuildOpt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ITRBuildOpt property page

IMPLEMENT_DYNCREATE(ITRBuildOpt, CPropertyPage)

ITRBuildOpt::ITRBuildOpt() : CPropertyPage(ITRBuildOpt::IDD)
{
	//{{AFX_DATA_INIT(ITRBuildOpt)
	m_geometryScale = 0.0f;
	m_lowDetail = FALSE;
	m_planeNormalPrecision = theApp.m_planeNormalPrecision;
	m_planeDistancePrecision = theApp.m_planeDistancePrecision;
	m_pointSnapPrecision = 0.0f;
	m_textureScale = 0.0f;
	//}}AFX_DATA_INIT
}

ITRBuildOpt::~ITRBuildOpt()
{
}

void ITRBuildOpt::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ITRBuildOpt)
	DDX_Text(pDX, IDC_BUILD_GEOMETRYSCALE, m_geometryScale);
	DDX_Check(pDX, IDC_BUILD_LOWDETAIL, m_lowDetail);
	DDX_Text(pDX, IDC_BUILD_NORMALPRECISION, m_planeNormalPrecision);
	DDX_Text(pDX, IDC_BUILD_PLANEDISTANCE, m_planeDistancePrecision);
	DDX_Text(pDX, IDC_BUILD_SNAPPRECISION, m_pointSnapPrecision);
	DDX_Text(pDX, IDC_BUILD_TEXTURESCALE, m_textureScale);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ITRBuildOpt, CPropertyPage)
	//{{AFX_MSG_MAP(ITRBuildOpt)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ITRBuildOpt message handlers
