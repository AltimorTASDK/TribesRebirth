/**********************************************************************
 *<
	FILE: pthelp.cpp

	DESCRIPTION:  A point helper implementation

	CREATED BY: 

	HISTORY: created 14 July 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#include "helpers.h"
#include "dummy.h"
#include "iparamm.h"

//------------------------------------------------------

#define SEQUENCE_CLASS_ID    0x09923023
#define TRANSITION_CLASS_ID1 0x22065872
#define TRANSITION_CLASS_ID2 0x5cb77942

#define PB_CEL_FRAMERATE	0
#define PB_FORWARD_TAG	   1
#define PB_REVERSE_TAG	   2
#define PB_SEQ_ONESHOT	   3
#define PB_SEQ_FORCEVIS	   4
#define PB_SEQ_VISONLY 	   5
#define PB_SEQ_NOCOLLAPSE	6
#define PB_SEQ_USECELRATE	7

static ParamUIDesc descParam[] = {
	// Cel Frame Rate
	ParamUIDesc(
		PB_CEL_FRAMERATE,
		EDITTYPE_FLOAT,
		IDC_CEL_FRAMERATE,IDC_CEL_FRAMERATESPINNER,
		0.0f,100.0f,
		0.0f),
   ParamUIDesc(
      PB_SEQ_ONESHOT,
      TYPE_SINGLECHEKBOX,
      IDC_SEQ_ONESHOT ),
   ParamUIDesc(
      PB_SEQ_FORCEVIS,
      TYPE_SINGLECHEKBOX,
      IDC_SEQ_FORCEVIS ),
   ParamUIDesc(
      PB_SEQ_VISONLY,
      TYPE_SINGLECHEKBOX,
      IDC_SEQ_VISONLY ),
   ParamUIDesc(
      PB_SEQ_NOCOLLAPSE,
      TYPE_SINGLECHEKBOX,
      IDC_SEQ_NOCOLLAPSE ),
   ParamUIDesc(
      PB_SEQ_USECELRATE,
      TYPE_SINGLECHEKBOX,
      IDC_SEQ_USECELRATE )
	};

#define PARAMDESC_LENGTH  (sizeof( descParam ) / sizeof( ParamUIDesc ))

// older versions.
static ParamBlockDescID descVer0[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 } // Frame Rate
};

static ParamBlockDescID descVer1[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 }, // Cel Frame Rate
	{ TYPE_FLOAT, NULL, TRUE, 1 } // Node Frame Rate
};

static ParamBlockDescID descVer2[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },  // Frame Rate
	{ TYPE_FLOAT, NULL, TRUE, 1 },  // Forward Tag
	{ TYPE_FLOAT, NULL, TRUE, 2 }   // Reverse Tag
};

// This is the current version.
static ParamBlockDescID descVer3[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },  // Frame Rate
	{ TYPE_FLOAT, NULL, TRUE, 1 },  // Forward Tag
	{ TYPE_FLOAT, NULL, TRUE, 2 },   // Reverse Tag
	{ TYPE_BOOL,  NULL, FALSE, 3 }   // oneshot
};

static ParamBlockDescID descVer4[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },  // Frame Rate
	{ TYPE_FLOAT, NULL, TRUE, 1 },  // Forward Tag
	{ TYPE_FLOAT, NULL, TRUE, 2 },   // Reverse Tag
	{ TYPE_BOOL,  NULL, FALSE, 3 },  // oneshot
	{ TYPE_BOOL,  NULL, FALSE, 4 }   // forcevis
};

static ParamBlockDescID descVer5[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },  // Frame Rate
	{ TYPE_FLOAT, NULL, TRUE, 1 },  // Forward Tag
	{ TYPE_FLOAT, NULL, TRUE, 2 },   // Reverse Tag
	{ TYPE_BOOL,  NULL, FALSE, 3 },  // oneshot
	{ TYPE_BOOL,  NULL, FALSE, 4 },  // forcevis
	{ TYPE_BOOL,  NULL, FALSE, 5 }   // visonly
};

static ParamBlockDescID descVer6[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },  // Frame Rate
	{ TYPE_FLOAT, NULL, TRUE, 1 },  // Forward Tag
	{ TYPE_FLOAT, NULL, TRUE, 2 },   // Reverse Tag
	{ TYPE_BOOL,  NULL, FALSE, 3 },  // oneshot
	{ TYPE_BOOL,  NULL, FALSE, 4 },  // forcevis
	{ TYPE_BOOL,  NULL, FALSE, 5 },  // visonly
	{ TYPE_BOOL,  NULL, FALSE, 6 }   // use cel frame rate
};

// Array of old versions
static ParamVersionDesc versions[] = 
{
	ParamVersionDesc( descVer0, 1, 0 ),
	ParamVersionDesc( descVer1, 2, 1 ),
	ParamVersionDesc( descVer2, 3, 2 ),
	ParamVersionDesc( descVer3, 4, 3 ),
	ParamVersionDesc( descVer4, 5, 4 ),
	ParamVersionDesc( descVer4, 6, 5 ),
	ParamVersionDesc( descVer4, 7, 6 )
};


// This is the current version.
static ParamBlockDescID curVer[] = {
	{ TYPE_FLOAT, NULL, TRUE, 0 },  // Frame Rate
	{ TYPE_FLOAT, NULL, TRUE, 1 },  // Forward Tag
	{ TYPE_FLOAT, NULL, TRUE, 2 },   // Reverse Tag
	{ TYPE_BOOL,  NULL, FALSE, 3 },   // oneshot
	{ TYPE_BOOL,  NULL, FALSE, 4 },  // forcevis
	{ TYPE_BOOL,  NULL, FALSE, 5 },  // visonly
	{ TYPE_BOOL,  NULL, FALSE, 6 },  // nocollapse
	{ TYPE_BOOL,  NULL, FALSE, 7 }   // use cel frame rate
};



#define NUM_OLDVERSIONS	   (sizeof(versions) / sizeof(ParamVersionDesc))
#define CURRENT_VERSION	NUM_OLDVERSIONS

// Current version
#define PBLOCK_LENGTH	(sizeof(curVer) / sizeof(ParamBlockDescID))
static ParamVersionDesc curVersion( curVer, PBLOCK_LENGTH, CURRENT_VERSION );

class SequenceObject: public DummyObject {
	public:			
		// Class vars
//		static HWND hParams;
//		static IObjParam *iObjParams;
		static IParamMap *pmapParam;
		static float dlgCelFrameRate;
		static BOOL dlgSeqOneshot;
		static BOOL dlgSeqForcevis;
		static BOOL dlgSeqVisOnly;
		static BOOL dlgSeqNoCollapse;
		static BOOL dlgSeqUseCelRate;
		static IObjParam *ip;


		IParamBlock *pblock;
		Interval ivalid;

		static SequenceObject *editOb;

      // Snap suspension flag (TRUE during creation only)
		BOOL suspendSnap;
					
		// Params
 		int extDispFlags;

		//  inherited virtual methods for Reference-management
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );		
	
		SequenceObject();
		~SequenceObject();
		
		// From BaseObject
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

		RefTargetHandle Clone(RemapDir& remap = NoRemap());

      // From Object
		virtual TCHAR *GetObjectName() { return _T("Sequence"); }
		void InitNodeName(TSTR& s) { s = GetString(IDS_DB_SEQUENCE); }
		Interval ObjectValidity(TimeValue t);
      /*
		ObjectState Eval(TimeValue time);
		ObjectHandle ApplyTransform(Matrix3& matrix) {return this;}
		Interval ObjectValidity(TimeValue t) {return FOREVER;}
		int CanConvertToType(Class_ID obtype) {return FALSE;}
		Object* ConvertToType(TimeValue t, Class_ID obtype) {assert(0);return NULL;}		
		void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
		void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
		int DoOwnSelectHilite()	{ return 1; }
      */

		// Animatable methods
		void FreeCaches(); 		
		int NumSubs() { return 1; }  
		Animatable* SubAnim(int i) { return pblock; }
		TSTR SubAnimName(int i);
		void DeleteThis() { delete this; }
		Class_ID ClassID() { return Class_ID(SEQUENCE_CLASS_ID,0); }  
		void GetClassName(TSTR& s) { s = TSTR(GetString(IDS_DB_SEQUENCE)); }
  //		int IsKeyable(){ return 0;}
		
		// From ref
		int NumRefs() {return 1;}
		RefTargetHandle GetReference(int i) {return pblock;}
		void SetReference(int i, RefTargetHandle rtarg) {pblock=(IParamBlock*)rtarg;}		
      //RefTargetHandle Clone(RemapDir& remap = NoRemap());
		IOResult Load(ILoad *iload);
