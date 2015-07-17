//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMTSSHAPE_H_
#define _SIMTSSHAPE_H_

//Includes
#include <ts.h>
#include "simMovingShape.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class DLLAPI SimTSShape : public SimMovingShape
{
   typedef SimMovingShape Parent;
   static int sm_fileVersion;

  private:
   static int sm_maxAnimations;  // Max number of animation is 8

   UInt8 m_currNumAnimations;       // - Number of properly named sequences in the m_resShape (G+/P+)
   UInt8 m_playingAnimations;       // - Bitfield: When b(x) is set, SequenceX anim is
                                    //  playing.  Note: x refers to the logical animation
                                    //  in this structure, the actual TS::Shape::Sequence
                                    //  is given by m_animParams[x].seqIndex
   char* m_pShapeName;
   char* m_pHulkName;

  protected:
   enum NetMaskFlags {
      SimShapeResPt  = 1 << 1,  // SimShape reserves all flags <= SimShapeResPt

      ShapeParamMask = SimShapeResPt << 1,
      AnimParamMask  = SimShapeResPt << 2,
      ActiveAnimMask = SimShapeResPt << 3,

      LastTSShapeBit = SimShapeResPt << 3,

      InitialUpdate  = 1 << 31
   };

   struct AnimationParams {
      bool  isSet;
      int   seqIndex;      // Gives the sequence index on the TS::Shape
      
      bool  autoStart;
      float animDuration;
      AnimationParams() : isSet(false), autoStart(false), animDuration(0.0f) { }
   };

   //-------------------------------------- Shape resources/Tracking info
   Resource<TSShape> m_resShape;
   Resource<TSShape> m_resHulk;

   bool reloadShapeResources(/*const bool in_reinitAnimation = true*/);
   bool reloadHulkResources();
   bool setShapeName(const char* in_pName);
   bool setHulkName(const char* in_pName);

   const char* getShapeName() const { return m_pShapeName; }
   const char* getHulkName()  const { return m_pHulkName; }

   virtual void unloadingResources() = 0;
   virtual void loadedResources()    = 0;

   //-------------------------------------- Animation related members
   AnimationParams m_animParams[8]; // - Only the first m_currNumAnimations are valid (G+/P+)
   bool acknowledgeAllAnimationSequences;

   static int getMaxAnimations();
   UInt8 getCurrNumAnimations() const { return m_currNumAnimations; }
   UInt8 getPlayingAnimations() const { return m_playingAnimations; }
   void  setAnimation(int index, bool on);

   void initAnimation();
   void autoStartAnimations();
   virtual void animationParamsChanged() = 0;
   virtual void playingAnimationsChanged(const UInt8 m_oldPlaying) = 0; // Will be called when
                                                                        //  m_playingAnimations
                                                                        //  changes
   //-------------------------------------- Event handlers
	bool onAdd();
	void inspectRead(Inspect *inspector);
	void inspectWrite(Inspect *inspector);
   bool onSimTriggerEvent(const SimTriggerEvent*);

   // Overrides from SimMovement
	virtual void buildContainerBox(const TMat3F& mat,Box3F* box);

  public:
   struct DLLAPI RenderImage : public SimRenderImage, public SimShapeImageDet {
      TMat3F           transform;
      TSShapeInstance* currShape;

      RenderImage() : currShape(NULL) { }
      ShapeImageType getImageType() const;
   };

   SimTSShape();
   ~SimTSShape();

   UInt32 getPartType() const;

   //-------------------------------------- Event/Query processing
   bool processEvent(const SimEvent*);
   bool processArguments(int, const char** argv);

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

#endif //_SIMTSSHAPE_H_
