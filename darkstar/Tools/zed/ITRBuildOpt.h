#if !defined(AFX_ITRBUILDOPT_H__20EFAB91_85F5_11D1_8D04_006097B90B28__INCLUDED_)
#define AFX_ITRBUILDOPT_H__20EFAB91_85F5_11D1_8D04_006097B90B28__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ITRBuildOpt.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ITRBuildOpt dialog

class ITRBuildOpt : public CPropertyPage
{
	DECLARE_DYNCREATE(ITRBuildOpt)

// Construction
public:
	ITRBuildOpt();
	~ITRBuildOpt();

// Dialog Data
	//{{AFX_DATA(ITRBuildOpt)
	enum { IDD = IDD_ITR_BUILD };
	float	m_geometryScale;
	BOOL	m_lowDetail;
	float	m_planeNormalPrecision;
	float	m_planeDistancePrecision;
	float	m_pointSnapPrecision;
	float	m_textureScale;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(ITRBuildOpt)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(ITRBuildOpt)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ITRBUILDOPT_H__20EFAB91_85F5_11D1_8D04_006097B90B28__INCLUDED_)
