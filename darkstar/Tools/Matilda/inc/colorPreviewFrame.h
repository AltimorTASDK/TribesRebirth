// colorPreviewFrame.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// colorPreviewFrame window

class colorPreviewFrame : public CStatic
{
// Construction
public:
	colorPreviewFrame();

// Attributes
public:

// Operations
public:

// Overrides
	//{{AFX_VIRTUAL(colorPreviewFrame)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~colorPreviewFrame();
	void setPreviewColor(BYTE red, BYTE green, BYTE blue);

	COLORREF	m_color;

	// Generated message map functions
protected:
	//{{AFX_MSG(colorPreviewFrame)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
