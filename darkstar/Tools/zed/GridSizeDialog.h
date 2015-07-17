// GridSizeDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGridSizeDialog dialog

#include "Grid.h"

class CGridSizeDialog : public CDialog
{
// Construction
public:
	CGridSizeDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGridSizeDialog)
	enum { IDD = IDD_GRIDSETTINGSDIALOG };
	double	m_GridSize;
	double	m_SnapDegrees;
	BOOL	m_ShowCoarse;
	BOOL	m_ShowFine;
	BOOL	m_UseRotationSnap;
	int		m_gridtype;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridSizeDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGridSizeDialog)
	afx_msg void OnSize16();
	afx_msg void OnSize32();
	afx_msg void OnSize64();
	afx_msg void OnSnap15();
	afx_msg void OnSnap30();
	afx_msg void OnSnap45();
	afx_msg void OnSnap60();
	afx_msg void OnSize4();
	afx_msg void OnSize8();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeGridtype();
	afx_msg void OnSnap25();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
