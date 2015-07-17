//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#ifndef _3DMIMPORT_H_
#define _3DMIMPORT_H_

#include <ml.h>
#include <itrgeometry.h>

//----------------------------------------------------------------------------

class ITRGeometry;


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

namespace ITR3DMImport
{
	extern float scale;
	extern float distancePrecision;
	extern float distancePrecisionSQ;
	extern float normalPrecision;
	extern float snapPrecision;
	extern float textureScale;

   void setMaxMipLevel(const int in_maxMipLevel);
	bool import(const char* file, ITRGeometry*, Vector<UInt32>* volumeMasks);

	extern bool lowDetailInterior;
};


#endif
