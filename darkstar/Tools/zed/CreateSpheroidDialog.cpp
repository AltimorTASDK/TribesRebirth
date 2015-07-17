// CreateSpheroidDialog.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>
#include <float.h>
#include "THRED.h"
#include "CreateSpheroidDialog.h"
#include "ThredBrush.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCreateSpheroidDialog dialog

CCreateSpheroidDialog::CCreateSpheroidDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateSpheroidDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateSpheroidDialog)
	m_HorizontalBands = 4;
	m_VerticalBands = 8;
	m_XSize = 256.0;
	m_YSize = 256.0;
	m_ZSize = 256.0;
	m_Solid = 0;
	m_Thickness = 16;
	//}}AFX_DATA_INIT

	mSolidSphere.LoadBitmap(IDB_SOLIDSPHERE);
	mHollowSphere.LoadBitmap(IDB_HOLLOWSPHERE);

    m_HalfSphere = FALSE;
}

CCreateSpheroidDialog::~CCreateSpheroidDialog()
{
	mSolidSphere.DeleteObject();
	mHollowSphere.DeleteObject();
}

void CCreateSpheroidDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateSpheroidDialog)
	DDX_Control(pDX, IDC_PICTURE, m_Picture);
	DDX_Text(pDX, IDC_HORSTRIPES, m_HorizontalBands);
	DDV_MinMaxInt(pDX, m_HorizontalBands, 2, 100);
	DDX_Text(pDX, IDC_VERTSTRIPES, m_VerticalBands);
	DDV_MinMaxInt(pDX, m_VerticalBands, 3, 100);
	DDX_Text(pDX, IDC_XSIZE, m_XSize);
	DDX_Text(pDX, IDC_YSIZE, m_YSize);
	DDX_Text(pDX, IDC_ZSIZE, m_ZSize);
	DDX_Radio(pDX, IDC_SOLID, m_Solid);
	DDX_Text(pDX, IDC_THICKNESS, m_Thickness);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCreateSpheroidDialog, CDialog)
	//{{AFX_MSG_MAP(CCreateSpheroidDialog)
	ON_BN_CLICKED(IDC_HOLLOW, OnHollow)
	ON_BN_CLICKED(IDC_SOLID, OnSolid)
	ON_BN_CLICKED(IDC_Defaults, OnDefaults)
	ON_BN_CLICKED(IDC_HALFSPHERERADIO, OnHalfsphereradio)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateSpheroidDialog message handlers

//===================================================
// This function runs the dialog box and then
// makes the call to create the sphereoid
//===================================================
void CCreateSpheroidDialog::DoDialog(CThredBrush* Brush)
{
	// make the box go
	if(DoModal() != IDOK )
		return;

	// create the thing
	CreateSpheroid(Brush );
}


void CCreateSpheroidDialog::OnHollow() 
{
	m_Picture.SetBitmap(mHollowSphere );
}

void CCreateSpheroidDialog::OnSolid() 
{
	m_Picture.SetBitmap(mSolidSphere );
}

