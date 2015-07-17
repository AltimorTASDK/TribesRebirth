#include <MAX.H>

#include "RESOURCE.H"
#include "DTSExporter.H"
#include "SceneEnum.H"

#define DLLEXPORT __declspec(dllexport)

HINSTANCE hInstance;

TCHAR *GetString(int id)
{
    static TCHAR buf[256];

    if (hInstance)
    {
        return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
    }

    return NULL;
}

static void MessageBox(int s1, int s2) 
{
    TSTR str1(GetString(s1));
    TSTR str2(GetString(s2));

    MessageBox(GetActiveWindow(), str1.data(), str2.data(), MB_OK);
}

static int MessageBox(int s1, int s2, int option = MB_OK) 
{
    TSTR str1(GetString(s1));
    TSTR str2(GetString(s2));

    return MessageBox(GetActiveWindow(), str1, str2, option);
}

static int Alert(int s1, int s2 = IDS_TH_DTSEXP, int option = MB_OK) 
{
    return MessageBox(s1, s2, option);
}

//------------------------------------------------------
// Our main plugin class:

class _DTSExport : public SceneExport
{
    public:
         _DTSExport();
        ~_DTSExport();

	    int           ExtCount();         // Number of extensions supported
		const TCHAR  *Ext(int);           // Extension #n
		const TCHAR  *LongDesc();         // Long ASCII description
		const TCHAR  *ShortDesc();        // Short ASCII description
		const TCHAR  *AuthorName();       // ASCII Author name
		const TCHAR  *CopyrightMessage(); // ASCII Copyright message
		const TCHAR  *OtherMessage1();    // Other message #1
		const TCHAR  *OtherMessage2();    // Other message #2
		unsigned int Version();           // Version number * 100
		void         ShowAbout(HWND);     // Show DLL's "About..." box

#ifdef MAX_2
	    int DoExport(const TCHAR *, ExpInterface *, Interface *, int);
#else
	    int DoExport(const TCHAR *, ExpInterface *, Interface *);
#endif
    };

//------------------------------------------------------
// Jaguar interface code

int controlsInit = FALSE;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved) 
{
	hInstance = hinstDLL;

    if (!controlsInit) 
    {
        controlsInit = TRUE;
        
        // 3DS custom controls
        InitCustomControls(hInstance);

        // Initialize common Windows controls
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


// Need this stuff for making different DLOs that can co-exist in the 
//    max\plugins directory.  Max seems to want different ID strings
//    and class IDs.  
// Now defined in configurations:
// define  _VICON_EXP_MODE    2

#if _VICON_EXP_MODE==0
#define  _EXPORTER_CLASS_NAME    GetString(IDS_TH_3SPACE)
#define  _EXPORTER_CLASS_ID      Class_ID(0x13332a12, 0x64106b77)
#define  _EXPORTER_CLASS_SDESC   GetString(IDS_TH_DTSFILE_SHORT)
#elif _VICON_EXP_MODE==1
#define  _EXPORTER_CLASS_NAME    "Changed Exporter"
#define  _EXPORTER_CLASS_ID      Class_ID(0x13332a10, 0x64106b70)
#define  _EXPORTER_CLASS_SDESC   "Modified Exporter"
#elif _VICON_EXP_MODE==2
#define  _EXPORTER_CLASS_NAME    "Tribes Exporter 3"
#define  _EXPORTER_CLASS_ID      Class_ID(0x13332a11, 0x64106b74)
#define  _EXPORTER_CLASS_SDESC   "Vicon Combining Exporter"
#endif


class _DTSClassDesc : public ClassDesc 
{
    public:
		int         IsPublic() { return 1; }
		void        *Create(BOOL loading = FALSE) { return new _DTSExport; }
		const TCHAR *ClassName() { return _EXPORTER_CLASS_NAME; }
		SClass_ID   SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
		Class_ID    ClassID() { return _EXPORTER_CLASS_ID; }
		const TCHAR *Category() { return GetString(IDS_TH_SHAPEEXPORT);  }
};

static _DTSClassDesc _DTSDesc;

DLLEXPORT const TCHAR *LibDescription() 
{ 
	return GetString(IDS_TH_DTSEXPORTDLL); 
}

DLLEXPORT int LibNumberClasses() 
{ 
	return 1; 
}

DLLEXPORT ClassDesc *LibClassDesc(int i) 
{
    switch(i) 
	{
        case 0: 
			return &_DTSDesc; 

        default: 
			return 0;
    }
}

// Return version so can detect obsolete DLLs
DLLEXPORT ULONG LibVersion() 
{ 
	return VERSION_3DSMAX; 
}

//
// ._DTS export module functions follow:
//

_DTSExport::_DTSExport() 
{
}

_DTSExport::~_DTSExport() 
{
}

int _DTSExport::ExtCount() 
{
	return 1;
}

// Extensions supported for import/export modules
const TCHAR *_DTSExport::Ext(int n) 
{
	switch(n) 
	{
        case 0:
            return _T("DTS");

		default:
		    return _T("");
    }
}

// Long ASCII description (i.e. "Targa 2.0 Image File")
const TCHAR *_DTSExport::LongDesc() 
{
	return GetString(IDS_TH_DTSFILE_LONG);
}
    
// Short ASCII description (i.e. "Targa")
const TCHAR *_DTSExport::ShortDesc() 
{
    return _EXPORTER_CLASS_SDESC;
    
}

// ASCII Author name
const TCHAR *_DTSExport::AuthorName() 
{
    return GetString(IDS_TH_AUTHOR);
}

// ASCII Copyright message
const TCHAR *_DTSExport::CopyrightMessage() 
{
    return GetString(IDS_TH_COPYRIGHT_DYNAMIX);
}

// Other message #1
const TCHAR *_DTSExport::OtherMessage1() 
{
    return _T("");
}

// Other message #2
const TCHAR *_DTSExport::OtherMessage2() 
{
    return _T("");
}

// Version number * 100 (i.e. v3.01 = 301)
unsigned int _DTSExport::Version() 
{
    return 100;
}

// Optional
void _DTSExport::ShowAbout(HWND hWnd) 
{
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
    if(myScene.Empty()) 
        {
        Alert(IDS_TH_NODATATOEXPORT);
        return 1;
        }

    //------------------------------------------
    // open a file to save the exported shape to:
   FileWStream ost(filename);
   if( ost.getStatus() != STRM_OK )
      {
        Alert(IDS_TH_CANTCREATE);
        return(0);
        }

    //-----------------------
    // actually do the export:
    myScene.getNodes( ost );

    //---------------------------------------
    // close the file and report any problems:
    
    if( ost.getStatus() != STRM_OK )
      {
        Alert(IDS_TH_WRITEERROR);
        ost.close();
        remove(filename);
        return(0);
        }
    else
        ost.close();
    
    return 1;    
    }

#ifdef MAX_2
int _DTSExport::DoExport(const TCHAR *filename,ExpInterface *ei,Interface *gi, int supressUI) 
#else
int _DTSExport::DoExport(const TCHAR *filename,ExpInterface *ei,Interface *gi) 
#endif
{
    int status;
        
    status = _dts_save(filename, ei, gi, this);

    if(status == 0)
	{
        return 1;        // Dialog cancelled
	}

    if(status < 0)
	{
        return 0;        // Real, honest-to-goodness error
	}

    return(status);
}
