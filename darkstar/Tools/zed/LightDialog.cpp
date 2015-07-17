// LightDialog.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "LightDialog.h"
#include "BrushGroupDialog.h"
#include "editsun.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static CLightDialog * pLightDialog = NULL;

/////////////////////////////////////////////////////////////////////////////
// CLightDialog dialog


CLightDialog::CLightDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CLightDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLightDialog)
	//}}AFX_DATA_INIT
}


void CLightDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLightDialog)
	DDX_Control(pDX, IDC_LD_COLORPICK, m_colorButton);
	DDX_Control(pDX, IDC_LD_STATESPIN, m_StateSpin);
	DDX_Control(pDX, IDC_LD_POSITIONEDIT, m_PositionEdit);
	DDX_Control(pDX, IDC_LD_DURATIONEDIT, m_DurationEdit);
	DDX_Control(pDX, IDC_LD_DISTANCE3EDIT, m_Distance3Edit);
	DDX_Control(pDX, IDC_LD_DISTANCE2EDIT, m_Distance2Edit);
	DDX_Control(pDX, IDC_LD_DISTANCE1EDIT, m_Distance1Edit);
	DDX_Control(pDX, IDC_LD_DIRECTIONEDIT, m_DirectionEdit);
	DDX_Control(pDX, IDC_LD_ANGLEEDIT, m_AngleEdit);
	DDX_Control(pDX, IDC_ID_STATELIST, m_StateList);
	DDX_Control(pDX, IDC_LD_LIGHTLIST, m_LightList);
	DDX_Control(pDX, IDC_LD_NAMEEDIT, m_NameEdit);
	DDX_Control(pDX, IDC_LD_COLOREDIT, m_ColorEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLightDialog, CDialog)
	//{{AFX_MSG_MAP(CLightDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_LD_COLORPICK, OnLdColorpick)
	ON_BN_CLICKED(IDC_LD_ADDSTATE, OnLdAddstate)
	ON_BN_CLICKED(IDC_LD_AUTOCHECK, OnLdAutocheck)
	ON_BN_CLICKED(IDC_LD_CREATELIGHT, OnLdCreatelight)
	ON_BN_CLICKED(IDC_LD_LINEARRADIO, OnLdLinearradio)
	ON_BN_CLICKED(IDC_LD_LOOPCHECK, OnLdLoopcheck)
	ON_BN_CLICKED(IDC_LD_POINTRADIO, OnLdPointradio)
	ON_BN_CLICKED(IDC_LD_QUADRATICRADIO, OnLdQuadraticradio)
	ON_BN_CLICKED(IDC_LD_RANDOMCHECK, OnLdRandomcheck)
	ON_BN_CLICKED(IDC_LD_REMOVESTATE, OnLdRemovestate)
	ON_BN_CLICKED(IDC_LD_SPOTRADIO, OnLdSpotradio)
	ON_NOTIFY(UDN_DELTAPOS, IDC_LD_STATESPIN, OnDeltaposLdStatespin)
	ON_LBN_SELCHANGE(IDC_LD_LIGHTLIST, OnSelchangeLdLightlist)
	ON_LBN_SELCHANGE(IDC_ID_STATELIST, OnSelchangeIdStatelist)
	ON_BN_CLICKED(IDC_LD_APPLY, OnLdApply)
	ON_EN_CHANGE(IDC_LD_NAMEEDIT, OnChangeLdNameedit)
	ON_BN_CLICKED(IDC_LD_DELETELIGHT, OnLdDeletelight)
	ON_EN_CHANGE(IDC_LD_POSITIONEDIT, OnChangeLdPositionedit)
	ON_EN_CHANGE(IDC_LD_ANGLEEDIT, OnChangeLdAngleedit)
	ON_EN_CHANGE(IDC_LD_COLOREDIT, OnChangeLdColoredit)
	ON_EN_CHANGE(IDC_LD_DIRECTIONEDIT, OnChangeLdDirectionedit)
	ON_EN_CHANGE(IDC_LD_DISTANCE1EDIT, OnChangeLdDistance1edit)
	ON_EN_CHANGE(IDC_LD_DISTANCE2EDIT, OnChangeLdDistance2edit)
	ON_EN_CHANGE(IDC_LD_DISTANCE3EDIT, OnChangeLdDistance3edit)
	ON_EN_CHANGE(IDC_LD_DURATIONEDIT, OnChangeLdDurationedit)
	ON_BN_CLICKED(IDC_LD_NONERADIO, OnLdNoneradio)
	ON_BN_CLICKED(IDC_LD_COPYCURRENT, OnLdCopycurrent)
	ON_BN_CLICKED(IDC_LD_LIGHT, OnLdLight)
	ON_BN_CLICKED(IDC_LD_MANAGEBYMISSIONEDITOR, OnLdManagebymissioneditor)
	ON_BN_CLICKED(IDC_LS_SUNEDIT, OnLsSunedit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CMyColorButton::SetDialog( CLightDialog * pLightDialog )
{
   m_lightDialog = pLightDialog;
}

void CMyColorButton::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
   COLORREF       Color;
   
   if( m_lightDialog->mCurrentLight == -1 || m_lightDialog->mCurrentState == -1 )
      return;
   
   CLight & Light = ( *m_lightDialog->mLightArray )[ m_lightDialog->mCurrentLight ];

   CLightState * pLightState;

   // check if current state is = 0
   if( !m_lightDialog->mCurrentState )
      pLightState = &Light.mDefaultState;
   else
      pLightState = &Light.mStates.GetAt( 
         Light.mStates.FindIndex( m_lightDialog->mCurrentState - 1 ) );

   // get the lightstate   
   CLightState & LightState = *pLightState;
	
   // create the color to set in the dialog
   Color = 0;
   Color |= ( ( 0xff & int( LightState.mColor.blue * 255 ) ) << 16 );
   Color |= ( ( 0xff & int( LightState.mColor.green * 255 ) ) << 8 );
   Color |= ( ( 0xff & int( LightState.mColor.red * 255 ) ) );
   
	// get a cdc
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC );

	// now draw a solid rectangle
	pDC->FillSolidRect(&lpDrawItemStruct->rcItem, Color );

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

