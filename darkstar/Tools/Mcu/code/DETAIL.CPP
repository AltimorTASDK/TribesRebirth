#include "stdafx.h"
#include "detail.h"
#include "mcudoc.h"


//--------------------------------------------------------------------


Bool Detail::CanSplit( PList const & left, PList const & right )
{
   // we don't consider it splitting if one side is empty! it's cheating!!

   AssertFatal( left.Count() && right.Count(), "Empty list" );
   
   // try a particular partitioning of a subset of the parts in the shape
   // to see if a separating plane can be found:

   static Polyhedron p1, p2;

   // First, do a crude check, building polyhedra from the center points
   // of the parts.  If these polyhedra intersect, we know the sets of parts
   // must also intersect:

   p1.Reset();
   p2.Reset();
   for( int i = 0; i < left.Count(); i++ )
      p1.AddVert( faParts[left.GetPart(i)].fPolyhedron.GetCentroid() );
   for( i = 0; i < right.Count(); i++ )
      p2.AddVert( faParts[right.GetPart(i)].fPolyhedron.GetCentroid() );

   Couple c(&p1, &p2);
   if( c.Collision() )
      return FALSE;

   // Passed the crude check, so try a more refined check.
   // The polyhedra corresponding to the parts are combined to build
   // two polydedra, one for each subset of parts in this partitioning.
   // The the resulting polyhedra (which have many vertices) are tested
   // for intersection.

   c.Reset();
   p1.Reset();
   p2.Reset();

   for( i = 0; i < left.Count(); i++ )
      p1.AddVerts( &faParts[left.GetPart(i)].fPolyhedron );
   for( i = 0; i < right.Count(); i++ )
      p2.AddVerts( &faParts[right.GetPart(i)].fPolyhedron );

   // Passed the refined check?
   if( !c.Collision() )
      {
      // The sets of parts don't intersect, so we need to get the
      // separating plane for use in the BSP tree:

      c.GetPlane(&fLastSplitNormal, &fLastSplitCoeff);
      fLastSplitLeft = left;
      fLastSplitRight = right;
      return TRUE;
      }
   return FALSE;
}

Bool Detail::TrySplit( PList left, PList right, PList more )
{
   // on entry, left or right or both must have at least one part
   // and be splitable from each other, and more must have at least
   // one more part in it.

   // we explore first adding another part to the left branch
   // if we don't find a solution that way, we move that part to
   // the right branch and try again.  If we can't find a solution
   // either way, we've come down a dead end.

   AssertFatal( more.Count() > 0 && right.Count() > 0, "Empty list" );
   Bool done = FALSE;
   left.AddPart( more.RemovePart() );

   if( CanSplit( left, right ) )
      {
      if( more.Count() )
         done = TrySplit( left, right, more );
      else
         done = TRUE;
      }

   if( !done )
      {
      right.AddPart( left.RemovePart() );
      if( !left.Count() )
         {
         if( more.Count() )
            done = TrySplit( left, right, more );
         else
            done = FALSE;
         }
      else if( CanSplit( left, right ) )
         {
         if( more.Count() )
            done = TrySplit( left, right, more );
         else
            done = TRUE;
         }
      }
   return done;
}

int Detail::BuildSortTree( int* pNodeIndex, PList & parts )
{
   // Try to build a sort tree for the list of parts.
   // Begin by trying to partition the parts into two subsets.
   // If no partitioning is found, report the error.
   // If a partitioning is found, recursively apply the process
   // two each partition.
   
   PList    left, right;
   right.AddPart( parts.RemovePart() );
   if( TrySplit( left, right, parts ) )
      {
      faNodes[*pNodeIndex].fNormal = fLastSplitNormal;
      faNodes[*pNodeIndex].fCoeff = fLastSplitCoeff;
      left = fLastSplitLeft;
      right = fLastSplitRight;

      //StatusPrintf("Got plane for node index %d", *pNodeIndex );

      int thisNodeIndex = (*pNodeIndex)++;
      int success = 1;

      // recursively do the same
      // Process the left/front partition:
      if (left.Count() == 1)
         faNodes[thisNodeIndex].fFrontChild = 
            left.GetPart(0) | TSBSP_CHILD_PART;
      else
         {
         faNodes[thisNodeIndex].fFrontChild = 
            (*pNodeIndex) | TSBSP_CHILD_NODE;
         if( !BuildSortTree( pNodeIndex, left ) )
            success = 0;
         }

      // process the right/back partition:
      if (right.Count() == 1)
         faNodes[thisNodeIndex].fBackChild = 
            right.GetPart(0) | TSBSP_CHILD_PART;
      else
         {
         faNodes[thisNodeIndex].fBackChild =
            (*pNodeIndex) | TSBSP_CHILD_NODE;
         if( !BuildSortTree( pNodeIndex, right ) )
            success = 0;
         }
      return success; // did both sides sort?
      }
   else
      {
      StatusPrintf("Unable to sort the following parts:");
      int i;
      for( i = 0; i < left.Count(); i++ )
         StatusPrintf( "left #%d: %s", i, faParts[left.GetPart(i)].fName );
      for( i = 0; i < right.Count(); i++ )
         StatusPrintf( "right #%d: %s", i, faParts[right.GetPart(i)].fName );
      for( i = 0; i < parts.Count(); i++ )
         StatusPrintf( "remaining #%d: %s", i, faParts[parts.GetPart(i)].fName );
      return 0;
      }
}

