// Splitter.h: interface for the CSplitter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPLITTER_H__58D70644_6C2D_11D1_8CF2_006097B90B28__INCLUDED_)
#define AFX_SPLITTER_H__58D70644_6C2D_11D1_8CF2_006097B90B28__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CSplitter : public CSplitterWnd  
{
	DECLARE_DYNCREATE(CSplitter)

public:
	CSplitter();
	virtual ~CSplitter();
	void SetSplitters( double SplitX, double SplitY );
	void UpdateSplitters( void );


	bool	Created;
	double	SplitX;
	double  SplitY;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSplitter)
	public:
	//}}AFX_VIRTUAL

//	afx_msg void OnLButtonUp( UINT, CPoint );
// Generated message map functions
protected:
	//{{AFX_MSG(CSplitter)
	afx_msg void OnKillFocus( CWnd * pWnd );
	afx_msg void OnLButtonDblClk( UINT flags, CPoint point );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_SPLITTER_H__58D70644_6C2D_11D1_8CF2_006097B90B28__INCLUDED_)
