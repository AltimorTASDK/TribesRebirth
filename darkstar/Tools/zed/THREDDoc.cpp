// THREDDoc.cpp : implementation of the CTHREDDoc class
//

#include "stdafx.h"
#include "THRED.h"

#include "MainFrm.h"
#include "THREDDoc.h"
#include "THREDView.h"
#include "BrushGroupDialog.h"
#include "gridsizedialog.h"
#include "entitiesdialog.h"
#include "thredparser.h"
#include "NewFDlg.h"
#include "AsPalMat.h"
#include "ts_light.h"
#include "TSDialog.h"
#include "LightDlg.h"
#include "SClrDlg.h"
#include "BOrdrDlg.h"
//john #include "lightdialog.h"
#include "ITRBuildOpt.h"
#include "ITRShapeOpt.h"
#include "ITRLightOpt.h"
#include "DetailSettings.h"
#include "progressbar.h"
#include "materialproperties.h"
#include "zedperslight.h"
#include "filename.h"

//#include "ts_res.h" // for ResourceTypeTSMaterialList;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static  int   GotoFour = 0;

/////////////////////////////////////////////////////////////////////////////
// CTHREDDoc

//IMPLEMENT_DYNCREATE(CTHREDDoc, CDocument)
IMPLEMENT_SERIAL(CTHREDDoc, CDocument, 0);


BEGIN_MESSAGE_MAP(CTHREDDoc, CDocument)
	//{{AFX_MSG_MAP(CTHREDDoc)
	ON_COMMAND(ID_TOOLS_BRUSH_ATTRIBUTES, OnToolsBrushAttributes)
	ON_COMMAND(ID_BRUSH_ADDTOWORLD, OnBrushAddtoworld)
	ON_COMMAND(ID_BRUSH_SUBTRACTFROMWORLD, OnBrushSubtractfromworld)
	ON_COMMAND(ID_BRUSH_PRIMITIVES_CUBE, OnBrushPrimitivesCube)
	ON_COMMAND(ID_BRUSH_PRIMITIVES_SPHEROID, OnBrushPrimitivesSpheroid)
	ON_COMMAND(ID_BRUSH_PRIMITIVES_CYLINDER, OnBrushPrimitivesCylinder)
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
	ON_COMMAND(ID_TOOLS_USEGRID, OnToolsUsegrid)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_USEGRID, OnUpdateToolsUsegrid)
	ON_COMMAND(ID_TOOLS_GRIDSETTINGS, OnToolsGridsettings)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_TOOLS_BSPMANAGER, OnToolsBspmanager)
	ON_COMMAND(ID_ENTITIES_EDITOR, OnEntitiesEditor)
	ON_COMMAND(ID_ENTITIES_PLACE, OnEntitiesPlace)
	ON_COMMAND(ID_ENTITIES_SHOW, OnEntitiesShow)
	ON_UPDATE_COMMAND_UI(ID_ENTITIES_SHOW, OnUpdateEntitiesShow)
	ON_COMMAND(ID_BRUSH_GROUPS_ADDTOGROUP, OnBrushGroupsAddtogroup)
	ON_COMMAND(ID_BRUSH_GROUPS_MAKENEWGROUP, OnBrushGroupsMakenewgroup)
	ON_COMMAND(ID_BRUSH_GROUPEDITOR, OnBrushGroupeditor)
	ON_COMMAND(ID_VIEW_SHOW_ALLBRUSHES, OnViewShowAllbrushes)
	ON_COMMAND(ID_VIEW_SHOW_CURRENTGROUP, OnViewShowCurrentgroup)
	ON_COMMAND(ID_VIEW_SHOW_VISIBLEBRUSHES, OnViewShowVisiblebrushes)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_VISIBLEBRUSHES, OnUpdateViewShowVisiblebrushes)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_ALLBRUSHES, OnUpdateViewShowAllbrushes)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_CURRENTGROUP, OnUpdateViewShowCurrentgroup)
	ON_COMMAND(ID_BRUSH_ADJUSTMENTMODE, OnBrushAdjustmentmode)
	ON_UPDATE_COMMAND_UI(ID_BRUSH_ADJUSTMENTMODE, OnUpdateBrushAdjustmentmode)
	ON_COMMAND(ID_BRUSH_SELECTED_DELETE, OnBrushSelectedDelete)
	ON_COMMAND(ID_BRUSH_SELECTED_NEXTINORDER, OnBrushSelectedNextinorder)
	ON_COMMAND(ID_BRUSH_SELECTED_PREVIOUSINORDER, OnBrushSelectedPreviousinorder)
	ON_COMMAND(ID_BRUSH_PRIMITIVES_STAIRCASE, OnBrushPrimitivesStaircase)
	ON_COMMAND(ID_FILE_MAPSETTINGS, OnFileMapsettings)
	ON_COMMAND(ID_QBSPNOWATER, OnQbspnowater)
	ON_COMMAND(ID_ENTITIES_COPYCURRENT, OnEntitiesCopycurrent)
	ON_UPDATE_COMMAND_UI(ID_ENTITIES_COPYCURRENT, OnUpdateEntitiesCopycurrent)
	ON_COMMAND(ID_TOOLS_GROUPMOVEMENTMODE, OnToolsGroupmovementmode)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_GROUPMOVEMENTMODE, OnUpdateToolsGroupmovementmode)
	ON_COMMAND(ID_BRUSH_REMOVESELECTEDFROMGROUP, OnBrushRemoveselectedfromgroup)
	ON_UPDATE_COMMAND_UI(ID_BRUSH_REMOVESELECTEDFROMGROUP, OnUpdateBrushRemoveselectedfromgroup)
	ON_COMMAND(ID_BRUSH_PRIMITIVES_TRIANGLE, OnBrushPrimitivesTriangle)
	ON_UPDATE_COMMAND_UI(ID_FILE_NEW, OnUpdateFileNew)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnUpdateFileOpen)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_COMMAND(ID_TOOLS_SETTEXTURESCALE, OnToolsSettexturescale)
	ON_COMMAND(ID_TOOLS_SETLIGHTING, OnToolsSetlighting)
	ON_COMMAND(ID_FILE_EXPORTOPTIMAL, OnFileExportOptimal)
	ON_COMMAND(ID_TOOLS_LOADNEWPALETTE, OnToolsLoadnewpalette)
	ON_COMMAND(ID_BRUSH_COPYTONEWGROUP, OnBrushCopytonewgroup)
	ON_UPDATE_COMMAND_UI(ID_BRUSH_COPYTONEWGROUP, OnUpdateBrushCopytonewgroup)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
   ON_COMMAND( ID_LOCKTEXTUREPOSITIONS, OnLockTexturePositions )
   ON_UPDATE_COMMAND_UI( ID_LOCKTEXTUREPOSITIONS, OnUpdateLockTexturePositions )
	ON_COMMAND(ID_AXIS_X, OnAxisX)
	ON_UPDATE_COMMAND_UI(ID_AXIS_X, OnUpdateAxisX)
	ON_COMMAND(ID_AXIS_Y, OnAxisY)
	ON_UPDATE_COMMAND_UI(ID_AXIS_Y, OnUpdateAxisY)
	ON_COMMAND(ID_AXIS_Z, OnAxisZ)
	ON_UPDATE_COMMAND_UI(ID_AXIS_Z, OnUpdateAxisZ)
	ON_COMMAND(ID_TOOLS_SETBACKGROUNDCOLOR, OnToolsSetbackgroundcolor)
	ON_COMMAND(ID_FILE_EXPORTBRUSHGROUP, OnFileExportbrushgroup)
	ON_COMMAND(ID_FILE_IMPORTBRUSHGROUP, OnFileImportbrushgroup)
	ON_COMMAND(ID_EDIT_SELECTALL, OnEditSelectall)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECTALL, OnUpdateEditSelectall)
	ON_COMMAND(ID_BRUSH_ADJUSTBRUSHORDER, OnBrushAdjustbrushorder)
	ON_UPDATE_COMMAND_UI(ID_BRUSH_ADJUSTBRUSHORDER, OnUpdateBrushAdjustbrushorder)
	ON_UPDATE_COMMAND_UI(ID_COMPUTELIGHTING, OnUpdateComputelighting)
	ON_UPDATE_COMMAND_UI(ID_FILE_GENERATELIGHTTABLEREPORT, OnUpdateFileGeneratelighttablereport)
	ON_COMMAND(ID_FILE_GENERATELIGHTTABLEREPORT, OnFileGeneratelighttablereport)
	ON_COMMAND(ID_TOOLS_ASSIGNVOLUMES, OnToolsAssignvolumes)
	ON_UPDATE_COMMAND_UI(ID_TOOLS_ASSIGNVOLUMES, OnUpdateToolsAssignvolumes)
	ON_COMMAND(ID_VIEW_SHOW_VOLUMES, OnViewShowVolumes)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_VOLUMES, OnUpdateViewShowVolumes)
	ON_COMMAND(ID_VIEW_SHOW_FILLEDVOLUMES, OnViewShowFilledvolumes)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_FILLEDVOLUMES, OnUpdateViewShowFilledvolumes)
	ON_COMMAND(ID_ENTITIES_DELETESELECTED, OnEntitiesDeleteselected)
	ON_UPDATE_COMMAND_UI(ID_ENTITIES_DELETESELECTED, OnUpdateEntitiesDeleteselected)
	ON_COMMAND(ID_DELETESELECTIONS, OnDeleteselections)
   ON_COMMAND( ID_FILE_EXPORTAS, OnFileExportAs )
   ON_COMMAND(ID_LIGHTS_EDITOR, OnLightsEditor )
	ON_UPDATE_COMMAND_UI(ID_LIGHTS_EDITOR, OnUpdateLightsEditor )
   ON_COMMAND(ID_LIGHTS_PLACE, OnLightsPlace )
   ON_COMMAND(ID_LIGHTS_SHOW, OnLightsShow )
   ON_UPDATE_COMMAND_UI(ID_LIGHTS_SHOW, OnUpdateLightsShow )
   ON_COMMAND(ID_LIGHTS_SHOWFALLOFFS, OnLightsShowFalloffs )
   ON_UPDATE_COMMAND_UI(ID_LIGHTS_SHOWFALLOFFS, OnUpdateLightsShowFalloffs )
   ON_COMMAND(ID_LIGHTS_COPYCURRENT, OnLightsCopyCurrent )
   ON_UPDATE_COMMAND_UI(ID_LIGHTS_COPYCURRENT, OnUpdateLightsCopyCurrent )
   ON_COMMAND(ID_LIGHTS_DELETESELECTED, OnLightsDeleteSelected )
   ON_UPDATE_COMMAND_UI(ID_LIGHTS_DELETESELECTED, OnUpdateLightsDeleteSelected )
	ON_COMMAND(ID_COMPUTELIGHTING, OnComputelighting)
	ON_UPDATE_COMMAND_UI(ID_LIGHT_LOCKMOVE, OnUpdateLightLockmove)
	ON_COMMAND(ID_LIGHT_LOCKMOVE, OnLightLockmove)
   ON_UPDATE_COMMAND_UI(ID_LIGHTS_SHOW3D, OnUpdateLightsShow3D )
   ON_COMMAND(ID_LIGHTS_SHOW3D, OnLightsShow3D )
   ON_UPDATE_COMMAND_UI(ID_ENTITIES_SHOW3D, OnUpdateEntitiesShow3D )
   ON_COMMAND(ID_ENTITIES_SHOW3D, OnEntitiesShow3D )
   ON_COMMAND(ID_ENTITIES_CONVERT, OnEntitiesConvert )
   
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI_RANGE(ID_BRUSH_SELECTED_NEXTINORDER, ID_BRUSH_SELECTED_PREVIOUSINORDER, OnSelectedTypeCmdUI)
	//ON_UPDATE_COMMAND_UI_RANGE(ID_BRUSH_SELECTED_DELETE, ID_BRUSH_SELECTED_COPYTOCURRENT, OnSelectedTypeCmdUI)
END_MESSAGE_MAP()

                                      
/////////////////////////////////////////////////////////////////////////////
// CTHREDDoc construction/destruction


// The Mother of All Kludges.  Force the TS resources to be loaded.
//static TS::ResourceTypeTSMaterialList	_resdml(".dml");

CTHREDDoc::CTHREDDoc()
{
   // initialize the shape information, this should be done in 
   // itreditshape or whatever
   ShapeInfo.m_numLightStates = 0;
   ShapeInfo.m_linkedInterior = FALSE;

   pActiveDetail = NULL;
   m_StateList = NULL;
   
   // create a state
   CShapeState * pShapeState = AddState( "" );

   // create a detail for this state
   CShapeDetail * pShapeDetail = pShapeState->AddDetail( "" );

   // set this as active
   pShapeDetail->SetActive();
 
   // set a flag
   pShapeDetail->mBuiltEditBox = TRUE;

   BuildEditBox(TRUE);
   
   // we are not adjusting.
   mAdjustmentMode = 0;

   // keepers
   mViewAxis = (AXIS_X|AXIS_Y|AXIS_Z);
   mDrawFine= mDrawCoarse= 1;
   mUseRotationSnap = 1;
   mRotationSnapDegrees = ((double)30.0/(double)360.0)*(double)M_TOTAL_DEGREES;
   
   // DPW - Added for TS compatibility
   m_SPalette = NULL;
   m_materialList = NULL;
   m_matresobj = NULL;
   m_mySun = NULL;
   m_lightVector.x = 0;
   m_lightVector.y = 0;
   m_lightVector.z = 1000;

   mGridSize = 32.0;
   mCurrentGroup = LB_ERR;
   mShow = ID_VIEW_SHOW_ALLBRUSHES;

   // Undo arrays - DPW
   UndoArray.SetSize(NUM_UNDOS);

   // Redo arrays - DPW
   RedoArray.SetSize(NUM_UNDOS);

   // set the sizes for the copy buffers
   mEntityCopyArray.SetSize( 0, 20 );
   mLightCopyArray.SetSize( 0, 20 );

   mCanUndo = FALSE;
   mCanRedo = FALSE;
   mCurrentUndoLevel = 0;
   mCurrentRedoLevel = 0;

   background_color = 0;
   curr_colorchange_selection = 0;

   mcopylist = NULL;

   m_VolumeDescription[0].Format("Normal Volume");

   for(int i = 1; i < TOTAL_STATES; i++)
   {
      m_VolumeDescription[i].Format("");
   }
}

CTHREDDoc::~CTHREDDoc()
{
   // delete all the states
   CShapeState * pTraverse;
   pTraverse = m_StateList;
   while( pTraverse )
   {
      m_StateList = pTraverse;
      pTraverse = pTraverse->pNext;
      delete m_StateList;
   }
   
	CThredBrush* Brush;
	CThredBrush* NextBrush;

	Brush = mcopylist;
	while( Brush )
   {
		NextBrush = Brush->GetNextBrush();
		delete Brush;
		Brush = NextBrush;
	}
}

UndoInfo::UndoInfo()
{
}

UndoInfo::UndoInfo(UndoInfo &inval)
{
  int n;

  for (n = 0; n <= inval.matrix.GetUpperBound(); n++)
  {
    matrix.InsertAt(n, inval.matrix.ElementAt(n));
  }

  for (n = 0; n <= inval.brushes.GetUpperBound(); n++)
  {
    brushes.InsertAt(n, inval.brushes.ElementAt(n));
  }
}

UndoInfo::~UndoInfo()
{
}

UndoInfo& UndoInfo::operator=(UndoInfo& inval)
{
  int n;

  for (n = 0; n <= inval.matrix.GetUpperBound(); n++)
  {
    matrix.InsertAt(n, inval.matrix.ElementAt(n));
  }

  for (n = 0; n <= inval.brushes.GetUpperBound(); n++)
  {
    brushes.InsertAt(n, inval.brushes.ElementAt(n));
  }

  return *this;
}

void CTHREDDoc::StoreUndoInfo()
{
  UndoInfo      temp;
  CThredBrush*  brush;

  brush = mCurrentBrush;

  while(brush)
  {
    temp.matrix.InsertAt(0, brush->mTotalMatrix);
    temp.brushes.InsertAt(0, brush->mBrushId);

    // Go to next one
    brush = brush->NextSelectedBrush;
  }

  UndoArray.InsertAt(0, temp);

  if (mCurrentUndoLevel < NUM_UNDOS)
  {
    mCurrentUndoLevel++;
  }

  mCanUndo = TRUE;
}

BOOL CTHREDDoc::OnNewDocument()
{
   if (!theApp.mAllowNew)
   {
      theApp.multiple_requests = TRUE;
      AfxMessageBox("Only 1 File May Be Opened At A Time...");
      return(false);
   }

   NewFileDlg newFileDlg(NewFileDlg::CreateIDD);
   newFileDlg.m_pDoc = this;

   CString newName;
   newName.Format("%s.zed", (LPCTSTR)GetTitle());

   // clear the views
   CMainFrame * pFrame = (CMainFrame*)AfxGetMainWnd();
   if(pFrame)
   {
      pFrame->m_wndObjectView.Enable(true);
      pFrame->m_wndLightView.Enable(true);
   }
   
   newFileDlg.m_strName = newName;
   SetTitle((LPCTSTR)newName);

   // update zee information.
   UpdateBrushInformation();
   UpdateGridInformation();

    if (!CDocument::OnNewDocument())
      return(false);

   // Get palette and material list preferences from .ini file
   GetPrivateProfileString("Preference", "Palette", "", palName, sizeof(palName), "ZED.ini");
   GetPrivateProfileString("Preference", "MatList", "", matName, sizeof(matName), "ZED.ini");
   GetPrivateProfileString("Preference", "PaletteVol", "", palVolName, sizeof(palVolName), "ZED.ini");
   GetPrivateProfileString("Preference", "MaterialsVol", "", matVolName, sizeof(matVolName), "ZED.ini");

   // Preset the values in the file dialog
   newFileDlg.m_strPalette.Format("%s", palName);
   newFileDlg.m_strBitmaps.Format("%s", matName);
   newFileDlg.m_strBitmapsVol.Format("%s", matVolName);
   newFileDlg.m_strPaletteVol.Format("%s", palVolName);
   newFileDlg.new_mat_count = 0;

   // Bring up dialog to ask for palette & materials
   if(newFileDlg.DoModal() != IDOK)
      return(false);

   newName = newFileDlg.m_strName;
   SetTitle((LPCTSTR)newName);
   theApp.mOpenFileName = newName;
   
   // use the vol if it exists
   if(strlen(newFileDlg.m_PaletteVol.getTitle().c_str()))
   {
      // load up the volume for the matlist
      theApp.m_resmgr.setSearchPath(newFileDlg.m_PaletteVol.getPath().c_str());
      theApp.m_resmgr.addVolume(newFileDlg.m_PaletteVol.getTitle().c_str());
   }
   else
      theApp.m_resmgr.setSearchPath(newFileDlg.m_PaletteFile.getPath().c_str());
      
   // Load and assign the palette
   m_palresobj = theApp.m_resmgr.load(newFileDlg.m_PaletteFile.getTitle().c_str(), TRUE);
   if(m_palresobj)   
      m_SPalette = (GFXPalette*)m_palresobj->resource;

   if(!m_SPalette)
   {
      AfxMessageBox("Error allocating palette", MB_OK, 0);
      return FALSE;
   }

   // use the vol if entered
   if(strlen(newFileDlg.m_TextureVol.getTitle().c_str()))
   {
      // load up the volume for the matlist
      theApp.m_resmgr.setSearchPath(newFileDlg.m_TextureVol.getPath().c_str());
      theApp.m_resmgr.addVolume(newFileDlg.m_TextureVol.getTitle().c_str());
   }
   else
      theApp.m_resmgr.setSearchPath( newFileDlg.m_TextureFile.getPath().c_str());
      
   // Load and assign the material list
   m_matresobj = theApp.m_resmgr.load(newFileDlg.m_TextureFile.getTitle().c_str(), TRUE);
   if(m_matresobj)   
      m_materialList = (TSMaterialList*)m_matresobj->resource;

   if(!m_materialList)
   {
      AfxMessageBox("Error loading material list", MB_OK, 0);
      return(false);
   } 
   else 
   {
      // Lock that baby to load the textures in...
      m_materialList->load(theApp.m_resmgr, true);
      CMaterialProperties::Initialize(m_materialList->getMaterialsCount());
   }

   // assign the texture stuff
   if(pFrame)
      pFrame->m_wndTextureView.Attach(GetMaterialList(), GetPalette());   

   strcpy(palName, (LPCTSTR)newFileDlg.m_strPalette);
   strcpy(matName, (LPCTSTR)newFileDlg.m_strBitmaps);
   strcpy(palVolName, (LPCTSTR)newFileDlg.m_strPaletteVol);
   strcpy(matVolName, (LPCTSTR)newFileDlg.m_strBitmapsVol);

   // Write out the palette and material preferences to the .ini file
   WritePrivateProfileString("Preference", "Palette", palName, "ZED.ini");
   WritePrivateProfileString("Preference", "MatList", matName, "ZED.ini");
   WritePrivateProfileString("Preference", "PaletteVol", palVolName, "ZED.ini");
   WritePrivateProfileString("Preference", "MaterialsVol", matVolName, "ZED.ini");

   // Disable new and open choices...
   theApp.mAllowOpen = FALSE;
   theApp.mAllowNew  = FALSE;

   // Grab your grid and some brush colors...
   color_request.peRed = 64;
   color_request.peGreen = 64;
   color_request.peBlue = 64;
   line_grid_color = m_SPalette->GetNearestColor(64, 64, 64);//color_request);
   dot_grid_color = 255;
   color_plus = 250;
   color_minus = 249;
   color_origin = 251;
   color_select = 253;
   color_def_brush = 252;  // Blue in default palette
   color_def_point = 249;  // Red in default palette
   color_def_entity = 255;
   volume_color = 254;
   color_bbox = 3;
   
   return(true);
}

/////////////////////////////////////////////////////////////////////////////
// CTHREDDoc serialization


 
/////////////////////////////////////////////////////////////////////////////
// CTHREDDoc diagnostics

#ifdef _DEBUG
void CTHREDDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTHREDDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTHREDDoc commands

#define CTHREDDOCTYPE                  "CTHREDDoc"
#define CTHREDVERSION                  "CTHREDVersion"
#define CTHREDDOCSTATEID               "StateID"
#define CTHREDDOCNUMSTATES             "NumStates"
#define CENDTHREDDOCTYPE               "End CTHREDDoc"

void CTHREDDoc::Serialize(CArchive& ar)
{
   CShapeState * pState;
   CShapeDetail * pDetail;
   int IntVal;

   char  tmpbuf[32];
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

   // call our base class serialize
   CThredParser Parser(&ar, NULL);

   if(ar.IsStoring())
   {
      // write the version number
      CTHREDApp * App = ( CTHREDApp * )AfxGetApp();
      Parser.WriteInt( CTHREDVERSION, App->m_version );

      Parser.WriteString( CTHREDDOCTYPE );

      // write out the settings
      CString fileName = ar.m_strFileName; 
      App->mOpenFileName = fileName;
      // set the apps filename
      SerializeSettings( Parser, fileName );
      
      // write the info ( number of states... )
//      Parser.WriteInt( CTHREDDOCSTATEID, mStateID );
      for( pState = m_StateList, IntVal = 0; pState; pState = pState->pNext,
         IntVal++ );
      Parser.WriteInt( CTHREDDOCNUMSTATES, IntVal );

      pState = m_StateList;
      while( pState )
      {
         pState->Serialize( Parser );
         pState = pState->pNext;
      }

      // store the material properties
      CMaterialProperties::Serialize( Parser );
      Parser.WriteString( CENDTHREDDOCTYPE );
   } 
   else 
   {
      // read the version number
      IntVal = 0;
      CTHREDApp * App = ( CTHREDApp * )AfxGetApp();
      Parser.GetKeyInt( CTHREDVERSION, IntVal );
      App->m_fileVersion = IntVal;

      // -------------------------------------------------------
      // ---- fix for stupid bug i introduced ----
      // need to convert any 3digit minor code into a 2digit one
      // CANNOT HAVE A MINOR VERSION ABOVE 100 FOR MAJOR VERSION 1!
      if( ( theApp.m_fileVersion > makeVersion( 1, 100 ) ) &&
         ( theApp.m_fileVersion < makeVersion( 2, 0 ) ) )
      {
         int minor = theApp.m_fileVersion & 0x0000ffff;
         minor /= 10;
         theApp.m_fileVersion = makeVersion( 1, minor );
      }
      
      // look at the version here...   
      // read the settings
      CString fileName = ar.m_strFileName; 
      App->mOpenFileName = fileName;
      if( ( App->m_fileVersion >> 16 ) != 1 )
      {
         if( AfxMessageBox( "Attempt to import this file? ", MB_YESNO ) 
            != IDYES )
            return;
           
         // remove all states
         while( m_StateList )
            RemoveState( m_StateList );
            
         // older version does not contain document version or settings information
         // add a state and a detail, will load all info into this detail
         pState = AddState( "" );
         pDetail = pState->AddDetail( "" );
         
         if( !SerializeSettings( Parser, fileName ) )
            return;
            
         // serialize this detail as the old version
         pDetail->SerializeOld( Parser );

         // set the detail as active
         pDetail->SetActive();
         
      }
      else
      {
         // load in the info
         if( !Parser.GetKey( CTHREDDOCTYPE ) )
            return;
     
         if( !SerializeSettings( Parser, fileName ) )
            return;
      
         // get the number of states
         Parser.GetKeyInt( CTHREDDOCNUMSTATES, IntVal );

         // remove all states
         while( m_StateList )
            RemoveState( m_StateList );
            
         // add all the states       
         while( IntVal )
         {
            // add a state
            pState = AddState( "" );
                   
            // get the state info
            pState->Serialize( Parser );
            
            IntVal--;
         }
         
         // set the first detail as active
         pState = m_StateList;
         while( pState )
         {
            if( pState->mDetails )
            {
               pState->mDetails->SetActive();
               break;
            }
            pState = pState->pNext;
         }
 
         // get the material properties
         CMaterialProperties::Serialize( Parser );
     
         Parser.SetEnd( CENDTHREDDOCTYPE );
		   Parser.GotoEnd();
      }

      //UpdateBrushInformation();
      UpdateActiveBrushes();
      
      // update the object view
      CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
      if( pFrame )
      {
         pFrame->m_wndObjectView.Rebuild( this );
         
//         // get the current light array
//         pFrame->m_wndLightView.Rebuild( this, *pLightArray );
      }
   }
}


#define CSETTINGSTYPE            "CSettingsType"
#define CENDSETTINGSTYPE         "End CSettingsType"
#define CSHAPEEDITNUMLIGHTSTATES "NumLightStates"
#define CSHAPEEDITLINKEDINTERIOR "LinkedInterior"
#define CSHAPEEDITHIGHDETAIL     "HighDetailBuild"
#define CBUILDGEOMETRYSCALE      "GeometryScale"
#define CBUILDSNAPPRECISION      "SnapPrecision"
#define CBUILDNORMALPRECISION    "NormalPrecision"
#define CBUILDDISTANCEPRECISION  "DistancePrecision"
#define CBUILDTEXTURESCALE       "TextureScale"
#define CBUILDLOWDETAIL          "LowDetail"
#define CBUILDMAXTEXMIP          "MaxTexMip"
#define CLIGHTGEOMETRYSCALE      "LightGeometryScale"
#define CLIGHTLIGHTSCALE         "LightScale"
#define CLIGHTUSENORMALS         "UseNormals"
#define CLIGHTQUANTUM            "QuantumNumber"
#define CLIGHTUSEMATPROPERTIES   "UseMaterialProperties"
#define CPALNAME                 "Palette"
#define CMATLISTNAME             "Material"
#define CPALVOLNAME              "PaletteVol"
#define CMATLISTVOLNAME          "MaterialVol"
#define CGRIDSIZE                "Gridsize"
#define CTEXTURESCALE            "TextureScale"
#define CGROUPSHOW               "GroupShow"
#define CCURRENTGROUP            "CurrentGroup"
#define CMININTENSITY            "MinIntensity"
#define COBJVIEWWIDTH            "ObjViewWidth"
#define COBJVIEWHEIGHT           "ObjViewHeight"
#define CLIGHTAMBIENT            "AmbientLight"
#define CAMBIENTOUTSIDEPOLYS     "AmbientOutsidePolys"

bool CTHREDDoc::SerializeSettings( CThredParser& Parser, CString & file )
{
//   ITRShapeEdit:
//      Int32  m_numLightStates;
//      bool   m_linkedInterior;
//      bool   m_highDetailBuild;
//
//   ITRBuildOptions:
//      float    m_geometryScale;
//      float    m_pointSnapPrecision;
//      float    m_planeNormalPrecision;
//      float    m_planeDistancePrecision;
//      float    m_textureScale;
//      bool     m_lowDetail;
//      UInt32   m_maximumTexMip;
//
//   ITRLightOptions:
//      float    m_geometryScale;
//      UInt32   m_lightScale;
//      bool     m_useNormals;
//      UInt32   m_emissionQuantumNumber;
//      bool     m_useMaterialProperties;
   if( Parser.IsStoring )
   {
      // start the transaction
      Parser.WriteString( CSETTINGSTYPE );

      // write out shape edit stuff
      Parser.WriteLong( CSHAPEEDITNUMLIGHTSTATES, ShapeInfo.m_numLightStates );
      Parser.WriteBool( CSHAPEEDITLINKEDINTERIOR, ShapeInfo.m_linkedInterior );
      
      // write out build options
      Parser.WriteFloat( CBUILDGEOMETRYSCALE, BuildOptions.m_geometryScale );
      Parser.WriteFloat( CBUILDSNAPPRECISION, BuildOptions.m_pointSnapPrecision );
      Parser.WriteFloat( CBUILDNORMALPRECISION, BuildOptions.m_planeNormalPrecision );
      Parser.WriteFloat( CBUILDDISTANCEPRECISION, BuildOptions.m_planeDistancePrecision );
      Parser.WriteFloat( CBUILDTEXTURESCALE, BuildOptions.m_textureScale );
      Parser.WriteBool( CBUILDLOWDETAIL, BuildOptions.m_lowDetail );
      Parser.WriteULong( CBUILDMAXTEXMIP, BuildOptions.m_maximumTexMip );
      
      // write out light options      
      Parser.WriteFloat( CLIGHTGEOMETRYSCALE, LightOptions.m_geometryScale );
      Parser.WriteULong( CLIGHTLIGHTSCALE, LightOptions.m_lightScale );
      Parser.WriteBool( CLIGHTUSENORMALS, LightOptions.m_useNormals );
      Parser.WriteULong( CLIGHTQUANTUM, LightOptions.m_emissionQuantumNumber );
      Parser.WriteBool( CLIGHTUSEMATPROPERTIES, LightOptions.m_useMaterialProperties );

      ThredPoint ambient( LightOptions.m_ambientIntensity.red,
         LightOptions.m_ambientIntensity.green,
         LightOptions.m_ambientIntensity.blue );
      Parser.WritePoint( CLIGHTAMBIENT, ambient );
      Parser.WriteBool( CAMBIENTOUTSIDEPOLYS, LightOptions.m_applyAmbientOutside );
      
      // write out the new info stuff
      Parser.WriteString( CPALNAME, palName );
      Parser.WriteString( CMATLISTNAME, matName );
      Parser.WriteString( CPALVOLNAME, palVolName);
      Parser.WriteString( CMATLISTVOLNAME, matVolName);
      Parser.WriteFloat( CGRIDSIZE, mGridSize );
      Parser.WriteFloat( CTEXTURESCALE, theApp.m_texturescale );
      Parser.WriteLong( CGROUPSHOW, mShow );
      Parser.WriteLong( CCURRENTGROUP, mCurrentGroup );
      Parser.WriteFloat( CMININTENSITY, theApp.minintensity );

      CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
      Parser.WriteULong( COBJVIEWWIDTH, pFrame->m_wndObjectView.CurrentSize.cx );
      Parser.WriteULong( COBJVIEWHEIGHT, pFrame->m_wndObjectView.CurrentSize.cy );
      
      // end the transaction      
      Parser.WriteString( CENDSETTINGSTYPE );
   }
   else
   {
      // make sure correct key
      if( !Parser.GetKey( CSETTINGSTYPE ) )
         return( false );
         
      // read in the shapeinfo
      Parser.GetKeyLong( CSHAPEEDITNUMLIGHTSTATES, ShapeInfo.m_numLightStates );
      Parser.GetKeyBool( CSHAPEEDITLINKEDINTERIOR, ShapeInfo.m_linkedInterior );
      
      // read in the build options
      Parser.GetKeyFloat( CBUILDGEOMETRYSCALE, BuildOptions.m_geometryScale );
      Parser.GetKeyFloat( CBUILDSNAPPRECISION, BuildOptions.m_pointSnapPrecision );
      Parser.GetKeyFloat( CBUILDNORMALPRECISION, BuildOptions.m_planeNormalPrecision );
      Parser.GetKeyFloat( CBUILDDISTANCEPRECISION, BuildOptions.m_planeDistancePrecision );
      Parser.GetKeyFloat( CBUILDTEXTURESCALE, BuildOptions.m_textureScale );
      Parser.GetKeyBool( CBUILDLOWDETAIL, BuildOptions.m_lowDetail );
      Parser.GetKeyULong( CBUILDMAXTEXMIP, BuildOptions.m_maximumTexMip );

      // set the precisions
      theApp.m_planeDistancePrecision = BuildOptions.m_planeDistancePrecision;
      theApp.m_planeNormalPrecision = BuildOptions.m_planeNormalPrecision;

      // read in the light options
      Parser.GetKeyFloat( CLIGHTGEOMETRYSCALE, LightOptions.m_geometryScale );
      Parser.GetKeyULong( CLIGHTLIGHTSCALE, LightOptions.m_lightScale );
      Parser.GetKeyBool( CLIGHTUSENORMALS, LightOptions.m_useNormals );
      Parser.GetKeyULong( CLIGHTQUANTUM, LightOptions.m_emissionQuantumNumber );
      Parser.GetKeyBool( CLIGHTUSEMATPROPERTIES, LightOptions.m_useMaterialProperties );
      
      // do the ambient
      ThredPoint ambient;
      if( Parser.GetKeyPoint( CLIGHTAMBIENT, ambient ) )
      {
         LightOptions.m_ambientIntensity.red = ambient.X;
         LightOptions.m_ambientIntensity.green = ambient.Y;
         LightOptions.m_ambientIntensity.blue = ambient.Z;
      }
      
      // get the outside amb. flag
      bool boolVal;   
      if( Parser.GetKeyBool( CAMBIENTOUTSIDEPOLYS, boolVal ) )
         LightOptions.m_applyAmbientOutside = boolVal;
         
      // check if need to read in the old zcf file
      if( theApp.m_fileVersion < makeVersion( 1, 80 ) )
      {
         char tmpbuf[32];
         char tmpbuf1[256];
         
         int length = file.GetLength();
         file.SetAt( length-3, 'Z' );
         file.SetAt( length-2, 'C' );
         file.SetAt( length-1, 'F' );
         
         GetPrivateProfileString("Files","Palette"," ",
            palName,sizeof(palName),(LPCTSTR)file );

         GetPrivateProfileString("Files","Material"," ",
            matName,sizeof(matName),(LPCTSTR)file );


         GetPrivateProfileString("Settings", "GridSize", "64.0", tmpbuf, sizeof(tmpbuf), (LPCTSTR)file );
         mGridSize = atof (tmpbuf);
         if (mGridSize < 1.0)
            mGridSize = 8.0;

         GetPrivateProfileString("Settings", "TextureScale", "1.0", tmpbuf, sizeof(tmpbuf), (LPCTSTR)file );
         theApp.m_texturescale = atof (tmpbuf);
         if (theApp.m_texturescale == 0.F)
            theApp.m_texturescale = 1.0;

         GetPrivateProfileString("Settings", "MinIntensity", "0.025", tmpbuf, sizeof(tmpbuf), (LPCTSTR)file );
            theApp.minintensity = atof (tmpbuf);
         if (theApp.minintensity == 0.F)
            theApp.minintensity = 0.025;

         GetPrivateProfileString("Settings", "GroupShow", "0", tmpbuf, sizeof(tmpbuf), (LPCTSTR)file );
         mShow = atoi(tmpbuf);
         if((mShow < ID_VIEW_SHOW_ALLBRUSHES) || (mShow > ID_VIEW_SHOW_CURRENTGROUP))
            mShow = ID_VIEW_SHOW_ALLBRUSHES;

         GetPrivateProfileString("Settings", "CurrentGroup", "-1", tmpbuf, sizeof(tmpbuf), (LPCTSTR)file );
         mCurrentGroup = atoi(tmpbuf);

         for( int i = 0; i < TOTAL_STATES; i++)
         {
            sprintf(tmpbuf, "D%d", i);
            GetPrivateProfileString("Volumes", tmpbuf, "", tmpbuf1, sizeof(tmpbuf1), (LPCTSTR)file);
            m_VolumeDescription[i].Format("%s", tmpbuf1);
         }
      }
      else
      {
         palName[0] = matName[0] = palVolName[0] = matVolName[0] = 0;
         
         // read in the new info stuff
         CString buf;      
         if(Parser.GetKeyString( CPALNAME, buf ))
            strcpy( palName, buf );
         if(Parser.GetKeyString( CMATLISTNAME, buf ))
            strcpy( matName, buf );

         if(Parser.GetKeyString(CPALVOLNAME, buf))
            strcpy(palVolName, buf);
         if(Parser.GetKeyString(CMATLISTVOLNAME, buf))
            strcpy(matVolName, buf);

         float valFloat;
         long valLong;
      
         valFloat = 64.0f; Parser.GetKeyFloat( CGRIDSIZE, valFloat ); mGridSize = valFloat;
         valFloat = 1.0f; Parser.GetKeyFloat( CTEXTURESCALE, valFloat ); theApp.m_texturescale = valFloat;
         valLong = ID_VIEW_SHOW_ALLBRUSHES; Parser.GetKeyLong( CGROUPSHOW, valLong ); mShow = valLong;
         valLong = -1; Parser.GetKeyLong( CCURRENTGROUP, valLong ); mCurrentGroup = valLong;
         valFloat = 0.025f; Parser.GetKeyFloat( CMININTENSITY, valFloat ); theApp.minintensity = valFloat;

         CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
         if( pFrame )
         {
            valLong = 160; Parser.GetKeyLong( COBJVIEWWIDTH, valLong ); pFrame->m_wndObjectView.CurrentSize.cx = valLong;
            valLong = 600; Parser.GetKeyLong( COBJVIEWWIDTH, valLong ); pFrame->m_wndObjectView.CurrentSize.cy = valLong;
         }
      }
      
      Parser.SetEnd( CENDSETTINGSTYPE );
      Parser.GotoEnd();

      // load up the pal
      if( !LoadPalette() )
         return( false );
   }
   
   return( true );
}

