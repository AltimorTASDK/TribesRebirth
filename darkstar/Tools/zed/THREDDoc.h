// THREDDoc.h : interface of the CTHREDDoc class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _THREDDOC_H_
#define _THREDDOC_H_

#include "thredprimitives.h"
#include "thredbrush.h"
#include "ConstructiveBsp.h"
#include "createboxdialog.h"
#include "CreateSpheroidDialog.h"
#include "CreateTriDialog.h"
#include "CreateCylDialog.h"
#include "CreateStaircaseDialog.h"
#include "VDescDlg.h"
#include "Entity.h"
#include "brushgroup.h"
#include "Grid.h"
#include "light.h"
#include "itrShapeEdit.h"

// DPW - Added for TS compatibility
//#include "gfxwin.h"
//#include "ts_init.h"
#include "g_pal.h"

// DPW - for new 3space (1/2/97)
#include "ResManager.h"
#include "ts_Material.h"
#include "ts_light.h"
#include "ts_RenderContext.h"


#define BBOX_PICK_NONE          0
#define BBOX_PICK_CENTER        1
#define BBOX_PICK_TOP           2
#define BBOX_PICK_TOP_RIGHT     3
#define BBOX_PICK_TOP_LEFT      4
#define BBOX_PICK_RIGHT         5
#define BBOX_PICK_LEFT          6
#define BBOX_PICK_BOTTOM        7
#define BBOX_PICK_BOTTOM_RIGHT  8
#define BBOX_PICK_BOTTOM_LEFT   9
#define BBOX_PICK_BRUSH_CENTER  10

// Editing States
#define ES_RESIZE_MOVE  0
#define ES_ROTATE       1
#define ES_SHEAR        2

#define NUM_UNDOS         25
#define NUM_UNDO_BRUSHES  10

typedef     CArray<CTransformMatrix, CTransformMatrix>  brushmat_array;
typedef     CArray<int, int>                            brushid_array;

class UndoInfo
{
public:
  UndoInfo();
  UndoInfo(UndoInfo &inval);
  ~UndoInfo();

  UndoInfo& operator=(UndoInfo& inval);

  brushmat_array    matrix;
  brushid_array     brushes;
};

// if anything below is changed, must change CShapeState::SetActive
// and CShapeState::GetActive accord.
#define SHAPE_INFO	                                    \
	/* the entire world.  This IS the document really */	\
   int     mCurrentTool;                                 \
   int     mShowBrush;                                   \
   int     mOldTool;										         \
   int     mUseGrid;										         \
   int		mGridType;										      \
   double  mGridSize;										      \
   CThredBrush   mRegularBrush;							      \
   CThredBrush*  mCurrentBrush;							      \
   CThredBrush*  mBrushList;								      \
   ThredBox      EditBox;									      \
   int           edit_state;								      \
   BOOL          mVolumeShow;								      \
   BOOL          mFilledVolumeShow;						      \
   /* our array of brush groups */						      \
   int mCurrentGroup;										      \
   int mShow;												         \
   int mLastOp;	/* DPW - 10/23 */    				      \
   int mUseRotationSnap;									      \
   /* our array of entities and entity info */		      \
   int                 mCurrentEntity;					      \
   BOOL                mSelectedEntity;				      \
   int					mShowEntities;						      \
   /* the array of lights and info for lights */	      \
   int					mCurrentLight;						      \
   int					mShowLights;						      \
   int					mShowEntities3D;						   \
   int					mShowLights3D;						      \
   BOOL				mSelectedLight;						      \
   int                 mAdjustmentMode;				      \
   int                 mBrushInBuffer;					      \
   int                 mGroupMovement;					      \
   TSSceneLighting*	m_GSceneLights;					      \
   /* our current setting for our grid movement */	      \
   int mCurrentBrushId;									         \
   GFXLightMap*        m_LMHandle;


// use this for cut'n and paste'n
class CEditObject
{
	public:
		CEditObject();
		~CEditObject();

		void Clear();

		class CShapeDetail * 	pDetail;
		class CShapeState * 	pState;
		class CLightSet * 		pLightSet;
};

// mfc wont allow nested templates
// collection of lights for each detail ( sun phases and such )
class CLightSet
{
public:
	CLightSet();
	~CLightSet();
	void SetActive( BOOL fActive );
	BOOL AddLight( CLight * pLight );
	void Serialize( CThredParser& Parser );

	CString			Name;
	CLightSet * 	pNext;
	CLightSet * 	pPrevious;
	HTREEITEM			hItem;

	BOOL 			mActive;

	// the lights associated with this set
   CLight * 		pLights;

