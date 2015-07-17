#include <stdlib.h>
#include <stdio.h>

#include "p_edge.h"
#include "p_funcs.h"
#include "g_pal.h"
#include "g_surfac.h"
#include "rn_manag.h"
#include "g_contxt.h"
#include "fn_all.h"
#include "p_txcach.h"
#include "gfxmetrics.h"


//-------------------------------------- Local Function Prototypes
void ClearEdgeLists(void);
void DrawSpans (GFXSurface *);
int  ScanEdges (int);
void AddPolygonEdges ( GFXPolyStore *screenpoly);


//-------------------------------------- Local vars
int           numPolys = 0;
GFXPolyStore* pavailsurf;

// NOTE!
// kInitialSpanTableSize must be greater than
// (kInitialSurfaceTableSize + kInitialTSurfaceTableSize) * 16

#define kInitialEdgeTableSize       1000
#define kInitialSurfaceTableSize    1000
#define kInitialTSurfaceTableSize   100
#define kInitialSpanTableSize       30000
#define kIncrementSurfaceTableSize  1000
#define kIncrementTSurfaceTableSize 100



extern "C" DWORD nextFreeBlock;

GFXPolyStore  surfs[1];
GFXPolyStore* solidSurfs       = NULL;
GFXPolyStore* transparentSurfs = NULL;

DWORD maxtsurfs = 0;
DWORD maxsurfs  = 0;

GFXPolyStore *curSolidSurf       = 0;
GFXPolyStore *curTransparentSurf = 0;
GFXPolyStore *maxSolidSurf       = 0;
GFXPolyStore *maxTransparentSurf = 0;

span_t* spans = NULL;
extern "C" DWORD maxspans;
extern bool g_triError;
extern bool g_wBuffer;

edge_t* edges    = NULL;
edge_t* lastedge = NULL;
int     maxedges = 0;

int xl, xr, yt, yb;
int height;
int currentcolor;
int numPoints, numTexPoints; // dummy vars for copying...
extern int sg_zsortEnable;
int g_polyFlags;

//------------------------------------------------------------------------------
//--------------------------------------

static int colorInc;

void 
resizeEdgeArray()
{
   // the funky thing about resizing this array is that
   // it can happen when there are already edges in the edge
   // table

   edge_t *oldEdgePtr = edges;

   maxedges += kInitialEdgeTableSize;
   edges = (edge_t *) realloc(edges, maxedges * sizeof(edge_t));

   int delta = int(edges) - int(oldEdgePtr);
   int i;
   for(i = 0; i < MAX_SCREEN_HEIGHT; i++)
   {
      edge_t **walk = &newedges[i];
      while(*walk)
      {
         *walk = (edge_t *) (int(*walk) + delta);
         walk = &(*walk)->pnext;
      }
   }
   pavailedge = (edge_t *) (int(pavailedge) + delta);
   lastedge = edges + maxedges;
}

void 
resizeSurfaceArray()
{
   DWORD oldRangeMin, oldRangeMax, delta;

   if(curSolidSurf == maxSolidSurf)
   {
      int oldmax = maxsurfs;

      oldRangeMin = DWORD(solidSurfs);
      oldRangeMax = DWORD(solidSurfs + maxsurfs);

      maxsurfs += kIncrementSurfaceTableSize;
      solidSurfs = (GFXPolyStore *) realloc(solidSurfs, maxsurfs * sizeof(GFXPolyStore));

      delta = int(solidSurfs) - oldRangeMin;
      
      curSolidSurf = solidSurfs + oldmax;
      maxSolidSurf = solidSurfs + maxsurfs;


      GFXPolyStore *walk = solidSurfs;
      while(walk < maxSolidSurf)
      {
         walk->myPoly = walk;
         walk++;
      }
   }
   else if(curTransparentSurf == maxTransparentSurf)
   {
      int oldmax = maxtsurfs;

      oldRangeMin = DWORD(transparentSurfs);
      oldRangeMax = DWORD(transparentSurfs + maxtsurfs);

      maxtsurfs += kIncrementTSurfaceTableSize;
      transparentSurfs = (GFXPolyStore *) realloc(transparentSurfs, maxtsurfs * sizeof(GFXPolyStore));

      delta = int(transparentSurfs) - oldRangeMin;

      curTransparentSurf = transparentSurfs + oldmax;
      maxTransparentSurf = transparentSurfs + maxtsurfs;

      GFXPolyStore *walk = transparentSurfs;
      while(walk < maxTransparentSurf)
      {
         walk->myPoly = walk;
         walk++;
      }
   }
   // gotta rearrange all the edges to point into the new surface stuff...
   int i;
   for(i = 0; i < MAX_SCREEN_HEIGHT;i++)
   {
      edge_t *walk = newedges[i];
      while(walk)
      {
         DWORD oldSptr = DWORD(walk->ptrailsurf);
         if(oldSptr >= oldRangeMin && oldSptr < oldRangeMax)
            walk->ptrailsurf = (GFXPolyStore *) (oldSptr + delta);
         oldSptr = DWORD(walk->pleadsurf);
         if(oldSptr >= oldRangeMin && oldSptr < oldRangeMax)
            walk->pleadsurf = (GFXPolyStore *) (oldSptr + delta);
         walk = walk->pnext;
      }
   }
   if((maxsurfs + maxtsurfs) * 16 > maxspans)
   {
      maxspans = (maxsurfs + maxtsurfs) * 16;
      spans = (span_t *) realloc(spans, sizeof(span_t) * (maxspans + 32));
   }
}

