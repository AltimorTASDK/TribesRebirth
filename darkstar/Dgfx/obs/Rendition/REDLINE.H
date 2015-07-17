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
#ifndef REDLINE_H
#define REDLINE_H

#include "verite.h"
#include "rlinerc.h"
#include "rlfifo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int vl_error;
typedef int vl_routine;

typedef struct _v_surface {
    v_u32	    width;			/* width in pixels*/
    v_u32	    height;			/* height in pixels*/
    v_u32	    pixel_fmt;
    v_u32	    chroma_color;	/* chromakey, hw format */
    v_u32	    chroma_mask;	/* chromakey mask, hw format */
    v_u32	    chromakey;		/* boolean enable flag */
    v_u32	    color_pad;		/* boolean enable flag */
    v_u32	    bgr;			/* boolean enable flag */
    v_u32	    clamp;			/* boolean mask: b0=uclamp, b1=vclamp */
    v_u32	    *palette;		/* 4-bit texture palette */
    v_u32	    start_index;	/* first used index in palette */
    v_u32	    num_entries;	/* number of index entries used */
    v_u32	    memsize;		/* Total memory allocated by all buffers */
    v_u32	    num_buffers;	/* Number of buffers */
    v_u32	    buffer_mask;	/* Flags used to create surface */
    v_buffer_group  buffer_group;  /* A group of buffers */
} v_surface;

#define V_SURFACE_UCLAMP 1
#define V_SURFACE_VCLAMP 2

typedef enum {
    V_SRCFUNC_NOTEXTURE=0,    /* SrcColor=RGB; SrcA=A */
    V_SRCFUNC_REPLACE=1,      /* SrcColor=TexRGB; SrcA=TexA */
    V_SRCFUNC_DECAL=2,        /* SrcColor=(1-TexA)RGB + TexA*TexRGB, SrcA=A */
    V_SRCFUNC_MODULATE=3      /* SrcColorA=TexRGBA*RGBA */
} v_SrcFunc;

typedef enum {
    V_SRCFILTER_POINT=0,
    V_SRCFILTER_BILINEAR=1
} v_SrcFilter;

typedef enum {
    V_SRCCOLORPAD_ENABLE=0,
    V_SRCCOLORPAD_DISABLE=1
} v_SrcColorNoPad;

typedef enum {
    V_SWAPUV_ENABLE=1,
    V_SWAPUV_DISABLE=0
} v_SwapUV;

typedef enum {
    V_CHROMAKEY_ENABLE=1,
    V_CHROMAKEY_DISABLE=0
} v_ChromaKey;

typedef enum {
    V_ZSCISSOR_ENABLE=1,
    V_ZSCISSOR_DISABLE=0
} v_ZScissorEn;

typedef enum {
    V_SRCBGR_ENABLE=1,
    V_SRCBGR_DISABLE=0
} v_SrcBGR;

typedef enum {
    V_UCLAMP_ENABLE=1,
    V_UCLAMP_DISABLE=0
} v_UClamp;

typedef enum {
    V_VCLAMP_ENABLE=1,
    V_VCLAMP_DISABLE=0
} v_VClamp;

typedef enum {
    V_ALUMODE_ZERO            = 0x0,
    V_ALUMODE_SRC_NOR_DST     = 0x1,
    V_ALUMODE_NOT_SRC_AND_DST = 0x2,
    V_ALUMODE_NOT_SRC         = 0x3,
    V_ALUMODE_SRC_AND_NOT_DST = 0x4,
    V_ALUMODE_NOT_DST         = 0x5,
    V_ALUMODE_SRC_XOR_DST     = 0x6,
    V_ALUMODE_SRC_NAND_DST    = 0x7,
    V_ALUMODE_SRC_AND_DST     = 0x8,
    V_ALUMODE_SRC_XNOR_DST    = 0x9,
    V_ALUMODE_DST             = 0xA,
    V_ALUMODE_NOT_SRC_OR_DST  = 0xB,
    V_ALUMODE_SRC             = 0xC,
    V_ALUMODE_SRC_OR_NOT_DST  = 0xD,
    V_ALUMODE_SRC_OR_DST      = 0xE,
    V_ALUMODE_ONE             = 0xF
} v_ALUMode;

