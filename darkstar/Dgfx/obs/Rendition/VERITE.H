;/****************************************************************************\ 
;* Copyright 1997 by Rendition, Inc., Mountain View, California
;*
;*      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF RENDITION, INC.
;*      The copyright notice above does not evidence any actual or
;*      intended publication of such source code.
;*
;*                      All Rights Reserved
;*
;* No permission to use, copy, modify, or distribute this software and/or
;* its documentation for any purpose has been granted by Rendition, Inc.
;* If any such permission has been granted ( by separate agreement ), it
;* is required that the above copyright notice appear in all copies and
;* that both that copyright notice and this permission notice appear in
;* supporting documentation, and that the name of Rendition, Inc. or any
;* of its trademarks may not be used in advertising or publicity pertaining
;* to distribution of the software without specific, written prior permission.
;*
;* RENDITION, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
;* SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
;* FITNESS. IN NO EVENT SHALL RENDITION CORPORATION BE LIABLE FOR ANY
;* SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
;* RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
;* CONTRACT, NEGLIGENCE OR OTHER TORTUOUS ACTION, ARISING OUT OF OR IN
;* CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
;****************************************************************************/
#ifndef VERITE_H
#define VERITE_H

#include "envdep.h"
#include "veriterc.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/
/*                                Types                                 */
/************************************************************************/

typedef int v_error;
typedef int v_routine;

typedef struct _v_memory *v_memory;

typedef struct _v_verite *v_handle;

typedef struct _v_cmdbuffer *v_cmdbuffer;

typedef struct _v_buffer_group *v_buffer_group;

typedef struct _v_rect {
    long	left;
    long	top;
    long	right;
    long	bottom;
} v_rect;

typedef struct _v_verite_info {
    unsigned long	veriteID;
    unsigned long	memMappedIoBase;
    unsigned long	memBase;
    unsigned long	memSize;
    unsigned short	ioBase;
    unsigned char	busMaster;
    unsigned char	vgaDevice;
} v_verite_info;

typedef v_cmdbuffer (V_CDECL *v_cmdbuffer_callback)(v_handle vhandle, v_cmdbuffer cmdbuffer);

/*
 * Error handling routine
 */
typedef void (V_CDECL *v_error_handler)(v_handle vhandle, v_routine routine, v_error err, int exterr);


/* Note: all structures have v_u32 components only; we assume therefore that
   alignment will not be affected by compiler structure packing options */

/*
 * Surface defines and structure definition
 */
#define V_PIXEL_SIZE(ptype)                              \
  (((ptype) == V_PIXFMT_332) ? (1) : (                   \
   ((ptype) == V_PIXFMT_8I) ? (1) : (                    \
   ((ptype) == V_PIXFMT_8A) ? (1) : (                    \
   ((ptype) == V_PIXFMT_565) ? (2) : (                   \
   ((ptype) == V_PIXFMT_4444) ? (2) : (                  \
   ((ptype) == V_PIXFMT_1555) ? (2) : (                  \
   ((ptype) == V_PIXFMT_Y0CBY1CR) ? (2) : (              \
   ((ptype) == V_PIXFMT_8888) ? (4) : (0)))))))))

typedef enum _v_buffer_mask {
	V_SURFACE_Z_BUFFER = 1,
	V_SURFACE_PRIMARY = 2,
        V_SURFACE_INTERLEAVED = 4
} v_buffer_mask;

typedef enum {
    V_WINDOWED_APP=0x1,
    V_FULLSCREEN_APP=0x2
} v_AppType;

typedef enum {
    V_WORD_FIFO_PORT = 0,
    V_BYTE_FIFO_PORT = 1,
    V_OTHER_FIFO_PORT = 2,
    V_SHORT_FIFO_PORT = 3,
} v_byte_swap;

/* For historical reasons, we don't have separate SrcFmt, DstFmt,
   and ScrnFmt. Note that only some PixFmt's are allowed as
   DstFmt or ScrnFmt. */
typedef enum {
  /* unused values 0..15 are RESERVED */
  V_PIXFMT_DSTFMT = 0,
  V_PIXFMT_332 = 1,
  V_PIXFMT_8I = 2,
  V_PIXFMT_8A = 3,
  V_PIXFMT_565 = 4,
  V_PIXFMT_4444 = 5,
  V_PIXFMT_1555 = 6,
  V_PIXFMT_4I_565 = 8,
  V_PIXFMT_4I_4444 = 9,
  V_PIXFMT_4I_1555 = 10,
  V_PIXFMT_8888 = 12,
  V_PIXFMT_Y0CRY1CB = 13
} v_PixFmt;

