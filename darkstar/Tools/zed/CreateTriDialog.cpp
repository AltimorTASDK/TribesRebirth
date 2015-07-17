// CreateTriDialog.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "CreateTriDialog.h"
#include "ThredBrush.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCreateTriDialog dialog


CCreateTriDialog::CCreateTriDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateTriDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateTriDialog)
	m_Solid = 0;
	m_Thickness = 16.0;
	m_XSizeBot = 128.0;
	m_YSize = 128.0;
	m_ZSizeBot = 128.0;
	m_ZSizeTop = 128.0;
	//}}AFX_DATA_INIT

    // we want to load up our bitmaps
	mHollowBitmap.LoadBitmap(IDB_HOLLOWTRIANGLE );
	mSolidBitmap.LoadBitmap(IDB_SOLIDTRIANGLE );

}

CCreateTriDialog::~CCreateTriDialog()
{
	// delete our stupid bitmaps
	mHollowBitmap.DeleteObject();
	mSolidBitmap.DeleteObject();
}

void CCreateTriDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateTriDialog)
	DDX_Control(pDX, IDC_PICTURE, m_Picture);
	DDX_Radio(pDX, IDC_TRISOLID, m_Solid);
	DDX_Text(pDX, IDC_TRITHICKNESS, m_Thickness);
	DDX_Text(pDX, IDC_TRIXSIZEBOT, m_XSizeBot);
	DDX_Text(pDX, IDC_TRIYSIZE, m_YSize);
	DDX_Text(pDX, IDC_TRIZSIZEBOT, m_ZSizeBot);
	DDX_Text(pDX, IDC_TRIZSIZETOP, m_ZSizeTop);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateTriDialog, CDialog)
	//{{AFX_MSG_MAP(CCreateTriDialog)
	ON_BN_CLICKED(IDC_Defaults, OnDefaults)
	ON_BN_CLICKED(IDC_TRIHOLLOW, OnTrihollow)
	ON_BN_CLICKED(IDC_TRISOLID, OnTrisolid)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateTriDialog message handlers

void CCreateTriDialog::Serialize(CArchive& ar) 
{
	if (ar.IsStoring())
	{	// storing code
	}
	else
	{	// loading code
	}
}

BOOL CCreateTriDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if(m_Solid == 0 )
		m_Picture.SetBitmap(mSolidBitmap );
	else
		m_Picture.SetBitmap(mHollowBitmap );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCreateTriDialog::OnDefaults() 
{
	m_Picture.SetBitmap(mSolidBitmap);
	m_ZSizeTop = 128.0;
	m_XSizeBot = 128.0;
	m_ZSizeBot = 128.0;
	m_YSize = 128.0;
	m_Solid = 0;
	m_Thickness = 16.0;
	UpdateData(FALSE);
}

void CCreateTriDialog::OnTrihollow() 
{
	m_Picture.SetBitmap(mHollowBitmap );	
}

void CCreateTriDialog::OnTrisolid() 
{
	m_Picture.SetBitmap(mSolidBitmap );	
}

