#include "SceneEnum.H"
#include "ResManager.h"

#include <ISTDPLUG.H>
#include <ANIMTBL.H>
#include <DUMMY.H>
#include <DECOMP.H>
#include <MODSTACK.H>

#define SEQUENCE_CLASS_ID    0x09923023
#define TRANSITION_CLASS_ID1 0x22065872
#define TRANSITION_CLASS_ID2 0x5cb77942

#define PB_SEQ_ONESHOT       3
#define PB_SEQ_FORCEVIS      4
#define PB_SEQ_VISONLY       5
#define PB_SEQ_NOCOLLAPSE    6
#define PB_SEQ_USECELRATE    7

#define PB_REVERSE_TRANS     0
#define PB_DURATION          1
#define PB_SHIFTX            2
#define PB_SHIFTY            3
#define PB_SHIFTZ            4

// Special code for VICON stuff (character studio)...
#define VICON_MUL            TRUE
#define COLLAPSE_SCALE       TRUE

// Special for biped stuff -- class id in no header, so we just def it here
// NOTE/WARNING:  This could change in cstudio updates
#define BipedObjectClassID Class_ID(37157,0)

#define DEFAULT_TIME         0

// This define will cause the exporter to make dummyalways nodes relative to the 
//    bounds node position.  One of the last changes made for Tribes was to 
//    make them relative to the origin, by checking in the getNodeTransform 
//    method.  To keep the old way, define DUMMYALWAYS_RELATIVE_TO_BOUNDS.

// This is defined in the configuration section.  There are a couple of 
//    exporter configurations happening now.. 
//       See Project->Settings->SettingsFor.  
// define VICON_EXPORT_BEHAVIOR   1
static char * debug_node_name;

void SceneEnumProc::getSequence(INode *pNode)    
{
    // Sequences must have "root" as parent:
#ifdef MAX_2
   if (!pNode->GetParentNode()->IsRootNode())
#else
   if(stricmp(pNode->GetParentNode()->GetName(), "root"))
#endif
   {
      return;
   }

   Object *ob = pNode->GetObjectRef();

   assert(ob->ClassID() == Class_ID(SEQUENCE_CLASS_ID, 0));

   // Get all keys in sequence's cel frame rate track and use them to
   // build list of all cel keyframe times:

#ifdef MAX_2
   Interval range = pNode->GetTimeRange(TIMERANGE_ALL        | 
                                         TIMERANGE_CHILDNODES | 
                               TIMERANGE_CHILDANIMS);
#else
   Interval range = pNode->GetTimeRange(TIMERANGE_ALL);
#endif

   ShapeBuilder::Sequence *pseq = 
       sb.AddSequence(pNode->GetName(), range.Start(), range.End());

   IParamBlock *pblock = (IParamBlock *)ob->GetReference(0);
   Control *control = pblock->GetController(0);
   
   if (!control)
   {
       AssertMessage(0, "Sequence is missing keyframes on top track to mark start and end");
       return;
   }

   IKeyControl *pkey = GetKeyControlInterface( control );
   IBezFloatKey key;
   int lastTime = range.Start();
   int lastRate, k;
   int oneshot, forcevis, visonly, no_collapse, use_celrate;
    
   pblock->GetValue( PB_SEQ_ONESHOT, range.Start(), oneshot, FOREVER );
   pblock->GetValue( PB_SEQ_FORCEVIS, range.Start(), forcevis, FOREVER );
   pblock->GetValue( PB_SEQ_VISONLY, range.Start(), visonly, FOREVER );
   pblock->GetValue( PB_SEQ_NOCOLLAPSE, range.Start(), no_collapse, FOREVER );
   pblock->GetValue( PB_SEQ_USECELRATE, range.Start(), use_celrate, FOREVER );
   pseq->SetCyclic( !oneshot );
   pseq->SetForceVis( forcevis );
   pseq->SetVisOnly( visonly );
   pseq->SetUseCelRate( use_celrate );
   
   if ( no_collapse )
       no_transform_collapse = TRUE;

   for( k = 0; k < pkey->GetNumKeys(); k++ )
   {
      pkey->GetKey( k, &key );

      if( k > 0 && lastRate != 0 )
         while( lastTime + lastRate < key.time )
         {
             lastTime += lastRate;
             pseq->AddCelKeyTime( lastTime );
         }
         
      lastTime = key.time;
      lastRate = 4800 / key.val;
      pseq->AddCelKeyTime( lastTime );
   }
    
   for ( int j = 1; j < 3; j++ )
      if ( pblock->GetController(j) )
      {
         pkey = GetKeyControlInterface( pblock->GetController(j) );
          
         // Check the second track for keyframes
         for( k = 0; k < pkey->GetNumKeys(); k++ )
         {
            pkey->GetKey( k, &key );
            float pos = (float)(key.time - range.Start()) / (range.End() - range.Start());
   
            int ft_id = sb.AddFrameTrigger( pos, key.val, j == 1 );
            pseq->AddFrameTrigger( ft_id );
         }
      }

   sDump( fp, "Sequence %s\n", pNode->GetName() );    
   sDump( fp, "  Anim time range: %d to %d\n", range.Start(), range.End() );
   sDump( fp, "  Number of cel keys: %d\n", pseq->GetCelNumTimes() );
}


void    SceneEnumProc::getTransition( INode *pNode )    
{
    // transitions must have "root" as parent:
#ifdef MAX_2
    if (!pNode->GetParentNode()->IsRootNode())
#else
    if( stricmp(pNode->GetParentNode()->GetName(), "root") )
#endif
        return;

    Object * ob = pNode->GetObjectRef();

#ifdef MAX_2
    Interval range = pNode->GetTimeRange(TIMERANGE_ALL        | 
                                         TIMERANGE_CHILDNODES | 
                                         TIMERANGE_CHILDANIMS);
#else
    Interval range = pNode->GetTimeRange(TIMERANGE_ALL);
#endif
   IParamBlock *pblock = (IParamBlock*)ob->GetReference(0);
    Control *control = pblock->GetController( 0 );
    
    if ( !control )
    {
        AssertMessage( 0, "Transition is missing keyframes to mark start and end" );
        return;
    }
    
    IKeyControl *pkey = GetKeyControlInterface( control );
   
   int reverse;
   float duration;
   Point3F shift;
   
   pblock->GetValue( PB_REVERSE_TRANS, range.Start(), reverse, FOREVER );
   pblock->GetValue( PB_DURATION, range.Start(), duration, FOREVER );
   pblock->GetValue( PB_SHIFTX, range.Start(), shift.x, FOREVER );
   pblock->GetValue( PB_SHIFTY, range.Start(), shift.y, FOREVER );
   pblock->GetValue( PB_SHIFTZ, range.Start(), shift.z, FOREVER );

   float start = range.Start();
   float end = range.End();

   if ( reverse )
   {
       float tmp = start;
       start = end;
       end = tmp;
   }

    ShapeBuilder::Transition *trans = sb.AddTransition( pNode->GetName(), 
                                                start, end, duration, &shift );
}

//--------------------------------------------------------------
static void convertToTransform( const Matrix3 &TM, MyTransform *xform )
{
    AffineParts parts;
    decomp_affine( TM, &parts );

    xform->fTranslate.x = parts.t[0];
    xform->fTranslate.y = parts.t[1];
    xform->fTranslate.z = parts.t[2];
    xform->fRotate.x = parts.q[0];
    xform->fRotate.y = parts.q[1];
    xform->fRotate.z = parts.q[2];
    xform->fRotate.w = parts.q[3];
    xform->fRotate.normalize();
    xform->fScale.x = parts.k[0];
    xform->fScale.y = parts.k[1];
    xform->fScale.z = parts.k[2];
    xform->fMirrored = parts.f < 0;
}   

static bool hasMesh( INode *pNode )
{ 
    const ObjectState &os = pNode->EvalWorldState(0);
    return( os.obj->CanConvertToType(triObjectClassID) && !(os.obj->ClassID() == BipedObjectClassID) );
}