/* for backwards compatibility define 233 same as 332 */
#define V_PIXFMT_233 V_PIXFMT_332
#define V_PIXFMT_Y0CBY1CR V_PIXFMT_Y0CRY1CB
typedef v_PixFmt v_SrcFmt;
typedef v_PixFmt v_DstFmt;
typedef v_PixFmt v_ScrnFmt;

/*
 * Function prototypes
 */
extern v_error V_DLLIMPORT V_CreateVerite(v_u32 board, HWND hwnd, v_handle *pvhandle, char* microcodefile);

extern v_error V_DLLIMPORT V_DestroyVerite(v_handle vhandle);

extern v_error V_DLLIMPORT V_EnumVeriteBoards(HWND hwnd, v_u32 board, v_verite_info *veriteInfo);

extern v_u32 V_DLLIMPORT V_GetUseDMA(v_handle vhandle);


extern void V_DLLIMPORT V_SetUserPrivate(v_handle vhandle, v_u32 priv);

extern v_u32 V_DLLIMPORT V_GetUserPrivate(v_handle vhandle);


extern v_u32 V_DLLIMPORT V_Stride(v_handle vhandle, v_u32 linebytes);

extern v_u32 V_DLLIMPORT V_Linebytes(v_handle vhandle, v_u32 stride);


extern v_error V_DLLEXPORT V_RegisterSyncCmd(v_handle verite, v_u32 sync_cmd);

/*
 * Memory functions for DMA
 */
extern v_memory V_DLLIMPORT V_AllocMemoryObject(v_handle vhandle, v_u32 *v_addr, v_u32 size_bytes);

extern v_error V_DLLIMPORT V_SetMemoryObject(v_memory memory, v_u32 *v_addr, v_u32 size_bytes);

extern v_u32 * V_DLLIMPORT V_GetMemoryObjectAddress(v_memory memory);

extern v_u32 V_DLLIMPORT V_GetMemoryObjectSize(v_memory memory);

extern v_u32 V_DLLIMPORT V_GetMemoryObjectStatus(v_memory memory);

extern void V_DLLIMPORT V_FreeMemoryObject(v_handle vhandle, v_memory memory);

extern v_memory V_DLLIMPORT V_AllocLockedMem(v_handle vhandle, v_u32 size_bytes);

extern v_error V_DLLIMPORT V_FreeLockedMem(v_handle vhandle, v_memory memory);

extern v_error V_DLLIMPORT V_LockPages(v_handle vhandle, v_memory memory);

extern v_error V_DLLIMPORT V_UnlockPages(v_handle vhandle, v_memory memory);

/*
 * Cmdbuffer function prototypes
 */
extern v_cmdbuffer V_DLLIMPORT V_CreateCmdBuffer(v_handle vhandle, int dmalist_entries, int cmdlist_size);

extern v_error V_DLLIMPORT V_IssueCmdBuffer(v_handle vhandle, v_cmdbuffer cmdbuffer);

extern v_error V_DLLIMPORT V_IssueCmdBufferAsync(v_handle vhandle, v_cmdbuffer cmdbuffer);

extern v_u32 V_DLLIMPORT V_QueryCmdBuffer(v_handle vhandle, v_cmdbuffer cmdbuffer);

extern v_error V_DLLIMPORT V_DestroyCmdBuffer(v_cmdbuffer cmdbuffer);

extern void V_DLLIMPORT V_ResetCmdBuffer(v_cmdbuffer cmdbuffer);

extern void V_DLLIMPORT V_SetCmdBufferCallBack(v_cmdbuffer cmdbuffer, v_cmdbuffer_callback pfn);

extern v_cmdbuffer_callback V_DLLIMPORT V_GetCmdBufferCallBack(v_cmdbuffer cmdbuffer);

extern v_handle V_DLLIMPORT V_GetCmdBufferHandle(v_cmdbuffer cmdbuffer);

extern v_error V_DLLIMPORT V_GetCmdBufferFreeSpace(v_cmdbuffer cmdbuffer, v_u32 *pentries, v_u32 *pdma_entries);

/*
 * Functions for adding commands and DMA entries
 */
extern v_u32 V_DLLIMPORT V_GetDMAEntries(v_memory memory, v_u32 *v_addr, v_u32 size_words);

extern v_u32 * V_DLLIMPORT V_AddToCmdList(v_cmdbuffer *cmdbuffer, v_u32 num_cmd_entries);

extern v_u32 * V_DLLIMPORT V_AddToCmdAndDMAList(v_cmdbuffer *cmdbuffer, v_u32 num_cmd_entries,
				v_u32 num_dma_entries);