void 
GFXDraw3DBegin(GFXSurface *io_pSurface)
{
	numPolys = 0;
	RectI *clipRect = io_pSurface->getClipRect();
	xl = clipRect->upperL.x;
	xr = clipRect->lowerR.x;
	yt = clipRect->upperL.y;
	yb = clipRect->lowerR.y;
   height = io_pSurface->getHeight();
   //clipRect->lowerR.y + 1;
	currentcolor = 0;
   nextFreeBlock = 0;
	GFXMetrics.reset();

   int i;
   for(i = 0; i <= height; i++)
      newedges[i] = NULL;

   if(!edges)
   {
      for(i = 0; i < MAX_SCREEN_HEIGHT; i++)
         newedges[i] = NULL;
      maxedges = kInitialEdgeTableSize;
      edges = (edge_t *) malloc(sizeof(edge_t) * maxedges);
      lastedge = edges + maxedges;

      maxspans = kInitialSpanTableSize;
      spans = (span_t *) malloc(sizeof(span_t) * (maxspans + 32));

      maxsurfs = kInitialSurfaceTableSize;
      solidSurfs = (GFXPolyStore *) malloc(sizeof(GFXPolyStore) * maxsurfs);
      maxSolidSurf = solidSurfs + maxsurfs;

      maxtsurfs = kInitialTSurfaceTableSize;
      transparentSurfs = (GFXPolyStore *) malloc(sizeof(GFXPolyStore) * maxtsurfs);
      maxTransparentSurf = transparentSurfs + maxtsurfs;

      GFXPolyStore *walk = solidSurfs;
      while(walk < maxSolidSurf)
      {
         walk->myPoly = walk;
         walk++;
      }
      walk = transparentSurfs;
      while(walk < maxTransparentSurf)
      {
         walk->myPoly = walk;
         walk++;
      }
   }
   pavailedge = edges;
	curSolidSurf = solidSurfs;
	curTransparentSurf = transparentSurfs;

#ifdef DEBUG
   GFXMetrics.textureSpaceUsed       = 0;
   GFXMetrics.textureBytesDownloaded = 0;
   gfxTextureCache.frameU++;
#endif
   colorInc = 0;
}

void 
GFXSort3dEnd(GFXSurface *io_pSurface)
{
	spanListPtr = spans;

	//GFXMetrics.numSpansUsed = ScanEdges(height);
   if(g_wBuffer)
	   GFXMetrics.numSpansUsed = gfx_scan_edges(height);
	   //GFXMetrics.numSpansUsed = ScanEdges(height);
   else
	   GFXMetrics.numSpansUsed = gfx_scan_edges_z(height);

	DrawSpans(io_pSurface);
   if(nextFreeBlock >= maxspans)
   {
      maxspans += kInitialSpanTableSize;
      spans = (span_t *) realloc(spans, sizeof(span_t) * (maxspans + 32));
   }
   io_pSurface->setZTest(GFX_NO_ZTEST);
}

