// MatVuDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// MaterialView dialog

#include "matcanvs.h"
#include "g_pal.h"


class MaterialView : public CDialog
{
// Construction
public:
	MaterialView(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(MaterialView)
	enum { IDD = IDD_MATERIALVIEW };
	CSpinButtonCtrl	m_spinControl2;
	CSpinButtonCtrl	m_spinControl1;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MaterialView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

public:
	int	material_index1;
	int	material_index2;

	int	material_count;
	GFXPalette* palette;
	TSMaterialList* material_list;
    int   stretchit;

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(MaterialView)
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnChangeEditindex1();
	afx_msg void OnChangeEditindex2();
	afx_msg void OnDeltaposSpinmat1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinmat2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnStretchcheck1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// DPW - needed to display textures
	CMatCanvas*	m_pFrame1;
	CMatCanvas*	m_pFrame2;
};