//		IOResult Save(ISave *isave);

		virtual void InvalidateUI();
		IParamArray *GetParamBlock();
		int GetParamBlockIndex(int id);
		virtual	ParamDimension *GetParameterDim(int pbIndex);
		virtual TSTR GetParameterName(int pbIndex);
	};				

// class variable for sequence class.
SequenceObject *SequenceObject::editOb = NULL;
//HWND SequenceObject::hParams = NULL;
//IObjParam *SequenceObject::iObjParams;
IParamMap *SequenceObject::pmapParam  = NULL;
IObjParam *SequenceObject::ip;
float SequenceObject::dlgCelFrameRate = 0.0f;
BOOL  SequenceObject::dlgSeqOneshot = 0;
BOOL  SequenceObject::dlgSeqForcevis = 0;
BOOL  SequenceObject::dlgSeqVisOnly = 0;
BOOL  SequenceObject::dlgSeqNoCollapse = 0;
BOOL  SequenceObject::dlgSeqUseCelRate = 0;


void resetSequenceParams() 
{
	SequenceObject::dlgCelFrameRate = 0.0f;
	SequenceObject::dlgSeqOneshot = 0;
	SequenceObject::dlgSeqForcevis = 0;
	SequenceObject::dlgSeqVisOnly = 0;
	SequenceObject::dlgSeqNoCollapse = 0;
	SequenceObject::dlgSeqUseCelRate = 0;
}


class SequenceObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new SequenceObject; }
	const TCHAR *	ClassName() { return GetString(IDS_DB_SEQUENCE); }
	SClass_ID		SuperClassID() { return HELPER_CLASS_ID; }
	Class_ID		ClassID() { return Class_ID(SEQUENCE_CLASS_ID,0); }
	const TCHAR* 	Category() { return GetString(IDS_DB_GENERAL);  }
	void			ResetClassParams(BOOL fileReset) { if(fileReset) resetSequenceParams(); }
	};

static SequenceObjClassDesc sequenceObjDesc;

ClassDesc* GetSequenceDesc() { return &sequenceObjDesc; }



// in prim.cpp  - The dll instance handle
extern HINSTANCE hInstance;

									 
/*
BOOL CALLBACK SequenceParamProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	SequenceObject *po = (SequenceObject*)GetWindowLong(hWnd,GWL_USERDATA);
	if (!po && msg!=WM_INITDIALOG) return FALSE;

	switch (msg) {
		case WM_INITDIALOG: {
			po = (SequenceObject*)lParam;
			SetWindowLong(hWnd,GWL_USERDATA,lParam);
			ISpinnerControl *spin = 
				GetISpinner(GetDlgItem(hWnd,IDC_FRAMERATESPINNER));
			spin->SetLimits(10,1000,FALSE);
			spin->SetScale(0.1f);
			spin->SetValue(po->frameRate,FALSE);
			spin->LinkToEdit(GetDlgItem(hWnd,IDC_FRAMERATE),EDITTYPE_FLOAT);
			ReleaseISpinner(spin);
			return FALSE;
			}

		case CC_SPINNER_CHANGE: {
			ISpinnerControl *spin = (ISpinnerControl*)lParam;
			po->frameRate = spin->GetFVal();
			po->NotifyDependents(FOREVER,PART_OBJ,REFMSG_CHANGE);
			po->iObjParams->RedrawViews(po->iObjParams->GetTime());
			break;
			}

		case WM_COMMAND:
			//switch (LOWORD(wParam)) {
			//	}
			break;
		
		default:
			return FALSE;
		}
	
	return TRUE;
	} 
*/

IParamArray *SequenceObject::GetParamBlock()
	{
	return pblock;
	}

int SequenceObject::GetParamBlockIndex(int id)
	{
	if (pblock && id>=0 && id<pblock->NumParams()) return id;
	else return -1;
	}

void SequenceObject::FreeCaches() 
	{
	ivalid.SetEmpty();
	} 

RefResult SequenceObject::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
   		PartID& partID, 
   		RefMessage message ) 
   	{
	switch (message) {
		case REFMSG_CHANGE:
			if (editOb==this) InvalidateUI();
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			gpd->dim = GetParameterDim(gpd->index);			
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			gpn->name = GetParameterName(gpn->index);			
			return REF_STOP; 
			}
		}
	return(REF_SUCCEED);
	}

Interval SequenceObject::ObjectValidity(TimeValue time) 
	{
	return ivalid;	
	}

TSTR SequenceObject::SubAnimName(int i) 
	{ 
	return _T("Parameters");//TSTR(GetResString(IDS_RB_PARAMETERS));
	}