//----------------------------------------------------------------

bool CTHREDDoc::LoadPalette()
{
   NewFileDlg newFileDlg(NewFileDlg::LoadIDD);
   
   newFileDlg.m_pDoc = this;
   newFileDlg.new_mat_count = 0;
   bool doDialog;
   
   // do not bother with palettes if run from commandline, 
   // and dont actually load the materiallist
   bool commandLine = theApp.mExportRun;
   if(theApp.mPathIsDML)
      strcpy(matName, theApp.mSearchPath);
      
   do {
      doDialog = false;
      if(!strlen(palName)||!strlen(matName))
         doDialog = true;
      else
      {
         FileName palFileName(palName);
         FileName matFileName(matName);
         
         // attempt to load the things - use the volumes if they exist
         if(!commandLine)
         {
            if(strlen(palVolName))
            {
               FileName fn(palVolName);
               // load up the volume for the matlist
               theApp.m_resmgr.setSearchPath(fn.getPath().c_str());
               theApp.m_resmgr.addVolume(fn.getTitle().c_str());
            }
            else
               theApp.m_resmgr.setSearchPath(palFileName.getPath().c_str());
         
   
            // load in the new palette
            m_palresobj = theApp.m_resmgr.load(palFileName.getTitle().c_str(), true);
            if(m_palresobj)
               m_SPalette = (GFXPalette*)m_palresobj->resource;
         }
         
         // do the material list - attempt volume load
         if(strlen(matVolName))
         {
            FileName fn(matVolName);
            theApp.m_resmgr.setSearchPath(fn.getPath().c_str());
            theApp.m_resmgr.addVolume(fn.getTitle().c_str());
         }
         else
            theApp.m_resmgr.setSearchPath(matFileName.getPath().c_str());

         // load in the matlist
         m_matresobj = theApp.m_resmgr.load(matFileName.getTitle().c_str(), true);
         if(m_matresobj)
            m_materialList = (TSMaterialList*)m_matresobj->resource;
                     
         // tell the user about it and set flag to bring up the dialog box
         if((!m_SPalette && !commandLine) || !m_materialList)
         {
            char * messages[] = {"Improper Palette", "Improper MaterialList",
               "Improper Palette and MaterialList"};
            int error = (((!m_SPalette && !commandLine) ? 0x01 : 0x00 ) + ((!m_materialList) ? 0x02 : 0x00)) - 1;
            AfxMessageBox(messages[error], MB_OK, 0);
            doDialog = true;
         }            
      }
         
      if(doDialog)
      {
         // set the values in the file dialog
         newFileDlg.m_strPalette.Format("%s", palName);
         newFileDlg.m_strBitmaps.Format("%s", matName);
         newFileDlg.m_strBitmapsVol.Format("%s", matVolName);
         newFileDlg.m_strPaletteVol.Format("%s", palVolName);
         
         if(newFileDlg.DoModal() != IDOK)
            return(false);
         
         // set the current name's
         strcpy(palName, (LPCTSTR)newFileDlg.m_strPalette);
         strcpy(matName, (LPCTSTR)newFileDlg.m_strBitmaps);
         strcpy(palVolName, (LPCTSTR)newFileDlg.m_strPaletteVol);
         strcpy(matVolName, (LPCTSTR)newFileDlg.m_strBitmapsVol);
      }
   } while(doDialog);

   if(!commandLine)
   {
      m_materialList->load(theApp.m_resmgr, true);
      m_materialList->load(theApp.m_resmgr, true);
      CMaterialProperties::Initialize(m_materialList->getMaterialsCount());
   }

   return( true );
}

//----------------------------------------------------------------

void CTHREDDoc::OnToolsBrushAttributes() 
{
   if( mCurrentBrush )
   {
   	mCurrentBrush->BrushAttributesDialog(this);
   	UpdateAllViews(NULL);
	   SetModifiedFlag();
   }
}

// When called this function will add a brush
void CTHREDDoc::OnBrushAddtoworld() 
{
  //if (!mGroupMovement)
  //{
  if (mCurrentBrush != &mRegularBrush)
    return;

	// we are in adjustment mode so just
	// say it's ADDED and then leave
	SetModifiedFlag();
	if(mAdjustmentMode)
    {
        CThredBrush*  tempbrush = mCurrentBrush;

        while(tempbrush)
        {
		  //mCurrentBrush->mTransactionType = BRUSH_ADD_TO_WORLD;
		  tempbrush->mTransactionType = BRUSH_ADD_TO_WORLD;

          tempbrush = tempbrush->NextSelectedBrush;
        }

		mCurrentBrush = &mRegularBrush;
		mAdjustmentMode = 0;

        // Need to make sure no other brushes are selected
        tempbrush = mBrushList;

        // Deselect them...
        while(tempbrush)
        {
          tempbrush->mFlags &= ~BRUSH_SELECTED;
          tempbrush->NextSelectedBrush = NULL;
          tempbrush->PrevSelectedBrush = NULL;

          tempbrush = tempbrush->GetNextBrush();
        }

		UpdateAllViews(NULL);
		return;
	}
	// make sure we are adding the regular brush
	_ASSERTE(mCurrentBrush == &mRegularBrush);

	// before we add the brush to the world
	// we will add it onto our list of brushes
	mCurrentBrush->mTransactionType = BRUSH_ADD_TO_WORLD;
	mCurrentBrush->mBrushGroup = mCurrentGroup;
	int Brush = AddBrushToList(*mCurrentBrush);

	UpdateBrushInformation();

    // Need to do box mapping here for mCurrentBrush...
    mCurrentBrush->BoxMap();

	(*pWorldBsp).ProcessBrush(mCurrentBrush, Brush, TRUE);

	UpdateAllViews(NULL);
  //}
}

void CTHREDDoc::OnBrushSubtractfromworld() 
{
  //if (!mGroupMovement)
  //{
  if (mCurrentBrush != &mRegularBrush)
  {
    return;
  }

	// we are in adjustment mode so just
	// say it's ADDED and then leave
	SetModifiedFlag();
	if(mAdjustmentMode)
    {
        CThredBrush*  tempbrush = mCurrentBrush;

        while (tempbrush)
        {
		  //mCurrentBrush->mTransactionType = BRUSH_SUBTRACT_FROM_WORLD;
		  tempbrush->mTransactionType = BRUSH_SUBTRACT_FROM_WORLD;

          tempbrush = tempbrush->NextSelectedBrush;
        }

		mCurrentBrush = &mRegularBrush;
		mAdjustmentMode = 0;

        // Need to make sure no other brushes are selected
        tempbrush = mBrushList;

        // Deselect them...
        while(tempbrush)
        {
          tempbrush->mFlags &= ~BRUSH_SELECTED;
          tempbrush->NextSelectedBrush = NULL;
          tempbrush->PrevSelectedBrush = NULL;

          tempbrush = tempbrush->GetNextBrush();
        }

        // Need to do box mapping here for mCurrentBrush...
//        mCurrentBrush->BoxMap();

        UpdateAllViews(NULL);
		return;
	}

	// make sure we are subbing the regular brush
	_ASSERTE(mCurrentBrush == &mRegularBrush);

	// add it to our list man.
	mCurrentBrush->mTransactionType = BRUSH_SUBTRACT_FROM_WORLD;
	mCurrentBrush->mBrushGroup = mCurrentGroup;
	int Brush = AddBrushToList(*mCurrentBrush);

	UpdateBrushInformation();

    // Need to do box mapping here for mCurrentBrush...
    mCurrentBrush->BoxMap();

    (*pWorldBsp).ProcessBrush(mCurrentBrush, Brush, TRUE);

	UpdateAllViews(NULL);
  //}
}


void CTHREDDoc::OnBrushPrimitivesCube() 
{
   // create a damn box
   SetModifiedFlag();

   if (mCurrentBrush->BrushType == CThredBrush::BOX_BRUSH)
   {
      mBoxCreation.m_YSize = mCurrentBrush->BrushSpecifics.box.m_YSize;
      mBoxCreation.m_Solid = mCurrentBrush->BrushSpecifics.box.m_Solid;
      mBoxCreation.m_Thickness = mCurrentBrush->BrushSpecifics.box.m_Thickness;
      mBoxCreation.m_XSizeBot = mCurrentBrush->BrushSpecifics.box.m_XSizeBot;
      mBoxCreation.m_XSizeTop = mCurrentBrush->BrushSpecifics.box.m_XSizeTop;
      mBoxCreation.m_ZSizeBot = mCurrentBrush->BrushSpecifics.box.m_ZSizeBot;
      mBoxCreation.m_ZSizeTop = mCurrentBrush->BrushSpecifics.box.m_ZSizeTop;

      mBoxCreation.DoDialog(mCurrentBrush);
      BuildEditBox(TRUE);
   }
   else
   {
      mBoxCreation.DoDialog(mCurrentBrush);
      BuildEditBox(TRUE);
      if( mCurrentBrush->BrushType == CThredBrush::BOX_BRUSH )
         (*pWorldBsp).RebuildQuick(NULL, 0, TRUE);
   }

   UpdateAllViews(NULL);
}

void CTHREDDoc::OnBrushPrimitivesSpheroid() 
{
   SetModifiedFlag();

   // Rebuild the bsp
   //????(*pWorldBsp).RebuildQuick(NULL, 0);

   if (mCurrentBrush->BrushType == CThredBrush::SPHERE_BRUSH)
   {
      mSpheroidCreation.m_HorizontalBands = mCurrentBrush->BrushSpecifics.sphere.m_HorizontalBands;
      mSpheroidCreation.m_VerticalBands = mCurrentBrush->BrushSpecifics.sphere.m_VerticalBands;
      mSpheroidCreation.m_XSize = mCurrentBrush->BrushSpecifics.sphere.m_XSize;
      mSpheroidCreation.m_YSize = mCurrentBrush->BrushSpecifics.sphere.m_YSize;
      mSpheroidCreation.m_ZSize = mCurrentBrush->BrushSpecifics.sphere.m_ZSize;
      mSpheroidCreation.m_Solid = mCurrentBrush->BrushSpecifics.sphere.m_Solid;
      mSpheroidCreation.m_Thickness = mCurrentBrush->BrushSpecifics.sphere.m_Thickness;
      mSpheroidCreation.m_HalfSphere = mCurrentBrush->BrushSpecifics.sphere.m_HalfSphere;

      mSpheroidCreation.DoDialog(mCurrentBrush);
      BuildEditBox(TRUE);
   }
   else
   {
      mSpheroidCreation.DoDialog(mCurrentBrush);
      BuildEditBox(TRUE);
      if( mCurrentBrush->BrushType == CThredBrush::SPHERE_BRUSH )
         (*pWorldBsp).RebuildQuick(NULL, 0, TRUE);
   }

   UpdateAllViews(NULL);
}

void CTHREDDoc::OnBrushPrimitivesCylinder() 
{
   SetModifiedFlag();

   // Rebuild the bsp
   //????(*pWorldBsp).RebuildQuick(NULL, 0);

   if (mCurrentBrush->BrushType == CThredBrush::CYL_BRUSH)
   {
      mCylCreation.m_BotXOffset = mCurrentBrush->BrushSpecifics.cylinder.m_BotXOffset;
      mCylCreation.m_BotXSize = mCurrentBrush->BrushSpecifics.cylinder.m_BotXSize;
      mCylCreation.m_BotZOffset = mCurrentBrush->BrushSpecifics.cylinder.m_BotZOffset;
      mCylCreation.m_BotZSize = mCurrentBrush->BrushSpecifics.cylinder.m_BotZSize;
      mCylCreation.m_Solid = mCurrentBrush->BrushSpecifics.cylinder.m_Solid;
      mCylCreation.m_Thickness = mCurrentBrush->BrushSpecifics.cylinder.m_Thickness;
      mCylCreation.m_TopXOffset = mCurrentBrush->BrushSpecifics.cylinder.m_TopXOffset;
      mCylCreation.m_TopXSize = mCurrentBrush->BrushSpecifics.cylinder.m_TopXSize;
      mCylCreation.m_TopZOffset = mCurrentBrush->BrushSpecifics.cylinder.m_TopZOffset;
      mCylCreation.m_TopZSize = mCurrentBrush->BrushSpecifics.cylinder.m_TopZSize;
      mCylCreation.m_VerticalStripes = mCurrentBrush->BrushSpecifics.cylinder.m_VerticalStripes;
      mCylCreation.m_YSize = mCurrentBrush->BrushSpecifics.cylinder.m_YSize;
      mCylCreation.m_Ring = mCurrentBrush->BrushSpecifics.cylinder.m_Ring;

      mCylCreation.DoDialog(mCurrentBrush);
      BuildEditBox(TRUE);
   }
   else
   {
      mCylCreation.DoDialog(mCurrentBrush);
      BuildEditBox(TRUE);
      if( mCurrentBrush->BrushType == CThredBrush::CYL_BRUSH )
         (*pWorldBsp).RebuildQuick(NULL, 0, TRUE);
   }

   UpdateAllViews(NULL);
}


void CTHREDDoc::OnFileExportOptimal() 
{
   if( MessageBox( NULL, "Are you sure you wish to export?", "Zed", MB_YESNO ) == IDYES )
        DoTheExport(TRUE,FALSE);
}

void CTHREDDoc::OnFileExport() 
{
   if( MessageBox( NULL, "Are you sure you wish to export?", "Zed", MB_YESNO ) == IDYES )
      DoTheExport(FALSE,FALSE);
}

void CTHREDDoc::OnFileExportAs()
{
   DoTheExport(TRUE,TRUE);
}

void CTHREDDoc::DoTheExport( bool do_optimal, bool do_dialog )
{
   CMainFrame * pFrame = ( CMainFrame * )AfxGetMainWnd();
   
   static char szFilter[] = "ZVL Export Files (*.zvl)|*.zvl|All Files (*.*)|*.*||";

   // generate the default filename
   char NameBuf[ 1024 ];
   char NameBuf1[ 1024 ];
   
   if(strlen(theApp.mOpenFileName))
      sprintf( NameBuf1, "%s", theApp.mOpenFileName );
   else   
      sprintf( NameBuf1, "%s", ( LPCTSTR )GetTitle() );
      
   for( int loc = 0; loc < strlen( NameBuf1 ); loc++ )
   {
      if( NameBuf1[ loc ] == '.' )
      {
         NameBuf1[ loc ] = 0;
         break;
      }
   }
   sprintf( NameBuf, "%s.zvl", NameBuf1 );
         
   // check the file
   if( !CheckExport() )
      return;

   CString Filename;
   CString Base;
   
   // check if we are getting input from user   
   if( do_dialog )
   {
      // need to get the filename for this shape
      CFileDialog FileDlg( FALSE, "ZVL", NameBuf, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
         szFilter, NULL );
      if( FileDlg.DoModal() != IDOK )
         return;
      Filename = FileDlg.GetPathName();
      Base = FileDlg.GetFileTitle();
   }
   else
   {
      // get the names
      Filename = NameBuf;
      NameBuf[ strlen( NameBuf ) - 4 ] = 0;
      int j;
      for( j = strlen( NameBuf ); j; j-- )
      {
         if( NameBuf[ j - 1 ] == '\\' )
         {
            Base = &NameBuf[ j ];
            break;
         }
      }
      if( !j )
         Base = NameBuf;
   }

   // store the current info prior to exporting
   if( pActiveDetail )
      pActiveDetail->GetInfo();
   
   // create this as a volume
   VolumeRWStream  Volume;
   
   // create the volume
   Volume.createVolume( Filename );
   
   // export everything in the volume
   if( !ExportVolume( Volume, Filename, Base, do_optimal ) )
      return;
   
   // close it
   Volume.closeVolume();

   int length = Filename.GetLength();
   
	// Write out the .ZCF initialization parameters
	Filename.SetAt(length-3, 'Z');
	Filename.SetAt(length-2, 'C');
	Filename.SetAt(length-1, 'F');

	WritePrivateProfileString("Files","Palette",palName,(LPCTSTR)Filename);
	WritePrivateProfileString("Files","Material",matName,(LPCTSTR)Filename);
   WritePrivateProfileString("Preference", "PaletteVol", palVolName, (LPCTSTR)Filename);
   WritePrivateProfileString("Preference", "MaterialsVol", matVolName, (LPCTSTR)Filename);

   char tmpbuf[ 128 ];
   sprintf(tmpbuf, "%f", mGridSize);
	WritePrivateProfileString("Settings","GridSize",
			tmpbuf,(LPCTSTR)Filename);

     sprintf(tmpbuf, "%f", theApp.m_texturescale);
	WritePrivateProfileString("Settings","TextureScale",
			tmpbuf,(LPCTSTR)Filename);

     sprintf(tmpbuf, "%d", mShow);
	WritePrivateProfileString("Settings","GroupShow",
			tmpbuf,(LPCTSTR)Filename);

     sprintf(tmpbuf, "%d", mCurrentGroup);
	WritePrivateProfileString("Settings","CurrentGroup",
			tmpbuf,(LPCTSTR)Filename);

     sprintf(tmpbuf, "%lf", theApp.minintensity);
	WritePrivateProfileString("Settings","MinIntensity",
			tmpbuf,(LPCTSTR)Filename);

   sprintf(tmpbuf, "%d", pFrame->m_wndObjectView.CurrentSize.cx);
   WritePrivateProfileString("Settings","ObjectViewWidth",
   tmpbuf,(LPCTSTR)Filename);

   sprintf(tmpbuf, "%d", pFrame->m_wndObjectView.CurrentSize.cy);
   WritePrivateProfileString("Settings","ObjectViewHeight",
   tmpbuf,(LPCTSTR)Filename);


   for (int i = 0; i < TOTAL_STATES; i++)
   {
      sprintf(tmpbuf, "D%d", i);
      WritePrivateProfileString("Volumes", tmpbuf, (LPCTSTR)m_VolumeDescription[i], (LPCTSTR)Filename);
   }
   
	pFrame->UpdateGeneralStatus("Export Finished");
   
   // restore the current info
   if( pActiveDetail )
      pActiveDetail->SetInfo();
}
      

void CTHREDDoc::OnToolsUsegrid() 
{
	mUseGrid ^= 1;
	UpdateGridInformation();
	UpdateAllViews(NULL);
}
void CTHREDDoc::OnUpdateToolsUsegrid(CCmdUI* pCmdUI) 
{
	if(mUseGrid)
		pCmdUI->SetCheck();
	else
		pCmdUI->SetCheck(0);
}

void CTHREDDoc::OnToolsGridsettings() 
{
	CGridSizeDialog dlg;

	dlg.m_UseRotationSnap = mUseRotationSnap;
	dlg.m_SnapDegrees = (mRotationSnapDegrees/(double)M_TOTAL_DEGREES)*360;
	dlg.m_ShowCoarse = mDrawCoarse;
	dlg.m_ShowFine = mDrawFine;
	dlg.m_GridSize = mGridSize;
	dlg.m_gridtype = mGridType;
	if(dlg.DoModal() == IDOK)
    {
		mGridSize = dlg.m_GridSize;
		mDrawCoarse = dlg.m_ShowCoarse;
		mDrawFine = dlg.m_ShowFine;
		mUseRotationSnap = dlg.m_UseRotationSnap;
		mRotationSnapDegrees = (dlg.m_SnapDegrees/(double)360) * (double)M_TOTAL_DEGREES;
		mGridType = dlg.m_gridtype;
	}

	UpdateGridInformation();
	UpdateAllViews(NULL);
}

void CTHREDDoc::OnEditUndo() 
{
  CThredBrush*  brush;
  int           brushid;
  int           whichbrush;
  UndoInfo      temp;
  int           top = UndoArray.ElementAt(0).brushes.GetUpperBound();
  int           undolistsize = UndoArray.GetUpperBound();

  if (mCurrentUndoLevel < 1)
  {
    return;
  }

  for (whichbrush = 0; whichbrush <= top; whichbrush++)
  {
    brushid = UndoArray.ElementAt(0).brushes.ElementAt(0);
    UndoArray.ElementAt(0).brushes.RemoveAt(0);

    if (brushid == -1)
    {
      brush = mCurrentBrush;
    } else {
      brush = mBrushList;
    }

    while (brush)
    {
      if (brush->mBrushId == brushid)
      {
        // Build up redo stack entry...
        temp.matrix.InsertAt(0, brush->mTotalMatrix);
        temp.brushes.InsertAt(0, brushid);

        // Undo the last matrix here...
        brush->mTotalMatrix = UndoArray.ElementAt(0).matrix.ElementAt(0);
      }

      brush = brush->GetNextBrush();
    }

    UndoArray.ElementAt(0).matrix.RemoveAt(0);
  }

  UndoArray.RemoveAt(0);

  // Place info onto redo stack
  RedoArray.InsertAt(0, temp);
  mCanRedo = TRUE;

  if (mCurrentUndoLevel > 0)
  {
    mCurrentUndoLevel--;
  }

  if (mCurrentRedoLevel < NUM_UNDOS)
  {
    mCurrentRedoLevel++;
  }

  if (mCurrentUndoLevel == 0)
  {
    mCanUndo = FALSE;
  }

  BuildEditBox(FALSE);
  UpdateAllViews(NULL);
}

void CTHREDDoc::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	if(CanUndo())
		pCmdUI->Enable();
	else
		pCmdUI->Enable(0);
}

void CTHREDDoc::OnEditRedo() 
{
  CThredBrush*  brush;
  int           brushid;
  int           whichbrush;
  UndoInfo      temp;
  int           top = RedoArray.ElementAt(0).brushes.GetUpperBound();

  if (mCurrentRedoLevel < 0)
  {
    return;
  }

  for (whichbrush = 0; whichbrush <= top; whichbrush++)
  {
    brushid = RedoArray.ElementAt(0).brushes.ElementAt(0);
    RedoArray.ElementAt(0).brushes.RemoveAt(0);

    if (brushid == -1)
    {
      brush = mCurrentBrush;
    } else {
      brush = mBrushList;
    }

    while (brush)
    {
      if (brush->mBrushId == brushid)
      {
        // Build up undo stack entry...
        temp.matrix.InsertAt(0, brush->mTotalMatrix);
        temp.brushes.InsertAt(0, brushid);

        // Redo the last matrix here...
        brush->mTotalMatrix = RedoArray.ElementAt(0).matrix.ElementAt(0);
      }

      brush = brush->GetNextBrush();
    }

    RedoArray.ElementAt(0).matrix.RemoveAt(0);
  }

  RedoArray.RemoveAt(0);

  // Place info onto redo stack
  UndoArray.InsertAt(0, temp);
  mCanUndo = TRUE;

  if (mCurrentUndoLevel < NUM_UNDOS)
  {
    mCurrentUndoLevel++;
  }

  if (mCurrentRedoLevel > 0)
  {
    mCurrentRedoLevel--;
  }

  if (mCurrentRedoLevel == 0)
  {
    mCanRedo = FALSE;
  }

  BuildEditBox(FALSE);
  UpdateAllViews(NULL);
}

void CTHREDDoc::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
	if(CanRedo())
		pCmdUI->Enable();
	else
		pCmdUI->Enable(0);

}

void CTHREDDoc::OnUpdateLightLockmove(CCmdUI* pCmdUI) 
{
//   if( CLightState::mLockLightPosition )
//      pCmdUI->SetCheck();
//   else
//      pCmdUI->SetCheck( 0 );
}

void CTHREDDoc::OnLightLockmove() 
{
//   CLightState::mLockLightPosition ^= 1;
}


//==========================================================
// This function will bring up the bsp manager dialog and
//==========================================================
void CTHREDDoc::OnToolsBspmanager() 
{
	SetModifiedFlag();
	(*pWorldBsp).BspManager();
	UpdateAllViews(NULL);
}

