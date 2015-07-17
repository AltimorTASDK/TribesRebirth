//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <stdlib.h>

#include "renditionProxy.h"

//-------------------------------------- Space saver macro to be used ONLY in
//                                        create()...
#define CHECK_GETPROC_SUCCESS(function)                                \
   if (function == NULL) {                                             \
      AssertWarn(0, "Unable to load a fntable function for rendition") \
      delete pProxy;                                                   \
      return NULL;                                                     \
   }

// Function table structure...
//
namespace Rendition {

struct RenditionTable {
   // Redline.dll functions...
   //
   vl_error (V_DLLIMPORT *VL_OpenVerite)(HWND, v_handle*);
   vl_error (V_DLLIMPORT *VL_CloseVerite)(v_handle);
   vl_error (V_DLLIMPORT *VL_CreateSurface)(v_handle, v_surface**, v_u32, v_u32, v_u32, v_u32, v_u32);
   vl_error (V_DLLIMPORT *VL_DestroySurface)(v_handle, v_surface*);


   vl_error (V_DLLIMPORT *VL_RegisterErrorHandler)(vl_error_handler);
   vl_error (V_DLLIMPORT *VL_GetErrorText)(vl_error, LPSTR, int);
   vl_error (V_DLLIMPORT *VL_GetFunctionName)(vl_routine, LPSTR, int);

   vl_error (V_DLLIMPORT *VL_RestoreSurface)(v_handle, v_surface*);
   vl_error (V_DLLIMPORT *VL_FillBuffer)(v_cmdbuffer*, v_surface*, v_u32, v_u32, v_u32, v_u32, v_u32, v_u32);

   vl_error (V_DLLIMPORT *VL_InstallDstBuffer)(v_cmdbuffer*, v_surface*);
   vl_error (V_DLLIMPORT *VL_SetDstBase)(v_cmdbuffer*, v_u32);
   vl_error (V_DLLIMPORT *VL_SetDisplayBase)(v_cmdbuffer*, v_foreignAddress);
   vl_error (V_DLLIMPORT *VL_SwapDisplaySurface)(v_cmdbuffer*, v_surface*);
   
   vl_error (V_DLLIMPORT *VL_SetDitherEnable)(v_cmdbuffer*, v_u32);

   vl_error (V_DLLIMPORT *VL_SetBlendEnable)(v_cmdbuffer*, v_u32);
   vl_error (V_DLLIMPORT *VL_SetBlendDstFunc)(v_cmdbuffer*, v_u32);
   vl_error (V_DLLIMPORT *VL_SetBlendSrcFunc)(v_cmdbuffer*, v_u32);

   vl_error (V_DLLIMPORT *VL_SetSrcFilter)(v_cmdbuffer*, v_u32);
   vl_error (V_DLLIMPORT *VL_SetSrcFunc)(v_cmdbuffer*, v_u32);
   vl_error (V_DLLIMPORT *VL_SetSrcFmt)(v_cmdbuffer*, v_u32);
   vl_error (V_DLLIMPORT *VL_SetDstFmt)(v_cmdbuffer*, v_u32);

   vl_error (V_DLLIMPORT *VL_SetAlphaThreshold)(v_cmdbuffer*, v_u32);
   vl_error (V_DLLIMPORT *VL_SetChromaColor)(v_cmdbuffer*, v_u32, v_u32);
   vl_error (V_DLLIMPORT *VL_SetChromaMask)(v_cmdbuffer*, v_u32, v_u32);
   vl_error (V_DLLIMPORT *VL_SetChromaKey)(v_cmdbuffer*, v_u32);
   vl_error (V_DLLIMPORT *VL_SetSurfaceChromaColor)(v_surface*, v_u32);
   vl_error (V_DLLIMPORT *VL_SetSurfaceChromaMask)(v_surface*, v_u32);
   vl_error (V_DLLIMPORT *VL_SetSurfaceChromaKey)(v_surface*, v_u32);
   vl_error (V_DLLIMPORT *VL_SetTranspReject)(v_cmdbuffer*, v_u32);
   
   vl_error (V_DLLIMPORT *VL_SetFogColorRGB)(v_cmdbuffer*, v_u32);
   vl_error (V_DLLIMPORT *VL_SetFogEnable)(v_cmdbuffer*, v_u32);
   vl_error (V_DLLIMPORT *VL_SetF)(v_cmdbuffer*, v_u32);
   
   vl_error (V_DLLIMPORT *VL_SetFGColorARGB)(v_cmdbuffer*, v_u32);
   
   vl_error (V_DLLIMPORT *VL_Dot)(v_cmdbuffer*, v_u32, v_u32*);

   vl_error (V_DLLIMPORT *VL_SetPalette)(v_cmdbuffer*, v_u16, v_u16, v_u32*);
   vl_error (V_DLLIMPORT *VL_SetTexturePalette)(v_cmdbuffer*, v_u32, v_u32,
                                         v_u32*, v_u32);
   vl_error (V_DLLIMPORT *VL_InstallTextureMapBasic)(v_cmdbuffer*, v_surface*);   
   vl_error (V_DLLIMPORT *VL_InstallTextureMap)(v_cmdbuffer*, v_surface*);   
   vl_error (V_DLLIMPORT *VL_SetCurrentTexture)(v_cmdbuffer*, v_u32, v_u32,
                                         v_u16, v_u16, v_u32, v_u32);
   vl_error (V_DLLIMPORT *VL_SetUClamp)(v_cmdbuffer*, v_u32);
   vl_error (V_DLLIMPORT *VL_SetUMask)(v_cmdbuffer*, v_u32);
   vl_error (V_DLLIMPORT *VL_SetUMultiplier)(v_cmdbuffer*, v_u32);
   vl_error (V_DLLIMPORT *VL_SetVClamp)(v_cmdbuffer*, v_u32);
   vl_error (V_DLLIMPORT *VL_SetVMask)(v_cmdbuffer*, v_u32);
   vl_error (V_DLLIMPORT *VL_SetVMultiplier)(v_cmdbuffer*, v_u32);
   
   vl_error (V_DLLIMPORT *VL_SetScissorX)(v_cmdbuffer*, v_u32);
   vl_error (V_DLLIMPORT *VL_SetScissorY)(v_cmdbuffer*, v_u32);

   vl_error (V_DLLIMPORT *VL_SetSOffset)(v_cmdbuffer*, v_u32);
   vl_error (V_DLLIMPORT *VL_SetTOffset)(v_cmdbuffer*, v_u32);

   vl_error (V_DLLIMPORT *VL_Bitblt)(v_cmdbuffer*, v_u16, v_u16, v_u16, v_u16, v_u16, v_u16);
   vl_error (V_DLLIMPORT *VL_BitfillNoPat)(v_cmdbuffer*, v_surface*, v_u32, v_u32,
                                         v_u16, v_u16, v_u16, v_u16);
   vl_error (V_DLLIMPORT *VL_Line)(v_cmdbuffer*, v_u32, v_u32*, v_u32*);
   vl_error (V_DLLIMPORT *VL_Rectangle)(v_cmdbuffer*, v_u32, v_u32, v_u32, v_u32*);
   vl_error (V_DLLIMPORT *VL_MemWriteRect)(v_cmdbuffer*, v_foreignAddress, v_u32,
                                         v_memory, v_u32*, v_u32, v_u32, v_u32,
                                         v_u32);
   vl_error (V_DLLIMPORT *VL_LoadBuffer)(v_cmdbuffer*, v_surface*, v_u32, v_u32,
                                       v_u32, v_u32, v_memory, v_u32*);
   vl_error (V_DLLIMPORT *VL_Lookup)(v_cmdbuffer*, v_u16, v_u16, v_u16, v_u16, v_memory, v_u32*);
   vl_error (V_DLLIMPORT *VL_Trifan)(v_cmdbuffer*, v_u32, v_u32, v_u32*);
   vl_error (V_DLLIMPORT *VL_Triangle)(v_cmdbuffer*, v_u32, v_u32*, v_u32*, v_u32*);
   vl_error (V_DLLIMPORT *VL_WaitForDisplaySwitch)(v_cmdbuffer*);
   vl_error (V_DLLIMPORT *VL_MemFillRect)(v_cmdbuffer*, v_foreignAddress, v_u32, v_u32, v_u32, v_u32, v_u32);

