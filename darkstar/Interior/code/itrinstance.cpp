//----------------------------------------------------------------------------
#include <console.h>
#include <windows.h>
#include "lock.h"
#include <g_types.h>
#include <g_surfac.h>
#include "ts_shapeinst.h"
#include "itrgeometry.h"
#include "itrinstance.h"
#include "itrres.h"
#include "itrcollision.h"
#include <itrbit.h>
#include <m_coll.h>
#include <steppers.h>

#include <m_random.h>



//----------------------------------------------------------------------------

static ResourceTypeITRGeometry         _resitr(".dig");
static ResourceTypeITRLighting         _reslit(".dil");
static ResourceTypeITRShape            _resdis(".dis");

namespace {

Random sg_random;

inline DWORD
buildPackedColor(UInt16 r, UInt16 g,
                 UInt16 b, UInt16 m)
{
   // The colors range in 8:8 fixed pt format from 255:0 to 0:0, add 0x80 for
   //  rounding...
   //
   return (((m + 0x80) >> 8) << 24) |
          (((r + 0x80) >> 8) << 16) |
          (((g + 0x80) >> 8) << 8) |
          (((b + 0x80) >> 8) << 0);
}

inline DWORD
buildPackedColor(float r, float g,
                 float b, float m)
{
   // number are same as above, but in floating point format...
   //
   UInt32 rNew = UInt8((r / 256.0f) + 0.5f);
   UInt32 gNew = UInt8((g / 256.0f) + 0.5f);
   UInt32 bNew = UInt8((b / 256.0f) + 0.5f);
   UInt32 mNew = UInt8((m / 256.0f) + 0.5f);

   return (mNew << 24) |
          (rNew << 16) |
          (gNew << 8) |
          (bNew << 0);
}

//----------------------------------------------------------------------------
// Merge src light map into dst light map.
//
inline UInt16
multiplyColor(DWORD packedColor, UInt8 inten)
{
   UInt8 m = (packedColor >> 24) & 0xff;
   UInt8 r = (packedColor >> 16) & 0xff;
   UInt8 g = (packedColor >> 8)  & 0xff;
   UInt8 b = (packedColor >> 0)  & 0xff;
   UInt16 mOr = ((UInt16(m) * UInt16(inten)) >> 12) << 12;
   UInt16 rOr = ((UInt16(r) * UInt16(inten)) >> 12) << 8;
   UInt16 gOr = ((UInt16(g) * UInt16(inten)) >> 12) << 4;
   UInt16 bOr = ((UInt16(b) * UInt16(inten)) >> 12) << 0;
   
   return (mOr | rOr | gOr | bOr);
}

inline float
distSquared(const Point2F& in_point1,
            const Point2F& in_point2)
{
   return (((in_point1.x - in_point2.x) * (in_point1.x - in_point2.x)) + 
           ((in_point1.y - in_point2.y) * (in_point1.y - in_point2.y)));
}

inline float
distSquared(const Point3F& in_point1,
            const Point3F& in_point2)
{
   return (((in_point1.x - in_point2.x) * (in_point1.x - in_point2.x)) + 
           ((in_point1.y - in_point2.y) * (in_point1.y - in_point2.y)) +
           ((in_point1.z - in_point2.z) * (in_point1.z - in_point2.z)));
}

UInt16
hashSurfaceNumber(const int& in_number)
{
   return UInt16((in_number >> 16) ^
                 in_number & 0xffff);
}

} // namespace { }

static Lock sg_instanceServerLock;
UInt32
ITRInstance::InstanceKeyServer::getNewInstanceKey()
{
   // This is a little paranoid, but shouldn't be a hit at all...
   sg_instanceServerLock.acquire();
   UInt32 retKey = m_currentKey++;
   sg_instanceServerLock.release();

   return retKey;
}
ITRInstance::InstanceKeyServer ITRInstance::sm_instanceKeyServer;


ITRInstance::SurfaceLightCache 
      ITRInstance::m_surfaceLightCache(hashSurfaceNumber, 1200 );

//----------------------------------------------------------------------------

/* do
      - make database static to itrInstance to save space.  
      - use scene methods for computing lighting values in mergeVectorLights()
      - better radius scene lights preparation. 
      - make mono work for vector lights.  
*/

// Dynamic light methods.  

#define  MONO_FORMULA(r,g,b)        (0.3*(r) + 0.6*(g) + 0.1*(b))

#define  _DEBUG_LIGHTS      0
#if _DEBUG_LIGHTS
static void _clear_light_map ( GFXLightMap* io_dst );
static int  _debug_num = -1;     // if > 0, just dlight this surface
static int  _debug_clr = 1;      // if set, clear out. 
static int  _debug_color = 0x5555;
static int  _debug_const = 1;    // causes pure circle cast (?).
#endif

void ITRInstance::startRender ( TSRenderContext& rc )
{
   const TMat3F &    mat = rc.getCamera()->getTOW();
   
   m_renderContext = &rc;
   
      // prepare if sun on interior or if there's a point light.  
   TSSceneLighting * sceneLights = rc.getLights();
      
   ITRGeometry       *pGeometry = getGeometry();
   Box3F &           box = pGeometry->box;
   
   float boxRad = box.len_x();        // get a crude radius
   if ( boxRad < box.len_y() )
      boxRad  = box.len_y();
   if ( boxRad < box.len_z() )
      boxRad  = box.len_z();
   
   SphereF  instSphere ( mat.p, boxRad );
   sceneLights->prepare ( instSphere, mat );

   // set affected surfaces for all point lights.  
   m_surfaceLightCache.clear();
   for ( TSSceneLighting::iterator ptr = sceneLights->begin();
               ptr != sceneLights->end(); ptr++ ) 
   {
      TSLight  *tsl = *ptr;
      
      // FIX - use scene lighting methods (will roll in ambient too...).  
      if ( tsl->fLight.fType == TS::Light::LightPoint )
         findAffectedSurfaces ( tsl );
   }
}

const char*
unmissionLightName(const char* pLightingName)
{
   static char buffer[256];
   strcpy(buffer, pLightingName);
   
   // get to the first '-'
   char * pEnd = strrchr( buffer, '-' );
   if( pEnd == NULL || pEnd == buffer )
      return NULL;
      
   char * pChar = pEnd;
   while( --pChar != buffer && isdigit(*pChar) );
   
   // check if second '-' - then this is a missionlit name
   if( *pChar == '-' )
   {
      strcpy( pEnd, ".dil" );
      return( buffer );
   }
   
   return( pLightingName );
}

