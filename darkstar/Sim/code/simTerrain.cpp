//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#ifdef __BORLANDC__
#pragma warn -inl
#pragma warn -aus
#endif

#include "grdCollision.h"
#include "simTerrain.h"
#include "simLightGrp.h"
#include "genericActions.h"
#include "editor.strings.h"
#include "simPersistTags.h"
#include "simMovement.h"
#include <winTools.h>
#include "simResource.h"
#include "simAction.h"
#include "console.h"
#include "simGame.h"
#include "stringTable.h"
#include "fileName.h"
#include "simsetiterator.h"
#include "simvolume.h"
#include "itrlighting.h"
#include "netghostmanager.h"
#include "ts_material.h"
#include "g_surfac.h"
#include "gOGLSfc.h"
#include "terrData.h"
//----------------------------------------------------------------------------

// only show items in container that are .85 the far distance from camera
// ...helps control floating buildings
#define containerVisFactor (.85f)

float SimTerrain::PrefVisibleDistance = 10000.0f;
float SimTerrain::PrefPerspectiveDistance = 1.0f;
float SimTerrain::PrefDetail = 1.0f;
float SimTerrain::PrefTerrainTextureDetail = 1.0f;


SimTerrain::Visibility DefTerrainVisibility =
{
   3000.0f,       // visible distance
   2000.0f,       // haze distance
	1000.0f,			// Min Vertical
	2000.0f,			// Max Vertical
};

SimTerrain::Detail DefTerrainDetail =
{
   500.0f,        // perspective distance
	120.0f,		   // pixel size
};


//----------------------------------------------------------------------------
// go through and calculate the file crc for the gridfile
bool SimTerrain::calcCrc( const char * fileName, UInt32 & crcVal )
{
   AssertFatal(fileName, "SimTerrain::calcCrc - filename cannot be null");
   AssertFatal(manager, "SimTerrain::calcCrc - manager is null");

   ResourceManager * rm = SimResource::get(manager);

   // add to the running crc
   if(!rm->getCrc(fileName, crcVal))
      return(false);

   // load in the gridfile so that we can look at the blocks
   Resource<GridFile> gridFile = rm->load(fileName);
   if(!bool(gridFile))
   {
      AssertFatal(false, avar("SimTerrain::calcCrc - '%s' could not load", fileName));
      return false;
   }
   GridBlockList * bList = gridFile->getBlockList();

   // walk through all the blocks and add to the checksum
  	for(GridBlockList::iterator itr = bList->begin(); itr != bList->end(); itr++)
   {
		GridBlockListElem & elm = **itr;
      const char * blockName = bList->makeBlockName( elm.blockId, fileName );

      // add to the running crc
      if(!rm->getCrc(blockName, crcVal, crcVal))
         return(false);
   }

   // add to the running crc   
   if(!rm->getCrc(gridFile->getMaterialListName(), crcVal, crcVal))
      return(false);

   return(true);
}

//----------------------------------------------------------------------------
// SimTerrain
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

SimTerrain::SimTerrain()
{
   id = SimTerrainId;
	type |= SimTerrainObjectType;
   tedFile = NULL;
   database = NULL;
   terrainCrc = 0xffffffff;
   collisionImage.gridFile = NULL;
   renderImage.terrainFile = NULL;
   renderImage.myTerrain = this;

	setContext(EulerF(.0f,.0f,.0f),Point3F(0.0f,0.0f,0.0f));
	updateBoundingBox();

	containerProperties.gravity.set(.0f,.0f,-9.8f);
	containerProperties.drag = 0.0f;
	// If containerProperties.height is zero, it defaults to the
	// max terrain height.
	containerProperties.height = 10000.0f;

   containerMask = Int32(-1);
   database = new SimXYDB;
   installDatabase(database);

   maxVisibility = DefTerrainVisibility;
	maxDetail = DefTerrainDetail;
   applyUserPrefs();
   suspendRender = false;
   interiorMask = SimInteriorObjectType;
   
   netFlags.set(SimNetObject::ScopeAlways);
   netFlags.set(SimNetObject::Ghostable);
}

SimTerrain::~SimTerrain()
{
}

float
SimTerrain::getScopeVisibleDist() const
{
   return getVisibleDistance();
}

//----------------------------------------------------------------------------
extern TerrainFile *createFileFromGridFile(GridFile *gf);

bool SimTerrain::create( const char *terrName, int GFsize, int GFgroundScale, int GBsize, int GBlightScale, GridFile::GBPatternMap pattern )
{
   tedFile = stringTable.insert(terrName);
	// Build a new grid file and place a default block in it.
   GridFile *gf = new GridFile;

	gf->setSize( Point2I(GFsize,GFsize) );
	gf->setScale( GFgroundScale );
   gf->setFileName(terrName);
   gf->setBlockPattern( pattern );

   ResourceManager *rm = SimResource::get(manager);
   rm->add(ResourceType::typeof(terrName), terrName, gf);
   gf->setResourceManager(rm);
   renderImage.gridFile = rm->load(terrName);

   delete renderImage.terrainFile;
   renderImage.terrainFile = createFileFromGridFile(renderImage.gridFile);

   createGridBlocks( GBsize, GBlightScale );

  	renderImage.gridRender.setHeightRange(renderImage.gridFile->getHeightRange());
	renderImage.gridRender.setScale(renderImage.gridFile->getScale());
	setContext(context.rot,context.pos);
	setVisibility(&maxVisibility);
	setDetail(&maxDetail);
	updateBoundingBox();
   return true;
}

//----------------------------------------------------------------------------

bool SimTerrain::createGridBlocks( int GBsize, int GBlightScale )
{
   int GFsize = renderImage.gridFile->getSize().x;
   bool success = true;
   int x, y;
   GridBlock* block;
   switch ( renderImage.gridFile->getBlockPattern() )
   {
      case GridFile::OneBlockMapsToAll:
      	block = new GridBlock;
         success &= (block != NULL);
         if ( !success ) break;
      	block->setSize( Point2I(GBsize,GBsize) );
      	block->setLightScale( GBlightScale );
         renderImage.gridFile->addBlock(block);
      	for ( y=0; y<GFsize; y++ )
         	for ( x=0; x<GFsize; x++ )
               renderImage.gridFile->positionBlock(Point2I(x, y), block);
         break;   

      case GridFile::EachBlockUnique:
       	for ( y=0; y<GFsize; y++ )
         	for ( x=0; x<GFsize; x++ )
         	{
            	block = new GridBlock;
               success &= (block != NULL);
               if ( !success ) break;
            	block->setSize( Point2I(GBsize,GBsize) );
            	block->setLightScale( GBlightScale );
               renderImage.gridFile->addBlock(block);
               renderImage.gridFile->positionBlock(Point2I(x, y), block);
         	}
         break;

      case GridFile::MosaicBlockPattern1:{
         // In this Terrain layout, the middle 4x4 grid blocks are unique
         // all other grid blocks surrounding the middle 4x4 are tiled 
         // form of the middle 4x4 sans terrain stamps
         // 0-15 are surrounding grid blocks
         // 16-31 are the inner 4x4 grid blocks
      	for ( x=0; x<32; x++ )   // allocate all grid blocks
      	{
         	block = new GridBlock;
            success &= (block != NULL);
            if ( !success ) break;
         	block->setSize( Point2I(GBsize,GBsize) );
         	block->setLightScale( GBlightScale );
            renderImage.gridFile->addBlock(block);
      	}
         GridBlockList *blockList = renderImage.gridFile->getBlockList();
         // set the tiled surrounding grid blocks, (tricky)
         int offset = ((GFsize-4)>>1)&3;
      	for ( y=0; y<GFsize; y++ )
         	for ( x=0; x<GFsize; x++ )
         	{
               block =  blockList->findBlock( ((x+offset)&3) + (((y+offset)&3)<<2) );
               renderImage.gridFile->positionBlock(Point2I(x,y),block);
         	}
         // set the inner 4x4 grid blocks
         offset = (GFsize-4)>>1;
      	for (x = 0; x < 16; x++)
      	{
            block = blockList->findBlock( 16+x );
            renderImage.gridFile->positionBlock(Point2I(((x&3)+offset), (x>>2)+offset), block);
      	}
         }
         break;

      default:{
         AssertFatal(0,avar("SimTerrain::createGridFile:  Unsupported Block Pattern: %i",renderImage.gridFile->getBlockPattern()) );
         success = false;
         }
         break;
   }
   return success;
}   

//----------------------------------------------------------------------------

