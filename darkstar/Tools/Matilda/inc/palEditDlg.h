// palEditDlg.h : header file
//

#include "types.h"
#include "ts_material.h"

#include "palSelListBox.h"


/////////////////////////////////////////////////////////////////////////////
// palEditDlg dialog

class palEditDlg : public CDialog
{
// Construction
public:
	palEditDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(palEditDlg)
	enum { IDD = IDD_PALEDIT_DIALOG };
	CButton	m_isTranslucent;
	palSelListBox	m_palListBox;
		CommonPropertyDataMembers;
	CButton	m_lightingUnlit;
	CButton	m_lightingSmooth;
	CButton	m_lightingFlat;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(palEditDlg)
	public:
		virtual int DoModal(UInt32 *, TS::Material::ShadingType *);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	Int32	m_selectedColor;
	TS::Material::ShadingType m_lightingType;

	// Generated message map functions
	//{{AFX_MSG(palEditDlg)
	afx_msg void OnPalLightingflat();
	afx_msg void OnPalLightingsmooth();
	afx_msg void OnPalLightingunlit();
	virtual BOOL OnInitDialog();
		CommonPropertyMethodsDeclared();
	afx_msg void OnDblclkPalList();
	virtual void OnOK();
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg BOOL OnQueryNewPalette();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