   // Verite.dll functions...
   //
   vl_error    (V_DLLIMPORT *V_SetDisplayType)(v_handle, v_u32);
   vl_error    (V_DLLIMPORT *V_SetDisplayMode)(v_handle, v_u32, v_u32, v_u32, v_u32);
   v_cmdbuffer (V_DLLIMPORT *V_CreateCmdBuffer)(v_handle, int, int);
   v_error     (V_DLLIMPORT *V_DestroyCmdBuffer)(v_cmdbuffer);

   v_error  (V_DLLIMPORT *V_GetErrorText)(v_error, LPSTR, int);
   v_u32*   (V_DLLIMPORT *V_AddToCmdList)(v_cmdbuffer*, v_u32);
   v_u32*   (V_DLLIMPORT *V_AddToDMAList)(v_cmdbuffer*, v_u32, v_memory, v_u32*, v_u32);
   v_error  (V_DLLIMPORT *V_IssueCmdBuffer)(v_handle, v_cmdbuffer);
   v_error  (V_DLLIMPORT *V_IssueCmdBufferAsync)(v_handle, v_cmdbuffer);
   v_error  (V_DLLIMPORT *V_GetCmdBufferFreeSpace)(v_cmdbuffer, v_u32*, v_u32*);
   v_u32    (V_DLLIMPORT *V_QueryCmdBuffer)(v_handle, v_cmdbuffer);
   void     (V_DLLIMPORT *V_SetCmdBufferCallBack)(v_cmdbuffer, v_cmdbuffer_callback);
   v_memory (V_DLLIMPORT *V_AllocLockedMem)(v_handle, v_u32);
   v_error  (V_DLLIMPORT *V_FreeLockedMem)(v_handle, v_memory);
   v_memory (V_DLLIMPORT *V_AllocMemoryObject)(v_handle, v_u32*, v_u32);
   v_error  (V_DLLIMPORT *V_FreeMemoryObject)(v_handle, v_memory);
   v_error  (V_DLLIMPORT *V_SetMemoryObject)(v_memory, v_u32*, v_u32);
   v_u32    (V_DLLIMPORT *V_GetMemoryObjectStatus)(v_memory);
   v_error  (V_DLLIMPORT *V_LockPages)(v_handle, v_memory);
   v_error  (V_DLLIMPORT *V_UnlockPages)(v_handle, v_memory);
   v_error  (V_DLLIMPORT *V_BltDisplayBuffer)(v_handle, v_buffer_group, v_u32, v_rect*, v_buffer_group, v_u32, v_rect*);
   v_error  (V_DLLIMPORT *V_CreateBufferGroup)(v_handle, v_buffer_group*, v_u32*, v_u32, v_u32, v_u32, v_u32, v_u32);
   v_error  (V_DLLIMPORT *V_DestroyBufferGroup)(v_handle, v_buffer_group);
   v_error  (V_DLLIMPORT *V_RegisterErrorHandler)(v_error_handler);
   v_error  (V_DLLIMPORT *V_GetFunctionName)(v_routine, LPSTR, int);
   v_u32    (V_DLLIMPORT *V_GetUserPrivate)(v_handle);
   void     (V_DLLIMPORT *V_SetUserPrivate)(v_handle, v_u32);
   void*    (V_DLLIMPORT *V_LockBuffer)(v_handle, v_buffer_group, v_u32);
   v_error  (V_DLLIMPORT *V_UnlockBuffer)(v_handle, v_buffer_group, v_u32);
   v_error  (V_DLLIMPORT *V_RestoreBuffer)(v_handle, v_buffer_group, v_u32);
   v_foreignAddress (V_DLLIMPORT *V_GetBufferAddress)(v_buffer_group, v_u32);
   v_u32            (V_DLLIMPORT *V_GetBufferLinebytes)(v_buffer_group, v_u32);
   v_u32*   (V_DLLIMPORT *V_GetMemoryObjectAddress)(v_memory);
   v_error  (V_DLLIMPORT *V_CreateVerite)(v_u32, HWND, v_handle*, char*);
   v_error  (V_DLLIMPORT *V_DestroyVerite)(v_handle);
};



RenditionProxy::RenditionProxy()
{
   m_pTable = new RenditionTable;
   memset(m_pTable, 0, sizeof(RenditionTable));
}


RenditionProxy* 
RenditionProxy::create(const HINSTANCE in_VeriteDLL,
                       const HINSTANCE in_RedlineDLL)
{
   AssertFatal(in_VeriteDLL != NULL, "Error, bad verite instance...");
   AssertFatal(in_RedlineDLL != NULL, "Error, bad redline instance...");

   RenditionProxy* pProxy = new RenditionProxy;
   

//-------------------------------------- REDLINE IMPORTS
   pProxy->m_pTable->VL_OpenVerite =
      (vl_error (V_DLLIMPORT *)(HWND, v_handle*))
      GetProcAddress(in_RedlineDLL, "VL_OpenVerite");
   pProxy->m_pTable->VL_CloseVerite = 
      (vl_error (V_DLLIMPORT *)(v_handle))
      GetProcAddress(in_RedlineDLL, "VL_CloseVerite");
   pProxy->m_pTable->VL_CreateSurface =
      (vl_error (V_DLLIMPORT *)(v_handle, v_surface**, v_u32, v_u32, v_u32, v_u32, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_CreateSurface");
   pProxy->m_pTable->VL_DestroySurface =
      (vl_error (V_DLLIMPORT *)(v_handle, v_surface*))
      GetProcAddress(in_RedlineDLL, "VL_DestroySurface");
   pProxy->m_pTable->VL_RegisterErrorHandler =
      (vl_error  (V_DLLIMPORT *)(vl_error_handler))
      GetProcAddress(in_RedlineDLL, "VL_RegisterErrorHandler");
   pProxy->m_pTable->VL_GetErrorText =
      (vl_error (V_DLLIMPORT *)(vl_error, LPSTR, int))
      GetProcAddress(in_RedlineDLL, "VL_GetErrorText");
   pProxy->m_pTable->VL_GetFunctionName =
      (vl_error  (V_DLLIMPORT *)(vl_routine, LPSTR, int))
      GetProcAddress(in_RedlineDLL, "VL_GetFunctionName");
   pProxy->m_pTable->VL_RestoreSurface =
      (vl_error (V_DLLIMPORT *)(v_handle, v_surface*))
      GetProcAddress(in_RedlineDLL, "VL_RestoreSurface");
   pProxy->m_pTable->VL_FillBuffer =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_surface*, v_u32, v_u32, v_u32, v_u32, v_u32, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_FillBuffer");
   pProxy->m_pTable->VL_InstallDstBuffer =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_surface*))
      GetProcAddress(in_RedlineDLL, "VL_InstallDstBuffer");
   pProxy->m_pTable->VL_SetDstBase =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetDstBase");
   pProxy->m_pTable->VL_SetDisplayBase =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_foreignAddress))
      GetProcAddress(in_RedlineDLL, "VL_SetDisplayBase");
   pProxy->m_pTable->VL_SwapDisplaySurface =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_surface*))
      GetProcAddress(in_RedlineDLL, "VL_SwapDisplaySurface");
   pProxy->m_pTable->VL_SetDitherEnable =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetDitherEnable");
   pProxy->m_pTable->VL_SetBlendEnable =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetBlendEnable");
   pProxy->m_pTable->VL_SetBlendDstFunc =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetBlendDstFunc");
   pProxy->m_pTable->VL_SetBlendSrcFunc =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetBlendSrcFunc");
   pProxy->m_pTable->VL_SetSrcFmt =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetSrcFmt");
   pProxy->m_pTable->VL_SetDstFmt =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetDstFmt");
   pProxy->m_pTable->VL_SetSrcFunc =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetSrcFunc");
   pProxy->m_pTable->VL_SetSrcFilter =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetSrcFilter");
   pProxy->m_pTable->VL_SetAlphaThreshold =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetAlphaThreshold");
   pProxy->m_pTable->VL_SetChromaColor =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetChromaColor");
   pProxy->m_pTable->VL_SetChromaMask =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetChromaMask");
   pProxy->m_pTable->VL_SetChromaKey =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetChromaKey");
   pProxy->m_pTable->VL_SetSurfaceChromaColor =
      (vl_error (V_DLLIMPORT *)(v_surface*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetSurfaceChromaColor");
   pProxy->m_pTable->VL_SetSurfaceChromaMask =
      (vl_error (V_DLLIMPORT *)(v_surface*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetSurfaceChromaMask");
   pProxy->m_pTable->VL_SetSurfaceChromaKey =
      (vl_error (V_DLLIMPORT *)(v_surface*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetSurfaceChromaKey");
   pProxy->m_pTable->VL_SetTranspReject =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetTranspReject");
   pProxy->m_pTable->VL_SetFogColorRGB =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetFogColorRGB");
   pProxy->m_pTable->VL_SetFogEnable =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetFogEnable");
   pProxy->m_pTable->VL_SetF =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetF");
   pProxy->m_pTable->VL_SetFGColorARGB =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetFGColorARGB");
   pProxy->m_pTable->VL_Dot =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32, v_u32*))
      GetProcAddress(in_RedlineDLL, "VL_Dot");
   pProxy->m_pTable->VL_SetPalette =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u16, v_u16, v_u32*))
      GetProcAddress(in_RedlineDLL, "VL_SetPalette");
   pProxy->m_pTable->VL_SetTexturePalette =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32, v_u32, v_u32*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetTexturePalette");
   pProxy->m_pTable->VL_InstallTextureMapBasic =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_surface*))
      GetProcAddress(in_RedlineDLL, "VL_InstallTextureMapBasic");   
   pProxy->m_pTable->VL_InstallTextureMap =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_surface*))
      GetProcAddress(in_RedlineDLL, "VL_InstallTextureMap");
   pProxy->m_pTable->VL_SetCurrentTexture =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32, v_u32, v_u16, v_u16, v_u32, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetCurrentTexture");
   pProxy->m_pTable->VL_SetUClamp =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetUClamp");
   pProxy->m_pTable->VL_SetUMask =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetUMask");
   pProxy->m_pTable->VL_SetUMultiplier =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetUMultiplier");
   pProxy->m_pTable->VL_SetVClamp =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetVClamp");
   pProxy->m_pTable->VL_SetVMask =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetVMask");
   pProxy->m_pTable->VL_SetVMultiplier =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetVMultiplier");
   pProxy->m_pTable->VL_SetScissorX =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetScissorX");
   pProxy->m_pTable->VL_SetScissorY =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetScissorY");
   pProxy->m_pTable->VL_SetSOffset =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetSOffset");
   pProxy->m_pTable->VL_SetTOffset =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_SetTOffset");
   pProxy->m_pTable->VL_Bitblt =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u16, v_u16, v_u16, v_u16, v_u16, v_u16))
      GetProcAddress(in_RedlineDLL, "VL_Bitblt");
   pProxy->m_pTable->VL_BitfillNoPat =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_surface*, v_u32, v_u32,
                              v_u16, v_u16, v_u16, v_u16))
      GetProcAddress(in_RedlineDLL, "VL_BitfillNoPat");
   pProxy->m_pTable->VL_Line =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32, v_u32*, v_u32*))
      GetProcAddress(in_RedlineDLL, "VL_Line");
   pProxy->m_pTable->VL_Rectangle =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32, v_u32, v_u32, v_u32*))
      GetProcAddress(in_RedlineDLL, "VL_Rectangle");
   pProxy->m_pTable->VL_MemWriteRect =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_foreignAddress, v_u32, v_memory, v_u32*, v_u32, v_u32, v_u32, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_MemWriteRect");
   pProxy->m_pTable->VL_LoadBuffer =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_surface*, v_u32, v_u32, v_u32, v_u32, v_memory, v_u32*))
      GetProcAddress(in_RedlineDLL, "VL_LoadBuffer");
   pProxy->m_pTable->VL_Lookup =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u16, v_u16, v_u16, v_u16, v_memory, v_u32*))
      GetProcAddress(in_RedlineDLL, "VL_Lookup");
   pProxy->m_pTable->VL_Trifan =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32, v_u32, v_u32*))
      GetProcAddress(in_RedlineDLL, "VL_Trifan");
   pProxy->m_pTable->VL_Triangle =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_u32, v_u32*, v_u32*, v_u32*))
      GetProcAddress(in_RedlineDLL, "VL_Triangle");
   pProxy->m_pTable->VL_WaitForDisplaySwitch =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*))
      GetProcAddress(in_RedlineDLL, "VL_WaitForDisplaySwitch");
   pProxy->m_pTable->VL_MemFillRect =
      (vl_error (V_DLLIMPORT *)(v_cmdbuffer*, v_foreignAddress, v_u32, v_u32, v_u32, v_u32, v_u32))
      GetProcAddress(in_RedlineDLL, "VL_MemFillRect");