void GFXSortDrawPoint3d(GFXSurface *io_pSurface, const Point2I *in_at, float in_w, DWORD in_color)
{
	io_pSurface;
   if(curSolidSurf == maxSolidSurf)
      resizeSurfaceArray();
   
   if(pavailedge + 2 >= lastedge)
	   resizeEdgeArray();

	edge_t *e1 = pavailedge, *e2 = pavailedge + 1;
	pavailedge += 2;

	curSolidSurf->drawFunc = (rendererManager::renderSpanFunc) GFXRenderSpan;
	curSolidSurf->color = in_color;
	curSolidSurf->dwdx = 0;
	curSolidSurf->dwdy = 0;
	curSolidSurf->w0 = in_w;
	curSolidSurf->tce = NULL;

   curSolidSurf->spanList = curSolidSurf->curblock = nextFreeBlock += 16;
	curSolidSurf->flags = 0;

	e1->x = in_at->x << 20;
	e2->x = e1->x + (1 << 20);
	e2->dx = e1->dx = 0;
	e1->leading = 0;
	e2->leading = 0;
 	e2->pleadsurf = e1->ptrailsurf = NULL;
	e1->pleadsurf = e2->ptrailsurf = curSolidSurf;
	
	e1->lasty = e2->lasty = in_at->y;
	edge_t **temp = newedges + in_at->y;
	edge_t *wtemp = *temp;
	while(wtemp)
	{
		if(e1->x <= wtemp->x)
			break;
		temp = &(wtemp->pnext);
		wtemp = wtemp->pnext;
	}
	*temp = e1;
	e1->pnext = wtemp;
	temp = &e1->pnext;
	while(wtemp)
	{
		if(e2->x <= wtemp->x)
			break;
		temp = &(wtemp->pnext);
		wtemp = wtemp->pnext;
	}
	*temp = e2;
	e2->pnext = wtemp;
   curSolidSurf++;
}

void 
GFXSortDrawRect3d(GFXSurface *io_pSurface, const RectI *in_rect, float in_w, DWORD in_color)
{
	io_pSurface;
   
   if(curSolidSurf == maxSolidSurf)
      resizeSurfaceArray();
   
   if(pavailedge + 2 >= lastedge)
	   resizeEdgeArray();

	edge_t *e1 = pavailedge, *e2 = pavailedge + 1;
	pavailedge += 2;

	curSolidSurf->drawFunc = (rendererManager::renderSpanFunc) GFXRenderSpan;
	curSolidSurf->color = in_color;
	curSolidSurf->dwdx = 0;
	curSolidSurf->dwdy = 0;
	curSolidSurf->w0 = in_w;
	curSolidSurf->tce = NULL;

   curSolidSurf->spanList = curSolidSurf->curblock = nextFreeBlock += 16;
	curSolidSurf->flags = 0;

	e1->x = in_rect->upperL.x << 20;
	e2->x = (in_rect->lowerR.x + 1) << 20;
	e2->dx = e1->dx = 0;
	e1->leading = 0;
	e2->leading = 0;
 	e2->pleadsurf = e1->ptrailsurf = NULL;
	e1->pleadsurf = e2->ptrailsurf = curSolidSurf;
	e1->lasty = e2->lasty = in_rect->lowerR.y;
	edge_t **temp = newedges + in_rect->upperL.y;
	edge_t *wtemp = *temp;
	while(wtemp)
	{
		if(e1->x <= wtemp->x)
			break;
		temp = &(wtemp->pnext);
		wtemp = wtemp->pnext;
	}
	*temp = e1;
	e1->pnext = wtemp;
	temp = &e1->pnext;
	while(wtemp)
	{
		if(e2->x <= wtemp->x)
			break;
		temp = &(wtemp->pnext);
		wtemp = wtemp->pnext;
	}
	*temp = e2;
	e2->pnext = wtemp;
	curSolidSurf++;
}

