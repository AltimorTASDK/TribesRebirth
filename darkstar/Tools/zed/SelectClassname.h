// SelectClassname.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectClassname dialog

class CSelectClassname : public CDialog
{
// Construction
public:
	char** ClassnameString;
	char** GetClassname();
	//static char ClassnameList[][20][100];
	static char **CSelectClassname::ClassnameList[];

	CSelectClassname(CWnd* pParent = NULL);   // standard constructor


// Dialog Data
	//{{AFX_DATA(CSelectClassname)
	enum { IDD = IDD_SELECTCLASSNAME };
	CListBox	m_ListBox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectClassname)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectClassname)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeList1();
	afx_msg void OnDblclkList1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