void SceneEnumProc::getNodeTransform( INode *pNode, INode *pRootTo, int time, 
                                      MyTransform *xform, Matrix3 *preScale )
{
    // get the transform of a node relative to it's (new) parent:
   Matrix3 nodeTM = pNode->GetNodeTM( time );
   Matrix3 rootTM = pRootTo->GetNodeTM( time );
#ifndef DUMMYALWAYS_RELATIVE_TO_BOUNDS    
   if( isBounds( pRootTo ) )
   {
      if( isDummyAlways( pNode ) )
      {
         rootTM.NoTrans();
         rootTM.IdentityMatrix();
      }
   }
#endif
    
   Matrix3 localTM = nodeTM*Inverse(rootTM);
 
#if VICON_MUL 
   // This actually tests for the detail nodes that hang off the VICON nodes and which don't actually
   //    have a transform, and so this is where they get set to identity.  the hasMesh() function
   //    is a little misnamed since it just refers to these nodes having one bogus mesh and which
   //       are mainly there to tell us what detail trees need to be created.  
   if ( isVICON(pNode->GetParentNode()) )
      if ( hasMesh( pNode ) )
      {
         localTM.IdentityMatrix();
         debug_node_name = pNode->GetName();
      }
#endif

   if ( preScale )
      localTM = localTM * *preScale;   
        
   convertToTransform( localTM, xform );   
   
#ifdef SDUMP
   fprintf( fp, "  translate: %8.8f %8.8f %8.8f\n", 
      xform->fTranslate.x, xform->fTranslate.y, xform->fTranslate.z );
   fprintf( fp, "  rotate: %8.8f %8.8f %8.8f %8.8f\n", 
      xform->fRotate.x, xform->fRotate.y, xform->fRotate.z, xform->fRotate.w );
   fprintf( fp, "  scale: %8.8f %8.8f %8.8f\n", 
      xform->fScale.x, xform->fScale.y, xform->fScale.z );
#endif
}

//--------------------------------------------------------------

void    SceneEnumProc::setNodeDefaultTransform( INode *pNode, INode *pRootTo,
    SBNode * pSBNode, Matrix3 *preScale )    
{
    // default transform is transform at time 0:
    sDump( fp, "default local TM:\n" );
    MyTransform xform;
    
    getNodeTransform( pNode, pRootTo, DEFAULT_TIME, &xform, preScale );
    Point3F &sc = xform.fScale;
    
    pSBNode->scale = sc;
    pSBNode->mirrored = xform.fMirrored;
    
    if ( !IsEqual( sc.x, 1.0f ) || !IsEqual( sc.y, 1.0f ) || 
          !IsEqual( sc.z, 1.0f ) )
    {
        sDump( fp, "non-identity scale\n" );
    }
    
#if COLLAPSE_SCALE    
    sc.x = sc.y = sc.z = 1.0f;
#endif    
    pSBNode->SetDefaultTransform( sb, xform );
}

//--------------------------------------------------------------

void    SceneEnumProc::makeNodeKeyframe( int time, int startTime, int endTime, 
    INode *pNode, INode *pRootTo, MyTransform * xform, float *pos, 
    Matrix3 *preScale )    
{
    // get the transform at time, and also calculate position parameter:
    *pos = (float)(time - startTime)/(endTime - startTime);
    sDump( fp, "local TM @ %8d == %8.3f %%:\n", time, *pos * 100.0f );
    getNodeTransform( pNode, pRootTo, time, xform, preScale );
    Point3F &sc = xform->fScale;
#if COLLAPSE_SCALE    
    sc.x = sc.y = sc.z = 1.0f;
#endif    
}


//--------------------------------------------------------------

void SceneEnumProc::getControllerTimes( Control * pControl, 
    TS::IndexList * pTimes )
{
    // get times of all keys for the controller and any subAnim controllers,
    // and add times to list pTimes:
#ifdef OLDSDUMP
    TSTR s;
    pControl->GetClassName(s);
    fprintf( fp, "Controller Class Name: %s\n", (char*)s );
#endif

    if( pControl->IsLeaf() )
    {
        IKeyControl *ikeys = GetKeyControlInterface(pControl);
        for( int ik = 0; ikeys && ik < ikeys->GetNumKeys(); ik++ )
        {
            IBezScaleKey k;
            ikeys->GetKey( ik, &k );
            pTimes->add( k.time );
        }
    }
    else
    {
        for( int sa = 0; sa < pControl->NumSubs(); sa++ )
            getControllerTimes( (Control*)pControl->SubAnim( sa ), pTimes );
    }
}

//--------------------------------------------------------------

void SceneEnumProc::getTransformTimes( INode *pNode, TS::IndexList * pTimes )
{
    // get times of all keys for the transform controller, if any, and all 
    // subAnim controllers, and add times to list pTimes.
    // If partial inheritance of transforms is used, also get times for parent's
    // transform controller:
    Control *ptmc = pNode->GetTMController();

    if( ptmc )
    {
        getControllerTimes( ptmc, pTimes );
        INode *parent = pNode->GetParentNode();

        if ( pNode->GetTMController()->GetInheritanceFlags() || hasMesh( pNode ) )
        {
            if( parent )
            {
#ifdef OLDSDUMP
                fprintf( fp, "Incomplete inheritance from %s to %s: %d\n",
                    parent->GetName(), pNode->GetName(), pNode->GetTMController()->GetInheritanceFlags() );
#endif
                getTransformTimes( parent, pTimes );
            }
        }
    }

    Control *pvc = pNode->GetVisController();

    if( pvc )
        getControllerTimes( pvc, pTimes );
}

//--------------------------------------------------------------

bool SceneEnumProc::getNodeTransformKeys( INode *pNode, INode * pRootTo, 
    SBNode * pSBNode, int priority, Matrix3 *preScale )
{
    bool animated = FALSE;

    // get times of transform keyframes:
    TS::IndexList xformKeys, &xformTimes = xformKeys;
    getTransformTimes( pNode, &xformKeys );
    xformKeys.sort();

    Interval range;
    range.SetInfinite();
    Vector<int> * pMorphTimes = sb.GetCelTimes( range.Start(), range.End() );
    TS::IndexList celKeys;

    for ( int i = 0; i < pMorphTimes->size(); i++ )
        celKeys.add( (*pMorphTimes)[i] );
        
    delete pMorphTimes;        

    // get the default transform for the node:
    const char *pname = pNode->GetName();
    setNodeDefaultTransform( pNode, pRootTo, pSBNode, preScale );
#ifdef SDUMP
    fprintf( fp, "default xform index = %d\n", pSBNode->snode.fDefaultTransform );
#endif

    for( VectorPtr<ShapeBuilder::Sequence*>::const_iterator si = 
        sb.sequences.begin(); si != sb.sequences.end(); si++ )
    {
        int seq = si - sb.sequences.begin();
        int count = 2;    // always have start,end keys
        int first = sb.keyframes.size();
        Bool needEndKey = FALSE;
        MyTransform startXform, endXform;
        float startPos, endPos;
        int startXformIndex, endXformIndex;

        if ( (*si)->fUseCelRate )
            xformTimes = celKeys;
        else
            xformTimes = xformKeys;

        // get transform for start time:
        makeNodeKeyframe( (*si)->fStartTime, (*si)->fStartTime, (*si)->fEndTime, 
            pNode, pRootTo, &startXform, &startPos, preScale );
        startXformIndex = sb.AddTransform( startXform );
#ifdef SDUMP
        fprintf( fp, "   xform index = %d\n", startXformIndex );
#endif

        // get transform for end time:
        makeNodeKeyframe( (*si)->fEndTime, (*si)->fStartTime, (*si)->fEndTime, 
            pNode, pRootTo, &endXform, &endPos, preScale );
        endXformIndex = sb.AddTransform( endXform );
#ifdef SDUMP
        fprintf( fp, "   xform index = %d\n", endXformIndex );
#endif


        // if start or end transform different from default,
        // we need to add keys for this sequence, starting with the start: 
        if( startXformIndex != pSBNode->snode.fDefaultTransform ||
             endXformIndex != pSBNode->snode.fDefaultTransform )
        {
#ifdef SDUMP
            fprintf( fp, "Adding start key %d (def = %d, end = %d )\n", startXformIndex,
                pSBNode->snode.fDefaultTransform, endXformIndex );
#endif
            // add the key for the start of the sequence:
            pSBNode->AddKey( sb, startPos, startXformIndex, 
                pNode->GetVisibility( (*si)->fStartTime ) > 0.0f );
            
            // the priority of the sequence is determined by the highest priority
            // node it animates:
         if ( startXformIndex != endXformIndex )
               if( priority < (*si)->fPriority )
                   (*si)->fPriority = priority;

            // if we add a start key, we will also need an end key:
            needEndKey = TRUE;
        }

        // check the intermediate transforms:
      TS::IndexList::iterator ti;
        int last_time = -1;
      
        for( ti = xformTimes.begin(); ti != xformTimes.end(); ti++ )
            if( (*ti >= (*si)->fStartTime) && (*ti <= (*si)->fEndTime) &&
                 *ti != last_time )
            {
                last_time = *ti;
                 // the time is within the range of the sequence:
                // get the intermediate transform and position:
                MyTransform xform;
                float pos;
                makeNodeKeyframe( (*ti), (*si)->fStartTime, (*si)->fEndTime, 
                    pNode, pRootTo, &xform, &pos, preScale );
            int xformIndex = sb.AddTransform( xform );
#ifdef SDUMP
                fprintf( fp, "   xform index = %d\n", xformIndex );
#endif

            if ( needEndKey  ||  xformIndex != pSBNode->snode.fDefaultTransform )
            {
                   if( !needEndKey )
                      {
                       // we haven't yet added a key for the start of the sequencem,
                       // so do it now:
                       pSBNode->AddKey( sb, startPos, startXformIndex,
                               pNode->GetVisibility( (*si)->fStartTime ) > 0.0f );

                       // adjust the sequences priority if indicated:
                  if ( xformIndex != startXformIndex )
                          if( priority < (*si)->fPriority )
                              (*si)->fPriority = priority;

                       // we will now need an end key:
                       needEndKey = TRUE;
                      }

                   // add the transform and adjust the count:
                   pSBNode->AddKey( sb, pos, xformIndex, pNode->GetVisibility( *ti ) > 0.0f );
                   count++;
            }
            }

        if( needEndKey )
        {
            // we added keys for this sequence, so we need an end key:
            pSBNode->AddKey( sb, endPos, endXformIndex,
                    pNode->GetVisibility( (*si)->fEndTime ) > 0.0f );
            // we also need to designate the range of keys as belonging
            // to this sequence:
            pSBNode->AddSubSequence( sb, seq, first, count );
            animated = TRUE;

#ifdef SDUMP
            fprintf( fp, "SubSequence: seq = %d, firstkey = %d, nkeys = %d\n", seq, first, count );
#endif
        }
#ifdef SDUMP
      else
            fprintf( fp, "SubSequence: seq = %d: no keys added\n", seq );
#endif
    }
    
    return animated;
}

