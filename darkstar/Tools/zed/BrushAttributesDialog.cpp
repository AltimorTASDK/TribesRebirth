// BrushAttributesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "THRED.h"
#include "BrushAttributesDialog.h"
#include "g_bitmap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBrushAttributesDialog dialog


CBrushAttributesDialog::CBrushAttributesDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CBrushAttributesDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBrushAttributesDialog)
	m_Position_X = 0.0;
	m_Position_Y = 0.0;
	m_Position_Z = 0.0;
	m_Rotation_Yaw = 0.0;
	m_Rotation_Pitch = 0.0;
	m_Rotation_Roll = 0.0;
	m_Scale_X = 0.0;
	m_Scale_Y = 0.0;
	m_Scale_Z = 0.0;
	m_ZShear = 0.0;
	m_XShear = 0.0;
	m_YShear = 0.0;
	m_Name = _T("");
	m_BBoxX = 0.0;
	m_BBoxY = 0.0;
	m_BBoxZ = 0.0;
	//}}AFX_DATA_INIT

    for (int i = 0; i < 32; i++)
    {
      m_VolumeDescriptions[i].Format("%d", i);
    }

    material_index1 = 0;
	material_count = 0;
	m_pFrame1 = NULL;
}

CBrushAttributesDialog::~CBrushAttributesDialog()
{
  if (m_pFrame1)
  {
    delete m_pFrame1;
  }
}

void CBrushAttributesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBrushAttributesDialog)
	DDX_Control(pDX, IDC_BRUSH_MAT_SPIN, m_spinControl);
	DDX_Text(pDX, IDC_BBX, m_BBoxX);
	DDX_Text(pDX, IDC_BBY, m_BBoxY);
	DDX_Text(pDX, IDC_BBZ, m_BBoxZ);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBrushAttributesDialog, CDialog)
	//{{AFX_MSG_MAP(CBrushAttributesDialog)
	ON_WM_PAINT()
	ON_NOTIFY(UDN_DELTAPOS, IDC_BRUSH_MAT_SPIN, OnDeltaposBrushMatSpin)
	ON_EN_CHANGE(IDC_BRUSH_MAT_INDEX, OnChangeBrushMatIndex)
	ON_BN_CLICKED(IDC_ADDRADIO, OnAddradio)
	ON_BN_CLICKED(IDC_SUBTRACTRADIO, OnSubtractradio)
	ON_BN_CLICKED(IDC_VOLRADIO, OnVolradio)
	ON_CBN_SELCHANGE(IDC_VOLSELECTION, OnSelchangeVolselection)
	ON_BN_CLICKED(IDC_DEFAULTBUTTON, OnDefaultbutton)
	ON_LBN_SELCHANGE(IDC_VOLSELECTION, OnSelchangeVolselection)
	ON_BN_CLICKED(IDC_SHOWVOLRADIO, OnShowvolradio)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrushAttributesDialog message handlers

void CBrushAttributesDialog::OnPaint() 
{
	if (!m_pFrame1)
		return;

	// Set the palette for the frames
	m_pFrame1->m_pSurface->setPalette(palette, false);

	// Redraw the textures
	if (material_list)
	{
		material_list->setDetailLevel(0);
		m_pFrame1->m_pMaterial = &material_list->getMaterial(material_index1);
		m_pFrame1->Refresh();
	}
	
	// Do not call CDialog::OnPaint() for painting messages
}

void CBrushAttributesDialog::OnDeltaposBrushMatSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	int newval = pNMUpDown->iPos + pNMUpDown->iDelta;

	if ((newval >= 0) && (newval < material_count))
	{
		char str_val[10];
		sprintf(str_val, "%d", newval);
		((CEdit*)GetDlgItem(IDC_BRUSH_MAT_INDEX))->SetSel(0, -1, TRUE);

		((CEdit*)GetDlgItem(IDC_BRUSH_MAT_INDEX))->ReplaceSel((LPCTSTR)str_val, FALSE);

		material_index1 = newval;

		OnPaint();
	}

	*pResult = 0;
}

