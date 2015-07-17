// BrushGroupDialog.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "threddoc.h"
#include "BrushGroupDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBrushGroupDialog dialog


CBrushGroupDialog::CBrushGroupDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CBrushGroupDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBrushGroupDialog)
	//}}AFX_DATA_INIT
}


void CBrushGroupDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBrushGroupDialog)
	DDX_Control(pDX, IDC_CHECK1, m_BrushVisible);
	DDX_Control(pDX, IDC_LIST2, m_BrushNameList);
	DDX_Control(pDX, IDC_LIST1, m_GroupList);
	DDX_Control(pDX, IDC_EDIT1, m_Name);
	//DDX_Control(pDX, IDC_COLOURBUTTON, m_ColourButton);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBrushGroupDialog, CDialog)
	//{{AFX_MSG_MAP(CBrushGroupDialog)
	ON_LBN_DBLCLK(IDC_LIST2, OnDblclkList2)
	ON_LBN_SELCHANGE(IDC_LIST1, OnSelchangeList1)
	ON_LBN_DBLCLK(IDC_LIST1, OnDblclkList1)
	ON_BN_CLICKED(IDC_BUTTON_TOBACK, OnButtonToback)
	ON_BN_CLICKED(IDC_BUTTON_TOBOTTOM, OnButtonTobottom)
	ON_BN_CLICKED(IDC_BUTTON_TOFORWARD, OnButtonToforward)
	ON_BN_CLICKED(IDC_BUTTON_TOTOP, OnButtonTotop)
	ON_LBN_SELCHANGE(IDC_LIST2, OnSelchangeList2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrushGroupDialog message handlers
/////////////////////////////////////////////////////////////////////////////
// CColourButton

CColourButton::CColourButton()
{
	mCurrentColour = RGB(255,0,255);
}

CColourButton::~CColourButton()
{
}


BEGIN_MESSAGE_MAP(CColourButton, CButton)
	//{{AFX_MSG_MAP(CColourButton)
	ON_CONTROL_REFLECT(BN_CLICKED, OnClicked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColourButton message handlers

void CColourButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// get a cdc
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC );

	// now draw a solid rectangle
	pDC->FillSolidRect(&lpDrawItemStruct->rcItem, mCurrentColour);

	// if we have the focus
	if(lpDrawItemStruct->itemState & ODS_FOCUS ) {
		// get a null brush
		CBrush *NullBrush = CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)), *OldBrush;

		// select the brush
		OldBrush = pDC->SelectObject(NullBrush );

		// draw a cute rectangle around it
		pDC->Rectangle(&lpDrawItemStruct->rcItem);

		// get old
		pDC->SelectObject(OldBrush );
	}
}

// this function returns our current colour
COLORREF CColourButton::GetColour()
{
	return mCurrentColour;
}

void CColourButton::SetColour(COLORREF Colour)
{
	mCurrentColour = Colour;
	// update ourselves
	RedrawWindow();
}

void CColourButton::OnClicked() 
{
	CColorDialog dlg;

	// get a colour
	dlg.DoModal();

	// assign what it was to the current colour
	mCurrentColour = dlg.GetColor();

	// update ourselves
	RedrawWindow();
}

//=============== BRUSHGROUPDIALOG STUFF ====================
//typedef CArray<CBrushGroup, CBrushGroup&> CBrushGroupArray;


// Do zee dialog.
int CBrushGroupDialog::DoDialog(CBrushGroupArray& Array, int CurrentGroup, CThredBrush* BrushList, CTHREDDoc* doc)
{
	mGroupArray = &Array;
	mBrushList = BrushList;
	mCurrentGroup = CurrentGroup;
    pDoc = doc;

	// now make the dialog go!!
	DoModal();

	return mCurrentGroup;
}

