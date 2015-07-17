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
#ifndef RLFIFO_H
#define RLFIFO_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * FIFO command header codes
 */
#define V_FIFO_RESERVED0                        0
#define V_FIFO_RESERVED1                        1
#define V_FIFO_CONTEXT_INIT                     2

/* 3 reserved */
#define V_FIFO_DISPLAY                          4
#define V_FIFO_STEREO_DISPLAY                   5
#define V_FIFO_WAIT_DISPLAY_SWITCH              6
/* 7 reserved */

#define V_FIFO_SYNC_AND_RESPOND                 8
#define V_FIFO_MEM_WRITE                        9
#define V_FIFO_MEM_WRITE_RECT                  10
#define V_FIFO_MEM_CLEAR_RECT                  13
#define V_FIFO_MEM_WRITE_SPRITE                14

#define V_FIFO_DOT                             15
#define V_FIFO_LINE                            16
#define V_FIFO_POLYLINE                        17
#define V_FIFO_INTLINE                         18
#define V_FIFO_INTPOLYLINE                     19
#define V_FIFO_AALINE                          20
#define V_FIFO_SPAN                            21
#define V_FIFO_SQUARE                          22
#define V_FIFO_RECTANGLE                       23
#define V_FIFO_TRIANGLE                        24
#define V_FIFO_AAEDGE                          25
#define V_FIFO_TRISTRIP                        26
#define V_FIFO_TRIFAN                          27
#define V_FIFO_AFFINE                          28
#define V_FIFO_WARP                            29
#define V_FIFO_BITBLT                          30
#define V_FIFO_BITFILL                         31
#define V_FIFO_RLEHACK                         32
#define V_FIFO_PALETTE                         33
#define V_FIFO_TIMER_FUNCS                     34
#define V_FIFO_FRAME_MARKER                    35
#define V_FIFO_COMPOSITE_RECT                  36
#define V_FIFO_PARTICLES                       37
#define V_FIFO_TRI_FILL                        38
#define V_FIFO_QSPAN                           39
#define V_FIFO_QLIGHT                          40
#define V_FIFO_BITBLT_MEM                      41
#define V_FIFO_LOOKUP                          42
#define V_FIFO_QAAZEDGE                        43
#define V_FIFO_FTRI_KXYZ                       48
#define V_FIFO_FTRI_KXYZUVQ                    49
#define V_FIFO_FTRI_KFXYZUVQ                   50
#define V_FIFO_FTRI_KSXYZ                      51
#define V_FIFO_FTRI_KSXYZUVQ                   52
#define V_FIFO_D3DTRI                          54
#define V_FIFO_D3DTRI_ODD                      55
#define V_FIFO_D3DTRI_EVEN                     56

/*
 * Command structure format definitions
 */
typedef struct {
    v_u32      cmd;
} v_fifo_sync_and_respond;

typedef struct {
    v_u32      cmd;
    v_u32      val;
} v_fifo_set_state;

typedef struct {
    v_u32      cmd;
    v_u32      src_base;
    v_u32      src_stride;
    v_u32      src_mask;       /* VMask << 16 | UMask */
    v_u32      umultiplier;
    v_u32      vmultiplier;
} v_fifo_set_current_texture;

typedef struct {
    v_u32      cmd;
    v_u32      texpal[256];	/* 8888 ARGB */
} v_fifo_texture_palette;

typedef struct {
    v_u32      cmd;
    v_foreignAddress dst_addr;
    v_u32      byte_count;
 /* v_u32      data... */
} v_fifo_mem_write;

typedef struct {
    v_u32      cmd;
    v_foreignAddress dst_addr;
    v_u32      dst_linebytes;
    v_u32      byte_width;
    v_u32      height;
 /* v_u32      data... */
} v_fifo_mem_write_rect;

typedef struct {
    v_u32      cmd;
    v_foreignAddress dst_addr;
    v_u32      byte_count;
    v_u32      data1;
    v_u32      data2;
} v_fifo_mem_fill;

typedef struct {
    v_u32      cmd;
    v_foreignAddress dst_addr;
    v_u32      dst_linebytes;
    v_u32      byte_width;
    v_u32      height;
    v_u32      data1;
    v_u32      data2;
} v_fifo_mem_fill_rect;

typedef struct {
    v_u32      cmd;
    v_foreignAddress dst_addr;
    v_u32      dst_linebytes;
    v_u32      byte_width;
    v_u32      height;
    v_u32      data1;
} v_fifo_mem_clear_rect;

typedef struct {
    v_u32       cmd;
    v_u32       numparticles;
} v_fifo_particles;

typedef struct {
    v_u32      cmd;
    v_u32      start_num;
 /* v_u32      entries ... */
} v_fifo_palette;

typedef struct {
    v_u32      cmd;
    v_foreignAddress addr;
} v_fifo_display;

typedef struct {
    v_u32      cmd;
    v_foreignAddress odd_addr;
    v_foreignAddress even_addr;
} v_fifo_stereo_display;

typedef struct {
    v_u32      cmd;
} v_fifo_wait_display_switch;

typedef struct {
    v_u32      cmd;
    v_u32      pattern;
    v_u32      frame_number;
} v_fifo_frame_marker;

/* define a bit pattern which is unlikely to be in the data
   to be used as an end of frame marker for debugging.
   Set op field to zero, msbs to 0xA to avoid match with coords,
   random other data */
#define V_FIFO_FRAME_MARKER_PATTERN     ((v_u32)0xA5C31697)
#define V_FIFO_FRAME_MARKER_CMD         ((v_u32)(0x5A3C0000|V_FIFO_FRAME_MARKER))

typedef struct {
    v_u32      cmd;
} v_fifo_timer_funcs;

/* set cmd to one of the below for timer funcs */
#define V_FIFO_TIMER_RESET      V_FIFO_TIMER_FUNCS
#define V_FIFO_TIMER_START(n)   (V_FIFO_TIMER_FUNCS|(1<<16)|(((n)&0x1f)<<24))
#define V_FIFO_TIMER_STOP(n)    (V_FIFO_TIMER_FUNCS|(2<<16)|(((n)&0x1f)<<24))
#define V_FIFO_TIMER_GET(n)     (V_FIFO_TIMER_FUNCS|(3<<16)|(((n)&0x1f)<<24))

/*
 * Vertex structure definitions
 */

/* Note:
   x,y,i,r,g,b,a,f terms are s15.16 fixed point
   u,v terms are 32 bit signed with binary point determined by
     value of UMultiplier/VMultiplier
   q terms are 8.24 unsigned, with the high 8 bits zero if at all
     possible
   bgr or rgb terms have 3 x 8bit RGB or BGR channels in the low 24 bits
*/

/* fixed point flavors of these types */

#define	V_SIGNED	v_s32
#define	V_UNSIGNED	v_u32
#define	V_PACKED	v_u32

typedef struct {
        V_SIGNED  x;
        V_SIGNED  y;
} v_xy;

typedef struct {
        V_SIGNED  x;
        V_SIGNED  y;
        V_SIGNED  u;
        V_SIGNED  v;
} v_xyuv;

typedef struct {
        V_SIGNED  x;
        V_SIGNED  y;
        V_SIGNED  u;
        V_SIGNED  v;
        V_UNSIGNED  q;
} v_xyuvq;

typedef struct {
        V_SIGNED  i;
        V_SIGNED  x;
        V_SIGNED  y;
        V_SIGNED  u;
        V_SIGNED  v;
        V_UNSIGNED  q;
} v_ixyuvq;