void CTHREDDoc::OnEntitiesEditor() 
{
	CEntitiesDialog Dialog;
    Dialog.min_intensity = theApp.minintensity;
	mCurrentEntity = Dialog.EditEntity((*pEntityArray), mCurrentEntity, this);
    theApp.minintensity = Dialog.min_intensity;
	UpdateEntityOrigins();
	UpdateBrushInformation();
	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void CTHREDDoc::OnEntitiesPlace() 
{
	// get a new entity
	CEntity NewEnt;

	// add in some key/value's
	NewEnt.mKeyArray.Add("classname");
	NewEnt.mValueArray.Add("light");

	NewEnt.mKeyArray.Add("State");
	NewEnt.mValueArray.Add(" ");

    // make an origin
	NewEnt.mOrigin.X = 1;
	NewEnt.mOrigin.Y = 1;
	NewEnt.mOrigin.Z = 1;
	NewEnt.mKeyArray.Add("pos");
	NewEnt.mValueArray.Add("0 0 0");

	NewEnt.mKeyArray.Add("color");
	NewEnt.mValueArray.Add("0.7 0.7 0.7");

	NewEnt.mKeyArray.Add("distance");
	NewEnt.mValueArray.Add("0.0 0.0007 0.0");

	NewEnt.mGroup = mCurrentGroup;
	NewEnt.UpdateOrigin();

    //NewEnt.mFlags |= ENTITY_ACTIVE;

	// add to the end
	mCurrentEntity = (*pEntityArray).Add(NewEnt);

	// update zee information.
	UpdateBrushInformation();
	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void CShapeDetail::ConvertEntities( void )
{
   MessageBox( NULL, "Feature removed for now... talk to JohnF if you need this.", "Notice", MB_OK );
   return;
/*
   CString  value;

   for( int count = 0; count < mEntityArray.GetSize(); count++ )
   {
      // get the classname
      value = mEntityArray[count].ValueByKey( "classname" );
      
      if( !value.CompareNoCase( "light" ) ) 
      {
         CLight   NewLight;
         
         if( pActiveLightSet )
            NewLight.flags |= LIGHT_INLIGHTSET;
         else
            NewLight.flags &= ~LIGHT_INLIGHTSET;
         
         // check for all the options
         value = mEntityArray[count].ValueByKey( "pos" );
         if( value.GetLength() )
         {
            int tmp1, tmp2, tmp3;
            if( sscanf( value, "%d %d %d", &tmp1, &tmp2, &tmp3 ) != 3 )
               return;
            NewLight.mDefaultState.mFlags |= STATE_HASPOSITION;
            NewLight.mFlags |= LIGHT_HASDEFAULTPOSITION;
            NewLight.mDefaultState.mPosition.X = tmp1;
            NewLight.mDefaultState.mPosition.Y = tmp2;
            NewLight.mDefaultState.mPosition.Z = tmp3;
            
            NewLight.mCurrentPosition = NewLight.mDefaultState.mPosition;
            NewLight.mMovePosition = NewLight.mCurrentPosition;
         }
         value = mEntityArray[count].ValueByKey( "color" );
         if( value.GetLength() )
         {
            float tmp1, tmp2, tmp3;
            NewLight.mDefaultState.mFlags |= STATE_HASCOLOR;
            NewLight.mFlags |= LIGHT_HASDEFAULTCOLOR;
            
            if( sscanf( value, "%f %f %f", &tmp1, &tmp2, &tmp3 ) != 3 )
               return;
            NewLight.mDefaultState.mColor.set( tmp1, tmp2, tmp3 );
         }
         value = mEntityArray[count].ValueByKey( "linear" );
         if( value.GetLength() )
         {
            int tmp1, tmp2;
            if( sscanf( value, "%d %d", &tmp1, &tmp2 ) != 2 )
               return;
               
            NewLight.mDefaultState.mFlags |= STATE_LINEAR_FALLOFF;
            NewLight.mDefaultState.mFlags &= ~STATE_DISTANCE_FALLOFF;
            NewLight.mFlags |= LIGHT_HASDEFAULTLINEARFALLOFF;
            
            NewLight.mDefaultState.mFalloff1 = tmp1;
            NewLight.mDefaultState.mFalloff2 = tmp2;
         }
         value = mEntityArray[count].ValueByKey( "distance" );
         if( value.GetLength() )
         {
            int tmp1, tmp2, tmp3;
            if( sscanf( value, "%d %d %d", &tmp1, &tmp2, &tmp3 ) != 3 )
               return;

            NewLight.mDefaultState.mFlags &= ~STATE_LINEAR_FALLOFF;
            NewLight.mDefaultState.mFlags |= STATE_DISTANCE_FALLOFF;
            NewLight.mFlags |= LIGHT_HASDEFAULTDISTANCEFALLOFF;
            
            NewLight.mDefaultState.mFalloff1 = tmp1;
            NewLight.mDefaultState.mFalloff2 = tmp2;
         }

         value = mEntityArray[count].ValueByKey( "State" );
         if( value.GetLength() )
         {
            NewLight.mName = value;
            NewLight.mFlags |= LIGHT_HASNAME;
         }

         // add the light
         mLightArray.Add( NewLight );

         // remove this entity
	      mEntityArray.RemoveAt( count );

         count--;
      }
   }
*/
}

void CTHREDDoc::OnEntitiesShow() 
{
	mShowEntities ^= 1;
	UpdateBrushInformation();
	UpdateAllViews(NULL);
}

void CTHREDDoc::OnEntitiesConvert()
{
   // convert them 
   if( pActiveDetail )
      pActiveDetail->ConvertEntities();
   UpdateBrushInformation();
   UpdateAllViews(NULL);
}

void CTHREDDoc::OnEntitiesShow3D() 
{
	mShowEntities3D ^= 1;
	UpdateBrushInformation();
	UpdateAllViews(NULL);
}

void CTHREDDoc::OnLightsShow3D() 
{
	mShowLights3D ^= 1;
	UpdateBrushInformation();
	UpdateAllViews(NULL);
}

void CTHREDDoc::OnUpdateEntitiesShow(CCmdUI* pCmdUI) 
{
	if(mShowEntities)
		pCmdUI->SetCheck();
	else
		pCmdUI->SetCheck(0);
}

void CTHREDDoc::OnUpdateEntitiesShow3D(CCmdUI* pCmdUI) 
{
	if(mShowEntities3D)
		pCmdUI->SetCheck();
	else
		pCmdUI->SetCheck(0);
}

// bring up the editor.
void CTHREDDoc::OnBrushGroupeditor() 
{
	CBrushGroupDialog dlg;
	mCurrentGroup = dlg.DoDialog((*pBrushGroupArray), mCurrentGroup, mBrushList, this);
	UpdateBrushInformation();
	UpdateAllViews(NULL);
	SetModifiedFlag();
}



void CTHREDDoc::OnViewShowAllbrushes() 
{
	mShow = ID_VIEW_SHOW_ALLBRUSHES;
	UpdateActiveBrushes();	
	UpdateAllViews(NULL);
}

void CTHREDDoc::OnViewShowCurrentgroup() 
{
	mShow = ID_VIEW_SHOW_CURRENTGROUP;
	UpdateActiveBrushes();	
	UpdateAllViews(NULL);
}

void CTHREDDoc::OnViewShowVisiblebrushes() 
{
	mShow = ID_VIEW_SHOW_VISIBLEBRUSHES;
	UpdateActiveBrushes();	
	UpdateAllViews(NULL);
}

void CTHREDDoc::OnUpdateViewShowVisiblebrushes(CCmdUI* pCmdUI) 
{
	if(mShow == (int)pCmdUI->m_nID)
		pCmdUI->SetCheck();
	else
		pCmdUI->SetCheck(0);
}

void CTHREDDoc::OnUpdateViewShowAllbrushes(CCmdUI* pCmdUI) 
{
	if(mShow == (int)pCmdUI->m_nID)
		pCmdUI->SetCheck();
	else
		pCmdUI->SetCheck(0);
}

void CTHREDDoc::OnUpdateViewShowCurrentgroup(CCmdUI* pCmdUI) 
{
	if(mShow == (int)pCmdUI->m_nID)
		pCmdUI->SetCheck();
	else
		pCmdUI->SetCheck(0);
}

void CTHREDDoc::RenderBrush(CRenderCamera& Camera, CDC* pDC, int ViewType, double ZoomFactor, TSRenderContext* rc, bool showbb)
{
    CThredBrush*  brush;

    brush = mCurrentBrush;

    if (brush == &mRegularBrush)
    {
        brush->RenderBrush(Camera, color_def_brush, color_def_point, color_def_brush, color_bbox, rc, showbb);
    } else {
      while (brush)
      {
        brush->RenderBrush(Camera, color_def_brush, color_def_point, color_select, color_bbox, rc, showbb);
        brush = brush->NextSelectedBrush;
      }
    }

	//mCurrentBrush->RenderBrush(Camera, color_def_brush, color_def_point, color_select, color_bbox, rc, showbb);
}

int CTHREDDoc::GetBrushListCount()
{
	int Count=0;
	CThredBrush* Brush;
	Brush = mBrushList;
	while(Brush) {
		Count++;
		Brush = Brush->GetNextBrush();
	}
	return Count;
}
// go through and delete the brush list.
void CTHREDDoc::DeleteBrushList()
{
	CThredBrush* Brush;
	CThredBrush* NextBrush;

	Brush = mBrushList;
	while(Brush) {
		NextBrush = Brush->GetNextBrush();
		delete Brush;
		Brush = NextBrush;
	}
}


void CTHREDDoc::RenderSolidWorld(CRenderCamera* Camera, TSRenderContext* rc)
{
	(*pWorldBsp).RenderSolidWorld(Camera, rc, TEXTURE_NONE, this);
	Camera->DrawOrthoGrid(mGridType, line_grid_color, dot_grid_color, color_origin, rc);
}


int CTHREDDoc::CanUndo()
{
	// can we undo?
    return mCanUndo;
	//return mCurrentBrush->CanUndo();	
}

int CTHREDDoc::CanRedo()
{
    return mCanRedo;
	//return mCurrentBrush->CanRedo();	
}

void CTHREDDoc::UpdateEntityOrigins()
{
	for(int Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++)
		(*pEntityArray)[Ent].UpdateOrigin();

	//UpdateBrushInformation();
}
//===============================================================
// render the entities
//===============================================================
void CTHREDDoc::RenderEntities(CRenderCamera* Camera, int ViewType, double mZoomFactor, TSRenderContext* rc)
{
   if( ( ( ViewType == ID_VIEW_SOLIDRENDER ) || ( ViewType == ID_VIEW_TEXTUREVIEW ) ) && 
      !( mShowEntities3D ) )
      return;
      
	// go through the ent's
	for(int Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++)
    {
		//if(Ent == mCurrentEntity)
		if((*pEntityArray)[Ent].mFlags & ENTITY_SELECTED)
        {
			// DPW - Needed for TS compatibility
			(*pEntityArray)[Ent].RenderEntity(Camera, color_select, rc, ViewType);
        } else {
			// DPW - Needed for TS compatibility
			(*pEntityArray)[Ent].RenderEntity(Camera, color_def_entity, rc, ViewType);
        }
	}
}

void CTHREDDoc::MoveEntity(double X, double Y, double Z)
{
  for (int Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++)
  {
    if ((*pEntityArray)[Ent].mFlags & ENTITY_SELECTED)
    {
	  (*pEntityArray)[Ent].MoveEntity(X, Y, Z);
    }
  }

  // set the mod flag
  SetModifiedFlag();
}

#define DISTANCE_THRESHHOLD (double)8.0
CEntity * CTHREDDoc::SelectEntity(CPoint point, CRenderCamera& Camera, 
   BOOL fClear )
{
	// distance
	double ShortestDistance = 200000.0;
	double CurrentDistance;
	int CurrentEnt, Ent;
	ThredPoint MousePoint;
	
   // check if should clear the flags
   if( fClear )
   {
      // Deselect all other entities...
      mCurrentEntity = -1;
      mSelectedEntity = FALSE;
	   for(Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++)
      {
         (*pEntityArray)[Ent].mFlags &= ~ENTITY_SELECTED;
      }
   }

	MousePoint.X = MousePoint.Y = MousePoint.Z = 0;
	Camera.TranslateToWorld(point, MousePoint);

	// go through the array & check for hits
	for(Ent = (*pEntityArray).GetSize() - 1; Ent >= 0; Ent -- )
   //( Ent < (*pEntityArray).GetSize(); Ent++)
    {
		CurrentDistance = (*pEntityArray)[Ent].GetDistance(MousePoint);

		if(CurrentDistance < ShortestDistance)
        {
			ShortestDistance = CurrentDistance;
			CurrentEnt = Ent;
		}
	}
	
	// check if we should assign
	if(ShortestDistance <= DISTANCE_THRESHHOLD)
   {
      if( !( (*pEntityArray)[CurrentEnt].mFlags & ENTITY_ACTIVE ) )
         return( NULL );
         
      if( fClear )
      {
         (*pEntityArray)[CurrentEnt].mFlags |= ENTITY_SELECTED;
         mCurrentEntity = CurrentEnt;
         mSelectedEntity = TRUE;
      }

      // returns the entity for the new entity selection
      return( &(*pEntityArray)[ CurrentEnt ] );
   }

   return( 0 );
}

// get the index and set the entity as the current one
int CTHREDDoc::SetCurrentEntity( CEntity * pEntity )
{
   for( int Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++ )
   {
      if( pEntity == &(*pEntityArray)[ Ent ] )
      {
         pEntity->mFlags |= ENTITY_SELECTED;
         mCurrentEntity = Ent;
         mSelectedEntity = TRUE;
         return( Ent );
      }
   }
   return( -1 );
}

// set the current light and return the index
int CTHREDDoc::SetCurrentLight( CLightEmitter * pEmitter )
{
   pEmitter->select( true );
   mSelectedLight = TRUE;

   for( int light = 0; light < (*pLightArray).GetSize(); light++ )
   {
      CLightState * pLightState = ( CLightState * )pEmitter->getParent();
      CLight * pLight = ( CLight * )pLightState->getParent();
      if( pLight == &(*pLightArray)[light] )
         mCurrentLight = light;
   }
   // index really doesnt matter.. so return -1 for fun!
   return( mCurrentLight );
}

void CTHREDDoc::ResetSelectedEntities( void )
{
   for( int Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++ )
      (*pEntityArray)[ Ent ].mFlags &= ~ENTITY_SELECTED;
   mCurrentEntity = -1;
   mSelectedEntity = FALSE;
}

void CTHREDDoc::ResetSelectedLights( void )
{
   for( int Light = 0; Light < (*pLightArray).GetSize(); Light++ )
      (*pLightArray)[ Light ].resetSelected();
   mCurrentLight = -1;
   mSelectedLight = FALSE;
}

void CTHREDDoc::RenderOrthoView(int mViewType, CRenderCamera* Camera, TSPerspectiveCamera* ts_cam, TSRenderContext* rc, BOOL showselectrect, CRect* srect)
{
	CThredBrush*  CurrentBrush;
    CPoint        tpoint;

	// draw zee grid
	Camera->DrawOrthoGrid(mGridType, line_grid_color, dot_grid_color, color_origin, rc);

	// draw the brushes.
	CurrentBrush = mBrushList;
	while(CurrentBrush)
	{
		// check/draw
		if(CurrentBrush != mCurrentBrush)
		{
			// Color based on transaction type
			if (CurrentBrush->mTransactionType == BRUSH_ADD_TO_WORLD)
            {
              if (CurrentBrush->volumestate)
              {
                if (mVolumeShow)
                {
				  CurrentBrush->RenderBrushStandard(*Camera, volume_color, color_select, rc);
                }
              } else {
				CurrentBrush->RenderBrushStandard(*Camera, color_plus, color_select, rc);
              }
            } else {
              if (CurrentBrush->volumestate)
              {
                if (mVolumeShow)
                {
				  CurrentBrush->RenderBrushStandard(*Camera, volume_color, color_select, rc);
                }
              } else {
				CurrentBrush->RenderBrushStandard(*Camera, color_minus, color_select, rc);
              }
            }
		}

		// next brush
		CurrentBrush = CurrentBrush->GetNextBrush();
	}

    if (showselectrect)
    {
      Camera->DrawRect(rc, srect, color_select);
    }

    // Draw the EditBox with proper grab points
    switch (mViewType)
    {
		case ID_VIEW_TOPVIEW:
          switch(edit_state)
          {
          case ES_RESIZE_MOVE:
            Camera->DrawWireframePolygon(&(EditBox.TopPolygon), color_bbox, color_def_point, rc, TRUE, TRUE);
            //Camera->TranslateToScreen(tpoint, EditBox.mCenter, rc);
            //Camera->DrawPoint(tpoint, color_def_point, rc);
            break;
          case ES_ROTATE:
            Camera->DrawWireframePolygon(&(EditBox.TopPolygon), color_bbox, color_def_point, rc, TRUE, FALSE);
            Camera->TranslateToScreen(tpoint, EditBox.mCenter, rc);
            Camera->DrawPoint(tpoint, color_def_point, rc);
            break;
          case ES_SHEAR:
            Camera->DrawWireframePolygon(&(EditBox.TopPolygon), color_bbox, color_def_point, rc, FALSE, TRUE);
            //Camera->TranslateToScreen(tpoint, EditBox.mCenter, rc);
            //Camera->DrawPoint(tpoint, color_def_point, rc);
            break;
          }
		  break;
		case ID_VIEW_SIDEVIEW:
          switch(edit_state)
          {
          case ES_RESIZE_MOVE:
            Camera->DrawWireframePolygon(&(EditBox.SidePolygon), color_bbox, color_def_point, rc, TRUE, TRUE);
            //Camera->TranslateToScreen(tpoint, EditBox.mCenter, rc);
            //Camera->DrawPoint(tpoint, color_def_point, rc);
            break;
          case ES_ROTATE:
            Camera->DrawWireframePolygon(&(EditBox.SidePolygon), color_bbox, color_def_point, rc, TRUE, FALSE);
            Camera->TranslateToScreen(tpoint, EditBox.mCenter, rc);
            Camera->DrawPoint(tpoint, color_def_point, rc);
            break;
          case ES_SHEAR:
            Camera->DrawWireframePolygon(&(EditBox.SidePolygon), color_bbox, color_def_point, rc, FALSE, TRUE);
            //Camera->TranslateToScreen(tpoint, EditBox.mCenter, rc);
            //Camera->DrawPoint(tpoint, color_def_point, rc);
            break;
          }
			break;
		case ID_VIEW_FRONTVIEW:
          switch(edit_state)
          {
          case ES_RESIZE_MOVE:
            Camera->DrawWireframePolygon(&(EditBox.FrontPolygon), color_bbox, color_def_point, rc, TRUE, TRUE);
            //Camera->TranslateToScreen(tpoint, EditBox.mCenter, rc);
            //Camera->DrawPoint(tpoint, color_def_point, rc);
            break;
          case ES_ROTATE:
            Camera->DrawWireframePolygon(&(EditBox.FrontPolygon), color_bbox, color_def_point, rc, TRUE, FALSE);
            Camera->TranslateToScreen(tpoint, EditBox.mCenter, rc);
            Camera->DrawPoint(tpoint, color_def_point, rc);
            break;
          case ES_SHEAR:
            Camera->DrawWireframePolygon(&(EditBox.FrontPolygon), color_bbox, color_def_point, rc, FALSE, TRUE);
            //Camera->TranslateToScreen(tpoint, EditBox.mCenter, rc);
            //Camera->DrawPoint(tpoint, color_def_point, rc);
            break;
          }
			break;
          break;
    }
}

#define CENTLIST "Class CEntList"
#define CCURRENTENT "CurEnt"
#define CENTLISTCOUNT "EntCount"
#define CENDENTLIST "End CEntList"

void CTHREDDoc::SerializeEntities(CThredParser &Parser)
{
	int Count;
	//CThredParser Parser(&ar, CENDENTLIST);
	
	Parser.SetEnd(CENDENTLIST);
	if(Parser.IsStoring) {
		Count = (*pEntityArray).GetSize();
		// Write out our ID string
		Parser.WriteString(CENTLIST);
		Parser.WriteInt(CENTLISTCOUNT, Count);
		Parser.WriteInt(CCURRENTENT, mCurrentEntity);

		for(int Ent = 0; Ent < Count; Ent++) 
				(*pEntityArray)[Ent].Serialize(Parser);
		Parser.WriteString(CENDENTLIST);
	}
	else {
		// we don't actually NEED an brushlist
		if(!Parser.GetKey(CENTLIST)) 
			return;

		// get the count
		if(!Parser.GetKeyInt(CENTLISTCOUNT, Count)) {
			AfxThrowArchiveException(CArchiveException::generic, NULL);
			return;
		}

		// get our current
		Parser.GetKeyInt(CCURRENTENT, mCurrentEntity);

		// go through and load
		while(Count--) {
			CEntity Ent;
			Ent.Serialize(Parser);
			(*pEntityArray).Add(Ent);
		}
		// update man... update.
		UpdateEntityOrigins();
	}

}

#define CLIGHTLIST      "Class CLightList"
#define CCURRENTLIGHT   "CurLight"
#define CLIGHTLISTCOUNT "LightCount"
#define CENDLIGHTLIST   "End CLightList"

void CTHREDDoc::SerializeLights( CThredParser &Parser )
{
	int Count;
	
	Parser.SetEnd( CENDLIGHTLIST );
	if( Parser.IsStoring ) 
   {
		Count = (*pLightArray).GetSize();
      
		// Write out our ID string
		Parser.WriteString( CLIGHTLIST );
		Parser.WriteInt( CLIGHTLISTCOUNT, Count );
		Parser.WriteInt( CCURRENTLIGHT, mCurrentLight );

		for( int Light = 0; Light < Count; Light++ )
		   (*pLightArray)[ Light ].Serialize( Parser );
		Parser.WriteString(CENDLIGHTLIST);
	}
	else {
		if(!Parser.GetKey( CLIGHTLIST ) ) 
			return;

		// get the count
		if( !Parser.GetKeyInt( CLIGHTLISTCOUNT, Count) ) 
      {
			AfxThrowArchiveException( CArchiveException::generic, NULL );
			return;
		}

		// get our current
		Parser.GetKeyInt( CCURRENTLIGHT, mCurrentLight );

		// go through and load
		while( Count-- )
      {
			CLight Light( false );
			Light.Serialize( Parser );
			(*pLightArray).Add( Light );
		}
	}

}


#define CGROUPLIST "Class CGroupList"
#define CCURRENTGROUP "CurGroup"
#define CGROUPLISTCOUNT "GroupCount"
#define CENDGROUPLIST "End CGroupList"
void CTHREDDoc::SerializeGroups(CThredParser& Parser)
{
	int Count;
	//CThredParser Parser(&ar, CENDGROUPLIST);
	Parser.SetEnd(CENDGROUPLIST);
			
	if(Parser.IsStoring) {
		Count = (*pBrushGroupArray).GetSize();
		// Write out our ID string
		Parser.WriteString(CGROUPLIST);
		Parser.WriteInt(CCURRENTGROUP, mCurrentGroup);
		Parser.WriteInt(CGROUPLISTCOUNT, Count);

		for(int Group = 0; Group < Count; Group++) 
			(*pBrushGroupArray)[Group].Serialize(Parser);
		Parser.WriteString(CENDGROUPLIST);
	}
	else {
		// we don't actually NEED an brushlist
		if(!Parser.GetKey(CGROUPLIST)) 
			return;

		// get our current
		Parser.GetKeyInt(CCURRENTGROUP, mCurrentGroup);

		// get the count
		if(!Parser.GetKeyInt(CGROUPLISTCOUNT, Count)) {
			AfxThrowArchiveException(CArchiveException::generic, NULL);
			return;
		}

		// go through and load
		while(Count--) {
			CBrushGroup Group;
			Group.Serialize(Parser);
			(*pBrushGroupArray).Add(Group);
		}
	}

}



// We put ourselves in adjustment mode.  This means that our
// current brush becomes another brush that was previously put
// down.  They hit add/subtract brush to bring themselves
// out of this mode.
void CTHREDDoc::OnBrushAdjustmentmode() 
{
	mCurrentTool = ID_BRUSH_ADJUSTMENTMODE;
}

void CTHREDDoc::OnUpdateBrushAdjustmentmode(CCmdUI* pCmdUI) 
{
	if(mCurrentTool == ID_BRUSH_ADJUSTMENTMODE)
		pCmdUI->SetCheck();
	else
		pCmdUI->SetCheck(0);
}

void CTHREDDoc::SelectABrushIndependent(CPoint point, CRenderCamera& Camera)
{
	ThredPoint WorldPoint;
	double SmallestVolume;
	CThredBrush* BestBrush = NULL;

    // Try to select an entity first,
    // then try to select a brush if no entity was selected
    if (SelectEntity(point, Camera, TRUE ))
    {
      return;
    }

	// get world coords for the b-box
	Camera.TranslateToWorld(point, WorldPoint);

	// go through the brushes
	CThredBrush* CurrentBrush = mBrushList;

	// check for intersections
	while(CurrentBrush)
    {
		if(CurrentBrush->mFlags & BRUSH_ACTIVE)
        {
			CurrentBrush->TransformBrushPolygons();
			if(CurrentBrush->CheckPoint(WorldPoint))
            {
				if((!BestBrush) || (CurrentBrush->mBoundingBox.GetVolume() < SmallestVolume))
                {
					BestBrush = CurrentBrush;
					SmallestVolume = CurrentBrush->mBoundingBox.GetVolume();
				}
			}
		}
		CurrentBrush = CurrentBrush->GetNextBrush();
	}

	// if we have a new good brush
	if(BestBrush)
    {
		// force the brush on
		mShowBrush = TRUE;
		mAdjustmentMode = 1;

        // DPW - Toggle state of brush selection
        if (BestBrush->mFlags & BRUSH_SELECTED)
        {
          // Remove brush from list of selected brushes
          BestBrush->mFlags &= ~BRUSH_SELECTED;

          // Loop through the list of brushes and pull this one out of the list
          CThredBrush*  brush;
          CThredBrush*  lastbrush;

          lastbrush = NULL;
          brush = mCurrentBrush;

          while (brush)
          {
            if (brush->mBrushId == BestBrush->mBrushId)
            {
              // Found the deselected brush in list
              if (lastbrush)
              {
                lastbrush->NextSelectedBrush = brush->NextSelectedBrush;
                brush->PrevSelectedBrush = lastbrush;
              } else {
                mCurrentBrush = mCurrentBrush->NextSelectedBrush;
              }

              if (mCurrentBrush == NULL)
              {
                mCurrentBrush = &mRegularBrush;
              }

              brush->NextSelectedBrush = NULL;
              brush->PrevSelectedBrush = NULL;
              brush = NULL;
            } else {
              lastbrush = brush;
              brush = brush->NextSelectedBrush;
            }
          }
        } else {
          // Add brush to list of selected brushes
          BestBrush->mFlags |= BRUSH_SELECTED;

          if (mCurrentBrush != &mRegularBrush)
          {
            // Not pointing to the default brush, so need to add this brush to list
            // of brushes in a potential group...
            CThredBrush*  brush;
            CThredBrush*  lastbrush = NULL;
            BOOL          added = FALSE;

            brush = mCurrentBrush;

            while (brush)
            {
              // Add this brush to its proper place in order in the list
              if (brush->mBrushId < BestBrush->mBrushId)
              {
                // Go to next brush in list
                lastbrush = brush;
                brush = brush->NextSelectedBrush;
              } else {
                // Add BestBrush here
                BestBrush->NextSelectedBrush = brush;

                if (mCurrentBrush == brush)
                {
                  mCurrentBrush = BestBrush;
                  BestBrush->PrevSelectedBrush = NULL;
                } else {
                  if (lastbrush)
                  {
                    BestBrush->PrevSelectedBrush = lastbrush;
                    lastbrush->NextSelectedBrush = BestBrush;
                  }
                }

                // End the loop
                added = TRUE;
                brush = NULL;
              }
            }

            if (!added)
            {
              lastbrush->NextSelectedBrush = BestBrush;
              BestBrush->NextSelectedBrush = NULL;
              BestBrush->PrevSelectedBrush = lastbrush;
            }

          } else {
            // This is the only selected brush, so make it the current one
            BestBrush->NextSelectedBrush = NULL;
            BestBrush->PrevSelectedBrush = NULL;
		    mCurrentBrush = BestBrush;
          }
        }

        //mCurrentBrush = BestBrush;
		//mCurrentTool = ID_TOOLS_BRUSH_MOVEROTATEBRUSH;
	}

    // Build EditBox bounding box
    BuildEditBox(TRUE);

    Point3F           brush_size;
    brush_size.x = (mCurrentBrush->mBoundingBox.mLargest.X - mCurrentBrush->mBoundingBox.mSmallest.X);
    brush_size.y = (mCurrentBrush->mBoundingBox.mLargest.Y - mCurrentBrush->mBoundingBox.mSmallest.Y);
    brush_size.z = (mCurrentBrush->mBoundingBox.mLargest.Z - mCurrentBrush->mBoundingBox.mSmallest.Z);
    //UpdateBrushInfo(brush_size, mCurrentBrush->mBrushPosition[0]);
}

void CTHREDDoc::SelectABrush(CPoint point, CRenderCamera& Camera)
{
	ThredPoint WorldPoint;
	double SmallestVolume;
	CThredBrush* BestBrush = NULL;

    // Try to select an entity first,
    // then try to select a brush if no entity was selected
    if (SelectEntity(point, Camera, TRUE ))
    {
      return;
    }

	// get world coords for the b-box
	Camera.TranslateToWorld(point, WorldPoint);

	// go through the brushes
	CThredBrush* CurrentBrush = mBrushList;

	// check for intersections
	while(CurrentBrush)
    {
		if(CurrentBrush->mFlags & BRUSH_ACTIVE)
        {
			CurrentBrush->TransformBrushPolygons();
			if(CurrentBrush->CheckPoint(WorldPoint))
            {
				if((!BestBrush) || (CurrentBrush->mBoundingBox.GetVolume() < SmallestVolume))
                {
					BestBrush = CurrentBrush;
					SmallestVolume = CurrentBrush->mBoundingBox.GetVolume();
				}
			}
		}
		CurrentBrush = CurrentBrush->GetNextBrush();
	}

	// if we have a new good brush
	if(BestBrush)
    {
		// force the brush on
		mShowBrush = TRUE;
		mAdjustmentMode = 1;
		mCurrentBrush = BestBrush;
        //BestBrush->NextSelectedBrush = NULL;

        CThredBrush*  brush = mBrushList;

        // Deselect everyone...
        while (brush)
        {
          brush->NextSelectedBrush = NULL;
          brush->PrevSelectedBrush = NULL;
          brush->mFlags &= ~BRUSH_SELECTED; // DPW

          brush = brush->GetNextBrush();
        }

        // reset the lists
        ResetSelectedLights();
        ResetSelectedEntities();
        
        mCurrentEntity = -1;
        mSelectedEntity = FALSE;
        for(int Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++)
   		  (*pEntityArray)[Ent].mFlags &= ~ENTITY_SELECTED;

         // deselect each light also
         mCurrentLight = -1;
         mSelectedLight = FALSE;
         for( int Light = 0; Light < (*pLightArray).GetSize(); Light++ )
            (*pLightArray)[ Light ].resetSelected();
            
        BestBrush->mFlags |= BRUSH_SELECTED; // DPW
	}

    // Build EditBox bounding box
    BuildEditBox(FALSE);

    Point3F           brush_size;

    brush_size.x = (mCurrentBrush->mBoundingBox.mLargest.X - mCurrentBrush->mBoundingBox.mSmallest.X);
    brush_size.y = (mCurrentBrush->mBoundingBox.mLargest.Y - mCurrentBrush->mBoundingBox.mSmallest.Y);
    brush_size.z = (mCurrentBrush->mBoundingBox.mLargest.Z - mCurrentBrush->mBoundingBox.mSmallest.Z);
    //UpdateBrushInfo(brush_size, mCurrentBrush->mBrushPosition[0]);
}


void CTHREDDoc::ForceBrushSelection(CThredBrush* brush)
{
	// if we have a new good brush
	if(brush)
    {
		// force the brush on
		mShowBrush = TRUE;
		mAdjustmentMode = 1;
		mCurrentBrush = brush;
		//mCurrentTool = ID_TOOLS_BRUSH_MOVEROTATEBRUSH;
        brush->mFlags |= BRUSH_SELECTED;
	}

    // Build EditBox bounding box
    BuildEditBox(FALSE);

    Point3F           brush_size;

    brush_size.x = (mCurrentBrush->mBoundingBox.mLargest.X - mCurrentBrush->mBoundingBox.mSmallest.X);
    brush_size.y = (mCurrentBrush->mBoundingBox.mLargest.Y - mCurrentBrush->mBoundingBox.mSmallest.Y);
    brush_size.z = (mCurrentBrush->mBoundingBox.mLargest.Z - mCurrentBrush->mBoundingBox.mSmallest.Z);
    //UpdateBrushInfo(brush_size, mCurrentBrush->mBrushPosition[0]);
}

void CTHREDDoc::OnSelectedTypeCmdUI(CCmdUI* pCmdUI)
{
	if(mAdjustmentMode)
		pCmdUI->Enable();
	else
		pCmdUI->Enable(0);
}

#if 0
void CTHREDDoc::OnBrushSelectedCopytocurrent() 
{
	// make the current brush a copy of the other
	// brush 
	mRegularBrush = *mCurrentBrush;
	mCurrentBrush = &mRegularBrush;
	mAdjustmentMode = 0;
	UpdateAllViews(NULL);
}
#endif

void CTHREDDoc::OnBrushSelectedDelete() 
{
    CThredBrush*  brush;
    CThredBrush*  tmpptr;

	// can't do it if we are the regular brush
    if (mCurrentBrush == &mRegularBrush)
    {
      return;
    }

	if(mCurrentBrush != &mRegularBrush)
    {
      brush = mCurrentBrush;

      while (brush)
      {
	    brush->RemoveBrush();

        // DPW
        // Yeah, I know this looks weird, but he's not handling
        // his brush list correctly.  This fixes the bug where
        // deletion of 1st brush kills program.
        if (int(brush) == int(mBrushList))
        {
          mBrushList = mBrushList->GetNextBrush();
          (*pWorldBsp).SetBrushList(mBrushList);
          (*pWorldBsp).ClearBsp();
        }

        tmpptr = brush;
        brush = brush->NextSelectedBrush;
	    delete tmpptr;
      }

	  mCurrentBrush = &mRegularBrush;
	  mAdjustmentMode = 0;

	  // fixup the pointers in the list.
	  (*pWorldBsp).FixupPointers(mBrushList);

      BuildEditBox(FALSE);
    }

	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void CTHREDDoc::OnBrushSelectedNextinorder() 
{
	if(mCurrentBrush->GetNextBrush())
    {
      mCurrentBrush->mFlags &= ~BRUSH_SELECTED;

	  mCurrentBrush = mCurrentBrush->GetNextBrush();
      mCurrentBrush->mFlags |= BRUSH_SELECTED;
    }

    BuildEditBox(FALSE);

	UpdateAllViews(NULL);
}

void CTHREDDoc::OnBrushSelectedPreviousinorder() 
{
	if(mCurrentBrush->PrevBrush)
    {
      mCurrentBrush->mFlags &= ~BRUSH_SELECTED;

	  mCurrentBrush = mCurrentBrush->PrevBrush;
      mCurrentBrush->mFlags |= BRUSH_SELECTED;
    }
    
    BuildEditBox(FALSE);

	UpdateAllViews(NULL);
}


void CTHREDDoc::OnBrushPrimitivesStaircase() 
{
   SetModifiedFlag();

   // Rebuild the bsp
   //????(*pWorldBsp).RebuildQuick(NULL, 0);

   if (mCurrentBrush->BrushType == CThredBrush::STAIR_BRUSH)
   {
      mStairCreation.m_Height = mCurrentBrush->BrushSpecifics.staircase.m_Height;
      mStairCreation.m_Length = mCurrentBrush->BrushSpecifics.staircase.m_Length;
      mStairCreation.m_NumberOfStairs = mCurrentBrush->BrushSpecifics.staircase.m_NumberOfStairs;
      mStairCreation.m_Width = mCurrentBrush->BrushSpecifics.staircase.m_Width;
      mStairCreation.m_MakeRamp = mCurrentBrush->BrushSpecifics.staircase.m_MakeRamp;

      mStairCreation.DoDialog(mCurrentBrush);
      BuildEditBox(TRUE);
   }
   else
   {
      mStairCreation.DoDialog(mCurrentBrush);
      BuildEditBox(TRUE);
      if( mCurrentBrush->BrushType == CThredBrush::STAIR_BRUSH )
         (*pWorldBsp).RebuildQuick(NULL, 0, TRUE);
   }

   UpdateAllViews(NULL);
   //	SetModifiedFlag();
}

BOOL CTHREDDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
   AssignPalMatDlg newFileDlg;
   newFileDlg.m_pDoc = this;
   char    tmpbuf[32];
   char    tmpbuf1[64];
   char	Filename[256];
   char    matpath[256];
   char    matname[256];
   char    palname[256];
   char    palpath[256];
   int     i;
   int		length = strlen (lpszPathName);
   CString TextMsg("Only 1 File May Be Opened At A Time...");

   if (!theApp.mAllowOpen)
   {
      theApp.multiple_requests = TRUE;
      AfxMessageBox(LPCTSTR(TextMsg));
      return FALSE;
   }

   // enable the object viewer
   pFrame->m_wndObjectView.Enable( TRUE );


   if (m_VolumeDescription[0].IsEmpty())
   {
      m_VolumeDescription[0].Format("Normal Volume");
   }

   UpdateActiveBrushes();	
   //pFrame = (CMainFrame*)AfxGetMainWnd();
   pFrame->LoadComboBox();


   if (!CDocument::OnOpenDocument(lpszPathName))
      return FALSE;

   CMaterialProperties::Initialize( m_materialList->getMaterialsCount() );

   pFrame->m_wndTextureView.Attach( GetMaterialList(), GetPalette() );   
      
   // update zee information.
   UpdateBrushInformation();

   // Disable new and open choices...
   theApp.mAllowOpen = FALSE;
   theApp.mAllowNew  = FALSE;

   // Grab your grid and some brush colors...
   color_request.peRed = 64;
   color_request.peGreen = 64;
   color_request.peBlue = 64;
   if( !theApp.mExportRun )
      line_grid_color = m_SPalette->GetNearestColor(64, 64, 64);//color_request);
   dot_grid_color = 255;
   color_plus = 250;
   color_minus = 249;
   color_origin = 251;
   color_select = 253;
   color_def_brush = 252;  // Blue in default palette
   color_def_point = 249;  // Red in default palette
   volume_color = 254;
   color_def_entity = 255;
   color_bbox = 3;

   // go through and set active the first detail we run across
   CShapeState * pState = m_StateList;
   while( pState )
   {
      if( pState->mDetails )
      {
         pState->mDetails->SetActive();
         break;
      }
      pState = pState->pNext;
   }   
   if( !pState )
   {
      AfxMessageBox( "No details found in shape file." );
      return( FALSE );
   }
   
   pFrame->UpdateGeneralStatus("Verifying World BSP");
   (*pWorldBsp).RebuildOptimal(NULL, 0, FALSE);

   pFrame->UpdateGeneralStatus("Load Finished");
   
   BuildEditBox(TRUE);

   // check if we are exporting
   if( theApp.mExportRun )
      DoTheExport( TRUE, FALSE );
           
   return TRUE;
}

BOOL CTHREDDoc::OnSaveDocument( LPCTSTR lpszPathName )
{
   CString FileName;
   
   FileName.Format( "%s", lpszPathName );
   if( FileName.GetLength() > 4 )
   {
      // check for 3dt file
      if( FileName.GetAt( FileName.GetLength() - 3 ) == '3' )
      {
         FileName.SetAt( FileName.GetLength() - 3, 'z' );
         FileName.SetAt( FileName.GetLength() - 2, 'e' );
         FileName.SetAt( FileName.GetLength() - 1, 'd' );
         SetPathName( FileName );
         SetModifiedFlag();
		 CString Test;
		 Test = GetTitle();
		 SetTitle( "test.zed" );
      }
   }
   
   // call the base function
   return CDocument::OnSaveDocument( FileName );
}

void CTHREDDoc::OnFileMapsettings() 
{
	Globals.DoDialog();	
}

void CTHREDDoc::OnQbspnowater() 
{
	SetModifiedFlag();
	(*pWorldBsp).RebuildOptimal(NULL, 0, FALSE);

    // TEST ONLY
	//(*pWorldBsp).RebuildOptimal(NULL, 0, TRUE);
    // TEST ONLY

	UpdateAllViews(NULL);
}

void CTHREDDoc::OnEntitiesCopycurrent() 
{
	// add to the end
	CEntity NewEnt;

	NewEnt = (*pEntityArray)[mCurrentEntity];
	mCurrentEntity = (*pEntityArray).Add(NewEnt);
	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void CTHREDDoc::OnUpdateEntitiesCopycurrent(CCmdUI* pCmdUI) 
{
	if(mCurrentEntity >= 0)
		pCmdUI->Enable();
	else
		pCmdUI->Enable(0);
}

//====================================================================
// This function will go through the brushes AND the entities
// AND the lights
// and try to select or de-select the ones in the bounding box.
// This is so stuff can be selected properly.
//====================================================================
void CTHREDDoc::SelectBrushes(CRect& Rect, CRenderCamera& Camera, int Status)
{
	// some vars
	ThredBox SelectBox;
	CPoint left(Rect.left, Rect.bottom), right(Rect.right, Rect.top);

	// get world coords for the b-box
	Camera.TranslateToWorld(left, SelectBox.mSmallest);
	Camera.TranslateToWorld(right, SelectBox.mLargest);

   // go through the lights
   mCurrentLight = -1;
   mSelectedLight = FALSE;
   for( int Light = 0; Light < (*pLightArray).GetSize(); Light++ )
   {
      (*pLightArray)[ Light ].resetSelected();
      if( (*pLightArray)[ Light ].active() && 
         (*pLightArray)[ Light ].inBox(SelectBox) )
      {
         if( Status )
         {
            // set the selected flag
            (*pLightArray)[ Light ].select(false);
            mSelectedLight = TRUE;
            mCurrentLight = Light;
         }
      }
   }
   
	// go through the entities
    mCurrentEntity = -1;
    mSelectedEntity = FALSE;
	for(int Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++)
    {
	  if((*pEntityArray)[Ent].IsActive())
      {
		if((*pEntityArray)[Ent].IsEntityInBox(SelectBox))
        {
		  if(Status)
          {
			  (*pEntityArray)[Ent].mFlags |= ENTITY_SELECTED;
              mSelectedEntity = TRUE;
          } else {
 			  (*pEntityArray)[Ent].mFlags &= ~ENTITY_SELECTED;
              //mSelectedEntity = FALSE;
          }
		}
	  }
	}

	// go through the brushes
	CThredBrush* CurrentBrush = mBrushList->GetActiveBrush();

	// check for intersections
	while(CurrentBrush)
    {
      if (!(CurrentBrush->mFlags & BRUSH_SELECTED))
      {
		CurrentBrush->TransformBrushPolygons();
		if(SelectBox.CheckSpecialIntersection(CurrentBrush->mBoundingBox))
        {
			if(Status)
            {
				CurrentBrush->mFlags |= BRUSH_SELECTED;

                //////////////////////////////////////////////
          if (mCurrentBrush != &mRegularBrush)
          {
            // Not pointing to the default brush, so need to add this brush to list
            // of brushes in a potential group...
            CThredBrush*  brush;
            CThredBrush*  lastbrush = NULL;
            BOOL          added = FALSE;

            brush = mCurrentBrush;

            while (brush)
            {
              // Add this brush to its proper place in order in the list
              if (brush->mBrushId < CurrentBrush->mBrushId)
              {
                // Go to next brush in list
                lastbrush = brush;
                brush = brush->NextSelectedBrush;
              } else {
                // Add BestBrush here
                brush->PrevSelectedBrush = CurrentBrush;
                CurrentBrush->NextSelectedBrush = brush;
                //brush->NextSelectedBrush = BestBrush;

                if (mCurrentBrush == brush)
                {
                  mCurrentBrush = CurrentBrush;
                } else {
                  if (lastbrush)
                  {
                    CurrentBrush->PrevSelectedBrush = lastbrush;
                    lastbrush->NextSelectedBrush = CurrentBrush;
                  }
                }

                // End the loop
                added = TRUE;
                brush = NULL;
                mAdjustmentMode = TRUE;
              }
            }

            if (!added)
            {
              lastbrush->NextSelectedBrush = CurrentBrush;
              CurrentBrush->NextSelectedBrush = NULL;
              CurrentBrush->PrevSelectedBrush = lastbrush;
            }

          } else {
            // This is the only selected brush, so make it the current one
		    mCurrentBrush = CurrentBrush;
            CurrentBrush->NextSelectedBrush = NULL;
            CurrentBrush->PrevSelectedBrush = NULL;
            mAdjustmentMode = TRUE;
          }
          ////////////////////////////////////////////
            //} else {
				//CurrentBrush->mFlags &= ~BRUSH_SELECTED;
            }
        }
	  }
	  CurrentBrush = CurrentBrush->NextActiveBrush();
	}

    // Build EditBox bounding box
    BuildEditBox(TRUE);

    SetCurrentBrushesCenters();
}

void CTHREDDoc::SelectAllBrushes()
{
    // Loop through brush list and select each brush
    CThredBrush*  brush;
    CThredBrush*  lastbrush;

    brush = mBrushList;
    lastbrush = NULL;

    while(brush)
    {
      //if ( (brush->mFlags | BRUSH_ACTIVE) &&
      //    ((brush->mBrushGroup == mCurrentGroup) ||
      //     (mCurrentGroup == NO_GROUP) ||
      //     (mShow == ID_VIEW_SHOW_ALLBRUSHES)))
      if ( ((mShow == ID_VIEW_SHOW_VISIBLEBRUSHES) && (brush->mFlags & BRUSH_ACTIVE))  ||
           ((mShow == ID_VIEW_SHOW_CURRENTGROUP ) && (brush->mBrushGroup == mCurrentGroup)) ||
            (mShow == ID_VIEW_SHOW_ALLBRUSHES))
      {
        brush->mFlags |= BRUSH_SELECTED;

        if (lastbrush)
        {
          lastbrush->NextSelectedBrush = brush;
          brush->PrevSelectedBrush = lastbrush;
          brush->NextSelectedBrush = NULL;
        } else {
          mCurrentBrush = brush;
          brush->PrevSelectedBrush = brush->NextSelectedBrush = NULL;
        }

        mAdjustmentMode = TRUE;
        lastbrush = brush;
      }

      // Next Please...
      brush = brush->GetNextBrush();
    }

    // Loop through entity list and select each entity
    mCurrentEntity = -1;
    mSelectedEntity = FALSE;
	for(int Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++)
    {
	  //if((*pEntityArray)[Ent].IsActive())
      if ( ((mShow == ID_VIEW_SHOW_VISIBLEBRUSHES) && ((*pEntityArray)[Ent].mFlags & ENTITY_ACTIVE))  ||
           ((mShow == ID_VIEW_SHOW_CURRENTGROUP ) && ((*pEntityArray)[Ent].mGroup == mCurrentGroup)) ||
            (mShow == ID_VIEW_SHOW_ALLBRUSHES))
      {
        mAdjustmentMode = TRUE;
		(*pEntityArray)[Ent].mFlags |= ENTITY_SELECTED;
        mSelectedEntity = TRUE;
	  }
	}

    // Build EditBox bounding box
    BuildEditBox(TRUE);

    SetCurrentBrushesCenters();
}

void CTHREDDoc::SetSurfaceInfo(GFXSurface* pBuffer)
{
  //(*pWorldBsp).pBuffer = pBuffer;
}

void CTHREDDoc::SetCurrentBrushesCenters()
{
  CThredBrush*  brush;

  brush = mCurrentBrush;

  while(brush)
  {
    brush->SetBrushCenter(EditBox.mCenter);

    brush = brush->NextSelectedBrush;
  }
}

void CTHREDDoc::ResizeCurrentBrushes(Point3F* minbound, Point3F* delta)
{
  CThredBrush*  brush;

  brush = mCurrentBrush;

  while(brush)
  {
    brush->Resize(minbound, delta, this, &EditBox);
    brush = brush->NextSelectedBrush;
  }
}

void CTHREDDoc::ShearCurrentBrushes(Point3F* minbound, Point3F* delta, ShearType wrt)
{
  CThredBrush*  brush;

  brush = mCurrentBrush;

  while(brush)
  {
    brush->Shear(minbound, delta, this, &EditBox, wrt);

    brush = brush->NextSelectedBrush;
  }
}

void CTHREDDoc::MoveCurrentBrushes(double X, double Y, double Z)
{
  CThredBrush*  brush;

  brush = mCurrentBrush;

  while(brush)
  {
    brush->MoveBrush(X, Y, Z, this, 1);

    brush = brush->NextSelectedBrush;
  }

  MoveEntity( X, Y, Z );
  MoveSelectedLights( X, Y, Z );
}

void CTHREDDoc::DoneMoveCurrentBrushes(int mViewType)
{
  CThredBrush*  brush;

  brush = mCurrentBrush;

  while(brush)
  {
    if(mUseGrid)
    {
      //brush->DoneMoveBrush(mGridSize, this, 1);
      double  dispx, dispy, dispz;

      switch (mViewType)
      {
      case ID_VIEW_TOPVIEW:
        dispx = EditBox.mLargest.X - (RoundFloat((EditBox.mLargest.X / mGridSize)) * mGridSize);
        dispy = 0.0;
        dispz = EditBox.mLargest.Z - (RoundFloat((EditBox.mLargest.Z / mGridSize)) * mGridSize);
        break;
      case ID_VIEW_FRONTVIEW:
        dispx = EditBox.mLargest.X - (RoundFloat((EditBox.mLargest.X / mGridSize)) * mGridSize);
        dispy = EditBox.mLargest.Y - (RoundFloat((EditBox.mLargest.Y / mGridSize)) * mGridSize);
        dispz = 0.0;
        break;
      case ID_VIEW_SIDEVIEW:
        dispx = 0.0;
        dispy = EditBox.mLargest.Y - (RoundFloat((EditBox.mLargest.Y / mGridSize)) * mGridSize);
        dispz = EditBox.mLargest.Z - (RoundFloat((EditBox.mLargest.Z / mGridSize)) * mGridSize);
        break;
      }
      brush->MoveBrush(dispx, dispy, dispz, this, 1);
    } else {
      brush->DoneMoveBrush(1, this, 1);
    }

    brush = brush->NextSelectedBrush;
  }

   BuildEditBox(TRUE);
   
   CThredBrush * tempbrush;
   // store info on the selected items some info
   if( Globals.mLockTexturePositions )
   {
      tempbrush = mBrushList;
      while( tempbrush )
      {
         if( tempbrush->mFlags & BRUSH_SELECTED )
      
            tempbrush->EndTextureChange( CThredBrush::TEX_MOVE );

         tempbrush = tempbrush->GetNextBrush();
      }
   }

  DoneMoveEntity();
  DoneMoveLights( true, mViewType );

  SetModifiedFlag();
}

void CTHREDDoc::BuildEditBox(BOOL redo_center)
{
    CThredBrush*  brush;
    ThredPoint    tempcenter;

    if (!redo_center)
    {
      tempcenter.X = EditBox.mCenter.X;
      tempcenter.Y = EditBox.mCenter.Y;
      tempcenter.Z = EditBox.mCenter.Z;
    }

    EditBox.ClearBox();

    brush = mCurrentBrush;

    while (brush)
    {
      brush->TransformBrushPolygons();
      EditBox.AddPoint(brush->mBoundingBox.mSmallest);
      EditBox.AddPoint(brush->mBoundingBox.mLargest);

      brush = brush->NextSelectedBrush;
    }

   // go through the lights
   for( int i = 0; i < pLightArray->GetSize(); i++ )
   {
      CLight & light = (*pLightArray)[i];
      for( int j = 0; j < light.states.GetSize(); j++ )
      {
         CLightState & state = light.states[j];
         for( int k = 0; k < state.emitters.GetSize(); k++ )
         {
            CLightEmitter & emitter = state.emitters[k];
            if( emitter.getSelect() )
            {
               ThredPoint pnt;
               pnt.X = emitter.pos.x;
               pnt.Y = emitter.pos.y;
               pnt.Z = emitter.pos.z;
               EditBox.AddPoint( pnt );
            }
         }
      }
   }

//   // go through the entities
//   for( int Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++ )
//   {
//      if( (*pEntityArray)[ Ent ].mFlags & ENTITY_SELECTED )
//         EditBox.AddPoint( (*pEntityArray)[ Ent ].mRenderOrigin );
//   }
            
    if (redo_center)
    {
      
      EditBox.mCenter.X = (EditBox.mSmallest.X + EditBox.mLargest.X) / 2.0;
      EditBox.mCenter.Y = (EditBox.mSmallest.Y + EditBox.mLargest.Y) / 2.0;
      EditBox.mCenter.Z = (EditBox.mSmallest.Z + EditBox.mLargest.Z) / 2.0;
    } else {
      EditBox.mCenter.X = tempcenter.X;
      EditBox.mCenter.Y = tempcenter.Y;
      EditBox.mCenter.Z = tempcenter.Z;
    }

    // Build and transform the box's polygons
    EditBox.TopPolygon.NumberOfPoints = 4;
    EditBox.FrontPolygon.NumberOfPoints = 4;
    EditBox.SidePolygon.NumberOfPoints = 4;

    // Top
    EditBox.TopPolygon.Points[0].X = EditBox.TopPolygon.Points[1].X = EditBox.mSmallest.X;
    EditBox.TopPolygon.Points[2].X = EditBox.TopPolygon.Points[3].X = EditBox.mLargest.X;

    EditBox.TopPolygon.Points[0].Y = EditBox.TopPolygon.Points[1].Y =
    EditBox.TopPolygon.Points[2].Y = EditBox.TopPolygon.Points[3].Y = EditBox.mLargest.Y;

    EditBox.TopPolygon.Points[0].Z = EditBox.TopPolygon.Points[3].Z = EditBox.mSmallest.Z;
    EditBox.TopPolygon.Points[1].Z = EditBox.TopPolygon.Points[2].Z = EditBox.mLargest.Z;

    // Front
    EditBox.FrontPolygon.Points[0].X = EditBox.FrontPolygon.Points[1].X = EditBox.mSmallest.X;
    EditBox.FrontPolygon.Points[2].X = EditBox.FrontPolygon.Points[3].X = EditBox.mLargest.X;

    EditBox.FrontPolygon.Points[0].Y = EditBox.FrontPolygon.Points[3].Y = EditBox.mSmallest.Y;
    EditBox.FrontPolygon.Points[1].Y = EditBox.FrontPolygon.Points[2].Y = EditBox.mLargest.Y;

    EditBox.FrontPolygon.Points[0].Z = EditBox.FrontPolygon.Points[1].Z =
    EditBox.FrontPolygon.Points[2].Z = EditBox.FrontPolygon.Points[3].Z = EditBox.mSmallest.Z;

    // Side
    EditBox.SidePolygon.Points[0].X = EditBox.SidePolygon.Points[1].X =
    EditBox.SidePolygon.Points[2].X = EditBox.SidePolygon.Points[3].X = EditBox.mLargest.X;

    EditBox.SidePolygon.Points[0].Y = EditBox.SidePolygon.Points[3].Y = EditBox.mSmallest.Y;
    EditBox.SidePolygon.Points[1].Y = EditBox.SidePolygon.Points[2].Y = EditBox.mLargest.Y;

    EditBox.SidePolygon.Points[0].Z = EditBox.SidePolygon.Points[1].Z = EditBox.mSmallest.Z;
    EditBox.SidePolygon.Points[2].Z = EditBox.SidePolygon.Points[3].Z = EditBox.mLargest.Z;
}

void CTHREDDoc::UpdateBrushInformation()
{
	CThredBrush* Current = mBrushList;
	while(Current)
    {
		if(Current->mBrushGroup >= 0)
			Current->mBrushColour = (*pBrushGroupArray)[Current->mBrushGroup].GetColour();
		else
			// DPW - Needed for TS compatibility
			Current->mBrushColour = 255;
		Current = Current->GetNextBrush();
	}

//   // go through the lights and update the colors
//   for( int Light = 0; Light < (*pLightArray).GetSize(); Light++ )
//   {
//      // check if in a group
//      if( (*pLightArray)[ Light ].group >= 0 )
//         (*pLightArray)[ Light ].mLightColour = 
//            (*pBrushGroupArray)[ (*pLightArray)[ Light ].mGroup ].GetColour();
//      else
//         (*pLightArray)[ Light ].mLightColour = 255;
//   }
   
	// go through the entity array and update it's colours too
	for(int Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++) {
		if((*pEntityArray)[Ent].mGroup >= 0)
			(*pEntityArray)[Ent].mEntityColour = (*pBrushGroupArray)[(*pEntityArray)[Ent].mGroup].GetColour();
		else
			// DPW (*pEntityArray)[Ent].mEntityColour = RGB(255,255,255);

			// DPW - Needed for TS compatibility
			(*pEntityArray)[Ent].mEntityColour = 255;
	}

	UpdateActiveBrushes();	
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->LoadComboBox();
}

CThredBrush* CTHREDDoc::GetBrushList()
{
  return mBrushList;
}

int CTHREDDoc::BrushesSelected()
{
	// go through the brushes
	CThredBrush* CurrentBrush = mBrushList;

	// check for intersections
	while(CurrentBrush) {
		if(CurrentBrush->mFlags & BRUSH_SELECTED)
			return 1;
		CurrentBrush = CurrentBrush->GetNextBrush();
	}

	// check if we have any selected entities.
	for(int Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++) {
		if((*pEntityArray)[Ent].mFlags & ENTITY_SELECTED) 
			return 1;
	}

	return 0;
}

// this function will go through and mark the
// brushes that we should be seeing as active
void CTHREDDoc::UpdateActiveBrushes()
{
	CThredBrush* CurrentBrush;
	int Ent;
   int Light;

	switch(mShow) {
	case ID_VIEW_SHOW_ALLBRUSHES:
		// draw the bloody brushes.
		CurrentBrush = mBrushList;
		while(CurrentBrush) {
			// not active
			CurrentBrush->mFlags &= ~BRUSH_ACTIVE;

			// if we are showing entities and this is
			// an entity brush make active
			if(mShowEntities || (!(CurrentBrush->mFlags & BRUSH_ENTITY)))
				CurrentBrush->mFlags |= BRUSH_ACTIVE;

			// next brush
			CurrentBrush = CurrentBrush->GetNextBrush();
		}

		// go through the entity array and update it
      if( mShowEntities )
		   for(Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++)
		   	(*pEntityArray)[Ent].mFlags |= ENTITY_ACTIVE;
      
      // go through all the lights and set as active
      if( mShowLights )
         for( Light = 0; Light < (*pLightArray).GetSize(); Light++ )
            (*pLightArray)[ Light ].setActive( true );
                        
		return;
	case ID_VIEW_SHOW_CURRENTGROUP:
   
//HERE
		// draw the bloody brushes.
		CurrentBrush = mBrushList;
		while(CurrentBrush) {
			CurrentBrush->mFlags &= ~BRUSH_ACTIVE;
			if(CurrentBrush->mBrushGroup == mCurrentGroup)
				if(mShowEntities || (!(CurrentBrush->mFlags & BRUSH_ENTITY)))
					CurrentBrush->mFlags |= BRUSH_ACTIVE;
			// next brush
			CurrentBrush = CurrentBrush->GetNextBrush();
		}

		// go through the entity array and update it
		for(Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++) {
			(*pEntityArray)[Ent].mFlags &= ~ENTITY_ACTIVE;
			if(((*pEntityArray)[Ent].mGroup == mCurrentGroup) && mShowEntities )
				(*pEntityArray)[Ent].mFlags |= ENTITY_ACTIVE;
		}
      
      // go through all the lights
      for( Light = 0; Light < (*pLightArray).GetSize(); Light++ )
      {
         // reset the active flag, set again if in the group
         (*pLightArray)[ Light ].setActive( false );
         if( ( (*pLightArray)[ Light ].group == mCurrentGroup ) && mShowLights )
            (*pLightArray)[ Light ].setActive( true );
      }
      
		return;
	case ID_VIEW_SHOW_VISIBLEBRUSHES:
		// draw the bloody brushes.
		CurrentBrush = mBrushList;
		while(CurrentBrush) {
			CurrentBrush->mFlags &= ~BRUSH_ACTIVE;
			if(CurrentBrush->mBrushGroup >= 0) {
				if((*pBrushGroupArray)[CurrentBrush->mBrushGroup].IsVisible())
					if(mShowEntities || (!(CurrentBrush->mFlags & BRUSH_ENTITY)))
						CurrentBrush->mFlags |= BRUSH_ACTIVE;
			}
			// next brush
			CurrentBrush = CurrentBrush->GetNextBrush();
		}

		// go through the entity array and update it
		for(Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++) {
			(*pEntityArray)[Ent].mFlags &= ~ENTITY_ACTIVE; 
			if(((*pEntityArray)[Ent].mGroup >= 0) && mShowEntities )
				if((*pBrushGroupArray)[(*pEntityArray)[Ent].mGroup].IsVisible())
					(*pEntityArray)[Ent].mFlags |= ENTITY_ACTIVE;
		}
      
      // update all the lights
      for( Light = 0; Light < (*pLightArray).GetSize(); Light++ )
      {
         // reset the active flag, check them all
         (*pLightArray)[ Light ].setActive( false );
         if( ((*pLightArray)[ Light ].group >= 0 ) && mShowLights)
            if( (*pBrushGroupArray)[ (*pLightArray)[ Light ].group ].IsVisible() )
               (*pLightArray)[ Light ].setActive( true );
      }
		return;
	}
}

// save the original brush id and restore it when we put
// the brush back in the list... gotta get the quick
// parser going first though.... so it doesn't
// seek past the end of entities.
// We want to route all deletions of entities through this
// because 
void CTHREDDoc::DeleteEntity(int EntityIndex)
{
	// now delete the entity, we'll do the fixups later
	(*pEntityArray).RemoveAt(EntityIndex);
    mCurrentEntity = -1;

	// draw the bloody brushes.
	CThredBrush* CurrentBrush = mBrushList;
	while(CurrentBrush) {

		// if this is an entity brush
		if(CurrentBrush->IsEntity()) {
			// check if it's equal to the one
			// we are getting rid of
			if(CurrentBrush->mEntityId == EntityIndex)
				CurrentBrush->mFlags &= ~BRUSH_ENTITY;

			// shift this entity's number down
			if(CurrentBrush->mEntityId > EntityIndex)
				CurrentBrush->mEntityId--;
		}

		// next brush
		CurrentBrush = CurrentBrush->GetNextBrush();
	}

}

//==============================================================
// This function will delete our current thing if we 
// have something. e.g. entity or brush
//==============================================================
void CTHREDDoc::DeleteCurrentThing()
{

	switch(mCurrentTool) {
	case ID_TOOLS_BRUSH_MOVEROTATEBRUSH:
	case ID_TOOLS_BRUSH_SCALEBRUSH:
	case ID_TOOLS_BRUSH_SHEARBRUSH:
		OnBrushSelectedDelete();
		break;
	case ID_TOOLS_ENTITY:
		DeleteEntity(mCurrentEntity);
		mCurrentEntity--;
		break;
	}
	UpdateAllViews(NULL);
}

//===============================================
// this function will update the information on the status bar
// about the grid
//===============================================
void CTHREDDoc::UpdateGridInformation()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->UpdateGridSize((int)mGridSize, mUseGrid);
}

//===============================================
// We want to make the grid larger or smaller by a power
// of 2 depending on the increment.
//===============================================
void CTHREDDoc::ChangeGridSize(int Increment)
{

	if(Increment) {
		mGridSize *= 2;
	}
	else {
		if(mGridSize >= 2)
			mGridSize /= 2;
	}

	UpdateGridInformation();
	UpdateAllViews(NULL);
}

//==========================================================
// Render the main world
//==========================================================
void CTHREDDoc::RenderWorld(CRenderCamera* Camera, TSRenderContext* rc)
{
	(*pWorldBsp).RenderSolidWorld(Camera, rc, TEXTURE_AFFINE, this);
}

//==========================================================
//  this function will move around all of the brushes that you have selected
//==========================================================
void CTHREDDoc::MoveSelectedBrushes(double X, double Y, double Z)
{
	CThredBrush* Current = mBrushList;
	while(Current) {
		// check if brush is selected
		if(Current->mFlags & BRUSH_SELECTED) {
			Current->MoveBrush(X,Y,Z, this);
		}
		Current = Current->GetNextBrush();
	}

    MoveEntity(X, Y, Z);
    MoveSelectedLights( X, Y, Z );
	//for(int Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++)
    //{
	//	if((*pEntityArray)[Ent].mFlags & ENTITY_SELECTED)
    //    {
	//		(*pEntityArray)[Ent].MoveEntity(X,Y,Z);
	//	}
	//}
}

void CTHREDDoc::DoneMoveEntity()
{
	//if(mCurrentTool == ID_TOOLS_BRUSH_MOVESELECTEDBRUSHES)
    //{
	for(int Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++)
    {
	  if((*pEntityArray)[Ent].mFlags & ENTITY_SELECTED)
      {
		if(mUseGrid)
        {
			(*pEntityArray)[Ent].DoneMoveEntity(mGridSize);
        } else {
			(*pEntityArray)[Ent].DoneMoveEntity(1);
        }
	  }
    }

	SetModifiedFlag();
}


//==========================================================
// This function goes through the list and reads it in
// or writes it out depending up which needs to be done.
//==========================================================
#define CBRUSHLIST "Class CBrushList"
#define CBRUSHLISTCOUNT "BrushCount"
#define ENDBRUSHLIST "End Class CBrushList"
void CTHREDDoc::SerializeBrushList(CThredParser& Parser)
{
    char  buf[80];
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	//CThredParser Parser(&ar, ENDBRUSHLIST);

	Parser.SetEnd(ENDBRUSHLIST);

	CThredBrush *Brush;	
	if(Parser.IsStoring) {

		// Write out our ID string
		Parser.WriteString(CBRUSHLIST);
		Parser.WriteInt(CBRUSHLISTCOUNT, GetBrushListCount());

		Brush = mBrushList;
		while(Brush) {
			Brush->Serialize(Parser);
			Brush = Brush->GetNextBrush();
		}
		Parser.WriteString(ENDBRUSHLIST);
	}
	else {
		int Count;
		// get rid of the list
		DeleteBrushList();

		// we don't actually NEED a brushlist
		if(!Parser.GetKey(CBRUSHLIST)) 
			return;

		// get the count
		if(!Parser.GetKeyInt(CBRUSHLISTCOUNT, Count)) {
			AfxThrowArchiveException(CArchiveException::generic, NULL);
			return;
		}

      // get the max texture id
      int maxTextureID = 0;
      maxTextureID = GetMaterialList()->getMaterialsCount();
      
		while(Count--)
        {
			//Brush = new CThredBrush;
			CThredBrush DaBrush;
         DaBrush.Serialize(Parser, maxTextureID );
            sprintf(buf, "%4d Brushes to load", Count);
	        pFrame->UpdateGeneralStatus(buf);
			AddBrushToList(DaBrush);
		}
	}
}


//==========================================================
// LIST TRACKING STUFF - mBrushList
//==========================================================
int CTHREDDoc::AddBrushToList(CThredBrush& Brush)
{
	CThredBrush* NewBrush;
	int BrushCount = 0;
	// add our brush onto the end
	if(mBrushList == NULL)
    {
		mBrushList = new CThredBrush;
		*mBrushList = Brush;
		mBrushList->pDoc = this;
		if(mBrushList->mBrushId == -1)
			mBrushList->mBrushId = mCurrentBrushId++;
		else
		{
			if(mCurrentBrushId < mBrushList->mBrushId)
				mCurrentBrushId = mBrushList->mBrushId;
		}
	} else {
		// go through the list
		NewBrush = mBrushList;
		while(NewBrush->GetNextBrush()) {
			BrushCount++;
			NewBrush = NewBrush->GetNextBrush();
		}

		CThredBrush* NextBrush = new CThredBrush;
        if (!NextBrush)
        {
          return 0;
        }
		// assign to the end
		NewBrush->SetNextBrush(NextBrush);
		*NextBrush = Brush;
		NextBrush->pDoc = this;

        // DPW -rewrite to update NextBrush not NewBrush (?)
		if(NextBrush->mBrushId == -1)
			NextBrush->mBrushId = mCurrentBrushId++;
		else
		{
			if(mCurrentBrushId < NextBrush->mBrushId)
				mCurrentBrushId = NextBrush->mBrushId;
		}

		NextBrush->PrevBrush = NewBrush;
		BrushCount++;
	}

	// set the current brush list of the bsp
	(*pWorldBsp).SetBrushList(mBrushList);

	return BrushCount;
}



// Toggle our group movement mode
void CTHREDDoc::OnToolsGroupmovementmode() 
{
	mGroupMovement ^= 1;
}

// Whether we are in group movement mode or not
void CTHREDDoc::OnUpdateToolsGroupmovementmode(CCmdUI* pCmdUI) 
{
#if 1
	if(mGroupMovement)
		pCmdUI->SetCheck();
	else
		pCmdUI->SetCheck(0);
#else
	pCmdUI->Enable(0);
#endif
}



void CTHREDDoc::OnBrushRemoveselectedfromgroup() 
{
	CThredBrush* Current = mBrushList;
	while(Current) {
		// check if brush is selected
		if(Current->mFlags & BRUSH_SELECTED) {
			Current->mBrushGroup = NO_GROUP;
			//Current->ResetGroupItems();
			ThredPoint X(128,128,128);
			Current->SetGroupPosition(X);
			Current->mFlags &= ~BRUSH_SELECTED;
		}
		Current = Current->GetNextBrush();
	}

	// assign the ents to the current group
	for(int Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++)
    {
		if((*pEntityArray)[Ent].mFlags & ENTITY_SELECTED)
        {
			(*pEntityArray)[Ent].mGroup = NO_GROUP;
		}
		(*pEntityArray)[Ent].mFlags &= ~ENTITY_SELECTED;
        mSelectedEntity = FALSE;
	}

   // do the same for the lights
   for( int Light = 0; Light < (*pLightArray).GetSize(); Light++ )
   {
      if( (*pLightArray)[ Light ].selected() )
         (*pLightArray)[ Light ].group = NO_GROUP;
      (*pLightArray)[ Light ].resetSelected();
      mSelectedLight= FALSE;
   }

	UpdateBrushInformation();
	UpdateAllViews(NULL);
	SetModifiedFlag();

}

void CTHREDDoc::OnUpdateBrushRemoveselectedfromgroup(CCmdUI* pCmdUI) 
{
	if(BrushesSelected())
		pCmdUI->Enable();
	else
		pCmdUI->Enable(0);
}

void CTHREDDoc::MoveEditBoxCenter(double X, double Y, double Z)
{
  EditBox.mCenter.X -= X;
  EditBox.mCenter.Y -= Y;
  EditBox.mCenter.Z -= Z;
}

void CTHREDDoc::DoneMoveEditBoxCenter()
{
  EditBox.mCenter.X = RoundFloat((EditBox.mCenter.X / mGridSize)) * mGridSize;
  EditBox.mCenter.Y = RoundFloat((EditBox.mCenter.Y / mGridSize)) * mGridSize;
  EditBox.mCenter.Z = RoundFloat((EditBox.mCenter.Z / mGridSize)) * mGridSize;
}

void CTHREDDoc::MoveBrushCenter(double X, double Y, double Z)
{
	mCurrentBrush->MoveBrushCenter(X,Y,Z, this);
}

// DPW - 10/23
//==========================================================
//  this function will move around all of the brushes that you have selected
//==========================================================
void CTHREDDoc::MoveBrush(double X, double Y, double Z)
{
	mLastOp = BRUSH_MOVE;
	// if we are group movement mode then rotate the group instead of a brush
	if(mGroupMovement)
	{
		// our current group
		if(mCurrentGroup != NO_GROUP)
		{
			// draw the bloody brushes.
			CThredBrush* CurrentBrush = mBrushList;
			while(CurrentBrush) {
				if(CurrentBrush->mBrushGroup == mCurrentGroup)
				{
					CurrentBrush->MoveBrush(X,Y,Z, this);
				}
				// next brush
				CurrentBrush = CurrentBrush->GetNextBrush();
			}
		}
	}
	else
		mCurrentBrush->MoveBrush(X,Y,Z, this);
}

void CTHREDDoc::DoneMoveBrushCenter(double GridSize)
{
    mCurrentBrush->DoneMoveBrushCenter(GridSize);
}

// Right now groups of brushes will never snap to the grid
// because they may go all weird.... the center of the group
// should really be what snaps.
void CTHREDDoc::DoneMoveBrush()
{
	if(mLastOp != BRUSH_MOVE)
		return;

	if(mCurrentTool == ID_TOOLS_BRUSH_MOVESELECTEDBRUSHES) {

		CThredBrush* Current = mBrushList;
		while(Current) {
			// check if brush is selected
			if(Current->mFlags & BRUSH_SELECTED) 
			{
					mCurrentBrush->DoneMoveBrush(1, this);
			}
			Current = Current->GetNextBrush();
		}
	}
	// if we are group movement mode then rotate the group instead of a brush
	if(mGroupMovement)
	{
		// our current group
		if(mCurrentGroup != NO_GROUP)
		{
			// set the center of the group
			(*pBrushGroupArray)[mCurrentGroup].SetGroupOrigin(mBrushList, mCurrentGroup);

			// draw the bloody brushes.
			CThredBrush* CurrentBrush = mBrushList;
			while(CurrentBrush) {
				if(CurrentBrush->mBrushGroup == mCurrentGroup)
				{
					if(mUseGrid)
						CurrentBrush->DoneMoveBrush(mGridSize, this);
					else
						CurrentBrush->DoneMoveBrush(1, this);
					CurrentBrush->SetGroupPosition((*pBrushGroupArray)[mCurrentGroup].GetGroupOrigin());

				}
				// next brush
				CurrentBrush = CurrentBrush->GetNextBrush();
			}
		}
	}
	else {
		if(mUseGrid)
			mCurrentBrush->DoneMoveBrush(mGridSize, this);
		else
			mCurrentBrush->DoneMoveBrush(1, this);
	}

	SetModifiedFlag();
}

void CTHREDDoc::ResetBrush(ThredPoint newpos)
{
  CThredBrush*  brush = mCurrentBrush;

  // Save away undo info
  StoreUndoInfo();

  // Reset all the selected brushes...
  while(brush)
  {
    brush->ResetBrush(newpos);

    brush = brush->NextSelectedBrush;
  }

//	if(mGroupMovement)
//    {
//		if(mCurrentGroup != NO_GROUP)
//        {
//			(*pBrushGroupArray)[mCurrentGroup].ResetBrush();
//		}
//	} else {
//		mCurrentBrush->ResetBrush();
//	}

	SetModifiedFlag();

    BuildEditBox(TRUE);
}

#define P_THRESHOLD  2.0
int CTHREDDoc::InCurrentBrush(ThredBox* box, /*CThredBrush* brush,*/ CPoint mStartPoint, UINT mViewType, CRenderCamera* Camera)
{
  double      PICK_THRESHOLD;
  ThredPoint  WorldPoint;
  Point3F     temp_center;

  PICK_THRESHOLD = P_THRESHOLD/Camera->mZoomFactor;
  if ((mViewType != ID_VIEW_TEXTUREVIEW) && (mViewType != ID_VIEW_SOLIDRENDER))
  {
      // get world coords for the b-box
	  Camera->TranslateToWorld(mStartPoint, WorldPoint);

      switch (mViewType)
      {
        case ID_VIEW_TOPVIEW:
          // See if the box's center point is being selected...
          if ((WorldPoint.X <= box->mCenter.X+PICK_THRESHOLD) &&
              (WorldPoint.X >= box->mCenter.X-PICK_THRESHOLD) &&
              (WorldPoint.Z <= box->mCenter.Z+PICK_THRESHOLD) &&
              (WorldPoint.Z >= box->mCenter.Z-PICK_THRESHOLD))
          {
            return BBOX_PICK_BRUSH_CENTER;
          }

          // Make sure you're inside the current box...
          if ((WorldPoint.X < box->mSmallest.X-PICK_THRESHOLD) ||
              (WorldPoint.X > box->mLargest.X+PICK_THRESHOLD) ||
              (WorldPoint.Z < box->mSmallest.Z-PICK_THRESHOLD) ||
              (WorldPoint.Z > box->mLargest.Z+PICK_THRESHOLD))
          {
            // Completely outside...
            return BBOX_PICK_NONE;
          }

          temp_center.x = box->mSmallest.X + 
                          ((box->mLargest.X - 
                            box->mSmallest.X) * 0.5);

          temp_center.y = box->mSmallest.Y + 
                          ((box->mLargest.Y - 
                            box->mSmallest.Y) * 0.5);

          temp_center.z = box->mSmallest.Z + 
                          ((box->mLargest.Z - 
                            box->mSmallest.Z) * 0.5);

          // Check left side of box
          if ((WorldPoint.X < box->mSmallest.X+PICK_THRESHOLD) &&
              (WorldPoint.X > box->mSmallest.X-PICK_THRESHOLD))
          {
            if ((WorldPoint.Z < box->mSmallest.Z+PICK_THRESHOLD) &&
                (WorldPoint.Z > box->mSmallest.Z-PICK_THRESHOLD))
            {
              // In bottom/left area
              return BBOX_PICK_BOTTOM_LEFT;
            }

            if ((WorldPoint.Z < temp_center.z+PICK_THRESHOLD) &&
                (WorldPoint.Z > temp_center.z-PICK_THRESHOLD))
            {
              // In left area
              return BBOX_PICK_LEFT;
            }

            if ((WorldPoint.Z < box->mLargest.Z+PICK_THRESHOLD) &&
                (WorldPoint.Z > box->mLargest.Z-PICK_THRESHOLD))
            {
              // In top/left area
              return BBOX_PICK_TOP_LEFT;
            }
          }

          // Check center areas of box
          if ((WorldPoint.X < temp_center.x+PICK_THRESHOLD) &&
              (WorldPoint.X > temp_center.x-PICK_THRESHOLD))
          {
            if ((WorldPoint.Z < box->mSmallest.Z+PICK_THRESHOLD) &&
                (WorldPoint.Z > box->mSmallest.Z-PICK_THRESHOLD))
            {
              // In bottom area
              return BBOX_PICK_BOTTOM;
            }

            if ((WorldPoint.Z < box->mLargest.Z+PICK_THRESHOLD) &&
                (WorldPoint.Z > box->mLargest.Z-PICK_THRESHOLD))
            {
              // In top area
              return BBOX_PICK_TOP;
            }
          }

          // Check right side of box
          if ((WorldPoint.X < box->mLargest.X+PICK_THRESHOLD) &&
              (WorldPoint.X > box->mLargest.X-PICK_THRESHOLD))
          {
            // Check right areas of box
            if ((WorldPoint.Z < box->mSmallest.Z+PICK_THRESHOLD) &&
                (WorldPoint.Z > box->mSmallest.Z-PICK_THRESHOLD))
            {
              // In bottom/right area
              return BBOX_PICK_BOTTOM_RIGHT;
            }

            if ((WorldPoint.Z < temp_center.z+PICK_THRESHOLD) &&
                (WorldPoint.Z > temp_center.z-PICK_THRESHOLD))
            {
              // In right area
              return BBOX_PICK_RIGHT;
            }

            if ((WorldPoint.Z < box->mLargest.Z+PICK_THRESHOLD) &&
                (WorldPoint.Z > box->mLargest.Z-PICK_THRESHOLD))
            {
              // In top/right area
              return BBOX_PICK_TOP_RIGHT;
            }
          }

          return BBOX_PICK_CENTER;
          break;
        case ID_VIEW_SIDEVIEW:
          // See if the box's center point is being selected...
          if ((WorldPoint.Z <= box->mCenter.Z+PICK_THRESHOLD) &&
              (WorldPoint.Z >= box->mCenter.Z-PICK_THRESHOLD) &&
              (WorldPoint.Y <= box->mCenter.Y+PICK_THRESHOLD) &&
              (WorldPoint.Y >= box->mCenter.Y-PICK_THRESHOLD))
          {
            return BBOX_PICK_BRUSH_CENTER;
          }

          if ((WorldPoint.Z < box->mSmallest.Z-PICK_THRESHOLD) ||
              (WorldPoint.Z > box->mLargest.Z+PICK_THRESHOLD) ||
              (WorldPoint.Y < box->mSmallest.Y-PICK_THRESHOLD) ||
              (WorldPoint.Y > box->mLargest.Y+PICK_THRESHOLD))
          {
            // Completely outside...
            return BBOX_PICK_NONE;
          }

          temp_center.x = box->mSmallest.X + 
                          ((box->mLargest.X - 
                            box->mSmallest.X) * 0.5);

          temp_center.y = box->mSmallest.Y + 
                          ((box->mLargest.Y - 
                            box->mSmallest.Y) * 0.5);

          temp_center.z = box->mSmallest.Z + 
                          ((box->mLargest.Z - 
                            box->mSmallest.Z) * 0.5);

          // Check left side of box
          if ((WorldPoint.Z < box->mSmallest.Z+PICK_THRESHOLD) &&
              (WorldPoint.Z > box->mSmallest.Z-PICK_THRESHOLD))
          {
            if ((WorldPoint.Y < box->mSmallest.Y+PICK_THRESHOLD) &&
                (WorldPoint.Y > box->mSmallest.Y-PICK_THRESHOLD))
            {
              // In bottom/left area
              return BBOX_PICK_BOTTOM_LEFT;
            }

            if ((WorldPoint.Y < temp_center.y+PICK_THRESHOLD) &&
                (WorldPoint.Y > temp_center.y-PICK_THRESHOLD))
            {
              // In left area
              return BBOX_PICK_LEFT;
            }

            if ((WorldPoint.Y < box->mLargest.Y+PICK_THRESHOLD) &&
                (WorldPoint.Y > box->mLargest.Y-PICK_THRESHOLD))
            {
              // In top/left area
              return BBOX_PICK_TOP_LEFT;
            }
          }

          // Check center areas of box
          if ((WorldPoint.Z < temp_center.z+PICK_THRESHOLD) &&
              (WorldPoint.Z > temp_center.z-PICK_THRESHOLD))
          {
            if ((WorldPoint.Y < box->mSmallest.Y+PICK_THRESHOLD) &&
                (WorldPoint.Y > box->mSmallest.Y-PICK_THRESHOLD))
            {
              // In bottom area
              return BBOX_PICK_BOTTOM;
            }

            if ((WorldPoint.Y < box->mLargest.Y+PICK_THRESHOLD) &&
                (WorldPoint.Y > box->mLargest.Y-PICK_THRESHOLD))
            {
              // In top area
              return BBOX_PICK_TOP;
            }
          }

          // Check right side of box
          if ((WorldPoint.Z < box->mLargest.Z+PICK_THRESHOLD) &&
              (WorldPoint.Z > box->mLargest.Z-PICK_THRESHOLD))
          {
            // Check right areas of box
            if ((WorldPoint.Y < box->mSmallest.Y+PICK_THRESHOLD) &&
                (WorldPoint.Y > box->mSmallest.Y-PICK_THRESHOLD))
            {
              // In bottom/right area
              return BBOX_PICK_BOTTOM_RIGHT;
            }

            if ((WorldPoint.Y < temp_center.y+PICK_THRESHOLD) &&
                (WorldPoint.Y > temp_center.y-PICK_THRESHOLD))
            {
              // In right area
              return BBOX_PICK_RIGHT;
            }

            if ((WorldPoint.Y < box->mLargest.Y+PICK_THRESHOLD) &&
                (WorldPoint.Y > box->mLargest.Y-PICK_THRESHOLD))
            {
              // In top/right area
              return BBOX_PICK_TOP_RIGHT;
            }
          }

          return BBOX_PICK_CENTER;
          break;
        case ID_VIEW_FRONTVIEW:
          // See if the box's center point is being selected...
          if ((WorldPoint.X <= box->mCenter.X+PICK_THRESHOLD) &&
              (WorldPoint.X >= box->mCenter.X-PICK_THRESHOLD) &&
              (WorldPoint.Y <= box->mCenter.Y+PICK_THRESHOLD) &&
              (WorldPoint.Y >= box->mCenter.Y-PICK_THRESHOLD))
          {
            return BBOX_PICK_BRUSH_CENTER;
          }

          // Make sure you're inside the current box...
          if ((WorldPoint.X < box->mSmallest.X-PICK_THRESHOLD) ||
              (WorldPoint.X > box->mLargest.X+PICK_THRESHOLD) ||
              (WorldPoint.Y < box->mSmallest.Y-PICK_THRESHOLD) ||
              (WorldPoint.Y > box->mLargest.Y+PICK_THRESHOLD))
          {
            // Completely outside...
            return BBOX_PICK_NONE;
          }

          temp_center.x = box->mSmallest.X + 
                          ((box->mLargest.X - 
                            box->mSmallest.X) * 0.5);

          temp_center.y = box->mSmallest.Y + 
                          ((box->mLargest.Y - 
                            box->mSmallest.Y) * 0.5);

          temp_center.z = box->mSmallest.Z + 
                          ((box->mLargest.Z - 
                            box->mSmallest.Z) * 0.5);

          // Check left side of box
          if ((WorldPoint.X < box->mSmallest.X+PICK_THRESHOLD) &&
              (WorldPoint.X > box->mSmallest.X-PICK_THRESHOLD))
          {
            if ((WorldPoint.Y < box->mSmallest.Y+PICK_THRESHOLD) &&
                (WorldPoint.Y > box->mSmallest.Y-PICK_THRESHOLD))
            {
              // In bottom/left area
              return BBOX_PICK_BOTTOM_LEFT;
            }

            if ((WorldPoint.Y < temp_center.y+PICK_THRESHOLD) &&
                (WorldPoint.Y > temp_center.y-PICK_THRESHOLD))
            {
              // In left area
              return BBOX_PICK_LEFT;
            }

            if ((WorldPoint.Y < box->mLargest.Y+PICK_THRESHOLD) &&
                (WorldPoint.Y > box->mLargest.Y-PICK_THRESHOLD))
            {
              // In top/left area
              return BBOX_PICK_TOP_LEFT;
            }
          }

          // Check center areas of box
          if ((WorldPoint.X < temp_center.x+PICK_THRESHOLD) &&
              (WorldPoint.X > temp_center.x-PICK_THRESHOLD))
          {
            if ((WorldPoint.Y < box->mSmallest.Y+PICK_THRESHOLD) &&
                (WorldPoint.Y > box->mSmallest.Y-PICK_THRESHOLD))
            {
              // In bottom area
              return BBOX_PICK_BOTTOM;
            }

            if ((WorldPoint.Y < box->mLargest.Y+PICK_THRESHOLD) &&
                (WorldPoint.Y > box->mLargest.Y-PICK_THRESHOLD))
            {
              // In top area
              return BBOX_PICK_TOP;
            }
          }

          // Check right side of box
          if ((WorldPoint.X < box->mLargest.X+PICK_THRESHOLD) &&
              (WorldPoint.X > box->mLargest.X-PICK_THRESHOLD))
          {
            // Check right areas of box
            if ((WorldPoint.Y < box->mSmallest.Y+PICK_THRESHOLD) &&
                (WorldPoint.Y > box->mSmallest.Y-PICK_THRESHOLD))
            {
              // In bottom/right area
              return BBOX_PICK_BOTTOM_RIGHT;
            }

            if ((WorldPoint.Y < temp_center.y+PICK_THRESHOLD) &&
                (WorldPoint.Y > temp_center.y-PICK_THRESHOLD))
            {
              // In right area
              return BBOX_PICK_RIGHT;
            }

            if ((WorldPoint.Y < box->mLargest.Y+PICK_THRESHOLD) &&
                (WorldPoint.Y > box->mLargest.Y-PICK_THRESHOLD))
            {
              // In top/right area
              return BBOX_PICK_TOP_RIGHT;
            }
          }

          return BBOX_PICK_CENTER;
          break;
      }
  }

  return 0;
}


void CTHREDDoc::ScaleBrush(double X, double Y, double Z)
{
	if(mGroupMovement) {
		// our current group
		if(mCurrentGroup != NO_GROUP)
		{
			// draw the bloody brushes.
			CThredBrush* CurrentBrush = mBrushList;
			while(CurrentBrush) {
				if(CurrentBrush->mBrushGroup == mCurrentGroup)
				{
					CurrentBrush->ScaleBrush(X,Y,Z,INDEX_GROUP);
				}
				// next brush
				CurrentBrush = CurrentBrush->GetNextBrush();
			}
		}
	}
	else {
		mCurrentBrush->ScaleBrush(X,Y,Z);
	}
}

void CTHREDDoc::ShearBrush(double X, double Y, double Z)
{
	if(mGroupMovement) {
		// our current group
		if(mCurrentGroup != NO_GROUP)
		{
			// draw the bloody brushes.
			CThredBrush* CurrentBrush = mBrushList;
			while(CurrentBrush) {
				if(CurrentBrush->mBrushGroup == mCurrentGroup)
				{
					CurrentBrush->ShearBrush(X,Y,Z,INDEX_GROUP);
				}
				// next brush
				CurrentBrush = CurrentBrush->GetNextBrush();
			}
		}
	}
	else {
		mCurrentBrush->ShearBrush(X,Y,Z);
	}
}

// DPW - 10/23
void CTHREDDoc::DoneShearBrush()
{
	if(mGroupMovement) {
		// our current group
		if(mCurrentGroup != NO_GROUP)
		{
			// draw the bloody brushes.
			CThredBrush* CurrentBrush = mBrushList;
			while(CurrentBrush) {
				if(CurrentBrush->mBrushGroup == mCurrentGroup)
				{
					CurrentBrush->DoneShearBrush(INDEX_GROUP);
				}
				// next brush
				CurrentBrush = CurrentBrush->GetNextBrush();
			}
		}
	}
	else {
		mCurrentBrush->DoneShearBrush();
	}
	SetModifiedFlag();
}


/**************************


  Where I'm at:

  1. Currently working on movement.  Seems that we need to just move the brushes origin, and
  not to even consider the origin of the group.  However, this may fuck up.

  2.  Need to have it so it works when we remove a brush from a group (e.g. use a fake matrix).
  Also brushes in groups that are moved need to have there invalidated movement flag set.
  


  DESIGN SPEC FOR ROTATING GROUPS:

  1. In each brush have a "old groups" matrix which is set to nothing at the beginning.
  2. In each brush have a current "group" rotation which is set to zero when you join a new group.

  TO ROTATE A GROUP:
  1. Go through each brush and add the rotation parameters to each of the brushes rotation params.
  Each brush will have a different rotation but all will be relative to when they entered the group.

  TO REMOVE A BRUSH FROM A GROUP:
  1. Take the current paramaters in the brush for group movement, make a matrix and concatenate with the 
  "old groups" matrix.
  2. Zero out the current group paramaters.


  TO TRANSLATE TO SCREEN
  1. Get a matrix for the current group translation, concatanate with the "old group" matrix
  and concatenate with the current brush matrix.

  Basically with this design we end up with 3 types of information in each brush
  an "old group" matrix for groups we used to belong to,
  a set of "current group" paramaters giving us information about our current group position and rotate
  and then our "local" rotation coordinates.

  **************************/


void CTHREDDoc::RotateCurrentBrushes(double Yaw, double Pitch, double Roll)
{		 
  CThredBrush*  brush;

  mLastOp = BRUSH_ROTATE;

  brush = mCurrentBrush;

  while(brush)
  {
    //brush->RotateBrush(Yaw, Pitch, Roll);
    brush->RotateBrushAroundPoint(Yaw, Pitch, Roll, EditBox.mCenter);

    brush = brush->NextSelectedBrush;
  }
  
  RotateSelectedLights( EulerF( Yaw, Pitch, Roll ), Point3F( EditBox.mCenter.X, EditBox.mCenter.Y, EditBox.mCenter.Z ) );
}

void CTHREDDoc::RotateBrush(double Yaw, double Pitch, double Roll)
{		 
	mLastOp = BRUSH_ROTATE;
	// if we are group movement mode then rotate the group instead of a brush
	if(mGroupMovement)
	{
		// our current group
		//if(mCurrentGroup != NO_GROUP)
		{
			// draw the bloody brushes.
			CThredBrush* CurrentBrush = mBrushList;
			while(CurrentBrush) {
				if(CurrentBrush->mBrushGroup == mCurrentGroup)
				{
					CurrentBrush->RotateBrush(Yaw, Pitch, Roll, INDEX_GROUP);
				}
				// next brush
				CurrentBrush = CurrentBrush->GetNextBrush();
			}
		}
	}
	else
		mCurrentBrush->RotateBrush(Yaw, Pitch, Roll);
}

void CTHREDDoc::OnBrushGroupsAddtogroup() 
{
	CThredBrush* Current = mBrushList;
	while(Current) {
		// check if brush is selected
		if(Current->mFlags & BRUSH_SELECTED) {
			Current->mBrushGroup = mCurrentGroup;
			//Current->ResetGroupItems();
			if(mCurrentGroup != NO_GROUP)
				Current->SetGroupPosition((*pBrushGroupArray)[mCurrentGroup].GetGroupOrigin());
			else
			{
				ThredPoint X(0,0,0);
				Current->SetGroupPosition(X);
			}
			Current->mFlags &= ~BRUSH_SELECTED;
		}
		Current = Current->GetNextBrush();
	}

	// assign the ents to the current group
	for(int Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++)
    {
		if((*pEntityArray)[Ent].mFlags & ENTITY_SELECTED)
        {
			(*pEntityArray)[Ent].mGroup = mCurrentGroup;
		}
		(*pEntityArray)[Ent].mFlags &= ~ENTITY_SELECTED;
        mSelectedEntity = FALSE;
	}

   // same for the lights
   for( int Light = 0; Light < (*pLightArray).GetSize(); Light++ )
   {
      if( (*pLightArray)[ Light ].selected() )
         (*pLightArray)[ Light ].group = mCurrentGroup;
      (*pLightArray)[ Light ].resetSelected();
      mSelectedLight = FALSE;
   }
   
	UpdateBrushInformation();
	UpdateAllViews(NULL);
	SetModifiedFlag();
}


void CTHREDDoc::OnBrushGroupsMakenewgroup() 
{
	CBrushGroup NewGroup;

	// add the new group
	(*pBrushGroupArray).Add(NewGroup);

	// make this group current
   
	mCurrentGroup = (*pBrushGroupArray).GetSize()-1;

	// set the center of the group
	// DPW - 10/23 - (*pBrushGroupArray)[mCurrentGroup].SetGroupOrigin(mBrushList);
	(*pBrushGroupArray)[mCurrentGroup].SetGroupOrigin(mBrushList, mCurrentGroup);

	// go through and add brushes to the group
	OnBrushGroupsAddtogroup();

	// bring up the dialog so we can name the group
	OnBrushGroupeditor();
}

void CTHREDDoc::DoneRotateBrush()
{
	if(mLastOp != BRUSH_ROTATE)
		return;

	// if we are group movement mode then rotate the group instead of a brush
	if(mGroupMovement)
	{
		// our current group
		if(mCurrentGroup != NO_GROUP)
		{
			// draw the bloody brushes.
			CThredBrush* CurrentBrush = mBrushList;
			while(CurrentBrush) {
				if(CurrentBrush->mBrushGroup == mCurrentGroup)
				{
					if(mUseRotationSnap)
						CurrentBrush->DoneRotateBrush(mRotationSnapDegrees, INDEX_GROUP);
					else
						// SNAP to a 360 degree system.....
						CurrentBrush->DoneRotateBrush(((double)1/(double)360)*(double)M_TOTAL_DEGREES, INDEX_GROUP);
				}
				// next brush
				CurrentBrush = CurrentBrush->GetNextBrush();
			}
		}
	}
	else
	{
		if(mUseRotationSnap)
			mCurrentBrush->DoneRotateBrush(mRotationSnapDegrees);
		else
			// SNAP to a 360 degree system.....
			mCurrentBrush->DoneRotateBrush(((double)1/(double)360)*(double)M_TOTAL_DEGREES);
	}

	SetModifiedFlag();
}

void CTHREDDoc::DoneRotateCurrentBrushes()
{
  CThredBrush*  brush;

  brush = mCurrentBrush;

  while(brush)
  {
	if(mUseRotationSnap)
    {
	  brush->DoneRotateBrush(mRotationSnapDegrees, 1);
    } else {
	  // SNAP to a 360 degree system.....
	  brush->DoneRotateBrush(((double)1/(double)360)*(double)M_TOTAL_DEGREES, 1);
    }

    brush = brush->NextSelectedBrush;
  }
  
   CThredBrush * tempbrush;
   // store info on the selected items some info
   if( Globals.mLockTexturePositions )
   {
      tempbrush = mBrushList;
      while( tempbrush )
      {
         if( tempbrush->mFlags & BRUSH_SELECTED )
      
            tempbrush->EndTextureChange( CThredBrush::TEX_ROTATE );

         tempbrush = tempbrush->GetNextBrush();
      }
   }

   if( mUseRotationSnap )
     DoneRotateSelectedLights( mRotationSnapDegrees );
   else
     DoneRotateSelectedLights( ((double)1/(double)360)*(double)M_TOTAL_DEGREES );
  
  SetModifiedFlag();
}

// DPW - Added for TS compatibility
GFXPalette* CTHREDDoc::GetPalette()
{
	return m_SPalette;
}

// DPW - Added for TS compatibility
void CTHREDDoc::OnCloseDocument()
{
   // get the ptrs back into the current detail
   if( pActiveDetail )
      pActiveDetail->GetInfo();
      
	// DPW - Added for TS compatibility
	releaseTS();

    // Enable new and open choices...
    if (!theApp.multiple_requests)
    {
      theApp.mAllowOpen = TRUE;
      theApp.mAllowNew  = TRUE;
    } else {
      theApp.multiple_requests = FALSE;
    }

    GotoFour = 0;

   // clear the views
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
   if( pFrame )
   {
      pFrame->m_wndObjectView.Enable( FALSE );
      pFrame->m_wndLightView.Enable( false );
   }
   CMaterialProperties::m_propertyInfo.clear();
   CMaterialProperties::m_propertyInfo.compact();
   
	CDocument::OnCloseDocument();
}

// DPW - Added for TS compatibility
void  CTHREDDoc::releaseTS(void)
{
   if((m_SPalette) || (m_materialList))
   { 
      CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
      pFrame->m_wndTextureView.Detatch();
      
      if(m_SPalette)
      {
         // Clean up after the palette...
         theApp.m_resmgr.unlock(m_palresobj);
         m_SPalette = NULL;
      }

      if (m_materialList)
      {
         //if (m_materialList->isLocked())
         if (m_materialList->isLoaded())
         {
            theApp.m_resmgr.unlock(m_matresobj);
            m_materialList = NULL;
         }
      }

//      if( m_PalPath )
//      {
//         m_PalPath->close();
//         m_PalPath = NULL;
//      }
//      if( m_MatPath )
//      {
//         m_MatPath->close();
//         m_MatPath = NULL;
//      }
      
      theApp.m_resmgr.purge(TRUE);
   }
}

// DPW - Browser to ONLY pick an existing file
BOOL CTHREDDoc::LoadBrowser(CString& filename, 
								CString filter, CString title)
{
	CFileDialog ImpDlg(TRUE, 
						NULL, 
						(LPCTSTR)filename,
						OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR,
						(LPCTSTR)filter);
	ImpDlg.m_ofn.lpstrTitle = (LPCTSTR)title;

	if(ImpDlg.DoModal() == IDOK)
    {
        filename = ImpDlg.GetPathName();
		return(TRUE);
	}
	return(FALSE);
}

// DPW - Browser to allow you to select OR enter a filename of your own to create
BOOL CTHREDDoc::LoadBrowserAny(CString& filename, 
								CString filter, CString title)
{
	CFileDialog ImpDlg(TRUE, 
						NULL, 
						(LPCTSTR)filename,
						OFN_HIDEREADONLY | OFN_NOCHANGEDIR,
						(LPCTSTR)filter);
	ImpDlg.m_ofn.lpstrTitle = (LPCTSTR)title;

	if(ImpDlg.DoModal() == IDOK)
    {
        filename = ImpDlg.GetPathName();
		return(TRUE);
	}
	return(FALSE);
}

// DPW - Browser to allow multiple file selections
BOOL CTHREDDoc::LoadBrowserMulti(CString& filename, 
								CString filter, CString title,
                                char* filenamebuffer, int buffersize)
{
	CFileDialog ImpDlg(TRUE, 
						NULL, 
						(LPCTSTR)filename,
						OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_ALLOWMULTISELECT,
						(LPCTSTR)filter);
	ImpDlg.m_ofn.lpstrTitle = (LPCTSTR)title;
    strcpy(filenamebuffer, (LPCTSTR)filename);
    ImpDlg.m_ofn.lpstrFile = filenamebuffer;
    ImpDlg.m_ofn.nMaxFile = buffersize;

	if(ImpDlg.DoModal() == IDOK)
    {
		return(TRUE);
	}
	return(FALSE);
}

 
TSMaterialList* CTHREDDoc::GetMaterialList()
{
	return m_materialList;
}

void CTHREDDoc::DoneScaleBrush()
{
	if(mGroupMovement)
    {
		// our current group
		if(mCurrentGroup != NO_GROUP)
		{
			// draw the bloody brushes.
			CThredBrush* CurrentBrush = mBrushList;
			while(CurrentBrush) {
				if(CurrentBrush->mBrushGroup == mCurrentGroup)
				{
					CurrentBrush->DoneScaleBrush(INDEX_GROUP);
				}
				// next brush
				CurrentBrush = CurrentBrush->GetNextBrush();
			}
		}
	} else {
		mCurrentBrush->DoneScaleBrush();
	}
	SetModifiedFlag();
}

void CTHREDDoc::SelectAPolygon(CPoint pos, CRenderCamera* Camera, TSRenderContext* rc)
{
    // Call the method to actually find the polygon you're pointing at
    (*pWorldBsp).SelectAPolygon (pos, Camera, rc);
}

void CTHREDDoc::OnBrushPrimitivesTriangle() 
{
   // create triangle
   SetModifiedFlag();

   // Rebuild the bsp
   //????(*pWorldBsp).RebuildQuick(NULL, 0);

   if (mCurrentBrush->BrushType == CThredBrush::TRI_BRUSH)
   {
      mTriangleCreation.m_Solid = mCurrentBrush->BrushSpecifics.triangle.m_Solid;
      mTriangleCreation.m_Thickness = mCurrentBrush->BrushSpecifics.triangle.m_Thickness;
      mTriangleCreation.m_XSizeBot = mCurrentBrush->BrushSpecifics.triangle.m_XSizeBot;
      mTriangleCreation.m_YSize = mCurrentBrush->BrushSpecifics.triangle.m_YSize;
      mTriangleCreation.m_ZSizeBot = mCurrentBrush->BrushSpecifics.triangle.m_ZSizeBot;
      mTriangleCreation.m_ZSizeTop = mCurrentBrush->BrushSpecifics.triangle.m_ZSizeTop;

      mTriangleCreation.DoDialog(mCurrentBrush);
      BuildEditBox(TRUE);
   }
   else
   {
      mTriangleCreation.DoDialog(mCurrentBrush);
      BuildEditBox(TRUE);
      
      if( mCurrentBrush->BrushType == CThredBrush::TRI_BRUSH )
         (*pWorldBsp).RebuildQuick(NULL, 0, TRUE);
   }

   UpdateAllViews(NULL);	
}

void CTHREDDoc::OnUpdateFileNew(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if(theApp.mAllowNew)
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

void CTHREDDoc::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
   CString Title;
   Title = GetTitle();
   
   if( Title.GetLength() < 4 )
   {
      pCmdUI->Enable( FALSE );
   }
   else
   {
      if( ( tolower( Title.GetAt( Title.GetLength() - 3 ) ) != 'z' ) ||
         ( tolower( Title.GetAt( Title.GetLength() - 2 ) ) != 'e' ) ||   
         ( tolower( Title.GetAt( Title.GetLength() - 1 ) ) != 'd' ) )
         pCmdUI->Enable( FALSE );
      else
         pCmdUI->Enable( TRUE );
   }
}

void CTHREDDoc::OnUpdateFileOpen(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if(theApp.mAllowOpen)
    {
		pCmdUI->Enable(TRUE);
    } else {
		pCmdUI->Enable(FALSE);
        //AfxGetMainWnd()->GetMenu()->EnableMenuItem(10, MF_GRAYED | MF_BYPOSITION);
    }
}

void CTHREDDoc::OnToolsSettexturescale() 
{
    TScaleDialog  tsdialog;

	// Pop up the texture scale dialog here and then set theApp.m_texturescale
    tsdialog.tscale = theApp.m_texturescale;

	if(tsdialog.DoModal() == IDOK)
    {
        if ((AfxMessageBox("Are You Sure?\nThis will overwrite any texture scale\nchanges you have made to this point...", MB_YESNO, 0)) == IDYES)
        {
		  theApp.m_texturescale = tsdialog.tscale;

          // Set the texture scale for all polygons
	      CThredBrush* Brush;
	      Brush = mBrushList;
	      while(Brush)
          {
            for (int j = 0; j < Brush->mNumberOfPolygons; j++)
            {
              Brush->mTextureID[j].TextureScale = tsdialog.tscale;
            }

		    Brush = Brush->GetNextBrush();
	      }
        }
	}
}

bool CTHREDDoc::ShouldGotoFourViews()
{
  if (GotoFour)
  {
    return false;
  } else {
    GotoFour++;
    return true;
  }
}

void CTHREDDoc::GotoNextEditState()
{
  edit_state++;
  edit_state = (edit_state % (ES_SHEAR+1));
}

void CTHREDDoc::GotoPrevEditState()
{
  edit_state--;
  if (edit_state == 0)
    edit_state = ES_SHEAR;
}

void CTHREDDoc::GotoNextBrush()
{
  OnBrushSelectedNextinorder();
}

void CTHREDDoc::GotoPrevBrush()
{
  OnBrushSelectedPreviousinorder();
}

void CTHREDDoc::OnToolsSetlighting() 
{
    CLightingDialog  ldialog;

	// Pop up the texture scale dialog here and then set theApp.lightval
    ldialog.lightval = theApp.lightval;

	if(ldialog.DoModal() == IDOK)
    {
		theApp.lightval = ldialog.lightval;

        (*pWorldBsp).SetLighting((*pWorldBsp).TreeTop);
        (*pWorldBsp).InvalidateNodeCacheKeys((*pWorldBsp).TreeTop);
		UpdateAllViews(NULL);	
    }
}

void CTHREDDoc::UpdateBrushInfoPOS(ThredPoint position)
{
  CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

  pFrame->UpdateBrushInfoPOS(position);
}

void CTHREDDoc::UpdateBrushInfoSIZE(ThredPoint size)
{
  CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

  pFrame->UpdateBrushInfoSIZE(size);
}

void CTHREDDoc::UpdateBrushInfoROTATION(double rotation)
{
  CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

  pFrame->UpdateBrushInfoROTATION(rotation);
}

void CTHREDDoc::OnToolsLoadnewpalette()
{
	NewFileDlg newFileDlg(NewFileDlg::LoadIDD);

   // Preset the values in the file dialog
   newFileDlg.m_strPalette.Format("%s", palName);
   newFileDlg.m_strBitmaps.Format("%s", matName);
   newFileDlg.m_strBitmapsVol.Format("%s", matVolName);
   newFileDlg.m_strPaletteVol.Format("%s", palVolName);
   newFileDlg.m_pDoc = this;
   newFileDlg.new_mat_count = 0;

   // Bring up dialog to ask for palette & materials
   if(newFileDlg.DoModal() != IDOK)
      return;

   // use the vol if it exists
   if(strlen(newFileDlg.m_PaletteVol.getTitle().c_str()))
   {
      // load up the volume for the matlist
      theApp.m_resmgr.setSearchPath(newFileDlg.m_PaletteVol.getPath().c_str());
      theApp.m_resmgr.addVolume(newFileDlg.m_PaletteVol.getTitle().c_str());
   }
   else
      theApp.m_resmgr.setSearchPath(newFileDlg.m_PaletteFile.getPath().c_str());

   // free the current palette
	if(m_SPalette)
	{
      theApp.m_resmgr.unlock(m_palresobj);
		m_SPalette = NULL;
	}

    // load in the new palette
   m_palresobj = theApp.m_resmgr.load(newFileDlg.m_PaletteFile.getTitle().c_str(), TRUE);
   if(m_palresobj)
      m_SPalette = (GFXPalette*)m_palresobj->resource;

   if(!m_SPalette)
   {
      AfxMessageBox("Error allocating palette", MB_OK, 0);
      return;
   }

   // free the matlist
   if (m_materialList)
   {
      if (m_materialList->isLoaded())
      {
         theApp.m_resmgr.unlock(m_matresobj);
         m_materialList = NULL;
      }
   }

   // use the vol if entered
   if(strlen(newFileDlg.m_TextureVol.getTitle().c_str()))
   {
      // load up the volume for the matlist
      theApp.m_resmgr.setSearchPath(newFileDlg.m_TextureVol.getPath().c_str());
      theApp.m_resmgr.addVolume(newFileDlg.m_TextureVol.getTitle().c_str());
   }
   else
      theApp.m_resmgr.setSearchPath( newFileDlg.m_TextureFile.getPath().c_str());

   // Load and assign the material list
   m_matresobj = theApp.m_resmgr.load(newFileDlg.m_TextureFile.getTitle().c_str(), TRUE);
   if(m_matresobj)
      m_materialList = (TSMaterialList*)m_matresobj->resource;

	if (!m_materialList)
	{
		AfxMessageBox("Error loading material list", MB_OK, 0);
		return;
	} 
   else 
   {
      // Lock that baby to load the textures in...
      m_materialList->load(theApp.m_resmgr, TRUE);
      CMaterialProperties::Initialize( m_materialList->getMaterialsCount() );
   }

   // set the current name's
   strcpy(palName, (LPCTSTR)newFileDlg.m_strPalette);
   strcpy(matName, (LPCTSTR)newFileDlg.m_strBitmaps);
   strcpy(palVolName, (LPCTSTR)newFileDlg.m_strPaletteVol);
   strcpy(matVolName, (LPCTSTR)newFileDlg.m_strBitmapsVol);
   
   // Tell all views to use the new palette
   SetViewPalettes();

   // update the textureviewer
   CMainFrame * pFrame = (CMainFrame*)AfxGetMainWnd();
   pFrame->m_wndTextureView.Attach(GetMaterialList(), GetPalette());
      
   // Validate all bsp tree node texture indices 
   (*pWorldBsp).ValidateNodeTextureIndex((*pWorldBsp).TreeTop, m_materialList->getMaterialsCount());

   // Invalidate the cache so everyone gets redrawn correctly...
   (*pWorldBsp).InvalidateNodeCacheKeys((*pWorldBsp).TreeTop);

   UpdateAllViews(NULL);
   SetModifiedFlag();
}

void CTHREDDoc::SetViewPalettes()
{
  CTHREDView* pThredView;
  POSITION pos = GetFirstViewPosition();

  while (pos != NULL)
  {
      CView* pView = GetNextView(pos);
      pThredView = (CTHREDView*)pView;

      pThredView->SetPalette(m_SPalette);
  }   
}

void CTHREDDoc::OnBrushCopytonewgroup() 
{
}


void CTHREDDoc::OnUpdateBrushCopytonewgroup(CCmdUI* pCmdUI) 
{
	if(BrushesSelected())
		pCmdUI->Enable();
	else
		pCmdUI->Enable(0);
}


void CTHREDDoc::OnEditCopy() 
{
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

   // check if the objectviewer window is active
   if( pFrame && ( pFrame->m_wndObjectView.IsActive() ) )
   {
      pFrame->m_wndObjectView.CopyCurrent();
   }
   else if( pFrame && ( pFrame->m_wndLightView.IsActive() ) )
   {
      pFrame->m_wndLightView.copyCurrent();
   }
   else
   {
      // Do copy stuff here...
      CopySelectedBrushes(mCurrentBrush);
      CopySelectedEntities();
      CopySelectedLights();
   }
}

void CTHREDDoc::CopySelectedEntities( void )
{
   // clear the current buffer
   mEntityCopyArray.RemoveAll();
   
	// add to the end
	CEntity NewEnt;

   // go through and add lights ( adds at end )
   int count = pEntityArray->GetSize();
      
   for( int Ent = 0; Ent < count; Ent++ )
   {
      if( (*pEntityArray)[ Ent ].mFlags & ENTITY_SELECTED )
      {
         CEntity NewEnt;
         NewEnt = (*pEntityArray)[ Ent ];

         // copy to the copy buffer
         mEntityCopyArray.Add( NewEnt );
      }
   }
}

void CTHREDDoc::CopySelectedLights( void )
{
   // clear out the current buffer
   mLightCopyArray.RemoveAll();
   
   // go through and add lights ( adds at end )
   int count = pLightArray->GetSize();
      
   for( int Light = 0; Light < count; Light++ )
   {
      if( (*pLightArray)[ Light ].selected() )
      {
         CLight NewLight( false );
         NewLight = (*pLightArray)[ Light ];

         // copy to the copy buffer
         mLightCopyArray.Add( NewLight );
      }
   }
}

void CTHREDDoc::PasteLights( void )
{
   int count = mLightCopyArray.GetSize();
   
	ResetSelectedLights();
   
   for( int Light = 0; Light < count; Light++ )
   {
      CLight NewLight( FALSE );
      NewLight = mLightCopyArray[ Light ];
      
//john      // check if need to throw in lightset or not
//      if( pActiveDetail->pActiveLightSet )
//         NewLight.mFlags |= LIGHT_INLIGHTSET;
//      else
//         NewLight.mFlags &= ~LIGHT_INLIGHTSET;
      
      pLightArray->Add( NewLight );
   }
   
   // clear out the current buffer
   mLightCopyArray.RemoveAll();
}

void CTHREDDoc::PasteEntities( void )
{
   int count = mEntityCopyArray.GetSize();
   
	ResetSelectedEntities();
   
   for( int Ent = 0; Ent < count; Ent++ )
   {
      CEntity NewEnt;
      NewEnt = mEntityCopyArray[ Ent ];
      
      pEntityArray->Add( NewEnt );
   }
   
   // clear out the current buffer
   mEntityCopyArray.RemoveAll();
}

void CTHREDDoc::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

   // check if the objectviewer window is active
   if( pFrame && ( pFrame->m_wndObjectView.IsActive() ) )
   {
		pCmdUI->Enable();
      return;
   }

   if( pFrame && pFrame->m_wndLightView.IsActive() )
   {
      pCmdUI->Enable();
      return;
   }   
    // need to check for brushes or lights or entities here
    if( ( mCurrentBrush != &mRegularBrush ) || mSelectedLight ||
      mSelectedEntity )
    {
      pCmdUI->Enable();
    } else {
      pCmdUI->Enable(0);
    }
}
   
void CTHREDDoc::OnEditCut()
{
   // there is no cut feature in the main document yet
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

   // check if the objectviewer window is active
   if( pFrame && ( pFrame->m_wndObjectView.IsActive() ) )
   {
      pFrame->m_wndObjectView.CutCurrent();
      return;
   }
   else if( pFrame && ( pFrame->m_wndLightView.IsActive() ) )
      pFrame->m_wndLightView.cutCurrent();
}

void CTHREDDoc::OnUpdateEditCut(CCmdUI* pCmdUI)
{
   // there is no cut feature in the main document yet
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

   // check if the objectviewer window is active
   if( pFrame && ( pFrame->m_wndObjectView.IsActive() || 
      pFrame->m_wndLightView.IsActive() ) )
		pCmdUI->Enable();
   else
		pCmdUI->Enable(0);
}

void CTHREDDoc::OnEditPaste() 
{
   // there is no cut feature in the main document yet
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

   // check if the objectviewer window is active
   if( pFrame && ( pFrame->m_wndObjectView.IsActive() ) )
   {
      pFrame->m_wndObjectView.PasteCurrent();
      return;
   }
   else if( pFrame && pFrame->m_wndLightView.IsActive() )
   {
      pFrame->m_wndLightView.pasteCurrent();
      return;
   }
   
	// TODO: Add your command handler code here
   PasteBrushes(mcopylist);
   mcopylist = NULL;
   
   PasteLights();
   PasteEntities();   
}

void CTHREDDoc::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

   // check if the objectviewer window is active
   if( pFrame && ( pFrame->m_wndObjectView.IsActive() ||
      pFrame->m_wndLightView.IsActive() ) )
   {
		pCmdUI->Enable();
      return;
   }
   
    if( mcopylist || mLightCopyArray.GetSize() || 
      mEntityCopyArray.GetSize() )
    {
      pCmdUI->Enable();
    } else {
      pCmdUI->Enable(0);
    }
}

