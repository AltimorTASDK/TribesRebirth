// colorPreviewFrame.cpp : implementation file
//

#include "stdafx.h"
#include "matilda2.h"
#include "colorPreviewFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// colorPreviewFrame

colorPreviewFrame::colorPreviewFrame()
{
}

colorPreviewFrame::~colorPreviewFrame()
{
}


BEGIN_MESSAGE_MAP(colorPreviewFrame, CStatic)
	//{{AFX_MSG_MAP(colorPreviewFrame)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// colorPreviewFrame message handlers

void colorPreviewFrame::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CBrush brRGB( m_color );
	dc.FillRect( &dc.m_ps.rcPaint, &brRGB );
}


void colorPreviewFrame::setPreviewColor(BYTE red, BYTE green, BYTE blue)
{
	m_color = (COLORREF)RGB(red, green, blue);
	Invalidate();
}