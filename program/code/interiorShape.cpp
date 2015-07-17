//------------------------------------------------------------------------------
// Description: 
//    
// $Workfile: $
// $Revision: $
// $Author:   $
// $Modtime:  $
//
//------------------------------------------------------------------------------
#include <console.h>
#include <itrShape.h>
#include <itrMetrics.h>
#include <simShape.h>
#include "editor.strings.h"
#include "commonEditor.strings.h"
#include "darkstar.strings.h"
#include "simResource.h"
#include "console.h"
#include "simTimerGrp.h"
#include "interiorShape.h"
#include "fearDcl.h"
#include "dataBlockManager.h"
#include "fearDynamicDataPlugin.h"
#include "stringTable.h"
#include "PlayerManager.h"
#include "netGhostManager.h"
#include "g_surfac.h"
#include "gOGLSfc.h"

IMPLEMENT_PERSISTENT_TAG(InteriorShape, InteriorShapePersTag);

const float InteriorShape::csm_boxContainerWidth = 6.0f;
const Int32 InteriorShape::csm_fileVersion = 2;
UInt32 InteriorShape::sm_minLightUpdateMS  = 67;  // default to 15 hz...

//extern bool g_oglDisableLightUpdates;
//extern bool g_prefOGLOverrideLights;

namespace {

Point3F
translateToInspectRotation(const Point3F& in_rRot)
{
   Point3F retVal;

   retVal.x = (float)floor((double(in_rRot.x) * 180.0 / M_PI) + 0.5);
   retVal.y = (float)floor((double(in_rRot.y) * 180.0 / M_PI) + 0.5);
   retVal.z = (float)floor((double(in_rRot.z) * 180.0 / M_PI) + 0.5);

   return retVal;
}

EulerF
translateFromInspectRotation(const Point3F in_rRot)
{
   EulerF retVal;

   retVal.x = float(double(in_rRot.x) * M_PI / 180.0);
   retVal.y = float(double(in_rRot.y) * M_PI / 180.0);
   retVal.z = float(double(in_rRot.z) * M_PI / 180.0);

   return retVal;
}

} // namespace {}


//-------------------------------------- InteriorShape database class declarations.
//                                        non-PVS'ed interiors use the Simple version
//
class InteriorShapeDatabaseSimple : public SimDefaultContainerDatabase
{
   typedef SimDefaultContainerDatabase Parent;

   InteriorShape* m_pInteriorShape;

  public:
   InteriorShapeDatabaseSimple(InteriorShape*);
   ~InteriorShapeDatabaseSimple();

   void render(SimContainerRenderContext& rq);
   void scope(SimContainerScopeContext& sc);
};

class InteriorShapeDatabasePVS : public SimContainerDatabase
{
   typedef SimContainerDatabase Parent;

  private:
   struct ObjectEntry {
      SimContainer* pContainer;
      UInt32        bspLeaf;
   };
   typedef Vector<ObjectEntry> PVSContainerList;

   struct BSPObjectEntry {
      UInt16        back;
      UInt16        next;
      SimContainer* pContainer;
      Int32         leafIndex;
   };
   typedef Vector<BSPObjectEntry> BSPObjectList;

   struct BSPNodeEntry {
      UInt16   start;
      UInt16   count;
   };
   typedef Vector<BSPNodeEntry> BSPNodeList;

   PVSContainerList  m_objectList;
   BSPObjectList     m_bspObjectList;
   BSPNodeList       m_bspNodeList;
   UInt16            m_freeList;

   void enterBSPObject(ObjectEntry& in_rEntry);
   void removeBSPObject(ObjectEntry& in_rEntry);

   InteriorShape* m_pInteriorShape;
   ITRGeometry*      m_pDatabaseGeometry;
   Box3F             m_interiorBox;

   Int32 findBSPLeaf(SimContainer*);
   Int32 findBSPLeaf(const Point3F& p);
   Int32 findLeaf(const Point3F& p, Int32 nodeIndex = 0);

   void scopeNode(SimContainerScopeContext& sc,
                  const BSPNodeEntry&       in_rEntry);
   void renderNode(SimContainerRenderContext& sc,
                   const BSPNodeEntry&        in_rEntry);

  public:
   InteriorShapeDatabasePVS(InteriorShape*);
   ~InteriorShapeDatabasePVS();

   bool add(SimContainer*);
   bool remove(SimContainer*);
   void update(SimContainer*);
   bool removeAll(SimContainer*);

   bool findContained(const SimContainerQuery&,
                      SimContainerList*);
   SimContainer* findOpenContainer(const SimContainerQuery&);
   bool          findIntersections(const SimContainerQuery&,
                                   SimContainerList*);

   void render(SimContainerRenderContext& rq);
   void scope(SimContainerScopeContext& rq);
};


//--------------------------------------
// File scope helpers...
//
const float InteriorShape::csm_BBoxExtension = 3.0f;
namespace {

const float sg_BBoxTolerance  = 0.001;

inline bool
testFaces(const float in_myMin,    const float in_myMax,
          const float in_otherMin, const float in_otherMax)
{
   // Since this is *not* a time critical function, we'll just do this nice and
   //  simple...
   //
   if ((fabs(in_myMin - in_otherMin) <= sg_BBoxTolerance) &&
       (fabs(in_myMax - in_otherMax) <= sg_BBoxTolerance))
      return true;
   if (in_myMin <= in_otherMin &&
       in_myMax <= in_otherMax &&
       in_myMax > (in_otherMin + 1))
      return true;
   if (in_myMax >= in_otherMax &&
       in_myMin >= in_otherMin &&
       in_myMin < (in_otherMax - 1))
      return true;
   if (in_myMin > in_otherMin &&
       in_myMax < in_otherMax)
      return true;
   if (in_myMin < in_otherMin &&
       in_myMax > in_otherMax)
      return true;

   return false;
}

}; // namespace {}

class LinkVertexArena {
  private:
   InteriorShape* m_pInterior0;
   InteriorShape* m_pInterior1;

   ItrLinkFace m_interior0LinkFace;
   ItrLinkFace m_interior1LinkFace;

  public:
   // LinkVertexArenas work through reference counting, so we make it impossible for
   //  outside objects to delete the class.  This also has the effect of making it
   //  mandatory to allocate the object dynamically
   //
  private:
   ~LinkVertexArena();
  public:
   LinkVertexArena();

   void addInterior(InteriorShape*    io_pAdd,
                    const ItrLinkFace       in_linkFace);
   void removeInterior(InteriorShape* io_pRemove);

   int getIndexKey(InteriorShape* io_pCompare) const {
      AssertFatal(m_pInterior0 != NULL && m_pInterior1 != NULL,
                  "Cannot execute wo/ full arena membership");

      if (io_pCompare == m_pInterior0)
         return 0;
      else if (io_pCompare == m_pInterior1)
         return 1;
      else {
         AssertFatal(0, "Interior is not a member of this arena");
         return -1;
      }
   }
};


//------------------------------------------------------------------------------
//--------------------------------------
// C'tors/D'tors
//--------------------------------------
//
InteriorShape::InteriorShape()
 : m_forceRender(0),
   m_scopeTag(0),
   m_shapeRotation(0, 0, 0),
   m_shapePosition(0, 0, 0),
	m_centerPos(0,0,0),
   m_pFileName(NULL)
{
   m_isContainer = Uninitialized;

   m_disableCollisions = false;

   type |= SimInteriorObjectType;

   containerMask = -1;
   netFlags.set(Ghostable | ScopeAlways);
}

InteriorShape::~InteriorShape()
{
   if (m_renderImage.currShape && m_lightThreads.size())
      shutdownLights();

   clearLinks();

   delete m_renderImage.currShape;
   m_renderImage.currShape = NULL;
}

//------------------------------------------------------------------------------
//--------------------------------------
// Location/BBox management
//--------------------------------------
//
void
InteriorShape::buildContainerBox(const TMat3F& in_rTrans,
                                    Box3F*        out_pBox)
{
   out_pBox->fMin.set(0, 0, 0);
   out_pBox->fMax.set(0, 0, 0);
   if (getInstance() == NULL)
      return;

   // Here we should send unlinkMe messages to all interiors we are linked to
   //  and clear our link database.
   //
   if (isOpen() == true && getContainer() != NULL)
      clearLinks();
   
   Box3F box0 = getInstance()->getGeometry()->box;
   m_mul(box0, in_rTrans, out_pBox);

//   if (getInstance()->isLinked() == true) {
      out_pBox->fMax += Point3F(csm_BBoxExtension, csm_BBoxExtension, csm_BBoxExtension);
      out_pBox->fMin -= Point3F(csm_BBoxExtension, csm_BBoxExtension, csm_BBoxExtension);
//   }

   // And now recollide with the container system, and relink to all contacted
   //  bounding boxes...
   //
   if (isOpen() == true && getContainer() != NULL)
      findAllLinks();
}


//------------------------------------------------------------------------------
//--------------------------------------
// Resource management
//--------------------------------------
//
void
InteriorShape::setTeam(int in_team)
{
   Parent::setTeam(in_team);

   // we only care about this if we're a ghost...
   if (isGhost() == false || manager == NULL)
      return;

   remapTeamTextures(in_team);
}

void
InteriorShape::remapTeamTextures(int in_team)
{
   AssertFatal(isGhost() == true, "error, only ghosts may run this...");
   if (m_renderImage.currShape == NULL)
      return;

   // First, determine the name of the new dml/bmp prefix by grabbing the name of the
   //  team.
   //
   PlayerManager* pFPM = PlayerManager::get(manager);
   AssertFatal(pFPM != NULL, "Error, no player manager?");

   const char* pTeamName;
   PlayerManager::TeamRep* pTeam = pFPM->findTeam(in_team);
   if (pTeam == NULL || pTeam->skinBase == NULL || strcmp(pTeam->skinBase, "") == 0) {
      pTeamName = "base";
   } else {
      pTeamName = pTeam->skinBase;
   }

   char dmlName[1024];
   if (stricmp(pTeamName, "base") == 0) {
      strcpy(dmlName, m_renderImage.currShape->getShape()->getMaterialListFileName());
   } else {
      sprintf(dmlName, "%s_%s",
              pTeamName,
              m_renderImage.currShape->getShape()->getMaterialListFileName());
   }

   ResourceManager* rm = SimResource::get(manager);
   ResourceObject*  ro = rm->find(dmlName);
   
   if (ro == NULL || ro->resource == NULL) {
      // Ok, the DML hasn't be added to the manager.  Set up the
      //  dml as a copy of the base, and replace any fileNames necessary...
      //
      TS::MaterialList* newML = new TS::MaterialList();

      Resource<TS::MaterialList> rOriginal = rm->load(m_renderImage.currShape->getShape()->getMaterialListFileName(), true);
      AssertFatal(bool(rOriginal) == true, "Error, base material list not loaded");
      *newML = *rOriginal;

      for (int i = 0; i < newML->getMaterialsCount(); i++) {
         // We only care about texture materials...
         //
         TS::Material *material = &newML->getMaterial(i);
         if ((material->fParams.fFlags & TS::Material::MatFlags) != TS::Material::MatTexture)
            continue;
         
         static char baseName[64];
         if (strncmp("base.", material->fParams.fMapFile, 5) == 0) {
            sprintf(baseName, "%s.%s", pTeamName, material->fParams.fMapFile + 5);
            if (rm->findFile(baseName) == true)
               strcpy(material->fParams.fMapFile, baseName);
         }
      }
      rm->add(ResourceType::typeof(dmlName), dmlName, (void *)newML);
   }

   Resource<TS::MaterialList> newMatList = rm->load(dmlName, true);
   newMatList->load(*rm, true);

   m_renderImage.currShape->setMaterialList(newMatList);
}