typedef enum {
    V_BLENDSRCCOLOR=0,
    V_BLENDSRCCOLORINV=1,
#define V_BLENDDSTCOLOR V_BLENDSRCCOLOR
#define V_BLENDDSTCOLORINV V_BLENDSRCCOLORINV
    V_BLENDSRCALPHA=2,
    V_BLENDSRCALPHAINV=3,
    V_BLENDDSTALPHA=4,
    V_BLENDDSTALPHAINV=5,
    V_BLEND0=6,
    V_BLEND1=7,
    V_BLENDSRCALPHASAT=8,
    V_BLENDSRCALPHASATINV=9,
} v_BlendFunc;

typedef enum {
    V_ZBUFMODE_ALWAYS=0,
    V_ZBUFMODE_LT=1,
    V_ZBUFMODE_EQ=2,
    V_ZBUFMODE_LE=3,
    V_ZBUFMODE_GT=4,
    V_ZBUFMODE_NE=5,
    V_ZBUFMODE_GE=6,
} v_ZBufMode;

typedef enum {
    V_ZBUFWRMODE_DISABLE = 0,
    V_ZBUFWRMODE_ENABLE = 1
} v_ZBufWrMode;

typedef enum {
    V_YUV2RGB_ENABLE=1,
    V_YUV2RGB_DISABLE=0
} v_YUV2RGB;

typedef enum {
    V_BLEND_ENABLE=1,
    V_BLEND_DISABLE=0
} v_BlendEnable;

typedef enum {
    V_DITHER_ENABLE=1,
    V_DITHER_DISABLE=0
} v_DitherEnable;

typedef enum {
    V_FOG_ENABLE=1,
    V_FOG_DISABLE=0
} v_FogEnable;

typedef enum {
    V_DSTRD_ENABLE=0,
    V_DSTRD_DISABLE=1       /* this is correct */
} v_DstRdDisable;

typedef enum {
    V_DSTBGR_ENABLE=1,
    V_DSTBGR_DISABLE=0
} v_DstBGR;

typedef enum {
    V_TRANSPREJECT_ENABLE=1,
    V_TRANSPREJECT_DISABLE=0
} v_TranspReject;

typedef enum {
  V_PAT_ENABLE = 1,
  V_PAT_DISABLE = 0
} v_PatEnable;

typedef enum {
  V_PATOPAQUE_ENABLE = 1,
  V_PATOPAQUE_DISABLE = 0
} v_PatOpaque;

/*
 * Error handling routine
 */
typedef void (V_CDECL *vl_error_handler)(v_handle vhandle, vl_routine routine, vl_error err, int exterr);

/* Note: arg declarations to helper functions matches FIFO format
   (with an occassional pointer to variable-length data) */

/*
 * Initialization of redline
 */
extern vl_error V_DLLIMPORT VL_OpenVerite(HWND hwnd, v_handle *pvhandle);
extern vl_error V_DLLIMPORT VL_CloseVerite(v_handle vhandle);

/*
 * Memory management function prototypes
 */
extern vl_error V_DLLIMPORT VL_MemWrite(v_cmdbuffer *cmdbuffer,
			    v_foreignAddress dest_addr,
			    v_memory data_memory,
			    v_u32 *data_addr,
			    v_u32 pixel_fmt,
			    v_u32 data_width);

extern vl_error V_DLLIMPORT VL_MemWriteRect(v_cmdbuffer *cmdbuffer,
				v_foreignAddress dest_addr,
				v_u32 dest_linebytes,
				v_memory data_memory,
				v_u32 *data_addr,
				v_u32 pixel_fmt,
				v_u32 data_linebytes,
				v_u32 data_width,
				v_u32 data_height);

extern vl_error V_DLLIMPORT VL_MemFill(v_cmdbuffer *cmdbuffer,
			    v_foreignAddress dest_addr,
			    v_u32 pixel_value,
			    v_u32 pixel_fmt,
			    v_u32 width);

extern vl_error V_DLLIMPORT VL_MemFillRect(v_cmdbuffer *cmdbuffer,
				v_foreignAddress dest_addr,
				v_u32 dest_linebytes,
				v_u32 pixel_value,
				v_u32 pixel_fmt,
				v_u32 width,
				v_u32 height);

/*
 * Display management function prototypes
 */
extern vl_error V_DLLIMPORT VL_SetDisplayBase(v_cmdbuffer *cmdbuffer, v_foreignAddress disp_base);

