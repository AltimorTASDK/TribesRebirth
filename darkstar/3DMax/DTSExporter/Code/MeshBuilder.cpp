#include "ShapeBuilder.H"

#define MAX_DUMPED_VERTS    1000

//--------------------------------------------------------------

CelAnimMeshBuilder::CelAnimMeshBuilder( ShapeBuilder * psb, int name, int node, 
    MyTransform const & oo, Bool IsShape, Int16 flags )
{
   fpSB = psb;
   fFlags = flags;
   fName = name;
   fNodeIndex = node;
   fnSubSequences = 0;
   fFirstSubSequence = 0;
   fnVertsPerFrame = 0;
   oo.setMatrix( &fObjectOffset );
   fFrameStart = 0;
   fIsShape = IsShape;
   fTexFrameStart = 0;
   vertList.setSize(0);
   faceList.setSize(0);
   frameList.setSize(0);
   fCountXLucent = 0;
}

CelAnimMeshBuilder::~CelAnimMeshBuilder()
{
}

//--------------------------------------------------------------

void CelAnimMeshBuilder::AddKey( FILE *fp, ShapeBuilder & sb, int time, 
                                            float pos, Bool visibility, int changes )
{
   int tex_frame = 0;
    
    if ( (changes & MAT_CHANGES) && fnTexVertsPerFrame )
        tex_frame = (fTexFrameStart / fnTexVertsPerFrame);

    TS::Shape::Keyframe key( pos, frameList.size(), tex_frame, 
                            (changes & VIS_CHANGES) != 0, visibility, 
                                     (changes & MAT_CHANGES) != 0, 
                                     (changes & MESH_CHANGES) != 0 );
    sb.keyframes.push_back( key );
   
#ifdef SDUMP   
    fprintf( fp, "Obj key @ %d, %8.3f %%:, vis=%d, frm=%d, mat=%d, firstVert = %d\n", time,  
         pos * 100.0f, visibility, frameList.size(), tex_frame, fFrameStart );
   fprintf( fp, "   vis_used = %d, frm_used = %d, mat_used = %d\n", 
             (changes & VIS_CHANGES) != 0, (changes & MESH_CHANGES) != 0, 
                (changes & MAT_CHANGES) != 0 );
#endif
   
   if ( changes & MESH_CHANGES )
   {
       TS::CelAnimMesh::Frame frame;
       frame.fFirstVert = fFrameStart;
       frameList.push_back( frame );
   }
}

//--------------------------------------------------------------

void CelAnimMeshBuilder::AddSubSequence( ShapeBuilder & sb, int seq, int first, int count )
{
    TS::Shape::SubSequence ss;
    ss.fSequenceIndex = seq;
    ss.fFirstKeyframe = first;
    ss.fnKeyframes = count;
    if( !fnSubSequences )
        fFirstSubSequence = sb.subsequences.size();
    fnSubSequences++;
    sb.subsequences.push_back( ss );
}

//--------------------------------------------------------------

