//--------------------------------------------------------------------------- 
//
// simLight.cpp
//
//--------------------------------------------------------------------------- 


#include <sim.h>
#pragma warn -inl
#pragma warn -aus

#include "simlightgrp.h"
#include "simLight.h"
#include "simPersistTags.h"
#include "darkstar.strings.h"
#include "editor.strings.h"
#include "console.h"
#include "simresource.h"
#include "ts_shapeinst.h"

//--------------------------------------------------------------------------- 

const char * shapeName = "pyrm2.dts";

bool SimLight::editMode = false;

SimLight::SimLight(const ColorF&  intensity,
                              const Point3F& direction,
                              const ColorF&  ambient)
{
   // basic directional light
   setType(TS::Light::LightDirectional);
   setDirection(direction);
   setIntensity(intensity);
   setAmbientIntensity(ambient);
   image.shape = 0;
	type = SimMissionObjectType;
   
   netFlags.set(ScopeAlways);
   netFlags.set(Ghostable);
}

SimLight::SimLight(float          range,
                              const ColorF&  intensity,
                              const Point3F& position,
                              const ColorF&  ambient)
{
   // basic point light
   setType(TS::Light::LightPoint);
   setPosition(position);
   setIntensity(intensity);
   setAmbientIntensity(ambient);
   setRange(range);
   image.shape = 0;
	type = SimMissionObjectType;

   netFlags.set(ScopeAlways);
   netFlags.set(Ghostable);
}

SimLight::SimLight()
{
   image.shape = 0;
	type = SimMissionObjectType;
   
   netFlags.set(ScopeAlways);
   netFlags.set(Ghostable);
}

SimLight::~SimLight()
{
   delete image.shape;
}

//--------------------------------------------------------------------------- 

bool SimLight::processArguments(int argc, const char **argv)
{
   if(argc < 1)
   {
      Console->printf("SimLight: type(Directional/Point) ...");
      return false;
   }
   if(!strncmp(argv[0], "Directional", sizeof("Directional") - 1))
   {
      if(argc == 7 || argc == 10)
      {
         if(!strcmp(argv[0], "DirectionalWrap"))
            setType(TS::Light::LightDirectionalWrap);
         else
            setType(TS::Light::LightDirectional);

         ColorF intensity(atof(argv[1]), atof(argv[2]), atof(argv[3]));
         Point3F dir(atof(argv[4]), atof(argv[5]), atof(argv[6]));
         dir.normalizef(1.0f);
         ColorF ambient;
         if (argc == 10)
            ambient.set(atof(argv[7]), atof(argv[8]), atof(argv[9]));
         else
            ambient.set();
         setDirection(dir);
         setIntensity(intensity);
         setAmbientIntensity(ambient);
         return true;
      }
      Console->printf("SimLight: Directional intR intG intB dirX dirY dirz [ambR ambG ambB]");
   }
   else if(!strcmp(argv[0], "Point"))
   {
      if (argc == 8 || argc == 11)
      {
         float range = atof(argv[1]);
         ColorF intensity(atof(argv[2]), atof(argv[3]), atof(argv[4]));
         Point3F pos(atof(argv[5]), atof(argv[6]), atof(argv[7]));
         ColorF ambient;
         if (argc == 11)
            ambient.set(atof(argv[8]), atof(argv[9]), atof(argv[10]));
         else
            ambient.set();

         setType(TS::Light::LightPoint);
         setPosition(pos);
         setIntensity(intensity);
         setAmbientIntensity(ambient);
         setRange(range);
         return true;
      }
      Console->printf("SimLight: Point range intR intG intB posX posY posZ [ambR ambG ambB]");
   }
   Console->printf("SimLight: type(Directional/Point) ...");
   return false;
}

//--------------------------------------------------------------------------- 

bool SimLight::onAdd()
{
   if(loadShape() && isGhost())
   {
	   // Altimor: 3rd param used to be root
      SimContainer * root = findObject(manager, SimRootContainerId, (SimContainer*)NULL);
      root->addObject(this);
   }
   
	if (!Parent::onAdd())
		return false;
	addToSet(SimLightSetId);
   setType((TS::Light::LightType)light.fLight.fType);
   
	return true;
}	