void CBrushAttributesDialog::OnChangeBrushMatIndex() 
{
	char	str_val[10];
	int		tmp;

	((CEdit*)GetDlgItem(IDC_BRUSH_MAT_INDEX))->GetLine(0, (LPTSTR)str_val);

	tmp = atoi(str_val);

	if ((tmp >= 0) && (tmp < material_count))
	{
		material_index1 = tmp;

		OnPaint();

        if (m_pFrame1)
        {
          char  buf[32];

          if ((m_pFrame1->m_pMaterial->fParams.fFlags & TSMaterial::MatFlags) == TSMaterial::MatTexture)
          {
            const GFXBitmap*  bmp = m_pFrame1->m_pMaterial->getTextureMap();
            sprintf(buf, "(%d x %d)", bmp->width, bmp->height);
	        ((CEdit*)GetDlgItem(IDC_BRUSH_TSIZEREADOUT))->SetSel(0, -1, FALSE);
            ((CEdit*)GetDlgItem(IDC_BRUSH_TSIZEREADOUT))->ReplaceSel(buf);
          } else {
            sprintf(buf, "Null Texture");
	        ((CEdit*)GetDlgItem(IDC_BRUSH_TSIZEREADOUT))->SetSel(0, -1, FALSE);
            ((CEdit*)GetDlgItem(IDC_BRUSH_TSIZEREADOUT))->ReplaceSel(buf);
          }
        }
	}
}

BOOL CBrushAttributesDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CWnd *temp = GetDlgItem(IDC_BRUSH_MAT_CANVAS);
	RECT rect;
	temp->GetClientRect(&rect);
	m_pFrame1 = new CMatCanvas;
	m_pFrame1->Create(temp,rect, IDC_FRAME4);

	if (material_list)
	{
		material_list->setDetailLevel(0);
		m_pFrame1->m_pMaterial = &material_list->getMaterial(material_index1);
	}

	// Set up the spin controls
	((CSpinButtonCtrl*)GetDlgItem(IDC_BRUSH_MAT_SPIN))->SetRange(0, material_count-1);
	((CSpinButtonCtrl*)GetDlgItem(IDC_BRUSH_MAT_SPIN))->SetPos(atoi(m_Name)/*material_index1*/);

    ((CButton*)GetDlgItem(IDC_ADDRADIO))->SetCheck(m_Additive);
    ((CButton*)GetDlgItem(IDC_SUBTRACTRADIO))->SetCheck(m_Subtractive);
    ((CButton*)GetDlgItem(IDC_VOLRADIO))->SetCheck(m_Volume);

    ((CButton*)GetDlgItem(IDC_SHOWVOLRADIO))->SetCheck(m_ShowVolPolys);
    GetDlgItem(IDC_SHOWVOLRADIO)->EnableWindow(m_Volume);

    if (m_pFrame1)
    {
      char  buf[32];
      if ((m_pFrame1->m_pMaterial->fParams.fFlags & TSMaterial::MatFlags) == TSMaterial::MatTexture)
      {
        const GFXBitmap*  bmp = m_pFrame1->m_pMaterial->getTextureMap();
        sprintf(buf, "(%d x %d)", bmp->width, bmp->height);
	    ((CEdit*)GetDlgItem(IDC_BRUSH_TSIZEREADOUT))->SetSel(0, -1, FALSE);
        ((CEdit*)GetDlgItem(IDC_BRUSH_TSIZEREADOUT))->ReplaceSel(buf);
      } else {
        sprintf(buf, "Null Texture");
	    ((CEdit*)GetDlgItem(IDC_BRUSH_TSIZEREADOUT))->SetSel(0, -1, FALSE);
        ((CEdit*)GetDlgItem(IDC_BRUSH_TSIZEREADOUT))->ReplaceSel(buf);
      }
    }

    unsigned int compbit = 0;

    for (int i = 0; i < TOTAL_VOLUME_STATES; i++)
    {
      ((CListBox*)GetDlgItem(IDC_VOLSELECTION))->AddString((LPCTSTR)m_VolumeDescriptions[i]);

      if (m_VolumeState & compbit)
      {
        ((CListBox*)GetDlgItem(IDC_VOLSELECTION))->SetSel(i);
      } else {
        ((CListBox*)GetDlgItem(IDC_VOLSELECTION))->SetSel(i, FALSE);
      }

      if (i > 0)
      {
        compbit = compbit << 1;
      } else {
        compbit = 1;
      }
    }

    if (!m_VolumeState)
    {
      ((CListBox*)GetDlgItem(IDC_VOLSELECTION))->SetSel(0);
    }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBrushAttributesDialog::OnAddradio() 
{
  m_Additive = TRUE;
  m_Subtractive = FALSE;
  m_Volume = FALSE;

  GetDlgItem(IDC_SHOWVOLRADIO)->EnableWindow(FALSE);

  GetDlgItem(IDC_VOLSELECTION)->SendMessage(WM_PAINT, NULL, 0);
}

