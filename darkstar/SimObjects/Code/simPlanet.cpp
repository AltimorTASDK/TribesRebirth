//--------------------------------------------------------------------------- 
//
// simPlanet.cpp
//
//--------------------------------------------------------------------------- 

#include <sim.h>
#include <g_pal.h>
#include <g_bitmap.h>

#pragma warn -inl
#pragma warn -aus

#include <ts.h>
#include <simLightGrp.h>
#include <simLight.h>
#include <simTerrain.h>
#include <editor.strings.h>
#include <simPreLoadManager.h>
#include "simSky.h"
#include "simPlanet.h"
#include "d_caps.h"
#include "g_surfac.h"
#include "simResource.h"
#include "console.h"

//--------------------------------------------------------------------------- 

#define DEGRAD (M_PI/180.0)
#define RADDEG (180.0/M_PI)

//--------------------------------------------------------------------------- 

static Point2F textCoord[4];

float boxPoints[4][2] = { {-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 1.0f} };

//--------------------------------------------------------------------------- 

const char *LensFlare::filename = "lensFlare.dml";

//--------------------------------------------------------------------------- 

bool LensFlare::renderDetail = true;

LensFlare::LensFlare()
{
   itype = Translucent;
   sortValue = 1.0;
   visible = false;
   sunPosProjected.set();
   sunPosWorld.set();
   initialized = false;
   color.set(1.0f, 1.0f, 1.0f);
   renderCount = 0;
   obscured = false;
}

//--------------------------------------------------------------------------- 

void LensFlare::initialize(SimManager *manager)
{
   initialized = true;
   ResourceManager &rm = *SimResource::get(manager);
   hMaterialList = rm.load(filename);
   AssertFatal((bool)hMaterialList, avar("Error reading materialList file \"%s\"", filename));
   AssertFatal(hMaterialList->getMaterialsCount() > 0, avar("material list \"%s\" is empty", filename));
   hMaterialList->load(rm, true);

   const GFXBitmap *bitmap = hMaterialList->getMaterial(0).getTextureMap();

   float dScale = 1.0f;
   if (flares.size() == 0) {
      flares.push_back(FlareInfo(1, -0.150f*dScale, 0.9f,   0.9f, true));
      flares.push_back(FlareInfo(0,  0.000f*dScale, 0.001f, 0.8f, false));
      flares.push_back(FlareInfo(1,  0.136f*dScale, 1.2f,   1.2f, true));
      flares.push_back(FlareInfo(2,  0.182f*dScale, 1.0f,   1.0f, true));
      flares.push_back(FlareInfo(4,  0.304f*dScale, 1.0f,   1.0f, true));
      flares.push_back(FlareInfo(4,  0.405f*dScale, 1.0f,   1.0f, true));
      flares.push_back(FlareInfo(3,  0.606f*dScale, 1.0f,   1.0f, true));
      flares.push_back(FlareInfo(3,  0.682f*dScale, 0.7f,   0.7f, true));
      flares.push_back(FlareInfo(5,  0.841f*dScale, 0.7f,   0.7f, true));
      flares.push_back(FlareInfo(1,  0.932f*dScale, 0.7f,   0.7f, true));
      flares.push_back(FlareInfo(1,  1.000f*dScale, 1.0f,   1.0f, true));
   }   
}

//--------------------------------------------------------------------------- 

void LensFlare::setSunPos(bool vis, const Point3F &sunP, const Point3F &worldPos)
{
   visible = vis;
   if(visible) {
      sunPosProjected = sunP;
      sunPosWorld = worldPos;
   }
}

//--------------------------------------------------------------------------- 