//-------------------------------------- VERITE IMPORTS
   pProxy->m_pTable->V_SetDisplayType = 
      (v_error (V_DLLIMPORT *)(v_handle, v_u32))
      GetProcAddress(in_VeriteDLL, "V_SetDisplayType");
   pProxy->m_pTable->V_SetDisplayMode =
      (v_error (V_DLLIMPORT *)(v_handle, v_u32, v_u32, v_u32, v_u32))
      GetProcAddress(in_VeriteDLL, "V_SetDisplayMode");
   pProxy->m_pTable->V_CreateCmdBuffer = 
      (v_cmdbuffer (V_DLLIMPORT *)(v_handle, int, int))
      GetProcAddress(in_VeriteDLL, "V_CreateCmdBuffer");
   pProxy->m_pTable->V_DestroyCmdBuffer =
      (v_error (V_DLLIMPORT *)(v_cmdbuffer))
      GetProcAddress(in_VeriteDLL, "V_DestroyCmdBuffer");
   pProxy->m_pTable->V_GetErrorText =
      (v_error  (V_DLLIMPORT *)(v_error, LPSTR, int))
      GetProcAddress(in_VeriteDLL, "V_GetErrorText");
   pProxy->m_pTable->V_AddToCmdList =
      (v_u32*   (V_DLLIMPORT *)(v_cmdbuffer*, v_u32))
      GetProcAddress(in_VeriteDLL, "V_AddToCmdList");
   pProxy->m_pTable->V_AddToDMAList =
      (v_u32*   (V_DLLIMPORT *)(v_cmdbuffer*, v_u32, v_memory, v_u32*, v_u32))
      GetProcAddress(in_VeriteDLL, "V_AddToDMAList");
   pProxy->m_pTable->V_IssueCmdBuffer =
      (v_error  (V_DLLIMPORT *)(v_handle, v_cmdbuffer))
      GetProcAddress(in_VeriteDLL, "V_IssueCmdBuffer");
   pProxy->m_pTable->V_IssueCmdBufferAsync =
      (v_error  (V_DLLIMPORT *)(v_handle, v_cmdbuffer))
      GetProcAddress(in_VeriteDLL, "V_IssueCmdBufferAsync");
   pProxy->m_pTable->V_GetCmdBufferFreeSpace =
      (v_error  (V_DLLIMPORT *)(v_cmdbuffer, v_u32*, v_u32*))
      GetProcAddress(in_VeriteDLL, "V_GetCmdBufferFreeSpace");
   pProxy->m_pTable->V_QueryCmdBuffer =
      (v_u32    (V_DLLIMPORT *)(v_handle, v_cmdbuffer))
      GetProcAddress(in_VeriteDLL, "V_QueryCmdBuffer");
   pProxy->m_pTable->V_SetCmdBufferCallBack =
      (void     (V_DLLIMPORT *)(v_cmdbuffer, v_cmdbuffer_callback))
      GetProcAddress(in_VeriteDLL, "V_SetCmdBufferCallBack");
   pProxy->m_pTable->V_AllocLockedMem =
      (v_memory (V_DLLIMPORT *)(v_handle, v_u32))
      GetProcAddress(in_VeriteDLL, "V_AllocLockedMem");
   pProxy->m_pTable->V_FreeLockedMem =
      (v_error  (V_DLLIMPORT *)(v_handle, v_memory))
      GetProcAddress(in_VeriteDLL, "V_FreeLockedMem");
   pProxy->m_pTable->V_AllocMemoryObject =
      (v_memory (V_DLLIMPORT *)(v_handle, v_u32*, v_u32))
      GetProcAddress(in_VeriteDLL, "V_AllocMemoryObject");
   pProxy->m_pTable->V_FreeMemoryObject =
      (v_error  (V_DLLIMPORT *)(v_handle, v_memory))
      GetProcAddress(in_VeriteDLL, "V_FreeMemoryObject");
   pProxy->m_pTable->V_SetMemoryObject =
      (v_error  (V_DLLIMPORT *)(v_memory, v_u32*, v_u32))
      GetProcAddress(in_VeriteDLL, "V_SetMemoryObject");
   pProxy->m_pTable->V_GetMemoryObjectStatus =
      (v_u32    (V_DLLIMPORT *)(v_memory))
      GetProcAddress(in_VeriteDLL, "V_GetMemoryObjectStatus");
   pProxy->m_pTable->V_LockPages =
      (v_error  (V_DLLIMPORT *)(v_handle, v_memory))
      GetProcAddress(in_VeriteDLL, "V_LockPages");
   pProxy->m_pTable->V_UnlockPages =
      (v_error  (V_DLLIMPORT *)(v_handle, v_memory))
      GetProcAddress(in_VeriteDLL, "V_UnlockPages");
   pProxy->m_pTable->V_BltDisplayBuffer =
      (v_error  (V_DLLIMPORT *)(v_handle, v_buffer_group, v_u32, v_rect*, v_buffer_group, v_u32, v_rect*))
      GetProcAddress(in_VeriteDLL, "V_BltDisplayBuffer");
   pProxy->m_pTable->V_CreateBufferGroup =
      (v_error  (V_DLLIMPORT *)(v_handle, v_buffer_group*, v_u32*, v_u32, v_u32, v_u32, v_u32, v_u32))
      GetProcAddress(in_VeriteDLL, "V_CreateBufferGroup");
   pProxy->m_pTable->V_DestroyBufferGroup =
      (v_error  (V_DLLIMPORT *)(v_handle, v_buffer_group))
      GetProcAddress(in_VeriteDLL, "V_DestroyBufferGroup");
   pProxy->m_pTable->V_RegisterErrorHandler =
      (v_error  (V_DLLIMPORT *)(v_error_handler))
      GetProcAddress(in_VeriteDLL, "V_RegisterErrorHandler");
   pProxy->m_pTable->V_GetFunctionName =
      (v_error  (V_DLLIMPORT *)(v_routine, LPSTR, int))
      GetProcAddress(in_VeriteDLL, "V_GetFunctionName");
   pProxy->m_pTable->V_GetUserPrivate =
      (v_u32    (V_DLLIMPORT *)(v_handle))
      GetProcAddress(in_VeriteDLL, "V_GetUserPrivate");
   pProxy->m_pTable->V_SetUserPrivate =
      (void     (V_DLLIMPORT *)(v_handle, v_u32))
      GetProcAddress(in_VeriteDLL, "V_SetUserPrivate");
   pProxy->m_pTable->V_LockBuffer =
      (void*    (V_DLLIMPORT *)(v_handle, v_buffer_group, v_u32))
      GetProcAddress(in_VeriteDLL, "V_LockBuffer");
   pProxy->m_pTable->V_UnlockBuffer =
      (v_error  (V_DLLIMPORT *)(v_handle, v_buffer_group, v_u32))
      GetProcAddress(in_VeriteDLL, "V_UnlockBuffer");
   pProxy->m_pTable->V_RestoreBuffer =
      (v_error  (V_DLLIMPORT *)(v_handle, v_buffer_group, v_u32))
      GetProcAddress(in_VeriteDLL, "V_RestoreBuffer");
   pProxy->m_pTable->V_GetBufferAddress =
      (v_foreignAddress (V_DLLIMPORT *)(v_buffer_group, v_u32))
      GetProcAddress(in_VeriteDLL, "V_GetBufferAddress");
   pProxy->m_pTable->V_GetBufferLinebytes =
      (v_u32 (V_DLLIMPORT *)(v_buffer_group, v_u32))
      GetProcAddress(in_VeriteDLL, "V_GetBufferLinebytes");
   pProxy->m_pTable->V_GetMemoryObjectAddress =
      (v_u32*   (V_DLLIMPORT *)(v_memory))
      GetProcAddress(in_VeriteDLL, "V_GetMemoryObjectAddress");
   pProxy->m_pTable->V_CreateVerite =
      (v_error (V_DLLIMPORT*)(v_u32, HWND, v_handle*, char*))
      GetProcAddress(in_VeriteDLL, "V_CreateVerite");
   pProxy->m_pTable->V_DestroyVerite =
      (v_error (V_DLLIMPORT*)(v_handle))
      GetProcAddress(in_VeriteDLL, "V_DestroyVerite");


   // Check for failure for any of these functions...
   //
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_OpenVerite);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_CloseVerite);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_CreateSurface);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_DestroySurface);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_RegisterErrorHandler);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_GetErrorText);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_GetFunctionName);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_RestoreSurface);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_FillBuffer);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_InstallDstBuffer);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetDstBase);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetDisplayBase);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SwapDisplaySurface);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetDitherEnable);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetBlendEnable);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetBlendDstFunc);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetBlendSrcFunc);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetSrcFmt);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetDstFmt);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetSrcFunc);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetSrcFilter);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetAlphaThreshold);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetChromaColor);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetChromaMask);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetChromaKey);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetSurfaceChromaColor);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetSurfaceChromaMask);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetSurfaceChromaKey);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetTranspReject);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetFogColorRGB);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetFogEnable);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetF);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetFGColorARGB);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_Dot);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetPalette);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetTexturePalette);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_InstallTextureMapBasic);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_InstallTextureMap);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetCurrentTexture);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetUClamp);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetUMask);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetUMultiplier);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetVClamp);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetVMask);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetVMultiplier);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetScissorX);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetScissorY);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetSOffset);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_SetTOffset);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_Bitblt);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_BitfillNoPat);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_Line);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_Rectangle);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_MemWriteRect);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_LoadBuffer);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_Lookup);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_Trifan);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_Triangle);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_WaitForDisplaySwitch);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->VL_MemFillRect);

   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_SetDisplayType);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_SetDisplayMode);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_CreateCmdBuffer);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_DestroyCmdBuffer);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_GetErrorText);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_AddToCmdList);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_AddToDMAList);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_IssueCmdBuffer);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_IssueCmdBufferAsync);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_GetCmdBufferFreeSpace);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_QueryCmdBuffer);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_SetCmdBufferCallBack);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_AllocLockedMem);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_FreeLockedMem);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_AllocMemoryObject);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_FreeMemoryObject);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_SetMemoryObject);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_GetMemoryObjectStatus);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_LockPages);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_UnlockPages);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_BltDisplayBuffer);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_CreateBufferGroup);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_DestroyBufferGroup);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_RegisterErrorHandler);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_GetFunctionName);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_GetUserPrivate);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_SetUserPrivate);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_LockBuffer);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_UnlockBuffer);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_RestoreBuffer);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_GetBufferAddress);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_GetBufferLinebytes);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_GetMemoryObjectAddress);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_CreateVerite);
   CHECK_GETPROC_SUCCESS(pProxy->m_pTable->V_DestroyVerite);
   
   return pProxy;
}


