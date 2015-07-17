#include <Max.h>
#include "SceneEnum.h"
#include "pb3deres.h"
#include "pb3dexp.h"

//------------------------------------------------------

// run gencid.exe in 3dsmax\maxsdk\help to generate class Ids.
#define CLASS_ID1	0x07315a9c
#define CLASS_ID2	0x59b32e5a

HINSTANCE hInstance;

//------------------------------------------------------
// some handy functions:

TCHAR *GetString(int id)
	{
	static TCHAR buf[256];
	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}

static void MessageBox(int s1, int s2) {
	TSTR str1(GetString(s1));
	TSTR str2(GetString(s2));
	MessageBox(GetActiveWindow(), str1.data(), str2.data(), MB_OK);
	}

static int MessageBox(int s1, int s2, int option = MB_OK) {
	TSTR str1(GetString(s1));
	TSTR str2(GetString(s2));
	return MessageBox(GetActiveWindow(), str1, str2, option);
	}

static int Alert(int s1, int s2 = IDS_TH_EXP, int option = MB_OK) {
	return MessageBox(s1, s2, option);
	}

//------------------------------------------------------
// Our main plugin class:

class _DTSExport : public SceneExport 
	{
public:
						_DTSExport();
						~_DTSExport();
	int				ExtCount();					// Number of extensions supported
	const TCHAR *	Ext(int n);					// Extension #n (i.e. "DTS")
	const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Dynamix 3Space Shape File")
	const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3Space Studio")
	const TCHAR *	AuthorName();				// ASCII Author name
	const TCHAR *	CopyrightMessage();		// ASCII Copyright message
	const TCHAR *	OtherMessage1();			// Other message #1
	const TCHAR *	OtherMessage2();			// Other message #2
	unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
	void				ShowAbout(HWND hWnd);	// Show DLL's "About..." box
	int				DoExport(const TCHAR *name,ExpInterface *ei,Interface *i);	// Export file
	};

//------------------------------------------------------
// Jaguar interface code

int controlsInit = FALSE;

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
	{
	hInstance = hinstDLL;

	if ( !controlsInit ) 
		{
		controlsInit = TRUE;
		
		// jaguar controls
		InitCustomControls(hInstance);

		// initialize Chicago controls
		InitCommonControls();
		}
	switch(fdwReason) 
		{
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
		}
	return(TRUE);
	}


//------------------------------------------------------

class _DTSClassDesc:public ClassDesc {
	public:
	int 				IsPublic() { return 1; }
	void *			Create(BOOL loading = FALSE) { return new _DTSExport; }
	const TCHAR *	ClassName() { return GetString(IDS_TH_CLASSDESC); }
	SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID			ClassID() { return Class_ID( CLASS_ID1, CLASS_ID2 ); }
	const TCHAR* 	Category() { return GetString(IDS_TH_SHAPEEXPORT);  }
	};

static _DTSClassDesc _DTSDesc;

//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() { return GetString(IDS_TH_EXPORTDLL); }

__declspec( dllexport ) int
LibNumberClasses() { return 1; }

__declspec( dllexport ) ClassDesc *
LibClassDesc(int i) {
	switch(i) {
		case 0: return &_DTSDesc; break;
		default: return 0; break;
		}
	}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

//
// ._DTS export module functions follow:
//

_DTSExport::_DTSExport() {
	}

_DTSExport::~_DTSExport() {
	}

int
_DTSExport::ExtCount() {
	return 1;
	}

const TCHAR *
_DTSExport::Ext(int n) {		// Extensions supported for import/export modules
	switch(n) {
		case 0:
			return _T("P3D");
		}
	return _T("");
	}

const TCHAR *
_DTSExport::LongDesc() {			// Long ASCII description (i.e. "Targa 2.0 Image File")
	return GetString(IDS_TH_FILETYPE_LONG);
	}
	
const TCHAR *
_DTSExport::ShortDesc() {			// Short ASCII description (i.e. "Targa")
	return GetString(IDS_TH_FILETYPE_SHORT);
	}

const TCHAR *
_DTSExport::AuthorName() {			// ASCII Author name
	return GetString(IDS_TH_AUTHOR);
	}

const TCHAR *
_DTSExport::CopyrightMessage() {	// ASCII Copyright message
	return GetString(IDS_TH_COPYRIGHT_DYNAMIX);
	}

const TCHAR *
_DTSExport::OtherMessage1() {		// Other message #1
	return _T("");
	}

const TCHAR *
_DTSExport::OtherMessage2() {		// Other message #2
	return _T("");
	}

unsigned int
_DTSExport::Version() {				// Version number * 100 (i.e. v3.01 = 301)
	return 100;
	}

void
_DTSExport::ShowAbout(HWND hWnd) {			// Optional
 	}

//------------------------------------------------------

int _dts_save(const TCHAR *filename, ExpInterface *ei, Interface *gi, 
				  _DTSExport *exp) 
{
	//-----------------------------------------------
	// Make sure there are nodes we're interested in!
	// Ask the scene to enumerate all its nodes so we 
	// can determine if there are any we can use:
	SceneEnumProc myScene(ei->theScene);

	//------------------
	// Any useful nodes?
	if ( !myScene.numShapes() ) 
	{
		Alert(IDS_TH_NODATATOEXPORT);
		return 1;
	}
	else if ( myScene.numShapes() == MAX_SHAPES )
	{
		Alert(IDS_TH_TOO_MANY);
		return 1;
	}
	

	//------------------------------------------
	// open a file to save the exported shape to:
	
	FILE *fp = fopen( filename, "wb" );
	
   if ( !fp )
   {
		Alert(IDS_TH_CANTCREATE);
		return(0);
	}

	//-----------------------
	// actually do the export:
	myScene.processNodes( fp );

	//---------------------------------------
	// close the file and report any problems:

	fclose( fp );	
	
	return 1;	
}

int _DTSExport::DoExport(const TCHAR *filename,ExpInterface *ei,Interface *gi) 
{
	int status;
		
	status = _dts_save(filename, ei, gi, this);

	if (status == 0)
		return 1;		// Dialog cancelled
	if (status < 0)
		return 0;		// Real, honest-to-goodness error
	return(status);
}
