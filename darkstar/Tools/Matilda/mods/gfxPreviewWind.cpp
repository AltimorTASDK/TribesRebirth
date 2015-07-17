// gfxPreviewWind.cpp : implementation file
//

#include "stdafx.h"
#include "matilda2.h"
#include "gfxPreviewWind.h"

#include "mledit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// gfxPreviewWind

gfxPreviewWind::gfxPreviewWind()
{
	m_matType   = TSMaterial::MatNull;
	m_pMaterial = NULL;
	pSurface  = NULL;
}

gfxPreviewWind::~gfxPreviewWind()
{
	if (pSurface) {
		delete pSurface;
		pSurface = NULL;
	}

	if (m_pMaterial != NULL) {
		delete m_pMaterial;
		m_pMaterial = NULL;
	}
}

void gfxPreviewWind::setPalette(GFXPalette *in_pal)
{
	pSurface->setPalette(in_pal, rsvPalFlag);
}


BEGIN_MESSAGE_MAP(gfxPreviewWind, CWnd)
	//{{AFX_MSG_MAP(gfxPreviewWind)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// gfxPreviewWind message handlers

void gfxPreviewWind::Create(CWnd* parent,CRect pos, UINT resID, Bool in_rsvPal)
{
 	const char* cName = AfxRegisterWndClass(CS_OWNDC | 
				 			CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT,
					 			::LoadCursor(NULL,IDC_ARROW));
	CWnd::Create(cName, "Canvas" ,WS_VISIBLE, pos, parent, resID,NULL);

	RECT cliRect;
	GetClientRect(&cliRect);

	GFXCDSSurface::setFunctionTable(&rclip_table);
	GFXMemSurface::setFunctionTable(&rclip_table);
	GFXDDSurface::setFunctionTable(&rclip_table);

	Point2I temp(0,0);
	GFXCDSSurface::create(pSurface, TRUE, cliRect.right, cliRect.bottom, this->GetSafeHwnd(), &temp, 8, NULL);

	rsvPalFlag = in_rsvPal;
	if (mlEdit.pGFXPal) {
		pSurface->setPalette(mlEdit.pGFXPal, in_rsvPal);
	}
	
	pSurface->lock();
	pSurface->clear(2);
	pSurface->unlock();
	pSurface->update();

//	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

void gfxPreviewWind::OnPaint() 
{
	if (!mlEdit.isMaterialListValid())
		return;
	
	if (m_matType == TSMaterial::MatPalette || m_matType == TSMaterial::MatTexture ||
		m_matType == TSMaterial::MatNull) {
		RECT rect;
		if (GetUpdateRect(&rect)) {
			PAINTSTRUCT ps;
			BeginPaint(&ps);
			Refresh();
			EndPaint(&ps);
		} else {
			Refresh();
		}
	} else {
		// The material is an RGB, so we'll call on windows to display it, dithering in 256
		// color mode, or displaying real color in true-color modes...
		//
		CPaintDC dc(this); // device context for painting
		
		UInt8 red   = m_pMaterial->fRGB.fRed;
		UInt8 green = m_pMaterial->fRGB.fGreen;
		UInt8 blue  = m_pMaterial->fRGB.fBlue;
		
		COLORREF m_color = RGB(red, green, blue);

		CBrush brRGB( m_color );
		dc.FillRect( &dc.m_ps.rcPaint, &brRGB );
	}
}




static Point2F otPoints[4] =
{
	Point2F(0.0f,0.0f),
	Point2F(1.0f,0.0f),
	Point2F(1.0f,1.0f),
	Point2F(0.0f,1.0f),
};

Point4I vPoints[4] =
{
	Point4I(0,0,0,0.0f),
	Point4I(0,0,0,0.0f),
	Point4I(0,0,0,0.0f),
	Point4I(0,0,0,0.0f),
};

Point4I* pVPoints[4] = 
{
	&vPoints[0],
	&vPoints[1],
	&vPoints[2],
	&vPoints[3],
};



void gfxPreviewWind::Refresh() 
{
	CRect wsize;
	GetWindowRect(wsize);
	Point2I zero(0,0);
	Point2I stretch(wsize.Width() - 1,wsize.Height() - 1);
	

	pSurface->lock();
	if (m_matType == TSMaterial::MatNull) {
		pSurface->clear(0);
	} else if (m_matType == TSMaterial::MatPalette) {
		pSurface->clear(m_pMaterial->fIndex);
	} else if (m_matType == TSMaterial::MatTexture) {
		if (updateBitmap() == false) {
			return;
		}
		m_pMaterial->lock();
		GFXPoly poly;
		if ((poly.pBitmap = m_pMaterial->getTextureMap()) != 0) {
			Point2F *pTPoints[4], tPoints[4];
			poly.flags = 0;
			poly.pTextureVertex = pTPoints;
			poly.pVertex = pVPoints;
			poly.vertexCount = 4;
			poly.pColorInfo = new GFXColorInfo *;
			poly.pColorInfo[0] = new GFXColorInfo;

			vPoints[1].x = stretch.x;
			vPoints[2].set(stretch.x,stretch.y);
			vPoints[3].y = stretch.y;

			for (int i = 0; i < poly.vertexCount; i++) {
				pTPoints[i] = &tPoints[i];	
				m_pMaterial->transformTexture(otPoints[i],&tPoints[i]);
			}
			pSurface->drawPoly2d_t((GFXPoly2d*)&poly);
			delete poly.pColorInfo[0];
			poly.pColorInfo[0] = NULL;
			delete poly.pColorInfo;
			poly.pColorInfo = NULL;

			m_pMaterial->unlock();
		} else {
			pSurface->clear(255);
			m_pMaterial->unlock();
		}
	}
	
	pSurface->unlock();
	pSurface->update();
}


bool gfxPreviewWind::updateBitmap()
{
	if(strcmp(m_pMaterial->fMapFile, "") == 0)
		return false;

	// load copy of bitmap
	GFXBitmap* bitmap = GFXBitmap::load(m_pMaterial->fMapFile,0);
	if(bitmap) {
		TSTextureManager::registerBmp(m_pMaterial->fMapFile,bitmap);
		// Texture manager deletes this bitmap...
		//delete bitmap;
	}
	else {
		char cErrorString[TMP_BSIZE];
		strcpy( cErrorString, "Unable to load Bitmap - " );
		strcat( cErrorString, m_pMaterial->fMapFile);
		AfxMessageBox(  cErrorString );
		return false;
	}
	return true;
}



BOOL gfxPreviewWind::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}


void gfxPreviewWind::installMaterial(TSMaterial *in_material, bool in_makeCopyFlag)
{
	if (m_pMaterial != NULL) {
		delete m_pMaterial;
	}
	if (in_makeCopyFlag == false) {
		m_pMaterial = in_material;
		m_matType   = m_pMaterial->fType;
	} else {
		m_pMaterial = new TSMaterial(*in_material);
		m_matType   = m_pMaterial->fType;
	}

	this->Invalidate();
}