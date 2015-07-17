// CreateTriDialog.h : header file
//

#include "thredprimitives.h"

class CThredBrush;

/////////////////////////////////////////////////////////////////////////////
// CCreateTriDialog dialog

class CCreateTriDialog : public CDialog
{
// Construction
public:
	CCreateTriDialog(CWnd* pParent = NULL);   // standard constructor
    ~CCreateTriDialog();

    void DoDialog(CThredBrush* Brush);
    void CreateTriangle(CThredBrush* Brush);

// Dialog Data
	//{{AFX_DATA(CCreateTriDialog)
	enum { IDD = IDD_CREATETRIANGLE };
	CStatic	m_Picture;
	int		m_Solid;
	double	m_Thickness;
	double	m_XSizeBot;
	double	m_YSize;
	double	m_ZSizeBot;
	double	m_ZSizeTop;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateTriDialog)
	public:
	virtual void Serialize(CArchive& ar);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreateTriDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnDefaults();
	afx_msg void OnTrihollow();
	afx_msg void OnTrisolid();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBitmap mHollowBitmap, mSolidBitmap;

    double  xtextureratio;
    double  ytextureratio;
    double  ztextureratio;

};
