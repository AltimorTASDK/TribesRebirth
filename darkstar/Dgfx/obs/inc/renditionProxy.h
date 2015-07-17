//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _RENDITIONPROXY_H_
#define _RENDITIONPROXY_H_

//Includes
#include <base.h>
#include "redline.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

namespace Rendition {

struct RenditionTable;

class RenditionProxy {
  private:
   RenditionProxy();
   
   RenditionTable* m_pTable;

  public:
   static RenditionProxy* create(const HINSTANCE in_VeriteDLL,
                                         const HINSTANCE in_RedlineDLL);
                                         
   // Redline Functions...
   //
   vl_error VL_RegisterErrorHandler(vl_error_handler);
   vl_error VL_GetErrorText(const vl_error, LPSTR, const int);
   vl_error VL_GetFunctionName(vl_routine, LPSTR, int);
   vl_error VL_OpenVerite(HWND, v_handle*);
   vl_error VL_CloseVerite(v_handle);
   vl_error VL_CreateSurface(v_handle, v_surface**, const v_u32,
                                     const v_u32, const v_u32,
                                     const v_u32, const v_u32);
   vl_error VL_DestroySurface(v_handle, v_surface*);
   vl_error VL_RestoreSurface(v_handle, v_surface*);
   vl_error VL_FillBuffer(v_cmdbuffer*, v_surface*, v_u32, v_u32, v_u32, v_u32, v_u32, v_u32);
   vl_error VL_InstallDstBuffer(v_cmdbuffer*, v_surface*);
   vl_error VL_SetDstBase(v_cmdbuffer*, v_u32);
   vl_error VL_SetDisplayBase(v_cmdbuffer*, v_foreignAddress);
   vl_error VL_SwapDisplaySurface(v_cmdbuffer*, v_surface*);
   vl_error VL_SetDitherEnable(v_cmdbuffer*, v_u32);
   vl_error VL_SetBlendEnable(v_cmdbuffer*, v_u32);
   vl_error VL_SetBlendDstFunc(v_cmdbuffer*, v_u32);
   vl_error VL_SetBlendSrcFunc(v_cmdbuffer*, v_u32);
   vl_error VL_SetSrcFilter(v_cmdbuffer*, v_u32);
   vl_error VL_SetSrcFunc(v_cmdbuffer*, v_u32);
   vl_error VL_SetSrcFmt(v_cmdbuffer*, v_u32);
   vl_error VL_SetDstFmt(v_cmdbuffer*, v_u32);
   vl_error VL_SetAlphaThreshold(v_cmdbuffer*, v_u32);
   vl_error VL_SetChromaColor(v_cmdbuffer*, v_u32, v_u32);
   vl_error VL_SetChromaMask(v_cmdbuffer*, v_u32, v_u32);
   vl_error VL_SetChromaKey(v_cmdbuffer*, v_u32);
   vl_error VL_SetSurfaceChromaColor(v_surface*, v_u32);
   vl_error VL_SetSurfaceChromaMask(v_surface*, v_u32);
   vl_error VL_SetSurfaceChromaKey(v_surface*, v_u32);
   vl_error VL_SetTranspReject(v_cmdbuffer*, v_u32);
   vl_error VL_SetFogColorRGB(v_cmdbuffer*, v_u32);
   vl_error VL_SetFogEnable(v_cmdbuffer*, v_u32);
   vl_error VL_SetF(v_cmdbuffer*, v_u32);
   vl_error VL_SetFGColorARGB(v_cmdbuffer*, v_u32);
   vl_error VL_Dot(v_cmdbuffer*, v_u32, v_u32*);
   vl_error VL_SetPalette(v_cmdbuffer*, v_u16, v_u16, v_u32*);
   vl_error VL_SetTexturePalette(v_cmdbuffer*, v_u32, v_u32,
                                         v_u32*, v_u32);
   vl_error VL_InstallTextureMapBasic(v_cmdbuffer*, v_surface*);   
   vl_error VL_InstallTextureMap(v_cmdbuffer*, v_surface*);   
   vl_error VL_SetCurrentTexture(v_cmdbuffer*, v_u32, v_u32,
                                         v_u16, v_u16, v_u32, v_u32);
   vl_error VL_SetUClamp(v_cmdbuffer*, v_u32);
   vl_error VL_SetUMask(v_cmdbuffer*, v_u32);
   vl_error VL_SetUMultiplier(v_cmdbuffer*, v_u32);
   vl_error VL_SetVClamp(v_cmdbuffer*, v_u32);
   vl_error VL_SetVMask(v_cmdbuffer*, v_u32);
   vl_error VL_SetVMultiplier(v_cmdbuffer*, v_u32);
   vl_error VL_SetScissorX(v_cmdbuffer*, v_u32);
   vl_error VL_SetScissorY(v_cmdbuffer*, v_u32);
   vl_error VL_SetTOffset(v_cmdbuffer*, v_u32);
   vl_error VL_SetSOffset(v_cmdbuffer*, v_u32);
   vl_error VL_Bitblt(v_cmdbuffer*, v_u16, v_u16, v_u16, v_u16, v_u16, v_u16);
   vl_error VL_BitfillNoPat(v_cmdbuffer*, v_surface*, v_u32, v_u32,
                                    v_u16, v_u16, v_u16, v_u16);
   vl_error VL_Line(v_cmdbuffer*, v_u32, v_u32*, v_u32*);
   vl_error VL_Rectangle(v_cmdbuffer*, v_u32, v_u32, v_u32, v_u32*);
   vl_error VL_MemWriteRect(v_cmdbuffer*, v_foreignAddress, v_u32,
                                    v_memory, v_u32*, v_u32, v_u32, v_u32,
                                    v_u32);
   vl_error VL_LoadBuffer(v_cmdbuffer*, v_surface*, v_u32, v_u32, v_u32,
                                  v_u32, v_memory, v_u32*);
   vl_error VL_Lookup(v_cmdbuffer*, v_u16, v_u16, v_u16, v_u16, v_memory, v_u32*);
   vl_error VL_Trifan(v_cmdbuffer*, v_u32, v_u32, v_u32*);
   vl_error VL_Triangle(v_cmdbuffer*, v_u32, v_u32*, v_u32*, v_u32*);
   vl_error VL_WaitForDisplaySwitch(v_cmdbuffer*);
   vl_error VL_MemFillRect(v_cmdbuffer*, v_foreignAddress, v_u32, v_u32, v_u32, v_u32, v_u32);

