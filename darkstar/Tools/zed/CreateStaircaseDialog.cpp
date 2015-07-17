// CreateStaircaseDialog.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "CreateStaircaseDialog.h"
#include "Thredbrush.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCreateStaircaseDialog dialog


CCreateStaircaseDialog::CCreateStaircaseDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateStaircaseDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateStaircaseDialog)
	m_Height = 128.0;
	m_Length = 128.0;
	m_NumberOfStairs = 8.0;
	m_Width = 64.0;
	m_MakeRamp = FALSE;
	//}}AFX_DATA_INIT
}


void CCreateStaircaseDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateStaircaseDialog)
	DDX_Text(pDX, IDC_HEIGHT, m_Height);
	DDX_Text(pDX, IDC_LENGTH, m_Length);
	DDX_Text(pDX, IDC_NUMBEROFSTAIRS, m_NumberOfStairs);
	DDX_Text(pDX, IDC_WIDTH, m_Width);
	DDX_Check(pDX, IDC_CHECK1, m_MakeRamp);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateStaircaseDialog, CDialog)
	//{{AFX_MSG_MAP(CCreateStaircaseDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateStaircaseDialog message handlers

void CCreateStaircaseDialog::DoDialog(CThredBrush* Brush)
{
	if(DoModal() == IDOK ) {
		MakeBrush(Brush );
	}
}