static TMat3F objOffsetMat;
static TMat3F collapseMat;
static TMat3F scaleMat;

// Calculate a matrix to run all the points in the mesh through...
//  This is used to pre-transform points when we want to collapse nodes,
//  get rid of the scale in a transform, etc.
static TMat3F *calcMulMat( INode *pNode, INode *rootTo, float time, FILE *fp )
{
    static TMat3F mat;
    TMat3F temp;
    
#if VICON_MUL 

#if VICON_EXPORT_BEHAVIOR==0
   INode *parent = pNode->GetParentNode();
   if ( SceneEnumProc::isVICON( parent ) )
#else
   // if we ARE the VICON node, we need to root to our parent (which should
   //    in fact be the bounds node).  (although we get linked to a detail node, but
   //    it has an identity xform, so we must calculate off of bounds here).  
   if ( SceneEnumProc::isVICON( pNode )  )
#endif   
   {
      // get the transform of a node relative to it's (new) parent:
      Matrix3 nodeTM = pNode->GetNodeTM( time );
      Matrix3 rootTM = rootTo->GetNodeTM( time );
      Matrix3 localTM = nodeTM * Inverse( rootTM );
        
      MyTransform xform;
      convertToTransform( localTM, &xform );
      xform.setMatrix( &collapseMat );
   }

#endif

   m_mul( scaleMat, collapseMat, &temp );
   m_mul( objOffsetMat, temp, &mat );

   return &mat;
}

//--------------------------------------------------------------

static TriObject *getTriObj( INode *pNode, Object *ob, float time, bool *deleteIt )
{
   TriObject * tri = 0;
   if ( ob->CanConvertToType(triObjectClassID)) 
   {
      tri = (TriObject *)ob->ConvertToType( time, triObjectClassID);
      *deleteIt = (tri && (tri != ob));
   }
   else
   {
      // if the object can't convert to a tri-mesh, eval world state to
      // get an object that can:
      const ObjectState &os = pNode->EvalWorldState(time);
      
      if ( os.obj->CanConvertToType(triObjectClassID) ) 
         tri = (TriObject *)os.obj->ConvertToType( time, triObjectClassID );

      *deleteIt = (tri && (tri != os.obj));
   }
   
   return tri;
}

Bool SceneEnumProc::addMorphDefault( INode *pNode, INode *pRootTo, Object *ob, 
    MeshBuilder * pMB )
{
   bool del_tri;
   TriObject *tri = getTriObj( pNode, ob, DEFAULT_TIME, &del_tri );
   
   if( tri )
   {
      Mesh &mesh = tri->mesh;
      mesh.buildNormals();
      TMat3F *mp = calcMulMat( pNode, pRootTo, DEFAULT_TIME, fp );
        
      pMB->TestSmoothing( pNode, mesh );
      pMB->addCelFrame( pNode, mesh, mp, TRUE );
        
#ifndef TEST
      if ( del_tri )
         tri->DeleteMe();
#endif
            
      return pMB->IsShape();
   }

   return FALSE;
}

//--------------------------------------------------------------
Bool SceneEnumProc::addMorphKeyframe( int time, int startTime, int endTime, 
         INode *pNode, Object *ob, MeshBuilder * pMB, INode *rootTo, int *last_vis,
         int changes )    
{
   float pos = (float)(time - startTime)/(endTime - startTime);
   int added = FALSE;

   bool del_tri;
   TriObject *tri = getTriObj( pNode, ob, time, &del_tri );

   if ( tri )
   {
      Mesh &mesh = tri->mesh;
      mesh.buildNormals();
      TMat3F *mp = calcMulMat( pNode, rootTo, time, fp );
      Bool first_frame = time == startTime;

      int vis = pNode->GetVisibility( time ) > 0.0f;
      Bool vis_matters = (changes & VIS_CHANGES) && 
                                 (first_frame || *last_vis != vis);
      Bool cel_added = FALSE;
      Bool mat_added = FALSE;
        
      if ( changes & MESH_CHANGES )
         cel_added = pMB->addCelFrame( pNode, mesh, mp, FALSE );
          
      if ( changes & MAT_CHANGES )
         mat_added = pMB->addTexVerts( mesh );

      if ( first_frame || cel_added || mat_added || vis_matters )
      {
         pMB->AddKey( fp, sb, time, pos, vis, changes );
         added = TRUE;
      }

#ifndef TEST
      if ( del_tri )
         tri->DeleteMe();
#endif

      *last_vis = vis;
   }
   return( added );
}
   
//--------------------------------------------------------------

struct CmpFace
{
   int vi[3];
   int tvi[3];
};

struct CmpVert
{
   float v[3];
};

class CmpMesh
{
public:
   Vector<CmpFace> faces;
   Vector<CmpVert> verts;
   Vector<CmpVert> tverts;
   
   CmpMesh( Mesh &maxmesh );    
};

CmpMesh::CmpMesh( Mesh &maxmesh )
{
   int i, j, k;
   
   for ( i = 0; i < maxmesh.numFaces; i++ )
   {
      CmpFace face;
      
      for ( j = 0; j < 3; j++ )
      {
         int vi = face.vi[j] = maxmesh.faces[i].v[j];
         
         if ( vi + 1 > verts.size() )
            verts.setSize( vi + 1 );
             
         CmpVert vert;
         
         for ( k = 0; k < 3; k++ )
            vert.v[k] = maxmesh.verts[ vi ][ k ];
             
         verts[ vi ] = vert;

         
         if ( maxmesh.getNumTVerts() )
         {
            int tvi = face.tvi[j] = maxmesh.tvFace[i].t[j];
            
            if ( tvi + 1 > tverts.size() )
               tverts.setSize( tvi + 1 );
                
            CmpVert tvert;
            
            for ( k = 0; k < 2; k++ )
            {
               float val = maxmesh.tVerts[ tvi ][ k ];
               
               if ( val > 1.0f )
                  val = fmod( val, 1.0f );
               else if ( val < 0.0f )
                  val = 1.0f + fmod( val, 1.0f );
                   
               tvert.v[k] = val;
            }
                
            tverts[ tvi ] = tvert;
         }
      }
          
      faces.push_back( face );            
   }
}

