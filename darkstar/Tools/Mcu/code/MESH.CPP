#include "stdafx.h"
#include "detail.h"

//---------------------------------------------------------------------------

Int16 Mesh::AddTexture( textvert3ds & text )
{
   if( fnTexVerts == MAX_VERTS )
      throw MeshError( fName, "Too many texture vertices in mesh!" );

   // Add a texture coordinate to the list for the current mesh.
   // uplicate texture coordinates are not stored, so before actually
   // adding the coord to the list, a check is made to see if the 
   // coord is already in the list.  The return value is the index
   // of the texture coord in the list.

   Point2F newText;
   newText.x = text.u;
   newText.y = 1.0f - text.v;

   for( int i = 0; i < fnTexVerts; i++ )
      if( faTexVerts[i] == newText )
         return (Int16)i;

   faTexVerts[fnTexVerts] = newText;
   return (Int16)fnTexVerts++;
}

//--------------------------------------------------------------------

Int16 Mesh::AddVertex( const Point3F & loc, const Point3F & normal,
							 RealF pointTolerance, RealF normalTolerance )
{
   if( fnVerts == MAX_VERTS )
      throw MeshError( fName, "Too many vertices in mesh!" );

   // Add a vertex structure consisting of location and normal vector
   // to the list for the current mesh.  
   // A check is made to see if an identical vertex has already been stored 
   // in the list; duplicates are not stored.  In making this check, a 
   // tolerance value is used.  If the vertex is not already in the list, 
   // it is added to the end of the list. The index of the vertex in the 
   // list is returned.

	for( int i = 0; i < fnVerts; i++ )
		if( m_dist( loc, faVerts[i].fPoint ) <= pointTolerance &&
			 m_dist( normal, faVerts[i].fNormal ) <= normalTolerance ) 
         return (Int16)i;

   faVerts[fnVerts].fPoint = loc;
   faVerts[fnVerts].fNormal = normal;
   
   return (Int16)fnVerts++;
}

//---------------------------------------------------------------------------

Int16 Mesh::AddVertex( point3ds & floc, Point3F & fnormal,
							 RealF pointTolerance, RealF normalTolerance )
{
   return AddVertex( Point3F( floc.x, floc.y, floc.z ), fnormal, 
			pointTolerance, normalTolerance );
}

//---------------------------------------------------------------------------

Bool Mesh::FaceIsConvex( TSMeshFace const & face )
{
   Point3F e1,e2,e3,e4;
   static Point3F normal;
   e1 = faVerts[face.fV[1]].fPoint;
   e1 -= faVerts[face.fV[0]].fPoint;
   e2 = faVerts[face.fV[2]].fPoint;
   e2 -= faVerts[face.fV[1]].fPoint;
   e3 = faVerts[face.fV[3]].fPoint;
   e3 -= faVerts[face.fV[2]].fPoint;
   e4 = faVerts[face.fV[0]].fPoint;
   e4 -= faVerts[face.fV[3]].fPoint;
   Point3F n1,n2,n3,n4;
   m_cross( e1, e2, &n1 );
   m_cross( e2, e3, &n2 );
   m_cross( e3, e4, &n3 );
   m_cross( e4, e1, &n4 );
   if( (m_dot(n1,n2) > 0.0f) && (m_dot(n2,n3) > 0.0f) && (m_dot(n3,n4) > 0.0f) )
		return TRUE;
	return FALSE;
}

//---------------------------------------------------------------------------

