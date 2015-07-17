/*******************************************************************
 * THIS FILE HAS BEEN HACKED BY AN EXPERIMENTAL PERL SCRIPT
 * ..\dynafy.pl
 * on 12 Aug 97
 *
 * The new functionality allows glide2x.dll to be loaded dynamically.
 * To do this, 
 * 1. #define DYNAHEADER wherever you include the new header ,
 *    glide.h.  
 * 2. Compile and link in glidein.c
 * 3. call LoadGlide();
 */
/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
*/

/*
** GLIDE.H
**
** The following #defines are relevant when using Glide:
**
** One of the following "platform constants" must be defined during
** compilation:
**
**            __DOS__           Defined for 32-bit DOS applications
**            __WIN32__         Defined for 32-bit Windows applications
**            __sparc__         Defined for Sun Solaris/SunOS
**            __linux__         Defined for Linux applications
**            __IRIX__          Defined for SGI Irix applications
**
*/
#ifndef __GLIDE_H__
#define __GLIDE_H__

#include <3dfx.h>
#include <sst1vid.h>
#include <glidesys.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** -----------------------------------------------------------------------
** TYPE DEFINITIONS
** -----------------------------------------------------------------------
*/
typedef FxU32 GrColor_t;
typedef FxU8  GrAlpha_t;
typedef FxU32 GrMipMapId_t;
typedef FxU8  GrFog_t;

/*
** -----------------------------------------------------------------------
** CONSTANTS AND TYPES
** -----------------------------------------------------------------------
*/
#define MAX_NUM_SST            4
#define MAX_MIPMAPS_PER_SST    1024
#define GR_FOG_TABLE_SIZE      64
#define GR_NULL_MIPMAP_HANDLE  ((GrMipMapId_t) -1)
#define GR_ZDEPTHVALUE_NEAREST  0xFFFF
#define GR_ZDEPTHVALUE_FARTHEST 0x0000
#define GR_WDEPTHVALUE_NEAREST  0x0000
#define GR_WDEPTHVALUE_FARTHEST 0xFFFF

#define GR_MIPMAPLEVELMASK_EVEN  FXBIT(0)
#define GR_MIPMAPLEVELMASK_ODD  FXBIT(1)
#define GR_MIPMAPLEVELMASK_BOTH (GR_MIPMAPLEVELMASK_EVEN | GR_MIPMAPLEVELMASK_ODD )

#define GR_LODBIAS_BILINEAR     0.5
#define GR_LODBIAS_TRILINEAR    0.0

typedef FxI32 GrChipID_t;
#define GR_TMU0         0x0
#define GR_TMU1         0x1
#define GR_TMU2         0x2
#define GR_FBI          0x3

typedef FxI32 GrCombineFunction_t;
#define GR_COMBINE_FUNCTION_ZERO        0x0
#define GR_COMBINE_FUNCTION_NONE        GR_COMBINE_FUNCTION_ZERO
#define GR_COMBINE_FUNCTION_LOCAL       0x1
#define GR_COMBINE_FUNCTION_LOCAL_ALPHA 0x2
#define GR_COMBINE_FUNCTION_SCALE_OTHER 0x3
#define GR_COMBINE_FUNCTION_BLEND_OTHER GR_COMBINE_FUNCTION_SCALE_OTHER
#define GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL 0x4
#define GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA 0x5 
#define GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL 0x6
#define GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL 0x7
#define GR_COMBINE_FUNCTION_BLEND GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL
#define GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA 0x8
#define GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL 0x9
#define GR_COMBINE_FUNCTION_BLEND_LOCAL GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL
#define GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA 0x10

typedef FxI32 GrCombineFactor_t;
#define GR_COMBINE_FACTOR_ZERO          0x0
#define GR_COMBINE_FACTOR_NONE          GR_COMBINE_FACTOR_ZERO
#define GR_COMBINE_FACTOR_LOCAL         0x1
#define GR_COMBINE_FACTOR_OTHER_ALPHA   0x2
#define GR_COMBINE_FACTOR_LOCAL_ALPHA   0x3
#define GR_COMBINE_FACTOR_TEXTURE_ALPHA 0x4
#define GR_COMBINE_FACTOR_DETAIL_FACTOR GR_COMBINE_FACTOR_TEXTURE_ALPHA
#define GR_COMBINE_FACTOR_LOD_FRACTION  0x5
#define GR_COMBINE_FACTOR_ONE           0x8
#define GR_COMBINE_FACTOR_ONE_MINUS_LOCAL 0x9
#define GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA 0xa
#define GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA 0xb
#define GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA 0xc
#define GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA
#define GR_COMBINE_FACTOR_ONE_MINUS_LOD_FRACTION 0xd


typedef FxI32 GrCombineLocal_t;
#define GR_COMBINE_LOCAL_ITERATED 0x0
#define GR_COMBINE_LOCAL_CONSTANT 0x1
#define GR_COMBINE_LOCAL_NONE GR_COMBINE_LOCAL_CONSTANT
#define GR_COMBINE_LOCAL_DEPTH  0x2

typedef FxI32 GrCombineOther_t;
#define GR_COMBINE_OTHER_ITERATED 0x0
#define GR_COMBINE_OTHER_TEXTURE 0x1
#define GR_COMBINE_OTHER_CONSTANT 0x2
#define GR_COMBINE_OTHER_NONE GR_COMBINE_OTHER_CONSTANT


typedef FxI32 GrAlphaSource_t;
#define GR_ALPHASOURCE_CC_ALPHA 0x0
#define GR_ALPHASOURCE_ITERATED_ALPHA 0x1
#define GR_ALPHASOURCE_TEXTURE_ALPHA 0x2
#define GR_ALPHASOURCE_TEXTURE_ALPHA_TIMES_ITERATED_ALPHA 0x3


typedef FxI32 GrColorCombineFnc_t;
#define GR_COLORCOMBINE_ZERO 0x0
#define GR_COLORCOMBINE_CCRGB 0x1
#define GR_COLORCOMBINE_ITRGB 0x2
#define GR_COLORCOMBINE_ITRGB_DELTA0 0x3
#define GR_COLORCOMBINE_DECAL_TEXTURE 0x4
#define GR_COLORCOMBINE_TEXTURE_TIMES_CCRGB 0x5
#define GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB 0x6
#define GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB_DELTA0 0x7
#define GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB_ADD_ALPHA 0x8
#define GR_COLORCOMBINE_TEXTURE_TIMES_ALPHA 0x9
#define GR_COLORCOMBINE_TEXTURE_TIMES_ALPHA_ADD_ITRGB 0xa
#define GR_COLORCOMBINE_TEXTURE_ADD_ITRGB 0xb
#define GR_COLORCOMBINE_TEXTURE_SUB_ITRGB 0xc
#define GR_COLORCOMBINE_CCRGB_BLEND_ITRGB_ON_TEXALPHA 0xd
#define GR_COLORCOMBINE_DIFF_SPEC_A 0xe
#define GR_COLORCOMBINE_DIFF_SPEC_B 0xf
#define GR_COLORCOMBINE_ONE 0x10

typedef FxI32 GrAlphaBlendFnc_t;
#define GR_BLEND_ZERO 0x0
#define GR_BLEND_SRC_ALPHA 0x1
#define GR_BLEND_SRC_COLOR 0x2
#define GR_BLEND_DST_COLOR GR_BLEND_SRC_COLOR
#define GR_BLEND_DST_ALPHA 0x3 
#define GR_BLEND_ONE 0x4
#define GR_BLEND_ONE_MINUS_SRC_ALPHA 0x5
#define GR_BLEND_ONE_MINUS_SRC_COLOR 0x6
#define GR_BLEND_ONE_MINUS_DST_COLOR GR_BLEND_ONE_MINUS_SRC_COLOR 
#define GR_BLEND_ONE_MINUS_DST_ALPHA 0x7
#define GR_BLEND_RESERVED_8 0x8
#define GR_BLEND_RESERVED_9 0x9
#define GR_BLEND_RESERVED_A 0xa
#define GR_BLEND_RESERVED_B 0xb
#define GR_BLEND_RESERVED_C 0xc
#define GR_BLEND_RESERVED_D 0xd
#define GR_BLEND_RESERVED_E 0xe
#define GR_BLEND_ALPHA_SATURATE 0xf
#define GR_BLEND_PREFOG_COLOR GR_BLEND_ALPHA_SATURATE