int CLightDialog::EditLight( CLightArray& Lights, int CurrentLight, 
   CTHREDDoc* Doc )
{
	pDoc = Doc;
	// the list of entities
   mCurrentLight = CurrentLight;
	mLightArray = &Lights;
	DoModal();
	return( mCurrentLight );
}


BOOL CLightDialog::OnInitDialog() 
{
   // set the ptr for the button
   m_colorButton.SetDialog( this );
   
	CDialog::OnInitDialog();

   pLightDialog = this;
   
   // set the size of the light array
   mSavedLightArray.SetSize( 0, 20 );

   // copy the lights
   int Count = (*mLightArray).GetSize();
   
   // go through and copy all the lights
   for( int Light = 0; Light < Count; Light++ )
   {
      CLight NewLight( FALSE );
      
      // copy the light
      NewLight = (*mLightArray)[ Light ];
      
      // add to the list
      mSavedLightArray.Add( NewLight );
   }
      
   FillDialogInfo();
   
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLightDialog::FillDialogInfo( void )
{
	int         CurrentLight;
	int         NumberOfLights = ( *mLightArray ).GetSize();
	int         RealIndex;
	CString     LightName;
	
	// clear our main stuff
	m_LightList.ResetContent();

   RealIndex = -1;

   // set to first item if nothing is selected
   if( NumberOfLights && ( mCurrentLight == LB_ERR ) )
      mCurrentLight = 0;
      
	// go through the array of entities
	for( CurrentLight = 0; CurrentLight < NumberOfLights; CurrentLight++ )
	{
   	// get the name
      // check if this light is in a light set ( put an asterix? )
      if( ( *mLightArray)[CurrentLight].mFlags & LIGHT_INLIGHTSET )
      {
         if( ( * mLightArray )[ CurrentLight ].mName.GetLength() )
            LightName.Format( "* %s", (*mLightArray)[CurrentLight].mName );
         else
	        LightName.Format( "* Light%d", CurrentLight );
      }
      else
      {
         if( ( * mLightArray )[ CurrentLight ].mName.GetLength() )
            LightName = ( *mLightArray )[ CurrentLight ].mName;
         else
	        LightName.Format( "Light%d", CurrentLight );
      }
            
		// add the name to the entity list
		int Index = m_LightList.AddString( LightName );

		if( CurrentLight == mCurrentLight )
			RealIndex = Index;

		// set the item data to tell us what entity this suck is from
		m_LightList.SetItemData( Index, CurrentLight );
	}

	// set the current selection to what it should be in the listbox
   if( RealIndex >= 0 )
      m_LightList.SetCurSel( RealIndex );

	// if we have no lights set to nothing.
	if( !NumberOfLights )
		mCurrentLight = LB_ERR;
      
   // fill the current info for this light
   FillLightInfo();
}

// fills in all the info for this light
void CLightDialog::FillLightInfo( void )
{
   // clear all the items
   ( ( CButton* )GetDlgItem( IDC_LD_AUTOCHECK ) )->SetCheck( FALSE );
   ( ( CButton* )GetDlgItem( IDC_LD_LOOPCHECK ) )->SetCheck( FALSE );
   ( ( CButton* )GetDlgItem( IDC_LD_RANDOMCHECK ) )->SetCheck( FALSE );
   m_PositionEdit.SetWindowText( "" );
   m_Distance1Edit.SetWindowText( "" );
   m_Distance2Edit.SetWindowText( "" );
   m_Distance3Edit.SetWindowText( "" );
   m_DirectionEdit.SetWindowText( "" );
   m_AngleEdit.SetWindowText( "" );
   m_DurationEdit.SetWindowText( "" );
   m_ColorEdit.SetWindowText( "" );
	m_StateList.ResetContent();
   
   
   bool fEnable = FALSE;
   
   // enable/disable the controls
   if( mCurrentLight == LB_ERR )
      m_NameEdit.SetWindowText( "" );
   else
      fEnable = TRUE;
   
   GetDlgItem(IDC_LD_DELETELIGHT )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_ADDSTATE )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_REMOVESTATE )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_COLORPICK )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_LINEARRADIO )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_QUADRATICRADIO )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_NONERADIO )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_SPOTRADIO )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_POINTRADIO )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_DIRECTIONEDIT )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_NAMEEDIT )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_RANDOMCHECK )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_LOOPCHECK )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_AUTOCHECK )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_STATESPIN )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_ANGLEEDIT )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_LIGHTLIST )->EnableWindow( fEnable );
   GetDlgItem(IDC_ID_STATELIST )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_POSITIONEDIT )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_COLOREDIT )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_DURATIONEDIT )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_DISTANCE1EDIT )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_DISTANCE2EDIT )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_DISTANCE3EDIT )->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_LIGHT )->EnableWindow( fEnable );
   GetDlgItem(IDC_LS_SUNEDIT)->EnableWindow( fEnable );
   GetDlgItem(IDC_LD_MANAGEBYMISSIONEDITOR)->EnableWindow( fEnable );
   
//   GetDlgItem(IDC_LD_ID)->EnableWindow( fEnable );

   if( !fEnable )
      return;
      
   CLight & Light = ( *mLightArray )[ mCurrentLight ];
   
   // go through all the values
   if( Light.mFlags & LIGHT_HASNAME )
   	m_NameEdit.SetWindowText( Light.mName );
   else
      m_NameEdit.SetWindowText( "" );