void
InteriorShape::initRenderImage()
{
   m_renderImage.pInteriorShape = this;
   m_renderImage.linkRendering     = false;
   m_renderImage.transform         = getTransform();

   // Make sure that the shape is in the proper state...
   //
   if (getInstance() != NULL) {
      getInstance()->setInteriorState(0);
   }
}

void
InteriorShape::setIsContainer(const bool in_isContainer)
{
   if (in_isContainer == true) {
      if (getDatabase() == NULL) {
         SimContainerDatabase* install;
         if (getInstance() == NULL) {
            install = new InteriorShapeDatabaseSimple(this);
         } else {
            install = new InteriorShapeDatabasePVS(this);
         }
         installDatabase(install);
      }
   } else {
      uninstallDatabase();
   }

   setMaskBits(InitialUpdate);
}

// checks to see if the filename signifies that the shape has been 
// processed by the mission lighter
bool
InteriorShape::missionLitName()
{
//   const char* ptr = m_pInteriorData->pShapeName;
   const char* ptr = m_pFileName;
   const char* end = ptr + strlen(m_pFileName);

   while( ( end != ptr ) && ( *end != '.' ) )
      end--;
   AssertFatal( end != ptr, "Invalid shape filename." );
   end--;   
   while( ( end != ptr ) && ( *end != '.' ) )
   {
      if( !isdigit( *end ) )
         return( false );
      end--;
   }
   
   // really a mission lit shape????
   return( ( *end == '.' ) ? true : false );
}

// converts a filename to its base filename ( un-missionlit ver )
void
InteriorShape::getBaseFilename( String & str )
{
   // check if not mission lit and return the filename
   if( !missionLitName() )
   {
      str = m_pFileName;
      return;
   }
   
   char * file = new char[ strlen( m_pFileName ) + 1 ];
   strcpy( file, m_pFileName );
   
   char * end = file + strlen( file );
   
   int count = 0;
   while( end != file )
   {
      if( *end == '.' )
         count++;
      if( count == 2 )
         break;
      end--;
   }
   AssertFatal( end != file, "Invalid shape filename." );
   
   // tack the extension back on and throw into the string
   sprintf( end, "%s", ITRShape::sm_pShapeFileExtension );
   str = file;
   delete [] file;
}

void
InteriorShape::setFileName(const char* in_pFileName)
{
   m_pFileName = stringTable.insert(in_pFileName);
}

bool
InteriorShape::initInteriorResources()
{
   setIsContainer(false);
   if (getInstance() != NULL) {
      delete m_renderImage.currShape;
      m_renderImage.currShape = NULL;
   }
   if (m_pFileName    == NULL ||
       m_pFileName[0] == '\0') {
      return false;
   }

   AssertFatal(manager != NULL, "Cannot load resources wo/ a manager...");
   ResourceManager* rm = SimResource::get(manager);
   
   Resource<ITRShape> resShape;
   bool missionLit;
   if( rm->findFile( m_pFileName ) )
   {
      missionLit = missionLitName();
      resShape = rm->load(m_pFileName, true);
   }
   else
   {
      if( !missionLitName() ) {
         return( false );
      }
      String base = String(m_pFileName);
      getBaseFilename( base );
      if( rm->findFile( base.c_str() ) )
         resShape = rm->load(base.c_str(), true );
      missionLit = false;
   }
   
   if (bool(resShape) == false) {
      initLightParams();
      return false;
   }

   // Otherwise we can move forward...
   //
   m_renderImage.currShape = new ITRInstance(rm, resShape, 0, missionLit, !isGhost() );
   initRenderImage();

   setPosition(getTransform(), true);

   if (m_isContainer == Uninitialized) {
      m_isContainer = IsContainer;
      if (m_renderImage.currShape->getHighestGeometry() != NULL) {
         Box3F box = m_renderImage.currShape->getHighestGeometry()->box;
         if (box.len_x() >= csm_boxContainerWidth ||
             box.len_y() >= csm_boxContainerWidth ||
             box.len_z() >= csm_boxContainerWidth) {
            m_isContainer = IsContainer;
         } else {
            m_isContainer = NotContainer;
         }
      }
   }

   setIsContainer(m_isContainer == IsContainer);
	m_mul(m_renderImage.currShape->getLowestCenterPt(),
		getTransform(),&m_centerPos);
   return true;
}


//------------------------------------------------------------------------------
//--------------------------------------
// Light animation functions
//--------------------------------------
//
void
InteriorShape::shutdownLights()
{
   AssertFatal(m_lightThreads.size() == 0 || getInstance() != NULL,
               "No instance, but we have threads?");
               
   for (int i = 0; i < m_lightThreads.size(); i++) {
      AssertFatal(m_lightThreads[i] != NULL, avar("No thread in slot %d", i));
      getInstance()->destroyLightThread(m_lightThreads[i]);
      m_lightThreads[i] = NULL;
   }
   m_lightThreads.clear();
   m_lightAnimParams.clear();
}

void
InteriorShape::initLightParams()
{
   if (getInstance() == NULL) {
      m_lightAnimParams.clear();
      return;
   }

   ITRLighting* pLighting = getInstance()->getLighting();

   m_lightAnimParams.setSize(pLighting->lightList.size());
   
   for (int i = 0; i < m_lightAnimParams.size(); i++) {
      ITRLighting::Light& rLight = pLighting->lightList[i];
      m_lightAnimParams[i].active = (rLight.animationFlags & ITRLIGHTANIM_AUTOSTART) != 0;
      m_lightAnimParams[i].time   = rLight.animationDuration;
      m_lightAnimParams[i].dummy1 = 0;
      m_lightAnimParams[i].dummy2 = 0;
   }
}

void
InteriorShape::restartLights()
{
   AssertFatal(m_lightAnimParams.size() == 0 || getInstance() != NULL,
               "No instance, but we need threads?");
   
   if (m_lightAnimParams.size() != 0) {
      ITRLighting* pLighting = getInstance()->getLighting();

      if (pLighting->lightList.size() != m_lightAnimParams.size())
         // ?? Change in the art, just throw away the saved states
         return;

      m_lightThreads.setSize(m_lightAnimParams.size());
      for (int i = 0; i < m_lightAnimParams.size(); i++) {
         const ITRLighting::Light& rLLight = pLighting->lightList[i];
         
         m_lightThreads[i] = getInstance()->createLightThread(rLLight.id);
         AssertFatal(m_lightThreads[i] != NULL, "No thread");

         float timeCoeff = (m_lightThreads[i]->getTotalAnimationTime() /
                            m_lightAnimParams[i].time);
         m_lightThreads[i]->setTimeScale(timeCoeff);
         m_lightThreads[i]->setTime(0.0f);
      }
   }
}


//------------------------------------------------------------------------------
//--------------------------------------
// Event/Query Processing
//--------------------------------------
//
bool
InteriorShape::processQuery(SimQuery* query)
{
   switch (query->type) {
      onQuery(SimRenderQueryImage);
      onQuery(SimCollisionImageQuery);
      onQuery(SimObjectTransformQuery);

     default:
      return Parent::processQuery(query);
   }
}


bool
InteriorShape::processEvent(const SimEvent* event)
{
   switch (event->type) {
      onEvent(SimEditEvent);
      onEvent(SimObjectTransformEvent);
      onEvent(SimRegisterTextureEvent);

     default:
      return Parent::processEvent(event);
   }
}

bool
InteriorShape::processArguments(int          argc,
                                const char** argv)
{
   if (argc < 1)
      return false;

   setFileName(argv[0]);
   if (initInteriorResources() == false) {
      Console->printf("Could not load interior: %s", argv[0]);
      return false;
   }

   shutdownLights();
   initLightParams();
   restartLights();
   
   return true;
}


//------------------------------------------------------------------------------
//--------------------------------------
// Event/Query Handlers
//--------------------------------------
//
bool
InteriorShape::onAdd()
{
   set(m_shapeRotation, m_shapePosition);

   shutdownLights();
   bool fInit = initInteriorResources();

   if (Parent::onAdd() == false)
      return false;

   // only verify if was able to init the resources (missioneditor)
   if(isGhost() == true) {
      if (fInit == false) {
         Net::setLastError(avar("You don't have a custom building the server is trying to load.  Check the Server Info for a possible download site.  Missing building file: %s.", m_pFileName));
         return(false);
      } else if (verifyCheckVals() == false) {
         Net::setLastError(avar("The server has a different version of a building file.  Check the Server Info for a possible download site.  Incorrect building file: %s.", m_pFileName));
         return(false);
      }
      setTeam(getTeam());
   } else if(fInit) {
      buildCheckVals();
   }

   addToSet(MoveableSetId);
   m_lastUpdatedTime = 0;

   initLightParams();
   restartLights();
   
   // We always add ourselves to the container system before seting our container
   //  state, which simplifies the setIsContainer() function...
   //
   SimContainer* root = findObject(manager,SimRootContainerId,root);
   root->addObject(this);
   
   // Link us to whoever's around...
   //
   if (isOpen() == true) {
      clearLinks();
      findAllLinks();
   }
   
   return true;
}

void
InteriorShape::clientProcess(DWORD in_curTime)
{
   if (m_lastUpdatedTime == 0) {
      m_lastUpdatedTime = in_curTime;
      return;
   }
       
   DWORD totalUpdateTime = 0;
   while (in_curTime > m_lastUpdatedTime) {
      // We increment the time in units of the minimum increment, until we're past
      //  the current time.
      //
      m_lastUpdatedTime += sm_minLightUpdateMS;
      totalUpdateTime   += sm_minLightUpdateMS;
   }
//   if (g_oglDisableLightUpdates == false || g_prefOGLOverrideLights == true) {
      if (totalUpdateTime != 0) {
         float floatTime = float(totalUpdateTime) / 1000.0f;
         for (int i = 0; i < m_lightThreads.size(); i++) {
         
            if (m_lightAnimParams[i].active == true)
               m_lightThreads[i]->advanceTime(floatTime);
         }
//      }
   }

   Parent::clientProcess(in_curTime);
}

