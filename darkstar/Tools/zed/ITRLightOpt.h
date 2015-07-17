#if !defined(AFX_ITRLIGHTOPT_H__20EFAB92_85F5_11D1_8D04_006097B90B28__INCLUDED_)
#define AFX_ITRLIGHTOPT_H__20EFAB92_85F5_11D1_8D04_006097B90B28__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ITRLightOpt.h : header file
//
#include "3dmworld.h"
/////////////////////////////////////////////////////////////////////////////
// ITRLightOpt dialog

class ITRLightOpt : public CPropertyPage
{
	DECLARE_DYNCREATE(ITRLightOpt)

// Construction
public:
	ITRLightOpt();
	~ITRLightOpt();

// Dialog Data
	//{{AFX_DATA(ITRLightOpt)
	enum { IDD = IDD_ITR_LIGHT };
	float	m_geometryScale;
	UINT	m_lightScale;
	UINT	m_emissionQuantumNumber;
	BOOL	m_useMaterialProperties;
	BOOL	m_useNormals;
	CString	m_ambientLight;
	BOOL	m_ambientOutsidePolys;
	//}}AFX_DATA

   ColorF m_ambientLightIntensity;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(ITRLightOpt)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(ITRLightOpt)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ITRLIGHTOPT_H__20EFAB92_85F5_11D1_8D04_006097B90B28__INCLUDED_)