void CTHREDDoc::OnLockTexturePositions()
{
   if( Globals.mLockTexturePositions )
      Globals.mLockTexturePositions = FALSE;
   else
      Globals.mLockTexturePositions = TRUE;
}

void CTHREDDoc::OnUpdateLockTexturePositions( CCmdUI* pCmdUI )
{
   if( Globals.mLockTexturePositions )
      pCmdUI->SetCheck();
   else
      pCmdUI->SetCheck( 0 );
}

void CTHREDDoc::OnAxisX() 
{
	mViewAxis ^= AXIS_X;
	Globals.mAxis = mViewAxis;
}

void CTHREDDoc::OnUpdateAxisX(CCmdUI* pCmdUI) 
{
	if(mViewAxis & AXIS_X)
		pCmdUI->SetCheck();
	else
		pCmdUI->SetCheck(0);
}

void CTHREDDoc::OnAxisY() 
{
	mViewAxis ^= AXIS_Y;
	Globals.mAxis = mViewAxis;
}

void CTHREDDoc::OnUpdateAxisY(CCmdUI* pCmdUI) 
{
	if(mViewAxis & AXIS_Y)
		pCmdUI->SetCheck();
	else
		pCmdUI->SetCheck(0);
}

void CTHREDDoc::OnAxisZ() 
{
	mViewAxis ^= AXIS_Z;
	Globals.mAxis = mViewAxis;
}

