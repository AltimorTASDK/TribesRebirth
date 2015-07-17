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
#include "zedPersLight.h"

namespace ZedLight {

IMPLEMENT_PERSISTENT(PersLight);
IMPLEMENT_PERSISTENT(PersLight::PersState);
IMPLEMENT_PERSISTENT(PersLightList);

PersLightList::~PersLightList()
{
   clear();
}

void 
PersLightList::clear()
{
   for (int i = 0; i < size(); i++) {
      delete operator[](i);
      operator[](i) = NULL;
   }
   
   Parent::clear();
}

Persistent::Base::Error 
PersLightList::read(StreamIO& io_sio,
                    int       version,
                    int       user)
{
   Int32 newSize;
   io_sio.read(&newSize);
   setSize(newSize);
   
   for (int i = 0; i < size(); i++) {
      operator[](i) = new PersLight;
      operator[](i)->read(io_sio, version, user);
   }

   return ((io_sio.getStatus() == STRM_OK) ? Ok : ReadError);
}

Persistent::Base::Error 
PersLightList::write(StreamIO& io_sio,
                     int       version,
                     int       user)
{
   // Write out the number of lights...
   //
   io_sio.write(Int32(size()));
   for (int i = 0; i < size(); i++) {
      operator[](i)->write(io_sio, version, user);
   }

   return ((io_sio.getStatus() == STRM_OK) ? Ok : WriteError);
}


PersLight::~PersLight()
{
   for (int i = 0; i < state.size(); i++) {
      delete state[i];
      state[i] = NULL;
   }
   state.clear();
}

Persistent::Base::Error 
PersLight::read(StreamIO& io_sio,
                int       version,
                int       user)
{
   // Read in states...
   //
   Int32 numStates;
   io_sio.read(&numStates);
   state.setSize(numStates);
   for (int i = 0; i < numStates; i++) {
      state[i] = new PersState;
      state[i]->read(io_sio, version, user);
   }
   
   io_sio.read(&id);
   io_sio.read(&name);
   io_sio.read(&animationDuration);
   io_sio.read(&flags);
   
   return ((io_sio.getStatus() == STRM_OK) ? Ok : ReadError);
}

Persistent::Base::Error 
PersLight::write(StreamIO& io_sio,
                 int       version,
                 int       user)
{
   // Write out states...
   //
   Int32 numStates = state.size();
   io_sio.write(numStates);
   for (int i = 0; i < numStates; i++) {
      state[i]->write(io_sio, version, user);
   }
   
   io_sio.write(id);
   io_sio.write(name);
   io_sio.write(animationDuration);
   io_sio.write(flags);
   
   return ((io_sio.getStatus() == STRM_OK) ? Ok : WriteError);
}


Persistent::Base::Error 
PersLight::PersState::read(StreamIO& io_sio,
                           int       /*version*/,
                           int       /*user*/)
{
   io_sio.read(&stateDuration);
   io_sio.read(sizeof(color), &color);

   UInt32 numEmitters;
   io_sio.read(&numEmitters);
   m_emitterList.setSize(numEmitters);
   io_sio.read(m_emitterList.size() * sizeof(Vector<Emitter>::value_type),
               m_emitterList.address());

   return ((io_sio.getStatus() == STRM_OK) ? Ok : ReadError);
}

Persistent::Base::Error 
PersLight::PersState::write(StreamIO& io_sio,
                            int       /*version*/,
                            int       /*user*/)
{
   io_sio.write(stateDuration);
   io_sio.write(sizeof(color), &color);

   io_sio.write(UInt32(m_emitterList.size()));
   io_sio.write(m_emitterList.size() * sizeof(Vector<Emitter>::value_type),
                m_emitterList.address());

   return ((io_sio.getStatus() == STRM_OK) ? Ok : WriteError);
}

}; // namespace ZedLight