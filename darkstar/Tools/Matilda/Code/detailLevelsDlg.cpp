// detailLevelsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "matilda2.h"
#include "detailLevelsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// detailLevelsDlg dialog


detailLevelsDlg::detailLevelsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(detailLevelsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(detailLevelsDlg)
	m_numLevels = 1;
	//}}AFX_DATA_INIT
}


void detailLevelsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(detailLevelsDlg)
	DDX_Text(pDX, ID_DETAIL_NUMLEVELS, m_numLevels);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(detailLevelsDlg, CDialog)
	//{{AFX_MSG_MAP(detailLevelsDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// detailLevelsDlg message handlers

int detailLevelsDlg::DoModal(int *out_levels) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	int returnVal = CDialog::DoModal();

	*out_levels = m_numLevels;
	return returnVal;
}
