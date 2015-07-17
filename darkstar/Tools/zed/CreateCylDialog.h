// CreateCylDialog.h : header file
//

#ifndef _CREATECYLDIALOG_H_
#define _CREATECYLDIALOG_H_

/////////////////////////////////////////////////////////////////////////////
// CCreateCylDialog dialog
#include "thredprimitives.h"

class CThredBrush;


class CCreateCylDialog : public CDialog
{
// Construction
public:
	void CreateCyl(CThredBrush* Brush);
	void DoDialog(CThredBrush* Brush);
	CCreateCylDialog(CWnd* pParent = NULL);   // standard constructor

    int m_Ring;

// Dialog Data
	//{{AFX_DATA(CCreateCylDialog)
	enum { IDD = IDD_CREATE_CYL };
	double	m_BotXOffset;
	double	m_BotXSize;
	double	m_BotZOffset;
	double	m_BotZSize;
	int		m_Solid;
	double	m_Thickness;
	double	m_TopXOffset;
	double	m_TopXSize;
	double	m_TopZOffset;
	double	m_TopZSize;
	double	m_VerticalStripes;
	double	m_YSize;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateCylDialog)
	public:
	virtual void Serialize(CArchive& ar);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreateCylDialog)
	afx_msg void OnDefaults();
	virtual void OnOK();
	afx_msg void OnRing();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    double  xtextureratio;
    double  ytextureratio;
    double  ztextureratio;

};

#endif
