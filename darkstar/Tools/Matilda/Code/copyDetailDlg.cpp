// copyDetailDlg.cpp : implementation file
//

#include "stdafx.h"
#include "matilda2.h"
#include "copyDetailDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// copyDetailDlg dialog


copyDetailDlg::copyDetailDlg(CWnd* pParent /*=NULL*/)
	: CDialog(copyDetailDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(copyDetailDlg)
	m_from = 0;
	m_to = 0;
	//}}AFX_DATA_INIT
}


void copyDetailDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(copyDetailDlg)
	DDX_Text(pDX, ID_COPY_FROM, m_from);
	DDX_Text(pDX, ID_COPY_TO, m_to);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(copyDetailDlg, CDialog)
	//{{AFX_MSG_MAP(copyDetailDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// copyDetailDlg message handlers

int copyDetailDlg::DoModal(int *out_copyFrom, int *out_copyTo) 
{
	int returnVal = CDialog::DoModal();

	*out_copyFrom = m_from;
	*out_copyTo   = m_to;

	return returnVal;
}