typedef struct {
        V_SIGNED  f;
        V_SIGNED  x;
        V_SIGNED  y;
        V_SIGNED  u;
        V_SIGNED  v;
        V_UNSIGNED  q;
} v_fxyuvq;

typedef struct {
        V_SIGNED  x;
        V_SIGNED  y;
        V_UNSIGNED  z;
        V_SIGNED  u;
        V_SIGNED  v;
        V_UNSIGNED  q;
} v_xyzuvq;

typedef struct {
        V_SIGNED  r;
        V_SIGNED  g;
        V_SIGNED  b;
        V_SIGNED  f;
        V_SIGNED  x;
        V_SIGNED  y;
} v_rgbfxy;

typedef struct {
        V_SIGNED  r;
        V_SIGNED  g;
        V_SIGNED  b;
        V_SIGNED  x;
        V_SIGNED  y;
        V_UNSIGNED  z;
} v_rgbxyz;

typedef struct {
        V_SIGNED  i;
        V_SIGNED  f;
        V_SIGNED  x;
        V_SIGNED  y;
        V_SIGNED  u;
        V_SIGNED  v;
        V_UNSIGNED  q;
} v_ifxyuvq;

typedef struct {
        V_SIGNED  i;
        V_SIGNED  f;
        V_SIGNED  x;
        V_SIGNED  y;
        V_UNSIGNED  z;
        V_SIGNED  u;
        V_SIGNED  v;
        V_UNSIGNED  q;
} v_ifxyzuvq;

typedef struct {
        V_SIGNED  i;
        V_SIGNED  x;
        V_SIGNED  y;
        V_UNSIGNED  z;
        V_SIGNED  u;
        V_SIGNED  v;
        V_UNSIGNED  q;
} v_ixyzuvq;

typedef struct {
        V_SIGNED  r;
        V_SIGNED  g;
        V_SIGNED  b;
        V_SIGNED  a;
        V_SIGNED  f;
        V_SIGNED  x;
        V_SIGNED  y;
        V_SIGNED  u;
        V_SIGNED  v;
        V_UNSIGNED  q;
} v_rgbafxyuvq;

typedef struct {
        V_SIGNED  r;
        V_SIGNED  g;
        V_SIGNED  b;
        V_SIGNED  a;
        V_SIGNED  x;
        V_SIGNED  y;
        V_UNSIGNED  z;
        V_SIGNED  u;
        V_SIGNED  v;
        V_UNSIGNED  q;
} v_rgbaxyzuvq;

typedef struct {
        V_PACKED  rgb;
        V_SIGNED  f;
        V_SIGNED  x;
        V_SIGNED  y;
} v_kfxy;

typedef struct {
        V_PACKED  rgb;
        V_SIGNED  x;
        V_SIGNED  y;
        V_UNSIGNED  z;
} v_kxyz;

typedef struct {
        V_PACKED  rgb;
        V_SIGNED  a;
        V_SIGNED  f;
        V_SIGNED  x;
        V_SIGNED  y;
        V_SIGNED  u;
        V_SIGNED  v;
        V_UNSIGNED  q;
} v_kafxyuvq;

typedef struct {
       V_PACKED    rgb;
       V_SIGNED    a;
       V_SIGNED    x;
       V_SIGNED    y;
       V_UNSIGNED    z;
       V_SIGNED    u;
       V_SIGNED    v;
       V_UNSIGNED    q;
} v_kaxyzuvq;

typedef struct {
        V_SIGNED  i;
        V_SIGNED  x;
        V_SIGNED  y;
        V_UNSIGNED  z;
        V_SIGNED  u;
        V_SIGNED  v;
} v_ixyzuv;

typedef struct {
        V_SIGNED  r;
        V_SIGNED  g;
        V_SIGNED  b;
        V_SIGNED  a;
        V_SIGNED  f;
        V_SIGNED  x;
        V_SIGNED  y;
        V_UNSIGNED  z;
        V_SIGNED  u;
        V_SIGNED  v;
        V_UNSIGNED  q;
} v_rgbafxyzuvq;

typedef struct {
       V_PACKED    rgb;
       V_SIGNED    a;
       V_SIGNED    f;
       V_SIGNED    x;
       V_SIGNED    y;
       V_UNSIGNED    z;
       V_SIGNED    u;
       V_SIGNED    v;
       V_UNSIGNED    q;
} v_kafxyzuvq;

typedef struct {
       V_PACKED    rgb;
       V_SIGNED    x;
       V_SIGNED    y;
       V_SIGNED    u;
       V_SIGNED    v;
       V_UNSIGNED    q;
} v_kxyuvq;

typedef struct {
       V_UNSIGNED	a;
       V_UNSIGNED	x;
       V_UNSIGNED	y;
} v_axy;

typedef struct {
       V_PACKED		rgb;
       V_UNSIGNED	x;
       V_UNSIGNED	y;
} v_kxy;

typedef struct {
       v_u32    rgb;
       v_s32    f;
       v_s32    x;
       v_s32    y;
       v_s32    z;
} v_kfxyz;

typedef struct {
       v_s32    f;
       v_s32    x;
       v_s32    y;
       v_s32    z;
} v_fxyz;

typedef struct {
        V_SIGNED  x;
        V_SIGNED  y;
        V_UNSIGNED  z;
} v_xyz;

typedef struct {
       V_PACKED    rgb;
       V_SIGNED    x;
       V_SIGNED    y;
       V_UNSIGNED    z;
       V_SIGNED    u;
       V_SIGNED    v;
       V_UNSIGNED    q;
} v_kxyzuvq;

typedef struct {
       V_PACKED    rgb;
       V_UNSIGNED  f;     /* f in upper byte */
       V_SIGNED    x;
       V_SIGNED    y;
       V_UNSIGNED    z;
       V_SIGNED    u;
       V_SIGNED    v;
       V_UNSIGNED    q;
} v_kfxyzuvq;

/*
 * Corresponding vertex types codes
 * The following letters represent the packed attributes:
 *     K	-RGB
 *     S	-R2G2B2 (specular)
 */

#define V_FIFO_XY                1
#define V_FIFO_XYUV              2
#define V_FIFO_XYUVQ             3
#define V_FIFO_IXYUVQ            4
#define V_FIFO_FXYUVQ            5
#define V_FIFO_XYZUVQ            6
#define V_FIFO_RGBFXY            7
#define V_FIFO_RGBXYZ            8
#define V_FIFO_IFXYUVQ           9
#define V_FIFO_IXYZUVQ          10
#define V_FIFO_RGBAFXYUVQ       11
#define V_FIFO_RGBAXYZUVQ       12

#define V_FIFO_KFXY             17
#define V_FIFO_KXYZ             18
#define V_FIFO_KAFXYUVQ         19
#define V_FIFO_KAXYZUVQ         20
#define V_FIFO_IXYZUV           21
#define V_FIFO_RGBAFXYZUVQ      22

#define V_FIFO_KAFXYZUVQ        24
#define V_FIFO_KXYUVQ           25
#define	V_FIFO_IFXYZUVQ			26
#define V_FIFO_AXY				27
#define V_FIFO_KXY				28
#define V_FIFO_KFXYZ            29
#define V_FIFO_FXYZ             30
#define V_FIFO_XYZ              31

/*
 * Rendering primitives
 */

