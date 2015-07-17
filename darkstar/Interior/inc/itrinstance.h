//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _ITRINSTANCE_H_
#define _ITRINSTANCE_H_

//Includes
#include <ts_material.h>
#include <ts_rendercontext.h>
#include <ts_light.h>
#include <ts_color.h>
#include <ts_camera.h>
#include <tMHashMap.h>
#include <ts_shadow.h> // for TS::Poly

#include "itrShape.h"
#include "itrlighting.h"
#include "m_collision.h"

//--------------------------------------Forward declarations
struct GFXLightMap;
class  GFXSurface;
class  ITRGeometry;

// class  ITRDynamicLight;

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//----------------------------------------------------------------------------
struct ITRLightRegistryEntry {
   UInt32         lightKey;
   TSLight        * pLight;
};


class ITRInstance
{
   class InstanceKeyServer
   {
      UInt32 m_currentKey;
     public:
      InstanceKeyServer() : m_currentKey(0) { }

      UInt32 getNewInstanceKey();
   };
   static InstanceKeyServer sm_instanceKeyServer;

  protected:
  
   UInt8 m_flags;

  public:
  
   class LightThread;
   friend class LightThread;

   UInt32 m_instanceKey;

   // flag bit values
   enum {
      MissionLit = ( 1 << 0 )
   };
   
   bool isMissionLit() const { return( m_flags & MissionLit ) != 0; }
   void setMissionLit(bool lit = true) { lit ? m_flags |= MissionLit : m_flags &= ~MissionLit; }
   
  public:
   struct Surface
   {
      enum {
         LightMapValid   = (1 << 0),
         TextureValid    = (1 << 1),
         DynamicLightMap = (1 << 2)
      };

      UInt8 flags;
      //
      bool isLightMapValid() const { return (flags & LightMapValid)   != 0; }
      bool isTextureValid()  const { return (flags & TextureValid)    != 0; }
      bool isDynamicLit()    const { return (flags & DynamicLightMap) != 0; }
      bool isMissionLit()    const { return (flags & MissionLit)      != 0; }
      void setTextureValid()     { flags |= TextureValid; }
      void setTextureNotValid()  { flags &= ~TextureValid; }
      void setLightMapValid()    { flags |= LightMapValid; }
      void setLightMapNotValid() { flags &= ~LightMapValid; }
      void setDynamicLit()       { flags |= DynamicLightMap; }
      void setDynamicClear()     { flags &= ~DynamicLightMap; }
   };
   
  private:
   // -------------- Typedefs and constants
   //
   struct Light {
      int   state;
      float animationTime;          // current place in animation
      int   animationLoopCount;     // 
      DWORD currentPackedColor;     //
   };

   struct StateMapInfo {
      ITRLighting::LightMap* stateMap;
      DWORD                  packedColor;
   };

   // work space for getPolys (based on itrcollision)
   struct PlaneEntry {
      int   planeIndex;
      float distance;
   };

   typedef Vector<Surface>       SurfaceList;
   typedef Vector<StateMapInfo>  LightMapList;
   typedef Vector<Light>         LightList;
   typedef Vector<PlaneEntry>    PlaneStack;
   // typedef Vector<ITRLightRegistryEntry>     LightRegistry;
   typedef MultiHashMap<int, TSLight*>       SurfaceLightCache;
   
  private:

   // -------------- Instance data.
   //
   SurfaceList  m_surfaceList;
   LightMapList m_lightMapList;
   LightList    m_lightList;
   static SurfaceLightCache  m_surfaceLightCache;
   TSRenderContext      * m_renderContext;

  protected:
  
   // ...Detailing members
   //
   Resource<ITRShape>              m_shapeResource;
   Resource<TSMaterialList>        m_materialListResource;
   Vector< Resource<ITRGeometry> > m_geometryResources;
   Vector< Resource<ITRLighting> > m_lightingResources;
   Vector< Resource<ITRMissionLighting> > m_missionLightingResources;

  private:
  
   // ...Constant pointers
   //
   ITRShape*       m_pShape;
   TSMaterialList* m_pMaterialList;

   // ...Pointers dependant on state/lod
   //
   ITRGeometry*         m_pCurrGeometry;
   ITRLighting*         m_pCurrLighting;
   ITRMissionLighting*         m_pCurrMissionLighting;

   // ...Pointers NOT dependant on lod
   // (state may be a problem)
   ITRGeometry* m_pHighestGeometry;
   ITRLighting* m_pHighestLighting;
   ITRMissionLighting* m_pHighestMissionLighting;

   // ...Current state info
   //
   UInt32           m_currentState;
   ITRShape::State* m_pCurrState;
   UInt32           m_currentLOD;
   ITRShape::LOD*   m_pCurrLOD;
   UInt32           m_currLightState;


   Box3F   m_highestBoundingBox;    // Detail selection helpers.  All coordinates
   float   m_lowestRadius;          //  are interior local...
   Point3F m_centerPoint;

