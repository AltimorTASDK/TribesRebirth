#define INITGUID
#include "objbase.h"
#include <ddraw.h>
#include "missionlighting.h"

CMain Main;

// the version has been reset to 1.00 for public release on 1/08/98
#ifdef FEAR
const char * const versionString = "( Tribes - 1.01 [" __DATE__ " - "__TIME__ "] )";
#else  // STARSIEGE
const char * const versionString = "( Starsiege - 1.01 [" __DATE__ " - "__TIME__ "] )";
#endif

#ifdef FEAR

// some globals required
WorldGlobals cg;
WorldGlobals sg;
WorldGlobals * wg;

//------------------------------------------------------------------------------

class ResourceTypeMissionScript: public ResourceType
{
  public:   
   ResourceTypeMissionScript(const char *ext = ".mis") :
	ResourceType( ResourceType::typeof(ext) )  { }
   void* construct(StreamIO *stream,int);
   void destruct(void *p);
};

ResourceTypeMissionScript missionLightingScript( ".mis" );

//------------------------------------------------------------------------------

void * ResourceTypeMissionScript::construct(StreamIO *stream, int size)
{
   static bool loadInstantOnly = true;
   
   // defined in FearDynamicDataPlugin.cpp
   const char * header = "//--- export object begin ---//\r\n";
   const char * footer = "//--- export object end ---//\r\n";
   
   ConsoleScript * ret = new ConsoleScript;

   // grab the data and zero terminate it
   BYTE * consoleData = new BYTE[ size + 1 ];
   stream->read( size, consoleData );
   consoleData[ size ] = 0;

   // load only the instantiaded stuff?
   if( loadInstantOnly )
   {
      // get to the begin'n of the instantiated info   
      BYTE * headerStart = consoleData;
      while( *headerStart && strncmp( headerStart, header, strlen( header ) ) )   
         headerStart++;

      // get to the end
      BYTE * footerEnd = headerStart;
      while( *footerEnd && strncmp( footerEnd, footer, strlen( footer ) ) )
         footerEnd++;
         
      // check for a bad script...
      AssertFatal( headerStart && *headerStart, "Invalid mission script." );

      // get the size of the data and terminate the buffer
      int instantSize = footerEnd - headerStart + 1;
      *footerEnd = '\0';

      // need to reallocate a buffer and copy it in...
      ret->data = new BYTE[ instantSize ];
      memcpy( ret->data, headerStart, instantSize );
      delete [] consoleData;
      ret->size = instantSize;
   
      loadInstantOnly = false;
   }
   else
   {
      ret->data = consoleData;
      ret->size = size;
   }
      
   return( ( void * )ret );
}

//------------------------------------------------------------------------------

void ResourceTypeMissionScript::destruct(void * p)
{
   delete ( ConsoleScript * )p;
}
#else  // STARSIEGE
bool fCDIsBuddy = false;  // i hate you cd!
DWORD g_dwCDDrive = 0;
void initializeServer(){}   
void forceRenderSingleFrame(){}
void declarePreferenceVariables(){}   
#endif

//------------------------------------------------------------------------------

CMain::CMain() :
   missionLighting( NULL )
{
#ifdef FEAR
   // set the default globals to  the servers
   wg = &sg;
#endif
}

//------------------------------------------------------------------------------