//---------------------------------------------------------------------------

Detail::Detail()
{
   fName[0] = 0;
   fShrink = RealF( 1.0 );
   fScale = RealF( 1.0 );
   fDetailSize = 0.0f;
   fSort = TRUE;

   fnParts = 0;
   fDatabase = -1;
	fnFaces = 0;
	fnVerts = 0;
}

//---------------------------------------------------------------------------

Detail::~Detail()
{
   Reset();
}

//---------------------------------------------------------------------------

void Detail::Reset()
{
   for( int p = 0; p < MAX_PARTS; p++ )
      faParts[p].Reset();
   fnParts = 0;
   fDatabase = -1;
   fMaxSeq = -1;
	fnFaces = 0;
	fnVerts = 0;
}

//---------------------------------------------------------------------------

void Detail::ShallowCopy( const Detail & other )
{
   strcpy( fName, other.fName );
   fShrink = other.fShrink;
   fScale = other.fScale;
   fDetailSize = other.fDetailSize;
   fSort = other.fSort;
}

//---------------------------------------------------------------------------

Bool Detail::LoadDatabase( DetailList * pDL )
{
	if( ((CMcuApp*)AfxGetApp())->fUsePaths )
		strcpy( fActualName, fName );
	else
		{
		strcpy( fActualName, pDL->fPath );
		char *file = strrchr( fName, '\\' );
		if( !file )
			file = fName;
		else
			file++;
		strcat( fActualName, file );
		}

	fDatabase = DatabaseOpen( fActualName );
	if( fDatabase == -1 )
      {
      StatusPrintf( "Unable to open database: '%s'", fActualName );
      return FALSE;
      }
   return TRUE;
}

//---------------------------------------------------------------------------

void Detail::ReleaseDatabase()
{
  	if( fDatabase != -1 )
      {
     	DatabaseClose( fDatabase );
      fDatabase = -1;
      }
}

//---------------------------------------------------------------------------

Bool Detail::GetMaterials( DetailList * pDL )
{
   Bool success = TRUE;
   for( int mi = 0; mi < DatabaseGetMaterialCount( fDatabase ); mi++ )
      {
      material3ds * mat3ds = DatabaseGetMaterial( fDatabase, mi );

         if( mat3ds == 0 )
            throw GeneralError( "Missing material!" );

         try
            {
            pDL->AddMaterial( Material( mat3ds, pDL->GetPath(), fDetailIndex ) );
            }
         catch( GeneralError g )
            {
            StatusPrintf( "Error: %s\n", g.type );
            success = FALSE;
            }

         DatabaseReleaseMaterial( mat3ds );
      }
   return success;
}

//---------------------------------------------------------------------------

void Detail::InsertMesh( Mesh * pMesh )
{
	fnFaces += pMesh->fnFaces;
	fnVerts += pMesh->fnVerts;

   for( int p = 0; p < fnParts; p++ )
      if( !strcmp( pMesh->fName, faParts[p].fName ) )
         break;
   faParts[p].AddCell( pMesh );
   if( p == fnParts )
      fnParts++;
   if( pMesh->fSeq > fMaxSeq )
      fMaxSeq = pMesh->fSeq;
}

//---------------------------------------------------------------------------

void Detail::ValidateCells()
{
   StatusPrintf( "Detail: %s", fActualName );

   for( int p = 0; p < fnParts; p++ )
      {
      faParts[p].ValidateCells();
      /*
      if( faParts[p].fSeq != -1 )
         StatusPrintf( "Part %d: %-11s seq:%d cells:%d",
            p, faParts[p].fName, faParts[p].fSeq, faParts[p].fnCells );
      else
         StatusPrintf( "Part %d: %-11s", p, faParts[p].fName );
      */
      }
}