//   // write out the id
//   m_IDString.Format( "%d", Light.mID );
//   m_ID.SetWindowText( m_IDString );
  
   // checkboxes
   if( Light.mFlags & LIGHT_AUTOSTART )
      ( ( CButton* )GetDlgItem( IDC_LD_AUTOCHECK ) )->SetCheck( TRUE );
   if( Light.mFlags & LIGHT_LOOPTOEND )
      ( ( CButton* )GetDlgItem( IDC_LD_LOOPCHECK ) )->SetCheck( TRUE );
   if( Light.mFlags & LIGHT_RANDOMFLICKER )
      ( ( CButton* ) GetDlgItem( IDC_LD_RANDOMCHECK ) )->SetCheck( TRUE );
   if( Light.mFlags & LIGHT_MANAGEBYMISSIONEDITOR )
      ( ( CButton* ) GetDlgItem( IDC_LD_MANAGEBYMISSIONEDITOR ) )->SetCheck( TRUE );
   
   // get the number of states
   int   NumStates = Light.mStates.GetCount();
   
   mCurrentState = LB_ERR;
   if( NumStates )
   {
      mCurrentState = 0;
      m_StateSpin.SetRange( 0, NumStates - 1 );
   }
   m_StateSpin.InvalidateRgn(NULL);
   m_StateSpin.UpdateWindow();
      
   // loop through the states and set the data
   for( int State = 0; State <= NumStates; State++ )
   {
      CString  String;
      
      if( State )
         String.Format( "State%d", State );
      else
         // create the special 'default' state
         String.Format( "- DEFAULT -", State );
         
		int Index = m_StateList.AddString( String );
		m_StateList.SetItemData( Index, State );
	}
   
   // fill in the data for state 0
   FillStateInfo();
   
   // reset all lights
   pDoc->ResetSelectedLights();
   
   // set this light as selected
   Light.mFlags |= LIGHT_SELECTED;
   pDoc->mCurrentLight = mCurrentLight;
   pDoc->mSelectedLight = TRUE;
   
   // apply the view
   OnLdApply();   
}

// fill in the state information
void CLightDialog::FillStateInfo( void )
{
   if( mCurrentLight == LB_ERR || mCurrentState == LB_ERR )
      return;

   CLight & Light = ( *mLightArray )[ mCurrentLight ];
   
   CLightState * pLightState;
   
   // check if current state is = 0
   if( !mCurrentState )
      pLightState = &Light.mDefaultState;
   else
      pLightState = &Light.mStates.GetAt( Light.mStates.FindIndex( mCurrentState - 1 ) );

   // get the lightstate   
   CLightState & LightState = *pLightState;

   // set as current state
   m_StateList.SetCurSel( mCurrentState );         
   
   // set the spinners position      
   m_StateSpin.SetPos( mCurrentState );

   if( LightState.mFlags & STATE_HASPOSITION )
   {
      m_PositionString.Format( "%d %d %d", int( LightState.mPosition.X ), 
         int( LightState.mPosition.Y ),
         int( LightState.mPosition.Z ) );
   }
   else
      m_PositionString = "";
   m_PositionEdit.SetWindowText( m_PositionString );
   
   if( LightState.mFlags & STATE_HASDURATION )
      m_DurationString.Format( "%f", float( LightState.mDuration ) );
   else
      m_DurationString = "";
   m_DurationEdit.SetWindowText( m_DurationString );
   
   if( LightState.mFlags & STATE_HASCOLOR )
      m_ColorString.Format( "%f %f %f", LightState.mColor.red, 
         LightState.mColor.green, LightState.mColor.blue );
   else
      m_ColorString = "";
   m_ColorEdit.SetWindowText( m_ColorString );

   if( LightState.mFlags & STATE_LINEAR_FALLOFF )
   {
      ( ( CButton* )GetDlgItem( IDC_LD_LINEARRADIO ) )->SetCheck( TRUE );
      ( ( CButton* )GetDlgItem( IDC_LD_QUADRATICRADIO ) )->SetCheck( FALSE );
      ( ( CButton* )GetDlgItem( IDC_LD_NONERADIO ) )->SetCheck( FALSE );
      m_Distance1String.Format( "%d", LightState.mFalloff1 );
      m_Distance1Edit.SetWindowText( m_Distance1String );
      m_Distance2String.Format( "%d", LightState.mFalloff2 );
      m_Distance2Edit.SetWindowText( m_Distance2String );
      m_Distance3Edit.SetWindowText( "" );
      m_Distance1Edit.EnableWindow( TRUE );
      m_Distance2Edit.EnableWindow( TRUE );
      m_Distance3Edit.EnableWindow( FALSE );
   }
   else if( LightState.mFlags & STATE_DISTANCE_FALLOFF )
   {
      ( ( CButton* )GetDlgItem( IDC_LD_LINEARRADIO ) )->SetCheck( FALSE );
      ( ( CButton* )GetDlgItem( IDC_LD_QUADRATICRADIO ) )->SetCheck( TRUE );
      ( ( CButton* )GetDlgItem( IDC_LD_NONERADIO ) )->SetCheck( FALSE );
      m_Distance1String.Format( "%d", LightState.mFalloff1 );
      m_Distance1Edit.SetWindowText( m_Distance1String );
      m_Distance2String.Format( "%d", LightState.mFalloff2 );
      m_Distance2Edit.SetWindowText( m_Distance2String );
      m_Distance3String.Format( "%d", LightState.mFalloff3 );
      m_Distance3Edit.SetWindowText( m_Distance3String );
      m_Distance1Edit.EnableWindow( TRUE );
      m_Distance2Edit.EnableWindow( TRUE );
      m_Distance3Edit.EnableWindow( TRUE );
   }
   else
   {
      ( ( CButton* )GetDlgItem( IDC_LD_LINEARRADIO ) )->SetCheck( FALSE );
      ( ( CButton* )GetDlgItem( IDC_LD_QUADRATICRADIO ) )->SetCheck( FALSE );
      ( ( CButton* )GetDlgItem( IDC_LD_NONERADIO ) )->SetCheck( TRUE );
      m_Distance1Edit.EnableWindow( FALSE );
      m_Distance2Edit.EnableWindow( FALSE );
      m_Distance3Edit.EnableWindow( FALSE );
   }
   if( LightState.mFlags & STATE_HASSPOTFACTOR )
   {
      ( ( CButton* )GetDlgItem( IDC_LD_SPOTRADIO ) )->SetCheck( TRUE );
      ( ( CButton* )GetDlgItem( IDC_LD_POINTRADIO ) )->SetCheck( FALSE );
      m_DirectionString.Format( "%f %f %f", LightState.mSpotVector.X,
         LightState.mSpotVector.Y, LightState.mSpotVector.Z );
      m_DirectionEdit.SetWindowText( m_DirectionString );
      m_AngleString.Format( "%f %f", LightState.mSpotTheta, LightState.mSpotPhi );
      m_AngleEdit.SetWindowText( m_AngleString );
      m_DirectionEdit.EnableWindow( TRUE );
      m_AngleEdit.EnableWindow( TRUE );
   }
   else
   {
      ( ( CButton* )GetDlgItem( IDC_LD_SPOTRADIO ) )->SetCheck( FALSE );
      ( ( CButton* )GetDlgItem( IDC_LD_POINTRADIO ) )->SetCheck( TRUE );
      m_DirectionEdit.SetWindowText( "" );
      m_AngleEdit.SetWindowText( "" );
      m_DirectionEdit.EnableWindow( FALSE );
      m_AngleEdit.EnableWindow( FALSE );
   }
   
   m_colorButton.RedrawWindow();
}      

