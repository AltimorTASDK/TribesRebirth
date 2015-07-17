// GlobalSettingsDialog.h : header file
//

#ifndef _GLOBALSETTINGSDIALOG_H_
#define _GLOBALSETTINGSDIALOG_H_

/////////////////////////////////////////////////////////////////////////////
// CGlobalSettingsDialog dialog

class CGlobalSettingsDialog : public CDialog
{
// Construction
public:
	CGlobalSettingsDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGlobalSettingsDialog)
	enum { IDD = IDD_GLOBALSETTINGS };
	CString	m_PalettePath;
	CString	m_WadPath;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGlobalSettingsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGlobalSettingsDialog)
	afx_msg void OnBrowsepal();
	afx_msg void OnBrowsewad();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