void GFXSortEmitPoly(GFXSurface* /*io_pSurface*/)
{
	GFXMetrics.incEmittedPolys();
   if(g_triError)
   {
      g_triError = false;
      vertexCount = 0;
      return;
   }

   if(pavailedge + vertexCount >= lastedge)
	   resizeEdgeArray();

	GFXPolyStore *polySurf;
	if(g_renderTable.isTransparent())
	{
      if(curTransparentSurf == maxTransparentSurf)
         resizeSurfaceArray();
		polySurf = curTransparentSurf;
		polySurf->flags = FLAG_TRANSPARENT | g_polyFlags;
	}
	else
	{
      if(curSolidSurf == maxSolidSurf)
         resizeSurfaceArray();
		polySurf = curSolidSurf;
		polySurf->flags = g_polyFlags;
	}
	
	switch(g_fillMode)
	{
		case GFX_FILL_CONSTANT:
			polySurf->textureMap = NULL;
			polySurf->color = gfxPDC.color;
			polySurf->tce = NULL;
			break;
		case GFX_FILL_TEXTURE:
			polySurf->textureMap = gfxPDC.textureMap;
			polySurf->textureMapStride = gfxPDC.textureMapStride;
			polySurf->textureMapHeight = gfxPDC.textureMapHeight;
			polySurf->tce = NULL;
			break;
		case GFX_FILL_TWOPASS:
			polySurf->tce = gfxTextureCache.getCurrentEntry();
			break;
	}
	polySurf->shadeScale = gfxPDC.shadeScale;
	
	if(g_renderTable.calcDelta(&(polySurf->dwdx), vertexCount == 3))
	{
	   polySurf->shadeMap = gfxPDC.shadeMap;
#ifdef DEBUG
      if(g_renderTable.needsShadeMap())
      {
         int i;
         for(i = 0; i < 256; i++)
         {
            char c = polySurf->shadeMap[i];
            polySurf->shadeMap[i] = c;
         }
      }
#endif
	   polySurf->drawFunc = g_renderTable.getRenderer();
		polySurf->nextPoly = NULL;

		gfx_add_edges(polySurf); 
      polySurf->spanList = polySurf->curblock = nextFreeBlock += 16;

		if(polySurf->flags & FLAG_TRANSPARENT)
			curTransparentSurf++;
		else
			curSolidSurf++;
	}
	else
	{
		GFXMetrics.incBackfacedPolys();
	}
	vertexCount = 0;
}

edge_t **addEdgeList;

ClippedPolyStore clipList[MAX_CLIP_SPANS];

int usedSpans;
int curY;
int clipIndex = 0;
GFXPolyStore *activePolyList, *transPolyList;


#define XFixedPointShift	20
#define XFixedPointCeilStep	0x000FFFFF
#define XFixedPointHalfStep	0x0007FFFF

void emit(int start, int end, GFXPolyStore *poly)
{
   if(start >= end)
      return;

   if(nextFreeBlock >= maxspans)
      return;

   span_t span;
   span.u.sp.y = curY;
   span.u.sp.x = start;
   span.u.sp.count = end - start;

   if((poly->curblock & 0xF) == 0xF)
   {
      DWORD nextbl = nextFreeBlock += 16;
      spanListPtr[poly->curblock].u.num = nextbl;
      poly->curblock = nextbl;
   }

   spanListPtr[poly->curblock++] = span;
}

void resort(GFXPolyStore *start, GFXPolyStore *new_elem)
{
   while(start->nextPoly && start->nextPoly->xstart < new_elem->xstart)
      start = start->nextPoly;
   new_elem->nextPoly = start->nextPoly;
   start->nextPoly = new_elem;
}

void insertReverse(GFXPolyStore **transPtr, GFXPolyStore *transPoly)
{
                     
   while(*transPtr && (*transPtr)->xstart > transPoly->xstart)
      transPtr = &(*transPtr)->nextPoly;

   transPoly->nextPoly = *transPtr;
   *transPtr = transPoly;
}

