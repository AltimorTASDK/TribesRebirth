// mcu.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"

#include "mainfrm.h"
#include "detailed.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

class CMcuCmdLineInfo : public CCommandLineInfo
{
public:
	CMcuCmdLineInfo();
	int	fBatch;
	void ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast );
};

CMcuCmdLineInfo::CMcuCmdLineInfo()
{
	fBatch = 0;
}

void CMcuCmdLineInfo::ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast )
{
	if( bFlag && !stricmp( lpszParam, "b" ) )
		fBatch = 1;
	else
		CCommandLineInfo::ParseParam( lpszParam, bFlag, bLast );
}

/////////////////////////////////////////////////////////////////////////////
// CMcuApp

BEGIN_MESSAGE_MAP(CMcuApp, CWinApp)
	//{{AFX_MSG_MAP(CMcuApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_USE3DSPATHS, OnFileUse3dspaths)
	ON_UPDATE_COMMAND_UI(ID_FILE_USE3DSPATHS, OnUpdateFileUse3dspaths)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMcuApp construction

CMcuApp::CMcuApp()
{
   DLInit();
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMcuApp object

CMcuApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMcuApp initialization

BOOL CMcuApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

   //m_pszHelpFilePath = "MCU.HLP";

	Enable3dControls();

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)
	fUsePaths = GetProfileInt( "MCU", "UsePaths", 0 );

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CMcuDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CDetailEditView));
	AddDocTemplate(pDocTemplate);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes();

	// simple command line parsing
	CMcuCmdLineInfo cli;
	ParseCommandLine( cli );
	fBatch = cli.fBatch;
	ProcessShellCommand( cli );

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

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

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
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
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CMcuApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CMcuApp commands

void CMcuApp::WinHelp(DWORD dwData, UINT nCmd) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	CWinApp::WinHelp(dwData, nCmd);
}

void CMcuApp::OnFileUse3dspaths() 
{
	fUsePaths = !fUsePaths;
	WriteProfileInt( "MCU", "UsePaths", fUsePaths );
}

void CMcuApp::OnUpdateFileUse3dspaths(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( fUsePaths );	
}

BOOL CMcuApp::OnIdle(LONG lCount) 
{
	if( fBatch == 1 )
		{
		fBatch++;
		m_pMainWnd->PostMessage( WM_COMMAND, MAKELONG(IDC_BUILD,BN_CLICKED) );
		}
	else if( fBatch == 3 )
		m_pMainWnd->DestroyWindow();
	
	return CWinApp::OnIdle(lCount);
}