	// detail level this light set belongs to
	class CShapeDetail * pDetail;
};

// a detail that is associated to some state for a shape
class CShapeDetail
{
public:
	void SerializeOld( CThredParser& Parser );
	CShapeDetail( class CShapeState * pShapeState );
	~CShapeDetail();
	void Serialize( CThredParser& Parser );

	CLightSet * AddLightSet( CString Name );
	CLightSet * AddLightSet( CLightSet * pLightSet );
	BOOL CopyLightSet( CLightSet * pLightSet );
	BOOL PasteLightSet( void );
	BOOL RemoveLightSet( CLightSet * pLightSet );
	CLightSet * GetLightSetByName( CString Name );
	void NameLightSets( void );
	BOOL SetActive( void );
	BOOL GetInfo( void );
	BOOL SetInfo( void );
	BOOL UpdateTitles( void );
	void DoSettings( void );
   void ConvertEntities( void );

	CString					Name;
	CLightSet * 			mLightSets;
	CLightSet * 			pActiveLightSet;
	BOOL					mBuiltEditBox;

	// info for exporting
	UInt32 m_maximumTexMip;
   	UInt8 m_linkableFaces;
    UInt32 m_minPixels;

	// all the info needed ( except light sets ) for this detail
	CBrushGroupArray mBrushGroupArray;
	CEntityArray        mEntityArray;
	CLightArray			mLightArray;
	CConstructiveBsp  	mWorldBsp;

	SHAPE_INFO

	// pointer to the shapestate object that this belongs to
	class CShapeState * pState;

	CShapeDetail * 			pNext;
	CShapeDetail * 			pPrevious;
	HTREEITEM				hItem;
};


// state for the shape ( under construction or damage stuff )
// use the CList template for this class
class CShapeState
{
public:
	CShapeState();
	~CShapeState();

	void Serialize( CThredParser& Parser );

	CShapeDetail * AddDetail( CString Name );
	CShapeDetail * AddDetail( CShapeDetail * pDetail );
	BOOL CopyDetail( CShapeDetail * pDetail );
	class CShapeDetail * PasteDetail( void );

	BOOL RemoveDetail( CShapeDetail * pDetail );
	void NameDetails( void );
	BOOL ImportDetail( void );
   int NumDetails( void );

	// the document this state belongs to
	CTHREDDoc *				pDoc;

	CString					Name;
	CShapeDetail * 			mDetails;
	HTREEITEM				hItem;
	class CShapeState * 	pNext;
	class CShapeState * 	pPrevious;
};

class CTHREDDoc : public CDocument
{
protected: // create from serialization only
	CTHREDDoc();
	//DECLARE_DYNCREATE(CTHREDDoc)
	DECLARE_SERIAL(CTHREDDoc);

	// DPW - Added for TS compatibility
	void releaseTS(void);		// three space cleanup

   int   color_plus, color_minus, color_origin, color_select;
   int   color_def_brush, color_def_point, color_def_entity;
   int   line_grid_color, dot_grid_color, color_bbox;
   int   volume_color;
			   
   PALETTEENTRY   color_request;

   BOOL      mCanUndo;
   BOOL      mCanRedo;
   int       mCurrentUndoLevel;
   int       mCurrentRedoLevel;
   int       curr_colorchange_selection;

   CThredBrush*      mcopylist;
   CEntityArray      mEntityCopyArray;
   CLightArray			mLightCopyArray;


// Attributes
public:
    // our main world in bsp form
	CConstructiveBsp  * pWorldBsp;
	int               mViewAxis;
    int               background_color;
    CString           m_VolumeDescription[TOTAL_STATES];

    // Undo & redo arrays
    CArray<UndoInfo, UndoInfo>  UndoArray;
    CArray<UndoInfo, UndoInfo>  RedoArray;

