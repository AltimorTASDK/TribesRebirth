//----------------------------------------------------------------------------
// SimObject wrapper for an interior geometry object
//----------------------------------------------------------------------------

#include "ml.h"
#include <itrDynamicLight.h>
#include <sim.h>
#include <console.h>
#include "simInterior.h"
#include "simAction.h"
#include "simDynamicLight.h"
#include "editor.strings.h"
#include <simPersistTags.h>
#include "simResource.h"
#include "simTimerGrp.h"
#include "stringTable.h"

//----------------------------------------------------------------------------
// Persistent data members and macros...
//
IMPLEMENT_PERSISTENT_TAG(SimInterior, SimInteriorPersTag);
const int SimInterior::sm_fileVersion = 3;

// If this flag is true, then we draw the bounding box around interiors...
//
bool g_drawSimInteriorBBox = false;


#define __OBJECT_BEGINS
//------------------------------------------------------------------------------
//--------------------------------------
// Constructors/Destructors
//--------------------------------------

SimInterior::SimInterior():
   m_pFilename(NULL),
   m_currentState(0)
{
   type = SimInteriorObjectType;

   renderImage.pSimInterior = this;
   renderImage.instance     = NULL;
   renderImage.itype        = SimRenderImage::Normal;
   collisionImage.instance = NULL;

   set(EulerF (0, 0, 0), Point3F (0, 0, 0));

   netFlags.set(ScopeAlways);
   netFlags.set(Ghostable);
}

SimInterior::~SimInterior()
{
   delete renderImage.instance;
   renderImage.pSimInterior = NULL;
}


//------------------------------------------------------------------------------
//--------------------------------------
// Object state functions
//--------------------------------------

bool
SimInterior::setFilename(const char* file)
{
   // Test to make sure this is a proper filename...
   //                 
   if (file == NULL)
      return false;
   
   // if the filename is the same, do nothing...
   //
   if (m_pFilename != NULL && stricmp(file, m_pFilename) == 0)
      return false;

   const char* ext = strrchr(file, '.');
   if (!ext ||
       stricmp(ext, ITRShape::sm_pShapeFileExtension) != 0) {
      return false;
   }

   if (m_pFilename == NULL ||
       stricmp(file, m_pFilename) != 0) {
      m_pFilename = stringTable.insert(file);
      setMaskBits(FileNameMask);
   }
   return true;
}

bool
SimInterior::setState(const UInt32 in_state)
{
   ITRInstance* pInstance = renderImage.instance;
   
   if (pInstance->setInteriorState(in_state) == false)
      return false;
   
   m_currentState = in_state;

   updateBoundingBox();
   setMaskBits(InteriorStateMask);
   return true;
}

bool
SimInterior::setLightState(const UInt32 in_lightState)
{
   ITRInstance* pInstance = renderImage.instance;
   ResourceManager* rm = SimResource::get(manager);
   
   if (pInstance->setInteriorLightState(rm, in_lightState) == false)
      return false;
   
   m_currentLightState = in_lightState;

   updateBoundingBox();
   setMaskBits(InteriorStateMask);
   return true;
}


void SimInterior::set(const TMat3F& mat, bool force)
{
   // Let's do as little work as possible here...
   //
   // always set it on add.
   if (!force && memcmp(&renderImage.transform, &mat, sizeof(TMat3F)) == 0) {
      return;
   }
   
   renderImage.transform = mat;
   renderImage.inverseTransform = mat;
   renderImage.inverseTransform.inverse();

   updateBoundingBox();
   setMaskBits(TransformMask);
}

void SimInterior::set(const EulerF& e,const Point3F& p)
{
   TMat3F mat;
   mat.set(e,p);
   set(mat);
}

//---------------------------------------------------------------------------
//---------------------------------------------
// Persistent functions/Network update (un)pack
//---------------------------------------------