void SequenceObject::BeginEditParams(
		IObjParam *ip, ULONG flags,Animatable *prev)
	{	
//  	MessageBox(GetActiveWindow(), "SequenceObject::BeginEditParams:1", "", MB_OK);
	editOb = this;
	// Save the interface pointer.
	this->ip = ip;

	if (pmapParam) {
		
		// Left over from last sequence ceated
		pmapParam->SetParamBlock(pblock);
	} 
   else
   {
		pmapParam = CreateCPParamMap(
			descParam,PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE(IDD_SEQUENCEPARAMS),
			_T("Parameters"),
			0);
		}

//  	MessageBox(GetActiveWindow(), "SequenceObject::BeginEditParams:2", "", MB_OK);
   /*
	iObjParams = ip;
	if (!hParams) {
		hParams = ip->AddRollupPage( 
				hInstance, 
				MAKEINTRESOURCE(IDD_SEQUENCEPARAMS),
				SequenceParamProc, 
				GetString(IDS_DB_PARAMETERS), 
				(LPARAM)this );
		ip->RegisterDlgWnd(hParams);
	} else {
		SetWindowLong(hParams,GWL_USERDATA,(LONG)this);
		ISpinnerControl *spin = 
			GetISpinner(GetDlgItem(hParams,IDC_FRAMERATESPINNER));
		spin->SetValue(frameRate,FALSE);
		ReleaseISpinner(spin);
		}
   */
	}
		
void SequenceObject::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
	{	
//  	MessageBox(GetActiveWindow(), "SequenceObject::EndEditParams:1", "", MB_OK);
	editOb = NULL;
	ClearAFlag(A_OBJ_CREATING);
	this->ip = NULL;

	if ( (flags&END_EDIT_REMOVEUI) && pmapParam ) 
   {
		// Remove the rollup pages from the command panel.
		DestroyCPParamMap(pmapParam);
		pmapParam  = NULL;
	}

   /*
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hParams,IDC_FRAMERATESPINNER));
	frameRate = spin->GetFVal();
	ReleaseISpinner(spin);
	if (flags&END_EDIT_REMOVEUI) {
		ip->UnRegisterDlgWnd(hParams);
		ip->DeleteRollupPage(hParams);
		hParams = NULL;
	} else {
		SetWindowLong(hParams,GWL_USERDATA,0);
		}
	iObjParams = NULL;
   */
	}


// This method is called when the user interface controls need to be
// updated to reflect new values because of the user moving the time
// slider.  Here we simply call a method of the parameter map to 
// handle this for us.
void SequenceObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

// This method returns the dimension of the parameter requested.
// This dimension describes the type and magnitude of the value
// stored by the parameter.
ParamDimension *SequenceObject::GetParameterDim(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_CEL_FRAMERATE:
			return stdNormalizedDim;			
		case PB_FORWARD_TAG:
		case PB_REVERSE_TAG:
      case PB_SEQ_ONESHOT:
			return stdNormalizedDim;			
		default:
			return defaultDim;
		}
	}

// This method returns the name of the parameter requested.
TSTR SequenceObject::GetParameterName(int pbIndex) 
	{
	switch (pbIndex) {
		case PB_CEL_FRAMERATE:
			return TSTR(_T("CelFrameRate"));
		case PB_FORWARD_TAG:
			return TSTR(_T("ForwardTag"));
		case PB_REVERSE_TAG:
			return TSTR(_T("ReverseTag"));
      case PB_SEQ_ONESHOT:
         return TSTR(_T("Oneshot"));
      case PB_SEQ_FORCEVIS:
         return TSTR(_T("Force Vis"));
      case PB_SEQ_VISONLY:
         return TSTR(_T("Vis Only"));
		default:
			return TSTR(_T(""));
		}
	}

// Make a copy of the sphere object parameter block.
RefTargetHandle SequenceObject::Clone(RemapDir& remap) 
	{
	SequenceObject* newob = new SequenceObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));
	newob->ivalid.SetEmpty();	
	return(newob);
	}