bool
InteriorShape::onSimRegisterTextureEvent(const SimRegisterTextureEvent *event)
{
   ITRInstance* pInstance = getInstance();
   if (pInstance == NULL)
      return true;

   GFXSurface* pSurface = const_cast<GFXSurface*>(event->pSurface);
   if (dynamic_cast<OpenGL::Surface*>(pSurface) == NULL)
      return true;

   TSMaterialList* pMaterialList = pInstance->getMaterialList();
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

bool
InteriorShape::onSimEditEvent(const SimEditEvent* event)
{
   if (event->state == true) {
      m_forceRender |= ForceEdit;
   } else {
      m_forceRender &= ~ForceEdit;
   }

   setMaskBits(EditModeMask);
   return true;
}   

void
InteriorShape::set(const TMat3F& in_trans,
                       const bool    in_reextractAngles)
{
   if (in_reextractAngles == true) {
      m_shapePosition = in_trans.p;
      in_trans.angles(&m_shapeRotation);

      TMat3F newTrans(m_shapeRotation, m_shapePosition);

      setPosition(newTrans, true);
   } else {
      setPosition(in_trans, true);
   }

	// Maintain center pos in world space.
	if (m_renderImage.currShape)
		m_mul(m_renderImage.currShape->getLowestCenterPt(),
			in_trans,&m_centerPos);

   setMaskBits(PosRotMask);
}

void
InteriorShape::set(const EulerF& in_rot, const Point3F& in_pos)
{
   m_shapeRotation = in_rot;
   m_shapePosition = in_pos;

   set(TMat3F(in_rot, in_pos), false);
}

bool 
InteriorShape::onSimObjectTransformEvent(const SimObjectTransformEvent *event)
{           
   set(event->tmat);
   return true;
}

bool
InteriorShape::onSimObjectTransformQuery(SimObjectTransformQuery *query)
{
   query->centerOffset.set();
   query->tmat = getTransform();

   return true;   
}

bool
InteriorShape::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   if (m_renderImage.currShape != NULL) {
      query->image[query->count++] = &m_renderImage;
      m_renderImage.transform      = getTransform();

      // get haze value from container
      SimContainer * cnt = getContainer();
      if (cnt) {
         m_renderImage.hazeValue = 
         	cnt->getHazeValue(*query->renderContext,m_centerPos);
      }

      // Let the image know if this is a link render...
      //
      m_renderImage.linkRendering = ((m_forceRender & ForceLink) != 0);
      
   } else {
      query->count    = 0;
      query->image[0] = NULL;
   }

   return true;
}

bool
InteriorShape::onSimCollisionImageQuery(SimCollisionImageQuery* query)
{
   if (getInstance() != NULL && m_disableCollisions == false) {
      query->image[0] = &m_collisionImage;
      query->count    = 1;
      
      m_collisionImage.instance  = getInstance();
      m_collisionImage.transform = getTransform();
   } else {
      query->count    = 0;
      query->image[0] = NULL;
   }

   return true;
}


//------------------------------------------------------------------------------
//--------------------------------------
// Inspection (read/write)
//--------------------------------------
//
void
InteriorShape::inspectRead(Inspect *inspector)
{
   Parent::inspectRead(inspector);
   inspector->readDivider();

   //--------------------------------------Position/Rotation
   Point3F tempPos, tempRot;
   inspector->read(IDITG_POSITION, tempPos);
   inspector->read(IDITG_ROTATION, tempRot);
   EulerF newRot = translateFromInspectRotation(tempRot);
   set(newRot, tempPos);

   // We only process some of this event if the filename hasn't changed, and existed
   //  previous to the event...
   //
   bool processAll = getInstance() != NULL;
   bool processAny = getInstance() != NULL;

   int numAnimations = 0;
   if (processAll) {
      ITRLighting* pLighting = getInstance()->getLighting();
      numAnimations = pLighting->lightList.size();
   }

   // Shape name
   char tempBuf[256];
   inspector->read(IDITG_FILENAME, tempBuf);
   if (m_pFileName != NULL &&
       stricmp(m_pFileName, tempBuf) == 0) {
      // File is the same, do nothing...
      //
   } else if (strlen(tempBuf) == 0) {
      AssertISV(0, "We no longer allow empty interior shapes.");
   } else {
      setFileName(tempBuf);

      if (initInteriorResources() == true) {
         shutdownLights();
         initLightParams();
         restartLights();
         processAll = false;
      } else {
#pragma message "how to handle this?"
         processAll = false;
      }
   }
   
   inspector->read(IDITG_SIS_DISABLECOLLISIONS, m_disableCollisions);
   inspector->read(IDITG_SIS_ISCONTAINER,       m_isContainer);
   if (m_isContainer < -1 || m_isContainer > 1)
      m_isContainer = -1;

   if (processAll == true) {
      shutdownLights();

      // Read any light animations...
      ITRLighting* pLighting = getInstance()->getLighting();
      AssertFatal(numAnimations == pLighting->lightList.size(), "Lighting mismatch...");

      m_lightAnimParams.setSize(numAnimations);
      for (int i = 0; i < pLighting->lightList.size(); i++) {
         char* pName;
         int   index;
         LightAnimParam& rParams = m_lightAnimParams[i];
         
         inspector->readDivider();
         inspector->readStringIndexDes(pName, index);
         inspector->read(IDITG_SIS_ACTIVEINIT, rParams.active);
         inspector->read(IDITG_SIS_ANIMTIME,   rParams.time);
         rParams.dummy1 = 0;
         rParams.dummy2 = 0;
      }
      
      restartLights();
   } else if (processAny == true) {
      // We have to trash whatever was here before...
      //
      for (int i = 0; i < numAnimations; i++) {
         bool  dummyBool;
         float dummyFloat;
         char* dummyPChar;
         int   dummyInt;
         inspector->readDivider();
         inspector->readStringIndexDes(dummyPChar, dummyInt);
         inspector->read(IDITG_SIS_ACTIVEINIT, dummyBool);
         inspector->read(IDITG_SIS_ANIMTIME,   dummyFloat);
      }
   }
}

void InteriorShape::inspectWrite(Inspect* inspector)
{
   int i;
   Parent::inspectWrite(inspector);
   inspector->writeDivider();

   //--------------------------------------Position/Rotation
   inspector->write(IDITG_POSITION, getLinearPosition());
   Point3F inspRot = translateToInspectRotation(getAngulerPosition());
   inspector->write(IDITG_ROTATION, inspRot);

   // Dat fileName
   char buffer[256];
   strcpy(buffer, m_pFileName);
   inspector->write(IDITG_FILENAME, buffer);
   
   inspector->write(IDITG_SIS_DISABLECOLLISIONS, m_disableCollisions);
   inspector->write(IDITG_SIS_ISCONTAINER,       m_isContainer);

   if (getInstance() != NULL) {
      // Output any light animations...
      ITRLighting* pLighting = getInstance()->getLighting();
      m_lightAnimParams.setSize(pLighting->lightList.size());
      for (i = 0; i < pLighting->lightList.size(); i++) {
         ITRLighting::Light& rLight = pLighting->lightList[i];
         const char* pName = pLighting->getName(rLight.nameIndex);
      
         inspector->writeDivider();
         inspector->writeStringIndexDes(pName, i);
         inspector->write(IDITG_SIS_ACTIVEINIT, m_lightAnimParams[i].active);
         inspector->write(IDITG_SIS_ANIMTIME,   m_lightAnimParams[i].time);
      }
   }
}

//------------------------------------------------------------------------------
//--------------------------------------
// Persistent Functions
//--------------------------------------
//
void
InteriorShape::readInitialPacket(BitStream* stream)
{
   m_pFileName   = stream->readSTString();
   m_isContainer = stream->readSignedInt(2);

   stream->read(sizeof(Point3F), &m_shapePosition);
   stream->read(sizeof(EulerF),  &m_shapeRotation);

   readCheckVals(stream);

   // Set up the initial light states...
   shutdownLights();
   int size;
   stream->read(&size);
   m_lightAnimParams.setSize(size);
   stream->read(m_lightAnimParams.size() * sizeof(LightAnimParam),
                (void*)m_lightAnimParams.address());

   m_disableCollisions = stream->readFlag();

   if (manager != NULL) {
      initInteriorResources();
      restartLights();
   }
}

void
InteriorShape::writeInitialPacket(BitStream* stream)
{
   stream->writeString(m_pFileName);
   stream->writeSignedInt(m_isContainer, 2);

   stream->write(sizeof(Point3F), &m_shapePosition);
   stream->write(sizeof(EulerF),  &m_shapeRotation);

   writeCheckVals(stream);

   stream->write(m_lightAnimParams.size());
   stream->write(m_lightAnimParams.size() * sizeof(LightAnimParam),
                 (void*)m_lightAnimParams.address());

   stream->writeFlag(m_disableCollisions);
}


DWORD
InteriorShape::packUpdate(Net::GhostManager* gm,
                              DWORD              mask,
                              BitStream*         stream)
{
   UInt32 partialMask = Parent::packUpdate(gm, mask, stream);
   
   // Interior stuff
   //
   if (mask & InitialUpdate) {
      stream->writeFlag(true);
      writeInitialPacket(stream);
      return partialMask;
   } else {
      stream->writeFlag(false);
   }

   if ((mask & PosRotMask) != 0) {
      stream->writeFlag(true);
      stream->write(sizeof(Point3F), &m_shapePosition);
      stream->write(sizeof(EulerF),  &m_shapeRotation);
   } else {
      stream->writeFlag(false);
   }

   if ((mask & LightActiveMask) != 0) {
      stream->writeFlag(true);

      for (int i = 0; i < m_lightAnimParams.size(); i++) {
         stream->writeFlag(m_lightAnimParams[i].active);
      }
   } else {
      stream->writeFlag(false);
   }

   if ((mask & EditModeMask) != 0) {
      stream->writeFlag(true);
      stream->writeFlag((m_forceRender & ForceEdit) != 0);
   } else {
      stream->writeFlag(false);
   }

   return partialMask;
}

void
InteriorShape::unpackUpdate(Net::GhostManager* gm,
                                BitStream*         stream)
{
   Parent::unpackUpdate(gm, stream);
   
   // Interior stuff
   //
   if (stream->readFlag() == true) {
      readInitialPacket(stream);
      return;
   }
   
   if (stream->readFlag() == true) {
      Point3F tempPos;
      EulerF  tempRot;
      stream->read(sizeof(Point3F), &tempPos);
      stream->read(sizeof(EulerF),  &tempRot);
      set(tempRot, tempPos);
   }

   if (stream->readFlag() == true) {
      for (int i = 0; i < m_lightAnimParams.size(); i++) {
         m_lightAnimParams[i].active = stream->readFlag();
      }
   }
   
   if (stream->readFlag() == true) {
      if (stream->readFlag()) {
         m_forceRender |= UInt32(ForceEdit);
      } else {
         m_forceRender &= ~UInt32(ForceEdit);
      }
   }
}

void InteriorShape::initPersistFields()
{
   addField("filename",    TypeString,         Offset(m_pFileName,     InteriorShape));
   addField("isContainer", TypeInt,            Offset(m_isContainer,   InteriorShape));
   addField("position",    TypePoint3F,        Offset(m_shapePosition, InteriorShape));
   addField("rotation",    TypePoint3F,        Offset(m_shapeRotation, InteriorShape));
   addField("lightParams", TypeLightAnimParam, Offset(m_lightAnimParams, InteriorShape));
}