DWORD
SimInterior::packUpdate(Net::GhostManager* /*gm*/,
                        DWORD              mask,
                        BitStream*         stream)
{
   if (mask & FileNameMask &&
       m_pFilename != NULL) {
      stream->writeFlag(true);
      stream->writeString(m_pFilename);
   } else {
      stream->writeFlag(false);
   }

   if (mask & TransformMask) {
      stream->writeFlag(true);

      TMat3F mat = getTransform();
      Point3F pnt = mat.p;
      EulerF  ang;
      mat.angles(&ang);
      stream->write(sizeof(pnt), &pnt);
      stream->write(sizeof(ang), &ang);
   } else {
      stream->writeFlag(false);
   }
   
   if (mask & InteriorStateMask) {
      stream->writeFlag(true);
      
      // We'll limit interiors to 32 states for the moment
      //
      UInt32 currentState = renderImage.instance->getInteriorState();
      AssertFatal(currentState < 32, "State overflow");
      stream->writeInt(currentState, 5);
   } else {
      stream->writeFlag(false);
   }

   return 0;
}

void
SimInterior::unpackUpdate(Net::GhostManager* /*gm*/,
                          BitStream*         stream)
{
   if (stream->readFlag()) {
      const char *temp = stream->readSTString();

      // If this is the first update, set the filename directly, and let
      //  the onAddNotify handler load the resources, else, load directly...
      //
      if (manager == NULL) {
         setFilename(temp);
      } else {
         loadShape(temp);
      }
   }

   if (stream->readFlag()) {
      Point3F tmpPnt;
      EulerF  tmpAng;
      stream->read(sizeof(tmpPnt), &tmpPnt);
      stream->read(sizeof(tmpAng), &tmpAng);
      set(tmpAng, tmpPnt);
   }
   
   if (stream->readFlag()) {
      
      // We'll limit interiors to 32 states for the moment
      //
      m_currentState = stream->readInt(5);
      if (manager)
         setState(m_currentState);
   }
}

void SimInterior::initPersistFields()
{
   addField("xformFlags", TypeInt, Offset(renderImage.transform.flags, SimInterior));
   addField("xformXAxis", TypePoint3F, Offset(renderImage.transform.m[0][0], SimInterior));
   addField("xformYAxis", TypePoint3F, Offset(renderImage.transform.m[1][0], SimInterior));
   addField("xformZAxis", TypePoint3F, Offset(renderImage.transform.m[2][0], SimInterior));
   addField("xformTranslate", TypePoint3F, Offset(renderImage.transform.p, SimInterior));
   addField("filename", TypeString, Offset(m_pFilename, SimInterior));
}

Persistent::Base::Error
SimInterior::read(StreamIO &sio, int in_version, int)
{
   AssertFatal(in_version == sm_fileVersion || in_version == 2,
		"Error, incorrect object verison");

   Parent::read (sio, 0, 0);
   
   Point3F position;
   EulerF rotation;
   sio.read (sizeof (rotation), &rotation);
   sio.read (sizeof (position), &position);
   set (rotation, position);

   int size;
   sio.read (&size);

   if (size) {
      char buf[256];
      sio.read(size, buf);
      buf[size] = 0;
      m_pFilename = stringTable.insert(buf);
   }
   else
      m_pFilename = 0;

	if (in_version == 2) {
		bool tmp;
	   sio.read(&tmp);
	}

   return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}

Persistent::Base::Error
SimInterior::write(StreamIO &sio, int, int)
{
   Parent::write (sio, 0, 0);
   
   EulerF rot;
   getTransform().angles(&rot);

   sio.write (sizeof (rot), &rot);
   sio.write (sizeof (Point3F), &getTransform().p);

   int size = m_pFilename ? strlen (m_pFilename): 0;
   sio.write (size);
   if (size)
      sio.write (size, m_pFilename);

   return (sio.getStatus() == STRM_OK)? Ok: WriteError;
}

int
SimInterior::version()
{
   return sm_fileVersion;
}

//------------------------------------------------------------------------------
//-----------------------------------------
// Object loading, initialization functions
//-----------------------------------------

void
SimInterior::unloadResources()
{
   if (renderImage.instance) {
      delete renderImage.instance;
      renderImage.instance = NULL;
   }
}


