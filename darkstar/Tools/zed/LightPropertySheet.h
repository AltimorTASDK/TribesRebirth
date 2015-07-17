#if !defined(AFX_LIGHTPROPERTYSHEET_H__CA1A4B82_267C_11D2_8D62_006097B90B28__INCLUDED_)
#define AFX_LIGHTPROPERTYSHEET_H__CA1A4B82_267C_11D2_8D62_006097B90B28__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// LightPropertySheet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLightPropertySheet

class CLightPropertySheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CLightPropertySheet)

// Construction
public:
	CLightPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CLightPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:
   CButton m_apply;
   enum { APPLY_ID = 0x4000 };
   
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLightPropertySheet)
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CLightPropertySheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(CLightPropertySheet)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIGHTPROPERTYSHEET_H__CA1A4B82_267C_11D2_8D62_006097B90B28__INCLUDED_)
