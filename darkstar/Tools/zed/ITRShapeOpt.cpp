// ITRShapeOpt.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "ITRShapeOpt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ITRShapeOpt property page

IMPLEMENT_DYNCREATE(ITRShapeOpt, CPropertyPage)

ITRShapeOpt::ITRShapeOpt() : CPropertyPage(ITRShapeOpt::IDD)
{
	//{{AFX_DATA_INIT(ITRShapeOpt)
	m_linkedInterior = FALSE;
	//}}AFX_DATA_INIT
}

ITRShapeOpt::~ITRShapeOpt()
{
}

void ITRShapeOpt::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ITRShapeOpt)
	DDX_Check(pDX, IDC_LINKEDINTERIOR, m_linkedInterior);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ITRShapeOpt, CPropertyPage)
	//{{AFX_MSG_MAP(ITRShapeOpt)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ITRShapeOpt message handlers