// This function will actually make the staircase
void CCreateStaircaseDialog::MakeBrush(CThredBrush* Brush)
{
	int NumberOfPolygons;
	double HalfWidth = m_Width/2;
	double HalfHeight = m_Height/2;
	double HalfLength = m_Length/2;
	int CPoly;
	double CurrentHeight;
	double CurrentLength;
	double HeightInc;
	double LengthInc;



	if(m_MakeRamp )
	{
		NumberOfPolygons = 5;

		Brush->AllocateBrushPolygons(NumberOfPolygons );
		Brush->AllocateBrushPolyTextures(NumberOfPolygons );

        // DPW - init texture ids...
        for (int i = 0; i < NumberOfPolygons; i++)
        {
          Brush->mTextureID[i].TextureID = atoi(Brush->mName);
          Brush->mTextureID[i].TextureShift.x = 0;
          Brush->mTextureID[i].TextureShift.y = 0;
          Brush->mTextureID[i].TextureFlipH = 0;
          Brush->mTextureID[i].TextureFlipV = 0;
          Brush->mTextureID[i].TextureRotate = 0.F;
        }

		// now we have to make the back of the staircase
		Brush->mPolygons[0].NumberOfPoints = 4;
		Brush->mPolygons[0].Points[0].X = -HalfWidth;
		Brush->mPolygons[0].Points[0].Y = -HalfHeight;
		Brush->mPolygons[0].Points[0].Z = HalfLength;
		Brush->mPolygons[0].Points[1].X = HalfWidth;
		Brush->mPolygons[0].Points[1].Y = -HalfHeight;
		Brush->mPolygons[0].Points[1].Z = HalfLength;

		Brush->mPolygons[0].Points[2].X = HalfWidth;
		Brush->mPolygons[0].Points[2].Y = HalfHeight;
		Brush->mPolygons[0].Points[2].Z = HalfLength;
		Brush->mPolygons[0].Points[3].X = -HalfWidth;
		Brush->mPolygons[0].Points[3].Y = HalfHeight;
		Brush->mPolygons[0].Points[3].Z = HalfLength;


		// now we make the bottom of the staircase
		Brush->mPolygons[1].NumberOfPoints = 4;
		Brush->mPolygons[1].Points[0].X = HalfWidth;
		Brush->mPolygons[1].Points[0].Y = -HalfHeight;
		Brush->mPolygons[1].Points[0].Z = HalfLength;
		Brush->mPolygons[1].Points[1].X = -HalfWidth;
		Brush->mPolygons[1].Points[1].Y = -HalfHeight;
		Brush->mPolygons[1].Points[1].Z = HalfLength;
		Brush->mPolygons[1].Points[2].X = -HalfWidth;
		Brush->mPolygons[1].Points[2].Y = -HalfHeight;
		Brush->mPolygons[1].Points[2].Z = -HalfLength;
		Brush->mPolygons[1].Points[3].X = HalfWidth;
		Brush->mPolygons[1].Points[3].Y = -HalfHeight;
		Brush->mPolygons[1].Points[3].Z = -HalfLength;


		// make the "ramp" part
		Brush->mPolygons[2].NumberOfPoints = 4;
		Brush->mPolygons[2].Points[0].X = -HalfWidth;
		Brush->mPolygons[2].Points[0].Y = HalfHeight;
		Brush->mPolygons[2].Points[0].Z = HalfLength;
		Brush->mPolygons[2].Points[1].X = HalfWidth;
		Brush->mPolygons[2].Points[1].Y = HalfHeight;
		Brush->mPolygons[2].Points[1].Z = HalfLength;
		Brush->mPolygons[2].Points[2].X = HalfWidth;
		Brush->mPolygons[2].Points[2].Y = -HalfHeight;
		Brush->mPolygons[2].Points[2].Z = -HalfLength;
		Brush->mPolygons[2].Points[3].X = -HalfWidth;
		Brush->mPolygons[2].Points[3].Y = -HalfHeight;
		Brush->mPolygons[2].Points[3].Z = -HalfLength;

		// make one side
		Brush->mPolygons[3].NumberOfPoints = 3;
		Brush->mPolygons[3].Points[0].X = HalfWidth;
		Brush->mPolygons[3].Points[0].Y = HalfHeight;
		Brush->mPolygons[3].Points[0].Z = HalfLength;

		Brush->mPolygons[3].Points[1].X = HalfWidth;
		Brush->mPolygons[3].Points[1].Y = -HalfHeight;
		Brush->mPolygons[3].Points[1].Z = HalfLength;

		Brush->mPolygons[3].Points[2].X = HalfWidth;
		Brush->mPolygons[3].Points[2].Y = -HalfHeight;
		Brush->mPolygons[3].Points[2].Z = -HalfLength;

		// make one side
		Brush->mPolygons[4].NumberOfPoints = 3;
		Brush->mPolygons[4].Points[2].X = -HalfWidth;
		Brush->mPolygons[4].Points[2].Y = HalfHeight;
		Brush->mPolygons[4].Points[2].Z = HalfLength;

		Brush->mPolygons[4].Points[1].X = -HalfWidth;
		Brush->mPolygons[4].Points[1].Y = -HalfHeight;
		Brush->mPolygons[4].Points[1].Z = HalfLength;

		Brush->mPolygons[4].Points[0].X = -HalfWidth;
		Brush->mPolygons[4].Points[0].Y = -HalfHeight;
		Brush->mPolygons[4].Points[0].Z = -HalfLength;


	}
	else
	{

		// calculate how many polygons we need for the brush
		// 1 for the back, 1 for the bottom and 4 for each stair.
		NumberOfPolygons = 2 + ((int)m_NumberOfStairs * 4 );

		// allocate the polygons
		Brush->AllocateBrushPolygons(NumberOfPolygons );
		Brush->AllocateBrushPolyTextures(NumberOfPolygons );

        // DPW - init texture ids...
        for (int i = 0; i < NumberOfPolygons; i++)
        {
          Brush->mTextureID[i].TextureID = 0;
          Brush->mTextureID[i].TextureShift.x = 0;
          Brush->mTextureID[i].TextureShift.y = 0;
          Brush->mTextureID[i].TextureRotate = 0.F;
        }

		// now we have to make the back of the staircase
		Brush->mPolygons[0].NumberOfPoints = 4;
		Brush->mPolygons[0].Points[0].X = -HalfWidth;
		Brush->mPolygons[0].Points[0].Y = -HalfHeight;
		Brush->mPolygons[0].Points[0].Z = HalfLength;
		Brush->mPolygons[0].Points[1].X = HalfWidth;
		Brush->mPolygons[0].Points[1].Y = -HalfHeight;
		Brush->mPolygons[0].Points[1].Z = HalfLength;
		Brush->mPolygons[0].Points[2].X = HalfWidth;
		Brush->mPolygons[0].Points[2].Y = HalfHeight;
		Brush->mPolygons[0].Points[2].Z = HalfLength;
		Brush->mPolygons[0].Points[3].X = -HalfWidth;
		Brush->mPolygons[0].Points[3].Y = HalfHeight;
		Brush->mPolygons[0].Points[3].Z = HalfLength;


		// now we make the bottom of the staircase
		Brush->mPolygons[1].NumberOfPoints = 4;
		Brush->mPolygons[1].Points[0].X = HalfWidth;
		Brush->mPolygons[1].Points[0].Y = -HalfHeight;
		Brush->mPolygons[1].Points[0].Z = HalfLength;
		Brush->mPolygons[1].Points[1].X = -HalfWidth;
		Brush->mPolygons[1].Points[1].Y = -HalfHeight;
		Brush->mPolygons[1].Points[1].Z = HalfLength;
		Brush->mPolygons[1].Points[2].X = -HalfWidth;
		Brush->mPolygons[1].Points[2].Y = -HalfHeight;
		Brush->mPolygons[1].Points[2].Z = -HalfLength;
		Brush->mPolygons[1].Points[3].X = HalfWidth;
		Brush->mPolygons[1].Points[3].Y = -HalfHeight;
		Brush->mPolygons[1].Points[3].Z = -HalfLength;


		// now we go through and create all of the
		// actual steps
		CPoly = 2;
		CurrentHeight = -HalfHeight;
		CurrentLength = -HalfLength;
		HeightInc = m_Height / m_NumberOfStairs;
		LengthInc = m_Length / m_NumberOfStairs;

		// for each step we have to create 4 polygons.
		for(int Cur = 0; Cur < m_NumberOfStairs; Cur++ ) {

			// start off with the front of the stair
			Brush->mPolygons[CPoly].NumberOfPoints = 4;
			Brush->mPolygons[CPoly].Points[0].X = -HalfWidth;
			Brush->mPolygons[CPoly].Points[0].Y = CurrentHeight;
			Brush->mPolygons[CPoly].Points[0].Z = CurrentLength;
			Brush->mPolygons[CPoly].Points[1].X = -HalfWidth;
			Brush->mPolygons[CPoly].Points[1].Y = CurrentHeight + HeightInc;
			Brush->mPolygons[CPoly].Points[1].Z = CurrentLength;
			Brush->mPolygons[CPoly].Points[2].X = HalfWidth;
			Brush->mPolygons[CPoly].Points[2].Y = CurrentHeight + HeightInc;
			Brush->mPolygons[CPoly].Points[2].Z = CurrentLength;
			Brush->mPolygons[CPoly].Points[3].X = HalfWidth;
			Brush->mPolygons[CPoly].Points[3].Y = CurrentHeight;
			Brush->mPolygons[CPoly].Points[3].Z = CurrentLength;
			CPoly++;

			// now do the top of the stair
			Brush->mPolygons[CPoly].NumberOfPoints = 4;
			Brush->mPolygons[CPoly].Points[0].X = -HalfWidth;
			Brush->mPolygons[CPoly].Points[0].Y = CurrentHeight + HeightInc;
			Brush->mPolygons[CPoly].Points[0].Z = CurrentLength;

			Brush->mPolygons[CPoly].Points[1].X = -HalfWidth;
			Brush->mPolygons[CPoly].Points[1].Y = CurrentHeight + HeightInc;
			Brush->mPolygons[CPoly].Points[1].Z = CurrentLength + LengthInc;

			Brush->mPolygons[CPoly].Points[2].X = HalfWidth;
			Brush->mPolygons[CPoly].Points[2].Y = CurrentHeight + HeightInc;
			Brush->mPolygons[CPoly].Points[2].Z = CurrentLength + LengthInc;

			Brush->mPolygons[CPoly].Points[3].X = HalfWidth;
			Brush->mPolygons[CPoly].Points[3].Y = CurrentHeight + HeightInc;
			Brush->mPolygons[CPoly].Points[3].Z = CurrentLength;
			CPoly++;

			// now do the -X side of the stair
			Brush->mPolygons[CPoly].NumberOfPoints = 4;
			Brush->mPolygons[CPoly].Points[0].X = -HalfWidth;
			Brush->mPolygons[CPoly].Points[0].Y = CurrentHeight;
			Brush->mPolygons[CPoly].Points[0].Z = CurrentLength;

			Brush->mPolygons[CPoly].Points[1].X = -HalfWidth;
			Brush->mPolygons[CPoly].Points[1].Y = CurrentHeight;
			Brush->mPolygons[CPoly].Points[1].Z = HalfLength;

			Brush->mPolygons[CPoly].Points[2].X = -HalfWidth;
			Brush->mPolygons[CPoly].Points[2].Y = CurrentHeight + HeightInc;
			Brush->mPolygons[CPoly].Points[2].Z = HalfLength;

			Brush->mPolygons[CPoly].Points[3].X = -HalfWidth;
			Brush->mPolygons[CPoly].Points[3].Y = CurrentHeight + HeightInc;
			Brush->mPolygons[CPoly].Points[3].Z = CurrentLength;
			CPoly++;

			// now do the +X side of the stair
			Brush->mPolygons[CPoly].NumberOfPoints = 4;
			Brush->mPolygons[CPoly].Points[3].X = HalfWidth;
			Brush->mPolygons[CPoly].Points[3].Y = CurrentHeight;
			Brush->mPolygons[CPoly].Points[3].Z = CurrentLength;

			Brush->mPolygons[CPoly].Points[2].X = HalfWidth;
			Brush->mPolygons[CPoly].Points[2].Y = CurrentHeight;
			Brush->mPolygons[CPoly].Points[2].Z = HalfLength;

			Brush->mPolygons[CPoly].Points[1].X = HalfWidth;
			Brush->mPolygons[CPoly].Points[1].Y = CurrentHeight + HeightInc;
			Brush->mPolygons[CPoly].Points[1].Z = HalfLength;

			Brush->mPolygons[CPoly].Points[0].X = HalfWidth;
			Brush->mPolygons[CPoly].Points[0].Y = CurrentHeight + HeightInc;
			Brush->mPolygons[CPoly].Points[0].Z = CurrentLength;
			CPoly++;

			// inc our deltas
			CurrentHeight += HeightInc;
			CurrentLength += LengthInc;
		}
		// make sure we made the right amount
		_ASSERTE(CPoly == NumberOfPolygons);

	}

    Brush->BrushType = CThredBrush::STAIR_BRUSH;
  	Brush->BrushSpecifics.staircase.m_Height = m_Height;
	Brush->BrushSpecifics.staircase.m_Length = m_Length;
	Brush->BrushSpecifics.staircase.m_NumberOfStairs = m_NumberOfStairs;
	Brush->BrushSpecifics.staircase.m_Width = m_Width;
	Brush->BrushSpecifics.staircase.m_MakeRamp = m_MakeRamp;
}

