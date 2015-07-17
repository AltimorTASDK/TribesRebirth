#include "ShapeBuilder.h"

//--------------------------------------------------------------


struct IFL_INFO
{
    char *fname;
    int hold_cnt;
};

struct IFL_FILE
{
    char *fname;
    IFL_INFO *info;
    int info_count;
    int mat_index;
};

static IFL_FILE ifl_files[ 16 ];
static int num_ifl_files;


ShapeBuilder::ShapeBuilder()
{
   num_ifl_files = 0;
   fp = 0;
   alwaysNode = -1;
   
   boundsBoxOffset.set();
}

ShapeBuilder::~ShapeBuilder()
{
   for( VectorPtr<Sequence*>::iterator it = sequences.begin(); it != sequences.end(); it++ )
      delete (*it);
}

//--------------------------------------------------------------



StdMat *getStdMaterial( INode * pNode, int mtlIndex )
{
    Mtl * pMtl = pNode->GetMtl();
    if( !pMtl )
        return 0;
    else if( pMtl->ClassID() == Class_ID(DMTL_CLASS_ID,0) )
        return (StdMat*)pMtl;
    else if( pMtl->ClassID() == Class_ID(MULTI_CLASS_ID,0) )
    {
        MultiMtl *pMMtl = (MultiMtl*)pMtl;

      if ( mtlIndex >= pMMtl->NumSubMtls() )
      {
         char str[ 100 ];
         sprintf( str, "material index out of range, #%d", mtlIndex );
           AssertMessage( 0, str );
           return 0;
      }
      
        pMtl = pMMtl->GetSubMtl( mtlIndex );
        if( pMtl->ClassID() == Class_ID(DMTL_CLASS_ID,0) )
            return (StdMat*)pMtl;

        AssertFatal( 0, "Unexpected material type" );
    }

    return 0;
}

int ShapeBuilder::AddMaterial( INode * pNode, int mtlIndex )
{
   StdMat *material = getStdMaterial( pNode, mtlIndex );
   if ( material )
      return AddStandardMaterial( material );
   else
      return 0;      
}
//--------------------------------------------------------------
#define MAX_IFL_INFOS  100

static IFL_FILE *get_ifl_info( char *dname )
{
    IFL_FILE *flp = ifl_files;
    
    for ( int i = 0; i < num_ifl_files; i++, flp++ )
        if ( !strcmp( flp->fname, dname ) )
            return flp;
            
    if ( ++num_ifl_files >= (sizeof( ifl_files ) / sizeof( IFL_FILE )) )
    {
        AssertMessage( 0, "too many .ifl files used" );
        return 0;
    }
            
    FILE *fp = fopen( dname, "rt" );
    
    if ( !fp )
    {
        AssertMessage( 0, "couldn't open .ifl file" );
        return 0;
    }
    
    flp->fname = new char[ strlen( dname ) + 1 ];
    strcpy( flp->fname, dname );
    
    char line[ 100 ];
    char bmpname[ 32 ];
    IFL_INFO *ifl = flp->info = new IFL_INFO[ MAX_IFL_INFOS ];
    flp->info_count = 0;
    
    while ( fgets( line, sizeof( line ), fp ) )
    {
        ifl->hold_cnt = 0;
        *bmpname = 0;
        sscanf( line, "%32s %d", bmpname, &ifl->hold_cnt );
        ifl->fname = new char[ strlen( bmpname ) + 1 ];
        strcpy( ifl->fname, bmpname );
        
        if ( ifl->hold_cnt < 1 )
            ifl->hold_cnt = 1;
            
        ifl++;
        
        if ( flp->info_count++ >= MAX_IFL_INFOS )
        {
            AssertMessage( 0, "Too many entries in .ifl file" );
            return 0;
        }
    }                
    
    fclose( fp );
    return flp;
}