void LensFlare::render(TSRenderContext &rc)
{
   if (!renderDetail) {
      renderCount = 0;
      return;
   }

   GFXSurface *gfxSurface = rc.getSurface();
   if(!visible || !flares.size() || !(gfxSurface->getCaps() & GFX_DEVCAP_SUPPORTS_CONST_ALPHA)) {
      renderCount = 0;
      return;
   }

   renderCount++;
   if (obscured && renderCount%15 != 1)
      // if something was in the way last time we checked LOS, and it isn't time
      // to check LOS again, bail out early
      return;

   TS::PointArray *pointArray = rc.getPointArray();
   pointArray->reset();
   pointArray->useIntensities(false);
   pointArray->useTextures(textCoord);
   pointArray->useTextures(true);
	pointArray->setVisibility( TS::ClipMask );
   pointArray->useHazes(false);

   gfxSurface->setHazeSource(GFX_HAZE_NONE);
   gfxSurface->setShadeSource(GFX_SHADE_NONE);
	gfxSurface->setTransparency(FALSE);
   gfxSurface->setTexturePerspective(FALSE);
	gfxSurface->setFillMode(GFX_FILL_TEXTURE);
   gfxSurface->setTransparency(TRUE);
   gfxSurface->setAlphaSource(GFX_ALPHA_TEXTURE);

   TS::VertexIndexPair V[4];
   // if it was in the screen, go ahead and draw the lens flare.
   TSCamera *camera = rc.getCamera();
   RectI const &screenVp = camera->getScreenViewport();
   const Point2F vpSize(screenVp.lowerR.x - screenVp.upperL.x, screenVp.upperL.y + screenVp.lowerR.y);
   // preserve relative size of the textures, they should be
   // 1:1 on a 640x480 viewport
   Point2F vpScale(vpSize.x/640.0f, vpSize.y/480.0f);
   float scrSize = min(vpSize.x, vpSize.y);
   Point2F sunP(sunPosProjected.x, sunPosProjected.y);
   Point2F delta((screenVp.upperL.x + screenVp.lowerR.x) >> 1, (screenVp.upperL.y + screenVp.lowerR.y) >> 1);
   
   // find vector of the flare
   delta -= sunP;
   float deltaLen = delta.len();
   float deltaFrac = 2.0f*deltaLen / scrSize;
   float constAlpha = .5f*(1.0f - deltaFrac);
   if (constAlpha <= 0.0f)
   {
		if (renderCount == 1)
			renderCount = 0;
      return;
	}

   if (root && renderCount%15 == 1) {
      // do a LOS query, see if anything is in the way
      SimCollisionInfo collision;
      SimContainerQuery query;
      query.id    = 0;
      query.type  = SimContainerQuery::DefaultDetail;
      query.mask  = -1;
      Vector3F v = sunPosWorld - camera->getTCW().p;
      v.normalizef();
      v *= 2.0f;
      query.box.fMin = camera->getTCW().p + v;
      query.box.fMax = sunPosWorld;
      if (root->findLOS(query, &collision, SimCollisionImageQuery::High)) {
         obscured = true;
         return;
      }
   }
   obscured = false;
   gfxSurface->setConstantAlpha(constAlpha);
   
   // generate rotated box info
   float angle;
   if (delta.x == 0.0f && delta.y == 0.0f)
      angle = 0.0f;
   else
      angle = m_atan(delta.x, delta.y) - M_PI/2.0f;
   float c = m_cos(angle), s = m_sin(angle);
   float a = c*(-1.0f) - s*(-1.0f), b = s*(-1.0f) + c*(-1.0f);
   Point2F rotPoints[4];
   rotPoints[0].x =  a;  rotPoints[0].y =  b;
   rotPoints[1].x = -b;  rotPoints[1].y =  a;
   rotPoints[2].x = -a;  rotPoints[2].y = -b;
   rotPoints[3].x =  b;  rotPoints[3].y = -a;

   for (int i = 0; i < flares.size(); i++) {
      const FlareInfo &flare = flares[i];
      Point2F flarePos = delta;
      flarePos *= flare.dist;
      flarePos += sunP;
      const GFXBitmap *bitmap = hMaterialList->getMaterial(flare.textureIndex).getTextureMap();
      gfxSurface->setTextureMap(bitmap);
      Point2F dimension(bitmap->width*vpScale.x, bitmap->height*vpScale.y);
      const float scale = 0.5*(flare.minScale + (1.0f - deltaFrac)*flare.scaleRange);
      for (int j = 0; j < 4; j++) {
         Point3F drawPoint;
         if (flare.rotate)
            drawPoint.set(rotPoints[j].x*dimension.x, rotPoints[j].y*dimension.y, 0);
         else
            drawPoint.set(boxPoints[j][0]*dimension.x, boxPoints[j][1]*dimension.y, 0);
         drawPoint *= scale;
         drawPoint += Point3F(flarePos.x, flarePos.y, 1.0);
         V[j].fVertexIndex = pointArray->addProjectedPoint(drawPoint);
         V[j].fTextureIndex = j;
      }
      pointArray->drawProjectedPoly(4, V, 0);
   }
   
   gfxSurface->setTransparency(FALSE);
   if(deltaFrac < .3)
   {
      pointArray->useTextures(false);
      gfxSurface->setAlphaSource(GFX_ALPHA_CONSTANT);
      gfxSurface->setFillMode(GFX_FILL_CONSTANT);
      gfxSurface->setConstantAlpha(3.3 * (.3 - deltaFrac));
      gfxSurface->setFillColor(&color);
      V[0].fVertexIndex = pointArray->addProjectedPoint(Point3F(screenVp.upperL.x, screenVp.upperL.y, 1.0));
      V[1].fVertexIndex = pointArray->addProjectedPoint(Point3F(screenVp.lowerR.x, screenVp.upperL.y, 1.0));
      V[2].fVertexIndex = pointArray->addProjectedPoint(Point3F(screenVp.lowerR.x, screenVp.lowerR.y, 1.0));
      V[3].fVertexIndex = pointArray->addProjectedPoint(Point3F(screenVp.upperL.x, screenVp.lowerR.y, 1.0));
      pointArray->drawProjectedPoly(4, V, 0);
   }

   gfxSurface->setAlphaSource(GFX_ALPHA_NONE);
}

