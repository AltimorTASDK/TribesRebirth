//--------------------------------------------------------------------------- 



//--------------------------------------------------------------------------- 

#ifndef _SIMTERRAIN_H_
#define _SIMTERRAIN_H_

#include <sim.h>
#include <grd.h>
#include <tstring.h>
#include <simNetObject.h>
#include <simContainer.h>
#include <simCollision.h>
#include <simCollisionImages.h>
#include <inspect.h>
#include <simCoordDb.h>

class SimActionEvent;
class MissionLighting;
struct TerrainFile;

//--------------------------------------------------------------------------- 

struct SimTerrainApplyEvent : public SimEvent
{
   SimTerrainApplyEvent() { type = SimTerrainApplyEventType; }
   DECLARE_PERSISTENT(SimTerrainApplyEvent);
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
};

inline Persistent::Base::Error SimTerrainApplyEvent::write( StreamIO &sio, int a, int b )
{
   Persistent::Base::Error error = SimEvent::write(sio, a, b);
   return error;
}   

inline Persistent::Base::Error SimTerrainApplyEvent::read( StreamIO &sio, int a, int b )
{
   return ( SimEvent::read(sio, a, b) ); 
}

//--------------------------------------------------------------------------- 

class DLLAPI SimTerrain: public SimContainer
{
   friend class MissionLighting;
	typedef SimContainer Parent;

public:
	enum Constants {
		CurrentFileVersion = 3,
	};
   enum {
   	BuildLightMapMessage = 0,
		SaveTerrainMessage,
		RefreshClientMessage,
   };
   enum NetMaskBits
   {
      TedFileMask    = BIT(0),
      VisibilityMask = BIT(1),
      DetailMask     = BIT(2),
      ContextMask    = BIT(3),
		MovementMask   = BIT(4),
      SuspendRenderMask = BIT(5),
      CrcMask        = BIT(6)
   };

	struct Visibility {
		// Parameters that are common to all players
	   float visibleDist;
	   float hazeDist;
		// Added in version 2 (file format)
		float hazeVerticalMin;
		float hazeVerticalMax;
	};
	struct Detail {
		// Local player detail settings
	   float perspectiveDist;
		float screenSize;
	};
	struct Context {
		Point3F pos;
		EulerF rot;
	};
	struct ContainerProperties {
		Point3F gravity;
		float drag;
		float height;
	};

   int      terrainTextureDetail;
   int      interiorMask;
   
protected:
   class RenderImage: public SimRenderImage
   {
   public:
      SimTerrain *myTerrain;
      GridRender gridRender;
      TerrainFile *terrainFile;
      Resource<GridFile> gridFile;
		TMat3F transform;
		TMat3F inverseTransform;
      void render( TSRenderContext &rc );
   };

   RenderImage renderImage;
	SimCollisionTerrImage collisionImage;
	Context context;
	Visibility maxVisibility;
	Visibility visibility;     // visibility after user prefs applied
	Detail maxDetail;
	Detail detail;             // detail after user prefs applied
	const char *tedFile;
	ContainerProperties containerProperties;
   SimXYDB * database;
   bool suspendRender;
   UInt32   terrainCrc;       // obtained from server object

   //   
   class LightInfoStrip
   {
      public:
      
         LightInfoStrip();
         ~LightInfoStrip();
         
         struct LightInfo
         {
            LightInfo();
            ~LightInfo();
            
            enum {
               Hit         =  BIT(0),
               Valid       =  BIT(1),
               Hires       =  BIT(2),
            };
            BitSet32 mFlags;
            
            ColorF   mCol;
            Point3F  mNormal;
            int      mBlockId;
            Point3F  mFilePos;
            Point2I  mBlockPos;
         };
      
         void create( int width, int scale, float offset, SimTerrain * terrain );
         void fillStrip( Point2I & pos );
         void hiresStrip();
         void loresStrip( LightInfoStrip * prevRow, LightInfoStrip * nextRow );
         
         void fillHiresBounds();

         LightInfo * getInfoList(){ return( mInfoList ); };

         // stats
         static int              mNumHires;
         static int              mNumHiresSkipped;
         
       private:
         
         int                     mWidth;
         LightInfo *             mInfoList;
         int                     mScale;
         float                   mOffset;
         SimTerrain *            mTerrain;
         Point2I                 mPos;
         static Vector<Box2F>    mHiresBounds;
   };

   //
   class LightingOptions
   {
      public:
         
         enum {
            InteriorLOS    = BIT(0),
            AllowHires     = BIT(1),
            CloudMap       = BIT(2),
            Console        = BIT(3),
            RemoveSolids   = BIT(4),
            SubSample      = BIT(5),

