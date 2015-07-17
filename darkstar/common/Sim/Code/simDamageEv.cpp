
#include <base.h>
#include <sim.h>
#include <SimCollision.h>

#include "esfEvDcl.h"
#include "SimDamageEv.h"

IMPLEMENT_PERSISTENT(SimDamageEvent);

Persistent::Base::Error SimDamageEvent::write( StreamIO &sio, int a, int b )
{
   SimEvent::write( sio, a, b );
	
	sio.write (damageValue);
	sio.write (sizeof (Point3F), (Int8 *)&mv);
	sio.write (sizeof (Point3F), (Int8 *)&hv);

	return (sio.getStatus() == STRM_OK)? Ok: WriteError;
}

Persistent::Base::Error SimDamageEvent::read( StreamIO &sio, int a, int b )
{
   SimEvent::read( sio, a, b );
	
	sio.read (&damageValue);
	sio.read (sizeof (mv), (Int8 *)&mv);
	sio.read (sizeof (mv), (Int8 *)&hv);
	
	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}