bool CMain::initInstance()
{
   const char * missionName = NULL;
   const char * cloudMap    = NULL;
   bool error = false;
   
   MissionLighting::VERBOSE verbose = MissionLighting::V_LOWEST;
   
   // parse the command line
   for( int i = 0; i < processedCmdLine.size() && !error; i++ )
   {
      // check if this is a switch
      if( processedCmdLine[i][0] == '-' )
      {
         // check the command
         switch( processedCmdLine[i][1] )
         {
            case 'v':
            case 'V':
               if( isdigit( processedCmdLine[i][2] ) )
                  verbose = ( MissionLighting::VERBOSE )( processedCmdLine[i][2] - '0' );
               break;
            case 'q':
            case 'Q':
            {
               // go through the options
               if( processedCmdLine[i][2] == '\0' )
               {
                  MissionLighting::missionTerrainLOSCheck = false;
                  MissionLighting::missionInteriorLOSCheck = false;
               }
               else
               {
                  int j = 2;
                  while( processedCmdLine[i][j] != '\0' )
                  {
                     switch( processedCmdLine[i][j] )
                     {  
                        case 't':
                        case 'T':
                           MissionLighting::missionTerrainLOSCheck = false;
                           break;
                        case 'i':
                        case 'I':
                           MissionLighting::missionInteriorLOSCheck = false;
                           break;
                        default:
                           break;
                     }
                     j++;
                  }
               }
               break;
            }
            case 'n':
            case 'N':
               MissionLighting::missionLightTerrain = false;
               break;
            case 'x':
            case 'X':
               MissionLighting::missionLightInteriors = false;
               break;
            case 'l':
            case 'L':
               MissionLighting::missionLightLockOnly = true;
               break;

#ifdef DEBUG
            case 'a':
            case 'A':
               MissionLighting::missionConvertWrapLights = false;
               break;
            case 'i':
            case 'I':
               MissionLighting::missionInvertCloudMap = true;
               break;   
            case 'h':
            case 'H':
               MissionLighting::missionUseHiresTerrain = true;
               break;
            case 's':
            case 'S':
               MissionLighting::missionTerrainSubSample = true;
               break;
            case 'c':
            case 'C':
               MissionLighting::missionClearHiresLightmaps = false;
               break;
            case 'm':
            case 'M':
            
               // get the cloud map name
               if( i < ( processedCmdLine.size() + 3 ) )
               {
                  // grab the name
                  cloudMap = processedCmdLine[++i];

                  MissionLighting::cloudMapScale = atof( processedCmdLine[++i] );
                  float absorption = atof( processedCmdLine[++i] );

                  // check the value
                  if( absorption < 0.f || absorption > 1.0f )
                     error = true;
                     
                  MissionLighting::cloudMapAbsorption = absorption;
                  MissionLighting::missionUseCloudMap = true;
               }
                  
               break;
#endif
         }
      }
      else
         missionName = processedCmdLine[i];
   }

   // check if we have the required params
   if( !missionName || error )
   {
#ifdef DEBUG
      printf( "\n**********************************************\n");
      printf( "\n  DEBUG BUILD - NOT FOR PUBLIC DISTRIBUTION!\n");
      printf( "\n**********************************************\n");
#endif

      // show the usage of the utility
      printf( "\nMissionLighting %s\n", versionString );
      printf( "--------------------------------------------------\n" );
#ifdef DEBUG
      printf( "usage: <input>.mis [-v#] [-q[i|t] [-l] [-n] [-x] [-c] [-a] [-i] [-h] \n\t" 
         "[-s] [-d] [-m <clouds.bmp> <scale> <saturation>]\n" );
#else
      printf( "usage: <input>.mis [-v#] [-q[i|t] [-l] [-n] [-x]\n" );
#endif
      printf( "  v: verbose level ( 0 - 4 )\n" );
      printf( "  q[i|t]: quick lighting ( Interior | Terrain )\n" );
      printf( "  l: light only locked interiors\n" );
      printf( "  n: do not light terrain\n" );
      printf( "  x: do not light interiors\n" );

#ifdef DEBUG
      printf( "  a: dont treat wrap lights as pure directional lights\n" );
      printf( "  m: apply cloud map <clouds.bmp> <scale in ppm> <absorption [0-1]>\n" );
      printf( "  i: invert cloud map colors\n" );
      printf( "  h: hires light terrain\n" );
      printf( "  s: subsample terrain lightmaps\n" );
      printf( "  c: do not clear hires lightmaps\n" );
#endif
      return( true );
   }

   // do the stuff   
   missionLighting = new MissionLighting( verbose, missionName );
   if( MissionLighting::missionUseCloudMap )
      missionLighting->loadCloudMap( cloudMap );
   missionLighting->loadBase();
   missionLighting->loadMission();
   if(MissionLighting::missionLightInteriors && (missionLighting->m_interiors.size() != 0))
   {
      missionLighting->createInteriorVolume();
      missionLighting->processInteriors();
   }
   if( MissionLighting::missionLightTerrain )
      missionLighting->processTerrains();
   missionLighting->saveMission();
   
   return( true );   
}

//------------------------------------------------------------------------------

void CMain::onExit()
{
   delete missionLighting;
}

/////////////////////////////////////////////////////////////////
// mission lighting code below
/////////////////////////////////////////////////////////////////

const char * const MissionLighting::lightVolName = "MissionLighting";
bool MissionLighting::missionLightTerrain = true;
bool MissionLighting::missionConvertWrapLights = true;
bool MissionLighting::missionLightInteriors = true;
bool MissionLighting::missionTerrainLOSCheck = true;
bool MissionLighting::missionInteriorLOSCheck = true;
bool MissionLighting::missionLightLockOnly = false;
bool MissionLighting::missionUseCloudMap = false;
bool MissionLighting::missionInvertCloudMap = false;
bool MissionLighting::missionUseHiresTerrain = false;
bool MissionLighting::missionTerrainSubSample = false;
bool MissionLighting::missionClearHiresLightmaps = true;

float MissionLighting::cloudMapScale = 0.f;
float MissionLighting::cloudMapAbsorption = 1.0f;

//------------------------------------------------------------------------------

int fpuStackExceptions = 0;
void __cdecl Catcher( int )
{
   fpuStackExceptions++;
   signal( SIGFPE, Catcher );
}

//------------------------------------------------------------------------------
// initialze stuff

