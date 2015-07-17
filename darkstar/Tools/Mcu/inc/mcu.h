// mcu.h : main header file for the MCU application
//

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CMcuApp:
// See mcu.cpp for the implementation of this class
//

class CMcuApp : public CWinApp
{
public:
	CMcuApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMcuApp)
	public:
	virtual BOOL InitInstance();
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementation

	int	fUsePaths;
	int	fBatch;

	//{{AFX_MSG(CMcuApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileUse3dspaths();
	afx_msg void OnUpdateFileUse3dspaths(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

