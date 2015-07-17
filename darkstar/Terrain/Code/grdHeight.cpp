//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#include "grdBlock.h"
#include "grdHeight.h"


//----------------------------------------------------------------------------

void GridHeight::getNormal(const Point2F& off,Vector3F* normal)
{
	switch (split) {
		case Split45:
			(getFace45(off) == FaceA)?
				getNormal45a(normal): getNormal45b(normal);
			break;
		case Split135:
			(getFace135(off) == FaceA)?
				getNormal135a(normal): getNormal135b(normal);
			break;
	}
}

void GridHeight::getNormal45a(Vector3F* v1)
{
	v1->x = heights[dy].height - heights[dx+dy].height;
	v1->y = heights[0].height - heights[dy].height;
	v1->z = width;
}

void GridHeight::getNormal45b(Vector3F* v2)
{
	v2->x = heights[0].height - heights[dx].height;
	v2->y = heights[dx].height - heights[dx+dy].height;
	v2->z = width;
}

void GridHeight::getNormal135a(Vector3F* v1)
{
	v1->x = heights[0].height - heights[dx].height;
	v1->y = heights[0].height - heights[dy].height;
	v1->z = width;
}

void GridHeight::getNormal135b(Vector3F* v2)
{
	v2->x = heights[dy].height - heights[dx+dy].height;
	v2->y = heights[dx].height - heights[dx+dy].height;
	v2->z = width;
}


//----------------------------------------------------------------------------

float GridHeight::getHeight(const Point2F& off)
{
	float height;
	switch (split) {
		case Split45:
			height = (getFace45(off) == FaceA)?
				getHeight45a(off): getHeight45b(off);
			break;
		case Split135:
			height = (getFace135(off) == FaceA)?
				getHeight135a(off): getHeight135b(off);
			break;
	}
	return height;
}

float GridHeight::getHeight45a(const Point2F& off)
{
	return heights[dy].height +
		((heights[dx+dy].height - heights[dy].height) * off.x +
		(heights[0].height - heights[dy].height) * (width - off.y)) / width;
}

float GridHeight::getHeight45b(const Point2F& off)
{
	return heights[dx].height + 
		((heights[0].height - heights[dx].height) * (width - off.x) +
		(heights[dx+dy].height - heights[dx].height) * off.y) / width;
}

float GridHeight::getHeight135a(const Point2F& off)
{
	return heights[0].height + 
		((heights[dx].height - heights[0].height) * off.x +
		(heights[dy].height - heights[0].height) * off.y) / width;
}

float GridHeight::getHeight135b(const Point2F& off)
{
	return heights[dx+dy].height +
		((heights[dy].height - heights[dx+dy].height) * (width - off.x) +
		(heights[dx].height - heights[dx+dy].height) * (width - off.y)) / width;
}


//----------------------------------------------------------------------------
// Line should be in the space of the square.
// This assumes that if there is an intersection one of the
// triangle planes, it takes place within the square.
//
#define MATH_ERROR 0.0001f
bool GridHeight::intersect(const LineSeg3F& line, Point3F* ip)
{
	PlaneF plane;
	Vector3F normal;
	switch (split) {
		case Split45: {
			getNormal45a(&normal);
			plane.set(Point3F(0.0f,0.0f,heights[0].height),normal);
			if (plane.intersect(line,ip) &&
				ip->x > -MATH_ERROR && ip->y < width + MATH_ERROR &&
				getFace45(*ip) == FaceA)
				return true;
			getNormal45b(&normal);
			plane.set(Point3F(0.0f,0.0f,heights[0].height),normal);
			if (plane.intersect(line,ip) && 
				getFace45(*ip) == FaceB)
				return true;
			return false;
		}
		case Split135: {
			getNormal135a(&normal);
			plane.set(Point3F(0.0f,width,heights[dy].height),normal);
			if (plane.intersect(line,ip) &&
				ip->x > -MATH_ERROR && ip->y > -MATH_ERROR &&
				getFace135(*ip) == FaceA)
				return true;
			getNormal135b(&normal);
			plane.set(Point3F(0.0f,width,heights[dy].height),normal);
			if (plane.intersect(line,ip) &&
				getFace135(*ip) == FaceB)
				return true;
			return false;
		}
	}
	return false;
}

