//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <simExplosion.h>
#include "simShape.h"
#include "simShapeGroupRep.h"
#include "simDamageEv.h"
#include "editor.strings.h"
#include "commonEditor.strings.h"

namespace {

Point3F
translateToInspectRotation(const Point3F& in_rRot)
{
   Point3F retVal;

   retVal.x = (float)floor((double(in_rRot.x) * 180.0 / M_PI) + 0.5);
   retVal.y = (float)floor((double(in_rRot.y) * 180.0 / M_PI) + 0.5);
   retVal.z = (float)floor((double(in_rRot.z) * 180.0 / M_PI) + 0.5);

   return retVal;
}

EulerF
translateFromInspectRotation(const Point3F in_rRot)
{
   EulerF retVal;

   retVal.x = float(double(in_rRot.x) * M_PI / 180.0);
   retVal.y = float(double(in_rRot.y) * M_PI / 180.0);
   retVal.z = float(double(in_rRot.z) * M_PI / 180.0);

   return retVal;
}

} // namespace {}

UInt32 SimShape::sm_frameKey      = 0;

int    SimShape::sm_fileVersion   = 2;
bool   SimShape::sm_drawShapeBBox = false;

SimShape::SimShape()
 : m_pDamageParent(NULL),
   m_damageDistribution(ToNone),
   m_destructionEvent(DoNothing),
   m_sustainableDamage(1.0f),
   m_currentDamage(0.0f),
   m_damageForwardAttenuation(0.10f),
   m_destructionDamage(0.0f),
   m_explosionTag(0),
   m_shapeFlags(SFIndestructible)
{
   type |= SimShapeObjectType;

   set(EulerF(0.0f, 0.0f, 0.0f), Point3F(0.0f, 0.0f, 0.0f));

   netFlags.set(ScopeAlways);
   netFlags.set(Ghostable);
}

SimShape::~SimShape()
{
   
}


bool
SimShape::getDrawShapeBBox()
{
   return sm_drawShapeBBox;
}

void
SimShape::setDrawShapeBBox(const bool in_drawBox)
{
   sm_drawShapeBBox = in_drawBox;
}


//-------------------------------------- Damage Parameter control.  Separated
//                                        out in case we have to ghost these
//                                        values eventually
//
void
SimShape::setDamageParent(SimShapeGroupRep* io_pParent)
{
   m_pDamageParent = io_pParent;
}

void
SimShape::clearDamageParent()
{
   m_pDamageParent = NULL;
}

SimShapeGroupRep*
SimShape::getDamageParent() const
{
   return m_pDamageParent;
}

void
SimShape::acceptForwardedDamage(const double in_damage)
{
   // Are we already blown up?
   //
   if (testShapeFlag(SFIndestructible) == true ||
       testShapeFlag(SFIsDestroyed)    == true)
      return;
      
   // Just subtract from the current damage, and call the destruction routine if we are 
   //  blown up.  DO NOT forward any of this damage...
   //
   m_currentDamage += in_damage;
   
   if (m_currentDamage >= m_sustainableDamage)
      destroySelf();
}

void
SimShape::spawnExplosion()
{
   if (m_explosionTag != 0) {
      SimExplosion* explosion = new SimExplosion(m_explosionTag);
      explosion->setPosition(getLinearPosition());
      explosion->setAxis(Point3F(0, 0, 1));

      SimExplosion::createOnClients(explosion, manager);
   }
}

void
SimShape::destroySelf()
{
   AssertFatal(m_currentDamage >= m_sustainableDamage, "Not yet destroyed?");
   
   setShapeFlag(SFIsDestroyed);

   // Apply damage event to parent if necessary
   //
   if (m_pDamageParent != NULL) {
      switch (m_destructionEvent) {
        case ApplyMaxDamage:
         m_pDamageParent->damageForward(this, 100000.0f);
         break;
        case ApplySpecifiedDamage:
         m_pDamageParent->damageForward(this, m_destructionDamage);
         break;
      }
   }

   // Do any explosion/debris spawning here.
   //
   spawnExplosion();
   
   shapeFlagsChanged();
   setMaskBits(FlagsMask);
}

void
SimShape::setDamageDistribution(const DamageDistribution in_dist)
{
   m_damageDistribution = in_dist;
   
}

void
SimShape::setDamageForwardAttenuation(const float in_atten)
{
   if (in_atten < 0.0f) {
      AssertWarn(false, "Attenuation must be >= 0");
      return;
   }

   m_damageForwardAttenuation = in_atten;
}