MissionLighting::MissionLighting( VERBOSE verbose, const char * missionName )
{
   // install a signal handler for floating point stack ****flow
   signal( SIGFPE, Catcher );
   
   // set the defaults
   game = NULL;
   manager = NULL;
   mission = NULL;
   verboseLevel = V_LOWEST;
   m_currentInterior = 0;
   useSurfaceInfo = false;

   AssertFatal( missionName, "No missionName!" );
   
   // set the verbose level
   verboseLevel = ( verbose > V_LOWEST ) ? V_LOWEST : verbose;
   print( V_LOW, " - Verbose level = %d\n", verboseLevel );
   print( V_LOW, " - Interior LOS check: %s\n", missionInteriorLOSCheck ? "True" : "False" );
   print( V_LOW, " - Terrain LOS check: %s\n", missionTerrainLOSCheck ? "True" : "False" );
   print( V_LOW, " - Light terrain: %s\n", missionLightTerrain ? "True" : "False" );
   print( V_LOW, " - Light interiors: %s\n", missionLightInteriors ? "True" : "False" );
   print( V_LOW, " - Light locked interiors only: %s\n", missionLightLockOnly ? "True" : "False" );

#ifdef DEBUG
   print( V_LOW, " - Convert wrap lights: %s\n", missionConvertWrapLights ? "True" : "False" );
   print( V_LOW, " - Use cloud lightmap: %s\n", missionUseCloudMap ? "True" : "False" );
   print( V_LOW, " - Invert cloud lightmap: %s\n", missionInvertCloudMap ? "True" : "False" );
   print( V_LOW, " - Hires light terrain: %s\n", missionUseHiresTerrain ? "True" : "False" );
   print( V_LOW, " - Subsample terrain lightmaps: %s\n", missionTerrainSubSample ? "True" : "False" );
   print( V_LOW, " - Clear hires lightmaps: %s\n", missionClearHiresLightmaps ? "True" : "False" );
#endif
   
   // get the mission name
   missionFile = missionName;
   
   // create the volume name   
   volumeFile = missionFile;
   reextendFile( volumeFile, ".vol" );
   
   // remove the file now
   if( missionLightInteriors )
      DeleteFile( volumeFile.c_str() );
   
   // create a game
   game = new SimGame;
   game->init();
   
   // get a manager
   SimWorld * server = new SimWorld;
   
   game->setWorld( SimGame::SERVER, server );
   
   // get focus and setup the search path
   game->getWorld( SimGame::SERVER );
   game->focus( SimGame::SERVER );
   
   // grab the manager
   manager = game->getManager( SimGame::SERVER );
   _assert( manager, "Could not get SimManager object!" );
   
   // grab the resource manager
   resManager = SimResource::get( manager );
   _assert( resManager, "Could not get resource manager object!" );
   
   sg.resManager = resManager;
}

//------------------------------------------------------------------------------

MissionLighting::~MissionLighting()
{
   delete game;
}

//------------------------------------------------------------------------------
// load up the base stuff ( data.vol, ... - and inventory/... data )

#include "fearplugin.h"
#include "simwinconsoleplugin.h"
#include "scriptplugin.h"

void MissionLighting::loadBase()
{
   print( V_MEDIUM, "Loading base volumes..." );

#ifdef FEAR
	// Install all the plugins that we need 
   game->registerPlugin(new FearPlugin);
   game->registerPlugin(new SimWinConsolePlugin);
   game->registerPlugin(new FearDynamicDataPlugin);
   game->registerPlugin(new ScriptPlugin);
#endif

   CMDConsole * con = CMDConsole::getLocked();
   con->setVariable("$ConsoleWorld::DefaultSearchPath", ".");
   con->evaluate( "exec( \"missionlighting.cs\" );\n", false );

   print( V_MEDIUM, "done.\n" );
}

//------------------------------------------------------------------------------
// load up the mission

void MissionLighting::loadMission()
{
   print( V_MEDIUM, "Loading mission ( %s )...", missionFile );
   
   Persistent::Base::Error err;
   Persistent::Base * base = Persistent::Base::fileLoad( missionFile.c_str(), &err );
   
#ifdef FEAR
   // check if should attempt to load as a script...
   if( !base || !( err == Persistent::Base::Ok ) )
   {
      CMDConsole * con = CMDConsole::getLocked();
      con->evaluate( "setInstantGroup(0);" );
      con->executef( 2, "exec", missionFile.c_str() );
   }

   // check that it loaded up
   mission = (SimGroup *)manager->findObject( "MissionGroup" );
   
   _assert( mission, "Invalid mission file: %s\n", missionFile );
#else // STARSIEGE

   _assert( base && ( err == Persistent::Base::Ok ), "Failed to open mission file: %s\n", 
   missionFile );
   
   mission = dynamic_cast<SimGroup *>( base );
   _assert( mission, "Invalid mission file: %s\n", missionFile );
      
   manager->addObject( mission, "MissionGroup" );
#endif

   print( V_MEDIUM, "done.\n" );
   
   // get all the interiors and lights
   print( V_MEDIUM, "Grabbing mission lights, interiors and volumes\n" );
   getInteriors( mission );
   getLights( mission );
   getVolumes( mission );
   getTerrains( mission );
   
   print( V_LOW, "Found %d interiors.\n", m_interiors.size() );
   print( V_LOW, "Found %d lights.\n", m_lights.size() );
   print( V_LOW, "Found %d volumes.\n", m_volumes.size() );
   print( V_LOW, "Found %d terrains.\n", m_terrains.size() );
}

//------------------------------------------------------------------------------

void MissionLighting::loadCloudMap( const char * file )
{
   cloudMapFile = file;

   print( V_LOW, "Loading cloud map: %s\n", cloudMapFile.c_str() );
   
   cloudMap = GFXBitmap::load( cloudMapFile.c_str() );

   _assert( cloudMap, "Failed to load cloud lightmap file: %s\n", cloudMapFile );

   // check the bitdepth
   if( cloudMap->getBitDepth() != 24 )
      _assert( 0, "Cloud Bitmap must be a 24bit image" );
}