//----------------------------------------------------------------------------
ITRInstance::ITRInstance(ResourceManager*          io_pRM,
                         Resource<ITRShape>&       io_rShape,
                         const UInt32              in_initialLightState,
                         bool                      in_missionLit,
                         bool                      in_loadDML )   
 : m_pShape(NULL),
   m_currentState(0xffffffff),
   m_pCurrState(NULL),
   m_currentLOD(0xffffffff),
   m_pCurrLOD(NULL),
   m_currLightState(in_initialLightState),
   m_flags(0)
{
   AssertFatal(in_initialLightState < io_rShape->getNumLightStates(),
               "Light state out of bounds.");

   setMissionLit( in_missionLit );
   m_renderContext = NULL;
   m_instanceKey = sm_instanceKeyServer.getNewInstanceKey();   

   // Shape never changes
   //
   m_shapeResource = io_rShape;
   m_pShape        = m_shapeResource;

   // load the material list we get from the shape...
   //
   const char* pMatFileName = io_rShape->getMaterialListFileName();
   m_materialListResource   = io_pRM->load(pMatFileName);
   AssertFatal(bool(m_materialListResource) == true,
               avar("could not load matList: %s", io_rShape->getMaterialListFileName()));
   m_pMaterialList = m_materialListResource;
   if(in_loadDML)
      m_pMaterialList->load(*io_pRM, true);

   // Load all geometries...
   //
   Int32 maxSurfaces = -1;
   int i;
   m_geometryResources.setSize(m_pShape->m_lodVector.size());
   for (i = 0; i < m_pShape->m_lodVector.size(); i++) {
      ITRShape::LOD& lod = m_pShape->m_lodVector[i];
      const char* pGeometryName = &m_pShape->m_nameBuffer[lod.geometryFileOffset];

      // First construct the resource, then load it
      new(&m_geometryResources[i]) Resource<ITRGeometry>;
      m_geometryResources[i] = io_pRM->load(pGeometryName);
      if (bool(m_geometryResources[i]) == false)
         AssertFatal(0, avar("Error loading geometry: %s", pGeometryName));
         
      if (m_geometryResources[i]->surfaceList.size() > maxSurfaces)
         maxSurfaces = m_geometryResources[i]->surfaceList.size(); 
   }
   AssertFatal(maxSurfaces > 0, "Bogus maxSurfaces");

   // Allocate our surface structures.  note that the size never changes, but
   //  we only deal with those that exist in the current geometry...
   //
   m_surfaceList.setSize(maxSurfaces);
   memset(m_surfaceList.address(), 0,
          m_surfaceList.size() * sizeof(SurfaceList::value_type));

   // Load initial lighting state
   //
   Int32 maxLightMaps = -1;
   m_lightingResources.setSize(m_pShape->m_lodVector.size());
   if( isMissionLit() )
      m_missionLightingResources.setSize(m_pShape->m_lodVector.size());
   for (i = 0; i < m_pShape->m_lodVector.size(); i++) {
      ITRShape::LOD& lod = m_pShape->m_lodVector[i];
      ITRShape::LODLightState& ls =
         m_pShape->m_lodLightStates[lod.lightStateIndex + in_initialLightState];
      const char* pLightingName = &m_pShape->m_nameBuffer[ls.lightFileOffset];

      // check if this is a missionlit
      if( isMissionLit() )
      {
         new(&m_missionLightingResources[i]) Resource<ITRMissionLighting>;
         m_missionLightingResources[i] = io_pRM->load(pLightingName);
         if( bool(m_missionLightingResources[i]) == false ||
             m_missionLightingResources[i]->geometryBuildId != m_geometryResources[i]->buildId )
         {
            // failed to get missionlit lighting - clear all missionlighting..
            Console->printf("Failed to load mission lighting for geometry: %s", io_rShape.getFileName());
            for( int j = 0; j <= i; j++ )
               m_missionLightingResources[j].~Resource<ITRMissionLighting>();
            m_missionLightingResources.clear();
            setMissionLit(false);
         }
      }

      // now get the unmissionlit version
      new(&m_lightingResources[i]) Resource<ITRLighting>;
      const char * pName = unmissionLightName(pLightingName);
      m_lightingResources[i] = io_pRM->load(pName);
      
      if( bool(m_lightingResources[i]) == false || 
         m_lightingResources[i]->geometryBuildId != m_geometryResources[i]->buildId) 
         AssertFatal(0, avar("Error loading lighting: %s, no non-mission lit verison either", pName ));

      if (m_lightingResources[i]->lightMapListSize > maxLightMaps)
         maxLightMaps = m_lightingResources[i]->lightMapListSize;
   }
   AssertFatal(maxLightMaps >= 0, "bogus maxLightMaps.");
   
   // Default to the 0th state. _setInteriorState() also sets the detail level
   //  to 0
   //
   _setInteriorState(0);

   // now in default state, set default pointers
   m_pHighestGeometry = m_pCurrGeometry;
   m_pHighestLighting = m_pCurrLighting;
   m_pHighestMissionLighting = m_pCurrMissionLighting;

   // Set up instance lighting structures.  Number of lights does not change
   //  from detail level to detail level within a state, but the number of
   //  lightmaps does, so we treat the latter like surfaces in the geometry...
   //
   m_lightList.setSize(m_pCurrLighting->lightList.size());
	if (m_lightList.size())
	   memset(m_lightList.address(), -1,
	          m_lightList.size() * sizeof(LightList::value_type));

   m_lightMapList.setSize(maxLightMaps);
	if (m_lightMapList.size())
	   memset(m_lightMapList.address(), 0,
	          m_lightMapList.size() * sizeof(LightMapList::value_type));

   // Lights default to state 0
   for (i = 0; i < m_lightList.size(); i++)
      setLightState(m_pCurrLighting->lightList[i].id, 0);

   // Loopcounts default to 0
   for (i = 0; i < m_lightList.size(); i++)
      setLightLoopCount(m_pCurrLighting->lightList[i].id, 0);
}



ITRInstance::~ITRInstance()
{
   // Do these by hand, since the vector won't...
   //
   int i;
   for (i = 0; i < m_geometryResources.size(); i++) {
      m_geometryResources[i].~Resource<ITRGeometry>();
   }
   for (i = 0; i < m_lightingResources.size(); i++) {
      m_lightingResources[i].~Resource<ITRLighting>();
   }
   for (i = 0; i < m_missionLightingResources.size(); i++) {
      m_missionLightingResources[i].~Resource<ITRMissionLighting>();
   }
}


void
ITRInstance::setMaterialList(Resource<TSMaterialList> io_newMatList)
{
   // It is presumed that the matlist is already loaded() at this point.
   //
   m_materialListResource   = io_newMatList;
   m_pMaterialList = m_materialListResource;

   // Invalidate surfaces...
   //
   for (int i = 0; i < m_surfaceList.size(); i++) {
      Surface& rSurface = m_surfaceList[i];
      rSurface.flags &= ~Surface::TextureValid;
   }
}

void ITRInstance::getAutoStartIDs(Vector<int>& out_rAnimating)
{
   AssertWarn(out_rAnimating.size() == 0,
              "Inserting auto animates into non-empty list...");
   
   ITRLighting* pLighting = getLighting();

   for (int i = 0; i < pLighting->lightList.size(); i++) {
      ITRLighting::Light& light = pLighting->lightList[i];
      
      if ((light.animationFlags & ITRLIGHTANIM_AUTOSTART) != 0)
         out_rAnimating.push_back(light.id);
   }
}

//----------------------------------------------------------------------------
void ITRInstance::incLightState(int lightId)
{
   ITRLighting* pLighting = getLighting();
   AssertFatal(pLighting != NULL, "No lighting associated with interior");

   int lightIndex = pLighting->findLight(lightId);
   if (lightIndex >= 0) {
      Light& light = m_lightList[lightIndex];
      ITRLighting::Light& llight = pLighting->lightList[lightIndex];
      if (llight.stateCount > 1) {
         if (++light.state >= llight.stateCount)
            light.state = 0;

         ITRLighting::State& lState = 
            pLighting->stateList[llight.stateIndex + light.state];
         light.currentPackedColor = buildPackedColor(lState.red,  lState.green,
                                                     lState.blue, lState.mono);
         light.animationTime      = lState.animationTime;
         
         installLightState(light, llight, light.state);
      }
   }
}


void
ITRInstance::setLightState(int lightId,
                           int state)
{
   ITRLighting* pLighting = getLighting();
   AssertFatal(pLighting != NULL, "No lighting associated with interior");

   int lightIndex = pLighting->findLight(lightId);

   if (lightIndex >= 0 && m_lightList[lightIndex].state != state) {
      Light& light               = m_lightList[lightIndex];
      ITRLighting::Light& llight = pLighting->lightList[lightIndex];

      AssertFatal(state >= 0 && state < llight.stateCount,
                  "ITRInstance::setLightState: State state out of bounds");
      if (state < 0)
         state = 0;
      if (state >= llight.stateCount)
         state = llight.stateCount;
         
      ITRLighting::State& lState = 
         pLighting->stateList[llight.stateIndex + state];

      light.state         = state;
      light.animationTime = lState.animationTime;

      light.currentPackedColor = buildPackedColor(lState.red,  lState.green,
                                                  lState.blue, lState.mono);
      installLightState(light, llight, state);
   }
}

void
ITRInstance::installLightState(Light&              instanceLight,
                               ITRLighting::Light& light,
                               int                 newState)
{
   ITRLighting* pLighting = getLighting();

   // Installs the state data into the instance data.
   ITRLighting::State& state = 
      pLighting->stateList[light.stateIndex + newState];
   //
   ITRLighting::StateData* data = &pLighting->stateDataList[state.dataIndex];
   for (int d = 0; d < state.dataCount; d++, data++) {
      if (data->mapIndex >= 0) {
         // Install the light map and invalidate the light cache.
         m_surfaceList[data->surface].setLightMapNotValid();

         m_lightMapList[data->lightIndex].stateMap =
            pLighting->getLightMap(data->mapIndex);
         m_lightMapList[data->lightIndex].packedColor = instanceLight.currentPackedColor;
      }
      else {
         // Only invalidate the ligth cache if the light is not
         // already empty.
         if (m_lightMapList[data->lightIndex].stateMap) {
            m_lightMapList[data->lightIndex].stateMap = 0;
            m_surfaceList[data->surface].setLightMapNotValid();
         }
      }
   }
}


