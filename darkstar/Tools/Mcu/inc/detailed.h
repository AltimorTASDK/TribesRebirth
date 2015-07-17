// detailed.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDetailEditView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif
#include "mcudoc.h"

class CDetailEditView : public CFormView
{
protected:
	CDetailEditView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CDetailEditView)

// Form Data
public:
	//{{AFX_DATA(CDetailEditView)
	enum { IDD = IDD_DETLIST };
	CEdit	m_MergePoint;
	CEdit	m_MergeNormal;
	CButton	m_bsp;
	CEdit	m_alphaColors;
	CButton	m_build;
	CButton	m_sort;
	CEdit	m_alphaLevels;
	CButton	m_insert;
	CButton	m_delete;
	CEdit	m_status;
	CEdit	m_shrink;
	CEdit	m_scale;
	CListBox	m_list;
	CEdit	m_size;
	//}}AFX_DATA

// Attributes
public:
	CMcuDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDetailEditView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	DetailList *pDL;
	void UpdateList(int index);
	void UpdateListItem(int index);
	void UpdateEditControls(int index);
	void OnUpdateView(int index);
	char * FormatDetail( int selIndex, char *s );
   void AddStatusText(char *s);
   void ClearStatus();
protected:
	virtual ~CDetailEditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CDetailEditView)
	afx_msg void OnBuild();
	afx_msg void OnDelete();
	afx_msg void OnInsert();
	afx_msg void OnSort();
	afx_msg void OnChangeScale();
	afx_msg void OnChangeDetSize();
	afx_msg void OnChangeShrink();
	afx_msg void OnSelchangeList();
	afx_msg void OnChangeAlphaColors();
	afx_msg void OnChangeAlphaLevels();
	afx_msg void OnBsp();
	afx_msg void OnChangeMergePoint();
	afx_msg void OnChangeMergeNormal();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in detailed.cpp
inline CMcuDoc* CDetailEditView::GetDocument()
   { return (CMcuDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