static int cmp_meshes( CmpMesh *m1, CmpMesh *m2 )
{
   int changes = 0;

   // hmmm...maybe should assert!
   if ( m1->faces.size() != m2->faces.size() )
      return MAT_CHANGES | MESH_CHANGES;

   bool has_tverts = m1->tverts.size() != 0;
   bool done = FALSE;

   for ( int k = 0; k < m1->faces.size() && !done; k++ )
   {
      CmpFace &f1 = m1->faces[ k ];
      CmpFace &f2 = m2->faces[ k ];
        
      for ( int i = 0; i < 3 && !done; i++ )
      {
         if ( !(changes & MESH_CHANGES) )
         {                    
            if ( f1.vi[i] != f2.vi[i] )
               changes |= MESH_CHANGES;

            CmpVert &v1 = m1->verts[ f1.vi[i] ];
            CmpVert &v2 = m2->verts[ f1.vi[i] ];
            
            for ( int j = 0; j < 3; j++ )
               if ( !IsEqual( v1.v[j], v2.v[j] ) )
               {
                  changes |= MESH_CHANGES;
                  break;
               }
         }

         if ( has_tverts )
            if ( !(changes & MAT_CHANGES) )
            {                    
               if ( f1.tvi[i] != f2.tvi[i] )
                  changes |= MAT_CHANGES;

               CmpVert &tv1 = m1->tverts[ f1.tvi[i] ];
               CmpVert &tv2 = m2->tverts[ f1.tvi[i] ];
               
               for ( int j = 0; j < 2; j++ )
                  if ( !IsEqual( tv1.v[j], tv2.v[j] ) )
                  {
                     changes |= MAT_CHANGES;
                     break;
                  }
            }
         
         if ( (!has_tverts || (changes & MAT_CHANGES)) )
             if ( changes & MESH_CHANGES )
                 done = TRUE;
      }
   }
   
   return changes;
}

static int scanForChanges( ShapeBuilder::Sequence *seq, 
                                      Vector<int> *pMorphTimes, INode *pNode, Object *ob )
{
   int vis, last_vis = -1000;
   Vector<int>::const_iterator ti;
   int changes = 0;
    
   // look for visibility changes in the sequence's keyframes.
   if ( seq->fForcevis )
       changes |= VIS_CHANGES;
   else                    
      for ( ti = pMorphTimes->begin(); ti != pMorphTimes->end(); ti++ )
         if ( (*ti >= seq->fStartTime) && (*ti <= seq->fEndTime) )
         {
            vis = pNode->GetVisibility( (*ti) ) > 0.0f;
                
            if ( last_vis <= -1000 )
               last_vis = vis;
            else if ( last_vis != vis )
            {
               changes |= VIS_CHANGES;
               break;
            }
         }

#ifdef TEST
    changes |= (MESH_CHANGES | MAT_CHANGES);
#else    
    CmpMesh *first_mesh = 0;

    // Scan through all the frames of this seq, checking for mesh 
    //  changes.  
    for ( ti = pMorphTimes->begin(); ti != pMorphTimes->end(); ti++ )
        if ( (*ti >= seq->fStartTime) && (*ti <= seq->fEndTime) )
        {
            float time = *ti;
            bool del_tri;
            TriObject *tri = getTriObj( pNode, ob, time, &del_tri );

            if ( tri )
           {
                if ( !first_mesh )
                    first_mesh = new CmpMesh( tri->mesh );
                else
                {
                    CmpMesh *cur_mesh = new CmpMesh( tri->mesh );
                    changes |= cmp_meshes( first_mesh, cur_mesh );
#ifndef TEST
                    delete cur_mesh;
#endif
                    
                    if ( (changes & MESH_CHANGES) && (changes && MAT_CHANGES) )
                        break;
                }
                
#ifndef TEST
                if ( del_tri )
                    tri->DeleteMe();
#endif
            }                        
        }

#ifndef TEST
    if ( first_mesh )
        delete first_mesh;                
#endif
#endif
        
    return changes;    
}




void    SceneEnumProc::getNodeMorphKeys( INode *pNode, Object *ob, 
    Interval &range, int nodeId, Bool IsShape, INode *rootTo,
    int nameId )    
{
    // we don't want dummys here:
    if ( !ob || ob->ClassID() == dummyClassID )
        return;
        
    Bool hasWSM = ob->SuperClassID() == GEN_DERIVOB_CLASS_ID;
    if( hasWSM  ||  !ob->IsAnimated() )
        range.SetInfinite();

    const char *pname = pNode->GetName();
    
    Matrix3 before = pNode->GetNodeTM( DEFAULT_TIME );
    Matrix3 after = pNode->GetObjTMAfterWSM( DEFAULT_TIME );
    Matrix3 wsm = after * Inverse( before );
    MyTransform xform;
    convertToTransform( wsm, &xform );

#ifdef SDUMP        
    fprintf( fp, "Node %s with parent %s\n", pname, pNode->GetParentNode()->GetName() );    
    fprintf( fp, "Anim time range: %d to %d\n", range.Start(), range.End() );
    fprintf( fp, "Object class ID: a: %x, b: %x\n", ob->ClassID().PartA(), ob->ClassID().PartB() );
    fprintf( fp, "Object super class ID: %x\n", ob->SuperClassID() );
    fprintf( fp, "wsm (Object Offset) translate: %8.8f %8.8f %8.8f\n", 
        xform.fTranslate.x, xform.fTranslate.y, xform.fTranslate.z );
    fprintf( fp, "wsm (Object Offset) rotate: %8.8f %8.8f %8.8f %8.8f\n", 
        xform.fRotate.x, xform.fRotate.y, xform.fRotate.z, xform.fRotate.w );
    fprintf( fp, "wsm (Object Offset) scale: %8.8f %8.8f %8.8f\n", 
        xform.fScale.x, xform.fScale.y, xform.fScale.z );
    if( hasWSM )
        fprintf( fp, "Node has WSM ?\n" );
    
    TSTR s;
    ob->GetClassName( s );
    fprintf( fp, "Object converted to tri-object: %s\n", (char*)s );
#endif

   // kill WSM portion of matrix...premultiply it into the mesh.
   //  it never animates, so this is ok.
   if ( pNode != bounds_node )        // bounds node is weird for some reason.
   {
      // save off objOffset xform to later apply to points in mesh
      xform.setMatrix( &objOffsetMat );
      wsm.IdentityMatrix();
      convertToTransform( wsm, &xform );
   }
   else
   {
      objOffsetMat.identity();
      
      // The xform doesn't have an offset that we need for bounds node, so get it here:  
      //    Note - This might mess something else up since some of the game code may depend
      //    on the bounds node having the offset of before.  In which case we should 
      //    just use this offset to affect the fBounds field below after the shape has
      //    been built.  .....  OK, that's what we've done with the boundsBoxOffset.  
      MyTransform    getXlat;
      Matrix3 nodeTM = pNode->GetNodeTM( DEFAULT_TIME );
      Matrix3 rootTM = rootTo->GetNodeTM( DEFAULT_TIME );
      Matrix3 localTM = nodeTM * Inverse( rootTM );
      convertToTransform( localTM, &getXlat );
      // xform.fTranslate += getXlat.fTranslate;
      sb.boundsBoxOffset = getXlat.fTranslate;
   }

   Int16 flags = 0;

   // If a node's name begins with "hide", that means to default it's 
   //  visibility to FALSE.  So if no sequences are currently running that
   //  affect the visibility of that node, it will be invisible.  Normally,
   //  it would be visible unless specifically turned off.
   if ( !strnicmp( pname, "hide", 4 ) )
       flags |= TS::Shape::Object::DefaultInvisible;
       
   // Get the (animated) mesh for this object:
   MeshBuilder *pMB = new CelAnimMeshBuilder( &sb, nameId, nodeId,
                                                            xform, IsShape, flags );

   // add default morph keyframe:  basically a frame at time == 0.
   if ( addMorphDefault( pNode, rootTo, ob, pMB ) )
   {
      if ( !IsShape )    // skip this for bounds
      {
            // get the list of cel times for this mesh:
         Vector<int> * pMorphTimes = sb.GetCelTimes( range.Start(), range.End() );

         // for each sequence, add cels that fall in the time range of the sequence:
         for( VectorPtr<ShapeBuilder::Sequence*>::const_iterator si = 
             sb.sequences.begin(); si != sb.sequences.end(); si++ )
         {
            int seq = si - sb.sequences.begin();
            
            // See what happens during this sequence's time range...
            //  looks for visibility, material, and morph changes.
            int changes = scanForChanges( *si, pMorphTimes, pNode, ob );

            // Special override switch on the sequence helper object.
            if ( (*si)->fVisOnly )
               changes &= ~(MESH_CHANGES | MAT_CHANGES);
                 
#ifdef SDUMP
            fprintf( fp, "SubSequence: seq = %d, changes: Vis/Mesh/Mat = %d/%d/%d\n",
                           seq, (changes & VIS_CHANGES) != 0, 
                           (changes & MESH_CHANGES) != 0, 
                           (changes & MAT_CHANGES) != 0 );
#endif

            if ( changes )
            {
               Vector<int>::const_iterator ti;
               int last_vis = -1;               
               int count = 0;
               int first = sb.keyframes.size();

               // If anythings changes, there should be a keyframe at the 
               //  beginning of the seq.  the if() probably won't ever fail 
               //  in this case, since it's the first frame...
               if ( addMorphKeyframe( (*si)->fStartTime, (*si)->fStartTime, 
                          (*si)->fEndTime, pNode, ob, pMB, rootTo, &last_vis, 
                                 changes )         )
               {
                  count++;
               }
                   
               // if animated, add keys for start and intermediate times.
               for ( ti = pMorphTimes->begin(); ti != pMorphTimes->end(); ti++ )
                   if ( (*ti > (*si)->fStartTime) && (*ti <= (*si)->fEndTime) )
                       if ( addMorphKeyframe( (*ti), (*si)->fStartTime, 
                                      (*si)->fEndTime, pNode, ob, pMB, rootTo, &last_vis,
                             changes ) )
                       {
                          count++;
                       }
               
               // if any keys were added, add an end key
               if ( count )
               {
                   // add end time:
                   if ( addMorphKeyframe( (*si)->fEndTime, (*si)->fStartTime, 
                                   (*si)->fEndTime, pNode, ob, pMB, rootTo, &last_vis,
                             changes ) )
                   {
                     count++;                               
                   }
                         
                   pMB->AddSubSequence( sb, seq, first, count );
#ifdef SDUMP
                   fprintf( fp, "SubSequence: seq = %d, firstkey = %d, nkeys = %d\n", seq, first, count );
#endif
               }
            }
         }
           
#ifndef TEST
         delete pMorphTimes;
#endif
      }
        
      sb.meshes.push_back( pMB );
   }
#ifndef TEST
   else
       delete pMB;
#endif        
}

