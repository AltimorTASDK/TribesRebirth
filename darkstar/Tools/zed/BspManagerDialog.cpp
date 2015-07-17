// BspManagerDialog.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "BspManagerDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBspManagerDialog dialog


CBspManagerDialog::CBspManagerDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CBspManagerDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBspManagerDialog)
	m_CoplanarNodes = 0;
	m_Nodes = 0;
	m_OriginalPolys = 0;
	m_Splits = 0;
	m_TotalPolys = 0;
	m_BuildWater = FALSE;
	//}}AFX_DATA_INIT
}


void CBspManagerDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBspManagerDialog)
	DDX_Control(pDX, IDC_PROGRESS1, m_Progress);
	DDX_Text(pDX, IDC_COPLANAR_NODES, m_CoplanarNodes);
	DDX_Text(pDX, IDC_NODES, m_Nodes);
	DDX_Text(pDX, IDC_ORIGINAL_POLYS, m_OriginalPolys);
	DDX_Text(pDX, IDC_SPLITS, m_Splits);
	DDX_Text(pDX, IDC_TOTAL_POLYS, m_TotalPolys);
	DDX_Check(pDX, IDC_BUILDWATER, m_BuildWater);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBspManagerDialog, CDialog)
	//{{AFX_MSG_MAP(CBspManagerDialog)
	ON_BN_CLICKED(IDC_REBUILDOPTIMAL, OnRebuildoptimal)
	ON_BN_CLICKED(IDC_RebuildQuick, OnRebuildQuick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBspManagerDialog message handlers

void CBspManagerDialog::DoDialog(CConstructiveBsp* Bsp)
{
	// get the current bsp
	mCurrentBsp = Bsp;

	// do ourselves
	DoModal();
}


void CBspManagerDialog::OnRebuildoptimal() 
{
	UpdateData(TRUE);
	m_Progress.ShowWindow(SW_SHOW);
	mCurrentBsp->RebuildOptimal(&m_Progress, m_BuildWater, FALSE);	

    // TEST ONLY
	//mCurrentBsp->RebuildOptimal(&m_Progress, m_BuildWater, TRUE);
    // TEST ONLY

	EndDialog(IDOK);
}

void CBspManagerDialog::OnRebuildQuick() 
{
	UpdateData(TRUE);
	m_Progress.ShowWindow(SW_SHOW);
	mCurrentBsp->RebuildQuick(&m_Progress, m_BuildWater, TRUE);	
	EndDialog(IDOK);
}

