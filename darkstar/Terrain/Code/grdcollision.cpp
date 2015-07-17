
//----------------------------------------------------------------------------
// terrain collision.

//----------------------------------------------------------------------------

#include <grdfile.h>
#include <grdblock.h>
#include <grdcollision.h>
#include <grdHeight.h>

#define MAX_FLOAT 1e20f
#define SMALL_FLOAT .00001

//----------------------------------------------------------------------------

GridCollision::GridCollision(GridFile *terrain, TMat3F *tmat)
{
   terrainFile = terrain;
   transform = tmat;
}

//----------------------------------------------------------------------------
// Collide a line with the ground
// Sets GridCollision::collisionPoint to the collision point
// closest to the start point.
// Assumes that start is not under the ground.
//

inline float GridCollision::GetHeight(int x, int y)
{
   int bs = 1 << blockShift;
   if(x < 0)
      x = 0;
   else if(x > bs)
      x = bs;
   if(y < 0)
      y = 0;
   else if(y > bs)
      y = bs;

   return((heightField + x + y + (y << blockShift))->height);
}

GridBlock::Material *GridCollision::GetMaterial(int x, int y)
{
   int bs = 1 << blockShift;
   if(x < 0)
      x = 0;
   else if(x > bs)
      x = bs;
   if(y < 0)
      y = 0;
   else if(y > bs)
      y = bs;

   GridBlock::Material * material = materialMap + x + ( y << blockShift );
   
   // check for a texture index that is out of range
   if( material->index >= terrainFile->getMaterialList()->getMaterialsCount() )
   {
      char message[200];
      sprintf( message, "GridCollision::GetMaterial: Grid texture index [%d] at position ( %d, %d ) is invalid.  Set to 0.", material->index, x, y );
      AssertWarn( material->index < terrainFile->getMaterialList()->getMaterialsCount(),
         message );
         
      material->index = 0;
   }
   
   return( material );
}

inline Point3F GridCollision::GetPoint(int x, int y)
{
   return Point3F(x << grdSize, y << grdSize, GetHeight(x, y));
}

inline void CalcTs(float &curT, float &deltaT, float N0, float N1)
{
   if(N1 == N0)
   {
      curT = MAX_FLOAT;
      deltaT = MAX_FLOAT;
   }
   else
   {
      deltaT = 1.0f / (N1 - N0);
      if(deltaT < 0)
      {
         curT = (floor(N0) - N0) * deltaT;
         deltaT = -deltaT;
      }
      else
      {
         curT = (ceil(N0) - N0) * deltaT;
      }
   }
}


//----------------------------------------------------------------------------

bool GridCollision::collide(const Point3F &start, const Point3F &end, bool LOS, bool collideWithHoles)
{
   tube = false;
   justLOS = LOS;
   hitHoles = collideWithHoles;
	if (transform) {
		Point3F sp,ep;
		m_mul(start,*transform,&sp);
		m_mul(end,*transform,&ep);
		return collideFile(sp,ep);
	}
	return collideFile(start,end);
}	

