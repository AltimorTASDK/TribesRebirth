// RGBEditDlg.h : header file
//
#ifndef _RGBEDITDLG_H_
#define _RGBEDITDLG_H_

/////////////////////////////////////////////////////////////////////////////
// RGBEditDlg dialog

// Darkstar include
//
#include "types.h"
#include "ts_material.h"

#include "colorPreviewFrame.h"
#include "resource.h"
#include "commonProps.h"


#define LENGTH_OF_SATSLIDER     (1000)
#define LENGTH_OF_HUESLIDER     (3600)
#define LENGTH_OF_BRIGHTSLIDER  (1000)


class RGBEditDlg : public CDialog
{
// Construction
public:
	RGBEditDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(RGBEditDlg)
	enum { IDD = IDD_RGBEDIT_DIALOG };
	colorPreviewFrame	m_colorPreview;
	CButton	m_lightingUnlit;
	CButton	m_lightingFlat;
	CButton	m_lightingSmooth;
	CSliderCtrl	m_blueSlider;
	CSliderCtrl	m_greenSlider;
	CEdit	m_redEdit;
	CEdit	m_greenEdit;
	CEdit	m_blueEdit;
		CommonPropertyDataMembers;
	CSliderCtrl	m_redSlider;
	BYTE	m_currentRed;
	BYTE	m_currentGreen;
	BYTE	m_currentBlue;
	BOOL	m_isTranslucent;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(RGBEditDlg)
	public:
		virtual int DoModal(WORD *r, WORD *g, WORD *b, TS::Material::ShadingType *);
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void resetEditControls();
	void resetColorSliders();
	void updateColorPreview();

	BYTE OnChangeByteEdit(CEdit &edit);

	TS::Material::ShadingType m_lightingType;


	// Generated message map functions
	//{{AFX_MSG(RGBEditDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnChangeRgbBluevalue();
	afx_msg void OnChangeRgbGreenvalue();
	afx_msg void OnChangeRgbRedvalue();
		CommonPropertyMethodsDeclared();
	afx_msg void OnRgbRadioflat();
	afx_msg void OnRgbRadiosmooth();
	afx_msg void OnRgbRadiounlit();
	afx_msg void OnRgbTranslucent();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
