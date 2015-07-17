// LightDlg.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "LightDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLightingDialog dialog


CLightingDialog::CLightingDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CLightingDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLightingDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLightingDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLightingDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLightingDialog, CDialog)
	//{{AFX_MSG_MAP(CLightingDialog)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLightingDialog message handlers

BOOL CLightingDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    // Initialize slider values...
	((CSliderCtrl*)GetDlgItem(IDC_LIGHTING_SLIDER))->SetRange(0, 15, TRUE);
	((CSliderCtrl*)GetDlgItem(IDC_LIGHTING_SLIDER))->SetPos(lightval);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLightingDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	UINT	new_pos;

	int which_slider = pScrollBar->GetDlgCtrlID();

	if ((nSBCode == SB_THUMBPOSITION) || (nSBCode == SB_THUMBTRACK))
	{
		switch (which_slider)
		{
		case IDC_LIGHTING_SLIDER:
            lightval = nPos;
			break;

		default:
			break;
		}
	} else {
		// Handle non-sliding event
		switch (which_slider)
		{
		case IDC_LIGHTING_SLIDER:
   			new_pos = ((CSliderCtrl*)GetDlgItem(IDC_LIGHTING_SLIDER))->GetPos();
            lightval = new_pos;
			break;

		default:
			break;
		}
	}
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}
