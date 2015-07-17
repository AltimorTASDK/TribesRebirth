// BOrdrDlg.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "threddoc.h"
#include "BOrdrDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBrushOrderDialog dialog


CBrushOrderDialog::CBrushOrderDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CBrushOrderDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBrushOrderDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CBrushOrderDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBrushOrderDialog)
	DDX_Control(pDX, IDC_BRUSHLIST, m_BrushList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBrushOrderDialog, CDialog)
	//{{AFX_MSG_MAP(CBrushOrderDialog)
	ON_LBN_SELCHANGE(IDC_BRUSHLIST, OnSelchangeBrushlist)
	ON_BN_CLICKED(IDC_BUTTON_BACK, OnButtonBack)
	ON_BN_CLICKED(IDC_BUTTON_BOTTOM, OnButtonBottom)
	ON_BN_CLICKED(IDC_BUTTON_FORWARD, OnButtonForward)
	ON_BN_CLICKED(IDC_BUTTON_TOP, OnButtonTop)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrushOrderDialog message handlers

void CBrushOrderDialog::OnSelchangeBrushlist() 
{
  // get the current brush number
  int Id = m_BrushList.GetCurSel();

  GetDlgItem(IDC_BUTTON_BACK)->EnableWindow(0);
  GetDlgItem(IDC_BUTTON_TOP)->EnableWindow(0);
  GetDlgItem(IDC_BUTTON_BOTTOM)->EnableWindow(0);
  GetDlgItem(IDC_BUTTON_FORWARD)->EnableWindow(0);

  // if we have no selection leave
  if(Id != LB_ERR)
  {
    if (Id == 0)
    {
      GetDlgItem(IDC_BUTTON_BOTTOM)->EnableWindow(1);
      GetDlgItem(IDC_BUTTON_FORWARD)->EnableWindow(1);
    }

    if (Id == m_BrushList.GetCount()-1)
    {
      GetDlgItem(IDC_BUTTON_BACK)->EnableWindow(1);
      GetDlgItem(IDC_BUTTON_TOP)->EnableWindow(1);
    }

    if ((Id > 0) && (Id < m_BrushList.GetCount()-1))
    {
      GetDlgItem(IDC_BUTTON_BACK)->EnableWindow(1);
      GetDlgItem(IDC_BUTTON_TOP)->EnableWindow(1);
      GetDlgItem(IDC_BUTTON_BOTTOM)->EnableWindow(1);
      GetDlgItem(IDC_BUTTON_FORWARD)->EnableWindow(1);
    }
  }
}