void Mesh::AddTri( Int16 v0, Int16 v1, Int16 v2,Int16 center,
   Int16 t0, Int16 t1, Int16 t2, Int16 mat, RealF normalTolerance )
{
	// first, see if this triangle can be merged with another
	// triangle to form a quad.  Must have same face normal & material:
	for( int f = 0; f < fnFaces; f++ )
		if( faFaces[f].fV[2] == faFaces[f].fV[3] && 
			 faFaces[f].fMaterial == mat &&
			 m_dist( faVerts[faFaces[f].fCenter].fNormal, 
						faVerts[center].fNormal ) < normalTolerance )
			{
			TSMeshFace face = faFaces[f];

			if( face.fV[0] == v0 && face.fV[1] == v2 &&
				 face.fT[0] == t0 && face.fT[1] == t2 )
				{
				face.fV[3] = face.fV[2];
				face.fV[2] = face.fV[1];
				face.fV[1] = v1;
				face.fT[3] = face.fT[2];
				face.fT[2] = face.fT[1];
				face.fT[1] = t1;
				if( FaceIsConvex( face ) )
					{
					faFaces[f] = face;
					return;
					}
				}
			else if( face.fV[0] == v2 && face.fV[1] == v1 &&
				 face.fT[0] == t2 && face.fT[1] == t1 )
				{
				face.fV[3] = face.fV[2];
				face.fV[2] = face.fV[1];
				face.fV[1] = v0;
				face.fT[3] = face.fT[2];
				face.fT[2] = face.fT[1];
				face.fT[1] = t0;
				if( FaceIsConvex( face ) )
					{
					faFaces[f] = face;
					return;
					}
				}
			else if( face.fV[0] == v1 && face.fV[1] == v0 &&
				 face.fT[0] == t1 && face.fT[1] == t0 )
				{
				face.fV[3] = face.fV[2];
				face.fV[2] = face.fV[1];
				face.fV[1] = v2;
				face.fT[3] = face.fT[2];
				face.fT[2] = face.fT[1];
				face.fT[1] = t2;
				if( FaceIsConvex( face ) )
					{
					faFaces[f] = face;
					return;
					}
				}
			else if( face.fV[1] == v0 && face.fV[2] == v2 &&
				 face.fT[1] == t0 && face.fT[2] == t2 )
				{
				face.fV[3] = face.fV[2];
				face.fV[2] = v1;
				face.fT[3] = face.fT[2];
				face.fT[2] = t1;
				if( FaceIsConvex( face ) )
					{
					faFaces[f] = face;
					return;
					}
				}
			else if( face.fV[1] == v2 && face.fV[2] == v1 &&
				 face.fT[1] == t2 && face.fT[2] == t1 )
				{
				face.fV[3] = face.fV[2];
				face.fV[2] = v0;
				face.fT[3] = face.fT[2];
				face.fT[2] = t0;
				if( FaceIsConvex( face ) )
					{
					faFaces[f] = face;
					return;
					}
				}
			else if( face.fV[1] == v1 && face.fV[2] == v0 &&
				 face.fT[1] == t1 && face.fT[2] == t0 )
				{
				face.fV[3] = face.fV[2];
				face.fV[2] = v2;
				face.fT[3] = face.fT[2];
				face.fT[2] = t2;
				if( FaceIsConvex( face ) )
					{
					faFaces[f] = face;
					return;
					}
				}
			else if( face.fV[2] == v0 && face.fV[0] == v2 &&
				 face.fT[2] == t0 && face.fT[0] == t2 )
				{
				face.fV[3] = v1;
				face.fT[3] = t1;
				if( FaceIsConvex( face ) )
					{
					faFaces[f] = face;
					return;
					}
				}
			else if( face.fV[2] == v2 && face.fV[0] == v1 &&
				 face.fT[2] == t2 && face.fT[0] == t1 )
				{
				face.fV[3] = v0;
				face.fT[3] = t0;
				if( FaceIsConvex( face ) )
					{
					faFaces[f] = face;
					return;
					}
				}
			else if( face.fV[2] == v1 && face.fV[0] == v0 &&
				 face.fT[2] == t1 && face.fT[0] == t0 )
				{
				face.fV[3] = v2;
				face.fT[3] = t2;
				if( FaceIsConvex( face ) )
					{
					faFaces[f] = face;
					return;
					}
				}
			}

   if( fnFaces == MAX_FACES )
      throw MeshError( fName, "Too many faces in mesh!" );

   // Add a face to the face list for the current mesh.

   faFaces[fnFaces].fV[0] = v0;
   faFaces[fnFaces].fV[1] = v1;
   faFaces[fnFaces].fV[2] = v2;
   faFaces[fnFaces].fV[3] = v2;
#ifndef MESH2
   faFaces[fnFaces].fCenter = center;
#endif
   faFaces[fnFaces].fT[0] = t0;
   faFaces[fnFaces].fT[1] = t1;
   faFaces[fnFaces].fT[2] = t2;
   faFaces[fnFaces].fT[3] = t2;
   faFaces[fnFaces].fMaterial = mat;
   faFaces[fnFaces].fPlaneCoefficientD = - m_dot( faVerts[center].fPoint,
                                                  faVerts[center].fNormal );
   faFaces[fnFaces].fUserData = 0;
   fnFaces++;
}

