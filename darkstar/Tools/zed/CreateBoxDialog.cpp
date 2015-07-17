// CreateBoxDialog.cpp : implementation file
//

#include "stdafx.h"
#include "THRED.h"
#include "CreateBoxDialog.h"
#include "ThredBrush.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCreateBoxDialog dialog


CCreateBoxDialog::CCreateBoxDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateBoxDialog::IDD, pParent)
{
	// We want to initialize our data to something
	// reasonable

	//{{AFX_DATA_INIT(CCreateBoxDialog)
	m_YSize = 128.0;
	m_Solid = 0;
	m_Thickness = 16.0;
	m_XSizeBot = 128.0;
	m_XSizeTop = 128.0;
	m_ZSizeBot = 128.0;
	m_ZSizeTop = 128.0;
	//}}AFX_DATA_INIT

	// we want to load up our bitmaps
	mHollowBitmap.LoadBitmap(IDB_HOLLOWBOX );
	mSolidBitmap.LoadBitmap(IDB_SOLIDBOX );

}

CCreateBoxDialog::~CCreateBoxDialog()
{
	// delete our stupid bitmaps
	mHollowBitmap.DeleteObject();
	mSolidBitmap.DeleteObject();
}


void CCreateBoxDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateBoxDialog)
	DDX_Control(pDX, IDC_PICTURE, m_Picture);
	DDX_Text(pDX, IDC_YSIZE, m_YSize);
	DDX_Radio(pDX, IDC_SOLID, m_Solid);
	DDX_Text(pDX, IDC_THICKNESS, m_Thickness);
	DDX_Text(pDX, IDC_XSIZEBOT, m_XSizeBot);
	DDX_Text(pDX, IDC_XSIZETOP, m_XSizeTop);
	DDX_Text(pDX, IDC_ZSIZEBOT, m_ZSizeBot);
	DDX_Text(pDX, IDC_ZSIZETOP, m_ZSizeTop);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateBoxDialog, CDialog)
	//{{AFX_MSG_MAP(CCreateBoxDialog)
	ON_BN_CLICKED(IDC_SOLID, OnSolid)
	ON_BN_CLICKED(IDC_HOLLOW, OnHollow)
	ON_BN_CLICKED(IDC_Defaults, OnDefaults)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateBoxDialog message handlers

//===========================================================
// This function when called will start up the dialog
// and then make the actual box
//===========================================================
void CCreateBoxDialog::DoDialog(CThredBrush* Brush)
{
	// do nothing because we haven't been a happy camper
	if(DoModal() != IDOK )
		return;

	// since we succesfully came out of the function we
	// want to actually create the box
	CreateBox(Brush );
}

