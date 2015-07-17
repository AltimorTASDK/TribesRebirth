#if !defined(AFX_LIGHTSTATEPROP_H__02ACCA54_23ED_11D2_8D61_006097B90B28__INCLUDED_)
#define AFX_LIGHTSTATEPROP_H__02ACCA54_23ED_11D2_8D61_006097B90B28__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// LightStateProp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLightStateProp dialog

class CLightStateProp;

class CMyColorButton : public CButton
{
   public: 
      void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
      void SetDialog( class CLightStateProp * pDialog );
      
   private:
      class CLightStateProp * m_dialog;
};

class CLightStateProp : public CPropertyPage
{
	DECLARE_DYNCREATE(CLightStateProp)

// Construction
public:
	CLightStateProp();
	~CLightStateProp();

// Dialog Data
	//{{AFX_DATA(CLightStateProp)
	enum { IDD = IDD_LIGHTSTATEPROP };
   CMyColorButton m_colorButton;
	float	m_duration;
	CString	m_colorString;
	//}}AFX_DATA

   ColorF m_color;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CLightStateProp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CLightStateProp)
	afx_msg void OnLdColorpick();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeLdColoredit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIGHTSTATEPROP_H__02ACCA54_23ED_11D2_8D61_006097B90B28__INCLUDED_)
