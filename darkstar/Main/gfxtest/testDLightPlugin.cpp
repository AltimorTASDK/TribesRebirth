//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "testDLightPlugin.h"

#include <shellapi.h>


class LinearDynamicLight : public SimDynamicLight {
   typedef SimDynamicLight Parent;

   const float m_delay;

   Point3F m_velocity;
   float   m_currentTime;
   Int32   m_numStepsRemaining;
   
  public:
   LinearDynamicLight(const Point3F& in_position,
                      const float    in_radius,
                      const Point3F& in_velocity,
                      const TMat3F&  in_transform,
                      const float    in_delay,
                      const Int32    in_numSteps);

   void removeLight();

   // SimEvent and Query handling...
   //
   bool COREAPI processEvent(const SimEvent* event);
   bool COREAPI onSimTimerEvent(const SimTimerEvent* event);
   bool COREAPI onSimAddNotifyEvent(const SimAddNotifyEvent* event);
};



void COREAPI 
TestDLightPlugin::init()
{
	console->printf("DynamicLight testPlugin");
	console->addCommand(NewLinearLight, "newLinearLight", this);
	console->addCommand(LinearLightFromObject, "linearLightFromObject", this);
}


int COREAPI 
TestDLightPlugin::consoleCallback(CMDConsole* /**/,
                                  int         id,
                                  int         argc,
                                  const char* argv[])
{
   switch(id) {
     case NewLinearLight: {
      if (argc == 10) {
         Point3F position, velocity;
         float   delay, radius;
         Int32   numSteps;
         
         sscanf(argv[1], "%f", &delay);
         sscanf(argv[2], "%d", &numSteps);
         sscanf(argv[3], "%f", &radius);
         
         sscanf(argv[4], "%f", &position.x);
         sscanf(argv[5], "%f", &position.y);
         sscanf(argv[6], "%f", &position.z);
         
         sscanf(argv[7], "%f", &velocity.x);
         sscanf(argv[8], "%f", &velocity.y);
         sscanf(argv[9], "%f", &velocity.z);

         TMat3F transform;
         transform.identity();
         LinearDynamicLight* newLight = new LinearDynamicLight(position, radius,
                                                               velocity,
                                                               transform,
                                                               delay, numSteps);
         manager->addObject(newLight);
         manager->assignName(newLight, "DynamicLight (Linear)");
      } else {
         console->printf("newLinearLight: <delay> <steps> <radius> <x, y, z> <vx, vy, vz>");
         return 1;
      }
      break;
     }
     case LinearLightFromObject: {
      if (argc == 6) {
         float delay;
         Int32 numSteps;
         float speed, radius;
         
         sscanf(argv[2], "%f", &delay);
         sscanf(argv[3], "%d", &numSteps);
         sscanf(argv[4], "%f", &radius);
         sscanf(argv[5], "%f", &speed);
         
         SimObject* pObject = manager->findObject(argv[1]);
         if (pObject == NULL) {
            console->printf("Could not find object: %s", argv[1]);
            return 1;
         }
         
         SimObjectTransformQuery oQuery;
         if (pObject->processQuery(&oQuery) == false) {
            console->printf("%s did not respond to SimImageTransformQuery",
                            argv[1]);
            return 1;
         }
         
         Point3F position, velocity, initVel(0.0, 1.0, 0.0);
         position = oQuery.tmat.p;
         oQuery.tmat.p = Point3F(0, 0, 0);

         m_mul(initVel, oQuery.tmat, &velocity);
         velocity *= (speed * delay);
         
         TMat3F transform;
         transform.identity();
         LinearDynamicLight* newLight = new LinearDynamicLight(position, radius,
                                                               velocity,
                                                               transform,
                                                               delay, numSteps);
         manager->addObject(newLight);
         manager->assignName(newLight, "DynamicLight (LinearFromObj)");
      } else {
         console->printf("linearLightFromObject: <objectName> <delay> <steps> <radius> <speed>");
         return 1;
      }
      break;
     }
   }
   return 0;
}


LinearDynamicLight::LinearDynamicLight(const Point3F& in_position,
                                       const float    in_radius,
                                       const Point3F& in_velocity,
                                       const TMat3F&  in_transform,
                                       const float    in_delay,
                                       const Int32    in_numSteps)
 : SimDynamicLight(in_position, in_transform, in_radius),
   m_delay(in_delay),
   m_velocity(in_velocity),
   m_currentTime(0.0f),
   m_numStepsRemaining(in_numSteps)
{
   // NULL
}


bool COREAPI 
LinearDynamicLight::processEvent(const SimEvent* event)
{
   switch (event->type) {
      onEvent(SimTimerEvent);
      onEvent(SimAddNotifyEvent);
   }
   
   return Parent::processEvent(event);
}


bool COREAPI 
LinearDynamicLight::onSimTimerEvent(const SimTimerEvent* event)
{
   if (m_numStepsRemaining != 0) {
      int stepMul = 0;
      m_currentTime += event->timerInterval;
   
      while (m_currentTime > m_delay &&
             m_numStepsRemaining != 0) {
         m_currentTime -= m_delay;
         stepMul++;
         m_numStepsRemaining--;
      }
   
      // Step the position by calling base class function if necessary
      //
      if (stepMul != 0) {
         Point3F newPosition = m_velocity;
         newPosition *= float(stepMul);
         newPosition += getPosition();
         setPosition(newPosition);
      }
   } else {
      // We're done...
      //
      deleteObject();
   }
   
   return true;
}

bool COREAPI 
LinearDynamicLight::onSimAddNotifyEvent(const SimAddNotifyEvent* event)
{
   // Hook into timer group...
   //
	bool timerSuccess = addToSet(SimTimerSetId);
   AssertFatal(timerSuccess == true, "Could not add to SimTimerGroup");

   return Parent::onSimAddNotifyEvent(event);
}