extern vl_error V_DLLIMPORT VL_SetStereoDisplayBase(v_cmdbuffer *cmdbuffer,
				   v_foreignAddress disp_even_base,
				   v_foreignAddress disp_odd_base);

extern vl_error V_DLLIMPORT VL_SetPalette(v_cmdbuffer *cmdbuffer,
			 v_u16 start_index, v_u16 num_entries, v_u32 *entries);

extern vl_error V_DLLIMPORT VL_WaitForDisplaySwitch(v_cmdbuffer *cmdbuffer);

/*
 * Surface management routine prototypes
 */
extern vl_error V_DLLIMPORT VL_CreateSurface(v_handle vhandle, 
                                 v_surface **psurface, 
                                 v_u32 buffer_mask, 
                                 v_u32 num_buffers,
                                 v_u32 pixel_fmt, 
                                 v_u32 width, 
                                 v_u32 height);

extern vl_error V_DLLIMPORT VL_PointSurface(v_handle vhandle, 
                               v_surface **psurface, 
                               v_u32 buffer_mask, 
                               v_u32 num_buffers,
                               v_u32 pixel_fmt, 
                               v_u32 width, 
                               v_u32 height,
                               v_foreignAddress addr);

extern vl_error V_DLLIMPORT VL_DestroySurface(v_handle vhandle, v_surface *surface);

extern vl_error V_DLLIMPORT VL_RestoreSurface(v_handle vhandle, v_surface *surface);

extern vl_error V_DLLIMPORT VL_SetSurfaceChromaKey(v_surface *surface, v_u32 value);

extern vl_error V_DLLIMPORT VL_SetSurfaceChromaColor(v_surface *surface, v_u32 value);

extern vl_error V_DLLIMPORT VL_SetSurfaceChromaMask(v_surface *surface, v_u32 value);

extern vl_error V_DLLIMPORT VL_SetSurfaceColorPad(v_surface *surface, v_u32 value);

extern vl_error V_DLLIMPORT VL_SetSurfaceBGR(v_surface *surface, v_u32 value);

extern vl_error V_DLLIMPORT VL_SetSurfaceClamp(v_surface *surface, v_u32 uclamp, v_u32 vclamp);

extern vl_error V_DLLIMPORT VL_SetSurfaceTexturePalette(v_surface *surface, int startEntry, int numEntries, v_u32 *palette);
	
extern vl_error V_DLLIMPORT VL_SwapDisplaySurface(v_cmdbuffer *cmdbuffer, v_surface *surface);

extern vl_error V_DLLIMPORT VL_FillBuffer(v_cmdbuffer *cmdbuffer,
			  v_surface *surface,
			  v_u32 buffer,
			  v_u32 xorg,
			  v_u32 yorg,
			  v_u32 width,
			  v_u32 height,
			  v_u32 pixel_value);

extern vl_error V_DLLIMPORT VL_LoadBuffer(v_cmdbuffer *cmdbuffer,
			      v_surface *surface,
			      v_u32 buffer,
			      v_u32 data_linebytes,
			      v_u32 data_width,
			      v_u32 data_height,
			      v_memory data_memory, v_u32 *data_addr);

extern vl_error V_DLLIMPORT VL_InstallDstBuffer(v_cmdbuffer *cmdbuffer, v_surface *surface);

extern vl_error V_DLLIMPORT VL_InstallTextureMap(v_cmdbuffer *cmdbuffer, v_surface *surface);

extern vl_error V_DLLIMPORT VL_InstallTextureMapBasic(v_cmdbuffer *cmdbuffer, v_surface *surface);

extern vl_error V_DLLIMPORT VL_InstallZBuffer(v_cmdbuffer *cmdbuffer, v_surface *surface);

extern vl_error V_DLLIMPORT VL_CopyBuffer(v_cmdbuffer *cmdbuffer,
			  v_surface *surface,
			  v_u32 buffer,
			  v_u32 alu_mode,
			  v_u16 dst_ulx, v_u16 dst_uly,
			  v_u16 width, v_u16 height,
			  v_u16 src_ulx, v_u16 src_uly);

extern vl_error V_DLLIMPORT VL_BitfillNoPat(v_cmdbuffer *cmdbuffer,
			   v_surface *surface,
			   v_u32 buffer,
			   v_u32 fgcolorrgb,
			   v_u16 ulx, v_u16 uly,
			   v_u16 width, v_u16 height);

