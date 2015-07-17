// VDescDlg.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "VDescDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVolDescriptionDlg dialog


CVolDescriptionDlg::CVolDescriptionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVolDescriptionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVolDescriptionDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CVolDescriptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVolDescriptionDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVolDescriptionDlg, CDialog)
	//{{AFX_MSG_MAP(CVolDescriptionDlg)
	ON_EN_CHANGE(IDC_VOLDESCEDIT, OnChangeVoldescedit)
	ON_LBN_SELCHANGE(IDC_VOLUMELIST1, OnSelchangeVolumelist1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVolDescriptionDlg message handlers

void CVolDescriptionDlg::FillDescriptions()
{
  for (int i = 0; i < TOTAL_STATES; i++)
  {
    CString tmp;

    tmp.Format("%2d - %s", i, (LPCTSTR)m_description[i]);
    ((CListBox*)GetDlgItem(IDC_VOLUMELIST1))->AddString((LPCTSTR)tmp);
  }
}

void CVolDescriptionDlg::ClearDescriptions()
{
  ((CListBox*)GetDlgItem(IDC_VOLUMELIST1))->ResetContent();
}

BOOL CVolDescriptionDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();
  
  m_currentvol = 0;

  FillDescriptions();

  ((CEdit*)GetDlgItem(IDC_VOLDESCEDIT))->ReplaceSel((LPCTSTR)m_description[m_currentvol]);
  ((CEdit*)GetDlgItem(IDC_VOLDESCEDIT))->SetSel(0, -1, FALSE);
  ((CEdit*)GetDlgItem(IDC_VOLDESCEDIT))->SetFocus();
  ((CListBox*)GetDlgItem(IDC_VOLUMELIST1))->SetCurSel(m_currentvol);

  return FALSE;

  //return TRUE;  // return TRUE unless you set the focus to a control
	            // EXCEPTION: OCX Property Pages should return FALSE
}

void CVolDescriptionDlg::OnChangeVoldescedit() 
{
  GetDlgItem(IDC_VOLDESCEDIT)->GetWindowText(m_description[m_currentvol]);

  CString tmp;

  tmp.Format("%2d - %s", m_currentvol, (LPCTSTR)m_description[m_currentvol]);
}

void CVolDescriptionDlg::OnSelchangeVolumelist1() 
{
  m_currentvol = ((CListBox*)GetDlgItem(IDC_VOLUMELIST1))->GetCurSel();

  ClearDescriptions();
  FillDescriptions();

  ((CListBox*)GetDlgItem(IDC_VOLUMELIST1))->SetCurSel(m_currentvol);

  ((CEdit*)GetDlgItem(IDC_VOLDESCEDIT))->SetSel(0, -1, FALSE);
  ((CEdit*)GetDlgItem(IDC_VOLDESCEDIT))->ReplaceSel((LPCTSTR)m_description[m_currentvol]);
  ((CEdit*)GetDlgItem(IDC_VOLDESCEDIT))->SetSel(0, -1, FALSE);
  ((CEdit*)GetDlgItem(IDC_VOLDESCEDIT))->SetFocus();
}