typedef FxI32 GrAspectRatio_t;
#define GR_ASPECT_8x1 0x0       /* 8W x 1H */
#define GR_ASPECT_4x1 0x1       /* 4W x 1H */
#define GR_ASPECT_2x1 0x2       /* 2W x 1H */
#define GR_ASPECT_1x1 0x3       /* 1W x 1H */
#define GR_ASPECT_1x2 0x4       /* 1W x 2H */
#define GR_ASPECT_1x4 0x5       /* 1W x 4H */
#define GR_ASPECT_1x8 0x6       /* 1W x 8H */

typedef FxI32 GrBuffer_t;
#define GR_BUFFER_FRONTBUFFER   0x0
#define GR_BUFFER_BACKBUFFER    0x1
#define GR_BUFFER_AUXBUFFER     0x2
#define GR_BUFFER_DEPTHBUFFER   0x3
#define GR_BUFFER_ALPHABUFFER   0x4
#define GR_BUFFER_TRIPLEBUFFER  0x5

typedef FxI32 GrChromakeyMode_t;
#define GR_CHROMAKEY_DISABLE    0x0
#define GR_CHROMAKEY_ENABLE     0x1

typedef FxI32 GrCmpFnc_t;
#define GR_CMP_NEVER    0x0
#define GR_CMP_LESS     0x1
#define GR_CMP_EQUAL    0x2
#define GR_CMP_LEQUAL   0x3
#define GR_CMP_GREATER  0x4
#define GR_CMP_NOTEQUAL 0x5
#define GR_CMP_GEQUAL   0x6
#define GR_CMP_ALWAYS   0x7

typedef FxI32 GrColorFormat_t;
#define GR_COLORFORMAT_ARGB     0x0
#define GR_COLORFORMAT_ABGR     0x1

#define GR_COLORFORMAT_RGBA     0x2
#define GR_COLORFORMAT_BGRA     0x3

typedef FxI32 GrCullMode_t;
#define GR_CULL_DISABLE         0x0
#define GR_CULL_NEGATIVE        0x1
#define GR_CULL_POSITIVE        0x2

typedef FxI32 GrDepthBufferMode_t;
#define GR_DEPTHBUFFER_DISABLE                  0x0
#define GR_DEPTHBUFFER_ZBUFFER                  0x1
#define GR_DEPTHBUFFER_WBUFFER                  0x2
#define GR_DEPTHBUFFER_ZBUFFER_COMPARE_TO_BIAS  0x3
#define GR_DEPTHBUFFER_WBUFFER_COMPARE_TO_BIAS  0x4

typedef FxI32 GrDitherMode_t;
#define GR_DITHER_DISABLE       0x0
#define GR_DITHER_2x2           0x1
#define GR_DITHER_4x4           0x2

typedef FxI32 GrFogMode_t;
#define GR_FOG_DISABLE          0x0
#define GR_FOG_WITH_ITERATED_ALPHA 0x1
#define GR_FOG_WITH_TABLE       0x2
#define GR_FOG_MULT2            0x100
#define GR_FOG_ADD2             0x200

typedef FxU32 GrLock_t;
#define GR_LFB_READ_ONLY  0x00
#define GR_LFB_WRITE_ONLY 0x01
#define GR_LFB_IDLE       0x00
#define GR_LFB_NOIDLE     0x10

typedef FxI32 GrLfbBypassMode_t;
#define GR_LFBBYPASS_DISABLE    0x0
#define GR_LFBBYPASS_ENABLE     0x1

typedef FxI32 GrLfbWriteMode_t;
#define GR_LFBWRITEMODE_565        0x0 /* RGB:RGB */
#define GR_LFBWRITEMODE_555        0x1 /* RGB:RGB */
#define GR_LFBWRITEMODE_1555       0x2 /* ARGB:ARGB */
#define GR_LFBWRITEMODE_RESERVED1  0x3
#define GR_LFBWRITEMODE_888        0x4 /* RGB */
#define GR_LFBWRITEMODE_8888       0x5 /* ARGB */
#define GR_LFBWRITEMODE_RESERVED2  0x6
#define GR_LFBWRITEMODE_RESERVED3  0x7
#define GR_LFBWRITEMODE_RESERVED4  0x8
#define GR_LFBWRITEMODE_RESERVED5  0x9
#define GR_LFBWRITEMODE_RESERVED6  0xa
#define GR_LFBWRITEMODE_RESERVED7  0xb
#define GR_LFBWRITEMODE_565_DEPTH  0xc /* RGB:DEPTH */
#define GR_LFBWRITEMODE_555_DEPTH  0xd /* RGB:DEPTH */
#define GR_LFBWRITEMODE_1555_DEPTH 0xe /* ARGB:DEPTH */
#define GR_LFBWRITEMODE_ZA16       0xf /* DEPTH:DEPTH */
#define GR_LFBWRITEMODE_ANY        0xFF


typedef FxI32 GrOriginLocation_t;
#define GR_ORIGIN_UPPER_LEFT    0x0
#define GR_ORIGIN_LOWER_LEFT    0x1
#define GR_ORIGIN_ANY           0xFF

typedef struct {
    int                size;
    void               *lfbPtr;
    FxU32              strideInBytes;        
    GrLfbWriteMode_t   writeMode;
    GrOriginLocation_t origin;
} GrLfbInfo_t;

typedef FxI32 GrLOD_t;
#define GR_LOD_256              0x0
#define GR_LOD_128              0x1
#define GR_LOD_64               0x2
#define GR_LOD_32               0x3
#define GR_LOD_16               0x4
#define GR_LOD_8                0x5
#define GR_LOD_4                0x6
#define GR_LOD_2                0x7
#define GR_LOD_1                0x8

typedef FxI32 GrMipMapMode_t;
#define GR_MIPMAP_DISABLE               0x0 /* no mip mapping  */
#define GR_MIPMAP_NEAREST               0x1 /* use nearest mipmap */
#define GR_MIPMAP_NEAREST_DITHER        0x2 /* GR_MIPMAP_NEAREST + LOD dith */


typedef FxI32 GrSmoothingMode_t;
#define GR_SMOOTHING_DISABLE    0x0
#define GR_SMOOTHING_ENABLE     0x1

typedef FxI32 GrTextureClampMode_t;
#define GR_TEXTURECLAMP_WRAP    0x0
#define GR_TEXTURECLAMP_CLAMP   0x1

typedef FxI32 GrTextureCombineFnc_t;
#define GR_TEXTURECOMBINE_ZERO          0x0 /* texout = 0 */
#define GR_TEXTURECOMBINE_DECAL         0x1 /* texout = texthis */
#define GR_TEXTURECOMBINE_OTHER         0x2 /* this TMU in passthru mode */
#define GR_TEXTURECOMBINE_ADD           0x3 /* tout = tthis + t(this+1) */
#define GR_TEXTURECOMBINE_MULTIPLY      0x4 /* texout = tthis * t(this+1) */
#define GR_TEXTURECOMBINE_SUBTRACT      0x5 /* Sutract from upstream TMU */
#define GR_TEXTURECOMBINE_DETAIL        0x6 /* detail--detail on tthis */
#define GR_TEXTURECOMBINE_DETAIL_OTHER  0x7 /* detail--detail on tthis+1 */
#define GR_TEXTURECOMBINE_TRILINEAR_ODD 0x8 /* trilinear--odd levels tthis*/
#define GR_TEXTURECOMBINE_TRILINEAR_EVEN 0x9 /*trilinear--even levels tthis*/
#define GR_TEXTURECOMBINE_ONE           0xa /* texout = 0xFFFFFFFF */

typedef FxI32 GrTextureFilterMode_t;
#define GR_TEXTUREFILTER_POINT_SAMPLED  0x0
#define GR_TEXTUREFILTER_BILINEAR       0x1