//------------------------------------------------------------------------------
//-------------------------------------- STUB FUNCTIONS...
//
vl_error 
RenditionProxy::VL_OpenVerite(HWND      io_hwnd,
                              v_handle* out_pVHandle)
{
   return m_pTable->VL_OpenVerite(io_hwnd, out_pVHandle);
}

vl_error 
RenditionProxy::VL_CloseVerite(v_handle io_vHandle)
{
   return m_pTable->VL_CloseVerite(io_vHandle);
}

vl_error 
RenditionProxy::VL_CreateSurface(v_handle    io_vHandle,
                                 v_surface** out_ppVSurface,
                                 const v_u32 in_bufferMask,
                                 const v_u32 in_numBuffers,
                                 const v_u32 in_pixelFmt,
                                 const v_u32 in_width,
                                 const v_u32 in_height)
{
   return m_pTable->VL_CreateSurface(io_vHandle, out_ppVSurface, in_bufferMask,
                                     in_numBuffers, in_pixelFmt,
                                     in_width, in_height);
}

vl_error 
RenditionProxy::VL_DestroySurface(v_handle   io_vHandle,
                                  v_surface* io_pVSurface)
{
   return m_pTable->VL_DestroySurface(io_vHandle, io_pVSurface);
}

vl_error 
RenditionProxy::V_SetDisplayType(v_handle    io_vHandle,
                                 const v_u32 in_displayType)
{
   return m_pTable->V_SetDisplayType(io_vHandle, in_displayType);
}


