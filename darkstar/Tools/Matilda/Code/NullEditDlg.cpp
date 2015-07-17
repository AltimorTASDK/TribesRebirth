
// NullEditDlg.cpp : implementation file

#include <math.h>
#include "stdafx.h"
#include "NullEditDlg.h"
#include "types.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// NullEditDlg dialog

NullEditDlg::NullEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(NullEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(NullEditDlg)
	//}}AFX_DATA_INIT
}


void NullEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NullEditDlg)
		CommonPropertyDataExchange(pDX, 3);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(NullEditDlg, CDialog)
	//{{AFX_MSG_MAP(NullEditDlg)
		CommonPropertyMessageMap(3)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// NullEditDlg message handlers

int NullEditDlg::DoModal ( void )
{
	return ( CDialog::DoModal() );
}

BOOL NullEditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
   InitCommonProps();	
	return TRUE;
}


CommonPropertyMethodsDefined (NullEditDlg)