int ScanEdges (int dstY)
{
   // remove, step, insert new edges and build poly store linked list.
   edgehead.pnext = &edgetail;
   edgetail.pprev = &edgehead;

   addEdgeList = newedges;
   //GFXPolyStore *backgroundPoly;

   for(curY = 0; curY < dstY; curY++)
   {
      edge_t *aelWalk = edgehead.pnext;
      edge_t *addWalk = *addEdgeList;
      *addEdgeList++ = NULL;
      //backgroundPoly = NULL;

      while(aelWalk != NULL)
      {
         if(aelWalk->lasty < curY)
         {
            edge_t *next = aelWalk->pnext;
            edge_t *prev = aelWalk->pprev;

            next->pprev = prev;
            prev->pnext = next;
            aelWalk = next;
         }
         else
         {
            // step the edge's x
            edge_t *next = aelWalk->pnext;
            aelWalk->x += aelWalk->dx;
            int x = (aelWalk->x + XFixedPointCeilStep) >> XFixedPointShift;
            
            if(aelWalk->pleadsurf)
            {
               aelWalk->pleadsurf->xstart = x;
               aelWalk->pleadsurf->flags &= ~FLAG_ALREADYDRAWN;
            }
            if(aelWalk->ptrailsurf)
               aelWalk->ptrailsurf->xend = x;

            while(aelWalk->x < aelWalk->pprev->x)
            {
               edge_t *tnext = aelWalk->pnext;
               edge_t *tprev = aelWalk->pprev;
               tprev->pnext = tnext;
               tnext->pprev = tprev;
               aelWalk->pprev = tprev->pprev;
               aelWalk->pnext = tprev;
               tprev->pprev = aelWalk;
               aelWalk->pprev->pnext = aelWalk;
            }
            while(addWalk && addWalk->x < aelWalk->x)
            {
               x = (addWalk->x + XFixedPointCeilStep) >> XFixedPointShift;
               if(addWalk->pleadsurf)
               {
                  addWalk->pleadsurf->xstart = x;
               	addWalk->pleadsurf->flags &= ~FLAG_ALREADYDRAWN;
               }
               if(addWalk->ptrailsurf)
                  addWalk->ptrailsurf->xend = x;

               edge_t *nextAdd = addWalk->pnext;
               addWalk->pnext = aelWalk;
               addWalk->pprev = aelWalk->pprev;
               addWalk->pprev->pnext = addWalk;
               aelWalk->pprev = addWalk;
               addWalk = nextAdd;
            }
            aelWalk = next;
         }
      }
      // ok... all edges are in...
      // zip through AEL backwards construct APL
      activePolyList = NULL;
      transPolyList = NULL;

      aelWalk = edgetail.pprev;

      while(aelWalk != &edgehead)
      {
         // NOTE!
         // all right edges must have been preinited to rightX =
         // right X of screen to avoid problems.

         GFXPolyStore *poly = aelWalk->pleadsurf;
         if(poly)
         {
            // only insert poly if it's span len > 0
            if(poly->xend - poly->xstart > 0 && ((poly->flags & FLAG_ALREADYDRAWN) == 0))
            {
               poly->flags |= FLAG_ALREADYDRAWN;

               if(poly->flags & FLAG_TRANSPARENT)
               {
                  poly->nextPoly = transPolyList;
                  transPolyList = poly;
               }
//               else if(poly->flags & FLAG_BACKGROUND_FILL)
//               {
//                  backgroundPoly = poly;
//               }
               else
               {
                  poly->nextPoly = activePolyList;
                  activePolyList = poly;
               }
               poly->curW = float(poly->w0 + curY * poly->dwdy);
            }
         }
         aelWalk = aelWalk->pprev;
      }

      // ok... active poly list is created
      // time to scan it.

      // possible cases...

      // 0. next does not overlap in x extent
      //    emit this->xstart, this->xend, this->surface
      //    this = next
      //
      // 1. next span is totally behind
      //    temp = next->next
      //    resort next, this->xend
      //    next = temp
      //
      // 2. next span is in front, occludes to end
      //    emit this->xstart, next->xstart, this->surface
      //    temp = this
      //    this = next
      //    resort temp, this->xend
      //
      // 3. next span is behind, intersects in visible region
      //    calc intersect point
      //    temp->xstart = intersect point
      //    temp->xend = this->xend
      //    temp->surface = this->surface
      //    this->xend = intersect point
      //    next->xstart = intersect point
      //    resort next, temp
      //
      // 4. next span is in front, intersects in visible region
      //    emit this->xstart, next->xstart, this->surface
      //    temp->xstart = intersect point
      //    temp->xend = next->xend
      //    temp->surface = next->surface
      //    next->xend = intersect point
      //    this->xstart = intersect point
      //    resort temp
      //    temp = this
      //    resort temp
      //    this = next
      //
      // resort xpos (make sure xpos < poly->xend)

      clipIndex = 0;

      GFXPolyStore *curPoly, *nextPoly;
      GFXPolyStore *temp;

      // new transparent faces get added to transProcessList
      // these are then clipped and placed on activeTransList
      // whenever a new curPoly gets set, activeTransList moves
      // to transProcessList.

      curPoly = activePolyList;
      if(!curPoly)
         goto backgroundPolyOnly;

      nextPoly = curPoly->nextPoly;
      for(;;)
      {   
         // first clip any transparent faces on the transparent face
         // list against curPoly, removing those that are done
         if(transPolyList)
         {
            GFXPolyStore *transPoly = transPolyList;
            transPolyList = NULL;

            do
            {
               GFXPolyStore *nextTrans = transPoly->nextPoly;

               if(transPoly->xstart >= curPoly->xend) // no more faces to process until later x
                  break;
               if(transPoly->xend <= curPoly->xstart) // this poly is clear, retire it.
               {
                  emit(transPoly->xstart, transPoly->xend, transPoly->myPoly);
               }
               else
               {
                  // there was some overlap...
                  GFXPolyStore *cp = curPoly->myPoly;
                  GFXPolyStore *tp = transPoly->myPoly;
                  int xstart = max(curPoly->xstart, transPoly->xstart);
                     
                  double cpf = cp->curW + xstart * cp->dwdx;
                  double tpf = tp->curW + xstart * tp->dwdx;
                  
                  double startStatus = cpf - tpf;
                  
                  int xend = min(curPoly->xend, transPoly->xend) - 1;

                  cpf = cp->curW + xend * cp->dwdx;
                  tpf = tp->curW + xend * tp->dwdx;

                  double endStatus = cpf - tpf;

                  if(curPoly->flags & FLAG_ALWAYS_BEHIND)
                     goto transCurPolyBehind;

                  if(startStatus >= 0 && endStatus >= 0)
                  {
                     // curPoly is totally in front
                     emit(transPoly->xstart, curPoly->xstart, transPoly->myPoly);
                     
                     // if there's any left, sort it into the remaining transparent
                     // spans

                     if(curPoly->xend < transPoly->xend)
                     {
                        transPoly->xstart = curPoly->xend;
                        insertReverse(&transPolyList, transPoly);
                     }
                  }
                  else if(startStatus <= 0 && endStatus <= 0)
                  {
                     // transPoly is totally in front:
                     // continue processing
transCurPolyBehind:
                     insertReverse(&transPolyList, transPoly);
                  }
                  else
                  {
                     // polys intersected.
                     int xPoint = int((tp->curW - cp->curW) /
                                      (cp->dwdx - tp->dwdx));

                     if(startStatus > 0)
                     {
                        // curPoly starts in front
                        emit(transPoly->xstart, curPoly->xstart, transPoly->myPoly);
                        transPoly->xstart = xPoint;

                        insertReverse(&transPolyList, transPoly);
                     }
                     else
                     {
                        // transPoly starts in front
                        if(transPoly->xend > curPoly->xend && clipIndex < MAX_CLIP_SPANS)
                        {
                           // allocate a new span
                           temp = (GFXPolyStore *) (clipList + clipIndex);
                           clipIndex++;
                           temp->xstart = curPoly->xend;
                           temp->xend = transPoly->xend;
                           temp->myPoly = transPoly->myPoly;

                           insertReverse(&transPolyList, temp);
                        }
                        transPoly->xend = xPoint;
                        insertReverse(&transPolyList, transPoly);
                     }
                  }
               }
               transPoly = nextTrans;
            }
            while(transPoly);

            // transPolyList is in backwards order
            // and transPoly is in forward order

            // start with transPoly and insert members of transPolyList
            // into transPoly.

            if(!transPoly)
            {
reverseList:
               // just reverse transPolyList
               while(transPolyList)
               {
                  temp = transPolyList->nextPoly;
                  transPolyList->nextPoly = transPoly;
                  transPoly = transPolyList;
                  transPolyList = temp;
               }
            }
            else
            {
               while(transPolyList)
               {
                  if(transPolyList->xstart <= transPoly->xstart)
                     goto reverseList;

                  GFXPolyStore *transTemp = transPoly;
                  while(transTemp->nextPoly && transPolyList->xstart > transTemp->nextPoly->xstart)
                     transTemp = transTemp->nextPoly;
                  temp = transPolyList->nextPoly;
                  transPolyList->nextPoly = transTemp->nextPoly;
                  transTemp->nextPoly = transPolyList;
                  transPolyList = temp;
               }
            }
            transPolyList = transPoly;
         }
skipTransClip:
         if(!nextPoly)
            break;
         if(nextPoly->xstart >= curPoly->xend)
         {
            emit(curPoly->xstart, curPoly->xend, curPoly->myPoly);
            curPoly = nextPoly;
            nextPoly = curPoly->nextPoly;
         }
         else
         {
            GFXPolyStore *cp = curPoly->myPoly;
            GFXPolyStore *np = nextPoly->myPoly;

            double cpf = cp->curW + nextPoly->xstart * cp->dwdx;
            double npf = np->curW + nextPoly->xstart * np->dwdx;

            double startStatus = cpf - npf;

            int xend = min(curPoly->xend, nextPoly->xend) - 1;

            cpf = cp->curW + xend * cp->dwdx;
            npf = np->curW + xend * np->dwdx;

            double endStatus = cpf - npf;
            if(curPoly->flags & FLAG_ALWAYS_BEHIND)
               goto curPolyBehind;
            if(nextPoly->flags & FLAG_ALWAYS_BEHIND)
               goto nextPolyBehind;


            if(startStatus >= 0 && endStatus >= 0)
            {
nextPolyBehind:
               // curPoly is totally in front

               if(curPoly->xend < nextPoly->xend)
               {
                  nextPoly->xstart = curPoly->xend;
                  curPoly->nextPoly = nextPoly->nextPoly;

                  resort(curPoly, nextPoly);
                  nextPoly = curPoly->nextPoly;
               }
               else
                  nextPoly = nextPoly->nextPoly;
               goto skipTransClip;
            }
            else if(startStatus <= 0 && endStatus <= 0)
            {
curPolyBehind:
               // curPoly is totally behind

               emit(curPoly->xstart, nextPoly->xstart, curPoly->myPoly);
               if(curPoly->xend > nextPoly->xend)
               {
                  curPoly->xstart = nextPoly->xend;
                  resort(nextPoly, curPoly);
               }
               curPoly = nextPoly;
               nextPoly = curPoly->nextPoly;
            }
            else
            {
               // they intersected...
               // calc the intersect point
               int xPoint = int((np->curW - cp->curW) /
                                (cp->dwdx - np->dwdx));
               if(startStatus > 0)
               {
                  // curPoly starts in front
                  // if necessary allocate a span for the excess
                  // of curPoly beyond nextPoly

                  if(curPoly->xend > nextPoly->xend)
                  {
                     // allocate the excess poly span info.

                     if(clipIndex >= MAX_CLIP_SPANS)
                     {
                        // no more clip spans left... oh well.
                        // assume that curPoly is in front.
                        // (this code is identical to above case)

                        if(curPoly->xend < nextPoly->xend)
                        {
                           nextPoly->xstart = curPoly->xend;
                           curPoly->nextPoly = nextPoly->nextPoly;

                           resort(curPoly, nextPoly);
                           nextPoly = curPoly->nextPoly;
                        }
                        else
                           nextPoly = nextPoly->nextPoly;
                        continue;
                     }

                     temp = (GFXPolyStore *) (clipList + clipIndex);
                     clipIndex++;

                     temp->xstart = nextPoly->xend;
                     temp->xend = curPoly->xend;
                     temp->myPoly = curPoly->myPoly;
                     resort(nextPoly, temp); // sort it in after next poly
                  }
                  curPoly->xend = xPoint;
                  nextPoly->xstart = xPoint;
                  curPoly->nextPoly = nextPoly->nextPoly;
                  resort(curPoly, nextPoly);
                  nextPoly = curPoly->nextPoly;
                  goto skipTransClip;
               }
               else
               {
                  // nextPoly starts in front
                  
                  if(nextPoly->xend > curPoly->xend)
                  {
                     // allocate a new poly span info struct
                     if(clipIndex >= MAX_CLIP_SPANS)
                     {
                        // curPoly is totally behind
                        // this code is the same as above

                        emit(curPoly->xstart, nextPoly->xstart, curPoly->myPoly);
                        if(curPoly->xend > nextPoly->xend)
                        {
                           curPoly->xstart = nextPoly->xend;
                           resort(nextPoly, curPoly);
                        }
                        curPoly = nextPoly;
                        nextPoly = curPoly->nextPoly;
                        continue;
                     }
                     temp = (GFXPolyStore *) (clipList + clipIndex);
                     clipIndex++;
                     temp->xstart = curPoly->xend;
                     temp->xend = nextPoly->xend;
                     temp->myPoly = nextPoly->myPoly;
                     resort(nextPoly, temp);
                  }
                  emit(curPoly->xstart, nextPoly->xstart, curPoly->myPoly);
                  curPoly->xstart = xPoint;
                  nextPoly->xend = xPoint;
                  temp = curPoly;
                  curPoly = nextPoly;
                  resort(curPoly, temp);
                  nextPoly = curPoly->nextPoly;
               }
            }
         }
      }
backgroundPolyOnly:
      if(curPoly)
         emit(curPoly->xstart, curPoly->xend, curPoly->myPoly);
      while(transPolyList)
      {
         emit(transPolyList->xstart, transPolyList->xend, transPolyList->myPoly);
         transPolyList = transPolyList->nextPoly;
      }
  }
  return nextFreeBlock;
}