int ShapeBuilder::AddStandardMaterial( StdMat * pMtl )
    {
    TSTR & name = pMtl->GetName();

    // from StdMat:
    int shading                = pMtl->GetShading(); // SHADE_CONST,SHADE_PHONG,SHADE_METAL
    int twoSided            = pMtl->GetTwoSided();
    int transparencyType = pMtl->GetTransparencyType();
    float selfIllum        = pMtl->GetSelfIllum(0);
    Color color                = pMtl->GetDiffuse(0);

    // from Mtl:
    float transparency    = pMtl->GetXParency();

    TS::Material mat;
    
    mat.fParams.fFlags = 0;
    
    // 
    if( IsEqual( selfIllum, 1.0f ) )
        mat.fParams.fFlags |= TS::Material::ShadingNone;
    else if( shading == SHADE_CONST )
        mat.fParams.fFlags |= TS::Material::ShadingFlat;
    else
        mat.fParams.fFlags |= TS::Material::ShadingSmooth;

    mat.fParams.fRGB.fRed = color.r * 255;
    mat.fParams.fRGB.fGreen = color.g * 255;
    mat.fParams.fRGB.fBlue = color.b * 255;
    
    Texmap * diffuse = pMtl->GetSubTexmap( ID_DI );
    IFL_FILE *ifl_file = 0;
    
    if( diffuse && diffuse->ClassID() == Class_ID(BMTEX_CLASS_ID,0))
        {
        mat.fParams.fFlags |= TS::Material::MatTexture;

        BitmapTex *bmt = (BitmapTex*)diffuse;

        // get name of bitmap w/o path:
        char *dname =  bmt->GetMapName();
      char bmpname[ 100 ];
      char *ext;
        strcpy( bmpname, dname );

        if ( strstr( bmpname, ".ifl" ) )
        {
            ifl_file = get_ifl_info( bmpname );
            
            if ( !ifl_file || !ifl_file->info_count )
                return 0;
                
            strcpy( bmpname, ifl_file->info[ 0 ].fname );
        }

      if (((ext = strstr( bmpname, ".tga" )) != NULL) ||
          ((ext = strstr( bmpname, ".TGA" )) != NULL)) {
            mat.fParams.fFlags |= TS::Material::TextureTranslucent;
         strcpy( ext, ".bmp" );
      } else if (((ext = strstr( bmpname, ".png" )) != NULL) ||
                 ((ext = strstr( bmpname, ".PNG" )) != NULL)) {
         AssertWarn(0, "Marked PNG as translucent");
         mat.fParams.fFlags |= TS::Material::TextureTranslucent;
         strcpy( ext, ".bmp" );
      }
        
        TSTR filename;
        SplitPathFile( TSTR(bmpname), 0, &filename );
        strncpy( mat.fParams.fMapFile, filename, sizeof(mat.fParams.fMapFile) );

        // '@' in name indicates color 0 is transparent
        if( strchr( bmpname, '@' ) )
            mat.fParams.fFlags |= TS::Material::TextureTransparent;

        // 2d transform of texture:
        // (We don't want to have to deal with this so it had better be
        // an identity transform)
        StdUVGen * uv = bmt->GetUVGen();
        float uoff = uv->GetUOffs(0);
        float voff = uv->GetVOffs(0);
        float uscl = uv->GetUScl(0);
        float vscl = uv->GetVScl(0);
        float ang = uv->GetAng(0);
        }
    else
        mat.fParams.fFlags |= TS::Material::MatRGB;

    int m;

    for( m = 0; m < materials.size(); m++ )
        if( mat == materials[m] )
            break;
            
    if ( m == materials.size() )            
        materials.push_back( mat );
    
    if ( ifl_file )
        ifl_file->mat_index = m;
    
    return m;
    }

//--------------------------------------------------------------

int ShapeBuilder::FindName( char *name )
    {
    for( Vector<TS::Name>::iterator ni = in_names.begin(); ni != in_names.end(); ni++ )
        if( !strcmp( name, *ni ) )
            return ni - in_names.begin();
    return -1;
    }

//--------------------------------------------------------------

