// detailed.cpp : implementation file
//

#include "stdafx.h"
#include "detailed.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////

#include <stdarg.h>

static CDetailEditView * gpDetailEditView = 0;

void __cdecl StatusPrintf( char *format, ... )
{
   if( gpDetailEditView )
      {
      char s[200];
      va_list argptr;
      va_start( argptr, format );
      vsprintf( s, format, argptr);
      va_end( argptr );
      gpDetailEditView->AddStatusText( s );
      }
}

void StatusClear()
{
   if( gpDetailEditView )
      gpDetailEditView->ClearStatus();
}

/////////////////////////////////////////////////////////////////////////////
// CDetailEditView

IMPLEMENT_DYNCREATE(CDetailEditView, CFormView)

CDetailEditView::CDetailEditView()
	: CFormView(CDetailEditView::IDD)
{
	//{{AFX_DATA_INIT(CDetailEditView)
	//}}AFX_DATA_INIT
   gpDetailEditView = this;
}

CDetailEditView::~CDetailEditView()
{
   gpDetailEditView = 0;
}

void CDetailEditView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDetailEditView)
	DDX_Control(pDX, IDC_MERGE_POINT, m_MergePoint);
	DDX_Control(pDX, IDC_MERGE_NORMAL, m_MergeNormal);
	DDX_Control(pDX, IDC_BSP, m_bsp);
	DDX_Control(pDX, IDC_ALPHA_COLORS, m_alphaColors);
	DDX_Control(pDX, IDC_BUILD, m_build);
	DDX_Control(pDX, IDSORT, m_sort);
	DDX_Control(pDX, IDC_ALPHA_LEVELS, m_alphaLevels);
	DDX_Control(pDX, IDC_INSERT, m_insert);
	DDX_Control(pDX, IDC_DELETE, m_delete);
	DDX_Control(pDX, IDC_STATUS, m_status);
	DDX_Control(pDX, IDC_SHRINK, m_shrink);
	DDX_Control(pDX, IDC_SCALE, m_scale);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Control(pDX, IDC_DET_SIZE, m_size);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDetailEditView, CFormView)
	//{{AFX_MSG_MAP(CDetailEditView)
	ON_BN_CLICKED(IDC_BUILD, OnBuild)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_INSERT, OnInsert)
	ON_BN_CLICKED(IDSORT, OnSort)
	ON_EN_CHANGE(IDC_SCALE, OnChangeScale)
	ON_EN_CHANGE(IDC_DET_SIZE, OnChangeDetSize)
	ON_EN_CHANGE(IDC_SHRINK, OnChangeShrink)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelchangeList)
	ON_EN_CHANGE(IDC_ALPHA_COLORS, OnChangeAlphaColors)
	ON_EN_CHANGE(IDC_ALPHA_LEVELS, OnChangeAlphaLevels)
	ON_BN_CLICKED(IDC_BSP, OnBsp)
	ON_EN_CHANGE(IDC_MERGE_POINT, OnChangeMergePoint)
	ON_EN_CHANGE(IDC_MERGE_NORMAL, OnChangeMergeNormal)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CDetailEditView diagnostics

#ifdef _DEBUG
void CDetailEditView::AssertValid() const
{
	CFormView::AssertValid();
}

