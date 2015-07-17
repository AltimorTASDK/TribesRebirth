//--------------------------------------------------------------------------- 
//
// simSky.cpp
//
//--------------------------------------------------------------------------- 


#include <sim.h>
#include <ts.h>
#include <g_bitmap.h>
#include <simTerrain.h>
#include <editor.strings.h>
#include "simSky.h"
#include "g_surfac.h"
#include "simResource.h"
#include "console.h"
#include "simPreLoadManager.h"

#pragma warn -inl
#pragma warn -aus

//--------------------------------------------------------------------------- 

#define DEGRAD (M_PI/180.0)
#define RADDEG (180.0/M_PI)
#define DISTANCE_FACTOR 1000.0
#define W_DIST (1.0f / 65535.0f)

//--------------------------------------------------------------------------- 

static Point2F textCoord[4];
static Point3F position[16];

float SimSky::PrefSkyDetail = 1.0f;


//--------------------------------------------------------------------------- 
void SimSky::makeBoundingBox()
{
   // This bounding box is less than the root container's bounding box, but
   //  greater than any "real" bounding box, ie the terrain.
   //
   Point3F pMin(-1e10f, -1e10f, -1e10f);
   Point3F pMax( 1e10f,  1e10f,  1e10f);
	setBoundingBox(Box3F(pMin, pMax));
}

//------------------------------------------------------------------------------
SimSky::SimSky()
{
   itype = Background; 
   sortValue = BACKGROUND_SORTVALUE;
   matListTag = 0;
   for (int i = 0; i < 16; i++)
      textures[i] = i%2;
   featurePosition = 0.0f;
   netFlags.set(SimNetObject::ScopeAlways);
   netFlags.set(SimNetObject::Ghostable);

   makeBoundingBox();
   size   = 7500.0f;                // size/height 
   distance = TEXTUREDSKY_DISTANCE; // default drawing distance
   paletteKey = 0xffffffff;
   skyIndex  = -1;                  // force color reevaluation
   hazeIndex = -1;
}

//------------------------------------------------------------------------------
SimSky::~SimSky()
{
} 

//------------------------------------------------------------------------------
void SimSky::unload()
{
   paletteKey = 0xffffffff;
   skyIndex  = -1;      // force color reevaluation
   hazeIndex = -1;
}

//------------------------------------------------------------------------------
void SimSky::onPreLoad(SimPreLoadManager *splm)
{
   if (!matListTag)
      return;

   const char *matList = SimTagDictionary::getString(matListTag);
   if (matList)
      splm->preLoadMaterialList(matList);
}   

//------------------------------------------------------------------------------
void SimSky::load()
{
   if (manager->isServer()) {
      // don't load sky resources in server
      setMaskBits(Modified);
      return;
   }      

   unload();
   if (matListTag != 0) {
      ResourceManager &rm = *SimResource::get(manager);
      const char *filename = SimTagDictionary::getString(matListTag);
   
      // load the material list from file
      hMaterialList = rm.load(filename);
      // don't want to bring down the mission editor, so if an invalid
      // dml is given, set the color to ugly
      AssertWarn((bool)hMaterialList, avar("Error reading materialList file \"%s\"", filename));
      if ((bool)hMaterialList && hMaterialList->getMaterialsCount() > 0) 
      {
         hMaterialList->load(rm, true);
         initialize();
      }
      else        
         // set ugly color if dml wasn't found
         set(ColorF(1.0f, 0.0f, 0.0f));
   }
   setMaskBits(Modified);
}

//------------------------------------------------------------------------------      
void SimSky::set(ColorF color)
{
   skyIndex = -1;
   skyColor = color;
   hazeIndex = -1;
   hazeColor = color;      
}   

