
// NullEditDlg.h : header file

#ifndef _NULLEDITDLG_H_
#define _NULLEDITDLG_H_

/////////////////////////////////////////////////////////////////////////////
// NullEditDlg dialog

#include "resource.h"
#include "commonProps.h"

class NullEditDlg : public CDialog
{
public:
	NullEditDlg(CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(NullEditDlg)
		enum { IDD = IDD_NULLEDIT_DIALOG };
		CommonPropertyDataMembers;
	//}}AFX_DATA

// Overrides

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(NullEditDlg)
	public:
		virtual int DoModal ( void );
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(NullEditDlg)
		virtual BOOL OnInitDialog();
		CommonPropertyMethodsDeclared();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