// if 's' ends in a number, chopNum returns pointer to first digit in this number
// if not, then returns pointer to '\0' at end of name or first of any trailing spaces
static const char * chopNum( const char * s)
{
   const char * p = s + strlen(s);

   if (p==s)
       return s;
   p--;

   // trim spaces from the end
   while (p!=s && *p==' ')
      p--;

   // back up until we reach a non-digit
   // gotta be better way than this...
   if (isdigit(*p))
      do
	  {
         if (p--==s)
            break;
	  } while (isdigit(*p));
   p++;
   return p;
}

//--------------------------------------------------------------
// recently changed how this works
// before it would return the first number found in a name
// now it returns a number only if it concludes the name
// so now bip01fart is not a detail (but used to be),
// but bipfart01 is (and always has been).
// -- caf, 6-21-99
static int getDetailNum( const char * name )
{
   // find detail size for the detail node:
   char s[100];
   strcpy( s, name );

   const char * p = chopNum(s);
   return atoi(p);
}

static int getDetailNum( INode *pNode )
{
   return getDetailNum( pNode->GetName() );
}

static bool childHasMesh( INode *pNode, int detailNum )
{
   for ( int d = 0; d < pNode->NumberOfChildren(); d++ )
   {
      INode *child = pNode->GetChildNode( d );
      
      if ( hasMesh( child ) )
         if ( detailNum < 0 || detailNum == getDetailNum( child ) )
            return TRUE;
   }
    
   return FALSE;
}

static char *makeDetailName( char *dest, char *name, int detail )
{
   strcpy( dest, name );
   
   if ( detail >= 0 )
   {
      if ( getDetailNum(name) != detail )
        itoa( detail, (char*)chopNum(dest), 10 );
   }
           
   return dest;        
}


static void MaxAssert( bool cond, const char * message )
{
   if( cond == false )
   {
      MessageBox(  GetActiveWindow(), message, 
            "Possible .DTS Exporter Code Problem, Please Report To Programmer:", 
                        MB_OK 
                        );
      exit(1);    // is this OK to exit?
   }
}



bool SceneEnumProc::isAlways( INode *pNode )
{
   return( !strnicmp(pNode->GetName(), "always", 6) || isDummyAlways(pNode) );
}
bool SceneEnumProc::isDummy( INode *pNode )
{
   return( !strncmp( pNode->GetName(), "dummy", 5 ) );
}
bool SceneEnumProc::isDummyAlways( INode *pNode )
{
   return ( !strnicmp( pNode->GetName(), "dummyalways", 11 ) );
}
bool SceneEnumProc::isBounds( INode *pNode )
{
   return ( !strnicmp( pNode->GetName(), "bounds", 6 ) );
}
bool SceneEnumProc::isVICON( INode *pNode )
{
   return ( ! strncmp( pNode->GetName(), "VICON", 5 ) || ! strcmp( pNode->GetName(), "Bip01") );
}