//--------------------------------------------------------------------------- 
bool SimSky::processArguments(int argc, const char **argv)
{
   if(argc == 0)
   {
      set(ColorF(0,0,0));
      return true;
   }
   if(argc >= 3)
   {
      set(ColorF(atof(argv[0]), atof(argv[1]), atof(argv[2])));
      if(argc == 3)
         return true;
      if(argc == 22)
      {
         int matTag = atoi(argv[3]);
         if (!matTag) {
            Console->printf("\"%s\" not found in tag dictionary", argv[3]);
            return false;
         }
         int tx[16];
         for (int i = 0; i < 16; i++)
            sscanf(argv[5 + i], "%d", &tx[i]);

         float featureAz = atof(argv[4]);
         set(matTag, featureAz, tx);
      }
   }
   Console->printf("SimSky: [colR colG colB] [matListTagDefine rotZ i1 ... i16]");
   return false;
}

//--------------------------------------------------------------------------- 
bool SimSky::processQuery(SimQuery *query)
{
   switch (query->type) {
      case SimRenderQueryImageType: {
         SimRenderQueryImage *q = static_cast<SimRenderQueryImage *>(query);
         if (q->containerRenderQuery == false) 
         {
            // Container system is querying...
            q->image[0] = this;
            q->count = 1;
         }
         break;
      }
      default:
         return false;
   }
   return true;
}

//--------------------------------------------------------------------------- 
bool SimSky::onAdd()
{
	if (!Parent::onAdd())
		return false;

   if (!manager->isServer()) {
      SimContainer* root = NULL;
      
      root = findObject(manager,SimRootContainerId,root);
      AssertFatal(root, "No root container!");
      root->addObject(this);
      addToSet(SimRenderSetId);
   }
   load();
	return true;
}

//--------------------------------------------------------------------------- 
void SimSky::onRemove()
{
	unload();
	Parent::onRemove();
}	


//--------------------------------------------------------------------------- 
void SimSky::set(int _matListTag, float _featurePosition, int _textures[16])
{
   matListTag = _matListTag;
   featurePosition = _featurePosition;
   for (int i = 0; i < 16; i++)
      textures[i] = _textures[i];
   if (manager)
      load();
}

//--------------------------------------------------------------------------- 
void SimSky::setMaterialListTag(int _matListTag)
{
   matListTag = _matListTag;
   if (manager)
      load();
}

//--------------------------------------------------------------------------- 
void SimSky::inspectWrite(Inspect *inspect)
{
	Parent::inspectWrite(inspect);
   Point3F c(skyColor.red, skyColor.green, skyColor.blue);
   inspect->write(IDITG_SKY_BACKGROUNDCOLOR, c);
   inspect->write(IDITG_MATERIAL_LIST_TAG, true, "IDDML_*", (Int32)matListTag);
   inspect->write(IDITG_SKY_ROTATION, (float)featurePosition);
   inspect->write(IDITG_SIZE, (float)size);
   for (int i = 0; i < 16; i++)
      inspect->write(IDITG_SKY_TEXTUREINDEX_1 + i, (Int32)textures[i]);
}

//--------------------------------------------------------------------------- 
void SimSky::inspectRead(Inspect *inspect)
{
	Parent::inspectRead(inspect);
   Point3F c;
   inspect->read(IDITG_SKY_BACKGROUNDCOLOR, c);
   skyColor.set(c.x, c.y, c.z);

   Int32 temp;
   inspect->read(IDITG_MATERIAL_LIST_TAG, NULL, NULL, temp);
   matListTag = DWORD(temp);
   
   inspect->read(IDITG_SKY_ROTATION, (float)featurePosition);
   inspect->read(IDITG_SIZE,   (float)size);

   for (int i = 0; i < 16; i++) {
      inspect->read(IDITG_SKY_TEXTUREINDEX_1 + i, temp);
      textures[i] = DWORD(temp);
   }
 
   load();
}   

//------------------------------------------------------------------------------
int SimSky::version()
{
   return (CurrentFileVersion);
}   

//--------------------------------------------------------------------------- 

