// CreateSpheroidDialog.h : header file
//

#ifndef _CREATESPHEROIDDIALOG_H_
#define _CREATESPHEROIDDIALOG_H_

/////////////////////////////////////////////////////////////////////////////
// CCreateSpheroidDialog dialog
#include "thredprimitives.h"

class CThredBrush;


class CCreateSpheroidDialog : public CDialog
{
// Construction
public:
	void CreateSpheroid(CThredBrush* Brush);
	void DoDialog(CThredBrush* Brush);
	CCreateSpheroidDialog(CWnd* pParent = NULL);   // standard constructor
	~CCreateSpheroidDialog();

    int m_HalfSphere;

// Dialog Data
	//{{AFX_DATA(CCreateSpheroidDialog)
	enum { IDD = IDD_CREATE_SPHEROID };
	CStatic	m_Picture;
	int		m_HorizontalBands;
	int		m_VerticalBands;
	double	m_XSize;
	double	m_YSize;
	double	m_ZSize;
	int		m_Solid;
	double	m_Thickness;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateSpheroidDialog)
	public:
	virtual void Serialize(CArchive& ar);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreateSpheroidDialog)
	afx_msg void OnHollow();
	afx_msg void OnSolid();
	virtual BOOL OnInitDialog();
	afx_msg void OnDefaults();
	virtual void OnOK();
	afx_msg void OnHalfsphereradio();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	CBitmap mSolidSphere, mHollowSphere;
};

#endif