#pragma warning( disable : 4101 ) // ignore unreferenced locals


/////////////////////////////////////////////////////////////////////
// Draw all the spans that were scanned out.
/////////////////////////////////////////////////////////////////////


void DrawSpans (GFXSurface *pSurf)
{
	gfxPDC.rowStride = pSurf->getStride();
	gfxPDC.rowAddr = pSurf->getAddress(0,0);

	GFXPolyStore *sf = solidSurfs;
	while(sf < curSolidSurf)
	{
		if(sf->spanList == sf->curblock)
		{
			sf++;
			continue;
		}
		gfxPDC.dudx = sf->dudx;
		gfxPDC.dudy = sf->dudy;
		gfxPDC.u0 = sf->u0;
		gfxPDC.dvdx = sf->dvdx;
		gfxPDC.dvdy = sf->dvdy;
		gfxPDC.v0 = sf->v0;
		gfxPDC.dwdx = sf->dwdx;
		gfxPDC.dwdy = sf->dwdy;
		gfxPDC.w0 = sf->w0;
		gfxPDC.dsdx = sf->dsdx;
		gfxPDC.dsdy = sf->dsdy;
		gfxPDC.s0 = sf->s0;
		gfxPDC.shadeMap = sf->shadeMap;
		gfxPDC.color = sf->color;
		if(sf->tce)
			gfxTextureCache.prepareForDraw(sf->tce, pSurf->getPalette());
		else
		{
			gfxPDC.textureMap = sf->textureMap;
			gfxPDC.textureMapStride = sf->textureMapStride;
			gfxPDC.textureMapHeight = sf->textureMapHeight;
		}

		// always call the renderer after steps have
		// been set up.
		sf->drawFunc();
		GFXMetrics.incRenderedPolys();
		NewRasterizeSpans(sf->spanList, sf->curblock);
		sf++;
	}
	// here we do the transparent surfaces...
	// might in the future want to sort or something

	sf = transparentSurfs;
	while(sf < curTransparentSurf)
	{
		if(sf->spanList == sf->curblock)
		{
			sf++;
			continue;
		}
		gfxPDC.dudx = sf->dudx;
		gfxPDC.dudy = sf->dudy;
		gfxPDC.u0 = sf->u0;
		gfxPDC.dvdx = sf->dvdx;
		gfxPDC.dvdy = sf->dvdy;
		gfxPDC.v0 = sf->v0;
		gfxPDC.dwdx = sf->dwdx;
		gfxPDC.dwdy = sf->dwdy;
		gfxPDC.w0 = sf->w0;
		gfxPDC.dsdx = sf->dsdx;
		gfxPDC.dsdy = sf->dsdy;
		gfxPDC.s0 = sf->s0;
		gfxPDC.shadeMap = sf->shadeMap;
		gfxPDC.color = sf->color;
		if(sf->tce)
			gfxTextureCache.prepareForDraw(sf->tce, pSurf->getPalette());
		else
		{
			gfxPDC.textureMap = sf->textureMap;
			gfxPDC.textureMapStride = sf->textureMapStride;
			gfxPDC.textureMapHeight = sf->textureMapHeight;
		}

		// always call the renderer after steps have
		// been set up.
		sf->drawFunc();
		GFXMetrics.incRenderedPolys();
		NewRasterizeSpans(sf->spanList, sf->curblock);
		sf++;
	}
}