void SimSky::initPersistFields()
{
   //Parent::initPersistFields();
   addField("skyColor", TypePoint3F, Offset(skyColor,SimSky));
   addField("hazeColor", TypePoint3F, Offset(hazeColor,SimSky));
   addField("matListTag", TypeInt, Offset(matListTag, SimSky));
   addField("featurePosition", TypeFloat, Offset(featurePosition, SimSky));
   addField("size", TypeFloat, Offset(size, SimSky));
   addField("distance", TypeFloat, Offset(distance, SimSky));
   addField("textures", TypeInt, Offset(textures, SimSky), 16);
}

//--------------------------------------------------------------------------- 

Persistent::Base::Error SimSky::write(StreamIO &sio, int ver, int u)
{
   Persistent::Base::Error error = SimNetObject::write(sio, ver, u);
   if (error != Persistent::Base::Ok) 
      return error;

   sio.write(sizeof(skyColor), (Int8 *)&skyColor);
   sio.write(sizeof(matListTag), (Int8 *)&matListTag);
   sio.write(sizeof(featurePosition), (Int8 *)&featurePosition);
   sio.write(size);
   sio.write(distance);
	for (int i = 0; i < 16; i++) 
		sio.write((Int8)textures[i]);

	return (sio.getStatus() == STRM_OK) ? Ok : WriteError;
}

//--------------------------------------------------------------------------- 
Persistent::Base::Error SimSky::read(StreamIO &sio, int ver, int u)
{
   Persistent::Base::Error error = SimNetObject::read(sio, ver, u);
   if (error != Persistent::Base::Ok) 
      return error;

   sio.read(sizeof(skyColor), (Int8 *)&skyColor);
   sio.read(sizeof(matListTag), (Int8 *)&matListTag);
   sio.read(sizeof(featurePosition), (Int8 *)&featurePosition);
   if (ver == CurrentFileVersion)
   {
      sio.read(&size);
      sio.read(&distance);  // distance no longer needed
   }
	for (int i = 0; i < 16; i++) {
      Int8 t;
		sio.read(&t);
      textures[i] = t;
	}
	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}


//--------------------------------------------------------------------------- 
DWORD SimSky::packUpdate(Net::GhostManager *, DWORD, BitStream *stream)
{
   stream->write(sizeof(ColorF), (BYTE *) &skyColor);
   stream->write(sizeof(ColorF), (BYTE *) &hazeColor);
   stream->write(distance);
   stream->write(matListTag);
   stream->write(featurePosition);
   stream->write(size);
	for (int i = 0; i < 16; i++) 
		stream->writeInt(textures[i], 4);
   return 0;
}


//------------------------------------------------------------------------------
void SimSky::unpackUpdate(Net::GhostManager *, BitStream *stream)
{
   skyIndex = -1;
   hazeIndex = -1;
   stream->read(sizeof(ColorF), (BYTE *) &skyColor);
   stream->read(sizeof(ColorF), (BYTE *) &hazeColor);
   stream->read(&distance);
   stream->read(&matListTag);
   stream->read(&featurePosition);
   stream->read(&size);
	for (int i = 0; i < 16; i++) 
		textures[i] = stream->readInt(4);

   if (manager)
      load();
}


//------------------------------------------------------------------------------
void SimSky::setFillColor(GFXSurface *gfxSurface, int &index, ColorF &color)
{
   if (index == -1) 
   {
      // lookup color index from RGB
      GFXPalette *palette = gfxSurface->getPalette();
      if (palette) 
      {
         index = palette->GetNearestColor(255.0f*color.red, 255.0f*color.green, 255.0f*color.blue);
         gfxSurface->setFillColor(index);
      }         
      else
         // oops, no palette, try again next time...
         gfxSurface->setFillColor(0, 0xffffffff);
   }
   else
      gfxSurface->setFillColor(index, paletteKey);
}   