typedef FxI32 GrTextureFormat_t;
#define GR_TEXFMT_8BIT                  0x0
#define GR_TEXFMT_RGB_332 GR_TEXFMT_8BIT
#define GR_TEXFMT_YIQ_422               0x1
#define GR_TEXFMT_ALPHA_8               0x2 /* (0..0xFF) alpha     */
#define GR_TEXFMT_INTENSITY_8           0x3 /* (0..0xFF) intensity */
#define GR_TEXFMT_ALPHA_INTENSITY_44    0x4
#define GR_TEXFMT_P_8                   0x5 /* 8-bit palette */
#define GR_TEXFMT_RSVD0                 0x6
#define GR_TEXFMT_RSVD1                 0x7
#define GR_TEXFMT_16BIT                 0x8
#define GR_TEXFMT_ARGB_8332 GR_TEXFMT_16BIT
#define GR_TEXFMT_AYIQ_8422             0x9
#define GR_TEXFMT_RGB_565               0xa
#define GR_TEXFMT_ARGB_1555             0xb
#define GR_TEXFMT_ARGB_4444             0xc
#define GR_TEXFMT_ALPHA_INTENSITY_88    0xd
#define GR_TEXFMT_AP_88                 0xe /* 8-bit alpha 8-bit palette */
#define GR_TEXFMT_RSVD2                 0xf

typedef FxU32 GrTexTable_t;
#define GR_TEXTABLE_NCC0    0x0
#define GR_TEXTABLE_NCC1    0x1
#define GR_TEXTABLE_PALETTE 0x2

typedef FxU32 GrNCCTable_t;
#define GR_NCCTABLE_NCC0    0x0
#define GR_NCCTABLE_NCC1    0x1

typedef FxU32 GrTexBaseRange_t;
#define GR_TEXBASE_256      0x0
#define GR_TEXBASE_128      0x1
#define GR_TEXBASE_64       0x2
#define GR_TEXBASE_32_TO_1  0x3

#define GLIDE_STATE_PAD_SIZE 312
#ifdef GLIDE_LIB
typedef struct _GrState_s GrState;
#else
typedef struct _GrState_s {
  char pad[GLIDE_STATE_PAD_SIZE];
} GrState;
#endif

/*
** -----------------------------------------------------------------------
** STRUCTURES
** -----------------------------------------------------------------------
*/
/*
** 3DF texture file structs
*/
typedef struct
{
  FxU32               width, height;
  int                 small_lod, large_lod;
  GrAspectRatio_t     aspect_ratio;
  GrTextureFormat_t   format;
} Gu3dfHeader;

typedef struct
{
  FxU8  yRGB[16];
  FxI16 iRGB[4][3];
  FxI16 qRGB[4][3];
  FxU32 packed_data[12];
} GuNccTable;

typedef struct {
    FxU32 data[256];
} GuTexPalette;

typedef union {
    GuNccTable   nccTable;
    GuTexPalette palette;
} GuTexTable;

typedef struct
{
  Gu3dfHeader  header;
  GuTexTable   table;
  void        *data;
  FxU32        mem_required;    /* memory required for mip map in bytes. */
} Gu3dfInfo;

typedef struct {
    GrLOD_t           smallLod;
    GrLOD_t           largeLod;
    GrAspectRatio_t   aspectRatio;
    GrTextureFormat_t format;
    void              *data;
} GrTexInfo;

typedef struct
{
  int           sst;                    /* SST where this texture map was stored  */
  FxBool        valid;                  /* set when this table entry is allocated*/
  int           width, height;
  GrAspectRatio_t aspect_ratio;         /* aspect ratio of the mip map.  */
  void          *data;                  /* actual texture data  */

  GrTextureFormat_t  format;                    /* format of the texture table */
  GrMipMapMode_t     mipmap_mode;               /* mip map mode for this texture */
  GrTextureFilterMode_t   magfilter_mode;       /* filtering to be used when magnified */
  GrTextureFilterMode_t   minfilter_mode;       /* filtering to be used with minified  */
  GrTextureClampMode_t    s_clamp_mode;         /* how this texture should be clamped in s */
  GrTextureClampMode_t    t_clamp_mode;         /* how this texture should be clamped in t */
  FxU32         tLOD;                   /* Register value for tLOD register */ 
  FxU32         tTextureMode;           /* Register value for tTextureMode register
                                           not including non-texture specific bits */
  FxU32         lod_bias;               /* LOD bias of the mip map in preshifted 4.2*/
  GrLOD_t       lod_min, lod_max;       /* largest and smallest levels of detail  */
  int           tmu;                    /* tmu on which this texture resides */
  FxU32         odd_even_mask;          /* mask specifying levels on this tmu  */
  FxU32         tmu_base_address;       /* base addr (in TMU mem) of this texture */
  FxBool        trilinear;              /* should we blend by lod? */

  GuNccTable    ncc_table;              /* NCC compression table (optional) */
} GrMipMapInfo;

typedef int GrSstType;
#define GR_SSTTYPE_VOODOO    0
#define GR_SSTTYPE_SST96     1
#define GR_SSTTYPE_AT3D      2

typedef struct GrTMUConfig_St {
  int    tmuRev;                /* Rev of Texelfx chip */
  int    tmuRam;                /* 1, 2, or 4 MB */
} GrTMUConfig_t;

typedef struct GrVoodooConfig_St {
  int    fbRam;                         /* 1, 2, or 4 MB */
  int    fbiRev;                        /* Rev of Pixelfx chip */
  int    nTexelfx;                      /* How many texelFX chips are there? */
  FxBool sliDetect;                     /* Is it a scan-line interleaved board? */
  GrTMUConfig_t tmuConfig[GLIDE_NUM_TMU];   /* Configuration of the Texelfx chips */
} GrVoodooConfig_t;

typedef struct GrSst96Config_St {
  int   fbRam;                  /* How much? */
  int   nTexelfx;
  GrTMUConfig_t tmuConfig;
} GrSst96Config_t;

typedef struct GrAT3DConfig_St {
  int   rev;
} GrAT3DConfig_t;


typedef struct {
  int num_sst;                  /* # of HW units in the system */
  struct {
    GrSstType type;             /* Which hardware is it? */
    union SstBoard_u {
      GrVoodooConfig_t  VoodooConfig;
      GrSst96Config_t   SST96Config;
      GrAT3DConfig_t    AT3DConfig;
    } sstBoard;
  } SSTs[MAX_NUM_SST];          /* configuration for each board */
} GrHwConfiguration;

typedef struct GrSstPerfStats_s {
  FxU32  pixelsIn;              /* # pixels processed (minus buffer clears) */
  FxU32  chromaFail;            /* # pixels not drawn due to chroma key */ 
  FxU32  zFuncFail;             /* # pixels not drawn due to Z comparison */
  FxU32  aFuncFail;             /* # pixels not drawn due to alpha comparison */
  FxU32  pixelsOut;             /* # pixels drawn (including buffer clears) */
} GrSstPerfStats_t;


typedef struct {
  float  sow;                   /* s texture ordinate (s over w) */
  float  tow;                   /* t texture ordinate (t over w) */  
  float  oow;                   /* 1/w (used mipmapping - really 0xfff/w) */
}  GrTmuVertex;

/*
** GrVertex
** If these are changed the C & assembly language trisetup routines MUST
** be changed, for they will no longer work.
*/
typedef struct
{
  float x, y, z;                /* X, Y, and Z of scrn space -- Z is ignored */
  float r, g, b;                /* R, G, B, ([0..255.0]) */
  float ooz;                    /* 65535/Z (used for Z-buffering) */
  float a;                      /* Alpha [0..255.0] */
  float oow;                    /* 1/W (used for W-buffering, texturing) */
  GrTmuVertex  tmuvtx[GLIDE_NUM_TMU];
} GrVertex;

/* For indexing GrVertex as a float *.
   CHANGE THESE IF THE VERTEX STRUCTURE CHANGES!
   */