void CTHREDDoc::OnUpdateAxisZ(CCmdUI* pCmdUI) 
{
	if(mViewAxis & AXIS_Z)
		pCmdUI->SetCheck();
	else
		pCmdUI->SetCheck(0);
}

void CTHREDDoc::OnToolsSetbackgroundcolor() 
{
  CSelectColorDialog  dialog;

  dialog.palette = GetPalette();

  for (int i = 0; i < TOTAL_COLORS; i++)
  {
    switch (i)
    {
    case BACKGROUND:
      dialog.selectedcolor[i] = background_color;
      break;
    case GRID_ORIGIN:
      dialog.selectedcolor[i] = color_origin;
      break;
    case ADDITIVE_BRUSH:
      dialog.selectedcolor[i] = color_plus;
      break;
    case SUBTRACTIVE_BRUSH:
      dialog.selectedcolor[i] = color_minus;
      break;
    case SELECTED_BRUSH:
      dialog.selectedcolor[i] = color_select;
      break;
    case DEFAULT_BRUSH:
      dialog.selectedcolor[i] = color_def_brush;
      break;
    case LINE_GRID:
      dialog.selectedcolor[i] = line_grid_color;
      break;
    case DOT_GRID:
      dialog.selectedcolor[i] = dot_grid_color;
      break;
    case BOUNDING_BOX:
      dialog.selectedcolor[i] = color_bbox;
      break;
    case VOLUME_BRUSH:
      dialog.selectedcolor[i] = volume_color;
      break;
    }
  }

  dialog.current_selection = curr_colorchange_selection;

  if (dialog.DoModal() == IDOK)
  {
    for (int i = 0; i < TOTAL_COLORS; i++)
    {
      switch (i)
      {
      case BACKGROUND:
        background_color = dialog.selectedcolor[i];
        break;
      case GRID_ORIGIN:
        color_origin = dialog.selectedcolor[i];
        break;
      case ADDITIVE_BRUSH:
        color_plus = dialog.selectedcolor[i];
        break;
      case SUBTRACTIVE_BRUSH:
        color_minus = dialog.selectedcolor[i];
        break;
      case SELECTED_BRUSH:
        color_select = dialog.selectedcolor[i];
        break;
      case DEFAULT_BRUSH:
        color_def_brush = dialog.selectedcolor[i];
        break;
      case LINE_GRID:
        line_grid_color = dialog.selectedcolor[i];
        break;
      case DOT_GRID:
        dot_grid_color = dialog.selectedcolor[i];
        break;
      case BOUNDING_BOX:
        color_bbox = dialog.selectedcolor[i];
        break;
      case VOLUME_BRUSH:
        volume_color = dialog.selectedcolor[i];
        break;
      }
    }

    curr_colorchange_selection = dialog.current_selection;
  }

  UpdateAllViews(NULL);
}

void CTHREDDoc::CopySelectedBrushes(CThredBrush* brushlist)
{
   CThredBrush*  wheretoadd;
   CThredBrush*  tmpptr;
   CThredBrush*  brush;

   if (mcopylist)
   {
      // Get rid of what we're currently storing before continuing on...
      brush = mcopylist;

      while (brush)
      {
         tmpptr = brush;

         brush = brush->GetNextBrush();

         delete tmpptr;
      }

      mcopylist = NULL;
   }

   brush = brushlist;
   wheretoadd = NULL;

   while (brush)
   {
      // Copy this brush
      tmpptr = new CThredBrush;
      *tmpptr = *brush;

      // Give this brush an unused ID
      tmpptr->mBrushId = mCurrentBrushId;
      mCurrentBrushId++;

      // Add the new brush to the copy list
      if (mcopylist)
      {
         wheretoadd->SetNextBrush(tmpptr);
         tmpptr->PrevBrush = wheretoadd;
         wheretoadd = wheretoadd->GetNextBrush();
      } 
      else 
      {
         mcopylist = tmpptr;
         wheretoadd = mcopylist;
      }

      //tmpptr = brush;
      //tmpptr->mFlags &= ~BRUSH_SELECTED;

      // Goto next brush that's selected
      brush = brush->NextSelectedBrush;

      // Deselect this brush
      //tmpptr->NextSelectedBrush = NULL;
   }

   //UpdateAllViews(NULL);
}

void CTHREDDoc::PasteBrushes(CThredBrush* copylist)
{
  CThredBrush*  brush;
  CThredBrush*  tmpptr;
  CThredBrush*  lastptr;

  if (!copylist)
  {
    // Nothing to paste...
    return;
  }

  // Go through the copylist and select all brushes in it...
  brush = copylist;
  tmpptr = lastptr = NULL;

  while (brush)
  {
    tmpptr = brush;
    tmpptr->mFlags = BRUSH_ACTIVE | BRUSH_SELECTED;

    tmpptr->PrevSelectedBrush = lastptr;
    
    lastptr = brush;
    brush = brush->GetNextBrush();

    tmpptr->NextSelectedBrush = brush;  // Iterating
  }

  // Append the copylist to the end of the brushlist
  brush = mBrushList;
  tmpptr = NULL;

  // Find the end of the brushlist...
  while (brush)
  {
    tmpptr = brush;

    // Deselect that baby!!!!!!!
    if (tmpptr->mFlags & BRUSH_SELECTED)
    {
      tmpptr->mFlags &= ~BRUSH_SELECTED;
      tmpptr->NextSelectedBrush = NULL;
      tmpptr->PrevSelectedBrush = NULL;
    }

    // Goto the next brush in the brush list
    brush = brush->GetNextBrush();
  }

  // This does the append
  if (tmpptr)
  {
    tmpptr->SetNextBrush(copylist);
    copylist->PrevBrush = tmpptr;
  } else {
    mBrushList = copylist;
    copylist->PrevBrush = NULL;
  }

  // Set the current brush to the copylist
  mCurrentBrush = copylist;

  BuildEditBox(TRUE);

  UpdateAllViews(NULL);
}