/* Note: vertex data in the below refers to one of the
   above variable-length vertex structures. Vertex type
   is constant for a single primitive. E.g., v_fifo_line
   actual structure for V_FIFO_XY is:
   struct {
      v_fifo_line l;
      v_xy v0;		[... or v_f_xy for float types ]
      v_xy v1;
   } foo;
   foo.l is set to V_FIFO_VTX_CMD(V_FIFO_XY, V_FIFO_LINE)
   v0.x, v0.y, v1.x, v1.y are set to whatever.
*/

#define V_FIFO_VTX_CMD(vtx_type, cmd) (((v_u32)(vtx_type)<<16)|((v_u32)(cmd)))

#define V_FIFO_CMD(cmd) ((v_u32)(cmd))

#define V_FIFO_FLAGS_CMD(flags, cmd)  (((v_u32)(flags)<<16)|((v_u32)(cmd)))
/* currently defined flags */
#define V_FIFO_LAST_PIXEL_DRAW (1)
#define V_FIFO_LAST_PIXEL_OMIT (0)

#define V_FIFO_PACK_XY(x,y) ((((v_u32)(x)&0xFFFF)<<16)|((v_u32)(y)&0xFFFF))

/* construct scissor values */
/* pixel is inside if (l <= x < r) && (t <= y < b) */
#define V_SCISSORX(l,r) ((((v_u32)(l)&0xFFFF)<<16)|((v_u32)(r)&0xFFFF))
#define V_SCISSORY(t,b) ((((v_u32)(t)&0xFFFF)<<16)|((v_u32)(b)&0xFFFF))

/* construct dither offset value */
#define V_DITHEROFFSET(x,y) ((((v_u32)(y)&0x3)<<4)|((v_u32)(x)&0x3))

typedef struct {
    v_u32      vtx_cmd;
 /* vertex 0 data */
} v_fifo_dot;

typedef struct {
    v_u32      vtx_cmd;
 /* vertex 0 data */
 /* vertex 1 data */
} v_fifo_line;

typedef struct {
    v_u32      vtx_cmd;
    v_u32      line_count;    /* N >= 1 lines */
 /* vertex 0 data */
 /* ... */
 /* vertex N data */
} v_fifo_polyline;

typedef struct {
    v_u32      flags_cmd;
    v_u32      v0;            /* x0 << 16 | y0 */
    v_u32      v1;            /* x1 << 16 | y1 */
} v_fifo_intline;

typedef struct {
    v_u32      lastpixelflag_cmd; /* flag << 16 | cmd */
    v_u32      line_count;    /* N >= 1 lines */
 /* v_u32      v0;               x1 << 16 | y1 */
 /* ... */
 /* v_u32      vN;               xN << 16 | yN */
} v_fifo_intpolyline;

typedef struct {
    v_u32      vtx_cmd;
 /* vertex 1 data */
 /* vertex 2 data */
} v_fifo_aaline;

/* Note: vertices with packed colors (C,K) won't interpolate well
   with span because the color deltas can't be small enough */
typedef struct {
    v_u32      vtx_cmd;
 /* start vertex data */
 /* delta vertex data */
} v_fifo_span;

typedef struct {
    v_u32      vtx_cmd;
    v_s32    side;
 /* vertex 1 data */
} v_fifo_square;

typedef struct {
    v_u32      vtx_cmd;
    v_s32    width;
    v_s32    height;
 /* vertex 1 data */
} v_fifo_rectangle;

typedef struct {
    v_u32      vtx_cmd;
 /* vertex 0 data */
 /* vertex 1 data */
 /* vertex 2 data */
} v_fifo_triangle;

typedef struct {
    v_u32      vtx_cmd;
    v_u32      tri_count;   /* N >= 1 triangles */
 /* vertex 0 data */
 /* vertex 1 data */
 /* vertex 2 data */
 /* ... */
 /* vertex N+1 data */
} v_fifo_tristrip;

typedef struct {
    v_u32      vtx_cmd;
    v_u32      vtx_count;   /* N >= 3 vertices */
 /* vertex 0 data */
 /* vertex 1 data */
 /* vertex 2 data */
 /* ... */
 /* vertex N-1 data */
} v_fifo_trifan;

typedef struct {
    v_u32      cmd;         /* V_FIFO_AAEDGE */
 /* vertex 0 data */
 /* vertex 1 data */
} v_fifo_aaedge;

typedef struct {
    v_u32      cmd;         /* V_FIFO_AFFINE */
    v_s32    x0;          /* upper left value */
    v_s32    y0;          /* upper left value */
    v_s32    u0;          /* upper left value */
    v_s32    v0;          /* upper left value */
    v_s32    width;
    v_s32    height;
    v_s32    dudx;        /* scaled by 2^8 */
    v_s32    dvdx;        /* scaled by 2^8 */
    v_s32    dudy;        /* scaled by 2^8 */
    v_s32    dvdy;        /* scaled by 2^8 */
} v_fifo_affine;

typedef struct {
    v_u32      cmd;         /* V_FIFO_WARP */
    v_u32      v0;          /* x0 << 16 | y0 */
    v_u32      dim;         /* width << 16 | height */
    v_s32    u_ofs;       /* U offset */
    v_s32    v_ofs;       /* V offset */
 /* v_s32     u;               lookup u+u_ofs */
 /* v_s32     v;               lookup v+v_ofs */
 /* total of width * height u,v pairs */
} v_fifo_warp;

typedef struct {
    v_u32      cmd;         /* V_FIFO_BITBLT */
    v_u32      dst_offset;  /* dst_x << 16 | dst_y */
    v_u32      dim;         /* width << 16 | height */
    v_u32      src_offset;  /* src_x << 16 | src_y */
} v_fifo_bitblt;

typedef struct {
    v_u32      cmd;         /* V_FIFO_BITFILL */
    v_u32      dst_offset;  /* dst_x << 16 | dst_y */
    v_u32      dim;         /* width << 16 | height */
} v_fifo_bitfill;

typedef struct {
    v_u32      cmd;         /* V_FIFO_MEM_WRITE_SPRITE */
    v_u32      dst_addr;    /* dest framebuffer addr. */
    v_u32      pix_fmt;     /* Pixel format */
} v_fifo_memwritesprite;

/* host to board bitblt */
typedef struct {
    v_u32       cmd;            /* V_FIFO_BITBLT_MEM */
    v_u32       dstx_dsty;      /* dst_x << 16 | dst_y */
    v_u32       dimx_dimy;      /* width (in dwords) << 16 | height */
    v_u32       pixel_depth;    /* 1, 2, or 4 bytes/pixel */
    /* scan line pixel data padded to dwords... */
} v_fifo_bitblt_mem;

/* lookup 8 bit to 8, 16, or 32 (whatever's DstFmt) */
/* also does a draw so you can blend, chromakey, whatever */
typedef struct {
    v_u32       cmd;            /* V_FIFO_LOOKUP */
    v_u32       dstx_dsty;      /* dst_x << 16 | dst_y */
    v_u32       dimx_dimy;      /* width << 16 | height */
    /* byte data padded to dwords... */
} v_fifo_lookup;


/*** Macros ***/

/*
 *     fifo command formats:
 *
 *          msb                                  lsb
 *           31      23          15  11    7 6     0
 *          +---------+---------+---+-------+-------+
 *          |        mask       | op| shift |  reg  |
 *          +---------+---------+---+-------+-------+
 *          |                  data                 |
 *          +---------+---------+---------+---------+
 *
 *    Note: if mask field == 0, then shift field
 *          is ignored and entire 32 bits of second
 *          field is loaded into specified register.
 *          op == 0 means this isn't a state set command.
 */

