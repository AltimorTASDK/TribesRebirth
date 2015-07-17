// THRED.cpp : Defines the class behaviors for the application.
//

/*============= PRIORITITHRED ==============DATE June 2, 1996


// Latest improvements
- Changed format of 3dt file to save out the brush id's in the brushes
and the polygon id's in the bsp tree
- made space bar be like a middle button last tool toggle







[ ] Texture mapping
[ ] optimization of ortho views


///////////////
There are lots of bugs in the brush/entity scheme right now and
I'm thinking I should possibly come up with a new data
representation for this stuff.  All of this liked list shit
combined with active brush this and that sucks.  Maybe a linked
list of brushes within the entity or something....


cannot select brushes which are attached to an entity
brushes attached to an entity don't build into bsp...
//////////////





[X]  instead of having a brush list, possibly do brush array and
[ ]  Allow use of entity array etc... but when doing stuff like
deleting etc go through member functions of the document.  This will
allow us to do fixups on the brushes that are attached to entities

call member function to iterate.  Pass flags to iterate through
only the types of brushes you want.  For instance 
GetNextBrush(BRUSH_ENTITYBRUSHES | BRUSH_ACTIVE ) etc.



[ ] Working on the file saving so that it doesn't parse
past it's current entity
[ ] Figure out a good system of associating brushes
with entities, etc so that when we build bsp's we have
a cohesive system of all this shit.


***************** [ ] Working on the file loading stuff.


[ ] Change the exporter.  Try to use the co-planarity
so that if one of those get's chosen as a splitter
we stick them in co-planar.  Then we only filter
down ONCE for that entire co-planar sucker... should
be pretty darn simple.  This may fix our problems with export.
[ ] Figure out what's causing that assert
 searchpath
[X] goto a 360 degree angle system????  Maybe just in
the dialogs?? Make a rotational snap system.
[X] Rotational snap system
[X] force the brush visible when you select a brush
[X] filling in groups names on load, 
[X] setting modified flag, 
[X] fixing dialog box order
[X] Re-write movement system so that the movement is only
applied to the brush when it's placed.  Then we can
check for shift holding, snap to grid etc.  This will make
it MUCHO cleaner.
(when you hold down shift it uses the major axis of movement.
This only works for rotation and for movement)
[X] change shear so that it only does 1 axis in each of
the ortho windows (except in top).  Shearing now easier...
[X] Put name of file in export dialog.


[ ] Make a cut/paste shit for groups work.
[ ] MAKE THE DAMN TEXTURE BROWSER WORK DAMMIT
[ ] allow moving around of entire groups??
[ ] When showing current groups ONLY show entities that are
withing the bounding boxes of the groups that we are showing.

  //************* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


  THINGS TO DO FOR BETA:
// currently working on getting more information
// into the brush dialog.
[X] Allow the user to create and select brush groups
and draw these groups correctly.
	- make the brush attributes dialog show more
	information about the brush such as it's current group
	-- show a list of the brush names attached to that group
	and allow editing of the brush through this method (getting
	at the attributes dialog for thebrush)
	-- make the button show the colour
	-- make a dialog bar which lists the groups, has
	a button for visible for the current group.
	-- have more buttons for show current group, show
	all, or show visible.  

[ ]	-- make shortcut keys to go to the next group
	so that groups can be quickly selected... have
	different keys.. for instance go to next group or
	go to next visible group.


[ ] Change coordinate system so that it looks
the same as quakes (that is when we move entities
around and save back translate then).  For now
leave everything else the same but maybe change
it later on....

[X] Redo the CONSTRUCTIVEBSP class and move some of
the math into other classes such as the node itself etc.
so that we can use these trees properly in seperate rendering
classes.  

[X]  Also make functions which can be used to transfer
from world to screen and back again easily and use
there where we draw stuff (possibly make these macros
or some kind of function pointer..)

[X] Get rid of mworld and move that stuff into the
document

[ ] Get a texture browser going, allow the user to select
multiple polygons and assign the current texture to them.


[X] Make exporter to .MAP format (maybe a loader too?) that
works correctly

[ ] Make the movement system SMOOTHER and have some sort
of sensitivity/speed option.  Maybe make the speen
you move in ortho views relevent to how zoomed in you are.
or make the zoom amount indpendent of window size

[ ] Get some kind of better representation of entities
so that they actually look like something and scale.  
[ ] Also allow selection of them.


[X] Get a text based .t3d file going that we can
expand upon forever.


=======================================




[X] Make a simple stack to save the order we did the brushes
in (this includes making a simpler brush representation to
save the brushes.. basically we don't want those dialogs
haning around).
[X] Make snap to grid system
[X] [X] Get dibsection stuff working for solid render
Make a brush movement undo.
[X] Make Orthogonal views
[X] (Make scaling work properly, TURN OFF POLYGONS
  THAT AREN'T SUPPOSED TO BE SEEN AS THEY ARE ALL DRAWING!!)
[X] Make a thing to show 4 predefined views (top/bottom/side/3d)
[X] Fix accelerator table to redo
[X] Put in a drawn on screen grid (in all views)
[X] In ORTHO views draw the brushes and not the geometry
[X] In ORTHO views draw different colours depending on the brush type.
[X] Add in shearing for brushes.
[X] Fix the cracks (what is the problem there!);
[X] Put reset button in the brush dialogs
[X] Make bsp rebuilder.... and fix the geom bugs
[X] Stick in prestepping so that we don't get crackeronnos
[X] Put in LIMITED brush filtering by quickly
building bsps for the brush that have polygons
which intersect with the polygon we are filtering
through.  Make a repository of CBspNode objects
that we can quickly build trees from.
[X] Make regular bsp building use the "FANCY" methods
we have developed for the rebuilding process
[X] Eliminate unneeded crap from contructivebsp.cpp
[X] Fancy up the rebuilding dialog.. Maybe try to
get the stat's right.. give them some feedback
when we are actually doing the processing.
[ ] Make a polygon intersection function and only
put the polygons that intersect in with the bsp when
we build it.. this should reduce even more splits.
[ ] in the brush attribues dialog put stuff like
the current size of the brush etc so people.. basically
just way more brush stats
[ ] Put in texturing.
	Texture Mapping observations:

	1) We want to define a vector for each side of the bitmap
	like in the radix editor.

	2) We need to interpolate the gradients when scanning down the
	left side of the poly, but when scanning down the right side
	we may just need to get the actual edges.  Then the texture
	mapper uses the vector to get the actual tmapping values.

	3) The inner loop should be in asm and interleave the divides
	so that they are (almost) for free.  We should probably
	do every 16 pixels like quake.

	4) for each node we have a brush number and polygon
	number.  We should also have a pointer to polygon
	texturing information, if it is null then we
	have to acquire that information.  If that
	information needs to be updated it should be done
	so automatically (such as calcing gradients,
	loading textures, making the texture from the lightmap, etc)
	
[ ] Put in lighting/mipmapping.
[X] make wireframe 3d view
[ ] Move the renderer into a seperate class, maybe just send
it polygons.
[ ] Fix that DAMN MOUSE CURSOR showing up all the time
[ ] Make undo system for add/subtract brush
[ ] Handle co-planar polys in bsp building so
they aren't on top of each other.
[ ] Make a custom dialog control which can put in different
bitmaps for us
[ ] Some camera reset functions. (maybe a dialog to or possibly
just a re-center... this should be a right click).
[ ] Make view menu hide/show the brush toolbar
[ ] Make a special sub-classed toolbar which can
accept right mouse clicks and possible have "sub" toolbar
[ ] Intersection and DeIntersection
[X] Make a render view that shows wireframe brushes
[ ] If a subtraction brush doesn't intersect with anything
in the world get rid of it.  Also possibly store
a subtraction by which brushes it cuts stuff off of.
==========


*/


