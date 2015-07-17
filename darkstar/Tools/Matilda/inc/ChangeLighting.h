// ChangeLighting.h : header file
//
#include "ts_material.h"
/////////////////////////////////////////////////////////////////////////////
// CChangeLighting dialog

class CChangeLighting : public CDialog
{
// Construction
public:
	CChangeLighting(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChangeLighting)
	enum { IDD = IDD_LIGHTMODIFY };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChangeLighting)
	public:
		virtual int DoModal(TS::Material::ShadingType *out_lightType);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	TS::Material::ShadingType m_lightMode;
	// Generated message map functions
	//{{AFX_MSG(CChangeLighting)
	afx_msg void OnFlat();
	afx_msg void OnSmooth();
	afx_msg void OnUnlit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
