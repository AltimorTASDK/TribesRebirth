//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#ifndef _GRDCOLLISION_H_
#define _GRDCOLLISION_H_


//----------------------------------------------------------------------------

class GridFile;
#include <ml.h>
#include <grdBlock.h>

class GridCollision
{
   GridFile *terrainFile;
   GridBlock *curBlock;
   TMat3F *transform;
   GridBlock::Height *heightField;
   GridBlock::Material *materialMap;
   bool tube;
   bool hitHoles;
   bool justLOS;
   int blockShift;      // shift for size of block in squares
   int grdSize;         // log based 2 of world units per square

   float GetHeight(int x, int y);
   Point3F GetPoint(int x, int y);
   GridBlock::Material *GetMaterial(int x, int y);
   float radius;
public:
	CollisionSurface surface;

   GridCollision(GridFile *file = 0, TMat3F *tmat = 0);

	// Operate in object space
   bool collideBlock(GridBlock *block, const Point3F &start, const Point3F &end);
   bool collideFile(const Point3F &start, const Point3F &end);

	// Use the transform
   bool collide(const Point3F &start, const Point3F &end, bool justDoLineOfSight = false, bool collideWithHoles=false);
   bool collide(const Point3F &start, const Point3F &end, float radius);
   bool testLineOfSight(const Point3F &start, const Point3F &end);
};

#endif

