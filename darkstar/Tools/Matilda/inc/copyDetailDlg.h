// copyDetailDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// copyDetailDlg dialog

class copyDetailDlg : public CDialog
{
// Construction
public:
	copyDetailDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(copyDetailDlg)
	enum { IDD = IDD_COPYDETAILDLG };
	UINT	m_from;
	UINT	m_to;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(copyDetailDlg)
	public:
	virtual int DoModal(int *, int *);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(copyDetailDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
