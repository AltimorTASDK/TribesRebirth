// GlobalSettingsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "GlobalSettingsDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGlobalSettingsDialog dialog


CGlobalSettingsDialog::CGlobalSettingsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CGlobalSettingsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGlobalSettingsDialog)
	m_PalettePath = _T("");
	m_WadPath = _T("");
	//}}AFX_DATA_INIT
}


void CGlobalSettingsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlobalSettingsDialog)
	DDX_Text(pDX, IDC_PALETTEPATH, m_PalettePath);
	DDX_Text(pDX, IDC_WADPATH, m_WadPath);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGlobalSettingsDialog, CDialog)
	//{{AFX_MSG_MAP(CGlobalSettingsDialog)
	ON_BN_CLICKED(IDC_BROWSEPAL, OnBrowsepal)
	ON_BN_CLICKED(IDC_BROWSEWAD, OnBrowsewad)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGlobalSettingsDialog message handlers

static char szPalFilter[] = "Lump Files (*.lmp)|*.lmp|All Files (*.*)|*.*||";
static char szWadFilter[] = "Wad Files (*.wad)|*.wad|All Files (*.*)|*.*||";
  

void CGlobalSettingsDialog::OnBrowsepal() 
{

	UpdateData(TRUE);
	// make the dialog
	CFileDialog dlg(TRUE, "lmp", NULL, NULL, szPalFilter);

	// show it to the user.
	if(dlg.DoModal() == IDOK ) {
		m_PalettePath = dlg.GetPathName();
	}
	UpdateData(FALSE);
}

void CGlobalSettingsDialog::OnBrowsewad() 
{
	UpdateData(TRUE);
	// make the dialog
	CFileDialog dlg(TRUE, "wad", NULL, NULL, szWadFilter);

	// show it to the user.
	if(dlg.DoModal() == IDOK ) {
		m_WadPath = dlg.GetPathName();
	}
	UpdateData(FALSE);
}

