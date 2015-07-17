//--------------------------------------------------------------------------- 
//
// simSky.h
//
//--------------------------------------------------------------------------- 

#ifndef _TEXTUREDSKY_H_
#define _TEXTUREDSKY_H_

#include <simNetObject.h>
#include <simRenderGrp.h>
#include <ts_shapeInst.h>
#include <move.h>
#include <inspect.h>
#include <simContainer.h>

// by default the stars are in front of the sky c
#define PLANET_DISTANCE           19000.0f
#define TEXTUREDSKY_DISTANCE      20000.0f
#define STARFIELD_FRONT_DISTANCE  19500.0f
#define STARFIELD_BACK_DISTANCE   21500.0f

extern float STARFIELD_DISTANCE;
                             
#define BACKGROUND_SORTVALUE       0.0f
#define TEXTUREDSKY_SORTVALUE      10.0f
#define STARFIELD_FRONT_SORTVALUE  20.0f
#define PLANET_SORTVALUE           30.0f
#define STARFIELD_BACK_SORTVALUE   40.0f

//--------------------------------------------------------------------------- 


class SimSky : public SimContainer, SimRenderImage
{
   private:
      typedef SimContainer Parent;
      
   protected:
	   enum Constants {
	   	CurrentFileVersion = 2,	
	   };

      bool resolveColor;
      int skyIndex;
      int hazeIndex;
      ColorF skyColor;
      ColorF hazeColor;
      int paletteKey;

      enum NetMaskBits
      {
         Modified = 1,
      };
            
      bool    loaded;
		int     matListTag; 
      int     bottomColor;
      int     textures[16];
      float   featurePosition;
      Resource<TSMaterialList> hMaterialList;
      float size;
      float distance;

      void streamRead(StreamIO &sio);
      void streamWrite(StreamIO &sio);
      
      void load();
      void unload();   
      void calcPoints();
      void initialize();
      void makeBoundingBox();
      void setFillColor(GFXSurface *surface, int &index, ColorF &color);

      void renderSkyBand(TSRenderContext &rc);
      void renderSolid(TSRenderContext &rc);
      void set(ColorF color);

   public:
      static float PrefSkyDetail;
		SimSky();
      ~SimSky();

      #pragma warn -hid
      void render(TSRenderContext &rc);
      #pragma warn +hid

      void set(int _matListTag, float _featurePosition, int _textures[16]);
      void setMaterialListTag(int _matListTag);

      bool processQuery(SimQuery*);
      bool processArguments(int argc, const char **argv);
		bool onAdd();
		void onRemove();
      void onPreLoad(SimPreLoadManager *splm);
		void inspectRead(Inspect *inspector);
		void inspectWrite(Inspect *inspector);

      DWORD packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream);
      void unpackUpdate(Net::GhostManager *gm, BitStream *stream);

	   DECLARE_PERSISTENT(SimSky);
      static void initPersistFields();
	   Persistent::Base::Error read(StreamIO &, int version, int user);
   	Persistent::Base::Error write(StreamIO &, int version, int user);
      int version();
};

//--------------------------------------------------------------------------- 

#endif