#define V_FIFO_SSMSK(mask)      (((mask) & 0xFFFF) << 16)
#define V_FIFO_SSOP(op)         (((op) & 0xF) << 12)
#define V_FIFO_SSSHFT(shift)    (((shift) & 0x1F) << 7)
#define V_FIFO_SSREG(reg)       ((reg) & 0x7F)

/*
 * First pixel engine state
 */
/* Source base register */
#define V_FIFO_SRCBASE_REG                  0
#define V_FIFO_SRCBASE_OP                   1
#define V_FIFO_SRCBASE_MASK                 0
#define V_FIFO_SRCBASE_SHIFT                0
#define V_FIFO_SRCBASE                                                   \
   V_FIFO_SSMSK(V_FIFO_SRCBASE_MASK)+V_FIFO_SSOP(V_FIFO_SRCBASE_OP)      \
   +V_FIFO_SSSHFT(V_FIFO_SRCBASE_SHIFT)+V_FIFO_SSREG(V_FIFO_SRCBASE_REG)

/* Source format register */
#define V_FIFO_SRCFMT_REG                  48
#define V_FIFO_SRCFMT_OP                    1
#define V_FIFO_SRCFMT_MASK                  0
#define V_FIFO_SRCFMT_SHIFT                 0
#define V_FIFO_SRCFMT                                                 \
   V_FIFO_SSMSK(V_FIFO_SRCFMT_MASK)+V_FIFO_SSOP(V_FIFO_SRCFMT_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_SRCFMT_SHIFT)+V_FIFO_SSREG(V_FIFO_SRCFMT_REG)

#define V_FIFO_SRCFUNC_REG                 49
#define V_FIFO_SRCFUNC_OP                   1
#define V_FIFO_SRCFUNC_MASK                 0
#define V_FIFO_SRCFUNC_SHIFT                4
#define V_FIFO_SRCFUNC                                                  \
   V_FIFO_SSMSK(V_FIFO_SRCFUNC_MASK)+V_FIFO_SSOP(V_FIFO_SRCFUNC_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_SRCFUNC_SHIFT)+V_FIFO_SSREG(V_FIFO_SRCFUNC_REG)

#define V_FIFO_SRCFILTER_REG               50
#define V_FIFO_SRCFILTER_OP                 1
#define V_FIFO_SRCFILTER_MASK               0
#define V_FIFO_SRCFILTER_SHIFT              7
#define V_FIFO_SRCFILTER                                                    \
   V_FIFO_SSMSK(V_FIFO_SRCFILTER_MASK)+V_FIFO_SSOP(V_FIFO_SRCFILTER_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_SRCFILTER_SHIFT)+V_FIFO_SSREG(V_FIFO_SRCFILTER_REG)

#define V_FIFO_SRCCOLORNOPAD_REG           51
#define V_FIFO_SRCCOLORNOPAD_OP             1
#define V_FIFO_SRCCOLORNOPAD_MASK           0
#define V_FIFO_SRCCOLORNOPAD_SHIFT          9
#define V_FIFO_SRCCOLORNOPAD                                                        \
   V_FIFO_SSMSK(V_FIFO_SRCCOLORNOPAD_MASK)+V_FIFO_SSOP(V_FIFO_SRCCOLORNOPAD_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_SRCCOLORNOPAD_SHIFT)+V_FIFO_SSREG(V_FIFO_SRCCOLORNOPAD_REG)

#define V_FIFO_SWAPUV_REG                  52
#define V_FIFO_SWAPUV_OP                    1
#define V_FIFO_SWAPUV_MASK                  0
#define V_FIFO_SWAPUV_SHIFT                10
#define V_FIFO_SWAPUV                                                 \
   V_FIFO_SSMSK(V_FIFO_SWAPUV_MASK)+V_FIFO_SSOP(V_FIFO_SWAPUV_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_SWAPUV_SHIFT)+V_FIFO_SSREG(V_FIFO_SWAPUV_REG)

#define V_FIFO_CHROMAKEY_REG               53
#define V_FIFO_CHROMAKEY_OP                 1
#define V_FIFO_CHROMAKEY_MASK               0
#define V_FIFO_CHROMAKEY_SHIFT             11
#define V_FIFO_CHROMAKEY                                                    \
   V_FIFO_SSMSK(V_FIFO_CHROMAKEY_MASK)+V_FIFO_SSOP(V_FIFO_CHROMAKEY_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_CHROMAKEY_SHIFT)+V_FIFO_SSREG(V_FIFO_CHROMAKEY_REG)

#define V_FIFO_ZSCISSOR_REG                54
#define V_FIFO_ZSCISSOR_OP                  1
#define V_FIFO_ZSCISSOR_MASK                0
#define V_FIFO_ZSCISSOR_SHIFT              12
#define V_FIFO_ZSCISSOR                                                   \
   V_FIFO_SSMSK(V_FIFO_ZSCISSOR_MASK)+V_FIFO_SSOP(V_FIFO_ZSCISSOR_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_ZSCISSOR_SHIFT)+V_FIFO_SSREG(V_FIFO_ZSCISSOR_REG)

#define V_FIFO_SRCBGR_REG                  55
#define V_FIFO_SRCBGR_OP                    1
#define V_FIFO_SRCBGR_MASK                  0
#define V_FIFO_SRCBGR_SHIFT                13
#define V_FIFO_SRCBGR                                                    \
   V_FIFO_SSMSK(V_FIFO_SRCBGR_MASK)+V_FIFO_SSOP(V_FIFO_SRCBGR_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_SRCBGR_SHIFT)+V_FIFO_SSREG(V_FIFO_SRCBGR_REG)

#define V_FIFO_UMASK_REG                   56
#define V_FIFO_UMASK_OP                     1
#define V_FIFO_UMASK_MASK                   0
#define V_FIFO_UMASK_SHIFT                  0
#define V_FIFO_UMASK                                                   \
   V_FIFO_SSMSK(V_FIFO_UMASK_MASK)+V_FIFO_SSOP(V_FIFO_UMASK_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_UMASK_SHIFT)+V_FIFO_SSREG(V_FIFO_UMASK_REG)

#define V_FIFO_VMASK_REG                   57
#define V_FIFO_VMASK_OP                     1
#define V_FIFO_VMASK_MASK                   0
#define V_FIFO_VMASK_SHIFT                 16
#define V_FIFO_VMASK                                                   \
   V_FIFO_SSMSK(V_FIFO_VMASK_MASK)+V_FIFO_SSOP(V_FIFO_VMASK_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_VMASK_SHIFT)+V_FIFO_SSREG(V_FIFO_VMASK_REG)

/* Stride register */
#define V_FIFO_SRCSTRIDE_REG               58
#define V_FIFO_SRCSTRIDE_OP                 1
#define V_FIFO_SRCSTRIDE_MASK               0
#define V_FIFO_SRCSTRIDE_SHIFT              0
#define V_FIFO_SRCSTRIDE                                                    \
   V_FIFO_SSMSK(V_FIFO_SRCSTRIDE_MASK)+V_FIFO_SSOP(V_FIFO_SRCSTRIDE_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_SRCSTRIDE_SHIFT)+V_FIFO_SSREG(V_FIFO_SRCSTRIDE_REG)

