//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <itrShape.h>
#include <itrMetrics.h>
#include "simInteriorShape.h"
#include "editor.strings.h"
#include "commonEditor.strings.h"
#include "darkstar.strings.h"
#include "simResource.h"
#include "console.h"
#include "simTimerGrp.h"
int SimInteriorShape::sm_fileVersion = 4;

//-------------------------------------- InteriorShape database class declarations.
//                                        non-PVS'ed interiors use the Simple version
//
class SimInteriorShapeDatabaseSimple : public SimDefaultContainerDatabase
{
   typedef SimDefaultContainerDatabase Parent;

   SimInteriorShape* m_pInteriorShape;

  public:
   SimInteriorShapeDatabaseSimple(SimInteriorShape*);
   ~SimInteriorShapeDatabaseSimple();

   void render(SimContainerRenderContext& rq);
   void scope(SimContainerScopeContext& sc);
};

class SimInteriorShapeDatabasePVS : public SimContainerDatabase
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
      UInt32        leafIndex;
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

   SimInteriorShape* m_pInteriorShape;
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
   SimInteriorShapeDatabasePVS(SimInteriorShape*);
   ~SimInteriorShapeDatabasePVS();

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
const float SimInteriorShape::csm_BBoxExtension = 3.0f;
namespace {

const float sg_BBoxTolerance  = 0.001f;

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
   SimInteriorShape* m_pInterior0;
   SimInteriorShape* m_pInterior1;

   LinkFace m_interior0LinkFace;
   LinkFace m_interior1LinkFace;

  public:
   // LinkVertexArenas work through reference counting, so we make it impossible for
   //  outside objects to delete the class.  This also has the effect of making it
   //  mandatory to allocate the object dynamically
   //
  private:
   ~LinkVertexArena();
  public:
   LinkVertexArena();

   void addInterior(SimInteriorShape*    io_pAdd,
                    const LinkFace       in_linkFace);
   void removeInterior(SimInteriorShape* io_pRemove);