void
SimShape::setDestructionEvent(const DestructionEvent in_event)
{
   m_destructionEvent = in_event;
}

void
SimShape::setDestructionDamage(const float in_damage)
{
   if (in_damage < 0.0f) {
      AssertWarn(false, "Damage must be >= 0");
      return;
   }

   m_destructionDamage = in_damage;
}

void
SimShape::setSustainableDamage(const float in_sust)
{
   if (in_sust < 0) {
      AssertWarn(false, "Sustainable Damage must be > 0");
      return;
   }

   m_sustainableDamage = in_sust;
}

void
SimShape::setCurrentDamage(const float in_curr)
{
   if (in_curr < 0) {
      AssertWarn(false, "Current Damage must be >= 0");
      return;
   }

   m_currentDamage = in_curr;
}


//-------------------------------------- Shape flag managers.  Note that this
//                                        state _is_ ghosted to the client
//
void
SimShape::setShapeFlag(const UInt32 in_flag)
{
   if (in_flag != 0) {
      m_shapeFlags |= in_flag;
      
      shapeFlagsChanged();
      setMaskBits(FlagsMask);
   }
}

void
SimShape::setAllShapeFlags(const UInt32 in_flags)
{
   m_shapeFlags = in_flags;

   shapeFlagsChanged();
   setMaskBits(FlagsMask);
}

void
SimShape::clearShapeFlag(const UInt32 in_flag)
{
   if (in_flag != 0) {
      m_shapeFlags &= ~(in_flag);
      
      shapeFlagsChanged();
      setMaskBits(FlagsMask);
   }
}

void
SimShape::shapeFlagsChanged()
{
   // Derived classes may override this function to hook flag change events,
   //  ie an object changing from untargetable to targetable...
   //
}

void
SimShape::set(const TMat3F& in_trans,
              const bool    in_reextractAngles)
{
   if (in_reextractAngles == true) {
      m_shapePosition = in_trans.p;
      in_trans.angles(&m_shapeRotation);

      TMat3F newTrans(m_shapeRotation, m_shapePosition);

      setPosition(newTrans, true);
   } else {
      setPosition(in_trans, true);
   }
   
   setMaskBits(PosRotMask);
}

void
SimShape::set(const EulerF& in_rot, const Point3F& in_pos)
{
   m_shapeRotation = in_rot;
   m_shapePosition = in_pos;

   set(TMat3F(in_rot, in_pos), false);
}


//-------------------------------------- Event/Query Processing
//
bool
SimShape::processEvent(const SimEvent* event)
{
   switch (event->type) {
      onEvent(SimDamageEvent);
      onEvent(SimMessageEvent);
      onEvent(SimObjectTransformEvent);

     default:
      return Parent::processEvent(event);
   }
}

bool
SimShape::processQuery(SimQuery* query)
{
   switch (query->type) {
      onQuery(SimObjectTransformQuery);

     default:
      return Parent::processQuery(query);
   }
}

//-------------------------------------- Event/Query Handlers
//
bool 
SimShape::onSimDamageEvent(const SimDamageEvent *event)
{           
   AssertFatal(isGhost() == false, "How did we get here?");

   // If we are already nuked, or we are indestructible, forget it...
   //
   if (testShapeFlag(SFIndestructible) == true ||
       testShapeFlag(SFIsDestroyed)    == true)
      return true;
   
   float keepDamage, forwardDamage;
   switch (m_damageDistribution) {
     case ToNone:
      keepDamage    = event->damageValue;
      forwardDamage = 0.0f;
      break;

     case ToParentAttenuated:
      forwardDamage = event->damageValue * m_damageForwardAttenuation;
      if (testShapeFlag(SFLocallyConserveDamage) == true)
         keepDamage = event->damageValue - forwardDamage;
      else
         keepDamage = event->damageValue;
      break;

     case ToParentFull:
      if (testShapeFlag(SFLocallyConserveDamage) == true)
         keepDamage = 0.0f;
      else
         keepDamage = event->damageValue;
      forwardDamage = event->damageValue;
      break;
      
     default:
      AssertFatal(0, "huh?");
   };
   
   // We can use the accept function to apply damage to ourself...
   //
   acceptForwardedDamage(keepDamage);

   if (forwardDamage   != 0.0f &&
       m_pDamageParent != NULL) {
      m_pDamageParent->damageForward(this, forwardDamage);
   }
   
   return true;
}

bool
SimShape::onSimMessageEvent(const SimMessageEvent* event)
{
   switch (event->message) {
     case TestSpawnExplosion:
      spawnExplosion();
      break;

     default:
      return Parent::processEvent(event);
   }

   return true;
}