// checks to see if the filename signifies that the shape has been 
// processed by the mission lighter
bool SimInterior::missionLitName()
{
   char * ptr = (char *) m_pFilename;
   char * end = ptr + strlen( m_pFilename );

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
void SimInterior::getBaseFilename( String & str )
{
   // check if not mission lit and return the filename
   if( !missionLitName() )
   {
      str = m_pFilename;
      return;
   }
   
   char * file = new char[ strlen( m_pFilename ) + 1 ];
   strcpy( file, m_pFilename );
   
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
SimInterior::loadShape(const char* fileName)
{
   // setFilename returns false if the filename is invalid, OR if the filename
   //  is the same as the one already set.  In either case, we exit wo/ doing
   //  any work...
   //
   if (setFilename(fileName) == false) {
      return false;
   }
   
   // NOTE: This function is VERY poor on error checking, there are only a few
   //  asserts in ITRInstance().  Maybe restructure to be a bit more robust?
   //
   ResourceManager *rm = SimResource::get(manager);

   Resource<ITRShape> itrShape;
    
   bool missionLit;  
   // check if we need to try and find the missionlit ver
   if( rm->findFile( fileName ) )
   {
      missionLit = missionLitName();
      itrShape = rm->load( fileName);
   }
   else
   {
      if( !missionLitName() )
         return( false );
      String base = String(fileName);
      getBaseFilename( base );
      if( rm->findFile( base.c_str() ) )
         itrShape = rm->load(base.c_str());
      missionLit = false;
   }
   
   if( !bool( itrShape ) )
      return( false );

   // If we make it to here, then all is cool, nuke the old resources...
   //
   unloadResources();

   ITRInstance* pInstance = new ITRInstance(rm, itrShape, 0);
   if( missionLit )
      pInstance->setMissionLit();
   renderImage.instance = pInstance;

   // Set the geometry for the database and collision image.  Note that this
   //  is the highest level of state 0 for the interior.  May have to change
   //  the collision image geometry pointer on detail level change, probably
   //  will only change the database pointer on state switches...
   //
   updateBoundingBox();
	SimContainer* root = NULL;
   
   root = findObject(manager,SimRootContainerId,root);
	root->addObject(this);

   getInstance()->getAutoStartIDs(animatingLights);

   SimSet* pITRTimerSet = dynamic_cast<SimSet*>(manager->findObject(SimITRTimerSetId));
   if (pITRTimerSet == NULL)
      manager->addObject(new SimTimerSet(1.0f/15.0f, SimITRTimerSetId));
   bool timerSuccess = addToSet(SimITRTimerSetId);
   AssertFatal(timerSuccess == true, "Could not add to SimITRTimerSet");


   return true;
}


bool
SimInterior::processArguments(int argc, const char **argv)
{
   CMDConsole *console = CMDConsole::getLocked();
   if(argc < 1) {
      console->printf("SimInterior: filename.dis [posX posY posZ] [rotX rotY rotZ]");
      return false;
   }

   if(!loadShape(argv[0])) {
      console->printf("SimInterior: unable to load interior shape %s", argv[0]);
      return false;
   }

   if (argc > 1) {
      float f[6];
      memset(f,0,sizeof(f));
      for (int i = 1; i < argc; i++)
         sscanf(argv[i], "%f", &f[i - 1]);
      const float p = float(M_PI/180.0);
      set(EulerF(p*f[3],p*f[4],p*f[5]),Point3F(f[0],f[1],f[2]));
   }

   return true;
}

//------------------------------------------------------------------------------
//--------------------------------------
// Event/Query Processing
//--------------------------------------

bool SimInterior::processEvent(const SimEvent* event)
{
   switch (event->type) {
      onEvent(SimObjectTransformEvent);
      onEvent(SimTimerEvent);
      
     default:
      return Parent::processEvent(event);
   }
}

bool SimInterior::processQuery(SimQuery* query)
{
   switch (query->type){
      onQuery(SimRenderQueryImage);
      onQuery(SimCollisionImageQuery);
      onQuery(SimObjectTransformQuery);
      
     default:
      return Parent::processQuery(query);
   }
}


bool
SimInterior::onSimObjectTransformEvent(const SimObjectTransformEvent *event)
{           
   set(event->tmat);
   return true;
}

bool
SimInterior::onAdd()
{
   if (Parent::onAdd() == false)
      return false;

   set(renderImage.transform, true);

   if (m_pFilename) {
      const char* pFilename = m_pFilename;
      m_pFilename = NULL;
      processArguments(1, (const char**)&pFilename);
      setState(m_currentState);
   }
   updateBoundingBox();

   return true;
}

bool
SimInterior::onSimTimerEvent(const SimTimerEvent* event)
{
   Vector<int> stoppedLights;
   
   for (int i = 0; i < animatingLights.size(); i++) {
      bool contAnim = getInstance()->stepLightTime(animatingLights[i],
                                                   event->timerInterval);
      if (contAnim == false)
         stoppedLights.push_back(i);
   }
   
   if (stoppedLights.empty() == false) {
      for (int j = stoppedLights.size() - 1; j >= 0; j--) {
         // This is really slow if there are many stopped lights and many
         // animating lights, but it will do for now...
         //
         animatingLights.erase(stoppedLights[j]);
      }
   }

   return true;
}


//----------------------------------------------------------------------------

bool SimInterior::onSimRenderQueryImage(SimRenderQueryImage* query)
{
   if (renderImage.instance) {
      // get haze value from container
      SimContainer * cnt = getContainer();
      if (cnt)
         renderImage.hazeValue = cnt->getHazeValue(*query->renderContext,renderImage.transform.p);

      // Return the image
      //
      query->count    = 1;
      query->image[0] = &renderImage;
   } else {
      query->count = 0;
   }

   return true;
}

bool SimInterior::onSimCollisionImageQuery(SimCollisionImageQuery* query)
{
   collisionImage.instance  = getInstance();
   collisionImage.transform = getTransform();
   query->count    = 1;
   query->image[0] = &collisionImage;

   return true;
}

bool SimInterior::onSimObjectTransformQuery(SimObjectTransformQuery *query)
{
   query->centerOffset.set();
   query->tmat = getTransform();

   return true;   
}


void
SimInterior::inspectWrite(Inspect* sd)
{
   Parent::inspectWrite(sd);

   // Filename edit
   sd->write(IDITG_FILENAME, (char *) m_pFilename);

   TMat3F mat = getTransform();
   EulerF rot;
   mat.angles(&rot);
   sd->write(IDITG_POSITION, mat.p);
   sd->write(IDITG_ROTATION, Point3F(rot.x, rot.y, rot.z));
}   

void
SimInterior::inspectRead(Inspect* sd)
{
   Parent::inspectRead(sd);

   // Filename edit
   char newFilename[Inspect::MAX_STRING_LEN + 1];
   sd->read(IDITG_FILENAME, newFilename);
   loadShape(newFilename);

   Point3F     pos;
   Point3F     rot;
   sd->read(IDITG_POSITION, pos);
   sd->read(IDITG_ROTATION, rot);
   
   set(EulerF(rot.x, rot.y, rot.z), pos);
}   

//------------------------------------------------------------------------------
//-----------------------------------------
// Light animation Processing
//-----------------------------------------

int
SimInterior::getLightId(const char* in_pLightName)
{
   return getInstance()->getLighting()->findLightId(in_pLightName);
}

bool
SimInterior::animateLight(const int   in_lightId,
                          const Int32 in_loopCount)
{
   if (in_lightId < 0)
      return false;

   ITRInstance* interior = getInstance();

   // first make sure we terminate any prior animations in progress...
   //
   deanimateLight(in_lightId);

   interior->setLightTime(in_lightId, 0);
   interior->setLightLoopCount(in_lightId, in_loopCount);

   animatingLights.push_back(in_lightId);
   
   return true;
}

bool
SimInterior::deanimateLight(const int in_lightId)
{
   if (in_lightId < 0)
      return false;

   // All that is required here is that we remove the light from the animation
   //  list.  It is left in the state it is in, unless the user further modifies
   //  its state...
   //
   for (int i = 0; i < animatingLights.size(); i++) {
      if (animatingLights[i] == in_lightId) {
         animatingLights.erase(i);
         break;
      }
   }
   
   return true;
}

bool
SimInterior::resetLight(const int in_lightId)
{
   if (in_lightId < 0)
      return false;

   ITRInstance* interior = getInstance();

   // first make sure we terminate any prior animations in progress...
   //
   deanimateLight(in_lightId);

   interior->setLightTime(in_lightId, 0.0f);
   interior->setLightLoopCount(in_lightId, 0);
   
   return true;
}

//------------------------------------------------------------------------------
//--------------------------------------
// Self rendering
//--------------------------------------

void
SimInterior::RenderImage::render( TSRenderContext &rc )
{
   //
   ITRMetrics.render.reset();
   ITRMetrics.numRenderedInteriors++;
   rc.getCamera()->pushTransform(transform);
   
   // Select the detail level we will be drawing at.  This is a function of the
   //  bounding box for the highest level of detail, and the radius of the minimum
   //  level in the current state.
   //  - If camera is inside object's bounding box, draw highest detail level.
   //  - else check the projected radius of the lowest level of detail...
   //
   Point3F cameraCoord = rc.getCamera()->getCC();
   Box3F   bbox        = instance->getHighestBoundingBox();

   // We only set the detail by the pixels iff we're outside the interior, it's
   //  not a linked interior, and we're not rendering it through a link.  Otherwise,
   //  the highest detail is drawn...
   //
   if (bbox.contains(cameraCoord) == false) {
      float projPixels = rc.getCamera()->
         transformProjectRadius(instance->getLowestCenterPt(),
            instance->getLowestRadius());
		projPixels *= 2.0f * rc.getCamera()->getPixelScale();
      instance->setDetailByPixels(projPixels);
   } else {
      // If we're inside or rendering through a link,
      //  we draw at the highest detail level...
      //
      instance->setDetailLevel(0);
   }

   rend.render(rc, instance);

   // Draw the bounding box if the flag is set...
   //
   if (g_drawSimInteriorBBox == true) {
      TS::PointArray* pArray = rc.getPointArray();
      Point3F bboxPts[8];
      bboxPts[0].set(bbox.fMin.x, bbox.fMin.y, bbox.fMin.z);
      bboxPts[1].set(bbox.fMin.x, bbox.fMax.y, bbox.fMin.z);
      bboxPts[2].set(bbox.fMin.x, bbox.fMax.y, bbox.fMax.z);
      bboxPts[3].set(bbox.fMin.x, bbox.fMin.y, bbox.fMax.z);
      bboxPts[4].set(bbox.fMax.x, bbox.fMin.y, bbox.fMin.z);
      bboxPts[5].set(bbox.fMax.x, bbox.fMax.y, bbox.fMin.z);
      bboxPts[6].set(bbox.fMax.x, bbox.fMax.y, bbox.fMax.z);
      bboxPts[7].set(bbox.fMax.x, bbox.fMin.y, bbox.fMax.z);
      
      int start = pArray->addPoints(8, bboxPts);
      
      pArray->drawLine(start + 0, start + 1, 253);
      pArray->drawLine(start + 1, start + 2, 253);
      pArray->drawLine(start + 2, start + 3, 253);
      pArray->drawLine(start + 3, start + 0, 253);
      pArray->drawLine(start + 4, start + 5, 253);
      pArray->drawLine(start + 5, start + 6, 253);
      pArray->drawLine(start + 6, start + 7, 253);
      pArray->drawLine(start + 7, start + 4, 253);
      pArray->drawLine(start + 0, start + 4, 253);
      pArray->drawLine(start + 1, start + 5, 253);
      pArray->drawLine(start + 2, start + 6, 253);
      pArray->drawLine(start + 3, start + 7, 253);
   }

   rc.getCamera()->popTransform();
}


//------------------------------------------------------------------------------
//-----------------------------------------------------
// Container helper funtions, links, and linkRender'ing
//-----------------------------------------------------

void
SimInterior::updateBoundingBox()
{
   if (renderImage.instance) {
      Box3F box0 = renderImage.instance->getGeometry()->box, box1;
      m_mul(box0, renderImage.transform, &box1);
      setBoundingBox(box1);
   }      
}  
