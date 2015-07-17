#include <stdlib.h>
#include "gdevice.h"
#include "gfxmetrics.h"
#include <windowsx.h>

void 
GFXDevice::setWindow(Point2I *size)
{
   if(!size)
   {
      size = &currentSize;
   }
   RECT rect;

   if ( !restoreWin )
      GetWindowRect( clientWind, &windowRect);

	SetRect(&rect,0,0,size->x, size->y);
	AdjustWindowRectEx(&rect,
		GetWindowStyle(clientWind),
		GetMenu(clientWind) != NULL,
		GetWindowExStyle(clientWind));

	SetWindowPos(clientWind, HWND_TOPMOST, 
	         rect.left,                 // X
	         rect.top,                  // Y
		      (rect.right - rect.left),  // Width
		      (rect.bottom - rect.top),  // Height
		      0 );

   restoreWin = true;
}

int _USERENTRY GFXDevice::compareResolution(const void *_a, const void *_b)
{
   ResSpecifier* a = (ResSpecifier*)_a;   
   ResSpecifier* b = (ResSpecifier*)_b;
   if ( a->res.x != b->res.x ) return ( a->res.x - b->res.x );     // by X resolution
   return ( a->res.y - b->res.y );                                 // by Y resolution
}

void GFXDevice::sortResolutions()
{
   if(resVector.size())
      qsort(resVector.address(), resVector.size(), sizeof(ResSpecifier), compareResolution);
}

void GFXDevice::restoreWindow()
{
   restoreWin = false;

	SetWindowPos(clientWind, HWND_NOTOPMOST, 
	         windowRect.left, 
	         windowRect.top, 
            (windowRect.right-windowRect.left),
            (windowRect.bottom-windowRect.top),
		      SWP_NOSENDCHANGING );
}

bool GFXDevice::nextRes()
{
   if(!isFullscr)
      return(true);
      
   RES_VECTOR::iterator i = resVector.begin();

   // find currentSize in the resolution list
   for ( ; i != resVector.end(); i++)
      if ( (*i) == currentSize )
      {
         if(i == resVector.end() - 1) return(false);
         i++ ; 
         return setResolution( (*i).res );
      }
   return ( false );
}


//------------------------------------------------------------------------------
bool GFXDevice::prevRes()
{
   if(!isFullscr)
      return(true);
      
   RES_VECTOR::iterator i = resVector.begin();

   // find currentSize in the resolution list
   for ( ; i != resVector.end(); i++)
      if ( (*i) == currentSize )
      {
         if ( i == resVector.begin() ) return ( false );
         // locate the first resolution smaller with same bit depth
         i--;
         return setResolution( (*i).res );
      }
   return ( false );
}

Point2I GFXDevice::closestRes(Point2I &res)
{
   Point2I closest(0,0);
   RES_VECTOR::iterator i = resVector.begin();
   for ( ; i != resVector.end(); i++)
   {
      if ( abs(res.x-(*i).res.x) <= abs(res.x-closest.x) &&
              abs(res.y-(*i).res.y) <= abs(res.y-closest.y)   )
            closest = (*i).res;
   }
   if ( !closest.x ) return ( res );
   return ( closest );
}

