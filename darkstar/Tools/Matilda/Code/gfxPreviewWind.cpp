// gfxPreviewWind.cpp : implementation file
//

#include <assert.h>
#include "stdafx.h"
#include "matilda2.h"
#include "gfxPreviewWind.h"

#include "mledit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern BOOL WantStretched;

/////////////////////////////////////////////////////////////////////////////
// gfxPreviewWind

gfxPreviewWind::gfxPreviewWind()
{
	m_matType   = TS::Material::MatNull;
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
	::RealizePalette(pSurface->getDC());
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

//	GFXCDSSurface::setFunctionTable(&rclip_table);
//	GFXMemSurface::setFunctionTable(&rclip_table);
//	GFXDDSurface::setFunctionTable(&rclip_table);

	Point2I temp(0,0);
	GFXCDSSurface::create(pSurface, TRUE, cliRect.right, cliRect.bottom, this->GetSafeHwnd(), &temp, 8 );

	rsvPalFlag = in_rsvPal;
	if (mlEdit.pGFXPal) {
		pSurface->setPalette(mlEdit.pGFXPal, in_rsvPal);
		::RealizePalette(pSurface->getDC());
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
	
	if (m_matType == TS::Material::MatPalette || m_matType == TS::Material::MatTexture ||
		m_matType == TS::Material::MatNull) {
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
		
		UInt8 red   = m_pMaterial->fParams.fRGB.fRed;
		UInt8 green = m_pMaterial->fParams.fRGB.fGreen;
		UInt8 blue  = m_pMaterial->fParams.fRGB.fBlue;
		
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


#if 0
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
#endif



void gfxPreviewWind::Refresh() 
{

	CRect wsize;
	GetWindowRect(wsize);
	Point2I zero(0,0);
	Point2I stretch(wsize.Width() - 1,wsize.Height() - 1);

	pSurface->lock();
	if (m_matType == TS::Material::MatNull) pSurface->clear(0);
	else if (m_matType == TS::Material::MatPalette) pSurface->clear(m_pMaterial->fParams.fIndex);
	else if (m_matType == TS::Material::MatTexture) {

		if (updateBitmap() == false) return;
		
	
	
/*		BOOL found=false;
		CString tempPath = pSearchPath;
		CString onePath;*/
		CString theFile = m_pMaterial->fParams.fMapFile;
/*		CString oneFullPath;
		int foundindex;
		
		while (!found && !tempPath.IsEmpty()) {

			foundindex = tempPath.ReverseFind(';') + 1;
			onePath= tempPath.Mid(foundindex);
			tempPath= tempPath.Left(foundindex);
			
			//look for file here
			oneFullPath+= onePath+theFile;
			if (GetFileAttributes(oneFullPath) != -1) found= 1;
		}

		if (!found) {

		} else {*/

		ResourceObject *obj= rm.load(theFile);
		if (obj && obj->resource) {
     		m_pMaterial->load(rm, 1);
			const GFXBitmap *pBitmap= m_pMaterial->getTextureMap();
			AssertFatal(pBitmap, "gfxPreviewWind::Refresh: get Material TextureMap failed!");
			pSurface->clear(0);

			if (WantStretched) {
				pSurface->drawBitmap2d(pBitmap, &Point2I(0,0), &Point2I(stretch.x, stretch.y));
			} else {
				pSurface->drawBitmap2d(pBitmap, &Point2I(0,0));
			}
		} else 
         pSurface->clear(0);


	
	}
	
	pSurface->unlock();
	pSurface->update();
	

}


bool gfxPreviewWind::updateBitmap()
{
	if(strcmp(m_pMaterial->fParams.fMapFile, "") == 0)
		return false;

	// load copy of bitmap
	//GFXBitmap* bitmap = GFXBitmap::load(m_pMaterial->fParams.fMapFile,0);
	//if(bitmap) {
		//TSTextureManager::registerBmp(m_pMaterial->fMapFile,bitmap);
		// Texture manager deletes this bitmap...
		//delete bitmap;

/*	if (m_pMaterial->isReady()) {
	}
	else {
		char cErrorString[TMP_BSIZE];
		strcpy( cErrorString, "Unable to load Bitmap - " );
		strcat( cErrorString, m_pMaterial->fParams.fMapFile);
		AfxMessageBox(  cErrorString );
		return false;
	}*/
	return true;
}



BOOL gfxPreviewWind::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}


void gfxPreviewWind::installMaterial(TS::Material *in_material, bool in_makeCopyFlag)
{
	if (m_pMaterial != NULL) {
		delete m_pMaterial;
		m_pMaterial=NULL;
	}

	if (in_makeCopyFlag == false) {
		m_pMaterial = in_material;
		m_pMaterial->fParams= in_material->fParams;
		m_matType   = (enum TS::Material::MatType)(m_pMaterial->fParams.fFlags &TS::Material::MatFlags);
	} else {
		m_pMaterial = new TS::Material(*in_material);
		m_pMaterial->fParams= in_material->fParams;
		m_matType   = (enum TS::Material::MatType)(m_pMaterial->fParams.fFlags &TS::Material::MatFlags);
	}

	this->Invalidate();
}