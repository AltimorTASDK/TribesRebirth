#if !defined(AFX_DETAILSETTINGS_H__20EFAB95_85F5_11D1_8D04_006097B90B28__INCLUDED_)
#define AFX_DETAILSETTINGS_H__20EFAB95_85F5_11D1_8D04_006097B90B28__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DetailSettings.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDetailSettings dialog

class CDetailSettings : public CDialog
{
// Construction
public:
	CDetailSettings(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDetailSettings)
	enum { IDD = IDD_DETAILSETTINGS };
	BOOL	m_LinkBack;
	BOOL	m_LinkBottom;
	BOOL	m_LinkFront;
	BOOL	m_LinkLeft;
	BOOL	m_LinkRight;
	BOOL	m_LinkTop;
	UINT	m_maximumTexMip;
	UINT	m_minPixels;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDetailSettings)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDetailSettings)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DETAILSETTINGS_H__20EFAB95_85F5_11D1_8D04_006097B90B28__INCLUDED_)
