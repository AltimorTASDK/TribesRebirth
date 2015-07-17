// THRED.h : main header file for the THRED application
//
#ifndef _INC_THRED
#define _INC_THRED

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "types.h"
#include "streams.h"
#include "resource.h"       // main symbols
#include "ResManager.h"

/////////////////////////////////////////////////////////////////////////////
// CTHREDApp:
// See THRED.cpp for the implementation of this class
//
// Define our standard template so we can use these other places
#define TEMPLATE_RESOURCE_ID			IDR_THREDTYPE
#define TEMPLATE_DOC_CLASS				RUNTIME_CLASS(CTHREDDoc )
#define TEMPLATE_CHILD_FRAME_CLASS		RUNTIME_CLASS(CChildFrame )
#define TEMPLATE_VIEW_CLASS				RUNTIME_CLASS(CTHREDView )

inline int makeVersion( int major, int minor )
{
   return( ( major << 16 ) | ( minor & 0xffff ) );
}

class CTHREDCommandLineInfo : public CCommandLineInfo
{
   public:
      CTHREDCommandLineInfo();
      void ParseParam( LPCTSTR, BOOL, BOOL );
      bool parseExport;
      int exportParam;
};

class CTHREDApp : public CWinApp
{
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CTHREDApp();
    ~CTHREDApp();

    HCURSOR hourglass_cursor;
    HCURSOR arrow_cursor;

    bool    mAllowOpen;
    bool    mAllowNew;
    bool    multiple_requests;

    int     lightval;   // Global value for lighting (shift left 12 bits & and with 0xFFFF)

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTHREDApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTHREDApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
    ResourceManager     m_resmgr;
    VolumeRStream       m_docRvol;
    double              m_texturescale;
    double              minintensity;
	UInt32				m_version;           // this version of zed
   UInt32            m_fileVersion;       // version of file on open
   double            m_planeDistancePrecision;  // distance from a plane to classify a point
   double            m_planeNormalPrecision; // normal precision for a plane
   
   // info for the batch running of zed
   CString              mOpenFileName;
   CString              mSearchPath;
   bool                 mExportRun;
   bool                 mPathIsDML;
};

extern CTHREDApp theApp;
/////////////////////////////////////////////////////////////////////////////

#endif