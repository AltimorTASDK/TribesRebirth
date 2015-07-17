//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <link.h>
#include <inspect.h>
#include "simTSShape.h"
#include "esf.strings.h"
#include "darkstar.strings.h"
#include "editor.strings.h"
#include "commonEditor.strings.h"
#include "simResource.h"
#include "console.h"

//------------------------------------------------------------------------------
//--------------------------------------
// Static Data Members and link forcer
//--------------------------------------
//
int SimTSShape::sm_fileVersion   = 2;
int SimTSShape::sm_maxAnimations = 8;

// Link forcer.  Link is forces in simObjectPersTags.cpp
//
DeclareLink(SimTSShapeLINK);

//------------------------------------------------------------------------------
//--------------------------------------
// C'tors/D'tors
//--------------------------------------
//
SimTSShape::SimTSShape()
 : m_pShapeName(NULL),
   m_pHulkName(NULL),
   m_playingAnimations(0),
   m_currNumAnimations(0)
{
   // Note: Do _not_ call any of the pure virtuals in the Ctor
   
   acknowledgeAllAnimationSequences = false;
}

SimTSShape::~SimTSShape()
{
   delete [] m_pShapeName;
   m_pShapeName = NULL;
   delete [] m_pHulkName;
   m_pHulkName = NULL;
}

int
SimTSShape::getMaxAnimations()
{
   return sm_maxAnimations;
}

//------------------------------------------------------------------------------
//--------------------------------------
// SimShape overrides
//--------------------------------------
//
UInt32
SimTSShape::getPartType() const
{
   return UInt32(Parent::SimTSShapeType);
}


//------------------------------------------------------------------------------
//--------------------------------------
// SimMovement overrides
//--------------------------------------
//
void
SimTSShape::buildContainerBox(const TMat3F& in_rTrans,
                              Box3F*        out_pBox)
{
   // Note that this function does NOT NOT NOT take into account any translational motion
   //  caused by animations.  It is also grotesquely inaccurate.
   //
   Box3F boundingBox;
   Point3F center;
   float   radius;
   if (testShapeFlag(SFIsDestroyed) == false) {
      if (bool(m_resShape) == true) {
         center = m_resShape->fCenter;
         radius = m_resShape->fRadius;
      } else {
         center.set(-1e10f, -1e10f, -1e10f);
         radius = 0.0f;
      }
   } else {
      if (bool(m_resHulk) == true) {
         center = m_resHulk->fCenter;
         radius = m_resHulk->fRadius;
      } else {
         center.set(-1e10f, -1e10f, -1e10f);
         radius = 0.0f;
      }
   }

   boundingBox.fMin.set(center.x - radius, center.y - radius, center.z - radius);
   boundingBox.fMax.set(center.x + radius, center.y + radius, center.z + radius);

   m_mul(boundingBox, in_rTrans, out_pBox);
}

//------------------------------------------------------------------------------
//--------------------------------------
// Resource Loading/Initialization
//--------------------------------------
//
bool
SimTSShape::reloadShapeResources()
{
   AssertFatal(manager != NULL, "Cannot load resources wo/ manager...");

   unloadingResources();
   if (bool(m_resShape))
      m_resShape.unlock();
   if (m_pShapeName == NULL) {
      loadedResources();
      return false;
   }
   
   ResourceManager* pResManager = SimResource::get(manager);
   m_resShape = pResManager->load(m_pShapeName, true);

   loadedResources();

   // Force a refresh of the bounding box...
   set(getTransform(), false);

   return bool(m_resShape);
}

bool
SimTSShape::reloadHulkResources()
{
   AssertFatal(manager != NULL, "Cannot load resources wo/ manager...");

   unloadingResources();
   if (bool(m_resHulk)) {
      m_resHulk.unlock();
   }
   if (m_pHulkName == NULL) {
      loadedResources();
      animationParamsChanged();
      return false;
   }

   ResourceManager* pResManager = SimResource::get(manager);
   m_resHulk = pResManager->load(m_pHulkName, true);

   loadedResources();
   animationParamsChanged();

   // Force a refresh of the bounding box...
   set(getTransform(), false);

   return bool(m_resHulk);
}