//---------------------------------------------------------------------------

void Detail::ScaleMesh3ds( mesh3ds *pMesh )
{
   for( int v = 0; v < pMesh->nvertices; v++ )
      {
      pMesh->vertexarray[v].x *= fScale;
      pMesh->vertexarray[v].y *= fScale;
      pMesh->vertexarray[v].z *= fScale;
      }
}

//---------------------------------------------------------------------------

void Detail::GetBoundingSphere()
{
      // Find the center and radius of the detail.
      // The center calculated is just the centroid of all vertices.
      // The radius is half of the diagonal extent of the bounding box
      // of all vertices.

   Point3F minVal, maxVal;

   int nMeshes = DatabaseGetMeshCount( fDatabase );

   for( int mi = 0; mi < nMeshes; mi++ )
      {
      mesh3ds * pMesh = DatabaseGetMesh( fDatabase, mi );
      ScaleMesh3ds( pMesh );
        
      if( pMesh->nvertices < 1 )
         throw MeshError( pMesh->name, "Empty mesh - no vertices" );
      else
         {
         for( int j = 0; j < pMesh->nvertices; j++ )
            {
            // get point:
            Point3F vert( pMesh->vertexarray[j].x,
                           pMesh->vertexarray[j].y,
                           pMesh->vertexarray[j].z );

            if( !mi && !j )
               {
               // set initial bounding box to include just first point:
               minVal = vert;
               maxVal = vert;
               }

            // grow bounding box to include additional points:
            maxVal.setMax( vert );
            minVal.setMin( vert );
            }
         }
      DatabaseReleaseMesh( pMesh );
      }
   fCenter = minVal;
   fCenter += maxVal;
   fCenter /= 2.0f;

   maxVal -= minVal;

   fRadius = maxVal.len() / 2;
}

//---------------------------------------------------------------------------

void Detail::CheckCollisions()
{
   if( fSort && fnParts > 1 )
      {
      fSortNow = TRUE;
      //StatusPrintf( "Checking collisions in detail %s", fName );

      int collisions = 0;
      for( int i = 0; i < fnParts - 1; i++ )
         for( int j = i + 1; j < fnParts ; j++ )
            {
            
            Couple c( &faParts[i].fPolyhedron, &faParts[j].fPolyhedron );
            if( c.Collision() )
               {
               collisions++;
               fSortNow = FALSE;
               StatusPrintf( "Collision: %d: %s with %d: %s", 
                     i, faParts[i].fName, j, faParts[j].fName );
               }
            }

      if( collisions )
         StatusPrintf("Unable to sort detail.  Continuing without sorting.");
      //else
      //   StatusPrintf("No collisions found.");
      }
   else
      fSortNow = FALSE;
}

//---------------------------------------------------------------------------

void Detail::ApplyShrinkage()
{
   for( int i = 0; i < fnParts; i++ )
      faParts[i].fPolyhedron.Scale( fShrink );
}

//---------------------------------------------------------------------------

void Detail::SortParts()
{
   //---------------------------------------------------------
   // now, if sorting is still called for, try to build a tree:
   if( fSortNow )
      {
      PList parts;
      for( int i = 0; i < fnParts; i++ )
         parts.AddPart( i );
      fnNodes = 0;
      if( !BuildSortTree( &fnNodes, parts ) )
         {
         StatusPrintf("Unable to sort shape.  Converting without sorting.\n");
         fSortNow = 0;
         }
      }
}

//---------------------------------------------------------------------------

TSPartBase * Detail::MakeTSPart()
{
   //---------------------------------------------------------
   // now, if sorting is still called for, build a BSPPart:
   if( fSortNow )
      {
      TSBSPPart * pBSP = new TSBSPPart();

      pBSP->setCenter( fCenter );
      pBSP->setRadius( fRadius );

      for( int i = 0; i < fnParts; i++ )
         pBSP->addPart( faParts[i].MakeTSCellAnimPart() );

      pBSP->setNodes( faNodes, fnNodes );

      //---------------------------------------------------------
      // we don't need transforms for the sorting planes:
      Int32 *pNodeTransforms = new Int32[fnNodes];
      for( i = 0; i < fnNodes; i++ )
         pNodeTransforms[i] = -1;
      pBSP->setNodeTransforms( pNodeTransforms );
      delete pNodeTransforms;

      return pBSP;
      }

   //---------------------------------------------------------
   // otherwise, just a mesh or part list:
   else
      {
      if( fnParts == 1 )
         {
         return faParts[0].MakeTSCellAnimPart();
         }
      else
         {
         TSPartList * pPartList = new TSPartList;

         pPartList->setCenter( fCenter );
         pPartList->setRadius( fRadius );

         for( int i = 0; i < fnParts; i++ )
            pPartList->addPart( faParts[i].MakeTSCellAnimPart() );
         return pPartList;
         }
      }

   return 0;
}