#define V_FIFO_DSTSTRIDE_REG               59
#define V_FIFO_DSTSTRIDE_OP                 1
#define V_FIFO_DSTSTRIDE_MASK               0
#define V_FIFO_DSTSTRIDE_SHIFT              8
#define V_FIFO_DSTSTRIDE                                                    \
   V_FIFO_SSMSK(V_FIFO_DSTSTRIDE_MASK)+V_FIFO_SSOP(V_FIFO_DSTSTRIDE_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_DSTSTRIDE_SHIFT)+V_FIFO_SSREG(V_FIFO_DSTSTRIDE_REG)

#define V_FIFO_ZSTRIDE_REG                 60
#define V_FIFO_ZSTRIDE_OP                   1
#define V_FIFO_ZSTRIDE_MASK                 0
#define V_FIFO_ZSTRIDE_SHIFT               16
#define V_FIFO_ZSTRIDE                                                  \
   V_FIFO_SSMSK(V_FIFO_ZSTRIDE_MASK)+V_FIFO_SSOP(V_FIFO_ZSTRIDE_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_ZSTRIDE_SHIFT)+V_FIFO_SSREG(V_FIFO_ZSTRIDE_REG)

#define V_FIFO_UCLAMP_REG                  62
#define V_FIFO_UCLAMP_OP                    1
#define V_FIFO_UCLAMP_MASK                  0
#define V_FIFO_UCLAMP_SHIFT                15
#define V_FIFO_UCLAMP                                                    \
   V_FIFO_SSMSK(V_FIFO_UCLAMP_MASK)+V_FIFO_SSOP(V_FIFO_UCLAMP_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_UCLAMP_SHIFT)+V_FIFO_SSREG(V_FIFO_UCLAMP_REG)

#define V_FIFO_VCLAMP_REG                  63
#define V_FIFO_VCLAMP_OP                    1
#define V_FIFO_VCLAMP_MASK                  0
#define V_FIFO_VCLAMP_SHIFT                16
#define V_FIFO_VCLAMP                                                    \
   V_FIFO_SSMSK(V_FIFO_VCLAMP_MASK)+V_FIFO_SSOP(V_FIFO_VCLAMP_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_VCLAMP_SHIFT)+V_FIFO_SSREG(V_FIFO_VCLAMP_REG)

#define V_FIFO_SPECULARENABLE_REG                  83
#define V_FIFO_SPECULARENABLE_OP                    1
#define V_FIFO_SPECULARENABLE_MASK                  0
#define V_FIFO_SPECULARENABLE_SHIFT                17
#define V_FIFO_SPECULARENABLE                                                 \
   V_FIFO_SSMSK(V_FIFO_SPECULARENABLE_MASK)+V_FIFO_SSOP(V_FIFO_SPECULARENABLE_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_SPECULARENABLE_SHIFT)+V_FIFO_SSREG(V_FIFO_SPECULARENABLE_REG)

/* NOTE: Umult and Vmult are op 3 so the microcode can cache their values
   in registers as well as in the shadow */
#define V_FIFO_UMULTIPLIER_REG             40
#define V_FIFO_UMULTIPLIER_OP               5
#define V_FIFO_UMULTIPLIER_MASK             0
#define V_FIFO_UMULTIPLIER_SHIFT            0
#define V_FIFO_UMULTIPLIER                                                   \
   V_FIFO_SSMSK(V_FIFO_UMULTIPLIER_MASK)+V_FIFO_SSOP(V_FIFO_UMULTIPLIER_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_UMULTIPLIER_SHIFT)+V_FIFO_SSREG(V_FIFO_UMULTIPLIER_REG)

#define V_FIFO_VMULTIPLIER_REG             41
#define V_FIFO_VMULTIPLIER_OP               5
#define V_FIFO_VMULTIPLIER_MASK             0
#define V_FIFO_VMULTIPLIER_SHIFT            0
#define V_FIFO_VMULTIPLIER                                                    \
   V_FIFO_SSMSK(V_FIFO_VMULTIPLIER_MASK)+V_FIFO_SSOP(V_FIFO_VMULTIPLIER_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_VMULTIPLIER_SHIFT)+V_FIFO_SSREG(V_FIFO_VMULTIPLIER_REG)

#define V_FIFO_SOFFSET_REG                 42
#define V_FIFO_SOFFSET_OP                   6
#define V_FIFO_SOFFSET_MASK                 0
#define V_FIFO_SOFFSET_SHIFT                0
#define V_FIFO_SOFFSET                                                   \
   V_FIFO_SSMSK(V_FIFO_SOFFSET_MASK)+V_FIFO_SSOP(V_FIFO_SOFFSET_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_SOFFSET_SHIFT)+V_FIFO_SSREG(V_FIFO_SOFFSET_REG)

#define V_FIFO_TOFFSET_REG                 43
#define V_FIFO_TOFFSET_OP                   6
#define V_FIFO_TOFFSET_MASK                 0
#define V_FIFO_TOFFSET_SHIFT                0
#define V_FIFO_TOFFSET                                                    \
   V_FIFO_SSMSK(V_FIFO_TOFFSET_MASK)+V_FIFO_SSOP(V_FIFO_TOFFSET_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_TOFFSET_SHIFT)+V_FIFO_SSREG(V_FIFO_TOFFSET_REG)

/* Dst base register */
#define V_FIFO_DSTBASE_REG                  4
#define V_FIFO_DSTBASE_OP                   1
#define V_FIFO_DSTBASE_MASK                 0
#define V_FIFO_DSTBASE_SHIFT                0
#define V_FIFO_DSTBASE                                                  \
   V_FIFO_SSMSK(V_FIFO_DSTBASE_MASK)+V_FIFO_SSOP(V_FIFO_DSTBASE_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_DSTBASE_SHIFT)+V_FIFO_SSREG(V_FIFO_DSTBASE_REG)

/* Dst Mode register */
#define V_FIFO_ALUMODE_REG                 64
#define V_FIFO_ALUMODE_OP                   1
#define V_FIFO_ALUMODE_MASK                 0
#define V_FIFO_ALUMODE_SHIFT                0
#define V_FIFO_ALUMODE                                                  \
   V_FIFO_SSMSK(V_FIFO_ALUMODE_MASK)+V_FIFO_SSOP(V_FIFO_ALUMODE_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_ALUMODE_SHIFT)+V_FIFO_SSREG(V_FIFO_ALUMODE_REG)

#define V_FIFO_BLENDSRCFUNC_REG            65
#define V_FIFO_BLENDSRCFUNC_OP              1
#define V_FIFO_BLENDSRCFUNC_MASK            0
#define V_FIFO_BLENDSRCFUNC_SHIFT           4
#define V_FIFO_BLENDSRCFUNC                                                       \
   V_FIFO_SSMSK(V_FIFO_BLENDSRCFUNC_MASK)+V_FIFO_SSOP(V_FIFO_BLENDSRCFUNC_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_BLENDSRCFUNC_SHIFT)+V_FIFO_SSREG(V_FIFO_BLENDSRCFUNC_REG)

#define V_FIFO_BLENDDSTFUNC_REG            66
#define V_FIFO_BLENDDSTFUNC_OP              1
#define V_FIFO_BLENDDSTFUNC_MASK            0
#define V_FIFO_BLENDDSTFUNC_SHIFT           8
#define V_FIFO_BLENDDSTFUNC                                                       \
   V_FIFO_SSMSK(V_FIFO_BLENDDSTFUNC_MASK)+V_FIFO_SSOP(V_FIFO_BLENDDSTFUNC_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_BLENDDSTFUNC_SHIFT)+V_FIFO_SSREG(V_FIFO_BLENDDSTFUNC_REG)