BOOL CCreateSpheroidDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if(m_Solid == 0 )
		m_Picture.SetBitmap(mSolidSphere);
	else
		m_Picture.SetBitmap(mHollowSphere);

    if (m_HalfSphere)
    {
      ((CButton*)GetDlgItem(IDC_HALFSPHERERADIO))->SetCheck(1);
    } else {
      ((CButton*)GetDlgItem(IDC_HALFSPHERERADIO))->SetCheck(0);
    }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCreateSpheroidDialog::CreateSpheroid(CThredBrush* Brush)
{
#if 1
	// my stuff
	int NumberOfVerts, NumberOfPolygons;
	ThredPolygon* Polygons;
	ThredPoint* Verts;
	ThredPoint* Verts2;
	CTransformMatrix ZRotation, YRotation;
	ThredPoint Current, Final;
	int VertexCount = 0;
	int PolygonCount = 0;
	int HBand, VBand;
	double CurrentWidth, CurrentDepth;
	double EllipseZ;
    double  internalval;

	// Make sure we have enough bands
	if(m_HorizontalBands < 2 || m_VerticalBands < 3)
		return;

	// make sure one is more than the other
	if(m_VerticalBands < m_HorizontalBands)
    {
		AfxMessageBox("There must be more vertical bands!");
		DoDialog(Brush);
		return;
	}

	// Get the number of vertices and polygons
	NumberOfVerts = 2 + ((m_HorizontalBands - 1) * m_VerticalBands);
	//NumberOfPolygons = (2 * m_VerticalBands) + (((m_HorizontalBands - 2) * m_VerticalBands));
	NumberOfPolygons = m_HorizontalBands * m_VerticalBands;
    if (m_HalfSphere)
    {
      //NumberOfPolygons /= 2;
      NumberOfPolygons = ((m_HorizontalBands)/2) * m_VerticalBands;
      NumberOfPolygons++;
    }

	// If we are doing a HOLLOW sphere then we have to double
	// the amount of verts and polys
	if(m_Solid != 0)
    {
		NumberOfVerts *= 2;
		NumberOfPolygons *= 2;
	}

	// allocate the number of verts we need
	Verts = new ThredPoint[NumberOfVerts];
	if(!Verts)
    {
	  return;
    }

	// allocate the number of gons that we need
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

	// get our pointer
	Polygons = Brush->mPolygons;

	// Create the 2 rotation matrices
	ZRotation.ClearMatrix();
	ZRotation.SetRoll((M_TOTAL_DEGREES/2) / m_HorizontalBands);
	YRotation.ClearMatrix();
	YRotation.SetYaw(M_TOTAL_DEGREES / m_VerticalBands );

	// Start with the top of sphere
	Current.X = 0;
	Current.Y = ( m_YSize / 2 );
	Current.Z = 0;

   ThredPoint origPoint;
   ThredPoint offset;
   double rotation;
   
   origPoint.X = ( m_XSize / 2 );
   origPoint.Y = ( m_YSize / 2 );
   origPoint.Z = ( m_ZSize / 2 );
   
    // ----------------- LENGTH = Y ---------------------

	// Store the peak of the sphere
	Verts[VertexCount++] = Current;

	for(HBand = 1;HBand < m_HorizontalBands; HBand++)
   {
      // calculate the offsets for this band
      rotation = ( ( 2.0 * M_PI_VALUE ) / ( m_HorizontalBands * 2 ) ) * HBand;
      
      offset.Y = origPoint.Y * cos( rotation );
      offset.X = origPoint.X * sin( rotation );
      offset.Z = origPoint.Z * sin( rotation );

//         // calc the x coord         
//		// Rotate the point around the Z first to get the next horizontal band
//		ZRotation.ApplyMatrix(Current );
//
//		// Get a working vector
//		Final = Current;
//
//        internalval = ((m_XSize/2)*(m_XSize/2))
//				* (1.0 - (Final.Y*Final.Y)
//				/ ((m_YSize/2)*(m_YSize/2) )) ;
//
//        if (internalval < 0)
//        {
//          CurrentWidth = 0.0;
//        } else {
//		  // Get the current band's ellipse dimensions
//		  CurrentWidth = sqrt(((m_XSize/2)*(m_XSize/2))
//				* (1.0 - (Final.Y*Final.Y)
//				/ ((m_YSize/2)*(m_YSize/2) )) ) * 2;
//        }
//
//        CurrentDepth = CurrentWidth * m_ZSize / m_XSize;
//
//		// Get the current X value on the band's ellipse
//		Final.X = CurrentWidth / 2;

		for(VBand = 0;VBand < m_VerticalBands;VBand++) 
      {
//			// Get the elliptical Z value
//			// (x^2/a^2) + (z^2/b^2) = 1
//			// z = sqrt(b^2(1 - x^2/a^2))
//          if (CurrentWidth == 0)
//          {
//            internalval = -1.0;
//          } else {
//            internalval = ((CurrentDepth/2)*(CurrentDepth/2))
//				* (1.0 - (Final.X*Final.X)
//				/ ((CurrentWidth/2)*(CurrentWidth/2) ));
//          }
//
//          if (internalval < 0)
//          {
//            EllipseZ = 0.0;
//          } else {
//			EllipseZ = sqrt(((CurrentDepth/2)*(CurrentDepth/2))
//				* (1.0 - (Final.X*Final.X)
//				/ ((CurrentWidth/2)*(CurrentWidth/2) )) );
//          }
//
//			// Check if we need to negate this thing
//			if(VBand > (m_VerticalBands/2))
//				EllipseZ = -EllipseZ;
//
//			// Store this vertex
//			Verts[VertexCount].X = Final.X;
//			Verts[VertexCount].Y = Final.Y;
//			Verts[VertexCount].Z = EllipseZ;

         // get the rotation
         rotation = ( ( 2.0 * M_PI_VALUE ) / m_VerticalBands ) * VBand;

         // calculate the start point to rotate around the axis
         Verts[ VertexCount ].X = offset.X * cos( rotation );
         Verts[ VertexCount ].Y = offset.Y;
         Verts[ VertexCount ].Z = offset.Z * sin( rotation );
      
			VertexCount++;

//			// Rotate the point around the Y to get the next vertical band
//			YRotation.ApplyMatrix(Final );
		}
	}

	// Finish with the bottom of sphere
	Current.X = 0;
	Current.Y = -(m_YSize / 2);
	Current.Z = 0;

	// Store the bottom of the sphere
	Verts[VertexCount++] = Current;

	// Generate the polygons-------------------------------------
	// Start with the top
	for(VBand = 0;VBand < m_VerticalBands;VBand++)
    {
		Polygons[PolygonCount].NumberOfPoints = 3;
		Polygons[PolygonCount].Points[0] = Verts[0];
		Polygons[PolygonCount].Points[1] = Verts[((1 + VBand) % m_VerticalBands) + 1];
		Polygons[PolygonCount].Points[2] = Verts[1 + VBand];
		PolygonCount++;
	}

	// Do the interior
	for(HBand = 1;HBand < (m_HorizontalBands - 1);HBand++)
    {
      if (((m_HalfSphere) && (HBand < (m_HorizontalBands /2))) || (!m_HalfSphere))
      {
		for(VBand = 0;VBand < m_VerticalBands;VBand++)
        {
			Polygons[PolygonCount].NumberOfPoints = 4;
			Polygons[PolygonCount].Points[0]
				= Verts[(((HBand - 1) * m_VerticalBands) + 1) + VBand];
			Polygons[PolygonCount].Points[1]
				= Verts[(((HBand - 1) * m_VerticalBands) + 1) + ((VBand + 1) % m_VerticalBands)];
			Polygons[PolygonCount].Points[2]
				= Verts[((HBand * m_VerticalBands) + 1) + ((VBand + 1) % m_VerticalBands)];
			Polygons[PolygonCount].Points[3]
				= Verts[((HBand * m_VerticalBands) + 1) + VBand];
			PolygonCount++;
		}
      }
	}

    if (!m_HalfSphere)
    {
	  // Now do the bottom
	  for(VBand = 0;VBand < m_VerticalBands;VBand++)
      {
		  Polygons[PolygonCount].NumberOfPoints = 3;
		  Polygons[PolygonCount].Points[0]
			  = Verts[1 + ((m_HorizontalBands-2) * m_VerticalBands) + VBand];
		  Polygons[PolygonCount].Points[1]
			  = Verts[1 + ((m_HorizontalBands-2) * m_VerticalBands)
			  + ((VBand + 1) % m_VerticalBands)];
		  Polygons[PolygonCount].Points[2]
			  = Verts[((m_HorizontalBands - 1) * m_VerticalBands) + 1];
		  PolygonCount++;
	  }
    } else {
      // Do the bottom of the half-sphere
      Polygons[PolygonCount].NumberOfPoints = m_VerticalBands;

      for (int basevert = 0; basevert < m_VerticalBands; basevert++)
      {
        Polygons[PolygonCount].Points[basevert] = Verts[((((m_HorizontalBands)/2)*m_VerticalBands)+1) - m_VerticalBands + basevert];
      }

      PolygonCount++;
    }

	// If we are doing a hollow sphere generate this second sphere
	if(m_Solid != 0)
    {
		// need the wallwidth
		double WallWidth = m_Thickness*2;

		Verts2 = &Verts[NumberOfVerts/2];

		// Start with the top of sphere
		Current.X = 0;
		Current.Y = ((m_YSize-WallWidth) / 2);
		Current.Z = 0;

		// ----------------- LENGTH = Y ---------------------

      if( ( m_Thickness >= ( m_XSize / 2 ) ) ||
         ( m_Thickness >= ( m_YSize / 2 ) ) ||
         ( m_Thickness >= ( m_ZSize / 2 ) ) )
         return;
         
      origPoint.X = ( m_XSize / 2 ) - m_Thickness;
      origPoint.Y = ( m_YSize / 2 ) - m_Thickness;
      origPoint.Z = ( m_ZSize / 2 ) - m_Thickness;
   
		// Store the top of the sphere
		Verts[VertexCount++] = Current;

		for(HBand = 1;HBand < m_HorizontalBands; HBand++) 
      {

         // calculate the offsets for this band
         rotation = ( ( 2.0 * M_PI_VALUE ) / ( m_HorizontalBands * 2 ) ) * HBand;
         
         offset.Y = origPoint.Y * cos( rotation );
         offset.X = origPoint.X * sin( rotation );
         offset.Z = origPoint.Z * sin( rotation );
      
//			// Rotate the point around the Z first to get the next horizontal band
//			ZRotation.ApplyMatrix(Current );
//
//			// Get a working vector
//			Final = Current;
//
//            if ((m_YSize-WallWidth) == 0)
//            {
//              internalval = 0.0;
//            } else {
//              internalval = (((m_XSize-WallWidth)/2)*((m_XSize-WallWidth)/2))
//					* (1.0 - (Final.Y*Final.Y)
//					/ (((m_YSize-WallWidth)/2)*((m_YSize-WallWidth)/2) )) ;
//            }
//
//            if (internalval < 0)
//            {
//              CurrentWidth = 0;
//            } else {
//			  // Get the current band's ellipse dimensions
//			  CurrentWidth = sqrt((((m_XSize-WallWidth)/2)*((m_XSize-WallWidth)/2))
//					* (1.0 - (Final.Y*Final.Y)
//					/ (((m_YSize-WallWidth)/2)*((m_YSize-WallWidth)/2) )) ) * 2;
//            }
//
//            if ((m_XSize-WallWidth) == 0)
//            {
//              CurrentDepth = 0;
//            } else {
//			  CurrentDepth = CurrentWidth * (m_ZSize-WallWidth) / (m_XSize-WallWidth);
//            }
//
//			// Get the current X value on the band's ellipse
//			Final.X = CurrentWidth / 2;

			for(VBand = 0;VBand < m_VerticalBands;VBand++)
         {
//              if (CurrentWidth == 0)
//              {
//                internalval = 0.0;
//              } else {
//                internalval = ((CurrentDepth/2)*(CurrentDepth/2))
//					* (1.0 - (Final.X*Final.X)
//					/ ((CurrentWidth/2)*(CurrentWidth/2) ));
//              }
//
//              if (internalval < 0)
//              {
//                EllipseZ = 0.0;
//              } else {
//				// Get the elliptical Z value
//				// (x^2/a^2) + (z^2/b^2) = 1
//				// z = sqrt(b^2(1 - x^2/a^2))
//				EllipseZ = sqrt(((CurrentDepth/2)*(CurrentDepth/2))
//					* (1.0 - (Final.X*Final.X)
//					/ ((CurrentWidth/2)*(CurrentWidth/2) )) );
//              }
//
//				// Check if we need to negate this thing
//				if(VBand > (m_VerticalBands/2))
//					EllipseZ = -EllipseZ;
//
//				// Store this vertex
//				Verts[VertexCount].X = Final.X;
//				Verts[VertexCount].Y = Final.Y;
//				Verts[VertexCount].Z = EllipseZ;

            // get the rotation
            rotation = ( ( 2.0 * M_PI_VALUE ) / m_VerticalBands ) * VBand;

            // calculate the start point to rotate around the axis
            Verts[ VertexCount ].X = offset.X * cos( rotation );
            Verts[ VertexCount ].Y = offset.Y;
            Verts[ VertexCount ].Z = offset.Z * sin( rotation );
         
				VertexCount++;

//				// Rotate the point around the Y to get the next vertical band
//				YRotation.ApplyMatrix(Final );
			}
		}

		// Finish with the bottom of sphere
		Current.X = 0;
		Current.Y = -((m_YSize-WallWidth) / 2);
		Current.Z = 0;

		// Store the bottom of the sphere
		Verts[VertexCount++] = Current;

        // DPW ---------------------------
        if (m_HalfSphere)
        {
          for (int ibasevert = 0; ibasevert < m_VerticalBands; ibasevert++)
          {
            Verts2[((((m_HorizontalBands)/2)*m_VerticalBands)+1) - m_VerticalBands + ibasevert].Y = 
                Verts[((((m_HorizontalBands)/2)*m_VerticalBands)+1) - m_VerticalBands + ibasevert].Y + m_Thickness; 
          }
        }
        // DPW ---------------------------

		// Generate the polygons

		// Start with the top
		for(VBand = 0;VBand < m_VerticalBands;VBand++)
        {
			Polygons[PolygonCount].NumberOfPoints = 3;
			Polygons[PolygonCount].Points[2] = Verts2[0];
			Polygons[PolygonCount].Points[1] = Verts2[((1 + VBand) % m_VerticalBands) + 1];
			Polygons[PolygonCount].Points[0] = Verts2[1 + VBand];
			PolygonCount++;
		}

		// Do the interior
		for(HBand = 1;HBand < (m_HorizontalBands - 1);HBand++)
        {
          if (((m_HalfSphere) && (HBand < (m_HorizontalBands /2))) || (!m_HalfSphere))
          {
			for(VBand = 0;VBand < m_VerticalBands;VBand++)
            {
				Polygons[PolygonCount].NumberOfPoints = 4;
				Polygons[PolygonCount].Points[3]
					= Verts2[(((HBand - 1) * m_VerticalBands) + 1) + VBand];
				Polygons[PolygonCount].Points[2]
					= Verts2[(((HBand - 1) * m_VerticalBands) + 1) + ((VBand + 1) % m_VerticalBands)];
				Polygons[PolygonCount].Points[1]
					= Verts2[((HBand * m_VerticalBands) + 1) + ((VBand + 1) % m_VerticalBands)];
				Polygons[PolygonCount].Points[0]
					= Verts2[((HBand * m_VerticalBands) + 1) + VBand];
				PolygonCount++;
			}
          }
		}

        if (!m_HalfSphere)
        {
		  // Now do the bottom
		  for(VBand = 0;VBand < m_VerticalBands;VBand++)
          {
			  Polygons[PolygonCount].NumberOfPoints = 3;
			  Polygons[PolygonCount].Points[2]
				  = Verts2[1 + ((m_HorizontalBands-2) * m_VerticalBands) + VBand];
			  Polygons[PolygonCount].Points[1]
				  = Verts2[1 + ((m_HorizontalBands-2) * m_VerticalBands)
				  + ((VBand + 1) % m_VerticalBands)];
			  Polygons[PolygonCount].Points[0]
				  = Verts2[((m_HorizontalBands - 1) * m_VerticalBands) + 1];
			  PolygonCount++;
		  }
        } else {
          // Do the bottom of the half-sphere
          Polygons[PolygonCount].NumberOfPoints = m_VerticalBands;

          for (int ibasevert = 0; ibasevert < m_VerticalBands; ibasevert++)
          {
            Polygons[PolygonCount].Points[m_VerticalBands - 1 - ibasevert] = Verts2[((((m_HorizontalBands)/2)*m_VerticalBands)+1) - m_VerticalBands + ibasevert];
          }

          PolygonCount++;
        }
	} // End hollow part...

	delete []Verts;

	// make sure we got the math right!
	_ASSERTE(PolygonCount == NumberOfPolygons );
	_ASSERTE(VertexCount == NumberOfVerts );

    Brush->BrushType = CThredBrush::SPHERE_BRUSH;
  	Brush->BrushSpecifics.sphere.m_HorizontalBands = m_HorizontalBands;
	Brush->BrushSpecifics.sphere.m_VerticalBands = m_VerticalBands;
	Brush->BrushSpecifics.sphere.m_XSize = m_XSize;
	Brush->BrushSpecifics.sphere.m_YSize = m_YSize;
	Brush->BrushSpecifics.sphere.m_ZSize = m_ZSize;
	Brush->BrushSpecifics.sphere.m_Solid = m_Solid;
	Brush->BrushSpecifics.sphere.m_Thickness = m_Thickness;
	Brush->BrushSpecifics.sphere.m_HalfSphere = m_HalfSphere;
   
#endif
}


