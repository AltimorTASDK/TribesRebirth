// RGBEditDlg.cpp : implementation file
//
#include <math.h>

#include "stdafx.h"
#include "RGBEditDlg.h"

// Darkstar #includes
//
#include "types.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// RGBEditDlg dialog


RGBEditDlg::RGBEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(RGBEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(RGBEditDlg)
	m_currentRed = 0;
	m_currentGreen = 0;
	m_currentBlue = 0;
	m_isTranslucent = FALSE;
	//}}AFX_DATA_INIT
}


void RGBEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(RGBEditDlg)
	DDX_Control(pDX, ID_RGB_CURRENTCOLOR, m_colorPreview);
	DDX_Control(pDX, ID_RGB_RADIOUNLIT, m_lightingUnlit);
	DDX_Control(pDX, ID_RGB_RADIOFLAT, m_lightingFlat);
	DDX_Control(pDX, ID_RGB_RADIOSMOOTH, m_lightingSmooth);
	DDX_Control(pDX, ID_RGB_BLUESLIDER, m_blueSlider);
	DDX_Control(pDX, ID_RGB_GREENSLIDER, m_greenSlider);
	DDX_Control(pDX, ID_RGB_REDVALUE, m_redEdit);
	DDX_Control(pDX, ID_RGB_GREENVALUE, m_greenEdit);
	DDX_Control(pDX, ID_RGB_BLUEVALUE, m_blueEdit);
	DDX_Control(pDX, ID_RGB_REDSLIDER, m_redSlider);
		CommonPropertyDataExchange(pDX, 1);
	DDX_Text(pDX, ID_RGB_REDVALUE, m_currentRed);
	DDV_MinMaxByte(pDX, m_currentRed, 0, 255);
	DDX_Text(pDX, ID_RGB_GREENVALUE, m_currentGreen);
	DDV_MinMaxByte(pDX, m_currentGreen, 0, 255);
	DDX_Text(pDX, ID_RGB_BLUEVALUE, m_currentBlue);
	DDV_MinMaxByte(pDX, m_currentBlue, 0, 255);
	DDX_Check(pDX, ID_RGB_TRANSLUCENT, m_isTranslucent);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(RGBEditDlg, CDialog)
	//{{AFX_MSG_MAP(RGBEditDlg)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(ID_RGB_BLUEVALUE, OnChangeRgbBluevalue)
	ON_EN_CHANGE(ID_RGB_GREENVALUE, OnChangeRgbGreenvalue)
	ON_EN_CHANGE(ID_RGB_REDVALUE, OnChangeRgbRedvalue)
		CommonPropertyMessageMap(1)
	ON_BN_CLICKED(ID_RGB_RADIOFLAT, OnRgbRadioflat)
	ON_BN_CLICKED(ID_RGB_RADIOSMOOTH, OnRgbRadiosmooth)
	ON_BN_CLICKED(ID_RGB_RADIOUNLIT, OnRgbRadiounlit)
	ON_BN_CLICKED(ID_RGB_TRANSLUCENT, OnRgbTranslucent)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// RGBEditDlg message handlers

int RGBEditDlg::DoModal(WORD *io_red, WORD *io_green, WORD *io_blue,
						TS::Material::ShadingType *io_lightingType)
{
	// Init member variables...
	//
	m_currentRed   = (UInt8)(*io_red);
	m_currentGreen = (UInt8)(*io_green);
	m_currentBlue  = (UInt8)(*io_blue);
	m_lightingType   = *io_lightingType;
	
	int returnVal = CDialog::DoModal();
	
	// Set return values...
	//
	*io_red			 = m_currentRed;
	*io_green		 = m_currentGreen;
	*io_blue		 = m_currentBlue;
	*io_lightingType = m_lightingType;

	return returnVal;
}


//   Once all the controls and such have been created, we need to
// set the default and current values for the sliders and the lighting
// radio buttons...
//
BOOL RGBEditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

   InitCommonProps();	

	m_redEdit.LimitText(3);
	m_greenEdit.LimitText(3);
	m_blueEdit.LimitText(3);

	m_redSlider.SetRange(0, 255);
	m_redSlider.SetPageSize(25);
	m_redSlider.SetPos(m_currentRed);

	m_greenSlider.SetRange(0, 255);
	m_greenSlider.SetPageSize(25);
	m_greenSlider.SetPos(m_currentGreen);
	
	m_blueSlider.SetRange(0, 255);
	m_blueSlider.SetPageSize(25);
	m_blueSlider.SetPos(m_currentBlue);

	switch(m_lightingType) {
	case TS::Material::ShadingNone:
			m_lightingUnlit.SetCheck(1);
			break;

	case TS::Material::ShadingFlat:
			m_lightingFlat.SetCheck(1);
			break;

	case TS::Material::ShadingSmooth:
			m_lightingSmooth.SetCheck(1);
			break;
	}
	updateColorPreview();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}




