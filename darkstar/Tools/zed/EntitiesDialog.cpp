// EntitiesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "EntitiesDialog.h"
#include "selectclassname.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEntitiesDialog dialog
#define KEY_STRING	"101010nobody101010"


CEntitiesDialog::CEntitiesDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CEntitiesDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEntitiesDialog)
	m_ShowLights = FALSE;
	//}}AFX_DATA_INIT
	mCurrentEntity = 0;
	mCurrentKey = LB_ERR;
	CString crep = KEY_STRING;
}

void CEntitiesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEntitiesDialog)
	DDX_Control(pDX, IDC_VALUE_EDIT, m_ValueEdit);
	DDX_Control(pDX, IDC_KEY_EDIT, m_KeyEdit);
	DDX_Control(pDX, IDC_PAIRS, m_Pairs);
	DDX_Control(pDX, IDC_ENTITYLIST, m_EntityList);
	//DDX_Check(pDX, IDC_SHOWLIGHTS, m_ShowLights);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEntitiesDialog, CDialog)
	//{{AFX_MSG_MAP(CEntitiesDialog)
	ON_BN_CLICKED(IDC_NEWENTITY, OnNewentity)
	ON_BN_CLICKED(IDC_NEWKEYVALUE, OnNewkeyvalue)
	ON_BN_CLICKED(IDC_NEWLIGHT, OnNewlight)
	ON_LBN_SELCHANGE(IDC_PAIRS, OnSelchangePairs)
	ON_EN_KILLFOCUS(IDC_KEY_EDIT, OnKillfocusKeyEdit)
	ON_EN_KILLFOCUS(IDC_VALUE_EDIT, OnKillfocusValueEdit)
	ON_LBN_SELCHANGE(IDC_ENTITYLIST, OnSelchangeEntitylist)
	ON_BN_CLICKED(IDC_DELETEENTITY, OnDeleteentity)
	ON_BN_CLICKED(IDC_DELETEKEYVALUE, OnDeletekeyvalue)
	ON_EN_CHANGE(IDC_MININTENSITY, OnChangeMinintensity)
	ON_BN_CLICKED(IDC_COLORBUTTON, OnColorbutton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEntitiesDialog message handlers

int CEntitiesDialog::EditEntity(CEntityArray& Entities, int CurrentEntity, CTHREDDoc* Doc)
{
	pDoc = Doc;
	// the list of entities
	//if(CurrentEntity != -1 )
		mCurrentEntity = CurrentEntity;
	mEntityArray = &Entities;
	DoModal();
	return mCurrentEntity;
}

int CEntitiesDialog::DoDialog(CEntityArray& Entities, CTHREDDoc* Doc)
{
	// the list of entities
	mEntityArray = &Entities;
	pDoc = Doc;
	DoModal();
	return mCurrentEntity;
}

BOOL CEntitiesDialog::OnInitDialog() 
{
	// do some stuff
	CDialog::OnInitDialog();
	
    // Insert min_intensity value...
    char str_val[40];
	sprintf(str_val, "%lf", min_intensity);
    ((CEdit*)GetDlgItem(IDC_MININTENSITY))->ReplaceSel((LPCTSTR)str_val, FALSE);

	// we have to go through and setup all of our
	// stuff!!!
	FillInDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//============================================================
// This funtion will go through the list of key value
// pairs and add all of the entities properly into the
// correct arrays
//============================================================
void CEntitiesDialog::FillInDialog()
{
	int CurrentEnt;
	int NumberOfEnts = (*mEntityArray).GetSize();
	int RealIndex;
	CString EntityName;

	// clear our main stuff
	m_EntityList.ResetContent();

    RealIndex = -1;

	// go through the array of entities
	for(CurrentEnt = 0; CurrentEnt < NumberOfEnts; CurrentEnt++ ) {
		// get the name
		EntityName = (*mEntityArray)[CurrentEnt].GetName();

		// add the name to the entity list
		int Index = m_EntityList.AddString(EntityName );

		if(CurrentEnt == mCurrentEntity )
			RealIndex = Index;

		// set the item data to tell us what entity this suck is from
		m_EntityList.SetItemData(Index, CurrentEnt);
	}

	// set the current selection to what it should be in the listbox
    if (RealIndex >= 0)
    {
	  m_EntityList.SetCurSel(RealIndex );
    }

	// if we have no entities set to nothing.
	if(!NumberOfEnts )
		mCurrentEntity = LB_ERR;

	// now we want to fill in the key/value stuff for the
	// current entity
	FillInKeyValuePairs();

}

//================================================================================
// This function fills in the key value pairs for the current dialog.
//================================================================================
void CEntitiesDialog::FillInKeyValuePairs(int Selection)
{
	CString KeyValueString;

	// now get the actual entity number from that
	int Entity = mCurrentEntity;

	// now go through that entity and add key/value pairs
	// to the dialog
	m_Pairs.ResetContent();

	// what do we have here?
	if(Entity == LB_ERR ) {
		mCurrentKey = LB_ERR;
		return;
	}

	// go through
	for(int Current = 0; Current < (*mEntityArray)[Entity].mKeyArray.GetSize(); Current++ ) {
		// get the string
		KeyValueString.Format("\"%s\" \"%s\"", (*mEntityArray)[Entity].mKeyArray[Current],
			(*mEntityArray)[Entity].mValueArray[Current]);

		// add it to the listbox
		m_Pairs.AddString(KeyValueString );
	}

	// set the current
	m_Pairs.SetCurSel(Selection );

	// update the current key and the text
	OnSelchangePairs();
}

// When this is called we want to add a new entity....
void CEntitiesDialog::OnNewentity() 
{
	CSelectClassname dlg;
	char **ClassnameList;
	// get a list of default key/value pairs for this object.
	ClassnameList = dlg.GetClassname();
	NewEntity(ClassnameList);
}

void CEntitiesDialog::OnNewlight() 
{
	char *LightList[] = { "classname", "light", "State", " ", "pos", "1 1 1", "color", "0.7 0.7 0.7", "distance", "0.0 0.0007 0.0", "" };
	NewEntity(LightList );
}
							
void CEntitiesDialog::OnSelchangePairs() 
{
	CString CurSelText;

	// get the text
	mCurrentKey = m_Pairs.GetCurSel();

	if(mCurrentKey != LB_ERR)
    {
		// stick it in the edit boxes
		m_KeyEdit.SetWindowText((*mEntityArray)[mCurrentEntity].mKeyArray[mCurrentKey]);
		m_ValueEdit.SetWindowText((*mEntityArray)[mCurrentEntity].mValueArray[mCurrentKey]);

        if (!strcmp( (*mEntityArray)[mCurrentEntity].mKeyArray[mCurrentKey], "color"))
        {
          GetDlgItem(IDC_COLORBUTTON)->EnableWindow(1);
        } else {
          GetDlgItem(IDC_COLORBUTTON)->EnableWindow(0);
        }
	} else {
		// stick it in the edit boxes
		m_KeyEdit.SetWindowText("");
		m_ValueEdit.SetWindowText("");
	}
}

// we have to get our next text in here
void CEntitiesDialog::OnKillfocusKeyEdit() 
{
	CString Text;

	if(mCurrentKey == LB_ERR )
		return;

	// get the text
	m_KeyEdit.GetWindowText(Text );

	// assign the text
	(*mEntityArray)[mCurrentEntity].mKeyArray[mCurrentKey] = Text;

	// update
	FillInKeyValuePairs(mCurrentKey);
}

// we have to get our next text in here
void CEntitiesDialog::OnKillfocusValueEdit() 
{
	CString Text;

	if(mCurrentKey == LB_ERR )
		return;

	// get the text
	m_ValueEdit.GetWindowText(Text );

	// assign the text
	(*mEntityArray)[mCurrentEntity].mValueArray[mCurrentKey] = Text;

	// check if we changed the class name
	if((*mEntityArray)[mCurrentEntity].mKeyArray[mCurrentKey] == CLASSNAME ) {
		FillInDialog();
	}

	// update
	FillInKeyValuePairs(mCurrentKey);
}

void CEntitiesDialog::OnNewkeyvalue() 
{
	if(mCurrentEntity == LB_ERR )
		return;

	// add a new string to the end of each array
	(*mEntityArray)[mCurrentEntity].mKeyArray.Add("NewKey");
	(*mEntityArray)[mCurrentEntity].mValueArray.Add("NewValue");

	// fill in the pairs
	FillInKeyValuePairs((*mEntityArray)[mCurrentEntity].mKeyArray.GetSize()-1 );

	// goto the edit control.
	GotoDlgCtrl(&m_KeyEdit);
}

void CEntitiesDialog::OnSelchangeEntitylist() 
{
	// get the current selection from the entity list
	int CurrentSelection = m_EntityList.GetCurSel();

	// we have an error
	if(CurrentSelection == LB_ERR )
		mCurrentEntity = LB_ERR;
	else {
		// now get the actual entity number from that
		mCurrentEntity = m_EntityList.GetItemData(CurrentSelection );
	}

	// this will just clear it
	FillInKeyValuePairs();
}

void CEntitiesDialog::OnDeleteentity() 
{
	if(mCurrentEntity == LB_ERR )
		return;

	// delete zee entity
	pDoc->DeleteEntity(mCurrentEntity );

	// refill the dialog
	mCurrentEntity = 0;
	FillInDialog();
}

void CEntitiesDialog::OnDeletekeyvalue() 
{
	if(mCurrentKey == LB_ERR )
		return;

	// assign the text
	(*mEntityArray)[mCurrentEntity].mKeyArray.RemoveAt(mCurrentKey );
	(*mEntityArray)[mCurrentEntity].mValueArray.RemoveAt(mCurrentKey );

	// update
	FillInKeyValuePairs();
}

// go through all of the brushes that are selected and associate
// them with this entity
// the brush inherites the current group of the current entity
void CEntitiesDialog::OnAssociateSelectedbrush() 
{
	// go through the brushes
	CThredBrush* CurrentBrush = pDoc->mBrushList;

	// check for selected brushes
	while(CurrentBrush ) {
		// unselect the brush after
		if(CurrentBrush->mFlags & BRUSH_SELECTED) {
			CurrentBrush->mFlags &= ~BRUSH_SELECTED;
			CurrentBrush->mFlags |= BRUSH_ENTITY;
			CurrentBrush->mEntityId = mCurrentEntity;
			CurrentBrush->mBrushGroup = (*mEntityArray)[mCurrentEntity].mGroup;
		}
		CurrentBrush = CurrentBrush->GetNextBrush();
	}
	pDoc->UpdateBrushInformation();
}

// go through and associate the brushes of the current
// group to this entity.
// the entity inherits the brushes group.
void CEntitiesDialog::OnAssociategroup() 
{
	// go through the brushes
	CThredBrush* CurrentBrush = pDoc->mBrushList;

	// if the group isn't >= 0 then they fucked up
	if(pDoc->mCurrentGroup < 0)
		return;

	// check for selected brushes
	while(CurrentBrush ) {
		// unselect the brush after
		if(CurrentBrush->mBrushGroup == pDoc->mCurrentGroup ) {
			CurrentBrush->mFlags |= BRUSH_ENTITY;
			CurrentBrush->mEntityId = mCurrentEntity;
			(*mEntityArray)[mCurrentEntity].mGroup = CurrentBrush->mBrushGroup;
		}
		CurrentBrush = CurrentBrush->GetNextBrush();
	}
	pDoc->UpdateBrushInformation();
}

void CEntitiesDialog::OnDisassociate() 
{
	// go through the brushes
	CThredBrush* CurrentBrush = pDoc->mBrushList;

	// check for selected brushes
	while(CurrentBrush ) {
		// unselect the brush after
		if((CurrentBrush->mFlags & BRUSH_ENTITY) && CurrentBrush->mEntityId == mCurrentEntity ) {
			// get rid of the attached flag
			CurrentBrush->mFlags &= ~BRUSH_ENTITY;
		}
		CurrentBrush = CurrentBrush->GetNextBrush();
	}
	pDoc->UpdateBrushInformation();
}

void CEntitiesDialog::NewEntity(char** ClassnameList)
{
	// get a new entity
	CEntity NewEnt;

	// make sure we have something
	if(!ClassnameList )
		return;

	int CurrentPair = 0;
	while(ClassnameList[CurrentPair][0] != 0) {
		NewEnt.mKeyArray.Add(ClassnameList[CurrentPair] );
		NewEnt.mValueArray.Add(ClassnameList[CurrentPair+1] );
		CurrentPair += 2;
	}

	// make an origin and set our brush type at the same time.
	// if we have no origin it will set us as a brush type.... sneaky eh?
	NewEnt.UpdateOrigin();
	NewEnt.mGroup = pDoc->mCurrentGroup;

	// add the entity onto the end of the array
	(*mEntityArray).Add(NewEnt );

	// set our current entity
	mCurrentEntity = (*mEntityArray).GetSize()-1;

	// update the dialog
	FillInDialog();
}

void CEntitiesDialog::OnChangeMinintensity() 
{
	CString string;

//	((CEdit*)GetDlgItem(IDC_MININTENSITY))->GetLine(0, (LPTSTR)str_val);
   ((CEdit*)GetDlgItem( IDC_MININTENSITY ) )->GetWindowText( string );
   
   float tmp;
   sscanf( LPCTSTR( string ), "%f", &tmp );
   min_intensity = double( tmp );
}

void CEntitiesDialog::OnOK() 
{
	CString strMessage;

    if (min_intensity <= 0.0)
    {
      strMessage.Format("Minimum Intensity Value Must Be > 0");
      AfxMessageBox(strMessage, MB_OK, 0);
	  ((CEdit*)GetDlgItem(IDC_MININTENSITY))->SetFocus();
	  ((CEdit*)GetDlgItem(IDC_MININTENSITY))->SetSel(0, -1, FALSE);
      return;
    }

	CDialog::OnOK();
}

void CEntitiesDialog::OnColorbutton() 
{
	CColorDialog  dialog;

    dialog.m_cc.lStructSize = sizeof(CHOOSECOLOR);
    dialog.m_cc.Flags = CC_FULLOPEN | CC_ENABLEHOOK;
    //dialog.m_cc.hwndOwner = NULL;
    //dialog.m_cc.rgbResult = 0;
    //dialog.m_cc.lpCustColors = NULL;
    //dialog.m_cc.lpfnHook = CCHookProc;
	
    if (dialog.DoModal() == IDOK)
    {
      COLORREF  cval = dialog.GetColor();

      float blue, green, red;

      red = float(cval & 0xFF) / 255.F;
      green = float((cval>>8) & 0xFF) / 255.F;
      blue = float((cval>>16) & 0xFF) / 255.F;

      char  buf[32];
      sprintf(buf, "%f, %f, %f", red, green, blue);
      ((*mEntityArray)[mCurrentEntity].mValueArray[mCurrentKey]).Format("%f, %f, %f", red, green, blue);

      m_ValueEdit.SetWindowText((*mEntityArray)[mCurrentEntity].mValueArray[mCurrentKey]);
      FillInKeyValuePairs(mCurrentEntity);
    }
}