//--------------------------------------------------------------------------- 
void SimSky::render(TSRenderContext &rc)
{
   TSCamera *camera = rc.getCamera();
   if(camera->getCameraType() == TS::PerspectiveCameraType && matListTag && PrefSkyDetail > 0.33f)
      renderSkyBand(rc);
   else
      renderSolid(rc);
}   

//--------------------------------------------------------------------------- 
void SimSky::renderSolid(TSRenderContext &rc)
{
   GFXSurface *gfxSurface = rc.getSurface();
   TS::PointArray *pointArray = rc.getPointArray();
   TSCamera *camera = rc.getCamera();
	gfxSurface->setFillMode(GFX_FILL_CONSTANT);
   gfxSurface->setHazeSource(GFX_HAZE_NONE);
   gfxSurface->setShadeSource(GFX_SHADE_NONE);
   gfxSurface->setAlphaSource(GFX_ALPHA_NONE);
   gfxSurface->setTexturePerspective(FALSE);
	gfxSurface->setTransparency(FALSE);

   setFillColor(gfxSurface, skyIndex, skyColor);
  
   RectI vp = rc.getCamera()->getScreenViewport();
   Point3F ul(vp.upperL.x, vp.upperL.y, W_DIST);
   Point3F ur(vp.lowerR.x, vp.upperL.y, W_DIST);
   Point3F lr(vp.lowerR.x, vp.lowerR.y, W_DIST);
   Point3F ll(vp.upperL.x, vp.lowerR.y, W_DIST);

   gfxSurface->setZTest(GFX_ZWRITE);
   gfxSurface->addVertex(&ul);
   gfxSurface->addVertex(&ur);
   gfxSurface->addVertex(&lr);
   gfxSurface->addVertex(&ll);
   gfxSurface->emitPoly();
   gfxSurface->setZTest(GFX_ZTEST_AND_WRITE);
}   

//------------------------------------------------------------------------------
void SimSky::calcPoints()
{
   // Each 128x128 bitmap gets wrapped 22.5 degrees, so there are 
   // 360/22.5 = 16 points around, but we duplicate the first point
   // in the last point for simplicity (so 17 total)
   float a = 90.0f + featurePosition - 22.5;
   for (int i=0; i < 16; i++) {
      position[i] = Point3F(distance*cos(DEGRAD*a), distance*sin(DEGRAD*a), 0.0f);
      a -= 22.5f;
   } 
}   


