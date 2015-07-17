// BrushAttributesDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBrushAttributesDialog dialog

#include "matcanvs.h"
#include "g_pal.h"

#define TOTAL_VOLUME_STATES 33


class CBrushAttributesDialog : public CDialog
{
// Construction
public:
	CBrushAttributesDialog(CWnd* pParent = NULL);   // standard constructor
	~CBrushAttributesDialog();

// Dialog Data
	//{{AFX_DATA(CBrushAttributesDialog)
	enum { IDD = IDD_BRUSHDIALOG };
	CSpinButtonCtrl	m_spinControl;
	double	m_Position_X;
	double	m_Position_Y;
	double	m_Position_Z;
	double	m_Rotation_Yaw;
	double	m_Rotation_Pitch;
	double	m_Rotation_Roll;
	double	m_Scale_X;
	double	m_Scale_Y;
	double	m_Scale_Z;
	double	m_ZShear;
	double	m_XShear;
	double	m_YShear;
    CString	m_Name;
	double	m_BBoxX;
	double	m_BBoxY;
	double	m_BBoxZ;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBrushAttributesDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

public:
	int	material_index1;

	int	material_count;
	GFXPalette* palette;
	TSMaterialList* material_list;

    BOOL    m_Subtractive;
    BOOL    m_Additive;
    BOOL    m_Volume;
    BOOL    m_ShowVolPolys;
    UInt32  m_VolumeState;

    CString m_VolumeDescriptions[TOTAL_VOLUME_STATES];

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBrushAttributesDialog)
	afx_msg void OnPaint();
	afx_msg void OnDeltaposBrushMatSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeBrushMatIndex();
	virtual BOOL OnInitDialog();
	afx_msg void OnAddradio();
	afx_msg void OnSubtractradio();
	afx_msg void OnVolradio();
	afx_msg void OnSelchangeVolselection();
	afx_msg void OnDefaultbutton();
	afx_msg void OnShowvolradio();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

      // DPW - needed to display textures
	CMatCanvas*	m_pFrame1;
};