char * ShapeBuilder::tweakName(char * name)
{
	if (!strcmp(name,"Bip01"))
	   // we're the VICON node...but a biped shape
	   return "VICON";

    if (!strncmp(name,"Bip01 ",6))
		// get rid of annoying prefix...
		return name+6;

	return name;
}
//--------------------------------------------------------------

int ShapeBuilder::AddName( char *name )
    {
    int index = FindName( name );
    if( index == -1 )
        {
		in_names.push_back( TS::Name(name) );

        // now let's tweak the name a little
        name = tweakName(name);

        names.push_back( TS::Name(name) );
        index = names.size() - 1;
        }
    return index;
    }

//--------------------------------------------------------------

ShapeBuilder::Sequence * ShapeBuilder::AddSequence( char *name, int startTime, int endTime )
    {
    Sequence * pSeq = new Sequence( AddName( name ), startTime, endTime );
    sequences.push_back( pSeq );
    return pSeq;
    }

//--------------------------------------------------------------

ShapeBuilder::Transition * ShapeBuilder::AddTransition( char *name, 
                                 int startTime, int endTime, float dur, Point3F *shift )
{
    Transition *tr = new Transition( AddName( name ), startTime, endTime, dur, shift );
    transitions.push_back( tr );
    return tr;
}

int ShapeBuilder::AddTransform( MyTransform const & xform )
{
    Vector<TS::Transform>::iterator ti;
    TS::Transform new_trans;
    new_trans.set( xform );
//   new_trans.setMirrored( xform.fMirrored );
        
    for ( ti = transforms.begin(); ti != transforms.end(); ti++ )
        if ( (*ti) == new_trans )        
            return ti - transforms.begin();
    
    transforms.push_back( new_trans );
    return transforms.size() - 1;
}
//--------------------------------------------------------------


int ShapeBuilder::FindNode( char *name )
{
    for ( int i = 0; i < nodes.size(); i++ )
        if ( !strcmp( names[ nodes[i]->snode.fName ], name ) )
            return i;
    return -1;            
}

void ShapeBuilder::DeleteNode( Node *delnode )
{
    for ( VectorPtr<Node*>::iterator ni = nodes.begin(); ni != nodes.end(); ni++ )
        if ( (*ni) == delnode )
        {
            nodes.erase( ni );
            return;
        }
}


ShapeBuilder::Node * ShapeBuilder::AddNode( char *name, char *parent )
{
    int parentName = FindName( parent );
    // if parent node is not already in list, it must be "root"
    int parentNode = -1;
    for( VectorPtr<Node*>::const_iterator ni = nodes.begin(); ni != nodes.end(); ni++ )
        if( (*ni)->snode.fName == parentName )
            {
            parentNode = ni - nodes.begin();
            break;
            }
    Node * pNode = new Node( AddName( name ), parentNode );
   pNode->scale.set( 1.0f, 1.0f, 1.0f );
    pNode->mirrored = 0;
    
    if ( !stricmp( name, "always" ) )
        alwaysNode = nodes.size();
        
    nodes.push_back( pNode );
    return pNode;
}

//--------------------------------------------------------------
int ShapeBuilder::consider_mat( int mat_index, IFL_INFO *inf )
{                    
   TSTR filename;
   TS::Material mat = materials[ mat_index ];
   char *ext;
   char bmpname[ 100 ];

   strcpy( bmpname, inf->fname );

   if (((ext = strstr( bmpname, ".tga" )) != NULL) ||
       ((ext = strstr( bmpname, ".TGA" )) != NULL)) {
      mat.fParams.fFlags |= TS::Material::TextureTranslucent;
      strcpy( ext, ".bmp" );
   } else if (((ext = strstr( bmpname, ".png" )) != NULL) ||
              ((ext = strstr( bmpname, ".PNG" )) != NULL)) {
      mat.fParams.fFlags |= TS::Material::TextureTranslucent;
      strcpy( ext, ".bmp" );
   }
        
    SplitPathFile( TSTR(bmpname), 0, &filename );
    strncpy( mat.fParams.fMapFile, filename, sizeof(mat.fParams.fMapFile) );

    // '@' in name indicates color 0 is transparent
    if ( strchr( bmpname, '@' ) )
        mat.fParams.fFlags |= TS::Material::TextureTransparent;

    int val;

    for( val = 0; val < materials.size(); val++ )
        if( mat == materials[val] )
            break;
            
    if ( val == materials.size() )            
        materials.push_back( mat );
        
    return val;        
}

