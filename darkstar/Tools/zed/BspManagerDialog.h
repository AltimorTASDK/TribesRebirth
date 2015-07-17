// BspManagerDialog.h : header file
//

#ifndef _BSPMANAGERDIALOG_H_
#define _BSPMANAGERDIALOG_H_

/////////////////////////////////////////////////////////////////////////////
// CBspManagerDialog dialog
#include "ConstructiveBsp.h"

class CBspManagerDialog : public CDialog
{
// Construction
public:
	CConstructiveBsp* mCurrentBsp;
	void DoDialog(CConstructiveBsp* Bsp);
	CBspManagerDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBspManagerDialog)
	enum { IDD = IDD_BSPMANAGERDIALOG };
	CProgressCtrl	m_Progress;
	int		m_CoplanarNodes;
	int		m_Nodes;
	int		m_OriginalPolys;
	int		m_Splits;
	int		m_TotalPolys;
	BOOL	m_BuildWater;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBspManagerDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBspManagerDialog)
	afx_msg void OnRebuildoptimal();
	afx_msg void OnRebuildQuick();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

