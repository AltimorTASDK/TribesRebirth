// mcudoc.cpp : implementation of the CMcuDoc class
//

#include "stdafx.h"

#include "mcudoc.h"
#include "filstrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMcuDoc

IMPLEMENT_DYNCREATE(CMcuDoc, CDocument)

BEGIN_MESSAGE_MAP(CMcuDoc, CDocument)
	//{{AFX_MSG_MAP(CMcuDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMcuDoc construction/destruction

CMcuDoc::CMcuDoc()
{
   pDL = new DetailList();
   pDL->fpDoc = this;
}

CMcuDoc::~CMcuDoc()
{
   delete pDL;
}

BOOL CMcuDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	pDL->Reset();
	return TRUE;
}

void CMcuDoc::Build()
{
 	if( !IsModified() || 
 	    DoFileSave() )
   pDL->BuildShape();
}

/////////////////////////////////////////////////////////////////////////////
// CMcuDoc serialization

void CMcuDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
		{
		FileWStream ost( (HANDLE)(ar.GetFile()->m_hFile) );
		pDL->writeItem( ost );
		}
	else
		{
      FileRStream ist( (HANDLE)(ar.GetFile()->m_hFile) );
		if( ist.getStatus() == STRM_OK )
			{
			ClassHeader ch;
			if( ClassItem::peekItem(ist, &ch) )
				{
				if( ch.type != CLS_DetailList &&
                ch.type != CLS_DetailListOld )
					MessageBox( NULL, "Not a DetailList", "Error!", MB_OK|MB_ICONERROR );
				else
					{
					delete pDL; 
					pDL = (DetailList*)ClassItem::loadItem( ist );
					pDL->fpDoc = this;
					}
				}
			}
      }
}

/////////////////////////////////////////////////////////////////////////////
// CMcuDoc diagnostics

#ifdef _DEBUG
void CMcuDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMcuDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMcuDoc commands

