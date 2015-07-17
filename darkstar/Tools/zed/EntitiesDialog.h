// EntitiesDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEntitiesDialog dialog
#ifndef _ENTITIES_DIALOG_H_
#define _ENTITIES_DIALOG_H_

#include "Entity.h"
#include "threddoc.h"

class CEntitiesDialog : public CDialog
{
// Construction
public:
	CTHREDDoc *pDoc;
	void NewEntity(char** ClassnameList);
	void FillInKeyValuePairs(int Selection = 0);
	void UnFillInDialog();
	void FillInDialog();
	int DoDialog(CEntityArray& Entities, CTHREDDoc* Doc);
	int EditEntity(CEntityArray& Entities, int CurrentEntity, CTHREDDoc* Doc);
	CEntitiesDialog(CWnd* pParent = NULL);   // standard constructor

    double  min_intensity;

// Dialog Data
	//{{AFX_DATA(CEntitiesDialog)
	enum { IDD = IDD_ENTITIES };
	CEdit	m_ValueEdit;
	CEdit	m_KeyEdit;
	CListBox	m_Pairs;
	CListBox	m_EntityList;
	BOOL	m_ShowLights;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEntitiesDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEntitiesDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnNewentity();
	afx_msg void OnNewkeyvalue();
	afx_msg void OnNewlight();
	afx_msg void OnSelchangePairs();
	afx_msg void OnKillfocusKeyEdit();
	afx_msg void OnKillfocusValueEdit();
	afx_msg void OnSelchangeEntitylist();
	afx_msg void OnDeleteentity();
	afx_msg void OnDeletekeyvalue();
	afx_msg void OnAssociateSelectedbrush();
	afx_msg void OnAssociategroup();
	afx_msg void OnDisassociate();
	afx_msg void OnChangeMinintensity();
	virtual void OnOK();
	afx_msg void OnColorbutton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// our entity array
	CEntityArray* mEntityArray;
	int mCurrentEntity;
	int mCurrentKey;
};


#endif