            // often used options            
            Hires          = ( InteriorLOS | AllowHires ),
         };
   
         BitSet32          mFlags;         

         // callback info   
         bool ( * mCollideCallback )( void * obj, Point3F & start, Point3F & end, ColorF & col );
         void * mCallbackObj;

         LightingOptions() : mFlags( 0 ) {};
   };
   
   LightingOptions lightingOptions;
   
	//
	struct Light {
		TS::Light* light;
		Point3F vector;
	};
   //
	typedef Vector<Light> LightList;
	LightList lightList;
	ColorF ambientLight;


	//
   bool onSimRegisterTextureEvent(const SimRegisterTextureEvent *event);
   bool onSimMessageEvent(const SimMessageEvent *event);
	bool onSimActionEvent(const SimActionEvent* event);
	bool onSimRenderQueryImage(SimRenderQueryImage* query);
	bool onSimObjectTransformQuery(SimObjectTransformQuery* query);
	bool onSimCollisionImageQuery(SimCollisionImageQuery* query);

	bool los(const Point3F& start, const Point3F& end);
   void lightArea( const Box2F & area);
   ColorF calcIntensity( const Point3F& pos, const Point3F& normal, bool & hitObject, bool allowShadow, bool hiresRun );
	void setColor(const Point2F& pos,const ColorF& color);
   void applyUserPrefs();

	// Container database virtuals
	void enterContainer(SimContainer*);
	void leaveContainer(SimContainer*);

public:
   friend class LightInfoStrip;
   
   static float PrefVisibleDistance;
   static float PrefPerspectiveDistance;
   static float PrefDetail;
   static float PrefTerrainTextureDetail;

	SimTerrain();
   ~SimTerrain();

   bool calcCrc( const char * fileName, UInt32 & crcVal );
	virtual bool create( const char *terrName, int GFsize=3, int GFgroundScale=6, int GBsize=256, int GBlightScale=0, GridFile::GBPatternMap pattern=GridFile::OneBlockMapsToAll );
   virtual bool createGridBlocks( int GBsize, int GBlightScale );
	virtual bool load( const char *filename );
   bool loadMaterials();
   bool save(const char *volumeName);

	virtual bool setVisibility( const Visibility *pTP=NULL, bool force = FALSE);
	virtual bool setDetail( const Detail *pTP=NULL );
	virtual bool setContainerProperties(const ContainerProperties& p);
	virtual const ContainerProperties &getContainerProperties() { return containerProperties; }

	void setContext(const EulerF&,const Point3F&);
   Context& getContext() { return context; }
	const TMat3F& getInvTransform() { return renderImage.inverseTransform; }
	const TMat3F& getTransform() { return renderImage.transform; }

   bool getSurfaceInfo(const Point2F &pos, CollisionSurface *info);
	bool getSurfaceHeight(const Point2F &pos, float *height, bool ignoreEmptyTiles = false);
	GridBlock::Material* getMaterial(const Point2F &pos);
	float getHazeValue(TSRenderContext &, const Point3F & objectPos);
   float getHazeValue(const Point3F &, const Point3F &);

   float getVisibleDistance() const;
   float getHazeDistance();
   float getPerspectiveDistance();
   float getScopeVisibleDist() const;

   GridFile* getGridFile();
   TerrainFile * getTerrainFile(){return(renderImage.terrainFile);}
	virtual void updateBoundingBox();

	void buildLightMap();
   void buildLightMap(const Box2F& area );
   void buildAllLightMaps( LightingOptions * options );

	//
	bool onAdd();
   bool processEvent(const SimEvent*);
   bool processQuery(SimQuery*);
   bool processArguments(int argc, const char **argv);
	void inspectRead(Inspect *inspector);
	void inspectWrite(Inspect *inspector);
   void setSuspendRender(bool _suspend);

   DWORD packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream);
   void unpackUpdate(Net::GhostManager *gm, BitStream *stream);

   DECLARE_PERSISTENT( SimTerrain );
   static void initPersistFields();
	int version();
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
};


//--------------------------------------------------------------------------- 

inline float
SimTerrain::getVisibleDistance() const
{
	return renderImage.gridRender.getVisibleDistance();
}

inline float SimTerrain::getHazeDistance()
{
   return renderImage.gridRender.getHazeDistance();
}

inline float SimTerrain::getPerspectiveDistance()
{
   return (renderImage.gridRender.getPerspectiveDistance());
}

inline GridFile* SimTerrain::getGridFile()
{
	return ( renderImage.gridFile );
}

#endif   // _TERRAIN_H_