//--------------------------------------------------------------------------- 

bool SimLight::loadShape()
{
	if(image.shape)
		return(true);

   ResourceManager * rm = SimResource::get(manager);
	shape = rm->load(shapeName,true);
   
	if(shape.operator bool())
   {
		TS::ShapeInstance * si = new TS::ShapeInstance(shape, *rm);
		image.shape = si;
		image.itype = SimRenderImage::Normal;

		const TSShape& shape = si->getShape();
      shapeBox = shape.fBounds;
      updateBoundingBox();
		collisionImage.box = shapeBox;
      collisionImage.trigger = false;
		return(true);
	}
	return(false);
}

//--------------------------------------------------------------------------- 

bool SimLight::processEvent(const SimEvent* event)
{
   switch (event->type)
   {
      onEvent(SimTriggerEvent);
      onEvent(SimObjectTransformEvent);
      onEvent(SimEditEvent);
   }
   return Parent::processEvent(event);
}

//--------------------------------------------------------------------------- 

bool SimLight::onSimTriggerEvent(const SimTriggerEvent* event)
{
   switch(event->action)
   {
      case SimTriggerEvent::Activate:
         setIntensity(event->value);
         break;

      default:
         return false;
   }

   return true;
}  

//--------------------------------------------------------------------------- 

bool SimLight::onSimObjectTransformEvent(const SimObjectTransformEvent * event)
{
	setTransform(event->tmat);
   updateBoundingBox();
	setMaskBits(PositionMask);
   return(true);
}

//--------------------------------------------------------------------------- 

bool SimLight::onSimEditEvent(const SimEditEvent * event)
{
   if(event->editorId == SimMissionEditorId)
   {
      if(event->state)
      {
         editMode = true;
	 // Altimor: 3rd param used to be root
         SimContainer * root = findObject(manager, SimRootContainerId, (SimContainer*)NULL);
         root->addObject(this);
      }
      else
      {
         editMode = false;
         if(getContainer())
            getContainer()->removeObject(this);
      }
   }
   return(true);
}

//--------------------------------------------------------------------------- 
void SimLight::inspectRead(Inspect *inspector)
{
	Parent::inspectRead(inspector);

   float   range;
   Point3F tempPt;
   ColorF  tempColor;
   Bool    tempBool;

   inspector->read(IDITG_SIMLT_STATICLIGHT, light.fLight.fStaticLight);
   setMaskBits(StaticMask);

   inspector->read(IDITG_SIMLT_LOCATION, tempPt);
   setPosition(tempPt);
   updateBoundingBox();

   inspector->read(IDITG_SIMLT_DIRECTION, tempPt);
   setDirection(tempPt);

   inspector->read(IDITG_SIMLT_RANGE, range);
   setRange(range);
   
   inspector->read(IDITG_SIMLT_INTENSITY, tempPt);
   tempColor.red   = tempPt.x;
   tempColor.green = tempPt.y;
   tempColor.blue  = tempPt.z;
   setIntensity(tempColor);

   inspector->read(IDITG_SIMLT_AMBIENTINTENSITY, tempPt);
   tempColor.red   = tempPt.x;
   tempColor.green = tempPt.y;
   tempColor.blue  = tempPt.z;
   setAmbientIntensity(tempColor);

   Int32 currType;
   inspector->read(IDITG_SIMLT_TYPE, &tempBool, "IDITG_SIMLT_TYPE_*", currType);
	switch (currType) {
		case IDITG_SIMLT_TYPE_POINT:
	      setType(TS::Light::LightPoint);
			break;
		case IDITG_SIMLT_TYPE_DIRECTIONAL:
	      setType(TS::Light::LightDirectional);
			break;
		case IDITG_SIMLT_TYPE_WRAP:
	      setType(TS::Light::LightDirectionalWrap);
			break;
		default:
	      AssertFatal(0, "Unknown light type");
   }
}