#define V_FIFO_ZBUFMODE_REG                67
#define V_FIFO_ZBUFMODE_OP                  1
#define V_FIFO_ZBUFMODE_MASK                0
#define V_FIFO_ZBUFMODE_SHIFT              12
#define V_FIFO_ZBUFMODE                                                   \
   V_FIFO_SSMSK(V_FIFO_ZBUFMODE_MASK)+V_FIFO_SSOP(V_FIFO_ZBUFMODE_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_ZBUFMODE_SHIFT)+V_FIFO_SSREG(V_FIFO_ZBUFMODE_REG)

#define V_FIFO_ZBUFWRMODE_REG              68
#define V_FIFO_ZBUFWRMODE_OP                1
#define V_FIFO_ZBUFWRMODE_MASK              0
#define V_FIFO_ZBUFWRMODE_SHIFT            16
#define V_FIFO_ZBUFWRMODE                                                     \
   V_FIFO_SSMSK(V_FIFO_ZBUFWRMODE_MASK)+V_FIFO_SSOP(V_FIFO_ZBUFWRMODE_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_ZBUFWRMODE_SHIFT)+V_FIFO_SSREG(V_FIFO_ZBUFWRMODE_REG)

#define V_FIFO_YUV2RGB_REG                 69
#define V_FIFO_YUV2RGB_OP                   9
#define V_FIFO_YUV2RGB_MASK                 0
#define V_FIFO_YUV2RGB_SHIFT               18
#define V_FIFO_YUV2RGB                                                  \
   V_FIFO_SSMSK(V_FIFO_YUV2RGB_MASK)+V_FIFO_SSOP(V_FIFO_YUV2RGB_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_YUV2RGB_SHIFT)+V_FIFO_SSREG(V_FIFO_YUV2RGB_REG)

#define V_FIFO_BLENDENABLE_REG             70
#define V_FIFO_BLENDENABLE_OP               8
#define V_FIFO_BLENDENABLE_MASK             0
#define V_FIFO_BLENDENABLE_SHIFT           19
#define V_FIFO_BLENDENABLE                                                      \
   V_FIFO_SSMSK(V_FIFO_BLENDENABLE_MASK)+V_FIFO_SSOP(V_FIFO_BLENDENABLE_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_BLENDENABLE_SHIFT)+V_FIFO_SSREG(V_FIFO_BLENDENABLE_REG)

#define V_FIFO_DITHERENABLE_REG            71
#define V_FIFO_DITHERENABLE_OP             10
#define V_FIFO_DITHERENABLE_MASK            0
#define V_FIFO_DITHERENABLE_SHIFT          20
#define V_FIFO_DITHERENABLE                                                       \
   V_FIFO_SSMSK(V_FIFO_DITHERENABLE_MASK)+V_FIFO_SSOP(V_FIFO_DITHERENABLE_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_DITHERENABLE_SHIFT)+V_FIFO_SSREG(V_FIFO_DITHERENABLE_REG)

#define V_FIFO_FOGENABLE_REG               72
#define V_FIFO_FOGENABLE_OP                 1
#define V_FIFO_FOGENABLE_MASK               0
#define V_FIFO_FOGENABLE_SHIFT             21
#define V_FIFO_FOGENABLE                                                    \
   V_FIFO_SSMSK(V_FIFO_FOGENABLE_MASK)+V_FIFO_SSOP(V_FIFO_FOGENABLE_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_FOGENABLE_SHIFT)+V_FIFO_SSREG(V_FIFO_FOGENABLE_REG)

#define V_FIFO_DSTRDDISABLE_REG            74
#define V_FIFO_DSTRDDISABLE_OP              1
#define V_FIFO_DSTRDDISABLE_MASK            0
#define V_FIFO_DSTRDDISABLE_SHIFT          23
#define V_FIFO_DSTRDDISABLE                                                       \
   V_FIFO_SSMSK(V_FIFO_DSTRDDISABLE_MASK)+V_FIFO_SSOP(V_FIFO_DSTRDDISABLE_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_DSTRDDISABLE_SHIFT)+V_FIFO_SSREG(V_FIFO_DSTRDDISABLE_REG)

#define V_FIFO_DSTBGR_REG                  75
#define V_FIFO_DSTBGR_OP                    1
#define V_FIFO_DSTBGR_MASK                  0
#define V_FIFO_DSTBGR_SHIFT                24
#define V_FIFO_DSTBGR                                                    \
   V_FIFO_SSMSK(V_FIFO_DSTBGR_MASK)+V_FIFO_SSOP(V_FIFO_DSTBGR_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_DSTBGR_SHIFT)+V_FIFO_SSREG(V_FIFO_DSTBGR_REG)

#define V_FIFO_TRANSPREJECT_REG            76
#define V_FIFO_TRANSPREJECT_OP              1
#define V_FIFO_TRANSPREJECT_MASK            0
#define V_FIFO_TRANSPREJECT_SHIFT          25
#define V_FIFO_TRANSPREJECT                                                    \
   V_FIFO_SSMSK(V_FIFO_TRANSPREJECT_MASK)+V_FIFO_SSOP(V_FIFO_TRANSPREJECT_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_TRANSPREJECT_SHIFT)+V_FIFO_SSREG(V_FIFO_TRANSPREJECT_REG)

/* Dst Fmt Register */
#define V_FIFO_DSTFMT_REG                   6
#define V_FIFO_DSTFMT_OP                    1
#define V_FIFO_DSTFMT_MASK                  0
#define V_FIFO_DSTFMT_SHIFT                 0
#define V_FIFO_DSTFMT                                                 \
   V_FIFO_SSMSK(V_FIFO_DSTFMT_MASK)+V_FIFO_SSOP(V_FIFO_DSTFMT_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_DSTFMT_SHIFT)+V_FIFO_SSREG(V_FIFO_DSTFMT_REG)

/* Plane mask register */
#define V_FIFO_PMASK_REG                    7
#define V_FIFO_PMASK_OP                     1
#define V_FIFO_PMASK_MASK                   0
#define V_FIFO_PMASK_SHIFT                  0
#define V_FIFO_PMASK                                                 \
   V_FIFO_SSMSK(V_FIFO_PMASK_MASK)+V_FIFO_SSOP(V_FIFO_PMASK_OP)      \
   +V_FIFO_SSSHFT(V_FIFO_PMASK_SHIFT)+V_FIFO_SSREG(V_FIFO_PMASK_REG)

/* Pattern register */
#define V_FIFO_PAT_REG                      8
#define V_FIFO_PAT_OP                       1
#define V_FIFO_PAT_MASK                     0
#define V_FIFO_PAT_SHIFT                    0
#define V_FIFO_PAT                                                 \
   V_FIFO_SSMSK(V_FIFO_PAT_MASK)+V_FIFO_SSOP(V_FIFO_PAT_OP)        \
   +V_FIFO_SSSHFT(V_FIFO_PAT_SHIFT)+V_FIFO_SSREG(V_FIFO_PAT_REG)

/* Pattern mode register */
#define V_FIFO_PATLENGTHM1_REG             80
#define V_FIFO_PATLENGTHM1_OP               1
#define V_FIFO_PATLENGTHM1_MASK             0
#define V_FIFO_PATLENGTHM1_SHIFT            0
#define V_FIFO_PATLENGTHM1                                                      \
   V_FIFO_SSMSK(V_FIFO_PATLENGTHM1_MASK)+V_FIFO_SSOP(V_FIFO_PATLENGTHM1_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_PATLENGTHM1_SHIFT)+V_FIFO_SSREG(V_FIFO_PATLENGTHM1_REG)

