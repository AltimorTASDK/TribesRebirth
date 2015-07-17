//////////////
// Matcanvs.h - materials canvas header...
////
#ifndef _MATCANVS_H_
#define _MATCANVS_H_

#include "g_surfac.h"
#include "ts_material.h" //#include "ts_mat.h"
#include "ts_RenderContext.h"
#include "ts_light.h"
#include "ts_camera.h"
#include "ts_pointarray.h"


class CMatCanvas : public CWnd
{
	typedef CWnd Base;
// Construction
public:
	CMatCanvas();

// Attributes
public:
	GFXSurface* m_pSurface;
	/*const*/ TSMaterial* m_pMaterial;
    int  stretchit;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMatCanvas)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMatCanvas();
	void Create(CWnd* parent,CRect pos, UINT in_winID);
	void Refresh();
    void DrawFilledRect(RectI* therect, int colorindex);
    void DrawRect(RectI* therect, int colorindex);
    void Lockit();
    void Unlockit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMatCanvas)
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnPaletteChanged(CWnd* focus);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

protected:
    TSRenderContext*      m_renderContext;
    TSPointArray          DefaultPointArray;
	TSPerspectiveCamera*  m_pTSCamera;
	TSSceneLighting*	  m_GSceneLights;

};



#endif