#define GR_VERTEX_X_OFFSET              0
#define GR_VERTEX_Y_OFFSET              1
#define GR_VERTEX_Z_OFFSET              2
#define GR_VERTEX_R_OFFSET              3
#define GR_VERTEX_G_OFFSET              4
#define GR_VERTEX_B_OFFSET              5
#define GR_VERTEX_OOZ_OFFSET            6
#define GR_VERTEX_A_OFFSET              7
#define GR_VERTEX_OOW_OFFSET            8
#define GR_VERTEX_SOW_TMU0_OFFSET       9
#define GR_VERTEX_TOW_TMU0_OFFSET       10
#define GR_VERTEX_OOW_TMU0_OFFSET       11
#define GR_VERTEX_SOW_TMU1_OFFSET       12
#define GR_VERTEX_TOW_TMU1_OFFSET       13
#define GR_VERTEX_OOW_TMU1_OFFSET       14
#if (GLIDE_NUM_TMU > 2)
#define GR_VERTEX_SOW_TMU2_OFFSET       15
#define GR_VERTEX_TOW_TMU2_OFFSET       16
#define GR_VERTEX_OOW_TMU2_OFFSET       17
#endif

typedef FxU32 GrLfbSrcFmt_t;
#define GR_LFB_SRC_FMT_565          0x00
#define GR_LFB_SRC_FMT_555          0x01
#define GR_LFB_SRC_FMT_1555         0x02
#define GR_LFB_SRC_FMT_888          0x04
#define GR_LFB_SRC_FMT_8888         0x05
#define GR_LFB_SRC_FMT_565_DEPTH    0x0c
#define GR_LFB_SRC_FMT_555_DEPTH    0x0d
#define GR_LFB_SRC_FMT_1555_DEPTH   0x0e
#define GR_LFB_SRC_FMT_ZA16         0x0f
#define GR_LFB_SRC_FMT_RLE16        0x80

typedef FxI32 GrPassthruMode_t;
#define GR_PASSTHRU_SHOW_VGA    0x0
#define GR_PASSTHRU_SHOW_SST1   0x1

typedef FxU32 GrHint_t;
#define GR_HINTTYPE_MIN                 0
#define GR_HINT_STWHINT                 0
#define GR_HINT_FIFOCHECKHINT           1
#define GR_HINT_FPUPRECISION            2
#define GR_HINT_ALLOW_MIPMAP_DITHER     3
#define GR_HINTTYPE_MAX                 3

typedef FxU32 GrSTWHint_t;
#define GR_STWHINT_W_DIFF_FBI   FXBIT(0)
#define GR_STWHINT_W_DIFF_TMU0  FXBIT(1)
#define GR_STWHINT_ST_DIFF_TMU0 FXBIT(2)
#define GR_STWHINT_W_DIFF_TMU1  FXBIT(3)
#define GR_STWHINT_ST_DIFF_TMU1 FXBIT(4)
#define GR_STWHINT_W_DIFF_TMU2  FXBIT(5)
#define GR_STWHINT_ST_DIFF_TMU2 FXBIT(6)

typedef FxU32 GrControl_t;
#define GR_CONTROL_ACTIVATE   0x1
#define GR_CONTROL_DEACTIVATE 0x2
#define GR_CONTROL_RESIZE     0x3
#define GR_CONTROL_MOVE       0x4

#define GR_GENERATE_FIFOCHECK_HINT_MASK(swHWM, swLWM) \
  (((swHWM & 0xffff) << 16) | (swLWM & 0xffff))

/*
** -----------------------------------------------------------------------
** FUNCTION PROTOTYPES
** -----------------------------------------------------------------------
*/
#ifndef FX_GLIDE_NO_FUNC_PROTO
/*
** rendering functions
*/
#ifdef DYNAHEADER
typedef void( FX_CALL *grDrawLine_fpt )( const GrVertex *v1, const GrVertex *v2 );
#else

FX_ENTRY void FX_CALL
grDrawLine( const GrVertex *v1, const GrVertex *v2 );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grDrawPlanarPolygon_fpt )( int nverts, const int ilist[], const GrVertex vlist[] );
#else

FX_ENTRY void FX_CALL
grDrawPlanarPolygon( int nverts, const int ilist[], const GrVertex vlist[] );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grDrawPlanarPolygonVertexList_fpt )( int nverts, const GrVertex vlist[] );
#else

FX_ENTRY void FX_CALL
grDrawPlanarPolygonVertexList( int nverts, const GrVertex vlist[] );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grDrawPoint_fpt )( const GrVertex *pt );
#else

FX_ENTRY void FX_CALL
grDrawPoint( const GrVertex *pt );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grDrawPolygon_fpt )( int nverts, const int ilist[], const GrVertex vlist[] );
#else

FX_ENTRY void FX_CALL
grDrawPolygon( int nverts, const int ilist[], const GrVertex vlist[] );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grDrawPolygonVertexList_fpt )( int nverts, const GrVertex vlist[] );
#else

FX_ENTRY void FX_CALL
grDrawPolygonVertexList( int nverts, const GrVertex vlist[] );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grDrawTriangle_fpt )( const GrVertex *a, const GrVertex *b, const GrVertex *c );
#else

FX_ENTRY void FX_CALL
grDrawTriangle( const GrVertex *a, const GrVertex *b, const GrVertex *c );

#endif /* DYNAHEADER */

/*
** buffer management
*/
#ifdef DYNAHEADER
typedef void( FX_CALL *grBufferClear_fpt )( GrColor_t color, GrAlpha_t alpha, FxU16 depth );
#else

FX_ENTRY void FX_CALL
grBufferClear( GrColor_t color, GrAlpha_t alpha, FxU16 depth );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef int( FX_CALL *grBufferNumPending_fpt )( void );
#else

FX_ENTRY int FX_CALL
grBufferNumPending( void );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grBufferSwap_fpt )( int swap_interval );
#else

FX_ENTRY void FX_CALL
grBufferSwap( int swap_interval );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grRenderBuffer_fpt )( GrBuffer_t buffer );
#else

FX_ENTRY void FX_CALL
grRenderBuffer( GrBuffer_t buffer );

#endif /* DYNAHEADER */

/*
** error management
*/
typedef void (*GrErrorCallbackFnc_t)( const char *string, FxBool fatal );

#ifdef DYNAHEADER
typedef void( FX_CALL *grErrorSetCallback_fpt )( GrErrorCallbackFnc_t fnc );
#else

FX_ENTRY void FX_CALL 
grErrorSetCallback( GrErrorCallbackFnc_t fnc );

#endif /* DYNAHEADER */

/*
** SST routines
*/
#ifdef DYNAHEADER
typedef void( FX_CALL *grSstIdle_fpt )(void );
#else

FX_ENTRY void FX_CALL 
grSstIdle(void);

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef FxU32( FX_CALL *grSstVideoLine_fpt )( void );
#else

FX_ENTRY FxU32 FX_CALL 
grSstVideoLine( void );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef FxBool( FX_CALL *grSstVRetraceOn_fpt )( void );
#else

FX_ENTRY FxBool FX_CALL 
grSstVRetraceOn( void );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef FxBool( FX_CALL *grSstIsBusy_fpt )( void );
#else

FX_ENTRY FxBool FX_CALL 
grSstIsBusy( void );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef FxBool( FX_CALL *grSstWinOpen_fpt )( FxU32 hWnd, GrScreenResolution_t screen_resolution, GrScreenRefresh_t refresh_rate, GrColorFormat_t color_format, GrOriginLocation_t origin_location, int nColBuffers, int nAuxBuffers );
#else

FX_ENTRY FxBool FX_CALL 
grSstWinOpen(
          FxU32                hWnd,
          GrScreenResolution_t screen_resolution,
          GrScreenRefresh_t    refresh_rate,
          GrColorFormat_t      color_format,
          GrOriginLocation_t   origin_location,
          int                  nColBuffers,
          int                  nAuxBuffers);

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grSstWinClose_fpt )( void );
#else

FX_ENTRY void FX_CALL
grSstWinClose( void );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef FxBool( FX_CALL *grSstControl_fpt )( FxU32 code );
#else

FX_ENTRY FxBool FX_CALL
grSstControl( FxU32 code );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef FxBool( FX_CALL *grSstQueryHardware_fpt )( GrHwConfiguration *hwconfig );
#else

FX_ENTRY FxBool FX_CALL 
grSstQueryHardware( GrHwConfiguration *hwconfig );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef FxBool( FX_CALL *grSstQueryBoards_fpt )( GrHwConfiguration *hwconfig );
#else

