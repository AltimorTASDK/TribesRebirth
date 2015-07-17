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
#include "itrShapeEdit.h"


namespace {

// Rather a simple (and fragile) function to append a new file extension.  Note
//  that the extension must include the '.'
//
void
reextendFileName(const String& in_rFileName,
                 const String& in_rNewExtension,
                 String&       out_rNewFileName)
{
   out_rNewFileName = in_rFileName;
   String::iterator itr;
   for (itr = out_rNewFileName.end(); itr != out_rNewFileName.begin(); --itr) {
      if (*itr == '.')
         break;
   }
   if (itr == out_rNewFileName.begin() ||
       (out_rNewFileName.end() - itr) < in_rNewExtension.length()) {
      // There was no extension, or not enough room for the new one...
      //
      char tmpBuf[512];
      strcpy(tmpBuf, out_rNewFileName.c_str());
      strcat(tmpBuf, in_rNewExtension.c_str());
      out_rNewFileName = tmpBuf;
   } else {
      char* pExt = itr;
      strcpy(pExt, in_rNewExtension.c_str());
   }
}

}; // namespace {}



IMPLEMENT_PERSISTENT(ITRShapeEdit);

ITRShapeEdit::ITRShapeEdit()
{
   
}


ITRShapeEdit::~ITRShapeEdit()
{
   for (int i = 0; i < m_pStates.size(); i++) {
      delete m_pStates[i];
      m_pStates[i] = NULL;
   }
   m_pStates.clear();
}


ITRShapeEdit::EditLOD::~EditLOD()
{
   for (int i = 0; i < m_pLightStates.size(); i++) {
      delete m_pLightStates[i];
      m_pLightStates[i] = NULL;
   }
   m_pLightStates.clear();
}


ITRShapeEdit::EditState::~EditState()
{
   for (int i = 0; i < m_pLODVector.size(); i++) {
      delete m_pLODVector[i];
      m_pLODVector[i] = NULL;
   }
   m_pLODVector.clear();
}

   
Persistent::Base::Error 
ITRShapeEdit::read(StreamIO& io_sio,
                   int       /*in_version*/,
                   int       /*in_user*/)
{
   // Get the shape global params...
   //
   io_sio.read(&m_materialList);
   io_sio.read(&m_numLightStates);
   io_sio.read(&m_linkedInterior);

   // Read the light state names
   //
   int i;
   for (i = 0; i < m_numLightStates; i++) {
      String temp;
      io_sio.read(&temp);

      m_lightStateNames.increment();
      new (&m_lightStateNames.last()) String;
      m_lightStateNames.last() = temp;
   }

   // Get the states, lods and lightstates...
   //
   Int32 numStates;
   io_sio.read(&numStates);
   for (i = 0; i < numStates; i++) {
      EditState* pEditState = new EditState;
      pEditState->read(io_sio);
      m_pStates.push_back(pEditState);
   }
   
   return ((io_sio.getStatus() == STRM_OK) ? Ok : ReadError);
}

Persistent::Base::Error 
ITRShapeEdit::write(StreamIO& io_sio,
                    int       /*in_version*/,
                    int       /*in_user*/)
{
   // Get the shape global params...
   //
   io_sio.write(m_materialList);
   io_sio.write(m_numLightStates);
   io_sio.write(m_linkedInterior);

   // Write out the light state names
   //
   AssertFatal(m_lightStateNames.size() == m_numLightStates,
               "Mismatch between lightstate size and name vector size");
   int i;
   for (i = 0; i < m_lightStateNames.size(); i++) {
      io_sio.write(m_lightStateNames[i]);
   }

   // Get the states, lods and lightstates...
   //
   Int32 numStates = m_pStates.size();
   io_sio.write(numStates);
   for (i = 0; i < numStates; i++) {
      EditState* pEditState = m_pStates[i];
      pEditState->write(io_sio);
   }

   return ((io_sio.getStatus() == STRM_OK) ? Ok : WriteError);
}


bool 
ITRShapeEdit::EditState::read(StreamIO& io_sio)
{
   io_sio.read(&m_stateName);

   Int32 numLODs;
   io_sio.read(&numLODs);
   for (int i = 0; i < numLODs; i++) {
      ITRShapeEdit::EditLOD* pLOD = new ITRShapeEdit::EditLOD;
      pLOD->read(io_sio);
      m_pLODVector.push_back(pLOD);
   }

   return (io_sio.getStatus() == STRM_OK);
}