// updates the info in the state portion of the dialog, prompts
// when info is formated incorrectly
bool CLightDialog::UpdateStateInfo( void )
{
   if( mCurrentLight == LB_ERR || mCurrentState == LB_ERR )
      return( FALSE );
      
   // get the light and the state
   CLight & Light = ( *mLightArray )[ mCurrentLight ];
   
   CLightState * pLightState;
   
   // check if current state is = 0
   if( !mCurrentState )
      pLightState = &Light.mDefaultState;
   else
      pLightState = &Light.mStates.GetAt( Light.mStates.FindIndex( mCurrentState - 1 ) );

   // get the lightstate   
   CLightState & LightState = *pLightState;

   if( !m_PositionString.GetLength() )
   {
      // check if first light, if so must get position from the default
      if( mCurrentState == 1 )
      {
         // check if the default has a position
         if( !( Light.mDefaultState.mFlags & STATE_HASPOSITION ) )
         {
            AfxMessageBox( "Position must be set in either the default or first states.", 
               MB_OK, 0 );
            return( FALSE );
         }
         
         // set the position, for use by the renderer only ( will not
         // be exported )
         LightState.mPosition = Light.mDefaultState.mPosition;
      }
         
      LightState.mFlags &= ~STATE_HASPOSITION;
   }
   else
   {
      int tmp1, tmp2, tmp3;
   
      // get position
      if( ( sscanf( LPCTSTR( m_PositionString ), "%d %d %d", 
         &tmp1, &tmp2, &tmp3 ) ) != 3 )
      {
         AfxMessageBox( "Failed to set Position", MB_OK, 0 );
         return( FALSE );
      }
   
      LightState.mPosition.X = float( tmp1 );
      LightState.mPosition.Y = float( tmp2 );
      LightState.mPosition.Z = float( tmp3 );

      // set the flag 
      LightState.mFlags |= STATE_HASPOSITION;
   }
         
   // set the position of the light if this is state 0 ( 1 because of default)
   if( mCurrentState == 1 )
      Light.mCurrentPosition = Light.mMovePosition = LightState.mPosition;
      
   if( !m_DurationString.GetLength() )
   {
      LightState.mFlags &= ~STATE_HASDURATION;
   }
   else
   {
      float tmp;
      
      // get duration
      if( ( sscanf( LPCTSTR( m_DurationString ), "%f", &tmp ) ) != 1 )
      {
         AfxMessageBox( "Failed to set Duration", MB_OK, 0 );
         return( FALSE );
      }

      LightState.mDuration = double( tmp );
      // set flag
      LightState.mFlags |= STATE_HASDURATION;
   }

   if( !m_ColorString.GetLength() )
   {
      LightState.mFlags &= ~STATE_HASCOLOR;
   }
   else
   {
      // color
      if( ( sscanf( LPCTSTR( m_ColorString ), "%f %f %f",
         &LightState.mColor.red, &LightState.mColor.green,
         &LightState.mColor.blue ) ) != 3 )
      {
         AfxMessageBox( "Failed to set Color", MB_OK, 0 );
         return( FALSE );
      }
      
      // set flag
      LightState.mFlags |= STATE_HASCOLOR;
   }
   

   // check for linear falloff values
   if( LightState.mFlags & STATE_LINEAR_FALLOFF )
   {
      if( ( sscanf( LPCTSTR( m_Distance1String ), "%d", 
         &LightState.mFalloff1 ) ) != 1 )
      {
         AfxMessageBox( "Failed to set Distance1", MB_OK, 0 );
         return( FALSE );
      }

      // get distance 2
      if( ( sscanf( LPCTSTR( m_Distance2String ), "%d", 
         &LightState.mFalloff2 ) ) != 1 )
      {
         AfxMessageBox( "Failed to set Distance2", MB_OK, 0 );
         return( FALSE );
      }
   }
   
   // check if should get distance 3
   if( LightState.mFlags & STATE_DISTANCE_FALLOFF )
   {
      if( ( sscanf( LPCTSTR( m_Distance1String ), "%d", 
         &LightState.mFalloff1 ) ) != 1 )
      {
         AfxMessageBox( "Failed to set Distance1", MB_OK, 0 );
         return( FALSE );
      }

      // get distance 2
      if( ( sscanf( LPCTSTR( m_Distance2String ), "%d", 
         &LightState.mFalloff2 ) ) != 1 )
      {
         AfxMessageBox( "Failed to set Distance2", MB_OK, 0 );
         return( FALSE );
      }

      // get distance 3
      if( ( sscanf( LPCTSTR( m_Distance3String ), "%d", 
         &LightState.mFalloff3 ) ) != 1 )
      {
         AfxMessageBox( "Failed to set Distance3", MB_OK, 0 );
         return( FALSE );
      }
   }
   
   // check for spotlight
   if( LightState.mFlags & STATE_HASSPOTFACTOR )
   {
      float tmp1, tmp2, tmp3;
      
      // get direction
      if( ( sscanf( LPCTSTR( m_DirectionString ), "%f %f %f", 
         &tmp1, &tmp2, &tmp3 ) ) != 3 )
      {
         AfxMessageBox( "Failed to set SpotVector", MB_OK, 0 );
         return( FALSE );
      }
        
      LightState.mSpotVector.X = float( tmp1 );
      LightState.mSpotVector.Y = float( tmp2 );
      LightState.mSpotVector.Z = float( tmp3 );
      
      // get angles
      if( ( sscanf( LPCTSTR( m_AngleString ), "%f %f", 
         &tmp1, &tmp2 ) ) != 2 )
      {
         AfxMessageBox( "Failed to set Angle", MB_OK, 0 );
         return( FALSE );
      }
      
      LightState.mSpotTheta = float( tmp1 );
      LightState.mSpotPhi = float( tmp2 );
   }
   
   // return success
   return( TRUE );
}

