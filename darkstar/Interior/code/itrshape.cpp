//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "ml.h"
#include "itrShape.h"

// Instantiate our map template...
//
template class Map<String, UInt32>;


const char * ITRShape::sm_pShapeFileExtension = ".dis";
const UInt32      ITRShape::sm_allFacesLinkable    = 252;    // = 011111100b



ITRShape::ITRShape()
 : m_numLightStates(0),
   m_materialListOffset(1),
   m_linkedInterior(false)
{
   //
}


ITRShape::~ITRShape()
{
   //
}


UInt32 ITRShape::addFileName(const char* in_pFileName)
{
   UInt32 offset = m_nameBuffer.size();
   UInt32 strLen = strlen(in_pFileName) + 1;

   m_nameBuffer.setSize(offset + strLen);
   strcpy(&m_nameBuffer[offset], in_pFileName);
   
   return offset;
}

void ITRShape::sortLODs()
{
   for (Int32 i = 0; i < m_stateVector.size(); i++) {
      State& state = m_stateVector[i];
      m_qsort(&m_lodVector[state.lodIndex],
              state.numLODs,
              sizeof(LOD),
              compareLOD);
   }
}

int _USERENTRY
ITRShape::compareLOD(const void* in_pElem1,
                     const void* in_pElem2)
{
   // Sorts by min pixels...
   //
   const LOD* pLOD1 = static_cast<const LOD*>(in_pElem1);
   const LOD* pLOD2 = static_cast<const LOD*>(in_pElem2);
   
   return (pLOD2->minPixels - pLOD1->minPixels);
}


ITRShape::State& 
ITRShape::getState(const UInt32 in_stateNum)
{
   AssertFatal(in_stateNum < UInt32(m_stateVector.size()), "Out of bounds...");
   
   return m_stateVector[in_stateNum];
}


UInt32 ITRShape::findDetail(const State* in_pState,
                     const UInt32 in_pixels) const
{
   UInt32 index = in_pState->lodIndex;
   
   // Find the first state with minPixels less than in_pixels. 
   //
   for (UInt32 i = 0; i < in_pState->numLODs; i++) {
      if (m_lodVector[i+index].minPixels < in_pixels) {
         return i;
      }
   }

   // If we're here, the last LOD had a minPixel set to something greater than
   //  the input.  Return the lowest (last) LOD
   //
   return (in_pState->numLODs - 1);
}

//------------------------------------------------------------------------------
// NAME 
//    Persistent::Base::Error 
//    ITRShape::write(StreamIO &s,int version,int)
//    Persistent::Base::Error 
//    ITRShape::read(StreamIO &s,int version,int)
//    int ITRShape::version()
//    
// DESCRIPTION 
//    Persistent functionality...
//    
// NOTES 
//    
//------------------------------------------------------------------------------
IMPLEMENT_PERSISTENT_TAG(ITRShape, FOURCC('I', 'T', 'R', 's'));
int ITRShape::sm_fileFormatVersion = 3;

Persistent::Base::Error 
ITRShape::write(StreamIO& s,
                int       version,
                int       /*user*/)
{
   AssertFatal(version == sm_fileFormatVersion,
               "ITRShape::write: Incorrect file version (!)");
   int i;

   // Write out the states...
   s.write(UInt32(m_stateVector.size()));
   for (i = 0; i < m_stateVector.size(); i++) {
      State& state = m_stateVector[i];
      s.write(state.nameIndex);
      s.write(state.lodIndex);
      s.write(state.numLODs);
   }

   // Write out the LODs
   s.write(UInt32(m_lodVector.size()));
   for (i = 0; i < m_lodVector.size(); i++) {
      LOD& lod = m_lodVector[i];
      s.write(lod.minPixels);
      s.write(lod.geometryFileOffset);
      s.write(lod.lightStateIndex);
      s.write(lod.m_linkableFaces);
   }

   // write out the lightStates...
   s.write(UInt32(m_lodLightStates.size()));
   for (i = 0; i < m_lodLightStates.size(); i++) {
      s.write(m_lodLightStates[i].lightFileOffset);
   }

   s.write(m_numLightStates);
   s.write(m_numLightStates * sizeof(UInt32), (void*)m_lightStateNames.address());

   // Write out the filename buffer
   //
   UInt32 size = m_nameBuffer.size();
   s.write(size);
   s.write(size, m_nameBuffer.address());

   s.write(m_materialListOffset);

   s.write(m_linkedInterior);

   return (s.getStatus() == STRM_OK)? Ok: WriteError;
}

Persistent::Base::Error 
ITRShape::read(StreamIO& s,
               int       version,
               int       /*user*/)
{
   AssertFatal(version == sm_fileFormatVersion,
               "ITRShape::read: Unhandlable file version");
   UInt32 temp;
   int i;
   
   // Read in the states...
   s.read(&temp);
   m_stateVector.setSize(temp);
   for (i = 0; i < m_stateVector.size(); i++) {
      State& state = m_stateVector[i];
      s.read(&state.nameIndex);
      s.read(&state.lodIndex);
      s.read(&state.numLODs);
   }

   // Read in the LODs
   //
   s.read(&temp);
   m_lodVector.setSize(temp);
   for (i = 0; i < m_lodVector.size(); i++) {
      LOD& lod = m_lodVector[i];
      s.read(&lod.minPixels);
      s.read(&lod.geometryFileOffset);
      s.read(&lod.lightStateIndex);
      s.read(&lod.m_linkableFaces);
   }

   // read in the lightStates...
   //
   s.read(&temp);
   m_lodLightStates.setSize(temp);
   for (i = 0; i < m_lodLightStates.size(); i++) {
      s.read(&m_lodLightStates[i].lightFileOffset);
   }

   s.read(&m_numLightStates);
   m_lightStateNames.setSize(m_numLightStates);
   s.read(m_numLightStates * sizeof(UInt32), (void*)m_lightStateNames.address());

   UInt32 size;
   s.read(&size);
   m_nameBuffer.setSize(size);
   s.read(size, m_nameBuffer.address());

   s.read(&m_materialListOffset);

   s.read(&m_linkedInterior);

   return (s.getStatus() == STRM_OK)? Ok: ReadError;
}


int ITRShape::version()
{
   return sm_fileFormatVersion;
}
