// GlobalEnvironment.cpp : implementation file
//

#include "stdafx.h"
#include "THRED.h"
#include "GlobalEnvironment.h"
#include "globalsettingsdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// This global variable contains all of the data
// that we want to be globally acessible.  Such
// as which axis's are currently available etc
CGlobalEnvironment Globals;

/////////////////////////////////////////////////////////////////////////////
// CGlobalEnvironment
CGlobalEnvironment::CGlobalEnvironment()
{
   mLockTexturePositions = FALSE;
	mWadPath = "MYWAD.WAD";
	mPalettePath = "PALETTE.LMP";
}


CString& CGlobalEnvironment::GetWadPath()
{
	return mWadPath;
}

CString& CGlobalEnvironment::GetPalettePath()
{
	return mPalettePath;
}

// bring up the global environment dialog
void CGlobalEnvironment::DoDialog()
{
	CGlobalSettingsDialog dlg;

	// setup the dialog
	dlg.m_WadPath = mWadPath;
	dlg.m_PalettePath = mPalettePath;

	// do the dialog
	if(dlg.DoModal() == IDOK ) {
		mWadPath = dlg.m_WadPath;
		mPalettePath = dlg.m_PalettePath;
	}
}