bool
SimTSShape::setShapeName(const char* in_pName)
{
   if (in_pName == NULL) {
      if (m_pShapeName != NULL) {
         // shape is being cleared
         delete [] m_pShapeName;
         m_pShapeName = NULL;
      } else {
         return false;
      }
   } else {
      char* pTemp = new char[strlen(in_pName) + 1];
      strcpy(pTemp, in_pName);
      delete [] m_pShapeName;
      m_pShapeName = pTemp;
   }
   
   setMaskBits(ShapeParamMask | AnimParamMask);
   bool ret;
   if (manager != NULL) {
      ret = reloadShapeResources();
   } else {
      ret = false;
   }
   
   if (ret == true) {
      initAnimation();
      animationParamsChanged();
   }

   return ret;
}

bool
SimTSShape::setHulkName(const char* in_pName)
{
   if (in_pName == NULL) {
      if (m_pHulkName != NULL) {
         // shape is being cleared
         delete [] m_pHulkName;
         m_pHulkName = NULL;
      } else {
         return false;
      }
   } else {
      char* pTemp = new char[strlen(in_pName) + 1];
      strcpy(pTemp, in_pName);
      delete [] m_pHulkName;
      m_pHulkName = pTemp;
   }
   
   setMaskBits(ShapeParamMask | AnimParamMask);

   if (manager != NULL) {
      return reloadHulkResources();
   } else {
      return false;
   }
}


//------------------------------------------------------------------------------
//--------------------------------------
// Animation management
//--------------------------------------
//
void
SimTSShape::autoStartAnimations()
{
   UInt8 playing = 0;
   for (int i = 0; i < 8; i++) {
      if (m_animParams[i].autoStart == true) {
         playing |= 1 << i;
      }
   }
   
   UInt8 old = m_playingAnimations;
   m_playingAnimations = playing;
   playingAnimationsChanged(old);
}

void
SimTSShape::initAnimation()
{
   // Ghosts get all their animparams from unpackUpdate...
   //
   if (isGhost() == false) {
      // We reset the autoStart field on all animationParam structs,
      //  animation duration is set to 0 if no appropriate SequenceX animation exists,
      //  otherwise, it defaults to the preset length of the animation...
      //
      m_currNumAnimations = 0;
      if (bool(m_resShape) == true) {
         if (acknowledgeAllAnimationSequences)
         {
            // track all sequences in the shape   
            #ifdef DEBUG
            if (m_resShape->fSequences.size() > sm_maxAnimations) {
               char buffer[256];
               sprintf(buffer, "shape \"%s\" has more anim sequences than the max (%d",
                  getShapeName(), sm_maxAnimations);
               AssertWarn(1, buffer);
            }
            #endif
            int count = min(sm_maxAnimations, m_resShape->fSequences.size());
            for (int j = 0; j < count; j++) {
               m_animParams[m_currNumAnimations].seqIndex = j;
               m_currNumAnimations++;
            }
         }
         else 
         {
            // track only sequences that match "Sequence%2.2d"
            for (int j = 0; j < sm_maxAnimations; j++) {
               char cmpSeqName[256];
               sprintf(cmpSeqName, "Sequence%2.2d", j + 1);
               
               int k;
               for (k = 0; k < m_resShape->fSequences.size(); k++) {
                  const TS::Shape::Sequence& sequence = m_resShape->fSequences[k];
                  const char* seqName                 = m_resShape->getName(sequence.fName);
                  
                  if (strcmp(cmpSeqName, seqName) == 0) {
                     break;
                  }
               }
               
               if (k != m_resShape->fSequences.size()) {
                  AnimationParams& animParams = m_animParams[m_currNumAnimations];
                  m_currNumAnimations++;
                  animParams.seqIndex = k;
               } else {
                  // We've found the last...
                  //
                  break;
               }
            }
         }
         
         int i;
         for (i = 0; i < m_currNumAnimations; i++) {
            AnimationParams& animParams = m_animParams[i];
            const TS::Shape::Sequence& sequence =
                  m_resShape->getSequence(animParams.seqIndex);
            
            animParams.isSet        = true;
            animParams.autoStart    = false;
            animParams.animDuration = sequence.fDuration;
         }
         for (i = m_currNumAnimations; i < sm_maxAnimations; i++) {
            AnimationParams& animParams = m_animParams[i];
            
            animParams.isSet        = false;
            animParams.autoStart    = false;
            animParams.animDuration = 0.0f;
         }
      } else {
         for (int i = 0; i < sm_maxAnimations; i++) {
            AnimationParams& animParams = m_animParams[i];
            
            animParams.isSet        = false;
            animParams.autoStart    = false;
            animParams.animDuration = 0.0f;
         }
      }
   
      UInt8 nowPlaying = 0;
      for (int i = 0; i < sm_maxAnimations; i++) {
         if (m_animParams[i].autoStart == true)
            nowPlaying |= (1 << i);
      }

      UInt8 old = m_playingAnimations;
      m_playingAnimations = nowPlaying;
      playingAnimationsChanged(old);

      setMaskBits(AnimParamMask);
   }
}

