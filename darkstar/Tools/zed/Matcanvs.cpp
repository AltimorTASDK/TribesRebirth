/////////////////////////////////////////////////////////////////////////////
// Matcanvs.cpp - implementation...
///

#include "stdafx.h"
#include "matcanvs.h"
#include "g_cds.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BEGIN_MESSAGE_MAP(CMatCanvas, CWnd)
	//{{AFX_MSG_MAP(CMatCanvas)
	ON_WM_QUERYNEWPALETTE()
	ON_WM_PAINT()
	ON_WM_PALETTECHANGED()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CMatCanvas::CMatCanvas()
{
	m_pMaterial = 0;
	m_pSurface = 0;

    m_renderContext = new TSRenderContext;
    m_GSceneLights = new TSSceneLighting;
}

void CMatCanvas::Create(CWnd* parent,CRect pos,UINT in_winId)
{
 	const char* cName = AfxRegisterWndClass(CS_OWNDC | 
 			CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNCLIENT,
 			::LoadCursor(NULL,IDC_ARROW));
	Base::Create(cName, "Canvas" ,WS_VISIBLE, pos, parent, in_winId,NULL);

	// DMMSOLID added point
	Point2I temp(0,0);
	GFXCDSSurface::create(m_pSurface, TRUE,
		pos.Width(), pos.Height(), GetSafeHwnd(), &temp);

	RectI screenView(Point2I(0,0),Point2I(pos.Width()-1, pos.Height()-1));

//    double max_dim = (pos.Width() > pos.Height()) ? (double)pos.Width() : (double)pos.Height();
//	RectF worldView(Point2F(-pos.Width()/max_dim, pos.Height()/max_dim),
//			            Point2F(pos.Width()/max_dim, -pos.Height()/max_dim));
	RectF worldView(Point2F(0, 0),
			            Point2F(pos.Width(), pos.Height()));

    m_pTSCamera = new TSPerspectiveCamera(screenView, worldView, 1.F, 1.0E4f);

    m_renderContext->setSurface(m_pSurface);
    m_renderContext->setPointArray(&DefaultPointArray);
    m_renderContext->setCamera(m_pTSCamera);
    m_renderContext->setLights(m_GSceneLights);

    m_renderContext->getSurface()->setHazeSource(GFX_HAZE_NONE);
//    m_renderContext->getSurface()->setShadeSource(GFX_SHADE_NONE);
    m_renderContext->getSurface()->setAlphaSource(GFX_ALPHA_NONE);
    m_renderContext->getSurface()->setTransparency(FALSE);

}

CMatCanvas::~CMatCanvas()
{
	// We don't own the bitmap;
	delete m_pSurface;

    if (m_renderContext)
    {
		delete m_renderContext;
        m_renderContext = NULL;
    }

    if (m_pTSCamera)
    {
        delete m_pTSCamera;
        m_pTSCamera = NULL;
    }

}

void CMatCanvas::OnPaletteChanged(CWnd* focus)
{
	// This method doesn't seem to get called?
	m_pSurface->setPalette(0,0,256,false);
}

BOOL CMatCanvas::OnQueryNewPalette()
{
	// This method doesn't seem to get called?
	// return CStatic::OnQueryNewPalette();
	// Assumes class CS_OWNDC
	HDC hDC = ::GetDC(m_hWnd);
	if (m_pSurface)
		// Reselect the HPALETTE
		m_pSurface->setPalette(NULL,0,1, false);
	if (RealizePalette(hDC)) {
		// Palette has changed
		InvalidateRect(NULL, TRUE);
		return true;
	}
	return false;
}

void CMatCanvas::OnPaint()
{
	RECT rect;
	if (GetUpdateRect(&rect)) {
		PAINTSTRUCT ps;
	  	BeginPaint(&ps);
		Refresh();
		EndPaint(&ps);
	}
	else
		Refresh();
}

static Point3F otPoints[4] =
{
	Point3F(0.0f,0.0f,0.0f),
	Point3F(1.0f,0.0f,0.0f),
	Point3F(1.0f,1.0f,0.0f),
	Point3F(0.0f,1.0f,0.0f),
};

#if 0
Point4I vPoints[4] =
{
	Point4I(0,0,0,0.0f),
	Point4I(0,0,0,0.0f),
	Point4I(0,0,0,0.0f),
	Point4I(0,0,0,0.0f),
};

Point2I dpvPoints[4] =
{
	Point2I(0,0),
	Point2I(0,0),
	Point2I(0,0),
	Point2I(0,0),
};

Point4I* pVPoints[4] = 
{
	&vPoints[0],
	&vPoints[1],
	&vPoints[2],
	&vPoints[3],
};

Point2I* dpVPoints[4] = 
{
	&dpvPoints[0],
	&dpvPoints[1],
	&dpvPoints[2],
	&dpvPoints[3],
};
#endif

void CMatCanvas::Refresh()
{
	CRect wsize;
	GetWindowRect(wsize);
	Point2I zero(0,0);
	Point2I stretch(wsize.Width(),wsize.Height());
    
    m_renderContext->lock();
    m_renderContext->getSurface()->clear(0);
    m_renderContext->getSurface()->draw3DBegin();

	if (!m_pMaterial)
		m_pSurface->drawRect2d_f(&RectI(zero,stretch),0);
	else
      switch ((m_pMaterial->fParams.fFlags & TSMaterial::MatFlags)) {
			case TSMaterial::MatNull:
				m_pSurface->drawRect2d_f(&RectI(zero,stretch),0);
				break;

			case TSMaterial::MatRGB:
			case TSMaterial::MatPalette:
				m_pSurface->drawRect2d_f(&RectI(zero,stretch),
					m_pMaterial->fParams.fIndex);
				break;

			case TSMaterial::MatTexture:
              {
				if (m_pMaterial->getTextureMap() != 0)
                {
                  if (stretchit)
                  {
                    m_renderContext->getSurface()->drawBitmap2d(m_pMaterial->getTextureMap(), &zero, &stretch);
                  } else {
                    m_renderContext->getSurface()->drawBitmap2d(m_pMaterial->getTextureMap(), &zero);
                  }
				}
				break;
			}
		}
    m_renderContext->getSurface()->update();
    m_renderContext->getSurface()->draw3DEnd();
    m_renderContext->unlock();
}

void CMatCanvas::Lockit()
{
    m_renderContext->lock();
    m_renderContext->getSurface()->draw3DBegin();
}

void CMatCanvas::Unlockit()
{
    m_renderContext->getSurface()->update();
    m_renderContext->getSurface()->draw3DEnd();
    m_renderContext->unlock();
}

void CMatCanvas::DrawFilledRect(RectI* therect, int colorindex)
{
	m_pSurface->drawRect2d_f(therect, colorindex);
}

void CMatCanvas::DrawRect(RectI* therect, int colorindex)
{
	m_pSurface->drawRect2d(therect, colorindex);
}