TS::Shape::Mesh * CelAnimMeshBuilder::MakeMesh(FILE *fp)
{
   if( !vertList.size() || (!fIsShape && !faceList.size()) )
      return 0;

    if( !textureVertList.size() && !fIsShape )
        textureVertList.push_back( Point2F( 0.0f, 0.0f ) );

    TS::CelAnimMesh * pMesh = new TS::CelAnimMesh;
    
    pMesh->fnVertsPerFrame = fnVertsPerFrame;
   pMesh->fnTextureVertsPerFrame = fnTexVertsPerFrame;
    pMesh->fFaces = faceList;
    pMesh->fFrames = frameList;
    pMesh->fTextureVerts = textureVertList;
    int f, v;

    Vector<TS::CelAnimMesh::Face>::iterator it = faceList.begin();
   int old_mat = -1;

   // Mark the faces where the material changes...
   while ( it != faceList.end() )
   {
      if ( it->fMaterial != old_mat )
         old_mat = it->fMaterial;
      else
         it->fMaterial = -1;     // flag to not set the material
   
      it++;
   }

    Point3F totalMax = vertList[ pMesh->fFrames[0].fFirstVert ].fPoint;
    Point3F totalMin = totalMax;
    Point3F maxScale( 0.0, 0.0, 0.0 );

   int size = vertList.size();
   if ( size > MAX_DUMPED_VERTS )
      size = MAX_DUMPED_VERTS;

    for ( f = 0; f < pMesh->fFrames.size(); f++ )
    {
        TS::CelAnimMesh::Frame &frm = pMesh->fFrames[f];
        int i = frm.fFirstVert;
        Point3F minVert = vertList[i++].fPoint;
        Point3F maxVert = minVert;
    
       for ( v = 1; v < fnVertsPerFrame; v++, i++ )
        {
            Point3F &pt = vertList[i].fPoint;
            minVert.setMin( pt );
            maxVert.setMax( pt );
            totalMin.setMin( pt );
            totalMax.setMax( pt );
        }
        
#ifdef SDUMP
        fprintf( fp, "frame #%d:\n", f );
        fprintf( fp, "   firstVert: %d\n", frm.fFirstVert );
       fprintf( fp, "   vertex min: %8.3f, %8.3f, %8.3f\n", 
                    minVert.x, minVert.y, minVert.z );
          fprintf( fp, "   vertex max: %8.3f, %8.3f, %8.3f\n", 
                    maxVert.x, maxVert.y, maxVert.z );
#endif

       maxVert -= minVert;
        Point3F tempscale;
        tempscale.set( maxVert.x/255.0f, maxVert.y/255.0f, maxVert.z/255.0f );
        
#ifdef SDUMP
          fprintf( fp, "scale: %8.3f, %8.3f, %8.3f\n", tempscale.x, tempscale.y, 
                    tempscale.z );
#endif
        
        frm.fOrigin = minVert;
        frm.fScale = tempscale;
        maxScale.setMax( tempscale );
    }

    maxScale *= (256.0f / 254.0f);
    Point3F &org = pMesh->fFrames[0].fOrigin;
    totalMax -= totalMin;   

#ifdef SDUMP
    fprintf( fp, "common scale: %8.3f, %8.3f, %8.3f\n", maxScale.x, maxScale.y,
                maxScale.z );
#endif

    for ( f = 0; f < pMesh->fFrames.size(); f++ )
    {
        TS::CelAnimMesh::Frame &frm = pMesh->fFrames[f];
        int j = frm.fFirstVert;

        // keep a unique origin and scale for each frame.
       for ( v = 0; v < fnVertsPerFrame; v++, j++ )
        {
            TS::PackedVertex pv;
            pv.set( vertList[j], frm.fScale, frm.fOrigin );

            if ( j >= pMesh->fVerts.size() )
	            pMesh->fVerts.push_back( pv );
#ifdef SDUMP
            else
            {
               fprintf( fp, "back-indexed vert\n" );
               if ( pv.x != pMesh->fVerts[j].x )
                  fprintf( fp, "   x coord doesn't match!!!\n" );
               if ( pv.y != pMesh->fVerts[j].y )
                  fprintf( fp, "   y coord doesn't match!!!\n" );
               if ( pv.z != pMesh->fVerts[j].z )
                  fprintf( fp, "   z coord doesn't match!!!\n" );
            }
            
            if ( j < size )
            {
               fprintf( fp, "Vert %d(%d): %d, %d, %d, %d\n", v, j, pv.x, pv.y, pv.z, pv.normal );
               fprintf( fp, "  point: %8.3f, %8.3f, %8.3f\n", vertList[j].fPoint.x,
                   vertList[j].fPoint.y, vertList[j].fPoint.z );
               fprintf( fp, "  normal: %8.3f, %8.3f, %8.3f\n", vertList[j].fNormal.x,
                   vertList[j].fNormal.y, vertList[j].fNormal.z );
            }
#endif
        }
    }

   // this looks like 2x radius actually...
   pMesh->fRadius = totalMax.len();


   // Note this IsShape flag reall just means it's the bounding box.
   if( fIsShape )
   {
      Point3F res;
      
      // totalMax at this point has the dimensions, so we take out totalMin.  
      //    then adjust by the offset.  Seems the bounds node can have
      //    scale values.  
      fpSB->fBounds.fMin = totalMin;
      fpSB->fBounds.fMax = totalMax;
      fpSB->fBounds.fMax += totalMin;
      // fpSB->fBounds += &fObjectOffset.p;
      m_mul( fpSB->fBounds.fMin, fObjectOffset, &res );
      fpSB->fBounds.fMin = res;
      m_mul( fpSB->fBounds.fMax, fObjectOffset, &res );
      fpSB->fBounds.fMax = res;
      
      // Add in our hack bounds box offset.  Note we don't want this part of the normal
      //    object offset xform as it upsets other code.  
      fpSB->fBounds.fMax += fpSB->boundsBoxOffset;
      fpSB->fBounds.fMin += fpSB->boundsBoxOffset;
      
      // Center and radius calculation:
      Point3F  cntr = fpSB->fBounds.fMax;
      cntr -= fpSB->fBounds.fMin;               // box dimensions.
      cntr /= 2;                                // half of box dimensions.
      fpSB->fRadius = cntr.len();               // distance out to corner.  
      cntr += fpSB->fBounds.fMin;               // get the center.  
      fpSB->fCenter = cntr;
      
      // totalMax /= 2;
      // fpSB->fCenter += totalMax;
      // m_mul( fpSB->fCenter, fObjectOffset, &res );
      // fpSB->fCenter = res;
      // fpSB->fRadius = pMesh->fRadius / 2;
      
      sDump( fp, "Shape center: (%8.3f, %8.3f, %8.3f)\n", cntr.x, cntr.y, cntr.z );
      sDump( fp, "Shape radius: %8.3f\n", fpSB->fRadius );
   }
   else
      sDump( fp, "Mesh radius: %8.3f\n", pMesh->fRadius / 2.0 );

   return pMesh;
}


