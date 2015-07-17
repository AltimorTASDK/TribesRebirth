#include "sim.h"
#include "simPalette.h"
#include "simGame.h"
#include "inspect.h"
#include "editor.strings.h"
#include "stringTable.h"
#include "netGhostManager.h"

SimPalette::SimPalette()
{
   fileName = stringTable.insert("");
   netFlags.set(SimNetObject::ScopeAlways);
   netFlags.set(SimNetObject::Ghostable);
}

SimPalette::~SimPalette()
{
}

DWORD SimPalette::packUpdate(Net::GhostManager* /*gm*/, DWORD /*mask*/, BitStream *stream)
{
   stream->writeString(fileName);
   return 0;
}

void SimPalette::unpackUpdate(Net::GhostManager* /*gm*/, BitStream *stream)
{
   fileName = stream->readSTString();
   open(fileName);
}

void SimPalette::setInGame()
{
   SimGame::get()->getWorld()->setPalette(palette);
}

bool SimPalette::open(const char *file, bool /*forceLoad*/)
{
   if(manager)
   {
      if(fileName != file)
         fileName = stringTable.insert(file);
      palette = SimResource::get(manager)->load(fileName);

      return bool(palette);
   }
   return false;
}

bool SimPalette::onAdd()
{
   if(!Parent::onAdd())
      return false;
   if(fileName[0]) {
      bool openSuccess = open(fileName);
      if (isGhost() && openSuccess == false)
         Net::setLastError(avar("You don't have the custom palette needed for this server.  Check the Server Info for a possible download site.  Missing palette: %s", fileName));

      return openSuccess;
   }
   return true;
}

bool SimPalette::processArguments(int argc, const char **argv)
{
   bool forceLoad = (argc == 2 && !stricmp(argv[1],"true") );
   if(argc == 1 || forceLoad )
      return open(argv[0], forceLoad);
   return false;
}

void SimPalette::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   Inspect_Str palName;

   insp->read(IDITG_PALETTE_NAME, palName);
   
   if(strcmp(palName, fileName))
   {
      // force a net update...
      setMaskBits(1);
      open(palName);
   }
}

void SimPalette::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);
   insp->write(IDITG_PALETTE_NAME, (char *)fileName);
}

void SimPalette::initPersistFields()
{
   addField("fileName", TypeString, Offset(fileName, SimPalette));
}

Persistent::Base::Error SimPalette::write( StreamIO &sio, int a, int b)
{
   Parent::write(sio, a, b);
   sio.writeString(fileName);

	return (sio.getStatus() == STRM_OK)? Ok: WriteError;
}

Persistent::Base::Error SimPalette::read( StreamIO &sio, int a, int b)
{
   Parent::read(sio, a, b);
   fileName = sio.readSTString();

	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}