void CCreateStaircaseDialog::OnOK() 
{
	CString strMessage;
    char    data[10];
    double   val;

	// Verify that none of the sizes are < 1
    ((CEdit*)GetDlgItem(IDC_HEIGHT))->GetLine(0, data, 10); 
    val = atof(data);

    if (val <= 0.0)
    {
      strMessage.Format("Height Must Be > 0");
      AfxMessageBox(strMessage, MB_OK, 0);
	  ((CEdit*)GetDlgItem(IDC_HEIGHT))->SetFocus();
	  ((CEdit*)GetDlgItem(IDC_HEIGHT))->SetSel(0, -1, FALSE);
      return;
    }

    ((CEdit*)GetDlgItem(IDC_WIDTH))->GetLine(0, data, 10); 
    val = atof(data);

    if (val <= 0.0)
    {
      strMessage.Format("Width Must Be > 0");
      AfxMessageBox(strMessage, MB_OK, 0);
	  ((CEdit*)GetDlgItem(IDC_WIDTH))->SetFocus();
	  ((CEdit*)GetDlgItem(IDC_WIDTH))->SetSel(0, -1, FALSE);
      return;
    }

    ((CEdit*)GetDlgItem(IDC_LENGTH))->GetLine(0, data, 10); 
    val = atof(data);

    if (val <= 0.0)
    {
      strMessage.Format("Length Must Be > 0");
      AfxMessageBox(strMessage, MB_OK, 0);
	  ((CEdit*)GetDlgItem(IDC_LENGTH))->SetFocus();
	  ((CEdit*)GetDlgItem(IDC_LENGTH))->SetSel(0, -1, FALSE);
      return;
    }

    ((CEdit*)GetDlgItem(IDC_NUMBEROFSTAIRS))->GetLine(0, data, 10); 
    val = atof(data);

    if (val <= 0.0)
    {
      strMessage.Format("Number Of Stairs Must Be > 0");
      AfxMessageBox(strMessage, MB_OK, 0);
	  ((CEdit*)GetDlgItem(IDC_NUMBEROFSTAIRS))->SetFocus();
	  ((CEdit*)GetDlgItem(IDC_NUMBEROFSTAIRS))->SetSel(0, -1, FALSE);
      return;
    }
	
	CDialog::OnOK();
}