#define CGRIDSIZE "GSize"
void CTHREDDoc::OnFileExportbrushgroup() 
{
  static char BASED_CODE szFilter[] = "Zed Group Files (*.zgp)|*.zgp|All Files (*.*)|*.*||";
  CFileDialog	dlg (FALSE,"ZGP",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,szFilter,NULL);

  FILE*	  fd=NULL;
  CString Filename;

  // Get the filename
  if(dlg.DoModal() == IDOK)
  {
  	Filename = dlg.GetPathName();

    CFile         pFile(Filename, CFile::modeCreate | CFile::modeWrite);
    CArchive      ar(&pFile, CArchive::store);
   	CThredParser  Parser(&ar, NULL);

   // write the version number
   CTHREDApp * App = ( CTHREDApp * )AfxGetApp();
   Parser.WriteInt( CTHREDVERSION, App->m_version );

    WriteCurrentGroup(Parser);

    // Write out current grid size
    Parser.WriteInt(CGRIDSIZE, (int)mGridSize);
  }
}

// This method writes out the brushes & entities belonging to the current group
void CTHREDDoc::WriteCurrentGroup(CThredParser& Parser)
{
    int         count, Ent, Light;
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

	Parser.SetEnd(ENDBRUSHLIST);

	CThredBrush *Brush;

    Brush = mBrushList;

    // Determine the number of brushes in the current group
    count = 0;
    while(Brush)
    {
      if (Brush->mBrushGroup == mCurrentGroup)
      {
        count++;
      }

	  Brush = Brush->GetNextBrush();
    }

	// Write out the brush info
	Parser.WriteString(CBRUSHLIST);
	Parser.WriteInt(CBRUSHLISTCOUNT, count);

	Brush = mBrushList;
	while(Brush)
    {
      if (Brush->mBrushGroup == mCurrentGroup)
      {
		Brush->Serialize(Parser);
      }

	  Brush = Brush->GetNextBrush();
	}

	Parser.WriteString(ENDBRUSHLIST);

    // Determine the number of entities in the current group
    count = 0;
	for(Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++)
	{
      if ((*pEntityArray)[Ent].mGroup == mCurrentGroup)
      {
        count++;
      }
    }

    // Write out the entity info
	Parser.WriteString(CENTLIST);
	Parser.WriteInt(CENTLISTCOUNT, count);

	for(Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++)
	{
      if ((*pEntityArray)[Ent].mGroup == mCurrentGroup)
      {
		(*pEntityArray)[Ent].Serialize(Parser);
      }
    }

   Parser.WriteString(CENDENTLIST);
   
   // do the lights now
   count = 0;
   for( Light = 0; Light < (*pLightArray).GetSize(); Light++ )
      if( (*pLightArray)[ Light ].group == mCurrentGroup )
         count++;
         
   // write it out
	Parser.WriteString( CLIGHTLIST );
	Parser.WriteInt( CLIGHTLISTCOUNT, count );

	for( Light = 0; Light < (*pLightArray).GetSize(); Light++ )
      if( (*pLightArray)[ Light ].group == mCurrentGroup )
         (*pLightArray)[ Light ].Serialize( Parser );
	Parser.WriteString(CENDLIGHTLIST);
   
}

void CTHREDDoc::OnFileImportbrushgroup() 
{
  static char BASED_CODE szFilter[] = "Zed Group Files (*.zgp)|*.zgp|All Files (*.*)|*.*||";
  CFileDialog	dlg (TRUE,"ZGP",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,szFilter,NULL);

  FILE*	  fd=NULL;
  CString Filename;
  CString Filetitle;
  int     gsize;

  // Get filename
  if(dlg.DoModal() == IDOK)
  {
  	Filename = dlg.GetPathName();
    Filetitle = dlg.GetFileTitle();

    CFile         pFile(Filename, CFile::modeRead);
    CArchive      ar(&pFile, CArchive::load);
   	CThredParser  Parser(&ar, NULL);

   // write the version number
   CTHREDApp * App = ( CTHREDApp * )AfxGetApp();
   int ver;
   
   // set to version 1.0 if version not found
   if( !Parser.GetKeyInt( CTHREDVERSION, ver ) )
      ver = 0x00010000;
   App->m_fileVersion = ver;
   
    ReadAGroup(Parser, Filetitle);

    // Read in the grid size & change if it's smaller
    if (Parser.GetKeyInt(CGRIDSIZE, gsize))
    {
      if (gsize < mGridSize)
      {
        mGridSize = gsize;
        UpdateGridInformation();
      }
    }
  }

  UpdateAllViews(NULL);
}

// This method reads in the brushes & entities belonging to a group
void CTHREDDoc::ReadAGroup(CThredParser& Parser, CString filename)
{
   int           count;
   CThredBrush*  lastbrush;
   CThredBrush*  localcopylist;
   CThredBrush*  InBrush;
   CMainFrame*   pFrame = (CMainFrame*)AfxGetMainWnd();
   CBrushGroup   Group;


   // Read in the brushes in the group...
   if(!Parser.GetKey(CBRUSHLIST)) 
      return;

   // get the count
   if(!Parser.GetKeyInt(CBRUSHLISTCOUNT, count))
      return;

   Group.mName = filename;
   (*pBrushGroupArray).Add(Group);
   mCurrentGroup = (*pBrushGroupArray).GetSize()-1;

   // get the maximum material index ( will reset if not passed into serialize )
   int maxMat = GetMaterialList()->getMaterialsCount();

   lastbrush = NULL;
   while(count--)
   {
      InBrush = new CThredBrush;
      InBrush->Serialize(Parser, maxMat );

      InBrush->PrevBrush = lastbrush;
      InBrush->mFlags = BRUSH_ACTIVE | BRUSH_SELECTED;
      InBrush->mBrushGroup = mCurrentGroup;
      InBrush->mBrushId = mCurrentBrushId;
      mCurrentBrushId++;

      if (lastbrush)
      {
         lastbrush->SetNextBrush(InBrush);
      } 
      else 
      {
         localcopylist = InBrush;
      }

      lastbrush = InBrush;
      //sprintf(buf, "%4d Brushes to load", count);
      //pFrame->UpdateGeneralStatus(buf);
   }

   // Append the brushes to the current brush list...
   PasteBrushes(localcopylist);
   (*pWorldBsp).SetBrushList(mBrushList);
   mAdjustmentMode = TRUE;

   // Read in the entities in the group...
   if(Parser.GetKey(CENTLIST))
   {	
      // get the count
      if(Parser.GetKeyInt(CENTLISTCOUNT, count))
      {
         // get our current
         //Parser.GetKeyInt(CCURRENTENT, mCurrentEntity);

         // go through and load
         while(count--)
         {
            CEntity Ent;
            Ent.Serialize(Parser);
            Ent.mFlags = ENTITY_SELECTED | ENTITY_ACTIVE;
            Ent.mGroup = mCurrentGroup;

            (*pEntityArray).Add(Ent);
         }

         // update origins from strings
         UpdateEntityOrigins();
      }
   } // Done with entities

   // read in the light information
   if( Parser.GetKey( CLIGHTLIST ) )
   {
      if( Parser.GetKeyInt( CLIGHTLISTCOUNT, count ) )
      {
         while( count-- )
         {
            CLight Light( FALSE );
            Light.Serialize( Parser );
            Light.select( true );
            Light.setActive( true );
            Light.group = mCurrentGroup;
            (*pLightArray).Add( Light );
         }
      }
   }

   //mCurrentGroup++;
   pFrame->LoadComboBox();
   UpdateActiveBrushes();
}

void CTHREDDoc::SelectAllLights( void )
{
   for( int Light = 0; Light < (*pLightArray).GetSize(); Light++ )
      (*pLightArray)[ Light ].select( true );
      
   if( pLightArray->GetSize() )
   {
      mCurrentLight = 0;
      mSelectedLight = TRUE;
   }
   else
   {
      mCurrentLight = -1;
      mSelectedLight = FALSE;
   }
}

void CTHREDDoc::SelectAllEntities( void )
{
   for( int Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++ )
      (*pEntityArray)[ Ent ].mFlags |= ENTITY_SELECTED;
      
   if( pEntityArray->GetSize() )
   {
      mCurrentEntity = 0;
      mSelectedEntity = TRUE;
   }
   else
   {
      mCurrentEntity = -1;
      mSelectedEntity = FALSE;
   }
}

void CTHREDDoc::OnEditSelectall() 
{
  SelectAllBrushes();
  SelectAllEntities();
  SelectAllLights();

  UpdateAllViews(NULL);
}

void CTHREDDoc::OnUpdateEditSelectall(CCmdUI* pCmdUI) 
{
  if (mBrushList)
  {
    pCmdUI->Enable();
  } else {
    pCmdUI->Enable(0);
  }
}

void CTHREDDoc::OnBrushAdjustbrushorder() 
{
  CBrushOrderDialog dialog;

  dialog.pDoc = this;

  dialog.DoModal();

  // Set the brush list for the bsp class
  (*pWorldBsp).SetBrushList(mBrushList);
}

void CTHREDDoc::OnUpdateBrushAdjustbrushorder(CCmdUI* pCmdUI) 
{
  if (mBrushList)
  {
    pCmdUI->Enable();
  } else {
    pCmdUI->Enable(0);
  }
}

void CTHREDDoc::OnComputelighting() 
{
   Computelighting( 0 );
}

void CTHREDDoc::OnUpdateComputelighting(CCmdUI* pCmdUI) 
{
//  if (mBrushList)
//  {
//    pCmdUI->Enable();
//  } else {
    pCmdUI->Enable(0);
//  }
}

void CTHREDDoc::OnUpdateFileGeneratelighttablereport(CCmdUI* pCmdUI) 
{
  if ((*pEntityArray).GetSize() > 0)
  {
    pCmdUI->Enable();
  } else {
    pCmdUI->Enable(0);
  }
}

void CTHREDDoc::OnFileGeneratelighttablereport() 
{
  int   Ent;
  char  namebuf[32];

  sprintf(namebuf, "%s", (LPCTSTR)GetTitle());
  for (int loc = 0; loc < strlen(namebuf); loc++)
  {
    if (namebuf[loc] == '.')
    {
      namebuf[loc] = 0;
      break;
    }
  }

  sprintf(namebuf, "%s.lmp", namebuf);

  static char BASED_CODE szFilter[] = "LMP Light Map Report (*.lmp)|*.lmp|All Files (*.*)|*.*||";
  CFileDialog	dlg (FALSE,"LMP",namebuf,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,szFilter,NULL);
  FILE*	fd=NULL;

  // get the filename
  CString Filename;

  // show it to the user.
  if(dlg.DoModal() == IDOK)
  {
  	Filename = dlg.GetPathName();

    fd = fopen(Filename, "w");

    if (fd)
    {
      fprintf(fd, "Light\t\tPosition\t\t\tNumber of States\n");
      fprintf(fd, "-----\t\t--------\t\t\t----------------\n");

      for (Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++)
      {
        // Make sure this is an active one...
        if ((*pEntityArray)[Ent].mFlags & ENTITY_ACTIVE)
        {
          fprintf(fd, "%3d.\t(%9.2f, %9.2f, %9.2f)\t\t%2d\n",
            (Ent+1),
            (*pEntityArray)[Ent].mRenderOrigin.X,
            (*pEntityArray)[Ent].mRenderOrigin.Z,
            (*pEntityArray)[Ent].mRenderOrigin.Y,
            (*pEntityArray)[Ent].mNumStates);
        }
      }

      fclose(fd);
    }
  }
}

void CTHREDDoc::OnToolsAssignvolumes() 
{
  CVolDescriptionDlg  dialog;

  int i;

  for (i = 0; i < TOTAL_STATES; i++)
  {
    dialog.m_description[i] = m_VolumeDescription[i];
  }

  if (dialog.DoModal() == IDOK)
  {
    for (i = 0; i < TOTAL_STATES; i++)
    {
      m_VolumeDescription[i] = dialog.m_description[i];
    }
  }
}

