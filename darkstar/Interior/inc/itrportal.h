//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#ifndef _ITRPORTAL_H_
#define _ITRPORTAL_H_

//----------------------------------------------------------------------------

class ITRGeometry;

namespace ITRPortal
{
	extern bool portalClipping;
	extern float boxExpansion;
	extern float distancePrecision;
	extern float normalPrecision;

	//
	void buildPVS(ITRGeometry* geometry);
};




#endif