//--------------------------------------------------------------------------- 

//--------------------------------------------------------------------------- 

bool SimPlanet::shadows = false;
Point3F SimPlanet::shadowDirection = Point3F(0.0f, 0.0f, 1.0f);

SimPlanet::SimPlanet()
{ 
   planet.itype = SimRenderImage::Background;
   planet.sortValue = PLANET_SORTVALUE;
   planet.lensFlare = NULL;
   planet.size = 2000.0f;
   planet.distance = PLANET_DISTANCE;
   textureTag = 0;
   azimuth = 0.0f;
   incidence = 30.0f;
   castShadows = false;
   useLensFlare = false;
   inLightSet = false;
   inRenderSet = false;
   netFlags.set(SimNetObject::ScopeAlways);
   netFlags.set(SimNetObject::Ghostable);
}

SimPlanet::~SimPlanet()
{
}   

void SimPlanet::set(int _textureTag, float _azimuth, float _incidence,
   bool _castShadows, bool _useLensFlare, ColorF _intensity, ColorF _ambient)
{
   textureTag = _textureTag;
   azimuth = _azimuth;
   incidence = _incidence;
   castShadows = _castShadows;
   useLensFlare = _useLensFlare;
   intensity = _intensity;
   ambient = _ambient;
   if (manager)
      load();
}
   
//--------------------------------------------------------------------------- 

void SimPlanet::load()
{
   unload();
   
   if ((textureTag != 0) && (!manager->isServer())) {
      ResourceManager &rm = *SimResource::get(manager);
      const char *filename = SimTagDictionary::getString(manager, textureTag);
   
      // load the texture
      hTexture = rm.load(filename);
      AssertWarn((bool)hTexture, 
         avar("Error reading bitmap file \"%s\"", filename));
      // don't want to assert fatal because we don't want to bring down
      // the mission editor
      if ((bool)hTexture) {
         planet.texture = (GFXBitmap *)hTexture;
         addToSet(SimRenderSetId);
         inRenderSet = true;
      }         
   }   
   else
      planet.texture = NULL;

   // calculate planet position in world coordinates
   // add 90 to azimuth so that zero is at up Y axis
   if (incidence > 89.0f)
      incidence = 89.0f;
   if (incidence < -89.0f)
      incidence = -89.0f;
   const float az = azimuth + 90.0f;
   const float c = planet.distance*m_cos(DEGRAD*incidence);
   planet.position = Point3F(c*m_cos(DEGRAD*az), c*m_sin(DEGRAD*az), planet.distance*m_sin(DEGRAD*incidence));

   // initialize light if any
   Point3F direction = planet.position;
   direction.normalize();
   direction *= -1.0f;

   if (castShadows) {
      // set static data items
      shadowDirection = direction;
      shadows = true;
   }


   light.setAim(direction);
   //light.setType(TS::Light::LightDirectional);
   light.setType(TS::Light::LightDirectionalWrap);
   light.setIntensity(intensity.red, intensity.green, intensity.blue);
   if (intensity.red > 0.0f || intensity.green > 0.0f || intensity.blue > 0.0f 
      || ambient.red > 0.0f || ambient.green > 0.0f || ambient.blue > 0.0f) {
      addToSet(SimLightSetId);
      inLightSet = true;
      lensFlare.setColor(intensity);
   }
   planet.lensFlare = useLensFlare ? &lensFlare : NULL;

   // initialize static texture coordinates
   textCoord[0].x = 0.0f;  textCoord[0].y = 0.0f;
   textCoord[1].x = 1.0f;  textCoord[1].y = 0.0f;
   textCoord[2].x = 1.0f;  textCoord[2].y = 1.0f;
   textCoord[3].x = 0.0f;  textCoord[3].y = 1.0f;

   setMaskBits(Modified);
}