//--------------------------------------------------------------

enum MaterialFlags
{
   MAT_FLAT_SHADED = 0x01,
   MAT_TRANSPARENT = 0x02,
};

static int cmpVert( const TS::Vertex &a, const TS::Vertex &b, 
                    Bool check_norm = TRUE )
{                  
   return( a.fPoint == b.fPoint && (!check_norm || a.fNormal == b.fNormal) );
}

static void copyVert( TS::Vertex *a, const TS::Vertex &b, 
                      Bool copy_norm = TRUE )
{                  
   a->fPoint = b.fPoint;
   
   if ( copy_norm )
      a->fNormal = b.fNormal;
}

static Point3F &calcNormal( Point3F *points )
{
   static Point3F normal;
    Point3F kj,lj;

   kj = points[ 0 ];
   kj -= points[ 1 ];
   lj = points[ 2 ];
   lj -= points[ 1 ];
   m_cross( lj, kj, &normal );
   normal.normalize();
   return normal;
}   

static TS::Vertex &CalcFlatVertex( Mesh &maxmesh, unsigned long *vi, int vert )
{
   static TS::Vertex vertex;   
   Point3F points[ 3 ];
   
   for ( int i = 0; i < 3; i++ )
   {
      points[i].x = maxmesh.verts[ vi[i] ][0];
      points[i].y = maxmesh.verts[ vi[i] ][1];
      points[i].z = maxmesh.verts[ vi[i] ][2];
   }
   
   vertex.fPoint = points[ vert ];
   vertex.fNormal = calcNormal( points );
   return vertex;
}

