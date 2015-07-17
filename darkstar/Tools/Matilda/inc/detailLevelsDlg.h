// detailLevelsDlg.h : header file
//
#ifndef _DETAILLEVELSDLG_H_
#define _DETAILLEVELSDLG_H_

/////////////////////////////////////////////////////////////////////////////
// detailLevelsDlg dialog

class detailLevelsDlg : public CDialog
{
// Construction
public:
	detailLevelsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(detailLevelsDlg)
	enum { IDD = IDD_DETAIL_LEVELS };
	UINT	m_numLevels;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(detailLevelsDlg)
	public:
	virtual int DoModal(int *out_levels);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(detailLevelsDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // Header