#include "stdafx.h"
#include "THRED.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "THREDDoc.h"
#include "THREDView.h"

//#include "p_txcach.h" // For texture cache key

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define VERSION_HIGH 1
#define VERSION_LOW  95
                       
/////////////////////////////////////////////////////////////////////////////
// CTHREDApp

BEGIN_MESSAGE_MAP(CTHREDApp, CWinApp)
	//{{AFX_MSG_MAP(CTHREDApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTHREDApp construction
CTHREDApp::CTHREDApp() :
   m_planeDistancePrecision( 0.1 ),
   m_planeNormalPrecision( 0.0001 ),
   mExportRun( false ),
   mPathIsDML( false )
{
   // set the version - high/low
   m_version = ( VERSION_HIGH << 16 ) + VERSION_LOW;
   
   // version of file on open
   m_fileVersion = 0;
   
	// Place all significant initialization in InitInstance
    //m_resmgr.add(&m_docRvol);
//    m_resmgr.add(NULL, NULL);

    m_texturescale = 1.0;
    minintensity = 1.0;    

    hourglass_cursor = LoadStandardCursor(IDC_WAIT);
    arrow_cursor = LoadStandardCursor(IDC_ARROW);

    mAllowOpen = TRUE;
    mAllowNew  = TRUE;
    multiple_requests = FALSE;

    lightval = 0xF;

    long  brk;

    brk = 44;
    //_CrtSetBreakAlloc(brk);
}

