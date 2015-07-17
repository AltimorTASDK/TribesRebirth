// LightBlankProp.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "LightBlankProp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLightBlankProp property page

IMPLEMENT_DYNCREATE(CLightBlankProp, CPropertyPage)

CLightBlankProp::CLightBlankProp() : CPropertyPage(CLightBlankProp::IDD)
{
	//{{AFX_DATA_INIT(CLightBlankProp)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CLightBlankProp::~CLightBlankProp()
{
}

void CLightBlankProp::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLightBlankProp)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLightBlankProp, CPropertyPage)
	//{{AFX_MSG_MAP(CLightBlankProp)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLightBlankProp message handlers