Persistent::Base::Error 
InteriorShape::read(StreamIO& sio,
                    int       ignore_version,
                    int       user)
{
   int version;
   sio.read(&version);
   AssertFatal(InteriorShape::version(),
               "Wrong file version: InteriorShape");
   if (Parent::read(sio, ignore_version, user) != Ok) {
      return ReadError;
   }

   m_pFileName   = sio.readSTString();
   sio.read(&m_isContainer);

   sio.read(sizeof(Point3F), &m_shapePosition);
   sio.read(sizeof(EulerF),  &m_shapeRotation);

   int size;
   sio.read(&size);
   m_lightAnimParams.setSize(size);
   sio.read(m_lightAnimParams.size() * sizeof(LightAnimParam),
            (void*)m_lightAnimParams.address());

   return (sio.getStatus() == STRM_OK) ? Ok : ReadError;
}

Persistent::Base::Error 
InteriorShape::write(StreamIO& sio,
                         int       ignore_version,
                         int       user)
{
   sio.write(InteriorShape::version());

   if (Parent::write(sio, ignore_version, user) != Ok) {
      return WriteError;
   }

   sio.writeString(m_pFileName);
   sio.write(m_isContainer);

   sio.write(sizeof(Point3F), &m_shapePosition);
   sio.write(sizeof(EulerF),  &m_shapeRotation);

   sio.write(m_lightAnimParams.size());
   sio.write(m_lightAnimParams.size() * sizeof(LightAnimParam),
             (void*)m_lightAnimParams.address());

   return (sio.getStatus() == STRM_OK) ? Ok : WriteError;
}

int
InteriorShape::version()
{
   return csm_fileVersion;
}

//------------------------------------------------------------------------------
//--------------------------------------
// Rendering functions
//--------------------------------------
//
bool
InteriorShape::render(SimContainerRenderContext& rc)
{
   if (m_renderImage.currShape == NULL)
      return false;

   if ((m_renderImage.currShape->isLinked() == true &&
        m_forceRender == 0                          &&
        rc.currentState().containCamera == false)) {
      return false;
   }

   if (contextTag == rc.getTag())
      return false;
   contextTag = rc.getTag();

   if (isOpen()) {
      // Depth first traversal based on camera pos.
      if (rc.currentState().containCamera) {
         SimContainer* cnt = getDatabase()->findOpenContainer(rc);
         if (cnt && rc.getTag() != cnt->getTag())
            if (!cnt->render(rc))
               return false;
      }

      // Push new state for the sub containers
      SimContainerRenderContext::State ss;
      ss = rc.currentState();
      ss.containCamera = false;
      rc.push(ss);

      // Render contents of our database
      getDatabase()->render(rc);
      rc.pop();

      // Render ourself
      rc.render(this);
   
      return renderLinks(rc);
   } else {
      // Render ourself
      rc.render(this);
      
      return true;
   }
}

bool
InteriorShape::scope(SimContainerScopeContext& sc)
{
   if (m_renderImage.currShape == NULL)
      return false;

   if ((m_renderImage.currShape->isLinked() == true &&
        m_forceRender == 0                          &&
        sc.currentState().containCamera == false)) {
      return false;
   }

   if (m_scopeTag == sc.getTag())
      return false;
   m_scopeTag = sc.getTag();

   if (isOpen()) {
      // Depth first traversal based on camera pos.
      if (sc.currentState().containCamera) {
         SimContainer* cnt = getDatabase()->findOpenContainer(sc);
         if (cnt && sc.getTag() != cnt->getTag())
            if (!cnt->scope(sc))
               return false;
      }

      // Scope ourself
      if (this->isScopeable())
         sc.scopeObject(this);

      // Push new state for the sub containers
      SimContainerScopeContext::State ss;
      ss = sc.currentState();
      ss.containCamera = false;
      sc.push(ss);

      // Render contents of our database
      getDatabase()->scope(sc);
      sc.pop();

      return scopeLinks(sc);
   } 
   
   return true;
}

bool
InteriorShape::scopeLinks(SimContainerScopeContext& sc)
{
   bool cameraContained = sc.currentState().containCamera;

   SimContainerScopeContext::State ss;
   ss = sc.currentState();
   ss.containCamera = false;
   sc.push(ss);

   bool drawOutside = false;

   Point3F localCamPos;
   Point3F temp = sc.currentState().cameraInfo->pos;
   m_mul(temp, getInvTransform(), &localCamPos);

   int outsideBits = m_renderImage.rend.getOutsideVisibility(localCamPos, m_renderImage.currShape);

   if (m_renderImage.currShape->isLinked() == false ||
       (m_forceRender & ForceLink) != 0             ||
       cameraContained == true) {

      int correctedOutsideBits = outsideBits;

      Box3F shapeBox = m_renderImage.currShape->getHighestBoundingBox();
      if (shapeBox.contains(localCamPos) == false &&
          cameraContained == true) {

         if (localCamPos.x < shapeBox.fMin.x && fabs(localCamPos.x - shapeBox.fMin.x) < csm_BBoxExtension)
            correctedOutsideBits |= ItrFaceMinX;
         if (localCamPos.y < shapeBox.fMin.y && fabs(localCamPos.y - shapeBox.fMin.y) < csm_BBoxExtension)
            correctedOutsideBits |= ItrFaceMinY;
         if (localCamPos.z < shapeBox.fMin.z && fabs(localCamPos.z - shapeBox.fMin.z) < csm_BBoxExtension)
            correctedOutsideBits |= ItrFaceMinZ;
         if (localCamPos.x > shapeBox.fMax.x && fabs(localCamPos.x - shapeBox.fMax.x) < csm_BBoxExtension)
            correctedOutsideBits |= ItrFaceMaxX;
         if (localCamPos.y > shapeBox.fMax.y && fabs(localCamPos.y - shapeBox.fMax.y) < csm_BBoxExtension)
            correctedOutsideBits |= ItrFaceMaxY;
         if (localCamPos.z > shapeBox.fMax.z && fabs(localCamPos.z - shapeBox.fMax.z) < csm_BBoxExtension)
            correctedOutsideBits |= ItrFaceMaxZ;
      }

      // Now we need to loop through any interiors that are linked to us, and
      //  scope those objects
      //
      for (int i = 0; i < m_currentLinks.size(); i++) {
         InteriorLink& link = m_currentLinks[i];
         
         if (((link.linkFace & correctedOutsideBits) == 0) ||
             link.pInterior->m_scopeTag == sc.getTag()) {
            continue;
         }

         // Well, we have to render this interior.  Since we are linked to it,
         //  it must be a container, so extract a pointer to its ITRContainer
         //  object, and pass it the dbRender message.  Let the interior know that
         //  it _has_ to scope link style, ie, highest detail...
         //
         UInt32 store = link.pInterior->m_forceRender;
         link.pInterior->m_forceRender = ForceLink;

         drawOutside |= link.pInterior->scope(sc);
         link.pInterior->m_forceRender = store;
      }
   }
   sc.pop();
   
   // If we are a non-linked, ie, "exterior interior", shape, we need to check
   //  the draw termination 
   if (m_renderImage.currShape->isLinked() == false) {
      if (m_renderImage.currShape->getHighestBoundingBox().contains(localCamPos)) {
         drawOutside |= (outsideBits != 0);
      } else {
         drawOutside = (outsideBits != 0);
      }
   }

   return drawOutside;
}

void
InteriorShape::RenderImage::render(TS::RenderContext &rc)
{
   ITRMetrics.render.reset();
   rc.getCamera()->pushTransform(transform);
   
   // Select the detail level we will be drawing at.  This is a function of the
   //  bounding box for the highest level of detail, and the radius of the minimum
   //  level in the current state.
   //  - If camera is inside object's bounding box, draw highest detail level.
   //  - else check the projected radius of the lowest level of detail...
   //
   Point3F cameraCoord = rc.getCamera()->getCC();
   Box3F   bbox        = currShape->getHighestBoundingBox();

   UInt32 oldDetailLevel = currShape->getDetailLevel();

   // We only set the detail by the pixels iff we're outside the interior, it's
   //  not a linked interior, and we're not rendering it through a link.  Otherwise,
   //  the highest detail is drawn...
   //
   if (linkRendering == false &&
       bbox.contains(cameraCoord) == false) {
      float projPixels = 
         2.0f * (rc.getCamera()->transformProjectRadius(currShape->getLowestCenterPt(),
                                                        currShape->getLowestRadius()));
      currShape->setDetailByPixels(projPixels);
   } else {
      // If we're inside or rendering through a link,
      //  we draw at the highest detail level...
      //
      currShape->setDetailLevel(0);
   }

   UInt32 newDetailLevel = currShape->getDetailLevel();

   if (pInteriorShape->isOpen() == true) {
      // Note that this does NOT take effect until the next frame.  Maybe the
      //  detail selection process could be moved to a higher level if this
      //  becomes a problem..
      //
      if (oldDetailLevel != newDetailLevel) {
         // If the new links aren't the same as the old, fix them up...
         //
         pInteriorShape->clearLinks();
         pInteriorShape->findAllLinks();
      }
   }

   rend.render(rc, currShape);

   // Draw the bounding box if the flag is set...
   //
   
   //if (SimShape::getDrawShapeBBox() == true) {
#if 0
      TS::PointArray* pArray = rc.getPointArray();
      Point3F bboxPts[8];
      
      // Draw our bbox
      bboxPts[0].set(bbox.fMin.x, bbox.fMin.y, bbox.fMin.z);
      bboxPts[1].set(bbox.fMin.x, bbox.fMax.y, bbox.fMin.z);
      bboxPts[2].set(bbox.fMin.x, bbox.fMax.y, bbox.fMax.z);
      bboxPts[3].set(bbox.fMin.x, bbox.fMin.y, bbox.fMax.z);
      bboxPts[4].set(bbox.fMax.x, bbox.fMin.y, bbox.fMin.z);
      bboxPts[5].set(bbox.fMax.x, bbox.fMax.y, bbox.fMin.z);
      bboxPts[6].set(bbox.fMax.x, bbox.fMax.y, bbox.fMax.z);
      bboxPts[7].set(bbox.fMax.x, bbox.fMin.y, bbox.fMax.z);
      
      pArray->reset();
      int start = pArray->addPoints(8, bboxPts);
      
      pArray->drawLine(start + 0, start + 1, 254);
      pArray->drawLine(start + 1, start + 2, 254);
      pArray->drawLine(start + 2, start + 3, 254);
      pArray->drawLine(start + 3, start + 0, 254);
      pArray->drawLine(start + 4, start + 5, 254);
      pArray->drawLine(start + 5, start + 6, 254);
      pArray->drawLine(start + 6, start + 7, 254);
      pArray->drawLine(start + 7, start + 4, 254);
      pArray->drawLine(start + 0, start + 4, 254);
      pArray->drawLine(start + 1, start + 5, 254);
      pArray->drawLine(start + 2, start + 6, 254);
      pArray->drawLine(start + 3, start + 7, 254);
      
      rc.getCamera()->popTransform();
      
      // Draw container bbox
      Box3F cntBbox = pInteriorShape->getBoundingBox();

      if (pInteriorShape->getInstance()->isLinked() == true) {
         cntBbox.fMax -= Point3F(csm_BBoxExtension, csm_BBoxExtension, csm_BBoxExtension);
         cntBbox.fMin += Point3F(csm_BBoxExtension, csm_BBoxExtension, csm_BBoxExtension);
      }
      bboxPts[0].set(cntBbox.fMin.x, cntBbox.fMin.y, cntBbox.fMin.z);
      bboxPts[1].set(cntBbox.fMin.x, cntBbox.fMax.y, cntBbox.fMin.z);
      bboxPts[2].set(cntBbox.fMin.x, cntBbox.fMax.y, cntBbox.fMax.z);
      bboxPts[3].set(cntBbox.fMin.x, cntBbox.fMin.y, cntBbox.fMax.z);
      bboxPts[4].set(cntBbox.fMax.x, cntBbox.fMin.y, cntBbox.fMin.z);
      bboxPts[5].set(cntBbox.fMax.x, cntBbox.fMax.y, cntBbox.fMin.z);
      bboxPts[6].set(cntBbox.fMax.x, cntBbox.fMax.y, cntBbox.fMax.z);
      bboxPts[7].set(cntBbox.fMax.x, cntBbox.fMin.y, cntBbox.fMax.z);
      
      pArray->reset();
      start = pArray->addPoints(8, bboxPts);
      
      int color;
      if (currShape->isLinked() == true)
         color = 250;
      else
         color = 253;

      pArray->drawLine(start + 0, start + 1, color);
      pArray->drawLine(start + 1, start + 2, color);
      pArray->drawLine(start + 2, start + 3, color);
      pArray->drawLine(start + 3, start + 0, color);
      pArray->drawLine(start + 4, start + 5, color);
      pArray->drawLine(start + 5, start + 6, color);
      pArray->drawLine(start + 6, start + 7, color);
      pArray->drawLine(start + 7, start + 4, color);
      pArray->drawLine(start + 0, start + 4, color);
      pArray->drawLine(start + 1, start + 5, color);
      pArray->drawLine(start + 2, start + 6, color);
      pArray->drawLine(start + 3, start + 7, color);

   } else {
#endif
      rc.getCamera()->popTransform();
}