//------------------------------------------------------------------------------
      
void MissionLighting::saveMission()
{
   AssertFatal( mission, "Mission not found, cannot save." );

#ifdef FEAR
   Console->executef( 3, "exportObjectToScript", "MissionGroup", missionFile.c_str());
#else // STARSIEGE
   Persistent::Base::openDictionary();
   Persistent::Base::Error err = mission->fileStore( missionFile.c_str() );
   Persistent::Base::closeDictionary();

   _assert( err == Persistent::Base::Ok, "Failed to save mission: %s\n", missionFile.c_str() );
#endif
}

// reextend a file with a new extension
void MissionLighting::reextendFile( String & file, const char * ext )
{
   char * str = new char[ strlen( file.c_str() ) + strlen( ext ) + 1 ];
   strcpy( str, file.c_str() );
   
   char * end = str + strlen( str );
   
   while( ( end >= str ) && ( *end != '.' ) )
      end--;
      
   AssertFatal( end >= str, "Invalid filename" );
      
   // tack on the extension
   *end = '\0';
   strcat( str, ext );
   file = str;
   
   delete [] str;
}

//------------------------------------------------------------------------------
// add a file to a volume and remove the file

void MissionLighting::dumpFileToVolumeAndRemove( const String& in_outVolName, const String& in_fileName )
{
   // Do the dump...
   //
   char volName[ 256 ];
   sprintf( volName, "\"%s\"", in_outVolName.c_str() );
   
   char fileName[ 256 ];
   sprintf( fileName, "\"%s\"", in_fileName.c_str() );
   
   char* argv[5];
   argv[0] = "vt.exe";
   argv[1] = "-sp";
   argv[2] = const_cast<char*>(volName);
   argv[3] = const_cast<char*>(fileName);
   argv[4] = NULL;

   spawnvp(P_WAIT, "vt.exe", argv);
   DeleteFile(in_fileName.c_str());
}

//------------------------------------------------------------------------------

const char * MissionLighting::stripPath( const char * file )
{
   AssertFatal( file, "Invalid filename." );
   
   char * end = const_cast< char *>( file + strlen( file ) );
   while( end != file )
   {
      if( *end == '\\' || *end == '/' || *end == ':' )
      {
         end++;
         break;
      }
      end--;
   }
   return( end );
}

//------------------------------------------------------------------------------
// create the volume that will be used with this mission

void MissionLighting::createInteriorVolume()
{
   VolumeRWStream vol;

   // create the volume
   _assert( vol.createVolume( volumeFile.c_str() ), "Failed to create volume: %s\n",
      volumeFile.c_str() );
      
   SimGroup * group = NULL;
   
   // go through and see if the volume has been added alredy
   for( int i = 0; i < m_volumes.size(); i++ )
   {
      if( ( m_volumes[i]->getName() ) && ( !stricmp( m_volumes[i]->getName(), lightVolName ) ) )
      {
         // remove the volume
         print( V_LOW, "Removing MissionLighting volume.\n" );
         group = m_volumes[i]->getGroup();
         if( group )
            group->removeObject( m_volumes[i] );
      }
   }
   
   print( V_MEDIUM, "Adding SimVolume to mission ( %s )\n",
      volumeFile.c_str() );

   // add to the mission in the volumes group
   SimVolume * simVolume = new SimVolume;
   manager->addObject( simVolume, lightVolName );
   simVolume->open( stripPath( volumeFile.c_str() ) );
   
   // try and place in a volumes group
   if( !group )
   {
      group = dynamic_cast<SimGroup*>( mission->findObject( "Volumes" ) );
      if( !group )
         group = mission;
   }
   group->addObject( simVolume, lightVolName );
}

//------------------------------------------------------------------------------
// go through the mission and grab the interior's

void MissionLighting::getInteriors( SimSet * set )
{
   AssertFatal( set, "Invalid SimSet object!" );
   
   // go through this set
   for( SimSet::iterator itr = set->begin(); itr != set->end(); itr++ )
   {
      // check if need to recurse into
      SimSet * ss = dynamic_cast<SimSet *>( *itr );
      if( ss )
         getInteriors( ss );
         
      // check if an interior type object
      SimInteriorShape * shape = dynamic_cast< SimInteriorShape * >( *itr );
      SimInterior * interior = dynamic_cast< SimInterior * >( *itr );

#ifdef FEAR
      InteriorShape * ishape = dynamic_cast< InteriorShape * >( *itr );
      if( shape || interior || ishape)
#else // STARSIEGE
      if( shape || interior )
#endif
      {
         // add to interior list
         SimContainer * container = dynamic_cast< SimContainer * >( *itr );
         AssertFatal( container, "Invalid interior object." );

         // grab the name
         String nameA, nameB;
         getInteriorBaseFilename( container, nameA );

         int instance = 0;
                  
         // go through and increment the instance if need be
         for( int i = 0; i <m_interiors.size(); i++ )
         {
            getInteriorBaseFilename( m_interiors[i], nameB );
            if( nameA == nameB )
               instance++;
         }
         
         m_interiors.push_back( container );
         interior_instance.push_back( instance );
         
         print( V_LOWEST, " - interior: %s:%d\n", nameA.c_str(), instance );
      }                       
   }
}