    BOOL BuildMaterialList( TS::MaterialList & matList );
    void  SetSurfaceInfo(GFXSurface* pBuffer);
    void  SelectAllBrushes();
    void  ReadAGroup(CThredParser& Parser, CString filename);
    void  WriteCurrentGroup(CThredParser& Parser);
    void  PasteBrushes(CThredBrush* copylist);
    void  CopySelectedBrushes(CThredBrush* brushlist);
	 void  CopySelectedEntities( void );
    void  PasteEntities( void );
    void  SelectAllEntities( void );
    void  StoreUndoInfo();
    void  ShearCurrentBrushes(Point3F* minbound, Point3F* delta, ShearType wrt);
    void  SetCurrentBrushesCenters();
    void  MoveEditBoxCenter(double X, double Y, double Z);
    void  DoneMoveEditBoxCenter();
    void  DoneRotateCurrentBrushes();
    void  RotateCurrentBrushes(double Yaw, double Pitch, double Roll);
    void  GotoNextEditState();
    void  GotoPrevEditState();
    void  DoneMoveCurrentBrushes(int mViewType);
    void  MoveCurrentBrushes(double X, double Y, double Z);
    void  ResizeCurrentBrushes(Point3F* minbound, Point3F* delta);
    void  BuildEditBox(BOOL redo_center);
    void  DoneMoveBrushCenter(double GridSize);
    void  MoveBrushCenter(double X, double Y, double Z);
    void  SetViewPalettes();
    void  DoTheExport(bool do_optimal, bool do_dialog );
    void  UpdateBrushInfoSIZE(ThredPoint size);
    void  UpdateBrushInfoPOS(ThredPoint position);
    void  UpdateBrushInfoROTATION(double rotation);
    void  GotoNextBrush();
    void  GotoPrevBrush();
    bool  ShouldGotoFourViews();
    int   InCurrentBrush(ThredBox* box, /*CThredBrush* brush,*/ CPoint mStartPoint, UINT mViewType, CRenderCamera* Camera);
    void  ForceBrushSelection(CThredBrush* brush);
	CThredBrush* GetBrushList();  // DPW - 11/14
    void  DoneShearBrush();	// DPW - 10/23
	void  DoneScaleBrush();	// DPW - 10/23
    void  SelectAPolygon(CPoint pos, CRenderCamera* Camera, TSRenderContext* rc);  // DPW - 11/13
	void  MoveSelectedBrushes(double X, double Y, double Z);
	void  ChangeGridSize(int Increment);
	void  UpdateGridInformation();
	void  DeleteCurrentThing();
	void  DoneMoveEntity();
	void  DeleteEntity(int EntityIndex);
	void  DoneRotateBrush();
	void  OnSelectedTypeCmdUI(CCmdUI* pCmdUI);
	void  SelectABrush(CPoint point, CRenderCamera& Camera);
    void  SelectABrushIndependent(CPoint point, CRenderCamera& Camera);
	void  SerializeGroups(CThredParser& Parser);
	void  SerializeEntities(CThredParser& Parser);
	void  SerializeBrushList(CThredParser& Parser);
	void  UpdateActiveBrushes();
	void  RenderOrthoView(int mViewType, CRenderCamera* Camera, TSPerspectiveCamera* ts_cam, TSRenderContext* rc, BOOL showselectrect, CRect* srect);
	int   BrushesSelected();
	void  SelectBrushes(CRect& Rect, CRenderCamera& Camera, int Status=TRUE);
	void  MoveEntity(double X, double Y, double Z);
	void  RenderEntities(CRenderCamera* Camera, int ViewType, double mZoomFactor, TSRenderContext* rc);
	void  UpdateEntityOrigins();
	int   CanRedo();
	int   CanUndo();
	void  RenderSolidWorld(CRenderCamera* Camera, TSRenderContext* rc);
	void  DeleteBrushList();
	void  RenderWorld(CRenderCamera* Camera, TSRenderContext* rc);
	void  DoneMoveBrush();
	void  MoveBrush(double X, double Y, double Z);
	void  RenderBrush(CRenderCamera& Camera, CDC* pDC, int ViewType, double ZoomFactor, TSRenderContext* rc, bool showbb);
	void  RotateBrush(double Yaw, double Pitch, double Roll);
	int   AddBrushToList(CThredBrush& Brush);
	int   GetBrushListCount();
	CEntity * SelectEntity(CPoint point, CRenderCamera& Camera, BOOL fClear );
	void  ScaleBrush(double X, double Y, double Z);
	void  ShearBrush(double X, double Y, double Z);
	void  ResetBrush(ThredPoint newpos);
	void  UpdateBrushInformation();
	int SetCurrentEntity( CEntity * pEntity );
	void ResetSelectedEntities( void );
	void Computelighting( int state );

   void CTHREDDoc::RenderLights( CRenderCamera* Camera, int ViewType, 
   	double mZoomFactor, TSRenderContext* rc);
      
   // lighting stuff
   void  MoveSelectedLights( double X, double Y, double Z );
   void  DoneMoveLights( bool useEditBox, int viewType );
   void  RotateSelectedLights( EulerF & rot, Point3F & center );
   void DoneRotateSelectedLights( float rotSnap );
   