void
SimTSShape::setAnimation(int index, bool on = true)
{
   int bit = 1 << index;
   
   // why does it work this way?  this is stupid
   UInt8 old = m_playingAnimations;
   if (on)
      m_playingAnimations |= bit;
   else
      m_playingAnimations &= ~bit;
      
   playingAnimationsChanged(old);
}

bool
SimTSShape::processArguments(int          argc,
                             const char** argv)
{
   if (argc >= 1) {
      setShapeName(argv[0]);
   }

   if (argc >= 2) {
      setHulkName(argv[1]);
   }

   return true;
}

bool
SimTSShape::processEvent(const SimEvent* event)
{
   switch (event->type) {
      onEvent(SimTriggerEvent);

     default:
      return Parent::processEvent(event);
   }
}


//------------------------------------------------------------------------------
//
bool SimTSShape::onAdd()
{
	if (!Parent::onAdd())
		return false;

   // Note that we don't want the animation parameters to be reset here...
   //
   if (reloadShapeResources() == true) {
      animationParamsChanged();
      autoStartAnimations();
   } else {
      // Unclear how to handle this error.  Derived classes must make sure that valid
      //  shape resources are loaded...
   }
   
   if (reloadHulkResources() == false) {
      // ""
   }

	return true;
}


//------------------------------------------------------------------------------
//
void SimTSShape::inspectRead(Inspect *sd)
{
	Parent::inspectRead(sd);

   bool processAnimations = true;
   int  numAnimsToProcess = m_currNumAnimations;
   
   // ignore divider
   sd->readDivider();
   char tempBuf[512];

   sd->read(IDITG_STSS_SHAPEFILENAME, tempBuf);
   if (m_pShapeName    != NULL &&
       stricmp(tempBuf, m_pShapeName) == 0) {
      // Shape is the same...
      //
   } else if (strlen(tempBuf) == 0) {
      // Shape should be removed
      processAnimations = false;
      setShapeName(NULL);
   } else {
      processAnimations = false;
      setShapeName(tempBuf);
   }
   
   sd->read(IDITG_STSS_HULKFILENAME, tempBuf);
   if (m_pHulkName     != NULL &&
       stricmp(tempBuf, m_pHulkName) == 0) {
      //
   } else if (strlen(tempBuf) == 0) {
      setHulkName(NULL);
   } else {
      setHulkName(tempBuf);
   }
   
   if (processAnimations == true) {
      // Shape is the same, read the data into the animParams...
      //
      for (int i = 0; i < numAnimsToProcess; i++) {
         int offset = i * 3;
         SimTag seqAutostart = IDITG_STSS_SEQUENCE0_AUTOSTART + offset;
         SimTag seqDuration  = IDITG_STSS_SEQUENCE0_DURATION  + offset;
      
         sd->readDivider();
         sd->read(seqAutostart, m_animParams[i].autoStart);
         sd->read(seqDuration,  m_animParams[i].animDuration);
      }

      UInt8 old           = m_playingAnimations;
      m_playingAnimations = 0;
      for (int j = 0; j < numAnimsToProcess; j++)
         if (m_animParams[j].autoStart == true)
            m_playingAnimations = 1 << j;

      setMaskBits(AnimParamMask);
      animationParamsChanged();
      if (m_playingAnimations != old)
         playingAnimationsChanged(old);
   } else {
      // Shape has been switched, toss away these values...
      //
      for (int i = 0; i < numAnimsToProcess; i++) {
         int offset = i * 3;
         SimTag seqAutostart = IDITG_STSS_SEQUENCE0_AUTOSTART + offset;
         SimTag seqDuration  = IDITG_STSS_SEQUENCE0_DURATION  + offset;
         bool  dummyBool;
         float dummyFloat;
      
         sd->readDivider();
         sd->read(seqAutostart, dummyBool);
         sd->read(seqDuration,  dummyFloat);
      }
   }
}

