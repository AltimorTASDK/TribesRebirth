// CreateCylDialog.cpp : implementation file
//

#include "stdafx.h"
#include <math.h>
#include "THRED.h"
#include "CreateCylDialog.h"
#include "thredbrush.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCreateCylDialog dialog


CCreateCylDialog::CCreateCylDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateCylDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateCylDialog)
	m_BotXOffset = 0.0;
	m_BotXSize = 128.0;
	m_BotZOffset = 0.0;
	m_BotZSize = 128.0;
	m_Solid = 0;
	m_Thickness = 16.0;
	m_TopXOffset = 0.0;
	m_TopXSize = 128.0;
	m_TopZOffset = 0.0;
	m_TopZSize = 128.0;
	m_VerticalStripes = 6.0;
	m_YSize = 256.0;
	//}}AFX_DATA_INIT

    m_Ring = 0;
}


void CCreateCylDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateCylDialog)
	DDX_Text(pDX, IDC_BOTXOFF, m_BotXOffset);
	DDX_Text(pDX, IDC_BOTXSIZE, m_BotXSize);
	DDX_Text(pDX, IDC_BOTZOFF, m_BotZOffset);
	DDX_Text(pDX, IDC_BOTZSIZE, m_BotZSize);
	DDX_Radio(pDX, IDC_SOLID, m_Solid);
	DDX_Text(pDX, IDC_THICKNESS, m_Thickness);
	DDV_MinMaxDouble(pDX, m_Thickness, 0., 10000.);
	DDX_Text(pDX, IDC_TOPXOFF, m_TopXOffset);
	DDX_Text(pDX, IDC_TOPXSIZE, m_TopXSize);
	DDX_Text(pDX, IDC_TOPZOFF, m_TopZOffset);
	DDX_Text(pDX, IDC_TOPZSIZE, m_TopZSize);
	DDX_Text(pDX, IDC_VERTSTRIPES, m_VerticalStripes);
	DDV_MinMaxDouble(pDX, m_VerticalStripes, 3., 31.);
	DDX_Text(pDX, IDC_YSIZE, m_YSize);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateCylDialog, CDialog)
	//{{AFX_MSG_MAP(CCreateCylDialog)
	ON_BN_CLICKED(IDC_Defaults, OnDefaults)
	ON_BN_CLICKED(IDC_RING, OnRing)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateCylDialog message handlers

//=========================================================
// Make our dialog go and then create the brush if necessary
//=========================================================
void CCreateCylDialog::DoDialog(CThredBrush* Brush)
{
	// go
	if(DoModal() != IDOK )
		return;

	// now make the thing
	CreateCyl(Brush );
}

