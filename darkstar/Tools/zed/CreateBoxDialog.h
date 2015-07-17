// CreateBoxDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCreateBoxDialog double
#ifndef _CREATEBOXDIALOG_H_
#define _CREATEBOXDIALOG_H_

#include "thredprimitives.h"

class CThredBrush;

class CCreateBoxDialog : public CDialog
{
// Construction
public:
	void CreateBox(CThredBrush* Brush);
	void DoDialog(CThredBrush* Brush);
	CCreateBoxDialog(CWnd* pParent = NULL);   // standard constructor
	~CCreateBoxDialog();

// Dialog Data
	//{{AFX_DATA(CCreateBoxDialog)
	enum { IDD = IDD_CREATEBOX };
	CStatic	m_Picture;
	double	m_YSize;
	int		m_Solid;
	double	m_Thickness;
	double	m_XSizeBot;
	double	m_XSizeTop;
	double	m_ZSizeBot;
	double	m_ZSizeTop;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateBoxDialog)
	public:
	virtual void Serialize(CArchive& ar);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreateBoxDialog)
	afx_msg void OnSolid();
	afx_msg void OnHollow();
	virtual BOOL OnInitDialog();
	afx_msg void OnDefaults();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBitmap mHollowBitmap, mSolidBitmap;

};


#endif