void SimTSShape::inspectWrite(Inspect *sd)
{
	Parent::inspectWrite(sd);

   sd->writeDivider();
   sd->write(IDITG_STSS_SHAPEFILENAME,    m_pShapeName);
   sd->write(IDITG_STSS_HULKFILENAME,     m_pHulkName);
   
   for (int i = 0; i < m_currNumAnimations; i++) {
      int offset = i * 3;
      SimTag seqAutostart = IDITG_STSS_SEQUENCE0_AUTOSTART + offset;
      SimTag seqDuration  = IDITG_STSS_SEQUENCE0_DURATION  + offset;
      
      sd->writeDivider();
      sd->write(seqAutostart, m_animParams[i].autoStart);
      sd->write(seqDuration,  m_animParams[i].animDuration);
   }
}

bool
SimTSShape::onSimTriggerEvent(const SimTriggerEvent* event)
{
	Parent::onSimTriggerEvent(event);

   // Make sure we have the right value
   int animation = int(event->value + 0.5);

   if (animation >= 1 && animation <= getCurrNumAnimations()) {
      // Only respond to triggers that are in range, respond by toggling that animation
      //  on or off...
      //
      UInt8 old = m_playingAnimations;
      switch (event->action)
      {
         case SimTriggerEvent::Activate:
            m_playingAnimations |= 1 << (animation - 1);
            break;
         case SimTriggerEvent::Deactivate:
            m_playingAnimations &= ~(1 << (animation - 1));
            break;
      }

      playingAnimationsChanged(old);
      return true;   
   } else {
      return false;
   }
}


//------------------------------------------------------------------------------
//--------------------------------------
// Persistent Functions
//--------------------------------------
//
DWORD
SimTSShape::packUpdate(Net::GhostManager* gm,
                                 DWORD              mask,
                                 BitStream*         stream)
{
   UInt32 partialMask = Parent::packUpdate(gm, mask, stream);
   
   // TS stuff
   //
   if (mask & ShapeParamMask) {
      stream->writeFlag(true);
      
      if (m_pShapeName != NULL) {
         int strLen = strlen(m_pShapeName) + 1;
         stream->write(strLen);
         stream->write(strLen, m_pShapeName);
      } else {
         stream->write(0);
      }
      
      if (m_pHulkName != NULL) {
         int strLen = strlen(m_pHulkName) + 1;
         stream->write(strLen);
         stream->write(strLen, m_pHulkName);
      } else {
         stream->write(0);
      }
   } else {
      stream->writeFlag(false);
   }
      
   if (mask & AnimParamMask) {
      stream->writeFlag(true);
      stream->writeInt(m_currNumAnimations, 3);
      stream->write(sizeof(AnimationParams) * sm_maxAnimations, m_animParams);
   } else {
      stream->writeFlag(false);
   }

   if (mask & ActiveAnimMask) {
      stream->writeFlag(true);
      stream->writeInt(m_playingAnimations, 8);
   } else {
      stream->writeFlag(false);
   }

   return partialMask;
}

