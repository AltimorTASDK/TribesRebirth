// matilda2Dlg.h : header file
//
#ifndef _MATILDA2DLG_H_
#define _MATILDA2DLG_H_

#include "ts_material.h"
//#include "ts_bmarr.h"
#include "ts_types.h"
//#include "ts_txman.h"

#include "RGBEditDlg.h"
#include "palEditDlg.h"
#include "TEXEditDlg.h"
#include "gfxPreviewWind.h"
#include "copyDetailDlg.h"

#include "g_pal.h"

/////////////////////////////////////////////////////////////////////////////
// CMatilda2Dlg dialog

class CMatilda2Dlg : public CDialog
{
// Construction
public:
	CMatilda2Dlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CMatilda2Dlg();

	gfxPreviewWind  m_preview;

	void UpdateList();
	void UpdateListMember(int in_index);

	BOOL loadPalette(const char *in_palName);
	void OnPaletteSetcurrentpalette();

	void	OnFileExit();
	void	OnFileNew();
	int		OnFileSave();
	int		OnFileSaveas();
	void	OnFileOpen();
	void	OpenTheFile(const char*);

	void OnEditCopy();
	void OnEditPaste(); 
	void OnEditPastetoend(); 
	void OnChangelightingflags(); 

// Dialog Data
	//{{AFX_DATA(CMatilda2Dlg)
	enum { IDD = IDD_MATILDA2_DIALOG };
	CSliderCtrl	m_detailSlider;
	CEdit	m_detailEdit;
	CListBox	m_matList;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMatilda2Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	int m_currDetailLevel;

	TS::Material **m_copyBuffer;
	int			 m_numCopyItems;

	// Generated message map functions
	//{{AFX_MSG(CMatilda2Dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnEditasRgb();
	afx_msg void OnEditasPalette();
	afx_msg void OnEditasTexture();
	afx_msg void OnDblclkMaterialList();
	afx_msg void OnSelchangeMaterialList();
	afx_msg void OnEditasNull();
	afx_msg void OnAppendNew();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnDeleteSelected();
	afx_msg void OnCopyDetaillevel();
	afx_msg void OnCheck1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


#endif