void CCreateTriDialog::OnOK() 
{
    CString strMessage;
    char    data[10];
    double   val;

    ((CEdit*)GetDlgItem(IDC_TRIXSIZEBOT))->GetLine(0, data, 10); 
    val = atof(data);

    if (val <= 0.0)
    {
      strMessage.Format("Bottom X Size Must Be > 0");
      AfxMessageBox(strMessage, MB_OK, 0);
	  ((CEdit*)GetDlgItem(IDC_TRIXSIZEBOT))->SetFocus();
	  ((CEdit*)GetDlgItem(IDC_TRIXSIZEBOT))->SetSel(0, -1, FALSE);
      return;
    }

    ((CEdit*)GetDlgItem(IDC_TRIZSIZETOP))->GetLine(0, data, 10); 
    val = atof(data);

    if (val <= 0.0)
    {
      strMessage.Format("Top Z Size Must Be > 0");
      AfxMessageBox(strMessage, MB_OK, 0);
	  ((CEdit*)GetDlgItem(IDC_TRIZSIZETOP))->SetFocus();
	  ((CEdit*)GetDlgItem(IDC_TRIZSIZETOP))->SetSel(0, -1, FALSE);
      return;
    }

    ((CEdit*)GetDlgItem(IDC_TRIZSIZEBOT))->GetLine(0, data, 10); 
    val = atof(data);

    if (val <= 0.0)
    {
      strMessage.Format("Bottom Z Size Must Be > 0");
      AfxMessageBox(strMessage, MB_OK, 0);
	  ((CEdit*)GetDlgItem(IDC_TRIZSIZEBOT))->SetFocus();
	  ((CEdit*)GetDlgItem(IDC_TRIZSIZEBOT))->SetSel(0, -1, FALSE);
      return;
    }

    ((CEdit*)GetDlgItem(IDC_TRIYSIZE))->GetLine(0, data, 10); 
    val = atof(data);

    if (val <= 0.0)
    {
      strMessage.Format("Y Size Must Be > 0");
      AfxMessageBox(strMessage, MB_OK, 0);
	  ((CEdit*)GetDlgItem(IDC_TRIYSIZE))->SetFocus();
	  ((CEdit*)GetDlgItem(IDC_TRIYSIZE))->SetSel(0, -1, FALSE);
      return;
    }
	
	CDialog::OnOK();
}

//===========================================================
// This function when called will start up the dialog
// and then make the actual box
//===========================================================
void CCreateTriDialog::DoDialog(CThredBrush* Brush)
{
	// do nothing because we haven't been a happy camper
	if(DoModal() != IDOK )
		return;

	// since we succesfully came out of the function we
	// want to actually create the box
	CreateTriangle(Brush);
}

