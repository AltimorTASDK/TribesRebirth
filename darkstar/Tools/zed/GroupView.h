#if !defined(AFX_GROUPVIEW_H__38977C52_719D_11D1_8CF7_006097B90B28__INCLUDED_)
#define AFX_GROUPVIEW_H__38977C52_719D_11D1_8CF7_006097B90B28__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "threddoc.h"


/////////////////////////////////////////////////////////////////////////////
// the tree control

class CGroupViewTree : public CTreeCtrl
{
public:
	void CancelDrag( void );

	BOOL 			m_fDragging;
	CImageList * 	pDragList;
	HTREEITEM		m_hTopItem;
	HTREEITEM		m_hSelItem;
	HTREEITEM		m_hDropItem;

	CGroupViewTree();

	// Generated message map functions
protected:
	//{{AFX_MSG(CGroupViewTree)
	afx_msg void OnPaint();
    afx_msg void OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBegindrag( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
	afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void OnCaptureChanged( CWnd* pWnd );
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

// GroupView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGroupView window

class CGroupView : public CControlBar
{
public:
	void OnUpdateCmdUI( CFrameWnd * pFrame, BOOL fDisable );
   	CSize CalcDynamicLayout( int nLength, DWORD dwMode );
	CSize CurrentSize;
	int	CaptionSize;
	CGroupViewTree m_TreeWnd;
	CImageList m_ImageList;

protected:
	
// Construction
public:
	CGroupView();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGroupView)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGroupView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CGroupView)
	afx_msg void OnPaint();
	afx_msg void OnWindowPosChanged( WINDOWPOS* lpwndpos );
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GROUPVIEW_H__38977C52_719D_11D1_8CF7_006097B90B28__INCLUDED_)