bool 
SimShape::onSimObjectTransformEvent(const SimObjectTransformEvent *event)
{           
   set(event->tmat, true);
   return true;
}

bool
SimShape::onSimObjectTransformQuery(SimObjectTransformQuery *query)
{
   query->centerOffset.set();
   query->tmat = getTransform();

   return true;   
}

void SimShape::inspectWrite(Inspect *sd)
{
   Parent::inspectWrite(sd);

   //-------------------------------------- Damage Forwarding
   sd->write(IDITG_SSP_LOCALLYCONSERVEDAMAGE, testShapeFlag(SFLocallyConserveDamage));

   sd->write(IDITG_SSP_DAMAGEDISTRIBUTION, false,
           IDSIMOBJ_SSP_DAMAGEDIST_BEGIN, IDSIMOBJ_SSP_DAMAGEDIST_END,
           SimTag(m_damageDistribution));
   sd->write(IDITG_SSP_DAMAGEFORWARDATTEN, m_damageForwardAttenuation);
   
   //-------------------------------------- Destruction Event
   sd->write(IDITG_SSP_DESTRUCTEVENT, false,
           IDSIMOBJ_SSP_DESTRUCTEV_BEGIN, IDSIMOBJ_SSP_DESTRUCTEV_END,
           SimTag(m_destructionEvent));
   sd->write(IDITG_SSP_DESTRUCTIONDAMAGE, m_destructionDamage);
   
   //-------------------------------------- Sustainable damage
   sd->write(IDITG_SSP_SUSTAINABLEDAMAGE, m_sustainableDamage);
   
   //-------------------------------------- Current damage
   sd->write(IDITG_SSP_CURRENTDAMAGE,     m_currentDamage);
   
   //-------------------------------------- Explosion tag
   sd->write(IDITG_SSP_EXPLOSIONTAG, true, "IDDTS_EXP_*", m_explosionTag);
   sd->writeActionButton(IDITG_SSP_TESTSPAWNEXP, this, TestSpawnExplosion);

   //--------------------------------------Position/Rotation
   sd->write(IDITG_POSITION, getLinearPosition());
   Point3F inspRot = translateToInspectRotation(getAngulerPosition());
   sd->write(IDITG_ROTATION, inspRot);

   //-------------------------------------- Indestructible?
   sd->write(IDITG_SSP_INDESTRUCTIBLE, testShapeFlag(SFIndestructible));

   //-------------------------------------- Predestroyed?
   sd->write(IDITG_SSP_DESTROYED, testShapeFlag(SFIsDestroyed));
}


void SimShape::inspectRead(Inspect *sd)
{
   Parent::inspectRead(sd);

   Int32 minTag, maxTag;
   SimTag simTag;
   Bool   dummyBool;
   Int32  dummyInt;
   
   //-------------------------------------- Damage Forwarding
   sd->read(IDITG_SSP_LOCALLYCONSERVEDAMAGE, dummyBool);
   if (dummyBool == true)
      setShapeFlag(SFLocallyConserveDamage);
   else
      clearShapeFlag(SFLocallyConserveDamage);

   sd->read(IDITG_SSP_DAMAGEDISTRIBUTION, &dummyBool, &minTag, &maxTag, simTag);
   if (simTag != 0) {
      setDamageDistribution(DamageDistribution(simTag));
   }

   float damageAtten;
   sd->read(IDITG_SSP_DAMAGEFORWARDATTEN, damageAtten);
   setDamageForwardAttenuation(damageAtten);
   
   //-------------------------------------- Destruction Event
   sd->read(IDITG_SSP_DESTRUCTEVENT, &dummyBool, &minTag, &maxTag, simTag);
   if (simTag != 0) {
      setDestructionEvent(DestructionEvent(simTag));
   }
   
   float destructDamage;
   sd->read(IDITG_SSP_DESTRUCTIONDAMAGE, destructDamage);
   setDestructionDamage(destructDamage);
   
   //-------------------------------------- Sustainable damage
   float sustainable;
   sd->read(IDITG_SSP_SUSTAINABLEDAMAGE, sustainable);
   setSustainableDamage(sustainable);
   
   //-------------------------------------- Current damage
   float current;
   sd->read(IDITG_SSP_CURRENTDAMAGE, current);
   setCurrentDamage(current);
   
   //-------------------------------------- Explosion tag
   SimObject* dummyPObj;
   sd->read(IDITG_SSP_EXPLOSIONTAG, &dummyBool, "IDDTS_EXP_*", m_explosionTag);
   sd->readActionButton(IDITG_SSP_TESTSPAWNEXP, &dummyPObj, &dummyInt);

   //--------------------------------------Position/Rotation
   Point3F tempPos, tempRot;
   sd->read(IDITG_POSITION, tempPos);
   sd->read(IDITG_ROTATION, tempRot);
   EulerF newRot = translateFromInspectRotation(tempRot);
   set(newRot, tempPos);

   //-------------------------------------- Indestructible?
   Bool indest;
   sd->read(IDITG_SSP_INDESTRUCTIBLE, indest);
   if (indest == true) {
      setShapeFlag(SFIndestructible);
   } else {
      clearShapeFlag(SFIndestructible);
   }

   //-------------------------------------- Predestroyed?
   bool preDestroyed;
   sd->read(IDITG_SSP_DESTROYED, preDestroyed);
   setIsDestroyed(preDestroyed);
}


