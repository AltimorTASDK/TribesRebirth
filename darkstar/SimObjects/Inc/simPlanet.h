//--------------------------------------------------------------------------- 
//
// simPlanet.h
//
//--------------------------------------------------------------------------- 

#ifndef _SIMPLANET_H_
#define _SIMPLANET_H_

#include <simNetObject.h>
#include <simRenderGrp.h>
#include <simLight.h>
#include <ts_shapeInst.h>
#include <move.h>

//--------------------------------------------------------------------------- 

class SimContainer;

class LensFlare : public SimRenderImage
{
   public:
      struct FlareInfo {
         int textureIndex;
         float dist;
         float minScale;
         float scaleRange;
         bool rotate;
         FlareInfo(int _index, float _dist, float _minScale, float _maxScale, bool _rotate) { 
            textureIndex = _index; 
            dist = _dist; 
            minScale = _minScale; 
            scaleRange = _maxScale - _minScale; 
            rotate = _rotate;
         }
      };

   private:
      static const char *filename;
      
      bool visible;
      bool initialized;
      int renderCount;     // controls how whether we do a LOS query
      bool obscured;
      Point3F sunPosProjected, sunPosWorld;
      ColorF color;
      Vector<FlareInfo> flares;
      Resource<TSMaterialList> hMaterialList;
      SimContainer *root;
            
   public:
      static bool renderDetail;
   
      static void setMaterialListFilename(const char *_filename) { filename = _filename; }
      static const char *getMaterialListName() {return filename;}
   
      void render(TSRenderContext &rc);
      void setSunPos(bool visible, const Point3F &sunPos, const Point3F &worldPos);
      void initialize(SimManager *manager);
      bool isInitialized() { return initialized; }
      void setColor(const ColorF &_color) { color = _color; }
      void setRootContainer(SimContainer *_root) { root = _root; }
      LensFlare();
};

//--------------------------------------------------------------------------- 

struct PlanetRenderImage : public SimRenderImage
{
   Point3F position;
   float visibleDistance;
   GFXBitmap *texture;
   LensFlare *lensFlare;  // for positional updates
   float size;
   float distance;
     
   void render(TSRenderContext &rc);
};  

//--------------------------------------------------------------------------- 

class SimPlanet : public SimNetObject
{
   private:
      typedef SimNetObject Parent;
      
   protected:
      enum NetMaskBits {
         Modified = 1,
      };
      enum Constants {
         CurrentVersion = 1,   
      };

      int textureTag;
      float azimuth;
      float incidence;
      bool castShadows;
      bool useLensFlare;
      ColorF intensity;
      ColorF ambient;

      bool inLightSet;
      bool inRenderSet;            
      Resource<GFXBitmap> hTexture;
      LensFlare lensFlare;
      PlanetRenderImage planet;
      TSLight light;
           
      void streamRead(StreamIO &sio);
      void streamWrite(StreamIO &sio);
 
      void load();
      void unload();
      
   public:
      static bool shadows;
      static Point3F shadowDirection;
 
      SimPlanet();
      ~SimPlanet();

   	bool processQuery(SimQuery*);
      bool processArguments(int argc, const char **argv);
		bool onAdd();
		void onRemove();
      void onPreLoad(SimPreLoadManager *splm);

		void inspectRead(Inspect *inspector);
		void inspectWrite(Inspect *inspector);

      void set(int _textureTag, float _azimuth, float _incidence,
         bool _useLensFlare = false, bool _castShadows = false,
         ColorF _intensity = ColorF(0.0f, 0.0f, 0.0f), 
         ColorF _ambient = ColorF(0.0f, 0.0f, 0.0f));

      DWORD packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream);
      void unpackUpdate(Net::GhostManager *gm, BitStream *stream);

      DECLARE_PERSISTENT(SimPlanet);
      static void initPersistFields();
      Persistent::Base::Error read(StreamIO &, int version, int user);
      Persistent::Base::Error write(StreamIO &, int version, int user);
      int version() { return (CurrentVersion); };
};

//--------------------------------------------------------------------------- 

#endif
