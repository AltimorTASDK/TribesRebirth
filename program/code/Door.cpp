#include <door.h>
#include <feardcl.h>
#include <datablockmanager.h>
#include <console.h>

Door::Door() : MoveableBase()
{
}

Door::~Door()
{
}

int Door::getDatGroup()
{
   return (DataBlockManager::DoorDataType);
}

IMPLEMENT_PERSISTENT_TAGS(Door, FOURCC('D', 'O', 'O', 'R'), DoorPersTag);

Persistent::Base::Error Door::read(StreamIO &sio, int iVer, int iUsr)
{
   return (Parent::read(sio, iVer, iUsr));
}

Persistent::Base::Error Door::write(StreamIO &sio, int iVer, int iUsr)
{
   return (Parent::write(sio, iVer, iUsr));
}

void Door::onFirst()
{
   if (const char *lpcszScript = scriptName("onClose"))
   {
      Console->executef(3, lpcszScript, scriptThis(), getId());
   }
}

void Door::onLast()
{
   if (const char *lpcszScript = scriptName("onOpen"))
   {
      Console->executef(3, lpcszScript, scriptThis(), getId());
   }
}
