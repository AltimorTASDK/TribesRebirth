#if !defined(AFX_LIGHTDIALOG_H__A64110D2_6104_11D1_8CE9_006097B90B28__INCLUDED_)
#define AFX_LIGHTDIALOG_H__A64110D2_6104_11D1_8CE9_006097B90B28__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// LightDialog.h : header file
//

#include "Light.h"
#include "threddoc.h"

class CMyDragListBox : public CDragListBox
{
	void Dropped( int Source, CPoint DropPoint );
};

class CMyColorButton : public CButton
{
   public: 
      void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
      void SetDialog( class CLightDialog * pLightDialog );
      
   private:
      class CLightDialog * m_lightDialog;
};

/////////////////////////////////////////////////////////////////////////////
// CLightDialog dialog

class CLightDialog : public CDialog
{
// Construction
public:
	CLightDialog(CWnd* pParent = NULL);   // standard constructor

	CTHREDDoc *		pDoc;
		
	// our light array
	CLightArray* 	mLightArray;
	CLightArray		mSavedLightArray;
	int mCurrentLight;
	int mCurrentState;

	int EditLight( CLightArray& Lights, int CurrentLight, CTHREDDoc* Doc );
	void FillDialogInfo( void );
	void FillLightInfo( void );
	void FillStateInfo( void );
	bool UpdateStateInfo( void );
   void SetSunValues( CLightState * pLightState, Point3F * pVector );

	// info strings
	CString m_PositionString;
	CString m_DurationString;
	CString m_ColorString;
	CString m_Distance1String;
	CString m_Distance2String;
	CString m_Distance3String;
	CString m_DirectionString;
	CString m_AngleString;
//	CString m_IDString;

// Dialog Data
	//{{AFX_DATA(CLightDialog)
	enum { IDD = IDD_LIGHT_DIALOG };
	CMyColorButton	m_colorButton;
	CSpinButtonCtrl	m_StateSpin;
	CEdit	m_PositionEdit;
	CEdit	m_DurationEdit;
	CEdit	m_Distance3Edit;
	CEdit	m_Distance2Edit;
	CEdit	m_Distance1Edit;
	CEdit	m_DirectionEdit;
	CEdit	m_AngleEdit;
	CListBox	m_StateList;
	CMyDragListBox	m_LightList;
	CEdit	m_NameEdit;
	CEdit	m_ColorEdit;
	//}}AFX_DATA



// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLightDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLightDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnLdColorpick();
	afx_msg void OnLdAddstate();
	afx_msg void OnLdAutocheck();
	afx_msg void OnLdCreatelight();
	afx_msg void OnLdLinearradio();
	afx_msg void OnLdLoopcheck();
	afx_msg void OnLdPointradio();
	afx_msg void OnLdQuadraticradio();
	afx_msg void OnLdRandomcheck();
	afx_msg void OnLdRemovestate();
	afx_msg void OnLdSpotradio();
	afx_msg void OnDeltaposLdStatespin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeLdLightlist();
	afx_msg void OnSelchangeIdStatelist();
	afx_msg void OnLdApply();
	afx_msg void OnChangeLdNameedit();
	afx_msg void OnLdDeletelight();
	afx_msg void OnChangeLdPositionedit();
	afx_msg void OnChangeLdAngleedit();
	afx_msg void OnChangeLdColoredit();
	afx_msg void OnChangeLdDirectionedit();
	afx_msg void OnChangeLdDistance1edit();
	afx_msg void OnChangeLdDistance2edit();
	afx_msg void OnChangeLdDistance3edit();
	afx_msg void OnChangeLdDurationedit();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnLdNoneradio();
	afx_msg void OnLdCopycurrent();
	afx_msg void OnLdLight();
	afx_msg void OnChangeLdId();
	afx_msg void OnLdManagebymissioneditor();
	afx_msg void OnLsSunedit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIGHTDIALOG_H__A64110D2_6104_11D1_8CE9_006097B90B28__INCLUDED_)