bool GridCollision::collide(const Point3F &start, const Point3F &end, float in_radius)
{
	Point3F sp,ep;
	if (transform) {
		m_mul(start,*transform,&sp);
		m_mul(end,*transform,&ep);
	}
	else
		sp = start,ep = end;

	// Not a real tube collision, but close enough for now.
   radius = in_radius;
   tube = true;
   justLOS = false;
   hitHoles = false;
	if (collideFile(sp,ep)) {
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------

bool GridCollision::collideFile(const Point3F &start, const Point3F &end)
{
   // loop through all the blocks that are crossed by the line...
   grdSize = terrainFile->getScale();
   blockShift = terrainFile->getBlock(Point2I(0,0))->getSizeShift();
   
   float invBlockSize = 1 / float(1 << (grdSize + blockShift));
   Point3F delta;
   
   delta = end;
   delta -= start;

   Point3F curStart = start;
   Point2I extent = terrainFile->getSize();

   float curT[3], deltaT[3];

   CalcTs(curT[0], deltaT[0], start.y * invBlockSize, end.y * invBlockSize);
   CalcTs(curT[1], deltaT[1], start.x * invBlockSize, end.x * invBlockSize);
	curT[2] = deltaT[2] = 0;

   float currentT;
   int currentI;
	bool crossed_terrain = FALSE;

   for(;;)
   {
      if(curT[0] < curT[1])
      {
         currentT = curT[0];
         currentI = 0;
      }
      else
      {
         currentT = curT[1];
         currentI = 1;
      }

      if(currentT >= 1.0f)
      {
         currentT = 1.0f;
         currentI = 2;
      }
      curT[currentI] += deltaT[currentI];

      Point3F cp = delta;
      cp *= currentT;
      cp += start;
      
      float xPos = cp.x * invBlockSize;
      float yPos = cp.y * invBlockSize;
      int ix, iy;

      if(currentI == 0)
      {
         // it was a horizontal line that we crossed...
         ix = int(floor(xPos));
         iy = (int)floor(yPos + .5);
         if(delta.y > 0)
            iy--;
      }
      else if(currentI == 1)
      {
         ix = (int)floor(xPos + .5);
         iy = int(floor(yPos));
         if(delta.x > 0)
            ix--;
      }
      else
      {
         ix = int(floor(xPos));
         iy = int(floor(yPos));
      }
      if(ix >= 0 && ix < extent.x && iy >= 0 && iy < extent.y)
      {
         GridBlock *blk = terrainFile->getBlock(Point2I(ix, iy));
         AssertFatal(blk, "GridCollision::collideFile: no gridblock found at the given position");
         Point3F blockStart = curStart;
         Point3F blockEnd = cp;
         curStart = cp;
         Point3F offset(ix << (grdSize + blockShift),
                        iy << (grdSize + blockShift), 0);
         blockStart -= offset;
         blockEnd -= offset;
			crossed_terrain = TRUE;

         if(collideBlock(blk, blockStart, blockEnd))
         {
				surface.part = blk->getId();
            surface.position += offset;
            return true;
         }
      }
      else if ( crossed_terrain )
         return false;
         
      if(currentI == 2) // did we hit the end?
         return false;
         
   }
}


//----------------------------------------------------------------------------

bool GridCollision::collideBlock(GridBlock *block, const Point3F &start, const Point3F &end)
{

   float curT[4], deltaT[4];  // current and delta t values for 
                              // each of the 4 lines
   heightField = block->getHeightMap();
   materialMap = block->getMaterialMap();
   float gridSize /*, diagGridSize, gridSizeSquared, diagGridSizeSquared*/; 

   gridSize = float(1 << grdSize);

   float invGrdSize = 1 / gridSize;
   float halfInvGrdSize = .5 * invGrdSize;
//   float invGrdSizeSquared, invDiagGrdSizeSquared;
//   float radiusSquared;

//   if(tube)
//   {
//      diagGridSize = gridSize * 1.4142135;
//      gridSizeSquared = gridSize * gridSize;
//      diagGridSizeSquared = 2 * gridSizeSquared;
//      invGrdSizeSquared = invGrdSize * invGrdSize;
//      invDiagGrdSizeSquared = invGrdSizeSquared * .5;
//      radiusSquared = radius * radius;
//   }

   // determine whether the start and end are above/below ground
   int hmwid = block->getHeightMapWidth() - 1;
   
   Point2I iStart(int(start.x) >> grdSize, int(start.y) >> grdSize);
   Point2I iEnd(int(end.x) >> grdSize, int(end.y) >> grdSize);

   if(iStart.x >= hmwid)
      iStart.x = hmwid - 1;
   else if(iStart.x < 0)
      iStart.x = 0;
   if(iStart.y >= hmwid)
      iStart.y = hmwid - 1;
   else if(iStart.y < 0)
      iStart.y = 0;

   if(iEnd.x >= hmwid)
      iEnd.x = hmwid -1;
   else if(iEnd.x < 0)
      iEnd.x = 0;
   if(iEnd.y >= hmwid)
      iEnd.y = hmwid - 1;
   else if(iEnd.y < 0)
      iEnd.y = 0;

   GridHeight ht;

   ht.width = float(1 << grdSize);
	ht.dx = 1;
	ht.dy = block->getHeightMapWidth();

   ht.split = ((iStart.x ^ iStart.y) & 1)? GridHeight::Split135:
		GridHeight::Split45;
   ht.heights = block->getHeight(0, iStart);
   Point2F inspos(start.x - (iStart.x << grdSize), start.y - (iStart.y << grdSize));

   float startHeight = ht.getHeight(inspos);

   ht.split = ((iEnd.x ^ iEnd.y) & 1)? GridHeight::Split135:
		GridHeight::Split45;
   ht.heights = block->getHeight(0, iEnd);
   inspos.set(end.x - (iEnd.x << grdSize), end.y - (iEnd.y << grdSize));

   float endHeight = ht.getHeight(inspos);

   bool above = start.z > startHeight;

   float deltaX = end.x - start.x;
   float deltaY = end.y - start.y;
   float deltaZ = end.z - start.z;

   Point3F currentHeight;

   // four line equations are:
   // N = Y / (1 << grdSize) : horizontal grid lines.
   // N = X / (1 << grdSize) : vertical grid lines.
   // N = (X - Y) / (2 << grdSize) : diagonal lines positive slope
   // N = (X + Y) / (2 << grdSize) : diagonal lines negative slope

   CalcTs(curT[0], deltaT[0], start.y * invGrdSize, end.y * invGrdSize);
   CalcTs(curT[1], deltaT[1], start.x * invGrdSize, end.x * invGrdSize);
   CalcTs(curT[2], deltaT[2], 
         (start.x - start.y) * halfInvGrdSize, 
         (end.x - end.y) * halfInvGrdSize);
   CalcTs(curT[3], deltaT[3], 
         (start.x + start.y) * halfInvGrdSize, 
         (end.x + end.y) * halfInvGrdSize);

   float currentT;
   int currentI;

   for(;;)
   {
      currentT = 1.0f; // start at end...
      int i;
      currentI = -1;
      for (i = 0; i < 4; i++)
      {
         if (curT[i] < currentT)
         {
            currentT = curT[i];
            currentI = i;
         }
      }
      if(currentI == -1)
         break;

      curT[currentI] += deltaT[currentI]; // step the t for the collided line.

      // now we gotta check collision - we know which type
      // of line (0 - 3) it is.

      surface.position.x = start.x + deltaX * currentT;
      surface.position.y = start.y + deltaY * currentT;
      surface.position.z = start.z + deltaZ * currentT;

      float xPos = surface.position.x * invGrdSize;
      float yPos = surface.position.y * invGrdSize;
      int iy, ix; 
      float height;
//      float delta; // difference between height and current position
      if(tube)
      {
         float dh;
//         float dh, deltah;
//         float dx, dx2, inv;
         switch(currentI)
         {
            case 0:
               // it's a horizontal grid line...
               ix = int(xPos);
               iy = (int)floor(yPos + .5);
               dh = GetHeight(ix+1,iy) - GetHeight(ix,iy);
//               dx = gridSize;
//               dx2 = gridSizeSquared;
//               inv = invGrdSizeSquared;
               height = GetHeight(ix, iy) + dh * (xPos - ix);
               break;
            case 1:
               // vertical grid line...
               iy = int(yPos);
               ix = (int)floor(xPos + .5);
               dh = GetHeight(ix, iy+1) - GetHeight(ix, iy);
//               dx = gridSize;
//               dx2 = gridSizeSquared;
//               inv = invGrdSizeSquared;
               height = GetHeight(ix, iy) + dh * (yPos - iy);
               break;
            case 2:
               // diagonal line from ix,iy to ix+1, iy+1;
               ix = int(xPos);
               iy = int(yPos);
               dh = GetHeight(ix+1,iy+1) - GetHeight(ix,iy);
//               dx = diagGridSize;
//               dx2 = diagGridSizeSquared;
//               inv = invDiagGrdSizeSquared;
               height = GetHeight(ix, iy) + dh * (xPos - ix);
               break;
            case 3:
               // diagonal line from ix,iy+1 to ix+1,iy
               ix = int(xPos);
               iy = int(yPos);
               dh = GetHeight(ix+1,iy) - GetHeight(ix,iy+1);
//               dx = diagGridSize;
//               dx2 = diagGridSizeSquared;
//               inv = invDiagGrdSizeSquared;
               height = GetHeight(ix, iy+1) + dh * (xPos - ix);
               break;
         }
//         deltah = radiusSquared * (dx2 + dh * dh) * inv;
//         delta = surface.position.z - height;
//         delta *= delta;
//         delta -= deltah;
      }
      else
      {
         float    h1, h2, pct;
         switch(currentI)
         {
            case 0:{
               // it's a horizontal grid line...
               ix = int(xPos);
               iy = (int)floor(yPos + .5);
               h1 = GetHeight(ix, iy);
               h2 = GetHeight(ix+1, iy);
               pct = (xPos - ix);
               break;
            }
            case 1:{
               // vertical grid line...
               iy = int(yPos);
               ix = (int)floor(xPos + .5);
               h1 = GetHeight(ix, iy);
               h2 = GetHeight(ix, iy+1);
               pct = (yPos - iy);
               break;
            }
            case 2:{
               // diagonal line from ix,iy to ix+1, iy+1;
               ix = int(xPos);
               iy = int(yPos);
               h1 = GetHeight(ix, iy);
               h2 = GetHeight(ix+1, iy+1);
               pct = (xPos - ix);
               break;
            }
            case 3:{
               // diagonal line from ix,iy+1 to ix+1,iy
               ix = int(xPos);
               iy = int(yPos);
               h1 = GetHeight(ix, iy+1);
               h2 = GetHeight(ix+1, iy);
               pct = (xPos - ix);
               break;
            }
         }
         height = h1 + (h2 - h1) * pct;
      }
      
      if(int(surface.position.z > height) ^ above)
      {
         // we collided... do something.

         // we need to know the square that was hit.
         // if we're on a diagonal then we're OK...
         // otherwise, we need to correct ix, iy

         if(currentI == 0 && deltaY > 0)
            iy--;
         else if(currentI == 1 && deltaX > 0)
            ix--;

         // check for a hole in the ground

         if(!hitHoles && GetMaterial(ix, iy)->getEmptyLevel())
         {
            above = !above;
            continue;
         }

         surface.position.z = height;

         if(justLOS)
            return(true);

         // it's not just asking for line of sight..

         enum splittypes
         {
            lower45, upper45, lower135, upper135
         };
         int spt;
         switch(currentI)
         {
            case 0:
               // horizontal line crossed...
               if(deltaY < 0) // crosses out to next square
               {
                  if((ix ^ iy) & 1)
                     spt = lower135;
                  else
                     spt = lower45;
               } 
               else
               {
                  if((ix ^ iy) & 1)
                     spt = upper135;
                  else
                     spt = upper45;
               }
               break;
            case 1:
               // vertical line crossed...
               if(deltaX < 0)
               {
                  if((ix ^ iy) & 1)
                     spt = lower135;
                  else
                     spt = upper45;
               }
               else
               {
                  if((ix ^ iy) & 1)
                     spt = upper135;
                  else
                     spt = lower45;
               }
               break;
            case 2:
               // diagonal 45 line.
               if(deltaX > deltaY)
                  spt = upper45;
               else
                  spt = lower45;
               break;
            case 3:
               // diagonal 135 line.  
               if(deltaX > -deltaY)
                  spt = lower135;
               else
                  spt = upper135;
               break;            
         }
         // now we've found which triangle on the square
         // we intersected.
         // now build a plane for that triangle.

         PlaneF plane;
			Point3F normal;
		   ht.heights = block->getHeight(0, Point2I(ix,iy));
         switch(spt)
         {
            case upper45:
					ht.getNormal45a(&normal);
					plane.set(GetPoint(ix, iy),normal);
               break;
            case lower45:
					ht.getNormal45b(&normal);
					plane.set(GetPoint(ix, iy),normal);
               break;
            case lower135:
					ht.getNormal135a(&normal);
					plane.set(GetPoint(ix, iy + 1),normal);
               break;
            case upper135:
					ht.getNormal135b(&normal);
					plane.set(GetPoint(ix, iy + 1),normal);
               break;
         }
         LineSeg3F seg(start, end);
         if(tube)
            plane.d += above ? radius : -radius;

         plane.intersect(seg, &surface.position);
			// Use plane normal, the variable "normal" will
			// not be normalized.

			surface.normal = plane;
			surface.distance = 0.0f;
			surface.surface = 0;
			surface.material = int(&(*terrainFile->getMaterialList())
				[GetMaterial(ix, iy)->index]);
         return true;
      }
   }

   if(tube)
      endHeight += above ? radius : -radius;

   if(above != (end.z > endHeight))
   {
      GridBlock::Material *mat = GetMaterial(iEnd.x, iEnd.y);
      if(mat->getEmptyLevel())
         return false;

      // we collided with the end... bwah ha hah
      Point3F soffset(iEnd.x << grdSize, iEnd.y << grdSize, 0);
      Point3F oStart = start;
      oStart -= soffset;
      Point3F oEnd = end;
      oEnd -= soffset;

      bool ret;
      LineSeg3F seg(oStart, oEnd);

      ret = ht.intersect(seg, &surface.position);
      surface.distance = 0.0f;
      surface.surface = 0;
		surface.material = int(&(*terrainFile->getMaterialList())
			[mat->index]);

      if(ret)
      {
         ht.getNormal(surface.position, &surface.normal);
         surface.normal.normalize();
      }
      surface.position += soffset;
      return ret;
   }

   return(false);
   
}