void CLightDialog::OnLdColorpick() 
{
   CColorDialog  Dialog;
   COLORREF       Color;
   
   Dialog.m_cc.lStructSize = sizeof(CHOOSECOLOR);
   Dialog.m_cc.Flags = CC_FULLOPEN | CC_ENABLEHOOK;

   CLight & Light = ( *mLightArray )[ mCurrentLight ];

   CLightState * pLightState;

   // check if current state is = 0
   if( !mCurrentState )
      pLightState = &Light.mDefaultState;
   else
      pLightState = &Light.mStates.GetAt( Light.mStates.FindIndex( mCurrentState - 1 ) );

   // get the lightstate   
   CLightState & LightState = *pLightState;
	
   // create the color to set in the dialog
   Color = 0;
   Color |= ( ( 0xff & int( LightState.mColor.blue * 255 ) ) << 16 );
   Color |= ( ( 0xff & int( LightState.mColor.green * 255 ) ) << 8 );
   Color |= ( ( 0xff & int( LightState.mColor.red * 255 ) ) );
   Dialog.m_cc.rgbResult = Color;
   Dialog.m_cc.Flags |= CC_RGBINIT;
   
   // create a color dialog
   if( Dialog.DoModal() == IDOK)
   {
      Color = Dialog.GetColor();
      
      float blue, green, red;

      red = float( Color & 0xFF) / 255.F;
      green = float( ( Color >> 8 ) & 0xFF ) / 255.F;
      blue = float( ( Color >> 16 ) & 0xFF ) / 255.F;

      LightState.mColor.blue = blue;
      LightState.mColor.red = red;
      LightState.mColor.green = green;
      
      m_ColorString.Format( "%f %f %f", red, green, blue );
      m_ColorEdit.SetWindowText( m_ColorString );
   }

   m_colorButton.RedrawWindow();
   
   UpdateStateInfo();
}

void CLightDialog::OnLdAddstate() 
{
   CLightState NewLightState;
   int         NewState;

   if( mCurrentState == LB_ERR || mCurrentLight == LB_ERR )
      return;
   
   if( !UpdateStateInfo() )
      return;

   CLight & Light = ( *mLightArray )[ mCurrentLight ];
   
   CLightState * pLightState;
   
   // check if current state is = 0
   if( !mCurrentState )
      mCurrentState = 1;

   // get a ptr to the lighstate
   pLightState = &Light.mStates.GetAt( Light.mStates.FindIndex( mCurrentState - 1 ) );

   // get the lightstate   
   CLightState & LightState = *pLightState;
  
   // reset the caps flags
   NewLightState.mFlags = 0; 

//   get none
//   if( LightState.mFlags & STATE_LINEAR_FALLOFF )
//      NewLightState.mFlags = STATE_LINEAR_FALLOFF;
//   if( LightState.mFlags & STATE_DISTANCE_FALLOFF )
//      NewLightState.mFlags = STATE_DISTANCE_FALLOFF;
//   NewLightState.mFalloff1 = LightState.mFalloff1;
//   NewLightState.mFalloff2 = LightState.mFalloff2;
//   NewLightState.mFalloff3 = LightState.mFalloff3;
         
   // add to the list
   Light.mStates.InsertAfter( Light.mStates.FindIndex( mCurrentState - 1 ), 
      NewLightState );
   
   NewState = mCurrentState;
   
   FillLightInfo();
   
   // get this next one
   mCurrentState = NewState;
   m_StateList.SetCurSel( mCurrentState + 1 );
   
   FillStateInfo();
}

void CLightDialog::OnLdRemovestate() 
{
   int      NewState;
   
   if( !mCurrentState || mCurrentState == LB_ERR || mCurrentLight == LB_ERR )
      return;

   // get the light class      
   CLight & Light = ( *mLightArray )[ mCurrentLight ];
      
   // remove the light
   Light.mStates.RemoveAt( Light.mStates.FindIndex( mCurrentState - 1) );
   
   // update the current position
   if( mCurrentState == 1 )
   {
      if( !Light.mStates.GetCount() )
      {
         OnLdDeletelight();
         return;
      }
   }
   else
      mCurrentState--;
   
   NewState = mCurrentState;
   FillLightInfo();
   mCurrentState = NewState;
   FillStateInfo();
}


void CLightDialog::OnLdAutocheck()
{
   if( mCurrentLight == LB_ERR )
      return;
      
   CLight & Light = ( *mLightArray )[ mCurrentLight ];

   if( ( ( CButton* )GetDlgItem( IDC_LD_AUTOCHECK ) )->GetState() & 0x01 )
      Light.mFlags |= LIGHT_AUTOSTART;
   else
      Light.mFlags &= ~LIGHT_AUTOSTART;
}