//----------------------------------------------------------------------------
//-----------------------------------------
// Interior Link Management
//-----------------------------------------
//
void
InteriorShape::unlinkMe(const InteriorShape* in_pLinked)
{
   // Since the Vector<> class doesn't have a find() method, we'll have to
   //  search by hand...
   //
   for (InteriorLinkList::iterator itr = m_currentLinks.begin();
        itr != m_currentLinks.end(); ++itr) {
      InteriorLink& link = *itr;

      if (link.pInterior == in_pLinked) {
         link.pVertexArena->removeInterior(this);
         m_currentLinks.erase(itr);

         return;
      }
   }

   AssertFatal(0, "Tried to unlink an interior not linked to this shape");
}

bool
InteriorShape::linkMe(InteriorShape* in_pNewLink,
                         LinkVertexArena*  io_pArena)
{
   if (m_renderImage.currShape == NULL)
      return false;
      
   // We have to transform the incoming interior's bounding box into our
   //  coordinate space to find the abutting face.
   //
   Box3F myBoundingBox = m_renderImage.currShape->getHighestBoundingBox();
   Box3F otherBBox     = getITRSpaceBBox(in_pNewLink);
   ItrLinkFace linkFace   = getAbuttingFace(myBoundingBox, otherBBox);
   
   if (linkFace == ItrFaceNone ||
       (UInt32(linkFace) & m_renderImage.currShape->getLinkableFaces()) == 0) {
      return false;
   }
   
   InteriorLink newLink;
   newLink.pInterior    = in_pNewLink;
   newLink.linkFace     = linkFace;
   newLink.pVertexArena = io_pArena;
   m_currentLinks.push_back(newLink);

   // Add ourselves to the arena, the other interior will handle
   //  calling to find shared verts...
   //
   io_pArena->addInterior(this, linkFace);

   return true;
}

void
InteriorShape::clearLinks()
{
   for (InteriorLinkList::iterator itr = m_currentLinks.begin();
        itr != m_currentLinks.end(); ++itr) {
      InteriorLink& link = *itr;
      
      link.pInterior->unlinkMe(this);
      link.pVertexArena->removeInterior(this);
   }
   m_currentLinks.clear();
}

void
InteriorShape::findAllLinks()
{
   if (manager == NULL || m_renderImage.currShape == NULL)
      return;

   // Query the Container database to determine which Containable objects
   //  are affected by this light...
   //
   SimContainer* root = findObject(manager, SimRootContainerId, root);
   AssertFatal(root != NULL, "No root container?");

   SimContainerQuery scQuery;
   scQuery.id     = id;
   scQuery.type   = -1;
   scQuery.mask   = SimInteriorObjectType;
   scQuery.detail = SimContainerQuery::DefaultDetail;
   
   // Box need to extend out a bit...
   //
   scQuery.box       = getBoundingBox();
   scQuery.box.fMin -= Point3F(1, 1, 1);
   scQuery.box.fMax += Point3F(1, 1, 1);
   
   SimContainerList scList;
   root->findIntersections(scQuery, &scList);
   
   Box3F myBoundingBox = m_renderImage.currShape->getHighestBoundingBox();
   for (int i = 0; i < scList.size(); i++) {
      if (scList[i]->isOpen() == false)
         continue;
      
      // Make sure we are dealing w/ an interior container, then extract the
      //  pointer to the other interior...
      //
      SimContainer*     pContainer = scList[i];
      InteriorShape* pOtherItr  = static_cast<InteriorShape*>(scList[i]);

      UInt32 properMask = pContainer->getType() & (SimInteriorObjectType);
      if (properMask != (SimInteriorObjectType))
         continue;

      // Make sure that the bounding boxes simply
      //  _abut_ not overlap.  For those that have abutting faces, determine
      //  first which face on this interior is touching.  If this face is not
      //  marked as linkable, throw out the link.  If it is, pass the linkMe
      //  message to the other interior to check that the corresponding face is
      //  also a linkable face.  If it passes, insert the link into the table.
      //  (Note: the linkMe message assumes that all aspects of the link are
      //  valid for the colliding object, and only checks that the face is linkable
      //  on the other interior.

      Box3F otherBBox   = getITRSpaceBBox(pOtherItr);
      ItrLinkFace linkFace = getAbuttingFace(myBoundingBox, otherBBox);
      
      if (linkFace == ItrFaceNone ||
          (UInt32(linkFace) & m_renderImage.currShape->getLinkableFaces()) == 0) {
         continue;
      }
      
      // Ok, at this point, we've done all the checking we can (easily) perform
      //  from this side, send the proposed link on...
      //
      LinkVertexArena* vertexArena = new LinkVertexArena;
      vertexArena->addInterior(this, linkFace);

      if (pOtherItr->linkMe(this, vertexArena) == true) {
         InteriorLink newLink;
         newLink.pInterior    = pOtherItr;
         newLink.linkFace     = linkFace;
         newLink.pVertexArena = vertexArena;

         m_currentLinks.push_back(newLink);
      } else {
         vertexArena->removeInterior(this);
      }
   }
}


//------------------------------------------------------------------------------
//-----------------------------------------------------
// links, and linkRender'ing
//-----------------------------------------------------
//
bool
InteriorShape::renderLinks(SimContainerRenderContext& rc)
{
   bool cameraContained = rc.currentState().containCamera;

   SimContainerRenderContext::State ss;
   ss = rc.currentState();
   ss.containCamera = false;
   rc.push(ss);

   ITRMetrics.render.numInteriorLinks = m_currentLinks.size();

   bool drawOutside = false;

   TS::Camera* tsCamera = rc.currentState().renderContext->getCamera();
   Point3F worldCamPos = tsCamera->getTOW().p;
   tsCamera->pushTransform(getTransform());
   Point3F localCamPos = tsCamera->getCC();
   int correctedOutsideBits;

   int outsideBits = m_renderImage.rend.getOutsideVisibility(localCamPos, m_renderImage.currShape);
   
   if (m_renderImage.currShape->isLinked() == false ||
       (m_forceRender & ForceLink) != 0             ||
       cameraContained == true) {

      correctedOutsideBits = outsideBits;

      Box3F shapeBox = m_renderImage.currShape->getHighestBoundingBox();
      if (shapeBox.contains(localCamPos) == false &&
          cameraContained == true) {

         if (localCamPos.x < shapeBox.fMin.x && fabs(localCamPos.x - shapeBox.fMin.x) < csm_BBoxExtension)
            correctedOutsideBits |= ItrFaceMinX;
         if (localCamPos.y < shapeBox.fMin.y && fabs(localCamPos.y - shapeBox.fMin.y) < csm_BBoxExtension)
            correctedOutsideBits |= ItrFaceMinY;
         if (localCamPos.z < shapeBox.fMin.z && fabs(localCamPos.z - shapeBox.fMin.z) < csm_BBoxExtension)
            correctedOutsideBits |= ItrFaceMinZ;
         if (localCamPos.x > shapeBox.fMax.x && fabs(localCamPos.x - shapeBox.fMax.x) < csm_BBoxExtension)
            correctedOutsideBits |= ItrFaceMaxX;
         if (localCamPos.y > shapeBox.fMax.y && fabs(localCamPos.y - shapeBox.fMax.y) < csm_BBoxExtension)
            correctedOutsideBits |= ItrFaceMaxY;
         if (localCamPos.z > shapeBox.fMax.z && fabs(localCamPos.z - shapeBox.fMax.z) < csm_BBoxExtension)
            correctedOutsideBits |= ItrFaceMaxZ;
      }

      tsCamera->popTransform();
      
      // Now we need to loop through any interiors that are linked to us, and
      //  render those objects
      //
      for (int i = 0; i < m_currentLinks.size(); i++) {
         InteriorLink& link = m_currentLinks[i];
         
         if (((link.linkFace & correctedOutsideBits) == 0) ||
             link.pInterior->contextTag == rc.getTag()) {
            continue;
         }
         
         // Well, we have to render this interior.  Since we are linked to it,
         //  it must be a container, so extract a pointer to its ITRContainer
         //  object, and pass it the dbRender message.  Let the interior know that
         //  it _has_ to render link style, ie, highest detail...
         //
         UInt32 store = link.pInterior->m_forceRender;
         link.pInterior->m_forceRender = ForceLink;

         drawOutside |= link.pInterior->render(rc);
         link.pInterior->m_forceRender = store;
      }
   } else {
      tsCamera->popTransform();
   }
   rc.pop();
   
   // If we are a non-linked, ie, "exterior interior", shape, we need to check
   //  the draw termination 
   if (m_renderImage.currShape->isLinked() == false) {
      if (m_renderImage.currShape->getHighestBoundingBox().contains(localCamPos)) {
         drawOutside |= (correctedOutsideBits != 0);
      } else {
         drawOutside = (outsideBits != 0);
      }
   }

   return drawOutside;
}