vl_error 
RenditionProxy::V_SetDisplayMode(v_handle    io_vHandle,
                                 const v_u32 in_width,
                                 const v_u32 in_height,
                                 const v_u32 in_bpp,
                                 const v_u32 in_refreshRate)
{
   return m_pTable->V_SetDisplayMode(io_vHandle,
                                     in_width, in_height,
                                     in_bpp,
                                     in_refreshRate);
}

v_cmdbuffer 
RenditionProxy::V_CreateCmdBuffer(v_handle  io_vHandle,
                                  const int in_dmaListEntries,
                                  const int in_cmdListSize)
{
   return m_pTable->V_CreateCmdBuffer(io_vHandle,
                                      in_dmaListEntries,
                                      in_cmdListSize);
}

v_error 
RenditionProxy::V_DestroyCmdBuffer(v_cmdbuffer io_trashBuffer)
{
   return m_pTable->V_DestroyCmdBuffer(io_trashBuffer);
}

vl_error 
RenditionProxy::VL_RegisterErrorHandler(vl_error_handler io_pErrorHandler)
{
   return m_pTable->VL_RegisterErrorHandler(io_pErrorHandler);
}

vl_error 
RenditionProxy::VL_GetErrorText(const vl_error in_error,
                                LPSTR          out_pString,
                                const int      in_bufSize)
{
   return m_pTable->VL_GetErrorText(in_error, out_pString, in_bufSize);
}

vl_error 
RenditionProxy::VL_GetFunctionName(vl_routine io_routine,
                                   LPSTR      out_pString,
                                   int        io_bufSize)
{
   return m_pTable->VL_GetFunctionName(io_routine, out_pString, io_bufSize);
}

vl_error 
RenditionProxy::VL_RestoreSurface(v_handle   io_vHandle,
                                  v_surface* io_pVSurface)
{
   return m_pTable->VL_RestoreSurface(io_vHandle, io_pVSurface);
}

vl_error 
RenditionProxy::VL_FillBuffer(v_cmdbuffer* io_pCmdBuff,
                              v_surface*   io_pVSurface,
                              v_u32        io_buffer,
                              v_u32        io_xOrg,
                              v_u32        io_yOrg,
                              v_u32        io_width,
                              v_u32        io_height,
                              v_u32        io_pixVal)
{
   return m_pTable->VL_FillBuffer(io_pCmdBuff, io_pVSurface, io_buffer,
                                  io_xOrg, io_yOrg, io_width, io_height,
                                  io_pixVal);
}


vl_error 
RenditionProxy::VL_InstallDstBuffer(v_cmdbuffer* io_pCmdBuff,
                                    v_surface*   io_pVSurface)
{
   return m_pTable->VL_InstallDstBuffer(io_pCmdBuff, io_pVSurface);
}

vl_error 
RenditionProxy::VL_SetDstBase(v_cmdbuffer* io_pCmdBuff,
                              v_u32        io_base)
{
   return m_pTable->VL_SetDstBase(io_pCmdBuff, io_base);
}

vl_error 
RenditionProxy::VL_SetDisplayBase(v_cmdbuffer*     io_pCmdBuff,
                                  v_foreignAddress io_addr)
{
   return m_pTable->VL_SetDisplayBase(io_pCmdBuff, io_addr);
}

vl_error 
RenditionProxy::VL_SwapDisplaySurface(v_cmdbuffer* io_pCmdBuff,
                                      v_surface*   io_pVSurface)
{
   return m_pTable->VL_SwapDisplaySurface(io_pCmdBuff, io_pVSurface);
}


vl_error 
RenditionProxy::VL_SetDitherEnable(v_cmdbuffer* io_pCmdBuff,
                                   v_u32        io_enable)
{
   return m_pTable->VL_SetDitherEnable(io_pCmdBuff, io_enable);
}

vl_error 
RenditionProxy::VL_SetBlendEnable(v_cmdbuffer* io_pCmdBuff,
                                  v_u32        io_enable)
{
   return m_pTable->VL_SetBlendEnable(io_pCmdBuff, io_enable);
}

vl_error 
RenditionProxy::VL_SetBlendDstFunc(v_cmdbuffer* io_pCmdBuff,
                                   v_u32        io_dstBlendFunc)
{
   return m_pTable->VL_SetBlendDstFunc(io_pCmdBuff, io_dstBlendFunc);
}