void
ITRInstance::updateSpecialLight(Light&              io_rLight,
                                ITRLighting::Light& io_rLLight)
{
   ITRLighting* pLighting = getLighting();

   if ((io_rLLight.animationFlags & ITRLIGHTANIM_RANDOMFRAME) != 0) {
      // Light is a randomly flickering light.  Flicker unit time is the first
      //  states duration.  State is chosen by random selection, weighted by
      //  overall animation duration.
      //
      ITRLighting::State* pStateBegin = &pLighting->stateList[io_rLLight.stateIndex + 1];
      if (io_rLight.animationTime >= pStateBegin->animationTime) {
         // time to select a new state...
         //
         int i;

         AssertFatal(pStateBegin->animationTime > 0.0f,
                     "Error, zero animation time in flicker light");
         while (io_rLight.animationTime >= pStateBegin->animationTime)
            io_rLight.animationTime -= pStateBegin->animationTime;
            
         float rand = sg_random.getFloat();
         rand *= io_rLLight.animationDuration;

         ITRLighting::State* pState;
         for (i = 0; i < io_rLLight.stateCount - 1; i++) {
            pState = &pLighting->stateList[io_rLLight.stateIndex + i + 1];
            
            if (pState->animationTime > rand) {
               //bingo
               pState--;
               break;
            }
         }
         
         // Already in this state...
         //
         if (io_rLight.state == i)
            return;

         io_rLight.state = i;
         io_rLight.currentPackedColor = buildPackedColor(pState->red,
                                                         pState->green,
                                                         pState->blue,
                                                         pState->mono);
         installLightState(io_rLight, io_rLLight, io_rLight.state);
      } else {
         // nothing to do...
         //
      }
   } else {
      AssertFatal(0, "unknown special animation...");
   }
}

void
ITRInstance::updateLight(Light&              io_rLight,
                         ITRLighting::Light& io_rLLight)
{
   ITRLighting* pLighting = getLighting();

   // On entry, io_rLight holds the new time, we need to update the packed color
   //  and the state.
   //
   float stateTimeBegin, stateTimeEnd;
   float rBegin, rEnd;
   float gBegin, gEnd;
   float bBegin, bEnd;
   float mBegin, mEnd;
   
   AssertFatal((io_rLight.animationTime >= 0.0f &&
                io_rLight.animationTime <= io_rLLight.animationDuration),
               "Out of bounds time on light...");

   if ((io_rLLight.animationFlags & ITRLIGHTANIM_SPECIALANIM) != 0) {
      updateSpecialLight(io_rLight, io_rLLight);
      return;
   }

   // find the current state. Note: this is currently a braindead linear search,
   //  if the time setting becomes a bottleneck, maybe replace with bin search.
   //
   int stateBeginIndex = 0;
   ITRLighting::State* pStateBegin = &pLighting->stateList[io_rLLight.stateIndex];

   while (stateBeginIndex < (io_rLLight.stateCount-1) &&
          io_rLight.animationTime > (pStateBegin + 1)->animationTime) {
      stateBeginIndex++;
      pStateBegin++;
   }
   
   ITRLighting::State* pStateEnd;
   if (stateBeginIndex != (io_rLLight.stateCount - 1)) {
      int stateEndIndex = stateBeginIndex+1;
      pStateEnd         = &pLighting->stateList[io_rLLight.stateIndex +
                                                      stateEndIndex];

      stateTimeBegin = pStateBegin->animationTime;
      stateTimeEnd   = pStateEnd->animationTime;
   } else {
      pStateEnd     = &pLighting->stateList[io_rLLight.stateIndex];
      
      stateTimeBegin = pStateBegin->animationTime;
      stateTimeEnd   = io_rLLight.animationDuration;
   }
   
   // At this point we have:  the state the light is in (stateBeginIndex)
   //                         the beginning and ending times of the state
   //                         the next state.
   //  Now we need to interpolate the color
   //
   rBegin = float(pStateBegin->red);
   gBegin = float(pStateBegin->green);
   bBegin = float(pStateBegin->blue);
   mBegin = float(pStateBegin->mono);
   rEnd   = float(pStateEnd->red);
   gEnd   = float(pStateEnd->green);
   bEnd   = float(pStateEnd->blue);
   mEnd   = float(pStateEnd->mono);
   
   rEnd -= rBegin;
   gEnd -= gBegin;
   bEnd -= bBegin;
   mEnd -= mBegin;
   
   float time     = io_rLight.animationTime - stateTimeBegin;
   float timeDiff = stateTimeEnd - stateTimeBegin;
   
   float multiplier;
   if (timeDiff != 0.0f)
      multiplier = time / timeDiff;
   else
      multiplier = 1.0f;

   rBegin += rEnd * multiplier;
   gBegin += gEnd * multiplier;
   bBegin += bEnd * multiplier;
   mBegin += mEnd * multiplier;

   // Begin colors now hold the new state color
   //
   DWORD newPackedColor = buildPackedColor(rBegin, gBegin, bBegin, mBegin);

   if (io_rLight.currentPackedColor != newPackedColor ||
       io_rLight.state              != stateBeginIndex) {
      io_rLight.state              = stateBeginIndex;
      io_rLight.currentPackedColor = newPackedColor;
      installLightState(io_rLight, io_rLLight, io_rLight.state);
   }
}

void
ITRInstance::setLightTime(int   lightId,
                          float time)
{
   ITRLighting* pLighting = getLighting();
   AssertFatal(pLighting != NULL, "no lighting w/ interior");


   int lightIndex = pLighting->findLight(lightId);
   if (lightIndex < 0) {
      // No light to operate on...
      return;
   }
   
   Light& light               = m_lightList[lightIndex];
   ITRLighting::Light& lLight = pLighting->lightList[lightIndex];

   AssertFatal(time >= 0.0f && time <= lLight.animationDuration,
               "Time out of bounds...");
   if (time < 0.0f)
      time = 0.0f;
   if (time > lLight.animationDuration)
      time = lLight.animationDuration;
   light.animationTime = time;

   updateLight(light, lLight);
}

float
ITRInstance::getLightTime(int lightId)
{
   ITRLighting* pLighting = getLighting();
   AssertFatal(pLighting != NULL, "no lighting w/ interior");

   int lightIndex = pLighting->findLight(lightId);
   if (lightIndex < 0) {
      // No light to operate on...
      return -1.0f;
   }
   
   Light& light = m_lightList[lightIndex];
   return light.animationTime;
}

int
ITRInstance::getLightState(int lightId)
{
   ITRLighting* pLighting = getLighting();
   AssertFatal(pLighting != NULL, "No lighting associated with interior");

   int lightIndex = pLighting->findLight(lightId);

   if (lightIndex >= 0) {
      Light& light = m_lightList[lightIndex];
      return light.state;
   }

   return -1;
}


void
ITRInstance::setLightLoopCount(int   lightId,
                               Int32 loopCount)
{
   int lightIndex = getLighting()->findLight(lightId);
   if (lightIndex < 0) {
      // No light to operate on...
      return;
   }
   
   Light& light              = m_lightList[lightIndex];
   light.animationLoopCount  = loopCount;
}

bool
ITRInstance::stepLightTime(int   lightId,
                           float timeStep)
{
   ITRLighting* pLighting = getLighting();

   int lightIndex = pLighting->findLight(lightId);
   if (lightIndex < 0) {
      // No light to operate on...
      return false;
   }
   
   bool continueAnimation     = true;
   Light& light               = m_lightList[lightIndex];
   ITRLighting::Light& lLight = pLighting->lightList[lightIndex];
   
   light.animationTime += timeStep;
   while (light.animationTime > lLight.animationDuration) {
      // animation has looped on itself.  We need to determine what happens to
      //  the light at the end of its cycle...
      //
      if (light.animationLoopCount == 0) {
         // 0 loop count indicates an infinite animation
         //
         light.animationTime -= lLight.animationDuration;
      } else {
         light.animationLoopCount--;
         if (light.animationLoopCount != 0) {
            // looping not finished...
            //
            light.animationTime -= lLight.animationDuration;
         } else {
            // looping finished.  check the lLight's flags to see if the animation
            //  end by returning to the beginning state, or the final state
            //
            continueAnimation = false;
            if ((lLight.animationFlags & ITRLIGHTANIM_LOOPTOENDFRAME) != 0) {
               ITRLighting::State& lState = 
                  (pLighting->stateList[lLight.stateIndex +
                                            (lLight.stateCount - 1)]);
               light.animationTime = lState.animationTime;
            } else {
               light.animationTime = 0;
            }
         }
      }
   }
   
   updateLight(light, lLight);
   
   return continueAnimation;
}


