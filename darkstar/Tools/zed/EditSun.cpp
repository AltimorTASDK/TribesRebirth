// EditSun.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "EditSun.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// EditSun dialog


EditSun::EditSun(CWnd* pParent /*=NULL*/)
	: CDialog(EditSun::IDD, pParent)
{
	//{{AFX_DATA_INIT(EditSun)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void EditSun::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EditSun)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(EditSun, CDialog)
	//{{AFX_MSG_MAP(EditSun)
	ON_EN_CHANGE(IDC_EDIT, OnChangeEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EditSun message handlers

void EditSun::OnChangeEdit() 
{
   CString string;
   
   GetDlgItem(IDC_EDIT)->GetWindowText( string );
   
   Vector.x = 0.0;
   Vector.y = 0.0;
   Vector.z = 0.0;
   sscanf( LPCTSTR( string ), "%f %f %f", &Vector.x, &Vector.y, &Vector.z );
}

void EditSun::OnOK() 
{
	CDialog::OnOK();
}