void CLightDialog::OnLdCreatelight() 
{
   // update the state info first
   if( mCurrentLight != LB_ERR )
   {
      if( !UpdateStateInfo() )
         return;
   }
   else
      mCurrentLight = 0;
      
   CLight NewLight;
   
   // check if there is an active lightset and set the flag accordingly
   if( pDoc->pActiveDetail->pActiveLightSet )
      NewLight.mFlags |= LIGHT_INLIGHTSET;
   else
      NewLight.mFlags &= ~LIGHT_INLIGHTSET;
   
   // add at the current light
   (*mLightArray).InsertAt( mCurrentLight, NewLight );

   // update the info
   FillDialogInfo();   
}

void CLightDialog::OnLdDeletelight() 
{
   if( mCurrentLight == LB_ERR )
      return;
      
   (*mLightArray).RemoveAt( mCurrentLight );
   if( mCurrentLight )
      mCurrentLight--;
   else
      if( (*mLightArray).GetSize() )
         mCurrentLight = 0;
      else
         mCurrentLight = LB_ERR;
         
   // refil the dialog info
   FillDialogInfo();
}

void CLightDialog::OnLdLoopcheck() 
{
   if( mCurrentLight == LB_ERR )
      return;
      
   CLight & Light = ( *mLightArray )[ mCurrentLight ];

   if( ( ( CButton* )GetDlgItem( IDC_LD_LOOPCHECK ) )->GetState() & 0x01 )
      Light.mFlags |= LIGHT_LOOPTOEND;
   else
      Light.mFlags &= ~LIGHT_LOOPTOEND;
}

void CLightDialog::OnLdSpotradio() 
{
   CLight & Light = ( *mLightArray )[ mCurrentLight ];

   CLightState * pLightState;

   // check if current state is = 0
   if( !mCurrentState )
      pLightState = &Light.mDefaultState;
   else
      pLightState = &Light.mStates.GetAt( Light.mStates.FindIndex( mCurrentState - 1 ) );

   // get the lightstate   
   CLightState & LightState = *pLightState;
   
   // set the flag
   LightState.mFlags |= STATE_HASSPOTFACTOR;
   
   // enable the windows
   GetDlgItem(IDC_LD_ANGLEEDIT )->EnableWindow( TRUE );
   GetDlgItem(IDC_LD_DIRECTIONEDIT )->EnableWindow( TRUE );
}

void CLightDialog::OnLdPointradio() 
{
   CLight & Light = ( *mLightArray )[ mCurrentLight ];

   CLightState * pLightState;
   
   // check if current state is = 0
   if( !mCurrentState )
      pLightState = &Light.mDefaultState;
   else
      pLightState = &Light.mStates.GetAt( Light.mStates.FindIndex( mCurrentState - 1 ) );

   // get the lightstate   
   CLightState & LightState = *pLightState;
   
   // reset the flag
   LightState.mFlags &= ~STATE_HASSPOTFACTOR;
   
   // un-enable the windows
   GetDlgItem(IDC_LD_ANGLEEDIT )->EnableWindow( FALSE );
   GetDlgItem(IDC_LD_DIRECTIONEDIT )->EnableWindow( FALSE );
}

void CLightDialog::OnLdRandomcheck() 
{
   if( mCurrentLight == LB_ERR )
      return;
      
   CLight & Light = ( *mLightArray )[ mCurrentLight ];

   if( ( ( CButton* )GetDlgItem( IDC_LD_RANDOMCHECK ) )->GetState() & 0x01 )
      Light.mFlags |= LIGHT_RANDOMFLICKER;
   else
      Light.mFlags &= ~LIGHT_RANDOMFLICKER;
}

void CLightDialog::OnDeltaposLdStatespin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
   
   CLight & Light = ( *mLightArray )[ mCurrentLight ];
   CLightState    TmpLightState;

   CLightState * pLightState;
   
   // check if current state is = 0
   if( !mCurrentState )
      return;
   else
      pLightState = &Light.mStates.GetAt( Light.mStates.FindIndex( mCurrentState - 1 ) );

   // get the lightstate   
   CLightState & LightStateA = *pLightState;
   
   if( !UpdateStateInfo() )
      return;

   int NumStates = Light.mStates.GetCount();
   
   // lets only adjust by one shall we
   if( pNMUpDown->iDelta < 0 )
      mCurrentState++;
   else if( pNMUpDown->iDelta > 0 )
      mCurrentState--;
   
   // clip
   if( !mCurrentState )
   {  
      mCurrentState++;
      return;
   }
   if( mCurrentState >= NumStates )
      mCurrentState = NumStates;

   CLightState & LightStateB = 
         Light.mStates.GetAt( Light.mStates.FindIndex( mCurrentState - 1 ) );
      
   // swap em
   TmpLightState = LightStateA;
   LightStateA = LightStateB;
   LightStateB = TmpLightState;
   
   FillStateInfo();   
   
   // update	
	*pResult = TRUE;
}

void CLightDialog::OnSelchangeLdLightlist() 
{
	int CurrentSelection = m_LightList.GetCurSel();

   // get the selection
	if( CurrentSelection == LB_ERR )
		mCurrentLight = LB_ERR;
	else 
   {
      if( !UpdateStateInfo() )
      {
         m_LightList.SetCurSel( mCurrentLight );
         return;
      }
      else
   		mCurrentLight = m_LightList.GetItemData( CurrentSelection );
   }

   // fill in the info
   FillLightInfo();	
}

void CLightDialog::OnSelchangeIdStatelist() 
{
	int CurrentSelection = m_StateList.GetCurSel();

   if( CurrentSelection != LB_ERR )
      if( !UpdateStateInfo() )
      {
         m_StateList.SetCurSel( mCurrentState );
         return;
      }
      
   // get the selection
	if( CurrentSelection == LB_ERR )
		mCurrentState = LB_ERR;
	else 
		mCurrentState = m_StateList.GetItemData( CurrentSelection );

   // fill in the info
   FillStateInfo();
}