SequenceObject::SequenceObject()
	{	
//  	MessageBox(GetActiveWindow(), "SequenceObject::SequenceObject:1", "", MB_OK);
	ivalid.SetEmpty();
	SetAFlag(A_OBJ_CREATING);
	suspendSnap = FALSE;
//  	MessageBox(GetActiveWindow(), "SequenceObject::SequenceObject:2", "", MB_OK);

   // Create the parameter block and make a reference to it.
	pblock = 0;
	MakeRefByID(FOREVER, 0, CreateParameterBlock( curVersion.desc, PBLOCK_LENGTH, 
      		CURRENT_VERSION));
//  	MessageBox(GetActiveWindow(), "SequenceObject::SequenceObject:3", "", MB_OK);
	assert(pblock);
//  	MessageBox(GetActiveWindow(), "SequenceObject::SequenceObject:4", "", MB_OK);

	// Initialize the default values.
	pblock->SetValue( PB_CEL_FRAMERATE, 30, dlgCelFrameRate );
	pblock->SetValue( PB_SEQ_ONESHOT, 0, dlgSeqOneshot );
	pblock->SetValue( PB_SEQ_FORCEVIS, 0, dlgSeqForcevis );
	pblock->SetValue( PB_SEQ_VISONLY, 0, dlgSeqVisOnly );
	pblock->SetValue( PB_SEQ_NOCOLLAPSE, 0, dlgSeqNoCollapse );
	pblock->SetValue( PB_SEQ_USECELRATE, 0, dlgSeqUseCelRate );
//  	MessageBox(GetActiveWindow(), "SequenceObject::SequenceObject:5", "", MB_OK);

//	pblock->SetValue(PB_NODE_FRAMERATE,0,dlgNodeFrameRate);
//  	MessageBox(GetActiveWindow(), "SequenceObject::SequenceObject:6", "", MB_OK);

   /*
   frameRate = dlgFrameRate;
	suspendSnap = FALSE;
   */
	}

SequenceObject::~SequenceObject()
	{	
	DeleteAllRefsFromMe();
	}

//
// Reference Managment:
//

#define FRAMERATE_CHUNK		0x0100

IOResult SequenceObject::Load(ILoad *iload)
	{
	// This is the callback that corrects for any older versions
	// of the parameter block structure found in the MAX file 
	// being loaded.
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,0));
	return IO_OK;

   /*ULONG nb;
	IOResult res = IO_OK;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case FRAMERATE_CHUNK:
				res = iload->Read(&frameRate,sizeof(frameRate),&nb);
				break;
			}

		res = iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}
	
	return IO_OK;
   */
	}

/*
IOResult SequenceObject::Save(ISave *isave)
	{
	ULONG nb;

	isave->BeginChunk(FRAMERATE_CHUNK);
	isave->Write(&frameRate,sizeof(frameRate),&nb);
	isave->EndChunk();
	
	return IO_OK;
	}
*/


#define PB_REVERSE_TRANS	0
#define PB_DURATION			1	
#define PB_GND_SHIFTX		2
#define PB_GND_SHIFTY		3
#define PB_GND_SHIFTZ		4

static ParamUIDesc t_descParam[] = 
{
   ParamUIDesc(
      PB_REVERSE_TRANS,
      TYPE_SINGLECHEKBOX,
      IDC_TRANS_DIRECTION ),
	ParamUIDesc(
		PB_DURATION,
		EDITTYPE_FLOAT,
		IDC_DURATION,IDC_DURATIONSPINNER,
		0.01f,100.0f,
		0.0f),
	ParamUIDesc(
		PB_GND_SHIFTX,
		EDITTYPE_FLOAT,
		IDC_GND_SHIFTX,IDC_GND_SHIFTXSPINNER,
		-100000.0f,100000.0f,
		0.0f),
	ParamUIDesc(
		PB_GND_SHIFTY,
		EDITTYPE_FLOAT,
		IDC_GND_SHIFTY,IDC_GND_SHIFTYSPINNER,
		-100000.0f,100000.0f,
		0.0f),
	ParamUIDesc(
		PB_GND_SHIFTZ,
		EDITTYPE_FLOAT,
		IDC_GND_SHIFTZ,IDC_GND_SHIFTZSPINNER,
		-100000.0f,100000.0f,
		0.0f),
};
#define T_PARAMDESC_LENGTH 5

// This one is the older version.
static ParamBlockDescID t_descVer0[] = {
	{ TYPE_BOOL, NULL, TRUE, PB_REVERSE_TRANS }, // Frame Rate
};

static ParamBlockDescID t_descVer1[] = 
{
   { TYPE_BOOL,  NULL, TRUE, PB_REVERSE_TRANS	},
   { TYPE_FLOAT, NULL, FALSE, PB_DURATION		},
   { TYPE_FLOAT, NULL, FALSE, PB_GND_SHIFTX		},
   { TYPE_FLOAT, NULL, FALSE, PB_GND_SHIFTY		},
   { TYPE_FLOAT, NULL, FALSE, PB_GND_SHIFTZ		},
};
#define T_PBLOCK_LENGTH	5



// Array of old versions
static ParamVersionDesc t_versions[] = {
	ParamVersionDesc( t_descVer0, 1, 0 )
	};
#define T_NUM_OLDVERSIONS	1

static ParamVersionDesc t_curVersion( t_descVer1, T_PBLOCK_LENGTH, 1 );
#define T_CURRENT_VERSION 1

