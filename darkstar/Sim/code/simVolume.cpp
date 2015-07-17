//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <sim.h>
#include "talgorithm.h"
#include "simVolume.h"
#include "inspect.h"          
#include "editor.strings.h"
#include "simPersistTags.h"
#include "simResource.h"
#include "stringTable.h"
#include "netGhostManager.h"
#pragma option -Jg

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

SimVolume::SimVolume()
{
   opened = false;
   fileName = stringTable.insert("");

   netFlags.set(SimNetObject::ScopeAlways);
   netFlags.set(SimNetObject::Ghostable);
}

SimVolume::~SimVolume()
{
}

 
DWORD SimVolume::packUpdate(Net::GhostManager* /*gm*/, DWORD /*mask*/, BitStream *stream)
{
   // the only thing that changes on a SimVolume is the file name
   stream->writeString(fileName);
   return 0;
}

void SimVolume::unpackUpdate(Net::GhostManager* /*gm*/, BitStream *stream)
{
   close();
   fileName = stream->readSTString();
   open(fileName);
}

//------------------------------------------------------------------------------

bool SimVolume::open(const char* filename)
{
   if(manager)
   {
   	if(opened)
         SimResource::get(manager)->removeVolume(fileName);
   	if(filename != fileName)
         fileName = stringTable.insert(filename);

      opened = SimResource::get(manager)->addVolume(fileName);
      return opened;
   }
   return false;
}


//------------------------------------------------------------------------------

bool SimVolume::close()
{
   if(manager)
   {
      if(opened)
      {
         SimResource::get(manager)->removeVolume(fileName);
         opened = false;
      }
   }
	return true;
}


//------------------------------------------------------------------------------

void SimVolume::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   Inspect_Str volName;
   insp->read(IDITG_VOLUME_NAME, volName);
   
   if(strcmp(volName, fileName))
   {
      // force a net update
      setMaskBits(1);
      close();
      open(volName);
   }
}

void SimVolume::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);
   insp->write(IDITG_VOLUME_NAME, (char *)fileName);
}

//------------------------------------------------------------------------------

bool SimVolume::onAdd()
{
   if(!Parent::onAdd())
      return false;
   if(fileName[0]) {
      bool openSuccess = open(fileName);

      if (isGhost() == true && openSuccess == false)
         Net::setLastError(avar("You don't have a custom volume file (or mission) needed for this server.  Check the Server Info for a possible download site.  Missing Volume: %s", fileName));

      return openSuccess;
   }
   return true;
}

void SimVolume::onRemove()
{
   close();
   Parent::onRemove();
}

bool SimVolume::processArguments(int argc, const char **argv)
{
	// Volumes constructed from the console are not ghosted by default.
   netFlags.clear(SimNetObject::Ghostable | SimNetObject::ScopeAlways);

   bool ret = false;
   if(argc >= 1)
   {
      ret = open(argv[0]);
      if(argc == 2)
      {
         if(argv[1][0] == 't' || argv[1][0] == 'T' || argv[1][0] == '1')
            netFlags.set(SimNetObject::Ghostable | SimNetObject::ScopeAlways);
      }
   }
   if(!netFlags.test(SimNetObject::Ghostable))
      removeFromSet(SimGhostAlwaysSetId);
   return ret;
}


//------------------------------------------------------------------------------

IMPLEMENT_PERSISTENT_TAGS( SimVolume, FOURCC('S','V','o','l'), SimVolumePersTag);

void SimVolume::initPersistFields()
{
   addField("fileName", TypeString, Offset(fileName, SimVolume));
}

Persistent::Base::Error SimVolume::write( StreamIO &sio, int a, int b)
{
   Parent::write(sio, a, b);
   sio.writeString(fileName);

	return (sio.getStatus() == STRM_OK)? Ok: WriteError;
}

Persistent::Base::Error SimVolume::read( StreamIO &sio, int a, int b)
{
   Parent::read(sio, a, b);
   fileName = sio.readSTString();

	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}