void CLightDialog::OnChangeLdNameedit() 
{
   CString String;
   
   if( mCurrentLight == LB_ERR )
      return;
      
   // get the text      
   m_NameEdit.GetWindowText( String );

   CLight & Light = ( *mLightArray )[ mCurrentLight ];

   // check length ( 0 will go to default )   
   if( String.GetLength() )
   {
      Light.mFlags |= LIGHT_HASNAME;   
      Light.mName = String;   
   }  
   else
   {
      String.Format( "Light%d", mCurrentLight );
      Light.mFlags &= ~LIGHT_HASNAME;
      Light.mName = "";   
   }
   
   // check if in a light set or not
   if( Light.mFlags & LIGHT_INLIGHTSET )
   {
      CString Tmp = String;
      String.Format( "* %s", Tmp );
   }
   
   // get the current light
   m_LightList.InsertString( mCurrentLight + 1, String );
   m_LightList.SetItemData( mCurrentLight + 1, mCurrentLight );
   
   // remove the old one
   m_LightList.DeleteString( mCurrentLight );
   
   // set the current one
   m_LightList.SetCurSel( mCurrentLight );
}

void CLightDialog::OnLdApply() 
{
   if( !UpdateStateInfo() )
      return;

   // remove all the items from the saved light array
   mSavedLightArray.RemoveAll();
   
   // copy the lights
   int Count = (*mLightArray).GetSize();
   
   // go through and copy all the lights
   for( int Light = 0; Light < Count; Light++ )
   {
      CLight NewLight( FALSE );
      
      // copy the light
      NewLight = (*mLightArray)[ Light ];
      
      // add to the list
      mSavedLightArray.Add( NewLight );
   }
   // update the positions on the display
	pDoc->SetModifiedFlag();
	pDoc->UpdateBrushInformation();
	pDoc->UpdateAllViews(NULL);
}

void CLightDialog::OnClose() 
{
   // remove all the elements from the array
   mSavedLightArray.RemoveAll();
   
   pLightDialog = NULL;
   
	CDialog::OnClose();
}

void CLightDialog::OnChangeLdPositionedit() 
{
   // get the text      
   m_PositionEdit.GetWindowText( m_PositionString );
}

void CLightDialog::OnChangeLdAngleedit() 
{
   // get the text      
   m_AngleEdit.GetWindowText( m_AngleString );
}

void CLightDialog::OnChangeLdColoredit() 
{
   // get the text      
   m_ColorEdit.GetWindowText( m_ColorString );
}

void CLightDialog::OnChangeLdDirectionedit() 
{
   // get the text      
   m_DirectionEdit.GetWindowText( m_DirectionString );
}

void CLightDialog::OnChangeLdDistance1edit() 
{
   // get the text      
   m_Distance1Edit.GetWindowText( m_Distance1String );
}

void CLightDialog::OnChangeLdDistance2edit() 
{
   // get the text      
   m_Distance2Edit.GetWindowText( m_Distance2String );
}

void CLightDialog::OnChangeLdDistance3edit() 
{
   // get the text      
   m_Distance3Edit.GetWindowText( m_Distance3String );
}

void CLightDialog::OnChangeLdDurationedit() 
{
   // get the text      
   m_DurationEdit.GetWindowText( m_DurationString );
}

void CLightDialog::OnOK() 
{
   // call the apply
   OnLdApply();
	
	CDialog::OnOK();
}

void CLightDialog::OnCancel() 
{
   // remove all the items from old array
   (*mLightArray).RemoveAll();
   
   // copy the lights
   int Count = mSavedLightArray.GetSize();
   
   // go through and copy all the lights
   for( int Light = 0; Light < Count; Light++ )
   {
      CLight NewLight( FALSE );
      
      // copy the light
      NewLight = mSavedLightArray[ Light ];
      
      // add to the list
      (*mLightArray).Add( NewLight );
   }
   
	CDialog::OnCancel();
}

void CLightDialog::OnLdNoneradio() 
{
   CLight & Light = ( *mLightArray )[ mCurrentLight ];

   CLightState * pLightState;
   
   // check if current state is = 0
   if( !mCurrentState )
      pLightState = &Light.mDefaultState;
   else
      pLightState = &Light.mStates.GetAt( Light.mStates.FindIndex( mCurrentState - 1 ) );

   // get the lightstate   
   CLightState & LightState = *pLightState;
   
   // reset the flags
   LightState.mFlags &= ~STATE_DISTANCE_FALLOFF;
   LightState.mFlags &= ~STATE_LINEAR_FALLOFF;
   
   // disable the third choice
   GetDlgItem(IDC_LD_DISTANCE1EDIT )->EnableWindow( FALSE );
   GetDlgItem(IDC_LD_DISTANCE2EDIT )->EnableWindow( FALSE );
   GetDlgItem(IDC_LD_DISTANCE3EDIT )->EnableWindow( FALSE );
}

void CLightDialog::OnLdLinearradio() 
{
   CLight & Light = ( *mLightArray )[ mCurrentLight ];

   CLightState * pLightState;
   
   // check if current state is = 0
   if( !mCurrentState )
      pLightState = &Light.mDefaultState;
   else
      pLightState = &Light.mStates.GetAt( Light.mStates.FindIndex( mCurrentState - 1 ) );

   // get the lightstate   
   CLightState & LightState = *pLightState;
   
   // reset the flag
   LightState.mFlags &= ~STATE_DISTANCE_FALLOFF;
   LightState.mFlags |= STATE_LINEAR_FALLOFF;
   
   // disable the third choice
   GetDlgItem(IDC_LD_DISTANCE1EDIT )->EnableWindow( TRUE );
   GetDlgItem(IDC_LD_DISTANCE2EDIT )->EnableWindow( TRUE );
   GetDlgItem(IDC_LD_DISTANCE3EDIT )->EnableWindow( FALSE );
}

