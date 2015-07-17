//----------------------------------------------------------------------------

//	$Workfile:   GRDCAM.H  $
//	$Version$
//	$Revision:   1.17  $
//	$Date:   12 Jan 1996 10:11:34  $

//  VO:timg

//----------------------------------------------------------------------------

#ifndef _GRDCAM_H_
#define _GRDCAM_H_

#include <base.h>
#include <ml.h>
#include <ts.h>
#include <tvector.h>
#include <viewcone.h>

#include "grdrange.h"


//----------------------------------------------------------------------------

struct Raster;

class DLLAPI GridEdgeTable
{
public:
	enum Constants {
		MaxDetail = 10,
	};

	typedef GridRange<int> Edge;
	typedef Vector<Edge> EdgeVector;

	struct Table {
		int starty;
		EdgeVector edges;
		const Edge& getRange(int y) const {
			return edges[y - starty];
		}
		const bool isVisible(const Point2I& p) const {
			int yoff = p.y - starty;
			return (yoff >= 0 && yoff < edges.size())?
				edges[yoff].inside(p.x): false;
		}
	};
private:
	//
	struct DetailData {
		Table edgeTable;			// Rasterized viewcone
		Point2I camera;
		Box2I boundingBox;
	};
	DetailData details[MaxDetail];

	// Grid File data
	int groundScale;
	int scanDetailLevel;
	float visibleDistance;
	GridRange<float> groundHeight;// Height range (used by viewcone)

	// Methods
	void trapezoid(int count,Raster& lx,Raster& rx);
	void rasterize(const Poly3I& poly);
	void buildEdgeTables(const Poly3I&);
	void setDetailCamera(Point3F& camPos);
public:
	GridEdgeTable();
	~GridEdgeTable();

	void buildTables(const TSCamera& cam);
	void buildTablesOrtho(const TSOrthographicCamera& cam);
	void setRasterData(int scale,int scanLevel,float visdist,GridRange<float> height);
	bool isVisible(int detail,const Point2I& p) const;
	const Table& getEdgeTable(int detail);
};


inline bool GridEdgeTable::isVisible(int detail,const Point2I& p) const
{
	return details[detail].edgeTable.isVisible(p);
}

inline const GridEdgeTable::Table& GridEdgeTable::getEdgeTable(int detail)
{
	return details[detail].edgeTable;
}


#endif