CTHREDApp::~CTHREDApp()
{
//    m_resmgr.remove(&m_docRvol);
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CTHREDApp object

CTHREDApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CTHREDApp initialization

BOOL CTHREDApp::InitInstance()
{
//   int tmp_flag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
//   tmp_flag |= _CRTDBG_CHECK_ALWAYS_DF;
//   _CrtSetDbgFlag( tmp_flag );
   
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_THREDTYPE,
		RUNTIME_CLASS(CTHREDDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CTHREDView));
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CTHREDCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// The main window has been initialized, so show and update it.
   pMainFrame->ShowWindow(m_nCmdShow);
   pMainFrame->UpdateWindow();

   RegisterHotKey( pMainFrame->m_hWnd, 0x0000, MOD_CONTROL, VK_TAB );

   // ok.. run without doing any grafx stuff...
   if( mExportRun )
   {
      cmdInfo.m_nShellCommand = CTHREDCommandLineInfo::FileOpen;
      cmdInfo.m_strFileName = mOpenFileName;
   }
   else
      cmdInfo.m_nShellCommand = CTHREDCommandLineInfo::FileNothing;
   
	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

   // lets leave while the gettin's good
   if( mExportRun )
   {
      CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
      if( pFrame )
      {
         CTHREDDoc * pDoc = ( CTHREDDoc * )pFrame->MDIGetActive()->GetActiveDocument();
         
         // check if we should convert the file
         if( m_fileVersion < makeVersion( 1, 80 ) )
            pDoc->OnSaveDocument( mOpenFileName );
         
         pDoc->OnCloseDocument();
      }
   
      PostQuitMessage( 0 );
   }
      
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
   
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTHREDApp commands

BOOL CTHREDApp::PreTranslateMessage(MSG* pMsg)
{
	return CWinApp::PreTranslateMessage(pMsg);
}



// App command to run the dialog
void CTHREDApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	// something went wrong with the dialog... exit the app
	if(aboutDlg.DoModal() != IDOK ) {
		ASSERT(AfxGetApp()->m_pMainWnd != NULL);
		AfxGetApp()->m_pMainWnd->SendMessage(WM_CLOSE);
	}


}

BOOL CAboutDlg::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
   return CDialog::Create(IDD, pParentWnd);
}

BOOL CAboutDlg::OnInitDialog() 
{
   CString  Version;
   CTHREDApp * App = ( CTHREDApp * )AfxGetApp();
   
   Version.Format( "Thred Version %d.%d\nDate: %s\nTime: %s", App->m_version >> 16, 
      App->m_version & 0x0000ffff, __DATE__, __TIME__ );
   ( ( CWnd * )GetDlgItem( IDC_ABOUT_VERSION ) )->SetWindowText( Version );
   return( TRUE );
}

CTHREDCommandLineInfo::CTHREDCommandLineInfo() :
   parseExport( false ),
   exportParam( 0 )
{
}

void CTHREDCommandLineInfo::ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast )
{
   // check if this is going to be an export run
   if( bFlag && lpszParam[0] == 'x' )
   {
      CTHREDApp * App = ( CTHREDApp * )AfxGetApp();
      App->mExportRun = true;
      
      parseExport = true;
      exportParam = 0;
      
      return;
   }
   
   // check if we are parsing for an export
   if( !bFlag && parseExport )
   {
      CTHREDApp * App = ( CTHREDApp * )AfxGetApp();
      if( exportParam == 0 )
      {
         App->mOpenFileName = lpszParam;
      }
      else if( exportParam == 1 )
      {
         App->mSearchPath = lpszParam;
         
         // do a test to see if '.dml' is at the end of path, 
         // if so then set flag to indicate a file instead of
         // a path
         if( strlen( lpszParam ) > 4 )
            if( !stricmp( lpszParam + strlen( lpszParam ) - 4, ".dml" ) )
               App->mPathIsDML = true;
      }
      exportParam++;
      return;
   }
   
   // send to default implementation
   CCommandLineInfo::ParseParam( lpszParam, bFlag, bLast );
}