//------------------------------------------------------------------------------
// NAME 
//    void 
//    ITRInstance::setDetailLevel(const UInt32 in_detailLevel)
//    
// DESCRIPTION 
//    High level interface used by the simInterior class to swap out detail
//   levels.  This function has the following responsibilities:
//    - clear Surface/Dynamic Light cache
//    - _setDetail()
//    - force reset Light times
//    - recollide registered dynamic lights
//
// NOTES 
//    
//------------------------------------------------------------------------------
void ITRInstance::setDetailLevel(const UInt32 in_detailLevel)
{
   if (in_detailLevel == m_currentLOD)
      return;

   int i;

   // Clear affected surfaces.  
   
   // Set the new geometry and lighting pointers...
   //
   _setDetailLevel(in_detailLevel);

   AssertFatal( m_lightList.size() <= getLighting()->lightList.size(),
      "Number of lights does not match between detail levels." );
   
   // Force a reset of all pertinent light states...
   //
   for (i = 0; i < m_lightList.size(); i++) {
      Light&              iLight = m_lightList[i];
      ITRLighting::Light& lLight = getLighting()->lightList[i];

      // forces state to reset...
      iLight.state = -1;
      updateLight(iLight, lLight);
   }
}

void ITRInstance::setDetailByPixels(const float in_projPixels)
{
   // make sure first that this is a detailed interior, and not just a regular
   //  one...
   //
   if (bool(m_shapeResource) == false) {
      // just the one level of detail and state...
      //
      return;
   }

   UInt32 close  = UInt32(in_projPixels);
   UInt32 newLOD = m_pShape->findDetail(m_pCurrState, close);

   if (newLOD != m_currentLOD) {
      setDetailLevel(newLOD);
   }
}


void ITRInstance::_setDetailLevel(const UInt32 in_detailLevel)
{
   // This function just muscles in the new geometry and lighting.  Any higher
   //  level management must occur in setDetailLevel(...)
   //
   if (in_detailLevel == m_currentLOD)
      return;
   
   UInt32 detailOffset = m_pCurrState->lodIndex + in_detailLevel;

   m_currentLOD = in_detailLevel;
   m_pCurrLOD   = &m_pShape->m_lodVector[detailOffset];

   m_pCurrGeometry = m_geometryResources[detailOffset];
   m_pCurrLighting = m_lightingResources[detailOffset];
   m_pCurrMissionLighting = isMissionLit() ? (ITRMissionLighting*)m_missionLightingResources[detailOffset] : NULL;
   AssertFatal(m_pCurrGeometry->buildId == m_pCurrLighting->geometryBuildId,
               "ITRInstance:: Light data does not match geometry (should not assert here anymore)");

   // Clear all surface flags.  Note that resetting light states, and
   //  re-colliding any dynamic lights must also be handled at a higher level...
   //
   memset(m_surfaceList.address(), 0,
          m_pCurrGeometry->surfaceList.size() * sizeof(SurfaceList::value_type));
}

void ITRInstance::_setInteriorState(const UInt32 in_state)
{
   AssertFatal(in_state < m_pShape->getNumStates(), "state out of bounds...");

   if (in_state == m_currentState) {
      return;
   }
   
   m_currentState = in_state;
   m_pCurrState   = &m_pShape->m_stateVector[m_currentState];

   ITRGeometry* pHighestGeometry = m_geometryResources[m_pCurrState->lodIndex];
   ITRGeometry* pLowestGeometry  = m_geometryResources[m_pCurrState->lodIndex +
                                                       m_pCurrState->numLODs - 1];

   m_highestBoundingBox = pHighestGeometry->box;
   
   // Compute the projection sphere for the lowest detail level...
   //
   Box3F m_lowestBound = pLowestGeometry->box;
   m_lowestRadius = (m_lowestBound.len_x() > m_lowestBound.len_y()) ? m_lowestBound.len_x() :
                                                                      m_lowestBound.len_y();
   m_lowestRadius = (m_lowestRadius > m_lowestBound.len_z()) ? m_lowestRadius :
                                                               m_lowestBound.len_z();
   m_centerPoint  = m_lowestBound.fMin;
   m_centerPoint += m_lowestBound.fMax;
   m_centerPoint *= 0.5f;

   // Force to detail level 0...
   //
   m_currentLOD = 1;
   _setDetailLevel(0);
}

bool ITRInstance::setInteriorState(const UInt32 in_state)
{
   if (in_state >= UInt32(m_pShape->m_stateVector.size())) {
      AssertWarn(0, "out of bounds state");
      return false;
   }
   
   // Ah well, go ahead...
   _setInteriorState(in_state);
   return true;
}


bool
ITRInstance::setInteriorLightState(ResourceManager* io_pRM,
                                   const UInt32     in_lightState)
{
   if (in_lightState >= UInt32(m_pShape->m_numLightStates)) {
      AssertWarn(0, "out of bounds light state");
      return false;
   }
   
   if (m_currLightState == in_lightState)
      return true;
   m_currLightState = in_lightState;
   
   int i;
   Int32 maxLightMaps = -1;
   for (i = 0; i < m_pShape->m_lodVector.size(); i++) {
      // Release the old
      m_lightingResources[i].unlock();

      // And in with the new
      ITRShape::LOD& lod = m_pShape->m_lodVector[i];
      ITRShape::LODLightState& ls =
         m_pShape->m_lodLightStates[lod.lightStateIndex + in_lightState];
      const char* pLightingName = &m_pShape->m_nameBuffer[ls.lightFileOffset];

      m_lightingResources[i] = io_pRM->load(pLightingName);
      if (bool(m_lightingResources[i]) == false)
         AssertFatal(0, avar("Error loading lighting: %s", pLightingName));
         
      if (m_lightingResources[i]->lightMapListSize > maxLightMaps)
         maxLightMaps = m_lightingResources[i]->lightMapListSize;
   }
   AssertFatal(maxLightMaps >= 0, "bogus maxLightMaps.");
   
   // Force to detail level 0...
   //
   m_currentLOD = 1;
   _setDetailLevel(0);
   
   // Set up instance lighting structures.  Number of lights does not change
   //  from detail level to detail level within a state, but the number of
   //  lightmaps does, so we treat the latter like surfaces in the geometry...
   //
   m_lightList.setSize(m_pCurrLighting->lightList.size());
   memset(m_lightList.address(), -1,
          m_lightList.size() * sizeof(LightList::value_type));

   m_lightMapList.setSize(maxLightMaps);
   memset(m_lightMapList.address(), 0,
          m_lightMapList.size() * sizeof(LightMapList::value_type));

   // Lights default to state 0
   for (i = 0; i < m_lightList.size(); i++)
      setLightState(m_pCurrLighting->lightList[i].id, 0);

   // Loopcounts default to 0
   for (i = 0; i < m_lightList.size(); i++)
      setLightLoopCount(m_pCurrLighting->lightList[i].id, 0);
      
   return true;
}

//----------------------------------------------------------------------------
//
// Handle building / merging of all light maps for a given interior surface.
// 
// Responsible for validating the light map.  
//
//----------------------------------------------------------------------------
GFXLightMap*
ITRInstance::buildLightMap(GFXSurface* gfxSurface,  int surface, float texScale ) 
{
   ITRLighting::Surface& lsurface = m_pCurrLighting->surfaceList[surface];
   int mapSize = lsurface.mapSize.x * lsurface.mapSize.y;

   GFXLightMap* lightMap = gfxSurface->allocateLightMap(mapSize);
   lightMap->size.x      = lsurface.mapSize.x;
   lightMap->size.y      = lsurface.mapSize.y;
   lightMap->offset.x    = lsurface.mapOffset.x;
   lightMap->offset.y    = lsurface.mapOffset.y;

   // Start with static light map if there is one.
   if (lsurface.mapIndex >= 0) 
   {
      // uncompressed
      if( (lsurface.mapIndex & 0x40000000) == 0)
      {
         ITRLighting::LightMap * smap = isMissionLit() ? 
            m_pCurrMissionLighting->getLightMap(lsurface.mapIndex) : NULL;

         // check if this was missionlit or not            
         if( smap == NULL)
            smap = m_pCurrLighting->getLightMap(lsurface.mapIndex);

         memcpy( &lightMap->data, &smap->data, lightMap->size.x * lightMap->size.y * sizeof(UInt16));
      } 
      else // compressed
      {
         int realIndex = lsurface.mapIndex & (~0x40000000);
         
         if( isMissionLit() )
         {
            ITRLighting::LightMap * smap = m_pCurrMissionLighting->getLightMap(realIndex);
            if( smap )
            {
               m_pCurrMissionLighting->decompressLightMap16((UInt8*)&smap->data,
                  lightMap->size.x, lightMap->size.y, &lightMap->data);
            }
            else
            {
               smap = m_pCurrLighting->getLightMap(realIndex);
               m_pCurrLighting->decompressLightMap16((UInt8*)&smap->data,
                  lightMap->size.x, lightMap->size.y, &lightMap->data);
            }
         }
         else
         {
            ITRLighting::LightMap* smap = m_pCurrLighting->getLightMap(realIndex);
            // compressed lightmap...
            m_pCurrLighting->decompressLightMap16((UInt8*)&smap->data,
               lightMap->size.x, lightMap->size.y, &lightMap->data);
         }
      }
   }
   else {
      UInt16 color = lsurface.mapColor & 0xffff;
      UInt16* ptr = &lightMap->data;
      UInt16* end = ptr + lightMap->size.x * lightMap->size.y;
      while (ptr != end)
         *ptr++ = color;
   }

   // Merge light states with static map
   if (lsurface.lightCount) {
      StateMapInfo* sMap = &m_lightMapList[lsurface.lightIndex];
      for (int i = 0; i < lsurface.lightCount; i++, sMap++) {
         // only merge states with a lightmap, and those that aren't
         //  black...
         //
         if (sMap->stateMap && sMap->packedColor != 0) {
            merge(lightMap, sMap->stateMap, sMap->packedColor);
         }
      }
   }

#if _DEBUG_LIGHTS
   if ( _debug_clr )
      _clear_light_map ( lightMap );
#endif

   // Dynamic Lights
   //
   Surface& iSurface = m_surfaceList[surface];
   if (iSurface.isDynamicLit() == true) 
   {
      SurfaceLightCache::iterator itr = m_surfaceLightCache.find(surface);
      
      // merge all dynamic lights that affect this surface.  surfaces are 
      //    next to each other in the list, so this loop works:
      while (itr.isDone() == false) 
      {
         if ((*itr)->isStaticLight() == false)
            mergeDynamicLight(lightMap, surface, *itr, texScale );
         itr++;
      }
      // force next frame update of this texture regardless by leaving the 
      //    light map invalidated.  
      iSurface.setDynamicClear();
   }
   else
      iSurface.setLightMapValid();


   ITRGeometry* pGeometry         = getGeometry();
   ITRGeometry::Surface& gSurface = pGeometry->surfaceList[surface];
   if ( !isLinked () && (gSurface.visibleToOutside != 0) && !isMissionLit() )
      mergeVectorLights ( lightMap, surface );

   return lightMap;
}