//---------------------------------------------------------------------------

Bool Mesh::GetCellInfoFromName( char *name )
{
   Bool success = TRUE;

   fSeq = -1;
   fPart = -1;
   fCell = 0;
   strcpy( fName, name );
   char *pCellInfo = strchr( fName, '.' );
   if( pCellInfo )
      {
      // must have ".SC"
      // where S is sequence number and C is cell number
      // or ".SCR"
      // where R is a reference to another cell number
      int sl = strlen( pCellInfo );
      if( sl >= 3 )
         {
         fSeq = pCellInfo[1] - 'a';
         fCell = pCellInfo[2] - 'a';
         if( fSeq < 0 || fSeq >= MAX_SEQUENCES ||
             fCell < 0 || fCell >= MAX_CELLS )
            throw MeshError( fName, "Invalid mesh name" );
         }
      if( sl == 4 )
         {
         fPart = pCellInfo[3] - 'a';
         if( fPart == fCell )
            throw MeshError( fName, "Cell cannot refer to itself" );
         if( fPart < 0 || fPart >= MAX_CELLS )
            throw MeshError( fName, "Invalid cell part reference" );
         }
      if( sl != 3 && sl != 4 ) 
         throw MeshError( fName, "Invalid mesh name" );

      // remove cell info from base name
      *pCellInfo = 0;
      }
   return success;
}

//--------------------------------------------------------------------

void Mesh::CalcCenterAndRadius( mesh3ds *p3dsMesh )
{
   if( p3dsMesh->nvertices == 0 )
      throw MeshError( fName, "Empty mesh - no vertices" );

   // Calculate the radius of the mesh as the diagonal extent
   // of the bounding box.  This value will tend to be larger
   // than the actual radius.  For the center, simply use the
   // center of the box.

   Point3F fmin, fmax;

   for( int i = 0; i < p3dsMesh->nvertices; i++ )
      {
      Point3F vert( p3dsMesh->vertexarray[i].x, 
                     p3dsMesh->vertexarray[i].y,
                     p3dsMesh->vertexarray[i].z );

      if( !i )
         {
         fmin = vert;
         fmax = vert;
         }
      else
         {
         fmin.setMin( vert );
         fmax.setMax( vert );
         }
      }

   fCenter = fmin;
   fCenter += fmax;
   fCenter /= 2.0f;

   fmax -= fmin;
   fRadius = fmax.len() / 2.0f;
}

//--------------------------------------------------------------------

Point3F & Mesh::CalcFaceNormal( int faceIndex, mesh3ds * p3dsMesh )
{
   face3ds *p3dsFace = &p3dsMesh->facearray[faceIndex];

   // calculate the normal vector for a face.

   // 3d studio assumes vertices are counterclockwise,
   // so the normal can be found by e1 cross e2, where
   // e1 is the vector from v1 to v2, and e2 is the vector
   // from v1 to v3:
   Point3F e1,e2;
   static Point3F normal;
   e1.x = p3dsMesh->vertexarray[p3dsFace->v2].x - 
          p3dsMesh->vertexarray[p3dsFace->v1].x;
   e1.y = p3dsMesh->vertexarray[p3dsFace->v2].y - 
          p3dsMesh->vertexarray[p3dsFace->v1].y;
   e1.z = p3dsMesh->vertexarray[p3dsFace->v2].z - 
          p3dsMesh->vertexarray[p3dsFace->v1].z;

   e2.x = p3dsMesh->vertexarray[p3dsFace->v3].x - 
          p3dsMesh->vertexarray[p3dsFace->v1].x;
   e2.y = p3dsMesh->vertexarray[p3dsFace->v3].y - 
          p3dsMesh->vertexarray[p3dsFace->v1].y;
   e2.z = p3dsMesh->vertexarray[p3dsFace->v3].z - 
          p3dsMesh->vertexarray[p3dsFace->v1].z;
   m_cross( e1, e2, &normal );

   // we want a unit normal, so divide by length:
   RealF len = normal.len();
   if(len <= .0000000001)
      {
      // Error: the length of the normal vector computed is too small
      // The probable cause is that the face is degenerate, that is
      // there are only two unique vertices or the face is a line.
      // There is not much we can do about this but complain.

      throw MeshError( fName, "Degenerate face - zero length normal" );
      }
   normal /= len;
   return normal;
}