void CDetailEditView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CMcuDoc* CDetailEditView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMcuDoc)));
	return (CMcuDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDetailEditView message handlers

void CDetailEditView::OnBuild() 
{
 	if( !GetDocument()->IsModified() || 
 	    GetDocument()->DoFileSave() )
   	pDL->BuildShape();
	if( ((CMcuApp*)AfxGetApp())->fBatch == 2 )
		((CMcuApp*)AfxGetApp())->fBatch++;
}

void CDetailEditView::OnDelete() 
{
	int delIndex = m_list.GetCurSel();
	if( delIndex != LB_ERR && delIndex < pDL->GetCount() )
		{
		pDL->DeleteDetail( delIndex );
      m_list.DeleteString( delIndex );
		if( delIndex == pDL->GetCount() )
			delIndex--;
      if( delIndex < 0 )
         delIndex = 0;
		m_list.SetCurSel( delIndex );
		UpdateEditControls( delIndex );
		}
}

void CDetailEditView::OnInsert() 
{
	CFileDialog fd( TRUE, ".3ds", NULL, 0, 
		TEXT("3D Studio Files|*.3ds|3Space Files|*.dts|All Files|*.*||"), 
		this );	
	if( fd.DoModal() == IDOK )
		{
		Detail d;
		strcpy( d.fName, fd.GetPathName() );
		int addIndex = m_list.GetCurSel();
		pDL->InsertDetail( addIndex, d );
      m_list.InsertString( addIndex, d.fName );
		UpdateListItem( addIndex );
		UpdateEditControls( addIndex );
		}
}

void CDetailEditView::OnSort() 
{
	if( pDL->SortDetails() )
		OnUpdateView( 0 );
}

void CDetailEditView::AddStatusText(char *s)
{
   CString line(s);
   line += "\r\n";
	int start,end;
	m_status.SetSel(0,-1);
	m_status.GetSel(start,end);
	m_status.SetSel(end,end);
	m_status.ReplaceSel(line);
}

void CDetailEditView::ClearStatus()
{
	m_status.SetWindowText("");
	m_status.UpdateWindow();
}

char * CDetailEditView::FormatDetail( int selIndex, char *s )
{
	char *filename = strrchr( (*pDL)[selIndex].fName, '\\' );
	if( !filename )
		filename = (*pDL)[selIndex].fName;
	else
		filename++;
	sprintf( s, "%-12s %4.2f %4.2f %4.2f %5s", filename,
		(*pDL)[selIndex].fScale,
		(*pDL)[selIndex].fShrink,
		(*pDL)[selIndex].fDetailSize,
      (*pDL)[selIndex].fSort ? "+sort" : "-sort"
		 );
	return s;
}

void CDetailEditView::UpdateListItem( int selIndex )
{
	AssertFatal( selIndex >= 0, "Bad!" );
	char s[100];
	// delete existing version if any:
	if( selIndex < m_list.GetCount() - 1 )
		m_list.DeleteString( selIndex );
	// insert new version:
	m_list.InsertString( selIndex, FormatDetail( selIndex, s ) );
	// make sure new version is selected:
	m_list.SetCurSel( selIndex );
}

void CDetailEditView::UpdateList( int selIndex )
{
	char s[200];
	m_list.ResetContent();
	for( int i = 0; i < pDL->GetCount(); i++ )
		m_list.AddString( FormatDetail( i, s ) );
	m_list.AddString("");
	AssertFatal( selIndex >= 0, "Bad!" );
	m_list.SetCurSel( selIndex );
}

void CDetailEditView::UpdateEditControls( int selIndex )
{
	AssertFatal( selIndex >= 0, "Bad!" );
	char s[100];
	if( selIndex < pDL->GetCount() )
		{
		m_delete.EnableWindow(TRUE);
		m_shrink.EnableWindow(TRUE);
		m_scale.EnableWindow(TRUE);
		m_size.EnableWindow(TRUE);
		m_bsp.EnableWindow(TRUE);
		sprintf( s, "%4.2f", (*pDL)[selIndex].fShrink );
		m_shrink.SetWindowText( s );
		sprintf( s, "%4.2f", (*pDL)[selIndex].fScale );
		m_scale.SetWindowText( s );
		sprintf( s, "%4.2f", (*pDL)[selIndex].fDetailSize );
		m_size.SetWindowText(s);
      m_bsp.SetCheck( (*pDL)[selIndex].fSort );
		}
	else
		{
		m_delete.EnableWindow(FALSE);
		m_shrink.EnableWindow(FALSE);
		m_scale.EnableWindow(FALSE);
		m_size.EnableWindow(FALSE);
		m_bsp.EnableWindow(FALSE);
		m_shrink.SetWindowText("");
		m_scale.SetWindowText("");
		m_size.SetWindowText("");
      m_bsp.SetCheck( 0 );
		}
}

void CDetailEditView::OnUpdateView( int selIndex )
{
   if( pDL )
      {
	   UpdateList( selIndex );
	   UpdateEditControls( selIndex );
      char s[100];
      sprintf( s, "%d", pDL->fAlphaNumColors );
      m_alphaColors.SetWindowText( s );
      sprintf( s, "%d", pDL->fAlphaNumLevels );
      m_alphaLevels.SetWindowText( s );
      sprintf( s, "%4.2f", pDL->fMergePointTolerance );
      m_MergePoint.SetWindowText( s );
      sprintf( s, "%4.2f", pDL->fMergeNormalTolerance );
      m_MergeNormal.SetWindowText( s );
      }
}

void CDetailEditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
   pDL = GetDocument()->pDL;
   OnUpdateView( 0 );
}