bool SimTerrain::load(const char *filename)
{
	tedFile = stringTable.insert(filename);
	if (manager) {
      ResourceManager *rm = SimResource::get(manager);

		renderImage.gridFile = rm->load(filename);
		if ( (bool)renderImage.gridFile == NULL )
         return false;
      renderImage.gridFile->setResourceManager(rm);
      renderImage.gridFile->setFileName(filename);

      loadMaterials();  // may or may not load materials, check bool(materialList);

      if(isGhost())
      {
         delete renderImage.terrainFile;
         renderImage.terrainFile = createFileFromGridFile(renderImage.gridFile);
      }
	  	renderImage.gridRender.setHeightRange(renderImage.gridFile->getHeightRange());
		renderImage.gridRender.setScale(renderImage.gridFile->getScale());
		setContext(context.rot,context.pos);
		setVisibility(&maxVisibility, true);
		setDetail(&maxDetail);
	}
	updateBoundingBox();
	return true;
}

bool SimTerrain::save(const char *volumeName)
{
   FileName fn( volumeName );
   FileName tedName( tedFile );
   
   // check for a change in the filename
   if( fn.getBase() != tedName.getBase() )
   {
      Console->setBoolVariable( "$TED::terrainNameChange", true );
   
      // find the SimVolume object with this fileName and change it
      ResourceManager * resManager = SimResource::get( manager );
      ResourceObject * resObj = resManager ? resManager->find( tedFile ) : NULL;
      while( resObj )
      {
         SimGroup * grp = NULL;
         SimObject * obj = NULL;

         // get to the volumes' group
         if((obj = getManager()->findObject("MissionGroup")) == NULL)
            break;
         if((grp = dynamic_cast<SimGroup *>(obj)) == NULL)
            break;
         if((obj = grp->findObject("Volumes")) == NULL)
            break;
         if((grp = dynamic_cast<SimGroup *>(obj)) == NULL)
            break;
          
         for( SimSetIterator itr( grp ); *itr; ++itr )
         {
            SimVolume * volObj = dynamic_cast< SimVolume *>(*itr);
            if(volObj && !stricmp(resObj->fileName, volObj->getFileName()))
            {
               volObj->open(fn.getTitle().c_str());
               break;
            }
         }
         break;
      }      
      
      fn.setExt( "dtf" );
      tedFile = stringTable.insert( fn.getTitle().c_str() );

      // need to load in all the blocks before changing the filename
      getGridFile()->loadAllBlocks();
      getGridFile()->setFileName( tedFile );
   }
   else
      Console->setBoolVariable( "$TED::terrainNameChange", false );
   
   return( getGridFile()->save( volumeName ) );
}

//----------------------------------------------------------------------------

bool SimTerrain::loadMaterials()
{
   ResourceManager *rm = SimResource::get(manager);
	// The material list is not stored in the gridFile,
	// only it's name.
	char matname[256];
	strcpy (matname,renderImage.gridFile->getMaterialListName());
	char* ext = matname + strlen(matname) - 1;
	while (*ext != '.' && ext != matname)
		--ext;
	if (ext == matname)
		ext = matname + strlen(matname);
	strcpy(ext,".dml");

	// Load materials using resource manager
	Resource<TSMaterialList> materialList = rm->load(matname);
   bool tempCheck = bool(materialList);
	AssertWarn(tempCheck, "SimTerrain:: Could not load material list");
   if ( tempCheck )
   {
   	if(isGhost())
   	   materialList->load(*rm,true);
   	renderImage.gridFile->setMaterialList(materialList);
      return true;
   }
   return false;
}   

//----------------------------------------------------------------------------