//--------------------------------------------------------------------

Point3F & Mesh::VertexNormal( int vertexIndex, int faceIndex, mesh3ds *p3dsMesh )
{
   // Calculate the normal vector at a vertex.  This is done by
   // averaging the face normals for the current face and all other
   // faces which have the vertex in common and are also share a smoothing
   // group with the current face.

   int count = 0;
   static Point3F normal;
   if( p3dsMesh->smootharray )
      {
      normal.set();
      for( int fi = 0; fi < p3dsMesh->nfaces; fi++ )
         {
         face3ds * p3dsFace = & p3dsMesh->facearray[fi];
         if( p3dsFace->v1 == vertexIndex ||
             p3dsFace->v2 == vertexIndex ||
             p3dsFace->v3 == vertexIndex )
            {
            if( p3dsMesh->smootharray[faceIndex] & 
                p3dsMesh->smootharray[fi] )
               {
               normal += faFaceNormals[fi];
               count++;
               }
            }
         }
      }

   if( count )
      normal /= (RealF)count;
   else
      normal = faFaceNormals[faceIndex];
   return normal;
}

//--------------------------------------------------------------------

Point3F * Mesh::CalcFaceCenter( Point3F *pCenter, face3ds *p3dsFace, mesh3ds *p3dsMesh )
{
   // Calculate the centroid of the face

   pCenter->x = ( (
                  p3dsMesh->vertexarray[p3dsFace->v1].x +
                  p3dsMesh->vertexarray[p3dsFace->v2].x +
                  p3dsMesh->vertexarray[p3dsFace->v3].x ) / 3.0f );

   pCenter->y = ( ( 
                  p3dsMesh->vertexarray[p3dsFace->v1].y +
                  p3dsMesh->vertexarray[p3dsFace->v2].y +
                  p3dsMesh->vertexarray[p3dsFace->v3].y ) / 3.0f );

   pCenter->z = ( ( 
                  p3dsMesh->vertexarray[p3dsFace->v1].z +
                  p3dsMesh->vertexarray[p3dsFace->v2].z +
                  p3dsMesh->vertexarray[p3dsFace->v3].z ) / 3.0f );
   return pCenter;
}