void CelAnimMeshBuilder::addVerts( Mesh &maxmesh, Face *maxface, 
                                  int face, int flags )
{                 
   int smooth = maxface->smGroup;
   TS::Vertex vert[ 5 ];
   TS::Vertex *vp = vert;
   int i;
   FaceVert *fv = &faceVerts[ face ];
   int vert_off = 0;
   
   for ( i = 0; i < 3; i++ )
   {
      if ( flags & MAT_FLAT_SHADED )
         vert[ i ] = CalcFlatVertex( maxmesh, maxface->v, i );
      else 
         vert[ i ] = CalcVertex( maxmesh, smooth, maxface->v[i] );
   }         

#if 0
   // in case we want to reorder the verts.
   vert[ 3 ] = vert[ 0 ];
   vert[ 4 ] = vert[ 1 ];

   // if flat shading, first see if any verts match completely, including
   //  their normals.  If so, rearrange them so that one comes first.
   if ( flags & MAT_FLAT_SHADED )
   {
      for ( i = 0; i < 3; i++ )
      {
          for ( int j = 2; j < fnVertsPerFrame; j++ )
          {
            TempVert &tv = tempVerts[ j ];
         
            if ( cmpVert( tv.vertex, vert[ i ], tv.uses_normal() ) )
               break;
         }
               
         if ( j < fnVertsPerFrame )
            break;         
      }         
      
      if ( i < 3 )
      {
         vert_off = i;
         vp += i;
      }
   }
#endif

   fv->vert_offset = vert_off;
   Bool check_norm = TRUE;
   Bool trans = flags & MAT_TRANSPARENT;
   
   // now find the verts in the list, or add them.
   //  set the face indices accordingly
   for ( i = 0; i < 3; i++, vp++ )
   {
      int j;
   
      if ( trans )
         j = fnVertsPerFrame; // always duplicate verts of transparent polys
      else        
          for ( j = 2; j < fnVertsPerFrame; j++ )
          {
            TempVert &tv = tempVerts[ j ];
            
            if ( !tv.unsharable() )
               if ( cmpVert( tv.vertex, *vp, check_norm && tv.uses_normal() ) )
                  break;
          }
   
      if ( j == fnVertsPerFrame )
      {
         tempVerts.setSize( ++fnVertsPerFrame );
         tempVerts[ j ].vertex = *vp;
         tempVerts[ j ].set_unsharable( trans );
         tempVerts[ j ].set_uses_normal( FALSE );
      }

      fv->v[i] = j;
      fv->uses_normal[i] = check_norm;
      
      if ( check_norm )
      {
         tempVerts[ j ].set_uses_normal( TRUE );
         
         // Only check the first point's normal for flat shading         
         if ( flags & MAT_FLAT_SHADED )
            check_norm = FALSE;            
      }
   }
}

static int getMaterialFlags( Mesh &maxmesh, INode *pNode, int face )
{
   int mat_index = maxmesh.getFaceMtlIndex( face );
   StdMat *mat = getStdMaterial( pNode, mat_index );
   int flags = 0;

   if ( mat )
   {
       int shading    = mat->GetShading(); 
       float selfIllum = mat->GetSelfIllum(0);
       Texmap * diffuse = mat->GetSubTexmap( ID_DI );
   
      if ( !IsEqual( selfIllum, 1.0f ) && shading == SHADE_CONST )
         flags |= MAT_FLAT_SHADED;

      // '@' in name indicates color 0 is transparent
       if ( diffuse && diffuse->ClassID() == Class_ID(BMTEX_CLASS_ID,0) )
           if ( strchr( ((BitmapTex*)diffuse)->GetMapName(), '@' ) )
               flags |= MAT_TRANSPARENT;
   }
      

   return flags;
}   

void CelAnimMeshBuilder::TestSmoothing( INode *pNode, Mesh & maxmesh )
{
    fnVertsPerFrame = 2;
    if( !fIsShape )
    {
      Face *maxface = maxmesh.faces;
      faceVerts.setSize( maxmesh.numFaces );
        for( int f = 0; f < maxmesh.numFaces; f++, maxface++ )
        {
         int mat_flags = getMaterialFlags( maxmesh, pNode, f );
         addVerts( maxmesh, maxface, f, mat_flags );
        }
    }
}

//--------------------------------------------------------------

#define SUPPRESS_CLAMPS

static Point2F *CalcTVertex( Point3 &maxtvert, Point2F *tvert )
{
   tvert->x = maxtvert[0];
   tvert->y = 1.0f - maxtvert[1];

#ifndef SUPPRESS_CLAMPS   
    if( tvert->x > 1.0f )
        tvert->x = fmod( tvert->x, 1.0f );
    else if( tvert->x < 0.0f )
        tvert->x = 1.0f + fmod( tvert->x, 1.0f );
    if( tvert->y > 1.0f )
        tvert->y = fmod( tvert->y, 1.0f );
    else if( tvert->y < 0.0f )
        tvert->y = 1.0f + fmod( tvert->y, 1.0f );
#endif        
      
   return tvert;
}   

