//================================================================
//	
// Initial Author: Rick Overman 
//	
// Description 
//	
// $Workfile:   ddrawerr.cpp  $
// $Revision:   1.1  $
// $Author  $
// $Modtime $
//
//================================================================

#include <windows.h>
#include "ddrawerr.h"
#include <winerror.h>

#ifdef DEBUG



struct DIRECT_DRAW_ERROR
{
	HRESULT errorCode;
	char    *errorString;
};

DIRECT_DRAW_ERROR DDRawErrorTable[] =
{
{DDERR_ALREADYINITIALIZED, "DDERR_ALREADYINITIALIZED: This object is already initialized "},
{DDERR_BLTFASTCANTCLIP, "DDERR_BLTFASTCANTCLIP: Return if a clipper object is attached to the source surface passed into a BltFast call."},
{DDERR_CANNOTATTACHSURFACE, "DDERR_CANNOTATTACHSURFACE: This surface can not be attached to the requested surface.	"},
{DDERR_CANNOTDETACHSURFACE, "DDERR_CANNOTDETACHSURFACE: This surface can not be detached from the requested surface.	"},
{DDERR_CANTCREATEDC, "DDERR_CANTCREATEDC: Windows can not create any more DCs"},
{DDERR_CANTDUPLICATE, "DDERR_CANTDUPLICATE: Can't duplicate primary & 3D surfaces, or surfaces that are implicitly created."},
{DDERR_CLIPPERISUSINGHWND, "DDERR_CLIPPERISUSINGHWND: An attempt was made to set a cliplist for a clipper object that is already monitoring an hwnd."},
{DDERR_COLORKEYNOTSET, "DDERR_COLORKEYNOTSET: No src color key specified for this operation."},
{DDERR_CURRENTLYNOTAVAIL, "DDERR_CURRENTLYNOTAVAIL: Support is currently not available.	"},
{DDERR_DIRECTDRAWALREADYCREATED, "DDERR_DIRECTDRAWALREADYCREATED: A DirectDraw object representing this driver has already been created for this process.	"},
{DDERR_EXCEPTION, "DDERR_EXCEPTION: An exception was encountered while performing the requested operation.	"},
{DDERR_EXCLUSIVEMODEALREADYSET, "DDERR_EXCLUSIVEMODEALREADYSET: An attempt was made to set the cooperative level when it was already set to exclusive."},
{DDERR_GENERIC, "DDERR_GENERIC: Generic failure."},
{DDERR_HEIGHTALIGN, "DDERR_HEIGHTALIGN: Height of rectangle provided is not a multiple of reqd alignment."},
{DDERR_HWNDALREADYSET, "DDERR_HWNDALREADYSET: The CooperativeLevel HWND has already been set. It can not be reset while the process has surfaces or palettes created."},
{DDERR_HWNDSUBCLASSED, "DDERR_HWNDSUBCLASSED: HWND used by DirectDraw CooperativeLevel has been subclassed, this prevents DirectDraw from restoring state."},
{DDERR_IMPLICITLYCREATED, "DDERR_IMPLICITLYCREATED: This surface can not be restored because it is an implicitly created surface."},
{DDERR_INCOMPATIBLEPRIMARY, "DDERR_INCOMPATIBLEPRIMARY: Unable to match primary surface creation request with existing primary surface."},
{DDERR_INVALIDCAPS, "DDERR_INVALIDCAPS: One or more of the caps bits passed to the callback are incorrect."},
{DDERR_INVALIDCLIPLIST, "DDERR_INVALIDCLIPLIST: DirectDraw does not support the provided cliplist."},
//{DDERR_INVALIDDIRECTDRAWUIID, "DDERR_INVALIDDIRECTDRAWUIID: The UUID passed to DirectDrawCreate is not a valid DirectDraw driver identifier."},
{DDERR_INVALIDMODE, "DDERR_INVALIDMODE: DirectDraw does not support the requested mode."},
{DDERR_INVALIDOBJECT, "DDERR_INVALIDOBJECT: DirectDraw received a pointer that was an invalid DIRECTDRAW object."},
{DDERR_INVALIDPARAMS, "DDERR_INVALIDPARAMS: One or more of the parameters passed to the function are incorrect."},
{DDERR_INVALIDPIXELFORMAT, "DDERR_INVALIDPIXELFORMAT: The pixel format was invalid as specified."},
{DDERR_INVALIDPOSITION, "DDERR_INVALIDPOSITION: Returned when the position of the overlay on the destionation is no longer legal for that destination."},
{DDERR_INVALIDRECT, "DDERR_INVALIDRECT: Rectangle provided was invalid."},
{DDERR_LOCKEDSURFACES, "DDERR_LOCKEDSURFACES: Operation could not be carried out because one or more surfaces are locked."},
{DDERR_NO3D, "DDERR_NO3D: There is no 3D present."},
{DDERR_NOALPHAHW, "DDERR_NOALPHAHW: Operation could not be carried out because there is no alpha accleration hardware present or available."},
//{DDERR_NOANTITEARHW, "DDERR_NOANTITEARHW: Operation could not be carried out because there is no hardware support for synchronizing blts to avoid tearing.	"},
{DDERR_NOBLTHW, "DDERR_NOBLTHW: No blter hardware present."},
//{DDERR_NOBLTQUEUEHW, "DDERR_NOBLTQUEUEHW: Operation could not be carried out because there is no hardware support for asynchronous blting.	"},
{DDERR_NOCLIPLIST, "DDERR_NOCLIPLIST: No cliplist available."},
{DDERR_NOCLIPPERATTACHED, "DDERR_NOCLIPPERATTACHED: No clipper object attached to surface object."},
{DDERR_NOCOLORCONVHW, "DDERR_NOCOLORCONVHW: Operation could not be carried out because there is no color conversion hardware present or available."},
{DDERR_NOCOLORKEY, "DDERR_NOCOLORKEY: Surface doesn't currently have a color key"},
{DDERR_NOCOLORKEYHW, "DDERR_NOCOLORKEYHW: Operation could not be carried out because there is no hardware support of the destination color key."},
{DDERR_NOCOOPERATIVELEVELSET, "DDERR_NOCOOPERATIVELEVELSET: Create function called without DirectDraw object method SetCooperativeLevel being called."},
{DDERR_NODC, "DDERR_NODC: No DC was ever created for this surface."},
{DDERR_NODDROPSHW, "DDERR_NODDROPSHW: No DirectDraw ROP hardware."},
{DDERR_NODIRECTDRAWHW, "DDERR_NODIRECTDRAWHW: A hardware-only DirectDraw object creation was attempted but the driver did not support any hardware."},
{DDERR_NOEMULATION, "DDERR_NOEMULATION: Software emulation not available."},
{DDERR_NOEXCLUSIVEMODE, "DDERR_NOEXCLUSIVEMODE: Operation requires the application to have exclusive mode but the application does not have exclusive mode."},
{DDERR_NOFLIPHW, "DDERR_NOFLIPHW: Flipping visible surfaces is not supported."},
{DDERR_NOGDI, "DDERR_NOGDI: There is no GDI present."},
{DDERR_NOHWND, "DDERR_NOHWND: Clipper notification requires an HWND or no HWND has previously been set as the CooperativeLevel HWND."},
{DDERR_NOMIRRORHW, "DDERR_NOMIRRORHW: Operation could not be carried out because there is no hardware present or available."},
{DDERR_NOOVERLAYDEST, "DDERR_NOOVERLAYDEST: Returned when GetOverlayPosition is called on an overlay that UpdateOverlay has never been called on to establish a destination."},
{DDERR_NOOVERLAYHW, "DDERR_NOOVERLAYHW: Operation could not be carried out because there is no overlay hardware present or available."},
{DDERR_NOPALETTEATTACHED, "DDERR_NOPALETTEATTACHED: No palette object attached to this surface.	"},
{DDERR_NOPALETTEHW, "DDERR_NOPALETTEHW: No hardware support for 16 or 256 color palettes."},
{DDERR_NORASTEROPHW, "DDERR_NORASTEROPHW: Operation could not be carried out because there is no appropriate raster op hardware present or available."},
{DDERR_NOROTATIONHW, "DDERR_NOROTATIONHW: Operation could not be carried out because there is no rotation hardware present or available."},
{DDERR_NOSTRETCHHW, "DDERR_NOSTRETCHHW: Operation could not be carried out because there is no hardware support for stretching."},
{DDERR_NOT4BITCOLOR, "DDERR_NOT4BITCOLOR: DirectDrawSurface is not in 4 bit color palette and the requested operation requires 4 bit color palette."},
{DDERR_NOT4BITCOLORINDEX, "DDERR_NOT4BITCOLORINDEX: DirectDrawSurface is not in 4 bit color index palette and the requested operation requires 4 bit color index palette."},
{DDERR_NOT8BITCOLOR, "DDERR_NOT8BITCOLOR: DirectDrawSurface is not in 8 bit color mode and the requested operation requires 8 bit color."},
{DDERR_NOTAOVERLAYSURFACE, "DDERR_NOTAOVERLAYSURFACE: Returned when an overlay member is called for a non-overlay surface."},
{DDERR_NOTEXTUREHW, "DDERR_NOTEXTUREHW: Operation could not be carried out because there is no texture mapping hardware present or available."},
{DDERR_NOTFLIPPABLE, "DDERR_NOTFLIPPABLE: An attempt has been made to flip a surface that is not flippable."},
{DDERR_NOTFOUND, "DDERR_NOTFOUND: Requested item was not found."},
{DDERR_NOTLOCKED, "DDERR_NOTLOCKED: Surface was not locked.  An attempt to unlock a surface that was not locked at all, or by this process, has been attempted."},
{DDERR_NOVSYNCHW, "DDERR_NOVSYNCHW: Operation could not be carried out because there is no hardware support for vertical blank synchronized operations."},
{DDERR_NOZBUFFERHW, "DDERR_NOZBUFFERHW: Operation could not be carried out because there is no hardware support for zbuffer blting."},
{DDERR_NOZOVERLAYHW, "DDERR_NOZOVERLAYHW: Overlay surfaces could not be z layered based on their BltOrder because the hardware does not support z layering of overlays."},
{DDERR_OUTOFCAPS, "DDERR_OUTOFCAPS: The hardware needed for the requested operation has already been allocated."},
{DDERR_OUTOFMEMORY, "DDERR_OUTOFMEMORY: DirectDraw does not have enough memory to perform the operation."},
{DDERR_OUTOFVIDEOMEMORY, "DDERR_OUTOFVIDEOMEMORY: DirectDraw does not have enough memory to perform the operation."},
{DDERR_OVERLAYCANTCLIP, "DDERR_OVERLAYCANTCLIP: The hardware does not support clipped overlays."},
{DDERR_OVERLAYCOLORKEYONLYONEACTIVE, "DDERR_OVERLAYCOLORKEYONLYONEACTIVE: Can only have ony color key active at one time for overlays."},
{DDERR_OVERLAYNOTVISIBLE, "DDERR_OVERLAYNOTVISIBLE: Returned when GetOverlayPosition is called on a hidden overlay."},
{DDERR_PALETTEBUSY, "DDERR_PALETTEBUSY: Access to this palette is being refused because the palette is already locked by another thread."},
{DDERR_PRIMARYSURFACEALREADYEXISTS, "DDERR_PRIMARYSURFACEALREADYEXISTS: This process already has created a primary surface."},
{DDERR_REGIONTOOSMALL, "DDERR_REGIONTOOSMALL: Region passed to Clipper::GetClipList is too small."},
{DDERR_SURFACEALREADYATTACHED, "DDERR_SURFACEALREADYATTACHED: This surface is already attached to the surface it is being attached to."},
{DDERR_SURFACEALREADYDEPENDENT, "DDERR_SURFACEALREADYDEPENDENT: This surface is already a dependency of the surface it is being made a dependency of."},
{DDERR_SURFACEBUSY, "DDERR_SURFACEBUSY: Access to this surface is being refused because the surface is already locked by another thread."},
{DDERR_SURFACEISOBSCURED, "DDERR_SURFACEISOBSCURED: Access to surface refused because the surface is obscured."},
{DDERR_SURFACELOST, "DDERR_SURFACELOST: Access to this surface is being refused because the surface memory is gone. The DirectDrawSurface object representing this surface should have Restore called on it."},
{DDERR_SURFACENOTATTACHED, "DDERR_SURFACENOTATTACHED: The requested surface is not attached."},
{DDERR_TOOBIGHEIGHT, "DDERR_TOOBIGHEIGHT: Height requested by DirectDraw is too large."},
{DDERR_TOOBIGSIZE, "DDERR_TOOBIGSIZE: Size requested by DirectDraw is too large --	the individual height and width are OK."},
{DDERR_TOOBIGWIDTH, "DDERR_TOOBIGWIDTH: Width requested by DirectDraw is too large."},
{DDERR_UNSUPPORTED, "DDERR_UNSUPPORTED: Action not supported."},
{DDERR_UNSUPPORTEDFORMAT, "DDERR_UNSUPPORTEDFORMAT: FOURCC format requested is unsupported by DirectDraw."},
{DDERR_UNSUPPORTEDMASK, "DDERR_UNSUPPORTEDMASK: Bitmask in the pixel format requested is unsupported by DirectDraw."},
{DDERR_VERTICALBLANKINPROGRESS, "DDERR_VERTICALBLANKINPROGRESS: Vertical blank is in progress."},
{DDERR_WASSTILLDRAWING, "DDERR_WASSTILLDRAWING: Informs DirectDraw that the previous Blt which is transfering information to or from this Surface is incomplete."},
{DDERR_WRONGMODE, "DDERR_WRONGMODE: This surface can not be restored because it was created in a different mode."},
{DDERR_XALIGN, "DDERR_XALIGN: Rectangle provided was not horizontally aligned on required boundary."},
{DD_OK, NULL},		//terminate with OK
};


const char* DDRAW_ERROR(HRESULT error)
{
	for (int i=0; DDRawErrorTable[i].errorCode != DD_OK; i++)
	{
		if (DDRawErrorTable[i].errorCode == error) 
			return ( DDRawErrorTable[i].errorString );
	}
	return ( "Unknown DirectDraw Error" );
}

#endif