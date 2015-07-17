#if !defined(AFX_LIGHTEMITTERPROP_H__02ACCA55_23ED_11D2_8D61_006097B90B28__INCLUDED_)
#define AFX_LIGHTEMITTERPROP_H__02ACCA55_23ED_11D2_8D61_006097B90B28__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// LightEmitterProp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLightEmitterProp dialog

class CLightEmitterProp : public CPropertyPage
{
	DECLARE_DYNCREATE(CLightEmitterProp)

// Construction
public:
	CLightEmitterProp();
	~CLightEmitterProp();

// Dialog Data
	//{{AFX_DATA(CLightEmitterProp)
	enum { IDD = IDD_LIGHTEMITTERPROP };
	CString	m_posString;
	float	m_distance3;
	float	m_distance2;
	float	m_distance1;
	CString	m_angleString;
	CString	m_directionString;
	//}}AFX_DATA

   int m_lightType;
   int m_falloff;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CLightEmitterProp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CLightEmitterProp)
	virtual BOOL OnInitDialog();
	afx_msg void OnLdLinearradio();
	afx_msg void OnLdPointradio();
	afx_msg void OnLdQuadraticradio();
	afx_msg void OnLdSpotradio();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIGHTEMITTERPROP_H__02ACCA55_23ED_11D2_8D61_006097B90B28__INCLUDED_)