#define V_FIFO_PATENABLE_REG               81
#define V_FIFO_PATENABLE_OP                 1
#define V_FIFO_PATENABLE_MASK               0
#define V_FIFO_PATENABLE_SHIFT             16
#define V_FIFO_PATENABLE                                                    \
   V_FIFO_SSMSK(V_FIFO_PATENABLE_MASK)+V_FIFO_SSOP(V_FIFO_PATENABLE_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_PATENABLE_SHIFT)+V_FIFO_SSREG(V_FIFO_PATENABLE_REG)

#define V_FIFO_PATOPAQUE_REG               82
#define V_FIFO_PATOPAQUE_OP                 1
#define V_FIFO_PATOPAQUE_MASK               0
#define V_FIFO_PATOPAQUE_SHIFT             17
#define V_FIFO_PATOPAQUE                                                    \
   V_FIFO_SSMSK(V_FIFO_PATOPAQUE_MASK)+V_FIFO_SSOP(V_FIFO_PATOPAQUE_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_PATOPAQUE_SHIFT)+V_FIFO_SSREG(V_FIFO_PATOPAQUE_REG)

/* Pattern Offset register */
#define V_FIFO_PATOFFSET_REG               13
#define V_FIFO_PATOFFSET_OP                 1
#define V_FIFO_PATOFFSET_MASK               0
#define V_FIFO_PATOFFSET_SHIFT              0
#define V_FIFO_PATOFFSET                                                    \
   V_FIFO_SSMSK(V_FIFO_PATOFFSET_MASK)+V_FIFO_SSOP(V_FIFO_PATOFFSET_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_PATOFFSET_SHIFT)+V_FIFO_SSREG(V_FIFO_PATOFFSET_REG)

/* X scissor register */
#define V_FIFO_SCISSORX_REG                14
#define V_FIFO_SCISSORX_OP                  1
#define V_FIFO_SCISSORX_MASK                0
#define V_FIFO_SCISSORX_SHIFT               0
#define V_FIFO_SCISSORX                                                   \
   V_FIFO_SSMSK(V_FIFO_SCISSORX_MASK)+V_FIFO_SSOP(V_FIFO_SCISSORX_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_SCISSORX_SHIFT)+V_FIFO_SSREG(V_FIFO_SCISSORX_REG)

/* Y scissor register */
#define V_FIFO_SCISSORY_REG                15
#define V_FIFO_SCISSORY_OP                  1
#define V_FIFO_SCISSORY_MASK                0
#define V_FIFO_SCISSORY_SHIFT               0
#define V_FIFO_SCISSORY                                                   \
   V_FIFO_SSMSK(V_FIFO_SCISSORY_MASK)+V_FIFO_SSOP(V_FIFO_SCISSORY_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_SCISSORY_SHIFT)+V_FIFO_SSREG(V_FIFO_SCISSORY_REG)

/* Zbase register */
#define V_FIFO_ZBASE_REG                   16
#define V_FIFO_ZBASE_OP                     1
#define V_FIFO_ZBASE_MASK                   0
#define V_FIFO_ZBASE_SHIFT                  0
#define V_FIFO_ZBASE                                                 \
   V_FIFO_SSMSK(V_FIFO_ZBASE_MASK)+V_FIFO_SSOP(V_FIFO_ZBASE_OP)      \
   +V_FIFO_SSSHFT(V_FIFO_ZBASE_SHIFT)+V_FIFO_SSREG(V_FIFO_ZBASE_REG)

/* Dither Offset register */
#define V_FIFO_DITHEROFFSET_REG           17
#define V_FIFO_DITHEROFFSET_OP             1
#define V_FIFO_DITHEROFFSET_MASK           0
#define V_FIFO_DITHEROFFSET_SHIFT          0
#define V_FIFO_DITHEROFFSET                                                        \
   V_FIFO_SSMSK(V_FIFO_DITHEROFFSET_MASK)+V_FIFO_SSOP(V_FIFO_DITHEROFFSET_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_DITHEROFFSET_SHIFT)+V_FIFO_SSREG(V_FIFO_DITHEROFFSET_REG)

/* FG Color register */
#define V_FIFO_FGCOLORARGB_REG             19
#define V_FIFO_FGCOLORARGB_OP               3
#define V_FIFO_FGCOLORARGB_MASK             0
#define V_FIFO_FGCOLORARGB_SHIFT            0
#define V_FIFO_FGCOLORARGB                  \
   V_FIFO_SSMSK(V_FIFO_FGCOLORARGB_MASK)    \
   +V_FIFO_SSOP(V_FIFO_FGCOLORARGB_OP)      \
   +V_FIFO_SSSHFT(V_FIFO_FGCOLORARGB_SHIFT) \
   +V_FIFO_SSREG(V_FIFO_FGCOLORARGB_REG)

/* BG Color register */
#define V_FIFO_BGCOLORARGB_REG             20
#define V_FIFO_BGCOLORARGB_OP               1
#define V_FIFO_BGCOLORARGB_MASK             0
#define V_FIFO_BGCOLORARGB_SHIFT            0
#define V_FIFO_BGCOLORARGB                  \
   V_FIFO_SSMSK(V_FIFO_BGCOLORARGB_MASK)    \
   +V_FIFO_SSOP(V_FIFO_BGCOLORARGB_OP)      \
   +V_FIFO_SSSHFT(V_FIFO_BGCOLORARGB_SHIFT) \
   +V_FIFO_SSREG(V_FIFO_BGCOLORARGB_REG)

/* Fog Color register */
#define V_FIFO_FOGCOLORRGB_REG             21
#define V_FIFO_FOGCOLORRGB_OP               1
#define V_FIFO_FOGCOLORRGB_MASK             0
#define V_FIFO_FOGCOLORRGB_SHIFT            0
#define V_FIFO_FOGCOLORRGB                  \
   V_FIFO_SSMSK(V_FIFO_FOGCOLORRGB_MASK)    \
   +V_FIFO_SSOP(V_FIFO_FOGCOLORRGB_OP)      \
   +V_FIFO_SSSHFT(V_FIFO_FOGCOLORRGB_SHIFT) \
   +V_FIFO_SSREG(V_FIFO_FOGCOLORRGB_REG)

/* Dst Color register */
#define V_FIFO_DSTCOLORARGB_REG            22
#define V_FIFO_DSTCOLORARGB_OP              1
#define V_FIFO_DSTCOLORARGB_MASK            0
#define V_FIFO_DSTCOLORARGB_SHIFT           0
#define V_FIFO_DSTCOLORARGB                 \
   V_FIFO_SSMSK(V_FIFO_DSTCOLORARGB_MASK)   \
   +V_FIFO_SSOP(V_FIFO_DSTCOLORARGB_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_DSTCOLORARGB_SHIFT)\
   +V_FIFO_SSREG(V_FIFO_DSTCOLORARGB_REG)

/* Chroma Color register */
#define V_FIFO_CHROMACOLOR_REG             23
#define V_FIFO_CHROMACOLOR_OP               1
#define V_FIFO_CHROMACOLOR_MASK             0
#define V_FIFO_CHROMACOLOR_SHIFT            0
#define V_FIFO_CHROMACOLOR                                                      \
   V_FIFO_SSMSK(V_FIFO_CHROMACOLOR_MASK)+V_FIFO_SSOP(V_FIFO_CHROMACOLOR_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_CHROMACOLOR_SHIFT)+V_FIFO_SSREG(V_FIFO_CHROMACOLOR_REG)

