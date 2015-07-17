#if !defined(AFX_LIGHTVIEW_H__38977C52_719D_11D1_8CF7_006097B90B28__INCLUDED_)
#define AFX_LIGHTVIEW_H__38977C52_719D_11D1_8CF7_006097B90B28__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "threddoc.h"
#include "lightblankprop.h"


class CLightView;

/////////////////////////////////////////////////////////////////////////////
// the tree control

class CLightViewTree : public CTreeCtrl
{
public:

	void CancelDrag( void );

	BOOL 			   m_fDragging;
	CImageList * 	pDragList;
	HTREEITEM		m_hTopItem;
	HTREEITEM		m_hSelItem;
	HTREEITEM		m_hDropItem;

   BOOL Rebuild( CItemBase * root );
   void verifyInspect();
   CItemBase blank;
	CLightViewTree();

	// Generated message map functions
protected:
	//{{AFX_MSG(CLightViewTree)
   afx_msg void OnPaint();
   afx_msg void OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult );
   afx_msg void OnBegindrag( NMHDR* pNMHDR, LRESULT* pResult );
   afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
   afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
   afx_msg void OnMouseMove( UINT nFlags, CPoint point );
   afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
   afx_msg void OnCaptureChanged( CWnd* pWnd );
   afx_msg void OnLButtonDblClk( UINT nFlags, CPoint point );
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

// LightView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLightView window

class CLightView : public CControlBar
{
public:

	void OnUpdateCmdUI( CFrameWnd * pFrame, BOOL fDisable );
 	CSize CalcDynamicLayout( int nLength, DWORD dwMode );
	CSize CurrentSize;
	int	CaptionSize;
	CLightViewTree m_TreeWnd;
	CImageList m_ImageList;
	BOOL m_Enabled;

	void Enable( BOOL fState );
	BOOL RightClick( HTREEITEM hItem );
	BOOL UseDropTarget( HTREEITEM hTarget );
	BOOL Rebuild( CTHREDDoc * pDoc, CLightArray & lightArray );

   // edit fuctions
   void copyCurrent();
   void pasteCurrent();
   void cutCurrent();
   void removeCurrent();

   bool IsOpen();
   bool IsActive();
   
protected:
	
// Construction
public:
	CLightView();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLightView)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CLightView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CLightView)
	afx_msg void OnPaint();
	afx_msg void OnWindowPosChanged( WINDOWPOS* lpwndpos );
   afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LightView_H__38977C52_719D_11D1_8CF7_006097B90B28__INCLUDED_)