//
// The determiningDetail is used for VICON node heirarchy replication.  For detail 
// mesh node replication we are passed a viconContext.  
//
void SceneEnumProc::getNode(INode *pNode, INode *pRootTo, char *linkName, 
                            int priority, Bool IsDetail, Bool IsShape,
                            int _detailNum, Matrix3 *preScale, 
                            ViconContext * viconContext )
{
   int         detailOfThisTree = _detailNum;
   int         determiningDetail = detailOfThisTree;
   char        *pnodeName = pNode->GetName();
   Matrix3     sc_xform;
   
   sc_xform.IdentityMatrix ();

   // If we're traversing a different tree, then we have two different numbers
   //    for the different detail-dependent choices below.  If not, then
   //    the code is the same as before.  
#if (VICON_EXPORT_BEHAVIOR == 2)
   if( viconContext != NULL )
      if( viconContext->replicatingDetNumber != 0 )
         determiningDetail = viconContext->detailNumbers[0];
#endif

   if (IsDetail)
   {
       TS::Shape::Detail det;
       det.fRootNodeIndex = sb.nodes.size();
       det.fSize = getDetailNum( pNode );
       // MaxAssert( det.fSize > 0, avar("%s has weird detail # (%d)",pnodeName,det.fSize) );
       // add the detail to the shape:
       sb.AddDetail( det );
#ifdef SDUMP
       fprintf( fp, "Detail Node %s with size %8.3f, root %d\n", pnodeName, 
                           det.fSize, det.fRootNodeIndex );
#endif
   }

#  if VICON_EXPORT_BEHAVIOR < 2
   bool detail_matches = (detailOfThisTree<0 || detailOfThisTree==getDetailNum(pNode));
#  endif
   bool has_mesh = hasMesh( pNode );
   bool child_has_mesh = childHasMesh( pNode, determiningDetail );
   bool is_always = false, node_named_always = false;
   bool suppress_child_recurse = false;
   
#if (VICON_EXPORT_BEHAVIOR > 0)
   // special case hack so we can add the vicon node..  yick.  
   if( isVICON( pNode ) )
   {
      suppress_child_recurse = true;
      MaxAssert( no_transform_collapse, "VICON shapes require that you enable the "
            "NO-TRANSFORM-COLLAPSE option" );
   }
#endif

   if( isAlways(pNode) )
   {
      node_named_always = true;
      if ( strcmp( linkName, "always" ) )
         is_always = true;
   }

   // the lowly simple dummy.  case sensitve?  Should we let "Dummy*" function
   //    as before?  Might be a good idea.  
   bool a_simple_dummy = ( isDummy(pNode) && !node_named_always );
   
   INode *new_root = pRootTo;
   
#  if ( VICON_EXPORT_BEHAVIOR < 2 )
   bool legal_mesh = has_mesh && !is_always && detail_matches;
#  else
   bool legal_mesh = (has_mesh && !is_always);
#  endif

#if (VICON_EXPORT_BEHAVIOR == 2)
   // if( viconContext != NULL )
   //    if( viconContext->isDetailMeshNode && !is_always )
   //       legal_mesh = true;
#endif



   if ( legal_mesh || child_has_mesh || a_simple_dummy )
   {
      Object * ob            = pNode->GetObjectRef();
#ifdef MAX_2
      Interval range = pNode->GetTimeRange(TIMERANGE_ALL        | 
                                           TIMERANGE_CHILDNODES | 
                                           TIMERANGE_CHILDANIMS);
#else
      Interval range        = pNode->GetTimeRange(TIMERANGE_ALL);
#endif

      char nodename[ 100 ];
      char parentname[ 100 ];
      makeDetailName( nodename, pnodeName, detailOfThisTree );
      
      if ( IsDetail || is_always || !strcmp( linkName, "bounds") )
          strcpy( parentname, linkName );
      else
          makeDetailName( parentname, linkName, detailOfThisTree );

#ifdef SDUMP
      fprintf( fp, "***********************************************\n" );
      char *node_type = legal_mesh? "Mesh" : "Parent";
      fprintf( fp, "%s node %s\n", node_type, pnodeName );
      fprintf( fp, "   will be called %s, rooted to %s, linked to %s\n", 
                  nodename, pRootTo->GetName(), parentname );
      fprintf( fp, "Anim time range: %d to %d\n", range.Start(), range.End() );
#endif

      SBNode * pSBNode = sb.AddNode( nodename, parentname );

      bool trans_anim;
      
#if (VICON_EXPORT_BEHAVIOR == 2)
      if ( determiningDetail != detailOfThisTree )
      {
         // This is a replicated node, so look up its analog in the main tree.  
         char  analogName[ 100 ];
         
         makeDetailName( analogName, pnodeName, determiningDetail );
         int   index = sb.FindNode( analogName );
         MaxAssert ( index != -1, 
                  avar("Couldn't find VICON analog %s", analogName)
               );
         MaxAssert ( !isAlways( pNode ), "Shouldn't be an always node here" );
         SBNode   * analog = sb.nodes[ index ];

         // copy over relevant values. 
         pSBNode->scale = analog->scale;
         pSBNode->mirrored = analog->mirrored;
         pSBNode->snode.fnSubSequences = analog->snode.fnSubSequences;
         pSBNode->snode.fFirstSubSequence = analog->snode.fFirstSubSequence;
         pSBNode->snode.fDefaultTransform = analog->snode.fDefaultTransform;
         
         trans_anim = (pSBNode->snode.fnSubSequences != 0);
      }
      else
#endif
         trans_anim = getNodeTransformKeys( pNode, pRootTo, pSBNode, 
                                                          priority, preScale );

      int nodeId = sb.nodes.size() - 1;
      int parentNodeId = sb.FindNode( parentname );
      int nameId = pSBNode->snode.fName;
      collapseMat.identity();

      // don't collapse unanimated transforms of mesh nodes
      if ( no_transform_collapse )    
      {        
#ifdef SDUMP
           if ( !trans_anim && parentNodeId >= 0 && !IsDetail && !IsShape )
               fprintf( fp, "collapse overridden: no_collapse specified\n" );
#endif
          trans_anim = true;
      }

      // Sometimes take the node out (note name hangs around...).  
      if ( !trans_anim && parentNodeId >= 0 && !IsDetail 
                  && !IsShape && !a_simple_dummy && !isDummyAlways(pNode) )
      {
         TS::Transform cxform;
         cxform = sb.transforms[ pSBNode->snode.fDefaultTransform ];
#ifdef SDUMP        
         fprintf( fp, "no transform animation found.  node %s removed\n",
                     nodename );
         fprintf( fp, " attaching to parent %s instead (#%d)..parentname = %s\n", 
                 (const char *)sb.names[ sb.nodes[ parentNodeId ]->snode.fName ], 
                 parentNodeId, parentname );
         const Point3F &pt = cxform.getTranslate();
         QuatF rot;
         cxform.getRotate( &rot );
         fprintf( fp, "  collapse translate: %8.8f %8.8f %8.8f\n", 
             pt.x, pt.y, pt.z );
         fprintf( fp, "  collapse rotate: %8.8f %8.8f %8.8f %8.8f\n", 
             rot.x, rot.y, rot.z, rot.w );
#endif
         cxform.setMatrix( &collapseMat );
         sb.nodes.decrement();
         nodeId = parentNodeId;
      }    
      else
      {    
         new_root = pNode;            
         linkName = nodename;
      }

#if COLLAPSE_SCALE    
      Point3 sc_vec;
      sc_vec.x = sb.nodes[ nodeId ]->scale.x;
      sc_vec.y = sb.nodes[ nodeId ]->scale.y;
      sc_vec.z = sb.nodes[ nodeId ]->scale.z;
      sc_xform.Scale( sc_vec );
#endif
       
      MyTransform sc_tsxform;
      convertToTransform( sc_xform, &sc_tsxform );
      sc_tsxform.setMatrix( &scaleMat );

#ifdef SDUMP
      fprintf( fp, "scaleMat: (%8.8f, %8.8f, %8.8f)\n", 
                  scaleMat.m[0][0], scaleMat.m[0][1], scaleMat.m[0][2] );
      fprintf( fp, "scaleMat: (%8.8f, %8.8f, %8.8f)\n", 
                  scaleMat.m[1][0], scaleMat.m[1][1], scaleMat.m[1][2] );
      fprintf( fp, "scaleMat: (%8.8f, %8.8f, %8.8f)\n", 
                  scaleMat.m[2][0], scaleMat.m[2][1], scaleMat.m[2][2] );
#endif

      if ( !isDummy(pNode) &&  has_mesh )
      {
#        if ( VICON_EXPORT_BEHAVIOR >= 2 )
         // We need to check in case we don't really have a mesh.  
         if( viconContext == NULL || viconContext->nodeHasMesh)
#        endif
            getNodeMorphKeys( pNode, ob, range, nodeId, IsShape, pRootTo, nameId );
      }
    }
#ifdef SDUMP
   else 
   {
      char reason[ 100 ];
      *reason = 0;
      
      if ( !has_mesh )
         sprintf( reason, "no mesh found." );
      else if ( is_always )
         sprintf( reason, "already an always node." );
      else {
#        if ( VICON_EXPORT_BEHAVIOR < 2 )
            if ( !detail_matches )
               sprintf( reason, "detail not %d.", detailOfThisTree );
#        else
            MaxAssert ( 0, "Node skipped for unknown reason" );
#        endif
      }
          
      fprintf( fp, "Node %s skipped because: %s\n", pnodeName, reason );
   }
#endif

   if ( strcmp( linkName, "always" ) && !suppress_child_recurse )
   {
#     if (VICON_EXPORT_BEHAVIOR >= 2)
      // 
      // We want to do a different call-down if we are building the multiple instances
      //    of the VICON node heirarchy for each of the detail levels, and if this is a 
      //    node with the detail-numbered mesh leaves.  
      //
      // We (re)use the suppress_child_recurse flag to signal that we are doing a 
      //       the special recurse.  nodeHasMesh member must be set as default.  
      // 
         if( _detailNum != -1 && viconContext != NULL )
         {
            viconContext->nodeHasMesh = true;
            int   c;
            
            if( ! has_mesh )
            {
               VectorPtr<INode *>   detailMeshList, subTrees;
               INode *              controllingNode = NULL;
               INode *              installingNode = NULL;
               bool                 curDetailHasMesh = false;

               for( c = 0; c < pNode->NumberOfChildren(); c++ )
               {
                  INode * child = pNode->GetChildNode(c);
                  
                  if( hasMesh( child ) && !isDummy(child) )
                  {
                     detailMeshList.push_back( child );
                     if( getDetailNum( child ) == viconContext->detailNumbers[0] )
                     {
                        suppress_child_recurse = true;
                        controllingNode = child;
                     }
                     
                     if ( getDetailNum( child ) == _detailNum )
                     {
                        curDetailHasMesh = true;
                        installingNode = child;
                     }
                  }
                  else
                     subTrees.push_back( child );
               }


               // Do a different call-down than the one at the bottom.  We recurse down 
               //    for each subtree, and then just a single level call for the mesh
               //    at the current detail we are rendering.  A better approach to all this
               //    probably would have been to have a separate routine which installs 
               //    a mesh.  As it is the above code is way to 'modal' - i.e. is it just a
               ///      mesh, or maybe it's a 'fake' mesh in the case where we make a bogus
               //          one to replicate the vicon hierarchy at lower levels, etc...
               if( suppress_child_recurse )
               {
                  // do the normal call down on non-meshes.
                  for( c = 0; c < subTrees.size(); c++ )
                     getNode( subTrees[ c ], new_root, linkName, priority+1, 
                           FALSE, FALSE, _detailNum, &sc_xform, viconContext );

                  // do the special mesh getNode() call on the current detail number.  
                  //    There are three cases:  the main detail, in which case we do all
                  //    the normal work;   a lesser detail where a mesh is present;  
                  //    a lesser detail where no mesh is present.  
                  // viconContext->isDetailMeshNode = true;
                  if( _detailNum != viconContext->detailNumbers[0] )
                  {
                     // if no mesh present, we have to "trick" the routine into 
                     //    at least setting things up.  
                     if( !(viconContext->nodeHasMesh = curDetailHasMesh) )
                        installingNode = controllingNode;
                  }
                  // for other case nodeHasMesh already set above.  

                  getNode( installingNode, new_root, linkName, priority+1, FALSE, FALSE,
                         _detailNum, &sc_xform, viconContext );
               }
            }
            else
            {
               const char * errTxt = avar( "%s should be childless", pNode->GetName() );
               // MaxAssert( !pNode->NumberOfChildren(), errTxt );
               MaxAssert( 1, errTxt );
            }
         }
#     endif

      if( ! suppress_child_recurse )
         for ( int c = 0; c < pNode->NumberOfChildren(); c++ ){
            INode *child = pNode->GetChildNode(c);
            getNode( child, new_root, linkName, priority+1, FALSE, FALSE,
                         _detailNum, &sc_xform, viconContext );
         }
   }
}


