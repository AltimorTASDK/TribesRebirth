#if !defined(AFX_MATERIALPROPERTIES_H__3CE88FF3_941E_11D1_8D13_006097B90B28__INCLUDED_)
#define AFX_MATERIALPROPERTIES_H__3CE88FF3_941E_11D1_8D13_006097B90B28__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MaterialProperties.h : header file
//

#include "matcanvs.h"
#include "g_pal.h"
#include "Thredparser.h"
#include "itrbasiclighting.h"

//// just define these for now
//struct MaterialProp {
//   bool  modified;
//   enum {
//      flatColor,
//      flatEmission,
//   } emissionType;
//
//   // had to change color
//   ColorF color;
//   float intensity;
//
//   bool  modifiedPhong;
//   float phongExp;
//   
//   GFXBitmap *pBitmap;
//   GFXBitmap *pEmissionMask;
//};
//typedef Vector<MaterialProp> MaterialPropList;

/////////////////////////////////////////////////////////////////////////////
// CMaterialProperties dialog

class CColorButton : public CButton
{
   public: 
      void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
      void SetColor( const ITRBasicLighting::Color& color );
   private:
      ITRBasicLighting::Color   m_color;
};

class CMaterialProperties : public CDialog
{
// Construction
public:
	CMaterialProperties(CWnd* pParent = NULL);   // standard constructor

	int	            m_materialIndex;
	GFXPalette*       m_palette;
	TSMaterialList*   m_materialList;
   int               m_stretchit;
	CMatCanvas*	      m_pFrame;
   bool              m_init;
   void UpdateMaterialInfo();
   void EnableControls( bool bModified );

	static ITRBasicLighting::MaterialPropList m_propertyInfo;
	static int m_materialCount;
   static void Serialize( CThredParser& Parser );
   static void Initialize( int MatCount );

// Dialog Data
	//{{AFX_DATA(CMaterialProperties)
	enum { IDD = IDD_MATERIALPROPERTIES };
	CColorButton	   m_colorPick;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMaterialProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMaterialProperties)
	afx_msg void OnMpColorpick();
	afx_msg void OnMpFlatcolorradio();
	afx_msg void OnMpFlatemissionradio();
	afx_msg void OnMpModifycheck();
	afx_msg void OnMpStretchcheck();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	virtual void OnCancel();
	afx_msg void OnDeltaposMpSpinmat(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeMpMatindex();
	afx_msg void OnChangeMpColoredit();
	afx_msg void OnChangeMpIntensityedit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MATERIALPROPERTIES_H__3CE88FF3_941E_11D1_8D13_006097B90B28__INCLUDED_)
