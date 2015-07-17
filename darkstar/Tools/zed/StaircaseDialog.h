// StaircaseDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStaircaseDialog dialog

class CStaircaseDialog : public CDialog
{
// Construction
public:
	void MakeBrush(CThredBrush* Brush);
	void DoDialog(CThredBrush* Brush);
	CStaircaseDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CStaircaseDialog)
	enum { IDD = IDD_STAIRCASEDIALOG };
	int		m_NumberOfSteps;
	double	m_XSize;
	double	m_YSize;
	double	m_ZSize;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStaircaseDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStaircaseDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
