// matilda2.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "matilda2.h"
#include "matilda2Dlg.h"

#include "ts_init.h"
#include "g_surfac.h"

#include "palHelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMatilda2App

BEGIN_MESSAGE_MAP(CMatilda2App, CWinApp)
	//{{AFX_MSG_MAP(CMatilda2App)
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_EXPORTTOTEXT, OnFileExportToText)
	ON_COMMAND(ID_FILE_IMPORTFROMTEXT, OnFileImportFromText)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_FILE_SAVEAS, OnFileSaveas)
	ON_COMMAND(ID_PALETTE_SETCURRENTPAL, OnPaletteSetcurrentpal)
	ON_COMMAND(ID_EDIT_PASTETOEND, OnEditPastetoend)
	ON_COMMAND(IDM_CHANGELIGHTINGFLAGS, OnChangelightingflags)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMatilda2App construction

CMatilda2App::CMatilda2App()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMatilda2App object

CMatilda2App theApp;

/////////////////////////////////////////////////////////////////////////////
// CMatilda2App initialization

BOOL CMatilda2App::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Initialize TS libs...
	//
	::TSInit();

	// Create the rasters for GFX
	//
	GFXSurface::createRasterList(3000);

	// Create the mapping palette
	//
	initMapPalette();

	CMatilda2Dlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.

	// First delete the raster list...
	//
	GFXSurface::disposeRasterList();

	return FALSE;
}

void CMatilda2App::OnFileExit() 
{
	// TODO: Add your command handler code here
	((CMatilda2Dlg *)m_pMainWnd)->OnFileExit();
}

void CMatilda2App::OnFileNew() 
{
	((CMatilda2Dlg *)m_pMainWnd)->OnFileNew();
}

void CMatilda2App::OnFileSave() 
{
	((CMatilda2Dlg *)m_pMainWnd)->OnFileSave();
}

void CMatilda2App::OnFileSaveas() 
{
	((CMatilda2Dlg *)m_pMainWnd)->OnFileSaveas();
}

void CMatilda2App::OnFileOpen() 
{
	((CMatilda2Dlg *)m_pMainWnd)->OnFileOpen();	
}

void CMatilda2App::OnFileExportToText() 
{
	((CMatilda2Dlg *)m_pMainWnd)->OnFileExportToText();	
}

void CMatilda2App::OnFileImportFromText() 
{
	((CMatilda2Dlg *)m_pMainWnd)->OnFileImportFromText();	
}

void CMatilda2App::OnEditCopy() 
{
	((CMatilda2Dlg *)m_pMainWnd)->OnEditCopy();
}

void CMatilda2App::OnEditPaste() 
{
	((CMatilda2Dlg *)m_pMainWnd)->OnEditPaste();	
}


void CMatilda2App::OnPaletteSetcurrentpal() 
{
	((CMatilda2Dlg *)m_pMainWnd)->OnPaletteSetcurrentpalette();
}

void CMatilda2App::OnEditPastetoend() 
{
	((CMatilda2Dlg *)m_pMainWnd)->OnEditPastetoend();
}

void CMatilda2App::OnChangelightingflags() 
{
	((CMatilda2Dlg *)m_pMainWnd)->OnChangelightingflags();
}