//=========================================================
// This function takes all of our fantastic member
// variables and creates a cylinder
//=========================================================
void CCreateCylDialog::CreateCyl( CThredBrush* Brush )
{
#if 1
   int numVerticalStripes;
   int numVertices;
   int numPolygons;
         
	ThredPoint * vertices;
	ThredPolygon * polygons;
   double rotation;
   int i;

   // make sure the number of bands is ok
   if( m_VerticalStripes < 3 )
      return;
   numVerticalStripes = m_VerticalStripes;   
   
   // calculate the number of vertices and polygons
   if( !m_Solid )
   {
      numVertices = numVerticalStripes * 2;
      numPolygons = numVerticalStripes + 2;
   }
   else
   {
      // check the thickness
      if( ( m_Thickness >= ( m_TopXSize / 2 ) ) ||
         ( m_Thickness >= ( m_BotXSize / 2 ) ) ||
         ( m_Thickness >= ( m_TopZSize / 2 ) ) ||
         ( m_Thickness >= ( m_BotZSize / 2 ) ) ||
         ( m_Thickness >= ( m_YSize / 2 ) ) )
         return;
         
      numPolygons = ( numVerticalStripes + 2 ) * 2;
      numVertices = ( numVerticalStripes * 4 );
   }
   
   if( numVertices > MAX_POLYGON_VERTS )
	   return;

	// allocate the number of vertices we need
	if( !( vertices = new ThredPoint[ numVertices ] ) )
      return;

	// allocate the number of gons that we need
	Brush->AllocateBrushPolygons( numPolygons );
	Brush->AllocateBrushPolyTextures( numPolygons );

   // initialize the texture id's
   for( i = 0; i < numPolygons; i++ )
   {
      Brush->mTextureID[ i ].TextureID = atoi(Brush->mName);
      Brush->mTextureID[ i ].TextureShift.x = 0;
      Brush->mTextureID[ i ].TextureShift.y = 0;
      Brush->mTextureID[ i ].TextureFlipH = 0;
      Brush->mTextureID[ i ].TextureFlipV = 0;
      Brush->mTextureID[ i ].TextureRotate = 0.F;
   }   
   
	// get ptr to the brush's polygons
	polygons = Brush->mPolygons;

   // go through and generate the vertices
   // order: ext top -> ext bottom -> int top -> int bottom
   for( i = 0; i < numVerticalStripes; i++ )
   {
      // get the rotation
      rotation = ( ( 2.0 * M_PI_VALUE ) / numVerticalStripes ) * i;
      
      // get the top point
      vertices[ i ].X = ( ( m_TopXSize / 2 ) * cos( rotation ) ) + m_TopXOffset;
      vertices[ i ].Y = ( m_YSize / 2 );
      vertices[ i ].Z = ( ( m_TopZSize / 2 ) * sin( rotation ) ) + m_TopZOffset;
      
      // now the bottom point
      vertices[ numVerticalStripes + i ].X = ( ( m_BotXSize / 2 ) * cos( rotation ) ) + m_BotXOffset;
      vertices[ numVerticalStripes + i ].Y = -( m_YSize / 2 );
      vertices[ numVerticalStripes + i ].Z = ( ( m_BotZSize / 2 ) * sin( rotation ) ) + m_BotZOffset;
      
      // generate interior vertices
      if( m_Solid )
      {
         int vertexOffset = ( numVertices / 2 ) + i;
         
         // get the top point
         vertices[ vertexOffset ].X = ( ( ( m_TopXSize / 2 ) - m_Thickness ) * cos( rotation ) ) + m_TopXOffset;
         vertices[ vertexOffset ].Y = ( ( m_YSize / 2 ) - m_Thickness );
         vertices[ vertexOffset ].Z = ( ( ( m_TopZSize / 2 ) - m_Thickness ) * sin( rotation ) ) + m_TopZOffset;
         
         // now the bottom point
         vertices[ numVerticalStripes + vertexOffset ].X = ( ( ( m_BotXSize / 2 ) - m_Thickness ) * cos( rotation ) ) + m_BotXOffset;
         vertices[ numVerticalStripes + vertexOffset ].Y = -( ( m_YSize / 2 ) - m_Thickness );
         vertices[ numVerticalStripes + vertexOffset ].Z = ( ( ( m_BotZSize / 2 ) - m_Thickness ) * sin( rotation ) ) + m_BotZOffset;
      }
   }
   
   // now go through the poly's and generate them ( CCW for exterior, CW for interior - well, CCW too )
   for( i = 0; i < numVerticalStripes; i++ )
   {
      // generate the exterior side poly's
      polygons[ i ].NumberOfPoints = 4;
      polygons[ i ].Points[ 0 ] = vertices[ i ];
      polygons[ i ].Points[ 1 ] = vertices[ i + numVerticalStripes ];
      polygons[ i ].Points[ 2 ] = vertices[ ( ( i + numVerticalStripes - 1 ) % numVerticalStripes ) + numVerticalStripes ];
      polygons[ i ].Points[ 3 ] = vertices[ ( i + numVerticalStripes - 1 ) % numVerticalStripes ];
      
      // generate the interior side poly's
      if( m_Solid )
      {
         int vertexOffset = ( numVertices / 2 );
         int polygonOffset = ( numPolygons / 2 ) + i;
         
         // generate the exterior side poly's
         polygons[ polygonOffset ].NumberOfPoints = 4;
         polygons[ polygonOffset ].Points[ 0 ] = vertices[ i + vertexOffset ];
         polygons[ polygonOffset ].Points[ 1 ] = vertices[ ( ( i + numVerticalStripes - 1 ) % numVerticalStripes ) + vertexOffset ];
         polygons[ polygonOffset ].Points[ 2 ] = vertices[ ( ( i + numVerticalStripes - 1 ) % numVerticalStripes ) + numVerticalStripes + vertexOffset ];
         polygons[ polygonOffset ].Points[ 3 ] = vertices[ i + numVerticalStripes + vertexOffset ];
      }
   }
   
   // generate the top/bottom poly's ( CCW )

   // top poly
   polygons[ numVerticalStripes ].NumberOfPoints = numVerticalStripes;
   for( i = 0; i < numVerticalStripes; i++ )
      polygons[ numVerticalStripes ].Points[ i ] = 
         vertices[ numVerticalStripes - i - 1 ];
      
   // bottom poly
   polygons[ numVerticalStripes + 1 ].NumberOfPoints = numVerticalStripes;
   for( i = 0; i < numVerticalStripes; i++ )
      polygons[ numVerticalStripes + 1 ].Points[ i ] = 
         vertices[ numVerticalStripes + i ];
         
   // generate the top/bottom poly's ( CW from top down )
   if( m_Solid )
   {
      int vertexOffset = ( numVertices / 2 );
      int polygonOffset = ( numPolygons / 2 );
      
      // top poly
      polygons[ polygonOffset + numVerticalStripes ].NumberOfPoints = numVerticalStripes;
      for( i = 0; i < numVerticalStripes; i++ )
         polygons[ polygonOffset + numVerticalStripes ].Points[ i ] = 
            vertices[ i + vertexOffset ];
         
      // bottom poly
      polygons[ polygonOffset + numVerticalStripes + 1 ].NumberOfPoints = numVerticalStripes;
      for( i = 0; i < numVerticalStripes; i++ )
         polygons[ polygonOffset + numVerticalStripes + 1 ].Points[ i ] = 
            vertices[ numVertices - i - 1 ];
   }

   // cleanup
   delete []vertices;

   Brush->BrushType = CThredBrush::CYL_BRUSH;
   Brush->BrushSpecifics.cylinder.m_BotXOffset = m_BotXOffset;
   Brush->BrushSpecifics.cylinder.m_BotXSize = m_BotXSize;
   Brush->BrushSpecifics.cylinder.m_BotZOffset = m_BotZOffset;
   Brush->BrushSpecifics.cylinder.m_BotZSize = m_BotZSize;
   Brush->BrushSpecifics.cylinder.m_Solid = m_Solid;
   Brush->BrushSpecifics.cylinder.m_Thickness = m_Thickness;
   Brush->BrushSpecifics.cylinder.m_TopXOffset = m_TopXOffset;
   Brush->BrushSpecifics.cylinder.m_TopXSize = m_TopXSize;
   Brush->BrushSpecifics.cylinder.m_TopZOffset = m_TopZOffset;
   Brush->BrushSpecifics.cylinder.m_TopZSize = m_TopZSize;
   Brush->BrushSpecifics.cylinder.m_VerticalStripes = m_VerticalStripes;
   Brush->BrushSpecifics.cylinder.m_YSize = m_YSize;
   Brush->BrushSpecifics.cylinder.m_Ring = m_Ring;
   
#endif

#if 0
	// my stuff
	int NumberOfVerts, NumberOfPolygons;
	ThredPoint* Verts;
	ThredPoint* Verts2;
	ThredPolygon* Polygons;
	CTransformMatrix YRotation;
	double CurrentXDiameter, CurrentZDiameter;
	double DeltaXDiameter, DeltaZDiameter;
	double CurrentXOffset, CurrentZOffset;
	double DeltaXOffset, DeltaZOffset;
	int NumberOfVerticalBands;
	ThredPoint Current, Final, Delta;
	double EllipseZ;
	int HBand, VBand;
	int VertexCount = 0, PolygonCount = 0;

	// Make sure we have enough bands
	if(m_VerticalStripes < 3)
		return;
	NumberOfVerticalBands = (int)m_VerticalStripes;

	if(m_Solid == 0)
    {
		// Get the number of vertices and polygons
		NumberOfVerts =   NumberOfVerticalBands * 2;

        if (!m_Ring)
        {
		  // number of bands plus the top and bottom.
		  NumberOfPolygons = NumberOfVerticalBands + 2;
        } else {
		  NumberOfPolygons = NumberOfVerticalBands;
        }
	} else {
			// Get the number of vertices and polygons
		NumberOfVerts =   NumberOfVerticalBands * 4;

        if (!m_Ring)
        {
		  // number of bands plus the top and bottom.
		  NumberOfPolygons = (NumberOfVerticalBands + 2)*2;
        } else {
		  NumberOfPolygons = (NumberOfVerticalBands)*2;
        }
	}

	// allocate the number of verts we need
	Verts = new ThredPoint[NumberOfVerts];

	// check this sucker.
	if(!Verts )
		return;

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

	// Create the rotation matrix
	YRotation.ClearMatrix();
	YRotation.SetYaw(M_TOTAL_DEGREES / NumberOfVerticalBands );

	// Start with the top of cylinder
	CurrentXDiameter = m_TopXSize;
	CurrentZDiameter = m_TopZSize;
	DeltaXDiameter = (m_BotXSize - m_TopXSize);
	DeltaZDiameter = (m_BotZSize - m_TopZSize);

	
	// Get the offset amounts
	CurrentXOffset = m_TopXOffset;
	CurrentZOffset = m_TopZOffset;
	DeltaXOffset = (m_BotXOffset - m_TopXOffset);
	DeltaZOffset = (m_BotZOffset - m_TopZOffset);

	// Get the band positions and deltas
	Current.X = (m_TopXSize / 2);
	Current.Y = (m_YSize / 2);
	Current.Z = 0;

	Delta.X = ((m_BotXSize / 2) - Current.X);
	Delta.Y = (-(m_YSize/2) - Current.Y);
	Delta.Z = 0;

//   ThredPoint myOrig;
//   myOrig.X = ( CurrentXDiameter / 2 );
//   myOrig.Z = 0; //( CurrentZDiameter / 2 );

	for(HBand = 0;HBand <= 1;HBand++) {
		// Get a working vector
		Final = Current;

		for(VBand = 0;VBand < NumberOfVerticalBands;VBand++) {
			// Get the elliptical Z value
			// (x^2/a^2) + (z^2/b^2) = 1
			// z = sqrt(b^2(1 - x^2/a^2))
			EllipseZ = sqrt(((CurrentZDiameter/2)*(CurrentZDiameter/2))
				* (1.0 - (Final.X*Final.X)
				/ ((CurrentXDiameter/2)*(CurrentXDiameter/2) )) );

			// Check if we need to negate this thing
			if(VBand > (NumberOfVerticalBands/2))
				EllipseZ = -EllipseZ;

			// Store this vertex
			Verts[VertexCount].X = Final.X + CurrentXOffset;
			Verts[VertexCount].Y = Final.Y;
			Verts[VertexCount].Z = EllipseZ + CurrentZOffset;
			VertexCount++;

         
               
			// Rotate the point around the Y to get the next vertical band
			YRotation.ApplyMatrix(Final );
		}

		CurrentXDiameter += DeltaXDiameter;
		CurrentZDiameter += DeltaZDiameter;
		CurrentXOffset += DeltaXOffset;
		CurrentZOffset += DeltaZOffset;
		Current.X += Delta.X;
		Current.Y += Delta.Y;
		Current.Z += Delta.Z;
	}

    if (!m_Ring)
    {
	  // Generate the top and bottom polygons first
	  Polygons[PolygonCount].NumberOfPoints = NumberOfVerticalBands;
	  for(VBand = NumberOfVerticalBands-1, HBand=0; VBand >=0; VBand--, HBand++ ) {
		  Polygons[PolygonCount].Points[HBand] = Verts[VBand];
	  }
	  PolygonCount++;
	  Polygons[PolygonCount].NumberOfPoints = NumberOfVerticalBands;
	  for(VBand = 0; VBand < NumberOfVerticalBands; VBand++ ) {
		  Polygons[PolygonCount].Points[VBand]
		  = Verts[VBand + NumberOfVerticalBands];
	  }
	  PolygonCount++;
    }


	// Generate the polygons
	for(HBand = 0;HBand < 1;HBand++) {
		for(VBand = 0;VBand < NumberOfVerticalBands;VBand++) {
			// First triangle
			Polygons[PolygonCount].NumberOfPoints = 4;

			Polygons[PolygonCount].Points[0]
				= Verts[(HBand * NumberOfVerticalBands) + VBand];
			Polygons[PolygonCount].Points[1]
				= Verts[(HBand * NumberOfVerticalBands) + ((VBand + 1) % NumberOfVerticalBands)];
			Polygons[PolygonCount].Points[2]
				= Verts[((HBand + 1) * NumberOfVerticalBands) + ((VBand + 1) % NumberOfVerticalBands)];
			Polygons[PolygonCount].Points[3]
				= Verts[((HBand + 1) * NumberOfVerticalBands) + VBand];
			PolygonCount++;
		}
	}

	if(m_Solid != 0)
    {
		// make a diameter thickness
		double Thickness = m_Thickness * 2;

		// assign our verts2
		Verts2 = &Verts[NumberOfVerts/2];

		// Start with the top of cylinder
		CurrentXDiameter = m_TopXSize - Thickness;
		CurrentZDiameter = m_TopZSize - Thickness;
		DeltaXDiameter = ((m_BotXSize-Thickness) - (m_TopXSize-Thickness));
		DeltaZDiameter = ((m_BotZSize-Thickness) - (m_TopZSize-Thickness));
		
		// Get the offset amounts
		CurrentXOffset = m_TopXOffset;
		CurrentZOffset = m_TopZOffset;
		DeltaXOffset = (m_BotXOffset - m_TopXOffset);
		DeltaZOffset = (m_BotZOffset - m_TopZOffset);

		// Get the band positions and deltas
		Current.X = ((m_TopXSize-Thickness) / 2);
		Current.Y = ((m_YSize-Thickness) / 2);
		Current.Z = 0;

		Delta.X = (((m_BotXSize-Thickness) / 2) - Current.X);
		Delta.Y = (-((m_YSize-Thickness)/2) - Current.Y);
		Delta.Z = 0;

		for(HBand = 0;HBand <= 1;HBand++)
        {
			// Get a working vector
			Final = Current;

			for(VBand = 0;VBand < NumberOfVerticalBands;VBand++)
            {
				// Get the elliptical Z value
				// (x^2/a^2) + (z^2/b^2) = 1
				// z = sqrt(b^2(1 - x^2/a^2))
				EllipseZ = sqrt(((CurrentZDiameter/2)*(CurrentZDiameter/2))
					* (1.0 - (Final.X*Final.X)
					/ ((CurrentXDiameter/2)*(CurrentXDiameter/2))));

				// Check if we need to negate this thing
				if(VBand > (NumberOfVerticalBands/2))
					EllipseZ = -EllipseZ;

				// Store this vertex
				Verts[VertexCount].X = Final.X + CurrentXOffset;
				Verts[VertexCount].Y = Final.Y;
				Verts[VertexCount].Z = EllipseZ + CurrentZOffset;
				VertexCount++;

				// Rotate the point around the Y to get the next vertical band
				YRotation.ApplyMatrix(Final);
			}

			CurrentXDiameter += DeltaXDiameter;
			CurrentZDiameter += DeltaZDiameter;
			CurrentXOffset += DeltaXOffset;
			CurrentZOffset += DeltaZOffset;
			Current.X += Delta.X;
			Current.Y += Delta.Y;
			Current.Z += Delta.Z;
		}

        if (!m_Ring)
        {
		  // Generate the top and bottom polygons first
		  Polygons[PolygonCount].NumberOfPoints = NumberOfVerticalBands;
		  for(VBand = NumberOfVerticalBands-1, HBand=0; VBand >=0; VBand--, HBand++)
          {
			  Polygons[PolygonCount].Points[HBand]
			  = Verts2[VBand+NumberOfVerticalBands];
		  }

		  PolygonCount++;
		  Polygons[PolygonCount].NumberOfPoints = NumberOfVerticalBands;
		  for(VBand = 0; VBand < NumberOfVerticalBands; VBand++)
          {
			  Polygons[PolygonCount].Points[VBand]
			  = Verts2[VBand];
		  }
		  PolygonCount++;
        }

		// Generate the polygons
		for(HBand = 0;HBand < 1;HBand++)
        {
			for(VBand = 0;VBand < NumberOfVerticalBands;VBand++)
            {
				// First triangle
				Polygons[PolygonCount].NumberOfPoints = 4;

				Polygons[PolygonCount].Points[3]
					= Verts2[(HBand * NumberOfVerticalBands) + VBand];
				Polygons[PolygonCount].Points[2]
					= Verts2[(HBand * NumberOfVerticalBands) + ((VBand + 1) % NumberOfVerticalBands)];
				Polygons[PolygonCount].Points[1]
					= Verts2[((HBand + 1) * NumberOfVerticalBands) + ((VBand + 1) % NumberOfVerticalBands)];
				Polygons[PolygonCount].Points[0]
					= Verts2[((HBand + 1) * NumberOfVerticalBands) + VBand];
				PolygonCount++;
			}
		}
	}

	delete []Verts;

	_ASSERTE(PolygonCount == NumberOfPolygons );
	_ASSERTE(VertexCount == NumberOfVerts );

    Brush->BrushType = CThredBrush::CYL_BRUSH;
  	Brush->BrushSpecifics.cylinder.m_BotXOffset = m_BotXOffset;
	Brush->BrushSpecifics.cylinder.m_BotXSize = m_BotXSize;
	Brush->BrushSpecifics.cylinder.m_BotZOffset = m_BotZOffset;
	Brush->BrushSpecifics.cylinder.m_BotZSize = m_BotZSize;
	Brush->BrushSpecifics.cylinder.m_Solid = m_Solid;
	Brush->BrushSpecifics.cylinder.m_Thickness = m_Thickness;
	Brush->BrushSpecifics.cylinder.m_TopXOffset = m_TopXOffset;
	Brush->BrushSpecifics.cylinder.m_TopXSize = m_TopXSize;
	Brush->BrushSpecifics.cylinder.m_TopZOffset = m_TopZOffset;
	Brush->BrushSpecifics.cylinder.m_TopZSize = m_TopZSize;
	Brush->BrushSpecifics.cylinder.m_VerticalStripes = m_VerticalStripes;
	Brush->BrushSpecifics.cylinder.m_YSize = m_YSize;
	Brush->BrushSpecifics.cylinder.m_Ring = m_Ring;

#endif
}