void CTHREDDoc::OnUpdateToolsAssignvolumes(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

void CTHREDDoc::OnViewShowVolumes() 
{
  if (mVolumeShow)
  {
	mVolumeShow = FALSE;
  } else {
    mVolumeShow = TRUE;
  }

  UpdateAllViews(NULL);
}

void CTHREDDoc::OnUpdateViewShowVolumes(CCmdUI* pCmdUI) 
{
  pCmdUI->SetCheck(mVolumeShow);
}

void CTHREDDoc::OnViewShowFilledvolumes() 
{
  if (mFilledVolumeShow)
  {
	mFilledVolumeShow = FALSE;
  } else {
    mFilledVolumeShow = TRUE;
  }

  UpdateAllViews(NULL);
}

void CTHREDDoc::OnUpdateViewShowFilledvolumes(CCmdUI* pCmdUI) 
{
  pCmdUI->SetCheck(mFilledVolumeShow);
}

void CTHREDDoc::OnEntitiesDeleteselected() 
{
	// can't do it if we are the regular brush
    if (!mSelectedEntity)
    {
      return;
    }

    int Ent = 0;
    int size = (*pEntityArray).GetSize();

    while (Ent < size)
    {
      //// Loop through and delete all selected entities
	  if((*pEntityArray)[Ent].mFlags & ENTITY_SELECTED)
      {
        DeleteEntity(Ent);

        // Reset the entity count since the entity array is a CArray,
        //and everything will get shifted down when one gets deleted...
        Ent = 0;
        size = (*pEntityArray).GetSize();
	  } else {
        Ent++;
      }
	}

    mSelectedEntity = FALSE;
	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void CTHREDDoc::OnUpdateEntitiesDeleteselected(CCmdUI* pCmdUI) 
{
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

   // check if the objectviewer window is active
   if( pFrame && ( pFrame->m_wndObjectView.IsActive() ||
      pFrame->m_wndLightView.IsActive() ) )
   {
		pCmdUI->Enable();
      return;
   }
   
   if (mSelectedEntity)
   {
     pCmdUI->Enable();
   } else {
     pCmdUI->Enable(0);
   }
}

void CTHREDDoc::OnDeleteselections() 
{
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
   
   // check if the objectviewer window is active
   if( pFrame && ( pFrame->m_wndObjectView.IsActive() ) )
   {
      pFrame->m_wndObjectView.DeleteCurrent();
      return;
   }
   else if( pFrame && pFrame->m_wndLightView.IsActive() )
   {
      pFrame->m_wndLightView.removeCurrent();
      return;
   }
   else
   {
      if (mSelectedEntity)
         OnEntitiesDeleteselected();

      // delete the light(s)
      if( mSelectedLight )
         OnLightsDeleteSelected();

      if (mCurrentBrush != &mRegularBrush)
         OnBrushSelectedDelete();
   }
}

// LIGHT functions
void CTHREDDoc::RenderLights( CRenderCamera* Camera, int ViewType, 
   double mZoomFactor, TSRenderContext* rc)
{
   if( ( ( ViewType == ID_VIEW_SOLIDRENDER ) || ( ViewType == ID_VIEW_TEXTUREVIEW ) ) && 
      !( mShowLights3D ) )
      return;
   
   // go through the lights - first unselected then selected ( so we can see them )
	for( int Light = 0; Light < (*pLightArray).GetSize(); Light++ )
      (*pLightArray)[Light].renderLight( Camera, rc, ViewType, false );
	for( Light = 0; Light < (*pLightArray).GetSize(); Light++ )
      (*pLightArray)[Light].renderLight( Camera, rc, ViewType, true );
}

void CTHREDDoc::OnLightsEditor() 
{
   // show the light editor...
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
   if( pFrame && pFrame->m_wndLightView.IsOpen() )
      pFrame->ShowControlBar( &pFrame->m_wndLightView, false, false );
   else
   {
      pFrame->ShowControlBar( &pFrame->m_wndLightView, true, false );
      pFrame->m_wndLightView.m_TreeWnd.Rebuild( NULL );
   }
   pFrame->RecalcLayout();   
}

void CTHREDDoc::OnUpdateLightsEditor( CCmdUI * pCmdUI )
{
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
   if( pFrame && pFrame->m_wndLightView.IsOpen() )
      pCmdUI->SetCheck();
   else
      pCmdUI->SetCheck(0);
}

void CTHREDDoc::OnLightsPlace()
{
   // add a light with a default state, and emitter...
	CLight Light( true );
	( *pLightArray ).Add( Light );

   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
   if( pFrame && pFrame->m_wndLightView.IsOpen() )
      pFrame->m_wndLightView.m_TreeWnd.Rebuild( NULL );

	// update the information.
	UpdateBrushInformation();
	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void CTHREDDoc::OnLightsShow()
{
   if( mShowLights )
   {
      mShowLights = FALSE;
      
      // select all lights as active
      for( int Light = 0; Light < (*pLightArray).GetSize(); Light++ )
         (*pLightArray)[ Light ].setActive( false );
   }
   else
   {
      mShowLights = TRUE;
      
      // set all lights as active
      for( int Light = 0; Light < (*pLightArray).GetSize(); Light++ )
         (*pLightArray)[ Light ].setActive( true );
   }

	UpdateBrushInformation();
	UpdateAllViews(NULL);
}

void CTHREDDoc::OnUpdateLightsShow( CCmdUI * pCmdUI )
{
   if( mShowLights )
      pCmdUI->SetCheck();
   else
      pCmdUI->SetCheck(0);
}

void CTHREDDoc::OnLightsShowFalloffs()
{
   CLight::showFalloffs ^= true;
}

void CTHREDDoc::OnUpdateLightsShowFalloffs( CCmdUI * pCmdUI )
{
   pCmdUI->SetCheck(CLight::showFalloffs);
   UpdateAllViews(NULL);
}

void CTHREDDoc::OnUpdateLightsShow3D( CCmdUI * pCmdUI )
{
   if( mShowLights3D )
      pCmdUI->SetCheck();
   else
      pCmdUI->SetCheck(0);
}

void CTHREDDoc::OnLightsDeleteSelected()
{
   // can't do it if we are the regular brush
   if( !mSelectedLight )
      return;

   // only delete's the emitter's 
   for( int i = 0; i < pLightArray->GetSize(); i++ )
      if( (*pLightArray)[i].selected() )
         DeleteLight( i );

   mSelectedLight = FALSE;
	UpdateAllViews(NULL);
	SetModifiedFlag();
}

void CTHREDDoc::OnUpdateLightsDeleteSelected( CCmdUI * pCmdUI )
{
  if( mSelectedLight )
  {
    pCmdUI->Enable();
  } else {
    pCmdUI->Enable(0);
  }
}

void CTHREDDoc::OnLightsCopyCurrent()
{
   for( int i = 0; i < pLightArray->GetSize(); i++ )
   {
      CLight & light = (*pLightArray)[i];
      for( int j = 0; j < light.states.GetSize(); j++ )
      {
         CLightState & state = light.states[j];
   
         // the size will change, grab the current size
         int count = state.emitters.GetSize();
         
         // go through and copy all the lights - unselect it...
         for( int k = 0; k < count; k++ )
         {
            CLightEmitter & emitter = state.emitters[k];
            
            if( emitter.getSelect() )
            {
               CLightEmitter em;
               em = emitter;
                
               // reset the emitter's selected state
               emitter.select( false );
               state.addEmitter(em);
            }
         }
      }
   }
   
   // rebuild
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
   if( pFrame && pFrame->m_wndLightView.IsOpen() )
      pFrame->m_wndLightView.m_TreeWnd.Rebuild( NULL );
   
	UpdateAllViews( NULL );
	SetModifiedFlag();
}

void CTHREDDoc::OnUpdateLightsCopyCurrent( CCmdUI * pCmdUI )
{
  if( mSelectedLight )
  {
    pCmdUI->Enable();
  } else {
    pCmdUI->Enable(0);
  }
}

//-----------------------------------------------------------------

void CTHREDDoc::ClearSelections()
{
   // Deselect any selected brushes, and make the regular brush current
   mCurrentBrush = &(mRegularBrush);
   mAdjustmentMode = FALSE;

   // Loop through and deselect everyone...
   CThredBrush*  tempbrush;
   tempbrush = mBrushList;
   while(tempbrush)
   {
      tempbrush->mFlags &= ~BRUSH_SELECTED;
      tempbrush->NextSelectedBrush = NULL;
      tempbrush->PrevSelectedBrush = NULL;
      tempbrush = tempbrush->GetNextBrush();
   }

   // clear the lights
   ResetSelectedLights();
   
   // clear the entities
   ResetSelectedEntities();
}

//-----------------------------------------------------------------

#define DISTANCE_THRESHHOLD (double)8.0
CLightEmitter * CTHREDDoc::SelectLight(CPoint point, CRenderCamera& Camera,
   BOOL fClear)
{
   // distance
   double ShortestDistance = 200000.0;
   double CurrentDistance;
   int CurrentLight, Light;
   ThredPoint MousePoint;

   // check the flag to clear the lights
   if( fClear )
   {
      // Deselect all other entities...
      mCurrentLight = -1;
      mSelectedLight = FALSE;
   
      // go through the lights and unselect them
	   for( Light = 0; Light < (*pLightArray).GetSize(); Light++)
         (*pLightArray)[ Light ].resetSelected();
   }

	MousePoint.X = MousePoint.Y = MousePoint.Z = 0;
	Camera.TranslateToWorld(point, MousePoint);

   // go through the array & check for hits
   for( Light = (*pLightArray).GetSize() - 1; Light >= 0; Light-- )
   {
      if( !(*pLightArray)[Light].selected() )
         continue;
         
      CurrentDistance = (*pLightArray)[ Light ].getDistance(MousePoint);
      if(CurrentDistance < ShortestDistance)
      {
         ShortestDistance = CurrentDistance;
         CurrentLight = Light;
      }
   }
	
   // check if need to go for a non-selected light
   if( ShortestDistance > DISTANCE_THRESHHOLD )
   {
      for( Light = (*pLightArray).GetSize() - 1; Light >= 0; Light-- )
      {
         if( (*pLightArray)[Light].selected() )
            continue;
            
         CurrentDistance = (*pLightArray)[ Light ].getDistance(MousePoint);
         if(CurrentDistance < ShortestDistance)
         {
            ShortestDistance = CurrentDistance;
            CurrentLight = Light;
         }
      }
   }
   
   // check if we should assign
   if(ShortestDistance <= DISTANCE_THRESHHOLD)
   {
      if( !( (*pLightArray)[CurrentLight].active() ) )
         return( NULL );
      
      // mark the emitter that can be selected
      CLightEmitter * pEmitter;
      pEmitter = ( *pLightArray )[ CurrentLight ].markSelected( MousePoint );
      
      if( fClear )
      {
         pEmitter->select( true );
         pEmitter->inspect(true,false);
         
         // select the light in the light viewer...
         CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

         // check if the objectviewer window is active
         if( pFrame && ( pFrame->m_wndLightView.IsOpen() ) )
         {
            pFrame->m_wndLightView.m_TreeWnd.SelectItem( pEmitter->hTreeItem );
            pFrame->m_wndLightView.m_TreeWnd.EnsureVisible( pEmitter->hTreeItem );
         }
         
         mCurrentLight = CurrentLight;
         mSelectedLight = TRUE;
      }
      
      // return light ptr for new light selection stuff in thredview.cpp
      return( pEmitter );
   }

   return( 0 );
}

//------------------------------------------------------------------
// move a light
void CTHREDDoc::MoveSelectedLights( double X, double Y, double Z )
{
   for( int Light = 0; Light < (*pLightArray).GetSize(); Light++)
      if( ( *pLightArray )[Light].selected() )
         ( *pLightArray )[Light].moveLight( X, Y, Z );

   // set the mod flag
   SetModifiedFlag();
}

//------------------------------------------------------------------

// done moving a light
void CTHREDDoc::DoneMoveLights( bool useEditBox, int viewType )
{
   for( int Light = 0; Light < (*pLightArray).GetSize(); Light++ )
   {
      if( (*pLightArray)[ Light ].selected() )
      {
         if( mUseGrid )
            (*pLightArray)[ Light ].doneMoveLight( mGridSize, useEditBox, viewType );
         else
            (*pLightArray)[ Light ].doneMoveLight( 1, useEditBox, viewType );
      }
   }
            
   SetModifiedFlag();
}

void CTHREDDoc::RotateSelectedLights( EulerF & rot, Point3F & center )
{
   // why degrees? argh!
   rot.x = ( rot.x * ( double )M_PI_VALUE ) / ( double )( M_TOTAL_DEGREES / 2 );
   rot.y = ( rot.y * ( double )M_PI_VALUE ) / ( double )( M_TOTAL_DEGREES / 2 );
   rot.z = ( rot.z * ( double )M_PI_VALUE ) / ( double )( M_TOTAL_DEGREES / 2 );
   
   for( int Light = 0; Light < (*pLightArray).GetSize(); Light++ )
      if( ( *pLightArray )[Light].selected() )
         (*pLightArray )[Light].rotateLight( rot, center );
   SetModifiedFlag();
}

void CTHREDDoc::DoneRotateSelectedLights( float snap )
{
   // to radians and beyond!
   snap = ( snap * ( double )M_PI_VALUE ) / (double)(M_TOTAL_DEGREES / 2 );
   for( int Light = 0; Light < (*pLightArray).GetSize(); Light++ )
      if( ( *pLightArray )[Light].selected() )
         (*pLightArray )[Light].doneRotateLight( snap );
   SetModifiedFlag();
}

// delete a light
void  CTHREDDoc::DeleteLight( int LightIndex )
{
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
   
   // go through and delete only the emitters....
   CLight * pLight = &( *pLightArray )[LightIndex];
   for( int i = 0; i < pLight->states.GetSize(); i++ )
   {
      bool removed = false;
      
      // go from end of array to start...
      CLightState * pState = &pLight->states[i];
      for( int j = pState->numEmitters() - 1; j >= 0; j-- )
      {
         // remove the emitter
         if( pState->emitters[j].getSelect() )
         {
            pState->emitters.RemoveAt( j );
            removed = true;
         }
      }
      
      // update
      if( pFrame && pFrame->m_wndLightView.IsOpen() )
         pFrame->m_wndLightView.m_TreeWnd.Rebuild( pState );
   }
}


void CTHREDDoc::Computelighting( int state )
{
//john  ThredPoint  lightpos;
//  double      lightdist1;
//  double      lightdist2;
//  double      lightpercentage;
//
//  // rebuild a quick bsp 
//  (*pWorldBsp).RebuildOptimal( NULL, 0, FALSE );
//   
//  // Clear all lighting
//  (*pWorldBsp).AssignGenericLightMaps( (*pWorldBsp).TreeTop, 0 ); 
//
//   // go through the lights
//   for( int light = 0; light < (*pLightArray).GetSize(); light++ )
//   {
//      if( ( (*pLightArray)[ light ].active() ) &&
//         ( (*pLightArray)[ light ].selected() ) )
//      {
//         // get the light to make life a little easier
//         CLight & Light = (*pLightArray)[ light ];
//
//         // get the state that should be rendered  ( 0 is default which
//         // uses
//         if( state < 0 )
//            state = 0;
//            
//         // make sure this state exits
//         if( Light.mStates.GetCount() <= state )
//            return;
//         
//         // get the state   
//         CLightState & LightState = Light.mStates.GetAt( Light.mStates.FindIndex( state ) );
//         
//         // check for position on state, or use default
//         if( LightState.mFlags & STATE_HASPOSITION )
//            lightpos = LightState.mPosition;
//         else lightpos = Light.mDefaultState.mPosition;
//
//         // check for state color, or use default
//         if( LightState.mFlags & STATE_HASCOLOR )
//         {
//            lightpercentage = ( LightState.mColor.red + 
//               LightState.mColor.green + LightState.mColor.blue ) / 3.0;
//         }         
//         else if( Light.mDefaultState.mFlags & STATE_HASCOLOR )
//         {
//            lightpercentage = ( Light.mDefaultState.mColor.red + 
//               Light.mDefaultState.mColor.green + 
//               Light.mDefaultState.mColor.blue ) / 3.0;
//         }
//         else lightpercentage = 1.0;
//
//         // do the distance, or default if none-existent
//         if( LightState.mFlags & STATE_LINEAR_FALLOFF )
//         {
//            lightdist1 = ( double( LightState.mFalloff1 ) * 
//               ( 1 / theApp.minintensity ) ); 
//            lightdist2 = ( double( LightState.mFalloff2 ) * 
//               ( 1 / theApp.minintensity ) ); 
//         }
//         else if( Light.mDefaultState.mFlags & STATE_LINEAR_FALLOFF )
//         {
//            lightdist1 = ( double( Light.mDefaultState.mFalloff1 ) * 
//               ( 1 / theApp.minintensity ) ); 
//            lightdist2 = ( double( Light.mDefaultState.mFalloff2 ) * 
//               ( 1 / theApp.minintensity ) ); 
//         }
//         else
//         {
//            // full bright
//            lightdist1 = 1000000.0;
//            lightdist2 = 1000000.0;
//         }
//            
//         // apply the color to this, not correct but looks ok
//         lightdist1 = lightdist1 * lightpercentage;
//         lightdist2 = lightdist2 * lightpercentage;
//         
//         (*pWorldBsp).CalculateLightMaps((*pWorldBsp).TreeTop, &lightpos, 
//            lightdist1, lightdist2, lightpercentage);
//      }
//   }
//#if 0
//  // Loop through all the lights
//  for (int Ent = 0; Ent < (*pEntityArray).GetSize(); Ent++)
//  {
//    // Make sure this is an active one...
//    if ((*pEntityArray)[Ent].mFlags & ENTITY_ACTIVE)
//    {
//      lightpos.X = (*pEntityArray)[Ent].mRenderOrigin.X;
//      lightpos.Y = (*pEntityArray)[Ent].mRenderOrigin.Y;
//      lightpos.Z = (*pEntityArray)[Ent].mRenderOrigin.Z;
//
//      // Calculate the lighting percentage of the total intensity...
//      CString RGBStr = (*pEntityArray)[Ent].ValueByKey("color");
//      double  r, g, b;
//	  sscanf(RGBStr, "%lf %lf %lf", &r, &g, &b);
//      lightpercentage = (r + g + b) / 3.0;
//
//      // Calculate the maximum distance this light will travel...
//      CString DistStr = (*pEntityArray)[Ent].ValueByKey("distance");
//      double  x, y, z;
//	  sscanf(DistStr, "%lf %lf %lf", &x, &y, &z);
//      lightdist = (1.0 / theApp.minintensity) / y;
//
//      // scale the maximum distance by the rgb weighting
//      lightdist = lightdist * lightpercentage;
//
//      (*pWorldBsp).CalculateLightMaps((*pWorldBsp).TreeTop, &lightpos, lightdist, lightpercentage);
//    }
//  }
//#endif
//
////  (*pWorldBsp).InvalidateNodeCacheKeys((*pWorldBsp).TreeTop);
//  UpdateAllViews(NULL);
}

//////////////////////////////////////////////////////////////////
// DETAIL INFORMATION
//////////////////////////////////////////////////////////////////

BOOL CTHREDDoc::DetailExist( CShapeDetail * pDetail )
{
   CShapeState * pState;
   CShapeDetail * pTraverse;
   
   pState = m_StateList;
   while( pState )
   {
      pTraverse = pState->mDetails;
      while( pTraverse )
      {
         if( pTraverse == pDetail )
            return( TRUE );
         pTraverse = pTraverse->pNext;
      }   
      pState = pState->pNext;
   }
   
   return( FALSE );
}

BOOL CTHREDDoc::CopyState( CShapeState * pState )
{
   CShapeState * pNew;
   
   if( !pState )
      return( FALSE );
   
   // clear the clip
   m_ObjectEdit.Clear();
   
   // create a new object
   pNew = new CShapeState;

   // copy all the info
   pNew->Name = pState->Name;
   pNew->pNext = NULL;
   pNew->pPrevious = NULL;
   pNew->hItem = NULL;
   pNew->pDoc = this;
   
   // traverse the detail list for pState and add to pNew
   CShapeDetail * pTraverse;
   CShapeDetail * pTail;
   pTraverse = pState->mDetails;
   
   // go through to the end
   while( pTraverse )
   {
      pState->CopyDetail( pTraverse );
      m_ObjectEdit.pDetail->pState = pNew;
      
      if( pTraverse == pState->mDetails )
      {
         pNew->mDetails = m_ObjectEdit.pDetail;
         pNew->mDetails->pPrevious = NULL;
         pNew->mDetails->pNext = NULL;
         pTail = pNew->mDetails;
      }
      else
      {
         pTail->pNext = m_ObjectEdit.pDetail;
         pTail->pNext->pPrevious = pTail;
         pTail = pTail->pNext;
         pTail->pNext = NULL;
      }
   
      m_ObjectEdit.pDetail = NULL;
         
      pTraverse = pTraverse->pNext;
   }
   
   // set the clip ptr
   m_ObjectEdit.pState = pNew;
   
   return( TRUE );
}

BOOL CTHREDDoc::PasteState( void )
{
   CShapeState * pState;
   CShapeState * pSave;
   
   if( !m_ObjectEdit.pState )
      return( FALSE );
      
   // copy and steal the ptr
   pSave = m_ObjectEdit.pState;
   m_ObjectEdit.pState = NULL;
   CopyState( pSave );
   pState = m_ObjectEdit.pState;
   m_ObjectEdit.pState = pSave;
   
   // add the state
   CShapeState * pNew;
   pNew = AddState( pState );   
   
   // go through and add all the details/lightsets to the object display
   CShapeDetail * pDetail;
   CLightSet * pLightSet;
   
   // get the main frame and then add this to the object view
   CMainFrame * pFrame = ( CMainFrame * )AfxGetMainWnd();
   
   // go through the details
   pDetail = pNew->mDetails;
   while( pDetail )
   {
      // add
      pFrame->m_wndObjectView.AddDetail( pNew, pDetail );
      
      // go through the lightsets
      pLightSet = pDetail->mLightSets;
      while( pLightSet )
      {
         // add
         pFrame->m_wndObjectView.AddLightSet( pDetail, pLightSet );
         pLightSet = pLightSet->pNext;
      }
      
      pDetail = pDetail->pNext;
   }
               
	SetModifiedFlag();
   return( TRUE );
}

// add an existing state
CShapeState * CTHREDDoc::AddState( CShapeState * pState )
{
   if( !m_StateList )
   {
      m_StateList = pState;
      pState->pNext = NULL;
      pState->pPrevious = NULL;
   }
   else
   {
      CShapeState * pTraverse = m_StateList;

      // get to last element
      while( pTraverse->pNext )
         pTraverse = pTraverse->pNext;
         
      pTraverse->pNext = pState;
      pState->pPrevious = pTraverse;
      pState->pNext = NULL;
   }
   
   // set the doc ptr
   pState->pDoc = this;

   CMainFrame * pFrame = ( CMainFrame * )AfxGetMainWnd();
   pFrame->m_wndObjectView.AddState( pState );
   
//   // rename all the states
//   NameStates();
      
	SetModifiedFlag();
   return( pState );
}

// add a state to the list of states
CShapeState * CTHREDDoc::AddState( CString Name )
{
   // create a state
   CShapeState  * pState;
   int Count = 0;
   pState = m_StateList;
   while( pState )
   {
      Count++;
      pState = pState->pNext;
   }
   pState = new CShapeState;

   if( !Name.GetLength() )
      Name.Format( "State %d", Count );

   pState->Name = Name;

	SetModifiedFlag();
   return( AddState( pState ) );
}

void CTHREDDoc::NameStates( void )
{
   CShapeState * pState;
   int            Count = 0;
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
   
   pState = m_StateList;
   while( pState )
   {
      pState->Name.Format( "State %d", Count++ );
      pFrame->m_wndObjectView.m_TreeWnd.SetItemText( pState->hItem, pState->Name );
      pState = pState->pNext;
   }
}

int CShapeState::NumDetails( void )
{
   CShapeDetail * pDetail;
   int i = 0;
   for( pDetail = mDetails; pDetail; pDetail = pDetail->pNext )
      i++;
      
   return( i );
}

void CShapeState::NameDetails( void )
{
   CShapeDetail * pDetail;
   int            Count = 0;
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
   
#if 1
   // do a sort on the details
   CShapeDetail * pNewList;
   
   pDetail = mDetails;
   pNewList = NULL;
   
   while( pDetail )
   {
      if( !pNewList )
      {
         pNewList = pDetail;
         pDetail = pDetail->pNext;
         pNewList->pNext = NULL;
         pNewList->pPrevious = NULL;
      }
      else
      {
         CShapeDetail * pTraverse;
         CShapeDetail * pLast;
         
         pTraverse = pNewList;
         
         while( pTraverse )
         {
            if( pDetail->m_minPixels > pTraverse->m_minPixels )
            {
               pDetail->pPrevious = pTraverse->pPrevious;
               if( pDetail->pPrevious )
                  pDetail->pPrevious->pNext = pDetail;
               else
                  pNewList = pDetail;
               pTraverse->pPrevious = pDetail;
               pDetail = pDetail->pNext;
               pTraverse->pPrevious->pNext = pTraverse;               
			   break;
            }
            pLast = pTraverse;
            pTraverse = pTraverse->pNext;
         }
         
         if( !pTraverse && pDetail )
         {
            pLast->pNext = pDetail;
            pDetail->pPrevious = pLast;
            pDetail = pDetail->pNext;
            pLast->pNext->pNext = NULL;
         }
      }
   }
   mDetails = pNewList;
   
   // go through and remove all the items in the list
   pDetail = mDetails;
   while( pDetail )
   {
      pFrame->m_wndObjectView.RemoveDetail( pDetail );
      pDetail = pDetail->pNext;
   }
   
   // go through and add them back in ( now in order )
   pDetail = mDetails;
   while( pDetail )
   {
      pFrame->m_wndObjectView.AddDetail( pDetail->pState, pDetail );
      
      // go thru the lightsets
      CLightSet * pLightSet;
      pLightSet = pDetail->mLightSets;
      while( pLightSet )
      {
         pFrame->m_wndObjectView.AddLightSet( pDetail, pLightSet );
         pLightSet = pLightSet->pNext;
      }
      
      // add to list
      pDetail = pDetail->pNext;
   }
#endif
   
   // name them
   pDetail = mDetails;
   while( pDetail )
   {
      pDetail->Name.Format( "Detail %d ( %d )", Count++, 
         pDetail->m_minPixels );
      pFrame->m_wndObjectView.m_TreeWnd.SetItemText( pDetail->hItem, 
         pDetail->Name );
      pDetail = pDetail->pNext;
   }
   
   // update the titles
   if( mDetails && mDetails->pState )
      mDetails->UpdateTitles();
   
}

void CShapeDetail::NameLightSets( void )
{
   CLightSet * pLightSet;
   int Count = 0;
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

   pLightSet = mLightSets;
   while( pLightSet )
   {
      pLightSet->Name.Format( "LightSet %d", Count++ );
      pFrame->m_wndObjectView.m_TreeWnd.SetItemText( pLightSet->hItem, 
         pLightSet->Name );
      pLightSet = pLightSet->pNext;
   }
   
}

// remove a state
BOOL CTHREDDoc::RemoveState( CShapeState * pState )
{  
   if( !pState )
      return( FALSE );
      
   if( pState->pPrevious )
      pState->pPrevious->pNext = pState->pNext;
   if( pState->pNext )
      pState->pNext->pPrevious = pState->pPrevious;
      
   // check for top of list
   if( pState == m_StateList )
      m_StateList = pState->pNext;
      
   // remove it ( do before removing from object viewer )
   while( pState->mDetails )
      pState->RemoveDetail( pState->mDetails );
   
   // get the main frame and then remove this from the object view
   CMainFrame * pFrame = ( CMainFrame * )AfxGetMainWnd();
   pFrame->m_wndObjectView.RemoveState( pState );

   // delete it
   delete pState;
   
//   // rename all the states
//   NameStates();
   
	SetModifiedFlag();
   return( TRUE );
}

int CTHREDDoc::NumDetails()
{
   CShapeState * pStateTraverse;
   CShapeDetail * pDetailTraverse;
   int count = 0;
             
   // go through and check for another detail in the file
   pStateTraverse = m_StateList;
   while( pStateTraverse )
   {
      pDetailTraverse = pStateTraverse->mDetails;
      while( pDetailTraverse )
      {
         count++;
         pDetailTraverse = pDetailTraverse->pNext;
      }
      pStateTraverse = pStateTraverse->pNext;
   }
   
   return( count );
}


CShapeDetail::CShapeDetail( class CShapeState * pShapeState )
{
   // settings ( for the dialog )
   m_maximumTexMip = 0;
   m_linkableFaces = 0xfc;
   m_minPixels = 0xff;
   
	mBuiltEditBox = FALSE;
   
   // grab the state ptr
   pState = pShapeState;
   
   mLightSets = NULL;

   mCurrentTool = ID_EDIT_MODE_BUTTON;
   mShowBrush = TRUE;
   mOldTool = ID_TOOLS_BRUSH_MOVEROTATEBRUSH;
   edit_state = ES_RESIZE_MOVE;

   // set the adjustment mode
   mAdjustmentMode = 0;
   
   // No brushes in copy buffer
   mBrushInBuffer = 0;

   // point to the regular brush when we start
   mCurrentBrush = &mRegularBrush;

   // turn off group movement on start
   mGroupMovement = 0;

   pState->pDoc->mBoxCreation.CreateBox( mCurrentBrush );
   mBrushList = NULL;
   mCurrentBrush->TransformBrushPolygons();

   // our grid setup
   mUseGrid = 1;		// DPW - Default to use snap-to-grid
   
   // get these valuse from the document ( they are saved in .zvl files )
   mGridSize = pShapeState->pDoc->mGridSize;
   mCurrentGroup = pShapeState->pDoc->mCurrentGroup;
   mShow = pShapeState->pDoc->mShow;

   // set the id of our current brush
   mCurrentBrushId = 0;

   // set the light information
   mLightArray.SetSize( 0, 20 );
   mShowLights = TRUE;
   mShowLights3D = TRUE;
   mCurrentLight = -1;
   mSelectedLight = FALSE;

   // set our entity list to nata
   mEntityArray.SetSize(0, 20);
   mCurrentEntity = -1;
   mSelectedEntity = FALSE;

   // show them
   mShowEntities = TRUE;
   mShowEntities3D = TRUE;

   // we have no current group
   mBrushGroupArray.SetSize(0,20);

   // we are currently showing everything
   EditBox.ClearBox();
      
   // DPW - Grid info
   mGridType = USE_LINE_GRID;
   mVolumeShow = TRUE;
   mFilledVolumeShow = FALSE;
	
   // set the active lightset to null
   pActiveLightSet = NULL;
}

// remove all light sets
CShapeDetail::~CShapeDetail()
{
   CLightSet * pLightSet;
   
   while( mLightSets )
   {
      pLightSet = mLightSets;
      mLightSets = mLightSets->pNext;
      delete pLightSet;
   }
   mLightSets = NULL;

	CThredBrush* Brush;
	CThredBrush* NextBrush;

	Brush = mBrushList;
	while(Brush) {
		NextBrush = Brush->GetNextBrush();
		delete Brush;
		Brush = NextBrush;
	}
}

// copy a lightset to the clip
BOOL CShapeDetail::CopyLightSet( CLightSet * pLightSet )
{
   return( true );
   
//john   BOOL  Active;
//   
//   if( !pLightSet )
//      return( FALSE );
//   CTHREDDoc * pDoc = pState->pDoc;
//   
//   Active = pLightSet->mActive;
//   
//   // check if active or not, if active deactivate temp. for the copy to wrk
//   if( Active )
//      if( this != pDoc->pActiveDetail )
//         return( FALSE );
//      else
//         pLightSet->SetActive( FALSE );
//
//   // remove the lightset that may be in the clip
//   pDoc->m_ObjectEdit.Clear();
//   
//   // create a new object
//   pDoc->m_ObjectEdit.pLightSet = new CLightSet;
//   
//   // copy the info
//   pDoc->m_ObjectEdit.pLightSet->Name = pLightSet->Name;
//
//   // go through and copy all the lights
//   CLight * pTraverse;
//   CLight * pTail;
//   CLight * pNewLight;
//   CLightSet * pNew = pDoc->m_ObjectEdit.pLightSet;
//   pNew->pLights = NULL;
//   
//   pTraverse = pLightSet->pLights;
//   while( pTraverse )
//   {
//      // create a new light and copy 
//      pNewLight = new CLight( FALSE );
//      *pNewLight = *pTraverse;      
//      
//      if( !pNew->pLights )
//      {
//         pNew->pLights = pNewLight;
//         pNew->pLights->pPrevious = NULL;
//         pNew->pLights->pNext = NULL;
//         pTail = pNew->pLights;
//      }
//      else
//      {
//         pTail->pNext = pNewLight;
//         pTail->pNext->pPrevious = pTail;
//         pTail = pTail->pNext;
//         pTail->pNext = NULL;
//      }
//      
//      pTraverse = pTraverse->pNext;
//   }
//      
//   // check if need to re-activate the lightset      
//   if( Active )
//      pLightSet->SetActive( TRUE );
//      
//   return( TRUE );           
}

// paste a lightset fromt he clip stuff
BOOL CShapeDetail::PasteLightSet( void )
{
//john
//
//   if( !pState->pDoc->m_ObjectEdit.pLightSet )
//      return( FALSE );
//      
//   CLightSet * pSave;
//   CLightSet * pNew;
//   
//   // copy the light set using the edit/copy and swapping ptrs
//   pSave = pState->pDoc->m_ObjectEdit.pLightSet;
//   pState->pDoc->m_ObjectEdit.pLightSet = NULL;
//   CopyLightSet( pSave );
//   pNew = pState->pDoc->m_ObjectEdit.pLightSet;
//   pState->pDoc->m_ObjectEdit.pLightSet = pSave;
//   
//   // add it to the list
//   AddLightSet( pNew );
//
//   // go through and copy all the lights
//   CLight * pTraverse;
//   CLight * pTail;
//   CLight * pNewLight;
//   pNew->pLights = NULL;
//   
//   pTraverse = pSave->pLights;
//   while( pTraverse )
//   {
//      // create a new light and copy 
//      pNewLight = new CLight( FALSE );
//      *pNewLight = *pTraverse;      
//      
//      if( !pNew->pLights )
//      {
//         pNew->pLights = pNewLight;
//         pNew->pLights->pPrevious = NULL;
//         pNew->pLights->pNext = NULL;
//         pTail = pNew->pLights;
//      }
//      else
//      {
//         pTail->pNext = pNewLight;
//         pTail->pNext->pPrevious = pTail;
//         pTail = pTail->pNext;
//         pTail->pNext = NULL;
//      }
//      
//      pTraverse = pTraverse->pNext;
//   }
//   
//	pState->pDoc->SetModifiedFlag();
   return( TRUE );
}
   
// add an existing lightset
CLightSet * CShapeDetail::AddLightSet( CLightSet * pLightSet )
{
//john
//   // check for creation
//   if( !pLightSet )
//      return NULL;
//      
//   if( !mLightSets )
//   {
//      mLightSets = pLightSet;
//      pLightSet->pNext = NULL;
//      pLightSet->pPrevious = NULL;
//   }
//   else
//   {
//      CLightSet * pTraverse = mLightSets;
//      while( pTraverse->pNext )
//         pTraverse = pTraverse->pNext;
//      pTraverse->pNext = pLightSet;
//      pLightSet->pPrevious = pTraverse;
//      pLightSet->pNext = NULL;
//   }
//   
//   
//   // set the detail this set belongs to
//   pLightSet->pDetail = this;
//   
//   // get the main frame and then add this to the object view
//   CMainFrame * pFrame = ( CMainFrame * )AfxGetMainWnd();
//   pFrame->m_wndObjectView.AddLightSet( this, pLightSet );
//      
////   // update the names for the lightsets
////   NameLightSets();
//   
//	pState->pDoc->SetModifiedFlag();
//   return( pLightSet );
   return( 0 );
}

// add a new lightset
CLightSet * CShapeDetail::AddLightSet( CString Name )
{
   CLightSet * pLightSet;
   pLightSet = mLightSets;
   int Count = 0;
   while( pLightSet )
   {
      Count++;
      pLightSet = pLightSet->pNext;
   }
   pLightSet = new CLightSet;
   
   // set the name
   if( !Name.GetLength() )
      Name.Format( "LightSet %d", Count );
   
   // set the info
   pLightSet->Name = Name;

	pState->pDoc->SetModifiedFlag();
   return( AddLightSet( pLightSet ) );
}

BOOL CShapeDetail::RemoveLightSet( CLightSet * pLightSet )
{
   if( !pLightSet )
      return( FALSE );

   if( pLightSet->mActive )
      pLightSet->SetActive( FALSE );
            
   if( pLightSet->pPrevious )
      pLightSet->pPrevious->pNext = pLightSet->pNext;
   if( pLightSet->pNext )
      pLightSet->pNext->pPrevious = pLightSet->pPrevious;
      
   // check for top of list
   if( pLightSet == mLightSets )
      mLightSets = pLightSet->pNext;
      
   // get the main frame and then remove this from the object view
   CMainFrame * pFrame = ( CMainFrame * )AfxGetMainWnd();
   pFrame->m_wndObjectView.RemoveLightSet( pLightSet );
   
   // remove it ( should be ok to delete )
   delete pLightSet;

//   // rename all the lightset
//   NameLightSets();
   
	pState->pDoc->SetModifiedFlag();
   return( TRUE );
}

BOOL CShapeDetail::SetInfo( void )
{
   CTHREDDoc * pDoc;
   
   // get the doc pointer
   pDoc = pState->pDoc;
   
   // terrible way to do things, but here we go
   pDoc->mCurrentTool = mCurrentTool;
   pDoc->mShowBrush = mShowBrush;
   pDoc->mOldTool = mOldTool;
   pDoc->mUseGrid = mUseGrid;
   pDoc->mGridType = mGridType;
   pDoc->mGridSize = mGridSize;

   // copy the regular brush
   pDoc->mRegularBrush = mRegularBrush;
   
   // get the ptrs back
   pDoc->mRegularBrush.NextSelectedBrush = mRegularBrush.NextSelectedBrush;
   pDoc->mRegularBrush.PrevSelectedBrush = mRegularBrush.PrevSelectedBrush;
   pDoc->mRegularBrush.PrevBrush = mRegularBrush.PrevBrush;
   pDoc->mRegularBrush.NextBrush = mRegularBrush.NextBrush;

   pDoc->mCurrentBrush = mCurrentBrush;
   pDoc->mBrushList = mBrushList;
   if( mCurrentBrush == &mRegularBrush )
      pDoc->mCurrentBrush = &pDoc->mRegularBrush;
   if( mBrushList == &mRegularBrush )
      pDoc->mBrushList = &pDoc->mRegularBrush;
   pDoc->EditBox = EditBox;
   pDoc->edit_state = edit_state;
   pDoc->mVolumeShow = mVolumeShow;
   pDoc->mFilledVolumeShow = mFilledVolumeShow;
   pDoc->mCurrentGroup = mCurrentGroup;
   pDoc->mShow = mShow;
   pDoc->mLastOp = mLastOp;
   pDoc->mUseRotationSnap = mUseRotationSnap;
   pDoc->mCurrentEntity = mCurrentEntity;
   pDoc->mSelectedEntity = mSelectedEntity;
   pDoc->mShowEntities = mShowEntities;
   pDoc->mShowEntities3D = mShowEntities3D;
   pDoc->mCurrentLight = mCurrentLight;
   pDoc->mShowLights = mShowLights;
   pDoc->mShowLights3D = mShowLights3D;
   pDoc->mSelectedLight = mSelectedLight;
   pDoc->mAdjustmentMode = mAdjustmentMode;
   pDoc->mBrushInBuffer = mBrushInBuffer;
   pDoc->mGroupMovement = mGroupMovement;
   pDoc->m_GSceneLights = m_GSceneLights;
   pDoc->m_LMHandle = m_LMHandle;
   pDoc->mCurrentBrushId = mCurrentBrushId;

   // set the pointers to our array's and such
   pDoc->pLightArray = &mLightArray;
   pDoc->pEntityArray = &mEntityArray;
   pDoc->pBrushGroupArray = &mBrushGroupArray;
   pDoc->pWorldBsp = &mWorldBsp;

   if( pActiveLightSet )
      pActiveLightSet->SetActive( TRUE );
      
   return( TRUE );
}

BOOL CShapeDetail::SetActive( void )
{
   if( !pState || !pState->pDoc )
      return( FALSE );
      
   // save the current info ( if there is any )
   if( pState->pDoc->pActiveDetail )
      pState->pDoc->pActiveDetail->GetInfo();
   
   if( !SetInfo() )
      return( FALSE );

   if( !mBuiltEditBox )
   {
      mBuiltEditBox = TRUE;
      pState->pDoc->BuildEditBox(TRUE);
   }   

   // set the brush list
   pState->pDoc->pWorldBsp->SetBrushList( pState->pDoc->mBrushList );
   
   // set this as the active detail
	pState->pDoc->pActiveDetail = this;
   
   // turn on waiting cursor
   SetCursor( LoadCursor( NULL, IDC_WAIT ) );
   
//   // rebuild the bsp
//   mWorldBsp.ClearBsp();
//   mWorldBsp.RebuildQuick( NULL, 0, TRUE );
   
   // update the child frame ptr
	CMainFrame* pMain = ( CMainFrame* )AfxGetMainWnd();

   if( !pMain )
      return( FALSE );
      
   // get the current light array
   pMain->m_wndLightView.Rebuild( pState->pDoc, mLightArray );
   
   CChildFrame * pFrame = ( CChildFrame * )pMain->MDIGetActive();
   
   if( !pFrame )
      return( FALSE );
 
   pFrame->pCurrentDetail = this;

   CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
   
   CShapeState * pStateTraverse;
   CShapeDetail * pDetailTraverse;
   
   // go through and reset all images that are set
   pStateTraverse = pState->pDoc->m_StateList;
   while( pStateTraverse )
   {
      pDetailTraverse = pStateTraverse->mDetails;
      while( pDetailTraverse )
      {
         int Image, SelectImage;
         pMainFrame->m_wndObjectView.m_TreeWnd.GetItemImage( 
            pDetailTraverse->hItem, Image, SelectImage );
         if( Image == CObjectView::DETAIL )
         {
            pMainFrame->m_wndObjectView.m_TreeWnd.SetItemImage( 
               pDetailTraverse->hItem, 
               CObjectView::DETAIL_OFF, CObjectView::DETAIL_OFF );
         }
         
         pDetailTraverse = pDetailTraverse->pNext;
      }
      pStateTraverse = pStateTraverse->pNext;
   }
   
   // set this image
   pMainFrame->m_wndObjectView.m_TreeWnd.SetItemImage( hItem, 
      CObjectView::DETAIL, CObjectView::DETAIL );
 
   // update the documetn
   pState->pDoc->UpdateAllViews( NULL );
   
   UpdateTitles();
   
   // turn off waiting cursor
   SetCursor( LoadCursor( NULL, IDC_ARROW ) );
   
   return( TRUE );
}

BOOL CShapeDetail::GetInfo( void )
{
   CTHREDDoc * pDoc;
   
   // get the doc pointer
   pDoc = pState->pDoc;
   
   mCurrentTool = pDoc->mCurrentTool;
   mShowBrush = pDoc->mShowBrush;
   mOldTool = pDoc->mOldTool;
   mUseGrid = pDoc->mUseGrid;
   mGridType = pDoc->mGridType;
   mGridSize = pDoc->mGridSize;
   
   // copy the regular brush
   mRegularBrush = pDoc->mRegularBrush;
   
   // get the ptrs ( usually a no-no )
   mRegularBrush.NextSelectedBrush = pDoc->mRegularBrush.NextSelectedBrush;
   mRegularBrush.PrevSelectedBrush = pDoc->mRegularBrush.PrevSelectedBrush;
   mRegularBrush.PrevBrush = pDoc->mRegularBrush.PrevBrush;
   mRegularBrush.NextBrush = pDoc->mRegularBrush.NextBrush;
   
   mCurrentBrush = pDoc->mCurrentBrush;
   if( pDoc->mCurrentBrush == &pDoc->mRegularBrush )
      mCurrentBrush = &mRegularBrush;
   mBrushList = pDoc->mBrushList;
   if( pDoc->mBrushList == &pDoc->mRegularBrush )
      mBrushList = &mRegularBrush;
   EditBox = pDoc->EditBox;
   edit_state = pDoc->edit_state;
   mVolumeShow = pDoc->mVolumeShow;
   mFilledVolumeShow = pDoc->mFilledVolumeShow;
   mCurrentGroup = pDoc->mCurrentGroup;
   mShow = pDoc->mShow;
   mLastOp = pDoc->mLastOp;
   mUseRotationSnap = pDoc->mUseRotationSnap;
   mCurrentEntity = pDoc->mCurrentEntity;
   mSelectedEntity = pDoc->mSelectedEntity;
   mShowEntities = pDoc->mShowEntities;
   mShowEntities3D = pDoc->mShowEntities3D;
   mCurrentLight = pDoc->mCurrentLight;
   mShowLights = pDoc->mShowLights;
   mShowLights3D = pDoc->mShowLights3D;
   mSelectedLight = pDoc->mSelectedLight;
   mAdjustmentMode = pDoc->mAdjustmentMode;
   mBrushInBuffer = pDoc->mBrushInBuffer;
   mGroupMovement = pDoc->mGroupMovement;
   m_GSceneLights = pDoc->m_GSceneLights;
   m_LMHandle = pDoc->m_LMHandle;
   mCurrentBrushId = pDoc->mCurrentBrushId;
   
   if( pActiveLightSet )
      pActiveLightSet->SetActive( FALSE );
      
   return( TRUE );
}

CShapeState::CShapeState()
{
   mDetails = NULL;
}

// remove all details
CShapeState::~CShapeState()
{
   CShapeDetail * pDetail;
   
   while( mDetails )
   {
      pDetail = mDetails;
      mDetails = mDetails->pNext;
      delete pDetail;
   }
}

// add an already existing ( paste, load... ) detail
CShapeDetail * CShapeState::AddDetail( CShapeDetail * pDetail )
{
   if( !pDetail ) 
      return( NULL );
      
   pDetail->pState = this;
   
//   if( !pDetail->Name.GetLength() )
//      pDetail->Name.Format( "Detail %d", mDetailID++ );

   if( !mDetails )
   {
      mDetails = pDetail;
      pDetail->pPrevious = NULL;
      pDetail->pNext = NULL;
   }
   else
   {
      // insert at end
      CShapeDetail * pTraverse = mDetails;
      while( pTraverse->pNext )
         pTraverse = pTraverse->pNext;
      pTraverse->pNext = pDetail;
      pDetail->pPrevious = pTraverse;
      pDetail->pNext = NULL;      
   }

   // add the deatil to the object view
   CMainFrame * pFrame = ( CMainFrame * )AfxGetMainWnd();
   pFrame->m_wndObjectView.AddDetail( this, pDetail );
   
   // name the details
   NameDetails();
   
	pDoc->SetModifiedFlag();
   return( pDetail );
}

CShapeDetail * CShapeState::PasteDetail( void )
{
   CShapeDetail * pSave;
   CShapeDetail * pNew;
   
   if( !pDoc->m_ObjectEdit.pDetail )
      return( NULL );
      
   // copy the light set using the edit/copy and swapping ptrs
   pSave = pDoc->m_ObjectEdit.pDetail;
   pDoc->m_ObjectEdit.pDetail = NULL;
   pSave->pState = this;
   CopyDetail( pSave );
   pNew = pDoc->m_ObjectEdit.pDetail;
   pNew->pState = this;
   pDoc->m_ObjectEdit.pDetail = pSave;
   
   // add it to the list
   AddDetail( pNew );
   
   // go through and add all the lights to the object display
   CLightSet * pLights;
   
   // get the main frame and then add this to the object view
   CMainFrame * pFrame = ( CMainFrame * )AfxGetMainWnd();
   pLights = pNew->mLightSets;
   while( pLights )
   {
      pFrame->m_wndObjectView.AddLightSet( pNew, pLights );
      pLights = pLights->pNext;
   }
   
   NameDetails();
	pDoc->SetModifiedFlag();
   return( pNew );
}

// copy a detail level to the 'clipboard'
BOOL CShapeState::CopyDetail( CShapeDetail * pDetail )
{
   CShapeDetail * pNew;
   
   if( !pDetail )
      return( FALSE );
   
   // remove the detail that may be in the clip
   pDoc->m_ObjectEdit.Clear();
   
   // create a new object
   pNew = new CShapeDetail( this );
   
   // copy the info by setting the detail, copy it, then setting the old back
   if( !pDoc->pActiveDetail )
      return( FALSE );
   pDoc->pActiveDetail->GetInfo();
   pDetail->SetInfo();
   pNew->GetInfo();   
   pDoc->pActiveDetail->SetInfo();

   // copy and set the other info
   pNew->Name = pDetail->Name;
   pNew->mBuiltEditBox = pDetail->mBuiltEditBox;
   pNew->hItem = NULL;
   pNew->pState = this;
   pNew->mLightSets = NULL;
   pNew->m_maximumTexMip = pDetail->m_maximumTexMip;
   pNew->m_linkableFaces = pDetail->m_linkableFaces;
   pNew->m_minPixels = pDetail->m_minPixels;
   pNew->mBrushList = NULL;
   
   // copy the brush list
   CThredBrush * pTraverse = pDetail->mBrushList;
   CThredBrush * pCurrent = NULL;
   
   while( pTraverse )
   {
      // add to the end of the list ( will create inorder copy )
      if( !pCurrent )
      {
         // create a new brush and copy
         pCurrent = new CThredBrush;
         *pCurrent = *pTraverse;
         pNew->mBrushList = pCurrent;
         pCurrent->PrevBrush = NULL;
         pCurrent->NextBrush = NULL;
      }
      else
      {
         pCurrent->NextBrush = new CThredBrush;
         *pCurrent->NextBrush = *pTraverse;
         pCurrent->NextBrush->PrevBrush = pCurrent;
         pCurrent = pCurrent->NextBrush;
         pCurrent->NextBrush = NULL;
      }
   
      // get the next brush
      pTraverse = pTraverse->NextBrush;
   }
   
      
   // set the current brush to the regular brush ( default to this )
   pNew->mRegularBrush = pDetail->mRegularBrush;
   pNew->mCurrentBrush = &pNew->mRegularBrush;
   
   // copy the arrays
   for( int count = 0; count < pDetail->mLightArray.GetSize();count++ )
      pNew->mLightArray.Add( pDetail->mLightArray[count] );
   for( count = 0; count < pDetail->mBrushGroupArray.GetSize();count++ )
      pNew->mBrushGroupArray.Add( pDetail->mBrushGroupArray[ count ] );
   for( count = 0; count < pDetail->mEntityArray.GetSize();count++ )
      pNew->mEntityArray.Add( pDetail->mEntityArray[ count ] );
   
   // copy all the light sets
   CLightSet * pSet;
   CLightSet * pTail;
   
   pSet = pDetail->mLightSets;
   while( pSet )
   {
      // copy then add
      pDetail->CopyLightSet( pSet );
      pDoc->m_ObjectEdit.pLightSet->pDetail = pNew;
      
      // now add to the tail
      if( !pNew->mLightSets )
      {
         pNew->mLightSets = pDoc->m_ObjectEdit.pLightSet;
         pTail = pNew->mLightSets;
         pTail->pNext = NULL;
         pTail->pPrevious = NULL;
      }
      else
      {
         pTail->pNext = pDoc->m_ObjectEdit.pLightSet;
         pTail->pNext->pPrevious = pTail;
         pTail->pNext->pNext = NULL;
         pTail = pTail->pNext;
      }
      
      // set the clip ptr to null
      pDoc->m_ObjectEdit.pLightSet = NULL;
      
      pSet = pSet->pNext;
   }
   
   // set the ptr in the clip
   pDoc->m_ObjectEdit.pDetail = pNew;
   
   // go through and copy all the 
   return( TRUE );
}

CLightSet * CShapeDetail::GetLightSetByName( CString Name )
{
   CLightSet * pTraverse = mLightSets;
   while( pTraverse )
   {
      if( Name == pTraverse->Name )
         return( pTraverse );
      pTraverse = pTraverse->pNext;
   }
   return( NULL );
}

// add a new detail
CShapeDetail * CShapeState::AddDetail( CString Name )
{
   CShapeDetail * pDetail = new CShapeDetail( this );
   
//   pDetail->Name = Name;

	pDoc->SetModifiedFlag();
   return( AddDetail( pDetail ) );   
}

BOOL CShapeDetail::UpdateTitles( void )
{
   CString Title;
   
   // go through and name all the child windows to the name of the
   // detail attached to it ( have to go through the views )
   CTHREDDoc * pDoc = pState->pDoc;
   
   POSITION pos = pDoc->GetFirstViewPosition();
   
   while( pos != NULL )    
   {
      CTHREDView * pView = ( CTHREDView * )pDoc->GetNextView( pos );
      
      // there will b several circumstances in which this SHOULD fail,
      // so put checks for all the ptrs that can be null during the process
      // of loading up a file from disk
      if( pView && pView->pFrame && pView->pFrame->pCurrentDetail &&
         pDoc->pActiveDetail )
      {
         Title.Format( "%s - %s  %s", pView->pFrame->pCurrentDetail->pState->Name,
            pView->pFrame->pCurrentDetail->Name, pDoc->GetTitle() );
         pView->pFrame->SetWindowText( Title );
      }
   }      
   
   return( TRUE );
}

// import an existing 3dt file into a detail for this state
BOOL CShapeState::ImportDetail( void )
{
   static char szFilter[] = "3DT Files (*.3dt)|*.3dt|All Files (*.*)|*.*||";

   // create the dialog
   CFileDialog FileDlg( TRUE, "3DT", NULL, OFN_HIDEREADONLY | 
      OFN_FILEMUSTEXIST, szFilter, NULL );
      
   // get the info
   if( FileDlg.DoModal() != IDOK )
      return( FALSE );
      
   CString Filename;
   CString Base;
   
   Filename = FileDlg.GetPathName();
   Base = FileDlg.GetFileTitle();
   
   // open the file
   CFile file;
   if( !file.Open( Filename, CFile::modeRead ) )
      return( FALSE );
   
   // add the detail
   CShapeDetail * pDetail = AddDetail( "" );
   
   // create a carchive object
   CArchive Archive( &file, CArchive::load );
   
   // create a parser object
   CThredParser Parser( &Archive, NULL );

   // parse the object
   pDetail->SerializeOld( Parser );
   
	pDoc->SetModifiedFlag();
   return( TRUE );
}
      
BOOL CShapeState::RemoveDetail( CShapeDetail * pDetail )
{
   if( pDetail == mDetails )
      mDetails = pDetail->pNext;
      
   if( pDetail->pPrevious )
      pDetail->pPrevious->pNext = pDetail->pNext;
   if( pDetail->pNext )
      pDetail->pNext->pPrevious = pDetail->pPrevious;
   
   // check if the active detail - find one and use it, if there
   // are none
   pDoc->pActiveDetail = NULL;
   
   CShapeState * pState;
   pState = pDoc->m_StateList;
   while( pState )
   {
      if( pState->mDetails )
      {
         pState->mDetails->SetActive();
         break;
      }
      pState = pState->pNext;
   }
      
   // remove the light sets - do before removing from objectviewer
   while( pDetail->mLightSets )
      pDetail->RemoveLightSet( pDetail->mLightSets );
      
   // remove the deatil from the object view
   CMainFrame * pFrame = ( CMainFrame * )AfxGetMainWnd();
   pFrame->m_wndObjectView.RemoveDetail( pDetail );

   // delete it
   delete pDetail;
   
   // rename the details
   NameDetails();
   
	pDoc->SetModifiedFlag();
   return TRUE;
}

// add a light to this set
BOOL CLightSet::AddLight( CLight * pLight )
{
//john   CLight * pNew = new CLight( FALSE );
//   
//   // copy it
//   *pNew = *pLight;
//   
//   if( !pLights )
//   {
//      pLights = pNew;
//      pNew->pPrevious = NULL;
//      pNew->pNext = NULL;
//   }
//   else
//   {
//      pNew->pNext = pLights;
//      pNew->pPrevious = NULL;
//      pLights = pNew;
//   }
//   
//	pDetail->pState->pDoc->SetModifiedFlag();
   return( TRUE );
}

CLightSet::CLightSet()
{
   pLights = NULL;
   mActive = FALSE;
   pNext = NULL;
   pPrevious = NULL;
   hItem = NULL;
   mActive = FALSE;
   pDetail = NULL;
}

CLightSet::~CLightSet()
{
//john   // delete all lights associated with this light set
//   CLight * pTraverse;
//   pTraverse = pLights;
//   while( pTraverse )
//   {
//      pLights = pTraverse;
//      pTraverse = pTraverse->pNext;
//      delete pLights;
//   }
}

void CLightSet::SetActive( BOOL fActive )
{
//john   CLightSet * pTraverse = pDetail->mLightSets;
//	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
//   CTHREDDoc * pDoc = pDetail->pState->pDoc;
//
//   if( pDetail != pDoc->pActiveDetail )
//      return;
//      
//   // check if this is being activated
//   if( fActive )
//   {
//      // save off the other one
//      if( pDetail->pActiveLightSet && ( pDetail->pActiveLightSet != this ) )
//         pDetail->pActiveLightSet->SetActive( FALSE );
//         
//      pDetail->pActiveLightSet = this;
//      
//      CLight * pTraverse;
//      pTraverse = pLights;
//      
//      // go through and copy and delete
//      while( pTraverse )
//      {
//         // add it
//         pDoc->pLightArray->Add( *pTraverse );
//         
//         pLights = pTraverse;
//         
//         pTraverse = pTraverse->pNext;
//
//         // remove it
//         delete pLights;
//      }
//      
//      pLights = NULL;
//      
//      pFrame->m_wndObjectView.m_TreeWnd.SetItemImage( hItem, 
//         CObjectView::LIGHT_SET,  CObjectView::LIGHT_SET );
//   }
//   else
//   {
//      int Light;
//      pDetail->pActiveLightSet = NULL;
//
//      // change the image on teh tree-view
//      pFrame->m_wndObjectView.m_TreeWnd.SetItemImage( hItem,
//         CObjectView::LIGHT_SET_OFF, CObjectView::LIGHT_SET_OFF );
//      
//      // go through the light array and add
//      for( Light = 0; Light < pDoc->pLightArray->GetSize(); Light++ )
//      {
//         if( (*pDoc->pLightArray)[ Light ].mFlags & LIGHT_INLIGHTSET )
//            AddLight( &(*pDoc->pLightArray)[Light] );
//      }
//      
//      // now go through and remove the lights from the array
//      Light = 0;
//      int size = (*pDoc->pLightArray).GetSize();
//
//      while( Light < size)
//      {
//         // Loop through and delete all selected entities
//	      if( (*pDoc->pLightArray)[ Light ].mFlags & LIGHT_INLIGHTSET )
//         {
//           pDoc->DeleteLight( Light );
//
//           // Reset the entity count since the array is a CArray,
//           // and everything will get shifted down when one gets deleted...
//           Light = 0;
//           size = (*pDoc->pLightArray).GetSize();
//   	   } 
//         else {
//           Light++;
//         }
//	   }
//   }
//   
//   // set the internal flag
//   mActive = fActive;
//
//   // redraw
//	pDoc->UpdateAllViews(NULL);
//	pDoc->SetModifiedFlag();
}

// serialize functions
#define CSHAPESTATETYPE       "CShapeState"
#define CSHAPESTATEID         "DetailID"
#define CSHAPESTATENAME       "Name"
#define CSHAPESTATENUMDETAILS "NumDetails"
#define CENDSHAPESTATETYPE    "End CShapeState"
void CShapeState::Serialize( CThredParser& Parser )
{
   int IntVal;
   CShapeDetail * pDetail;
      
   // check if reading or writing this
   if( Parser.IsStoring )
   {
      // write out transaction type
		Parser.WriteString( CSHAPESTATETYPE );

      // write the info
      Parser.WriteString( CSHAPESTATENAME, Name );
//      Parser.WriteInt( CSHAPESTATEID, mDetailID );            
      
      // get the number of details to write out
      for( pDetail = mDetails, IntVal = 0; pDetail; 
         pDetail = pDetail->pNext, IntVal++ );
      Parser.WriteInt( CSHAPESTATENUMDETAILS, IntVal );
      
      CProgressBar Progress( IntVal, TRUE, 1 );
      
      // serialize the details
      pDetail = mDetails;
      while( pDetail )
      {
         pDetail->Serialize( Parser );
         pDetail = pDetail->pNext;
         
         Progress.StepIt();
      }
      
      // terminate this one
      Parser.WriteString( CENDSHAPESTATETYPE );
   }
   else
   {
      // make sure correct key
      if( !Parser.GetKey( CSHAPESTATETYPE ) )
         return;
         
      // read in the name
      Parser.GetKeyString( CSHAPESTATENAME, Name );
      
//      // read in the detail id
//      Parser.GetKeyInt( CSHAPESTATEID, IntVal );
//      mDetailID = IntVal;
      
      // read in the number of details
      Parser.GetKeyInt( CSHAPESTATENUMDETAILS, IntVal );
      
      CProgressBar Progress( IntVal, TRUE, 1 );
      
      // got through create details and read them in
      while( IntVal )
      {
         // create a detail
         pDetail = AddDetail( "" );
         pDetail->Serialize( Parser );
      
         Progress.StepIt();
            
         IntVal--;
      }
     
      NameDetails();
       
      Parser.SetEnd( CENDSHAPESTATETYPE );
		Parser.GotoEnd();
   }
}

#define CSHAPEDETAILTYPE         "CShapeDetail"
#define CSHAPEDETAILID           "LightSetID"
#define CSHAPEDETAILNUMLIGHTSETS "NumLightSets"
#define CSHAPEDETAILMAXTEXMIP    "MaxTexMip"
#define CSHAPEDETAILLINKABLE     "LinkbleFaces"
#define CSHAPEDETAILMINPIXELS    "MinPixels"
#define CSHAPEDETAILNAME         "Name"
#define CENDSHAPEDETAILTYPE      "End CShapeDetail"
void CShapeDetail::Serialize( CThredParser& Parser )
{
   int IntVal;
   CLightSet * pLightSet;
   CTHREDDoc * pDoc = pState->pDoc;
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
   
   // check for reading or writing
   if( Parser.IsStoring )
   {
      // write transaction type
      Parser.WriteString( CSHAPEDETAILTYPE );

      // write out the settins
      Parser.WriteULong( CSHAPEDETAILMAXTEXMIP, m_maximumTexMip );
      Parser.WriteULong( CSHAPEDETAILMINPIXELS, m_minPixels );
      IntVal = m_linkableFaces;
      Parser.WriteInt( CSHAPEDETAILLINKABLE, IntVal );
      
//      // write out the info
//      Parser.WriteString( CSHAPEDETAILNAME, Name );
//      Parser.WriteInt( CSHAPEDETAILID, mLightSetID );
      
      // count the lightsets
      for( pLightSet = mLightSets, IntVal = 0; pLightSet; IntVal++,
         pLightSet = pLightSet->pNext );
      Parser.WriteInt( CSHAPEDETAILNUMLIGHTSETS, IntVal );

      // turn off the current lightset
      if( pActiveLightSet )
         pActiveLightSet->SetActive( FALSE );
                  
      // go through the light states
      pLightSet = mLightSets;
      while( pLightSet )
      {
      
         pLightSet->SetActive( FALSE );
         pLightSet->Serialize( Parser );
         pLightSet = pLightSet->pNext;
      }
      
      // set the info for this detail
      if( pDoc->pActiveDetail != this )
      {
         // swap out the active info temp.
         pDoc->pActiveDetail->GetInfo();  
         SetInfo();
      }
      
      // serialize the info for this guy
      mRegularBrush.Serialize( Parser );
      pDoc->SerializeBrushList( Parser );
      //(*pWorldBsp).Serialize(Parser);
      
      // serialize our current brush
      pDoc->SerializeEntities( Parser );
      pDoc->SerializeLights( Parser );
      pDoc->SerializeGroups( Parser );

      // swap stuff back in if needed
      if( pDoc->pActiveDetail != this )
         pDoc->pActiveDetail->SetInfo();
            
      // terminate this transaction
      Parser.WriteString( CENDSHAPEDETAILTYPE );
   }
   else
   {
      // make sure correct key
      if( !Parser.GetKey( CSHAPEDETAILTYPE ) )
         return;
         
//      // read in the name
//      Parser.GetKeyString( CSHAPEDETAILNAME, Name );
//      
//      // read in the detail id
//      Parser.GetKeyInt( CSHAPEDETAILID, IntVal );
//      mLightSetID = IntVal;
      
      // read the settins
      Parser.GetKeyULong( CSHAPEDETAILMAXTEXMIP, m_maximumTexMip );
      Parser.GetKeyULong( CSHAPEDETAILMINPIXELS, m_minPixels );
      if( Parser.GetKeyInt( CSHAPEDETAILLINKABLE, IntVal ) )
         m_linkableFaces = IntVal;
      
      // read in the number of light sets
      Parser.GetKeyInt( CSHAPEDETAILNUMLIGHTSETS, IntVal );
      
      // got through create details and read them in
      while( IntVal )
      {
         // create a detail
         pLightSet = AddLightSet( "" );
         pLightSet->Serialize( Parser );
         
         IntVal--;
      }
      
      // setup the ptrs to grab the info
      SetInfo();
      
      // read in the other stuff here
      pDoc->mRegularBrush.Serialize(Parser);
      pDoc->mCurrentBrushId = 0;

      pFrame->UpdateGeneralStatus("Loading Brushes");
      pDoc->SerializeBrushList(Parser);

//      // get the worldbsp
//      pFrame->UpdateGeneralStatus("Loading World BSP");
//      pDoc->pWorldBsp->Serialize(Parser);

      // fixup the pointers in the list.
      //?????(*pWorldBsp).FixupPointers(mBrushList);

      pFrame->UpdateGeneralStatus("Loading Entities");
      pDoc->SerializeEntities(Parser);

      pFrame->UpdateGeneralStatus("Loading Lights");
      pDoc->SerializeLights( Parser );

      pFrame->UpdateGeneralStatus("Loading Brush Group Info");
      pDoc->SerializeGroups(Parser);

      // get the info from the serialize
      GetInfo();
      
      Parser.SetEnd( CENDSHAPEDETAILTYPE );
		Parser.GotoEnd();
   }
}

void CShapeDetail::SerializeOld( CThredParser& Parser  )
{
   CTHREDDoc * pDoc = pState->pDoc;
   CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
   
   // check for reading or writing
   if( Parser.IsStoring )
   {
      AfxMessageBox("Cannot write old style zed document", MB_OK, 0);
      return;
   }
   else
   {
      // adjust the ptr's
      if( pDoc->pActiveDetail )
         pDoc->pActiveDetail->GetInfo();
      SetInfo();
      
      // read in the other stuff here
      pDoc->mRegularBrush.Serialize(Parser);
      pDoc->mCurrentBrushId = 0;

      pFrame->UpdateGeneralStatus("Loading Brushes");
      pDoc->SerializeBrushList(Parser);

      pFrame->UpdateGeneralStatus("Loading Entities");
      pDoc->SerializeEntities(Parser);
      
      // go through and convert the entities to lights
      ConvertEntities();

      pFrame->UpdateGeneralStatus("Loading Lights");
      pDoc->SerializeLights( Parser );

      pFrame->UpdateGeneralStatus("Loading Brush Group Info");
      pDoc->SerializeGroups(Parser);

      // swap the info
      GetInfo();
      if( pDoc->pActiveDetail )
         pDoc->pActiveDetail->SetInfo();
   }
}

#define CLIGHTSETTYPE         "CLightSet"
#define CLIGHTSETNAME         "Name"
#define CENDLIGHTSETTYPE      "End CLightSet"
#define CLIGHTSETNUMLIGHTS    "NumLights"
void CLightSet::Serialize( CThredParser& Parser  )
{
//john   CLight * pLight;
//   int IntVal;
//   
//   // check for read or write
//   if( Parser.IsStoring )
//   {
//      // start the transaction
//      Parser.WriteString( CLIGHTSETTYPE );
//      
//      // write the name
//      Parser.WriteString( CLIGHTSETNAME, Name );
//      
//      // count the lights
//      for( pLight = pLights, IntVal = 0; pLight; pLight = pLight->pNext,
//         IntVal++ );
//      Parser.WriteInt( CLIGHTSETNUMLIGHTS, IntVal );
//   
//      // write out the lights
//      pLight = pLights;
//      while( pLight )
//      {
//         pLight->Serialize( Parser );
//         pLight = pLight->pNext;
//      }
//      
//      // end the transaction
//      Parser.WriteString( CENDLIGHTSETTYPE );
//   }
//   else
//   {
//      // make sure correct key
//      if( !Parser.GetKey( CLIGHTSETTYPE ) )
//         return;
//         
//      // read in the name
//      Parser.GetKeyString( CLIGHTSETNAME, Name );
//      
//      // read in the number of lights
//      Parser.GetKeyInt( CLIGHTSETNUMLIGHTS, IntVal );
//      
//      // got through create lights and read them in
//      while( IntVal )
//      {
//         CLight   Light( FALSE );
//      
//         // add a light
//         Light.Serialize( Parser );
//         AddLight( &Light );
//         
//         IntVal--;
//      }
// 
//      SetActive( FALSE );
//      
//      Parser.SetEnd( CENDLIGHTSETTYPE );
//      Parser.GotoEnd();
//   }
}

// assume Front:Back:Top:Bottom:Left:Right:0:0
#define LINK_FRONT      BIT_7
#define LINK_BACK       BIT_6
#define LINK_TOP        BIT_5
#define LINK_BOTTOM     BIT_4
#define LINK_LEFT       BIT_3
#define LINK_RIGHT      BIT_2

// detail settings used in export only
void CShapeDetail::DoSettings( void )
{
   CDetailSettings Dlg;
   
   // store the settings
   Dlg.m_maximumTexMip = m_maximumTexMip;
   Dlg.m_LinkBack = ( m_linkableFaces & LINK_BACK ) ? TRUE : FALSE;
   Dlg.m_LinkBottom = ( m_linkableFaces & LINK_BOTTOM ) ? TRUE : FALSE;
   Dlg.m_LinkFront = ( m_linkableFaces & LINK_FRONT ) ? TRUE : FALSE;
   Dlg.m_LinkLeft = ( m_linkableFaces & LINK_LEFT ) ? TRUE : FALSE;
   Dlg.m_LinkRight = ( m_linkableFaces & LINK_RIGHT ) ? TRUE : FALSE;
   Dlg.m_LinkTop = ( m_linkableFaces & LINK_TOP ) ? TRUE : FALSE;
   Dlg.m_minPixels = m_minPixels;
   
   if( Dlg.DoModal() == IDOK )
   {
      // get the settings
      m_maximumTexMip = Dlg.m_maximumTexMip;
      
      // do the faces 
      m_linkableFaces = 0;
      m_linkableFaces |= ( ( Dlg.m_LinkBack * 0xff ) & LINK_BACK );
      m_linkableFaces |= ( ( Dlg.m_LinkBottom * 0xff ) & LINK_BOTTOM );
      m_linkableFaces |= ( ( Dlg.m_LinkFront * 0xff ) & LINK_FRONT );
      m_linkableFaces |= ( ( Dlg.m_LinkLeft * 0xff ) & LINK_LEFT );
      m_linkableFaces |= ( ( Dlg.m_LinkRight * 0xff ) & LINK_RIGHT );
      m_linkableFaces |= ( ( Dlg.m_LinkTop * 0xff ) & LINK_TOP );

      m_minPixels = Dlg.m_minPixels;
      
      pState->pDoc->SetModifiedFlag();
      
      pState->NameDetails();
   }
}

// -----------------------------------------------------------
// Edit objects class
// -----------------------------------------------------------
CEditObject::CEditObject()
{
   pDetail = NULL;
   pState = NULL;
   pLightSet = NULL;   
}

CEditObject::~CEditObject()
{
   if( pState )
      delete pState;
   if( pDetail )
      delete pDetail;
   if( pLightSet )
      delete pLightSet;
}

void CEditObject::Clear()
{
   if( pState )
      delete pState;
   if( pDetail )
      delete pDetail;
   if( pLightSet )
      delete pLightSet;
      
   // reset hte ptrs
   pDetail = NULL;
   pState = NULL;
   pLightSet = NULL;
}

   
// ------------------------------------------------------------
// Options property sheet stuff
// ------------------------------------------------------------

void CTHREDDoc::Properties( void )
{
   ITRShapeOpt Shape;
   ITRBuildOpt Build;
   ITRLightOpt Light;
   
   CPropertySheet Sheet( "Properties" );
   
   // set the info for shape page
   Shape.m_linkedInterior = ShapeInfo.m_linkedInterior;
   Sheet.AddPage( ( CPropertyPage * )&Shape );
   
   // set info for build page
   Build.m_geometryScale = BuildOptions.m_geometryScale;
   Build.m_pointSnapPrecision = BuildOptions.m_pointSnapPrecision;
   Build.m_planeDistancePrecision = BuildOptions.m_planeDistancePrecision;
   Build.m_planeNormalPrecision = BuildOptions.m_planeNormalPrecision;

   BuildOptions.m_textureScale = theApp.m_texturescale;
   Build.m_textureScale = BuildOptions.m_textureScale;
   
   Build.m_lowDetail = BuildOptions.m_lowDetail;
   Sheet.AddPage( ( CPropertyPage * )&Build );
   
   // set info for the light page
   Light.m_geometryScale = LightOptions.m_geometryScale;
   Light.m_lightScale = LightOptions.m_lightScale;
   Light.m_useNormals = LightOptions.m_useNormals;
   Light.m_emissionQuantumNumber = LightOptions.m_emissionQuantumNumber;
   Light.m_useMaterialProperties = LightOptions.m_useMaterialProperties;
   Light.m_ambientLightIntensity = LightOptions.m_ambientIntensity;
   Light.m_ambientOutsidePolys = LightOptions.m_applyAmbientOutside;
   
   Sheet.AddPage( ( CPropertyPage * )&Light );
   
   // apply the data
   if( Sheet.DoModal() == IDOK )
   {
      // do shape info
      ShapeInfo.m_linkedInterior = Shape.m_linkedInterior;
      
      // do build info
      BuildOptions.m_geometryScale = Build.m_geometryScale;
      BuildOptions.m_pointSnapPrecision = Build.m_pointSnapPrecision;
      BuildOptions.m_planeDistancePrecision = Build.m_planeDistancePrecision;
      BuildOptions.m_planeNormalPrecision = Build.m_planeNormalPrecision;

      // set the info for theApp
      theApp.m_planeDistancePrecision = BuildOptions.m_planeDistancePrecision;
      theApp.m_planeNormalPrecision = BuildOptions.m_planeNormalPrecision;
      
      if( Build.m_textureScale != theApp.m_texturescale )
      {
         if( ( AfxMessageBox( "Are you sure you want to change the texture scale?\n"
            "This will overwrite any texture scale\nchanges you have made to this point...", 
            MB_YESNO, 0)) == IDYES)
         {
            theApp.m_texturescale = Build.m_textureScale;
            BuildOptions.m_textureScale = Build.m_textureScale;

            // Set the texture scale for all polygons
            CThredBrush* Brush;
            Brush = mBrushList;
            while(Brush)
            {
               for (int j = 0; j < Brush->mNumberOfPolygons; j++)
               {
                  Brush->mTextureID[j].TextureScale = theApp.m_texturescale;
               }

               Brush = Brush->GetNextBrush();
            }
         }
      }      
      
      BuildOptions.m_lowDetail = Build.m_lowDetail;
      
      // light and build geometry scale needs to match up
      LightOptions.m_geometryScale = Build.m_geometryScale;

      // do light info
      LightOptions.m_lightScale = Light.m_lightScale;
      LightOptions.m_useNormals = Light.m_useNormals;
      LightOptions.m_emissionQuantumNumber = Light.m_emissionQuantumNumber;
      LightOptions.m_useMaterialProperties = Light.m_useMaterialProperties;
      LightOptions.m_ambientIntensity = Light.m_ambientLightIntensity;
      LightOptions.m_applyAmbientOutside = Light.m_ambientOutsidePolys;

   	SetModifiedFlag();
   }
}



// -----------------------------------------------------------------------
// Export functions
// -----------------------------------------------------------------------

UInt32 GetFileLength( CString FileName )
{
   FILE * File;
   
   // open the file
   if( !( File = fopen( FileName, "r" ) ) )
         return( 0 );

   // seek to end
   fseek( File, 0, SEEK_END );

   // get file length
   int Length = ftell( File );
   
   fclose( File );
   
   return( Length );
}

// go through and make sure that everything is cool before exporting
//  - check for duplicate id's for lights in each detail
//  - check that each detail contains same number ( and named/id'd ) animated lights
//  - check for the correct number of lightsets per detail
//  - check for lights with id's that are not set but has states
//  - check for improper time states
BOOL CTHREDDoc::CheckExport( void )
{
   // check certain details in the shape file
   CShapeState * pState;
   CShapeDetail * pDetail;
   String   tmpString;
   
   //ShapeInfo.m_lightStateNames.size();
   ShapeInfo.m_numLightStates = 0; 

   // go through and assign all the ids for the lights
   for( pState = m_StateList; pState; pState = pState->pNext )
   {
      // array of light strings
      CStringArray lightNames;
     
      for( pDetail = pState->mDetails; pDetail; pDetail = pDetail->pNext )
      {
         int multiStateLights = 0;
         CStringArray names;
         
         // go through all the lights in the detail
         for( int Light = 0; Light < pDetail->mLightArray.GetSize(); Light++ )
         {
            CLight & light = pDetail->mLightArray[ Light ];
            
            // check if this light needs an id
            if( light.states.GetSize() > 1 )
            {
               multiStateLights++;

               float totalDuration = 0.0f;
               
               // walk the states looking for 0.0 timed states or no time
               for( int j = 0; j < light.states.GetSize(); j++ )
               {
                  CLightState & state = light.states[j];
                  totalDuration += state.stateDuration;
               }
               if( totalDuration == 0.0f )
               {
                  char message[200];
                  sprintf( message, "Total duration for animated light cannot be 0 ( %s )",
                     light.name );
                  AfxMessageBox( message );
                  return( false );
               }
                                 
               // check for first time through
               if( pDetail == pState->mDetails )
               {
                  // add to both lists
                  lightNames.Add( light.name );
                  names.Add( light.name );
                  light.id = 0xffff + lightNames.GetSize();
               }
               else
               {
                  // walk through and assign an id or fail
                  for( int i = 0; i < lightNames.GetSize(); i++ )
                  {
                     if( light.name == lightNames[i] )
                     {
                        // add to the current name list ( just for dup check'n )
                        names.Add( light.name );
                        light.id = 0xffff + i;
                        
                        break;  
                     }
                  }
                  if( i == lightNames.GetSize() )
                  {
                     char message[200];
                     sprintf( message, "Not all details contain animated light [ %s ]",   
                        light.name );
                     AfxMessageBox( message );
                     return( false );
                  }
               }
            }
            else
               light.id = -1;
         }
         
         
         // check for duplicate names
         for( int i = 0; i < names.GetSize(); i++ )
         {
            for( int j = i+1; j < names.GetSize(); j++ )
            {
               if( names[i] == names[j] )
               {
                  char message[200];
                  sprintf( message, "At least two animated lights share names in a single detail [ %s ]",
                     names[i] );
                  AfxMessageBox( message );
                  return( false );
               }
            }
         }        

         // check that we have the same number of multistate lights as we expect
         if( multiStateLights != lightNames.GetSize() )
         {
            char message[200];
            sprintf( message, "Not all details contain the same number of animated lights." );
            AfxMessageBox( message );
            return( false );
         } 
      }
   }
   
   // check for too many textures...
   if( C3DMWorld::matMap.size() >= 256 )
   {
      char message[200];
      sprintf( message, "Too many materials in this shape\n\t( max of 255 - found %d )", C3DMWorld::matMap.size() );
      AfxMessageBox( message );
      return( false );
   }
        
   
//   // go through the states
//   for( pState = m_StateList; pState; pState = pState->pNext )
//   {
//      for( pDetail = pState->mDetails; pDetail; pDetail = pDetail->pNext )
//      {
//         int   LightSetCount = 0;
//         int   LightCount = 0;
//         Vector< int > LightIDList;
//         
//         // go through the lights in this one
//         for( int Light = 0; Light < pDetail->mLightArray.GetSize(); Light++ )
//         {
//            // only those not in the light sets
//            if( !( pDetail->mLightArray[ Light ].mFlags & LIGHT_INLIGHTSET ) )
//            {
//               // check for duplicate id
//               for( int i = 0; i < LightCount; i ++ )
//               {
//                  if( LightIDList[ i ] == pDetail->mLightArray[ Light ].mID )
//                  {
//                     char message[200];
//                     sprintf( message, "At least two lights share id's in a single detail [ %s ]",
//                        pDetail->mLightArray[Light].mName );
//                     AfxMessageBox( message );
//                     return( FALSE );
//                  }
//               }
//               
//               // check for states and id
//               if( ( pDetail->mLightArray[ Light ].mStates.GetCount() > 1 ) &&
//                  pDetail->mLightArray[ Light ].mID == -1 )
//               {
//                  AfxMessageBox( "A light contains multiple states but has no id associated" );
//                  return( FALSE );
//               }
//               
//               // add to list of light ids
//               if( pDetail->mLightArray[ Light ].mID != -1 )
//               {
//                  LightIDList.push_back( pDetail->mLightArray[ Light ].mID );
//                  LightCount++;
//               }
//            }
//         }
         
//         // just cound the lightsets for now, the ids are not needed now
//         for( pLightSet = pDetail->mLightSets; pLightSet; pLightSet = pLightSet->pNext )
//         {
//            CLight * pLight;
//
//            // go through the lights
//            for( pLight = pLightSet->pLights; pLight; pLight = pLight->pNext )
//            {
//               // check for duplicate id
//               for( int i = 0; i < LightCount; i ++ )
//               {
//                  if( LightIDList[ i ] == pLight->mID )
//                  {
//                     AfxMessageBox( "At least two lights share id's in a single detail" );
//                     return( FALSE );
//                  }
//               }
//               
//               // add to list of light ids
//               if( pLight->mID != -1 )
//               {
//                  LightIDList.push_back( pLight->mID );
//                  LightCount++;
//               }
//               
//               // check for state and id
//               if( ( pLight->mStates.GetCount() > 1 ) &&  pLight->mID == -1 )
//               {
//                  AfxMessageBox( "A light contains multiple states but has no id associated" );
//                  return( FALSE );
//               }
//            }
//            LightSetCount++;
//         }
//
//         // check the number of light sets
//         if( LightSetCount != ShapeInfo.m_numLightStates )         
//         {
//            AfxMessageBox( "One or more details do not contain the\n"
//               "proper number of light states" );
//            return( FALSE );
//         }
//      }
//   }

   return( TRUE );
}

// go through and generate a unique material list for this file
BOOL CTHREDDoc::BuildMaterialList( TS::MaterialList & matList )
{
   matList.setSize( C3DMWorld::matMap.size(), 1 );
 
   // go through the materials collected through the export
   for( int i = 0; i < C3DMWorld::matMap.size(); i++ )
      matList[i] = (*m_materialList)[ C3DMWorld::matMap[i] ];

   return( true );
}

BOOL CTHREDDoc::ExportVolume( VolumeRWStream & Volume, 
   CString ExportFileName, CString Base, BOOL OptimalBuild )
{
   int StateCount, DetailCount, LightSetCount;
   CShapeState * pState;
   CShapeDetail * pDetail;
   CLightSet * pLightSet;
   CString FileName;
   FILE * File;
   FileRWStream Stream;
   int Length;
   char * pBuf;
   VolumeRWStream * pVolume = &Volume;
   char szBuffer[ 128 ];
   int Pos;

   // clear out the material index map so we can get a proper material list
   C3DMWorld::matMap.clear();
   
   // set the cursor to the waiting version
   SetCursor( LoadCursor( NULL, IDC_WAIT ) );
      
   // fill in any info that may not be in the ITRShapeEdit member - rest
   // is set as default or setup through user actions
   sprintf( szBuffer, "%s", matName );
   for( Pos = strlen( szBuffer ); Pos; Pos-- )
   {
      if( szBuffer[ Pos - 1 ] == '\\' )
      {
         ShapeInfo.m_materialList = &szBuffer[ Pos ];
         break;
      }
   }
   if( !Pos )
      ShapeInfo.m_materialList = matName;
   
   // clear the string vector
   ShapeInfo.m_lightStateNames.clear();

   // go through and generate an array of strings for the lightset names
   // because they need to be output in order of their names
   pState = m_StateList;
   while( pState )
   {
      if( pState->mDetails )
      {
         pLightSet = pState->mDetails->mLightSets;
         while( pLightSet )
         {
            String   tmpString = pLightSet->Name;

            ShapeInfo.m_lightStateNames.increment();
            
   			// )(*@$&%&! MSVC
	   		memset(&(ShapeInfo.m_lightStateNames.last()), 0, sizeof(String));
            ShapeInfo.m_lightStateNames.last() = tmpString;

            pLightSet = pLightSet->pNext;
         }
         break;
      }
      pState = pState->pNext;
   }
   int NumLightSets = ShapeInfo.m_lightStateNames.size();

   // set light state number
   ShapeInfo.m_numLightStates = NumLightSets;
   
   // check for no lightsets and add into 'default'
   bool fAddingLightsets = FALSE;
   if( !NumLightSets )
   {
//      // tell the user that there are no light sets
//      if( !theApp.mExportRun )
//         AfxMessageBox( "No lightsets were found in shape ( determined by first state/detail ).\n"
//            "Outputing light informating in lightset 'default'." );

      // set flag      
      fAddingLightsets = TRUE;  

      // set that we have one lightset to get through to the proper logic
      NumLightSets = 1;

      // add the name to the lightset list
      ShapeInfo.m_lightStateNames.increment();

      // )(*@$&%&! MSVC
      String tmpString = "default";
      memset(&(ShapeInfo.m_lightStateNames.last()), 0, sizeof(String));
      ShapeInfo.m_lightStateNames.last() = tmpString;

      // set light state
      ShapeInfo.m_numLightStates = NumLightSets;
   }
   
   // get the number of states
   for( StateCount = 0, pState = m_StateList; pState; pState = pState->pNext, StateCount++ );
   
   CProgressBar Progress( StateCount, TRUE, 1 );
   
   // walk the entire document - go through all the states
   pState = m_StateList;
   StateCount = 0;
   while( pState )
   {
      ITRShapeEdit::EditState * pEditState;
      pEditState = new ITRShapeEdit::EditState;
      
      // go through the details
      pDetail = pState->mDetails;
      DetailCount = 0;
      while( pDetail )
      {
         ITRShapeEdit::EditLOD * pEditLOD;
         pEditLOD = new ITRShapeEdit::EditLOD;
            
         // generate the filename for this detail
         FileName.Format( "%s-%d%d.3dm", Base, StateCount, DetailCount );
                  
         CMainFrame * pFrame = ( CMainFrame * )AfxGetMainWnd();
                  
         // set this as the active detail
         pActiveDetail->GetInfo();  
         pDetail->SetInfo();
         
         // write out he 3dm file
         pFrame->UpdateGeneralStatus("Building optimal polygon set");
         if( OptimalBuild )
            (*pWorldBsp).RebuildOptimal(NULL, 0, TRUE);
         else 
            (*pWorldBsp).RebuildQuick(NULL, 0, TRUE);

         pFrame->UpdateGeneralStatus("Exporting polygons");
         (*pWorldBsp).Export( FileName, this );

         // create a volume and copy this file
         Length = GetFileLength( FileName );     
         
         pBuf = new char[ Length ];
         Stream.open( FileName );
         Stream.read( Length, pBuf );
         Stream.close();
         
         DeleteFile( FileName );
         Volume.open( FileName, STRM_COMPRESS_NONE, Length );
         Volume.write( Length, pBuf );
         Volume.close();
         delete [] pBuf;
         
         pFrame->UpdateGeneralStatus("Verifying exported polygons");
         // Restore the bsp tree
         if( OptimalBuild )
            (*pWorldBsp).RebuildOptimal(NULL, 0, FALSE);
         else 
            (*pWorldBsp).RebuildQuick(NULL, 0, TRUE);
      
         // set the data for this lod
         pEditLOD->m_geometryFileName = FileName;
         pEditLOD->m_minPixels = pDetail->m_minPixels;
         pEditLOD->m_linkableFaces = pDetail->m_linkableFaces;

         // generate an options file using the existing object
         BuildOptions.m_maximumTexMip = pDetail->m_maximumTexMip;
         
         // adjust the filename
         sprintf( szBuffer, "%s", FileName );
         for( Pos = 0; Pos < strlen( szBuffer ); Pos++ )
         {
            if( szBuffer[ Pos ] == '.' )
            {
               szBuffer[ Pos ] = 0;
               break;
            }
         }
         strcat( szBuffer, ".igo" );
         FileName.Format( "%s", szBuffer );

         // get the texture scale from the app settings
         BuildOptions.m_textureScale = theApp.m_texturescale;
         
         // adjust the texture scale for the geometry 
         // ( used to be done at the command line? )
         BuildOptions.m_textureScale /= BuildOptions.m_geometryScale;
         
         // use the global light options object and write out its info -
         // if userStream was just not a protected member.. shrug
         Stream.open( FileName );
         BuildOptions.store( Stream );
         Stream.close();

         Length = GetFileLength( FileName );
         
         pBuf = new char[ Length ];
         Stream.open( FileName );
         Stream.read( Length, pBuf );
         Stream.close();
         DeleteFile( FileName );
         Volume.open( FileName, STRM_COMPRESS_NONE, Length );
         Volume.write( Length, pBuf );
         Volume.close();
         delete [] pBuf;
   
         CString tmpCString;
         LightSetCount = 0;
         // go through the lightsets by referencing their names
         for( int LightSetPos = 0; LightSetPos < NumLightSets; LightSetPos++ )
         {
            tmpCString = ShapeInfo.m_lightStateNames[ LightSetPos ].c_str();

            if( !fAddingLightsets )
            {      
               pLightSet = pDetail->GetLightSetByName( tmpCString );
               if( !pLightSet )
                  return( FALSE );
            }
               
            ITRShapeEdit::EditLightState * pEditLightState;
            pEditLightState = new ITRShapeEdit::EditLightState;

            // generate a file name for this lightset
            FileName.Format( "%s-%d%d%d.ilt", Base, StateCount, 
               DetailCount, LightSetCount );
            exportLights( FileName );
//            
//            // open the file - because of fprintf and that we need the
//            // size of the file before storing, we need to create a file 
//            // and copy it into the volume
//            if( !( File = fopen( FileName, "w" ) ) )
//            {
//               SetCursor( LoadCursor( NULL, IDC_ARROW ) );
//               
//               // go through the vectors and delete everything!! muahahaha
//               for( int i = 0; i < ShapeInfo.m_pStates.size(); i++ )
//                  delete ShapeInfo.m_pStates[ i ];
//                  
//               return( FALSE );
//            }
//            
//            // write out the ilt file info for the lightset if it exists
//            if( !fAddingLightsets )
//            {
//               CLight * pLight;
//               pLight = pLightSet->pLights;
//               while( pLight )
//               {
//                  if( pLight->mFlags & LIGHT_INLIGHTSET )
//                  {
//                     // export this light
//                     pLight->Export( File );
//                     fprintf( File, "\n" );
//                  }
//                  pLight = pLight->pNext;
//               }
//            }
//
//            // write out the base lights
//            for( int Light = 0; Light < pLightArray->GetSize(); Light++ )
//            {
//               if( !( (*pLightArray)[ Light ].mFlags & LIGHT_INLIGHTSET ) )
//               {
//                  (*pLightArray)[ Light ].Export( File );
//                  fprintf( File, "\n" );
//               }
//            }
//         
//            fseek( File, 0, SEEK_SET );
//            fseek( File, 0, SEEK_END );
//            Length = ftell( File );
//            fclose( File );
            
            Length = GetFileLength( FileName );
            pBuf = new char[ Length ];
            Stream.open( FileName );
            Stream.read( Length, pBuf );
            Stream.close();
            
            // create a volume file
            Volume.open( FileName, STRM_COMPRESS_NONE, Length );
            
            // write the entire length
            Volume.write( Length, pBuf );
            
            // close the volume file
            Volume.close();
            delete [] pBuf;
            
            DeleteFile( FileName );
            File = NULL;
            
            // set the data for this lightset
            pEditLightState->m_lightFileName = FileName;
            
            // adjust the filename ( same base but differing end )
            sprintf( szBuffer, "%s", FileName );
            for( Pos = 0; Pos < strlen( szBuffer ); Pos++ )
            {
               if( szBuffer[ Pos ] == '.' )
               {
                  szBuffer[ Pos ] = 0;
                  break;
               }
            }
            strcat( szBuffer, ".ilo" );
            FileName.Format( "%s", szBuffer );
                        
            // use the global light options object and write out its info
            Stream.open( FileName );
            LightOptions.store( Stream );
            Stream.close();
            
            // get the length and copy into a buffer
            Length = GetFileLength( FileName );
            pBuf = new char[ Length ];
            
            // reset the file ptr and read in the file
            Stream.open( FileName );
            Stream.read( Length, pBuf );
            Stream.close();
            DeleteFile( FileName );
            
            // create a volume file and copy this info
            Volume.open( FileName, STRM_COMPRESS_NONE, Length );
            Volume.write( Length, pBuf );
            Volume.close();
            delete [] pBuf;
            
            // add the EditLightState
            pEditLOD->m_pLightStates.push_back( pEditLightState );
            
            LightSetCount++;            
         }
      
         // restore
         pActiveDetail->SetInfo();  
            
         // add the EditLOD to the list
         pEditState->m_pLODVector.push_back( pEditLOD );
            
         pDetail = pDetail->pNext;
         DetailCount++;
      }

      // set the name
      pEditState->m_stateName = pState->Name;
      
      // add the EditState to the list
      ShapeInfo.m_pStates.push_back( pEditState );
      
      pState = pState->pNext;
      StateCount++;     
      
      Progress.StepIt(); 
   }

   // write out the material properties - geez this sucks!
   FileName.Format( "%s.imp", Base );
   Stream.open( FileName );
   ITRBasicLighting::writeMaterialPropList( Stream, 
      CMaterialProperties::m_propertyInfo );
   Stream.close();
   Length = GetFileLength( FileName );
   pBuf = new char[ Length ];
   Stream.open( FileName );
   Stream.read( Length, pBuf );
   Stream.close();
   DeleteFile( FileName );
   Volume.open( FileName, STRM_COMPRESS_NONE, Length );
   Volume.write( Length, pBuf );
   Volume.close();
   delete [] pBuf;

   // write out the material list - this method is really lame
   TS::MaterialList matList;
   if( !BuildMaterialList( matList ) )
      return( false );
   FileName.Format( "zedtemp.dml" );
   Stream.open( FileName );
   matList.store( Stream );
   Stream.close();
   Length = GetFileLength( FileName );
   pBuf = new char[ Length ];
   Stream.open( FileName );
   Stream.read( Length, pBuf );
   Stream.close();
   DeleteFile( FileName );
   
   // change the name of the dml to the correct one.. 
   FileName.Format( "%s.dml", Base );
   Volume.open( FileName, STRM_COMPRESS_NONE, Length );
   Volume.write( Length, pBuf );
   Volume.close();
   delete [] pBuf;
   
   // change the shapeinfo dml filename
   ShapeInfo.m_materialList = FileName;
      
   // adjust the filename
   FileName.Format( "%s", "input.its" );
   
   // write this header file - what a pain
   Stream.open( FileName );
   
   ShapeInfo.store( Stream );
   Stream.close();
   Length = GetFileLength( FileName );
   pBuf = new char[ Length ];
   Stream.open( FileName );
   Stream.read( Length, pBuf );
   Stream.close();
   DeleteFile( FileName );
   Volume.open( FileName, STRM_COMPRESS_NONE, Length );
   Volume.write( Length, pBuf );
   Volume.close();
   delete [] pBuf;

   // go through the vectors and delete everything!! muahahaha
   for( int i = 0; i < ShapeInfo.m_pStates.size(); i++ )
      delete ShapeInfo.m_pStates[ i ];
      
   // clear it all out
   ShapeInfo.m_pStates.clear();
   SetCursor( LoadCursor( NULL, IDC_ARROW ) );

   return( TRUE );
}

// ----------------------------------------------------------
// export the lights...
// ----------------------------------------------------------
bool CTHREDDoc::exportLights( CString & fileName )
{  
   ZedLight::PersLightList list;

   // go through all the lights and all
   for( int i = 0; i < pLightArray->GetSize(); i++ )
   {
      CLight & light = (*pLightArray)[i];
      ZedLight::PersLight * persLight = new ZedLight::PersLight;
      
      // set the light info
      persLight->id = light.id;
      persLight->name = light.name;
      persLight->animationDuration = light.animationDuration;
      persLight->flags = light.flags;
      
      // go through the states...
      for( int j = 0; j < light.states.GetSize(); j++ )
      {
         CLightState & state = light.states[j];
         ZedLight::PersLight::PersState * persState = new ZedLight::PersLight::PersState;
         
         // copy the info
         persState->stateDuration = state.stateDuration;
         persState->color = state.color;
         
         // go through the emitters...
         for( int k = 0; k < state.emitters.GetSize(); k++ )
         {
            CLightEmitter & emitter = state.emitters[k];
            ZedLight::PersLight::PersState::Emitter persEmitter;
            
            // copy the info...
            persEmitter.lightType = ( emitter.lightType == CLightEmitter::LightType::PointLight ) ?
               ( ZedLight::PersLight::PersState::LightType::PointLight ) :
               ( ZedLight::PersLight::PersState::LightType::SpotLight );
            persEmitter.pos = emitter.pos;
            persEmitter.spotVector = emitter.spotVector;
            persEmitter.spotTheta = emitter.spotTheta;
            persEmitter.spotPhi = emitter.spotPhi;
            persEmitter.falloff = ( emitter.falloff == CLightEmitter::Falloff::Distance ) ?
               ( ZedLight::PersLight::PersState::Falloff::Distance ) :
               ( ZedLight::PersLight::PersState::Falloff::Linear );
            persEmitter.d1 = emitter.d1;
            persEmitter.d2 = emitter.d2;
            persEmitter.d3 = emitter.d3;
            
            // add to the state
            persState->m_emitterList.push_back( persEmitter );
         }
         
         // add to the list
         persLight->state.push_back( persState );
      }
      
      // add to the list
      list.push_back( persLight );
   }
 
   // save it
   list.fileStore( fileName );  
   
   // cleans itself out
//   // now go through and cleanup
//   for( int l = 0; l < list.size(); l++ )
//   {
//      ZedLight::PersLight * persLight = list[l];
//      
//      for( int m = 0; m < persLight->state.size(); m++ )
//      {
//         ZedLight::PersLight::PersState * persState = persLight->state[m];
//         delete persState;
//      }
//      delete persLight;
//   }
   
   return( true );
}