void ITRInstance::merge(GFXLightMap*           io_dst,
                   ITRLighting::LightMap* io_src,
                   DWORD                  io_packedColor)
{
   // NOTE: We cheated, and stored the UInt8 intensity data in the UInt16 data
   //  member of the lightmap...
   //
   UInt8*  sptr = (UInt8*)&io_src->data;
   UInt16* dptr = &io_dst->data;
   UInt16* end  = dptr + io_dst->size.x * io_dst->size.y;
   for (; dptr != end; dptr++, sptr++) {
      UInt8  inten    = *sptr;
      
      if (inten < 16)
         continue;

      UInt16 srcColor = multiplyColor(io_packedColor, inten);

      register unsigned int m, i;
      
      i = (*dptr & ~0x0fff) + (srcColor & ~0x0fff);
      m = (i > 0xf000)? 0xf000: i;

      i = (*dptr & ~0xf0ff) + (srcColor & ~0xf0ff);
      m |= (i > 0x0f00)? 0x0f00: i;

      i = (*dptr & ~0xff0f) + (srcColor & ~0xff0f);
      m |= (i > 0x00f0)? 0x00f0: i;

      i = (*dptr & ~0xfff0) + (srcColor & ~0xfff0);
      m |= (i > 0x000f)? 0x000f: i;

      *dptr = m;
   }
}



#if _DEBUG_LIGHTS
static void _clear_light_map ( GFXLightMap* io_dst )
{
   UInt16* dptr = &io_dst->data;
   int      cnt = io_dst->size.x * io_dst->size.y;
   while ( cnt-- )
      *dptr++ = _debug_color;
}
#endif


// Looking for optimization possibilities, like early outs and such.  
#define  _GATHER_DYNAMIC_STATS   0


// 
// The data merging loop for mergeDynamicLight().  This loop was made separate in order 
// to: profile separately;  to make local variable management better in the compiled 
//    code (also- reading large offsets within the stack frame means extended assembly 
//       instructions   i.e. mov eax,[bp-0x20] is smaller than mov eax,[bp-0x90].
//       (Which is what happened in this particular case)).  
// 
int ITRInstance::mergeLoop ( 
         TSInten16Xlat::RemapTable &            cMapper, 
         const Point2F &                        texCoords, 
         const Point2F &                        lMapStart, 
         float                                  lightScale, 
         float                                  radiusSquared,
         float                                  distFromPlaneSquared,
         Point2I                                mapSize, 
         register UInt16 *                      pData 
   )
{
   int   total_hits = 0;
   DistSquaredHelper<float>   xSquaredHelper;
   DistSquaredHelper<float>   xSquaredStarter ( texCoords.x - lMapStart.x, lightScale );
   DistSquaredHelper<float>   ySquaredHelper  ( texCoords.y - lMapStart.y, lightScale );
   
   float    distSquaredCheck = radiusSquared - distFromPlaneSquared, xDistSquared;
   float    yDistSquared = float( ySquaredHelper );
   
   while ( --mapSize.y >= 0 )
   {
      xDistSquared = float( xSquaredHelper = xSquaredStarter );

      for(Int16 x = mapSize.x; x > 0; x --)
      {
         if ( (xDistSquared += yDistSquared) < distSquaredCheck )
         {
#           if _GATHER_DYNAMIC_STATS
               total_hits++;
#           endif

            UInt16   mappedColor = cMapper [ xDistSquared + distFromPlaneSquared ];
            *pData = TSInten16::add ( *pData, mappedColor );
         }
         xDistSquared = xSquaredHelper.next();
         pData++;
      }

      // We could check for zero hits, and an INCREASING value here to do an 
      //    early out.  Wait - this isn't monotonic....  doh!  
      yDistSquared = ySquaredHelper.next();
   }
   
   return total_hits;
}



int ITRInstance::mergeDynamicLight(GFXLightMap*     io_dst,
                               int              surfaceIndex,
                               TSLight* io_pLight, 
                               float texScale )
{
   ITRGeometry* pGeometry         = getGeometry();
   ITRLighting* pLighting         = getLighting();
   ITRGeometry::Surface& gSurface = pGeometry->surfaceList[surfaceIndex];
   ITRLighting::Surface& lSurface = pLighting->surfaceList[surfaceIndex];
   int         mipLevel = pGeometry->highestMipLevel, hits;
   
   // UInt16   scaleShift = gSurface.textureScaleShift;
   // float    texScale = scale_mapper [ scaleShift ];
   float    in_radius = io_pLight->fLight.fRange;
   float    radiusSquared = in_radius * in_radius;

   // Project point onto plane
   //
   Point3F planePoint = io_pLight->fLight.fPosition;
   TPlaneF& plane = pGeometry->planeList[gSurface.planeIndex];
   Point3F  temp  = plane;

   float dist = plane.distance(planePoint);
   temp       *= dist;
   planePoint -= temp;

   dist *= dist;        // we need this as dist^2

#if 0
   // old box map adjust...
   temp = plane;
   float scale = fabs(temp.x) > fabs(temp.y) ? fabs(temp.x) : fabs(temp.y);
   scale       = scale > fabs(temp.z) ? scale : fabs(temp.z);
   scale       = 1.0f / scale;
   temp *= scale;
   scale       = distSquared(temp, Point3F(0, 0, 0));
   scale    *= (texScale * texScale);
#else
   float scale = (texScale * texScale);
   scale *= float ( 1 << (mipLevel * 2));
#endif   
   
   // At this point, we have projected point on the plane and scale for texture
   //  distance to coordinate distance.  We need to find the texture coordinates
   //  of the planePoint, then we can compute the distance from the light
   //  to each point on the surface's lightmap.
   //
   Point2F texCoords = pGeometry->getTextureCoord(surfaceIndex, planePoint);
   AssertFatal(texCoords.x >= 0 || texCoords.x <= 0, "foo");
   AssertFatal(texCoords.y >= 0 || texCoords.y <= 0, "foo");
   
   int   lightShift   = pLighting->lightScaleShift - mipLevel;
   float lightScale   = float(1 << lightShift);


   Point2F lMapStart;
   lMapStart.x = float(-lSurface.mapOffset.x);
   lMapStart.y = float(-lSurface.mapOffset.y);


   // Convert all the math into texture space size.  
   radiusSquared /= scale;
   UInt16   maxCol = TSInten16::getPackedColor ( io_pLight->fLight.fRed, 
               io_pLight->fLight.fGreen, io_pLight->fLight.fBlue );
   TSInten16Xlat::RemapTable  &cMapper = 
         TSInten16Xlat::getMapper( maxCol, radiusSquared );
   float distFromPlaneSquared = dist / scale;

   UInt16* pData = &io_dst->data;
   
#if   0
   hits = 0;
   for (int y = 0; y < lSurface.mapSize.y; y++) {
      Point2F currCoord = lMapStart;
      currCoord.y += y << lightShift;
      float partialDistSquared = distFromPlaneSquared + 
                     (currCoord.y-texCoords.y) * (currCoord.y-texCoords.y);
      for (int x = 0; x < lSurface.mapSize.x; x++) {
         float dSquared = partialDistSquared + 
                     (currCoord.x-texCoords.x) * (currCoord.x-texCoords.x);
         
         if ( dSquared < radiusSquared )
         {
            UInt16   mappedColor = cMapper [ dSquared ];

#           if _GATHER_DYNAMIC_STATS
               hits++;
#           endif

            *pData = TSInten16::add ( *pData, mappedColor );
         }
         pData++;
         currCoord.x += lightScale;
      }
   }
#else
   Point2I  mapSize ( lSurface.mapSize.x, lSurface.mapSize.y );

   hits = mergeLoop( cMapper, texCoords, lMapStart, lightScale, 
               radiusSquared, distFromPlaneSquared, mapSize, pData );
   
#endif

   
#  if _GATHER_DYNAMIC_STATS
   {
      // OPT ideas:  check for increase in dist-squared and early out of loops.  Can 
      //    this possibly do any better than 50%?  Or rather - can it even approach 
      //    that?  Is it worth the checks?  
      // Maybe the incremental square stepper can be used?  Actually - since we are 
      //    stepping in LIXEL space, it can basically use integers for the basic test,
      //    and then do a last float test if that threshold is crossed.  
      //
      // Some results of stats:  Our collision checker seems good - with big lights near
      //    the ground we have 40% hits.  For outside gunfire - about 20% average.  
      //    Number of lixels 80-140, so a tighter loop is probably good.  
      //    The 20% indicates an early out of the (outer) loop might be worth it.  
      struct statistics {
            int   lixels;
            int   total_calls;
            int   total_hits;
            int   total_lixels;
            int   num_times_low_pct;
            float average_lixels;
            float this_pct;
            float total_pct;
         };
      
      static statistics s;
      
      s.lixels = lSurface.mapSize.y * lSurface.mapSize.x;
      s.total_calls++;
      s.total_hits += hits;
      s.total_lixels += s.lixels;
      s.average_lixels = float(s.total_lixels) / float(s.total_calls);
      s.this_pct = (float(hits) / float(s.lixels)) * 100.0f;
      s.total_pct = (float(s.total_hits) / float(s.total_lixels)) * 100.0f;
      
      if( s.this_pct < 0.10f )
         s.num_times_low_pct++;
   }
#  endif

   return hits;
}