bool 
ITRShapeEdit::EditState::write(StreamIO& io_sio)
{
   io_sio.write(m_stateName);

   io_sio.write(Int32(m_pLODVector.size()));
   for (int i = 0; i < m_pLODVector.size(); i++) {
      ITRShapeEdit::EditLOD* pLOD = m_pLODVector[i];
      pLOD->write(io_sio);
   }

   return (io_sio.getStatus() == STRM_OK);
}


bool 
ITRShapeEdit::EditLOD::read(StreamIO& io_sio)
{
   io_sio.read(&m_geometryFileName);
   io_sio.read(&m_minPixels);

   Int32 numLightStates;
   io_sio.read(&numLightStates);
   for (int i = 0; i < numLightStates; i++) {
      ITRShapeEdit::EditLightState* pLS = new ITRShapeEdit::EditLightState;
      io_sio.read(&pLS->m_lightFileName);
      m_pLightStates.push_back(pLS);
   }

   io_sio.read(&m_linkableFaces);

   return (io_sio.getStatus() == STRM_OK);
}


bool 
ITRShapeEdit::EditLOD::write(StreamIO& io_sio)
{
   io_sio.write(m_geometryFileName);
   io_sio.write(m_minPixels);

   Int32 numLightStates = m_pLightStates.size();
   io_sio.write(numLightStates);
   for (int i = 0; i < numLightStates; i++) {
      ITRShapeEdit::EditLightState* pLS = m_pLightStates[i];
      io_sio.write(pLS->m_lightFileName);
   }

   io_sio.write(m_linkableFaces);

   return (io_sio.getStatus() == STRM_OK);
}

//------------------------------------------------------------------------------

void 
ITRShapeEdit::exportToITRShape(ITRShape* out_pExportShape)
{
   // Match up the state sizes in ITRShape...
   //
   out_pExportShape->m_numLightStates = m_numLightStates;
   out_pExportShape->m_stateVector.setSize(m_pStates.size());
   for (int i = 0; i < m_pStates.size(); i++) {
      exportState(out_pExportShape,
                  out_pExportShape->m_stateVector[i],
                  m_pStates[i]);
   }
   out_pExportShape->sortLODs();
   
   out_pExportShape->m_materialListOffset =
      out_pExportShape->addFileName(m_materialList.c_str());

   out_pExportShape->m_linkedInterior = m_linkedInterior;
   
   // Light state names...
   //
   out_pExportShape->m_lightStateNames.setSize(m_numLightStates);
   for (int j = 0; j < m_numLightStates; j++) {
      out_pExportShape->m_lightStateNames[j] =
         out_pExportShape->addFileName(m_lightStateNames[j].c_str());
   }
}

void 
ITRShapeEdit::exportState(ITRShape*        out_pExportShape,
                          ITRShape::State& out_rState,
                          const EditState* in_pEditState)
{
   // NOTE: exportState does NOT set the bounding box or radius information...
   //
   out_rState.nameIndex = out_pExportShape->addFileName(in_pEditState->m_stateName.c_str());
   out_rState.lodIndex  = out_pExportShape->m_lodVector.size();
   out_rState.numLODs   = in_pEditState->m_pLODVector.size();
   
   out_pExportShape->m_lodVector.setSize(out_rState.lodIndex + out_rState.numLODs);
   for (UInt32 i = 0; i < out_rState.numLODs; i++) {
      EditLOD* exportFrom = in_pEditState->m_pLODVector[i];
      ITRShape::LOD& exportTo =
         out_pExportShape->m_lodVector[out_rState.lodIndex + i];

      exportLOD(out_pExportShape,
                exportTo,
                exportFrom);
   }
}