//------------------------------------------------------------------------------
//--------------------------------------
// Misc. Linking Helper functions...
//--------------------------------------
//
Box3F
InteriorShape::getITRSpaceBBox(const InteriorShape* io_pOtherItr) const
{
   // Note: This function assumes that the bounding boxes are aligned with the
   //  same major coordinate axes,
   //
   Box3F otherBBox = io_pOtherItr->m_renderImage.currShape->getHighestBoundingBox();

   Point3F tempMin,  tempMax;
   Point3F otherMin, otherMax;

   // transform into our space...
   //
   m_mul(otherBBox.fMin, io_pOtherItr->getTransform(), &tempMin);
   m_mul(otherBBox.fMax, io_pOtherItr->getTransform(), &tempMax);
   m_mul(tempMin, getInvTransform(), &otherMin);
   m_mul(tempMax, getInvTransform(), &otherMax);
   
   if (otherMax.x < otherMin.x) {
      float temp = otherMax.x;
      otherMax.x = otherMin.x;
      otherMin.x = temp;
   }
   if (otherMax.y < otherMin.y) {
      float temp = otherMax.y;
      otherMax.y = otherMin.y;
      otherMin.y = temp;
   }
   if (otherMax.z < otherMin.z) {
      float temp = otherMax.z;
      otherMax.z = otherMin.z;
      otherMin.z = temp;
   }

   return Box3F(otherMin, otherMax);
}

ItrLinkFace
InteriorShape::getAbuttingFace(const Box3F& in_rMyBox,
                                  const Box3F& in_rOtherBox) const
{
   if (fabs(in_rMyBox.fMin.x - in_rOtherBox.fMax.x) < sg_BBoxTolerance) {
      if (testFaces(in_rMyBox.fMin.y, in_rMyBox.fMax.y, in_rOtherBox.fMin.y, in_rOtherBox.fMax.y) &&
          testFaces(in_rMyBox.fMin.z, in_rMyBox.fMax.z, in_rOtherBox.fMin.z, in_rOtherBox.fMax.z)) {
         return ItrFaceMinX;
      }
   }
   if (fabs(in_rMyBox.fMax.x - in_rOtherBox.fMin.x) < sg_BBoxTolerance) {
      if (testFaces(in_rMyBox.fMin.y, in_rMyBox.fMax.y, in_rOtherBox.fMin.y, in_rOtherBox.fMax.y) &&
          testFaces(in_rMyBox.fMin.z, in_rMyBox.fMax.z, in_rOtherBox.fMin.z, in_rOtherBox.fMax.z)) {
         return ItrFaceMaxX;
      }
   }
   if (fabs(in_rMyBox.fMin.y - in_rOtherBox.fMax.y) < sg_BBoxTolerance) {
      if (testFaces(in_rMyBox.fMin.x, in_rMyBox.fMax.x, in_rOtherBox.fMin.x, in_rOtherBox.fMax.x) &&
          testFaces(in_rMyBox.fMin.z, in_rMyBox.fMax.z, in_rOtherBox.fMin.z, in_rOtherBox.fMax.z)) {
         return ItrFaceMinY;
      }
   }
   if (fabs(in_rMyBox.fMax.y - in_rOtherBox.fMin.y) < sg_BBoxTolerance) {
      if (testFaces(in_rMyBox.fMin.x, in_rMyBox.fMax.x, in_rOtherBox.fMin.x, in_rOtherBox.fMax.x) &&
          testFaces(in_rMyBox.fMin.z, in_rMyBox.fMax.z, in_rOtherBox.fMin.z, in_rOtherBox.fMax.z)) {
         return ItrFaceMaxY;
      }
   }
   if (fabs(in_rMyBox.fMin.z - in_rOtherBox.fMax.z) < sg_BBoxTolerance) {
      if (testFaces(in_rMyBox.fMin.x, in_rMyBox.fMax.x, in_rOtherBox.fMin.x, in_rOtherBox.fMax.x) &&
          testFaces(in_rMyBox.fMin.y, in_rMyBox.fMax.y, in_rOtherBox.fMin.y, in_rOtherBox.fMax.y)) {
         return ItrFaceMinZ;
      }
   }
   if (fabs(in_rMyBox.fMax.z - in_rOtherBox.fMin.z) < sg_BBoxTolerance) {
      if (testFaces(in_rMyBox.fMin.x, in_rMyBox.fMax.x, in_rOtherBox.fMin.x, in_rOtherBox.fMax.x) &&
          testFaces(in_rMyBox.fMin.y, in_rMyBox.fMax.y, in_rOtherBox.fMin.y, in_rOtherBox.fMax.y)) {
         return ItrFaceMaxZ;
      }
   }

   return ItrFaceNone;
}


int
InteriorShape::correctOutsideBits(const int       in_oldBits,
                                     TS::Camera*     io_camera,
                                     TS::PointArray* io_pointArray)
{
   static const struct _FaceData {
      int   index0;
      int   index1;
      int   index2;
      int   index3;
      int   failMask;
   } faceTestData[6] = { {0, 1, 2, 3, ~(ItrFaceMinX)},
                         {0, 3, 7, 4, ~(ItrFaceMinY)},
                         {0, 4, 5, 1, ~(ItrFaceMinZ)},
                         {4, 7, 6, 5, ~(ItrFaceMaxX)},
                         {5, 6, 2, 1, ~(ItrFaceMaxY)},
                         {2, 6, 7, 3, ~(ItrFaceMaxZ)} };

   int newBits = in_oldBits;
   newBits    &= m_renderImage.currShape->getLinkableFaces();

   Box3F bbox     = m_renderImage.currShape->getHighestBoundingBox();
   Point3F camPos = io_camera->getCC();
   
   // Build Bounding box points...
   //
   Point3F bboxPts[8];
   bboxPts[0].set(bbox.fMin.x, bbox.fMin.y, bbox.fMin.z);
   bboxPts[1].set(bbox.fMin.x, bbox.fMax.y, bbox.fMin.z);
   bboxPts[2].set(bbox.fMin.x, bbox.fMax.y, bbox.fMax.z);
   bboxPts[3].set(bbox.fMin.x, bbox.fMin.y, bbox.fMax.z);
   bboxPts[4].set(bbox.fMax.x, bbox.fMin.y, bbox.fMin.z);
   bboxPts[5].set(bbox.fMax.x, bbox.fMax.y, bbox.fMin.z);
   bboxPts[6].set(bbox.fMax.x, bbox.fMax.y, bbox.fMax.z);
   bboxPts[7].set(bbox.fMax.x, bbox.fMin.y, bbox.fMax.z);

   io_pointArray->reset();
   io_pointArray->useTextures(false);
   io_pointArray->useIntensities(false);
   io_pointArray->useTextures(false);
   io_pointArray->setVisibility( TS::ClipMask );

   int start = io_pointArray->addPoints(8, bboxPts);
   TS::VertexIndexPair vIList[4];

   float camPosTest[6];
   float boxTest[6];
   camPosTest[0] = camPosTest[3] = camPos.x;
   camPosTest[1] = camPosTest[4] = camPos.y;
   camPosTest[2] = camPosTest[5] = camPos.z;
   boxTest[0] = bbox.fMin.x;
   boxTest[1] = bbox.fMin.y;
   boxTest[2] = bbox.fMin.z;
   boxTest[3] = bbox.fMax.x;
   boxTest[4] = bbox.fMax.y;
   boxTest[5] = bbox.fMax.z;

   for (int i = 0; i < 6; i++) {
      const _FaceData& faceRef = faceTestData[i];
      if (camPosTest[i] >= boxTest[i]) {
         // Make sure that the camera is far enough away from the hither clip
         // plane for this to be valid...
         //
         if (fabs(camPosTest[i] - boxTest[i]) > 2.0f) {
            vIList[0].fVertexIndex = start + faceRef.index0;
            vIList[1].fVertexIndex = start + faceRef.index1;
            vIList[2].fVertexIndex = start + faceRef.index2;
            vIList[3].fVertexIndex = start + faceRef.index3;

            if (io_pointArray->testPolyVis(4, vIList, 0) == false)
               newBits &= faceRef.failMask;
         }
      } else {
         newBits &= faceRef.failMask;
      }
   }
   
   io_pointArray->reset();
   return newBits;
}

bool
InteriorShape::getWeatherDistance(const TMat3F& in_rWorldTrans,
                                  float&        out_rDistance)
{
   ITRInstance* pInstance = getInstance();
   if (pInstance == NULL ||
       pInstance->isLinked() == true)
      return false;

   TMat3F  interiorTrans;
   Point3F interiorPos;
   Point3F interiorViewVector;
   m_mul(in_rWorldTrans, getInvTransform(), &interiorTrans);
   interiorPos = interiorTrans.p;
   interiorTrans.getRow(1, &interiorViewVector);

   // It's possible that we could be inside the interior's container box, but not
   //  inside the interior's acutal bounding box, check for that here...
   //
   ITRGeometry* pGeometry = pInstance->getGeometry();

   if (pGeometry->box.contains(interiorPos) == false) {
      out_rDistance = -1.0f;
      return true;
   }

   int leafIndex;
   if ((leafIndex = pGeometry->externalLeaf(interiorPos)) == 0)
      leafIndex = pGeometry->findLeaf(interiorPos);
   ITRGeometry::BSPLeafWrap leafWrap(pGeometry, leafIndex);
   
   int camPosOutsideBits = pGeometry->getOutsideBits(leafWrap);

   if (camPosOutsideBits == 0)
      return false;

   float maxDist = -1.0f;
   if ((camPosOutsideBits & ITRGeometry::OutsideMinX) != 0 &&
       m_dot(interiorViewVector, Point3F(-1, 0, 0)) >= 0.0f)
      if ((interiorPos.x - pGeometry->box.fMin.x) > maxDist)
         maxDist = interiorPos.x - pGeometry->box.fMin.x;
   if ((camPosOutsideBits & ITRGeometry::OutsideMinY) != 0 &&
       m_dot(interiorViewVector, Point3F(0, -1, 0)) >= 0.0f)
      if ((interiorPos.y - pGeometry->box.fMin.y) > maxDist)
         maxDist = interiorPos.y - pGeometry->box.fMin.x;
   if ((camPosOutsideBits & ITRGeometry::OutsideMinZ) != 0 &&
       m_dot(interiorViewVector, Point3F(0, 0, -1)) >= 0.0f)
      if ((interiorPos.z - pGeometry->box.fMin.z) > maxDist)
         maxDist = interiorPos.z - pGeometry->box.fMin.x;
   if ((camPosOutsideBits & ITRGeometry::OutsideMaxX) != 0 &&
       m_dot(interiorViewVector, Point3F(1, 0, 0)) >= 0.0f)
      if ((pGeometry->box.fMin.x - interiorPos.x) > maxDist)
         maxDist = pGeometry->box.fMax.x - interiorPos.x;
   if ((camPosOutsideBits & ITRGeometry::OutsideMaxY) != 0 &&
       m_dot(interiorViewVector, Point3F(0, 1, 0)) >= 0.0f)
      if ((pGeometry->box.fMin.y - interiorPos.y) > maxDist)
         maxDist = pGeometry->box.fMax.y - interiorPos.y;
   if ((camPosOutsideBits & ITRGeometry::OutsideMaxZ) != 0 &&
       m_dot(interiorViewVector, Point3F(0, 0, 1)) >= 0.0f)
      if ((pGeometry->box.fMin.z - interiorPos.z) > maxDist)
         maxDist = pGeometry->box.fMax.z - interiorPos.z;

   out_rDistance = maxDist;

   return true;
}


