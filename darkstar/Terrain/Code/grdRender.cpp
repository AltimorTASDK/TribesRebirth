//---------------------------------------------------------------------------

#define  DEBUG_LIGHTS   0

//---------------------------------------------------------------------------
#include <g_surfac.h>
#include <g_types.h>
#include <g_bitmap.h>

#include "grdRender.h"
#include "grdMetrics.h"
#include "gOGLSfc.h"

static UInt32 g_downloadCount = 0;
static const UInt32 sg_downloadCap = 16;

namespace OpenGL {

void HandleSetLightMapTer(GFXSurface*           io_pSurface,
                          GFXLightMap*          io_pLightMap,
                          GFXBitmap::CacheInfo* io_pCacheInfo);

void
externCheckCache(OpenGL::Surface*);

} // namespace OpenGL


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

struct Poly
{
	int polyCount;
	int vertexList[3 * 3];
} PolyList[] = {
//
// Triangle subdivision for squares with up to five vertices
// organized as follows:
//
//    2 3 4
//    1
//    0
//
/*43210*/
//
/*00000*/ { 0, },
/*00001*/ { 0, },
/*00010*/ { 0, },
/*00011*/ { 0, },
//
/*00100*/ { 0, },
/*00101*/ { 0, },
/*00110*/ { 0, },
/*00111*/ { 0, },
//
/*01000*/ { 0, },
/*01001*/ { 0, },
/*01010*/ { 0, },
/*01011*/ { 0, }, // ?
/*01100*/ { 0, },
/*01101*/ { 1, 0,2,3, }, // ?
/*01110*/ { 1, 1,2,3, }, // ?
/*01111*/ { 1, 0,1,3, 1,2,3, }, // ?
//
/*10000*/ { 0, },
/*10001*/ { 0, },
/*10010*/ { 0, },
/*10011*/ { 1, 0,1,4, }, // ?
/*10100*/ { 0, },
/*10101*/ { 1, 0,2,4, },
/*10110*/ { 1, 1,2,4, }, // ?
/*10111*/ { 2, 0,1,4, 1,2,4, },
/*11000*/ { 0, },
/*11001*/ { 1, 0,3,4, }, // ?
/*11010*/ { 0, }, // ?
/*11011*/ { 2, 0,1,4, 1,3,4, },
/*11100*/ { 0, },
/*11101*/ { 2, 0,2,3, 0,3,4, },
/*11110*/ { 2, 1,3,4, 1,2,3, }, // ?
/*11111*/ { 3, 0,3,4, 0,1,3, 1,2,3 },
};

struct Face
{
	int startIndex;
	int rollCount;
} FaceList[] =  {
	{ 0, 0}, // 45 a
	{ 4, 4}, // 45 b
	{ 6, 2}, // 135 a
	{ 2, 6}, // 135 b
};


//---------------------------------------------------------------------------
// Rotated texture coors for level 0 squars.  Indexed using the
// material flags.

Point2F TextureCoor[8][9] = {
	{ // Plain
	Point2F(0.0f,1.0f),
	Point2F(0.0f,0.5f),
	Point2F(0.0f,0.0f),
	Point2F(0.5f,0.0f),
	Point2F(1.0f,0.0f),
	Point2F(1.0f,0.5f),
	Point2F(1.0f,1.0f),
	Point2F(0.5f,1.0f),
	Point2F(0.5f,0.5f),
	},
//	// Initialized in GridRender constructor.
//	{}, // Rotate
//	{}, // FlipX
//	{}, // Rotate | FlipX
//	{}, // FlipY
//	{}, // FlipY | Rotate
//	{}, // FlipY | FlipX
//	{}, // FlipY | Rotate | FlipX
};


//---------------------------------------------------------------------------

GridMetricsInfo GridMetrics;

void GridMetricsInfo::Render::reset()
{
	squares = polys = textureBuild = textureRegister = 
	vertices = blocks = lightMapBuild = 0;
}


//---------------------------------------------------------------------------

static int getShift(int x)
{
	// Returns the shift value of the first bit
	int i = 0;
	if (x)
		while (!(x & 1))
			i++, x >>= 1;
	return i;
}

static int getPower(int x)
{
	// Returns 2^n (the highest bit).
	int i = 0;
	if (x)
		do
			i++;
		while (x >>= 1);
	return i;
}

inline void buildGlobalKey(GFXTextureHandle& handle,GridBlock* block)
{
   handle.key[0] = int(block);
   handle.key[1] = 1 << 30;
}

inline void buildKey(GFXTextureHandle& handle,GridBlock* block,
	int detail,const Point2I& pos)
{
	handle.key[0] = int(block);
	handle.key[1] = (detail << 24) | (pos.y << 12) | pos.x;
}

inline GridBlock* getKeyBlock(const GFXTextureHandle& handle)
{
	return (GridBlock*) handle.key[0];
}

inline void getKeyPos(const GFXTextureHandle& handle,Point2I& pos)
{
   register UInt32   key1 = ((UInt32)handle.key[1]) & ~GridRender::IsDynamicLit;
	pos.y = (key1 >> 12) & ((1 << 12) - 1);
	pos.x = key1 & ((1 << 12) - 1);
}

inline int getKeyDetail(const GFXTextureHandle& handle)
{
	return ( (UInt32(handle.key[1]) & ~GridRender::IsDynamicLit) >> 24);
}	