vl_error 
RenditionProxy::VL_SetBlendSrcFunc(v_cmdbuffer* io_pCmdBuff,
                                   v_u32        io_srcBlendFunc)
{
   return m_pTable->VL_SetBlendSrcFunc(io_pCmdBuff, io_srcBlendFunc);
}


vl_error 
RenditionProxy::VL_SetSrcFmt(v_cmdbuffer* io_pCmdBuff,
                             v_u32        io_srcFmt)
{
   return m_pTable->VL_SetSrcFmt(io_pCmdBuff, io_srcFmt);
}

vl_error 
RenditionProxy::VL_SetDstFmt(v_cmdbuffer* io_pCmdBuff,
                             v_u32        io_dstFmt)
{
   return m_pTable->VL_SetDstFmt(io_pCmdBuff, io_dstFmt);
}

vl_error 
RenditionProxy::VL_SetSrcFunc(v_cmdbuffer* io_pCmdBuff,
                              v_u32        io_srcFunc)
{
   return m_pTable->VL_SetSrcFunc(io_pCmdBuff, io_srcFunc);
}

vl_error 
RenditionProxy::VL_SetSrcFilter(v_cmdbuffer* io_pCmdBuff,
                                v_u32        io_srcFilter)
{
   return m_pTable->VL_SetSrcFilter(io_pCmdBuff, io_srcFilter);
}


vl_error 
RenditionProxy::VL_SetAlphaThreshold(v_cmdbuffer* io_pCmdBuff,
                                     v_u32        io_alphaThreshold)
{
   return m_pTable->VL_SetAlphaThreshold(io_pCmdBuff, io_alphaThreshold);
}

vl_error 
RenditionProxy::VL_SetChromaColor(v_cmdbuffer* io_pCmdBuff,
                                  v_u32        io_color,
                                  v_u32        io_fmt)
{
   return m_pTable->VL_SetChromaColor(io_pCmdBuff, io_color, io_fmt);
}

vl_error 
RenditionProxy::VL_SetChromaMask(v_cmdbuffer* io_pCmdBuff,
                                  v_u32        io_mask,
                                  v_u32        io_fmt)
{
   return m_pTable->VL_SetChromaMask(io_pCmdBuff, io_mask, io_fmt);
}

vl_error 
RenditionProxy::VL_SetChromaKey(v_cmdbuffer* io_pCmdBuff,
                                v_u32        io_enable)
{
   return m_pTable->VL_SetChromaKey(io_pCmdBuff, io_enable);
}

vl_error 
RenditionProxy::VL_SetSurfaceChromaColor(v_surface* io_pVSurface,
                                         v_u32      io_color)
{
   return m_pTable->VL_SetSurfaceChromaColor(io_pVSurface, io_color);
}

vl_error 
RenditionProxy::VL_SetSurfaceChromaMask(v_surface* io_pVSurface,
                                        v_u32      io_mask)
{
   return m_pTable->VL_SetSurfaceChromaMask(io_pVSurface, io_mask);
}

vl_error 
RenditionProxy::VL_SetSurfaceChromaKey(v_surface* io_pVSurface,
                                       v_u32      io_enable)
{
   return m_pTable->VL_SetSurfaceChromaKey(io_pVSurface, io_enable);
}

vl_error 
RenditionProxy::VL_SetTranspReject(v_cmdbuffer* io_pCmdBuff,
                                   v_u32        io_enable)
{
   return m_pTable->VL_SetTranspReject(io_pCmdBuff, io_enable);
}


vl_error 
RenditionProxy::VL_SetFogColorRGB(v_cmdbuffer* io_pCmdBuff,
                                  v_u32        io_fogColor)
{
   return m_pTable->VL_SetFogColorRGB(io_pCmdBuff, io_fogColor);
}

vl_error 
RenditionProxy::VL_SetFogEnable(v_cmdbuffer* io_pCmdBuff,
                                v_u32        io_enable)
{
   return m_pTable->VL_SetFogEnable(io_pCmdBuff, io_enable);
}


vl_error 
RenditionProxy::VL_SetF(v_cmdbuffer* io_pCmdBuff,
                        v_u32        io_level)
{
   return m_pTable->VL_SetF(io_pCmdBuff, io_level);
}


vl_error 
RenditionProxy::VL_SetFGColorARGB(v_cmdbuffer* io_pCmdBuff,
                                  v_u32        io_fgColor)
{
   return m_pTable->VL_SetFGColorARGB(io_pCmdBuff, io_fgColor);
}


vl_error 
RenditionProxy::VL_Dot(v_cmdbuffer* io_pCmdBuff,
                       v_u32        io_vType,
                       v_u32*       io_pVert)
{
   return m_pTable->VL_Dot(io_pCmdBuff, io_vType, io_pVert);
}


vl_error 
RenditionProxy::VL_SetPalette(v_cmdbuffer* io_pCmdBuff,
                              v_u16        io_start,
                              v_u16        io_numEntries,
                              v_u32*       io_pEntries)
{
   return m_pTable->VL_SetPalette(io_pCmdBuff, io_start,
                                  io_numEntries, io_pEntries);
}

vl_error 
RenditionProxy::VL_SetTexturePalette(v_cmdbuffer* io_pCmdBuff,
                                     v_u32        io_start,
                                     v_u32        io_numEntries,
                                     v_u32*       io_pEntries,
                                     v_u32        io_fmt)
{
   return m_pTable->VL_SetTexturePalette(io_pCmdBuff, io_start,
                                         io_numEntries, io_pEntries,
                                         io_fmt);
}

vl_error 
RenditionProxy::VL_InstallTextureMapBasic(v_cmdbuffer* io_pCmdBuff,
                                     v_surface*   io_pVSurface)
{
   return m_pTable->VL_InstallTextureMapBasic(io_pCmdBuff, io_pVSurface);
}

vl_error 
RenditionProxy::VL_InstallTextureMap(v_cmdbuffer* io_pCmdBuff,
                                     v_surface*   io_pVSurface)
{
   return m_pTable->VL_InstallTextureMap(io_pCmdBuff, io_pVSurface);
}
   
vl_error 
RenditionProxy::VL_SetCurrentTexture(v_cmdbuffer* io_pCmdBuff,
                                     v_u32        io_srcBase,
                                     v_u32        io_srcStride,
                                     v_u16        io_vMask,
                                     v_u16        io_uMask,
                                     v_u32        io_uMult,
                                     v_u32        io_vMult)
{
   return m_pTable->VL_SetCurrentTexture(io_pCmdBuff,
                                         io_srcBase, io_srcStride,
                                         io_vMask, io_uMask,
                                         io_uMult, io_vMult);
}

vl_error 
RenditionProxy::VL_SetUClamp(v_cmdbuffer* io_pCmdBuff,
                             v_u32        io_enabled)
{
   return m_pTable->VL_SetUClamp(io_pCmdBuff, io_enabled);
}

vl_error 
RenditionProxy::VL_SetUMask(v_cmdbuffer* io_pCmdBuff,
                            v_u32        io_mask)
{
   return m_pTable->VL_SetUMask(io_pCmdBuff, io_mask);
}

vl_error 
RenditionProxy::VL_SetUMultiplier(v_cmdbuffer* io_pCmdBuff,
                                  v_u32        io_mult)
{
   return m_pTable->VL_SetUMultiplier(io_pCmdBuff, io_mult);
}

vl_error 
RenditionProxy::VL_SetVClamp(v_cmdbuffer* io_pCmdBuff,
                             v_u32        io_enable)
{
   return m_pTable->VL_SetVClamp(io_pCmdBuff, io_enable);
}

