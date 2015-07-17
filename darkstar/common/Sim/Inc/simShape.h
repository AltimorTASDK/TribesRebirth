//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMSHAPEPART_H_
#define _SIMSHAPEPART_H_

//Includes
#include <simBase.h>
#include <inspect.h>
#include <simMovement.h>
#include <simDamageEv.h>
#include <esf.strings.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class SimShapeImageDet {
  public:
   enum ShapeImageType {
      TS3Image,
      ITRImage
   };

   virtual ShapeImageType getImageType() const = 0;
};


class SimShapeGroupRep;
class SimShapePlugin;

class DLLAPI SimShape : public SimMovement
{
   typedef SimNetObject Parent;
   friend class SimShapePlugin;

   // Derived classes and/or program specific flags, i.e. targetable designation
   //
   UInt32  m_shapeFlags;   // Ghost+/Persistent+

  private:
   static UInt32 sm_frameKey;
  protected:
   UInt32 getFrameKey() const;

  protected:
   static int  sm_fileVersion;
   static bool sm_drawShapeBBox;

   enum ActionMessages {
      TestSpawnExplosion
   };

   //-------------------------------------- Public types
  public:
   enum DamageDistribution {
      ToParentFull       = IDSIMOBJ_SSP_DAMAGETOPARENT,
      ToParentAttenuated = IDSIMOBJ_SSP_DAMAGETOPARENTATTEN, 
      ToNone             = IDSIMOBJ_SSP_DAMAGETONONE,
   };

   enum DestructionEvent {
      DoNothing            = IDSIMOBJ_SSP_DESTDONOTHING,
      ApplyMaxDamage       = IDSIMOBJ_SSP_DESTAPPLYMAXDAMAGE,
      ApplySpecifiedDamage = IDSIMOBJ_SSP_DESTAPPLYDAMAGEPOINTS,
   };

   // These are the default types.  Derived classes may define new types if
   //  necessary when overiding getPartType()...
   //
   enum PartType {
      SimInteriorShapeType = 1,
      SimTSShapeType       = 2
   };

  public:
   static bool getDrawShapeBBox();
  protected:
   static void setDrawShapeBBox(const bool in_drawBox);

   enum ShapeFlags {
      SFIndestructible        = 1 << 0,
      SFIsDestroyed           = 1 << 1,
      SFLocallyConserveDamage = 1 << 2,
      SFIsLocked              = 1 << 3,
   };

   enum NetMaskFlags {
      FlagsMask       = 1 << 0,
      PosRotMask      = 1 << 1,
      InitialUpdate   = 1 << 31
   };

   // Damage controllers and trackers.  Note: no damage state is maintained on
   //  the client side...
   //
   SimShapeGroupRep*  m_pDamageParent;

   DamageDistribution m_damageDistribution;  // G-/P+
   DestructionEvent   m_destructionEvent;    // G-/P+

   float m_sustainableDamage;                // G-/P+
   float m_currentDamage;                    // G-/P+
   
   float m_damageForwardAttenuation;         // G-/P+
   float m_destructionDamage;                // G-/P+
   
   //--------------------------------------
   // DMMTODO
   // debris    info
   //--------------------------------------
   //
   SimTag   m_explosionTag;                  // G-/P+

   // Position and rotation tracked by the shape.  NB: This should be handled very
   //  carefully: derived classes should only access these through the set(...) functions
   //  and get them only for persistence.  Otherwise, use the SimMovement analogs...
  private:
   Point3F  m_shapePosition;
   EulerF   m_shapeRotation;
  protected:
   const Point3F& getShapePosition() const { return m_shapePosition; }
   const EulerF&  getShapeRotation() const { return m_shapeRotation; }

   //-------------------------------------- Event/Query handlers
   bool onSimDamageEvent(const SimDamageEvent*);
   bool onSimMessageEvent(const SimMessageEvent* event);
   bool onSimObjectTransformEvent(const SimObjectTransformEvent*);
   bool onSimObjectTransformQuery(SimObjectTransformQuery *query);

   //-------------------------------------- State modifiers
   void setDamageDistribution(const DamageDistribution);
   void setDamageForwardAttenuation(const float);
   void setDestructionEvent(const DestructionEvent);
   void setDestructionDamage(const float);
   void setSustainableDamage(const float);
   void setCurrentDamage(const float);
   
   void   setShapeFlag(const UInt32);
   void   setAllShapeFlags(const UInt32);
   void   clearShapeFlag(const UInt32);
   bool   testShapeFlag(const UInt32 in_test) const { return (m_shapeFlags & in_test) != 0; }
   UInt32 getShapeFlags() const                     { return m_shapeFlags; }
   
   virtual void spawnExplosion();
   void destroySelf();
   
   //-------------------------------------- Derived classes override this if necessary
   //
   virtual void shapeFlagsChanged();
   
   //-------------------------------------- Gross shape state tracking/manipulation
   //
   void setIsDestroyed(bool in_dest) { in_dest ? setShapeFlag(SFIsDestroyed) :
                                                 clearShapeFlag(SFIsDestroyed); }

   //-------------------------------------- Positional Functions
   //
   void set(const TMat3F&, const bool in_reextractAngles);
   void set(const EulerF&, const Point3F&);
   
   int getDerivedVersion(int) { return 0x100; }   
  public:
   //-------------------------------------- C'tors/D'tors
   SimShape();
   ~SimShape();
   
   // Damage Parenting representation...
   //
   bool getIsDestroyed() const       { return testShapeFlag(SFIsDestroyed); }
   virtual void setDamageParent(SimShapeGroupRep* io_pParent);
   virtual void clearDamageParent();
   SimShapeGroupRep* getDamageParent() const;
   
   virtual void acceptForwardedDamage(const double);

   //-------------------------------------- SimEvent/SimQuery handling
   bool processEvent(const SimEvent*);
   bool processQuery(SimQuery*);
	void inspectRead(Inspect *inspector);
	void inspectWrite(Inspect *inspector);

   //-------------------------------------- Like Container System, this is
   //                                        quicker than a dyn_cast
   virtual UInt32 getPartType() const = 0;

   //-------------------------------------- Persistent Functionality
   DWORD packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream);
   void  unpackUpdate(Net::GhostManager *gm, BitStream *stream);
	Persistent::Base::Error read(StreamIO &, int version, int user);
	Persistent::Base::Error write(StreamIO &, int version, int user); 
   int version();
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_SIMSHAPEPART_H_
