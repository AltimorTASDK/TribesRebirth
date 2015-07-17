//--------------------------------------------------------------------------- 
//
// simLight.h
//
//--------------------------------------------------------------------------- 

#ifndef _SIMLIGHT_H_
#define _SIMLIGHT_H_

#include <sim.h>
#include <simNetObject.h>
#include <simcontainer.h>
#include <simrendergrp.h>
#include <move.h>
#include <inspect.h>
#include <ts_shape.h>

//--------------------------------------------------------------------------- 

class SimLight : public SimContainer
{
   typedef SimContainer Parent;
   enum NetMaskBits {
      TypeMask             = 1 << 0,
      RangeMask            = 1 << 1,
      IntensityMask        = 1 << 2,
      AmbientIntensityMask = 1 << 3,
      PositionMask         = 1 << 4,
      DirectionMask        = 1 << 5,
      StaticMask           = 1 << 6
   };
   static int sm_fileVersion;

 protected:
   ColorF  ambientIntensity;
   TSLight light;
   
   SimRenderDefaultImage image;
	Resource<TSShape> shape;
	SimCollisionBoxImage collisionImage;
   Box3F shapeBox;
   static bool editMode;
   
 public:
   void setType(TS::Light::LightType type);
   void setRange(float range);
   void setIntensity(float i);
   void setIntensity(const ColorF &);
   void setAmbientIntensity(const ColorF &ambient);
   void setPosition(const Point3F &position);
   void setDirection(Point3F direction);

   SimLight();
   SimLight(const ColorF &intensity, const Point3F &direction, const ColorF &ambient);
   SimLight(float range, const ColorF &intensity, const Point3F &position, const ColorF &ambient);
   ~SimLight();
   
   bool loadShape();
   
   bool onSimTriggerEvent(const SimTriggerEvent* event);
   bool onSimObjectTransformEvent(const SimObjectTransformEvent * event);
   bool onSimEditEvent(const SimEditEvent * event);

   TMat3F& getTransform();
   void setTransform(const TMat3F & mat);
   void updateBoundingBox();
   
   bool onSimObjectTransformQuery(SimObjectTransformQuery * query);
   bool onSimCollisionImageQuery(SimCollisionImageQuery * query);
   bool onSimRenderQueryImage(SimRenderQueryImage * query);

   TSLight& getTSLight() { return( light ); }
   bool processEvent(const SimEvent*);
   bool processQuery(SimQuery*);
   bool processArguments(int argc, const char **argv);
	bool onAdd();
	void inspectRead(Inspect *inspector);
	void inspectWrite(Inspect *inspector);

   DECLARE_PERSISTENT( SimLight );

   DWORD packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream);
   void unpackUpdate(Net::GhostManager *gm, BitStream *stream);

   static void initPersistFields();
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
   int version();
};

//--------------------------------------------------------------------------- 

inline void SimLight::setType(TS::Light::LightType type)
{
   light.setType(type);
   setMaskBits(TypeMask);
}

inline void SimLight::setRange(float range)
{
   light.setRange(range);
   setMaskBits(RangeMask);
}

inline void SimLight::setIntensity(float i)
{
   light.setIntensity(i, i, i);
   setMaskBits(IntensityMask);
}

inline void SimLight::setIntensity(const ColorF &intensity)
{
   light.setIntensity(intensity.red, intensity.green, intensity.blue);
   setMaskBits(IntensityMask);
}

inline void SimLight::setAmbientIntensity(const ColorF &ambient)
{
   ambientIntensity = ambient;
   setMaskBits(AmbientIntensityMask);
}

inline void SimLight::setPosition(const Point3F &position)
{
   light.setPosition(position);
   setMaskBits(PositionMask);
}

inline void SimLight::setDirection(Point3F direction)
{
   direction.normalize();
   light.setAim(direction);
   setMaskBits(DirectionMask);
}

//--------------------------------------------------------------------------- 

#endif