bool SimTerrain::setVisibility(const Visibility *pTP, bool force)
{
	// Clients can set details but not visibility.
	if (!isGhost() || force) {
	   if ( !pTP )
	      pTP = &DefTerrainVisibility;
		maxVisibility = *pTP;
      applyUserPrefs();
      setMaskBits(VisibilityMask);
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------

bool SimTerrain::setDetail(const Detail *pTP)
{
   if ( !pTP )
      pTP = &DefTerrainDetail;
	maxDetail = *pTP;
   applyUserPrefs();
   setMaskBits(DetailMask);
	return true;
}


//------------------------------------------------------------------------------
void SimTerrain::applyUserPrefs()
{
   float hazeFactor = maxVisibility.hazeDist/maxVisibility.visibleDist;
#ifdef DEBUG
   AssertFatal(PrefVisibleDistance > 1.0f, "Pref::PrefVisibleDistance is now a float 0-maxDist in meters.")
#endif
   visibility.visibleDist = min(maxVisibility.visibleDist, PrefVisibleDistance);
   visibility.hazeDist    = maxVisibility.hazeDist*hazeFactor;
   visibility.hazeVerticalMin = maxVisibility.hazeVerticalMin;
   visibility.hazeVerticalMax = maxVisibility.hazeVerticalMax;

   // don't allow them to cross over
   if (visibility.hazeDist > (visibility.visibleDist * 0.75f))
      visibility.hazeDist = visibility.visibleDist*0.75f;

   detail.perspectiveDist = maxDetail.perspectiveDist*PrefPerspectiveDistance;
   detail.screenSize      = maxDetail.screenSize;

   // apply changes to grid file
	if (bool(renderImage.gridFile)) 
	{  // visibility
		renderImage.gridRender.setVisibleDistance(visibility.visibleDist);
		renderImage.gridRender.setHazeDistance(visibility.hazeDist);
		//renderImage.gridRender.setHazeVertical(visibility.hazeVerticalMin, visibility.hazeVerticalMax);
      // detail
      terrainTextureDetail = renderImage.gridFile->getDetailCount() * (1-PrefTerrainTextureDetail);
	 	renderImage.gridRender.setScreenSize(detail.screenSize);
		renderImage.gridRender.setPerspectiveDistance(detail.perspectiveDist);
      renderImage.gridRender.setTerrainTextureDetail(terrainTextureDetail);
	}
	if (database)
	   database->setFarDist(visibility.visibleDist * containerVisFactor);
}   


//----------------------------------------------------------------------------

void SimTerrain::setContext(const EulerF& e,const Point3F& p)
{
	context.pos = p;
	context.rot = e;
	renderImage.transform.set(e,p);
	renderImage.inverseTransform = renderImage.transform;
	renderImage.inverseTransform.inverse();
	updateBoundingBox();
}


//----------------------------------------------------------------------------

bool SimTerrain::setContainerProperties(const ContainerProperties& p)
{
	containerProperties = p;
	// Should probably change all the forces on the objects
	// within the container.  Unfortunatly, their is no way to
	// iterate through a parent container classes object list.
	return true;
}


//----------------------------------------------------------------------------

bool SimTerrain::getSurfaceInfo(const Point2F &pos, CollisionSurface *info)
{
   Point3F p(pos.x, pos.y, 0.0f), p0;
   m_mul(p, renderImage.inverseTransform, &p0);
   AssertFatal(getGridFile(), "SimTerrain::getSurfaceInfo: no grid file");
   return getGridFile()->getSurfaceInfo(p0, info);
}


//----------------------------------------------------------------------------

bool SimTerrain::getSurfaceHeight(const Point2F &pos, float *height, bool ignoreEmptyTiles)
{
   Point3F p(pos.x, pos.y, 0.0f), p0;
   m_mul(p, renderImage.inverseTransform, &p0);
   AssertFatal(getGridFile(), "SimTerrain::getSurfaceHeight: no grid file");
   return getGridFile()->getSurfaceHeight(p0, height, ignoreEmptyTiles);
}


//----------------------------------------------------------------------------

GridBlock::Material* SimTerrain::getMaterial(const Point2F &pos)
{
   Point3F p(pos.x, pos.y, 0.0f), p0;
   m_mul(p, renderImage.inverseTransform, &p0);
   return getGridFile()->getMaterial(p0);
}

float SimTerrain::getHazeValue(TSRenderContext & rc, const Point3F & objectPos)
{
   return (getHazeValue(rc.getCamera()->getTCW().p, objectPos));
}

float SimTerrain::getHazeValue(const Point3F &cameraPos, const Point3F &objPos)
{
   float rDistance = min(m_distf(cameraPos, objPos), visibility.visibleDist);
   return (renderImage.gridRender.getHaze(rDistance));
}

//----------------------------------------------------------------------------

bool SimTerrain::processEvent(const SimEvent* event)
{
	switch (event->type)
	{
		onEvent(SimActionEvent);
      onEvent(SimMessageEvent);
      onEvent(SimRegisterTextureEvent);
	}
	// SimContainer base wants delete notifys.
	return Parent::processEvent(event);
}

bool
SimTerrain::onSimRegisterTextureEvent(const SimRegisterTextureEvent *event)
{
   GridFile* grdFile = getGridFile();
   if (grdFile == NULL)
      return true;

   GFXSurface* pSurface = const_cast<GFXSurface*>(event->pSurface);
   if (dynamic_cast<OpenGL::Surface*>(pSurface) == NULL)
      return true;

   TSMaterialList* pMaterialList = grdFile->getMaterialList();
   AssertFatal(pMaterialList != NULL, "No Material list?");

   for (int i = 0; i < pMaterialList->getMaterialsCount(); i++) {
      TS::Material& rMaterial = (*pMaterialList)[i];
      if ((rMaterial.fParams.fFlags & TS::Material::MatFlags) == TS::Material::MatTexture) {
         const GFXBitmap* pBitmap = rMaterial.getTextureMap();
         if (pBitmap == NULL)
            continue;

         pSurface->setTextureMap(pBitmap);

         pSurface->lock();
         pSurface->drawBitmap2d(pBitmap, &Point2I(0, 0));
         pSurface->unlock();
         pSurface->flip();
      }
   }

   return true;
}

bool SimTerrain::onSimMessageEvent(const SimMessageEvent *event)
{
   switch (event->message) {
      case BuildLightMapMessage:
      {
         buildLightMap();
         if (manager->isServer())
         {
            // send a message to ghosts to relight
         }
         break;
      }
      case SaveTerrainMessage:
      {
         FileInfo tedVol;
         tedVol.init(NULL, "vol", FileInfo::TYPE_VOL, NULL );
         if ( tedVol.getName( FileInfo::SAVE_AS_MODE, "Save TED Volume" ) )
         {
            getGridFile()->loadAllBlocks();
            getGridFile()->markLoadedBlocksAsDirty();
            save( tedVol.fileName() );
         }
			break;
      }
		case RefreshClientMessage: {
			// Refresh client visiblity to make sure they aren't cheating.
	      setMaskBits(VisibilityMask);
			SimMessageEvent::post(this,manager->getCurrentTime() + 30.0f,
				RefreshClientMessage);
			break;
		}
      default:
         return false;
   }
   return true;
}
   
bool SimTerrain::onAdd()
{
	if (!Parent::onAdd())
		return false;
	if (tedFile && tedFile[0])
   {
      // check if server or not...
      if(!isGhost()) // server
      {
         // calculate the crc and set to send
         if(!calcCrc(tedFile, terrainCrc))
         {
            Net::setLastError(avar("Could not verify Terrain: %s", tedFile));
            return(false);
         }
         setMaskBits(CrcMask);
      }
      else // ghost
      {
         // check the validity of this file
         UInt32 ghostCrc;
         if( !calcCrc(tedFile, ghostCrc))
         {
            Net::setLastError(avar("You don't have the custom map the server is running.  Check the Server Info for a possible download site.  Missing map file: %s.", tedFile));
            return(false);
         }
         
         if((ghostCrc != terrainCrc))
         {
            Net::setLastError(avar("The server has a different version of the map.  Check the Server Info for a possible download site.  Incorrect map file: %s.", tedFile));
            return(false);
         }
      }
      load(tedFile);
   }

	updateBoundingBox();
	SimContainer* root = NULL;
   
   root = findObject(manager,SimRootContainerId,root);
	root->addObject(this);
   manager->assignId( this, SimTerrainId );

	// Start refresh messages on the server
	if (!isGhost())
		SimMessageEvent::post(this,manager->getCurrentTime() + 15.0f,
			RefreshClientMessage);
	return true;
}

bool SimTerrain::onSimActionEvent(const SimActionEvent* event)
{
	switch(event->action) {
		case MoveUp:
      {
//         static toggle = true;
//         getGridFile()->getBlockList()->enableHires(toggle);
//         toggle ^= 1;
         buildLightMap();
         
			break;
      }
      case MoveDown:
      {  
         // use default settings   
         LightingOptions options;
         if( Console->getBoolVariable( "$TED::castInteriorShadows", false ) )
            options.mFlags.set( LightingOptions::InteriorLOS );
         if( Console->getBoolVariable( "$TED::castHiresShadows", false ) )
            options.mFlags.set( LightingOptions::AllowHires | LightingOptions::RemoveSolids );
         buildAllLightMaps( &options );
         Console->evaluate( "flushTextureCache();", false );
         break;
      }
	}
	return true;
}

void SimTerrain::inspectRead(Inspect *inspector)
{
	Parent::inspectRead(inspector);
   Detail newDetail;
   Visibility newVis;
   Inspect_Str fileName;
   Inspect_Str DMLfileName;

   inspector->read(IDITG_TERRAIN_CONTEXT_POS, context.pos );
   inspector->read(IDITG_VISIBILITY_DIST, newVis.visibleDist);
   inspector->read(IDITG_HAZE_DIST, newVis.hazeDist);
   inspector->read(IDITG_HAZE_VMINDIST, newVis.hazeVerticalMin);
   inspector->read(IDITG_HAZE_VMAXDIST, newVis.hazeVerticalMax);
   inspector->read(IDITG_PERSPECTIVE_DIST, newDetail.perspectiveDist);
   inspector->read(IDITG_SCREEN_SIZE, newDetail.screenSize);
   inspector->read(IDITG_GRID_FILE_NAME, fileName);
   inspector->read(IDITG_DML_FILE_NAME, DMLfileName);
   inspector->read(IDITG_TERRAIN_GRAVITY,containerProperties.gravity);
   inspector->read(IDITG_TERRAIN_DRAG,containerProperties.drag);
   inspector->read(IDITG_TERRAIN_HEIGHT,containerProperties.height);

   inspector->readActionButton(IDITG_BUILD_LIGHTMAP, NULL, NULL);
   inspector->readActionButton(IDITG_SAVE_TERRAIN, NULL, NULL);

   if(strcmp(tedFile, fileName) != 0)
      load(fileName);

   if(strcmp(renderImage.gridFile->getMaterialListName(), DMLfileName))
   {
      renderImage.gridFile->setMaterialListName(DMLfileName);
      loadMaterials();
   }

   setVisibility(&newVis);
   setDetail(&newDetail);

   setMaskBits(TedFileMask | VisibilityMask | DetailMask | ContextMask);
}

void SimTerrain::inspectWrite(Inspect *inspector)
{
	Parent::inspectWrite(inspector);

   inspector->write(IDITG_TERRAIN_CONTEXT_POS,context.pos );
   inspector->write(IDITG_VISIBILITY_DIST, maxVisibility.visibleDist);
   inspector->write(IDITG_HAZE_DIST, maxVisibility.hazeDist);
   inspector->write(IDITG_HAZE_VMINDIST, maxVisibility.hazeVerticalMin);
   inspector->write(IDITG_HAZE_VMAXDIST, maxVisibility.hazeVerticalMax);
   inspector->write(IDITG_PERSPECTIVE_DIST, maxDetail.perspectiveDist);
   inspector->write(IDITG_SCREEN_SIZE, maxDetail.screenSize);
   inspector->write(IDITG_GRID_FILE_NAME, (char *) tedFile);
   inspector->write(IDITG_DML_FILE_NAME, (char *) renderImage.gridFile->getMaterialListName());
   inspector->write(IDITG_TERRAIN_GRAVITY,containerProperties.gravity);
   inspector->write(IDITG_TERRAIN_DRAG,containerProperties.drag);
   inspector->write(IDITG_TERRAIN_HEIGHT,containerProperties.height);

   inspector->writeActionButton(IDITG_BUILD_LIGHTMAP, this, BuildLightMapMessage);
   inspector->writeActionButton(IDITG_SAVE_TERRAIN, this, SaveTerrainMessage);
}


//----------------------------------------------------------------------------

bool SimTerrain::processQuery(SimQuery* query)
{
	switch (query->type)
	{
		onQuery(SimRenderQueryImage);
		onQuery(SimObjectTransformQuery);
		onQuery(SimCollisionImageQuery);
	}
	// Parent container wants delete notifies
	return Parent::processQuery(query);
}

void SimTerrain::setSuspendRender(bool _suspend)
{
   suspendRender = _suspend;
   setMaskBits(SuspendRenderMask);
}

bool SimTerrain::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   applyUserPrefs();
   if (suspendRender) {
      query->count = 0;
      return false;
   }
	query->count = 1;
	query->image[0] = &renderImage;
   renderImage.itype = SimRenderImage::Normal;
	return true;
}

bool SimTerrain::onSimObjectTransformQuery(SimObjectTransformQuery* query)
{
	query->tmat = renderImage.transform;
	return true;
}


bool SimTerrain::onSimCollisionImageQuery(SimCollisionImageQuery* query)
{
	collisionImage.gridFile = renderImage.gridFile;
	collisionImage.transform = renderImage.transform;
	collisionImage.editor = query->detail == SimCollisionImageQuery::Editor;
   query->count = 1;
	query->image[0] = &collisionImage;
	return true;
}

//----------------------------------------------------------------------------

bool SimTerrain::processArguments(int argc, const char **argv)
{
   if(argc > 0)
   {
      if(!strcmp(argv[0], "Create"))
      {
         if(argc < 7)
         {
            Console->printf("SimTerrain: Create fileName.dtf GFSize GFGroundScale GBSize GBLightScale uniqueBlocks [posX posY posZ] [rotX rotY rotZ]");
            return false;
         }
		   create(argv[1],          // terrain file name (mars.dtf)
					 atoi(argv[2]),    // GFsize, number of gridBlocks per side
					 atoi(argv[3]),    // GFGroundScale, (1<<n) world units per poly
					 atoi(argv[4]),    // GBsize, matrix size for each gridBlock
					 atoi(argv[5]),    // GBlightScale, 
					 (GridFile::GBPatternMap)atoi(argv[6]));   // uniqueBlocks, (is each gridBlock unique, or the same)
		   setDetail();

         // next six arguments are position & rot vel if they exist
         float f[6];
			memset(f,0,sizeof(f));
         for (int i = 7; i < argc; i++)
            sscanf(argv[i], "%f", &f[i - 7]);
         const float p = float(M_PI/180.0);
         setContext(EulerF(p*f[3],p*f[4],p*f[5]),Point3F(f[0],f[1],f[2]));
         return true;
      }
      else if(!strcmp(argv[0], "Load"))
      {
         if(argc < 2)
         {
            Console->printf("SimTerrain: Load fileName.dtf [posX posY posZ] [rotX rotY rotZ]");
            return false;
         }
			load(argv[1]);
			setDetail();
			setVisibility();

         // next six arguments are position & rot vel if they exist
         float f[6];
			memset(f,0,sizeof(f));
         for (int i = 2; i < argc; i++)
            sscanf(argv[i], "%f", &f[i - 2]);
         const float p = float(M_PI/180.0);
         setContext(EulerF(p*f[3],p*f[4],p*f[5]),Point3F(f[0],f[1],f[2]));
         return true;
      }
   }
   Console->printf("SimTerrain: Create/Load ...");
   return false;
}

//----------------------------------------------------------------------------

bool SimTerrain::los(const Point3F& start, const Point3F& end)
{
	GridCollision collision(renderImage.gridFile);
	if (!collision.collide(start,end,true))
   {
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------

void SimTerrain::updateBoundingBox()
{
	if (GridFile* file = getGridFile()) {
		Point2I size = file->getSize();
		int scale = file->getScale() + file->getDetailCount();
		GridRange<float> range = file->getHeightRange(false);

		Box3F bb,tb;
		bb.fMin = bb.fMax = Point3F(0.0f,0.0f,0.0f);
		bb.fMax.x += float(size.x << scale);
		bb.fMax.y += float(size.y << scale);

		float hh = containerProperties.height?
			 containerProperties.height: range.fMax;
		bb.fMax.z += hh;
		bb.fMin.z -= hh;

		m_mul(bb.fMin,renderImage.transform,&tb.fMin);
		m_mul(bb.fMax,renderImage.transform,&tb.fMax);
		Box3F box;
		box = tb;
		box.fMin.setMin(tb.fMax);
		box.fMax.setMax(tb.fMin);
		setBoundingBox(box);
	}
}	


//----------------------------------------------------------------------------
// Management of SimMovement objects comming in and out of the container.
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

void SimTerrain::enterContainer(SimContainer* obj)
{
	Parent::enterContainer(obj);
	if (SimMovement* mobj = dynamic_cast<SimMovement*>(obj)) {
		mobj->addForce(SimMovementGravityForce,containerProperties.gravity,true);
		mobj->setContainerDrag(containerProperties.drag);
		mobj->setContainerCurrent(Point3F(.0f,.0f,.0f),.0f);
	}
}

void SimTerrain::leaveContainer(SimContainer* obj)
{
	if (SimMovement* mobj = dynamic_cast<SimMovement*>(obj))
		mobj->removeForce(SimMovementGravityForce);
	Parent::leaveContainer(obj);
}


//----------------------------------------------------------------------------
// LightMap management
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

void SimTerrain::buildLightMap()
{
   // Get the scene lighting information
   SimLightSet *simLightSet = 
      dynamic_cast<SimLightSet *>(manager->findObject(SimLightSetId));
   if (simLightSet) {
	   TSSceneLighting *sceneLights = simLightSet->getSceneLighting();
 		sceneLights->lock();
		sceneLights->prepare(SphereF(Point3F(0,0,0), 0) , renderImage.transform);

		// ok... this is lame.
		// gotta loop back through all the lights and force
		// optimizations off.  This is because of the large area
		// of the terrain - messes with point lights.
		if (simLightSet) {
			SimLightSet::iterator ptr;
			SimLightQuery query;
			for (ptr = simLightSet->begin(); ptr != simLightSet->end(); ptr++)
				if ((*ptr)->processQuery(&query))
					for (int i = 0; i < query.count; i++)
						query.light[i]->fLight.fOptimized = 1;
		}

		// Now tell all the blocks to build their light maps
		Point2I pos,size = renderImage.gridFile->getSize();
		for (pos.y = 0; pos.y < size.y; pos.y++)
			for (pos.x = 0; pos.x < size.x; pos.x++) {
			   GridBlock *block = renderImage.gridFile->getBlock(pos);
            block->setDirtyFlag();
				block->buildLightMapTest(renderImage.gridFile,
					sceneLights,renderImage.transform.p );
            if(pos.x)
            {
               // copy the left most column into the rightmost
               // column of the previous block.
               Point2I posprev(pos.x-1, pos.y);
               GridBlock *prevBlock = renderImage.gridFile->getBlock(posprev);
               UInt16 *lm = block->getLightMap();
               UInt16 *pm = prevBlock->getLightMap();
               int stride = block->getLightMapWidth();
               int i;
               pm += stride-1;
               for(i = 0; i < stride; i++)
               {
                  *pm = *lm;
                  pm += stride;
                  lm += stride;
               }
            }
            if(pos.y)
            {
               // copy the left most column into the rightmost
               // column of the previous block.
               Point2I posprev(pos.x, pos.y-1);
               GridBlock *prevBlock = renderImage.gridFile->getBlock(posprev);
               UInt16 *lm = block->getLightMap();
               UInt16 *pm = prevBlock->getLightMap();
               int stride = block->getLightMapWidth();
               int i;
               pm += (stride-1) * stride;
               for(i = 0; i < stride; i++)
                  *pm++ = *lm++;
            }
			}

		sceneLights->unlock();
   }
	
   Console->evaluate("flushTextureCache();", false);
}

//----------------------------------------------------------------------------

void SimTerrain::buildAllLightMaps( LightingOptions * options )
{
   AssertFatal( options, "No lighting options found." );
   lightingOptions = *options;   

   // get the interior mask...
   interiorMask = Console->getIntVariable( "$SimTerrain::InteriorMask", SimInteriorObjectType );
   
   Vector<Box2F> boxList;
   Point2I pos( 0, 0 );
   GridFile * gridFile = getGridFile();
   GridBlock * gridBlock = gridFile->getBlock( pos );

   // get the size's and such
   Point2I fileSize = gridFile->getSize();
   Point2I blockSize = gridBlock->getSize();
   
   // determine the type of terrain file
   GridFile::GBPatternMap pattern = gridFile->getBlockPattern();
   Box2F box;
   
   // get the terrain context position
   SimTerrain::Context context;
   context = getContext();
   
   switch( pattern )
   {
      case GridFile::OneBlockMapsToAll:
      {
         // push just the one block ( try and get the center one )
         Point2I center = Point2I( ( fileSize.x + 1 ) / 2, ( fileSize.y + 1 ) / 2 );
         
         box.fMin.x = ( center.x - 1 ) * blockSize.x * ( 1 << gridFile->getScale() );
         box.fMin.y = ( center.y - 1 ) * blockSize.y * ( 1 << gridFile->getScale() );
         box.fMax.x = center.x * blockSize.x * ( 1 << gridFile->getScale() );
         box.fMax.y = center.x * blockSize.y * ( 1 << gridFile->getScale() );
         
         // adjust to the terrain
         box.fMin += context.pos;
         box.fMax += context.pos;
         
         boxList.push_back( box );
         
         break;
      }
      case GridFile::EachBlockUnique:
      {
         Point2I size = gridFile->getSize();
         
         // push the entire file
         box.fMin.x = box.fMin.y = 0.0f;
         box.fMax.x = blockSize.x * fileSize.x * ( 1 << gridFile->getScale() );
         box.fMax.y = blockSize.y * fileSize.y * ( 1 << gridFile->getScale() );

         // adjust to the terrain
         box.fMin += context.pos;
         box.fMax += context.pos;
         
         boxList.push_back(box);
         
         break;
      }
      case GridFile::MosaicBlockPattern1:
      {
         // ok, this is a 3x3 grid with 3nx3n blocks.. grab the center
         Point2I gridCount;
         gridCount.x = fileSize.x / 3;
         gridCount.y = fileSize.y / 3;
         
         // push the center
         box.fMin.x = blockSize.x * ( 1 << gridFile->getScale() ) * gridCount.x;
         box.fMin.y = blockSize.y * ( 1 << gridFile->getScale() ) * gridCount.y;
         box.fMax.x = blockSize.x * ( 1 << gridFile->getScale() ) * gridCount.x * 2;
         box.fMax.y = blockSize.y * ( 1 << gridFile->getScale() ) * gridCount.y * 2;

         // adjust to the terrain
         box.fMin += context.pos;
         box.fMax += context.pos;
         
         boxList.push_back(box);

         // need to grab the first block that contains an interior, top-left
         // if none
         SimObject * obj = getManager()->findObject("MissionGroup");
         AssertFatal(obj, "SimTerrain::buildAllLightMaps: could not get mission group");

         SimGroup * grp = dynamic_cast<SimGroup*>(obj);
         AssertFatal(grp, "SimTerrain::buildAllLightMaps: invalid mission group");   
   
         // walk through all the shapes and stop on the first one
         // that is not in the center block   
         Point2I lightBlock(0,0);
         
         for(SimSetIterator itr(grp); *itr; ++itr)
         {
            // only grab the containers that we want
            SimContainer * container = dynamic_cast<SimContainer *>(*itr);
            if(container && (container->getType() & interiorMask))
            {
               // just look at the min point in the bounding box
               // if the bounding box crosses a terrain block it will
               // most likely light improperly anyways - ignores z, so an
               // interior entirely underground will still be used!
               Box3F interiorBox = container->getBoundingBox();
               Point2F pnt(interiorBox.fMin.x, interiorBox.fMin.y);
               
               // check if this contains it - box is the center block
               if(!box.contains(pnt))
               {
                  // get to the quadrant that this belongs to
                  if(pnt.x > box.fMin.x)
                     lightBlock.x++;
                  if(pnt.x > box.fMax.x)
                     lightBlock.x++;
                     
                  if(pnt.y > box.fMin.y)
                     lightBlock.y++;
                  if(pnt.y > box.fMax.y)
                     lightBlock.y++;
                  
                  break;
               }
            }
         }
         
         // fill in the block coord
         box.fMin.x = blockSize.x * ( 1 << gridFile->getScale() ) * gridCount.x * lightBlock.x;
         box.fMin.y = blockSize.y * ( 1 << gridFile->getScale() ) * gridCount.y * lightBlock.y;
         box.fMax.x = blockSize.x * ( 1 << gridFile->getScale() ) * gridCount.x * (lightBlock.x + 1);
         box.fMax.y = blockSize.y * ( 1 << gridFile->getScale() ) * gridCount.y * (lightBlock.y + 1);
         
         // adjust to the terrain
         box.fMin += context.pos;
         box.fMax += context.pos;
         
         boxList.push_back(box);
         break;
      }
   }      
   
   Vector<UInt16*> mapData;
   for(int i = 0; i < 2; i++)
   {
      int j;
      // now go through and light all the areas
      for( j = 0; j < boxList.size(); j++ )
      {
         if( lightingOptions.mFlags.test( LightingOptions::Console ) )
            printf( " Lighting block(%d/%d):\n", j+1, boxList.size() );
         buildLightMap( boxList[j] );
      }
   
      // fill in the edges of the blocks
      Point2I size = renderImage.gridFile->getSize();
      for( pos.y = 0; pos.y < size.y; pos.y++ )
      {
         for( pos.x = 0; pos.x < size.x; pos.x++ ) 
         {
            GridBlock * block = renderImage.gridFile->getBlock( pos );
            if(pos.x)
            {
               // copy the left most column into the rightmost
               // column of the previous block.
               Point2I posprev(pos.x-1, pos.y);
               GridBlock *prevBlock = renderImage.gridFile->getBlock(posprev);
               UInt16 *lm = block->getLightMap();
               UInt16 *pm = prevBlock->getLightMap();
               int stride = block->getLightMapWidth();
               int i;
               pm += stride-1;
               for(i = 0; i < stride; i++)
               {
                  *pm = *lm;
                  pm += stride;
                  lm += stride;
               }
            }
            if(pos.y)
            {
               // copy the left most column into the rightmost
               // column of the previous block.
               Point2I posprev(pos.x, pos.y-1);
               GridBlock *prevBlock = renderImage.gridFile->getBlock(posprev);
               UInt16 *lm = block->getLightMap();
               UInt16 *pm = prevBlock->getLightMap();
               int stride = block->getLightMapWidth();
               int i;
               pm += (stride-1) * stride;
               for(i = 0; i < stride; i++)
                  *pm++ = *lm++;
            }
         }
      }
      
      // only continue if there is hires   
      if(!lightingOptions.mFlags.test(LightingOptions::AllowHires) && !i)
         break;
      
      // walk through all the blocks and save off the lightmaps 
      GridBlockList * bList = renderImage.gridFile->getBlockList();
      j = 0;
      for(GridBlockList::iterator itr = bList->begin(); itr != bList->end(); itr++, j++)
      {
         GridBlockListElem & elm = **itr;

         // check which pass this is         
         if(!i)
         {
            // copy the lightmap
            int size = elm.block->getLightMapWidth() * elm.block->getLightMapWidth();
            UInt16 * map = new UInt16[size];
            memcpy(map, elm.block->getLightMap(), size * sizeof(UInt16));
            mapData.push_back(map);

            // clear the flag
            lightingOptions.mFlags.clear(LightingOptions::AllowHires);
         }
         else
         {
            // create the hires info
            elm.block->setHiresLightMap(mapData[j]);
            
            // set the flag back
            lightingOptions.mFlags.set(LightingOptions::AllowHires);   
         }
      }
   }
}

//----------------------------------------------------------------------------

void SimTerrain::buildLightMap(const Box2F& area )
{
	//
   SimLightSet *simLightSet = 
      dynamic_cast<SimLightSet *>(manager->findObject(SimLightSetId));
	//
   if (simLightSet) {
	   TSSceneLighting* sceneLights = simLightSet->getSceneLighting();
 		sceneLights->lock();
		sceneLights->prepare(SphereF(Point3F(0,0,0), 0) , renderImage.transform);
		ambientLight = sceneLights->getAmbientIntensity();

		float hh = renderImage.gridFile->getHeightRange().length();

		//
		for (TSSceneLighting::iterator ptr = sceneLights->begin();
				ptr != sceneLights->end(); ptr++) {
			lightList.increment();
			Light& ll = lightList.last();
			ll.light = *ptr;

			// Turn off light optimization, the large size
			// of the terrain messes with this feature.
			ll.light->fLight.fOptimized = 1;

			switch (ll.light->fLight.fType) {
				case TS::Light::LightDirectionalWrap:
				case TS::Light::LightDirectional: {
					// Scale the vector to cover the maximun
					// ground distance possible for occlusion
					ll.vector = ll.light->fLight.fDirection;
					ll.vector *= abs(hh / ll.light->fLight.fDirection.z);
					ll.vector.neg();
					break;
				}
				case TS::Light::LightPoint:
				case TS::Light::LightParallelPoint:
				case TS::Light::LightCone:
					// These lights have positions.
					break;
			}
		}

		// Convert area into File coor.
		Box3F a1,a2;
		a1.fMin.set(area.fMin.x,area.fMin.y,0.0f);
		a1.fMax.set(area.fMax.x,area.fMax.y,0.0f);
		m_mul(a1.fMin,renderImage.inverseTransform,&a2.fMin);
		m_mul(a1.fMax,renderImage.inverseTransform,&a2.fMax);
		Box2F a3;
		a3.fMin.set(a2.fMin.x,a2.fMin.y);
		a3.fMax.set(a2.fMax.x,a2.fMax.y);
		lightArea(a3);

		//
		sceneLights->unlock();
		lightList.clear();
   }
}

//----------------------------------------------------------------------------

int SimTerrain::LightInfoStrip::mNumHiresSkipped   = 0;
int SimTerrain::LightInfoStrip::mNumHires          = 0;
Vector<Box2F> SimTerrain::LightInfoStrip::mHiresBounds;

//----------------------------------------------------------------------------

SimTerrain::LightInfoStrip::LightInfoStrip() :
   mInfoList( NULL ), 
   mWidth(0),
   mScale(0),
   mOffset(0.f),
   mTerrain(NULL)
{
   mPos.set(0,0);
}

//----------------------------------------------------------------------------

SimTerrain::LightInfoStrip::LightInfo::LightInfo() :
   mFlags(0),
   mBlockId(0)
{
   mCol.set( 0.f, 0.f, 0.f );
}

//----------------------------------------------------------------------------

SimTerrain::LightInfoStrip::LightInfo::~LightInfo()
{
}

//----------------------------------------------------------------------------

SimTerrain::LightInfoStrip::~LightInfoStrip()
{
   delete [] mInfoList;
}

//----------------------------------------------------------------------------

void SimTerrain::LightInfoStrip::fillHiresBounds()
{
   SimGroup * grp = NULL;
   SimObject * obj = NULL;

   // get to the volumes' group
   if((obj = mTerrain->getManager()->findObject("MissionGroup")) == NULL)
      return;
   if((grp = dynamic_cast<SimGroup *>(obj)) == NULL)
      return;

   SimTerrain::Context context = mTerrain->getContext();

   // fill in all the directional lights
   for( SimSetIterator itr( grp ); *itr; ++itr )
   {
      SimContainer * cont = dynamic_cast<SimContainer *>(*itr);
      if( !cont )
         continue;
         
      if( cont->getType() & mTerrain->interiorMask )
      {
         Box3F bBox = cont->getBoundingBox();

         // walk through all the lights..
         for( LightList::iterator ll_itr = mTerrain->lightList.begin(); ll_itr != mTerrain->lightList.end(); ll_itr++ )
         {
   	      Light & ll = *ll_itr;
      
            switch( ll.light->fLight.fType )
            {
               case TS::Light::LightDirectionalWrap:
               case TS::Light::LightDirectional:
               {
                  Point3F  vec;
                  Box2F    terrainBox;
                  Point2F  col[8];

                  vec = ll.vector;
                  vec.normalize();
            
                  bool missed = false;
      
                  int i;            
                  for( i = 0; i < 8; i++ )
                  {
                     Point3F  start;
                     Point3F  end;
                     
                     start.x = i & 0x01 ? bBox.fMin.x : bBox.fMax.x;
                     start.y = i & 0x02 ? bBox.fMin.y : bBox.fMax.y;
                     start.z = i & 0x04 ? bBox.fMin.z : bBox.fMax.z;

                     start.x -= context.pos.x;
                     start.y -= context.pos.y;
                     
                     start = vec * 2000.f + start;                     
                     end = (-vec) * 4000.f + start;
                     
                     GridCollision collision( mTerrain->renderImage.gridFile );
                     if( collision.collide( start, end, true, true ) )
                        col[i] = collision.surface.position;
                     else
                        missed = true;
                  }
                  
                  if( missed )
                     continue;
                     
                  Box2F bounds( 100000.f, 100000.f, -100000.f, -100000.f );

                  for( i = 0; i < 8; i++ )
                  {
                     if( col[i].x < bounds.left() )
                        bounds.left() = col[i].x;
                     if( col[i].x > bounds.right() )
                        bounds.right() = col[i].x;
                     if( col[i].y < bounds.top() )
                        bounds.top() = col[i].y;
                     if( col[i].y > bounds.bottom() )
                        bounds.bottom() = col[i].y;
                  }
                  
                  // add a border
                  bounds.left() -= mOffset;
                  bounds.right() += mOffset;
                  bounds.top() -= mOffset;
                  bounds.bottom() += mOffset;
                  
                  // add to the list
                  mHiresBounds.push_back( bounds );
            
                  break;
               }
            }
         }
      }
   }
   
   // now fill in the point lights
   for( LightList::iterator ll_itr = mTerrain->lightList.begin(); ll_itr != mTerrain->lightList.end(); ll_itr++ )
   {
      Light & ll = *ll_itr;
      Box2F bounds;
      
      switch( ll.light->fLight.fType )
      {
         case TS::Light::LightPoint:
         
            // the bounds are just the sqr with width of half range
            bounds.left() = ll.light->fLight.fPosition.x -
               ll.light->fLight.fRange - mOffset;
            bounds.right() = ll.light->fLight.fPosition.x +
               ll.light->fLight.fRange + mOffset;
            bounds.top() = ll.light->fLight.fPosition.y -
               ll.light->fLight.fRange - mOffset;
            bounds.bottom() = ll.light->fLight.fPosition.y +
               ll.light->fLight.fRange + mOffset;
               
            // add to the list
            mHiresBounds.push_back( bounds );

            break;
      }
   }
}

//----------------------------------------------------------------------------

void SimTerrain::LightInfoStrip::create( int width, int scale, float offset, SimTerrain * terrain )
{
   mScale = scale;
   mOffset = offset;
   mTerrain = terrain;
   mWidth = width + 2;
   
   delete[] mInfoList;
   mInfoList = new LightInfo[ mWidth ];
}

//----------------------------------------------------------------------------

void SimTerrain::LightInfoStrip::fillStrip( Point2I & pos )
{
   mPos = pos;

   int scale = mTerrain->renderImage.gridFile->getScale();
   int groundShift = mTerrain->renderImage.gridFile->getBlockSizeShift() + scale;
   
   for( int i = 0; i < mWidth; i++ )
   {
      Point2F fPos;
      fPos.x = float( ( pos.x + i - 1 ) << mScale ) + mOffset;
      fPos.y = float( pos.y << mScale ) + mOffset;

      LightInfo & lightInfo = mInfoList[i];
      
      // clear the info
      lightInfo.mFlags = 0;
      lightInfo.mCol.set(0.f,0.f,0.f);
      
		CollisionSurface surfInfo;
      
      if( mTerrain->renderImage.gridFile->getSurfaceInfo( fPos, &surfInfo ) )
      {
         surfInfo.position.z += 0.1f;
         lightInfo.mFlags.set( LightInfo::Valid );
         
         lightInfo.mBlockId = surfInfo.part;
         lightInfo.mNormal = surfInfo.normal;
         lightInfo.mFilePos.x = fPos.x - mOffset;
         lightInfo.mFilePos.y = fPos.y - mOffset;
         lightInfo.mFilePos.z = surfInfo.position.z;
         
         // block pos
         lightInfo.mBlockPos.x = int( lightInfo.mFilePos.x - float( int(lightInfo.mFilePos.x) >> groundShift << groundShift ) ) >> scale;
         lightInfo.mBlockPos.y = int( lightInfo.mFilePos.y - float( int(lightInfo.mFilePos.y) >> groundShift << groundShift ) ) >> scale;

         // set this to false for no object collision color application ( still does object los )
         bool hitObject;
         bool allowShadow = !mTerrain->lightingOptions.mFlags.test( LightingOptions::AllowHires );
         lightInfo.mCol = mTerrain->calcIntensity( surfInfo.position, surfInfo.normal, hitObject, allowShadow, false );

         // check if needs to be hires'd
         for( int i = 0; i < mHiresBounds.size(); i++ )
            if( mHiresBounds[i].contains( surfInfo.position ) )
               lightInfo.mFlags.set( LightInfo::Hit, true );
      }
   }   
}

//----------------------------------------------------------------------------
// does set the flag for hires - so must be called before if you want hires
void SimTerrain::LightInfoStrip::loresStrip( LightInfoStrip * prevRow, LightInfoStrip * nextRow )
{
   LightInfo * prevList = prevRow ? prevRow->getInfoList() : NULL;
   LightInfo * nextList = nextRow ? nextRow->getInfoList() : NULL;
   
   for( int i = 1; i < ( mWidth - 1 ); i++ )
   {        
      LightInfo & currentInfo = mInfoList[i];
      
      int colCount = 0;
      ColorF colSum( 0.f, 0.f, 0.f );
      for( int y = -1; y < 2; y++ )
      {
         for( int x = -1; x < 2; x++ )
         {
            LightInfo * lightInfo;
            
            // select the info that we are wanting...
            if( y == -1 )
               lightInfo = prevList ? &prevList[i+x] : NULL;
            else if( y == 0 )
               lightInfo = &mInfoList[i+x];
            else 
               lightInfo = nextList ? &nextList[i+x] : NULL;
               
            // make sure a valid point
            if( lightInfo && lightInfo->mFlags.test( LightInfo::Valid ) )
            {
               // do the hires flag - move this into fillStrip
               if( lightInfo->mFlags.test( LightInfo::Hit ) )
                  currentInfo.mFlags.set( LightInfo::Hires );
               
               if( x <= 0 && y <= 0 )
               {
                  // do lores stuff...
                  colCount++;
                  colSum += currentInfo.mCol;
               }
            }
         }
      }
      
      if( colCount )
      {
   		float cc = 1.0f / colCount;
   		colSum.red = max(min(colSum.red * cc,1.0f),0.0f);
   		colSum.green = max(min(colSum.green * cc,1.0f),0.0f);
   		colSum.blue = max(min(colSum.blue * cc,1.0f),0.0f);
      }
      
      Point2F fPos;
      fPos.x = float( ( mPos.x + i - 1 ) << mScale ) + mOffset;
      fPos.y = float( mPos.y << mScale ) + mOffset;
      mTerrain->setColor( fPos, colSum );
   }
}
   
//----------------------------------------------------------------------------

void SimTerrain::LightInfoStrip::hiresStrip()
{
   int lastBlockId = -1;
   GridBlock * block;
   
   for( int i = 1; i < ( mWidth - 1 ); i++ )
   {        
      LightInfo & currentInfo = mInfoList[i];
      
      if( currentInfo.mFlags.test( LightInfo::Hires ) )
      {
         // check for a new gridblock
         if( lastBlockId != currentInfo.mBlockId )
         {
            block = mTerrain->renderImage.gridFile->getBlockList()->findBlock( currentInfo.mBlockId );
            lastBlockId = currentInfo.mBlockId;
         }
         
         float step = float( 1 << mScale ) / 17;
         float qtrStep = step / 4;
         bool hitObject = false;
         
         UInt16 lightMap[17*17];
         // sum the surrounding cols
         for( int y = 0; y < 17; y++ )
            for( int x = 0; x < 17; x++ )
            {
               ColorF col(0.f,0.f,0.f);
                  
               Point2F fPos( currentInfo.mFilePos.x + float( x ) * step + step / 2,
                  currentInfo.mFilePos.y + float( y ) * step + step / 2 );
         
               // subsample 2 x 2 + center...
               if( mTerrain->lightingOptions.mFlags.test( LightingOptions::SubSample ) )
               {
                  for( int j = -1; j <= 1; j+=2 )
                     for( int k = -1; k <= 1; k+=2 )
                     {
                        Point2F tPos( fPos.x + qtrStep * k, fPos.y + qtrStep * j );

                        // check if it hits...
                        CollisionSurface surfInfo;
                        mTerrain->renderImage.gridFile->getSurfaceInfo( tPos, &surfInfo );
                        
                        Point3F pos( tPos.x, tPos.y, surfInfo.position.z + 0.1f );
                        bool hit;
                        col += mTerrain->calcIntensity( pos, currentInfo.mNormal, hit, true, true );
                        hitObject |= hit;
                     }
                     
                  // average the cols...
                  col.red = max(min(col.red * 0.25f,1.0f),0.0f);
                  col.green = max(min(col.green * 0.25f,1.0f),0.0f);
                  col.blue = max(min(col.blue * 0.25f,1.0f),0.0f);
               }
               else
               {
                  // check if it hits...
                  CollisionSurface surfInfo;
                  mTerrain->renderImage.gridFile->getSurfaceInfo( fPos, &surfInfo );

                  Point3F pos( fPos.x, fPos.y, surfInfo.position.z + 0.1f);
                  bool hit;
                  col += mTerrain->calcIntensity( pos, currentInfo.mNormal, hit, true, true );
                  hitObject |= hit;
               }
               
               lightMap[y*17+x] = ITRLighting::buildColor( col.red, col.green, col.blue );
            }

         // check if hit anything
         if( hitObject )
         {
            bool skipHires;
            if( mTerrain->lightingOptions.mFlags.test( LightingOptions::RemoveSolids ) )
            {
               skipHires = true;
               // check for the same color
               for( int j = 0; j < (17*17); j++ )
                  if( lightMap[j] != lightMap[0] )
                  {
                     skipHires = false;
                     break;
                  }
            }

            if( skipHires )
            {
               block->addHRLM( lightMap[0], currentInfo.mBlockPos );
               mNumHiresSkipped++;
            }
            else
            {
               block->addHRLM( lightMap, currentInfo.mBlockPos );
               mNumHires++;
            }
               
            if( mTerrain->lightingOptions.mFlags.test( LightingOptions::Console ) )
               printf( skipHires ? "x" : "." );
         }
      }
   }
}

//----------------------------------------------------------------------------

void SimTerrain::lightArea( const Box2F & area )
{
   // get the interior mask...
   interiorMask = Console->getIntVariable( "$SimTerrain::InteriorMask", SimInteriorObjectType );
   
   int lightScale = 0;
   float filterRadius = 1.0f;
   int scale = renderImage.gridFile->getScale() - lightScale;

	// Area in grid light map coor.
	Box2I iarea;
	iarea.fMin.x = int(area.fMin.x) >> scale;
	iarea.fMin.y = int(area.fMin.y) >> scale;
	iarea.fMax.x = int(area.fMax.x) >> scale;
	iarea.fMax.y = int(area.fMax.y) >> scale;
	float offset = float(1 << (scale - 1)) * filterRadius;

   int width = iarea.fMax.x - iarea.fMin.x + 1;
   int height = iarea.fMax.y - iarea.fMin.y + 1;

   LightingOptions & opts = lightingOptions;

   // need three strips
   LightInfoStrip * strips[3];
   int i;
   for( i = 0; i < 3; i++ )
   {
      strips[i] = new LightInfoStrip;
      
      strips[i]->create( width, scale, offset, this );

      // just need this once
      if( i == 0 )
         strips[0]->fillHiresBounds();
   
      strips[i]->fillStrip( Point2I( iarea.fMin.x, iarea.fMin.y + i - 1 ) );
   }

   // walk down the area
   for( i = 0; i < height; i++ )
   {
      // test against the previous and the next...
      strips[1]->loresStrip( strips[0], strips[2] );

      if( opts.mFlags.test( LightingOptions::AllowHires ) )
         strips[1]->hiresStrip();
            
      // swap them around and fill the last one...   
      LightInfoStrip * tmp = strips[0];
      strips[0] = strips[1];
      strips[1] = strips[2];
      strips[2] = tmp;
      tmp->fillStrip( Point2I( iarea.fMin.x, iarea.fMin.y + i + 2 ) );
   }
   
   delete strips[0];
   delete strips[1];
   delete strips[2];
   
   // show some stats...
   if( opts.mFlags.test( LightingOptions::Console ) &&
      opts.mFlags.test( LightingOptions::AllowHires ) )
   {
      printf( "\n" );
      printf( "  - Number of hires lightmaps generated: %d\n", LightInfoStrip::mNumHires );
      printf( "  - Number of hires lightmaps skipped: %d\n", LightInfoStrip::mNumHiresSkipped );
   }
}

//----------------------------------------------------------------------------

ColorF SimTerrain::calcIntensity( const Point3F& pos, const Point3F& normal, bool & hitObject, bool allowShadow, bool hiresRun )
{
   ColorF colorSum( 0.f, 0.f, 0.f );

   hitObject = false;   
   LightingOptions & opts = lightingOptions;

   // walk through all the lights..
   for( LightList::iterator itr = lightList.begin(); itr != lightList.end(); itr++ )
   {
   	Light & ll = *itr;
      bool directional = false;
      Point3F end;
      Point3F vec;
      
      switch( ll.light->fLight.fType )
      {
         case TS::Light::LightDirectionalWrap:
         case TS::Light::LightDirectional:

            directional = true;
            vec = ll.vector;
            vec.normalize();
            
            // adjust the end the same way the interiors do...
            end = vec * 2000.f + pos;
            
            break;
            
         case TS::Light::LightPoint:
         case TS::Light::LightParallelPoint:
         case TS::Light::LightCone:
         
            end = ll.light->fLight.fPosition;
            
            break;   
      }  
      
      ColorF currentColor( 0.f, 0.f, 0.f );

      bool hit = false;
         
      // needs to hit the terrain
      if( los( pos, end ) )
      {
         // check the interiors for collision
         if( opts.mFlags.test( LightingOptions::InteriorLOS ) )
         {
            SimCollisionInfo info;
            SimContainerQuery query;

            query.id = getId();
            query.type = -1;
            query.box.fMin = pos + context.pos;
            query.box.fMax = end + context.pos;
            query.mask = interiorMask;
            SimContainer * root = findObject( manager, SimRootContainerId, root );
            

            // do the mission los and the callback if a directional
            if( root->findLOS( query, &info ) )
               hit = true;
            else if( directional && opts.mFlags.test( LightingOptions::CloudMap ) && allowShadow && !hiresRun )
               opts.mCollideCallback( opts.mCallbackObj, query.box.fMin, query.box.fMax, currentColor );
         }
         
         float scale = 0.f;
         bool applyLight = false;
         
         if( directional )
         {
            if( !allowShadow || !hit )
            {
               scale = fabs( m_dot( normal, vec ) );
               applyLight = true;
            }
         }
         else
         {
            Point3F diff = ( end - pos );
            float length = diff.len();
            
            if( ll.light->fLight.fRange > length )
            {
               scale = ( ll.light->fLight.fRange - length ) /
                  ll.light->fLight.fRange;
               if( !hit || !allowShadow )
                  applyLight = true;
            }
            else
               hit = false;
         }         

         // check if the color should be applied
         if( applyLight )
         {
            currentColor.red = scale * ll.light->fLight.fRed;
            currentColor.green = scale * ll.light->fLight.fGreen;
            currentColor.blue = scale * ll.light->fLight.fBlue;
         }
      }
      
      colorSum += currentColor;
      hitObject |= hit;
   }
   
   colorSum += ambientLight;
   
   return( colorSum );
}

//----------------------------------------------------------------------------

void SimTerrain::setColor(const Point2F& pos,const ColorF& color)
{
	// Position in file coord.
	GridBlockInfo info;
	if (renderImage.gridFile->getBlockInfo(pos,&info)) {
		// Convert to 16bit color
		float inten = color.red * .22 + color.green * .67 + color.blue * .11;
		if(inten > 1.0f)
			inten = 1.0f;
		UInt16 c = int(inten * 65535) & 0xF000;
		c |= int(color.red * 15.9) << 8;
      c |= int(color.green * 15.9) << 4;
		c |= int(color.blue * 15.9);

		//
		Point2I lpos;
		int scale = renderImage.gridFile->getScale() + info.block->getLightScale();
		lpos.x = int(info.pos.x) >> scale;
		lpos.y = int(info.pos.y) >> scale;
		UInt16* lmap = info.block->getLightMap();
		int width = info.block->getLightMapWidth();
		lmap[(width * lpos.y) + lpos.x] = c;
		info.block->setDirtyFlag();

		// Patch up adjoining blocks
		if (!lpos.x) {
			Point2F bpos = pos;
			bpos.x -= float(1 << scale);
			if (renderImage.gridFile->getBlockInfo(bpos,&info)) {
				info.pos.x += float(1 << scale);
				int scale = renderImage.gridFile->getScale() + info.block->getLightScale();
				lpos.x = int(info.pos.x) >> scale;
				lpos.y = int(info.pos.y) >> scale;
				UInt16* lmap = info.block->getLightMap();
				int width = info.block->getLightMapWidth();
				lmap[(width * lpos.y) + lpos.x] = c;
				info.block->setDirtyFlag();
			}
		}
		if (!lpos.y) {
			Point2F bpos = pos;
			bpos.y -= float(1 << scale);
			if (renderImage.gridFile->getBlockInfo(bpos,&info)) {
				info.pos.y += float(1 << scale);
				int scale = renderImage.gridFile->getScale() + info.block->getLightScale();
				lpos.x = int(info.pos.x) >> scale;
				lpos.y = int(info.pos.y) >> scale;
				UInt16* lmap = info.block->getLightMap();
				int width = info.block->getLightMapWidth();
				lmap[(width * lpos.y) + lpos.x] = c;
				info.block->setDirtyFlag();
			}
		}
	}
}


//----------------------------------------------------------------------------
// IO
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAGS( SimTerrain, FOURCC('S','T','E','R'), SimTerrainPersTag );

int SimTerrain::version()
{
	return CurrentFileVersion;
}

DWORD SimTerrain::packUpdate(Net::GhostManager* /*gm*/, DWORD mask, BitStream *stream)
{
   if(mask & TedFileMask)
   {
      stream->writeFlag(true);
      stream->writeString(tedFile);
   }
   else
      stream->writeFlag(false);
   if(mask & VisibilityMask)
   {
      stream->writeFlag(true);
		stream->write(sizeof(maxVisibility),&maxVisibility);
   }
   else
      stream->writeFlag(false);

   if(mask & DetailMask)
   {
      stream->writeFlag(true);
	   stream->write(sizeof(maxDetail),&maxDetail);
   }
   else
      stream->writeFlag(false);

   if (mask & ContextMask) {
      stream->writeFlag(true);
	   stream->write(sizeof(context), &context);
   }
   else
      stream->writeFlag(false);

   if (mask & MovementMask) {
      stream->writeFlag(true);
	   stream->write(sizeof(ContainerProperties), &containerProperties);
   }
   else
      stream->writeFlag(false);

   stream->writeFlag(suspendRender);
   
   if(mask & CrcMask)
   {
      stream->writeFlag(true);
      stream->write(sizeof(UInt32),&terrainCrc);
   }
   else
      stream->writeFlag(false);
      
   return 0;
}

void SimTerrain::unpackUpdate(Net::GhostManager* /*gm*/, BitStream *stream)
{
   if(stream->readFlag())
   {
      tedFile = stream->readSTString();
      load(tedFile);
   }
   if(stream->readFlag())
		stream->read(sizeof(maxVisibility),&maxVisibility);
   if(stream->readFlag())
	   stream->read(sizeof(maxDetail),&maxDetail);
   if (stream->readFlag())
	   stream->read(sizeof(context), &context);
   if (stream->readFlag())
	   stream->read(sizeof(ContainerProperties), &containerProperties);
   
   suspendRender = stream->readFlag();

   if(stream->readFlag())
      stream->read(sizeof(UInt32), &terrainCrc);
      
   applyUserPrefs();
}

void SimTerrain::initPersistFields()
{
   //Parent::initPersistFields();
   addField("tedFileName", TypeString, Offset(tedFile, SimTerrain));
   addField("visibleDistance", TypeFloat, Offset(maxVisibility.visibleDist, SimTerrain));
   addField("hazeDistance", TypeFloat, Offset(maxVisibility.hazeDist, SimTerrain));
   addField("hazeVerticalMin", TypeFloat, Offset(maxVisibility.hazeVerticalMin, SimTerrain));
   addField("hazeVerticalMax", TypeFloat, Offset(maxVisibility.hazeVerticalMax, SimTerrain));
   addField("perspectiveDistance", TypeFloat, Offset(maxDetail.perspectiveDist, SimTerrain));
   addField("screenSize", TypeFloat, Offset(maxDetail.screenSize, SimTerrain));
   addField("contGravity", TypePoint3F, Offset(containerProperties.gravity, SimTerrain));
   addField("contDrag", TypeFloat, Offset(containerProperties.drag, SimTerrain));
   addField("contHeight", TypeFloat, Offset(containerProperties.height, SimTerrain));
   addField("position", TypePoint3F, Offset(context.pos, SimTerrain));
   addField("rotation", TypePoint3F, Offset(context.rot, SimTerrain));
}

Persistent::Base::Error 
SimTerrain::read(StreamIO &s,int version,int b)
{
	SimContainer::read(s,version,b);
	s.read(sizeof(context),&context);
   if ( version >= 2)
		s.read(sizeof(maxVisibility),&maxVisibility);
	else {
		s.read(sizeof(float),(void*)&maxVisibility.visibleDist);
		s.read(sizeof(float),(void*)&maxVisibility.hazeDist);
		visibility.hazeVerticalMin = DefTerrainVisibility.hazeVerticalMin;
		visibility.hazeVerticalMax = DefTerrainVisibility.hazeVerticalMax;
	}
	s.read(sizeof(maxDetail),&maxDetail);

   tedFile = s.readSTString();

	if (version >= 3) {
	   s.read(sizeof(ContainerProperties), &containerProperties);
	}

	return (s.getStatus() == STRM_OK)? Ok: ReadError;
}

Persistent::Base::Error 
SimTerrain::write(StreamIO &s,int version,int b)
{
	SimContainer::write(s,version,b);
	s.write(sizeof(context),&context);
	s.write(sizeof(maxVisibility),&maxVisibility);
	s.write(sizeof(maxDetail),&maxDetail);

	s.writeString(tedFile);

	if (version >= 3) {
	   s.write(sizeof(ContainerProperties), &containerProperties);
	}

	return (s.getStatus() == STRM_OK)? Ok: WriteError;
}


//----------------------------------------------------------------------------
// Image classes
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

bool gRenderSimTerrain = true;


void SimTerrain::RenderImage::render( TSRenderContext &rc )
{
   if ( bool(gridFile) && gridFile->getMaterialList() )
   {
      TSCamera *camera = rc.getCamera();
   	camera->pushTransform(transform);

      float screenSize = gridRender.getScreenSize();
      if(camera->getCameraType() == TS::OrthographicCameraType)
         gridRender.setScreenSize(0.01f);
      else if(terrainFile && !gRenderSimTerrain)
      {
	      terrainFile->setVisibleDistance(myTerrain->getVisibleDistance());
         terrainFile->setHazeDistance(myTerrain->getHazeDistance());
         terrainFile->setScreenSize(gridRender.getScreenSize());
         terrainFile->render(rc);
         camera->popTransform();
         return;
   	}
      gridRender.setHeightRange(gridFile->getHeightRange());
		gridRender.setScale(gridFile->getScale());
   	gridRender.setContext(rc);
   	gridRender.render(gridFile);

      if(camera->getCameraType() == TS::OrthographicCameraType)
         gridRender.setScreenSize(screenSize);

   	camera->popTransform();
   }
}