/* Chroma Color register */
#define V_FIFO_YUVCONTRAST_REG             24
#define V_FIFO_YUVCONTRAST_OP               1
#define V_FIFO_YUVCONTRAST_MASK             0
#define V_FIFO_YUVCONTRAST_SHIFT            0

#define V_FIFO_CHROMAMASK_REG              26
#define V_FIFO_CHROMAMASK_OP                1
#define V_FIFO_CHROMAMASK_MASK              0
#define V_FIFO_CHROMAMASK_SHIFT             0
#define V_FIFO_CHROMAMASK                                                     \
   V_FIFO_SSMSK(V_FIFO_CHROMAMASK_MASK)+V_FIFO_SSOP(V_FIFO_CHROMAMASK_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_CHROMAMASK_SHIFT)+V_FIFO_SSREG(V_FIFO_CHROMAMASK_REG)

#define V_FIFO_ALPHATHRESHOLD_REG          29
#define V_FIFO_ALPHATHRESHOLD_OP            1
#define V_FIFO_ALPHATHRESHOLD_MASK          0
#define V_FIFO_ALPHATHRESHOLD_SHIFT         0
#define V_FIFO_ALPHATHRESHOLD                                                     \
   V_FIFO_SSMSK(V_FIFO_ALPHATHRESHOLD_MASK)+V_FIFO_SSOP(V_FIFO_ALPHATHRESHOLD_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_ALPHATHRESHOLD_SHIFT)+V_FIFO_SSREG(V_FIFO_ALPHATHRESHOLD_REG)

#define V_FIFO_TEX4PAL_REG                 32
#define V_FIFO_TEX4PAL_OP                   7
#define V_FIFO_TEX4PAL_MASK                 0
#define V_FIFO_TEX4PAL_SHIFT                0
#define V_FIFO_TEX4PAL                                                  \
   V_FIFO_SSMSK(V_FIFO_TEX4PAL_MASK)+V_FIFO_SSOP(V_FIFO_TEX4PAL_OP)     \
   +V_FIFO_SSSHFT(V_FIFO_TEX4PAL_SHIFT)+V_FIFO_SSREG(V_FIFO_TEX4PAL_REG)

/*
 * Vertex attributes
 */

#define V_FIFO_Z_REG                       84
#define V_FIFO_Z_OP                         2
#define V_FIFO_Z_MASK                       0
#define V_FIFO_Z_SHIFT                      0
#define V_FIFO_Z                                                     \
   V_FIFO_SSMSK(V_FIFO_Z_MASK)+V_FIFO_SSOP(V_FIFO_Z_OP)              \
   +V_FIFO_SSSHFT(V_FIFO_Z_SHIFT)+V_FIFO_SSREG(V_FIFO_Z_REG)

#define V_FIFO_A_REG                       85
#define V_FIFO_A_OP                         2
#define V_FIFO_A_MASK                       0
#define V_FIFO_A_SHIFT                      0
#define V_FIFO_A                                                     \
   V_FIFO_SSMSK(V_FIFO_A_MASK)+V_FIFO_SSOP(V_FIFO_A_OP)              \
   +V_FIFO_SSSHFT(V_FIFO_A_SHIFT)+V_FIFO_SSREG(V_FIFO_A_REG)

#define V_FIFO_R_REG                       86
#define V_FIFO_R_OP                         2
#define V_FIFO_R_MASK                       0
#define V_FIFO_R_SHIFT                      0
#define V_FIFO_R                                                     \
   V_FIFO_SSMSK(V_FIFO_R_MASK)+V_FIFO_SSOP(V_FIFO_R_OP)              \
   +V_FIFO_SSSHFT(V_FIFO_R_SHIFT)+V_FIFO_SSREG(V_FIFO_R_REG)

#define V_FIFO_V_REG                       87
#define V_FIFO_V_OP                         2
#define V_FIFO_V_MASK                       0
#define V_FIFO_V_SHIFT                      0
#define V_FIFO_V                                                     \
   V_FIFO_SSMSK(V_FIFO_V_MASK)+V_FIFO_SSOP(V_FIFO_V_OP)              \
   +V_FIFO_SSSHFT(V_FIFO_V_SHIFT)+V_FIFO_SSREG(V_FIFO_V_REG)

#define V_FIFO_G_REG                       88
#define V_FIFO_G_OP                         2
#define V_FIFO_G_MASK                       0
#define V_FIFO_G_SHIFT                      0
#define V_FIFO_G                                                     \
   V_FIFO_SSMSK(V_FIFO_G_MASK)+V_FIFO_SSOP(V_FIFO_G_OP)              \
   +V_FIFO_SSSHFT(V_FIFO_G_SHIFT)+V_FIFO_SSREG(V_FIFO_G_REG)

#define V_FIFO_B_REG                       89
#define V_FIFO_B_OP                         2
#define V_FIFO_B_MASK                       0
#define V_FIFO_B_SHIFT                      0
#define V_FIFO_B                                                     \
   V_FIFO_SSMSK(V_FIFO_B_MASK)+V_FIFO_SSOP(V_FIFO_B_OP)              \
   +V_FIFO_SSSHFT(V_FIFO_B_SHIFT)+V_FIFO_SSREG(V_FIFO_B_REG)

#define V_FIFO_U_REG                       90
#define V_FIFO_U_OP                         2
#define V_FIFO_U_MASK                       0
#define V_FIFO_U_SHIFT                      0
#define V_FIFO_U                                                     \
   V_FIFO_SSMSK(V_FIFO_U_MASK)+V_FIFO_SSOP(V_FIFO_U_OP)              \
   +V_FIFO_SSSHFT(V_FIFO_U_SHIFT)+V_FIFO_SSREG(V_FIFO_U_REG)

#define V_FIFO_F_REG                       91
#define V_FIFO_F_OP                         2
#define V_FIFO_F_MASK                       0
#define V_FIFO_F_SHIFT                      0
#define V_FIFO_F                                                     \
   V_FIFO_SSMSK(V_FIFO_F_MASK)+V_FIFO_SSOP(V_FIFO_F_OP)              \
   +V_FIFO_SSSHFT(V_FIFO_F_SHIFT)+V_FIFO_SSREG(V_FIFO_F_REG)


/*
 * Other State Set commands
 */

/* NOTE: the REG for this command must match the SRCBASE reg */
#define V_FIFO_SET_CURRENT_TEXTURE_REG                  0
#define V_FIFO_SET_CURRENT_TEXTURE_OP                   4
#define V_FIFO_SET_CURRENT_TEXTURE_MASK                 0
#define V_FIFO_SET_CURRENT_TEXTURE_SHIFT                0
#define V_FIFO_SET_CURRENT_TEXTURE                                                    \
 V_FIFO_SSMSK(V_FIFO_SET_CURRENT_TEXTURE_MASK)+V_FIFO_SSOP(V_FIFO_SET_CURRENT_TEXTURE_OP)     \
 +V_FIFO_SSSHFT(V_FIFO_SET_CURRENT_TEXTURE_SHIFT)+V_FIFO_SSREG(V_FIFO_SET_CURRENT_TEXTURE_REG)


#ifdef __cplusplus
}
#endif

#endif /*RLFIFO_H*/