void CDetailEditView::OnInitialUpdate() 
{
   CFormView::OnInitialUpdate();
   GetParentFrame()->RecalcLayout();
   ResizeParentToFit(FALSE/*FALSE*/); // default argument is TRUE
   ResizeParentToFit(TRUE/*FALSE*/); // default argument is TRUE
}

void CDetailEditView::OnChangeScale() 
{
	int selIndex = m_list.GetCurSel();
	AssertFatal( selIndex >= 0, "Bad!" );
	char s[100];
	RealF scale;
	m_scale.GetWindowText(s,sizeof(s));
	if( 1 == sscanf(s,"%f",&(scale)) && scale <= 1000.0 && scale >= 0.001 )
		{
      if( scale != (*pDL)[selIndex].fScale )
         {
   	   GetDocument()->SetModifiedFlag();
		   (*pDL)[selIndex].fScale = scale;
		   UpdateListItem( selIndex );
         }
		}
}

void CDetailEditView::OnChangeShrink() 
{
	int selIndex = m_list.GetCurSel();
	AssertFatal( selIndex >= 0, "Bad!" );
	char s[100];
	RealF shrink;
	m_shrink.GetWindowText(s,sizeof(s));
	if( 1 == sscanf(s,"%f",&(shrink)) && shrink <= 1.0 && shrink >= 0.50 )
		{
      if( shrink != (*pDL)[selIndex].fShrink )
         {
   	   GetDocument()->SetModifiedFlag();
		   (*pDL)[selIndex].fShrink = shrink;
		   UpdateListItem( selIndex );
         }
		}
}

void CDetailEditView::OnChangeDetSize() 
{
	int selIndex = m_list.GetCurSel();
	AssertFatal( selIndex >= 0, "Bad!" );
	char s[100];
	RealF size;
	m_size.GetWindowText(s,sizeof(s));
	if( 1 == sscanf(s,"%f",&(size)) && size <= 1000.0 && size >= 1.0 )
		{
      if( size != (*pDL)[selIndex].fDetailSize )
         {
   	   GetDocument()->SetModifiedFlag();
		   (*pDL)[selIndex].fDetailSize = size;
		   UpdateListItem( selIndex );
         }
		}
}

void CDetailEditView::OnSelchangeList() 
{
	int selIndex = m_list.GetCurSel();
	AssertFatal( selIndex >= 0, "Bad!" );
	UpdateEditControls( selIndex );
}

void CDetailEditView::OnChangeAlphaColors() 
{
	char s[100];
	int ncolors;
	m_alphaColors.GetWindowText(s,sizeof(s));
	if( 1 == sscanf(s,"%d",&(ncolors)) && ncolors <= 128 && ncolors >= 1 )
   	{
      if( ncolors != pDL->fAlphaNumColors )
         {
   	   GetDocument()->SetModifiedFlag();
		   pDL->fAlphaNumColors = ncolors;
         }
      }
}

void CDetailEditView::OnChangeAlphaLevels() 
{
	char s[100];
	int nlevels;
	m_alphaLevels.GetWindowText(s,sizeof(s));
	if( 1 == sscanf(s,"%d",&(nlevels)) && nlevels <= 32 && nlevels >= 1 )
   	{
      if( nlevels != pDL->fAlphaNumLevels )
         {
   	   GetDocument()->SetModifiedFlag();
		   pDL->fAlphaNumLevels = nlevels;
         }
      }
}

void CDetailEditView::OnBsp() 
{
   int selIndex = m_list.GetCurSel();
   if( m_bsp.GetCheck() != (*pDL)[selIndex].fSort )
      {
      GetDocument()->SetModifiedFlag();
      (*pDL)[selIndex].fSort = m_bsp.GetCheck();
      UpdateListItem( selIndex );
      }
}

void CDetailEditView::OnChangeMergePoint() 
{
	char s[100];
	RealF tolerance;
	m_MergePoint.GetWindowText(s,sizeof(s));
	if( 1 == sscanf(s,"%f",&(tolerance)) )
   	{
      if( tolerance != pDL->fMergePointTolerance )
         {
   	   GetDocument()->SetModifiedFlag();
		   pDL->fMergePointTolerance = tolerance;
         }
      }
}

void CDetailEditView::OnChangeMergeNormal() 
{
	char s[100];
	RealF tolerance;
	m_MergeNormal.GetWindowText(s,sizeof(s));
	if( 1 == sscanf(s,"%f",&(tolerance)) )
   	{
      if( tolerance != pDL->fMergeNormalTolerance )
         {
   	   GetDocument()->SetModifiedFlag();
		   pDL->fMergeNormalTolerance = tolerance;
         }
      }
}
