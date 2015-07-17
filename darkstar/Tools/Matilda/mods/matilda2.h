// matilda2.h : main header file for the MATILDA2 application
//

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#include "MLedit.h"
#include "detailLevelsDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CMatilda2App:
// See matilda2.cpp for the implementation of this class
//

class CMatilda2App : public CWinApp
{
public:
	CMatilda2App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMatilda2App)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMatilda2App)
	afx_msg void OnFileExit();
	afx_msg void OnFileNew();
	afx_msg void OnFileSave();
	afx_msg void OnFileOpen();
	afx_msg void OnFileImportFromText();
	afx_msg void OnFileExportToText();
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnFileSaveas();
	afx_msg void OnPaletteSetcurrentpal();
	afx_msg void OnEditPastetoend();
	afx_msg void OnChangelightingflags();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
