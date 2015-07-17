// LightProp.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "LightProp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLightProp property page

IMPLEMENT_DYNCREATE(CLightProp, CPropertyPage)

CLightProp::CLightProp() : CPropertyPage(CLightProp::IDD)
{
	//{{AFX_DATA_INIT(CLightProp)
	m_AutoCheck = FALSE;
	m_Duration = 0.0f;
	m_LoopCheck = FALSE;
	m_ManageCheck = FALSE;
	m_Name = _T("");
	m_Random = FALSE;
	//}}AFX_DATA_INIT
}

CLightProp::~CLightProp()
{
}

void CLightProp::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLightProp)
	DDX_Check(pDX, IDC_LD_AUTOCHECK, m_AutoCheck);
	DDX_Text(pDX, IDC_LD_DURATIONEDIT, m_Duration);
	DDX_Check(pDX, IDC_LD_LOOPCHECK, m_LoopCheck);
	DDX_Check(pDX, IDC_LD_MANAGEBYMISSIONEDITOR, m_ManageCheck);
	DDX_Text(pDX, IDC_LD_NAMEEDIT, m_Name);
	DDX_Check(pDX, IDC_LD_RANDOMCHECK, m_Random);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLightProp, CPropertyPage)
	//{{AFX_MSG_MAP(CLightProp)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLightProp message handlers