extern vl_error V_DLLIMPORT VL_BitfillPatTransparent(v_cmdbuffer *cmdbuffer,
				    v_surface *surface,
				    v_u32 buffer,
				    v_u32 fgcolorrgb,
				    v_u32 pat,
				    v_u32 pat_len,
				    v_u32 pat_offset,
				    v_u16 ulx, v_u16 uly,
				    v_u16 width, v_u16 height);

extern vl_error V_DLLIMPORT VL_BitfillPatOpaque( v_cmdbuffer *cmdbuffer,
				v_surface *surface,
				v_u32 buffer,
				v_u32 fgcolorrgb,
				v_u32 bgcolorrgb,
				v_u32 pat,
				v_u32 pat_len,
				v_u32 pat_offset,
				v_u16 ulx, v_u16 uly,
				v_u16 width, v_u16 height);

/*
 * Pixel Engine State management function prototypes
 */
extern vl_error V_DLLIMPORT VL_ContextInit(v_cmdbuffer *cmdbuffer);

extern vl_error V_DLLIMPORT VL_SetSrcBase(v_cmdbuffer *cmdbuffer, v_u32 src_base);

extern vl_error V_DLLIMPORT VL_SetSrcFmt(v_cmdbuffer *cmdbuffer, v_u32 src_fmt);

extern vl_error V_DLLIMPORT VL_SetSrcFunc(v_cmdbuffer *cmdbuffer, v_u32 src_func);

extern vl_error V_DLLIMPORT VL_SetSrcFilter(v_cmdbuffer *cmdbuffer, v_u32 src_filter);

extern vl_error V_DLLIMPORT VL_SetSrcColorNoPad(v_cmdbuffer *cmdbuffer, v_u32 src_color_no_pad);

extern vl_error V_DLLIMPORT VL_SetSwapUV(v_cmdbuffer *cmdbuffer, v_u32 swap_uv);

extern vl_error V_DLLIMPORT VL_SetChromaKey(v_cmdbuffer *cmdbuffer, v_u32 chroma_key);

extern vl_error V_DLLIMPORT VL_SetZScissorEnable(v_cmdbuffer *cmdbuffer, v_u32 z_scissor_enable);

extern vl_error V_DLLIMPORT VL_SetSrcBGR(v_cmdbuffer *cmdbuffer, v_u32 src_bgr);

extern vl_error V_DLLIMPORT VL_SetUClamp(v_cmdbuffer *cmdbuffer, v_u32 u_clamp);

extern vl_error V_DLLIMPORT VL_SetVClamp(v_cmdbuffer *cmdbuffer, v_u32 v_clamp);

extern vl_error V_DLLIMPORT VL_SetUMask(v_cmdbuffer *cmdbuffer, v_u32 u_mask);

extern vl_error V_DLLIMPORT VL_SetVMask(v_cmdbuffer *cmdbuffer, v_u32 v_mask);

extern vl_error V_DLLIMPORT VL_SetUMultiplier(v_cmdbuffer *cmdbuffer, v_u32 u_mult);

extern vl_error V_DLLIMPORT VL_SetVMultiplier(v_cmdbuffer *cmdbuffer, v_u32 v_mult);

extern vl_error V_DLLIMPORT VL_SetSOffset(v_cmdbuffer *cmdbuffer, v_u32 s_offset);

extern vl_error V_DLLIMPORT VL_SetTOffset(v_cmdbuffer *cmdbuffer, v_u32 t_offset);

extern vl_error V_DLLIMPORT VL_SetSrcStride(v_cmdbuffer *cmdbuffer, v_u32 src_stride);

extern vl_error V_DLLIMPORT VL_SetDstStride(v_cmdbuffer *cmdbuffer, v_u32 dst_stride);

extern vl_error V_DLLIMPORT VL_SetZStride(v_cmdbuffer *cmdbuffer, v_u32 z_stride);

extern vl_error V_DLLIMPORT VL_SetDstBase(v_cmdbuffer *cmdbuffer, v_u32 dst_base);

extern vl_error V_DLLIMPORT VL_SetALUMode(v_cmdbuffer *cmdbuffer, v_u32 alu_mode);

extern vl_error V_DLLIMPORT VL_SetBlendSrcFunc(v_cmdbuffer *cmdbuffer, v_u32 blend_src_func);

