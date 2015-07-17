// StaircaseDialog.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "thredbrush.h"
#include "StaircaseDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStaircaseDialog dialog


CStaircaseDialog::CStaircaseDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CStaircaseDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStaircaseDialog)
	m_NumberOfSteps = 8;
	m_XSize = 64.0;
	m_YSize = 128.0;
	m_ZSize = 128.0;
	//}}AFX_DATA_INIT
}


void CStaircaseDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStaircaseDialog)
	DDX_Text(pDX, IDC_NUMSTEPS, m_NumberOfSteps);
	DDX_Text(pDX, IDC_XSIZEWIDTH, m_XSize);
	DDX_Text(pDX, IDC_YSIZEHEIGHT, m_YSize);
	DDX_Text(pDX, IDC_ZSIZELENGTH, m_ZSize);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStaircaseDialog, CDialog)
	//{{AFX_MSG_MAP(CStaircaseDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStaircaseDialog message handlers

// xsize = width, ysize = height, zsize = length




void CStaircaseDialog::DoDialog(CThredBrush* Brush)
{
	if(DoModal() == IDOK )
		MakeBrush(Brush );
}


void CStaircaseDialog::MakeBrush(CThredBrush* Brush)
{
//	m_NumberOfSteps = 8;
//	m_XSize = 64.0;
//	m_YSize = 128.0;
//	m_ZSize = 128.0;

	// Boxes have maximum 16 verts.
	ThredPoint	Verts[16];
	ThredPolygon* Polygons;

	// check if we are solid or not
	if(m_Solid != 0 )
    {
		Brush->AllocateBrushPolygons(12 );
		Brush->AllocateBrushPolyTextures(12 );

        // DPW - init texture ids...
        for (int i = 0; i < NumberOfPolygons; i++)
        {
          Brush->mTextureID[i].TextureID = 0;
          Brush->mTextureID[i].TextureShift.x = 0;
          Brush->mTextureID[i].TextureShift.y = 0;
          Brush->mTextureID[i].TextureFlipH = 0;
          Brush->mTextureID[i].TextureFlipV = 0;
          Brush->mTextureID[i].TextureRotate = 0.F;
        }

    }
	else
    {
		Brush->AllocateBrushPolygons(6 );
		Brush->AllocateBrushPolyTextures(6 );

        // DPW - init texture ids...
        for (int i = 0; i < NumberOfPolygons; i++)
        {
          Brush->mTextureID[i].TextureID = 0;
          Brush->mTextureID[i].TextureShift.x = 0;
          Brush->mTextureID[i].TextureShift.y = 0;
          Brush->mTextureID[i].TextureFlipH = 0;
          Brush->mTextureID[i].TextureFlipV = 0;
          Brush->mTextureID[i].TextureRotate = 0.F;
        }
    }

	// get our pointer
	Polygons = Brush->mPolygons;


	// we need to calculate how many polygons we
	// need.  1 for the bottom, 1 for the
	// back and 4 for each stair.
	int NumberOfPolygons = 2 + (4 * m_NumberOfSteps );
	Brush->AllocateBrushPolygons(NumberOfPolygons );
	Brush->AllocateBrushPolyTextures(NumberOfPolygons );

    // DPW - init texture ids...
    for (int i = 0; i < NumberOfPolygons; i++)
    {
      Brush->mTextureID[i].TextureID = 0;
      Brush->mTextureID[i].TextureShift.x = 0;
      Brush->mTextureID[i].TextureShift.y = 0;
      Brush->mTextureID[i].TextureFlipH = 0;
      Brush->mTextureID[i].TextureFlipV = 0;
      Brush->mTextureID[i].TextureRotate = 0.F;
    }

	double StepHeight = m_YSize / (double)m_NumberOfSteps;
	double StepLength = m_ZSize / (double)m_NumberOfSteps;
	double HalfWidth = m_XSize / 2;
	double HalfHeight = m_YSize / 2;
	double HalfLength = m_ZSize / 2;

	// now make the back
	Brush->mPolygons[0].NumberOfPoints = 4;
	// assign the verts for the back
	Brush->mPolygons[0].Points[0].X = -HalfWidth;
	Brush->mPolygons[0].Points[0].Y = -HalfHeight
	Brush->mPolygons[0].Points[0].Z = HalfLength;

	Brush->mPolygons[0].Points[1].X = HalfWidth;
	Brush->mPolygons[0].Points[1].Y = -HalfHeight
	Brush->mPolygons[0].Points[1].Z = HalfLength;

	Brush->mPolygons[0].Points[2].X = HalfWidth;
	Brush->mPolygons[0].Points[2].Y = HalfHeight
	Brush->mPolygons[0].Points[2].Z = HalfLength;

	Brush->mPolygons[0].Points[3].X = -HalfWidth;
	Brush->mPolygons[0].Points[3].Y = HalfHeight
	Brush->mPolygons[0].Points[3].Z = HalfLength;


	// skip over the first 2
	int CurrentPolygon = 2;







}