void Mesh::ConvertFace( int faceIndex, DetailList *pDL, mesh3ds *p3dsMesh )
{
	RealF pointTolerance = pDL->fMergePointTolerance;
	RealF normalTolerance = pDL->fMergeNormalTolerance;

   // Convert a single face from the 3d Studio mesh to a face in our
   // own mesh format.
   int matIndex = pDL->FindMaterial( faceIndex, p3dsMesh, fDetailIndex );
   Int16 matTSIndex = pDL->GetMaterialTSIndex( matIndex );
   const Material & mat = pDL->GetMaterial( matIndex );

   face3ds * p3dsFace = &p3dsMesh->facearray[faceIndex];
   int center,v1,v2,v3,t1,t2,t3;
   if( mat.fTextured &&
       p3dsFace->v1 < p3dsMesh->ntextverts &&
       p3dsFace->v2 < p3dsMesh->ntextverts &&
       p3dsFace->v3 < p3dsMesh->ntextverts )
      {
      textvert3ds tv1 = p3dsMesh->textarray[p3dsFace->v1];
      textvert3ds tv2 = p3dsMesh->textarray[p3dsFace->v2];
      textvert3ds tv3 = p3dsMesh->textarray[p3dsFace->v3];
      t1 = AddTexture( tv1 );
      t2 = AddTexture( tv2 );
      t3 = AddTexture( tv3 );
      }
   else
      {
      t1 = t2 = t3 = 0;
      if( mat.fTextured )
         {
         StatusPrintf("Textured face missing texture coords");
         matTSIndex = 0;
         }
      }

   v1 = AddVertex( p3dsMesh->vertexarray[p3dsFace->v1], 
      VertexNormal( p3dsFace->v1, faceIndex, p3dsMesh ),
		pointTolerance, normalTolerance );

   v2 = AddVertex( p3dsMesh->vertexarray[p3dsFace->v2], 
      VertexNormal( p3dsFace->v2, faceIndex, p3dsMesh ),
		pointTolerance, normalTolerance );

   v3 = AddVertex( p3dsMesh->vertexarray[p3dsFace->v3], 
      VertexNormal( p3dsFace->v3, faceIndex, p3dsMesh ),
		pointTolerance, normalTolerance );

#ifndef MESH2
   Point3F ctrPnt;
   CalcFaceCenter( &ctrPnt, p3dsFace, p3dsMesh );
   center = AddVertex( ctrPnt, faFaceNormals[faceIndex],
		pointTolerance, normalTolerance );

   // 3d studio assumes vertices are counterclockwise, but we assume 
   // vertices are clockwise:
   AddTri( v3, v2, v1, center, t3, t2, t1, matTSIndex, normalTolerance );
#else
   AddTri( v3, v2, v1, 0, t3, t2, t1, matTSIndex, normalTolerance );
#endif
   // handle twosided materials by duplicating face:
   if( mat.fTwoSided )
      {
      Point3F normal;
      normal = VertexNormal( p3dsFace->v1, faceIndex, p3dsMesh );
      normal.neg();
      v1 = AddVertex( p3dsMesh->vertexarray[p3dsFace->v1], normal,
			pointTolerance, normalTolerance );

      normal = VertexNormal( p3dsFace->v2, faceIndex, p3dsMesh );
      normal.neg();
      v2 = AddVertex( p3dsMesh->vertexarray[p3dsFace->v2], normal,
			pointTolerance, normalTolerance );

      normal = VertexNormal( p3dsFace->v3, faceIndex, p3dsMesh );
      normal.neg();
      v3 = AddVertex( p3dsMesh->vertexarray[p3dsFace->v3], normal,
			pointTolerance, normalTolerance );

#ifndef MESH2
      normal = faFaceNormals[faceIndex];
      normal.neg();
      center = AddVertex( ctrPnt, normal,
			pointTolerance, normalTolerance );

      // 3d studio assumes vertices are counterclockwise, but we assume 
      // vertices are clockwise.  But here we are flipping the face,
      // so we want them counterclockwise:
      AddTri( v1, v2, v3, center, t1, t2, t3, matTSIndex, normalTolerance );
#else
      AddTri( v1, v2, v3, 0, t1, t2, t3, matTSIndex, normalTolerance );
#endif
      }
}

//---------------------------------------------------------------------------

void Mesh::ConvertFaces( DetailList * pDL, mesh3ds * p3dsMesh )
{
   fnFaces = 0;
   fnVerts = 0;
   fnTexVerts = 0;

   // get face normals:
   for( int i = 0; i < p3dsMesh->nfaces; i++ )
      faFaceNormals[i] = CalcFaceNormal( i, p3dsMesh );

   // convert the faces:
   for( i = 0; i < p3dsMesh->nfaces; i++ )
      ConvertFace( i, pDL, p3dsMesh );

}

//---------------------------------------------------------------------------

Mesh::Mesh( mesh3ds *p3dsMesh, DetailList *pDL, RealF shrink, Int32 detailIndex )
{
   fDetailIndex = detailIndex;

   GetCellInfoFromName( p3dsMesh->name );

   if( fPart == -1 )
      {
      CalcCenterAndRadius( p3dsMesh );
      ConvertFaces( pDL, p3dsMesh );
		SortFaces();
      fPolyhedron.AddVerts( p3dsMesh->vertexarray, p3dsMesh->nvertices );
      }
}

//--------------------------------------------------------------------

Mesh::~Mesh()
{
}

//--------------------------------------------------------------------

