// CreateStaircaseDialog.h : header file
//

#ifndef _CREATESTAIRCASEDIALOG_H_
#define _CREATESTAIRCASEDIALOG_H_

/////////////////////////////////////////////////////////////////////////////
// CCreateStaircaseDialog dialog

class CThredBrush;

class CCreateStaircaseDialog : public CDialog
{
// Construction
public:
	void MakeBrush(CThredBrush* Brush);
	void DoDialog(CThredBrush* Brush);
	CCreateStaircaseDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCreateStaircaseDialog)
	enum { IDD = IDD_STAIRCASEDIALOG };
	double	m_Height;
	double	m_Length;
	double	m_NumberOfStairs;
	double	m_Width;
	BOOL	m_MakeRamp;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateStaircaseDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreateStaircaseDialog)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