//------------------------------------------------------------------------------
//--------------------------------------
// LinkVertexArena Implementation...
//--------------------------------------
//
LinkVertexArena::LinkVertexArena()
 : m_pInterior0(NULL),
   m_pInterior1(NULL),
   m_interior0LinkFace(ItrFaceNone),
   m_interior1LinkFace(ItrFaceNone)
{
   //
}

LinkVertexArena::~LinkVertexArena()
{
   AssertFatal(m_pInterior0 == NULL && m_pInterior1 == NULL,
               "Object still referenced by this Arena");
}

void
LinkVertexArena::addInterior(InteriorShape* io_pAdd,
                             const ItrLinkFace    in_linkFace)
{
   AssertFatal(io_pAdd != NULL, "Can't add NULL Interior!");
   AssertFatal(m_pInterior0 == NULL || m_pInterior1 == NULL,
               "Error, arena membership already filled");

   if (m_pInterior0 == NULL) {
      m_pInterior0        = io_pAdd;
      m_interior0LinkFace = in_linkFace;
   } else {
      m_pInterior1        = io_pAdd;
      m_interior1LinkFace = in_linkFace;
   }
}

void
LinkVertexArena::removeInterior(InteriorShape* io_pRemove)
{
   AssertFatal(io_pRemove != NULL, "Can't remove NULL Interior!");
   AssertFatal(m_pInterior0 != NULL || m_pInterior1 != NULL,
               "Error, arena membership already cleared");

   if (io_pRemove == m_pInterior0) {
      m_pInterior0 = NULL;
      m_interior0LinkFace = ItrFaceNone;
      if (m_pInterior1 == NULL)
         delete this;
   } else if (io_pRemove == m_pInterior1) {
      m_pInterior1 = NULL;
      m_interior0LinkFace = ItrFaceNone;
      if (m_pInterior0 == NULL)
         delete this;
   } else {
      AssertFatal(0, "Error, interior is not an arena member...");
   }
}


//------------------------------------------------------------------------------
//--------------------------------------
// InteriorContainer methods...
//--------------------------------------

//------------------------------------------------------------------------------
//-------------------------------------- SIMPLE DATABASE
//
InteriorShapeDatabaseSimple::InteriorShapeDatabaseSimple(InteriorShape* in_pInteriorShape)
 : m_pInteriorShape(in_pInteriorShape)
{
   //
}

InteriorShapeDatabaseSimple::~InteriorShapeDatabaseSimple()
{
   m_pInteriorShape = NULL;
}

void
InteriorShapeDatabaseSimple::render(SimContainerRenderContext& rc)
{
   // Render all objects
   //
   SimContainerRenderContext::State ss;
   ss = rc.currentState();
   ss.containCamera = false;
   rc.push(ss);

   const int size = objectList.size();
   for (SimContainerList::iterator itr = objectList.begin();
         itr != objectList.end(); itr++ ) {

      if ((*itr)->isOpen()) {
         SimContainer* cnt = static_cast<SimContainer*>(*itr);
         if (rc.getTag() != cnt->getTag())
            cnt->render(rc);
      }
      else
         rc.render(*itr);
      
      AssertFatal(size == objectList.size(),
         "object changed containers during render cycle");
   }
   rc.pop();
}

void
InteriorShapeDatabaseSimple::scope(SimContainerScopeContext& sc)
{
   // Scope all the objects and sub-containers.
   // This method is a little different then the normal database 
   // functions in that it recursively operates on nested open
   // containers.
   //
   for (SimContainerList::iterator itr = objectList.begin();
         itr != objectList.end(); itr++ ) {
      if ((*itr)->isOpen()) {
         if (sc.getTag() != (*itr)->getTag())
            (*itr)->scope(sc);
      }
      else if ((*itr)->isScopeable()) {
         sc.scopeObject(*itr);
      }
   }
}


//------------------------------------------------------------------------------
//-------------------------------------- PVS DATABASE
//
InteriorShapeDatabasePVS::InteriorShapeDatabasePVS(InteriorShape* io_pInteriorShape)
 : m_pInteriorShape(io_pInteriorShape)
{
   m_pDatabaseGeometry = io_pInteriorShape->getInstance()->getHighestGeometry();
   m_interiorBox       = io_pInteriorShape->getInstance()->getHighestBoundingBox();

   AssertFatal(m_pDatabaseGeometry != NULL, "Error, no geometry for database?");

   // The free list is initially empty.  Will grow in size as objects are
   //  added and removed
   //
   m_freeList = 0xffff;

   // Intialize the BSP node list, initially all have a count of 0,
   //  and a null start index
   //
   m_bspNodeList.setSize(m_pDatabaseGeometry->emptyLeafList.size() +
                         m_pDatabaseGeometry->solidLeafList.size() + 1);
   for (int i = 0; i < m_bspNodeList.size(); i++) {
      m_bspNodeList[i].start = 0xffff;
      m_bspNodeList[i].count = 0;
   }
}

InteriorShapeDatabasePVS::~InteriorShapeDatabasePVS()
{
   m_pInteriorShape    = NULL;
   m_pDatabaseGeometry = NULL;
}

Int32
InteriorShapeDatabasePVS::findLeaf(const Point3F& p, Int32 nodeIndex)
{
   if (nodeIndex < 0)
      return -(nodeIndex+1);

   ITRGeometry::BSPNode& node = m_pDatabaseGeometry->nodeList[nodeIndex];
   if (m_pDatabaseGeometry->planeList[node.planeIndex].whichSide(p) == TPlaneF::Inside)
      return findLeaf(p, node.front);
   return findLeaf(p, node.back);
}

Int32
InteriorShapeDatabasePVS::findBSPLeaf(const Point3F& in_rPoint)
{
   // The point we use to find the BSP node is the center of the container bounding
   //  box.  Note that this can cause some wierd effects.  We may have to move
   //  to a sphere or box based union of nodes to counteract this...
   //
   Int32 leafIndex = m_pDatabaseGeometry->externalLeaf(in_rPoint);
   if (leafIndex == 0)
      leafIndex = findLeaf(in_rPoint);

   return leafIndex;
}

Int32
InteriorShapeDatabasePVS::findBSPLeaf(SimContainer* io_pContainer)
{
   // The point we use to find the BSP node is the center of the container bounding
   //  box.  Note that this can cause some wierd effects.  We may have to move
   //  to a sphere or box based union of nodes to counteract this...
   //
   const Box3F& rBox = io_pContainer->getBoundingBox();
   Point3F centerPoint = (rBox.fMin + rBox.fMax) * 0.5;
   Point3F trCenterPoint;
   
   m_mul(centerPoint, m_pInteriorShape->getInvTransform(), &trCenterPoint);

   Int32 leafIndex = m_pDatabaseGeometry->externalLeaf(trCenterPoint);
   if (leafIndex == 0)
      leafIndex = findLeaf(trCenterPoint);

   return leafIndex;
}

void
InteriorShapeDatabasePVS::enterBSPObject(ObjectEntry& in_rEntry)
{
   AssertFatal(in_rEntry.bspLeaf == 0, "Error, already in BSP database");
   
   in_rEntry.bspLeaf = findBSPLeaf(in_rEntry.pContainer);

   ITRGeometry::BSPLeafWrap leafWrap(m_pDatabaseGeometry, in_rEntry.bspLeaf);

   if (leafWrap.isSolid()) {
      // All objects in solid nodes go into the special 0 node, which is always
      //  scoped, and always rendered.
      //
      in_rEntry.bspLeaf = -1;

      BSPNodeEntry& rNodeEntry = m_bspNodeList[0];

      // Need a free bspOjectEntry
      UInt16 freeEntry;
      if (m_freeList == 0xffff) {
         m_bspObjectList.increment();
         freeEntry = m_bspObjectList.size() - 1;
      } else {
         freeEntry  = m_freeList;
         m_freeList = m_bspObjectList[freeEntry].next;
      }

      BSPObjectEntry& rObjectEntry = m_bspObjectList[freeEntry];
      rObjectEntry.back       = 0xffff;
      rObjectEntry.next       = 0xffff;
      rObjectEntry.pContainer = in_rEntry.pContainer;
      rObjectEntry.leafIndex  = in_rEntry.bspLeaf;

      // Insert into the leaf's list, and we're done!
      if (rNodeEntry.count == 0) {
         rNodeEntry.start = freeEntry;
         rNodeEntry.count = 1;
      } else {
         UInt16 oldStart = rNodeEntry.start;
         BSPObjectEntry& rOldStart = m_bspObjectList[oldStart];

         rObjectEntry.next = oldStart;
         rOldStart.back    = freeEntry;
         rNodeEntry.start  = freeEntry;
         rNodeEntry.count++;
      }

      return;
   } else {
      // All objects in empty leaves go into the normal nodes
      //
      BSPNodeEntry& rNodeEntry = m_bspNodeList[in_rEntry.bspLeaf + 1];
   
      // Need a free bspOjectEntry
      UInt16 freeEntry;
      if (m_freeList == 0xffff) {
         m_bspObjectList.increment();
         freeEntry = m_bspObjectList.size() - 1;
      } else {
         freeEntry = m_freeList;
         m_freeList = m_bspObjectList[freeEntry].next;
      }

      BSPObjectEntry& rObjectEntry = m_bspObjectList[freeEntry];
      rObjectEntry.back       = 0xffff;
      rObjectEntry.next       = 0xffff;
      rObjectEntry.pContainer = in_rEntry.pContainer;
      rObjectEntry.leafIndex  = in_rEntry.bspLeaf;

      // Insert into the leaf's list, and we're done!
      if (rNodeEntry.count == 0) {
         rNodeEntry.start = freeEntry;
         rNodeEntry.count = 1;
      } else {
         UInt16 oldStart = rNodeEntry.start;
         BSPObjectEntry& rOldStart = m_bspObjectList[oldStart];

         rObjectEntry.next = oldStart;
         rOldStart.back    = freeEntry;
         rNodeEntry.start  = freeEntry;
         rNodeEntry.count++;
      }
   }
}