FX_ENTRY FxBool FX_CALL 
grSstQueryBoards( GrHwConfiguration *hwconfig );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grSstOrigin_fpt )(GrOriginLocation_t origin );
#else

FX_ENTRY void FX_CALL
grSstOrigin(GrOriginLocation_t  origin);

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grSstSelect_fpt )( int which_sst );
#else

FX_ENTRY void FX_CALL 
grSstSelect( int which_sst );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef FxU32( FX_CALL *grSstScreenHeight_fpt )( void );
#else

FX_ENTRY FxU32 FX_CALL 
grSstScreenHeight( void );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef FxU32( FX_CALL *grSstScreenWidth_fpt )( void );
#else

FX_ENTRY FxU32 FX_CALL 
grSstScreenWidth( void );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef FxU32( FX_CALL *grSstStatus_fpt )( void );
#else

FX_ENTRY FxU32 FX_CALL 
grSstStatus( void );

#endif /* DYNAHEADER */

/*
**  Drawing Statistics
*/
#ifdef DYNAHEADER
typedef void( FX_CALL *grSstPerfStats_fpt )(GrSstPerfStats_t *pStats );
#else

FX_ENTRY void FX_CALL
grSstPerfStats(GrSstPerfStats_t *pStats);

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grSstResetPerfStats_fpt )(void );
#else

FX_ENTRY void FX_CALL
grSstResetPerfStats(void);

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grResetTriStats_fpt )( );
#else

FX_ENTRY void FX_CALL
grResetTriStats();

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grTriStats_fpt )(FxU32 *trisProcessed, FxU32 *trisDrawn );
#else

FX_ENTRY void FX_CALL
grTriStats(FxU32 *trisProcessed, FxU32 *trisDrawn);

#endif /* DYNAHEADER */

/*
** Glide configuration and special effect maintenance functions
*/
#ifdef DYNAHEADER
typedef void( FX_CALL *grAlphaBlendFunction_fpt )( GrAlphaBlendFnc_t rgb_sf, GrAlphaBlendFnc_t rgb_df, GrAlphaBlendFnc_t alpha_sf, GrAlphaBlendFnc_t alpha_df );
#else

FX_ENTRY void FX_CALL
grAlphaBlendFunction(
                     GrAlphaBlendFnc_t rgb_sf,   GrAlphaBlendFnc_t rgb_df,
                     GrAlphaBlendFnc_t alpha_sf, GrAlphaBlendFnc_t alpha_df
                     );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grAlphaCombine_fpt )( GrCombineFunction_t function, GrCombineFactor_t factor, GrCombineLocal_t local, GrCombineOther_t other, FxBool invert );
#else

FX_ENTRY void FX_CALL
grAlphaCombine(
               GrCombineFunction_t function, GrCombineFactor_t factor,
               GrCombineLocal_t local, GrCombineOther_t other,
               FxBool invert
               );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grAlphaControlsITRGBLighting_fpt )( FxBool enable );
#else

FX_ENTRY void FX_CALL
grAlphaControlsITRGBLighting( FxBool enable );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grAlphaTestFunction_fpt )( GrCmpFnc_t function );
#else

FX_ENTRY void FX_CALL
grAlphaTestFunction( GrCmpFnc_t function );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grAlphaTestReferenceValue_fpt )( GrAlpha_t value );
#else

FX_ENTRY void FX_CALL
grAlphaTestReferenceValue( GrAlpha_t value );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grChromakeyMode_fpt )( GrChromakeyMode_t mode );
#else

FX_ENTRY void FX_CALL 
grChromakeyMode( GrChromakeyMode_t mode );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grChromakeyValue_fpt )( GrColor_t value );
#else

FX_ENTRY void FX_CALL 
grChromakeyValue( GrColor_t value );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grClipWindow_fpt )( FxU32 minx, FxU32 miny, FxU32 maxx, FxU32 maxy );
#else

FX_ENTRY void FX_CALL 
grClipWindow( FxU32 minx, FxU32 miny, FxU32 maxx, FxU32 maxy );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grColorCombine_fpt )( GrCombineFunction_t function, GrCombineFactor_t factor, GrCombineLocal_t local, GrCombineOther_t other, FxBool invert );
#else

FX_ENTRY void FX_CALL 
grColorCombine(
               GrCombineFunction_t function, GrCombineFactor_t factor,
               GrCombineLocal_t local, GrCombineOther_t other,
               FxBool invert );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grColorMask_fpt )( FxBool rgb, FxBool a );
#else

FX_ENTRY void FX_CALL
grColorMask( FxBool rgb, FxBool a );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grCullMode_fpt )( GrCullMode_t mode );
#else

FX_ENTRY void FX_CALL 
grCullMode( GrCullMode_t mode );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grConstantColorValue_fpt )( GrColor_t value );
#else

FX_ENTRY void FX_CALL 
grConstantColorValue( GrColor_t value );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grConstantColorValue4_fpt )( float a, float r, float g, float b );
#else

FX_ENTRY void FX_CALL 
grConstantColorValue4( float a, float r, float g, float b );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grDepthBiasLevel_fpt )( FxI16 level );
#else

FX_ENTRY void FX_CALL 
grDepthBiasLevel( FxI16 level );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grDepthBufferFunction_fpt )( GrCmpFnc_t function );
#else

FX_ENTRY void FX_CALL 
grDepthBufferFunction( GrCmpFnc_t function );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grDepthBufferMode_fpt )( GrDepthBufferMode_t mode );
#else

FX_ENTRY void FX_CALL 
grDepthBufferMode( GrDepthBufferMode_t mode );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grDepthMask_fpt )( FxBool mask );
#else

FX_ENTRY void FX_CALL 
grDepthMask( FxBool mask );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grDisableAllEffects_fpt )( void );
#else

FX_ENTRY void FX_CALL 
grDisableAllEffects( void );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grDitherMode_fpt )( GrDitherMode_t mode );
#else

FX_ENTRY void FX_CALL 
grDitherMode( GrDitherMode_t mode );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grFogColorValue_fpt )( GrColor_t fogcolor );
#else

FX_ENTRY void FX_CALL 
grFogColorValue( GrColor_t fogcolor );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grFogMode_fpt )( GrFogMode_t mode );
#else

FX_ENTRY void FX_CALL 
grFogMode( GrFogMode_t mode );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grFogTable_fpt )( const GrFog_t ft[GR_FOG_TABLE_SIZE] );
#else

FX_ENTRY void FX_CALL 
grFogTable( const GrFog_t ft[GR_FOG_TABLE_SIZE] );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grGammaCorrectionValue_fpt )( float value );
#else

FX_ENTRY void FX_CALL 
grGammaCorrectionValue( float value );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grSplash_fpt )(float x, float y, float width, float height, FxU32 frame );
#else

FX_ENTRY void FX_CALL
grSplash(float x, float y, float width, float height, FxU32 frame);

#endif /* DYNAHEADER */

/*
** texture mapping control functions
*/
#ifdef DYNAHEADER
typedef FxU32( FX_CALL *grTexCalcMemRequired_fpt )( GrLOD_t lodmin, GrLOD_t lodmax, GrAspectRatio_t aspect, GrTextureFormat_t fmt );
#else

FX_ENTRY FxU32 FX_CALL 
grTexCalcMemRequired(
                     GrLOD_t lodmin, GrLOD_t lodmax,
                     GrAspectRatio_t aspect, GrTextureFormat_t fmt);

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef FxU32( FX_CALL *grTexTextureMemRequired_fpt )( FxU32 evenOdd, GrTexInfo *info );
#else

FX_ENTRY FxU32 FX_CALL 
grTexTextureMemRequired( FxU32     evenOdd,
                                 GrTexInfo *info   );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef FxU32( FX_CALL *grTexMinAddress_fpt )( GrChipID_t tmu );
#else

FX_ENTRY FxU32 FX_CALL 
grTexMinAddress( GrChipID_t tmu );

#endif /* DYNAHEADER */


#ifdef DYNAHEADER
typedef FxU32( FX_CALL *grTexMaxAddress_fpt )( GrChipID_t tmu );
#else

FX_ENTRY FxU32 FX_CALL 
grTexMaxAddress( GrChipID_t tmu );

#endif /* DYNAHEADER */