void SimLight::inspectWrite(Inspect *inspector)
{
	Parent::inspectWrite(inspector);

   inspector->write(IDITG_SIMLT_STATICLIGHT, light.fLight.fStaticLight);

   inspector->write(IDITG_SIMLT_LOCATION,  light.fLight.fWPosition);
   inspector->write(IDITG_SIMLT_DIRECTION, light.fLight.fWAim);
   inspector->write(IDITG_SIMLT_RANGE,     light.fLight.fRange);

   Point3F tempPt;
   tempPt.x = light.fLight.fRed;
   tempPt.y = light.fLight.fGreen;
   tempPt.z = light.fLight.fBlue;
   inspector->write(IDITG_SIMLT_INTENSITY, tempPt);
   tempPt.x = ambientIntensity.red;
   tempPt.y = ambientIntensity.green;
   tempPt.z = ambientIntensity.blue;
   inspector->write(IDITG_SIMLT_AMBIENTINTENSITY, tempPt);

   int currType;
   switch (light.fLight.fType) {
		case TS::Light::LightPoint:
			currType = IDITG_SIMLT_TYPE_POINT;
			break;
		case TS::Light::LightDirectional:
			currType = IDITG_SIMLT_TYPE_DIRECTIONAL;
			break;
		case TS::Light::LightDirectionalWrap:
			currType = IDITG_SIMLT_TYPE_WRAP;
			break;
   }
   inspector->write(IDITG_SIMLT_TYPE, true, "IDITG_SIMLT_TYPE_*", currType);
}


bool SimLight::processQuery(SimQuery* query)
{
   switch (query->type) {
      case SimLightQueryType: {
         SimLightQuery *qp = static_cast<SimLightQuery*>(query);
         qp->count = 1;
         qp->ambientIntensity = ambientIntensity;
         qp->light[0] = &light;
         return true;
      }
      onQuery(SimObjectTransformQuery);
      onQuery(SimCollisionImageQuery);
      onQuery(SimRenderQueryImage);
   }

   return Parent::processQuery(query);
}

//--------------------------------------------------------------------

TMat3F & SimLight::getTransform()
{
   static TMat3F mat;
   return(mat.set(EulerF(0.f,0.f,0.f), light.fLight.fWPosition));
}
      
//--------------------------------------------------------------------

void SimLight::setTransform(const TMat3F & mat)
{
   light.fLight.fWPosition = mat.p;
}

//--------------------------------------------------------------------

void SimLight::updateBoundingBox()
{
   Box3F bbox(shapeBox);
   bbox += &light.fLight.fWPosition,
   setBoundingBox(bbox);
}
      
//--------------------------------------------------------------------

bool SimLight::onSimObjectTransformQuery(SimObjectTransformQuery * query)
{
   query->tmat = getTransform();
   return(true);
}

//--------------------------------------------------------------------

bool SimLight::onSimCollisionImageQuery(SimCollisionImageQuery * query)
{
   if(!editMode || (light.fLight.fType != TS::Light::LightPoint))
      return(false);
	query->count = 1;
	query->image[0] = &collisionImage;
	collisionImage.transform = getTransform();
   return(true);
}

//--------------------------------------------------------------------

bool SimLight::onSimRenderQueryImage(SimRenderQueryImage * query)
{
   if(!editMode || (light.fLight.fType != TS::Light::LightPoint))
      return(false);
	if(image.shape) {
		query->count = 1;
		query->image[0] = &image;
		image.transform = getTransform();
		return(true);
	}
	return(false);
}

//--------------------------------------------------------------------

DWORD SimLight::packUpdate(Net::GhostManager* /*gm*/, DWORD mask, BitStream *stream)
{
   if (stream->writeFlag((mask & StaticMask) != 0) == true) {
      stream->writeFlag(light.fLight.fStaticLight);
   }

   if (mask & TypeMask) {
      stream->writeFlag(true);
      stream->writeInt(int(light.fLight.fType), 3);
   } else {
      stream->writeFlag(false);
   }

   if (mask & RangeMask) {
      stream->writeFlag(true);
      stream->write(light.fLight.fRange);
   } else {
      stream->writeFlag(false);
   }

   if (mask & IntensityMask) {
      stream->writeFlag(true);
      ColorF temp(light.fLight.fRed, light.fLight.fGreen, light.fLight.fBlue);
      stream->write(sizeof(temp), &temp);
   } else {
      stream->writeFlag(false);
   }

   if (mask & AmbientIntensityMask) {
      stream->writeFlag(true);
      stream->write(sizeof(ambientIntensity), &ambientIntensity);
   } else {
      stream->writeFlag(false);
   }

   if (mask & PositionMask) {
      stream->writeFlag(true);
      stream->write(sizeof(light.fLight.fWPosition), &light.fLight.fWPosition);
   } else {
      stream->writeFlag(false);
   }

   if (mask & DirectionMask) {
      stream->writeFlag(true);
      stream->write(sizeof(light.fLight.fWAim), &light.fLight.fWAim);
   } else {
      stream->writeFlag(false);
   }
   return 0;
}