//--------------------------------------------------------------------------- 
void SimSky::renderSkyBand(TSRenderContext &rc)
{
   TSCamera *camera = rc.getCamera();
   TS::PointArray *pointArray = rc.getPointArray();
   pointArray->reset();

   Point3F displacement = camera->getTCW().p;

   //--------------------------------------
   // find the terrain's visible distance 
   float visibleDistance;
   SimTerrain *terrain = static_cast<SimTerrain*>(manager->findObject(SimTerrainId));
   if (terrain)
      visibleDistance = terrain->getVisibleDistance()*1.5f;
   else
       visibleDistance = 5000.0f;

   displacement.z = -displacement.z*(distance/visibleDistance);
   //displacement.z = 0.0f;
   //--------------------------------------

   int i;
   for(i = 0; i < 16; i++)
   {
      Point3F pt = position[i];
      pt += displacement;
      pointArray->addPoint(pt);
      pt.z += size;
      pointArray->addPoint(pt);
   }
   Point3F pt = displacement;    
   pt.z -= 500;
   pointArray->addPoint(pt);     // bottom
   pt.z += 100000 + size;
   pointArray->addPoint(pt);     // top

   GFXSurface *gfxSurface = rc.getSurface();
   gfxSurface->setZTest(GFX_ZALWAYSBEHIND);

   gfxSurface->setHazeSource(GFX_HAZE_NONE);
   gfxSurface->setShadeSource(GFX_SHADE_NONE);
   gfxSurface->setAlphaSource(GFX_ALPHA_NONE);
   gfxSurface->setFillMode(GFX_FILL_CONSTANT);
   gfxSurface->setTexturePerspective(FALSE);
	gfxSurface->setTransparency(FALSE);

   pointArray->useIntensities(false);
   pointArray->useTextures(false);
   pointArray->useHazes(false);

   // bottom haze solid fill
   setFillColor(gfxSurface, hazeIndex, hazeColor);
   for(i = 0; i < 16; i++)
   {
      int next = (i + 1) & 0xF;
      TS::VertexIndexPair v[3];
      v[0].fVertexIndex = i + i;
      v[1].fVertexIndex = next + next;
      v[2].fVertexIndex = 32;
      pointArray->drawPoly(3, v, 0);
   }

   // top sky solid fill
   setFillColor(gfxSurface, skyIndex, skyColor);
   for(i = 0; i < 16; i++)
   {
      int next = (i + 1) & 0xF;
      TS::VertexIndexPair v[3];
      v[0].fVertexIndex = i + i + 1;
      v[2].fVertexIndex = next + next + 1;
      v[1].fVertexIndex = 33;
      pointArray->drawPoly(3, v, 0);
   }

	gfxSurface->setFillMode(GFX_FILL_TEXTURE);
   gfxSurface->setHazeSource(GFX_HAZE_NONE);

   gfxSurface->setShadeSource(GFX_SHADE_CONSTANT);
   gfxSurface->setConstantShade(1.0f);

   gfxSurface->setAlphaSource(GFX_ALPHA_NONE);
   gfxSurface->setTexturePerspective(PrefSkyDetail > 0.66 ? true : false);
	gfxSurface->setTransparency(FALSE);

   pointArray->useTextures(textCoord);
   pointArray->useTextures(true);

   // sky texture band
   for(i = 0; i < 16; i++)
   {
      int textureNo = textures[i];
      if (textureNo != -1) {
         int next = (i + 1) & 0xF;
         int index = textures[i]%hMaterialList->getMaterialsCount();
         gfxSurface->setTextureMap(hMaterialList->getMaterial(index).getTextureMap());

         TS::VertexIndexPair v[4];
         v[0].fVertexIndex = i + i;
         v[0].fTextureIndex = 1;
         v[1].fVertexIndex = i + i + 1;
         v[1].fTextureIndex = 2;
         v[2].fVertexIndex = next + next + 1;
         v[2].fTextureIndex = 3;
         v[3].fVertexIndex = next + next;
         v[3].fTextureIndex = 0;

         pointArray->drawPoly(4, v, 0);
      }
   }

   gfxSurface->setZTest(GFX_ZTEST_AND_WRITE);
   
}   

//--------------------------------------------------------------------------- 
void SimSky::initialize()
{
   calcPoints();

   // initialize static texture coordinates
   textCoord[0].x = 1.0f;   textCoord[0].y = 1.0f;
   textCoord[1].x = 0.0f;   textCoord[1].y = 1.0f;
   textCoord[2].x = 0.0f;   textCoord[2].y = 0.0f;
   textCoord[3].x = 1.0f;   textCoord[3].y = 0.0f;

   // find the color of the bottom polygon by looking at the last pixel
   // in the first column of the first bitmap (all the pixels in the bottom
   // rows of all the bitmaps should be this color).  Also find the top
   // color and pass off to the sky for the background
   for (int i = 0; i < 16; i++) 
      if (textures[i] != -1) 
      {
         int ndx = textures[i];
         if (ndx >= hMaterialList->getMaterialsCount()) 
            ndx = 0;
         const GFXBitmap *bitmap = hMaterialList->getMaterial(ndx).getTextureMap();
         hazeIndex   = bitmap->pBits[bitmap->stride*(bitmap->height - 1)];
         skyIndex    = bitmap->pBits[0];
         paletteKey  = bitmap->paletteIndex;
         break;
      }  
}

