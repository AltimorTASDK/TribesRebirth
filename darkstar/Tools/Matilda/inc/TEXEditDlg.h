// TEXEditDlg.h : header file
//

#include "types.h"
#include "ts_material.h"

#include "gfxPreviewWind.h"

#include  "commonProps.h"

/////////////////////////////////////////////////////////////////////////////
// TEXEditDlg dialog

class TEXEditDlg : public CDialog
{
// Construction
public:
	TEXEditDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(TEXEditDlg)
	enum { IDD = IDD_TEXTURE_DIALOG };
	CButton	m_perspective;
	CButton	m_transparent;
	CButton	m_translucent;
	CButton	m_rotate;
	CButton	m_hazed;
	CButton	m_flipY;
	CButton	m_flipX;
	CommonPropertyDataMembers;
	CEdit	m_fileNameEdit;
	CButton	m_lightingUnlit;
	CButton	m_lightingFlat;
	CButton	m_lightingSmooth;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TEXEditDlg)
	public:
		virtual int DoModal(CString &io_fileName, 
				TS::Material::ShadingType *io_lightingType, 
				UInt32 *io_flags );
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CString						m_fileName;
	TS::Material::ShadingType	m_lightingType;
	UInt32						m_texType;
	TS::Material				m_tempMaterial;
	bool						m_previewIsValid;

	gfxPreviewWind				m_preview;

	// Generated message map functions
	//{{AFX_MSG(TEXEditDlg)
	afx_msg void OnTexRadioflat();
	afx_msg void OnTexRadiosmooth();
	afx_msg void OnTexRadiounlit();
	virtual BOOL OnInitDialog();
	afx_msg void OnTexSelectfile();
	afx_msg void OnTexFlipx();
	afx_msg void OnTexFlipy();
	CommonPropertyMethodsDeclared ();
	afx_msg void OnTexPerspective();
	afx_msg void OnTexRotate();
	afx_msg void OnTexTranslucent();
	afx_msg void OnTexTransparent();
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg BOOL OnQueryNewPalette();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
