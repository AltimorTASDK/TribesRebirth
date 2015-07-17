//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#ifndef _TSIMPORT_H_
#define _TSIMPORT_H_

#include <ml.h>
#include <ts.h>
#include <ts_mesh.h>
#include <itrgeometry.h>

//----------------------------------------------------------------------------

class TSMesh;
class ITRGeometry;


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

namespace TSShapeImport
{
	extern float distancePrecision;
	extern float normalPrecision;
	extern float snapPrecision;

	bool import(const char* file, ITRGeometry*);
	bool import(TSShape* shape, ITRGeometry*);
};


#endif