//---------------------------------------------------------------------------

void Detail::GetSequenceLengths( Int32 * pLengths )
{
   for( int i = 0; i < fnParts; i++ )
      faParts[i].GetSequenceLengths( pLengths );
}

//---------------------------------------------------------------------------

void Detail::GetMeshes( DetailList * pDL )
{
   GetBoundingSphere();

   int nMeshes = DatabaseGetMeshCount( fDatabase );

   for( int mi = 0; mi < nMeshes; mi++ )
      {
      mesh3ds * p3dsMesh = DatabaseGetMesh( fDatabase, mi );
      ScaleMesh3ds( p3dsMesh );
      
      Mesh * pMesh = new Mesh( p3dsMesh, pDL, fShrink, fDetailIndex );

      DatabaseReleaseMesh( p3dsMesh );

      if( !pMesh )
         throw AllocError();

      InsertMesh( pMesh );
      }

   ValidateCells();
   ApplyShrinkage();
   CheckCollisions();
   SortParts();
   StatusPrintf( "Number of Vertices: %d   Faces: %d", fnVerts, fnFaces );
}

//---------------------------------------------------------------------------

DetailList::DetailList()
{
   fpDoc = 0;

   fpMeshes = 0;

   Reset();
   BuildReset();
}

//---------------------------------------------------------------------------

void DetailList::Reset()
{
   fnDetails = 0;
   fAlphaNumLevels = 1;
   fAlphaNumColors = 1;
   fMergePointTolerance = 0.0f;
   fMergeNormalTolerance = 0.0f;
}

void DetailList::BuildReset()
{
   fnMats = 0;
   fnTSMats = 0;
   delete fpMeshes;
   fpMeshes = 0;
	for( int di = 0; di < MAX_DETAILS; di++ )
      faDetails[di].Reset();
   fMaxSeq = -1;
}

//---------------------------------------------------------------------------

void DetailList::SetBaseName( const char *base )
{
   AssertFatal( strlen( base ) < sizeof( fBaseName ),
      "File name too long" );
   strcpy( fBaseName, base );
	if( strrchr( fBaseName, '.' ) )
	   *strrchr( fBaseName, '.' ) = 0;
   strcpy( fPath, fBaseName );
   if( strrchr( fPath, '\\' ) )
      *(1 + strrchr( fPath, '\\' )) = 0;
   else
      fPath[0] = 0;
}

//---------------------------------------------------------------------------

int DetailList::GetCount()  const
{ 
   return fnDetails; 
}

//---------------------------------------------------------------------------

int DetailList::InsertDetail( int index, const Detail & d )
{
	fpDoc->SetModifiedFlag();
	// nothing selected? just add to end of list:
	if( index < 0 )
		{
		faDetails[fnDetails].ShallowCopy( d );
		return fnDetails++;
		}

	// move up everything from the selection point on by one
	for( int i = fnDetails; i > index; i-- )
		faDetails[i].ShallowCopy( faDetails[i-1] );
	faDetails[index].ShallowCopy( d );
	fnDetails++;
	return index;
}

//---------------------------------------------------------------------------

int DetailList::DeleteDetail( int index )
{
	fpDoc->SetModifiedFlag();
	for( int i = index; i < fnDetails - 1; i++ )
			faDetails[i].ShallowCopy( faDetails[i+1] );
	fnDetails--;
	if( index < fnDetails )
		return index;
	else
		return fnDetails - 1;
}

//---------------------------------------------------------------------------

Bool DetailList::SortDetails()
{
	fpDoc->SetModifiedFlag();
	Bool swap = FALSE;
	// sort in order of increasing detail size
	if( fnDetails < 2 )
		return swap;

	// simple sort:
	for( int i = 0; i < fnDetails - 1; i++ )
		for( int j = i + 1; j < fnDetails; j++ )
			if( faDetails[i].fDetailSize > faDetails[j].fDetailSize )
				{
				Detail temp;
				temp.ShallowCopy( faDetails[i] );
				faDetails[i].ShallowCopy( faDetails[j] );
				faDetails[j].ShallowCopy( temp );
				swap = TRUE;
				}
	return swap;

}

