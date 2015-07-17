#include <feardcl.h>
#include <elevator.h>
#include <datablockmanager.h>

Elevator::Elevator() : MoveableBase()
{
}

Elevator::~Elevator()
{
}

int Elevator::getDatGroup()
{
   return (DataBlockManager::ElevatorDataType);
}

IMPLEMENT_PERSISTENT_TAGS(Elevator, FOURCC('E', 'L', 'E', 'V'), ElevatorPersTag);

Persistent::Base::Error Elevator::read(StreamIO &sio, int iVer, int iUsr)
{
   return (Parent::read(sio, iVer, iUsr));
}

Persistent::Base::Error Elevator::write(StreamIO &sio, int iVer, int iUsr)
{
   return (Parent::write(sio, iVer, iUsr));
}
