// TSDialog.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "TSDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TScaleDialog dialog


TScaleDialog::TScaleDialog(CWnd* pParent /*=NULL*/)
	: CDialog(TScaleDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(TScaleDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void TScaleDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TScaleDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(TScaleDialog, CDialog)
	//{{AFX_MSG_MAP(TScaleDialog)
	ON_EN_CHANGE(IDC_TEXTURESCALE, OnChangeTexturescale)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TScaleDialog message handlers

void TScaleDialog::OnChangeTexturescale() 
{
    char  mybuf[80];

    ((CEdit*)GetDlgItem(IDC_TEXTURESCALE))->GetWindowText(mybuf, 80);

	tscale = atof(mybuf);
	
    if (tscale == 0.F)
      tscale = 1.F;
}

BOOL TScaleDialog::OnInitDialog() 
{
    char  mybuf[80];

    sprintf(&(mybuf[0]), "%6.3f", tscale);

	CDialog::OnInitDialog();
	
	// Init variables if necessary...
    ((CEdit*)GetDlgItem(IDC_TEXTURESCALE))->ReplaceSel((LPCTSTR)&(mybuf[0]), TRUE);
	((CEdit*)GetDlgItem(IDC_TEXTURESCALE))->SetSel(0, -1, FALSE);
    ((CEdit*)GetDlgItem(IDC_TEXTURESCALE))->SetFocus();

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