//------------------------------------------------------------------------------

void MissionLighting::getLights( SimSet * set )
{
   AssertFatal( set, "Invalid SimSet object!" );
   
   // walk the tree
   for( SimSet::iterator itr = set->begin(); itr != set->end(); itr++ )
   {
      SimSet * ss = dynamic_cast<SimSet *>( *itr );
      if( ss )
         getLights( ss );
         
      SimLightQuery query;
      
      // check if a light type object
      if( (*itr)->processQuery( &query ) )
      {
         m_lights.push_back( (*itr) );
         print( V_LOWEST, " - light: %s\n", (*itr)->getName() );
      }
   }
}

//------------------------------------------------------------------------------
// grab all the terrains ( might have more than one someday... )

void MissionLighting::getTerrains( SimSet * set )
{
   AssertFatal( set, "Invalid SimSet object!" );
   
   // go through it
   for( SimSet::iterator itr = set->begin(); itr != set->end(); itr++ )
   {
      SimSet * ss = dynamic_cast<SimSet *>( *itr );
      if( ss )
         getTerrains( ss );
      
      // check if a terrain
      SimTerrain * terrain = dynamic_cast< SimTerrain * >( *itr );
      if( terrain )
      {
         m_terrains.push_back( terrain );
         print( V_LOWEST, " - terrain: %s\n", terrain->getName() );
      }
   }
}

//------------------------------------------------------------------------------
// grab all the volumes to load

void MissionLighting::getVolumes( SimSet * set )
{
   AssertFatal( set, "Invalid SimSet object!" );
   
   // walk it
   for( SimSet::iterator itr = set->begin(); itr != set->end(); itr++ )
   {
      SimSet * ss = dynamic_cast<SimSet * >( *itr );
      if( ss )
         getVolumes( ss );
         
      // check if a simvolume
      SimVolume * vol = dynamic_cast< SimVolume * >( *itr );
      if( vol )
      {
         m_volumes.push_back( vol );
         print( V_LOWEST, " - volume: %s\n", vol->getName() );
      }
   }
}

//------------------------------------------------------------------------------
// callback function for itrbasiclight, the points are in interior space, convert to world 
// and test for a collision

bool MissionLighting::collide( void * obj, Point3F & start, Point3F & end, ColorF & col )
{
   col;
   MissionLighting * lighter = ( MissionLighting * )obj;
   bool retVal = false;
   Point3F a, b;
   
   // chekc if lighting the terrain or the interiors ( terrain already did the los check )
   if( !lighter->lightingTerrain )
   {
      // translate into world space
      SimContainer * interior = lighter->m_interiors[ lighter->m_currentInterior ];
   
      TMat3F mat = lighter->getInteriorTransform( *interior );
      m_mul( start, mat, &a );
      m_mul( end, mat, &b );
   
      // setup for the call to the los function
      SimCollisionInfo info;
      SimContainerQuery query;
   
      query.id = interior->getId();
      query.type = -1;
      query.box.fMin = a;
      query.box.fMax = b;
#ifdef FEAR
      query.mask = ( SimInteriorObjectType | SimTerrainObjectType );
#else  // StarSiege
      query.mask = ( StaticInteriorObjectType | SimTerrainObjectType );
#endif
   
      // do the collision check
      SimContainer * root = findObject( lighter->manager, SimRootContainerId, root );
      retVal = root->findLOS( query, &info );
   }
   
   // do the cloud map stuff...
   if( missionUseCloudMap )
      lighter->applyCloudMapIntensity( lighter->lightingTerrain ? end : b, col );
   
   return( retVal );
}

//------------------------------------------------------------------------------

void MissionLighting::applyCloudMapIntensity( Point3F & pos, ColorF & col )
{
   // transform the point into cloudmap coords...
   Point2I bmpPos;
   bmpPos.x = ( int )( pos.x * cloudMapScale );
   bmpPos.y = ( int )( pos.y * cloudMapScale );
   
   // make sure on the bmp
   bmpPos.x = bmpPos.x % ( cloudMap->getWidth() - 1 );
   bmpPos.y = bmpPos.y % ( cloudMap->getHeight() - 1 );

   if( bmpPos.x < 0 ) 
      bmpPos.x += ( cloudMap->getWidth() - 1 );
   if( bmpPos.y < 0 )
      bmpPos.y += ( cloudMap->getHeight() - 1 );
   
   // flip the x
   bmpPos.x = ( cloudMap->getWidth() - 1 ) - bmpPos.x;
   
   // get to the info
   BYTE * pbmp = cloudMap->getAddress( &bmpPos );
   AssertFatal( pbmp, "Failed to get bitmap bits" );
   
   // get the bitmap color for the current pixel
   ColorF bmpColor;
   
   // bmp colors go blue-green-red
   bmpColor.blue = ( ( float )( *pbmp++ ) / 0xff );
   bmpColor.green = ( ( float )( *pbmp++ ) / 0xff );
   bmpColor.red = ( ( float )( *pbmp ) / 0xff );

   // check for inversion   
   if( missionInvertCloudMap )
   {
      bmpColor.blue = 1.0f - bmpColor.blue;
      bmpColor.green = 1.0f - bmpColor.green;
      bmpColor.red = 1.0f - bmpColor.red;
   }

   // calculate the colr
   col.red = ( ( 1.0f - cloudMapAbsorption ) * col.red ) + ( cloudMapAbsorption * bmpColor.red );
   col.green = ( ( 1.0f - cloudMapAbsorption ) * col.green ) + ( cloudMapAbsorption * bmpColor.green );
   col.blue = ( ( 1.0f - cloudMapAbsorption ) * col.blue ) + ( cloudMapAbsorption * bmpColor.blue );
}