  int SetCurrentLight( CLightEmitter * pEmitter );
   void  CopySelectedLights( void );
   void  PasteLights( void );
   void  SelectAllLights( void );
   void  SerializeLights( CThredParser &Parser );
   CLightEmitter * SelectLight(CPoint point, CRenderCamera& Camera, BOOL fClear );
   void ResetSelectedLights( void );
   void  DeleteLight( int LightIndex );
   void ClearSelections();

	GFXPalette*     GetPalette();
	BOOL            LoadBrowser(CString& filename, CString filter, CString title);
	BOOL            LoadBrowserAny(CString& filename, CString filter, CString title);
	BOOL            LoadBrowserMulti(CString& filename, CString filter, CString title, char* filenamebuffer, int buffersize);
	TSMaterialList* GetMaterialList();


// Operations
public:
	friend class CTHREDView;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTHREDDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	virtual BOOL OnSaveDocument( LPCTSTR lpszPathName );
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTHREDDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTHREDDoc)
	afx_msg void OnToolsBrushAttributes();
	afx_msg void OnBrushAddtoworld();
	afx_msg void OnBrushSubtractfromworld();
	afx_msg void OnBrushPrimitivesCube();
	afx_msg void OnBrushPrimitivesSpheroid();
	afx_msg void OnBrushPrimitivesCylinder();
	afx_msg void OnFileExport();
	afx_msg void OnToolsUsegrid();
	afx_msg void OnUpdateToolsUsegrid(CCmdUI* pCmdUI);
	afx_msg void OnToolsGridsettings();
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnToolsBspmanager();
	afx_msg void OnEntitiesEditor();
	afx_msg void OnEntitiesPlace();
	afx_msg void OnEntitiesShow();
	afx_msg void OnUpdateEntitiesShow(CCmdUI* pCmdUI);
	afx_msg void OnBrushGroupsAddtogroup();
	afx_msg void OnBrushGroupsMakenewgroup();
	afx_msg void OnBrushGroupeditor();
	afx_msg void OnViewShowAllbrushes();
	afx_msg void OnViewShowCurrentgroup();
	afx_msg void OnViewShowVisiblebrushes();
	afx_msg void OnUpdateViewShowVisiblebrushes(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewShowAllbrushes(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewShowCurrentgroup(CCmdUI* pCmdUI);
	afx_msg void OnBrushAdjustmentmode();
	afx_msg void OnUpdateBrushAdjustmentmode(CCmdUI* pCmdUI);
	afx_msg void OnBrushSelectedDelete();
	afx_msg void OnBrushSelectedNextinorder();
	afx_msg void OnBrushSelectedPreviousinorder();
	afx_msg void OnBrushPrimitivesStaircase();
	afx_msg void OnFileMapsettings();
	afx_msg void OnQbspnowater();
	afx_msg void OnEntitiesCopycurrent();
	afx_msg void OnUpdateEntitiesCopycurrent(CCmdUI* pCmdUI);
	afx_msg void OnToolsGroupmovementmode();
	afx_msg void OnUpdateToolsGroupmovementmode(CCmdUI* pCmdUI);
	afx_msg void OnBrushRemoveselectedfromgroup();
	afx_msg void OnUpdateBrushRemoveselectedfromgroup(CCmdUI* pCmdUI);
	afx_msg void OnBrushPrimitivesTriangle();
	afx_msg void OnUpdateFileNew(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileOpen(CCmdUI* pCmdUI);
	afx_msg void OnToolsSettexturescale();
	afx_msg void OnToolsSetlighting();
	afx_msg void OnFileExportOptimal();
	afx_msg void OnToolsLoadnewpalette();
	afx_msg void OnBrushCopytonewgroup();
	afx_msg void OnUpdateBrushCopytonewgroup(CCmdUI* pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnAxisX();
	afx_msg void OnUpdateAxisX(CCmdUI* pCmdUI);
	afx_msg void OnAxisY();
	afx_msg void OnUpdateAxisY(CCmdUI* pCmdUI);
	afx_msg void OnAxisZ();
	afx_msg void OnUpdateAxisZ(CCmdUI* pCmdUI);
	afx_msg void OnToolsSetbackgroundcolor();
	afx_msg void OnFileExportbrushgroup();
	afx_msg void OnFileImportbrushgroup();
	afx_msg void OnEditSelectall();
	afx_msg void OnUpdateEditSelectall(CCmdUI* pCmdUI);
	afx_msg void OnBrushAdjustbrushorder();
	afx_msg void OnUpdateBrushAdjustbrushorder(CCmdUI* pCmdUI);
	afx_msg void OnUpdateComputelighting(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileGeneratelighttablereport(CCmdUI* pCmdUI);
	afx_msg void OnFileGeneratelighttablereport();
	afx_msg void OnToolsAssignvolumes();
	afx_msg void OnUpdateToolsAssignvolumes(CCmdUI* pCmdUI);
	afx_msg void OnViewShowVolumes();
	afx_msg void OnUpdateViewShowVolumes(CCmdUI* pCmdUI);
	afx_msg void OnViewShowFilledvolumes();
	afx_msg void OnUpdateViewShowFilledvolumes(CCmdUI* pCmdUI);
	afx_msg void OnEntitiesDeleteselected();
	afx_msg void OnUpdateEntitiesDeleteselected(CCmdUI* pCmdUI);
	afx_msg void OnDeleteselections();
	afx_msg void OnFileExportAs();
	afx_msg void OnLightsEditor();
   afx_msg void OnUpdateLightsEditor( CCmdUI * pCmdUI );
	afx_msg void OnLightsPlace();
	afx_msg void OnLightsShow();
	afx_msg void OnUpdateLightsShow(CCmdUI * pCmdUI );
   afx_msg void OnLightsShowFalloffs();
   afx_msg void OnUpdateLightsShowFalloffs( CCmdUI * pCmdUI );
	afx_msg void OnLightsCopyCurrent();
	afx_msg void OnUpdateLightsCopyCurrent(CCmdUI * pCmdUI);
	afx_msg void OnLightsDeleteSelected();
	afx_msg void OnUpdateLightsDeleteSelected(CCmdUI * pCmdUI);
	afx_msg void OnComputelighting();
	afx_msg void OnUpdateLightLockmove(CCmdUI* pCmdUI);
	afx_msg void OnLightLockmove();
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnLockTexturePositions();
	afx_msg void OnUpdateLockTexturePositions(CCmdUI* pCmdUI);
   afx_msg void OnUpdateLightsShow3D(CCmdUI* pCmdUI);
   afx_msg void OnUpdateEntitiesShow3D(CCmdUI* pCmdUI);
   afx_msg void OnLightsShow3D();
   afx_msg void OnEntitiesShow3D();
   afx_msg void OnEntitiesConvert();
      
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:

	int mDrawFine, mDrawCoarse;
	double mRotationSnapDegrees;

	// our main dialogs
	CCreateBoxDialog	  mBoxCreation;
	CCreateTriDialog	  mTriangleCreation;
	CCreateSpheroidDialog mSpheroidCreation;
	CCreateCylDialog	  mCylCreation;
	CCreateStaircaseDialog mStairCreation;

	// DPW - Added for TS compatibility
	GFXPalette*			m_SPalette;
	TSLight* 			m_mySun;
	TSMaterialList*		m_materialList;
    //GFXBitmap*          m_entityBitmap;
    TSMaterial          m_entityMaterial;

    // DPW - Newest 3space stuff (1/2/97)
    ResourceObject*     m_matresobj;
    ResourceObject*     m_palresobj;

	double				m_fIntensity;
	Point3I				m_lightVector;
	char				   palName[256];
	char				   matName[256];
	char				   palVolName[256];
	char				   matVolName[256];

    //CBitmap             LightBitmap;

//	VolumeListEntry *	m_PalPath;
//	VolumeListEntry *   m_MatPath;

public:
	CLightArray * 		pLightArray;
	CBrushGroupArray * 	pBrushGroupArray;
	CEntityArray * 		pEntityArray;
	
	// SHAPE INFO that is set on a switch of details - major crud way of
	// doing things but unless we want a complete rewrite, there is no
	// other way of doing this
	SHAPE_INFO

	// stuff for the state/detail/lightset info
	CShapeState *		m_StateList;

	CEditObject			m_ObjectEdit;

	// the active detail for this document ( the one the user can 
	//	manipulate)
	CShapeDetail * 	pActiveDetail;

	CShapeState * AddState( CShapeState * pState );
	CShapeState * AddState( CString Name );
	BOOL CopyState( CShapeState * pState );
	BOOL PasteState( void );
	BOOL RemoveState( CShapeState * pState );
	void NameStates( void );
	BOOL DetailExist( CShapeDetail * pDetail );
   int NumDetails();

	BOOL CheckExport( void );
	BOOL ExportVolume( VolumeRWStream & Volume, CString ExportFileName,
		CString Base, BOOL OptimalBuild );
   bool exportLights( CString & fileName );
   bool SerializeSettings( CThredParser& Parser, CString & file );
   bool LoadPalette();

	// option information
	ITRShapeEdit 		   ShapeInfo;
	ITRBuildOptions		BuildOptions;
   ITRLightingOptions   LightOptions;

	void Properties( void );
};

////////////////////////////////////////////////////////////////////////


#endif
