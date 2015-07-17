// MatVuDlg.cpp : implementation file
//
#include "stdafx.h"
#include "thred.h"
#include "MatVuDlg.h"
#include "g_bitmap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MaterialView dialog


MaterialView::MaterialView(CWnd* pParent /*=NULL*/)
	: CDialog(MaterialView::IDD, pParent)
{
	//{{AFX_DATA_INIT(MaterialView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	material_index1 = material_index2 = 0;
	material_count = 0;
	m_pFrame1 = NULL;
	m_pFrame2 = NULL;
    stretchit = 0;
}


void MaterialView::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MaterialView)
	DDX_Control(pDX, IDC_SPINMAT2, m_spinControl2);
	DDX_Control(pDX, IDC_SPINMAT1, m_spinControl1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(MaterialView, CDialog)
	//{{AFX_MSG_MAP(MaterialView)
	ON_WM_PAINT()
	ON_EN_CHANGE(IDC_EDITINDEX1, OnChangeEditindex1)
	ON_EN_CHANGE(IDC_EDITINDEX2, OnChangeEditindex2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPINMAT1, OnDeltaposSpinmat1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPINMAT2, OnDeltaposSpinmat2)
	ON_BN_CLICKED(IDC_STRETCHCHECK1, OnStretchcheck1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MaterialView message handlers

void MaterialView::OnPaint() 
{
	//CPaintDC dc(this); // device context for painting
	
	// Set the palette for the frames
	if ((!m_pFrame1) || (!m_pFrame2))
		return;

	m_pFrame1->m_pSurface->setPalette(palette, false);
	m_pFrame2->m_pSurface->setPalette(palette, false);

	// Redraw the textures
	if (material_list)
	{
		material_list->setDetailLevel(0);
		m_pFrame1->m_pMaterial = &material_list->getMaterial(material_index1);
		m_pFrame1->Refresh();
		m_pFrame2->m_pMaterial = &material_list->getMaterial(material_index2);
		m_pFrame2->Refresh();
	}

	// Do not call CDialog::OnPaint() for painting messages
	CPaintDC dc( this );
}

BOOL MaterialView::OnInitDialog() 
{
	char s_val[8];

	CDialog::OnInitDialog();

	CWnd *temp = GetDlgItem(IDC_CANVAS1);
	RECT rect;
	temp->GetClientRect(&rect);
	m_pFrame1 = new CMatCanvas;
	m_pFrame1->Create(temp, rect, IDC_FRAME1);

	temp = GetDlgItem(IDC_CANVAS2);
	temp->GetClientRect(&rect);
	m_pFrame2 = new CMatCanvas;
	m_pFrame2->Create(temp,rect, IDC_FRAME2);

    m_pFrame1->stretchit = stretchit;
    m_pFrame2->stretchit = stretchit;

	if (material_list)
	{
		material_list->setDetailLevel(0);
		m_pFrame1->m_pMaterial = &material_list->getMaterial(material_index1);
		m_pFrame2->m_pMaterial = &material_list->getMaterial(material_index1);
	}

	// Set up the spin controls
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPINMAT1))->SetRange(0, material_count-1);
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPINMAT2))->SetRange(0, material_count-1);

	((CSpinButtonCtrl*)GetDlgItem(IDC_SPINMAT1))->SetPos(material_index1);
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPINMAT2))->SetPos(material_index2);

	sprintf(s_val, "%d", material_index1);
//	((CEdit*)GetDlgItem(IDC_EDITINDEX1))->ReplaceSel((LPCTSTR)s_val, FALSE);

	sprintf(s_val, "%d", material_index2);
//	((CEdit*)GetDlgItem(IDC_EDITINDEX2))->ReplaceSel((LPCTSTR)s_val, FALSE);

    if (m_pFrame1)
    {
      char  buf[32];
      if ((m_pFrame1->m_pMaterial->fParams.fFlags & TSMaterial::MatFlags) == TSMaterial::MatTexture)
      {
        const GFXBitmap*  bmp = m_pFrame1->m_pMaterial->getTextureMap();
        sprintf(buf, "(%d x %d)", bmp->width, bmp->height);
	    ((CEdit*)GetDlgItem(IDC_VIEWTSIZEREADOUT1))->SetSel(0, -1, FALSE);
        ((CEdit*)GetDlgItem(IDC_VIEWTSIZEREADOUT1))->ReplaceSel(buf);
      } else {
        sprintf(buf, "Null Texture");
	    ((CEdit*)GetDlgItem(IDC_VIEWTSIZEREADOUT1))->SetSel(0, -1, FALSE);
        ((CEdit*)GetDlgItem(IDC_VIEWTSIZEREADOUT1))->ReplaceSel(buf);
      }
    }

    if (m_pFrame2)
    {
      char  buf[32];
      if ((m_pFrame2->m_pMaterial->fParams.fFlags & TSMaterial::MatFlags) == TSMaterial::MatTexture)
      {
        const GFXBitmap*  bmp = m_pFrame2->m_pMaterial->getTextureMap();
        sprintf(buf, "(%d x %d)", bmp->width, bmp->height);
	    ((CEdit*)GetDlgItem(IDC_VIEWTSIZEREADOUT2))->SetSel(0, -1, FALSE);
        ((CEdit*)GetDlgItem(IDC_VIEWTSIZEREADOUT2))->ReplaceSel(buf);
      } else {
        sprintf(buf, "Null Texture");
	    ((CEdit*)GetDlgItem(IDC_VIEWTSIZEREADOUT2))->SetSel(0, -1, FALSE);
        ((CEdit*)GetDlgItem(IDC_VIEWTSIZEREADOUT2))->ReplaceSel(buf);
      }
    }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void MaterialView::OnCancel() 
{
	// TODO: Add extra cleanup here
	delete m_pFrame1;
	delete m_pFrame2;

	CDialog::OnCancel();
}

