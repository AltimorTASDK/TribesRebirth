// BrushGroupDialog.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CColourButton window
#ifndef _BRUSHGROUPDIALOG_H_
#define _BRUSHGROUPDIALOG_H_

#include "brushgroup.h"
#include "thredbrush.h"

class CColourButton : public CButton
{
// Construction
public:
	void SetColour(COLORREF Colour);
	COLORREF mCurrentColour;
	COLORREF GetColour();
	CColourButton();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColourButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CColourButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CColourButton)
	afx_msg void OnClicked();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};



/////////////////////////////////////////////////////////////////////////////
// CBrushGroupDialog dialog

class CBrushGroupDialog : public CDialog
{
// Construction
public:
	void UnfillCurrentGroup();
	void FillInCurrentGroup();
	void FillInNames();
	int mCurrentGroup;
	CThredBrush* mBrushList;
	CBrushGroupArray* mGroupArray;
	int DoDialog(CBrushGroupArray& Array, int CurrentGroup, CThredBrush* BrushList, CTHREDDoc* doc);
	CBrushGroupDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBrushGroupDialog)
	enum { IDD = IDD_GROUPDIALOG };
	CButton	m_BrushVisible;
	CListBox	m_BrushNameList;
	CListBox	m_GroupList;
	CEdit	m_Name;
	//CColourButton	m_ColourButton;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBrushGroupDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBrushGroupDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkList2();
	afx_msg void OnSelchangeList1();
	virtual void OnOK();
	afx_msg void OnDblclkList1();
	afx_msg void OnButtonToback();
	afx_msg void OnButtonTobottom();
	afx_msg void OnButtonToforward();
	afx_msg void OnButtonTotop();
	afx_msg void OnSelchangeList2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    CTHREDDoc*  pDoc;
};
/////////////////////////////////////////////////////////////////////////////


#endif