class TransitionObject: public DummyObject 
{
	public:			
		// Class vars
		static IParamMap *pmapParam;
		static int dlgReversed;
		static float duration;
		static float gnd_shiftx;
		static float gnd_shifty;
		static float gnd_shiftz;
		static IObjParam *ip;

		IParamBlock *pblock;
		Interval ivalid;

		static TransitionObject *editOb;

      // Snap suspension flag (TRUE during creation only)
		BOOL suspendSnap;
					
		// Params
 		int extDispFlags;

		//  inherited virtual methods for Reference-management
		RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message );		
	
		TransitionObject();
		~TransitionObject();
		
		// From BaseObject
		void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

		RefTargetHandle Clone(RemapDir& remap = NoRemap());

      // From Object
		virtual TCHAR *GetObjectName();
		void InitNodeName(TSTR& s);
		Interval ObjectValidity(TimeValue t);
      /*
		ObjectState Eval(TimeValue time);
		ObjectHandle ApplyTransform(Matrix3& matrix) {return this;}
		Interval ObjectValidity(TimeValue t) {return FOREVER;}
		int CanConvertToType(Class_ID obtype) {return FALSE;}
		Object* ConvertToType(TimeValue t, Class_ID obtype) {assert(0);return NULL;}		
		void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
		void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box );
		int DoOwnSelectHilite()	{ return 1; }
      */

		// Animatable methods
		void FreeCaches(); 		
		TSTR SubAnimName(int i);
		int NumSubs();
		Animatable* SubAnim(int i);
		void DeleteThis();
		Class_ID ClassID();
		void GetClassName(TSTR& s);
  //		int IsKeyable(){ return 0;}
		
		// From ref
		int NumRefs();
		RefTargetHandle GetReference(int i);
		void SetReference(int i, RefTargetHandle rtarg);
      //RefTargetHandle Clone(RemapDir& remap = NoRemap());
		IOResult Load(ILoad *iload);
//		IOResult Save(ISave *isave);

		virtual void InvalidateUI();
		IParamArray *GetParamBlock();
		int GetParamBlockIndex(int id);
		virtual	ParamDimension *GetParameterDim(int pbIndex);
		virtual TSTR GetParameterName(int pbIndex);
};				


TCHAR * TransitionObject::GetObjectName() { return _T("TRANSITION"); }

void TransitionObject::InitNodeName(TSTR& s) 
{ 
//	MessageBox(GetActiveWindow(), "TO::initNodeName", "", MB_OK);
	s = GetString(IDS_DB_TRANSITION); 
}

int TransitionObject::NumSubs() { return 1; }  
Animatable* TransitionObject::SubAnim(int i) { return pblock; }
void TransitionObject::DeleteThis() { delete this; }
Class_ID TransitionObject::ClassID() { return Class_ID(TRANSITION_CLASS_ID1,TRANSITION_CLASS_ID2); }  
void TransitionObject::GetClassName(TSTR& s) 
{ 
//	MessageBox(GetActiveWindow(), "TO::getClassName", "", MB_OK);
	s = TSTR(GetString(IDS_DB_TRANSITION)); 
}

int TransitionObject::NumRefs() {return 1;}
RefTargetHandle TransitionObject::GetReference(int i) {return pblock;}
void TransitionObject::SetReference(int i, RefTargetHandle rtarg) {pblock=(IParamBlock*)rtarg;}		


TransitionObject *TransitionObject::editOb = NULL;
IParamMap *TransitionObject::pmapParam  = NULL;
IObjParam *TransitionObject::ip;
int TransitionObject::dlgReversed = 0;
float TransitionObject::duration = 0.1f;
float TransitionObject::gnd_shiftx = 0.0f;
float TransitionObject::gnd_shifty = 0.0f;
float TransitionObject::gnd_shiftz = 0.0f;

void resetTransitionParams() 
{
	TransitionObject::dlgReversed = 0;
	TransitionObject::duration = 0.1f;
	TransitionObject::gnd_shiftx = 0.0f;
	TransitionObject::gnd_shifty = 0.0f;
	TransitionObject::gnd_shiftz = 0.0f;
}

class TransitionObjClassDesc:public ClassDesc {
	public:
	int 			IsPublic();
	void *			Create(BOOL loading = FALSE);
	const TCHAR *	ClassName();
	SClass_ID		SuperClassID();
	Class_ID		ClassID();
	const TCHAR* 	Category();
	void			ResetClassParams(BOOL fileReset);
	};

