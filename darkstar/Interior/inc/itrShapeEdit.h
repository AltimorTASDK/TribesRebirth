//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _ITRSHAPEEDIT_H_
#define _ITRSHAPEEDIT_H_

//Includes
#include <types.h>
#include <tString.h>
#include <persist.h>
#include "itrshape.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class ITRShapeEdit : public Persistent::Base {
  public:
   struct EditLightState {
      String m_lightFileName;
   };

   struct EditLOD {
      String                  m_geometryFileName;
      UInt32                  m_minPixels;
      Vector<EditLightState*> m_pLightStates;

      UInt8    m_linkableFaces;

      ~EditLOD();
      bool  read(StreamIO&);    // Mini-persist functionality
      bool  write(StreamIO&);
   };
   
   struct EditState {
      Vector<EditLOD*> m_pLODVector;
      String           m_stateName;

      ~EditState();
      bool read(StreamIO&);    // Mini-persist functionality
      bool write(StreamIO&);
   };
   
   String m_materialList;

   Int32          m_numLightStates;
   Vector<String> m_lightStateNames;

   bool   m_linkedInterior;

   Vector<EditState*> m_pStates;

   ITRShapeEdit();
   ~ITRShapeEdit();
   
   DECLARE_PERSISTENT(ITRShapeEdit);
   Error read(StreamIO&, int, int);
   Error write(StreamIO&, int, int);

   // Exporting functionality...
   //
  public:
   void exportToITRShape(ITRShape* out_pExportShape);

  private:
   void exportState(ITRShape*        out_pExportShape,
                            ITRShape::State& out_rState,
                            const EditState* in_pEditState);
   void exportLOD(ITRShape*      out_pExportShape,
                          ITRShape::LOD& out_rLOD,
                          const EditLOD* in_pEditLOD);
};


class ITRBuildOptions : public Persistent::Base {
  public:
   float    m_geometryScale;
   
   float    m_pointSnapPrecision;
   float    m_planeNormalPrecision;
   float    m_planeDistancePrecision;
   
   float    m_textureScale;
   
   bool     m_lowDetail;
   
   UInt32   m_maximumTexMip;

   ITRBuildOptions();

   DECLARE_PERSISTENT(ITRBuildOptions);
   Error read(StreamIO&, int, int);
   Error write(StreamIO&, int, int);
};


class ITRLightOptions : public Persistent::Base {
  public:
   enum LightingType {
      Normal      = 0,
      Default     = 1,
      TestPattern = 2
   } m_lightingType;
   
   float    m_geometryScale;
   UInt32   m_lightScale;

   bool     m_useNormals;
   UInt32   m_emissionQuantumNumber;
   
   bool     m_useMaterialProperties;
   
   ITRLightOptions();
   
   DECLARE_PERSISTENT(ITRLightOptions);
   Error read(StreamIO&, int, int);
   Error write(StreamIO&, int, int);
};

class ITRLightOptions2 : public ITRLightOptions 
{
   public:
   ColorF   m_ambientIntensity;
   
   ITRLightOptions2();
   
   DECLARE_PERSISTENT(ITRLightOptions2);
   Error read(StreamIO&, int, int);
   Error write(StreamIO&, int, int);
};

//---------------------------------------------------------------------

class ITRLightingOptions : public Persistent::VersionedBase
{
   public:
      enum LightingType {
         Normal      = 0,
         Default     = 1,
         TestPattern = 2
      } m_lightingType;
      
      float    m_geometryScale;
      UInt32   m_lightScale;
      bool     m_useNormals;
      UInt32   m_emissionQuantumNumber;
      bool     m_useMaterialProperties;
      ColorF   m_ambientIntensity;
      bool     m_applyAmbientOutside;
      
      ITRLightingOptions();
      
      DECLARE_PERSISTENT(ITRLightingOptions);
      int version();
      Persistent::Base::Error read(StreamIO&, int version, int user);
      Persistent::Base::Error write(StreamIO&, int version, int user);
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_ITRSHAPEEDIT_H_