extern vl_error V_DLLIMPORT VL_SetBlendDstFunc(v_cmdbuffer *cmdbuffer, v_u32 blend_dst_func);

extern vl_error V_DLLIMPORT VL_SetZBufMode(v_cmdbuffer *cmdbuffer, v_u32 z_buf_mode);

extern vl_error V_DLLIMPORT VL_SetZBufWrMode(v_cmdbuffer *cmdbuffer, v_u32 z_buf_wr_mode);

extern vl_error V_DLLIMPORT VL_SetYUV2RGB(v_cmdbuffer *cmdbuffer, v_u32 yuv_2_rgb);

extern vl_error V_DLLIMPORT VL_SetBlendEnable(v_cmdbuffer *cmdbuffer, v_u32 blend_enable);

extern vl_error V_DLLIMPORT VL_SetDitherEnable(v_cmdbuffer *cmdbuffer, v_u32 dither_enable);

extern vl_error V_DLLIMPORT VL_SetFogEnable(v_cmdbuffer *cmdbuffer, v_u32 fog_enable);

extern vl_error V_DLLIMPORT VL_SetDstRdDisable(v_cmdbuffer *cmdbuffer, v_u32 dst_read_disable);

extern vl_error V_DLLIMPORT VL_SetDstBGR(v_cmdbuffer *cmdbuffer, v_u32 dst_bgr);

extern vl_error V_DLLIMPORT VL_SetTranspReject(v_cmdbuffer *cmdbuffer, v_u32 transp_reject);

extern vl_error V_DLLIMPORT VL_SetAlphaThreshold(v_cmdbuffer *cmdbuffer, v_u8 alpha_threshold);

extern vl_error V_DLLIMPORT VL_SetDstFmt(v_cmdbuffer *cmdbuffer, v_u32 dst_fmt);

extern vl_error V_DLLIMPORT VL_SetPMask(v_cmdbuffer *cmdbuffer, v_u32 p_mask);

extern vl_error V_DLLIMPORT VL_SetPat(v_cmdbuffer *cmdbuffer, v_u32 pat);

extern vl_error V_DLLIMPORT VL_SetPatLength(v_cmdbuffer *cmdbuffer, v_u32 pat_len);

extern vl_error V_DLLIMPORT VL_SetPatEnable(v_cmdbuffer *cmdbuffer, v_u32 pat_enable);

extern vl_error V_DLLIMPORT VL_SetPatOpaque(v_cmdbuffer *cmdbuffer, v_u32 pat_opaque);

extern vl_error V_DLLIMPORT VL_SetPatOffset(v_cmdbuffer *cmdbuffer, v_u32 pat_offset);

extern vl_error V_DLLIMPORT VL_SetScissorX(v_cmdbuffer *cmdbuffer, v_u32 scissor_x);

extern vl_error V_DLLIMPORT VL_SetScissorY(v_cmdbuffer *cmdbuffer, v_u32 scissor_y);

extern vl_error V_DLLIMPORT VL_SetZBase(v_cmdbuffer *cmdbuffer, v_u32 z_base);

extern vl_error V_DLLIMPORT VL_SetDitherOffset(v_cmdbuffer *cmdbuffer, v_u32 dither_offset);

extern vl_error V_DLLIMPORT VL_SetFGColorABGR(v_cmdbuffer *cmdbuffer, v_u32 fg_color);

extern vl_error V_DLLIMPORT VL_SetFGColorARGB(v_cmdbuffer *cmdbuffer, v_u32 fg_color);

extern vl_error V_DLLIMPORT VL_SetBGColorABGR(v_cmdbuffer *cmdbuffer, v_u32 bg_color);

extern vl_error V_DLLIMPORT VL_SetBGColorARGB(v_cmdbuffer *cmdbuffer, v_u32 bg_color);

extern vl_error V_DLLIMPORT VL_SetFogColorBGR(v_cmdbuffer *cmdbuffer, v_u32 fog_color);

extern vl_error V_DLLIMPORT VL_SetFogColorRGB(v_cmdbuffer *cmdbuffer, v_u32 fog_color);

extern vl_error V_DLLIMPORT VL_SetDstColorABGR(v_cmdbuffer *cmdbuffer, v_u32 dst_color);

extern vl_error V_DLLIMPORT VL_SetDstColorARGB(v_cmdbuffer *cmdbuffer, v_u32 dst_color);