//--------------------------------------------------------------



//
// This is the main shape building outer loop, builds the shape from the 3DS nodes
// that were stored off from the callback calls.  
//
void SceneEnumProc::getNodes(FileWStream &ost)
{
   // Make sure they have a bounding box
   MaxAssert( bounds_node != NULL, "You need a bounding box node named 'bounds'!"
            "  Without it we can't export your shape.... " 
      );
   
   if (bounds_node)
   {
      ShapeBuilder::Statistics   vStats[10];
      int      numViconStats = 0;

      // Leave bounds rooted to root node
#ifdef MAX_2
      getNode(bounds_node, bounds_node->GetParentNode(), 
               "Scene Root", 0, FALSE, TRUE, -1 );
#else
      getNode(bounds_node, bounds_node->GetParentNode(), 
      "root", 0, FALSE, TRUE, -1);        
#endif                  
      VectorPtr<INode *>::iterator it;
       
      // If there are any nodes that we want attached to the shape, 
      // but not attached to a particular detail (always drawn and 
      // animated), they will be in the always_nodes list.  Root 
      // these nodes to the bounds node (transform-wise), but link 
      // them to a special "always" node we create.  3space handles 
      // this node separately, animating and rendering it as well 
      // as the current detail's node.
      if (always_nodes.size() >= 1)
      {
         MyTransform       xform;
         Matrix3           mat;
         SBNode            *alwaysNode = sb.AddNode("always", "bounds");

         mat.IdentityMatrix();
         convertToTransform(mat, &xform);

          // The default transform is identity (no transform)
         alwaysNode->SetDefaultTransform(sb, xform);
              
          // Root all of the always nodes to the bounding box node
         for(it = always_nodes.begin(); it != always_nodes.end(); it ++)
         {
            getNode(*it, bounds_node, "always", 0, FALSE, FALSE, -1);
         }
      }
      
      // Then recurse through the tree below each child of the root.
      // Each of these will be interpreted as detail nodes.
      // Root each detail node to the bounds node (represents the shape)
      for (it = entries.begin(); it != entries.end(); it ++)
      {
         INode *pNode = *it;

         // Do special case for VICON hierarchy
         if (isVICON(pNode) )
         {
#if VICON_EXPORT_BEHAVIOR==0

            //****************************************************************
                  for (int c = 0; c < pNode->NumberOfChildren(); c++)
                  {
                     INode *detail = pNode->GetChildNode(c);
                     INode *rootTo = detail;
                     
                     if (hasMesh(detail))
                     {
                        int detailNum = getDetailNum(detail);
                        getNode(detail, bounds_node, "bounds", 
                                1, TRUE, FALSE, detailNum);
                        
#                       if VICON_MUL                     
                        // Detail's transform is cleared (pre-mul'd into mesh)
                        // so root to bounds, instead of rooting to detail
                        rootTo = bounds_node;
#                       endif

                        for (int d = 0; d < pNode->NumberOfChildren(); d++)
                        {
                           INode *child = pNode->GetChildNode(d);
                           
                           if (!hasMesh(child))
                           {
                              getNode(child, rootTo, detail->GetName(), 
                                       2, FALSE, FALSE, detailNum);
                           }
                        }
                     }
                  }//for
            //****************************************************************
            
#elif VICON_EXPORT_BEHAVIOR==1

            // New VICON export behavior.  We need to have a VICON node present which the 
            // other nodes are attached to.  The structure now is to create the detail nodes 
            // and attach a separate copy of VICON node under each, and then to have the 
            // tree below that. 

            //****************************************************************
            INode *VICON_node = pNode;
            
            // Now create detail children and attach to the VICON node just like above. 
            for (int c = 0; c < VICON_node->NumberOfChildren(); c++)
            {
               INode *detail = VICON_node->GetChildNode(c);
               INode *rootTo = detail;
               
               // build list of all details, the first detail will represent the tree,
               //    while the next ones will reuse all the transform and keyframe data.  
               //    This will basically be a node for node COPY, but with different 
               //    mesh data... (?).  
               
               // Fetch list of details first.  Build the main tree, then use special
               //    recursor to make the copies - I just don't want to make getNode()
               //       any more mode-dependent than it already is.  
               if (hasMesh(detail))
               {
                  int detailNum = getDetailNum(detail);
                  getNode(detail, bounds_node, "bounds", 1, TRUE, FALSE, detailNum);
                  getNode ( VICON_node, bounds_node, detail->GetName(), 1, 
                           FALSE, FALSE, detailNum );

                  sb.dumpStatistics(avar("Building node %s:\n", detail->GetName()));
                     
#                 if VICON_MUL 
                  // Does this port directly over for the insertion of VICON node?  
                  rootTo = VICON_node;
#                 endif
                  for (int d = 0; d < VICON_node->NumberOfChildren(); d++)
                  {
                     INode *child = VICON_node->GetChildNode(d);
                     
                     if (!hasMesh(child))
                     {
                        // getNode(child, rootTo, detail->GetName(), 3, FALSE, FALSE, detailNum);
                        getNode(child, rootTo, rootTo->GetName(), 2, FALSE, FALSE, detailNum);
                     }
                  }
                  
                  sb.dumpStatistics( avar("%s node done.\n", detail->GetName()) );
               }//for
            }
            //****************************************************************
            
#elif VICON_EXPORT_BEHAVIOR==2

            // Second VICON export revision:  we want to duplicate the node hierarchy 
            // for each detail level, but reuse the sequences to save on 
            // keyframes (space).  

            //****************************************************************
            INode    *VICON_node = pNode, *rootTo;
            int      index;
      
            // Build list of details first, make sure we know the "main" one and that 
            //     it comes first.  
            VectorPtr<INode *>   detailList, subTrees;
            ViconContext         vc;
            for( index = 0; index < VICON_node->NumberOfChildren(); index++ )
            {
               INode *child = VICON_node->GetChildNode(index);

               if( hasMesh(child) )
               {
                  sb.dumpStatistics( avar("adding detail %s\n", child->GetName()));
                  detailList.push_back( child );
                  vc.detailNumbers.push_back( getDetailNum(child) );
               }
               else
                  subTrees.push_back( child );
            }
            
            // Make sure the highest detail comes first.  Not really a sort here - we just
            //    want the highest one first, so the loop below works.  
            for( index = 1; index < vc.detailNumbers.size(); index++ )
            {
               if( vc.detailNumbers[index] >= vc.detailNumbers[0] )
               {
                  MaxAssert( vc.detailNumbers[index] > vc.detailNumbers[0], 
                           "Must not have two detail meshes with same number" );

                  // Swap the higher number into first place.  
                  int   tempInt = vc.detailNumbers[index];
                  vc.detailNumbers[index] = vc.detailNumbers[0];
                  vc.detailNumbers[0] = tempInt;

                  INode * tempNode = detailList[index];
                  detailList[index] = detailList[0];
                  detailList[0] = tempNode;
               }
            }

            // Now we build the node heirarchy completely at the highest detail level, 
            //    just like before, and the other detail levels use the node replication 
            //    mode of the getNode() recursor.  The check is in the inner loop. 
            rootTo = VICON_node; 
            char * rootName = rootTo->GetName();
            
            for( int dNum = 0; dNum < vc.detailNumbers.size(); dNum++ ) 
            {
               sb.captureStats ( vStats [ numViconStats ] );      // for sdump
               int   detailNum = vc.detailNumbers [ dNum ];
               INode *detail = detailList[dNum];
                              
               MaxAssert( !detail->NumberOfChildren(),
                     avar("Detail %s can't have kids", detail->GetName())  );
               getNode( detail, bounds_node, "bounds", 1, TRUE, FALSE, detailNum );
               getNode( VICON_node, bounds_node, detail->GetName(), 1, 
                        FALSE, FALSE, detailNum );

               vc.replicatingDetNumber = dNum;
            
               for( index = 0; index < subTrees.size(); index++ )
               {
                  INode *child = subTrees[ index ];
        
                  getNode(child, rootTo, rootName, 2, 
                        FALSE, FALSE, detailNum,   NULL, &vc );
                  
               }
               sb.getStatsDifference ( vStats [ numViconStats++ ] );
            }
            
            // ****************************************************************
            
#else
#  error("Must define VICON export behavior")
#endif
         }
         else//not VICON node.
         {
            getNode(pNode, bounds_node, "bounds", 1, TRUE, FALSE, -1);
         }
      }

#if SDUMP
      fprintf(fp, "*****\n");
      for(VectorPtr<ShapeBuilder::Sequence*>::const_iterator si = 
          sb.sequences.begin(); si != sb.sequences.end(); si++)
      {
          int seq = si - sb.sequences.begin();

          fprintf(fp, "sequence %s:  priority = %d\n", 
                 (const char *)sb.names[seq], (*si)->fPriority);

      }
      fprintf(fp, "*****\n");

      for( int v = 0; v < numViconStats; v++ )
         sb.dumpStats ( vStats [ v ], "Vicon Mesh Statistics\n" );
#endif
        
      TS::Shape *pShape = sb.MakeShape();

	  // validate shape -- for now just make sure at least one detail
	  MaxAssert(pShape->fDetails.size() && pShape->fNodes.size()>1,"No details...must have at least one detail");
      pShape->store(ost);
      
#if SDUMP
    //---------------------------------------
    // load the just saved shape in as a shape resource, and create
    // create a shape instance out of shape (don't have resource
	// so we use dummy constructor).
    // next, write the shapeInstance structure to the dump file
    TSShapeInstance * pShapeInst = new TSShapeInstance();
	pShapeInst->setMaterialList(const_cast<TSMaterialList*>(pShape->getMaterialList()));
	pShapeInst->init(pShape);
    dumpStructure(pShapeInst);
	delete pShapeInst;
#endif

      // Make sure the material list isn't deleted twice:
      pShape->setMaterialList(0);
      delete pShape;
      
   }//if bounds_node exists
}