void CCreateSpheroidDialog::Serialize(CArchive& ar) 
{
	if (ar.IsStoring())
	{	// storing code
		ar << m_HorizontalBands;
		ar << m_Solid;
		ar << m_Thickness;
		ar << m_VerticalBands;
		ar << m_XSize;
		ar << m_YSize;
		ar << m_ZSize;
	}
	else
	{	// loading code
		ar >> m_HorizontalBands;
		ar >> m_Solid;
		ar >> m_Thickness;
		ar >> m_VerticalBands;
		ar >> m_XSize;
		ar >> m_YSize;
		ar >> m_ZSize;
	}
}

void CCreateSpheroidDialog::OnDefaults() 
{
	m_HorizontalBands = 4;
	m_VerticalBands = 8;
	m_XSize = 256.0;
	m_YSize = 256.0;
	m_ZSize = 256.0;
	m_Solid = 0;
	m_Thickness = 16;
	UpdateData(FALSE);
	m_Picture.SetBitmap(mSolidSphere );
}

void CCreateSpheroidDialog::OnOK() 
{
	CString strMessage;
    char    data[10];
    double   val;

	// Verify that none of the sizes are < 1
    ((CEdit*)GetDlgItem(IDC_XSIZE))->GetLine(0, data, 10); 
    val = atof(data);

    if (val <= 0.0)
    {
      strMessage.Format("X Size Must Be > 0");
      AfxMessageBox(strMessage, MB_OK, 0);
	  ((CEdit*)GetDlgItem(IDC_XSIZE))->SetFocus();
	  ((CEdit*)GetDlgItem(IDC_XSIZE))->SetSel(0, -1, FALSE);
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

    ((CEdit*)GetDlgItem(IDC_ZSIZE))->GetLine(0, data, 10); 
    val = atof(data);

    if (val <= 0.0)
    {
      strMessage.Format("Z Size Must Be > 0");
      AfxMessageBox(strMessage, MB_OK, 0);
	  ((CEdit*)GetDlgItem(IDC_ZSIZE))->SetFocus();
	  ((CEdit*)GetDlgItem(IDC_ZSIZE))->SetSel(0, -1, FALSE);
      return;
    }

	
	CDialog::OnOK();
}

void CCreateSpheroidDialog::OnHalfsphereradio() 
{
  if (m_HalfSphere)
  {
    ((CButton*)GetDlgItem(IDC_HALFSPHERERADIO))->SetCheck(0);
    m_HalfSphere = 0;
  } else {
    ((CButton*)GetDlgItem(IDC_HALFSPHERERADIO))->SetCheck(1);
    m_HalfSphere = 1;
  }
}
