#if !defined(AFX_LIGHTPROP_H__02ACCA53_23ED_11D2_8D61_006097B90B28__INCLUDED_)
#define AFX_LIGHTPROP_H__02ACCA53_23ED_11D2_8D61_006097B90B28__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// LightProp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLightProp dialog

class CLightProp : public CPropertyPage
{
	DECLARE_DYNCREATE(CLightProp)

// Construction
public:
	CLightProp();
	~CLightProp();

// Dialog Data
	//{{AFX_DATA(CLightProp)
	enum { IDD = IDD_LIGHTPROP };
	BOOL	m_AutoCheck;
	float	m_Duration;
	BOOL	m_LoopCheck;
	BOOL	m_ManageCheck;
	CString	m_Name;
	BOOL	m_Random;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CLightProp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CLightProp)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIGHTPROP_H__02ACCA53_23ED_11D2_8D61_006097B90B28__INCLUDED_)