//------------------------------------------------------------------------------

void MissionLighting::addLight( ITRBasicLighting::LightList & lightList,
   SimContainer & interior, TSLight & light, ColorF & ambient )
{
   ITRBasicLighting::Light * basicLight = new ITRBasicLighting::Light;

   // fill in the light info
   basicLight->id = 0xffff0000 + lightList.size() + 1;
   basicLight->name = NULL;
   basicLight->flags = 0;
   
   ITRBasicLighting::Light::State * state = new ITRBasicLighting::Light::State;
   
   // fill in the default state info
   state->stateDuration = 0.f;
   state->color.red = light.fLight.fRed;
   state->color.green = light.fLight.fGreen;
   state->color.blue = light.fLight.fBlue;

   ITRBasicLighting::Light::State::Emitter emitter;

   // set the ambient intensity
   ITRBasicLighting::m_ambientIntensity += ambient;
   
   // determine the light type
   switch( light.fLight.fType )
   {
      case TS::Light::LightPoint:
      {
         emitter.lightType = ITRBasicLighting::Light::State::PointLight;
         
         TMat3F mat = getInteriorTransform( interior );
         mat.inverse();

         m_mul( light.fLight.fWPosition, mat, &emitter.pos );
         
         // setup the falloff params ( no linear falloff )
         emitter.falloff = ITRBasicLighting::Light::State::Linear;
         emitter.d1 = emitter.d2 = emitter.radius = light.fLight.fRange;
         
         break;
      }
         
      // check for treating wrap lights like directional
      case TS::Light::LightDirectionalWrap:
      {
         if( !missionConvertWrapLights )
         {
            delete basicLight;
            delete state;
            return;
         }
      }
            
      case TS::Light::LightDirectional:
      {
         emitter.lightType = ITRBasicLighting::Light::State::DirectionalLight;

         RMat3F mat = getInteriorTransform( interior );
         mat.inverse();
                  
         m_mul( light.fLight.fWAim, mat, &emitter.spotVector );
         
         break;
      }
         
      default:
         
         // unsupported light type
         delete basicLight;
         delete state;
         return;
   }

   // add to the lists
   lightList.push_back( basicLight );
   basicLight->state.push_back( state );
   state->m_emitterList.push_back( emitter );
}

//------------------------------------------------------------------------------

void MissionLighting::processLights( ITRBasicLighting::LightList & lightList,
   SimContainer & interior )
{
   // reset the ambient intensity
   ITRBasicLighting::m_ambientIntensity.set( 0.f, 0.f, 0.f );

   // walk the lights
   for( int i = 0; i < m_lights.size(); i++ )
   {
      SimLightQuery query;

      // grab the light info
      if( ( m_lights[i]->processQuery( &query ) ) )
      {
         // go through the lights associated with this object
         for( int j = 0; j < query.count; j++ )
            addLight( lightList, interior, *query.light[j], query.ambientIntensity );
      }
   }      
}

//------------------------------------------------------------------------------

const TMat3F & MissionLighting::getInteriorTransform( SimContainer & obj )
{
#ifdef FEAR
   InteriorShape * ishape = dynamic_cast< InteriorShape * >( &obj );
   if( ishape )
      return( ishape->getTransform() );
#endif

   SimInteriorShape * shape = dynamic_cast< SimInteriorShape * >( &obj );
   if( shape )
      return( shape->getTransform() );
   
   SimInterior * interior = dynamic_cast< SimInterior * >( &obj );
   if( interior )
      return( interior->getTransform() );
   AssertFatal( interior, "Invalid interior object." );
   static TMat3F tmp;
   return( tmp );
}

//------------------------------------------------------------------------------

ITRInstance * MissionLighting::getInteriorInstance( SimContainer * obj )
{
#ifdef FEAR
   InteriorShape * ishape = dynamic_cast< InteriorShape * >( obj );
   if( ishape )
      return( ishape->getInstance() );
#endif
   
   SimInteriorShape * shape = dynamic_cast< SimInteriorShape * >( obj );
   if( shape )
      return( shape->getInstance() );
   
   SimInterior * interior = dynamic_cast< SimInterior * >( obj );
   if( interior )
      return( interior->getInstance() );
   AssertFatal( interior, "Invalid interior object." );
   return( NULL );   
}

//------------------------------------------------------------------------------
               