int 			TransitionObjClassDesc::IsPublic() { return 1; }
void *			TransitionObjClassDesc::Create(BOOL loading) 
{ 
//	MessageBox(GetActiveWindow(), "TOCD::Create", "", MB_OK);
	return new TransitionObject; 
}
const TCHAR *	TransitionObjClassDesc::ClassName() 
{ 
  //	MessageBox(GetActiveWindow(), "TOCD::ClassName", "", MB_OK);
	return GetString(IDS_DB_TRANSITION); 
}
SClass_ID		TransitionObjClassDesc::SuperClassID() { return HELPER_CLASS_ID; }
Class_ID		TransitionObjClassDesc::ClassID() { return Class_ID(TRANSITION_CLASS_ID1,TRANSITION_CLASS_ID2); }
const TCHAR* 	TransitionObjClassDesc::Category() { return GetString(IDS_DB_GENERAL);  }
void			TransitionObjClassDesc::ResetClassParams(BOOL fileReset) { if(fileReset) resetTransitionParams(); }


static TransitionObjClassDesc transitionObjDesc;
ClassDesc* GetTransitionDesc() { return &transitionObjDesc; }


IParamArray *TransitionObject::GetParamBlock()
	{
	return pblock;
	}

int TransitionObject::GetParamBlockIndex(int id)
	{
	if (pblock && id>=0 && id<pblock->NumParams()) return id;
	else return -1;
	}

void TransitionObject::FreeCaches() 
	{
	ivalid.SetEmpty();
	} 

RefResult TransitionObject::NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
   		PartID& partID, 
   		RefMessage message ) 
   	{
	switch (message) {
		case REFMSG_CHANGE:
			if (editOb==this) InvalidateUI();
			break;

		case REFMSG_GET_PARAM_DIM: {
			GetParamDim *gpd = (GetParamDim*)partID;
			gpd->dim = GetParameterDim(gpd->index);			
			return REF_STOP; 
			}

		case REFMSG_GET_PARAM_NAME: {
			GetParamName *gpn = (GetParamName*)partID;
			gpn->name = GetParameterName(gpn->index);			
			return REF_STOP; 
			}
		}
	return(REF_SUCCEED);
	}

Interval TransitionObject::ObjectValidity(TimeValue time) 
	{
	return ivalid;	
	}

TSTR TransitionObject::SubAnimName(int i) 
	{ 
	return _T("Parameters");//TSTR(GetResString(IDS_RB_PARAMETERS));
	}

void TransitionObject::BeginEditParams(
		IObjParam *ip, ULONG flags,Animatable *prev)
	{	
//  	MessageBox(GetActiveWindow(), "TransitionObject::BeginEditParams:1", "", MB_OK);
	editOb = this;
	// Save the interface pointer.
	this->ip = ip;

	if (pmapParam) {
		
		// Left over from last sequence ceated
		pmapParam->SetParamBlock(pblock);
	} else {
		pmapParam = CreateCPParamMap(
			t_descParam, T_PARAMDESC_LENGTH,
			pblock,
			ip,
			hInstance,
			MAKEINTRESOURCE( IDD_TRANSITIONPARAMS ),
			_T("Parameters"),
			0);
		}
//  	MessageBox(GetActiveWindow(), "TransitionObject::BeginEditParams: end", "", MB_OK);
	}
		
void TransitionObject::EndEditParams(
		IObjParam *ip, ULONG flags,Animatable *next)
	{	
//  	MessageBox(GetActiveWindow(), "TransitionObject::EndEditParams:1", "", MB_OK);
	editOb = NULL;
	ClearAFlag(A_OBJ_CREATING);
	this->ip = NULL;

	if (flags&END_EDIT_REMOVEUI ) {
		// Remove the rollup pages from the command panel.
		DestroyCPParamMap(pmapParam);
		pmapParam  = NULL;
		}

//  	MessageBox(GetActiveWindow(), "TransitionObject::EndEditParams:end", "", MB_OK);
   /*
	ISpinnerControl *spin = GetISpinner(GetDlgItem(hParams,IDC_FRAMERATESPINNER));
	frameRate = spin->GetFVal();
	ReleaseISpinner(spin);
	if (flags&END_EDIT_REMOVEUI) {
		ip->UnRegisterDlgWnd(hParams);
		ip->DeleteRollupPage(hParams);
		hParams = NULL;
	} else {
		SetWindowLong(hParams,GWL_USERDATA,0);
		}
	iObjParams = NULL;
   */
	}