void MaterialView::OnChangeEditindex1() 
{
	char	str_val[10];
	int		tmp;

	((CEdit*)GetDlgItem(IDC_EDITINDEX1))->GetLine(0, (LPTSTR)str_val);

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
	        ((CEdit*)GetDlgItem(IDC_VIEWTSIZEREADOUT1))->SetSel(0, -1, FALSE);
            ((CEdit*)GetDlgItem(IDC_VIEWTSIZEREADOUT1))->ReplaceSel(buf);
          } else {
            sprintf(buf, "Null Texture");
	        ((CEdit*)GetDlgItem(IDC_VIEWTSIZEREADOUT1))->SetSel(0, -1, FALSE);
            ((CEdit*)GetDlgItem(IDC_VIEWTSIZEREADOUT1))->ReplaceSel(buf);
          }
        }
	}
}

void MaterialView::OnChangeEditindex2() 
{
	char	str_val[10];
	int		tmp;

	((CEdit*)GetDlgItem(IDC_EDITINDEX2))->GetLine(0, (LPTSTR)str_val);

	tmp = atoi(str_val);

	if ((tmp >= 0) && (tmp < material_count))
	{
		material_index2 = tmp;

		OnPaint();

        if (m_pFrame2)
        {
          char  buf[32];

          if ((m_pFrame2->m_pMaterial->fParams.fFlags & TSMaterial::MatFlags) == TSMaterial::MatTexture)
          {
            const GFXBitmap*  bmp = m_pFrame2->m_pMaterial->getTextureMap();
            sprintf(buf, "(%d x %d)", bmp->width, bmp->height);
	        ((CEdit*)GetDlgItem(IDC_VIEWTSIZEREADOUT2))->SetSel(0, -1, FALSE);
            ((CEdit*)GetDlgItem(IDC_VIEWTSIZEREADOUT2))->ReplaceSel(buf);
          } else {
            sprintf(buf, "Null Texture");
	        ((CEdit*)GetDlgItem(IDC_VIEWTSIZEREADOUT2))->SetSel(0, -1, FALSE);
            ((CEdit*)GetDlgItem(IDC_VIEWTSIZEREADOUT2))->ReplaceSel(buf);
          }
        }
	}
	
}

void MaterialView::OnDeltaposSpinmat1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	int newval = pNMUpDown->iPos + pNMUpDown->iDelta;

	if ((newval >= 0) && (newval < material_count))
	{
		char str_val[10];
		sprintf(str_val, "%d", newval);
		((CEdit*)GetDlgItem(IDC_EDITINDEX1))->SetSel(0, -1, TRUE);

		((CEdit*)GetDlgItem(IDC_EDITINDEX1))->ReplaceSel((LPCTSTR)str_val, FALSE);

		material_index1 = newval;

		OnPaint();
	}

	*pResult = 0;
}

void MaterialView::OnDeltaposSpinmat2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	int newval = pNMUpDown->iPos + pNMUpDown->iDelta;

	if ((newval >= 0) && (newval < material_count))
	{
		char str_val[10];
		sprintf(str_val, "%d", newval);
		((CEdit*)GetDlgItem(IDC_EDITINDEX2))->SetSel(0, -1, TRUE);

		((CEdit*)GetDlgItem(IDC_EDITINDEX2))->ReplaceSel((LPCTSTR)str_val, FALSE);

		material_index2 = newval;

		OnPaint();
	}
	
	*pResult = 0;
}

BOOL MaterialView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class
	cs.lpszClass = AfxRegisterWndClass(CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT,
										::LoadCursor(NULL,IDC_ARROW));
	
	return CDialog::PreCreateWindow(cs);
}

void MaterialView::OnStretchcheck1() 
{
  stretchit ^= 1;

  m_pFrame1->stretchit = stretchit;
  m_pFrame2->stretchit = stretchit;

  OnPaint();
}
