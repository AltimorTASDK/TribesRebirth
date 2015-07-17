#if !defined(AFX_LIGHTBLANKPROP_H__02ACCA52_23ED_11D2_8D61_006097B90B28__INCLUDED_)
#define AFX_LIGHTBLANKPROP_H__02ACCA52_23ED_11D2_8D61_006097B90B28__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// LightBlankProp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLightBlankProp dialog

class CLightBlankProp : public CPropertyPage
{
	DECLARE_DYNCREATE(CLightBlankProp)

// Construction
public:
	CLightBlankProp();
	~CLightBlankProp();

// Dialog Data
	//{{AFX_DATA(CLightBlankProp)
	enum { IDD = IDD_LIGHTBLANKPROP };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CLightBlankProp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CLightBlankProp)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIGHTBLANKPROP_H__02ACCA52_23ED_11D2_8D61_006097B90B28__INCLUDED_)