// This method is called when the user interface controls need to be
// updated to reflect new values because of the user moving the time
// slider.  Here we simply call a method of the parameter map to 
// handle this for us.
void TransitionObject::InvalidateUI() 
	{
	if (pmapParam) pmapParam->Invalidate();
	}

// This method returns the dimension of the parameter requested.
// This dimension describes the type and magnitude of the value
// stored by the parameter.
ParamDimension *TransitionObject::GetParameterDim(int pbIndex) 
	{
//	MessageBox(GetActiveWindow(), "TO::GetParameterDim", "", MB_OK);
	switch (pbIndex) {
		case PB_REVERSE_TRANS:
			return stdNormalizedDim;			
		case PB_DURATION:
		case PB_GND_SHIFTX:
		case PB_GND_SHIFTY:
		case PB_GND_SHIFTZ:
			return stdWorldDim;
		default:
			return defaultDim;
		}
	}

// This method returns the name of the parameter requested.
TSTR TransitionObject::GetParameterName(int pbIndex) 
	{
//	MessageBox(GetActiveWindow(), "TO::GetParameterName", "", MB_OK);
	switch (pbIndex) {
		case PB_REVERSE_TRANS:
			return TSTR(_T("Reverse Trans"));
		case PB_DURATION:
			return TSTR(_T("Duration"));
		case PB_GND_SHIFTX:
			return TSTR(_T("x shift"));
		case PB_GND_SHIFTY:
			return TSTR(_T("y shift"));
		case PB_GND_SHIFTZ:
			return TSTR(_T("z shift"));
		default:
			return TSTR(_T(""));
		}
	}

// Make a copy of the sphere object parameter block.
RefTargetHandle TransitionObject::Clone(RemapDir& remap) 
	{
	TransitionObject* newob = new TransitionObject();	
	newob->ReplaceReference(0,pblock->Clone(remap));
	newob->ivalid.SetEmpty();	
	return(newob);
	}

TransitionObject::TransitionObject()
	{	
//  	MessageBox(GetActiveWindow(), "TransitionObject::TransitionObject:1", "", MB_OK);
	ivalid.SetEmpty();
	SetAFlag(A_OBJ_CREATING);
	suspendSnap = FALSE;
//  	MessageBox(GetActiveWindow(), "TransitionObject::TransitionObject:2", "", MB_OK);

   // Create the parameter block and make a reference to it.
	pblock = 0;
	MakeRefByID(FOREVER, 0, CreateParameterBlock( t_descVer1, T_PBLOCK_LENGTH, 
		T_CURRENT_VERSION));
//  	MessageBox(GetActiveWindow(), "TransitionObject::TransitionObject:3", "", MB_OK);
	assert(pblock);
//  	MessageBox(GetActiveWindow(), "TransitionObject::TransitionObject:4", "", MB_OK);

	// Initialize the default values.
	pblock->SetValue( PB_REVERSE_TRANS, 0, dlgReversed );

	pblock->SetValue( PB_DURATION, 0, duration );
	pblock->SetValue( PB_GND_SHIFTX, 0, gnd_shiftx );
	pblock->SetValue( PB_GND_SHIFTY, 0, gnd_shifty );
	pblock->SetValue( PB_GND_SHIFTZ, 0, gnd_shiftz );
//  	MessageBox(GetActiveWindow(), "TransitionObject::TransitionObject:5", "", MB_OK);
//
   /*
   frameRate = dlgFrameRate;
	suspendSnap = FALSE;
   */
//	MessageBox(GetActiveWindow(), "TO::TO end", "", MB_OK);
	}

TransitionObject::~TransitionObject()
	{	
	DeleteAllRefsFromMe();
	}

//
// Reference Managment:
//

#define FRAMERATE_CHUNK		0x0100

IOResult TransitionObject::Load(ILoad *iload)
	{
	// This is the callback that corrects for any older versions
	// of the parameter block structure found in the MAX file 
	// being loaded.
//  	MessageBox(GetActiveWindow(), "TransitionObject::load", "", MB_OK);
	iload->RegisterPostLoadCallback(
		new ParamBlockPLCB( t_versions, T_NUM_OLDVERSIONS, &t_curVersion, this, 0 ) );
	return IO_OK;

   /*ULONG nb;
	IOResult res = IO_OK;

	while (IO_OK==(res=iload->OpenChunk())) {
		switch (iload->CurChunkID()) {
			case FRAMERATE_CHUNK:
				res = iload->Read(&frameRate,sizeof(frameRate),&nb);
				break;
			}

		res = iload->CloseChunk();
		if (res!=IO_OK)  return res;
		}
	
	return IO_OK;
   */
	}