   //
   void findAffectedSurfaces(TSLight * io_pLight);
   // void clearAffectedSurfaces(ITRDynamicLight* io_pLight);

   void _setInteriorState(const UInt32 in_state);
   void _setDetailLevel(const UInt32 in_detailLevel);


   // Lighting functions...
   //
   void merge(GFXLightMap*           dst,
                      ITRLighting::LightMap* src,
                      DWORD                  packedColor);
   int mergeLoop ( TSInten16Xlat::RemapTable &, const Point2F &, const Point2F &, 
                        float, float, float, Point2I, UInt16 * );
   int mergeDynamicLight(GFXLightMap * io_dst, int surfaceIndex,
                                  TSLight* io_pLight, float texScale );
   void mergeVectorLights ( GFXLightMap * io_dst, int surfaceIndex );
   void installLightState(Light&              instanceLight,
                                  ITRLighting::Light& light,
                                  int                 state);
   void updateSpecialLight(Light&              io_rLight,
                                   ITRLighting::Light& io_rLLight);
   void updateLight(Light&              io_rLight,
                            ITRLighting::Light& io_rLLight);

   Point3F center;
   float   radius;
   PlaneStack  planeStack;
   Vector<int> surfaceStack; // used by getPolys to keep track of which surfaces have been chosen
   Point3F     vec[3];

   // private routines for getting polys
   Int32 findLeaf(const Point3F & p, Int32 nodeIndex=0);
   Int32 findLeafGeom(ITRGeometry*, const Point3F & p, Int32 nodeIndex=0);
   void getPolys(int nodeIndex, const Box3F & box, const TMat3F & transBack,
                         TS::PolyList &pl);
   void getPolysLeaf(int nodeIndex, const Box3F & box, const TMat3F & transBack,
                             TS::PolyList &pl);

  public:
   ITRInstance(ResourceManager*          io_pRM,
                          Resource<ITRShape>&       io_rShape,
                          const UInt32              in_initialLightState,
                          bool                      in_missionLit = false,
                          bool                      in_loadDML=true );
   ~ITRInstance();

   Surface* getSurface(int surface);
   
   bool getSurfaceInfo(CollisionSurface & info, UInt16 * lighting);

   void  incLightState(int lightId);
   void  setLightState(int lightId, int state);
   void  setLightTime(int lightId, float time);
   bool  stepLightTime(int lightId, float timeStep);
   void  setLightLoopCount(int lightId, Int32 loopCount);

   float getLightTime(int lightId);    // Both return -1 on error
   int   getLightState(int lightId);

   void  getAutoStartIDs(Vector<int>& out_rAnimating);
   
   // For Dynamic Lights
   //
   // ITRDynamicLight* addDynamicLight(const UInt32 in_lightKey);
   // void             removeDynamicLight(const UInt32 in_lightKey);

   // State/Detail Management...
   //
   bool   setInteriorState(const UInt32 in_state);
   UInt32 getInteriorState() const { return m_currentState; }

   bool   setInteriorLightState(ResourceManager* io_pRM,
                                const UInt32     in_lightState);
   UInt32 getInteriorLightState() const { return m_currLightState; }

   UInt32 getDetailLevel() const { return m_currentLOD; }
   void setDetailLevel(const UInt32 in_detailLevel);
   void setDetailByPixels(const float in_projectedPixels);

   bool isLinked() const;

   void startRender ( TSRenderContext & rc );
   GFXLightMap* buildLightMap(GFXSurface* gfxSurface, int surface, float texScale );
   int          getLightScale();

   ITRShape*       getShape()        { return m_pShape; }
   int             getNumDetails()   { return m_geometryResources.size(); }
   ITRGeometry*    getGeometry()     { return m_pCurrGeometry; }
   ITRLighting*    getLighting()     { return m_pCurrLighting; }
   ITRMissionLighting*    getMissionLighting() { return m_pCurrMissionLighting; }
   ITRGeometry *   getGeometry( UInt32 index ){ return( m_geometryResources[index] ); }
   ITRLighting *   getLighting( UInt32 index ){ return( m_lightingResources[index] ); }
   ITRMissionLighting *   getMissionLighting( UInt32 index ){ return( m_missionLightingResources[index] ); }
      
   TSMaterialList* getMaterialList() { return m_pMaterialList; }
   void setMaterialList(Resource<TSMaterialList> io_newMatList);

   // public routines for getting surface polys from interior
   void getPolys(const Box3F& box, const TMat3F & toITR,
                 const TMat3F & transBack, TS::PolyList & pl, int maxPolys);

   ITRGeometry*   getHighestGeometry()          { return m_pHighestGeometry; }
   const Box3F&   getHighestBoundingBox() const { return m_highestBoundingBox; }
   UInt32         getLinkableFaces()      const;
   const Point3F& getLowestCenterPt()     const { return m_centerPoint; }
   float          getLowestRadius()       const { return m_lowestRadius; }