extern vl_error V_DLLIMPORT VL_SetChromaColor(v_cmdbuffer *cmdbuffer, v_u32 chroma_color, v_u32 pixel_fmt);

extern vl_error V_DLLIMPORT VL_SetChromaMask(v_cmdbuffer *cmdbuffer, v_u32 chroma_mask, v_u32 pixel_fmt);

extern vl_error V_DLLIMPORT VL_SetCurrentTexture(v_cmdbuffer *cmdbuffer,
				v_u32 src_base, v_u32 src_stride,
				v_u16 v_mask, v_u16 u_mask,
				v_u32 u_mult, v_u32 v_mult);

extern vl_error V_DLLIMPORT VL_SetZ(v_cmdbuffer *cmdbuffer, v_u32 z);

extern vl_error V_DLLIMPORT VL_SetR(v_cmdbuffer *cmdbuffer, v_u32 r);

extern vl_error V_DLLIMPORT VL_SetG(v_cmdbuffer *cmdbuffer, v_u32 g);

extern vl_error V_DLLIMPORT VL_SetB(v_cmdbuffer *cmdbuffer, v_u32 b);

extern vl_error V_DLLIMPORT VL_SetA(v_cmdbuffer *cmdbuffer, v_u32 a);

extern vl_error V_DLLIMPORT VL_SetF(v_cmdbuffer *cmdbuffer, v_u32 f);

extern vl_error V_DLLIMPORT VL_SetU(v_cmdbuffer *cmdbuffer, v_u32 u);

extern vl_error V_DLLIMPORT VL_SetV(v_cmdbuffer *cmdbuffer, v_u32 v);

extern vl_error V_DLLIMPORT VL_SetTexturePalette(v_cmdbuffer *cmdbuffer, v_u32 startEntry, v_u32 numEntries, 
	v_u32 *palette, v_u32 pixel_fmt);

/*
 * Rendering routine prototypes
 */
extern v_u32 V_DLLIMPORT VL_VertexSize(v_u32 v_type);

extern vl_error V_DLLIMPORT VL_QOffset(v_u32 v_type);

extern void V_DLLIMPORT VL_QNormalize(v_u32 *q0, v_u32 *q1, v_u32 *q2);

extern vl_error V_DLLIMPORT VL_Dot(v_cmdbuffer *cmdbuffer, v_u32 v_type, v_u32 *vert);

extern vl_error V_DLLIMPORT VL_Line(v_cmdbuffer *cmdbuffer, v_u32 v_type, v_u32 *v1, v_u32 *v2);

extern vl_error V_DLLIMPORT VL_Polyline(v_cmdbuffer *cmdbuffer, v_u32 v_type, v_u32 seg_count, v_u32 *verts);

extern vl_error V_DLLIMPORT VL_IntLine(v_cmdbuffer *cmdbuffer, v_u32 last_pixel_enable, v_u16 x0, v_u16 y0, v_u16 x1, v_u16 y1);

extern vl_error V_DLLIMPORT VL_IntPolyline(v_cmdbuffer *cmdbuffer, v_u32 last_pixel_enable, v_u32 seg_count, v_u32 *verts);

extern vl_error V_DLLIMPORT VL_AALine(v_cmdbuffer *cmdbuffer, v_u32 v_type, v_u32 *v1, v_u32 *v2);

extern vl_error V_DLLIMPORT VL_AAEdge(v_cmdbuffer *cmdbuffer, v_u32 vtype, v_u32 *v1, v_u32 *v2);

extern vl_error V_DLLIMPORT VL_Span(v_cmdbuffer *cmdbuffer, v_u32 v_type, v_u32 *start, v_u32 *delta);

extern vl_error V_DLLIMPORT VL_Square(v_cmdbuffer *cmdbuffer, v_u32 v_type, v_u32 side, v_u32 *vert);

extern vl_error V_DLLIMPORT VL_Rectangle(v_cmdbuffer *cmdbuffer, v_u32 v_type, v_u32 width, v_u32 height, v_u32 *vert);

extern vl_error V_DLLIMPORT VL_Triangle(v_cmdbuffer *cmdbuffer, v_u32 v_type, v_u32 *v1, v_u32 *v2, v_u32 *v3);

