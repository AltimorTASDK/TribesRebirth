// TSDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// TScaleDialog dialog

class TScaleDialog : public CDialog
{
// Construction
public:
	TScaleDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(TScaleDialog)
	enum { IDD = IDD_TSDIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TScaleDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

protected:

	// Generated message map functions
	//{{AFX_MSG(TScaleDialog)
	afx_msg void OnChangeTexturescale();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


public:
    double   tscale;

};
