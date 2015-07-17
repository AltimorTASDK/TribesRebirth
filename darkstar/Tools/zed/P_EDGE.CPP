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
void ScanEdges (void);
void AddPolygonEdges ( GFXPolyStore *screenpoly);


//-------------------------------------- Local vars
int             numPolys = 0;
span_t          spans[MAX_SPANS];
GFXPolyStore    surfs[MAX_POLYS];
GFXPolyStore   *pavailsurf;
GFXPolyStore   *curSolidSurf;
GFXPolyStore   *curTransparentSurf;

int xl, xr, yt, yb;
int height;
int currentcolor;
int numPoints, numTexPoints; // dummy vars for copying...

//------------------------------------------------------------------------------
//--------------------------------------
void COREAPI
GFXDraw3DBegin(GFXSurface *io_pSurface)
{
	numPolys = 0;
	RectI *clipRect = io_pSurface->getClipRect();
	xl = clipRect->upperL.x;
	xr = clipRect->lowerR.x;
	yt = clipRect->upperL.y;
	yb = clipRect->lowerR.y;
   height = clipRect->lowerR.y + 1;
   pavailedge = edges; // replace ClearEdgeLists()
	currentcolor = 0;
	GFXMetrics.reset();
	curSolidSurf = surfs;
	curTransparentSurf = surfs + MAX_POLYS - 1;
   int i;
   for(i =0; i < height; i++)
      newedges[i] = 0;

#ifdef DEBUG
   GFXMetrics.textureSpaceUsed       = 0;
   GFXMetrics.textureBytesDownloaded = 0;
   gfxTextureCache.frameU++;
#endif
}

void COREAPI
GFXSort3dEnd(GFXSurface *io_pSurface)
{
   // Set up the active edge list as initially empty, containing
   // only the sentinels (which are also the background fill). Most
   // of these fields could be set up just once at start-up
   edgehead.pnext = &edgetail;
   edgehead.pprev = NULL;
   edgehead.x = 0;           // left edge of screen
   edgehead.leading = 0;
   edgehead.pleadsurf = &surfstack;
	edgehead.ptrailsurf = 0;

   edgetail.pnext = NULL;          // mark edge of list
   edgetail.pprev = &edgehead;
   edgetail.x = xr + 1;    // right edge of screen
   edgetail.leading = 0;
   edgetail.ptrailsurf = &surfstack;
	edgetail.pleadsurf = 0;

	maxedge.x = 0x7FFFFFFF;

   // The background surface is the entire stack initially, and
   // is infinitely far away, so everything sorts in front of it.
   // This could be set just once at start-up
   surfstack.pnext = surfstack.pprev = &surfstack;
   surfstack.w0 = -999999.0;
   surfstack.dwdx = surfstack.dwdy = 0.0;
	surfstack.spanList = NULL;
	spanListPtr = spans;

	GFXMetrics.numSpansUsed = gfx_scan_edges(height);
	DrawSpans(io_pSurface);
}

