//
// extras.h
//

#ifndef _EXTRAS_H_
#define _EXTRAS_H_

#define max(x,y) ((x)>(y)?(x):(y))
#define min(x,y) ((x)<(y)?(x):(y))

inline int round(float x)
{
	if (fmod(x, 1.0) >= 0.5) return(int(x) + 1);
	else return(int(x));
}

inline int inBoundingBox(int x, int y, int x0, int y0, int x1, int y1)
{
	return((x >= x0) && (x <=x1) && (y >= y0) && (y <= y1));
}

#endif