//-------------------------------------------------------------------------- 

void SimPlanet::unload()
{
   if (inLightSet) {
      removeFromSet(SimLightSetId);
      inLightSet = false;
   }      
   if (inRenderSet) {
      removeFromSet(SimRenderSetId);
      inRenderSet = false;
   }
   if (castShadows) 
      shadows = false;
      
   planet.lensFlare = NULL;
}

//--------------------------------------------------------------------------- 

void SimPlanet::inspectWrite(Inspect *inspect)
{
	Parent::inspectWrite(inspect);

   inspect->write(IDITG_BMP_TAG, true, "IDBMP_SKY_*", (Int32)textureTag);
   inspect->write(IDITG_SKY_AZIMUTH, (float)azimuth);
   inspect->write(IDITG_SKY_INCIDENCE, (float)incidence);
   inspect->write(IDITG_SIZE, (float)planet.size);
   inspect->write(IDITG_DISTANCE, (float)planet.distance);
   
   Point3F i(intensity.red, intensity.green, intensity.blue);
   inspect->write(IDITG_SKY_LIGHTINTENSITY, i);
   Point3F a(ambient.red, ambient.green, ambient.blue);
   inspect->write(IDITG_SKY_LIGHTAMBIENT, a);

   inspect->write(IDITG_SKY_LENSFLARE, useLensFlare);
   inspect->write(IDITG_SKY_SHADOWS, castShadows);
}   

//--------------------------------------------------------------------------- 

void SimPlanet::inspectRead(Inspect *inspect)
{
	Parent::inspectRead(inspect);
   unload();
   
   Int32 temp;
   inspect->read(IDITG_BMP_TAG, NULL, NULL, temp);
   textureTag = DWORD(temp);
   inspect->read(IDITG_SKY_AZIMUTH, (float)azimuth);
   inspect->read(IDITG_SKY_INCIDENCE, (float)incidence);
   inspect->read(IDITG_SIZE, (float)planet.size);
   inspect->read(IDITG_DISTANCE, (float)planet.distance);
   
   Point3F i;
   inspect->read(IDITG_SKY_LIGHTINTENSITY, i);
   intensity.set(i.x, i.y, i.z);
   Point3F a;
   inspect->read(IDITG_SKY_LIGHTAMBIENT, a);
   ambient.set(a.x, a.y, a.z);
   
   inspect->read(IDITG_SKY_LENSFLARE, useLensFlare);
   inspect->read(IDITG_SKY_SHADOWS, castShadows);

   load();
}   

//--------------------------------------------------------------------------- 

bool SimPlanet::processArguments(int argc, const char **argv)
{
   if(argc == 0)
      return true;

   if(argc != 8 && argc != 11)
   {
      Console->printf(
         "SimPlanet: textureTagDefine az inc  shadowsT/F flareT/F  R G B  [ambR ambG ambB]");
      return false;
   }
   int tag = atoi(argv[0]);
   if (!tag) 
      tag = 0;

   bool shadows = false;
   if (argv[3][0] == 't' || argv[3][0] == 'T' || argv[3][0] == '1')
      shadows = true;

   bool flare = true;
   if (argv[4][0] == 'f' || argv[4][0] == 'F' || argv[4][0] == '0')
      flare = false;

   ColorF intensity;
   intensity.red = atof(argv[5]);
   intensity.green = atof(argv[6]);
   intensity.blue = atof(argv[7]);
   ColorF ambient(0.0f, 0.0f, 0.0f);
   if (argc == 12) {
      ambient.red = atof(argv[8]);
      ambient.green = atof(argv[9]);
      ambient.blue = atof(argv[10]);
   }         



   set(tag, atof(argv[1]), atof(argv[2]), shadows, flare,
      intensity, ambient);
   return true;
}

//--------------------------------------------------------------------------- 