extern vl_error V_DLLIMPORT VL_TriangleNormQ(v_cmdbuffer *cmdbuffer, v_u32 v_type, v_u32 *v1, v_u32 *v2, v_u32 *v3);

#define	VL_Triangle3 VL_TriangleNormQ

extern vl_error V_DLLIMPORT VL_Tristrip(v_cmdbuffer *cmdbuffer, v_u32 v_type, v_u32 tri_count, v_u32 *verts);

extern vl_error V_DLLIMPORT VL_Trifan(v_cmdbuffer *cmdbuffer, v_u32 v_type, v_u32 vert_count, v_u32 *verts);

extern vl_error V_DLLIMPORT VL_TriangleFill(v_cmdbuffer *cmdbuffer, v_u32 x0, v_u32 y0, v_u32 x1, v_u32 y1, v_u32 x2, v_u32 y2);

extern vl_error V_DLLIMPORT VL_Affine(v_cmdbuffer *cmdbuffer,
		     v_u32 ulx,
		     v_u32 uly,
		     v_u32 ulu,
		     v_u32 ulv,
		     v_u32 width,
		     v_u32 height,
		     v_u32 dudx,
		     v_u32 dvdx,
		     v_u32 dudy,
		     v_u32 dvdy);

extern vl_error V_DLLIMPORT VL_Warp(v_cmdbuffer *cmdbuffer,
			v_u16 ulx, v_u16 uly,
			v_u16 width, v_u16 height,
			v_u32 ulu, v_u32 ulv,
			v_memory tex_coords);

extern vl_error V_DLLIMPORT VL_Bitblt(v_cmdbuffer *cmdbuffer,
		     v_u16 dst_ulx, v_u16 dst_uly,
		     v_u16 width, v_u16 height,
		     v_u16 src_ulx, v_u16 src_uly);

extern vl_error V_DLLIMPORT VL_CompositeRect(v_cmdbuffer *cmdbuffer,
				 v_u16 pixel_fmt,
				 v_u16 ulx, v_u16 uly,
				 v_u16 width, v_u16 height,
				 v_u32 u_start, v_u32 v_start,
				 v_u32 u_stop, v_u32 v_stop,
				 v_memory pix_data);

extern vl_error V_DLLIMPORT VL_Bitfill(v_cmdbuffer *cmdbuffer, v_u16 ulx, v_u16 uly, v_u16 width, v_u16 height);

extern vl_error V_DLLIMPORT VL_BitbltMem(v_cmdbuffer *cmdbuffer,
				v_u16 ulx, v_u16 uly,
				v_u16 width, v_u16 height,
				v_byte_swap byte_swap,
				v_memory pix_memory,
				v_u32 *pix_addr);

extern vl_error V_DLLIMPORT VL_Lookup(v_cmdbuffer *cmdbuffer,
			    v_u16 ulx, v_u16 uly,
			    v_u16 width, v_u16 height,
			    v_memory pix_memory,
			    v_u32 *pix_addr);

extern vl_error V_DLLIMPORT VL_MemWriteSprite(v_cmdbuffer *cmdbuffer,
				   v_u32 dest_addr, v_u32 pixel_fmt,
				   v_u32 compressed_data_bytes, v_memory compressed_data);

extern vl_error V_DLLIMPORT VL_Particles(v_cmdbuffer *cmdbuffer,
			       v_u32 num_particles, v_memory particle_data);

/*
 * End of frame marker (for debugging)
 */
extern vl_error V_DLLIMPORT VL_FrameMarker(v_cmdbuffer *cmdbuffer, int frame_num);

/*
 * Error Handling
 */
extern vl_error V_DLLIMPORT VL_RegisterErrorHandler(vl_error_handler peh);

extern vl_error_handler V_DLLIMPORT VL_GetErrorHandler(void);

extern vl_error V_DLLIMPORT VL_GetFunctionName(vl_routine routine, LPSTR s, int nbuff);

extern vl_error V_DLLIMPORT VL_GetErrorText(vl_error err, LPSTR s, int nbuff);

extern vl_error V_DLLIMPORT VL_GetLastError(vl_error *eperr, vl_routine *proutine, int *pexterr);

/*
 * Extensions
 */
extern char * V_DLLIMPORT VL_GetExtensions(v_handle vhandle);

extern void * V_DLLIMPORT VL_GetExtensionFunction(char *function);

#ifdef __cplusplus
}
#endif

#endif /*REDLINE_H*/