inline void makeKeyLit ( GFXTextureHandle& handle )
{
   handle.key[1] |= GridRender::IsDynamicLit;
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

inline void swap(Point2F& a,Point2F& b) {
	Point2F t = a; a = b; b = t;
}


//---------------------------------------------------------------------------
#ifdef DEBUG
GFXBitmap *GridRender::defaultTexture=NULL;
#endif


GridRender::GridRender()
{
	visibleDistance = 1.0f;
	hazeDistance = 0.5f;
	hazeScale = 0.0f;
	hazeVertical.fMin = 100000.0f;
	hazeVertical.fMax = 200000.0f;
	hazeVerticalScale = 0.0f;
	screenSize = 50;
	customDetailTable = false;
   // dynamic lighting data members:
   dlightsEnabled = true;
   lightedSquares = NULL;
   renderSquare = NULL;
   dLightCount = 0;
   htStepBits = 0;

//Point2F TextureCoor[8][9] = {
//	{ // Plain
//	Point2F(0.0f,1.0f),     0
//	Point2F(0.0f,0.5f),     1
//	Point2F(0.0f,0.0f),     2
//	Point2F(0.5f,0.0f),     3
//	Point2F(1.0f,0.0f),     4
//	Point2F(1.0f,0.5f),     5
//	Point2F(1.0f,1.0f),     6
//	Point2F(0.5f,1.0f),     7
//	Point2F(0.5f,0.5f),     8
//	},
////	// Initialized in GridRender constructor.
////	{}, // Rotate
////	{}, // FlipX
////	{}, // Rotate | FlipX
////	{}, // FlipY
////	{}, // FlipY | Rotate
////	{}, // FlipY | FlipX
////	{}, // FlipY | Rotate | FlipX
//};
	//
	static bool coorBuilt;
	if (!coorBuilt) {
		coorBuilt = true;
		// Build all the rotations texture coors.
		for (int i = 1; i < 8; i++) {
			Point2F* tex = TextureCoor[i];
			// Copy the unrotated to start with
			for (int p = 0; p < 9; p++)
				tex[p] = TextureCoor[0][p];
			//
			if (i & GridBlock::Material::FlipX) {
				swap(tex[0],tex[6]);
				swap(tex[1],tex[5]);
				swap(tex[2],tex[4]);
			}
			if (i & GridBlock::Material::FlipY) {
				swap(tex[0],tex[2]);
				swap(tex[3],tex[7]);
				swap(tex[4],tex[6]);
			}
			if (i & GridBlock::Material::Rotate) {
				swap(tex[2],tex[4]);
				swap(tex[2],tex[6]);
				swap(tex[2],tex[0]);
				swap(tex[7],tex[1]);
				swap(tex[7],tex[3]);
				swap(tex[7],tex[5]);
			}
		}
	}
}

GridRender::~GridRender()
{
   if ( lightedSquares != NULL )
      delete lightedSquares;
}


//---------------------------------------------------------------------------

void GridRender::setScreenSize(float sz)
{
	// The projected size in world coordinates of the square
	// onto the view plane.
	customDetailTable = false;
	screenSize = sz;
}

void GridRender::setDetailTable(const Detail dt[MaxDetailLevel])
{
	customDetailTable = true;
	memcpy(detailData,dt,sizeof(detailData));
}

void GridRender::setTerrainTextureDetail( int level )
{
   terrainTextureDetail = level;
}   

//---------------------------------------------------------------------------

void GridRender::setVisibleDistance(float dist)
{
	visibleDistance = dist;
}

void GridRender::setHazeDistance(float dist)
{
	AssertFatal(dist != visibleDistance,
		"GridRender::setHazeDistance: Haze distance must be less than"
		"visible distance");
	hazeDistance = dist;
	// The output range is a shortened to acount for interpolation
	// problems with GFX.
	float delta = visibleDistance - hazeDistance;
	hazeScale = 1.0f / (delta + delta * 0.02f);
}

void GridRender::setHazeVertical(float minDist,float maxDist)
{
	hazeVertical.fMin = minDist;
	hazeVertical.fMax = maxDist;
	float delta = hazeVertical.fMax - hazeVertical.fMin;
	hazeVerticalScale = delta? 1.0f / delta: 1.0f;
}


//---------------------------------------------------------------------------

void GridRender::setContext(TSRenderContext& rc)
{
	GridMetrics.render.reset();

	renderContext = &rc;
	camera = rc.getCamera();
   isOrtho = !(camera->getCameraType() == TS::PerspectiveCameraType);
	pointArray = rc.getPointArray();
	gfxSurface = rc.getSurface();

	gfxSurface->setFillMode(GFX_FILL_TWOPASS);
   gfxSurface->setTexturePerspective(!isOrtho);
	gfxSurface->setHazeSource(GFX_HAZE_NONE);
   gfxSurface->setShadeSource(GFX_SHADE_NONE);
   gfxSurface->setAlphaSource(GFX_ALPHA_NONE);
	gfxSurface->setTransparency(false);

	// DMM - State order dependancy fix
	extern void GFXShadeHazeChanged(GFXSurface*);
	GFXShadeHazeChanged(gfxSurface);

	pointArray->reset();
	pointArray->useHazes(!isOrtho);
	pointArray->useIntensities(false);
	pointArray->useTextures(true);
	pointArray->setVisibility( TS::ClipMask );
   pointArray->setFarDist(isOrtho ? 1.0e6f : visibleDistance);
	pointArray->useBackFaceTest(false);

	if (visibleDistance <= hazeDistance)
		hazeDistance = visibleDistance * 0.6f;

	// Camera pos in object space
	TMat3F mat = renderContext->getCamera()->getTOC();
	mat.inverse();
	cameraPos = mat.p;

	// Build distances at which detail levels change based
	// on the projected size of the squares.
	float ps = screenSize / camera->getPixelScale();
	for (int i = MaxDetailLevel; i >= 0; --i) {
		Detail& d = detailData[i];
		if (!customDetailTable) {
			float size = float(1 << (i + scale));
			d.detailDistance = camera->projectionDistance(ps,size);

			if (i < MaxDetailLevel - 1) {
				Detail& pd = detailData[i+1];
				d.maxDistance = pd.detailDistance - size;
				d.minDistance = (d.maxDistance + d.detailDistance) * 0.5f;
			}
			else {
				d.minDistance = d.detailDistance + size * 2.0f;
				d.maxDistance = d.minDistance * 2.0f;
			}
		}
		float delta = d.maxDistance - d.minDistance;
		d.distanceScale = (delta > .0f)? 1.0 / delta: 1.0f;
		d.level = i;
	}

	// Pick scan level for the new detail distances
	for (scanDetailLevel = 0; scanDetailLevel < MaxDetailLevel &&
			detailData[scanDetailLevel].maxDistance != .0f &&
			visibleDistance > detailData[scanDetailLevel].maxDistance;
			scanDetailLevel++)
		;

	// Builds edge table of ground intersection with
	// the viewcone.
	edgeTables.setRasterData(scale,scanDetailLevel,
		visibleDistance,heightRange);
	edgeTables.buildTables(*camera);

	// Build bounding box in scanDetailLevel object space.
	// pos >= fMin.{xy} and pos <= fMax.{xy}
	const GridEdgeTable::Table& et = 
		edgeTables.getEdgeTable(scanDetailLevel);
	visibleBox.fMin.y = et.starty;
	visibleBox.fMax.y = et.starty + et.edges.size() - 1;
	visibleBox.fMin.x = Int32(M_MAX_INT);
	visibleBox.fMax.x = Int32(M_MIN_INT);

	for (GridEdgeTable::EdgeVector::const_iterator itr = et.edges.begin();
			itr != et.edges.end(); itr++) {
		if (itr->fMin < visibleBox.fMin.x)
			visibleBox.fMin.x = itr->fMin;
		if (itr->fMax > visibleBox.fMax.x)
			visibleBox.fMax.x = itr->fMax;
	}

	Point2I cp;
	cp.x = int(cameraPos.x) >> (scale + scanDetailLevel);
	cp.y = int(cameraPos.y) >> (scale + scanDetailLevel);
	visibleBox.fMin += cp;
	visibleBox.fMax += cp;

	// Setup the buffer
	// Round to nearest scan detail so buildPoly doesn't try to
	// get points outside the buffer.
	Box2I visibleBuffer;
	visibleBuffer.fMin = visibleBox.fMin;
	visibleBuffer.fMin <<= scanDetailLevel;
	visibleBuffer.fMax.x = visibleBox.fMax.x + 2;
	visibleBuffer.fMax.y = visibleBox.fMax.y + 2;
	visibleBuffer.fMax <<= scanDetailLevel;

	bufferOrigin = visibleBuffer.fMin;
	bufferSize.x = visibleBuffer.len_x();
	bufferSize.y = visibleBuffer.len_y();

	int bufferTotal = bufferSize.x * bufferSize.y;
	bool bufferClear = bufferTotal > buffer.capacity();
	buffer.setSize(bufferTotal);
	if (bufferClear)
		// Clear out any new space in the buffer (currently
		// just clears the whole thing).
		memset(buffer.address(),0,buffer.size() * sizeof(int));
	else
		if (pointList.size()) {
			// Clear out the buffer entries we used last time.
			// (First entry is not used).
			Point* ptr = pointList.address();
			for (int i = pointList.size() - 1; i > 0;)
				*ptr[i--].buffer = 0;
		}

	// Clear out the pointList. The first entry is reserved for 
	// unused buffer points.
	pointList.setSize(1);

	// Update offsets in detailData for buffer
	for (int i2 = 0; i2 <= scanDetailLevel; i2++) {
		Detail& dd = detailData[i2];
		dd.bpOffsetX = 1 << i2;
		dd.bpOffsetY = bufferSize.x << i2;
		dd.bpOffsetXY = dd.bpOffsetX + dd.bpOffsetY;
		dd.bpOffsetX2Y = dd.bpOffsetX + 2 * dd.bpOffsetY;
		dd.bpOffset2XY = 2 * dd.bpOffsetX + dd.bpOffsetY;
	}
}


//---------------------------------------------------------------------------

void GridRender::render(GridFile* aFile)
{
	file = aFile;
	AssertFatal(file->scale == scale,
		"GridRender::render: File scale not the same as render scale");

	// Update height offsets in detailData
	// All blocks in a file should have the same size.
	int blockWidth = (1 << (file->detailCount - 1)) + 1;
	for (int i = 0; i <= scanDetailLevel; i++) {
		Detail& dd = detailData[i];
		dd.hpOffsetX = 1 << i;
		dd.hpOffsetY = blockWidth << i;
		dd.hpOffsetXY = dd.hpOffsetX + dd.hpOffsetY;
		dd.hpOffsetX2Y = dd.hpOffsetX + 2 * dd.hpOffsetY;
		dd.hpOffset2XY = 2 * dd.hpOffsetX + dd.hpOffsetY;
	}

	// Scan all the blocks that intersect the visible box
	Box2I vbox;
	int detail = file->detailCount - scanDetailLevel - 1;
	vbox.fMin.x = visibleBox.fMin.x >> detail;
	vbox.fMin.y = visibleBox.fMin.y >> detail;
	vbox.fMax.x = (visibleBox.fMax.x >> detail) + 1;
	vbox.fMax.y = (visibleBox.fMax.y >> detail) + 1;

	vbox.fMin += file->origin;
	vbox.fMax += file->origin;

	vbox.fMin.setMax(Point2I(0,0));
	vbox.fMax.setMin(Point2I(file->size.x,file->size.y));
   
   prepLightsRender ();

	squareList.clear();
	Point2I pos;
	for (pos.y = vbox.fMin.y; pos.y < vbox.fMax.y; pos.y++)
		for (pos.x = vbox.fMin.x; pos.x < vbox.fMax.x; pos.x++)
			if ((block = file->getBlock(pos)) != 0)
				scanBlock(block,pos);

   if (dynamic_cast<OpenGL::Surface*>(gfxSurface) != NULL) {
	// Render all the squares in the queue
      g_downloadCount = 0;
	   SquareList::iterator itr;
	   for (itr = squareList.begin();
			   itr != squareList.end(); itr++) {
		   Square& square = *itr;
		   block = square.block;
         if ( square.isUnderDLight () )
            prepLightsSquare(square);
		
		   drawSquareOGL(square, false);
	   }
	   for (itr = squareList.begin();
			   itr != squareList.end(); itr++) {
		   Square& square = *itr;
		   block = square.block;
		
		   drawSquareOGL(square, true);
	   }

      OpenGL::externCheckCache(dynamic_cast<OpenGL::Surface*>(gfxSurface));
   } else {
	   // Render all the squares in the queue
	   for (SquareList::iterator itr = squareList.begin();
			   itr != squareList.end(); itr++) {
		   Square& square = *itr;
		   block = square.block;
         if ( square.isUnderDLight () )
            prepLightsSquare(square);
		
         drawSquare(square);
	   }
   }


	gfxSurface->setHazeSource(GFX_HAZE_NONE);
}


//---------------------------------------------------------------------------**
//                            Dynamic Lights                                 **
//---------------------------------------------------------------------------**


#if DEBUG_LIGHTS
static UInt16  debugDarken = 0;
#define  DebugDarken(color)      (TSInten16::subtract(color,debugDarken))
#else
#define  DebugDarken(color)      (color)
#endif


// Return whether or not the given z range overlaps the height grid specified 
// by box, which is in grid coordinates.  
//    Note this works relative to the current block, which is here specified
//    by the current square via renderSquare.  
bool GridRender::rangeOverlaps ( Box2I gbox, float zmin, float zmax )
{
   float    lowest = 100000000.0f;
   float    highest = -lowest;

   // Figure out range spanned by all the heights within the grid box given.  Then
   // check this range against the range supplied.  
   for ( ; gbox.fMin.y <= gbox.fMax.y;  gbox.fMin.y++ )  {
      GridBlock::Height    *hp = renderSquare->block->getHeight ( 0, gbox.fMin );
      for ( int i = gbox.len_x () + 1; i > 0; i-- )  {
         float h = hp++ -> height;
         if ( h > highest )     highest = h;
         if ( h < lowest )      lowest = h;
      }
   }
   return (zmin <= highest && zmax >= lowest);
}


// Fetch an array of heights.  Our position is in grid coordinates (no matter
// what detail level we're working at, so we just pass level 0 to getHeight()).
//    Note this works relative to the current block, which is here specified
//    by the current square via renderSquare.  
void GridRender::GetHeights ( float * hts, const Point2I & pos, int N )
{
   GridBlock::Height    *hp = renderSquare->block->getHeight ( 0, pos );
   while ( N-- )
      *hts++ = (*hp++).height;
}


static inline void swap_floats ( float & f1, float & f2 )  {
   float temp=f1; f1=f2; f2=temp;
}


void GridRender::mergeDLight ( DLightInfo * dli, UInt16 * ldata, int lwidth )
{
   float    htTab1 [ MaxLightMapWidth + 1 ];
   float    htTab2 [ MaxLightMapWidth + 1 ];
   float    htIncs [ MaxLightMapWidth + 1 ];
   float    * htThisRow = htTab2, * htNextRow = htTab1;

   Point2F  xyBlockMin;
   Point3F  loc = dli->tsl->fLight.fPosition;
   float    lightRange = dli->tsl->fLight.fRange;
   float    maxDistSquared = lightRange * lightRange;
   float    gridStep = (float)(1 << scale) / (float)(1 << htStepBits);

   // Get our location and grid position in block space:
   Point2I  gridPos = dli->xyGrid.fMin;
   xyBlockMin.x = float (gridPos.x << scale);
   xyBlockMin.y = float (gridPos.y << scale);
   loc -= blockOrigin;

   // compute looping Steps.  
   Point2I Steps = dli->xyGrid.fMax;
   Steps -= dli->xyGrid.fMin;
   int heightsPerRow = Steps.x + 1;
   AssertFatal ( Steps.x <= MaxLightMapWidth, "GrdRender: too much light" );
   Steps <<= htStepBits;

   // This value tells if Z heights need to be interpolated and gives
   //    the mask to determine when we're interpolating.  The htStepBits
   //       gives number of bits set in htStepMask.  
   // INV: ySteps is a multiple of (1 << htStepBits).
   Int32    htStepMask = (1 << htStepBits) - 1;


   // Request from the lighting system a handle to an intensity mapper - 
   UInt16   maxCol = TSInten16::getPackedColor (  dli->tsl->fLight.fRed, 
               dli->tsl->fLight.fGreen,  dli->tsl->fLight.fBlue );
   TSInten16Xlat::RemapTable  &cMapper = TSInten16Xlat::getMapper(maxCol, maxDistSquared);


   // Setup for non-level-0 and level 0.  Level 0 gets it's four corners before the loop.
   // The others get them within and a first row fetch is needed.  
   bool  sideStepper;
   if ( renderSquare->dp->level )  {
      sideStepper = true;
      GetHeights ( htNextRow, gridPos, heightsPerRow );
   }
   else {
      AssertFatal ( heightsPerRow == 2, "grdRender: bad height count" );
      GetHeights ( htTab1, gridPos, heightsPerRow );
      gridPos.y++;
      GetHeights ( htTab2, gridPos, heightsPerRow );
      sideStepper = false;
      
      // We must rotate or flip our location based on the rotation flags.  By default texture
      // data is stored bottom to top with respect to normal, so flip in Y means don't flip 
      // in Y, actually....  First get a location relative to the square center, then flip it 
      // about.  This is only done for level 0 squares.  (levels above 0 step backwards 
      // through target data)
      if ( dlightMatFlags ^ GridBlock::Material::FlipY )
      {
         Point2F  center, rotateLoc;
         center.x = (float)((dli->squareBox.fMin.x + dli->squareBox.fMax.x) << (scale - 1));
         center.y = (float)((dli->squareBox.fMin.y + dli->squareBox.fMax.y) << (scale - 1));
         
         if ( dlightMatFlags & GridBlock::Material::Rotate )   {
            rotateLoc.x = center.y - loc.y;
            rotateLoc.y = loc.x - center.x;
            
            float temp = htTab1[ 0 ];       
            htTab1[ 0 ] = htTab2[ 0 ];    htTab2[ 0 ] = htTab2 [ 1 ];
            htTab2[ 1 ] = htTab1[ 1 ];    htTab1[ 1 ] = temp;
         }
         else {
            rotateLoc.x = loc.x - center.x;
            rotateLoc.y = loc.y - center.y;   }
         
         if ( dlightMatFlags & GridBlock::Material::FlipX )  {
            rotateLoc.x = -rotateLoc.x;
            swap_floats ( htTab1[0], htTab1[1] );
            swap_floats ( htTab2[0], htTab2[1] );     
         }
         if ( !(dlightMatFlags & GridBlock::Material::FlipY) )  {
            rotateLoc.y = -rotateLoc.y;
            swap_floats ( htTab1[0], htTab2[0] );
            swap_floats ( htTab1[1], htTab2[1] );
         }
         loc.x = center.x + rotateLoc.x;
         loc.y = center.y + rotateLoc.y;
      }//rotation & flips setup done
   }

   // Set up our square-of-distance steppers:
   float    XDiffSqrd, YDiffSqrd, ZDiffSqrd, DistanceSquared;
   DistSquaredHelper<float>   ZSquaredHelp, ZLastColHelp, XSquaredHelp;
   DistSquaredHelper<float>   XHelpStart ( loc.x - xyBlockMin.x, gridStep );
   DistSquaredHelper<float>   YSquaredHelp ( loc.y - xyBlockMin.y, gridStep );
   
   // The following loop is really too involved - probably need to separate into several
   // distinct cases:  A level 0 square;  A rectangle composed of squares which are
   // light map expanded (beyond one lixel per grid);  A non-expanded large rect.
   YDiffSqrd = YSquaredHelp;
   for ( int y = 0; y <= Steps.y; y++, xyBlockMin.y += gridStep, ldata += lwidth )
   {
      // The heights are a little tricky since we may or may not be 
      //    interpolating.  If we are interpolating, then we'll need some 
      //    stepping values, so the stepping increments tell how to step the 
      //    heights down each collumn between the rows.  
      if ( htStepMask )  {
         if ( y & htStepMask )         // Step all our rows.
            for ( int i = 0; i < heightsPerRow; i++ )
               htThisRow[i] += htIncs[i];
         else  {
            // time to get the heights and compute increments.
            // make the Next row into This row, get next row if not done. 
            float    *temp = htThisRow;
            htThisRow = htNextRow;
            htNextRow = temp;

            if ( y != Steps.y )  {
               if (sideStepper ) {
                  gridPos.y++;
                  GetHeights ( htNextRow, gridPos, heightsPerRow );
               }
               for ( int i = 0; i < heightsPerRow; i++ )
                  htIncs[i] = (htNextRow[i] - htThisRow[i]) / (1 << htStepBits);
            }
         }
      }
      else   {  // we know we're sideStepper in this case, needn't test.  
         GetHeights ( htThisRow, gridPos, heightsPerRow );
         gridPos.y++;
      }

      XDiffSqrd = (XSquaredHelp = XHelpStart);
      UInt16   *ld = ldata;
      for ( int x = 0; x <= Steps.x; x++, ld++ )
      {
         // This first IF clause handles all the Z interpolation between
         //    the heights when we have a higher resolution light map.  
         //    It's a bit involved because the last row and collumns are 
         //    different, and because we want to minimize math with our
         //    square stepper as much as possible.  
         if ( htStepMask )   {
            if ( htStepMask & x )
               ZDiffSqrd = ZSquaredHelp.next ();
            else   {
               if ( x == Steps.x )      // last collumn special
               {
                  if ( y & htStepMask ) 
                     ZDiffSqrd = ZLastColHelp.next (); 
                  else
                  {
                     if ( y == Steps.y )      // last corner..
                     {
                        float zDiff = loc.z - htThisRow [ x >> htStepBits ];
                        ZDiffSqrd = zDiff * zDiff;
                     }
                     else     // Time to compute new zLastColHelp...  
                        ZDiffSqrd = ZLastColHelp.init ( 
                                       loc.z - htThisRow [ x >> htStepBits ], 
                                       htIncs [ x >> htStepBits ] );
                  }
               }
               else  // Not last collumn, need new Z step.  
               {
                  int      col = x >> htStepBits;
                  float    ht1 = htThisRow [ col ];
                  float    ht2 = htThisRow [ col + 1 ];
                  ZDiffSqrd = ZSquaredHelp.init ( loc.z - ht1,
                                    (ht2 - ht1) / (1 << htStepBits) );
               }
            }
         }
         else
            ZDiffSqrd = (loc.z - htThisRow[x]) * (loc.z - htThisRow[x]);

         // Got our squares, compute distance and light the lixel if needed.
         DistanceSquared = XDiffSqrd + YDiffSqrd + ZDiffSqrd;
         if ( DistanceSquared <= maxDistSquared )
            *ld = TSInten16::add ( *ld, cMapper [ DistanceSquared ] );

         XDiffSqrd = XSquaredHelp.next();
      }//x 

      YDiffSqrd = YSquaredHelp.next();
   }//y
}


void GridRender::mergeAllDLights ( GFXLightMap* lightMap )
{
   DLightInfoList::iterator   itr;

   for ( itr = dlightInfoList.begin(); itr != dlightInfoList.end(); itr++ ) 
   {
      DLightInfo  *info = itr;
      int         mapw = lightMap->size.x, offset, ladd;
      UInt16      *data = &lightMap->data;

      // get offset into light map data using offset into grid.  for detail squares
      // the merge happens from bottom to top.  
      Point2I     gridBox = info->xyGrid.fMin;
      gridBox -= info->squareBox.fMin;
      if ( renderSquare->dp->level ) {
         gridBox.y = info->squareBox.len_y() - gridBox.y; 
         ladd = -mapw;
      }
      else 
         ladd = mapw;

      offset = (gridBox.y << htStepBits) * mapw + (gridBox.x << htStepBits);
      AssertFatal ( gridBox.x >= 0 && gridBox.y >= 0, "GrdRender: bad bounds" );
      
      mergeDLight ( info, data + offset, ladd );
   }
}



// Last method called to prepare lights for the merge.  Does a more accurate height
//    check to figure out which lights should actually be merged.  
void GridRender::prepLightsSquare (Square & square)
{
   renderSquare = &square;       // set our local "global"
   blockOrigin.x = square.blockOrg.x;
   blockOrigin.y = square.blockOrg.y;
   blockOrigin.z = 0;

   dlightInfoList.clear ();
   // TSLightBuffer::iterator    itr;
   // for ( itr = lightsOverBlock.begin(); itr != lightsOverBlock.end(); itr++ ) 
   TSSceneLighting   *sceneLights = renderContext->getLights();
   TSSceneLighting::iterator ptr;
   for ( ptr = sceneLights->begin(); ptr != sceneLights->end(); ptr++ )  {
      TSLight  *tsl = *ptr;
      if ( tsl->fLight.fType != TS::Light::LightPoint || tsl->isStaticLight() == true )
         continue;
      Point3F  boxmin = tsl->fLight.fPosition;
      boxmin -= blockOrigin;
      Point3F  boxmax = boxmin;
      float    rangeAdj = tsl->fLight.fRange;
      boxmin -=  rangeAdj, boxmax +=  rangeAdj;

      // compute where light is in grid space.
      DLightInfo  info;
      int   extra = (1 << scale);
      info.xyGrid.fMin.x = int(boxmin.x) >> scale;
      info.xyGrid.fMin.y = int(boxmin.y) >> scale;
      info.xyGrid.fMax.x = (int(boxmax.x) + extra) >> scale;
      info.xyGrid.fMax.y = (int(boxmax.y) + extra) >> scale;
      // get square in grid space.
      Point2I     sqrMin = square.pos, sqrMax;
      sqrMin <<= square.dp->level;
      sqrMax = sqrMin;
      sqrMax += (1 << square.dp->level);
      info.squareBox ( &sqrMin, &sqrMax );
      
      // intersect with square (it SHOULD overlap).  Then see check against
      //    heights in the intersecting region of the grid.  
      if ( info.squareBox.overlaps ( info.xyGrid ) )   {
         info.xyGrid.intersect ( info.squareBox );
         if ( info.xyGrid.fMin.x != info.xyGrid.fMax.x )
            if ( info.xyGrid.fMin.y != info.xyGrid.fMax.y )
               if ( rangeOverlaps ( info.xyGrid, boxmin.z, boxmax.z ) )   {
                  info.tsl = tsl;
                  dlightInfoList.push_back ( info );
               }
      }
      else
         AssertWarn ( 1, "GrdRender: Some rounding strangeness?" );
   }

   if ( ! dlightInfoList.empty() )
      square.flags |= Square::SqrDynamicLit;
}


void  GridRender::prepLightsRender (void)
{
   if ( lightedSquares == NULL && dlightsEnabled )  {
      dlBitsetSize = file->detailCount - 1;
      if ( lightBitsetMax < dlBitsetSize )  {
         AssertWarn ( 1, 
            avar("grdRender: No dlights with blocksize > %dx%d", 
                     (1<<dlBitsetSize),(1<<dlBitsetSize))  );
         dlightsEnabled = false;
      }
      else
         lightedSquares = new BitSquare ( dlBitsetSize );
   }

   // prepare lights
   if ( dlightsEnabled )  {
      const TMat3F & mat = renderContext->getCamera()->getTOW();
      TSSceneLighting * sceneLights = renderContext->getLights();
      SphereF  instSphere ( mat.p, 1000000.0f );
      sceneLights->prepare ( instSphere, mat );
      // difference between block grid resolution and our bitset.  
      squareLightShift = file->detailCount - 1 - dlBitsetSize;
   }
}


//    Build a bit grid of affected regions, for use later by testSquare 
//       to see if it should scan for lights.  
void  GridRender::prepLightsBlock ( void ) 
{
   if ( dlightsEnabled == false )
      return;

   int         blockRadShift = file->detailCount - 1 + scale;
   float       block_rad = float (1 << blockRadShift);
   
   Point3F  boxmin = block->origin; 
   Point3F  boxmax ( block_rad, block_rad, block_rad ); 
   boxmax += boxmin; 
   Box3F    blockBound ( boxmin, boxmax );

   if ( dLightCount )    {    // only clear if it's not already clear..
      lightedSquares->zero();
      dLightCount = 0;
   }
   
   // lightsOverBlock.clear ();
   TSSceneLighting   *sceneLights = renderContext->getLights();
   TSSceneLighting::iterator ptr;
   for ( ptr = sceneLights->begin(); ptr != sceneLights->end(); ptr++ )     {
      TSLight  *tsl = *ptr;
      if ( tsl->fLight.fType == TS::Light::LightPoint && tsl->isStaticLight() == false) {
         Box3F    lightBox ( tsl->fLight.fPosition, tsl->fLight.fPosition );
         lightBox.fMin -= tsl->fLight.fRange;
         lightBox.fMax += tsl->fLight.fRange;
         if ( lightBox.overlaps ( blockBound ) )    {
            lightBox.intersect ( blockBound );
            
            // Flag the boxes in the array.  
            int   downShift = blockRadShift - dlBitsetSize;
            int   add = (1 << downShift) - 1;
            
            lightBox -= &boxmin;        // shift our box.  
            
            // convert this into the bit set.  
            int   x1 = int(lightBox.fMin.x) >> downShift;
            int   y1 = int(lightBox.fMin.y) >> downShift;

            int   x2 = (int(lightBox.fMax.x) + add) >> downShift;
            int   y2 = (int(lightBox.fMax.y) + add) >> downShift;
            
            lightedSquares->set ( x1, y1, x2-x1+1, y2-y1+1 );
            // lightsOverBlock.push_back ( tsl );
            dLightCount++;
         }
      }
   }
}


// Look up in the bit grid for any lights...  Note we need to have another
// bit grid to be used by large squares doing lookups.  This call could result in 
// several hundred bytes being tested.  
// This check is done during the scanBlock() phase, just to flag the square for 
//    more closer inspection during the draw loop.  
bool GridRender::isUnderLights ( const Square & square )
{
   AssertFatal ( dlightsEnabled == true, "grdRender: shouldn't get here" );
   if ( dLightCount && square.dp->level <= MaxSquareLightLevel )   {
      Point2I  pos = square.pos;
      pos <<= square.dp->level;
      AssertFatal ( ! squareLightShift, "grdRender: undone" );
      
      if ( squareLightShift > 0 )  {   // Block square resolution > bit grid resolution
		   pos >>= squareLightShift;
         // if ( ! (squareW >>= squareLightShift) )
         //   squareW = 1;
      }
      else if ( squareLightShift < 0 )    // bit grid resolution is extra.
         AssertFatal ( 1, "GrdRender: unneeded" );

      return lightedSquares->testPwr2Sqr( pos.x, pos.y, square.dp->level );
   }
   return false;
}


// If we're lighting, the square level and the light shift combine to make it 
// be the same resolution light map.  Otherwise the level gives the resolution. 
int GridRender::Square::lightShift ( int txtrShift ) const 
{
   if ( (txtrShift -= (dp->level + getLevelInc())) < 0 )
      txtrShift = 0;
   return txtrShift;
}



void GridRender::dlightsEnable(void)
{ 
   if ( dlightsEnabled == false )  {
      dlightsEnabled = true;
      dLightCount = 0;
   }
}

void GridRender::dlightsDisable(void)
{ 
   if ( dlightsEnabled == true )   {
      if ( lightedSquares != NULL )
         delete lightedSquares;
      lightedSquares = NULL;
      dlightsEnabled = false; 
      dLightCount = 0;
   }
}
// ^^Dynamic Lights^^
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

int* GridRender::getBuffer(int detail, const Point2I& pos)
{
	// detail and pos in current block
	Point2I bp;
	bp.x = (pos.x << detail) - block->bufferOrigin.x;
	bp.y = (pos.y << detail) - block->bufferOrigin.y;
	AssertFatal(bp.x >= 0 && bp.x < bufferSize.x &&
		bp.y >= 0 && bp.y < bufferSize.y,
		"GridRender::getBuffer: Out of bounds");
	return &buffer[bp.y * bufferSize.x + bp.x];
}


//---------------------------------------------------------------------------

void GridRender::scanBlock(GridBlock* blck,const Point2I& bpos)
{
	GridMetrics.render.incBlocks();
	block = blck;

	// Corner point in detail 0
	Point2I blockPos;
	int detail = file->detailCount - 1;
	blockPos.x = bpos.x << detail;
	blockPos.y = bpos.y << detail;

	// Init block render data
	block->materialList = file->materialList;
	block->origin.x = float(blockPos.x << scale);
	block->origin.y = float(blockPos.y << scale);
	block->origin.z = 0.0f;

   if (block->getCoverBitmap() == NULL)
      block->buildCoverBitmap();

	block->bufferOrigin.x = bufferOrigin.x - blockPos.x;
	block->bufferOrigin.y = bufferOrigin.y - blockPos.y;

	Detail& d0 = detailData[0];
	d0.cameraPos.x = (int(cameraPos.x) >> scale) - blockPos.x;
	d0.cameraPos.y = (int(cameraPos.y) >> scale) - blockPos.y;
	for (int i = 1; i <= scanDetailLevel; i++) {
		Detail& dd = detailData[i];
		dd.cameraPos.x = d0.cameraPos.x >> i;
		dd.cameraPos.y = d0.cameraPos.y >> i;
	}

	// All the bitmaps in the material list must be the
	// same size, so we'll just look at the first one.
	const GFXBitmap* bmp = (const GFXBitmap *)
		(*block->materialList)[0].getTextureMap();
	block->textureScale = getShift(bmp->getWidth());
	block->textureWorldScale = float(1 << file->scale) /
		float(1 << block->textureScale);

#ifdef DEBUG
   if (!defaultTexture)
      defaultTexture = (GFXBitmap *)(*block->materialList)[0].getTextureMap();
#endif

	// Scan the squares in the edge table.
	Detail* dp = &detailData[scanDetailLevel];
	const GridEdgeTable::Table& et = edgeTables.getEdgeTable(dp->level);

	Point2I maxSize;
	maxSize.x = block->size.x >> dp->level;
	maxSize.y = block->size.y >> dp->level;

	Point2I pos,end;
	pos.y = et.starty + dp->cameraPos.y;
	end.y = min(pos.y + et.edges.size(),maxSize.y);
	pos.y = max(pos.y,Int32(0));

   prepLightsBlock ();

	const GridEdgeTable::Edge* edge = &et.edges
		[pos.y - et.starty - dp->cameraPos.y];
	for (; pos.y < end.y; pos.y++) {
		pos.x = edge->fMin + dp->cameraPos.x;
		end.x = min(Int32(edge->fMax + dp->cameraPos.x + 1),maxSize.x);
		pos.x = max(pos.x,Int32(0));

		if (pos.x < end.x) {
			GridBlock::Height* hp = block->getHeight(dp->level,pos);
			int *bp = getBuffer(dp->level,pos);
			transformRow(pos,end.x,dp,hp,bp);

			for (; pos.x < end.x; pos.x++) {
				testSquare(pos,dp,hp,bp);
				hp += 1 << dp->level;
				bp += 1 << dp->level;
			}
		}
		edge++;
	}
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

float GridRender::interpolateHeight(const Point2I& pos,const Detail* dp,
	const GridBlock::Height *hp, float frac)
{
	int offset = 0;

	switch (((pos.y & 1) << 1) | (pos.x & 1)) {
		case 1:
			// Interplate horizontal
			offset = dp->hpOffsetX;
			break;
		case 2:
			// Interplate vertical
			offset = dp->hpOffsetY;
			break;
		case 3:
			// Pick & Interplate diagonal base on how the
			// parent is split
			offset = dp->hpOffsetY;
			if ((pos.x ^ pos.y) & 2)
				offset -= dp->hpOffsetX;
			else
				offset += dp->hpOffsetX;
			break;
	}
	//
	float target = ((hp + offset)->height + (hp - offset)->height) * 0.5f;
	return (frac >= 1.0f)? target:
		hp->height + (target - hp->height) * frac;
}


//---------------------------------------------------------------------------


void GridRender::transform(const Point3F& vector,float distance,int* bp)
{
	*bp = pointList.size();
	pointList.increment();
	Point* point = &pointList[*bp];
	point->buffer = bp;

   point->distance = distance;

	point->paIndex = pointArray->addPoint(vector);
	TS::TransformedVertex& tv = pointArray->getTransformedVertex(point->paIndex);
   tv.fDist = distance;

	if (distance > visibleDistance && !isOrtho)
		// spherical far clipping.
	   tv.fStatus |= TS::ClipFarSphere;

   tv.fColor.haze = getHaze( distance );
}


//---------------------------------------------------------------------------

void GridRender::transform(const Point2I& pos,const Detail* dp,
	const GridBlock::Height *hp,int* bp)
{
	Point3F vector;
	int shift = dp->level + scale;
	vector.x = block->origin.x + float(pos.x << shift);
	vector.y = block->origin.y + float(pos.y << shift);
	vector.z = hp->height;
	float distance = m_distf(vector,cameraPos);
	if (!block->isHeightPinned(hp) && distance > dp->minDistance) {
		float frac = (distance - dp->minDistance) * dp->distanceScale;
		vector.z = interpolateHeight(pos,dp,hp,frac);
	}
	transform(vector,distance,bp);
	GridMetrics.render.incVertices();
}


//---------------------------------------------------------------------------

void GridRender::transformCross(const Point2I& pos,
	const Detail* dp,const GridBlock::Height* hp,int* bp)
{
	if (!bp[dp->bpOffsetY])
		transform(Point2I(pos.x,pos.y+1),
			dp,hp + dp->hpOffsetY,bp + dp->bpOffsetY);
	if (!bp[dp->bpOffsetXY])
		transform(Point2I(pos.x+1,pos.y+1),
			dp,hp + dp->hpOffsetXY,bp + dp->bpOffsetXY);
	if (!bp[dp->bpOffset2XY])
		transform(Point2I(pos.x+2,pos.y+1),
			dp,hp + dp->hpOffset2XY,bp + dp->bpOffset2XY);
	if (!bp[dp->bpOffsetX2Y])
		transform(Point2I(pos.x+1,pos.y+2),
			dp,hp + dp->hpOffsetX2Y,bp + dp->bpOffsetX2Y);
	if (!bp[dp->bpOffsetX])
		transform(Point2I(pos.x+1,pos.y),
			dp,hp + dp->hpOffsetX,bp + dp->bpOffsetX);
}


//---------------------------------------------------------------------------

void GridRender::transformRow(Point2I pos,int endx,
	const Detail* dp,const GridBlock::Height* hp,int *bp)
{
	// Transforms all points for squares pos.x to endx
	// No pinning or interpolation
	float vectorInc = float(1 << (dp->level + scale));
	Point3F vector1,vector2;
	vector1.x = block->origin.x + float(pos.x << (dp->level + scale));
	vector1.y = block->origin.y + float(pos.y << (dp->level + scale));
	vector2.x = vector1.x;
	vector2.y = vector1.y + vectorInc;

	for (; pos.x <= endx; pos.x++) {
		if (!*bp) {
			vector1.z = hp->height;
			transform(vector1,m_distf(vector1,cameraPos),bp);
		}
		if (!bp[dp->bpOffsetY]) {
			vector2.z = hp[dp->hpOffsetY].height;
			transform(vector2,m_distf(vector2,cameraPos),bp + dp->bpOffsetY);
		}
		vector1.x += vectorInc;
		vector2.x += vectorInc;
		hp += dp->hpOffsetX;
		bp += dp->bpOffsetX;
	}
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

bool GridRender::isVisible(const Point2I& pos,const Detail* dp)
{
	return edgeTables.isVisible(dp->level,
		Point2I(pos.x - dp->cameraPos.x,pos.y - dp->cameraPos.y));
}


//---------------------------------------------------------------------------

float GridRender::getDistance(const Detail* dp,const int* bp)
{
	Point* pl = pointList.address();
	float distance = pl[bp[0]].distance;
	distance += pl[bp[dp->bpOffsetY]].distance;
	distance += pl[bp[dp->bpOffsetXY]].distance;
	distance += pl[bp[dp->bpOffsetX]].distance;
	return distance * (1.0f / 4.0f);
}


//---------------------------------------------------------------------------

void GridRender::testSquare(Point2I pos,const Detail* dp,
	const GridBlock::Height* hp,int* bp)
{
	// Check first to see if it's in the viewcone.
	if (isVisible(pos,dp)) {
		// Check to see if it's marked as empty
		GridBlock::Material* material = 
			block->getMaterial(dp->level,pos);
		if (dp->level >= material->getEmptyLevel()) {
			// See if we want to decompress it.
			bool higher = false;
			float distance = getDistance(dp,bp);
			if (dp != detailData) {
				if (block->isSquarePinned(dp->level,pos))
					higher = true;
				else
					higher = distance < dp->detailDistance;
			}
			if (higher) {
				// Transform/project inner points
				// then process all four sub-squares
				dp--; pos <<= 1;
				transformCross(pos,dp,hp,bp);
				testSquare(pos,dp,hp,bp);
				hp += dp->hpOffsetX;
				bp += dp->bpOffsetX;
				pos.x++;
				testSquare(pos,dp,hp,bp);
				hp += dp->hpOffsetY;
				bp += dp->bpOffsetY;
				pos.y++;
				testSquare(pos,dp,hp,bp);
				hp -= dp->hpOffsetX;
				bp -= dp->bpOffsetX;
				pos.x--;
				testSquare(pos,dp,hp,bp);
			}
			else {
				// Queue square for rendering
				squareList.increment();
				Square& square = squareList.last();
				square.material = material;
				square.distance = distance;
				square.block = block;
				square.split = (pos.x ^ pos.y) & 1;
				square.pos = pos;
				square.dp = dp;
				square.hp = hp;
				square.bp = bp;
            square.flags = 0;
            square.blockOrg.x = block->origin.x;
            square.blockOrg.y = block->origin.y;
            if ( isPossiblyLit() )
               if ( isUnderLights(square) )
                  square.flags |= Square::SqrUnderDLight;
			}
		}
	}
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

int GridRender::getSquarePoints(const Detail* dp,const int* bp,Point** points)
{
	Point* pl = pointList.address();
	int px,mask = 0;

	if ((px = bp[0]) != 0)
      { points[0] = pl + px; mask |= 1 << 0; }
	if ((px = bp[dp->bpOffsetY]) != 0)
		{ points[2] = pl + px; mask |= 1 << 2; }
	if ((px = bp[dp->bpOffsetXY]) != 0)
		{ points[4] = pl + px; mask |= 1 << 4; }
	if ((px = bp[dp->bpOffsetX]) != 0)
		{ points[6] = pl + px; mask |= 1 << 6; }

	if (dp != detailData) {
		dp--;
		if ((px = bp[dp->bpOffsetY]) != 0)
			{ points[1] = pl + px; mask |= 1 << 1; }
		if ((px = bp[dp->bpOffsetXY]) != 0)
			{ points[9] = pl + px; mask |= 1 << 9; }
		if ((px = bp[dp->bpOffset2XY]) != 0)
			{ points[5] = pl + px; mask |= 1 << 5; }
		if ((px = bp[dp->bpOffsetX2Y]) != 0)
			{ points[3] = pl + px; mask |= 1 << 3; }
		if ((px = bp[dp->bpOffsetX]) != 0)
			{ points[7] = pl + px; mask |= 1 << 7; }
	}
	return mask;
}


//---------------------------------------------------------------------------


int inline GridRender::getTextureShift( int blkTxtrScale , int mipLevel, int detail )
{
   int   txtrShift = blkTxtrScale - (mipLevel + terrainTextureDetail);
   
   if( txtrShift < 2 )
      txtrShift = 2;

   if( (txtrShift += detail) > 8 )
      txtrShift = 8;
      
   return txtrShift;
}


namespace OpenGL {
extern void RegisterTextureCBTer(GFXSurface*           io_pSurface,
                                 GFXTextureHandle      in_handle,
                                 GFXCacheCallback      in_cb,
                                 int                   in_csizeX,
                                 GFXLightMap*          io_pLightMap,
                                 GFXBitmap::CacheInfo* io_pCacheInfo,
                                 const RectI&          io_rSubSection);
extern void RegisterTextureTer(GFXSurface*           io_pSurface,
                               GFXTextureHandle      in_handle,
                               int                   in_sizeX,
                               int                   in_sizeY,
                               GFXLightMap*          io_pLightMap,
                               const GFXBitmap*      in_pTexture,
                               int                   /*in_mipLevel*/,
                               GFXBitmap::CacheInfo* io_pCacheInfo,
                               const RectI&          io_rSubSection,
                               const int             in_flags);
extern void RegisterTextureTerCover(GFXSurface*           io_pSurface,
                                    GFXTextureHandle      in_handle,
                                    GFXLightMap*          io_pLightMap,
                                    const GFXBitmap*      in_pTexture,
                                    GFXBitmap::CacheInfo* io_pCacheInfo);
}

//---------------------------------------------------------------------------
void
GridRender::finishSquareAsTempOGL(const Square&        square,
                                  int                  numVisPolys,
                                  int*                 polyVis,
                                  TS::VertexIndexPair* ilist)
{
   int detail = square.dp->level;
   GridBlock::Material* material = square.material;


   Point2F l_textureCoor[9];
   if (!detail) {
      memcpy(l_textureCoor, TextureCoor[material->flags & GridBlock::Material::RotateMask], 9 * sizeof(float));
   } else {
      memcpy(l_textureCoor, TextureCoor[0], 9 * sizeof(float));
   }
   pointArray->useTextures(l_textureCoor);

   float startX = square.pos.x;
   float endX   = square.pos.x + (1 << detail);
   float startY = square.pos.y;
   float endY   = square.pos.y + (1 << detail);

   for (int i = 0; i < 9; i++) {
      l_textureCoor[i].x = (startX + l_textureCoor[i].x * (endX - startX)) / float(square.block->size.x);
      l_textureCoor[i].y = (startY + l_textureCoor[i].y * (endY - startY)) / float(square.block->size.y);
   }

   GFXTextureHandle handle;
   buildGlobalKey(handle, square.block);

   if (!gfxSurface->setTextureHandle(handle)) {
      // Handle not present...
      GFXBitmap* pBitmap           = square.block->getCoverBitmap();
      GFXLightMap* pLMap           = square.block->pSubLightmaps[0];
      GFXBitmap::CacheInfo* pCInfo = &(square.block->pSubLMCacheInfo[0]);

      OpenGL::RegisterTextureTerCover(gfxSurface,
                                      handle,
                                      pLMap,
                                      pBitmap,
                                      pCInfo);
   } else {
      // Handle present...
   }

   // Setup GFX state Haze overrides Persp.
   float squareSize = float(1 << (scale + detail));
   if(!isOrtho)
   {
      if (square.distance > hazeDistance - squareSize) {
         gfxSurface->setHazeSource(GFX_HAZE_VERTEX);
      } else {
         gfxSurface->setHazeSource(GFX_HAZE_NONE);
      }
   }   
   // draw the polygons
   drawPolys( numVisPolys, polyVis, ilist );
}

void
GridRender::drawSquareOGL(const Square& square, bool drawPass)
{
   GridMetrics.render.incSquares();
   int detail = square.dp->level;

   // Check material and setup the texture coors. based
   // on material rotation (only for level 0 squares)
   GridBlock::Material* material = square.material;
   if (!detail)
      pointArray->useTextures( TextureCoor[material->flags & GridBlock::Material::RotateMask] );
   else
      pointArray->useTextures(TextureCoor[0]);

   int numVisPolys;
   
   // need to have room for 6 polys ( 3 per face )
   int polyVis[6];
   TS::VertexIndexPair ilist[18];
   
   // get the list of polys that are visible in this square
   if( ( numVisPolys = buildPolyList( square, polyVis, ilist ) ) == 0 )
      return;
      
   // GFX Texture cache and lightmap management
   GFXTextureHandle gfxHandle;
   buildKey(gfxHandle,square.block,detail,square.pos);
   if( square.isLit () ) 
      makeKeyLit( gfxHandle );

   float squareSize = float(1 << (scale + detail));
   float squareDist = square.distance - squareSize;
   int mipLevel = (squareDist <= 0.0f)? 0:
      getPower(int(1.0f / camera->projectRadius
         (squareDist,block->textureWorldScale)));
         
   AssertFatal(dynamic_cast<OpenGL::Surface*>(gfxSurface) != NULL, "Error, not an opengl surface!");
   if (!gfxSurface->setTextureHandle(gfxHandle))
   {
      if (drawPass == true) {
//         finishSquareAsTempOGL(square, numVisPolys, polyVis, ilist);
         AssertWarn(false, "This should never happen");
         return;
      }
//      else if (++g_downloadCount > sg_downloadCap && detail != 0) {
//         return;
//      }

      if (detail) 
      {
         int textureShift = getTextureShift( block->textureScale, mipLevel, detail );

         GFXLightMap*          pLightMap  = NULL;
         GFXBitmap::CacheInfo* pCacheInfo = NULL;
         RectI subRegion;
         buildLightMapOGL(square, GridBlock::Material::Plain, textureShift, 
                          pLightMap, pCacheInfo, subRegion);
         AssertFatal(pLightMap != NULL, "No lightmap?");

         if (pCacheInfo == NULL) {
            gfxSurface->registerTexture(gfxHandle,
                                        buildTexture,
                                        1 << textureShift,
                                        1 << textureShift,
                                        square.lightShift(textureShift),
                                        pLightMap);
         } else {
            OpenGL::RegisterTextureCBTer(gfxSurface,
                                         gfxHandle,
                                         buildTexture,
                                         1 << textureShift,
                                         pLightMap,
                                         pCacheInfo,
                                         subRegion);
         }

         GridMetrics.render.incTextureRegister();
      }
      else 
      {
         GFXLightMap*          pLightMap  = NULL;
         GFXBitmap::CacheInfo* pCacheInfo = NULL;
         RectI subRegion;
         buildLightMapOGL(square, material->flags, 1000, 
                          pLightMap, pCacheInfo, subRegion);
         AssertFatal(pLightMap != NULL, "No lightmap?");

         int textureWidth = 1 << block->textureScale;

         if (pCacheInfo == NULL) {
            gfxSurface->registerTexture
            (  
                  gfxHandle,
                  textureWidth,textureWidth,0,0,
                  square.lightShift(block->textureScale),
                  pLightMap,
                  (*block->materialList)[material->index].getTextureMap(),
                  mipLevel+terrainTextureDetail
            );
         } else {
            OpenGL::RegisterTextureTer(gfxSurface,
                                       gfxHandle,
                                       textureWidth, textureWidth,
                                       pLightMap,
                                       (*block->materialList)[material->index].getTextureMap(),
                                       mipLevel+terrainTextureDetail,
                                       pCacheInfo,
                                       subRegion, material->flags);
         }

         GridMetrics.render.incTextureRegister();
      }
   }
   else     // handle found
   {
      if (drawPass == false) {
         int   txtrShift = -1;
      
         if( detail )
            txtrShift = getTextureShift( block->textureScale, mipLevel, detail );

         GFXLightMap*          pLightMap  = NULL;
         GFXBitmap::CacheInfo* pCacheInfo = NULL;
         RectI subRegion;

         if ( square.isLit () ) {
            if ( detail ) {
               buildLightMapOGL(square, GridBlock::Material::Plain, 1000, 
                                pLightMap, pCacheInfo, subRegion);
            } else {
               buildLightMapOGL(square, material->flags, 1000, 
                                pLightMap, pCacheInfo, subRegion);
            }
         }

         if( ! detail ) {
            if ( pLightMap != NULL ) {
               OpenGL::HandleSetLightMapTer(gfxSurface,
                                            pLightMap,
                                            pCacheInfo == NULL ? new GFXBitmap::CacheInfo(0xfffffffe) : pCacheInfo);
            }
         }
         else
         {
            gfxSurface->handleSetTextureSize ( 1 << txtrShift );
            if ( pLightMap != NULL ) {
               OpenGL::HandleSetLightMapTer(gfxSurface,
                                            pLightMap,
                                            pCacheInfo == NULL ? new GFXBitmap::CacheInfo(0xfffffffe) : pCacheInfo);
            }
         }
      } else {
         // Set in the first pass...
      }
   }


   if (drawPass == true) {
      // Setup GFX state Haze overrides Persp.
      if(!isOrtho)
      {
         if (square.distance > hazeDistance - squareSize) {
            gfxSurface->setHazeSource(GFX_HAZE_VERTEX);
         } else {
            gfxSurface->setHazeSource(GFX_HAZE_NONE);
         }
      }   
      // draw the polygons
      drawPolys( numVisPolys, polyVis, ilist );
   }
}


void GridRender::drawSquare(const Square& square)
{
   GridMetrics.render.incSquares();
   int detail = square.dp->level;

   // Check material and setup the texture coors. based
   // on material rotation (only for level 0 squares)
   GridBlock::Material* material = square.material;
   if (!detail)
      pointArray->useTextures( TextureCoor[material->flags & GridBlock::Material::RotateMask] );
   else
      pointArray->useTextures(TextureCoor[0]);

   int numVisPolys;
   
   // need to have room for 6 polys ( 3 per face )
   int polyVis[6];
   TS::VertexIndexPair ilist[18];
   
   // get the list of polys that are visible in this square
   if( ( numVisPolys = buildPolyList( square, polyVis, ilist ) ) == 0 )
      return;
      
   // GFX Texture cache and lightmap management
   GFXTextureHandle gfxHandle;
   buildKey(gfxHandle,square.block,detail,square.pos);
   if( square.isLit () ) 
      makeKeyLit( gfxHandle );

   float squareSize = float(1 << (scale + detail));
   float squareDist = square.distance - squareSize;
   int mipLevel = (squareDist <= 0.0f)? 0:
      getPower(int(1.0f / camera->projectRadius
         (squareDist,block->textureWorldScale)));
         
         
   if (!gfxSurface->setTextureHandle(gfxHandle))
   {
      if (detail) 
      {
         //int textureShift = max(2,block->textureScale - (mipLevel+terrainTextureDetail)) + detail;
         //if(textureShift > 8)
         //   textureShift = 8;
         int textureShift = getTextureShift( block->textureScale, mipLevel, detail );
         GFXLightMap * lightMap = buildLightMap(square, GridBlock::Material::Plain,textureShift);
         gfxSurface->registerTexture(gfxHandle,buildTexture,
                                     1 << textureShift,1 << textureShift,
                                     square.lightShift(textureShift),lightMap);
         GridMetrics.render.incTextureRegister();
      }
      else 
      {
#ifdef DEBUG
         int maxMatIndex = block->materialList->getMaterialsCount();
         if (material->index >= maxMatIndex) {
            static bool warned;
            AssertWarn(warned,"GridFile::drawSquare: "
               "Material index out of range.");
            warned = true;
         }
#endif
         GFXLightMap* lightMap = buildLightMap(square,material->flags);
         int textureWidth = 1 << block->textureScale;
         gfxSurface->registerTexture
         (  
               gfxHandle,
               textureWidth,textureWidth,0,0,
               square.lightShift(block->textureScale),lightMap,
#ifdef DEBUG
               (material->index >= maxMatIndex) ? defaultTexture :
                  (*block->materialList)[material->index].getTextureMap(),
#else
               (*block->materialList)[material->index].getTextureMap(),
#endif
               mipLevel+terrainTextureDetail
         );

         GridMetrics.render.incTextureRegister();
      }
   }
   else     // handle found
   {
      int   txtrShift = -1;
      
      if( detail )
         txtrShift = getTextureShift( block->textureScale, mipLevel, detail );
   
      GFXLightMap* lightMap = NULL;
      if ( square.isLit () )
         if ( detail )
            lightMap = buildLightMap(square, GridBlock::Material::Plain, txtrShift);
         else
            lightMap = buildLightMap(square, material->flags);

      
      if( ! detail )
      {
         gfxSurface->handleSetMipLevel(mipLevel+terrainTextureDetail);
         if ( lightMap != NULL )
            gfxSurface->handleSetLightMap(square.lightShift(block->textureScale), lightMap);
      }
      else
      {
         gfxSurface->handleSetTextureSize ( 1 << txtrShift );
         // min(1 << (max(2,block->textureScale - (mipLevel+terrainTextureDetail)) + detail), 256
         if ( lightMap != NULL )
            gfxSurface->handleSetLightMap(square.lightShift(txtrShift), lightMap);
      }
   }

   // Setup GFX state Haze overrides Persp.
   if(!isOrtho)
   {
      if (square.distance > hazeDistance - squareSize)
      {
         gfxSurface->setHazeSource(GFX_HAZE_VERTEX);
         gfxSurface->setTexturePerspective(false);
      }
      else
      {
         gfxSurface->setHazeSource(GFX_HAZE_NONE);
         if (square.distance < perspectiveDistance + squareSize) 
            gfxSurface->setTexturePerspective(true);
         else 
            gfxSurface->setTexturePerspective(false);
      }
   }   
   // draw the polygons
   drawPolys( numVisPolys, polyVis, ilist );
}


//---------------------------------------------------------------------------

int GridRender::buildPolyList( const Square & square, int * polyVis, TS::VertexIndexPair * ilist )
{
   // get the points for the square   
	Point * points[10];
	int indexBits = ( getSquarePoints( square.dp, square.bp, points ) & 0xff );
   
   int numVisPolys = 0;
   
   // generate a list of polys to test
   for( int f = 0; f < 2; f++ )
   {
      // grab the face
      Face & face = FaceList[ ( square.split << 1 ) + f ];
		int faceStart = face.startIndex;
		int faceType = ( ( indexBits << face.rollCount ) |
			( indexBits >> ( 8 - face.rollCount ) ) ) & 0x1f;
      
      Poly & polyList = PolyList[ faceType ];
      int * vertexList = polyList.vertexList;
   
      // go through the polys on this face
      for( int p = polyList.polyCount; p > 0; p-- )
      {
         // grab the index's
         int a, b, c;
         a = ( faceStart + vertexList[0] ) & 0x7;
         b = ( faceStart + vertexList[1] ) & 0x7;
         c = ( faceStart + vertexList[2] ) & 0x7;
         
         // grab the points
         int pa, pb, pc;
         pa = points[a]->paIndex;
         pb = points[b]->paIndex;
         pc = points[c]->paIndex;

         // check if visible
         polyVis[ numVisPolys ] = faceVisibility( pa, pb, pc );
         if( polyVis[ numVisPolys ] != TS::ClipNoneVis )
         {
            // int offset = numVisPolys * 3;
            
            // ilist[ offset ].fTextureIndex = a;
            // ilist[ offset ].fVertexIndex = pa;
            // ilist[ offset + 1 ].fTextureIndex = b;
            // ilist[ offset + 1 ].fVertexIndex = pb;
            // ilist[ offset + 2 ].fTextureIndex = c;
            // ilist[ offset + 2 ].fVertexIndex = pc;

            // NOTE:  the following was changed as an optimization attempt, but it 
            //    relies on the invariant that ilist's size == 3 * numVisPolys, and 
            //    that ilist isn't used elsewhere in the routine.  
            ilist   ->  fTextureIndex = a;
            ilist++ ->  fVertexIndex = pa;
            ilist   ->  fTextureIndex = b;
            ilist++ ->  fVertexIndex = pb;
            ilist   ->  fTextureIndex = c;
            ilist++ ->  fVertexIndex = pc;
            
            // increment the number of polys visible
            numVisPolys++;   
         }
         
         // get to the next set of vertices
         vertexList += 3;
      }
   }
   
   return( numVisPolys );
}

//---------------------------------------------------------------------------

void GridRender::drawPolys( const int numVisPolys, const int * polyVis,
   const TS::VertexIndexPair * ilist )
{
   // walk through the poly list
   for( int i = 0; i < numVisPolys; i++ )
   {
      // set the clipping mask and draw the poly
      pointArray->setVisibility( polyVis[i] );
      pointArray->drawTriangle( &ilist[i*3], 0 );
      GridMetrics.render.incPolys();
   }
}

//---------------------------------------------------------------------------

int GridRender::faceVisibility(int va,int vb,int vc)
{
   TS::TransformedVertex *v0, *v1, *v2;
   v0 = &pointArray->getTransformedVertex(va);
   v1 = &pointArray->getTransformedVertex(vb);
   v2 = &pointArray->getTransformedVertex(vc);
   
	int or = TS::ClipMask & (v0->fStatus | v1->fStatus | v2->fStatus);
	if (!or) {
	   // No vertices are clipped, do cross product in screen space
		if (((v2->fPoint.x - v1->fPoint.x) * (v0->fPoint.y - v1->fPoint.y) - 
			(v2->fPoint.y - v1->fPoint.y) * (v0->fPoint.x - v1->fPoint.x)) >= 0.0f)
			return TS::ClipAllVis;
	}
	else {
		// Make sure it's not totally out of the viewport
		if (!(TS::ClipMask & (v0->fStatus & v1->fStatus & v2->fStatus))) {
			// Cross product to get normal of poly, then
			// dot with vector to origin.
			Point3F x3;
         Point3F x1(v0->fTransformedPoint.x - v1->fTransformedPoint.x,
            v0->fTransformedPoint.y - v1->fTransformedPoint.y,
            v0->fTransformedPoint.w - v1->fTransformedPoint.w);
         Point3F x2(v2->fTransformedPoint.x - v1->fTransformedPoint.x,
            v2->fTransformedPoint.y - v1->fTransformedPoint.y,
            v2->fTransformedPoint.w - v1->fTransformedPoint.w);
         Point3F vx1(v0->fTransformedPoint.x, v0->fTransformedPoint.y,
                  v0->fTransformedPoint.w);
			m_cross(x1,x2,&x3);
			if (m_dot(x3,vx1) > 0.0f)
			   return or;
		}
	}

   return TS::ClipNoneVis;
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void
GridRender::buildLightMapOGL(const Square&          square,
                             int                    flags,
                             int                    txtShift,
                             GFXLightMap*&          out_rpLightMap,
                             GFXBitmap::CacheInfo*& out_rpCacheInfo,
                             RectI&                 out_rSubRegion)
{
	GridMetrics.render.incLightMapBuild();
   AssertFatal ( square.block->lightScale == 0, "grdRender: non-zero light scale" 
                        " untested with the terrain dynamic lights" );
   
   int   level = square.dp->level;
   int   levelInc, falloffLevel;
   
   if ( square.isLit () )
      falloffLevel = 0, levelInc = 4;
   else
      falloffLevel = levelInc = 0;
         
   if( level > falloffLevel )
      if ( (levelInc -= (level - falloffLevel)) < 0 )
         levelInc = 0;

   // Cap our expanded size according to the txtShift (which specifies the size of 
   //    texture as a power of two).  The software renderer has some problems when
   //    the light map is larger than the texture (although the light map can be 
   //    bigger by ONE, such as 17x17 for a 16x16 texture).  
   if( (level + levelInc) > txtShift )
   {
      // Will this IF ever happen?  
      if( (levelInc = txtShift - level) < 0 )
         levelInc = 0;
   }
         
   square.stuffLevelInc( levelInc );
      
   if (square.isLit()) {
      // We have to build the dynamic lightmap here.
      //
      out_rpCacheInfo = NULL;

      int lmSize = (1 << (level + levelInc)) + 1;
      GFXLightMap* pLightMap = gfxSurface->allocateLightMap(lmSize * lmSize);
      pLightMap->size.x   = pLightMap->size.y   = lmSize;
      pLightMap->offset.x = pLightMap->offset.y = 0;
      UInt16* ld = &pLightMap->data;

      square.block->fillLightMap(ld, square.pos, level, levelInc, flags, false);
      dlightMatFlags = flags;
      htStepBits     = levelInc;
      mergeAllDLights(pLightMap);
      
      out_rpLightMap = pLightMap;
   } else {
      // We want to use a whole lightmap here...
      //
      int startX = square.pos.x << level;
      int startY = square.pos.y << level;
      int endX   = startX + (1 << level);
      int endY   = startY + (1 << level);

      AssertFatal(square.block->subLightmapSide != 0, "No sub lightmaps!");
      if (square.block->subLightmapSide == 1) {
         out_rpLightMap  = square.block->pSubLightmaps[0];
         out_rpCacheInfo = square.block->pSubLMCacheInfo;

         out_rSubRegion.upperL.x = startX;
         out_rSubRegion.upperL.y = startY;
         out_rSubRegion.lowerR.x = endX;
         out_rSubRegion.lowerR.y = endY;
      } else {
         AssertFatal(square.block->subLightmapSide == 2, "Not yet supported");

         const Int32 coordSpots[2] = { 0, 1 };
         
         Int32 xBlock = -1;
         Int32 yBlock = -1;

         int i;
         for (i = 0; i < 2; i++) {
            if (coordSpots[i]       <= startX &&
                coordSpots[i] + 255 >= endX) {
               xBlock = i;
               break;
            }
         }
         for (i = 0; i < 2; i++) {
            if (coordSpots[i]       <= startY &&
                coordSpots[i] + 255 >= endY) {
               yBlock = i;
               break;
            }
         }
         AssertFatal(xBlock != -1 && yBlock != -1, "No lightmap block works?");

         out_rpLightMap  = square.block->pSubLightmaps[yBlock * 2 + xBlock];
         out_rpCacheInfo = &(square.block->pSubLMCacheInfo[yBlock * 2 + xBlock]);

         out_rSubRegion.upperL.x = startX - coordSpots[xBlock];
         out_rSubRegion.upperL.y = startY - coordSpots[yBlock];
         out_rSubRegion.lowerR.x = endX   - coordSpots[xBlock];
         out_rSubRegion.lowerR.y = endY   - coordSpots[yBlock];
      }
   }
}


GFXLightMap* GridRender::buildLightMap(const Square& square,int flags, int txtShift)
{
	GridMetrics.render.incLightMapBuild();
   AssertFatal ( square.block->lightScale == 0, "grdRender: non-zero light scale" 
                        " untested with the terrain dynamic lights" );
   
   static int  litHRFalloff = 2;
   static int  unlitHRFalloff = 1;

   // we only need to do the following check when the light map is built, and draw
   // code then uses that data, so we "cheat" and stuff some data into the otherwise
   // now const square.  
   square.checkHighRes ();

   int   level = square.dp->level;
   bool  hasHRLM = square.hasHighRes ();
   
   int   levelInc, falloffLevel;
   
   if ( square.isLit () )
      if ( hasHRLM )
         falloffLevel = litHRFalloff, levelInc = 4;
      else
         falloffLevel = 0, levelInc = 4;
   else
      if( hasHRLM )
         falloffLevel = unlitHRFalloff, levelInc = 4;
      else
         falloffLevel = levelInc = 0;
         
   if( level > falloffLevel )
      if ( (levelInc -= (level - falloffLevel)) < 0 )
         levelInc = 0;

   // Cap our expanded size according to the txtShift (which specifies the size of 
   //    texture as a power of two).  The software renderer has some problems when
   //    the light map is larger than the texture (although the light map can be 
   //    bigger by ONE, such as 17x17 for a 16x16 texture).  
   if( (level + levelInc) > txtShift )
   {
      // Will this IF ever happen?  
      if( (levelInc = txtShift - level) < 0 )
         levelInc = 0;
   }
         
   square.stuffLevelInc( levelInc );
      
   int   lmSize = (1 << (level + levelInc)) + 1;
   
	GFXLightMap * lightMap = gfxSurface->allocateLightMap(lmSize * lmSize);
	lightMap->size.x = lightMap->size.y = lmSize;
	lightMap->offset.x = lightMap->offset.y = 0;
   UInt16 *ld = &lightMap->data;

   square.block->fillLightMap( ld, square.pos, level, levelInc, flags, hasHRLM );
      
   #if DEBUG_LIGHTS
      if ( debugDarken ){
         // doh! We have a clash of debugging features: don't darken if a high 
         //    res node exists for this square:
         if ( !square.block->hrLightMaps.findHighResNode( square.pos, level ) )
            for ( int i = lmSize * lmSize; i > 0; i--, ld++ )
               *ld = DebugDarken ( *ld );
      }
   #endif
   
   if ( square.isLit () ) { 
      dlightMatFlags = flags;
      htStepBits = levelInc;
      mergeAllDLights ( lightMap );
   }
   
   #if (_DEBUG_LIGHT_DUMP && 0)
      // testing code to leave high res light maps laying around.  the 
      //   manager will merge to highest level if we overlay.  
      if ( level == 0 && levelInc == 4 ) {
         square.block->hrLightMaps.setMaxLMaps ( _DEBUG_LIGHT_DUMP );
         HiresLightMap  * hrlm = new HiresLightMap ( square.pos, &lightMap->data );
         if ( ! square.block->hrLightMaps.push_back ( *hrlm ) )
            delete hrlm;
      }
   #endif
   
	return lightMap;
}


//---------------------------------------------------------------------------

extern DWORD gridCreateMipMap( int _mipLevel, Box2I& _area,
	 Point2I& _matMapSize, GridBlock::Material* _matMap,
	 TSMaterialList* _matList, int stride, BYTE *baseAddr, GFXBitmap* _defTex );

void GridRender::buildTexture(GFXTextureHandle handle,GFXBitmap* bitmap,int)
{
	GridMetrics.render.incTextureBuild();
	Point2I pos;
	GridBlock* block = getKeyBlock(handle);
	int detail = getKeyDetail(handle);
	getKeyPos(handle,pos);

	int bitmapShift = getShift(bitmap->getWidth());
	int mipLevel = block->textureScale - (bitmapShift - detail);

	Box2I mapArea;
	mapArea.fMin.x = pos.x << detail;
	mapArea.fMin.y = pos.y << detail;
	mapArea.fMax.x = mapArea.fMin.x + (1 << detail) - 1;
	mapArea.fMax.y = mapArea.fMin.y + (1 << detail) - 1;

	Point2I mapSize;
	mapSize.x = mapSize.y = block->getMaterialMapWidth();

#ifdef DEBUG
	bitmap->paletteIndex = gridCreateMipMap(mipLevel,mapArea,mapSize,
		block->materialMap,block->materialList,bitmap->stride, bitmap->pBits, defaultTexture);
#else
	bitmap->paletteIndex = gridCreateMipMap(mipLevel,mapArea,mapSize,
		block->materialMap,block->materialList,bitmap->stride, bitmap->pBits, 0);
#endif
}

