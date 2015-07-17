#if !defined(AFX_OBJECTVIEW_H__38977C52_719D_11D1_8CF7_006097B90B28__INCLUDED_)
#define AFX_OBJECTVIEW_H__38977C52_719D_11D1_8CF7_006097B90B28__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "threddoc.h"


/////////////////////////////////////////////////////////////////////////////
// the tree control

class CObjectViewTree : public CTreeCtrl
{
public:
	void CancelDrag( void );

	BOOL 			m_fDragging;
	CImageList * 	pDragList;
	HTREEITEM		m_hTopItem;
	HTREEITEM		m_hSelItem;
	HTREEITEM		m_hDropItem;

	CObjectViewTree();

	// Generated message map functions
protected:
	//{{AFX_MSG(CObjectViewTree)
	afx_msg void OnPaint();
    afx_msg void OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult);
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

// ObjectView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CObjectView window

class CObjectView : public CControlBar
{
public:
	enum ITEM_TYPES{
		STATE = 0, 
		DETAIL,
      DETAIL_OFF,
		LIGHT_SET,
		LIGHT_SET_OFF
	};
	void OnUpdateCmdUI( CFrameWnd * pFrame, BOOL fDisable );
   	CSize CalcDynamicLayout( int nLength, DWORD dwMode );
	CSize CurrentSize;
	int	CaptionSize;
	CObjectViewTree m_TreeWnd;
	CImageList m_ImageList;
	BOOL m_Enabled;

	void Enable( BOOL fState );
	BOOL AddState( CShapeState * pState );
	BOOL RemoveState( CShapeState * pState );
	BOOL AddDetail( CShapeState * pParent, CShapeDetail * pDetail );
	BOOL RemoveDetail( CShapeDetail * pDetail );
	BOOL AddLightSet( CShapeDetail * pParent, CLightSet * pLightSet );
	BOOL RemoveLightSet( CLightSet * pLightSet );
	BOOL RightClick( HTREEITEM hItem );
	BOOL UseDropTarget( HTREEITEM hTarget );
	BOOL MoveItem( HTREEITEM hSrc, HTREEITEM hDest );
	BOOL Rebuild( CTHREDDoc * pDoc );

   bool IsActive();
   void CopyCurrent();
   void CutCurrent();
   void DeleteCurrent();
   void PasteCurrent();
   
protected:
	
// Construction
public:
	CObjectView();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectView)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CObjectView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CObjectView)
	afx_msg void OnPaint();
	afx_msg void OnWindowPosChanged( WINDOWPOS* lpwndpos );
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ObjectView_H__38977C52_719D_11D1_8CF7_006097B90B28__INCLUDED_)
