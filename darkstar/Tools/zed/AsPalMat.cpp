// AsPalMat.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "AsPalMat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// AssignPalMatDlg dialog


AssignPalMatDlg::AssignPalMatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(AssignPalMatDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(AssignPalMatDlg)
	m_strBitmaps = _T("");
	m_strPalette = _T("");
	//}}AFX_DATA_INIT
}


void AssignPalMatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AssignPalMatDlg)
	DDX_Text(pDX, IDC_ASNEDARRAY, m_strBitmaps);
	DDX_Text(pDX, IDC_ASNEDPAL, m_strPalette);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(AssignPalMatDlg, CDialog)
	//{{AFX_MSG_MAP(AssignPalMatDlg)
	ON_BN_CLICKED(IDC_ASNARRAY, OnAsnarray)
	ON_BN_CLICKED(IDC_ASNPALETTE, OnAsnpalette)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// AssignPalMatDlg message handlers

void AssignPalMatDlg::OnAsnarray() 
{
	// Browse for a material list
	UpdateData(TRUE);
	CString title = "Material List File";
	CString filter = "Material List Files (*.dml)|*.dml|All Files|*.*||";
	if(m_pDoc->LoadBrowser(m_strBitmaps, filter, title))
		UpdateData(FALSE);
	
}

void AssignPalMatDlg::OnAsnpalette() 
{
	// Browse for a palette
	UpdateData(TRUE);
	CString title = "Import Palette File";
	CString filter =
	"Palette Files (*.ppl)|*.ppl|All Files|*.*||";
	if(m_pDoc->LoadBrowser(m_strPalette, filter, title))
		UpdateData(FALSE);
	
}

BOOL AssignPalMatDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