int Mesh::CompareFaces( int a, int b )
{
	TSMeshFace * faceA = &faFaces[a];
	TSMeshFace * faceB = &faFaces[b];

	int v;
	int AinFrontOfB = 0;
	int AinBackOfB = 0;
	int AsplitsB = 0;
	for( v = 0; v < 4; v++ )
		{
		RealF dist = m_dot( faVerts[faceA->fV[v]].fPoint, 
								  faVerts[faceB->fCenter].fNormal ) 
							+ faceB->fPlaneCoefficientD;
		if( dist > 0.001 )
			AinFrontOfB = 1;
		else if( dist < -0.001 )
			AinBackOfB = 1;
		}
	if( AinFrontOfB && AinBackOfB )
		AsplitsB = 1;

	int BinFrontOfA = 0;
	int BinBackOfA = 0;
	int BsplitsA = 0;
	for( v = 0; v < 4; v++ )
		{
		RealF dist = m_dot( faVerts[faceB->fV[v]].fPoint, 
								  faVerts[faceA->fCenter].fNormal ) 
							+ faceA->fPlaneCoefficientD;
		if( dist > 0.001 )
			BinFrontOfA = 1;
		else if( dist < -0.001 )
			BinBackOfA = 1;
		}
	if( BinFrontOfA && BinBackOfA )
		BsplitsA = 1;

	if( (AinFrontOfB && BinFrontOfA && !AsplitsB && !BsplitsA ) || 
		 (AinBackOfB && BinBackOfA && !AsplitsB && !BsplitsA ) ||
		 (AsplitsB && BsplitsA) )
		return -1;
	if( AinFrontOfB )
		return b;
	if( BinFrontOfA )
		return a;
	if( AinBackOfB )
		return a;
	if( BinBackOfA )
		return b;
	return -1;
}

void Mesh::SortFaces()
{
	// determine dependencies:
	Int32	* order = new Int32[fnFaces];
	char * before = new char[fnFaces * fnFaces];
   for( int faceA = 0; faceA < fnFaces; faceA++ )
		{
		before[faceA*fnFaces+faceA] = 0;	// mark as not yet put to ordered list
		for( int faceB = faceA + 1; faceB < fnFaces; faceB++ )
			{
			int compare = CompareFaces( faceA, faceB );
			if( compare == faceA )
				{
				before[faceA*fnFaces+faceB] = 0;
				before[faceB*fnFaces+faceA] = 1;
				}
			else if( compare == faceB )
				{
				before[faceA*fnFaces+faceB] = 1;
				before[faceB*fnFaces+faceA] = 0;
				}
			else
				{
				before[faceA*fnFaces+faceB] = 0;
				before[faceB*fnFaces+faceA] = 0;
				}
			}
		}

	int nPut = 0;
	while( nPut < fnFaces )
		{
		int putAnyThisPass = 0;
		for( int face = 0; face < fnFaces; face++ )
			if( !before[face*fnFaces+face] )
				{
				for( int faceBefore = 0; faceBefore < fnFaces; faceBefore++ )
					if( face != faceBefore &&
						 before[face*fnFaces+faceBefore] &&
						 !before[faceBefore*fnFaces+faceBefore] )
						break;
				if( faceBefore == fnFaces )
					{
					order[nPut++] = face;
					before[face*fnFaces+face] = 1;
					putAnyThisPass = 1;
					}
				}
		if( !putAnyThisPass )
			{
			StatusPrintf( "Unable to sort faces due to cyclic graph in mesh: '%s'", fName );
			break;
			}
		}
	delete before;

	if( nPut == fnFaces )
		{
		TSMeshFace * temp = new TSMeshFace[fnFaces];
		int face;
		for( face = 0; face < fnFaces; face++ )
			temp[face] = faFaces[order[face]];
		for( face = 0; face < fnFaces; face++ )
			faFaces[face] = temp[face];
		delete temp;
		}
	delete order;
}

//--------------------------------------------------------------------

TSPartBase * Mesh::MakeTSMesh()
{

	if( fnFaces == 0 )
      {
      TSNullPart * pTSNull = new TSNullPart();
      pTSNull->setCenter( fCenter );
      pTSNull->setRadius( fRadius );
      return pTSNull;
      }
#ifdef MESH2
   TSMesh2 * pTSMesh = new TSMesh2();
#else
   TSMesh * pTSMesh = new TSMesh();
#endif

   pTSMesh->setCenter( fCenter );
   pTSMesh->setRadius( fRadius );

   // set up TSMesh stuff:

   pTSMesh->setVertexList( faVerts, fnVerts );

   if( fnTexVerts )
      pTSMesh->setTextureCoordList( faTexVerts, fnTexVerts );

   pTSMesh->setFaceList( faFaces, fnFaces );
   return pTSMesh;
}

//--------------------------------------------------------------------