void MissionLighting::getInteriorBaseFilename( SimContainer * obj, String & file )
{
#ifdef FEAR
   InteriorShape * ishape = dynamic_cast< InteriorShape * >( obj );
   if( ishape )
   {
      ishape->getBaseFilename( file );
      return;
   }
#endif
   
   SimInteriorShape * shape = dynamic_cast< SimInteriorShape * >( obj );
   if( shape )
   {
      shape->getBaseFilename( file );
      return;
   }

   SimInterior * interior = dynamic_cast< SimInterior * >( obj );
   if( interior )
   {
      interior->getBaseFilename( file );
      return;
   }
   AssertFatal( interior, "Invalid interior object. " );
}

//------------------------------------------------------------------------------

void MissionLighting::setInteriorFilename( SimContainer * obj, const char * name )
{
#ifdef FEAR
   InteriorShape * ishape = dynamic_cast< InteriorShape * >( obj );
   if( ishape )
   {
      ishape->setFileName( name );
      return;
   }
#endif
   
   SimInteriorShape * shape = dynamic_cast< SimInteriorShape * >( obj );
   if( shape )
   {
      shape->setFileName( name );
      return;
   }
   
   SimInterior * interior = dynamic_cast< SimInterior * >( obj );
   if( interior )
   {
      interior->setFilename( name );
      return;
   }
   AssertFatal( interior, "Invalid interior object." );
}
               
//------------------------------------------------------------------------------
// process the lights and objects

void MissionLighting::processInteriors()
{
   // walk though the interiors and light each one
   for( int i = 0; i < m_interiors.size(); i++ )
   {
      m_currentInterior = i;
      
      // check if skipping this one
      if( missionLightLockOnly && !m_interiors[i]->isLocked() )
      {
         String name;
         getInteriorBaseFilename( m_interiors[i], name );
         print( V_LOW, "Skipping unlocked interior ( %s )\n", name.c_str() );
         continue;
      }
      
      ITRInstance * instance = getInteriorInstance( m_interiors[i] );

      // check if the shape exists
      if( !instance || !instance->getShape() )
      {
         String name;
         getInteriorBaseFilename( m_interiors[i], name );
         print( V_HIGH, " *****************************************************\n" );
         print( V_HIGH, " ***** Failed to get interior: %s\n", name.c_str() );
         print( V_HIGH, " *****************************************************\n" );
         continue;
      }
      
      ITRShape & src = *( instance->getShape() );
      
      // test if an interior or not
      if( instance->isLinked() )
      {
         String name;
         getInteriorBaseFilename( m_interiors[i], name );
         print( V_MEDIUM, "Skipping linked interior ( %s )\n", name.c_str() );
         continue;
      }      
 
      // check for cloud mapping
      if( missionUseCloudMap )
         print( V_MEDIUM, "Using cloud map: %s\n", cloudMapFile.c_str() );
      
      // create our new shape
      ITRShape dest = src;
      dest.m_nameBuffer.clear();
      
      // go through all the states in this one
     for( UInt32 j = 0; j < src.getNumStates(); j++ )
      {
         ITRShape::State srcState = src.getState( j );
         ITRShape::State & destState = dest.getState( j );

         // add the name to the name index
         destState.nameIndex = dest.addFileName( src.getFileName( srcState.nameIndex ) );

         // determine the number of details
         for( UInt32 k = 0; k < srcState.numLODs; k++ )
         {
            ITRShape::LOD srcLod = src.m_lodVector[ srcState.lodIndex + k ];
            ITRShape::LOD & destLod = dest.m_lodVector[ destState.lodIndex + k ];
            
            // add the name to the name index
            destLod.geometryFileOffset = dest.addFileName( src.getFileName( srcLod.geometryFileOffset ) );
         
            // get the geometry for this detail
            ITRGeometry * geometry = instance->getGeometry( srcState.lodIndex + k );
            
            // go through the number of light sets
            for( UInt32 l = 0; l < src.getNumLightStates(); l++ )
            {
               ITRShape::LODLightState srcLightState = src.m_lodLightStates[ srcLod.lightStateIndex + l ];
               ITRShape::LODLightState & destLightState = dest.m_lodLightStates[ destLod.lightStateIndex + l ];
            
               // set the lightstate to grab
               instance->setInteriorLightState( resManager, l );
                                
               // get the lighting for this lightset/geometry
               ITRLighting lighting = *instance->getLighting( k );
               
               // process
               ITRBasicLighting::MaterialPropList matProps;
               
               // add one thing for now
               ITRBasicLighting::LightList lightList;                         
               
               // process the lights ( move them into the interior's space
               processLights( lightList, *m_interiors[i] );
               
              // convert the lightmap file
               String fileName = src.getFileName( srcLightState.lightFileOffset );
               char ext[64];
               sprintf( ext, "-%d.dil", interior_instance[i] );
               reextendFile( fileName, ext );
               
               // add the name to the alt dis file
               destLightState.lightFileOffset = dest.addFileName( fileName.c_str() );
               
               print( V_MEDIUM, "lighting-> %s - %s\n",
                  dest.getFileName( destLightState.lightFileOffset ),
                  dest.getFileName( destLod.geometryFileOffset ) );
               
               // light it
               ITRBasicLighting::g_useNormal = true;
//               ITRBasicLighting::g_useLightMaps = true;
//               ITRBasicLighting::g_missionLighting = true;
               ITRBasicLighting::filterScale = 0.5f;
               
               // fill in the callback info
               ITRBasicLighting::CallbackInfo callback;
               callback.obj = this;
               callback.collide = collide;

               // set the flag to indicate that this is not the terrain we are lighting ( for the callback )
               lightingTerrain = false;
               
               // do the lighting
//               ITRBasicLighting::light( *geometry, lightList, matProps, 
//                  &lighting, missionInteriorLOSCheck ? &callback : NULL );

               ITRMissionLighting missionLighting;
               ITRBasicLighting::light( *geometry, lightList, &lighting, &missionLighting, 
                  missionInteriorLOSCheck ? &callback : NULL );
               missionLighting.fileStore( fileName.c_str() );
               
               // restore the lightmap list size
//               lighting.fileStore( fileName.c_str() );

               dumpFileToVolumeAndRemove( volumeFile, fileName );
            }
         }

         // add the materiallist filename
         dest.m_materialListOffset = dest.addFileName( src.getMaterialListFileName() );

         // copy the light state names in
         for( UInt32 m = 0; m < src.getNumLightStates(); m++ )
         {
            dest.m_lightStateNames[m] = dest.addFileName( 
               src.getFileName( src.m_lightStateNames[m] ) );
         }

         // get the new dis file name
         String disFile;
         getInteriorBaseFilename( m_interiors[i], disFile );
         char ext[64];
         sprintf( ext, ".%d.dis", interior_instance[i] );
         reextendFile( disFile, ext );

         // set the filename for this SimInterior
         setInteriorFilename( m_interiors[i], disFile.c_str() );
         
         // write out the file
         FileRWStream Stream;
         Stream.open( disFile.c_str() );
         dest.store( Stream );
         Stream.close();

         // add to the volume
         dumpFileToVolumeAndRemove( volumeFile, disFile );
      }
   }
}