vl_error 
RenditionProxy::VL_SetVMask(v_cmdbuffer* io_pCmdBuff,
                            v_u32        io_mask)
{
   return m_pTable->VL_SetVMask(io_pCmdBuff, io_mask);
}

vl_error 
RenditionProxy::VL_SetVMultiplier(v_cmdbuffer* io_pCmdBuff,
                                  v_u32        io_mult)
{
   return m_pTable->VL_SetVMultiplier(io_pCmdBuff, io_mult);
}

vl_error 
RenditionProxy::VL_SetScissorX(v_cmdbuffer* io_pCmdBuff,
                               v_u32        io_scissorX)
{
   return m_pTable->VL_SetScissorX(io_pCmdBuff, io_scissorX);
}

vl_error 
RenditionProxy::VL_SetScissorY(v_cmdbuffer* io_pCmdBuff,
                               v_u32        io_scissorY)
{
   return m_pTable->VL_SetScissorY(io_pCmdBuff, io_scissorY);
}

vl_error 
RenditionProxy::VL_SetSOffset(v_cmdbuffer* io_pCmdBuff,
                              v_u32        io_offset)
{
   return m_pTable->VL_SetSOffset(io_pCmdBuff, io_offset);
}

vl_error 
RenditionProxy::VL_SetTOffset(v_cmdbuffer* io_pCmdBuff,
                              v_u32        io_offset)
{
   return m_pTable->VL_SetTOffset(io_pCmdBuff, io_offset);
}

vl_error 
RenditionProxy::VL_BitfillNoPat(v_cmdbuffer* io_pCmdBuff,
                                v_surface*   io_pVSurface,
                                v_u32        io_buffer,
                                v_u32        io_fgColor,
                                v_u16        io_ulx,
                                v_u16        io_uly,
                                v_u16        io_width,
                                v_u16        io_height)
{
   return m_pTable->VL_BitfillNoPat(io_pCmdBuff, io_pVSurface, io_buffer,
                                    io_fgColor, io_ulx, io_uly, io_width,
                                    io_height);
}

vl_error 
RenditionProxy::VL_Bitblt(v_cmdbuffer* io_pCmdBuff,
                          v_u16        io_dstULx,
                          v_u16        io_dstULy,
                          v_u16        io_width,
                          v_u16        io_height,
                          v_u16        io_srcULx,
                          v_u16        io_srcULy)
{
   return m_pTable->VL_Bitblt(io_pCmdBuff, io_dstULx, io_dstULy, io_width, io_height,
                              io_srcULx, io_srcULy);
}

vl_error 
RenditionProxy::VL_Line(v_cmdbuffer* io_pCmdBuff,
                        v_u32        io_vType,
                        v_u32*       io_pVert0,
                        v_u32*       io_pVert1)
{
   return m_pTable->VL_Line(io_pCmdBuff, io_vType, io_pVert0, io_pVert1);
}

vl_error 
RenditionProxy::VL_Rectangle(v_cmdbuffer* io_pCmdBuff,
                             v_u32        io_vType,
                             v_u32        io_width,
                             v_u32        io_height,
                             v_u32*       io_pVertex)
{
   return m_pTable->VL_Rectangle(io_pCmdBuff, io_vType, io_width, io_height,
                                 io_pVertex);
}

vl_error 
RenditionProxy::VL_MemWriteRect(v_cmdbuffer*     io_pCmdBuffer,
                                v_foreignAddress io_vfDstAddr,
                                v_u32            io_vDstLinebytes,
                                v_memory         io_vMemory,
                                v_u32*           io_dataAddr,
                                v_u32            io_pixelFmt,
                                v_u32            io_dataLinebytes,
                                v_u32            io_width,
                                v_u32            io_height)
{
   return m_pTable->VL_MemWriteRect(io_pCmdBuffer,
                                    io_vfDstAddr, io_vDstLinebytes,
                                    io_vMemory, io_dataAddr, io_pixelFmt,
                                    io_dataLinebytes,
                                    io_width, io_height);
}

vl_error 
RenditionProxy::VL_LoadBuffer(v_cmdbuffer* io_pCmdBuffer,
                              v_surface*   io_pSurface,
                              v_u32        io_bufferNum,
                              v_u32        io_dataLB,
                              v_u32        io_width,
                              v_u32        io_height,
                              v_memory     io_vMemory,
                              v_u32*       io_dataAddr)
{
   return m_pTable->VL_LoadBuffer(io_pCmdBuffer, io_pSurface, io_bufferNum,
                                  io_dataLB, io_width, io_height, io_vMemory,
                                  io_dataAddr);
}


vl_error 
RenditionProxy::VL_Lookup(v_cmdbuffer* io_pCmdBuffer,
                          v_u16        io_ulx,
                          v_u16        io_uly,
                          v_u16        io_width,
                          v_u16        io_height,
                          v_memory     io_memory,
                          v_u32*       io_pPixels)
{
   return m_pTable->VL_Lookup(io_pCmdBuffer,
                              io_ulx, io_uly,
                              io_width, io_height,
                              io_memory, io_pPixels);
}

vl_error 
RenditionProxy::VL_Trifan(v_cmdbuffer* io_pCmdBuffer,
                          v_u32        io_vertType,
                          v_u32        io_vertCount,
                          v_u32*       io_pVerts)
{
   return m_pTable->VL_Trifan(io_pCmdBuffer,
                              io_vertType,
                              io_vertCount,
                              io_pVerts);
}

vl_error 
RenditionProxy::VL_Triangle(v_cmdbuffer* io_pCmdBuffer,
                            v_u32        io_vertType,
                            v_u32*       io_pV0,
                            v_u32*       io_pV1,
                            v_u32*       io_pV2)
{
   return m_pTable->VL_Triangle(io_pCmdBuffer,
                                io_vertType,
                                io_pV0, io_pV1, io_pV2);
}

vl_error 
RenditionProxy::VL_WaitForDisplaySwitch(v_cmdbuffer* io_pCmdBuffer)
{
   return m_pTable->VL_WaitForDisplaySwitch(io_pCmdBuffer);
}

