//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMINTERIORSHAPE_H_
#define _SIMINTERIORSHAPE_H_

//Includes
#include <itrInstance.h>
#include <itrGeometry.h>
#include <itrRender.h>
#include <simCollisionImages.h>

#include "simMovingShape.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class DLLAPI SimInteriorShape;
class MissionLighting;
class LinkVertexArena;
class SimTimerEvent;
class SimInteriorShapeDatabaseSimple;
class SimInteriorShapeDatabasePVS;

enum LinkFace {
   FaceNone = 0,

   FaceMinX = 1 << 7,   // These values map to those in the outside bits
   FaceMinY = 1 << 6,   //  field on the geometry...
   FaceMinZ = 1 << 5,
   FaceMaxX = 1 << 4,
   FaceMaxY = 1 << 3,
   FaceMaxZ = 1 << 2,

   FaceAll  = 252,
   
   __forceDWord__ = 0x7fffffff
};
      
class DLLAPI SimInteriorShape : public SimMovingShape
{
   typedef SimMovingShape Parent;
   static int sm_fileVersion;

   friend class SimInteriorShapeDatabaseSimple;
   friend class SimInteriorShapeDatabasePVS;
   friend class MissionLighting;
   friend class InteriorPlugin;

  protected:
   struct InteriorLink {
      SimInteriorShape* pInterior;
      LinkVertexArena*  pVertexArena;
      LinkFace          linkFace;
      
      bool testFace(const int in_outsideBits) const;
   };
   typedef Vector<InteriorLink> InteriorLinkList;

   struct RenderImage : public SimRenderImage, public SimShapeImageDet {
      SimInteriorShape* pSimInteriorShape;
      bool              linkRendering;
      
      TMat3F       transform;
      ITRInstance* currShape;
      
      ITRRender    rend;

      ShapeImageType getImageType() const;
      void render(TS::RenderContext &rc);
      RenderImage() : currShape(NULL), pSimInteriorShape(NULL), linkRendering(false)
      {
         itype = SimRenderImage::Normal;
      }
   };
   friend struct RenderImage;
   
   struct LightAnimParam {
      float  time;
      bool   active;
      UInt32 dummy1;
      UInt32 dummy2;
   };
   
   enum NetMaskFlags {
      SimShapeResPt   = 1 << 1,  // SimShape reserves all flags <= SimShapeResPt
      
      ShapeLightActiveMask = SimShapeResPt << 1,
      EditModeMask         = SimShapeResPt << 2,
      
      InitialUpdate   = 1 << 31
   };

   enum RenderForce {
      ForceEdit = 1 << 0,
      ForceLink = 1 << 1
   };

   char* m_pFileName;         // G+/P+
   bool  m_disableCollisions; // G+/P+

   ITRInstance*         m_pITRInstance;   // G-/P-
   RenderImage          m_renderImage;    // G-/P-
   SimCollisionItrImage m_collisionImage; // G-/P-

   int   m_initialState;      // G+/P+  defaults to 0
   int   m_destroyedState;    // G+/P+  -1 indicates no such state or not yet set.
   int   m_lightState;        // G+/P+

   float m_lightUpdateDelta;       // G-/P- (D)
   Int32 m_maxLightUpdatePerSec;   // G+/P+

   float m_lastUpdatedTime;        // G-/P-

   Vector<LightAnimParam>            m_lightAnimParams;  // G+/P+
   Vector<ITRInstance::LightThread*> m_lightThreads;     // G-/P-

   InteriorLinkList  m_currentLinks;

   UInt32   m_forceRender;

   bool     m_isContainer;

   //-------------------------------------- Shape state functions
  protected:
   bool setFileName(const char* in_pFileName);
   void setIsContainer(const bool in_isContainer);
   void setShapeStates(const UInt32 in_init, const UInt32 in_dest);
   bool setLightState(const UInt32 in_lightState);


   //-------------------------------------- Resource management
  protected:
   bool reloadResources();
   void initRenderImage();
   
  public:
   bool missionLitName();
   void getBaseFilename( String & str );
   

   //-------------------------------------- Lighting functions
  protected:
   void shutdownLights();     // Called before the ITRInstance is switched
   void restartLights();      // Called after  the ITRInstance is reloaded
   void initLightParams();    // Called to start any appropriate animations

   //-------------------------------------- Interior linking functions...
  protected:
   void clearLinks();
   void findAllLinks();
   void unlinkMe(const SimInteriorShape* in_pLinked);
   bool linkMe(SimInteriorShape* in_pNewLink,
               LinkVertexArena*  io_pArena);

   Box3F getITRSpaceBBox(const SimInteriorShape* in_pOtherItr) const;
   LinkFace getAbuttingFace(const Box3F& in_rMyBox,
                            const Box3F& in_rOtherBox) const;
   int correctOutsideBits(const int       in_oldBits,
                          TS::Camera*     io_camera,
                          TS::PointArray* io_pointArray);

   bool renderLinks(SimContainerRenderContext&);
   bool render(SimContainerRenderContext& rc);

   int  m_scopeTag;
   bool scopeLinks(SimContainerScopeContext&);
   bool scope(SimContainerScopeContext& sc);


   //-------------------------------------- Event/Query handlers
  protected:
   bool onAdd();
	bool onSimEditEvent(const SimEditEvent*);
   bool onSimTimerEvent(const SimTimerEvent*);
   bool onSimTriggerEvent(const SimTriggerEvent*);
   bool onSimRenderQueryImage(SimRenderQueryImage*);
   bool onSimCollisionImageQuery(SimCollisionImageQuery*);

   void inspectRead(Inspect *inspector);
   void inspectWrite(Inspect *inspector);


   //-------------------------------------- Virtual Overrides
  protected:
   // SimShape
   void shapeFlagsChanged();

   // SimMovement
   void buildContainerBox(const TMat3F& mat, Box3F* box);

   //-------------------------------------- Event/Query/Arg processing
  public:
   bool processQuery(SimQuery* query);
   bool processEvent(const SimEvent* event);
   bool processArguments(int argc, const char **argv);

   
   //-------------------------------------- Basic functions
  public:
   SimInteriorShape();
   ~SimInteriorShape();
	ITRInstance*  getInstance() const { return m_renderImage.currShape; }

   UInt32 getPartType() const;

   ITRInstance* getInstance() { return m_pITRInstance; }
   
   bool getWeatherDistance(const Point3F& in_rWorldPos,
                           float&         out_rDistance);

   // Bounding box extension.  Needed for some light calculations outside
   //  of class...
   //
   static const float csm_BBoxExtension;

   //-------------------------------------- Persistent Functionality
  public:
   DWORD packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream);
   void  unpackUpdate(Net::GhostManager *gm, BitStream *stream);
   Persistent::Base::Error read(StreamIO &, int version, int user);
   Persistent::Base::Error write(StreamIO &, int version, int user); 
   int version();
   DECLARE_PERSISTENT(SimInteriorShape);
};


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_SIMINTERIORSHAPE_H_