void 
ITRShapeEdit::exportLOD(ITRShape*      out_pExportShape,
                        ITRShape::LOD& out_rLOD,
                        const EditLOD* in_pEditLOD)
{
   // Note: Exports light states for the LOD as well...
   //
   out_rLOD.minPixels = in_pEditLOD->m_minPixels;

   String renamedLOD;
   reextendFileName(in_pEditLOD->m_geometryFileName,
                    String(".dig"),
                    renamedLOD);

   out_rLOD.geometryFileOffset = 
      out_pExportShape->addFileName(renamedLOD.c_str());
   
   out_rLOD.lightStateIndex = out_pExportShape->m_lodLightStates.size();
   out_pExportShape->m_lodLightStates.setSize(out_rLOD.lightStateIndex + m_numLightStates);
   for (Int32 i = 0; i < m_numLightStates; i++) {
      ITRShape::LODLightState& ls =
         out_pExportShape->m_lodLightStates[out_rLOD.lightStateIndex + i];

      String renamedLS;
      reextendFileName(in_pEditLOD->m_pLightStates[i]->m_lightFileName,
                       String(".dil"),
                       renamedLS);

      ls.lightFileOffset =
         out_pExportShape->addFileName(renamedLS.c_str());
   }
   
   out_rLOD.m_linkableFaces  = in_pEditLOD->m_linkableFaces;
}


IMPLEMENT_PERSISTENT(ITRBuildOptions);

// Outdated - use 'ITRLightingOptions' instead
IMPLEMENT_PERSISTENT(ITRLightOptions);
IMPLEMENT_PERSISTENT(ITRLightOptions2);

// Outdated - use 'ITRLightingOptions' instead
ITRBuildOptions::ITRBuildOptions()
 : m_geometryScale(1.0f),
   m_pointSnapPrecision(0.0f),
   m_planeNormalPrecision(0.0001f),
   m_planeDistancePrecision(0.01f),
   m_textureScale(1.0f),
   m_lowDetail(false),
   m_maximumTexMip(0)
{
   //
}

// Outdated - use 'ITRLightingOptions' instead
Persistent::Base::Error 
ITRBuildOptions::write(StreamIO& io_sio,
                       int       /*version*/,
                       int       /*user*/)
{
   io_sio.write(m_geometryScale);
   io_sio.write(m_pointSnapPrecision);
   io_sio.write(m_planeNormalPrecision);
   io_sio.write(m_planeDistancePrecision);
   io_sio.write(m_textureScale);
   io_sio.write(m_lowDetail);
   io_sio.write(m_maximumTexMip);

   return ((io_sio.getStatus() == STRM_OK) ? Ok : WriteError);
}

// Outdated - use 'ITRLightingOptions' instead
Persistent::Base::Error 
ITRBuildOptions::read(StreamIO& io_sio,
                      int       /*version*/,
                      int       /*user*/)
{
   io_sio.read(&m_geometryScale);
   io_sio.read(&m_pointSnapPrecision);
   io_sio.read(&m_planeNormalPrecision);
   io_sio.read(&m_planeDistancePrecision);
   io_sio.read(&m_textureScale);
   io_sio.read(&m_lowDetail);
   io_sio.read(&m_maximumTexMip);

   return ((io_sio.getStatus() == STRM_OK) ? Ok : WriteError);
}


// Outdated - use 'ITRLightingOptions' instead
ITRLightOptions::ITRLightOptions()
 : m_lightingType(Normal),
   m_geometryScale(1.0f),
   m_lightScale(4),
   m_useNormals(false),
   m_emissionQuantumNumber(100),
   m_useMaterialProperties(false)
{
   //
}

// Outdated - use 'ITRLightingOptions' instead
Persistent::Base::Error 
ITRLightOptions::write(StreamIO& io_sio,
                       int       /*version*/,
                       int       /*user*/)
{
   io_sio.write(UInt32(m_lightingType));
   io_sio.write(m_geometryScale);
   io_sio.write(m_lightScale);
   io_sio.write(m_useNormals);
   io_sio.write(m_emissionQuantumNumber);
   io_sio.write(m_useMaterialProperties);

   return ((io_sio.getStatus() == STRM_OK) ? Ok : WriteError);
}

// Outdated - use 'ITRLightingOptions' instead
Persistent::Base::Error 
ITRLightOptions::read(StreamIO& io_sio,
                      int       /*version*/,
                      int       /*user*/)
{
   UInt32 temp;
   io_sio.read(&temp);
   m_lightingType = LightingType(temp);

   io_sio.read(&m_geometryScale);
   io_sio.read(&m_lightScale);
   io_sio.read(&m_useNormals);
   io_sio.read(&m_emissionQuantumNumber);
   io_sio.read(&m_useMaterialProperties);

   return ((io_sio.getStatus() == STRM_OK) ? Ok : WriteError);
}

