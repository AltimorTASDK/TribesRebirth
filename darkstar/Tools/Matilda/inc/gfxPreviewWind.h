// gfxPreviewWind.h : header file
//
#ifndef _GFXPREVIEWWIND_H_
#define _GFXPREVIEWWIND_H_

#include <types.h>
//#include <gfx.h>
#include <g_cds.h>
#include <g_ddraw.h>
#include <g_mem.h>
//#include <g_smanag.h>
#include <fn_table.h>

#include <ts.h>
#include <ts_material.h>

#include "MLedit.h"

extern ResourceManager	rm;
extern VolumeRStream	vrStream;
extern CString pSearchPath;

/////////////////////////////////////////////////////////////////////////////
// gfxPreviewWind window

class gfxPreviewWind : public CWnd
{
// Construction
public:
	gfxPreviewWind();

// Attributes
public:
	void Create(CWnd* parent,CRect pos, UINT resID, Bool in_rsvPal = false);
	void Refresh();

// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(gfxPreviewWind)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~gfxPreviewWind();

	void setPalette(GFXPalette *in_pal);
	bool updateBitmap();

	void installMaterial(TS::Material *in_material, bool in_makeCopyFlag);

	// Generated message map functions
protected:
	GFXSurface			*pSurface;
	TS::Material::MatType  m_matType;
	TS::Material          *m_pMaterial;
	COLORREF			 m_color;
	Bool				 rsvPalFlag;

	//{{AFX_MSG(gfxPreviewWind)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif // HEADER THING
