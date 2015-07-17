#if !defined(AFX_EDITSUN_H__BF888578_A241_11D1_8D20_006097B90B28__INCLUDED_)
#define AFX_EDITSUN_H__BF888578_A241_11D1_8D20_006097B90B28__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// EditSun.h : header file
//

#include "thredprimitives.h"

/////////////////////////////////////////////////////////////////////////////
// EditSun dialog

class EditSun : public CDialog
{
// Construction
public:
	EditSun(CWnd* pParent = NULL);   // standard constructor

	Point3F	Vector;
      
// Dialog Data
	//{{AFX_DATA(EditSun)
	enum { IDD = IDD_SUN_EDIT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EditSun)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(EditSun)
	afx_msg void OnChangeEdit();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITSUN_H__BF888578_A241_11D1_8D20_006097B90B28__INCLUDED_)