   // Verite Functions...
   //
   v_error V_SetDisplayType(v_handle, const v_u32);
   v_error V_SetDisplayMode(v_handle, const v_u32, const v_u32,
                                    const v_u32, const v_u32);
   v_cmdbuffer V_CreateCmdBuffer(v_handle, const int, const int);
   v_error     V_DestroyCmdBuffer(v_cmdbuffer);
   v_error  V_GetErrorText(v_error, LPSTR, int);
   v_u32*   V_AddToCmdList(v_cmdbuffer*, v_u32);
   v_u32*   V_AddToDMAList(v_cmdbuffer*, v_u32, v_memory, v_u32*, v_u32);
   v_error  V_IssueCmdBuffer(v_handle, v_cmdbuffer);
   v_error  V_IssueCmdBufferAsync(v_handle, v_cmdbuffer);
   v_error  V_GetCmdBufferFreeSpace(v_cmdbuffer, v_u32*, v_u32*);
   v_u32    V_QueryCmdBuffer(v_handle, v_cmdbuffer);
   void     V_SetCmdBufferCallBack(v_cmdbuffer, v_cmdbuffer_callback);
   v_memory V_AllocLockedMem(v_handle, v_u32);
   v_error  V_FreeLockedMem(v_handle, v_memory);
   v_memory V_AllocMemoryObject(v_handle, v_u32*, v_u32);
   v_error  V_FreeMemoryObject(v_handle, v_memory);
   v_error  V_SetMemoryObject(v_memory, v_u32*, v_u32);
   v_u32    V_GetMemoryObjectStatus(v_memory);
   v_error  V_LockPages(v_handle, v_memory);
   v_error  V_UnlockPages(v_handle, v_memory);
   v_error  V_BltDisplayBuffer(v_handle, v_buffer_group, v_u32, v_rect*, v_buffer_group, v_u32, v_rect*);
   v_error  V_CreateBufferGroup(v_handle, v_buffer_group*, v_u32*, v_u32, v_u32, v_u32, v_u32, v_u32);
   v_error  V_DestroyBufferGroup(v_handle, v_buffer_group);
   v_error  V_RegisterErrorHandler(v_error_handler);
   v_error  V_GetFunctionName(v_routine, LPSTR, int);
   v_u32    V_GetUserPrivate(v_handle);
   void     V_SetUserPrivate(v_handle, v_u32);
   void*    V_LockBuffer(v_handle, v_buffer_group, v_u32);
   v_error  V_UnlockBuffer(v_handle, v_buffer_group, v_u32);
   v_error  V_RestoreBuffer(v_handle, v_buffer_group, v_u32);
   v_u32*   V_GetMemoryObjectAddress(v_memory);
   v_foreignAddress V_GetBufferAddress(v_buffer_group, v_u32);
   v_u32            V_GetBufferLinebytes(v_buffer_group, v_u32);
   v_error  V_CreateVerite(v_u32, HWND, v_handle*, char*);
   v_error  V_DestroyVerite(v_handle);
};

}; // namespace Rendition

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_RENDITIONPROXY_H_