BOOL CBrushOrderDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    ((CListBox*)GetDlgItem(IDC_BRUSHLIST))->SetTabStops(4);

	FillInNames();
	
    GetDlgItem(IDC_BUTTON_BACK)->EnableWindow(0);
    GetDlgItem(IDC_BUTTON_TOP)->EnableWindow(0);
    GetDlgItem(IDC_BUTTON_BOTTOM)->EnableWindow(0);
    GetDlgItem(IDC_BUTTON_FORWARD)->EnableWindow(0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBrushOrderDialog::FillInNames()
{
  CThredBrush*  brush;
  char          tmpbuf[64];

  brush = pDoc->mBrushList;

  while (brush)
  {
    sprintf(tmpbuf, "%3d\t\t\t\t\t%s\t%s\t%2d",
            brush->mBrushId,
            (brush->mFlags & BRUSH_ACTIVE) ? "Active\t\t\t" : "Inactive\t\t",
            (brush->mFlags & BRUSH_SELECTED) ? "Selected\t\t\t\t\t" : "Unselected\t\t\t",
            brush->mBrushGroup);

	int Id = m_BrushList.AddString(tmpbuf);

	// set the information to tell us what
	// group this name goes with
	m_BrushList.SetItemData(Id, (unsigned long)brush);

    brush = brush->NextBrush;
  }
}

void CBrushOrderDialog::OnButtonTop() 
{
  CThredBrush*  tgtbrush;
  CThredBrush*  brush;

  int id = m_BrushList.GetCurSel();

  if (id < 0)
    return;

  // get the data
  tgtbrush = (CThredBrush*)m_BrushList.GetItemData(id);

  // Move selected brush back one in the brush list (wrt group)
  brush = pDoc->mBrushList;

  // Remove brush from its current brush list position
  while(brush)
  {
    if (brush == tgtbrush)
    {
      if (brush->NextBrush)
      {
        brush->NextBrush->PrevBrush = brush->PrevBrush;
      }

      if (brush->PrevBrush)
      {
        brush->PrevBrush->NextBrush = brush->NextBrush;
      }

      if (brush == pDoc->mBrushList)
      {
        // Trying to remove the 1st brush...
        pDoc->mBrushList = brush->NextBrush;
      }

      brush = NULL; // Get outta the loop
    } else {
      brush = brush->GetNextBrush();
    }
  }

  // Reposition brush to its new brush list position
  brush = pDoc->mBrushList;

  tgtbrush->PrevBrush = NULL;
  tgtbrush->NextBrush = brush;
  brush->PrevBrush = tgtbrush;
  pDoc->mBrushList = tgtbrush;

  m_BrushList.ResetContent();
  FillInNames();
  id = 0;
  m_BrushList.SetCurSel(id);
  OnSelchangeBrushlist();
}

void CBrushOrderDialog::OnButtonBottom() 
{
  CThredBrush*  tgtbrush;
  CThredBrush*  brush;
  CThredBrush*  lastbrush;
  int           tgtbrushid;
  int           tgtgroupid;

  int id = m_BrushList.GetCurSel();

  if (id < 0)
    return;

  // get the data
  tgtbrush = (CThredBrush*)m_BrushList.GetItemData(id);
  tgtbrushid = tgtbrush->mBrushId;
  tgtgroupid = tgtbrush->mBrushGroup;

  // Move selected brush back one in the brush list (wrt group)
  brush = pDoc->mBrushList;

  // Remove brush from its current brush list position
  while(brush)
  {
    if (brush == tgtbrush)
    {
      if (brush->NextBrush)
      {
        brush->NextBrush->PrevBrush = brush->PrevBrush;
      }
      if (brush->PrevBrush)
      {
        brush->PrevBrush->NextBrush = brush->NextBrush;
      }

      if (brush == pDoc->mBrushList)
      {
        // Trying to remove the 1st brush...
        pDoc->mBrushList = brush->NextBrush;
      }

      brush = NULL; // Get outta the loop
    } else {
      brush = brush->GetNextBrush();
    }
  }

  // Reposition brush to its new brush list position
  brush = pDoc->mBrushList;
  lastbrush = NULL;

  while(brush)
  {
    lastbrush = brush;
    brush = brush->GetNextBrush();
  }

  // lastbrush should always point to something since
  // we won't do this if there's only 1 brush in the group
  tgtbrush->PrevBrush = lastbrush;
  tgtbrush->NextBrush = lastbrush->NextBrush;
    
  if (lastbrush->NextBrush)
  {
    lastbrush->NextBrush->PrevBrush = tgtbrush;
  }

  lastbrush->NextBrush = tgtbrush;

  m_BrushList.ResetContent();
  FillInNames();
  id = m_BrushList.GetCount() - 1;
  m_BrushList.SetCurSel(id);
  OnSelchangeBrushlist();
}

void CBrushOrderDialog::OnButtonBack() 
{
  CThredBrush*  tgtbrush;
  CThredBrush*  brush;
  CThredBrush*  lastbrush;
  int           tgtbrushid;
  int           tgtgroupid;

  int id = m_BrushList.GetCurSel();

  if (id < 0)
    return;

  // get the data
  tgtbrush = (CThredBrush*)m_BrushList.GetItemData(id);
  tgtbrushid = tgtbrush->mBrushId;
  tgtgroupid = tgtbrush->mBrushGroup;

  // Move selected brush back one in the brush list (wrt group)
  brush = pDoc->mBrushList;

  // Remove brush from its current brush list position
  while(brush)
  {
    if (brush == tgtbrush)
    {
      lastbrush = brush->PrevBrush;

      if (brush->NextBrush)
      {
        brush->NextBrush->PrevBrush = brush->PrevBrush;
      }
      if (brush->PrevBrush)
      {
        brush->PrevBrush->NextBrush = brush->NextBrush;
      }

      if (brush == pDoc->mBrushList)
      {
        // Trying to remove the 1st brush...
        pDoc->mBrushList = brush->NextBrush;
      }

      brush = NULL; // Get outta the loop
    } else {
      brush = brush->GetNextBrush();
    }
  }

  // Reposition brush to its new brush list position
  brush = lastbrush;

  // Okay, put it before this one...
  tgtbrush->PrevBrush = brush->PrevBrush;
  tgtbrush->NextBrush = brush;

  if (brush->PrevBrush)
  {
    brush->PrevBrush->NextBrush = tgtbrush;
  }

  brush->PrevBrush = tgtbrush;

  if (brush == pDoc->mBrushList)
  {
    pDoc->mBrushList = tgtbrush;
  }

  m_BrushList.ResetContent();
  FillInNames();
  id--;
  m_BrushList.SetCurSel(id);
  OnSelchangeBrushlist();
}

void CBrushOrderDialog::OnButtonForward() 
{
  CThredBrush*  tgtbrush;
  CThredBrush*  brush;
  CThredBrush*  lastbrush;
  int           tgtbrushid;
  int           tgtgroupid;

  int id = m_BrushList.GetCurSel();

  if (id < 0)
    return;

  // get the data
  tgtbrush = (CThredBrush*)m_BrushList.GetItemData(id);
  tgtbrushid = tgtbrush->mBrushId;
  tgtgroupid = tgtbrush->mBrushGroup;

  brush = pDoc->mBrushList;

  // Remove brush from its current brush list position
  while(brush)
  {
    if (brush == tgtbrush)
    {
      lastbrush = brush->NextBrush;

      if (brush->NextBrush)
      {
        brush->NextBrush->PrevBrush = brush->PrevBrush;
      }
      if (brush->PrevBrush)
      {
        brush->PrevBrush->NextBrush = brush->NextBrush;
      }

      if (brush == pDoc->mBrushList)
      {
        // Trying to remove the 1st brush...
        pDoc->mBrushList = brush->NextBrush;
      }

      brush = NULL; // Get outta the loop
    } else {
      brush = brush->GetNextBrush();
    }
  }

  // Reposition brush to its new brush list position
  brush = lastbrush;

  // Okay, put it after this one...
  tgtbrush->PrevBrush = brush;
  tgtbrush->NextBrush = brush->NextBrush;

  if (brush->NextBrush)
  {
    brush->NextBrush->PrevBrush = tgtbrush;
  }

  brush->NextBrush = tgtbrush;

  m_BrushList.ResetContent();
  FillInNames();
  id++;
  m_BrushList.SetCurSel(id);

  OnSelchangeBrushlist();
}