TS::Shape * ShapeBuilder::MakeShape()
{
   TS::Shape * pShape = new TS::Shape;
   pShape->fNames = names;
   pShape->fTransforms = transforms;
   pShape->fFrameTriggers = frameTriggers;

   for( VectorPtr<ShapeBuilder::Node*>::const_iterator ni = nodes.begin();
               ni != nodes.end(); ni++ )
      pShape->fNodes.push_back( (*ni)->snode );

   pShape->fnDefaultMaterials = materials.size();
   pShape->fAlwaysNode = alwaysNode; 

   for( VectorPtr<ShapeBuilder::Sequence*>::const_iterator si = sequences.begin();
       si != sequences.end(); si++ )
   {
      TS::Shape::Sequence seq;
      seq.fName = (*si)->fName;
      seq.fDuration = ((*si)->fEndTime - (*si)->fStartTime) / 4800.0f;
      seq.fCyclic = (*si)->fCyclic;
      seq.fPriority = (*si)->fPriority;
      seq.fNumFrameTriggers = (*si)->fNumFrameTriggers;
      seq.fFirstFrameTrigger = (*si)->fFirstFrameTrigger;
      seq.fNumIFLSubSequences = 0;


      IFL_FILE *fil = ifl_files;

      for ( int i = 0; i < num_ifl_files; i++, fil++ )
      {
         float time = 0;
         float dur = (*si)->fEndTime - (*si)->fStartTime;
         int mi = fil->mat_index;
         TS::Material mat;
         int first_key = 0;
         int key_count = 0;
         IFL_INFO    *last_inf = fil->info;
         int val;
         int start_added = FALSE;
          
         while ( time < (*si)->fEndTime )
         {
            IFL_INFO *inf = fil->info;

            for ( int j = 0; j < fil->info_count; j++, inf++ )
            {
               if ( time > (*si)->fStartTime  &&  time < (*si)->fEndTime )
               {
                  if ( !start_added )
                  {
                     val = consider_mat( mi, last_inf );
                     TS::Shape::Keyframe start_key( 0.0f, val, mi );
                     first_key = keyframes.size();
                     keyframes.push_back( start_key );
                     key_count++;
                     start_added = TRUE;
                  }
                  val = consider_mat( mi, inf );
                  float pos = (time - (*si)->fStartTime) / dur;
                  TS::Shape::Keyframe k( pos, val, mi );
                  keyframes.push_back( k );
                  key_count++;
               }
               else 
                  last_inf = inf;
               
               time += ((inf->hold_cnt) * 160);
            }                    
         }
             
         if ( start_added )
         {
            TS::Shape::SubSequence ss;
            ss.fSequenceIndex = pShape->fSequences.size();
            ss.fFirstKeyframe = first_key;
            ss.fnKeyframes = key_count;
            if( !seq.fNumIFLSubSequences++ )
               seq.fFirstIFLSubSequence = subsequences.size();
            subsequences.push_back( ss );
         }
      }
     
      pShape->fSequences.push_back( seq );
   }

   if( materials.size() )
   {
      materialList.setSize( materials.size(), 1 );
      for( int m = 0; m < materials.size(); m++ )
         materialList.setMaterial( m, materials[m] );
      pShape->setMaterialList( &materialList );
   }
   
   pShape->fSubSequences = subsequences;
   pShape->fKeyframes = keyframes;
     
   VectorPtr<ShapeBuilder::Transition*>::const_iterator ti;
  
   for ( ti = transitions.begin(); ti != transitions.end(); ti++ )
   {
      TS::Shape::Transition trans;
      trans.fDuration = (*ti)->duration;
      trans.fTransform.identity();
      trans.fTransform.setTranslate( (*ti)->gnd_shift );
     
      VectorPtr<ShapeBuilder::Sequence*>::const_iterator si;
      int cnt = 0;
      int found_begin = 0;
      int found_end = 0;
     
      for ( si = sequences.begin(); si != sequences.end(); si++, cnt++ )
      {
         if ( (*ti)->fStartTime >= (*si)->fStartTime && (*ti)->fStartTime <= (*si)->fEndTime )
         {
            trans.fStartSequence = cnt;
            trans.fStartPosition = float((*ti)->fStartTime - (*si)->fStartTime) / 
                                   ((*si)->fEndTime - (*si)->fStartTime);
            AssertMessage( !found_begin, "found 2 transition begin seqs." );
            found_begin = 1;
         }
         else if( (*ti)->fEndTime >= (*si)->fStartTime && (*ti)->fEndTime <= (*si)->fEndTime )
         {
            trans.fEndSequence = cnt;
            trans.fEndPosition = float((*ti)->fEndTime - (*si)->fStartTime) / 
                                    ((*si)->fEndTime - (*si)->fStartTime);
            AssertMessage( !found_end, "found 2 transition end seqs." );
            found_end = 1;
         }
      }
     
      AssertMessage( found_begin, "Couldn't find transition begin seq." );
      AssertMessage( found_end, "Couldn't find transition end seq." );
      pShape->fTransitions.push_back( trans );
   }

   // Here's the meaning of the sortPass:  we need to put meshes containing Translucent
   //    at the END of the list (has to do with them turning off hazing when they're 
   //    displayed).  So here's the order we want to put everything in:
   // 
   //       Meshes with NO translucent faces.
   //       Meshes with SOME (but not all).  
   //       Meshes with ALL translucent faces.
   // 
   // We just sum up a couple of conditions to see if a given item should be added on 
   //    a given pass.  
   //
   // 12/1/98: This code didn't have the intended results - so it's left out - the 
   //    sortPassKey() always returns 0.  See LAST_98_ADDITION #define and related code.  
   //
   for( int sortPass = 0; sortPass < 3; sortPass++ )
   {
      VectorPtr<MeshBuilder*>::const_iterator   mi;
      
      for( mi = meshes.begin(); mi != meshes.end(); mi++ )
      {
         if( (* mi)->sortPassKey() == sortPass )
         {
            TS::Shape::Object obj;
           
            (*mi)->SetupObject( &obj );
            obj.fMeshIndex = pShape->fMeshes.size();
            pShape->fObjects.push_back( obj );
            
            const char *name = names[ (*mi)->getName() ];
            sDump( fp, "mesh name = %s: (%s)\n", name, 
                        sortPass==0 ?  "NO translucent" :
                           (sortPass==1 ? "SOME translucent" : "ALL xlucent")
                  );

            TS::Shape::Mesh *pMesh = (*mi)->MakeMesh(fp);
            pShape->fMeshes.push_back( pMesh );
         }
      }
   }
  
   pShape->fDetails = details;
   pShape->fCenter = fCenter;
   pShape->fRadius = fRadius;
   pShape->fBounds = fBounds;

   return pShape;
}

