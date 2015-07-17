// GridSizeDialog.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "GridSizeDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridSizeDialog dialog


CGridSizeDialog::CGridSizeDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CGridSizeDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGridSizeDialog)
	m_GridSize = 0.0;
	m_SnapDegrees = 0.0;
	m_ShowCoarse = FALSE;
	m_ShowFine = FALSE;
	m_UseRotationSnap = FALSE;
	m_gridtype = USE_LINE_GRID;
	//}}AFX_DATA_INIT
}


void CGridSizeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGridSizeDialog)
	DDX_Text(pDX, IDC_GRIDSIZE, m_GridSize);
	DDV_MinMaxDouble(pDX, m_GridSize, 1., 2500.);
	DDX_Text(pDX, IDC_ROTSNAPDEGREES, m_SnapDegrees);
	DDX_Check(pDX, IDC_USEROTSNAP, m_UseRotationSnap);
	DDX_CBIndex(pDX, IDC_GRIDTYPE, m_gridtype);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGridSizeDialog, CDialog)
	//{{AFX_MSG_MAP(CGridSizeDialog)
	ON_BN_CLICKED(IDC_SIZE16, OnSize16)
	ON_BN_CLICKED(IDC_SIZE32, OnSize32)
	ON_BN_CLICKED(IDC_SIZE64, OnSize64)
	ON_BN_CLICKED(IDC_SNAP15, OnSnap15)
	ON_BN_CLICKED(IDC_SNAP30, OnSnap30)
	ON_BN_CLICKED(IDC_SNAP45, OnSnap45)
	ON_BN_CLICKED(IDC_SNAP60, OnSnap60)
	ON_BN_CLICKED(IDC_SIZE4, OnSize4)
	ON_BN_CLICKED(IDC_SIZE8, OnSize8)
	ON_CBN_SELCHANGE(IDC_GRIDTYPE, OnSelchangeGridtype)
	ON_BN_CLICKED(IDC_SNAP2_5, OnSnap25)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGridSizeDialog message handlers

void CGridSizeDialog::OnSize16() 
{
	UpdateData();
	m_GridSize	= 16.0;
	UpdateData(FALSE);
}
void CGridSizeDialog::OnSize32() 
{
	UpdateData();
	m_GridSize	= 32.0;
	UpdateData(FALSE);
}
void CGridSizeDialog::OnSize64() 
{
	UpdateData();
	m_GridSize	= 64.0;
	UpdateData(FALSE);
}
void CGridSizeDialog::OnSnap15() 
{
	UpdateData();
	m_SnapDegrees = 15.0;
	UpdateData(FALSE);
}
void CGridSizeDialog::OnSnap30() 
{
	UpdateData();
	m_SnapDegrees = 30.0;
	UpdateData(FALSE);
}

void CGridSizeDialog::OnSnap45() 
{
	UpdateData();
	m_SnapDegrees = 45.0;
	UpdateData(FALSE);
}
void CGridSizeDialog::OnSnap60() 
{
	UpdateData();
	m_SnapDegrees = 60.0;
	UpdateData(FALSE);
}

void CGridSizeDialog::OnSize4() 
{
	UpdateData();
	m_GridSize	= 4.0;
	UpdateData(FALSE);
}

void CGridSizeDialog::OnSize8() 
{
	UpdateData();
	m_GridSize	= 8.0;
	UpdateData(FALSE);
}

BOOL CGridSizeDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	((CComboBox*)GetDlgItem(IDC_GRIDTYPE))->SetCurSel(m_gridtype);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGridSizeDialog::OnSelchangeGridtype() 
{
	m_gridtype = ((CComboBox*)GetDlgItem(IDC_GRIDTYPE))->GetCurSel();
}

void CGridSizeDialog::OnSnap25() 
{
	UpdateData();
	m_SnapDegrees = 2.5;
	UpdateData(FALSE);
}