   //-------------------------------------- new LightThread functionality
  protected:
   class LightThreadList : public VectorPtr<LightThread*> {
     public:
      // a list of threads
      void insertThread( LightThread* );
      void removeThread( LightThread* );
      bool isThreadActive(const int in_lightId);
      
      iterator findById(const int);
   };

   LightThreadList   m_lightThreadList;

  public:
   class LightThread {
      friend class ITRInstance;

     public:
      enum AnimationState {
         UnInitialized,
         Animating,
         Stopped,
      };
      
     private:
      ITRInstance*        m_pInstance;
      AnimationState      m_animState;
      int                 m_lightId;
      float               m_timeScale;

      // LightThreads must be created/destroyed through (create/destroy)LightThread
      LightThread();
      ~LightThread();
     public:
      AnimationState getState() const { return m_animState; }
      int getLightId() const;
      
      float  getTotalAnimationTime() const;
      float  getCurrentAnimationTime() const;
      float  getTimeScale() const;
      int    getNumStates() const;
      UInt32 getAnimationFlags() const;

      void setTimeScale(const float in_timeScale);
      void setTime(const float in_newTime);
      void advanceTime(const float in_deltaTime);
      void setState(const int in_state);
   };

   LightThread* createLightThread(const int in_lightId);
   void destroyLightThread(LightThread* io_pDestroy);
};


//----------------------------------------------------------------------------

inline ITRInstance::Surface* 
ITRInstance::getSurface(int surface)
{
   return &m_surfaceList[surface];
}

inline int ITRInstance::getLightScale()
{
   return getLighting()->lightScaleShift;
}

inline bool ITRInstance::isLinked() const
{
   if (m_pShape != NULL)
      return m_pShape->isLinked();
   else
      return false;
}

inline UInt32 ITRInstance::getLinkableFaces() const
{
   if (m_pCurrLOD != NULL) {
      return m_pCurrLOD->getLinkableFaces();
   } else {
      return ITRShape::sm_allFacesLinkable;
   }
}


//----------------------------------------------------------------------------

inline int
ITRInstance::LightThread::getLightId() const
{
   AssertFatal(m_animState != UnInitialized, "Error, bad light state");
   
   return m_lightId;
}

inline float
ITRInstance::LightThread::getTotalAnimationTime() const
{
   AssertFatal(m_pInstance != NULL, "No Installing instance");
   AssertFatal(m_lightId >= 0, "no light");
   
   // Need to look this up in the lighting structure...
   //
   ITRLighting* pLighting = m_pInstance->getLighting();
   int lightIndex = pLighting->findLight(m_lightId);
   AssertFatal(lightIndex >= 0, "Could not find light in lighting?");
   
   ITRLighting::Light& lLight = pLighting->lightList[lightIndex];
   return lLight.animationDuration;
}

inline float
ITRInstance::LightThread::getCurrentAnimationTime() const
{
   AssertFatal(m_pInstance != NULL, "No initializing instance");
   AssertFatal(m_animState != UnInitialized, "Error, bad light state");
   AssertFatal(m_lightId >= 0, "no light");
   
   ITRLighting* pLighting = m_pInstance->getLighting();
   int lightIndex = pLighting->findLight(m_lightId);
   AssertFatal(lightIndex >= 0, "Bad lightid?");
   Light& light = m_pInstance->m_lightList[lightIndex];

   return light.animationTime;
}

inline UInt32
ITRInstance::LightThread::getAnimationFlags() const
{
   AssertFatal(m_pInstance != NULL, "No Installing instance");
   AssertFatal(m_lightId >= 0, "bad light id");
   
   // Need to look this up in the lighting structure...
   //
   ITRLighting* pLighting = m_pInstance->getLighting();
   int lightIndex = pLighting->findLight(m_lightId);
   AssertFatal(lightIndex >= 0, "Could not find light in lighting?");
   
   ITRLighting::Light& lLight = pLighting->lightList[lightIndex];
   return lLight.animationFlags;
}

inline int
ITRInstance::LightThread::getNumStates() const
{
   AssertFatal(m_pInstance != NULL, "No Installing instance");
   AssertFatal(m_lightId >= 0, "bad light id");
   
   // Need to look this up in the lighting structure...
   //
   ITRLighting* pLighting = m_pInstance->getLighting();
   int lightIndex = pLighting->findLight(m_lightId);
   AssertFatal(lightIndex >= 0, "Could not find light in lighting?");
   
   ITRLighting::Light& lLight = pLighting->lightList[lightIndex];
   return lLight.stateCount;
}

inline void
ITRInstance::LightThread::setTimeScale(const float in_timeScale)
{
   if (in_timeScale <= 0.0f) {
      // Maybe should just be != 0?
      AssertWarn(0, "Invalid time scale, must be >= 0");
   } else {
      m_timeScale = in_timeScale;
   }
}

inline float
ITRInstance::LightThread::getTimeScale() const
{
   return m_timeScale;
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_ITRINSTANCE_H_