//-------------------------------------- Object initialization/Persistent
//                                        functionality
//

DWORD
SimShape::packUpdate(Net::GhostManager* /*gm*/,
                     DWORD              mask,
                     BitStream*         stream)
{
   if (mask & InitialUpdate) {
      // For right now, we don't care
      //
   } else {
      //
   }

   if (mask & FlagsMask) {
      stream->writeFlag(true);
      stream->write(m_shapeFlags);
   } else {
      stream->writeFlag(false);
   }

   if (mask & PosRotMask) {
      stream->writeFlag(true);

      stream->write(sizeof(Point3F), &m_shapePosition);
      stream->write(sizeof(EulerF),  &m_shapeRotation);
   } else {
      stream->writeFlag(false);
   }

   return 0;
}

void
SimShape::unpackUpdate(Net::GhostManager* /*gm*/,
                       BitStream*         stream)
{
   if (stream->readFlag()) {
      UInt32 newFlags;
      stream->read(&newFlags);
      
      setAllShapeFlags(newFlags);
   }
   
   if (stream->readFlag()) {
      Point3F tempPos;
      EulerF  tempRot;

      stream->read(sizeof(Point3F), &tempPos);
      stream->read(sizeof(EulerF),  &tempRot);
      set(tempRot, tempPos);
   }
}

Persistent::Base::Error
SimShape::read(StreamIO& sio,
               int       /*version*/,
               int       /*user*/)
{
   int version;
   sio.read(&version);
   AssertFatal(version == SimShape::version(), "Wrong file version: SimShape");

   sio.read(&m_shapeFlags);
   
   // check if should set locked flag on SimObject flag
   SimObject::setLocked( testShapeFlag( SimShape::SFIsLocked ) );
   
   Int32 damdist, destev;

   sio.read(&damdist);
   sio.read(&destev);
   m_damageDistribution = DamageDistribution(damdist);
   m_destructionEvent   = DestructionEvent(destev);

   sio.read(&m_sustainableDamage);
   sio.read(&m_currentDamage);

   sio.read(&m_explosionTag);

   sio.read(&m_damageForwardAttenuation);
   sio.read(&m_destructionDamage);

   sio.read(sizeof(Point3F), &m_shapePosition);
   sio.read(sizeof(EulerF),  &m_shapeRotation);
   set(TMat3F(m_shapeRotation, m_shapePosition), false);

   return (sio.getStatus() == STRM_OK) ? Ok : ReadError;
}

Persistent::Base::Error
SimShape::write(StreamIO& sio,
                int       /*version*/,
                int       /*user*/)
{
   sio.write(SimShape::version());

   // check for editing locked flag
   SimObject::isLocked() ? setShapeFlag( SimShape::SFIsLocked ) : 
      clearShapeFlag( SimShape::SFIsLocked );
   
   sio.write(m_shapeFlags);
   
   sio.write(m_damageDistribution);
   sio.write(m_destructionEvent);
   
   sio.write(m_sustainableDamage);
   sio.write(m_currentDamage);

   sio.write(m_explosionTag);
   
   sio.write(m_damageForwardAttenuation);
   sio.write(m_destructionDamage);

   sio.write(sizeof(Point3F), &m_shapePosition);
   sio.write(sizeof(EulerF),  &m_shapeRotation);

   return (sio.getStatus() == STRM_OK) ? Ok : WriteError;
}

int
SimShape::version()
{
   return sm_fileVersion;
}


UInt32
SimShape::getFrameKey() const
{
   return sm_frameKey;
}