//--------------------------------------------------------------
// callback for EnumTree:

int SceneEnumProc::callback(INode *pNode) 
{
    // We are only interested in nodes which are
    // sequences or meshes
    const char *name  = pNode->GetName();
    const char *pname = pNode->GetParentNode()->GetName();

    ObjectState os = pNode->EvalWorldState(0);

   // If it is a sequence, get the sequence
    if (os.obj->ClassID() == Class_ID(SEQUENCE_CLASS_ID, 0))
    {
        getSequence(pNode);
    }

   // If it is a transition, get the transition
    else if (os.obj->ClassID() == Class_ID(TRANSITION_CLASS_ID1, TRANSITION_CLASS_ID2))
    {
        getTransition(pNode);
    }

    else if (os.obj->CanConvertToType(triObjectClassID)) 
    {
        if (!strnicmp(name, "always", 6)   ||
            !strnicmp(name, "dummyalways", 11))
        {
#ifdef SDUMP
            fprintf(fp, "Mesh Node %s with parent %s added to always list\n",
                        name, pname);
#endif
            always_nodes.push_back(pNode);
        }

      // If this node is a direct descendant of the root node ...
#ifdef MAX_2
        else if (pNode->GetParentNode()->IsRootNode())
#else
        else if (!stricmp(pname, "root"))
#endif
        {
         // See if it is a bounding box.  If so, save it as THE bounding
         // box for the scene
            if (!stricmp(name, "bounds"))
            {
                AssertMessage(!bounds_node, "More than one bounds nodes found.");
                bounds_node = pNode;
            }

         // If it is not a bounding box, and it is not a camera, it is
         // a mesh node that we want to keep
            else if (strnicmp(name, "camera", 6))
            {
#ifdef SDUMP
                fprintf(fp, "Mesh Node %s with parent %s added to entry list\n",
                            name, pname);
#endif
                entries.push_back(pNode);
            }
        }
   }

   // VICON's are special cases
   else if ( isVICON( pNode ) )
   {
#ifdef SDUMP
      fprintf(fp, "VICON Node %s with parent %s added to entry list\n",
                    name, pname);    
#endif
      entries.push_back(pNode);
   }

    return TREE_CONTINUE;    // Keep on enumeratin'!
}

//--------------------------------------------------------------

void SceneEnumProc::dumpNode(int level, TSShapeInstance * pShapeInst, TSShapeInstance::NodeInstance * node)
{
#ifdef SDUMP
   if (node == NULL)
      return;

   const TSShape * pShape = &pShapeInst->getShape();

   char space[256];
   for (int i = 0; i < level*5; i++)
      space[i] = ' ';
   space[level*5] = '\0';
   
   const char *nodeName = "";
   if (node->fNode.fName != -1)
     nodeName = pShape->fNames[node->fNode.fName];
   fprintf(fp , "%s%s\r\n", space, nodeName);

   for (int j=0;j<node->fObjectList.size(); j++)
   {
	   nodeName = "";
	   const TSShape::Object & object = static_cast<const TSShapeInstance::ShapeObjectInstance *>(node->fObjectList[j])->fObject;
	   if (object.fName!=-1)
		   nodeName = pShape->fNames[object.fName];
	   fprintf(fp,"%s  mesh: %s\r\n",space,nodeName);
   }

   dumpNode(level + 1, pShapeInst, node->fpChild);      
   dumpNode(level, pShapeInst, node->fpNext);      
#endif
}

void SceneEnumProc::dumpStructure(TSShapeInstance * pShapeInst)
{
#ifdef SDUMP
   const TSShape * pShape = &pShapeInst->getShape();
   
   fprintf(fp,"Hierarchy:\r\n");

   int i;
   for (i = 0; i < pShape->fDetails.size(); i++)
   {
      TSShapeInstance::NodeInstance *node = pShapeInst->getDetail(i);
      const char *nodeName = "";
      if (node->fNode.fName != -1)
        nodeName = pShape->fNames[node->fNode.fName];
      fprintf(fp, "%s (detail level %d)\r\n", nodeName, i);
      dumpNode(1, pShapeInst, node->fpChild);
   }

   fprintf(fp, "\r\nSequences:\r\n");
   for (i = 0; i < pShape->fSequences.size(); i++)
   {
      const char *name = "(none)";
      if (pShape->fSequences[i].fName != -1)
         name = pShape->fNames[pShape->fSequences[i].fName];
      fprintf( fp, "%3d: %s\r\n", i, name);
   }

   char buffer1[256];
   char buffer2[256];
   fprintf(fp,"\r\n\nMaterials:\r\n");
   const TSMaterialList & ml = *pShape->getMaterialList();
   for (i=0; i<ml.getMaterialsCount(); i++)
   {
	   strcpy(buffer1,"null");
	   strcpy(buffer2,"null");
	   const TSMaterial & mat = ml[i];
	   switch (mat.fParams.fFlags & TSMaterial::MatFlags)
	   {
	   case TSMaterial::MatPalette:
		   sprintf(buffer1,"palette %i",mat.fParams.fIndex);
		   break;
	   case TSMaterial::MatRGB:
		   sprintf(buffer1,"rgb (%i,%i,%i)",mat.fParams.fRGB.fRed,mat.fParams.fRGB.fGreen,mat.fParams.fRGB.fBlue);
		   break;
	   case TSMaterial::MatTexture:
		   sprintf(buffer1,"texture (%s)",mat.fParams.fMapFile);
		   break;
	   }

	   switch (mat.fParams.fFlags & TSMaterial::ShadingFlags)
	   {
	   case TSMaterial::ShadingNone:
		   strcpy(buffer2,"none");
		   break;
	   case TSMaterial::ShadingFlat:
		   strcpy(buffer2,"flat");
		   break;
	   case TSMaterial::ShadingSmooth:
		   strcpy(buffer2,"smooth");
		   break;
	   }
	   fprintf(fp,"material %i:  %s, shading: %s\r\n",i,buffer1,buffer2);
   }

#endif
}

//--------------------------------------------------------------

SceneEnumProc::SceneEnumProc(IScene *scene) 
{
    // open debugging file:
#ifdef SDUMP
    fp = fopen( "dump", "w" );
    assert( fp );
    sb.fp = fp;
#endif
    bounds_node = 0;
    no_transform_collapse = FALSE;

    scene->EnumTree(this);
}

SceneEnumProc::~SceneEnumProc() 
{
    // close debugging file:
    fclose( fp );
}

//--------------------------------------------------------------