// add ambient light to the options
// Outdated - use 'ITRLightingOptions' instead
ITRLightOptions2::ITRLightOptions2() :
   m_ambientIntensity( 0.f, 0.f, 0.f )
{
   //
}

// Outdated - use 'ITRLightingOptions' instead
Persistent::Base::Error ITRLightOptions2::write( StreamIO & io_sio,
   int /*version*/, int /*user*/ )
{
   io_sio.write(UInt32(m_lightingType));
   io_sio.write(m_geometryScale);
   io_sio.write(m_lightScale);
   io_sio.write(m_useNormals);
   io_sio.write(m_emissionQuantumNumber);
   io_sio.write(m_useMaterialProperties);

   // ambient
   io_sio.write(m_ambientIntensity.red);
   io_sio.write(m_ambientIntensity.green);
   io_sio.write(m_ambientIntensity.blue);
   
   return ((io_sio.getStatus() == STRM_OK) ? Ok : WriteError);
}

// Outdated - use 'ITRLightingOptions' instead
Persistent::Base::Error ITRLightOptions2::read( StreamIO & io_sio,
   int /*version*/, int /*user*/ )
{
   UInt32 temp;
   io_sio.read(&temp);
   m_lightingType = LightingType(temp);

   io_sio.read(&m_geometryScale);
   io_sio.read(&m_lightScale);
   io_sio.read(&m_useNormals);
   io_sio.read(&m_emissionQuantumNumber);
   io_sio.read(&m_useMaterialProperties);

   // ambient
   io_sio.read(&m_ambientIntensity.red);
   io_sio.read(&m_ambientIntensity.green);
   io_sio.read(&m_ambientIntensity.blue);
   
   return ((io_sio.getStatus() == STRM_OK) ? Ok : WriteError);
}

//---------------------------------------------------------------------
// zed outputs ITRLightingOptions... ITRLightOptions(2) should be 
// phased out of existence at some point with this versioned
// implementation

IMPLEMENT_PERSISTENT(ITRLightingOptions);

//---------------------------------------------------------------------

ITRLightingOptions::ITRLightingOptions()
 : m_lightingType(Normal),
   m_geometryScale(1.0f),
   m_lightScale(4),
   m_useNormals(false),
   m_emissionQuantumNumber(100),
   m_useMaterialProperties(false),
   m_ambientIntensity(0.f,0.f,0.f),
   m_applyAmbientOutside(true)
{
}

//---------------------------------------------------------------------

Persistent::Base::Error ITRLightingOptions::read( StreamIO & sio, 
   int /*version*/, int /*user*/ )
{
   UInt32 temp;
   sio.read( &temp );
   m_lightingType = LightingType( temp );

   sio.read( &m_geometryScale );
   sio.read( &m_lightScale );
   sio.read( &m_useNormals );
   sio.read( &m_emissionQuantumNumber );
   sio.read( &m_useMaterialProperties );

   // ambient
   sio.read( &m_ambientIntensity.red );
   sio.read( &m_ambientIntensity.green );
   sio.read( &m_ambientIntensity.blue );
   sio.read( &m_applyAmbientOutside );
   
   return( ( sio.getStatus() == STRM_OK ) ? Ok : WriteError );
   
}

//---------------------------------------------------------------------

Persistent::Base::Error ITRLightingOptions::write( StreamIO & sio, int, int )
{
   sio.write( UInt32( m_lightingType ) );
   sio.write( m_geometryScale );
   sio.write( m_lightScale );
   sio.write( m_useNormals );
   sio.write( m_emissionQuantumNumber );
   sio.write(m_useMaterialProperties );

   // ambient
   sio.write( m_ambientIntensity.red );
   sio.write( m_ambientIntensity.green );
   sio.write( m_ambientIntensity.blue );
   sio.write( m_applyAmbientOutside );
   
   return( ( sio.getStatus() == STRM_OK ) ? Ok : WriteError );
}

//---------------------------------------------------------------------

int ITRLightingOptions::version()
{
   // 0:  initial version
   return( 0 );
}