void RGBEditDlg::OnHScroll(UINT /*nSBCode*/, UINT /*nPos*/, CScrollBar* pScrollBar) 
{
     // TODO: Add your message handler code here and/or call default
	int scrollID = pScrollBar->GetDlgCtrlID();

	switch(scrollID) {
	case ID_RGB_REDSLIDER:
		m_currentRed = (BYTE)((CSliderCtrl*)pScrollBar)->GetPos();
		resetEditControls();
		break;
	
	case ID_RGB_GREENSLIDER:
		m_currentGreen = (BYTE)((CSliderCtrl*)pScrollBar)->GetPos();
		resetEditControls();
		break;

	case ID_RGB_BLUESLIDER:
		m_currentBlue = (BYTE)((CSliderCtrl*)pScrollBar)->GetPos();
		resetEditControls();
		break;
	
	default:
		break;
	}

	//CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


void
RGBEditDlg::resetEditControls()
{
	CString buffer;
	
	int bufferVal;

	m_redEdit.GetWindowText(buffer);
	sscanf(buffer,"%d",&bufferVal);
	if (bufferVal != m_currentRed) {
		buffer.Format("%3.3d", m_currentRed);
		m_redEdit.SetSel(0, -1, TRUE);
		m_redEdit.ReplaceSel(buffer);
	}

	m_greenEdit.GetWindowText(buffer);
	sscanf(buffer,"%d",&bufferVal);
	if (bufferVal != m_currentGreen) {
		buffer.Format("%3.3d", m_currentGreen);
		m_greenEdit.SetSel(0, -1, TRUE);
		m_greenEdit.ReplaceSel(buffer);
	}

	m_blueEdit.GetWindowText(buffer);
	sscanf(buffer,"%d",&bufferVal);
	if (bufferVal != m_currentBlue) {
		buffer.Format("%3.3d", m_currentBlue);
		m_blueEdit.SetSel(0, -1, TRUE);
		m_blueEdit.ReplaceSel(buffer);
	}

	updateColorPreview();
}

void
RGBEditDlg::resetColorSliders()
{
	if (m_redSlider.GetPos() != m_currentRed) {
		m_redSlider.SetPos(m_currentRed);
	}

	if (m_greenSlider.GetPos() != m_currentGreen) {
		m_greenSlider.SetPos(m_currentGreen);
	}
	
	if (m_blueSlider.GetPos() != m_currentBlue) {
		m_blueSlider.SetPos(m_currentBlue);
	}
	
	updateColorPreview();
}


void
RGBEditDlg::updateColorPreview()
{
	m_colorPreview.setPreviewColor(m_currentRed, m_currentGreen, m_currentBlue);
}


BYTE
RGBEditDlg::OnChangeByteEdit(CEdit &edit)
{
	CString buffer;
	int returnVal;

	edit.GetWindowText(buffer);
	if (buffer.GetLength() == 0) {
		// buffer is empty
		//
		return 0;
	} else {
		sscanf(buffer, "%u", &returnVal);
	}

	// MFC Validates the byte controls, this test is not necessary...
	//if (returnVal >= 256)

	return (BYTE)returnVal;
}


void
RGBEditDlg::OnChangeRgbRedvalue() 
{
	m_currentRed = OnChangeByteEdit(m_redEdit);
	resetColorSliders();
	//resetEditControls();
}

void RGBEditDlg::OnChangeRgbGreenvalue() 
{
	m_currentGreen = OnChangeByteEdit(m_greenEdit);
	resetColorSliders();
	//resetEditControls();
}

void
RGBEditDlg::OnChangeRgbBluevalue() 
{
	m_currentBlue = OnChangeByteEdit(m_blueEdit);
	resetColorSliders();
	//resetEditControls();
}

void RGBEditDlg::OnRgbRadiounlit() 
{
	m_lightingSmooth.SetCheck(0);
	m_lightingFlat.SetCheck(0);
	m_lightingType = TS::Material::ShadingNone;
}

void RGBEditDlg::OnRgbRadioflat() 
{
	m_lightingSmooth.SetCheck(0);
	m_lightingUnlit.SetCheck(0);
	m_lightingType = TS::Material::ShadingFlat;
}

void RGBEditDlg::OnRgbRadiosmooth() 
{

	m_lightingFlat.SetCheck(0);
	m_lightingUnlit.SetCheck(0);
	m_lightingType = TS::Material::ShadingSmooth;
}

void RGBEditDlg::OnRgbTranslucent() 
{
	// TODO: Add your control notification handler code here
	
}


CommonPropertyMethodsDefined (RGBEditDlg)