extern v_error V_DLLIMPORT V_AddToDMAList(v_cmdbuffer *cmdbuffer, v_u32 fifo_port,
				v_memory memory, v_u32 *v_addr, v_u32 size_words);

/*
 * Display state utilities 
 */
extern v_error V_DLLIMPORT V_SetDisplayType(v_handle vhandle, v_u32 appType);

extern v_u32 V_DLLIMPORT V_GetDisplayType(v_handle vhandle);

extern v_error V_DLLIMPORT V_SetDisplayMode(v_handle vhandle,
				v_u32 width, v_u32 height, v_u32 bpp, 
				v_u32 refresh_rate);

extern v_error V_DLLIMPORT V_SetDisplayLinebytes(v_handle vhandle, v_u32 linebytes);

/*
 *  Buffer management functions
 */
extern v_error V_DLLIMPORT V_CreateBufferGroup(v_handle vhandle, 
				   v_buffer_group *pbuffer_group, 
				   v_u32 *pmemsize,
				   v_u32 buffer_mask,
				   v_u32 num_buffers,
				   v_u32 pixel_fmt,
				   v_u32 width,
				   v_u32 height);

extern v_error V_DLLIMPORT V_PointBufferGroup(v_handle vhandle, 
				  v_buffer_group *pbuffer_group, 
				  v_u32 *pmemsize,
				  v_u32 buffer_mask, 
				  v_u32 num_buffers,
				  v_u32 pixel_fmt, 
				  v_u32 width, 
				  v_u32 height,
				  v_foreignAddress addr);

extern v_error V_DLLIMPORT V_DestroyBufferGroup(v_handle vhandle, v_buffer_group buffer_group);
	
extern v_error V_DLLIMPORT V_BltDisplayBuffer(v_handle vhandle, 
				    v_buffer_group dstgroup, v_u32 dstbuffer, v_rect *dstrect,
				    v_buffer_group srcgroup, v_u32 srcbuffer, v_rect *srcrect);

extern v_error V_DLLIMPORT V_ReadBuffer(v_handle vhandle, v_buffer_group buffer_group,
			       v_u32 buffer,
			       v_u32 xorg,
			       v_u32 yorg,
			       v_u32 width,
			       v_u32 height,
			       v_u32 data_linebytes,
			       v_u32 *data);

extern void * V_DLLIMPORT V_LockBuffer(v_handle vhandle, v_buffer_group buffer_group, v_u32 buffer);

extern v_error V_DLLIMPORT V_UnlockBuffer(v_handle vhandle, v_buffer_group buffer_group, v_u32 buffer);

extern v_u32 V_DLLIMPORT V_IsBufferLost(v_handle vhandle, v_buffer_group buffer_group, v_u32 buffer);

extern v_error V_DLLIMPORT V_RestoreBuffer(v_handle vhandle, v_buffer_group buffer_group, v_u32 buffer);

extern v_u32 V_DLLIMPORT V_GetBufferLinebytes(v_buffer_group buffer_group, v_u32 buffer);

extern v_u32 V_DLLIMPORT V_GetBufferStride(v_buffer_group buffer_group, v_u32 buffer);

extern v_foreignAddress V_DLLIMPORT V_GetBufferAddress(v_buffer_group buffer_group, v_u32 buffer);

extern v_error V_DLLIMPORT V_SwapBuffers(v_buffer_group buffer_group, v_u32 buffer0, v_u32 buffer1);

extern v_error V_DLLIMPORT V_GetBufferDC(v_buffer_group buffer_group, v_u32 buffer, HDC *pDC);

extern v_error V_DLLIMPORT V_ReleaseBufferDC(v_buffer_group buffer_group, v_u32 buffer, HDC hdc);

extern v_u32 V_DLLIMPORT V_GetNumBuffers(v_buffer_group buffer_group);

extern v_u32 V_DLLIMPORT V_GetBufferMask(v_buffer_group buffer_group);

extern v_error V_DLLIMPORT V_GetBoardAddress(v_handle verite, v_u32 *vaddr, v_foreignAddress *board_addr);

/*
 *  Error Handling functions 
 */
extern v_error V_DLLIMPORT V_RegisterErrorHandler(v_error_handler peh);

extern v_error_handler V_DLLIMPORT V_GetErrorHandler(void);

extern v_error V_DLLIMPORT V_GetFunctionName(v_routine routine, LPSTR s, int nbuff);

extern v_error V_DLLIMPORT V_GetErrorText(v_error err, LPSTR s, int nbuff);

extern v_error V_DLLIMPORT V_GetLastError(v_routine *proutine, v_error *perr, int *pexterr);

#ifdef __cplusplus
}
#endif

#endif /*VERITE_H*/