#ifdef DYNAHEADER
typedef void( FX_CALL *grTexNCCTable_fpt )( GrChipID_t tmu, GrNCCTable_t table );
#else

FX_ENTRY void FX_CALL 
grTexNCCTable( GrChipID_t tmu, GrNCCTable_t table );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grTexSource_fpt )( GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo *info );
#else

FX_ENTRY void FX_CALL 
grTexSource( GrChipID_t tmu,
             FxU32      startAddress,
             FxU32      evenOdd,
             GrTexInfo  *info );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grTexClampMode_fpt )( GrChipID_t tmu, GrTextureClampMode_t s_clampmode, GrTextureClampMode_t t_clampmode );
#else

FX_ENTRY void FX_CALL 
grTexClampMode(
               GrChipID_t tmu,
               GrTextureClampMode_t s_clampmode,
               GrTextureClampMode_t t_clampmode
               );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grTexCombine_fpt )( GrChipID_t tmu, GrCombineFunction_t rgb_function, GrCombineFactor_t rgb_factor, GrCombineFunction_t alpha_function, GrCombineFactor_t alpha_factor, FxBool rgb_invert, FxBool alpha_invert );
#else

FX_ENTRY void FX_CALL 
grTexCombine(
             GrChipID_t tmu,
             GrCombineFunction_t rgb_function,
             GrCombineFactor_t rgb_factor, 
             GrCombineFunction_t alpha_function,
             GrCombineFactor_t alpha_factor,
             FxBool rgb_invert,
             FxBool alpha_invert
             );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grTexCombineFunction_fpt )( GrChipID_t tmu, GrTextureCombineFnc_t fnc );
#else

FX_ENTRY void FX_CALL 
grTexCombineFunction(
                     GrChipID_t tmu,
                     GrTextureCombineFnc_t fnc
                     );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grTexDetailControl_fpt )( GrChipID_t tmu, int lod_bias, FxU8 detail_scale, float detail_max );
#else

FX_ENTRY void FX_CALL 
grTexDetailControl(
                   GrChipID_t tmu,
                   int lod_bias,
                   FxU8 detail_scale,
                   float detail_max
                   );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grTexFilterMode_fpt )( GrChipID_t tmu, GrTextureFilterMode_t minfilter_mode, GrTextureFilterMode_t magfilter_mode );
#else

FX_ENTRY void FX_CALL 
grTexFilterMode(
                GrChipID_t tmu,
                GrTextureFilterMode_t minfilter_mode,
                GrTextureFilterMode_t magfilter_mode
                );

#endif /* DYNAHEADER */


#ifdef DYNAHEADER
typedef void( FX_CALL *grTexLodBiasValue_fpt )(GrChipID_t tmu, float bias );
#else

FX_ENTRY void FX_CALL 
grTexLodBiasValue(GrChipID_t tmu, float bias );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grTexDownloadMipMap_fpt )( GrChipID_t tmu, FxU32 startAddress, FxU32 evenOdd, GrTexInfo *info );
#else

FX_ENTRY void FX_CALL 
grTexDownloadMipMap( GrChipID_t tmu,
                     FxU32      startAddress,
                     FxU32      evenOdd,
                     GrTexInfo  *info );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grTexDownloadMipMapLevel_fpt )( GrChipID_t tmu, FxU32 startAddress, GrLOD_t thisLod, GrLOD_t largeLod, GrAspectRatio_t aspectRatio, GrTextureFormat_t format, FxU32 evenOdd, void *data );
#else

FX_ENTRY void FX_CALL 
grTexDownloadMipMapLevel( GrChipID_t        tmu,
                          FxU32             startAddress,
                          GrLOD_t           thisLod,
                          GrLOD_t           largeLod,
                          GrAspectRatio_t   aspectRatio,
                          GrTextureFormat_t format,
                          FxU32             evenOdd,
                          void              *data );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grTexDownloadMipMapLevelPartial_fpt )( GrChipID_t tmu, FxU32 startAddress, GrLOD_t thisLod, GrLOD_t largeLod, GrAspectRatio_t aspectRatio, GrTextureFormat_t format, FxU32 evenOdd, void *data, int start, int end );
#else

FX_ENTRY void FX_CALL 
grTexDownloadMipMapLevelPartial( GrChipID_t        tmu,
                                 FxU32             startAddress,
                                 GrLOD_t           thisLod,
                                 GrLOD_t           largeLod,
                                 GrAspectRatio_t   aspectRatio,
                                 GrTextureFormat_t format,
                                 FxU32             evenOdd,
                                 void              *data,
                                 int               start,
                                 int               end );

#endif /* DYNAHEADER */


#ifdef DYNAHEADER
typedef void( FX_CALL *ConvertAndDownloadRle_fpt )( GrChipID_t tmu, FxU32 startAddress, GrLOD_t thisLod, GrLOD_t largeLod, GrAspectRatio_t aspectRatio, GrTextureFormat_t format, FxU32 evenOdd, FxU8 *bm_data, long bm_h, FxU32 u0, FxU32 v0, FxU32 width, FxU32 height, FxU32 dest_width, FxU32 dest_height, FxU16 *tlut );
#else

FX_ENTRY void FX_CALL 
ConvertAndDownloadRle( GrChipID_t        tmu,
                        FxU32             startAddress,
                        GrLOD_t           thisLod,
                        GrLOD_t           largeLod,
                        GrAspectRatio_t   aspectRatio,
                        GrTextureFormat_t format,
                        FxU32             evenOdd,
                        FxU8              *bm_data,
                        long              bm_h,
                        FxU32             u0,
                        FxU32             v0,
                        FxU32             width,
                        FxU32             height,
                        FxU32             dest_width,
                        FxU32             dest_height,
                        FxU16             *tlut);

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grCheckForRoom_fpt )(FxI32 n );
#else

FX_ENTRY void FX_CALL 
grCheckForRoom(FxI32 n);

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grTexDownloadTable_fpt )( GrChipID_t tmu, GrTexTable_t type, void *data );
#else

FX_ENTRY void FX_CALL
grTexDownloadTable( GrChipID_t   tmu,
                    GrTexTable_t type, 
                    void         *data );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grTexDownloadTablePartial_fpt )( GrChipID_t tmu, GrTexTable_t type, void *data, int start, int end );
#else

FX_ENTRY void FX_CALL
grTexDownloadTablePartial( GrChipID_t   tmu,
                           GrTexTable_t type, 
                           void         *data,
                           int          start,
                           int          end );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grTexMipMapMode_fpt )( GrChipID_t tmu, GrMipMapMode_t mode, FxBool lodBlend );
#else

FX_ENTRY void FX_CALL 
grTexMipMapMode( GrChipID_t     tmu, 
                 GrMipMapMode_t mode,
                 FxBool         lodBlend );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grTexMultibase_fpt )( GrChipID_t tmu, FxBool enable );
#else

FX_ENTRY void FX_CALL 
grTexMultibase( GrChipID_t tmu,
                FxBool     enable );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grTexMultibaseAddress_fpt )( GrChipID_t tmu, GrTexBaseRange_t range, FxU32 startAddress, FxU32 evenOdd, GrTexInfo *info );
#else

FX_ENTRY void FX_CALL
grTexMultibaseAddress( GrChipID_t       tmu,
                       GrTexBaseRange_t range,
                       FxU32            startAddress,
                       FxU32            evenOdd,
                       GrTexInfo        *info );

#endif /* DYNAHEADER */

/*
** utility texture functions
*/
#ifdef DYNAHEADER
typedef GrMipMapId_t( FX_CALL *guTexAllocateMemory_fpt )( GrChipID_t tmu, FxU8 odd_even_mask, int width, int height, GrTextureFormat_t fmt, GrMipMapMode_t mm_mode, GrLOD_t smallest_lod, GrLOD_t largest_lod, GrAspectRatio_t aspect, GrTextureClampMode_t s_clamp_mode, GrTextureClampMode_t t_clamp_mode, GrTextureFilterMode_t minfilter_mode, GrTextureFilterMode_t magfilter_mode, float lod_bias, FxBool trilinear );
#else

