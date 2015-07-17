//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#ifndef _ITRBSP_H_
#define _ITRBSP_H_


//----------------------------------------------------------------------------

class ITRGeometry;

namespace ITRBSPBuild
{
	extern float distancePrecision;
	extern float normalPrecision;

	//
	bool buildTree(ITRGeometry* geometry,Vector<UInt32>* volumeMasks, bool isLinked);
};


#endif
