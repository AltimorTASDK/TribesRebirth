#if !defined(AFX_ITRSHAPEOPT_H__20EFAB93_85F5_11D1_8D04_006097B90B28__INCLUDED_)
#define AFX_ITRSHAPEOPT_H__20EFAB93_85F5_11D1_8D04_006097B90B28__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ITRShapeOpt.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ITRShapeOpt dialog

class ITRShapeOpt : public CPropertyPage
{
	DECLARE_DYNCREATE(ITRShapeOpt)

// Construction
public:
	ITRShapeOpt();
	~ITRShapeOpt();

// Dialog Data
	//{{AFX_DATA(ITRShapeOpt)
	enum { IDD = IDD_ITR_SHAPE };
	BOOL	m_linkedInterior;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(ITRShapeOpt)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(ITRShapeOpt)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ITRSHAPEOPT_H__20EFAB93_85F5_11D1_8D04_006097B90B28__INCLUDED_)