// only used after default mesh has been added...adds a frame of 
//  texture verts, if they're different from last, but doesn't adjust 
//  face indexing.
Bool CelAnimMeshBuilder::addTexVerts( Mesh &maxmesh )
{
   int changed = FALSE;
   
    if ( maxmesh.getNumTVerts() )
    {
      int i;

      // see if this mesh's texverts are different from last frame's
      for ( i = 0; i < faceList.size(); i++ )
      {
         int id = fTexFrameStart + i * 3;
         unsigned long *tv = maxmesh.tvFace[ i ].t;
         Point2F tvert1, tvert2, tvert3;
         
         CalcTVertex( maxmesh.tVerts[ tv[0] ], &tvert1 );
         CalcTVertex( maxmesh.tVerts[ tv[1] ], &tvert2 );
         CalcTVertex( maxmesh.tVerts[ tv[2] ], &tvert3 );
         
           if ( (textureVertList[ id + 0 ] != tvert1) ||
                (textureVertList[ id + 1 ] != tvert2) ||
                (textureVertList[ id + 2 ] != tvert3) )
         {
            changed = TRUE;
            break;
           }
      }
   
      if ( changed )
      {
          fTexFrameStart = textureVertList.size();
         
         for ( i = 0; i < faceList.size(); i++ )
         {
            unsigned long *tv = maxmesh.tvFace[ i ].t;
            
              AddTVertex( maxmesh.tVerts[ tv[0] ] );
              AddTVertex( maxmesh.tVerts[ tv[1] ] );
              AddTVertex( maxmesh.tVerts[ tv[2] ] );
         }
      }
    }
   
   return changed;
}   

//--------------------------------------------------------------
static void mulVert( TS::Vertex *v, TMat3F *mat )
{
   if ( mat )
   {
      Point3F pt;
      m_mul( v->fPoint, *mat, &pt );
      v->fPoint = pt;
      m_mul( v->fNormal, *(RMat3F *)mat, &pt );
      v->fNormal = pt;
   }
}

void CelAnimMeshBuilder::getVerts( TS::Vertex *verts, Mesh &maxmesh, 
                                   Face &maxface, TMat3F *mat, Bool flat )
{
   for ( int i = 0; i < 3; i++ )
   {
      if ( flat )
         verts[ i ] = CalcFlatVertex( maxmesh, maxface.v, i );
      else 
         verts[ i ] = CalcVertex( maxmesh, maxface.smGroup, maxface.v[i] );
         
      mulVert( &verts[ i ], mat );
   }
}                  

void CelAnimMeshBuilder::setMinMaxVerts()
{
    // determine min & max vertices for this frame
    Vector<TS::Vertex>::iterator it = vertList.end() - 1;
    Point3F minVert = (*it).fPoint;
    Point3F maxVert = (*it).fPoint;
    it--;
    for( int i = 0; i < fnVertsPerFrame - 3; i++, it-- )
    {
        minVert.setMin( (*it).fPoint );
        maxVert.setMax( (*it).fPoint );
    }

    (*it--).fPoint = maxVert;
    (*it--).fPoint = minVert;
}


#define   xLucentMaterial(m)   \
      ((fpSB->materials[m].fParams.fFlags & TS::Material::TextureTranslucent) != 0)


// See ShapeBuilder: wants to sort meshes based off of translucency.  
int CelAnimMeshBuilder::sortPassKey()
{
#if LAST_98_ADDITION
   return (fCountXLucent != 0) + (fCountXLucent == faceList.size());
#else
   return 0;
#endif
}