void CLightDialog::OnLdQuadraticradio() 
{
   CLight & Light = ( *mLightArray )[ mCurrentLight ];

   CLightState * pLightState;
   
   // check if current state is = 0
   if( !mCurrentState )
      pLightState = &Light.mDefaultState;
   else
      pLightState = &Light.mStates.GetAt( Light.mStates.FindIndex( mCurrentState - 1 ) );

   // get the lightstate   
   CLightState & LightState = *pLightState;
   
   // reset the flag
   LightState.mFlags |= STATE_DISTANCE_FALLOFF;
   LightState.mFlags &= ~STATE_LINEAR_FALLOFF;
   
   // enable the third choice
   GetDlgItem(IDC_LD_DISTANCE1EDIT )->EnableWindow( TRUE );
   GetDlgItem(IDC_LD_DISTANCE2EDIT )->EnableWindow( TRUE );
   GetDlgItem(IDC_LD_DISTANCE3EDIT )->EnableWindow( TRUE );
}

// override the dropped function for the draglist box ( normally it will
// just move the item, but we need to rename them when moved and so forth )
void CMyDragListBox::Dropped( int Source, CPoint DropPoint )
{
   if( !pLightDialog )
      return;
      
   // remove the gray insert bar thingy
   this->DrawInsert( -1 );
   
   // get the item we want to insert at, return if bad or eqaul to source
   int Target = this->ItemFromPt( DropPoint );
   if( ( Target == LB_ERR ) || ( Target == Source ) )
      return;

   // insert the light
   CLight Tmp( FALSE );
   Tmp = ( *pLightDialog->mLightArray )[ Source ];
   (*pLightDialog->mLightArray).InsertAt( Target, Tmp );
   
   // check if need to adjust the source index
   if( Target < Source )
      Source++;
      
   // remove this light
   (*pLightDialog->mLightArray).RemoveAt( Source );
    
   // set the current light
   pLightDialog->mCurrentLight = Target;
   
   // update 
   pLightDialog->FillDialogInfo();   
}

// copy the current state and add to this light
void CLightDialog::OnLdCopycurrent() 
{
   CLightState NewLightState;
   int         NewState;

   if( mCurrentState == LB_ERR || mCurrentLight == LB_ERR )
      return;
   
   if( !UpdateStateInfo() )
      return;

   CLight & Light = ( *mLightArray )[ mCurrentLight ];
   
   CLightState * pLightState;
   
   // check if current state is = 0
   if( !mCurrentState )
      pLightState = &Light.mDefaultState;
   else
      pLightState = &Light.mStates.GetAt( Light.mStates.FindIndex( mCurrentState - 1 ) );

   // get the lightstate   
   CLightState & LightState = *pLightState;
  
   //  get the info
   NewLightState = LightState;
   
   // add to the list ( figure out where - which depends on what is sel. )
   if( !mCurrentState )
      Light.mStates.AddHead( NewLightState );
   else
      Light.mStates.InsertAfter( Light.mStates.FindIndex( mCurrentState - 1 ), 
         NewLightState );
   
   NewState = mCurrentState;
   
   FillLightInfo();
   
   // get this next one
   mCurrentState = NewState;
   m_StateList.SetCurSel( mCurrentState + 1 );
   
   FillStateInfo();
}

// generate the light map info
void CLightDialog::OnLdLight() 
{
   if( ( mCurrentState != LB_ERR ) && UpdateStateInfo() )
      pDoc->Computelighting( mCurrentState - 1 );
}

//void CLightDialog::OnChangeLdId() 
//{
//   Int32 tmp;
//   
//   // get the text      
//   m_ID.GetWindowText( m_IDString );
//
//   CLight & Light = ( *mLightArray )[ mCurrentLight ];
//
//   // convert
//   if( ( sscanf( LPCTSTR( m_IDString ), "%d", &tmp ) ) != 1 )
//      return;
//      
//   Light.mID = tmp;
//}

void CLightDialog::OnLdManagebymissioneditor() 
{
   if( mCurrentLight == LB_ERR )
      return;
      
   CLight & Light = ( *mLightArray )[ mCurrentLight ];

   if( ( ( CButton* )GetDlgItem( IDC_LD_MANAGEBYMISSIONEDITOR ) )->GetState() & 0x01 )
      Light.mFlags |= LIGHT_MANAGEBYMISSIONEDITOR;
   else
      Light.mFlags &= ~LIGHT_MANAGEBYMISSIONEDITOR;
}

void CLightDialog::OnLsSunedit() 
{
   if( mCurrentLight == LB_ERR || mCurrentState == LB_ERR )
      return;
      
   EditSun sun;
   if( sun.DoModal() == IDOK )
   {
      // get the light and the state
      CLight & Light = ( *mLightArray )[ mCurrentLight ];
   
      CLightState * pLightState;
   
      pLightState = &Light.mDefaultState;
      
      SetSunValues( pLightState, &sun.Vector );
      
      pLightState = &Light.mStates.GetAt( Light.mStates.FindIndex( 0 ) );

      SetSunValues( pLightState, &sun.Vector );
   }
}

void CLightDialog::SetSunValues( CLightState * pLightState, Point3F * pVector )
{
   // set the falloff flag
   pLightState->mFlags &= ~STATE_DISTANCE_FALLOFF;
   pLightState->mFlags |= STATE_LINEAR_FALLOFF;
   
   // set the position flag
   pLightState->mFlags |= STATE_HASPOSITION;
   
   // set the falloff distances big
   pLightState->mFalloff1 = 60000;
   pLightState->mFalloff2 = 60000;

   // get largest componant
   float   max_value;
   ( fabs( pVector->x ) > fabs( pVector->y ) ) ? max_value = fabs( pVector->x ) : 
      max_value = fabs( pVector->y );
   ( fabs( pVector->z ) > max_value ) ? max_value = fabs( pVector->z ) : max_value = max_value;
   
   pVector->x = ( pVector->x * 30000 ) / max_value;
   pVector->y = ( pVector->y * 30000 ) / max_value;
   pVector->z = ( pVector->z * 30000 ) / max_value;
   
   pLightState->mPosition.X = pVector->x;
   pLightState->mPosition.Y = pVector->y;
   pLightState->mPosition.Z = pVector->z;
   
   FillStateInfo();
   UpdateStateInfo();
}
