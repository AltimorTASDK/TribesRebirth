// DetailSettings.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "DetailSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDetailSettings dialog


CDetailSettings::CDetailSettings(CWnd* pParent /*=NULL*/)
	: CDialog(CDetailSettings::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDetailSettings)
	m_LinkBack = FALSE;
	m_LinkBottom = FALSE;
	m_LinkFront = FALSE;
	m_LinkLeft = FALSE;
	m_LinkRight = FALSE;
	m_LinkTop = FALSE;
	m_maximumTexMip = 0;
	m_minPixels = 0;
	//}}AFX_DATA_INIT
}


void CDetailSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDetailSettings)
	DDX_Check(pDX, IDC_BACK, m_LinkBack);
	DDX_Check(pDX, IDC_BOTTOM, m_LinkBottom);
	DDX_Check(pDX, IDC_FRONT, m_LinkFront);
	DDX_Check(pDX, IDC_LEFT, m_LinkLeft);
	DDX_Check(pDX, IDC_RIGHT, m_LinkRight);
	DDX_Check(pDX, IDC_TOP, m_LinkTop);
	DDX_Text(pDX, IDC_BUILD_MAXTEXTURE, m_maximumTexMip);
	DDX_Text(pDX, IDC_MIN_PIXELS, m_minPixels);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDetailSettings, CDialog)
	//{{AFX_MSG_MAP(CDetailSettings)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDetailSettings message handlers