void
InteriorShapeDatabasePVS::removeBSPObject(ObjectEntry& in_rEntry)
{
   AssertFatal(in_rEntry.bspLeaf != 0, "Error, not in BSP database");
   
   BSPNodeEntry& rNodeEntry = m_bspNodeList[in_rEntry.bspLeaf + 1];;
   AssertFatal(rNodeEntry.count != 0, "Error, searching in empty node");

   UInt16 foundIndex = 0xffff;
   UInt16 currIndex = rNodeEntry.start;

   while (currIndex != 0xffff) {
      if (m_bspObjectList[currIndex].pContainer == in_rEntry.pContainer) {
         foundIndex = currIndex;
         break;
      }
      currIndex = m_bspObjectList[currIndex].next;
   }
   AssertFatal(foundIndex != 0xffff, "Not found in the proper node list");

   BSPObjectEntry& rObjectEntry = m_bspObjectList[foundIndex];
   if (foundIndex == rNodeEntry.start) {
      // special case for the first entry in the list.  Sigh.  w/ indices, there
      //  doesn't seem to be a convenient way around this...
      //
      rNodeEntry.start = rObjectEntry.next;
   } else {
      // There will definitely be a back entry, possibly a next entry
      //
      m_bspObjectList[rObjectEntry.back].next = rObjectEntry.next;
      if (rObjectEntry.next != 0xffff)
         m_bspObjectList[rObjectEntry.next].back = rObjectEntry.back;
   }
   rNodeEntry.count--;

   // Place the objectEntry on the free list...
   //
   rObjectEntry.pContainer = NULL;
   rObjectEntry.leafIndex  = 0;
   rObjectEntry.back = 0xffff;
   rObjectEntry.next = m_freeList;
   m_freeList        = foundIndex;

   in_rEntry.bspLeaf = 0;
}

bool
InteriorShapeDatabasePVS::add(SimContainer* io_pContainer)
{
   ObjectEntry entry;
   entry.pContainer = io_pContainer;
   entry.bspLeaf    = 0;

   enterBSPObject(entry);
   m_objectList.push_back(entry);

   return true;
}

bool
InteriorShapeDatabasePVS::remove(SimContainer* io_pRemove)
{
   PVSContainerList::iterator itr;
   for (itr = m_objectList.begin(); itr != m_objectList.end(); itr++) {
      if (itr->pContainer == io_pRemove)
         break;
   }
   AssertFatal(itr != m_objectList.end(), "Unable to find container in database");

   removeBSPObject(*itr);
   m_objectList.erase(itr);

   return true;
}

void
InteriorShapeDatabasePVS::update(SimContainer* io_pUpdate)
{
   PVSContainerList::iterator itr;
   for (itr = m_objectList.begin(); itr != m_objectList.end(); itr++) {
      if (itr->pContainer == io_pUpdate)
         break;
   }
   AssertFatal(itr != m_objectList.end(), "Unable to find container in database");

   UInt32 leaf = findBSPLeaf(io_pUpdate);
   if (itr->bspLeaf != leaf) {
      removeBSPObject(*itr);
      enterBSPObject(*itr);
   } else {
      // nada.
   }
}

bool
InteriorShapeDatabasePVS::removeAll(SimContainer* io_pParent)
{
   // Remove all objects, adding to our parent if necessary...

   if (io_pParent != NULL) {
      while (m_objectList.size() != 0) {
         SimContainer* object = m_objectList.first().pContainer;

         // Haven't really looked to see why we should
         // get here with deleted objects in our list,
         // but seems like we do.
         if (object->isDeleted() == true) {
            object->getContainer()->removeObject(object);
         } else {
            io_pParent->addObject(object);
         }
      }
   } else {
      while (m_objectList.size() != 0) {
         SimContainer* object = m_objectList.first().pContainer;
         object->getContainer()->removeObject(object);
      }
   }

   return true;
}

bool
InteriorShapeDatabasePVS::findContained(const SimContainerQuery& query,
                                        SimContainerList*        containedList)
{
   // Find all objects contained in the query box
   for (PVSContainerList::iterator itr = m_objectList.begin();
         itr != m_objectList.end(); itr++) {
      if (query.isContained(itr->pContainer) == true) {
         containedList->push_back(itr->pContainer);
      }
   }

   return containedList->empty() == false;
}

SimContainer*
InteriorShapeDatabasePVS::findOpenContainer(const SimContainerQuery& query)
{
   // Find the container that containes the query box.
   //
   for (PVSContainerList::iterator itr = m_objectList.begin();
         itr != m_objectList.end(); itr++ ) {
      SimContainer* pContainer = itr->pContainer;
      if (pContainer->isOpen() && pContainer->isContained(query))
         return pContainer;
   }

   return NULL;
}

bool
InteriorShapeDatabasePVS::findIntersections(const SimContainerQuery& query,
                                               SimContainerList*        intersectionList)
{
   for (PVSContainerList::iterator itr = m_objectList.begin();
        itr != m_objectList.end(); itr++) {
      SimContainer* pContainer = itr->pContainer;
      if ((pContainer->isOpen() && pContainer->isIntersecting(query.box)) ||
          pContainer->isIntersecting(query)) {
         intersectionList->push_back(pContainer);
      }
   }

   return intersectionList->empty() == false;
}

void
InteriorShapeDatabasePVS::scopeNode(SimContainerScopeContext& sc,
                                    const BSPNodeEntry&       in_rEntry)
{
   AssertFatal(in_rEntry.count != 0 && in_rEntry.start != 0xffff,
               "No elements in node, should not be scoping it.");
   UInt16 currIndex = in_rEntry.start;
   while (currIndex != 0xffff) {
      BSPObjectEntry& rEntry = m_bspObjectList[currIndex];
      if (rEntry.pContainer->isOpen()) {
         if (sc.getTag() != rEntry.pContainer->getTag())
            rEntry.pContainer->scope(sc);
      } else if (rEntry.pContainer->isScopeable()) {
         sc.scopeObject(rEntry.pContainer);
      }

      currIndex = rEntry.next;
   }
}

void
InteriorShapeDatabasePVS::renderNode(SimContainerRenderContext& rc,
                                     const BSPNodeEntry&        in_rEntry)
{
   AssertFatal(in_rEntry.count != 0 && in_rEntry.start != 0xffff,
               "No elements in node, should not be scoping it.");
   UInt16 currIndex = in_rEntry.start;
   while (currIndex != 0xffff) {
      BSPObjectEntry& rEntry = m_bspObjectList[currIndex];

      if (rEntry.pContainer->isOpen()) {
         SimContainer* cnt = rEntry.pContainer;
         if (rc.getTag() != cnt->getTag())
            cnt->render(rc);
      } else {
         rc.render(rEntry.pContainer);
      }

      currIndex = rEntry.next;
   }
}

void
InteriorShapeDatabasePVS::render(SimContainerRenderContext& rc)
{
   TS::Camera* tsCamera = rc.currentState().renderContext->getCamera();
   Point3F localCamPos;
   m_mul(tsCamera->getTCW().p, m_pInteriorShape->getInvTransform(), &localCamPos);

   // And the leaf associated with it...
   //
   Int32 leafIndex = findBSPLeaf(localCamPos);
   ITRGeometry::BSPLeafWrap leafWrap(m_pDatabaseGeometry, leafIndex);

   // Nothing is visibile from a solid node
   if (leafWrap.isSolid())
      return;

	UInt8* pbegin = &m_pDatabaseGeometry->bitList[leafWrap.getPVSIndex()];
	UInt8* pend   = pbegin + leafWrap.getPVSCount();
   int outsideMask = m_pDatabaseGeometry->getOutsideBits(leafWrap);
	for (ITRCompressedBitVector::iterator itr(pbegin,pend); ++itr; ) {
      if (*itr >= 43) {
         BSPNodeEntry& rNode = m_bspNodeList[(*itr) + 1];
         if (rNode.count > 0)
            renderNode(rc, rNode);
      }
   }
   BSPNodeEntry& rSolidNode = m_bspNodeList[0];
   if (rSolidNode.count > 0)
      renderNode(rc, rSolidNode);

   if (outsideMask != 0) {
      for (int i = 0; i < 43; i++) {
         BSPNodeEntry& rNode = m_bspNodeList[i + 1];
         if (rNode.count > 0)
            renderNode(rc, rNode);
      }
   }
}

void
InteriorShapeDatabasePVS::scope(SimContainerScopeContext& sc)
{
   // First, get the local camera position...
   //
   Point3F localCamPos;
   m_mul(sc.currentState().cameraInfo->pos, m_pInteriorShape->getInvTransform(), &localCamPos);

   // And the leaf associated with it...
   //
   Int32 leafIndex = findBSPLeaf(localCamPos);
   ITRGeometry::BSPLeafWrap leafWrap(m_pDatabaseGeometry, leafIndex);

   // Nothing is visibile from a solid node
   if (leafWrap.isSolid())
      return;

	UInt8* pbegin = &m_pDatabaseGeometry->bitList[leafWrap.getPVSIndex()];
	UInt8* pend   = pbegin + leafWrap.getPVSCount();
   int outsideMask = m_pDatabaseGeometry->getOutsideBits(leafWrap);
	for (ITRCompressedBitVector::iterator itr(pbegin,pend); ++itr; ) {
      if (*itr >= 43) {
         BSPNodeEntry& rNode = m_bspNodeList[(*itr) + 1];
         if (rNode.count > 0)
            scopeNode(sc, rNode);
      }
   }
   BSPNodeEntry& rSolidNode = m_bspNodeList[0];
   if (rSolidNode.count > 0)
      scopeNode(sc, rSolidNode);

   if (outsideMask != 0) {
      for (int i = 0; i < 43; i++) {
         BSPNodeEntry& rNode = m_bspNodeList[i + 1];
         if (rNode.count > 0)
            scopeNode(sc, rNode);
      }
   }
}

void
InteriorShape::writeCheckVals(StreamIO* pStream)
{
   // may need to generate the check values
   if(m_checkVals.size() != (getInstance()->getNumDetails() * 2))
      buildCheckVals();
      
   pStream->write(UInt32(m_checkVals.size()));
   for (Int32 i = 0; i < m_checkVals.size(); i++)
      pStream->write(m_checkVals[i]);
}

void
InteriorShape::readCheckVals(StreamIO* pStream)
{
   UInt32 size;
   pStream->read(&size);
   m_checkVals.setSize(size);

   for (UInt32 i = 0; i < size; i++)
      pStream->read(&m_checkVals[i]);
}

bool
InteriorShape::verifyCheckVals()
{
   Vector<UInt32> test;

   ITRInstance* pInstance = getInstance();
   int i;
   for (i = 0; i < pInstance->getNumDetails(); i++) {
      ITRGeometry* pGeom = pInstance->getGeometry(i);
      test.push_back(pGeom->buildId);
   }
   for (i = 0; i < pInstance->getNumDetails(); i++) {
      ITRLighting* pLighting = pInstance->getLighting(i);
      test.push_back(pLighting->getCheckVal());
   }

   if (test.size() != m_checkVals.size())
      return false;

   for (i = 0; i < test.size(); i++)
      if (test[i] != m_checkVals[i])
         return false;

   return true;
}

void
InteriorShape::buildCheckVals()
{
   ITRInstance* pInstance = getInstance();
   int i;
   for (i = 0; i < pInstance->getNumDetails(); i++) {
      ITRGeometry* pGeom = pInstance->getGeometry(i);
      m_checkVals.push_back(pGeom->buildId);
   }
   for (i = 0; i < pInstance->getNumDetails(); i++) {
      ITRLighting* pLighting = pInstance->getLighting(i);
      m_checkVals.push_back(pLighting->getCheckVal());
   }
}

bool
InteriorShape::isMissionLit()
{
   ITRInstance* pInstance = getInstance();

   if (pInstance != NULL) {
      return pInstance->isMissionLit();
   } else {
      return false;
   }
}