//---------------------------------------------------------------------------

void DetailList::BuildShape()
{
	StatusClear();

   if( !fnDetails )
      {
      StatusPrintf( "No details defined!" );
      }
  	else
      {
      SetBaseName( fpDoc->GetPathName() );
	
	   if( LoadDatabases() )
	      ProcessDatabases();
      ReleaseDatabases();
	   StatusPrintf( "Build complete" );
      }
   BuildReset();
}

//---------------------------------------------------------------------------

Bool DetailList::LoadDatabases()
{
   Bool success = TRUE;

	// try to load all 3ds databases:
	StatusPrintf( "Loading Databases" );
	for( int di = 0; di < fnDetails; di++ )
      {
      faDetails[di].fDetailIndex = di;
      success = success && faDetails[di].LoadDatabase( this );
      }
   return success;
}

//---------------------------------------------------------------------------

void DetailList::ReleaseDatabases()
{
   StatusPrintf( "Releasing Databases" );
	for( int di = 0; di < fnDetails; di++ )
      faDetails[di].ReleaseDatabase();
}

//---------------------------------------------------------------------------

void DetailList::ProcessDatabases()
{
   try
      {
      GetMaterials();
      SaveMaterials();
      GetMeshes();
      MakeTSShape();
      }
   catch( AllocError )
      {
      StatusPrintf( "Out of memory!" );
      }
   catch( MeshError e)
      {
      StatusPrintf( "Mesh Error %s: %s", e.name, e.type );
      }
   catch( GeneralError e)
      {
      StatusPrintf( "General Error: %s", e.type );
      }
#if 0
   catch( ... )
      {
      StatusPrintf( "Something very bad happened!" );
      }
#endif
}

//---------------------------------------------------------------------------

void DetailList::GetMeshes()
{
	if( !fnDetails )
		return;

   for( int di = 0; di < fnDetails; di++ )
      {
      faDetails[di].GetMeshes( this );
      if( fMaxSeq < faDetails[di].fMaxSeq )
         fMaxSeq = faDetails[di].fMaxSeq;
      if( !di )
         {
			fCenter = faDetails[di].fCenter;
			fRadius = faDetails[di].fRadius;
         }
      else
			{
			SphereF s( fCenter , fRadius );
			SphereF sd( faDetails[di].fCenter, faDetails[di].fRadius ); 
			s.adjustToEnclose( sd );
			fCenter = s.center;
			fRadius = s.radius;
			}
      }
}        

//---------------------------------------------------------------------------

TSPartBase * DetailList::MakeTSDetailPart()
{
   if( fnDetails == 1 )
      return faDetails[0].MakeTSPart();

   TSDetailPart * pDetailPart = new TSDetailPart();

   pDetailPart->setCenter( fCenter );
   pDetailPart->setRadius( fRadius );

   for( int di = 0; di < fnDetails; di++ )
      pDetailPart->addPart( faDetails[di].MakeTSPart() );

   RealF * pSizes = new RealF[fnDetails];
   for( di = 0; di < fnDetails; di++ )
      pSizes[di] = faDetails[di].fDetailSize;
   pDetailPart->setSizeList( pSizes );
   delete pSizes;

   return pDetailPart;
}

//---------------------------------------------------------------------------

Int32 * DetailList::GetSequenceLengths()
{
   Int32 * pLengths = new Int32[fMaxSeq + 1];
   for( int s = 0; s < fMaxSeq + 1; s++ )
      pLengths[s] = 0;
   for( int di = 0; di < fnDetails; di++ )
      faDetails[di].GetSequenceLengths( pLengths );
   return pLengths;
}

//---------------------------------------------------------------------------

void DetailList::MakeTSShape()
{
   TSShape *pShape = new TSShape();

   pShape->setCenter( fCenter );
   pShape->setRadius( fRadius );

   pShape->addPart( MakeTSDetailPart() );

   if( fMaxSeq != -1 )
      {
      Int32 * pLengths = GetSequenceLengths();
      pShape->setSequenceLengthArray( pLengths, fMaxSeq + 1 );
      delete pLengths;
      }

   char outName[256];
   strcpy( outName, fBaseName );
   strcat( outName, ".dts" );
   StatusPrintf( "Writing shape to: %s", outName );
   remove( outName );
	FileWStream	ost( outName );
   pShape->writeItem( ost );
   delete pShape;
}

//---------------------------------------------------------------------------
