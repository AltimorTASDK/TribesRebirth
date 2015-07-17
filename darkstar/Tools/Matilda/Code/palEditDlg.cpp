// palEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "matilda2.h"
#include "palEditDlg.h"

#include "palHelp.h"

// Darkstar includes
//
#include "base.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// palEditDlg dialog


palEditDlg::palEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(palEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(palEditDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void palEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(palEditDlg)
	DDX_Control(pDX, ID_PAL_TRANSLUCENT, m_isTranslucent);
	DDX_Control(pDX, ID_PAL_LIST, m_palListBox);
	DDX_Control(pDX, ID_PAL_LIGHTINGUNLIT, m_lightingUnlit);
	DDX_Control(pDX, ID_PAL_LIGHTINGSMOOTH, m_lightingSmooth);
	DDX_Control(pDX, ID_PAL_LIGHTINGFLAT, m_lightingFlat);
		CommonPropertyDataExchange(pDX, 2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(palEditDlg, CDialog)
	//{{AFX_MSG_MAP(palEditDlg)
	ON_BN_CLICKED(ID_PAL_LIGHTINGFLAT, OnPalLightingflat)
	ON_BN_CLICKED(ID_PAL_LIGHTINGSMOOTH, OnPalLightingsmooth)
	ON_BN_CLICKED(ID_PAL_LIGHTINGUNLIT, OnPalLightingunlit)
	ON_LBN_DBLCLK(ID_PAL_LIST, OnDblclkPalList)
	ON_WM_PALETTECHANGED()
	ON_WM_QUERYNEWPALETTE()
		CommonPropertyMessageMap(2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// palEditDlg message handlers

void
palEditDlg::OnPalLightingsmooth() 
{
	m_lightingFlat.SetCheck(0);
	m_lightingUnlit.SetCheck(0);
	m_lightingType = TS::Material::ShadingSmooth;	
}

void
palEditDlg::OnPalLightingflat() 
{
	m_lightingSmooth.SetCheck(0);
	m_lightingUnlit.SetCheck(0);
	m_lightingType = TS::Material::ShadingFlat;
}

void
palEditDlg::OnPalLightingunlit() 
{
	m_lightingSmooth.SetCheck(0);
	m_lightingFlat.SetCheck(0);
	m_lightingType = TS::Material::ShadingNone;	
}

int
palEditDlg::DoModal(UInt32 *io_palIndex, TS::Material::ShadingType *io_lightingType) 
{
	m_selectedColor = *io_palIndex;
	m_lightingType  = *io_lightingType;

	int returnVal = CDialog::DoModal();

	*io_palIndex     = m_selectedColor;
	*io_lightingType = m_lightingType;

	return returnVal;
}

BOOL
palEditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
   InitCommonProps();	

	switch(m_lightingType) {
	case TS::Material::ShadingSmooth:
		m_lightingSmooth.SetCheck(1);
		break;

	case TS::Material::ShadingFlat:
		m_lightingFlat.SetCheck(1);
		break;

	case TS::Material::ShadingNone:
		m_lightingUnlit.SetCheck(1);
		break;
	}

	

	
	// Set up the selection box...
	//
	RECT listRect;
	int  nHeight, nWidth;

	m_palListBox.GetClientRect(&listRect);
	nHeight = listRect.bottom / 16;
	nWidth  = listRect.right / 16;

	m_palListBox.SetItemHeight(0, nHeight);
	m_palListBox.SetColumnWidth(nWidth);
		
	//get the system palette
	PALETTEENTRY pal[256];
	CWnd* pWnd = GetDlgItem( ID_PAL_LIST );
	CDC* pListDC = pWnd->GetDC();
	int err = GetSystemPaletteEntries( pListDC->GetSafeHdc(), 0, 256, pal );
	AssertFatal(err == 256, "Error retreiving palette!");

	for (int i = 0; i < 256; i++) {
		m_palListBox.AddString((LPCTSTR)i);
	}

	if (m_selectedColor >= 0 && m_selectedColor <= 255) {
		m_palListBox.SetCurSel(m_selectedColor);
	} else {
		m_palListBox.SetCurSel(0);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void palEditDlg::OnDblclkPalList() 
{
	m_selectedColor = m_palListBox.GetCurSel();
	OnOK();	
}

void palEditDlg::OnOK() 
{
	m_selectedColor = m_palListBox.GetCurSel();
	CDialog::OnOK();
}


BOOL palEditDlg::OnQueryNewPalette() 
{
	CClientDC dc(this);	
	return RealizeSysPalette( dc );
}

void palEditDlg::OnPaletteChanged(CWnd* pFocusWnd) 
{
	if (pFocusWnd == this || IsChild(pFocusWnd))
		return;

	OnQueryNewPalette();
}


CommonPropertyMethodsDefined ( palEditDlg )