void CelAnimMeshBuilder::addFace
( 
   Mesh &mesh, TS::CelAnimMesh::Face &face, 
   INode *pNode, int face_id 
)
{
   if( mesh.getNumTVerts() )
   {
       face.fVIP[0].fTextureIndex = AddTVertex( mesh.tVerts[mesh.tvFace[face_id].t[0]] );
       face.fVIP[2].fTextureIndex = AddTVertex( mesh.tVerts[mesh.tvFace[face_id].t[1]] );
       face.fVIP[1].fTextureIndex = AddTVertex( mesh.tVerts[mesh.tvFace[face_id].t[2]] );
   }
   else
   {
      face.fVIP[0].fTextureIndex = 0;
      face.fVIP[2].fTextureIndex = 0;
      face.fVIP[1].fTextureIndex = 0;
   }

   fnTexVertsPerFrame = textureVertList.size();
   

   // AddMaterial() returns index of the material in the ShapeBuilder array of 
   // materials, although it returns zero (which is actually a valid index) for 
   // what looks like a failed add...   Hence we check material.size() before the 
   //    XLucent search loop below.. 
   face.fMaterial = fpSB->AddMaterial( pNode, mesh.getFaceMtlIndex( face_id ) );

   Vector<TS::CelAnimMesh::Face>::iterator it = faceList.begin();
   
   // We want to put the translucent materials at the end.  If our face material
   //    is translucent, then adjust our search to the start of whatever translucent
   //    materials already exist in the list (or the end if this is our first one). 
#if LAST_98_ADDITION
   bool  faceIsXLucent = false;
   if( fpSB->materials.size() )
   {
      if( xLucentMaterial( face.fMaterial ) )
      {
         // set this for the mesh - the meshes are sorted for this.  
         fCountXLucent++;
         faceIsXLucent = true;
         while ( it != faceList.end() )
         {
            AssertFatal( it->fMaterial < fpSB->materials.size(), "weird mat index" );
            if( xLucentMaterial( it->fMaterial ) )
               break;
            it++;
         }
      }
   }
    
   // Put it next to a face with same material if possible.  
   //
   // If we're not inserting a face with a xlucent color, then we want to stop the 
   // search if any xlucent colors are there at the end of the list.  But if the 
   // color we're inserting is translucent, then we do the normal loop to find where
   // in the translucents this one belongs.  
   //
   if( ! faceIsXLucent )
   {
      while ( it != faceList.end() &&  ! xLucentMaterial( it->fMaterial ) )
      {
         if ( it->fMaterial == face.fMaterial )
            break;
         it++;
      }
   }
   else //IS transLucent.  
#endif
   {
      while ( it != faceList.end() )
      {
         if ( it->fMaterial == face.fMaterial )
            break;
         it++;
      }
   }
      
   faceList.insert( it, face );
}

void CelAnimMeshBuilder::setShapeVerts( Mesh &maxmesh, Face &maxface, int face_id )
{
    Point3F v0, v1, v2;
    v0 = ShapeVertex( maxmesh, maxface.v[0] );
    v1 = ShapeVertex( maxmesh, maxface.v[1] );
    v2 = ShapeVertex( maxmesh, maxface.v[2] );
    if( !face_id )
    {
        vertList[0].fPoint = v0;
        vertList[1].fPoint = v0;
    }
    vertList[fFrameStart + 0].fPoint.setMin( v0 ); 
    vertList[fFrameStart + 1].fPoint.setMax( v0 );
    vertList[fFrameStart + 0].fPoint.setMin( v1 ); 
    vertList[fFrameStart + 1].fPoint.setMax( v1 );
    vertList[fFrameStart + 0].fPoint.setMin( v2 ); 
    vertList[fFrameStart + 1].fPoint.setMax( v2 );
}


Bool CelAnimMeshBuilder::addCelFrame( INode *pNode, Mesh &maxmesh, 
                                      TMat3F *mat, Bool addFaces )
{
   int matched = TRUE;
   int i;
    TS::Vertex verts[3];
   
   if ( fIsShape || addFaces )
      matched = FALSE;     // skip compare
   
   // see if this frame same as last.
   for ( i = 0; matched && i < maxmesh.numFaces; i++ )
   {
      Face &maxface = maxmesh.faces[ i ];

      Bool flat = getMaterialFlags( maxmesh, pNode, i ) & MAT_FLAT_SHADED;
      getVerts( verts, maxmesh, maxface, mat, flat );

      for ( int j = 0; j < 3; j++ )
      {
         FaceVert &fv = faceVerts[ i ];
         int v_id = (j + fv.vert_offset) % 3;
               
         if ( !cmpVert( vertList[ fFrameStart + fv.v[j] ], verts[ v_id ], 
                       fv.uses_normal[j] ) )
            break;
      }
      
      matched = j == 3;
   }

   if ( !matched )
   {
       // set starting index for this frame:
       fFrameStart = vertList.size();
       // make space for this frames vertices:
       vertList.setSize( vertList.size() + fnVertsPerFrame );

        // make sure default frame gets a frame too...
        if ( addFaces )
        {
           TS::CelAnimMesh::Frame frame;
           frame.fFirstVert = fFrameStart;
           frameList.push_back( frame );
        }
        
      for ( i = 0; i < maxmesh.numFaces; i++ )
      {
         Face &maxface = maxmesh.faces[ i ];
          TS::CelAnimMesh::Face face;
      
          if ( fIsShape )
            setShapeVerts( maxmesh, maxface, i );
         else
         {
            Bool flat = getMaterialFlags( maxmesh, pNode, i ) & MAT_FLAT_SHADED;
            getVerts( verts, maxmesh, maxface, mat, flat );
            FaceVert &fv = faceVerts[ i ];
            
            for ( int j = 0; j < 3; j++ )
            {
               int v_id = (j + fv.vert_offset) % 3;
               
               copyVert( &vertList[ fFrameStart + fv.v[j] ], verts[ v_id ], 
                         fv.uses_normal[j] );
            }
            
            // reverse direction of poly to match GFX convention
            face.fVIP[0].fVertexIndex = fv.v[0];
            face.fVIP[1].fVertexIndex = fv.v[2];
            face.fVIP[2].fVertexIndex = fv.v[1];
         
             if( addFaces )
               addFace( maxmesh, face, pNode, i );
         }
      }

       if ( !fIsShape )
         setMinMaxVerts();
   }

   return !matched;
}                                         