#define  MAX_RGB_VALUE        (0.999999999)
#define  MAP_RGB_TO_4(val)    UInt16( (val) * 16.0 )

void ITRInstance::mergeVectorLights ( GFXLightMap * io_dst, int surfaceIndex ) 
{
   ITRGeometry* pGeometry         = getGeometry();
   ITRLighting* pLighting         = getLighting();
   ITRGeometry::Surface& gSurface = pGeometry->surfaceList[surfaceIndex];
   ITRLighting::Surface& lSurface = pLighting->surfaceList[surfaceIndex];
   float       r = 0.0, g = 0.0, b = 0.0;    // m = 0.0;
   bool        found = false;

   TPlaneF    plane = pGeometry->planeList[gSurface.planeIndex];
   if( gSurface.planeFront == false )
      plane.neg();
   
   TSSceneLighting * sceneLights = m_renderContext->getLights();

   // add in the ambient intensity
   ColorF ambient = sceneLights->getAmbientIntensity();
   r += ambient.red;
   g += ambient.green;
   b += ambient.blue;
   
   // Find the R, G, B, total lighting on this face. 
   for ( TSSceneLighting::iterator ptr = sceneLights->begin();
               ptr != sceneLights->end(); ptr++ ) 
   {
      TSLight  *tsl = *ptr;
      
      // FIX - use scene lighting methods (will roll in ambient too...).  
      if ( (tsl->fLight.fType == TS::Light::LightDirectional || 
            tsl->fLight.fType == TS::Light::LightDirectionalWrap) )
      {
         float    dot_product = - m_dot ( tsl->fLight.fDirection, plane );
         
         // FIX:  use scene lighting methods.  
         
         if ( tsl->fLight.fType == TS::Light::LightDirectionalWrap )
            dot_product = (dot_product + 1.0) / 2.0;
         
         if ( dot_product > 0.0 )
         {
            found = true;
            r += tsl->fLight.fRed * dot_product;
            g += tsl->fLight.fGreen * dot_product;
            b += tsl->fLight.fBlue * dot_product;
         }
      }
   }
   
   if ( found )
   {
      float m = MONO_FORMULA ( r, g, b );
      
      if ( m > MAX_RGB_VALUE )
         m = float(MAX_RGB_VALUE);
      if ( r > MAX_RGB_VALUE )
         r = float(MAX_RGB_VALUE);
      if ( g > MAX_RGB_VALUE )
         g = float(MAX_RGB_VALUE);
      if ( b > MAX_RGB_VALUE )
         b = float(MAX_RGB_VALUE);
         
      UInt16      m4 = MAP_RGB_TO_4 ( m ) << 12;
      UInt16      r4 = MAP_RGB_TO_4 ( r ) << 8;
      UInt16      g4 = MAP_RGB_TO_4 ( g ) << 4;
      UInt16      b4 = MAP_RGB_TO_4 ( b ) << 0;

      if ( r4 || g4 || b4 || m4 )
      {
         UInt16      *pData = &io_dst->data;
         int         count = lSurface.mapSize.y * lSurface.mapSize.x;

         while ( count-- )
         {
            register unsigned int   m, i;
            
            i = (*pData & ~0x0fff) + m4;
            m = (i > 0xf000)? 0xf000: i;
            
            i = (*pData & ~0xf0ff) + r4;
            m |= (i > 0x0f00)? 0x0f00: i;

            i = (*pData & ~0xff0f) + g4;
            m |= (i > 0x00f0)? 0x00f0: i;

            i = (*pData & ~0xfff0) + b4;
            m |= (i > 0x000f)? 0x000f: i;

            *pData = m;
            pData++;
         }
      }
   }
}


//----------------------------------------------------------------------------
Int32
ITRInstance::findLeaf(const Point3F& p, Int32 nodeIndex)
{
   if (nodeIndex < 0)
      return -(nodeIndex+1);

   ITRGeometry::BSPNode& node = m_pHighestGeometry->nodeList[nodeIndex];
   if (m_pHighestGeometry->planeList[node.planeIndex].whichSide(p) == TPlaneF::Inside)
      return findLeaf(p, node.front);
   return findLeaf(p, node.back);
}

//----------------------------------------------------------------------------
Int32
ITRInstance::findLeafGeom(ITRGeometry* pGeom, const Point3F& p, Int32 nodeIndex)
{
   if (nodeIndex < 0)
      return -(nodeIndex+1);

   ITRGeometry::BSPNode& node = pGeom->nodeList[nodeIndex];
   if (pGeom->planeList[node.planeIndex].whichSide(p) == TPlaneF::Inside)
      return findLeafGeom(pGeom, p, node.front);
   return findLeafGeom(pGeom, p, node.back);
}

//----------------------------------------------------------------------------
// Collide a Box with the geometry, put all polys pl
// Adapted from ITRCollision -- but changed much since then.
// Box is in source space, toITR maps it to itr space.
// transBack goes from interior to some other space
// (not nec. original space) -- polys will be in this space
// Note: The following statics laying around make this routine not thread safe.
//----------------------------------------------------------------------------
static TMat3F invToItr;
static BitVector visibleNodeList;
static BitVector surfaceList;
static int emptyStartIndex;
static Point3F v[32];
static int maxPolys;