void CCreateCylDialog::Serialize(CArchive& ar) 
{
	if (ar.IsStoring())
	{	
		ar << m_BotXOffset;
		ar << m_BotXSize;
		ar << m_BotZOffset;
		ar << m_BotZSize;
		ar << m_Solid;
		ar << m_Thickness;
		ar << m_TopXOffset;
		ar << m_TopXSize;
		ar << m_TopZOffset;
		ar << m_TopZSize;
		ar << m_VerticalStripes;
		ar << m_YSize;
	}
	else
	{	// loading code
		ar >> m_BotXOffset;
		ar >> m_BotXSize;
		ar >> m_BotZOffset;
		ar >> m_BotZSize;
		ar >> m_Solid;
		ar >> m_Thickness;
		ar >> m_TopXOffset;
		ar >> m_TopXSize;
		ar >> m_TopZOffset;
		ar >> m_TopZSize;
		ar >> m_VerticalStripes;
		ar >> m_YSize;
	}
}

void CCreateCylDialog::OnDefaults() 
{
	m_BotXOffset = 0.0;
	m_BotXSize = 128.0;
	m_BotZOffset = 0.0;
	m_BotZSize = 128.0;
	m_Solid = 0;
	m_Thickness =  4.0;
	m_TopXOffset = 0.0;
	m_TopXSize = 128.0;
	m_TopZOffset = 0.0;
	m_TopZSize = 128.0;
	m_VerticalStripes = 6.0;
	m_YSize = 256.0;
	UpdateData(FALSE);
}

