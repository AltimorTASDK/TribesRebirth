// ChangeLighting.cpp : implementation file
//

#include "stdafx.h"
#include "matilda2.h"
#include "ChangeLighting.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChangeLighting dialog


CChangeLighting::CChangeLighting(CWnd* pParent /*=NULL*/)
	: CDialog(CChangeLighting::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChangeLighting)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CChangeLighting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChangeLighting)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChangeLighting, CDialog)
	//{{AFX_MSG_MAP(CChangeLighting)
	ON_BN_CLICKED(IDC_FLAT, OnFlat)
	ON_BN_CLICKED(IDC_SMOOTH, OnSmooth)
	ON_BN_CLICKED(IDC_UNLIT, OnUnlit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChangeLighting message handlers

int CChangeLighting::DoModal(TS::Material::ShadingType *out_lightMode) 
{
	// TODO: Add your specialized code here and/or call the base class
	

	int returnVal = CDialog::DoModal();

	*out_lightMode = m_lightMode;

	return returnVal;
}

void CChangeLighting::OnFlat() 
{
	m_lightMode = TS::Material::ShadingFlat;
}

void CChangeLighting::OnSmooth() 
{
	m_lightMode = TS::Material::ShadingSmooth;
}

void CChangeLighting::OnUnlit() 
{
	m_lightMode = TS::Material::ShadingNone;
}