bool SimPlanet::processQuery(SimQuery *query)
{
   switch (query->type) {
      case SimRenderQueryImageType: {
         SimRenderQueryImage *q = static_cast<SimRenderQueryImage *>(query);
         // planet needs the terrain's visible distance 
         SimTerrain *terrain = static_cast<SimTerrain *>
            (manager->findObject(SimTerrainId));
         if (terrain)
            planet.visibleDistance = terrain->getVisibleDistance();
         else
            planet.visibleDistance = 10000.0f;
         q->count = 0;
         if (textureTag != 0)
            q->image[q->count++] = &planet;
         if (useLensFlare) {
            if (!lensFlare.isInitialized())
               lensFlare.initialize(manager);
            lensFlare.setRootContainer(static_cast<SimContainer *>(manager->findObject(SimRootContainerId)));
            q->image[q->count++] = &lensFlare;
         }
         return (q->count > 0);
      }

		case SimLightQueryType: {
			SimLightQuery *qp = static_cast<SimLightQuery *>(query);
			qp->count = 1;
         qp->ambientIntensity = ambient;
			qp->light[0] = &light;
			return true;
		}
	
      default:
         return false;
   }
}

//--------------------------------------------------------------------------- 

bool SimPlanet::onAdd()
{
	if (!Parent::onAdd())
		return false;
	load();
	return true;
}


//--------------------------------------------------------------------------- 

void SimPlanet::onRemove()
{
	unload();
	Parent::onRemove();
}

//------------------------------------------------------------------------------
void SimPlanet::onPreLoad(SimPreLoadManager *splm)
{
   Parent::onPreLoad(splm);

   if (LensFlare::getMaterialListName())
      splm->preLoadMaterialList(LensFlare::getMaterialListName());
}

//--------------------------------------------------------------------------- 

DWORD SimPlanet::packUpdate(Net::GhostManager *, DWORD, BitStream *stream)
{
   streamWrite(*stream);
   stream->write(planet.size);
   stream->write(planet.distance);
   return 0;
}

//--------------------------------------------------------------------------- 

void SimPlanet::unpackUpdate(Net::GhostManager *, BitStream *stream)
{
   streamRead(*stream);
   stream->read(&planet.size);
   stream->read(&planet.distance);
   if (manager)
      load();
}

//--------------------------------------------------------------------------- 

void SimPlanet::initPersistFields()
{
   //Parent::initPersistFields();
   addField("textureTag", TypeInt, Offset(textureTag,SimPlanet));
   addField("azimuth", TypeFloat, Offset(azimuth,SimPlanet));
   addField("incidence", TypeFloat, Offset(incidence, SimPlanet));
   addField("castShadows", TypeBool, Offset(castShadows, SimPlanet));
   addField("useLensFlare", TypeBool, Offset(useLensFlare, SimPlanet));
   addField("intensity", TypePoint3F, Offset(intensity, SimPlanet));
   addField("ambient", TypePoint3F, Offset(ambient, SimPlanet));
}

//--------------------------------------------------------------------------- 

void SimPlanet::streamWrite(StreamIO &sio)
{
   sio.write(sizeof(textureTag), (Int8 *)&textureTag);
   sio.write(sizeof(azimuth), (Int8 *)&azimuth);
   sio.write(sizeof(incidence), (Int8 *)&incidence);
   sio.write(sizeof(castShadows), (Int8 *)&castShadows);
   sio.write(sizeof(useLensFlare), (Int8 *)&useLensFlare);
   sio.write(sizeof(intensity), (Int8 *)&intensity);
   sio.write(sizeof(ambient), (Int8 *)&ambient);
}

//--------------------------------------------------------------------------- 

void SimPlanet::streamRead(StreamIO &sio)
{
   sio.read(sizeof(textureTag), (Int8 *)&textureTag);
   sio.read(sizeof(azimuth), (Int8 *)&azimuth);
   sio.read(sizeof(incidence), (Int8 *)&incidence);
   sio.read(sizeof(castShadows), (Int8 *)&castShadows);
   sio.read(sizeof(useLensFlare), (Int8 *)&useLensFlare);
   sio.read(sizeof(intensity), (Int8 *)&intensity);
   sio.read(sizeof(ambient), (Int8 *)&ambient);
}

//--------------------------------------------------------------------------- 

Persistent::Base::Error SimPlanet::write(StreamIO &sio, int a, int b)
{
   Persistent::Base::Error error = SimNetObject::write(sio, a, b);
   if (error != Persistent::Base::Ok) 
      return error;
   streamWrite(sio);
   sio.write(planet.size);
   sio.write(planet.distance);
	return (sio.getStatus() == STRM_OK) ? Ok : WriteError;
}

//--------------------------------------------------------------------------- 

Persistent::Base::Error SimPlanet::read(StreamIO &sio, int version, int b)
{
   Persistent::Base::Error error = SimNetObject::read(sio, version, b);
   if (error != Persistent::Base::Ok) 
      return error;
   streamRead(sio);
   if (version == CurrentVersion)
   {
      sio.read(&planet.size);
      sio.read(&planet.distance);
   }
	return (sio.getStatus() == STRM_OK) ? Ok : ReadError;
}