void CCreateCylDialog::OnOK() 
{
	CString strMessage;
    char    data[10];
    double   val;

	// Verify that none of the sizes are < 1
    ((CEdit*)GetDlgItem(IDC_TOPXSIZE))->GetLine(0, data, 10); 
    val = atof(data);

    if (val <= 0.0)
    {
      strMessage.Format("Top X Size Must Be > 0");
      AfxMessageBox(strMessage, MB_OK, 0);
	  ((CEdit*)GetDlgItem(IDC_TOPXSIZE))->SetFocus();
	  ((CEdit*)GetDlgItem(IDC_TOPXSIZE))->SetSel(0, -1, FALSE);
      return;
    }

    ((CEdit*)GetDlgItem(IDC_BOTXSIZE))->GetLine(0, data, 10); 
    val = atof(data);

    if (val <= 0.0)
    {
      strMessage.Format("Bottom X Size Must Be > 0");
      AfxMessageBox(strMessage, MB_OK, 0);
	  ((CEdit*)GetDlgItem(IDC_BOTXSIZE))->SetFocus();
	  ((CEdit*)GetDlgItem(IDC_BOTXSIZE))->SetSel(0, -1, FALSE);
      return;
    }

    ((CEdit*)GetDlgItem(IDC_TOPZSIZE))->GetLine(0, data, 10); 
    val = atof(data);

    if (val <= 0.0)
    {
      strMessage.Format("Top Z Size Must Be > 0");
      AfxMessageBox(strMessage, MB_OK, 0);
	  ((CEdit*)GetDlgItem(IDC_TOPZSIZE))->SetFocus();
	  ((CEdit*)GetDlgItem(IDC_TOPZSIZE))->SetSel(0, -1, FALSE);
      return;
    }

    ((CEdit*)GetDlgItem(IDC_BOTZSIZE))->GetLine(0, data, 10); 
    val = atof(data);

    if (val <= 0.0)
    {
      strMessage.Format("Bottom Z Size Must Be > 0");
      AfxMessageBox(strMessage, MB_OK, 0);
	  ((CEdit*)GetDlgItem(IDC_BOTZSIZE))->SetFocus();
	  ((CEdit*)GetDlgItem(IDC_BOTZSIZE))->SetSel(0, -1, FALSE);
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

void CCreateCylDialog::OnRing() 
{
#if 0
  if (m_Ring)
  {
    ((CButton*)GetDlgItem(IDC_RING))->SetCheck(0);
    m_Ring = 0;
  } else {
    ((CButton*)GetDlgItem(IDC_RING))->SetCheck(1);
    m_Ring = 1;
  }
#endif
}
