// LightPropertySheet.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "LightPropertySheet.h"
#include "light.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLightPropertySheet

IMPLEMENT_DYNAMIC(CLightPropertySheet, CPropertySheet)

CLightPropertySheet::CLightPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CLightPropertySheet::CLightPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CLightPropertySheet::~CLightPropertySheet()
{
}


BEGIN_MESSAGE_MAP(CLightPropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CLightPropertySheet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLightPropertySheet message handlers

BOOL CLightPropertySheet::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();	
   
   CRect rectWnd;
	GetWindowRect(rectWnd);	
   
   SetWindowPos( NULL, 0, 0, rectWnd.Width(),
		rectWnd.Height() + 30, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
      
   // create the button
	m_apply.CreateEx( WS_EX_CLIENTEDGE, _T("BUTTON"), _T("Apply"),
      WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
      rectWnd.Width() - 100, rectWnd.Height() - 24, 80, 26, m_hWnd, ( HMENU )APPLY_ID, 0 );	
	CenterWindow();	
   
   return bResult;
}

BOOL CLightPropertySheet::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if( wParam == APPLY_ID )
   {
      if( CItemBase::currentInspectObj )
      {
         // do the data exchange and then tell the object to 
         // grab it's info...
         CPropertyPage * page = CItemBase::currentInspectObj->getPropPage();
         page->UpdateData();
         if( !CItemBase::currentInspectObj->onInspect( false ) )
            MessageBox( "Could not write object data.  Make sure all fields are correct.", "Error", MB_OK );
         
         // now put the data back in...
         CItemBase::currentInspectObj->onInspect( true );
         page->UpdateData( false );
      }
   }
   
	return CPropertySheet::OnCommand(wParam, lParam);
}