vl_error 
RenditionProxy::VL_MemFillRect(v_cmdbuffer*     io_pCmdBuffer,
                               v_foreignAddress io_boardAddress,
                               v_u32            io_lineBytes,
                               v_u32            io_pixVal,
                               v_u32            io_pixFmt,
                               v_u32            io_width,
                               v_u32            io_height)
{
   return m_pTable->VL_MemFillRect(io_pCmdBuffer, io_boardAddress, io_lineBytes,
                                   io_pixVal, io_pixFmt, io_width, io_height);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
v_error 
RenditionProxy::V_GetErrorText(v_error io_error,
                               LPSTR   out_pString,
                               int     io_bufSize)
{
   return m_pTable->V_GetErrorText(io_error, out_pString, io_bufSize);
}

v_u32* 
RenditionProxy::V_AddToCmdList(v_cmdbuffer* io_pCmdBuffer,
                               v_u32        io_numCmdEntries)
{
   return m_pTable->V_AddToCmdList(io_pCmdBuffer,
                                   io_numCmdEntries);
}

v_u32* 
RenditionProxy::V_AddToDMAList(v_cmdbuffer* io_pCmdBuffer,
                               v_u32        io_fifoPort,
                               v_memory     io_memory,
                               v_u32*       io_vAddr,
                               v_u32        io_sizeWords)
{
   return m_pTable->V_AddToDMAList(io_pCmdBuffer,
                                   io_fifoPort, io_memory,
                                   io_vAddr, io_sizeWords);
}

v_error 
RenditionProxy::V_IssueCmdBuffer(v_handle    io_vHandle,
                                 v_cmdbuffer io_cmdBuffer)
{
   return m_pTable->V_IssueCmdBuffer(io_vHandle, io_cmdBuffer);
}

v_error 
RenditionProxy::V_IssueCmdBufferAsync(v_handle    io_vHandle,
                                      v_cmdbuffer io_cmdBuffer)
{
   return m_pTable->V_IssueCmdBufferAsync(io_vHandle, io_cmdBuffer);
}

v_error 
RenditionProxy::V_GetCmdBufferFreeSpace(v_cmdbuffer io_cmdBuffer,
                                        v_u32*      io_pEntries,
                                        v_u32*      io_pDMAEntries)
{
   return m_pTable->V_GetCmdBufferFreeSpace(io_cmdBuffer,
                                            io_pEntries, io_pDMAEntries);
}

v_u32 
RenditionProxy::V_QueryCmdBuffer(v_handle    io_vHandle,
                                 v_cmdbuffer io_cmdBuffer)
{
   return m_pTable->V_QueryCmdBuffer(io_vHandle, io_cmdBuffer);
}

void 
RenditionProxy::V_SetCmdBufferCallBack(v_cmdbuffer          io_cmdBuffer,
                                       v_cmdbuffer_callback io_pCallback)
{
   m_pTable->V_SetCmdBufferCallBack(io_cmdBuffer, io_pCallback);
}

v_memory 
RenditionProxy::V_AllocLockedMem(v_handle io_vHandle,
                                 v_u32    io_sizeBytes)
{
   return m_pTable->V_AllocLockedMem(io_vHandle, io_sizeBytes);
}

v_error  
RenditionProxy::V_FreeLockedMem(v_handle io_vHandle,
                                v_memory io_memory)
{
   return m_pTable->V_FreeLockedMem(io_vHandle, io_memory);
}

v_memory 
RenditionProxy::V_AllocMemoryObject(v_handle io_vHandle,
                                    v_u32*   io_vAddr,
                                    v_u32    io_sizeBytes)
{
   return m_pTable->V_AllocMemoryObject(io_vHandle, io_vAddr, io_sizeBytes);
}

v_error  
RenditionProxy::V_FreeMemoryObject(v_handle io_vHandle,
                                   v_memory io_memory)
{
   return m_pTable->V_FreeMemoryObject(io_vHandle, io_memory);
}

v_error  
RenditionProxy::V_SetMemoryObject(v_memory io_vHandle,
                                  v_u32*   io_vAddr,
                                  v_u32    io_sizeBytes)
{
   return m_pTable->V_SetMemoryObject(io_vHandle, io_vAddr, io_sizeBytes);
}

v_u32    
RenditionProxy::V_GetMemoryObjectStatus(v_memory io_memory)
{
   return m_pTable->V_GetMemoryObjectStatus(io_memory);
}

v_error  
RenditionProxy::V_LockPages(v_handle io_vHandle,
                            v_memory io_memory)
{
   return m_pTable->V_LockPages(io_vHandle, io_memory);
}

v_error  
RenditionProxy::V_UnlockPages(v_handle io_vHandle,
                              v_memory io_memory)
{
   return m_pTable->V_UnlockPages(io_vHandle, io_memory);
}

v_error 
RenditionProxy::V_BltDisplayBuffer(v_handle        io_vHandle,
                                   v_buffer_group  io_bufGroupDst,
                                   v_u32           io_dstBuffer,
                                   v_rect*         io_pDestRect,
                                   v_buffer_group  io_bufGroupSrc,
                                   v_u32           io_srcBuffer,
                                   v_rect*         io_pSrcRect)
{
   return m_pTable->V_BltDisplayBuffer(io_vHandle,
                                       io_bufGroupDst, io_dstBuffer, io_pDestRect,
                                       io_bufGroupSrc, io_srcBuffer, io_pSrcRect);
}

v_error 
RenditionProxy::V_CreateBufferGroup(v_handle        io_vHandle,
                                    v_buffer_group* out_pBufferGroup,
                                    v_u32*          out_pBufferGrpSize,
                                    v_u32           io_bufferMask,
                                    v_u32           io_numBuffers,
                                    v_u32           io_fmt,
                                    v_u32           io_width,
                                    v_u32           io_height)
{
   return m_pTable->V_CreateBufferGroup(io_vHandle,
                                        out_pBufferGroup, out_pBufferGrpSize,
                                        io_bufferMask, io_numBuffers, io_fmt,
                                        io_width, io_height);
}

v_error 
RenditionProxy::V_DestroyBufferGroup(v_handle       io_vHandle,
                                     v_buffer_group io_trashGroup)
{
   return m_pTable->V_DestroyBufferGroup(io_vHandle, io_trashGroup);
}

v_error 
RenditionProxy::V_RegisterErrorHandler(v_error_handler io_pErrorHandler)
{
   return m_pTable->V_RegisterErrorHandler(io_pErrorHandler);
}

v_error 
RenditionProxy::V_GetFunctionName(v_routine io_routine,
                                  LPSTR     out_pString,
                                  int       io_bufSize)
{
   return m_pTable->V_GetFunctionName(io_routine, out_pString, io_bufSize);
}

v_u32   
RenditionProxy::V_GetUserPrivate(v_handle io_vHandle)
{
   return m_pTable->V_GetUserPrivate(io_vHandle);
}

void    
RenditionProxy::V_SetUserPrivate(v_handle io_vHandle,
                                 v_u32    io_userData)
{
   m_pTable->V_SetUserPrivate(io_vHandle, io_userData);
}

void*   
RenditionProxy::V_LockBuffer(v_handle       io_vHandle,
                             v_buffer_group io_bufferGroup,
                             v_u32          io_lockBuffer)
{
   return m_pTable->V_LockBuffer(io_vHandle, io_bufferGroup, io_lockBuffer);
}

v_error 
RenditionProxy::V_UnlockBuffer(v_handle       io_vHandle,
                               v_buffer_group io_bufferGroup,
                               v_u32          io_unlockBuffer)
{
   return m_pTable->V_UnlockBuffer(io_vHandle, io_bufferGroup, io_unlockBuffer);
}

v_error 
RenditionProxy::V_RestoreBuffer(v_handle       io_vHandle,
                                v_buffer_group io_bufferGroup,
                                v_u32          io_restoreBuffer)
{
   return m_pTable->V_RestoreBuffer(io_vHandle,
                                    io_bufferGroup, io_restoreBuffer);
}

v_foreignAddress 
RenditionProxy::V_GetBufferAddress(v_buffer_group io_bufferGroup,
                                   v_u32          io_bufferNum)
{
   return m_pTable->V_GetBufferAddress(io_bufferGroup, io_bufferNum);
}

v_u32 
RenditionProxy::V_GetBufferLinebytes(v_buffer_group io_bufferGroup,
                                     v_u32          io_bufferNum)
{
   return m_pTable->V_GetBufferLinebytes(io_bufferGroup, io_bufferNum);
}


v_u32* 
RenditionProxy::V_GetMemoryObjectAddress(v_memory io_vMemory)
{
   return m_pTable->V_GetMemoryObjectAddress(io_vMemory);
}


v_error 
RenditionProxy::V_CreateVerite(v_u32     io_board,
                               HWND      io_hWnd,
                               v_handle* out_pVHandle,
                               char*     io_pMicroFile)
{
   return m_pTable->V_CreateVerite(io_board, io_hWnd, out_pVHandle, io_pMicroFile);
}


v_error 
RenditionProxy::V_DestroyVerite(v_handle io_vHandle)
{
   return m_pTable->V_DestroyVerite(io_vHandle);
}

}; // namespace Rendition