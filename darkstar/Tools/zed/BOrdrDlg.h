// BOrdrDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBrushOrderDialog dialog

#include "thredbrush.h"


class CBrushOrderDialog : public CDialog
{
// Construction
public:
	CBrushOrderDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBrushOrderDialog)
	enum { IDD = IDD_BRUSHORDERDIALOG };
	CListBox	m_BrushList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBrushOrderDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBrushOrderDialog)
	afx_msg void OnSelchangeBrushlist();
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonBack();
	afx_msg void OnButtonBottom();
	afx_msg void OnButtonForward();
	afx_msg void OnButtonTop();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    void  FillInNames() ;

public:
  CTHREDDoc*  pDoc;

};
