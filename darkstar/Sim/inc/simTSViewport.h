//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#ifndef _SIMTSVIEWPORT_H_
#define _SIMTSVIEWPORT_H_

#include <simBase.h>
#include <simEv.h>

class SimCameraSet: public SimSet
{
public:
	SimCameraSet() { id = SimCameraSetId; }
};

class SimCameraMountSet: public SimSet
{
public:
	SimCameraMountSet() { id = SimCameraMountSetId; }
};

#endif