//------------------------------------------------------------------------------
// process the terrain objects

void MissionLighting::processTerrains()
{
   // go through all the terrains
   for( int i = 0; i < m_terrains.size(); i++ )
   {
      GridFile * grdFile = m_terrains[i]->getGridFile();
      _assert( grdFile, "Failed to get grid file!." );
      
      ResourceManager * resManager = SimResource::get( m_terrains[i]->manager );
      ResourceObject * resObj = resManager ? resManager->find( grdFile->getFileName() ) : NULL;

      FileName fileName;
      fileName.setTitle( resObj->fileName );
      fileName.setPath( resObj->filePath );
            
      print( V_MEDIUM, "lighting-> %s\n", fileName.c_str() );
      
      // fill in the callback info
      SimTerrain::LightingOptions options;
      options.mFlags.set( SimTerrain::LightingOptions::Console );
      
      // check some flags
      if( missionTerrainLOSCheck )
         options.mFlags.set( SimTerrain::LightingOptions::InteriorLOS );
      if( missionUseHiresTerrain )
      {
         options.mFlags.set( SimTerrain::LightingOptions::AllowHires );
         options.mFlags.set( SimTerrain::LightingOptions::RemoveSolids );
      }
      if( missionTerrainSubSample )
         options.mFlags.set( SimTerrain::LightingOptions::SubSample );
         
      // check for cloudmapping
      if( missionUseCloudMap )
      {
         options.mFlags.set( SimTerrain::LightingOptions::CloudMap );
         options.mCollideCallback = collide;
         options.mCallbackObj = this;
      }
      
      // set the flag to indicate that this is the terrain we are lighting ( for the callback )
      lightingTerrain = true;
      
      // clear the lightmaps   
      if( missionClearHiresLightmaps )
         m_terrains[i]->getGridFile()->getBlockList()->clearHiresLightmaps();

      // build all the lightmaps for the terrain
      m_terrains[i]->buildAllLightMaps( &options );

      if( fpuStackExceptions )
      {
         print( V_HIGH, "*** Caught %d fpu stack exceptions - data may be corrupt?!?!\n", 
            fpuStackExceptions );
         fpuStackExceptions = 0;
      }
         
      print( V_LOW, "saving terrain: %s\n", fileName.c_str() );
      bool saved = m_terrains[i]->save(fileName.c_str());
      _assert( saved, "Failed to save %s. Write protected?", fileName.c_str());
   }
}

//------------------------------------------------------------------------------
// used for printing to the screen at a particular verbosity (word??)

void MissionLighting::print( VERBOSE level, char * str, ... )
{
   // only print the ones we want
   if( level > verboseLevel )
      return;
 
   // send off to vprintf     
   va_list v_list;
   va_start( v_list, str );
   vprintf( str, v_list );
   va_end( v_list );
}

//------------------------------------------------------------------------------
// used instead of AssertFatal ( which is reserved for *should never happen* conditions )

void MissionLighting::_assert( bool cond, char * str, ... )
{
   // check the condition
   if( !cond )
   {
      // print out the message
      printf( "\n   *** Fatal Error! ***\n" );
      va_list v_list;
      va_start( v_list, str );
      vprintf( str, v_list );
      va_end( v_list );
      printf( "\n" );
      exit( 1 );
   }
}
  