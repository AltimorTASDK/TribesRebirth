// VDescDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVolDescriptionDlg dialog

#define TOTAL_STATES 33


class CVolDescriptionDlg : public CDialog
{
// Construction
public:
	CVolDescriptionDlg(CWnd* pParent = NULL);   // standard constructor

    CString m_description[TOTAL_STATES];
    int     m_currentvol;

// Dialog Data
	//{{AFX_DATA(CVolDescriptionDlg)
	enum { IDD = IDD_VOLDESCDIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVolDescriptionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

    void FillDescriptions();
    void ClearDescriptions();

	// Generated message map functions
	//{{AFX_MSG(CVolDescriptionDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeVoldescedit();
	afx_msg void OnSelchangeVolumelist1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