//--------------------------------------------------------------

void ShapeBuilder::AddDetail( TS::Shape::Detail const & det )
    {
    // sort with largest details first:
    for( int d = 0; d < details.size(); d++ )
        if( details[d].fSize < det.fSize )
            {
            details.insert( d );
            details[d] = det;
            return;
            }
    details.push_back( det );
    }

//--------------------------------------------------------------


//
// Just want to dump the state of all the vectors at this point.  
//
void ShapeBuilder::dumpStatistics(const char * txt)
{
#ifdef SDUMP
   static const char line[] = 
      "======================================"
      "===================================\n";

   fprintf( fp, line );
   fprintf( fp, "Current Vector sizes:\n" );
   if( txt != NULL )
      fprintf( fp, txt );
   fprintf( fp, line );
   
   fprintf( fp, "--> # of sequences:      %d\n",   sequences.size() );
   fprintf( fp, "--> # of transitions:    %d\n",   transitions.size() );
   fprintf( fp, "--> # of subsequences:   %d\n",   subsequences.size() );
   fprintf( fp, "--> # of nodes:          %d\n",   nodes.size() );
   fprintf( fp, "--> # of transforms:     %d\n",   transforms.size() );
   fprintf( fp, "--> # of keyframes:      %d\n",   keyframes.size() );
   fprintf( fp, "--> # of names:          %d\n",   names.size() );
   fprintf( fp, "--> # of meshes:         %d\n",   meshes.size() );
   fprintf( fp, "--> # of materials:      %d\n",   materials.size() );
   fprintf( fp, "--> # of details:        %d\n",   details.size() );
   fprintf( fp, "--> # of frameTriggers:  %d\n",   frameTriggers.size() );
   
   fprintf( fp, line );
   
#else
   txt;
#endif
}



