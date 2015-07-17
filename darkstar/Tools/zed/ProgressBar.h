// ProgressBar.h : header file
//

#ifndef _INCLUDE_PROGRESSBAR_H_
#define _INCLUDE_PROGRESSBAR_H_


/////////////////////////////////////////////////////////////////////////////
// CProgressBar -  status bar progress control
//

class CProgressBar: public CProgressCtrl
// Creates a ProgressBar in the status bar
{
public:
   CProgressBar();
	CProgressBar( int MaxValue, BOOL bSmooth, int Pane );
   ~CProgressBar();
	BOOL Create( int MaxValue = 100, BOOL bSmooth = FALSE, int Pane = 0 );

    DECLARE_DYNCREATE(CProgressBar)

// operations
public:
    BOOL Success() {return m_bSuccess;}			// Was the creation successful?

    void SetRange(int nLower, int nUpper, int nStep = 1);
    void SetSize(int nSize);
    int  SetPos(int nPos);
    int  OffsetPos(int nPos);
    int  SetStep(int nStep);
    int  StepIt();
    void Clear();

// Overrides
    //{{AFX_VIRTUAL(CProgressBar)
    //}}AFX_VIRTUAL

// implementation
protected:
    BOOL		   m_bSuccess;		// Successfully created?
	 int			m_pane;		// pane to belong to 
 	 CRect		m_Rect;			// Dimensions of the whole thing
	 int			m_range;		// range allowed to alter for current stage

    CStatusBar *GetStatusBar();
    void Resize();

// Generated message map functions
protected:
    //{{AFX_MSG(CProgressBar)
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

};

#endif
/////////////////////////////////////////////////////////////////////////////

