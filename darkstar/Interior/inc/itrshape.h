//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _ITRSHAPENEW_H_
#define _ITRSHAPENEW_H_

//Includes
#include <stdlib.h>
#include <base.h>
#include <tvector.h>
#include <tMap.h>
#include <persist.h>
#include <m_box.h>
#include <tString.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class ITRInstance;
class MissionLighting;

class ITRShape : public Persistent::VersionedBase {
   // Class constants
   //
   static int sm_fileFormatVersion;

   // Friends...
   //
   friend class ITRInstance;
   friend class ITRShapeEdit;
   friend class ITRInstance;
   friend class MissionLighting;

  public:
   static const char * sm_pShapeFileExtension;

  public:
   struct State {
      UInt32 nameIndex; // index into NameBuffer of state name
      UInt32 lodIndex;  // index of first LOD structure
      UInt32 numLODs;   // number of LODs in this state
   };
   struct LOD {
      UInt32 minPixels;          // Minimum projected pixels for this state
      UInt32 geometryFileOffset; // offset of geometryFileName in nameBuffer
      
      UInt32 lightStateIndex;

      UInt32 m_linkableFaces;
      
      UInt32 getLinkableFaces() const;
   };
   struct LODLightState {
      UInt32 lightFileOffset;
   };

   typedef Vector<State>         StateVector;
   typedef Vector<LOD>           LODVector;
   
   typedef Vector<LODLightState> LODLightStateVector;
   typedef Vector<char>          NameBuffer;
   
  protected:
   StateVector         m_stateVector;
   LODVector           m_lodVector;

   LODLightStateVector m_lodLightStates;

   Int32               m_numLightStates;
   Vector<UInt32>      m_lightStateNames;

   bool     m_linkedInterior;
   Int32    m_materialListOffset;
   UInt32 addFileName(const char* in_pFileName);

  public:
   NameBuffer m_nameBuffer;

  protected:
   void sortLODs();
   static int _USERENTRY compareLOD(const void* in_pElem1,
                                    const void* in_pElem2);

  public:
   static const UInt32 sm_allFacesLinkable;

   ITRShape();
   virtual ~ITRShape();

   UInt32  getNumStates() const { return m_stateVector.size(); }
   State&  getState(const UInt32 in_stateNum);
   UInt32  getNumLightStates() const { return m_numLightStates; }
   const char* getFileName(const UInt32 in_fileOffset) { return &m_nameBuffer[in_fileOffset]; }
   const char* getMaterialListFileName() const { return &m_nameBuffer[m_materialListOffset]; }
   bool isLinked() const;

   UInt32 findDetail(const State* in_pState,
                     const UInt32 in_pixels) const;

   const char* getName(const int in_index) const
   {
      return &m_nameBuffer[in_index];
   }
   const char* getLightStateName(const int in_lightStateIndex)
   {
      return getName(m_lightStateNames[in_lightStateIndex]);
   }

   // Persistent IO
   DECLARE_PERSISTENT(ITRShape);
   Error read( StreamIO &, int version, int );
   Error write( StreamIO &, int version, int );
   int   version();
};

inline bool
ITRShape::isLinked() const
{
   return m_linkedInterior;
}

inline UInt32
ITRShape::LOD::getLinkableFaces() const
{
   return m_linkableFaces;
}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_ITRSHAPENEW_H_

