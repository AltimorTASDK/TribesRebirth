// 3DMWorld.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "3DMWorld.h"
#include "ConstructiveBsp.h"
#include "thredbrush.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// C3DMWorld

Vector< int > C3DMWorld::matMap;

C3DMWorld::C3DMWorld()
{
	NumberOfVerts = NumberOfPolys = 0;
}

int C3DMWorld::AddVert(ThredPoint& Vert)
{
	// look through our list
	for(int CurrentVert = 0; CurrentVert < NumberOfVerts; CurrentVert++)
    {
		if(Verts[CurrentVert].X == Vert.X &&
			Verts[CurrentVert].Y == Vert.Y &&
			Verts[CurrentVert].Z == Vert.Z)
			return CurrentVert;
	}

	// if we didn't find it make a vert
	Verts[NumberOfVerts++] = Vert;

	_ASSERTE(NumberOfVerts <= MAX_3DM_VERTS );

	// return our vert number
	return NumberOfVerts-1;
}

// Take this polygon.. add it's verts and then
// stick the vert number in the X
// of each point
//void C3DMWorld::AddPolygon(ThredPolygon& Poly, CString TextureName, Point2I tshift)
void C3DMWorld::AddPolygon(ThredPolygon& Poly, int TextureID, Point2I tshift, Point3F normal, double planeoffset)
{
    TSMaterial mattest;

    // Test to see if texture is a NULL material
    mattest = matlist->getMaterial(TextureID);
    if ((mattest.fParams.fFlags & TSMaterial::MatFlags) == TSMaterial::MatTexture)
    {
	  Polygons[NumberOfPolys].TextureID = TextureID;
    } else {
	  Polygons[NumberOfPolys].TextureID = -1;
    }

	// assign the number
	Polygons[NumberOfPolys].NumberOfPoints = Poly.NumberOfPoints;

    // Assign the texture size & offset
    Polygons[NumberOfPolys].mTextureSize.x = Poly.mTextureSize.x;
    Polygons[NumberOfPolys].mTextureSize.y = Poly.mTextureSize.y;
    Polygons[NumberOfPolys].mTextureOffset.x = Poly.mTextureOffset.x;
    Polygons[NumberOfPolys].mTextureOffset.y = Poly.mTextureOffset.y;

   // store off the texture scale shift value
   Polygons[NumberOfPolys].mTextureScaleShift = Poly.mTextureScaleShift;
   
   // grab the apply ambient light intensity flag
   Polygons[NumberOfPolys].mApplyAmbient = Poly.mApplyAmbient;
      
    // Store off the volume state info
    Polygons[NumberOfPolys].volumestate = Poly.volumestate;

	// go through and get the 'real' vert numbers
	for(int CurrentVert = 0; CurrentVert < Poly.NumberOfPoints; CurrentVert++)
    {
      // Clamp them vertex points to within 0.01
      Poly.Points[CurrentVert].X = (floor((Poly.Points[CurrentVert].X * 100.0) + 0.5)) / 100.0;
      Poly.Points[CurrentVert].Y = (floor((Poly.Points[CurrentVert].Y * 100.0) + 0.5)) / 100.0;
      Poly.Points[CurrentVert].Z = (floor((Poly.Points[CurrentVert].Z * 100.0) + 0.5)) / 100.0;

	  Polygons[NumberOfPolys].Points[CurrentVert] = AddVert(Poly.Points[CurrentVert]);

      // Clamp them texture points to within 0.01
      Poly.mTextureUV[CurrentVert].x = (floor((Poly.mTextureUV[CurrentVert].x * 100.0) + 0.5)) / 100.0;
      Poly.mTextureUV[CurrentVert].y = (floor((Poly.mTextureUV[CurrentVert].y * 100.0) + 0.5)) / 100.0;

      Polygons[NumberOfPolys].mTextureUV[CurrentVert].x = Poly.mTextureUV[CurrentVert].x;
      Polygons[NumberOfPolys].mTextureUV[CurrentVert].y = Poly.mTextureUV[CurrentVert].y;
	}

    Polygons[NumberOfPolys].constA = normal.x;
    Polygons[NumberOfPolys].constB = normal.y;
    Polygons[NumberOfPolys].constC = normal.z;
    Polygons[NumberOfPolys].constD = planeoffset;

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
    char  buf[80];

	// increment our num
	NumberOfPolys++;

    // Give some feedback...
    sprintf(buf, "Building polygon # %d", NumberOfPolys);
	pFrame->UpdateGeneralStatus(buf);

	// make sure
	_ASSERTE(NumberOfPolys <= MAX_3DM_POLYS);
}

void C3DMWorld::Export3DM(CBspNode* TreeTop, CString& Filename, CTHREDDoc* pDoc)
{
    doc = pDoc;
    matlist = pDoc->GetMaterialList();

	MakeVertsAndPolys(TreeTop );
	DumpWorld(Filename );
}