//--------------------------------------------------------------

Point3F const & CelAnimMeshBuilder::ShapeVertex( Mesh & maxmesh, int vi )
{
    static Point3F vert;
   vert.x = maxmesh.verts[vi][0];
   vert.y = maxmesh.verts[vi][1];
   vert.z = maxmesh.verts[vi][2];

    return vert;
}

//--------------------------------------------------------------

TS::Vertex const & CelAnimMeshBuilder::CalcVertex( Mesh & maxmesh, unsigned smGroup, int vi )
{
    // calculate vertex normal, taking into account smoothing groups
    // (Note: this doesn't really work as written, unless all faces share the
    // same smoothing group)

    static TS::Vertex vert;
   vert.fPoint.x = maxmesh.verts[vi][0];
   vert.fPoint.y = maxmesh.verts[vi][1];
   vert.fPoint.z = maxmesh.verts[vi][2];

   RVertex &rv = maxmesh.getRVert(vi);
   Point3 normal;
   if( (rv.rFlags & NORCT_MASK) == 1 )
   {
      //??? assert( !smGroup || (smGroup == rv.rn.getSmGroup()) );
      normal = rv.rn.getNormal();
   }
   else
   {
      for( unsigned n = 0; n < (rv.rFlags & NORCT_MASK); n++ )
         if( rv.ern[n].getSmGroup() == smGroup )
         {
            normal = rv.ern[n].getNormal();
            break;
         }
      if( n >= (rv.rFlags & NORCT_MASK) )
      {
            if( smGroup )
            {
                char s[100];
                sprintf( s, "vertex %d: smooth group: %d", vi, smGroup );
            AssertMessage( 0, s );
//              MessageBox(GetActiveWindow(), s, "", MB_OK);
            }
         normal = rv.ern[0].getNormal();
      }
   }

   vert.fNormal.x = normal.x;
   vert.fNormal.y = normal.y;
   vert.fNormal.z = normal.z;
   return vert;
}

//--------------------------------------------------------------

int CelAnimMeshBuilder::AddTVertex( Point3 &maxtvert )
{
    // Add a texture vertex, with duplicates eliminated:
   Point2F tvert;
   CalcTVertex( maxtvert, &tvert );

#ifndef SUPPRESS_CLAMPS   
   if ( tvert.x < 0.0f  ||  tvert.x > 1.0f )
       tvert.x = 0;
   if ( tvert.y < 0.0f  ||  tvert.y > 1.0f )
       tvert.y = 0;
#endif        

    textureVertList.push_back( tvert );
    return textureVertList.size() - 1;
}

//--------------------------------------------------------------


void CelAnimMeshBuilder::SetupObject( TS::Shape::Object *obj )
{
    obj->fName =             fName;
    obj->fFlags =                  fFlags;
    obj->fNodeIndex =        fNodeIndex; 
    obj->fnSubSequences =    fnSubSequences;
    obj->fFirstSubSequence = fFirstSubSequence;
    obj->fObjectOffset =     fObjectOffset.p;
}

int CelAnimMeshBuilder::IsShape()
{
    return( fIsShape || fnVertsPerFrame != 2 );
}


//--------------------------------------------------------------


// Good place to dump this routine.  Tired of looking at the ifdefs everywhere (which can still
// be used in time critical places if desired).  
void sDump(FILE * fp, const char * string, ...)
{
#ifdef SDUMP
   char buffer[1024];
   va_list args;
   va_start(args, string);
   vsprintf(buffer, string, args);
   fprintf( fp, buffer );
#else
   fp; string;
#endif
}   


//--------------------------------------------------------------