void SimLight::unpackUpdate(Net::GhostManager* /*gm*/, BitStream *stream)
{
   if (stream->readFlag()) {
      light.fLight.fStaticLight = stream->readFlag();
   }

   if(stream->readFlag()) {
      setType(TS::Light::LightType(stream->readInt(3)));
   }
   if(stream->readFlag()) {
      float range;
      stream->read(&range);
      setRange(range);
   }
   if(stream->readFlag()) {
      ColorF newColor;
      stream->read(sizeof(newColor), &newColor); 
      setIntensity(newColor);
   }
   if(stream->readFlag()) {
      ColorF newColor;
      stream->read(sizeof(newColor), &newColor); 
      setAmbientIntensity(newColor);
   }
   if(stream->readFlag()) {
      Point3F newPos;
      stream->read(sizeof(newPos), &newPos);
      setPosition(newPos);
      updateBoundingBox();
   }
   if(stream->readFlag()) {
      Point3F newAim;
      stream->read(sizeof(newAim), &newAim);
      setDirection(newAim);
   }
}

int SimLight::sm_fileVersion = 2;

void SimLight::initPersistFields()
{
   addField("type", TypeInt, Offset(light.fLight.fType, SimLight));
   addField("range", TypeFloat, Offset(light.fLight.fRange, SimLight));
   addField("color", TypePoint3F, Offset(light.fLight.fRed, SimLight));
   addField("ambient", TypePoint3F, Offset(ambientIntensity, SimLight));
   addField("position", TypePoint3F, Offset(light.fLight.fWPosition, SimLight));
   addField("aim", TypePoint3F, Offset(light.fLight.fWAim, SimLight));
   addField("isStatic", TypeBool, Offset(light.fLight.fStaticLight, SimLight));
}

Persistent::Base::Error 
SimLight::write( StreamIO &sio, int v, int u)
{
   Parent::write(sio, v, u);

   sio.write(int(light.fLight.fType));
   sio.write(light.fLight.fRange);

   ColorF temp(light.fLight.fRed, light.fLight.fGreen, light.fLight.fBlue);
   sio.write(sizeof(temp), &temp);

   sio.write(sizeof(ambientIntensity), &ambientIntensity);
   sio.write(sizeof(light.fLight.fWPosition), &light.fLight.fWPosition);
   sio.write(sizeof(light.fLight.fWAim),      &light.fLight.fWAim);

   sio.write(light.fLight.fStaticLight);

   return ((sio.getStatus() == STRM_OK) ? Ok : WriteError);
}

Persistent::Base::Error 
SimLight::read( StreamIO &sio, int version, int u)
{
   AssertFatal(version == sm_fileVersion || version == 1, "Unknown file Version...");

   Parent::read(sio, version, u);

   int temp;
   sio.read(&temp);
   setType(TS::Light::LightType(temp));
   
   float range;
   sio.read(&range);
   setRange(range);
   
   ColorF tempC;
   sio.read(sizeof(tempC), &tempC);
   setIntensity(tempC);
   
   sio.read(sizeof(tempC), &tempC);
   setAmbientIntensity(tempC);
   
   Point3F tempP;
   sio.read(sizeof(tempP), &tempP);
   setPosition(tempP);
   
   sio.read(sizeof(tempP), &tempP);
   setDirection(tempP);

   if (version == sm_fileVersion)
      sio.read(&light.fLight.fStaticLight);

   return ((sio.getStatus() == STRM_OK) ? Ok : ReadError);
}

int 
SimLight::version()
{
   return sm_fileVersion;
}