void
SimTSShape::unpackUpdate(Net::GhostManager* gm,
                         BitStream*         stream)
{
   Parent::unpackUpdate(gm, stream);
   
   // TS stuff
   //
   char* shapeName = NULL;
   char* hulkName  = NULL;

   bool shapeChanged = stream->readFlag();
   if (shapeChanged == true) {         // Filenames changed...
      int strLen;
      
      stream->read(&strLen);
      if (strLen != 0) {
         shapeName = new char[strLen];
         stream->read(strLen, shapeName);
      }
      stream->read(&strLen);
      if (strLen != 0) {
         hulkName = new char[strLen];
         stream->read(strLen, hulkName);
      }
   }

   bool animChanged  = stream->readFlag();
   if (animChanged == true) {
      m_currNumAnimations = stream->readInt(3);
      stream->read(sizeof(AnimationParams) * sm_maxAnimations, m_animParams);
   }
   
   if (manager == NULL) {
      // have to wait for addNotify
      if (shapeChanged == true) {
         AssertFatal(m_pShapeName == NULL, "ah, memory leak about to happen");
         m_pShapeName = shapeName;
         AssertFatal(m_pHulkName == NULL, "ah, memory leak about to happen");
         m_pHulkName  = hulkName;
      }
   } else {
      if (shapeChanged == true) {
         setShapeName(shapeName);
         setHulkName(hulkName);
         delete [] shapeName;
         delete [] hulkName;
      }
      if (animChanged == true) {
//      animationParamsChanged();
      }
   }

   if (stream->readFlag() == true) {         // Currently playing animations changed
      UInt8 temp, old;
      temp = stream->readInt(8);
      
      old = m_playingAnimations;
      m_playingAnimations = temp;
      if (manager)
         playingAnimationsChanged(old);
   }
}

Persistent::Base::Error
SimTSShape::write(StreamIO& sio,
                  int       version,
                  int       user)
{
   sio.write(SimTSShape::version());

   if (Parent::write(sio, version, user) != Ok) {
      return WriteError;
   }

   if (m_pShapeName != NULL) {                        // Shape
      int strLen = strlen(m_pShapeName) + 1;
      sio.write(strLen);
      sio.write(strLen, m_pShapeName);
   } else {
      sio.write(0);
   }

   if (m_pHulkName != NULL) {                         // Hulk
      int strLen = strlen(m_pHulkName) + 1;
      sio.write(strLen);
      sio.write(strLen, m_pHulkName);
   } else {
      sio.write(0);
   }

   sio.write(m_currNumAnimations);
   sio.write(8 * sizeof(AnimationParams), m_animParams);

   return ((sio.getStatus() == STRM_OK) ? Ok : WriteError);
}

Persistent::Base::Error
SimTSShape::read(StreamIO& sio,
                 int       ignore_version,
                 int       user)
{
#if 1
	if (Console->getBoolVariable("SimShapeOldFormat")) {
	   if (Parent::read(sio, ignore_version, user) != Ok) {
	      return ReadError;
	   }
	   int version;
	   sio.read(&version);
	}
	else {
	   int version;
	   sio.read(&version);
	   AssertFatal(version == SimTSShape::version(),
	   	 "Wrong file version: SimTSShape");
	   if (Parent::read(sio, ignore_version, user) != Ok) {
	      return ReadError;
	   }
	}
#else
   int version;
   sio.read(&version);
   AssertFatal(version == SimTSShape::version(),
   	 "Wrong file version: SimTSShape");
   if (Parent::read(sio, ignore_version, user) != Ok) {
      return ReadError;
   }
#endif

   int strLen;
   sio.read(&strLen);
   if (strLen != 0) {
      m_pShapeName = new char[strLen];
      sio.read(strLen, m_pShapeName);
   } else {
      m_pShapeName = NULL;
   }

   sio.read(&strLen);
   if (strLen != 0) {
      m_pHulkName = new char[strLen];
      sio.read(strLen, m_pHulkName);
   } else {
      m_pHulkName = NULL;
   }

   sio.read(&m_currNumAnimations);
   sio.read(8 * sizeof(AnimationParams), m_animParams);

   return ((sio.getStatus() == STRM_OK) ? Ok : ReadError);
}

int
SimTSShape::version()
{
   return sm_fileVersion;
}


//------------------------------------------------------------------------------
//--------------------------------------
// Renderimage functions...
//--------------------------------------
//
SimShapeImageDet::ShapeImageType
SimTSShape::RenderImage::getImageType() const
{
   return TS3Image;
}