   int getIndexKey(SimInteriorShape* io_pCompare) const {
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
SimInteriorShape::SimInteriorShape()
 : m_pITRInstance(NULL),
   m_pFileName(NULL),
   m_disableCollisions(false),
   m_initialState(0),
   m_destroyedState(-1),
   m_lightState(0),


   m_maxLightUpdatePerSec(15),
   m_lightUpdateDelta(1.0f/15.0f),

   m_forceRender(0),
   m_scopeTag(0),
   m_isContainer(true)
{
   type |= SimInteriorObjectType;

   containerMask = -1;

   netFlags.set(Ghostable);
}

SimInteriorShape::~SimInteriorShape()
{
   if (m_pITRInstance && m_lightThreads.size())
      shutdownLights();

   clearLinks();

   delete m_pITRInstance;
   m_pITRInstance = NULL;
   delete [] m_pFileName;
   m_pFileName = NULL;
}

//------------------------------------------------------------------------------
//--------------------------------------
// Internal variable management...
//--------------------------------------
//
bool
SimInteriorShape::setFileName(const char* in_pFileName)
{
   delete [] m_pFileName;
   m_pFileName = NULL;
   
   if (in_pFileName != NULL) {
      m_pFileName = new char[strlen(in_pFileName) + 1];
      strcpy(m_pFileName, in_pFileName);
      
      return true;
   }
   
   setMaskBits(InitialUpdate);
   return false;
}

void
SimInteriorShape::setIsContainer(const bool in_isContainer)
{
   if (in_isContainer == true) {
      if (getDatabase() == NULL) {
         SimContainerDatabase* install;
//         if (getInstance() == NULL) {
            install = new SimInteriorShapeDatabaseSimple(this);
//         } else {
//            install = new SimInteriorShapeDatabasePVS(this);
//         }
         installDatabase(install);
      }
   } else {
      uninstallDatabase();
   }

   m_isContainer = in_isContainer;
   setMaskBits(InitialUpdate);
}

void
SimInteriorShape::setShapeStates(const UInt32 in_init,
                                 const UInt32 in_dest)
{
   m_initialState   = in_init;
   m_destroyedState = in_dest;

   initRenderImage();

   setMaskBits(InitialUpdate);
}

bool
SimInteriorShape::setLightState(const UInt32 in_lightState)
{
   m_lightState = in_lightState;
   
   if (m_pITRInstance != NULL) {
      AssertFatal(manager != NULL, "How did we get here wo/ a manager?");
      ResourceManager* rm = SimResource::get(manager);
      m_pITRInstance->setInteriorLightState(rm, in_lightState);
   }
   
   setMaskBits(InitialUpdate);
   return true;
}


//------------------------------------------------------------------------------
//--------------------------------------
// Location/BBox management
//--------------------------------------
//
void
SimInteriorShape::buildContainerBox(const TMat3F& in_rTrans,
                                    Box3F*        out_pBox)
{
   out_pBox->fMin.set(0, 0, 0);
   out_pBox->fMax.set(0, 0, 0);
   if (m_pITRInstance == NULL)
      return;

   // Here we should send unlinkMe messages to all interiors we are linked to
   //  and clear our link database.
   //
   if (isOpen() == true && getContainer() != NULL)
      clearLinks();
   
   Box3F box0 = m_pITRInstance->getGeometry()->box;
   m_mul(box0, in_rTrans, out_pBox);

   out_pBox->fMax += Point3F(csm_BBoxExtension, csm_BBoxExtension, csm_BBoxExtension);
   out_pBox->fMin -= Point3F(csm_BBoxExtension, csm_BBoxExtension, csm_BBoxExtension);

   // And now recollide with the container system, and relink to all contacted
   //  bounding boxes...
   //
   if (isOpen() == true && getContainer() != NULL)
      findAllLinks();
}

void
SimInteriorShape::shapeFlagsChanged()
{
   // The only thing we care about is switching the collision and render images in the
   //  event that the shape has just been destroyed.
   //
   initRenderImage();

   Parent::shapeFlagsChanged();
}


//------------------------------------------------------------------------------
//--------------------------------------
// Resource management
//--------------------------------------
//
void
SimInteriorShape::initRenderImage()
{
   m_renderImage.pSimInteriorShape = this;
   m_renderImage.linkRendering     = false;
   m_renderImage.transform         = getTransform();

   // Make sure that the shape is in the proper state...
   //
   if (m_pITRInstance != NULL) {
      if (testShapeFlag(SFIsDestroyed) == false) {
         m_pITRInstance->setInteriorState(m_initialState);
         m_renderImage.currShape = m_pITRInstance;
      } else {
         if (m_destroyedState != -1) {
            m_pITRInstance->setInteriorState(m_destroyedState);
            m_renderImage.currShape = m_pITRInstance;
         } else {
            m_renderImage.currShape = NULL;
         }
      }
   } else {
      m_renderImage.currShape = NULL;
   }
}

// checks to see if the filename signifies that the shape has been 
// processed by the mission lighter
bool SimInteriorShape::missionLitName()
{
   char * ptr = m_pFileName;
   char * end = ptr + strlen( m_pFileName );

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
void SimInteriorShape::getBaseFilename( String & str )
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

bool
SimInteriorShape::reloadResources()
{
   bool isContainer = isOpen();
   setIsContainer(false);

   if (m_pITRInstance != NULL) {
      delete m_pITRInstance;
      m_pITRInstance = NULL;
   }
   
   if (m_pFileName == NULL) {
      setIsContainer(isContainer);
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
         setIsContainer(isContainer);
         return( false );
      }
      String base(m_pFileName);
      getBaseFilename( base );
      if( rm->findFile( base.c_str() ) )
         resShape = rm->load(base.c_str(), true );
      missionLit = false;
   }
   
   if (bool(resShape) == false) {
      initLightParams();
      setIsContainer(isContainer);
      return false;
   }
   
   // Otherwise we can move forward...
   //
   m_pITRInstance = new ITRInstance(rm, resShape, 0);
   if( missionLit )
      m_pITRInstance->setMissionLit();
   initRenderImage();

   set(getTransform(), true);

   setIsContainer(isContainer);
   return true;
}


//------------------------------------------------------------------------------
//--------------------------------------
// Light animation functions
//--------------------------------------
//
void
SimInteriorShape::shutdownLights()
{
   AssertFatal(m_lightThreads.size() == 0 || m_pITRInstance != NULL,
               "No instance, but we have threads?");
               
   for (int i = 0; i < m_lightThreads.size(); i++) {
      AssertFatal(m_lightThreads[i] != NULL, avar("No thread in slot %d", i));
      m_pITRInstance->destroyLightThread(m_lightThreads[i]);
      m_lightThreads[i] = NULL;
   }
   m_lightThreads.clear();
   m_lightAnimParams.clear();
}

void
SimInteriorShape::initLightParams()
{
   if (m_pITRInstance == NULL) {
      m_lightAnimParams.clear();
      return;
   }

   ITRLighting* pLighting = m_pITRInstance->getLighting();

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
SimInteriorShape::restartLights()
{
   AssertFatal(m_lightAnimParams.size() == 0 || m_pITRInstance != NULL,
               "No instance, but we need threads?");
   
   if (m_lightAnimParams.size() != 0) {
      ITRLighting* pLighting = m_pITRInstance->getLighting();

      if (pLighting->lightList.size() != m_lightAnimParams.size())
         // ?? Change in the art, just throw away the saved states
         return;

      m_lightThreads.setSize(m_lightAnimParams.size());
      for (int i = 0; i < m_lightAnimParams.size(); i++) {
         const ITRLighting::Light& rLLight = pLighting->lightList[i];
         
         m_lightThreads[i] = m_pITRInstance->createLightThread(rLLight.id);
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
// SimShape overrides
//--------------------------------------
//
UInt32
SimInteriorShape::getPartType() const
{
   return UInt32(Parent::SimInteriorShapeType);
}


//------------------------------------------------------------------------------
//--------------------------------------
// Event/Query Processing
//--------------------------------------
//
bool
SimInteriorShape::processQuery(SimQuery* query)
{
   switch (query->type) {
      onQuery(SimRenderQueryImage);
      onQuery(SimCollisionImageQuery);
      
     default:
      return Parent::processQuery(query);
   }
}


bool
SimInteriorShape::processEvent(const SimEvent* event)
{
   switch (event->type) {
      onEvent(SimEditEvent);
      onEvent(SimTimerEvent);
      onEvent(SimTriggerEvent);

     default:
      return Parent::processEvent(event);
   }
}

bool
SimInteriorShape::processArguments(int          argc,
                                   const char** argv)
{
   if (argc == 0) {
      setFileName(NULL);
   } else {
      setFileName(argv[0]);
      shutdownLights();
      reloadResources();
      initLightParams();
      restartLights();
   }
   
   return true;
}


//------------------------------------------------------------------------------
//--------------------------------------
// Event/Query Handlers
//--------------------------------------
//
bool
SimInteriorShape::onAdd()
{
   if (Parent::onAdd() == false)
      return false;

   addToSet(SimTimerSetId);
   m_lastUpdatedTime = manager->getCurrentTime();

   reloadResources();
   restartLights();
   
   // We always add ourselves to the container system before seting our container
   //  state, which simplifies the setIsContainer() function...
   //
   SimContainer* root = NULL;
   
   root = findObject(manager,SimRootContainerId,root);
   root->addObject(this);
   
   // Link us to whoever's around...
   //
   if (isOpen() == true) {
      clearLinks();
      findAllLinks();
   }
   
   return true;
}

bool
SimInteriorShape::onSimEditEvent(const SimEditEvent* event)
{
   if (event->state == true) {
      m_forceRender |= ForceEdit;
   } else {
      m_forceRender &= ~ForceEdit;
   }

   setMaskBits(EditModeMask);
   return true;
}   


bool
SimInteriorShape::onSimTimerEvent(const SimTimerEvent* event)
{
   float timeDelta = manager->getCurrentTime() - m_lastUpdatedTime;
   
   if (timeDelta > m_lightUpdateDelta) {
      for (int i = 0; i < m_lightThreads.size(); i++) {
         if (m_lightAnimParams[i].active == true)
            m_lightThreads[i]->advanceTime(timeDelta);
      }

      m_lastUpdatedTime = manager->getCurrentTime();
   }

   return Parent::onSimTimerEvent(event);
}

bool
SimInteriorShape::onSimTriggerEvent(const SimTriggerEvent* event)
{
   Parent::onSimTriggerEvent(event);

   // Make sure we have the right value
   int animation = int(event->value + 0.5);

   if (animation >= 0 && animation < m_lightAnimParams.size()) {
      // Only respond to triggers that are in range, respond by toggling that animation
      //  on or off...
      //
      switch (event->action)
      {
         case SimTriggerEvent::Activate:
            m_lightAnimParams[animation].active = true;
            break; 
         case SimTriggerEvent::Deactivate:
            m_lightAnimParams[animation].active = false;
            break; 
         case SimTriggerEvent::Toggle:
            m_lightAnimParams[animation].active = !m_lightAnimParams[animation].active;
            break; 
      }
      setMaskBits(ShapeLightActiveMask);
      return true;
   } else {
      return false;
   }
}

bool
SimInteriorShape::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   if (m_renderImage.currShape != NULL) {
      query->image[query->count++] = &m_renderImage;
      m_renderImage.transform      = getTransform();

      // get haze value from container
      SimContainer * cnt = getContainer();
      if (cnt)
         m_renderImage.hazeValue = cnt->getHazeValue(*query->renderContext,
                                                     m_renderImage.transform.p);

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
SimInteriorShape::onSimCollisionImageQuery(SimCollisionImageQuery* query)
{
   if ((m_disableCollisions == false &&
        m_pITRInstance      != NULL) &&
       (testShapeFlag(SFIsDestroyed) == false ||
        m_destroyedState != -1)) {
      query->image[0] = &m_collisionImage;
      query->count    = 1;
      
      m_collisionImage.instance  = m_pITRInstance;
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
SimInteriorShape::inspectRead(Inspect *inspector)
{
   Parent::inspectRead(inspector);
   inspector->readDivider();

   // We only process some of this event if the filename hasn't changed, and existed
   //  previous to the event...
   //
   bool processAll = m_pITRInstance != NULL;
   bool processAny = m_pITRInstance != NULL;

   int numAnimations = 0;
   if (processAll) {
      ITRLighting* pLighting = m_pITRInstance->getLighting();
      numAnimations = pLighting->lightList.size();
   }

   // Shape name
   char tempBuf[256];
   inspector->read(IDITG_FILENAME, tempBuf);
   if (m_pFileName != NULL &&
       stricmp(m_pFileName, tempBuf) == 0) {
      // File is the same, do nothing...
   } else if (strlen(tempBuf) == 0) {
      // Shape is being cleared...
      setFileName(NULL);
      processAll = false;
   } else {
      setFileName(tempBuf);
      shutdownLights();
      reloadResources();
      initLightParams();
      restartLights();
      processAll = false;
   }
   
   // IsContainer
   bool tempBool;
   inspector->read(IDITG_SIS_ISCONTAINER, tempBool);
   setIsContainer(tempBool);

   // Disable collisions
   inspector->read(IDITG_SIS_DISABLECOLLISIONS, m_disableCollisions);

   // Light update freqency
   inspector->read(IDITG_SIS_MAXLIGHTUPDATEPERSEC, m_maxLightUpdatePerSec);
   if (m_maxLightUpdatePerSec <= 0)
      m_maxLightUpdatePerSec = 1;
   m_lightUpdateDelta = 1.0f / float(m_maxLightUpdatePerSec);


   if (processAll == true) {
      shutdownLights();

      Int32 initState;
      inspector->readIStringPairList(IDITG_SIS_INITIALSTATE, initState);
      
      Int32 destState;
      inspector->readIStringPairList(IDITG_SIS_DESTROYEDSTATE, destState);
      
      setShapeStates(initState, destState);

      Int32 lightState;
      inspector->readIStringPairList(IDITG_SIS_LIGHTSTATE, lightState);
      setLightState(lightState);
   
      // Read any light animations...
      ITRLighting* pLighting = m_pITRInstance->getLighting();
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
      Int32 dummyInt;
      inspector->readIStringPairList(IDITG_SIS_INITIALSTATE,   dummyInt);
      inspector->readIStringPairList(IDITG_SIS_DESTROYEDSTATE, dummyInt);
      inspector->readIStringPairList(IDITG_SIS_LIGHTSTATE,     dummyInt);
      
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

void SimInteriorShape::inspectWrite(Inspect* inspector)
{
   int i;
   Parent::inspectWrite(inspector);
   inspector->writeDivider();

   // Shape name
   inspector->write(IDITG_FILENAME, m_pFileName);
   
   // IsContainer
   inspector->write(IDITG_SIS_ISCONTAINER, isOpen());
   
   // DisableCollisions
   inspector->write(IDITG_SIS_DISABLECOLLISIONS, m_disableCollisions);
   
   // Light update freqency
   inspector->write(IDITG_SIS_MAXLIGHTUPDATEPERSEC, m_maxLightUpdatePerSec);

   if (m_pITRInstance != NULL) {
      // Set up the IStringValPair list with the names of the states...
      //
      ITRShape* pShape = m_pITRInstance->getShape();
      
      // Initial State
      int numStates = pShape->getNumStates();
      Inspect::IStringValPair* pStatePairs = new Inspect::IStringValPair[numStates];
      for (i = 0; i < numStates; i++) {
         ITRShape::State& state = pShape->getState(i);
         pStatePairs[i].string  = pShape->getName(state.nameIndex);
         pStatePairs[i].val     = i;
      }

      // Destroyed State
      Inspect::IStringValPair* pDestStatePairs = new Inspect::IStringValPair[numStates + 1];
      static const char* noneSelected = "<NONE SELECTED>";
      pDestStatePairs[0].string = noneSelected;
      pDestStatePairs[0].val    = -1;
      for (int j = 0; j < numStates; j++) {
         ITRShape::State& state = pShape->getState(j);
         pDestStatePairs[j + 1].string  = pShape->getName(state.nameIndex);
         pDestStatePairs[j + 1].val     = j;
      }
      
      // LightState
      int numLightStates = pShape->getNumLightStates();
      Inspect::IStringValPair* pLightStatePairs = new Inspect::IStringValPair[numLightStates];
      for (int k = 0; k < numLightStates; k++) {
         pLightStatePairs[k].string  = pShape->getLightStateName(k);
         pLightStatePairs[k].val     = k;
      }

      inspector->writeIStringPairList(IDITG_SIS_INITIALSTATE,
                                      m_initialState,
                                      numStates,
                                      pStatePairs);
      inspector->writeIStringPairList(IDITG_SIS_DESTROYEDSTATE,
                                      m_destroyedState,
                                      (numStates + 1),
                                      pDestStatePairs);
      inspector->writeIStringPairList(IDITG_SIS_LIGHTSTATE,
                                      m_lightState,
                                      numLightStates,
                                      pLightStatePairs);
      delete [] pStatePairs;
      delete [] pDestStatePairs;
      delete [] pLightStatePairs;
      
      // Output any light animations...
      ITRLighting* pLighting = m_pITRInstance->getLighting();
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
DWORD SimInteriorShape::packUpdate(Net::GhostManager* gm,
                                   DWORD              mask,
                                   BitStream*         stream)
{
   UInt32 partialMask = Parent::packUpdate(gm, mask, stream);
   
   // Interior stuff
   //
   if (mask & InitialUpdate) {
      stream->writeFlag(true);
      
      if (m_pFileName != NULL) {
         int strLen = strlen(m_pFileName) + 1;
         stream->write(strLen);
         stream->write(strLen, m_pFileName);
      } else {
         stream->write(int(0));
      }
      
      stream->writeFlag(isOpen());
      
      stream->write(m_initialState);
      stream->write(m_destroyedState);
      stream->write(m_lightState);
      
      stream->write(m_maxLightUpdatePerSec);
      
      stream->writeFlag(m_disableCollisions);
      
      stream->write(m_lightAnimParams.size());
      stream->write(m_lightAnimParams.size() * sizeof(LightAnimParam),
                    (void*)m_lightAnimParams.address());
   } else {
      stream->writeFlag(false);
   }

   if (((mask & ShapeLightActiveMask) != 0) &&
       ((mask & InitialUpdate)        == 0)) {
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

void SimInteriorShape::unpackUpdate(Net::GhostManager* gm,
                                   BitStream*         stream)
{
   Parent::unpackUpdate(gm, stream);
   
   // Interior stuff
   //
   if (stream->readFlag() == true) {
      int strLen;
      stream->read(&strLen);
      
      if (strLen != 0) {
         char* tempBuf = new char[strLen];
         stream->read(strLen, tempBuf);
         setFileName(tempBuf);
         delete [] tempBuf;
      } else {
         setFileName(NULL);
      }
      
      bool isContainer = stream->readFlag();
      
      int initState, destState;
      int lightState;
      
      stream->read(&initState);
      stream->read(&destState);
      stream->read(&lightState);
      
      stream->read(&m_maxLightUpdatePerSec);
      if (m_maxLightUpdatePerSec == 0)
         m_maxLightUpdatePerSec = 1;
      m_lightUpdateDelta = 1.0f / float(m_maxLightUpdatePerSec);
      
      m_disableCollisions = stream->readFlag();

      shutdownLights();

      int size;
      stream->read(&size);
      m_lightAnimParams.setSize(size);
      stream->read(m_lightAnimParams.size() * sizeof(LightAnimParam),
                   (void*)m_lightAnimParams.address());

      if (manager != NULL) {
         reloadResources();
         restartLights();
         setIsContainer(isContainer);
         setShapeStates(initState, destState);
         setLightState(lightState);
      } else {
         // First update, just set, and wait for onAdd()
         //
         setIsContainer(isContainer);
         m_initialState   = initState;
         m_destroyedState = destState;
         m_lightState     = lightState;
      }
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

Persistent::Base::Error 
SimInteriorShape::read(StreamIO& sio,
                           int   ignore_version,
                           int   user)
{
#if 1
   if (Console->getBoolVariable("SimShapeOldFormat")) {
      if (Parent::read(sio, ignore_version, user) != Ok) {
         return ReadError;
      }
      int version;
      sio.read(&version);
   }
   else {
      int version;
      sio.read(&version);
      AssertFatal(SimInteriorShape::version(),
                  "Wrong file version: SimInteriorShape");
      if (Parent::read(sio, ignore_version, user) != Ok) {
         return ReadError;
      }
   }
#else
   int version;
   sio.read(&version);
   AssertFatal(SimInteriorShape::version(),
               "Wrong file version: SimInteriorShape");
   if (Parent::read(sio, ignore_version, user) != Ok) {
      return ReadError;
   }
#endif

   String tempFileName;
   sio.read(&tempFileName);
   if (tempFileName.c_str() != NULL) {
      m_pFileName = new char[strlen(tempFileName.c_str()) + 1];
      strcpy(m_pFileName, tempFileName.c_str());
   } else {
      m_pFileName = NULL;
   }

   sio.read(&m_isContainer);
   setIsContainer(m_isContainer);

   sio.read(&m_initialState);
   sio.read(&m_destroyedState);
   sio.read(&m_lightState);

   sio.read(&m_maxLightUpdatePerSec);
   m_lightUpdateDelta = 1.0f / float(m_maxLightUpdatePerSec);

   sio.read(&m_disableCollisions);

   int size;
   sio.read(&size);
   m_lightAnimParams.setSize(size);
   sio.read(m_lightAnimParams.size() * sizeof(LightAnimParam),
            (void*)m_lightAnimParams.address());

   return (sio.getStatus() == STRM_OK) ? Ok : ReadError;
}

Persistent::Base::Error 
SimInteriorShape::write(StreamIO& sio,
                            int   ignore_version,
                            int   user)
{
   sio.write(SimInteriorShape::version());

   if (Parent::write(sio, ignore_version, user) != Ok) {
      return WriteError;
   }

   String tempFileName(m_pFileName);
   sio.write(tempFileName);

   sio.write(isOpen());
   
   sio.write(m_initialState);
   sio.write(m_destroyedState);
   sio.write(m_lightState);

   sio.write(m_maxLightUpdatePerSec);

   sio.write(m_disableCollisions);

   sio.write(m_lightAnimParams.size());
   sio.write(m_lightAnimParams.size() * sizeof(LightAnimParam),
             (void*)m_lightAnimParams.address());

   return (sio.getStatus() == STRM_OK) ? Ok : WriteError;
}

int SimInteriorShape::version()
{
   return sm_fileVersion;
}

//------------------------------------------------------------------------------
//--------------------------------------
// Rendering functions
//--------------------------------------
//
SimShapeImageDet::ShapeImageType
SimInteriorShape::RenderImage::getImageType() const
{
   return ITRImage;
}

bool
SimInteriorShape::render(SimContainerRenderContext& rc)
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
SimInteriorShape::scope(SimContainerScopeContext& sc)
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
SimInteriorShape::scopeLinks(SimContainerScopeContext& sc)
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
            correctedOutsideBits |= FaceMinX;
         if (localCamPos.y < shapeBox.fMin.y && fabs(localCamPos.y - shapeBox.fMin.y) < csm_BBoxExtension)
            correctedOutsideBits |= FaceMinY;
         if (localCamPos.z < shapeBox.fMin.z && fabs(localCamPos.z - shapeBox.fMin.z) < csm_BBoxExtension)
            correctedOutsideBits |= FaceMinZ;
         if (localCamPos.x > shapeBox.fMax.x && fabs(localCamPos.x - shapeBox.fMax.x) < csm_BBoxExtension)
            correctedOutsideBits |= FaceMaxX;
         if (localCamPos.y > shapeBox.fMax.y && fabs(localCamPos.y - shapeBox.fMax.y) < csm_BBoxExtension)
            correctedOutsideBits |= FaceMaxY;
         if (localCamPos.z > shapeBox.fMax.z && fabs(localCamPos.z - shapeBox.fMax.z) < csm_BBoxExtension)
            correctedOutsideBits |= FaceMaxZ;
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
SimInteriorShape::RenderImage::render(TS::RenderContext &rc)
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

   if (pSimInteriorShape->isOpen() == true) {
      // Note that this does NOT take effect until the next frame.  Maybe the
      //  detail selection process could be moved to a higher level if this
      //  becomes a problem..
      //
      if (oldDetailLevel != newDetailLevel) {
         // If the new links aren't the same as the old, fix them up...
         //
         pSimInteriorShape->clearLinks();
         pSimInteriorShape->findAllLinks();
      }
   }

   rend.render(rc, currShape);

   // Draw the bounding box if the flag is set...
   //
   if (SimShape::getDrawShapeBBox() == true) {
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
      Box3F cntBbox = pSimInteriorShape->getBoundingBox();
      cntBbox.fMax -= Point3F(csm_BBoxExtension, csm_BBoxExtension, csm_BBoxExtension);
      cntBbox.fMin += Point3F(csm_BBoxExtension, csm_BBoxExtension, csm_BBoxExtension);
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
      rc.getCamera()->popTransform();
   }
}

//----------------------------------------------------------------------------
//-----------------------------------------
// Interior Link Management
//-----------------------------------------
//
void
SimInteriorShape::unlinkMe(const SimInteriorShape* in_pLinked)
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
SimInteriorShape::linkMe(SimInteriorShape* in_pNewLink,
                         LinkVertexArena*  io_pArena)
{
   if (m_renderImage.currShape == NULL)
      return false;
      
   // We have to transform the incoming interior's bounding box into our
   //  coordinate space to find the abutting face.
   //
   Box3F myBoundingBox = m_renderImage.currShape->getHighestBoundingBox();
   Box3F otherBBox     = getITRSpaceBBox(in_pNewLink);
   LinkFace linkFace = getAbuttingFace(myBoundingBox, otherBBox);
   
   if (linkFace == FaceNone ||
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
SimInteriorShape::clearLinks()
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
SimInteriorShape::findAllLinks()
{
   if (manager == NULL || m_renderImage.currShape == NULL)
      return;

   // Query the Container database to determine which Containable objects
   //  are affected by this light...
   //
   SimContainer* root = NULL;
   
   root = findObject(manager, SimRootContainerId, root);
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
      SimInteriorShape* pOtherItr  = static_cast<SimInteriorShape*>(scList[i]);

      UInt32 properMask = pContainer->getType() & (SimInteriorObjectType | SimShapeObjectType);
      if (properMask != (SimInteriorObjectType | SimShapeObjectType))
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
      LinkFace linkFace = getAbuttingFace(myBoundingBox, otherBBox);
      
      if (linkFace == FaceNone ||
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
SimInteriorShape::renderLinks(SimContainerRenderContext& rc)
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
            correctedOutsideBits |= FaceMinX;
         if (localCamPos.y < shapeBox.fMin.y && fabs(localCamPos.y - shapeBox.fMin.y) < csm_BBoxExtension)
            correctedOutsideBits |= FaceMinY;
         if (localCamPos.z < shapeBox.fMin.z && fabs(localCamPos.z - shapeBox.fMin.z) < csm_BBoxExtension)
            correctedOutsideBits |= FaceMinZ;
         if (localCamPos.x > shapeBox.fMax.x && fabs(localCamPos.x - shapeBox.fMax.x) < csm_BBoxExtension)
            correctedOutsideBits |= FaceMaxX;
         if (localCamPos.y > shapeBox.fMax.y && fabs(localCamPos.y - shapeBox.fMax.y) < csm_BBoxExtension)
            correctedOutsideBits |= FaceMaxY;
         if (localCamPos.z > shapeBox.fMax.z && fabs(localCamPos.z - shapeBox.fMax.z) < csm_BBoxExtension)
            correctedOutsideBits |= FaceMaxZ;
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
SimInteriorShape::getITRSpaceBBox(const SimInteriorShape* io_pOtherItr) const
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

LinkFace
SimInteriorShape::getAbuttingFace(const Box3F& in_rMyBox,
                                  const Box3F& in_rOtherBox) const
{
   if (fabs(in_rMyBox.fMin.x - in_rOtherBox.fMax.x) < sg_BBoxTolerance) {
      if (testFaces(in_rMyBox.fMin.y, in_rMyBox.fMax.y, in_rOtherBox.fMin.y, in_rOtherBox.fMax.y) &&
          testFaces(in_rMyBox.fMin.z, in_rMyBox.fMax.z, in_rOtherBox.fMin.z, in_rOtherBox.fMax.z)) {
         return FaceMinX;
      }
   }
   if (fabs(in_rMyBox.fMax.x - in_rOtherBox.fMin.x) < sg_BBoxTolerance) {
      if (testFaces(in_rMyBox.fMin.y, in_rMyBox.fMax.y, in_rOtherBox.fMin.y, in_rOtherBox.fMax.y) &&
          testFaces(in_rMyBox.fMin.z, in_rMyBox.fMax.z, in_rOtherBox.fMin.z, in_rOtherBox.fMax.z)) {
         return FaceMaxX;
      }
   }
   if (fabs(in_rMyBox.fMin.y - in_rOtherBox.fMax.y) < sg_BBoxTolerance) {
      if (testFaces(in_rMyBox.fMin.x, in_rMyBox.fMax.x, in_rOtherBox.fMin.x, in_rOtherBox.fMax.x) &&
          testFaces(in_rMyBox.fMin.z, in_rMyBox.fMax.z, in_rOtherBox.fMin.z, in_rOtherBox.fMax.z)) {
         return FaceMinY;
      }
   }
   if (fabs(in_rMyBox.fMax.y - in_rOtherBox.fMin.y) < sg_BBoxTolerance) {
      if (testFaces(in_rMyBox.fMin.x, in_rMyBox.fMax.x, in_rOtherBox.fMin.x, in_rOtherBox.fMax.x) &&
          testFaces(in_rMyBox.fMin.z, in_rMyBox.fMax.z, in_rOtherBox.fMin.z, in_rOtherBox.fMax.z)) {
         return FaceMaxY;
      }
   }
   if (fabs(in_rMyBox.fMin.z - in_rOtherBox.fMax.z) < sg_BBoxTolerance) {
      if (testFaces(in_rMyBox.fMin.x, in_rMyBox.fMax.x, in_rOtherBox.fMin.x, in_rOtherBox.fMax.x) &&
          testFaces(in_rMyBox.fMin.y, in_rMyBox.fMax.y, in_rOtherBox.fMin.y, in_rOtherBox.fMax.y)) {
         return FaceMinZ;
      }
   }
   if (fabs(in_rMyBox.fMax.z - in_rOtherBox.fMin.z) < sg_BBoxTolerance) {
      if (testFaces(in_rMyBox.fMin.x, in_rMyBox.fMax.x, in_rOtherBox.fMin.x, in_rOtherBox.fMax.x) &&
          testFaces(in_rMyBox.fMin.y, in_rMyBox.fMax.y, in_rOtherBox.fMin.y, in_rOtherBox.fMax.y)) {
         return FaceMaxZ;
      }
   }

   return FaceNone;
}


int
SimInteriorShape::correctOutsideBits(const int       in_oldBits,
                                     TS::Camera*     io_camera,
                                     TS::PointArray* io_pointArray)
{
   static const struct _FaceData {
      int   index0;
      int   index1;
      int   index2;
      int   index3;
      int   failMask;
   } faceTestData[6] = { {0, 1, 2, 3, ~(FaceMinX)},
                         {0, 3, 7, 4, ~(FaceMinY)},
                         {0, 4, 5, 1, ~(FaceMinZ)},
                         {4, 7, 6, 5, ~(FaceMaxX)},
                         {5, 6, 2, 1, ~(FaceMaxY)},
                         {2, 6, 7, 3, ~(FaceMaxZ)} };

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
SimInteriorShape::getWeatherDistance(const Point3F& in_rWorldPos,
                                     float&         out_rDistance)
{
   ITRInstance* pInstance = getInstance();
   if (pInstance == NULL ||
       pInstance->isLinked() == true)
      return false;

   Point3F interiorPos;
   m_mul(in_rWorldPos, getInvTransform(), &interiorPos);

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
   if ((camPosOutsideBits & ITRGeometry::OutsideMinX) != 0)
      if ((interiorPos.x - pGeometry->box.fMin.x) > maxDist)
         maxDist = interiorPos.x - pGeometry->box.fMin.x;
   if ((camPosOutsideBits & ITRGeometry::OutsideMinY) != 0)
      if ((interiorPos.y - pGeometry->box.fMin.y) > maxDist)
         maxDist = interiorPos.y - pGeometry->box.fMin.x;
   if ((camPosOutsideBits & ITRGeometry::OutsideMinZ) != 0)
      if ((interiorPos.z - pGeometry->box.fMin.z) > maxDist)
         maxDist = interiorPos.z - pGeometry->box.fMin.x;
   if ((camPosOutsideBits & ITRGeometry::OutsideMaxX) != 0)
      if ((pGeometry->box.fMin.x - interiorPos.x) > maxDist)
         maxDist = pGeometry->box.fMax.x - interiorPos.x;
   if ((camPosOutsideBits & ITRGeometry::OutsideMaxY) != 0)
      if ((pGeometry->box.fMin.y - interiorPos.y) > maxDist)
         maxDist = pGeometry->box.fMax.y - interiorPos.y;
   if ((camPosOutsideBits & ITRGeometry::OutsideMaxZ) != 0)
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
   m_interior0LinkFace(FaceNone),
   m_interior1LinkFace(FaceNone)
{
   //
}

LinkVertexArena::~LinkVertexArena()
{
   AssertFatal(m_pInterior0 == NULL && m_pInterior1 == NULL,
               "Object still referenced by this Arena");
}

void
LinkVertexArena::addInterior(SimInteriorShape* io_pAdd,
                             const LinkFace    in_linkFace)
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
LinkVertexArena::removeInterior(SimInteriorShape* io_pRemove)
{
   AssertFatal(io_pRemove != NULL, "Can't remove NULL Interior!");
   AssertFatal(m_pInterior0 != NULL || m_pInterior1 != NULL,
               "Error, arena membership already cleared");

   if (io_pRemove == m_pInterior0) {
      m_pInterior0 = NULL;
      m_interior0LinkFace = FaceNone;
      if (m_pInterior1 == NULL)
         delete this;
   } else if (io_pRemove == m_pInterior1) {
      m_pInterior1 = NULL;
      m_interior0LinkFace = FaceNone;
      if (m_pInterior0 == NULL)
         delete this;
   } else {
      AssertFatal(0, "Error, interior is not an arena member...");
   }
}


//------------------------------------------------------------------------------
//--------------------------------------
// SimInteriorContainer methods...
//--------------------------------------

//------------------------------------------------------------------------------
//-------------------------------------- SIMPLE DATABASE
//
SimInteriorShapeDatabaseSimple::SimInteriorShapeDatabaseSimple(SimInteriorShape* in_pInteriorShape)
 : m_pInteriorShape(in_pInteriorShape)
{
   //
}

SimInteriorShapeDatabaseSimple::~SimInteriorShapeDatabaseSimple()
{
   m_pInteriorShape = NULL;
}

void
SimInteriorShapeDatabaseSimple::render(SimContainerRenderContext& rc)
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
SimInteriorShapeDatabaseSimple::scope(SimContainerScopeContext& sc)
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
SimInteriorShapeDatabasePVS::SimInteriorShapeDatabasePVS(SimInteriorShape* io_pInteriorShape)
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
                         m_pDatabaseGeometry->solidLeafList.size());
   for (int i = 0; i < m_bspNodeList.size(); i++) {
      m_bspNodeList[i].start = 0xffff;
      m_bspNodeList[i].count = 0;
   }
}

SimInteriorShapeDatabasePVS::~SimInteriorShapeDatabasePVS()
{
   m_pInteriorShape    = NULL;
   m_pDatabaseGeometry = NULL;
}

Int32
SimInteriorShapeDatabasePVS::findLeaf(const Point3F& p, Int32 nodeIndex)
{
   if (nodeIndex < 0)
      return -(nodeIndex+1);

   ITRGeometry::BSPNode& node = m_pDatabaseGeometry->nodeList[nodeIndex];
   if (m_pDatabaseGeometry->planeList[node.planeIndex].whichSide(p) == TPlaneF::Inside)
      return findLeaf(p, node.front);
   return findLeaf(p, node.back);
}

Int32
SimInteriorShapeDatabasePVS::findBSPLeaf(const Point3F& in_rPoint)
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
SimInteriorShapeDatabasePVS::findBSPLeaf(SimContainer* io_pContainer)
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
SimInteriorShapeDatabasePVS::enterBSPObject(ObjectEntry& in_rEntry)
{
   AssertFatal(in_rEntry.bspLeaf == 0, "Error, already in BSP database");
   
   in_rEntry.bspLeaf = findBSPLeaf(in_rEntry.pContainer);
   
   BSPNodeEntry& rNodeEntry = m_bspNodeList[in_rEntry.bspLeaf];
   
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

void
SimInteriorShapeDatabasePVS::removeBSPObject(ObjectEntry& in_rEntry)
{
   AssertFatal(in_rEntry.bspLeaf != 0, "Error, not in BSP database");
   
   BSPNodeEntry& rNodeEntry = m_bspNodeList[in_rEntry.bspLeaf];
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
SimInteriorShapeDatabasePVS::add(SimContainer* io_pContainer)
{
   ObjectEntry entry;
   entry.pContainer = io_pContainer;
   entry.bspLeaf    = 0;

   enterBSPObject(entry);
   m_objectList.push_back(entry);

   return true;
}

bool
SimInteriorShapeDatabasePVS::remove(SimContainer* io_pRemove)
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
SimInteriorShapeDatabasePVS::update(SimContainer* io_pUpdate)
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
SimInteriorShapeDatabasePVS::removeAll(SimContainer* io_pParent)
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
SimInteriorShapeDatabasePVS::findContained(const SimContainerQuery& query,
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
SimInteriorShapeDatabasePVS::findOpenContainer(const SimContainerQuery& query)
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
SimInteriorShapeDatabasePVS::findIntersections(const SimContainerQuery& query,
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
SimInteriorShapeDatabasePVS::scopeNode(SimContainerScopeContext& sc,
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
SimInteriorShapeDatabasePVS::renderNode(SimContainerRenderContext& rc,
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
SimInteriorShapeDatabasePVS::render(SimContainerRenderContext& rc)
{
   TS::Camera* tsCamera = rc.currentState().renderContext->getCamera();
   Point3F localCamPos;
   m_mul(tsCamera->getTCW().p, m_pInteriorShape->getInvTransform(), &localCamPos);

   // And the leaf associated with it...
   //
   Int32 leafIndex = findBSPLeaf(localCamPos);
   ITRGeometry::BSPLeafWrap leafWrap(m_pDatabaseGeometry, leafIndex);

   // Nothing is visibile from a solid node...
   if (leafWrap.isSolid())
      return;

	UInt8* pbegin = &m_pDatabaseGeometry->bitList[leafWrap.getPVSIndex()];
	UInt8* pend   = pbegin + leafWrap.getPVSCount();
	for (ITRCompressedBitVector::iterator itr(pbegin,pend); ++itr; ) {
      // Only scope the outside leafs if the camera is inside the
      //  interior proper...
      //
      
      // DMMNOTE: TEMPORARY FIX UNTIL THE ART IS REBUILT...
      //
      if (leafIndex >= 43 || *itr >= 43) {
         BSPNodeEntry& rNode = m_bspNodeList[*itr];
         if (rNode.count > 0)
            renderNode(rc, rNode);
      }
   }
   if (leafIndex < 43) {
      for (int i = 1; i < 43; i++) {
         BSPNodeEntry& rNode = m_bspNodeList[i];
         if (rNode.count > 0)
            renderNode(rc, rNode);
      }
   }
}

void
SimInteriorShapeDatabasePVS::scope(SimContainerScopeContext& sc)
{
   // First, get the local camera position...
   //
   Point3F localCamPos;
   m_mul(sc.currentState().cameraInfo->pos, m_pInteriorShape->getInvTransform(), &localCamPos);

   // And the leaf associated with it...
   //
   Int32 leafIndex = findBSPLeaf(localCamPos);
   ITRGeometry::BSPLeafWrap leafWrap(m_pDatabaseGeometry, leafIndex);

   // Nothing is visibile from a solid node...
   if (leafWrap.isSolid())
      return;

	UInt8* pbegin = &m_pDatabaseGeometry->bitList[leafWrap.getPVSIndex()];
	UInt8* pend   = pbegin + leafWrap.getPVSCount();
	for (ITRCompressedBitVector::iterator itr(pbegin,pend); ++itr; ) {
      // Only scope the outside leafs if the camera is inside the
      //  interior proper...
      //
      // DMMNOTE: TEMPORARY FIX UNTIL THE ART IS REBUILT...
      //
      if (leafIndex >= 43 || *itr >= 43) {
         BSPNodeEntry& rNode = m_bspNodeList[*itr];
         if (rNode.count > 0)
            scopeNode(sc, rNode);
      }
   }
   if (leafIndex < 43) {
      for (int i = 1; i < 43; i++) {
         BSPNodeEntry& rNode = m_bspNodeList[i];
         if (rNode.count > 0)
            scopeNode(sc, rNode);
      }
   }
}