FX_ENTRY GrMipMapId_t FX_CALL 
guTexAllocateMemory(
                    GrChipID_t tmu,
                    FxU8 odd_even_mask,
                    int width, int height,
                    GrTextureFormat_t fmt,
                    GrMipMapMode_t mm_mode,
                    GrLOD_t smallest_lod, GrLOD_t largest_lod,
                    GrAspectRatio_t aspect,
                    GrTextureClampMode_t s_clamp_mode,
                    GrTextureClampMode_t t_clamp_mode,
                    GrTextureFilterMode_t minfilter_mode,
                    GrTextureFilterMode_t magfilter_mode,
                    float lod_bias,
                    FxBool trilinear
                    );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef FxBool( FX_CALL *guTexChangeAttributes_fpt )( GrMipMapId_t mmid, int width, int height, GrTextureFormat_t fmt, GrMipMapMode_t mm_mode, GrLOD_t smallest_lod, GrLOD_t largest_lod, GrAspectRatio_t aspect, GrTextureClampMode_t s_clamp_mode, GrTextureClampMode_t t_clamp_mode, GrTextureFilterMode_t minFilterMode, GrTextureFilterMode_t magFilterMode );
#else

FX_ENTRY FxBool FX_CALL 
guTexChangeAttributes(
                      GrMipMapId_t mmid,
                      int width, int height,
                      GrTextureFormat_t fmt,
                      GrMipMapMode_t mm_mode,
                      GrLOD_t smallest_lod, GrLOD_t largest_lod,
                      GrAspectRatio_t aspect,
                      GrTextureClampMode_t s_clamp_mode,
                      GrTextureClampMode_t t_clamp_mode,
                      GrTextureFilterMode_t minFilterMode,
                      GrTextureFilterMode_t magFilterMode
                      );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *guTexCombineFunction_fpt )( GrChipID_t tmu, GrTextureCombineFnc_t fnc );
#else

FX_ENTRY void FX_CALL 
guTexCombineFunction(
                     GrChipID_t tmu,
                     GrTextureCombineFnc_t fnc
                     );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef GrMipMapId_t( FX_CALL *guTexGetCurrentMipMap_fpt )( GrChipID_t tmu );
#else

FX_ENTRY GrMipMapId_t FX_CALL 
guTexGetCurrentMipMap( GrChipID_t tmu );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef GrMipMapInfo *( FX_CALL *guTexGetMipMapInfo_fpt )( GrMipMapId_t mmid );
#else

FX_ENTRY GrMipMapInfo * FX_CALL 
guTexGetMipMapInfo( GrMipMapId_t mmid );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef FxU32( FX_CALL *guTexMemQueryAvail_fpt )( GrChipID_t tmu );
#else

FX_ENTRY FxU32 FX_CALL 
guTexMemQueryAvail( GrChipID_t tmu );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *guTexMemReset_fpt )( void );
#else

FX_ENTRY void FX_CALL 
guTexMemReset( void );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *guTexDownloadMipMap_fpt )( GrMipMapId_t mmid, const void *src, const GuNccTable *table );
#else

FX_ENTRY void FX_CALL 
guTexDownloadMipMap(
                    GrMipMapId_t mmid,
                    const void *src,
                    const GuNccTable *table
                    );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *guTexDownloadMipMapLevel_fpt )( GrMipMapId_t mmid, GrLOD_t lod, const void **src );
#else

FX_ENTRY void FX_CALL 
guTexDownloadMipMapLevel(
                         GrMipMapId_t mmid,
                         GrLOD_t lod,
                         const void **src
                         );

#endif /* DYNAHEADER */
#ifdef DYNAHEADER
typedef void( FX_CALL *guTexSource_fpt )( GrMipMapId_t id );
#else

FX_ENTRY void FX_CALL 
guTexSource( GrMipMapId_t id );

#endif /* DYNAHEADER */

/*
** linear frame buffer functions
*/

#ifdef DYNAHEADER
typedef FxBool( FX_CALL *grLfbLock_fpt )( GrLock_t type, GrBuffer_t buffer, GrLfbWriteMode_t writeMode, GrOriginLocation_t origin, FxBool pixelPipeline, GrLfbInfo_t *info );
#else

FX_ENTRY FxBool FX_CALL
grLfbLock( GrLock_t type, GrBuffer_t buffer, GrLfbWriteMode_t writeMode,
           GrOriginLocation_t origin, FxBool pixelPipeline, 
           GrLfbInfo_t *info );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef FxBool( FX_CALL *grLfbUnlock_fpt )( GrLock_t type, GrBuffer_t buffer );
#else

FX_ENTRY FxBool FX_CALL
grLfbUnlock( GrLock_t type, GrBuffer_t buffer );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grLfbConstantAlpha_fpt )( GrAlpha_t alpha );
#else

FX_ENTRY void FX_CALL 
grLfbConstantAlpha( GrAlpha_t alpha );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grLfbConstantDepth_fpt )( FxU16 depth );
#else

FX_ENTRY void FX_CALL 
grLfbConstantDepth( FxU16 depth );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grLfbWriteColorSwizzle_fpt )(FxBool swizzleBytes, FxBool swapWords );
#else

FX_ENTRY void FX_CALL 
grLfbWriteColorSwizzle(FxBool swizzleBytes, FxBool swapWords);

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grLfbWriteColorFormat_fpt )(GrColorFormat_t colorFormat );
#else

FX_ENTRY void FX_CALL
grLfbWriteColorFormat(GrColorFormat_t colorFormat);

#endif /* DYNAHEADER */


#ifdef DYNAHEADER
typedef FxBool( FX_CALL *grLfbWriteRegion_fpt )( GrBuffer_t dst_buffer, FxU32 dst_x, FxU32 dst_y, GrLfbSrcFmt_t src_format, FxU32 src_width, FxU32 src_height, FxI32 src_stride, void *src_data );
#else

FX_ENTRY FxBool FX_CALL
grLfbWriteRegion( GrBuffer_t dst_buffer, 
                  FxU32 dst_x, FxU32 dst_y, 
                  GrLfbSrcFmt_t src_format, 
                  FxU32 src_width, FxU32 src_height, 
                  FxI32 src_stride, void *src_data );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef FxBool( FX_CALL *grLfbReadRegion_fpt )( GrBuffer_t src_buffer, FxU32 src_x, FxU32 src_y, FxU32 src_width, FxU32 src_height, FxU32 dst_stride, void *dst_data );
#else

FX_ENTRY FxBool FX_CALL
grLfbReadRegion( GrBuffer_t src_buffer,
                 FxU32 src_x, FxU32 src_y,
                 FxU32 src_width, FxU32 src_height,
                 FxU32 dst_stride, void *dst_data );

#endif /* DYNAHEADER */


/*
**  Antialiasing Functions
*/
#ifdef DYNAHEADER
typedef void( FX_CALL *grAADrawLine_fpt )(const GrVertex *v1, const GrVertex *v2 );
#else

FX_ENTRY void FX_CALL
grAADrawLine(const GrVertex *v1, const GrVertex *v2);

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grAADrawPoint_fpt )(const GrVertex *pt );
#else

FX_ENTRY void FX_CALL
grAADrawPoint(const GrVertex *pt );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grAADrawPolygon_fpt )(const int nverts, const int ilist[], const GrVertex vlist[] );
#else

FX_ENTRY void FX_CALL
grAADrawPolygon(const int nverts, const int ilist[], const GrVertex vlist[]);

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grAADrawPolygonVertexList_fpt )(const int nverts, const GrVertex vlist[] );
#else

FX_ENTRY void FX_CALL
grAADrawPolygonVertexList(const int nverts, const GrVertex vlist[]);

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grAADrawTriangle_fpt )( const GrVertex *a, const GrVertex *b, const GrVertex *c, FxBool ab_antialias, FxBool bc_antialias, FxBool ca_antialias );
#else

FX_ENTRY void FX_CALL
grAADrawTriangle(
                 const GrVertex *a, const GrVertex *b, const GrVertex *c,
                 FxBool ab_antialias, FxBool bc_antialias, FxBool ca_antialias
                 );

#endif /* DYNAHEADER */

/*
** glide management functions
*/
#ifdef DYNAHEADER
typedef void( FX_CALL *grGlideInit_fpt )( void );
#else