//===========================================================
// This function get's rid of whatever is in the brush
// and sticks a box with our current variables in there
// The reason we check m_Solid for 0 is that this is an
// index into which control is select in the group
//===========================================================
void CCreateTriDialog::CreateTriangle(CThredBrush* Brush)
{
	// Triangles have maximum 12 verts.
	ThredPoint	  Verts[12];
	ThredPolygon* Polygons;
    double        ztopratio, zbotratio;

    // check if we are solid or not
	if(m_Solid != 0 )
    {
		Brush->AllocateBrushPolygons(10 );
		Brush->AllocateBrushPolyTextures(10 );
    }
	else
    {
		Brush->AllocateBrushPolygons(5 );
		Brush->AllocateBrushPolyTextures(5 );
    }

	// get our pointer
	Polygons = Brush->mPolygons;

	// init the number of verts for each gon
    if (m_Solid != 0)
    {
      // DPW - init texture ids...
      for (int i = 0; i < Brush->mNumberOfPolygons; i++)
      {
        Brush->mTextureID[i].TextureID = atoi(Brush->mName);
        Brush->mTextureID[i].TextureShift.x = 0;
        Brush->mTextureID[i].TextureShift.y = 0;
        Brush->mTextureID[i].TextureFlipH = 0;
        Brush->mTextureID[i].TextureFlipV = 0;
        Brush->mTextureID[i].TextureRotate = 0.F;
      }

      Polygons[0].NumberOfPoints = 3;
      Polygons[1].NumberOfPoints = 3;
      Polygons[2].NumberOfPoints = 4;
      Polygons[3].NumberOfPoints = 4;
      Polygons[4].NumberOfPoints = 4;

      Polygons[5].NumberOfPoints = 3;
      Polygons[6].NumberOfPoints = 3;
      Polygons[7].NumberOfPoints = 4;
      Polygons[8].NumberOfPoints = 4;
      Polygons[9].NumberOfPoints = 4;
    } else {
      // DPW - init texture ids...
      for (int i = 0; i < 5; i++)
      {
        Brush->mTextureID[i].TextureID = 0;
        Brush->mTextureID[i].TextureShift.x = 0;
        Brush->mTextureID[i].TextureShift.y = 0;
        Brush->mTextureID[i].TextureFlipH = 0;
        Brush->mTextureID[i].TextureFlipV = 0;
        Brush->mTextureID[i].TextureRotate = 0.F;
      }

      Polygons[0].NumberOfPoints = 3;
      Polygons[1].NumberOfPoints = 3;
      Polygons[2].NumberOfPoints = 4;
      Polygons[3].NumberOfPoints = 4;
      Polygons[4].NumberOfPoints = 4;
   }

    if (m_ZSizeBot > m_ZSizeTop)
    {
      ztopratio = (m_ZSizeTop / m_ZSizeBot);
      zbotratio = 1.0;
      ztextureratio = m_ZSizeBot / 256.0;
    }

    if (m_ZSizeTop > m_ZSizeBot)
    {
      ztopratio = 1.0;
      zbotratio = (m_ZSizeBot / m_ZSizeTop);
      ztextureratio = m_ZSizeTop / 256.0;
    }

    if (m_ZSizeTop == m_ZSizeBot)
    {
      ztopratio = 1.0;
      zbotratio = 1.0;
      ztextureratio = m_ZSizeTop / 256.0;
    }

    xtextureratio = m_XSizeBot / 256.0;
    ytextureratio = m_YSize / 256.0;

	// Vertices 0 to 2 are the 3 vertices of the front face
	Verts[0].X = -(m_XSizeBot/2);
	Verts[0].Y = -(m_YSize/2);
	Verts[0].Z = -(m_ZSizeBot/2);
	Verts[1].X =   0.0;
	Verts[1].Y =  (m_YSize/2);
	Verts[1].Z = -(m_ZSizeTop/2);
	Verts[2].X =  (m_XSizeBot/2);
	Verts[2].Y = -(m_YSize/2);
	Verts[2].Z = -(m_ZSizeBot/2);

	// Vertices 3 to 5 are the 3 vertices of the back face
	Verts[3].X = -(m_XSizeBot/2);
	Verts[3].Y = -(m_YSize/2);
	Verts[3].Z =  (m_ZSizeBot/2);
	Verts[4].X =   0.0;
	Verts[4].Y =  (m_YSize/2);
	Verts[4].Z =  (m_ZSizeTop/2);
	Verts[5].X =  (m_XSizeBot/2);
	Verts[5].Y = -(m_YSize/2);
	Verts[5].Z =  (m_ZSizeBot/2);

	// Create the polygons		
	// Front
	Polygons[0].Points[0] = Verts[0];
	Polygons[0].Points[1] = Verts[1];
	Polygons[0].Points[2] = Verts[2];

	// Back
	Polygons[1].Points[0] = Verts[5];
	Polygons[1].Points[1] = Verts[4];
	Polygons[1].Points[2] = Verts[3];

	// Bottom
	Polygons[2].Points[0] = Verts[2];
	Polygons[2].Points[1] = Verts[5];
	Polygons[2].Points[2] = Verts[3];
	Polygons[2].Points[3] = Verts[0];

	// Right
	Polygons[3].Points[0] = Verts[1];
	Polygons[3].Points[1] = Verts[4];
	Polygons[3].Points[2] = Verts[5];
	Polygons[3].Points[3] = Verts[2];

	// Left
	Polygons[4].Points[0] = Verts[4];
	Polygons[4].Points[1] = Verts[1];
	Polygons[4].Points[2] = Verts[0];
	Polygons[4].Points[3] = Verts[3];

	// check if we are solid or not
	if(m_Solid != 0 )
    {
		// Vertices 6 to 8 are the 3 vertices of the front face
		// of the inside    
        Verts[6].X = -(m_XSizeBot/2) + m_Thickness;
        Verts[6].Y = -(m_YSize/2) + m_Thickness;
        Verts[6].Z = -(m_ZSizeBot/2) + m_Thickness;
        Verts[7].X =   0.0;
        Verts[7].Y =  (m_YSize/2) - m_Thickness;
        Verts[7].Z = -(m_ZSizeTop/2) + m_Thickness;
        Verts[8].X =  (m_XSizeBot/2) - m_Thickness;
        Verts[8].Y = -(m_YSize/2) + m_Thickness;
        Verts[8].Z = -(m_ZSizeBot/2) + m_Thickness;

		// Vertices 9 to 11 are the 3 vertices of the back face
		// of the inside    
        Verts[9].X =  -(m_XSizeBot/2) + m_Thickness;
        Verts[9].Y =  -(m_YSize/2) + m_Thickness;
        Verts[9].Z =   (m_ZSizeBot/2) - m_Thickness;
        Verts[10].X =   0.0;
        Verts[10].Y =  (m_YSize/2) - m_Thickness;
        Verts[10].Z =  (m_ZSizeTop/2) - m_Thickness;
        Verts[11].X =  (m_XSizeBot/2) - m_Thickness;
        Verts[11].Y = -(m_YSize/2) + m_Thickness;
        Verts[11].Z =  (m_ZSizeBot/2) - m_Thickness;


		// Create the polygons		
		// Front
		Polygons[5].Points[0] = Verts[8];
		Polygons[5].Points[1] = Verts[7];
		Polygons[5].Points[2] = Verts[6];

		// Back
		Polygons[6].Points[0] = Verts[9];
		Polygons[6].Points[1] = Verts[10];
		Polygons[6].Points[2] = Verts[11];

		// Bottom
		Polygons[7].Points[0] = Verts[6];
		Polygons[7].Points[1] = Verts[9];
		Polygons[7].Points[2] = Verts[11];
		Polygons[7].Points[3] = Verts[8];

		// Right
		Polygons[8].Points[0] = Verts[8];
		Polygons[8].Points[1] = Verts[11];
		Polygons[8].Points[2] = Verts[10];
		Polygons[8].Points[3] = Verts[7];

		// Left
		Polygons[9].Points[3] = Verts[10];
		Polygons[9].Points[2] = Verts[7];
		Polygons[9].Points[1] = Verts[6];
		Polygons[9].Points[0] = Verts[9];
	}

#if 0
    // DPW - Added this Normal calculation for lighting
	for(int i = 0; i < Brush->mNumberOfPolygons; i++ )
    {
      Point3F vector1;
      Point3F vector2;

      vector1.x = Polygons[i].Points[1].X - Polygons[i].Points[0].X;
      vector1.y = Polygons[i].Points[1].Y - Polygons[i].Points[0].Y;
      vector1.z = Polygons[i].Points[1].Z - Polygons[i].Points[0].Z;

      vector2.x = Polygons[i].Points[1].X - Polygons[i].Points[2].X,
      vector2.y = Polygons[i].Points[1].Y - Polygons[i].Points[2].Y,
      vector2.z = Polygons[i].Points[1].Z - Polygons[i].Points[2].Z;
      m_cross (vector1, vector2, &(Polygons[i].Normal));
    }
#endif

    // Set this stuff up for default param placing in dialog box...
    Brush->BrushType = CThredBrush::TRI_BRUSH;
  	Brush->BrushSpecifics.triangle.m_Solid = m_Solid;
	Brush->BrushSpecifics.triangle.m_Thickness = m_Thickness;
	Brush->BrushSpecifics.triangle.m_XSizeBot = m_XSizeBot;
	Brush->BrushSpecifics.triangle.m_YSize = m_YSize;
	Brush->BrushSpecifics.triangle.m_ZSizeBot = m_ZSizeBot;
	Brush->BrushSpecifics.triangle.m_ZSizeTop = m_ZSizeTop;
}