void C3DMWorld::MakeVertsAndPolys(CBspNode* Node)
{
	if(!Node )
		return;

	// do kids
	MakeVertsAndPolys(Node->BackChild);
	MakeVertsAndPolys(Node->FrontChild);

	// dump our polys
	while(Node)
    {
		if(Node->mValidated && Node->mBrushPtr)
        {
            Point3F normal;
            normal.x = Node->ConstantA;
            normal.y = Node->ConstantB;
            normal.z = Node->ConstantC;
            normal.normalize();

			AddPolygon( Node->Polygon,
                        Node->mBrushPtr->mTextureID[Node->mPolygonNumber].TextureID,
                        Node->mBrushPtr->mTextureID[Node->mPolygonNumber].TextureShift,
                        normal, Node->ConstantD);
        }
		Node = Node->CoplanarChild;
	}
}

// go dump the stuff we saved up
void C3DMWorld::DumpWorld(CString& Filename)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
    char  buf[80];
	CStdioFile OutFile;
	CString OutString;

	// open the file
	if(!OutFile.Open(Filename, CFile::modeWrite|CFile::modeCreate ) )
		return;

	// now write out some info
	OutFile.WriteString("; 3DM Export from ZED\n");

   // need to keep this file version'd from now on
   OutFile.WriteString( "VERSION\n");
   OutString.Format( "%d\n", theApp.m_version );
   OutFile.WriteString( OutString );
   
	OutFile.WriteString("VERTICES\n");
	OutString.Format("%d\n", NumberOfVerts);
	OutFile.WriteString(OutString);

	// do the actual verts
	for(int CurrentVert = 0; CurrentVert < NumberOfVerts; CurrentVert++ ) {
		OutString.Format("%lf %lf %lf\n", 
			Verts[CurrentVert].X, Verts[CurrentVert].Y, Verts[CurrentVert].Z);
		OutFile.WriteString(OutString);
	}

	// now write out the gons
	OutFile.WriteString("POLYGONS\n");
	OutString.Format("%d\n", NumberOfPolys);
	OutFile.WriteString(OutString);

	for(int CurrentPoly = 0; CurrentPoly < NumberOfPolys; CurrentPoly++ )
    {
        // Give some feedback...
        sprintf(buf, "Exporting polygon %d of %d", CurrentPoly, NumberOfPolys);
	    pFrame->UpdateGeneralStatus(buf);

      // determine what the REAL id for the texture will be ( new mat list stuff..)
      int textureID = Polygons[CurrentPoly].TextureID;
      if( textureID != -1 )
      {
         int i;
         
         // check if this already exists in the table
         for( i = 0; i < matMap.size(); i++ )
         {
            if( matMap[i] == textureID )
            {
               textureID = i;
               break;
            }
         }
         
         // check if we found it or not
         if( i == matMap.size() )
         {
            // add to it
            matMap.push_back( textureID );
            
            // grab the new one
            textureID = matMap.size() - 1;
         }
      }
      
		OutString.Format("%d\n", textureID );
		OutFile.WriteString(OutString);
      
      
		OutString.Format("%d\n", Polygons[CurrentPoly].NumberOfPoints);
		OutFile.WriteString(OutString);

        // DPW - write out texture size
        OutString.Format("%d %d\n", Polygons[CurrentPoly].mTextureSize.x, Polygons[CurrentPoly].mTextureSize.y);
		OutFile.WriteString(OutString);

        // DPW - write out texture offset
        OutString.Format("%d %d\n", Polygons[CurrentPoly].mTextureOffset.x, Polygons[CurrentPoly].mTextureOffset.y);
		OutFile.WriteString(OutString);

      // write out the texture scale factor
      OutString.Format( "%d\n", Polygons[CurrentPoly].mTextureScaleShift );
      OutFile.WriteString( OutString );
      
      // write out the apply ambient flag
      OutString.Format( "%d\n", Polygons[CurrentPoly].mApplyAmbient );
      OutFile.WriteString( OutString );
      
		// write out the verts
		for(CurrentVert = 0; CurrentVert < Polygons[CurrentPoly].NumberOfPoints; CurrentVert++ )
        {
			OutString.Format("%d ", Polygons[CurrentPoly].Points[CurrentVert]);
			OutFile.WriteString(OutString);
		}
		OutFile.WriteString("\n");

		// write out the texture verts
		for(CurrentVert = 0; CurrentVert < Polygons[CurrentPoly].NumberOfPoints; CurrentVert++ )
        {
			OutString.Format("%.3lf ", Polygons[CurrentPoly].mTextureUV[CurrentVert].x);
			OutFile.WriteString(OutString);
			OutString.Format("%.3lf ", Polygons[CurrentPoly].mTextureUV[CurrentVert].y);
			OutFile.WriteString(OutString);
		}
		OutFile.WriteString("\n");

        // Write out the volume state
        OutString.Format("%lu\n", Polygons[CurrentPoly].volumestate);
		OutFile.WriteString(OutString);

        // Write out the plane equation coefficients
        OutString.Format("%lf %lf %lf %lf\n",
                          Polygons[CurrentPoly].constA,
                          Polygons[CurrentPoly].constB,
                          Polygons[CurrentPoly].constC,
                          Polygons[CurrentPoly].constD);
		OutFile.WriteString(OutString);
	}

	// close da file
	OutFile.Close();
}