void CBrushAttributesDialog::OnSubtractradio() 
{
  m_Additive = FALSE;
  m_Subtractive = TRUE;
  m_Volume = FALSE;

  GetDlgItem(IDC_SHOWVOLRADIO)->EnableWindow(FALSE);

  GetDlgItem(IDC_VOLSELECTION)->SendMessage(WM_PAINT, NULL, 0);
}

void CBrushAttributesDialog::OnVolradio() 
{
  m_Additive = FALSE;
  m_Subtractive = FALSE;
  m_Volume = TRUE;

  GetDlgItem(IDC_SHOWVOLRADIO)->EnableWindow(TRUE);

  GetDlgItem(IDC_VOLSELECTION)->SendMessage(WM_PAINT, NULL, 0);
}

void CBrushAttributesDialog::OnShowvolradio() 
{
  if (m_ShowVolPolys)
  {
    // Turn off the button
    m_ShowVolPolys = FALSE;
  } else {
    m_ShowVolPolys = TRUE;
  }

  ((CButton*)GetDlgItem(IDC_SHOWVOLRADIO))->SetCheck(m_ShowVolPolys);	
}

BOOL CBrushAttributesDialog::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.lpszClass = AfxRegisterWndClass(CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT,
										::LoadCursor(NULL,IDC_ARROW));
    
	return CDialog::PreCreateWindow(cs);
}

void CBrushAttributesDialog::OnSelchangeVolselection() 
{
    int selection = ((CListBox*)GetDlgItem(IDC_VOLSELECTION))->GetCurSel();
    int numselected;
    int selectarray[TOTAL_VOLUME_STATES];

    if (selection)
    {
      ((CListBox*)GetDlgItem(IDC_VOLSELECTION))->SetSel(0, FALSE);
    } else {
      for (int i = 1; i < TOTAL_VOLUME_STATES; i++)
      {
        ((CListBox*)GetDlgItem(IDC_VOLSELECTION))->SetSel(i, FALSE);
      }
    }

    numselected = ((CListBox*)GetDlgItem(IDC_VOLSELECTION))->GetSelItems(TOTAL_VOLUME_STATES, selectarray);
    m_VolumeState = 0;

    for (int i = 0; i < numselected; i++)
    {
      if (selectarray[i])
      {
        // non-item 0 was selected...
        m_Additive = FALSE;
        m_Subtractive = FALSE;
        m_Volume = TRUE;
        ((CButton*)GetDlgItem(IDC_ADDRADIO))->SetCheck(0);
        ((CButton*)GetDlgItem(IDC_SUBTRACTRADIO))->SetCheck(0);
        ((CButton*)GetDlgItem(IDC_VOLRADIO))->SetCheck(1);

        m_VolumeState += (unsigned int)pow(2.0, (double)(selectarray[i]-1.0));
      } else {
        // item 0 was selected...
        m_Additive = TRUE;
        m_Subtractive = FALSE;
        m_Volume = FALSE;
        ((CButton*)GetDlgItem(IDC_ADDRADIO))->SetCheck(1);
        ((CButton*)GetDlgItem(IDC_SUBTRACTRADIO))->SetCheck(0);
        ((CButton*)GetDlgItem(IDC_VOLRADIO))->SetCheck(0);

        m_VolumeState = 0;
      }
    }
//#if 0
    if (m_VolumeState)
    {
      GetDlgItem(IDC_SHOWVOLRADIO)->EnableWindow(TRUE);
    } else {
      GetDlgItem(IDC_SHOWVOLRADIO)->EnableWindow(FALSE);
    }
//#endif
    GetDlgItem(IDC_VOLSELECTION)->SendMessage(WM_PAINT, NULL, 0);
}

void CBrushAttributesDialog::OnDefaultbutton() 
{
	m_VolumeState = 0;
    m_Additive = TRUE;
    m_Subtractive = FALSE;
    m_Volume = FALSE;
    ((CButton*)GetDlgItem(IDC_ADDRADIO))->SetCheck(1);
    ((CButton*)GetDlgItem(IDC_SUBTRACTRADIO))->SetCheck(0);
    ((CButton*)GetDlgItem(IDC_VOLRADIO))->SetCheck(0);
	
    for (int i = 1; i < TOTAL_VOLUME_STATES; i++)
    {
      ((CListBox*)GetDlgItem(IDC_VOLSELECTION))->SetSel(i, FALSE);
    }

    ((CListBox*)GetDlgItem(IDC_VOLSELECTION))->SetSel(0, TRUE);

    GetDlgItem(IDC_VOLSELECTION)->SendMessage(WM_PAINT, NULL, 0);
}

