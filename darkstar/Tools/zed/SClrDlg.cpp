// SClrDlg.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "SClrDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectColorDialog dialog


CSelectColorDialog::CSelectColorDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectColorDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectColorDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

    m_pFrame = NULL;
}

CSelectColorDialog::~CSelectColorDialog()
{
    if (m_pFrame)
    {
	  delete m_pFrame;
    }
}

void CSelectColorDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectColorDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectColorDialog, CDialog)
	//{{AFX_MSG_MAP(CSelectColorDialog)
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_CBN_SELCHANGE(IDC_COLORSELECT, OnSelchangeColorselect)
	ON_BN_CLICKED(ID_COLOR_DEFAULTS, OnColorDefaults)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectColorDialog message handlers

void CSelectColorDialog::OnPaint() 
{
  RectI   therect;
  int     row, col;

  // Set the palette for the frame
  m_pFrame->m_pSurface->setPalette(palette, false);
  m_pFrame->Lockit();
  m_pFrame->m_pSurface->clear(255);

  for (row = 0; row < 16; row++)
  {
    for (col = 0; col < 16; col++)
    {
	  if (m_pFrame)
      {
        therect.upperL.x = (col * 15) + 2;
        therect.upperL.y = (row * 15) + 2;
        therect.lowerR.x = therect.upperL.x + 12;
        therect.lowerR.y = therect.upperL.y + 12;

	    // Redraw the colors
	    m_pFrame->DrawFilledRect(&therect, ((row * 16) + col));
      }
    }
  }

  DrawOutlineRect();
  m_pFrame->Unlockit();

  // Do not call CDialog::OnPaint() for painting messages
}

void CSelectColorDialog::DrawOutlineRect()
{
  int row, col;
  RectI therect;

  row = (selectedcolor[current_selection] / 16);
  col = selectedcolor[current_selection] - ((selectedcolor[current_selection] / 16) * 16);

  therect.upperL.x = (col * 15) + 1;
  therect.upperL.y = (row * 15) + 1;
  therect.lowerR.x = therect.upperL.x + 13;
  therect.lowerR.y = therect.upperL.y + 13;
  
  m_pFrame->DrawRect(&therect, 249);
}

void CSelectColorDialog::EraseOutlineRect()
{
  int row, col;
  RectI therect;

  m_pFrame->m_pSurface->setPalette(palette, false);
  m_pFrame->Lockit();

  row = (selectedcolor[current_selection] / 16);
  col = selectedcolor[current_selection] - ((selectedcolor[current_selection] / 16) * 16);

  therect.upperL.x = (col * 15) + 1;
  therect.upperL.y = (row * 15) + 1;
  therect.lowerR.x = therect.upperL.x + 13;
  therect.lowerR.y = therect.upperL.y + 13;
  
  m_pFrame->DrawRect(&therect, 255);

  m_pFrame->Unlockit();
}

BOOL CSelectColorDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CWnd *temp;
	RECT rect;

    temp = GetDlgItem(IDD_RECT0);
	temp->GetClientRect(&rect);

	m_pFrame = new CMatCanvas;
	m_pFrame->Create(temp, rect, IDD_RECT0);

    ((CComboBox*)GetDlgItem(IDC_COLORSELECT))->SetCurSel(current_selection);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectColorDialog::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
    switch (nChar)
    {
    case VK_UP:
      selectedcolor[current_selection] -= 16;
      if (selectedcolor[current_selection] < 0)
      {
        selectedcolor[current_selection] += 256;
      }
      break;
    case VK_DOWN:
      selectedcolor[current_selection] += 16;
      if (selectedcolor[current_selection] > 255)
      {
        selectedcolor[current_selection] -= 256;
      }
      break;
    case VK_LEFT:
      selectedcolor[current_selection]--;
      if (selectedcolor[current_selection] < 0)
      {
        selectedcolor[current_selection] += 256;
      }
      break;
    case VK_RIGHT:
      selectedcolor[current_selection]++;
      if (selectedcolor[current_selection] > 255)
      {
        selectedcolor[current_selection] -= 256;
      }
      break;
    }
	
    OnPaint();

	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CSelectColorDialog::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
    {
      int nVirtKey = (int)pMsg->wParam;

      EraseOutlineRect();

      switch (nVirtKey)
      {
      case VK_UP:
        selectedcolor[current_selection] -= 16;
        if (selectedcolor[current_selection] < 0)
        {
          selectedcolor[current_selection] += 256;
        }
        OnPaint();
        return TRUE;
        break;
      case VK_DOWN:
        selectedcolor[current_selection] += 16;
        if (selectedcolor[current_selection] > 255)
        {
          selectedcolor[current_selection] -= 256;
        }
        OnPaint();
        return TRUE;
        break;
      case VK_LEFT:
        selectedcolor[current_selection]--;
        if (selectedcolor[current_selection] < 0)
        {
          selectedcolor[current_selection] += 256;
        }
        OnPaint();
        return TRUE;
        break;
      case VK_RIGHT:
        selectedcolor[current_selection]++;
        if (selectedcolor[current_selection] > 255)
        {
          selectedcolor[current_selection] -= 256;
        }
        OnPaint();
        return TRUE;
        break;
      }
    }

	return CDialog::PreTranslateMessage(pMsg);
}

void CSelectColorDialog::OnSelchangeColorselect() 
{
  current_selection = ((CComboBox*)GetDlgItem(IDC_COLORSELECT))->GetCurSel();
}

void CSelectColorDialog::OnColorDefaults() 
{
  PALETTEENTRY  color_request;

  selectedcolor[BACKGROUND] = 0;
  selectedcolor[GRID_ORIGIN] = 251;
  selectedcolor[ADDITIVE_BRUSH] = 250;
  selectedcolor[SUBTRACTIVE_BRUSH] = 249;
  selectedcolor[SELECTED_BRUSH] = 253;
  selectedcolor[DEFAULT_BRUSH] = 252;
  selectedcolor[VOLUME_BRUSH] = 254;

  color_request.peRed = 64;
  color_request.peGreen = 64;
  color_request.peBlue = 64;
  selectedcolor[LINE_GRID] = palette->GetNearestColor(color_request);

  selectedcolor[DOT_GRID] = 255;
  selectedcolor[BOUNDING_BOX] = 3;
}