//--------------------------------------------------------------------------- 

//--------------------------------------------------------------------------- 

void PlanetRenderImage::render(TSRenderContext &rc)
{
   // A simple planet culling scheme would be to dot the line of sight
   // with the vector from the camera to the planet.  This would eliminate
   // the length test of v below (after m_cross((Point3F)plane, vpNormal, &v))
   GFXSurface *gfxSurface = rc.getSurface();

   gfxSurface->setHazeSource(GFX_HAZE_NONE);
   gfxSurface->setShadeSource(GFX_SHADE_CONSTANT);
   gfxSurface->setAlphaSource(GFX_ALPHA_NONE);
	gfxSurface->setFillMode(GFX_FILL_TEXTURE);
	gfxSurface->setTransparency(FALSE);
   gfxSurface->setTexturePerspective(FALSE);

   gfxSurface->setConstantShade(1.0f);

   int textureHeight;
   gfxSurface->setTextureMap(texture);
   textureHeight = texture->height;
   
   TSCamera *camera = rc.getCamera();
  
   TS::PointArray *pointArray = rc.getPointArray();
   pointArray->reset();
   pointArray->useIntensities(false);
   pointArray->useTextures(textCoord);
   pointArray->useTextures(true);
	pointArray->setVisibility( TS::ClipMask );

   // find out how high the bitmap is at 100% as projected onto the viewport,
   // texel:pixel will be 1:1 at 640x480
   //const RectF &worldVP  = camera->getWorldViewport();
   //const float h = textureHeight*((worldVP.upperL.y - worldVP.lowerR.y)/480.0f);
   //const float sz = 0.5*distance*(h/camera->getNearDist());

   // find the position of the planet
   Point3F displacement = camera->getTCW().p;
   //displacement.z *= -(distance - visibleDistance)/visibleDistance;
   displacement.z = -displacement.z*(distance/(visibleDistance*1.5f));
   Point3F pos = position;
   pos += displacement;
 
   // find the normal to the view plane in world coords
   Point3F v0(0.0f, 1.0f, 0.0f), vpNormal;
   m_mul(v0, (RMat3F)camera->getTCW(), &vpNormal);
   vpNormal.normalize();

   // construct the plane that the camera, planet pos & celestial NP all
   // lie on
   PlaneF plane(pos, camera->getTCW().p, 
      Point3F(displacement.x, displacement.y, displacement.z + distance));

   // the cross product of the VP normal and the normal to the plane just
   // constructed is the up vector for the planet
   Point3F v;
   m_cross((Point3F)plane, vpNormal, &v);
   if (IsEqual(v.len(), 0.0f))
      // planet is directly to the right or left of camera
      return;
   v.normalize();
   
   // cross the up with the normal and we get the right vector
   Point3F u;
   m_cross(vpNormal, v, &u);
   u *= size;
   v *= size;

   TS::VertexIndexPair V[6];
   Point3F ul = pos;
   ul -= u; ul += v;
   V[0].fVertexIndex   = pointArray->addPoint(ul);
   V[0].fTextureIndex  = 0;
   Point3F ur = pos;
   ur += u; ur += v;
   V[1].fVertexIndex   = pointArray->addPoint(ur);
   V[1].fTextureIndex  = 1;
   Point3F lr = pos;
   lr += u; lr -= v;
   V[2].fVertexIndex   = pointArray->addPoint(lr);
   V[2].fTextureIndex  = 2;
   Point3F ll = pos;
   ll -= u; ll -=v;
   V[3].fVertexIndex   = pointArray->addPoint(ll);
   V[3].fTextureIndex  = 3;
	if (gfxSurface->getCaps() & GFX_DEVCAP_SUPPORTS_CONST_ALPHA)
	   gfxSurface->setZTest(GFX_NO_ZTEST);
   pointArray->drawPoly(4, V, 0);
	if (gfxSurface->getCaps() & GFX_DEVCAP_SUPPORTS_CONST_ALPHA)
	   gfxSurface->setZTest(GFX_ZTEST_AND_WRITE);
   if(lensFlare) {
      TS::TransformedVertex vx;
      camera->transformProject(pos, &vx);
      bool vis = vx.fStatus & TS::TransformedVertex::Projected;
      lensFlare->setSunPos(vis, vx.fPoint, pos);
   }
}

