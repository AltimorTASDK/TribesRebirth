// AsPalMat.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// AssignPalMatDlg dialog

#include "THREDDoc.h"

class AssignPalMatDlg : public CDialog
{
// Construction
public:
	AssignPalMatDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(AssignPalMatDlg)
	enum { IDD = IDD_PALMATDIALOG };
	CString	m_strBitmaps;
	CString	m_strPalette;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AssignPalMatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	CTHREDDoc* m_pDoc;
    char  m_texturepath[256];
    char  m_palettepath[256];

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(AssignPalMatDlg)
	afx_msg void OnAsnarray();
	afx_msg void OnAsnpalette();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