void ITRInstance::getPolys(const Box3F& box, const TMat3F & toITR,
                      const TMat3F & transBack, TS::PolyList & pl, int maxP)
{
   maxPolys = maxP;
   
   if (pl.size()>=maxPolys)
      return;

   // Box is assumed to be axis aligned in the source space.
   // Transform into geometry space
   toITR.getRow(0,&vec[0]);
   vec[0] *=  box.len_x() * 0.5f;
   toITR.getRow(1,&vec[1]);
   vec[1] *= box.len_y() * 0.5f;
   toITR.getRow(2,&vec[2]);
   vec[2] *= box.len_z() * 0.5f;

   Point3F tmp;
   tmp.x = (box.fMin.x + box.fMax.x) * 0.5f;
   tmp.y = (box.fMin.y + box.fMax.y) * 0.5f;
   tmp.z = (box.fMin.z + box.fMax.z) * 0.5f;
   m_mul(tmp,toITR,&center);
   
   invToItr = toITR;
   invToItr.inverse();

   ITRGeometry* pGeometry = getGeometry();

   planeStack.clear();
   surfaceList.setSize(pGeometry->surfaceList.size());
   surfaceList.zero();
   visibleNodeList.setSize(pGeometry->emptyLeafList.size() - ITRGeometry::ReservedOutsideLeafs);
   visibleNodeList.zero();
   emptyStartIndex = pGeometry->solidLeafList.size() + ITRGeometry::ReservedOutsideLeafs;

   // find out what leaf we are in
   Int32 leafIndex = pGeometry->externalLeaf(toITR.p);
   if (leafIndex==0)
      leafIndex = findLeafGeom(pGeometry, toITR.p);
   ITRGeometry::BSPLeafWrap leafWrap(pGeometry, leafIndex);
   if (leafWrap.isSolid())
      // hey, we're in a wall...
      return;
      
   // build list of visible leafs
   UInt8 * pbegin = &pGeometry->bitList[leafWrap.getPVSIndex()];
   UInt8 * pend   = pbegin + leafWrap.getPVSCount();
   for (ITRCompressedBitVector::iterator itr(pbegin,pend); ++itr;)
   {
      int idx = *itr;
      if (idx < ITRGeometry::ReservedOutsideLeafs)
         // don't care about outside leaf visibility
         continue;
      ITRGeometry::BSPLeafWrap lw(pGeometry,idx);
      // we only care about empty leafs
      if (!lw.isSolid())
         visibleNodeList.set(idx-emptyStartIndex);
   }

   radius = m_distf(box.fMin,box.fMax) * 0.5f;
   getPolys(0,box,transBack,pl);
}

void ITRInstance::getPolys(int nodeIndex, const Box3F & box, const TMat3F & transBack,
                      TS::PolyList &pl)
{
   if (pl.size()>=maxPolys)
      return;

   ITRGeometry* pGeometry = getGeometry();

   ITRGeometry::BSPNode& node = pGeometry->nodeList[nodeIndex];
   TPlaneF& plane = pGeometry->planeList[node.planeIndex];

   float distance = m_dot(plane,center) + plane.d;
   float absDist = fabs(distance);
   bool hitPlane = false;

   if ( absDist < radius) 
   {
      float f1 = fabs( m_dot(plane,vec[0]) );
      float f2 = fabs( m_dot(plane,vec[1]) );
      float f3 = fabs( m_dot(plane,vec[2]) );
      if (f1+f2+f3 > absDist )
      {
         // Box intersects the plane
         planeStack.increment();
         PlaneEntry& pe = planeStack.last();
         pe.planeIndex = node.planeIndex;
         pe.distance = distance;
         hitPlane = true;
      }
   }

   if (hitPlane || distance >= .0f) 
   {
      if (node.front >= 0)
         getPolys(node.front,box,transBack,pl);
      else
         getPolysLeaf(node.front,box,transBack,pl);
   }
   if (hitPlane || distance < .0f) 
   {
      if (node.back >= 0)
         getPolys(node.back,box,transBack,pl);
      else
         getPolysLeaf(node.back,box,transBack,pl);
   }
   if (hitPlane)
      planeStack.decrement();
}

//----------------------------------------------------------------------------
// Test all the collision planes on the stack with the planes
// bounding this node.  Also, only collect polys facing light (vec[1])
//
void ITRInstance::getPolysLeaf(int nodeIndex, const Box3F & box, const TMat3F & transBack,
                               TS::PolyList &pl)
{
   if (pl.size()>=maxPolys)
      return;
      
   int ni = -(nodeIndex+1);
   
   // visible from where we stand...
   if (!visibleNodeList.test(ni-emptyStartIndex))
      return;

   ITRGeometry* pGeometry = getGeometry();

   ITRGeometry::BSPLeafWrap leafWrap(pGeometry, ni);
   if (!leafWrap.isSolid()) 
   {
      UInt8* pbegin = &pGeometry->bitList[leafWrap.getSurfaceIndex()];
      UInt8* pend = pbegin + leafWrap.getSurfaceCount();

      // See if any intersected planes bound this leaf.
      for (int i = 0; i < planeStack.size(); i++) 
      {
         int planeIndex = planeStack[i].planeIndex;

         // Unlike when looking for collisions, we find all intersections
         // of surfaces and planes.
         for (ITRCompressedBitVector::iterator itr(pbegin,pend); ++itr; ) 
         {
            int sfcIndex = *itr;
            ITRGeometry::Surface& surface = pGeometry->surfaceList[sfcIndex];
            if (surface.planeIndex == planeIndex && 
                !surfaceList.test(sfcIndex) &&
                surface.material!=ITRGeometry::Surface::NullMaterial &&
            	 surface.type!=ITRGeometry::Surface::Link)
            {
               // so that we don't get poly's from same surface more than once
               surfaceList.set(sfcIndex);
               
               // make sure poly is facing light first -- we assume vertexCount > 2
               AssertFatal(surface.vertexCount>2,"ITRInstance::getPolys: vertexCount < 3");
               Point3F v1,v2,v3,n;
               UInt32 vIdx = 0;
               v[vIdx++] = pGeometry->point3List[pGeometry->vertexList[vIdx+surface.vertexIndex].pointIndex];
               v[vIdx++] = pGeometry->point3List[pGeometry->vertexList[vIdx+surface.vertexIndex].pointIndex];
               v[vIdx++] = pGeometry->point3List[pGeometry->vertexList[vIdx+surface.vertexIndex].pointIndex];
               m_normal(v[0],v[1],v[2],n);
               // facing light light?
               if (m_dot(n,vec[1])>0)
                  continue;

               // form a sphere around poly to see if it's even close to box
               Point3F center;

               // first deal with points already touched
               center.x = v[0].x + v[1].x + v[2].x;
               center.y = v[0].y + v[1].y + v[2].y;
               center.z = v[0].z + v[1].z + v[2].z;

               for (; vIdx<(UInt32)surface.vertexCount; vIdx++)
               {
                  v[vIdx] = pGeometry->point3List[pGeometry->vertexList[vIdx+surface.vertexIndex].pointIndex];
                  center += v[vIdx];
               }
               center *= 1.0f/(float)vIdx;

               float radSq=0.0f;
               for (int i=0;(UInt32)i<vIdx;i++)
               {
                  Point3F pnt = v[i] - center;
                  float pntSq = m_dot(pnt,pnt);
                  if (pntSq>radSq)
                     radSq=pntSq;
               }

               // project center
               Point3F projCenter;
               m_mul(center,invToItr,&projCenter);
               float tmp;
               if (!m_sphereAABox(projCenter,radSq,box.fMin,box.fMax,tmp))
                  continue;

               pl.increment();
               pl.last() = new TS::Poly;
               TS::Poly & ts3Poly = *pl.last();
               ts3Poly.vertexList.setSize(vIdx);
               for (int j=0;(UInt32)j<vIdx;j++)
                  m_mul(v[j],transBack,&ts3Poly.vertexList[j].point);

               // only create so many polys
               if (pl.size()>=maxPolys)
                  return;
            }
         }
      }
   }
}