// when we are making the dialog
BOOL CBrushGroupDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	FillInNames();
	
    GetDlgItem(IDC_BUTTON_TOBACK)->EnableWindow(0);
    GetDlgItem(IDC_BUTTON_TOTOP)->EnableWindow(0);
    GetDlgItem(IDC_BUTTON_TOBOTTOM)->EnableWindow(0);
    GetDlgItem(IDC_BUTTON_TOFORWARD)->EnableWindow(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// go through the list of groups and fill the names
// in the box... assign the groupnumber to the name
void CBrushGroupDialog::FillInNames()
{
	m_GroupList.ResetContent();
	// iterate
	for(int Group = 0; Group < mGroupArray->GetSize(); Group++)
    {
		// get the name
		CString FillName;
		FillName.Format("%s [%c]", (LPCTSTR)(*mGroupArray)[Group].GetName() ,(*mGroupArray)[Group].IsVisible() ? 'X' : ' ');

		// add the string to the box
		int Id = m_GroupList.AddString(FillName);

		// set the information to tell us what
		// group this name goes with
		m_GroupList.SetItemData(Id, Group);

		// if this is the current group select it
		if(Group == mCurrentGroup)
        {
			m_GroupList.SetCurSel(Id);
        }
	}

	// get the current group
	mCurrentGroup = m_GroupList.GetCurSel();
	if(mCurrentGroup != LB_ERR )
		mCurrentGroup = m_GroupList.GetItemData(mCurrentGroup);
	FillInCurrentGroup();
}

// fill in the current group
void CBrushGroupDialog::FillInCurrentGroup()
{
	// check if we are ok
	if(mCurrentGroup == LB_ERR )
		return;

	// if we are ok fill in the information
	m_Name.SetWindowText((*mGroupArray)[mCurrentGroup].GetName() );

	// set whether we are visible
	if((*mGroupArray)[mCurrentGroup].IsVisible() )
		m_BrushVisible.SetCheck(1);
	else
		m_BrushVisible.SetCheck(0);

	// set our current colour
	//dpw m_ColourButton.SetColour((*mGroupArray)[mCurrentGroup].GetColour() );

	// go through our list of brushes and add any with the
	// same group
	m_BrushNameList.ResetContent();
	//CThredBrush* CurrentBrush = mBrushList;
	CThredBrush* CurrentBrush = pDoc->mBrushList;

	while(CurrentBrush)
    {
		if(CurrentBrush->mBrushGroup == mCurrentGroup)
        {
			// add the string
			//int Id = m_BrushNameList.AddString(CurrentBrush->mName );
            char  tmpbuf[16];
            sprintf(tmpbuf, "%d", CurrentBrush->mBrushId);
			//int Id = m_BrushNameList.AddString(CurrentBrush->mName );
			int Id = m_BrushNameList.AddString(tmpbuf);

			// put a pointer in the data
			m_BrushNameList.SetItemData(Id, (DWORD)CurrentBrush);
		}
		// go to the next brush
		CurrentBrush = CurrentBrush->GetNextBrush();
	}
}

void CBrushGroupDialog::UnfillCurrentGroup()
{
	// check if we are ok
	if(mCurrentGroup == LB_ERR )
		return;

	CString Name;

	// if we are ok fill in the information
	m_Name.GetWindowText(Name );
	(*mGroupArray)[mCurrentGroup].SetName(Name);

	// get whether we are visible
	(*mGroupArray)[mCurrentGroup].SetVisible(m_BrushVisible.GetCheck());

	// get our current colour
	//dpw (*mGroupArray)[mCurrentGroup].SetColour(m_ColourButton.GetColour()) ;

}

void CBrushGroupDialog::OnDblclkList2() 
{
	// get the id
	int Id = m_BrushNameList.GetCurSel();

	// get the data
	CThredBrush* CurrentBrush = (CThredBrush*)m_BrushNameList.GetItemData(Id);

	// get the dialog
	CurrentBrush->BrushAttributesDialog(pDoc);
}

void CBrushGroupDialog::OnSelchangeList1() 
{
	// unfill
	UnfillCurrentGroup();	

	// get the new brush number
	int Id = m_GroupList.GetCurSel();

	// if we have no selection leave
	if(Id == LB_ERR ) {
		mCurrentGroup = Id;
		return;
	}

	// get the data
	mCurrentGroup = m_GroupList.GetItemData(Id);

	// fill in
	FillInNames();
}

void CBrushGroupDialog::OnOK() 
{
	UnfillCurrentGroup();
	CDialog::OnOK();
}

// When they double-click the group we want to change the
// visibility
void CBrushGroupDialog::OnDblclkList1() 
{
	if(m_BrushVisible.GetCheck() )
		m_BrushVisible.SetCheck(0);
	else
		m_BrushVisible.SetCheck(1);

	UnfillCurrentGroup();	
	FillInNames();
}

void CBrushGroupDialog::OnButtonToback() 
{
  CThredBrush*  tgtbrush;
  CThredBrush*  brush;
  CThredBrush*  lastbrush;
  int           tgtbrushid;
  int           tgtgroupid;

  int id = m_BrushNameList.GetCurSel();

  if (id < 0)
    return;

  // get the data
  tgtbrush = (CThredBrush*)m_BrushNameList.GetItemData(id);
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
  brush = lastbrush;//pDoc->mBrushList;

  while(brush)
  {
    if (brush->mBrushGroup == tgtgroupid)
    {
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

      // Get outta the loop
      brush = NULL;
    } else {
      brush = brush->PrevBrush;
    }
  }

  OnSelchangeList1();
  OnSelchangeList2();
}

// Move selected brush to bottom of brush list (wrt group)
void CBrushGroupDialog::OnButtonTobottom() 
{
  CThredBrush*  tgtbrush;
  CThredBrush*  brush;
  CThredBrush*  lastbrush;
  int           tgtbrushid;
  int           tgtgroupid;

  int id = m_BrushNameList.GetCurSel();

  if (id < 0)
    return;

  // get the data
  tgtbrush = (CThredBrush*)m_BrushNameList.GetItemData(id);
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
    if (brush->mBrushGroup == tgtgroupid)
    {
      lastbrush = brush;
    }
    
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

  OnSelchangeList1();
  OnSelchangeList2();
}

// Move selected brush to top of brush list (wrt group)
void CBrushGroupDialog::OnButtonTotop() 
{
  CThredBrush*  tgtbrush;
  CThredBrush*  brush;
  int           tgtbrushid;
  int           tgtgroupid;

  int id = m_BrushNameList.GetCurSel();

  if (id < 0)
    return;

  // get the data
  tgtbrush = (CThredBrush*)m_BrushNameList.GetItemData(id);
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

  while(brush)
  {
    if (brush->mBrushGroup == tgtgroupid)
    {
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

      // Get outta the loop
      brush = NULL;
    } else {
      brush = brush->GetNextBrush();
    }
  }

  OnSelchangeList1();
  OnSelchangeList2();
}

// Move selected brush forward one in the brush list (wrt group)
void CBrushGroupDialog::OnButtonToforward() 
{
  CThredBrush*  tgtbrush;
  CThredBrush*  brush;
  CThredBrush*  lastbrush;
  int           tgtbrushid;
  int           tgtgroupid;

  int id = m_BrushNameList.GetCurSel();

  if (id < 0)
    return;

  // get the data
  tgtbrush = (CThredBrush*)m_BrushNameList.GetItemData(id);
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

  while(brush)
  {
    if (brush->mBrushGroup == tgtgroupid)
    {
      // Okay, put it after this one...
      tgtbrush->PrevBrush = brush;
      tgtbrush->NextBrush = brush->NextBrush;

      if (brush->NextBrush)
      {
        brush->NextBrush->PrevBrush = tgtbrush;
      }

      brush->NextBrush = tgtbrush;

      //if (brush == pDoc->mBrushList)
      //{
      //  pDoc->mBrushList = tgtbrush;
      //}

      // Get outta the loop
      brush = NULL;
    } else {
      brush = brush->NextBrush;
    }
  }

  OnSelchangeList1();
  OnSelchangeList2();
}

void CBrushGroupDialog::OnSelchangeList2() 
{
	// get the new brush number
	int Id = m_BrushNameList.GetCurSel();

    GetDlgItem(IDC_BUTTON_TOBACK)->EnableWindow(0);
    GetDlgItem(IDC_BUTTON_TOTOP)->EnableWindow(0);
    GetDlgItem(IDC_BUTTON_TOBOTTOM)->EnableWindow(0);
    GetDlgItem(IDC_BUTTON_TOFORWARD)->EnableWindow(0);

	// if we have no selection leave
	if(Id != LB_ERR)
    {
      if (Id == 0)
      {
        GetDlgItem(IDC_BUTTON_TOBOTTOM)->EnableWindow(1);
        GetDlgItem(IDC_BUTTON_TOFORWARD)->EnableWindow(1);
      }

      if (Id == m_BrushNameList.GetCount()-1)
      {
        GetDlgItem(IDC_BUTTON_TOBACK)->EnableWindow(1);
        GetDlgItem(IDC_BUTTON_TOTOP)->EnableWindow(1);
      }

      if ((Id > 0) && (Id < m_BrushNameList.GetCount()-1))
      {
        GetDlgItem(IDC_BUTTON_TOBACK)->EnableWindow(1);
        GetDlgItem(IDC_BUTTON_TOTOP)->EnableWindow(1);
        GetDlgItem(IDC_BUTTON_TOBOTTOM)->EnableWindow(1);
        GetDlgItem(IDC_BUTTON_TOFORWARD)->EnableWindow(1);
      }
    }
}