//===========================================================
// This function get's rid of whatever is in the brush
// and sticks a box with our current variables in there
// The reason we check m_Solid for 0 is that this is an
// index into which control is select in the group
//===========================================================
void CCreateBoxDialog::CreateBox(CThredBrush* Brush)
{
	// Boxes have maximum 16 verts.
	ThredPoint	  Verts[16];
	ThredPolygon* Polygons;

    CThredBrush*  tempBrush;

    tempBrush = new CThredBrush;
    *tempBrush = *Brush;

	// check if we are solid or not
	if(m_Solid != 0 )
    {
		tempBrush->AllocateBrushPolygons(12 );
		tempBrush->AllocateBrushPolyTextures(12 );
    }
	else
    {
		tempBrush->AllocateBrushPolygons(6 );
		tempBrush->AllocateBrushPolyTextures(6 );
    }

	// get our pointer
	Polygons = tempBrush->mPolygons;

	// go through a loop and init the number of verts
	// for each gon
	for(int CurrentPoly = 0; CurrentPoly < tempBrush->mNumberOfPolygons; CurrentPoly++ )
    {
		Polygons[CurrentPoly].NumberOfPoints = 4;
        tempBrush->mTextureID[CurrentPoly].TextureID = atoi(tempBrush->mName);
        tempBrush->mTextureID[CurrentPoly].TextureShift.x = 0;
        tempBrush->mTextureID[CurrentPoly].TextureShift.y = 0;
        tempBrush->mTextureID[CurrentPoly].TextureFlipH = 0;
        tempBrush->mTextureID[CurrentPoly].TextureFlipV = 0;
        tempBrush->mTextureID[CurrentPoly].TextureRotate = 0.F;
    }

	// now we actually want to make the box
	// Vertices 0 to 3 are the 4 corners of the top face
	Verts[0].X = -(m_XSizeTop/2);
	Verts[0].Y = (m_YSize/2);
	Verts[0].Z = -(m_ZSizeTop/2);
	Verts[1].X = -(m_XSizeTop/2);
	Verts[1].Y = (m_YSize/2);
	Verts[1].Z = (m_ZSizeTop/2);
	Verts[2].X = (m_XSizeTop/2);
	Verts[2].Y = (m_YSize/2);
	Verts[2].Z = (m_ZSizeTop/2);
	Verts[3].X = (m_XSizeTop/2);
	Verts[3].Y = (m_YSize/2);
	Verts[3].Z = -(m_ZSizeTop/2);

	// Vertices 4 to 7 are the 4 corners of the bottom face
	Verts[4].X = -(m_XSizeBot/2);
	Verts[4].Y = -(m_YSize/2);
	Verts[4].Z = -(m_ZSizeBot/2);
	Verts[5].X = (m_XSizeBot/2);
	Verts[5].Y = -(m_YSize/2);
	Verts[5].Z = -(m_ZSizeBot/2);
	Verts[6].X = (m_XSizeBot/2);
	Verts[6].Y = -(m_YSize/2);
	Verts[6].Z = (m_ZSizeBot/2);
	Verts[7].X = -(m_XSizeBot/2);
	Verts[7].Y = -(m_YSize/2);
	Verts[7].Z = (m_ZSizeBot/2);

	// Create the polygons		
	// Top
	Polygons[0].Points[0] = Verts[0];
	Polygons[0].Points[1] = Verts[1];
	Polygons[0].Points[2] = Verts[2];
	Polygons[0].Points[3] = Verts[3];

	// Bottom
	Polygons[1].Points[0] = Verts[4];
	Polygons[1].Points[1] = Verts[5];
	Polygons[1].Points[2] = Verts[6];
	Polygons[1].Points[3] = Verts[7];

	// Front
	Polygons[2].Points[0] = Verts[1];
	Polygons[2].Points[1] = Verts[7];
	Polygons[2].Points[2] = Verts[6];
	Polygons[2].Points[3] = Verts[2];

	// Back
	Polygons[3].Points[0] = Verts[0];
	Polygons[3].Points[1] = Verts[3];
	Polygons[3].Points[2] = Verts[5];
	Polygons[3].Points[3] = Verts[4];

	// Left
	Polygons[4].Points[0] = Verts[0];
	Polygons[4].Points[1] = Verts[4];
	Polygons[4].Points[2] = Verts[7];
	Polygons[4].Points[3] = Verts[1];

	// Right
	Polygons[5].Points[0] = Verts[3];
	Polygons[5].Points[1] = Verts[2];
	Polygons[5].Points[2] = Verts[6];
	Polygons[5].Points[3] = Verts[5];

	// check if we are solid or not
	if(m_Solid != 0 ) {

		// now we actually want to make the box
		// Vertices 8 to 11 are the 4 corners of the top face
		// of the inside
		Verts[8].X = -(m_XSizeTop/2) + m_Thickness;
		Verts[8].Y = (m_YSize/2) - m_Thickness;
		Verts[8].Z = -(m_ZSizeTop/2) + m_Thickness;
		Verts[9].X = -(m_XSizeTop/2) + m_Thickness;
		Verts[9].Y = (m_YSize/2) - m_Thickness;
		Verts[9].Z = (m_ZSizeTop/2) - m_Thickness;
		Verts[10].X = (m_XSizeTop/2) - m_Thickness;
		Verts[10].Y = (m_YSize/2) - m_Thickness;
		Verts[10].Z = (m_ZSizeTop/2) - m_Thickness;
		Verts[11].X = (m_XSizeTop/2) - m_Thickness;
		Verts[11].Y = (m_YSize/2) - m_Thickness;
		Verts[11].Z = -(m_ZSizeTop/2) + m_Thickness;

		// Vertices 12 to 15 are the 4 corners of the bottom face
		// of the inside gon.
		Verts[12].X = -(m_XSizeBot/2) + m_Thickness;
		Verts[12].Y = -(m_YSize/2) + m_Thickness;
		Verts[12].Z = -(m_ZSizeBot/2) + m_Thickness;
		Verts[13].X = (m_XSizeBot/2) - m_Thickness;
		Verts[13].Y = -(m_YSize/2) + m_Thickness;
		Verts[13].Z = -(m_ZSizeBot/2) + m_Thickness;
		Verts[14].X = (m_XSizeBot/2) - m_Thickness;
		Verts[14].Y = -(m_YSize/2) + m_Thickness;
		Verts[14].Z = (m_ZSizeBot/2) - m_Thickness;
		Verts[15].X = -(m_XSizeBot/2) + m_Thickness;
		Verts[15].Y = -(m_YSize/2) + m_Thickness;
		Verts[15].Z = (m_ZSizeBot/2) - m_Thickness;


		// Create the polygons		
		// Top
		Polygons[6].Points[3] = Verts[8];
		Polygons[6].Points[2] = Verts[9];
		Polygons[6].Points[1] = Verts[10];
		Polygons[6].Points[0] = Verts[11];

		// Bottom
		Polygons[7].Points[3] = Verts[12];
		Polygons[7].Points[2] = Verts[13];
		Polygons[7].Points[1] = Verts[14];
		Polygons[7].Points[0] = Verts[15];

		// Front
		Polygons[8].Points[3] = Verts[9];
		Polygons[8].Points[2] = Verts[15];
		Polygons[8].Points[1] = Verts[14];
		Polygons[8].Points[0] = Verts[10];

		// Back
		Polygons[9].Points[3] = Verts[8];
		Polygons[9].Points[2] = Verts[11];
		Polygons[9].Points[1] = Verts[13];
		Polygons[9].Points[0] = Verts[12];

		// Left
		Polygons[10].Points[3] = Verts[8];
		Polygons[10].Points[2] = Verts[12];
		Polygons[10].Points[1] = Verts[15];
		Polygons[10].Points[0] = Verts[9];

		// Right
		Polygons[11].Points[3] = Verts[11];
		Polygons[11].Points[2] = Verts[10];
		Polygons[11].Points[1] = Verts[14];
		Polygons[11].Points[0] = Verts[13];
	}

    //tempBrush->SplitBrushPolygons (Brush);
    *Brush = *tempBrush;

    // Set this stuff up for default param placing in dialog box...
    Brush->BrushType = CThredBrush::BOX_BRUSH;
  	Brush->BrushSpecifics.box.m_YSize = m_YSize;
	Brush->BrushSpecifics.box.m_Solid = m_Solid;
	Brush->BrushSpecifics.box.m_Thickness = m_Thickness;
	Brush->BrushSpecifics.box.m_XSizeBot = m_XSizeBot;
	Brush->BrushSpecifics.box.m_XSizeTop = m_XSizeTop;
	Brush->BrushSpecifics.box.m_ZSizeBot = m_ZSizeBot;
	Brush->BrushSpecifics.box.m_ZSizeTop = m_ZSizeTop;
	
    delete tempBrush;
}