void COREAPI GFXSortDrawPoint3d(GFXSurface *io_pSurface, const Point2I *in_at, float in_w, DWORD in_color)
{
	io_pSurface;
	if(curSolidSurf < curTransparentSurf && pavailedge < &(edges[MAX_EDGES - 2]))
	{
		edge_t *e1 = pavailedge, *e2 = pavailedge + 1;
		pavailedge += 2;

		curSolidSurf->drawFunc = (rendererManager::renderSpanFunc) GFXRenderSpan;
		curSolidSurf->color = in_color;
		curSolidSurf->dwdx = 0;
		curSolidSurf->dwdy = 0;
		curSolidSurf->w0 = in_w;
		curSolidSurf->tce = NULL;
		curSolidSurf->spanList = NULL;
   		curSolidSurf->state = 0;
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
}

void COREAPI GFXSortDrawRect3d(GFXSurface *io_pSurface, const RectI *in_rect, float in_w, DWORD in_color)
{
	io_pSurface;
	if(curSolidSurf < curTransparentSurf && pavailedge < &(edges[MAX_EDGES - 2]))
	{
		edge_t *e1 = pavailedge, *e2 = pavailedge + 1;
		pavailedge += 2;

		curSolidSurf->drawFunc = (rendererManager::renderSpanFunc) GFXRenderSpan;
		curSolidSurf->color = in_color;
		curSolidSurf->dwdx = 0;
		curSolidSurf->dwdy = 0;
		curSolidSurf->w0 = in_w;
		curSolidSurf->tce = NULL;
		curSolidSurf->spanList = NULL;
   		curSolidSurf->state = 0;
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
}

void COREAPI GFXSortEmitPoly(GFXSurface* /*io_pSurface*/)
{
	GFXMetrics.incEmittedPolys();
	if(curSolidSurf < curTransparentSurf && pavailedge < &(edges[MAX_EDGES - vertexCount]))
	{
		GFXPolyStore *polySurf;
		if(g_renderTable.isTransparent())
		{
			polySurf = curTransparentSurf;
			polySurf->flags = FLAG_TRANSPARENT;
		}
		else
		{
			polySurf = curSolidSurf;
			polySurf->flags = 0;
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
		   polySurf->drawFunc = g_renderTable.getRenderer();
			
			//AddPolygonEdges(surfs + numPolys);
			gfx_add_edges(polySurf); 
			polySurf->spanList = NULL;
    		polySurf->state = 0;
			if(polySurf->flags)
				curTransparentSurf--;
			else
				curSolidSurf++;
		}
		else
		{
			GFXMetrics.incBackfacedPolys();
		}
	}
	vertexCount = 0;
}

#pragma warning( disable : 4101 ) // ignore unreferenced locals

#if 0

void AddPolygonEdges ( GFXPolyStore *screenpoly)
{
    double  distinv, deltax, deltay, slope;
    int     i, nextvert, numverts, temp, topy, bottomy, height;
    edge_t  **pedge;

    // Add each edge in turn
	i = vertexCount - 1;
	int yi = (int)ceil(vList[i]->y);
	int ynv;
    for (nextvert = 0 ; nextvert <vertexCount ; yi = ynv, i = nextvert++)
    {
        topy = yi;
		DWORD key = kList[i];
		ynv = bottomy = (int)ceil(vList[nextvert]->y);
		const Point4F *top = vList[i];
        if(topy == bottomy)
			continue;

        if ((pavailedge->leading = (bottomy < topy)))
        {
            // Leading edge
            topy = ynv;
            bottomy = yi;
			top = vList[nextvert];
            // Edge coordinates are in 16.16 fixed point
        }
#ifdef DEBUG
		height = bottomy - topy;
		if(pavailedge->leading)
			GFXMetrics.numPixelsRendered += height * (xr - (vList[i]->x + vList[nextvert]->x)/2);
		else
			GFXMetrics.numPixelsRendered -= height * (xr - (vList[i]->x + vList[nextvert]->x)/2);

#endif
        pavailedge->psurf = screenpoly;
		pavailedge->leading |= key;
		// check for shared edge insertion now...
		edge_t *e = newedges[topy];
		int leadingtest = pavailedge->leading ^ 1;
		if(key)
		{
			while(e)
			{
				if(e->leading == leadingtest && !e->link)
				{
					if(e->leading & 1)
					{
						// pavailedge is a trailing edge...
						// this means we have to make e a trailing
						// edge too, link it to it's surface then
						// make it's surface our surface.
						e->link = e->psurf;
						e->leading = 0;
						e->psurf = pavailedge->psurf;
						
					}
					else
					{
						e->link = pavailedge->psurf;
					}
					GFXMetrics.incSharedEdges();
					goto nextEdge;  // go on to the next edge... reuse space for this edge
				}
				e = e->pnext;
			}
		}

		slope = (vList[i]->x - vList[nextvert]->x) / (vList[i]->y - vList[nextvert]->y);
		pavailedge->dx = (int)(slope * (float)0x10000);
		pavailedge->x = int((vList[i]->x + (topy - vList[i]->y) * slope) * 65536.0f);

//		pavailedge->x = (int)((top->x +	((float)topy - top->y) *
//                slope) * (float)0x10000);

        // Associate the edge with the surface we'll create for
        // this polygon

        // Put the edge on the list to be added on top scan
        pedge = &newedges[topy];
        while (*pedge && (*pedge)->x < pavailedge->x)
            pedge = &((*pedge)->pnext);

        // Put the edge on the list to be removed after final scan
        pavailedge->pnextremove = removeedges[bottomy - 1];
        removeedges[bottomy - 1] = pavailedge;

		pavailedge->link = 0;
        pavailedge->pnext = *pedge;
		*pedge = pavailedge;
		pavailedge++;
		GFXMetrics.incUsedEdges();
nextEdge:
		;  // Microsoft doesn't seem to like this label right before the end
		   // of a block.  Compiler bug?
    }
}

int testval;

/////////////////////////////////////////////////////////////////////
// Scan all the edges in the global edge table into spans.
/////////////////////////////////////////////////////////////////////

void ScanEdges (void)
{
    int     x, y;
    double  fx, fy, zinv, zinv2;
    edge_t  *pedge, *pedge2, *ptemp;
    span_t  *pspan;
    GFXPolyStore  *psurf, *psurf2;

    pspan = spans;

	/*while(pedge->x > pedge2->x)
		pedge2 = pedge2->next

	ptemp = pedge->pnext
	pedge->pnext = pedge2
	pedge->pprev = pedge2->pprev
	pedge2->pprev->next = pedge
	pedge2->pprev = pedge
	pedge = ptemp*/

    for (y=0 ; y<height ; y++)
    {
        fy = (double)y;

		if(y == 270)
			testval = 1;
        // Sort in any edges that start on this scan
        pedge = newedges[y];
        pedge2 = &edgehead;
        while (pedge)
        {
            while (pedge->x > pedge2->pnext->x)
                pedge2 = pedge2->pnext;

            ptemp = pedge->pnext;
            pedge->pnext = pedge2->pnext;
            pedge->pprev = pedge2;
            pedge2->pnext->pprev = pedge;
            pedge2->pnext = pedge;

            pedge2 = pedge;
            pedge = ptemp;
        }

        // Scan out the active edges into spans

        // Start out with the left background edge already inserted,
        // and the surface stack containing only the background
        surfstack.state = 1;
        surfstack.visxstart = 0;

        for (pedge=edgehead.pnext ; pedge ; pedge=pedge->pnext)
        {
            psurf = pedge->psurf;

            if (pedge->leading)
            {
                // It's a leading edge. Figure out where it is
                // relative to the current surfaces and insert in
                // the surface stack; if it's on top, emit the span
                // for the current top.
                // First, make sure the edges don't cross
                if (++psurf->state == 1)
                {
					// we really wanna know what's going to happen
					// a little ways in the future... should use X + 1 or so.
					// try it the old way for now.

                    fx = (double)(pedge->x + 0x0800) * (1.0 / (double)0x10000);
                    // Calculate the surface's 1/z value at this pixel
                    zinv = psurf->w0 + psurf->dwdx * fx +
                            psurf->dwdy * fy;

                    // See if that makes it a new top surface
                    psurf2 = surfstack.pnext;
                    zinv2 = psurf2->w0 + psurf2->dwdx * fx +
                            psurf2->dwdy * fy;
                    if (zinv >= zinv2)
                    {
                        // It's a new top surface
                        // emit the span for the current top
                        x = (pedge->x + 0xFFFF) >> 16;
                        pspan->count = x - psurf2->visxstart;
                        if (pspan->count > 0)
                        {
                            pspan->y = y;
                            pspan->x = psurf2->visxstart;
							pspan->next = psurf2->spanList;
							psurf2->spanList = pspan;

                            pspan++;

                            // Make sure we don't overflow
                            // the span array
                            if (pspan >= &spans[MAX_SPANS])
                                return;
                        }

                        psurf->visxstart = x;

                        // Add the edge to the stack
                        psurf->pnext = psurf2;
                        psurf2->pprev = psurf;
                        surfstack.pnext = psurf;
                        psurf->pprev = &surfstack;
                    }
                    else
                    {
                        // Not a new top; sort into the surface stack.
                        // Guaranteed to terminate due to sentinel
                        // background surface
                        do
                        {
                            psurf2 = psurf2->pnext;
                            zinv2 = psurf2->w0 +
                                    psurf2->dwdx * fx +
                                    psurf2->dwdy * fy;
                        } while (zinv < zinv2);

                        // Insert the surface into the stack
                        psurf->pnext = psurf2;
                        psurf->pprev = psurf2->pprev;
                        psurf2->pprev->pnext = psurf;
                        psurf2->pprev = psurf;
                    }
                }
            }
            else
            {
                // It's a trailing edge; if this was the top surface,
                // emit the span and remove it.
                // First, make sure the edges didn't cross
                if (--psurf->state == 0)
                {
                    if (surfstack.pnext == psurf)
                    {
                        // It's on top, emit the span
                        x = ((pedge->x + 0xFFFF) >> 16);
                        pspan->count = x - psurf->visxstart;
                        if (pspan->count > 0)
                        {
                            pspan->y = y;
                            pspan->x = psurf->visxstart;
                            pspan->next = psurf->spanList;
							psurf->spanList = pspan;

                            pspan++;

                            // Make sure we don't overflow
                            // the span array
                            if (pspan >= &spans[MAX_SPANS])
                               return;
                        }
						// check if the edge is shared
						if(pedge->link)
							pedge->link->visxstart = x;
						else
                        	psurf->pnext->visxstart = x;
                    }

                    // Remove the surface from the stack
					if(pedge->link)
					{
						psurf->pnext->pprev = pedge->link;
						psurf->pprev->pnext = pedge->link;
						pedge->link->pnext = psurf->pnext;
						pedge->link->pprev = psurf->pprev;
					}
					else
					{
	                    psurf->pnext->pprev = psurf->pprev;
	                    psurf->pprev->pnext = psurf->pnext;
					}
                }
            }
        }

        // Remove edges that are done
        pedge = removeedges[y];
        while (pedge)
        {
            pedge->pprev->pnext = pedge->pnext;
            pedge->pnext->pprev = pedge->pprev;
            pedge = pedge->pnextremove;
        }

        // Step the remaining edges one scan line, and re-sort
        for (pedge=edgehead.pnext ; pedge != &edgetail ; )
        {
            ptemp = pedge->pnext;

            // Step the edge
            pedge->x += pedge->dx;

            // Move the edge back to the proper sorted location,
            // if necessary
            while (pedge->x < pedge->pprev->x)
            {
                pedge2 = pedge->pprev;
                pedge2->pnext = pedge->pnext;
                pedge->pnext->pprev = pedge2;
                pedge2->pprev->pnext = pedge;
                pedge->pprev = pedge2->pprev;
                pedge->pnext = pedge2;
                pedge2->pprev = pedge;
            }

            pedge = ptemp;
        }
    }
}
#endif

/////////////////////////////////////////////////////////////////////
// Draw all the spans that were scanned out.
/////////////////////////////////////////////////////////////////////


void DrawSpans (GFXSurface *pSurf)
{
	gfxPDC.rowStride = pSurf->getStride();
	gfxPDC.rowAddr = pSurf->getAddress(0,0);

	GFXPolyStore *sf = surfs;
	while(sf < curSolidSurf)
	{
		if(!sf->spanList)
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

	sf = surfs + MAX_POLYS - 1;
	while(sf > curTransparentSurf)
	{
		if(!sf->spanList)
		{
			sf--;
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
		sf--;
	}
}