// Take a snapshot of the statistics at the current moment.  
void ShapeBuilder::captureStats( Statistics & stats )
{
#ifdef SDUMP
   stats.num_sequences = sequences.size();
   stats.num_transitions = transitions.size();
   stats.num_subsequences = subsequences.size();
   stats.num_nodes = nodes.size();
   stats.num_transforms = transforms.size();
   stats.num_keyframes = keyframes.size();
   stats.num_names = names.size();
   stats.num_meshes = meshes.size();
   stats.num_materials = materials.size();
   stats.num_details = details.size();
   stats.num_frameTriggers = frameTriggers.size();
#else
   stats;
#endif
}


void ShapeBuilder::getStatsDifference( Statistics & stats ) 
{
#ifdef SDUMP
   Statistics  currentStats;
   captureStats( currentStats );
   
   currentStats.num_sequences     -=   stats.num_sequences;
   currentStats.num_transitions   -=   stats.num_transitions;
   currentStats.num_subsequences  -=   stats.num_subsequences;
   currentStats.num_nodes         -=   stats.num_nodes;
   currentStats.num_transforms    -=   stats.num_transforms;
   currentStats.num_keyframes     -=   stats.num_keyframes;
   currentStats.num_names         -=   stats.num_names;
   currentStats.num_meshes        -=   stats.num_meshes;
   currentStats.num_materials     -=   stats.num_materials;
   currentStats.num_details       -=   stats.num_details;
   currentStats.num_frameTriggers -=   stats.num_frameTriggers;
   
   stats = currentStats;
#else
   stats;
#endif
}

void ShapeBuilder::dumpStats ( const Statistics & stats, const char * txt ) 
{
#ifdef SDUMP
   static const char line[] = 
      "======================================"
      "===================================\n";

   fprintf( fp, line );
   fprintf( fp, "Current Vector sizes:\n" );
   if( txt != NULL )
      fprintf( fp, txt );
   fprintf( fp, line );
   
   fprintf( fp, "--> # of sequences:      %d\n",   stats.num_sequences );
   fprintf( fp, "--> # of transitions:    %d\n",   stats.num_transitions );
   fprintf( fp, "--> # of subsequences:   %d\n",   stats.num_subsequences );
   fprintf( fp, "--> # of nodes:          %d\n",   stats.num_nodes );
   fprintf( fp, "--> # of transforms:     %d\n",   stats.num_transforms );
   fprintf( fp, "--> # of keyframes:      %d\n",   stats.num_keyframes );
   fprintf( fp, "--> # of names:          %d\n",   stats.num_names );
   fprintf( fp, "--> # of meshes:         %d\n",   stats.num_meshes );
   fprintf( fp, "--> # of materials:      %d\n",   stats.num_materials );
   fprintf( fp, "--> # of details:        %d\n",   stats.num_details );
   fprintf( fp, "--> # of frameTriggers:  %d\n",   stats.num_frameTriggers );
   
   fprintf( fp, line );
   
#else
   stats;
   txt;
#endif
}



//--------------------------------------------------------------
