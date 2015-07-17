// palSelListBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// palSelListBox window

class palSelListBox : public CListBox
{
// Construction
public:
	palSelListBox();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(palSelListBox)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~palSelListBox();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
	// Generated message map functions
protected:
	//{{AFX_MSG(palSelListBox)
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg BOOL OnQueryNewPalette();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