void CCreateBoxDialog::OnSolid() 
{
	m_Picture.SetBitmap(mSolidBitmap );
}

void CCreateBoxDialog::OnHollow() 
{
	m_Picture.SetBitmap(mHollowBitmap );	
}

BOOL CCreateBoxDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if(m_Solid == 0 )
		m_Picture.SetBitmap(mSolidBitmap );
	else
		m_Picture.SetBitmap(mHollowBitmap );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCreateBoxDialog::Serialize(CArchive& ar) 
{
	if (ar.IsStoring())
	{	
	}
	else
	{	// loading code
	}
}

void CCreateBoxDialog::OnDefaults() 
{
	m_Picture.SetBitmap(mSolidBitmap );
	m_XSizeTop = 128.0;
	m_ZSizeTop = 128.0;
	m_XSizeBot = 128.0;
	m_ZSizeBot = 128.0;
	m_YSize = 128.0;
	m_Solid = 0;
	m_Thickness = 16.0;
	UpdateData(FALSE);
}

void CCreateBoxDialog::OnOK() 
{
    CString strMessage;
    char    data[10];
    double   val;

	// Verify that none of the sizes are < 1
    ((CEdit*)GetDlgItem(IDC_XSIZETOP))->GetLine(0, data, 10); 
    val = atof(data);

    if (val <= 0.0)
    {
      strMessage.Format("Top X Size Must Be > 0");
      AfxMessageBox(strMessage, MB_OK, 0);
	  ((CEdit*)GetDlgItem(IDC_XSIZETOP))->SetFocus();
	  ((CEdit*)GetDlgItem(IDC_XSIZETOP))->SetSel(0, -1, FALSE);
      return;
    }

    ((CEdit*)GetDlgItem(IDC_XSIZEBOT))->GetLine(0, data, 10); 
    val = atof(data);

    if (val <= 0.0)
    {
      strMessage.Format("Bottom X Size Must Be > 0");
      AfxMessageBox(strMessage, MB_OK, 0);
	  ((CEdit*)GetDlgItem(IDC_XSIZEBOT))->SetFocus();
	  ((CEdit*)GetDlgItem(IDC_XSIZEBOT))->SetSel(0, -1, FALSE);
      return;
    }

    ((CEdit*)GetDlgItem(IDC_ZSIZETOP))->GetLine(0, data, 10); 
    val = atof(data);

    if (val <= 0.0)
    {
      strMessage.Format("Top Z Size Must Be > 0");
      AfxMessageBox(strMessage, MB_OK, 0);
	  ((CEdit*)GetDlgItem(IDC_ZSIZETOP))->SetFocus();
	  ((CEdit*)GetDlgItem(IDC_ZSIZETOP))->SetSel(0, -1, FALSE);
      return;
    }

    ((CEdit*)GetDlgItem(IDC_ZSIZEBOT))->GetLine(0, data, 10); 
    val = atof(data);

    if (val <= 0.0)
    {
      strMessage.Format("Bottom Z Size Must Be > 0");
      AfxMessageBox(strMessage, MB_OK, 0);
	  ((CEdit*)GetDlgItem(IDC_ZSIZEBOT))->SetFocus();
	  ((CEdit*)GetDlgItem(IDC_ZSIZEBOT))->SetSel(0, -1, FALSE);
      return;
    }

    ((CEdit*)GetDlgItem(IDC_YSIZE))->GetLine(0, data, 10); 
    val = atof(data);

    if (val <= 0.0)
    {
      strMessage.Format("Y Size Must Be > 0");
      AfxMessageBox(strMessage, MB_OK, 0);
	  ((CEdit*)GetDlgItem(IDC_YSIZE))->SetFocus();
	  ((CEdit*)GetDlgItem(IDC_YSIZE))->SetSel(0, -1, FALSE);
      return;
    }

	
	CDialog::OnOK();
}