//-------------------------------------------------------------------------------------
// NAME
// 	bool 
//    ITRCollision::getSurfaceInfo(CollisionSurface & info, Int16 * lighting)
//
// NOTES
// 	Places material and surface index into "info".  If "lighting" is not null,
// 	then also returns 4 x 4 bit lighting value in "lighting".  Assumes that 
// 	info.part holds leaf index, info.surface holds plane index, and info.position
// 	holds point on surface.  ITRCollision line intersect does this for you.
// 	Function returns true if surface found, false o.w.
//-------------------------------------------------------------------------------------
bool ITRInstance::getSurfaceInfo(CollisionSurface & info, UInt16 * lighting)
{
   ITRGeometry * pGeometry = m_pHighestGeometry;
   ITRLighting * pLighting = m_pHighestLighting;

   ITRGeometry::BSPLeafWrap leafWrap(pGeometry, info.part);
	if (leafWrap.isSolid() == false)
		return false;

	Point3F & point = info.position;

	bool foundIt = false;
	UInt8* pbegin = &pGeometry->bitList[leafWrap.getSurfaceIndex()];
	UInt8* pend = pbegin + leafWrap.getSurfaceCount();

	// get plane from info structure
	int & planeIndex = info.surface;
	TPlaneF & plane = pGeometry->planeList[planeIndex];

	// Test against all the surfaces in the leaf
	ITRCompressedBitVector::iterator itr(pbegin,pend);
	ITRGeometry::Surface * gsurface;
	for (; ++itr; )
	{
		gsurface = &pGeometry->surfaceList[*itr];
		if (gsurface->planeIndex == planeIndex)
		{
			// test against surface poly
			Vector<Point3F> poly;
			poly.setSize(gsurface->vertexCount);
			int fv = gsurface->vertexIndex; // first vertex index
			for (int i=0; i<gsurface->vertexCount; i++)
				poly[i] = pGeometry->point3List[pGeometry->vertexList[fv+i].pointIndex];
			if (m_pointInPoly(point,plane,poly.address(),poly.size()))
			{
				foundIt = true;
				break; // this is the surface
			}
		}
	}
	if (!foundIt)
		return false;

	info.material = gsurface->material;
	info.surface  = *itr;

	if (!lighting)
		return true; // got all we need

	ITRLighting::Surface & lsurface = pLighting->surfaceList[info.surface];

	if (lsurface.mapIndex < 0) {
		*lighting = lsurface.mapColor & 0xffff;
		return true;
	} else {

	   Point2F textCoord = pGeometry->getTextureCoord(info.surface,point);
	   Point2I lightCoord;
      Int32 half = 1 << (pLighting->lightScaleShift - 1);
	   lightCoord.x = textCoord.x - half;
	   lightCoord.y = textCoord.y - half;
	   lightCoord.x += pLighting->surfaceList[info.surface].mapOffset.x;
	   lightCoord.y += pLighting->surfaceList[info.surface].mapOffset.y;
	   lightCoord >>= pLighting->lightScaleShift;
	   int xsize = lsurface.mapSize.x;

      if(lightCoord.x < 0)
         lightCoord.x = 0;
      if(lightCoord.x >= xsize)
         lightCoord.x = xsize - 1;
      if(lightCoord.y < 0)
         lightCoord.y = 0;
      if(lightCoord.y >= lsurface.mapSize.y)
         lightCoord.y = lsurface.mapSize.y;

	   if ((lsurface.mapIndex & 0x40000000) == 0) {
         // Uncompressed lightmap...
         ITRLighting::LightMap * pMap = isMissionLit() ? 
            m_pHighestMissionLighting->getLightMap(lsurface.mapIndex) : NULL;

         if( pMap == NULL )
            pMap = pLighting->getLightMap(lsurface.mapIndex);
            
	      *lighting = (&pMap->data)[lightCoord.y * xsize + lightCoord.x];
      } else {
         // compressed lightmap, tbi...
         int actualIndex = lsurface.mapIndex & ~0x40000000;
         int colorIndex  = lightCoord.y * xsize + lightCoord.x;
         
         UInt8* pCompData = isMissionLit() ? 
            (UInt8*)m_pHighestMissionLighting->getLightMap(actualIndex) : NULL;

         if( pCompData == NULL )
            pCompData = (UInt8*)pLighting->getLightMap(actualIndex);
         else
            pLighting = m_pHighestMissionLighting;
            
         BitStream bs(pCompData, 65536);

         *lighting = pLighting->findNthColor16(bs, colorIndex);
      }
   }

	return true;
}



//------------------------------------------------------------------------------
//    
// NOTES 
//    Collides with the interior, to find _all_ surfaces that intersect the
//    lights sphere.  For each affected surface, we then:
//    - enter a hashmap entry with the surface/light pair.
//    - Set the surfaces DynamicLight bit, and clear the lightmap valid
//      bit...
//
// Note see old code for prior method, was a clearAffected..(), plus a separate
//    dynamic light class.  
// 
//------------------------------------------------------------------------------
void
ITRInstance::findAffectedSurfaces( TSLight * io_pLight)
{
   Point3F     &lightPos = io_pLight->fLight.fPosition;
   float       lightRad = io_pLight->fLight.fRange;
   CollisionSurfaceList csList;
   
   // ITRGeometry* pCurrGeometry = getGeometry();
   ITRCollision collSphere ( getGeometry() );
   collSphere.collisionList = &csList;
   collSphere.collide ( lightPos, lightRad );
   
   for (int i = 0; i < csList.size(); i++) 
   {
      int sNum = csList[i].surface, surfaceLightCount = 0;

      // count how many times this surface occurs, i.e. how many lights have 
      // registered it.  the current light shouldn't be there...
      SurfaceLightCache::iterator itr = m_surfaceLightCache.find( sNum );
      while( ! itr.isDone() ){
         surfaceLightCount++;
         AssertFatal ( *itr != io_pLight, "Bad surface light cache op" );
         itr++;
      }
      
      if( surfaceLightCount < 4 ){
         if ( m_surfaceLightCache.isFull () ){
            // AssertWarn ( 0, "Surface light cache filling up" );
            return;
         }
         m_surfaceLightCache.insert ( sNum, io_pLight );
         Surface* pISurface = getSurface ( sNum );
         pISurface->setLightMapNotValid ();
         pISurface->setDynamicLit ();
      } 
   }
}

//------------------------------------------------------------------------------
//--------------------------------------
// ITRInstance::LightThread imp.
//--------------------------------------
//
ITRInstance::LightThread::LightThread()
 : m_pInstance(NULL),
   m_animState(UnInitialized),
   m_lightId(-1),
   m_timeScale(1.0f)
{
   //
}

ITRInstance::LightThread::~LightThread()
{
   m_pInstance = NULL;
   m_animState = UnInitialized,
   m_lightId   = -1;
   m_timeScale = 1.0f;
}

void
ITRInstance::LightThread::setTime(const float in_newTime)
{
   AssertFatal(m_pInstance != NULL, "No instance");
   AssertFatal(m_lightId >= 0, "Bad light");
   
   float totalTime = getTotalAnimationTime();
   AssertFatal(in_newTime <= totalTime, "Error, bad time");

   m_pInstance->setLightTime(m_lightId, in_newTime);
   m_animState = Animating;
}

void
ITRInstance::LightThread::advanceTime(const float in_deltaTime)
{
   AssertFatal(m_animState != UnInitialized, "Invalid state");
   AssertFatal(m_pInstance != NULL, "No instance");
   AssertFatal(m_lightId >= 0, "Bad light");

   if (m_animState == Animating) {
      float actualDelta = in_deltaTime * m_timeScale;
      bool contAnim = m_pInstance->stepLightTime(m_lightId, actualDelta);
      
      if (contAnim == false)
         m_animState = Stopped;
   }
}

void
ITRInstance::LightThread::setState(const int in_state)
{
   AssertFatal(m_pInstance != NULL, "No instance");
   AssertFatal(m_lightId >= 0, "Bad light");
   AssertFatal(in_state >= 0 && in_state < getNumStates(),
               "Out of bounds state");

   m_pInstance->setLightState(m_lightId, in_state);
   m_animState = Animating;
}


//------------------------------------------------------------------------------
//--------------------------------------
// Factory methods for LightThreads...
//--------------------------------------
//
ITRInstance::LightThread*
ITRInstance::createLightThread(const int in_lightId)
{
   AssertFatal(m_lightThreadList.isThreadActive(in_lightId) == false,
               "Cannot assign multiple threads to a single light...");
   
   LightThread* pThread = new LightThread;
   pThread->m_pInstance = this;
   pThread->m_lightId   = in_lightId;
   m_lightThreadList.insertThread(pThread);
   
   return pThread;
}

void
ITRInstance::destroyLightThread(LightThread* io_pDestroy)
{
   AssertFatal(io_pDestroy->m_pInstance == this,
               "Error, thread does not belong to this instance");
   
   m_lightThreadList.removeThread(io_pDestroy);
   delete io_pDestroy;
}


//------------------------------------------------------------------------------
//--------------------------------------
// ITRInstance::LightThreadList imp.
//--------------------------------------
//
void
ITRInstance::LightThreadList::insertThread(LightThread* io_pInsert)
{
   AssertFatal( end() == ::find( begin(), end(), io_pInsert ),
      "TS::ShapeInstance::ThreadList::insert: thread already in list" );
   push_back( io_pInsert );
}

void
ITRInstance::LightThreadList::removeThread(LightThread* io_pRemove)
{
   iterator loc = ::find( begin(), end(), io_pRemove );
   AssertFatal( loc != end(), 
      "TS::ShapeInstance::ThreadList::remove: thread not in list" );
   *loc = last();
   decrement();
}

bool
ITRInstance::LightThreadList::isThreadActive(const int in_lightId)
{
   iterator itr = findById( in_lightId );
   if (itr == end()) {
      return false;
   } else {
      return true;
   }
}

ITRInstance::LightThreadList::iterator
ITRInstance::LightThreadList::findById(const int in_lightId)
{
   iterator itr = begin();
   for (; itr != end(); itr++) {
      if ((*itr)->getLightId() == in_lightId)
         break;
   }
   
   return itr;
}

