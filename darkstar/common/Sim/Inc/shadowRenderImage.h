#ifndef _SHADOWRENDERIMAGE_H_
#define _SHADOWRENDERIMAGE_H_

#include "baseshadowrenderImage.h"

class SimInterior;
class SimInteriorShape;

class ShadowRenderImage : public BaseShadowRenderImage
{
protected:
   void getUnknownPolys(SimContainer *);
	void getItrPolys(SimInterior *);
	void getItrPolys(SimInteriorShape *);
};

#endif