FX_ENTRY void FX_CALL
grGlideInit( void );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grGlideShutdown_fpt )( void );
#else

FX_ENTRY void FX_CALL
grGlideShutdown( void );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grGlideGetVersion_fpt )( char version[80] );
#else

FX_ENTRY void FX_CALL
grGlideGetVersion( char version[80] );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grGlideGetState_fpt )( GrState *state );
#else

FX_ENTRY void FX_CALL
grGlideGetState( GrState *state );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grGlideSetState_fpt )( const GrState *state );
#else

FX_ENTRY void FX_CALL
grGlideSetState( const GrState *state );

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grGlideShamelessPlug_fpt )(const FxBool on );
#else

FX_ENTRY void FX_CALL
grGlideShamelessPlug(const FxBool on);

#endif /* DYNAHEADER */

#ifdef DYNAHEADER
typedef void( FX_CALL *grHints_fpt )(GrHint_t hintType, FxU32 hintMask );
#else

FX_ENTRY void FX_CALL
grHints(GrHint_t hintType, FxU32 hintMask);

#endif /* DYNAHEADER */

#endif /* FX_GLIDE_NO_FUNC_PROTO */

#ifdef __cplusplus
}
#endif

#include <glideutl.h>



#if defined DYNAHEADER && ! defined DYNAHEADER_CREATE_STORAGE 
extern ConvertAndDownloadRle_fpt ConvertAndDownloadRle;
extern grAADrawLine_fpt grAADrawLine;
extern grAADrawPoint_fpt grAADrawPoint;
extern grAADrawPolygon_fpt grAADrawPolygon;
extern grAADrawPolygonVertexList_fpt grAADrawPolygonVertexList;
extern grAADrawTriangle_fpt grAADrawTriangle;
extern grAlphaBlendFunction_fpt grAlphaBlendFunction;
extern grAlphaCombine_fpt grAlphaCombine;
extern grAlphaControlsITRGBLighting_fpt grAlphaControlsITRGBLighting;
extern grAlphaTestFunction_fpt grAlphaTestFunction;
extern grAlphaTestReferenceValue_fpt grAlphaTestReferenceValue;
extern grBufferClear_fpt grBufferClear;
extern grBufferNumPending_fpt grBufferNumPending;
extern grBufferSwap_fpt grBufferSwap;
extern grCheckForRoom_fpt grCheckForRoom;
extern grChromakeyMode_fpt grChromakeyMode;
extern grChromakeyValue_fpt grChromakeyValue;
extern grClipWindow_fpt grClipWindow;
extern grColorCombine_fpt grColorCombine;
extern grColorMask_fpt grColorMask;
extern grConstantColorValue_fpt grConstantColorValue;
extern grConstantColorValue4_fpt grConstantColorValue4;
extern grCullMode_fpt grCullMode;
extern grDepthBiasLevel_fpt grDepthBiasLevel;
extern grDepthBufferFunction_fpt grDepthBufferFunction;
extern grDepthBufferMode_fpt grDepthBufferMode;
extern grDepthMask_fpt grDepthMask;
extern grDisableAllEffects_fpt grDisableAllEffects;
extern grDitherMode_fpt grDitherMode;
extern grDrawLine_fpt grDrawLine;
extern grDrawPlanarPolygon_fpt grDrawPlanarPolygon;
extern grDrawPlanarPolygonVertexList_fpt grDrawPlanarPolygonVertexList;
extern grDrawPoint_fpt grDrawPoint;
extern grDrawPolygon_fpt grDrawPolygon;
extern grDrawPolygonVertexList_fpt grDrawPolygonVertexList;
extern grDrawTriangle_fpt grDrawTriangle;
extern grErrorSetCallback_fpt grErrorSetCallback;
extern grFogColorValue_fpt grFogColorValue;
extern grFogMode_fpt grFogMode;
extern grFogTable_fpt grFogTable;
extern grGammaCorrectionValue_fpt grGammaCorrectionValue;
extern grGlideGetState_fpt grGlideGetState;
extern grGlideGetVersion_fpt grGlideGetVersion;
extern grGlideInit_fpt grGlideInit;
extern grGlideSetState_fpt grGlideSetState;
extern grGlideShamelessPlug_fpt grGlideShamelessPlug;
extern grGlideShutdown_fpt grGlideShutdown;
extern grHints_fpt grHints;
extern grLfbConstantAlpha_fpt grLfbConstantAlpha;
extern grLfbConstantDepth_fpt grLfbConstantDepth;
extern grLfbLock_fpt grLfbLock;
extern grLfbReadRegion_fpt grLfbReadRegion;
extern grLfbUnlock_fpt grLfbUnlock;
extern grLfbWriteColorFormat_fpt grLfbWriteColorFormat;
extern grLfbWriteColorSwizzle_fpt grLfbWriteColorSwizzle;
extern grLfbWriteRegion_fpt grLfbWriteRegion;
extern grRenderBuffer_fpt grRenderBuffer;
extern grResetTriStats_fpt grResetTriStats;
extern grSplash_fpt grSplash;
extern grSstControl_fpt grSstControl;
extern grSstIdle_fpt grSstIdle;
extern grSstIsBusy_fpt grSstIsBusy;
extern grSstOrigin_fpt grSstOrigin;
extern grSstPerfStats_fpt grSstPerfStats;
extern grSstQueryBoards_fpt grSstQueryBoards;
extern grSstQueryHardware_fpt grSstQueryHardware;
extern grSstResetPerfStats_fpt grSstResetPerfStats;
extern grSstScreenHeight_fpt grSstScreenHeight;
extern grSstScreenWidth_fpt grSstScreenWidth;
extern grSstSelect_fpt grSstSelect;
extern grSstStatus_fpt grSstStatus;
extern grSstVRetraceOn_fpt grSstVRetraceOn;
extern grSstVideoLine_fpt grSstVideoLine;
extern grSstWinClose_fpt grSstWinClose;
extern grSstWinOpen_fpt grSstWinOpen;
extern grTexCalcMemRequired_fpt grTexCalcMemRequired;
extern grTexClampMode_fpt grTexClampMode;
extern grTexCombine_fpt grTexCombine;
extern grTexCombineFunction_fpt grTexCombineFunction;
extern grTexDetailControl_fpt grTexDetailControl;
extern grTexDownloadMipMap_fpt grTexDownloadMipMap;
extern grTexDownloadMipMapLevel_fpt grTexDownloadMipMapLevel;
extern grTexDownloadMipMapLevelPartial_fpt grTexDownloadMipMapLevelPartial;
extern grTexDownloadTable_fpt grTexDownloadTable;
extern grTexDownloadTablePartial_fpt grTexDownloadTablePartial;
extern grTexFilterMode_fpt grTexFilterMode;
extern grTexLodBiasValue_fpt grTexLodBiasValue;
extern grTexMaxAddress_fpt grTexMaxAddress;
extern grTexMinAddress_fpt grTexMinAddress;
extern grTexMipMapMode_fpt grTexMipMapMode;
extern grTexMultibase_fpt grTexMultibase;
extern grTexMultibaseAddress_fpt grTexMultibaseAddress;
extern grTexNCCTable_fpt grTexNCCTable;
extern grTexSource_fpt grTexSource;
extern grTexTextureMemRequired_fpt grTexTextureMemRequired;
extern grTriStats_fpt grTriStats;
extern guTexAllocateMemory_fpt guTexAllocateMemory;
extern guTexChangeAttributes_fpt guTexChangeAttributes;
extern guTexCombineFunction_fpt guTexCombineFunction;
extern guTexDownloadMipMap_fpt guTexDownloadMipMap;
extern guTexDownloadMipMapLevel_fpt guTexDownloadMipMapLevel;
extern guTexGetCurrentMipMap_fpt guTexGetCurrentMipMap;
extern guTexGetMipMapInfo_fpt guTexGetMipMapInfo;
extern guTexMemQueryAvail_fpt guTexMemQueryAvail;
extern guTexMemReset_fpt guTexMemReset;
extern guTexSource_fpt guTexSource;
extern FxBool LoadGlide();
extern "C" { void __cdecl UnloadGlide(); }
#endif /* DYNAHEADER */

#endif /* __GLIDE_H__ */
