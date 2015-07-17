#if !defined(AFX_TEXTUREVIEW_H__38977C52_719D_11D1_8CF7_006097B90B28__INCLUDED_)
#define AFX_TEXTUREVIEW_H__38977C52_719D_11D1_8CF7_006097B90B28__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "threddoc.h"
#include "gfxwin.h"
#include "matcanvs.h"
#include "g_pal.h"


// TextureView.h : header file
//

class CTextureListBox : public CListBox
{
	public:

	protected:
		class CTextureView * m_TextureView;

	public:
		void MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct );		
		void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
		CTextureListBox( class CTextureView * TextureView );
      
   	GFXSurface*          pBuffer;

public:
	virtual ~CTextureListBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTextureListBox)
	afx_msg void OnDrawItem( int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct );
	afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
   afx_msg UINT OnGetDlgCode();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CTextureView window

class CTextureView : public CControlBar
{
public:
	void OnUpdateCmdUI( CFrameWnd * pFrame, BOOL fDisable );
   	CSize CalcDynamicLayout( int nLength, DWORD dwMode );
	void Attach( TSMaterialList * pList, GFXPalette * pPalette );
	void Detatch( void );
	CSize CurrentSize;
	int	CaptionSize;
	CTextureListBox m_ListWnd;
   
	int	            m_materialIndex;
	int	            m_materialCount;
	GFXPalette*       m_palette;
	TSMaterialList*   m_materialList;
	CMatCanvas*	      m_pFrame;
   int               m_ItemHeight;
   int               m_ItemWidth;

protected:
	
// Construction
public:
	CTextureView();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextureView)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTextureView();
      int getMaterialIndex();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTextureView)
	afx_msg void OnPaint();
	afx_msg void OnWindowPosChanged( WINDOWPOS* lpwndpos );
   afx_msg void OnDblClk(NMHDR * pNotify, LRESULT * result);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTUREVIEW_H__38977C52_719D_11D1_8CF7_006097B90B28__INCLUDED_)
