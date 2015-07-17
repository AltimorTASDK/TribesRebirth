// mcudoc.h : interface of the CMcuDoc class
//
/////////////////////////////////////////////////////////////////////////////
#include "detail.h"

class CDetListDlg;

class CMcuDoc : public CDocument
{
protected: // create from serialization only
	CMcuDoc();
	DECLARE_DYNCREATE(CMcuDoc)

// Attributes
public:
   DetailList *   pDL;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMcuDoc)
	public:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	void	Build();
	virtual ~CMcuDoc();
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CMcuDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
