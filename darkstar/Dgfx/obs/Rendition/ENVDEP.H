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
#ifndef ENVDEP_H
#define ENVDEP_H

#include <windows.h>

/**********/
/* TYPES  */
/**********/

/* The following are the only things we define that aren't
   prefixed with V or v */

#ifndef FALSE
#define FALSE   0
#endif /*FALSE*/

#ifndef TRUE
#define TRUE    !FALSE
#endif /*TRUE*/

#ifndef NULL
#define NULL    0
#endif

/* Use these types religiously to prevent problems when the length of
 * the data is important (which is most of the time)
 */
typedef unsigned long   v_u32;
typedef unsigned short  v_u16;
typedef unsigned char   v_u8;
typedef signed long     v_s32;
typedef signed short    v_s16;
typedef signed char     v_s8;


/*
 * Generic Defines
 */
#define V_PAGE_SIZE                     4096
#define V_WORDS_PER_PAGE                1024
#define V_PAGE_SHIFT                    12
#define V_PAGE_ADDR_BITS                0xFFFFF000  /* bits 12 .. 31 */

#define V_PAGE_PRESENT                  0x1
#define V_CARRY_BIT                     0x1     /* bit 0 of FLAGS reg */

#define V_PAGES_LOCKED                  0x101
#define V_PAGES_UNLOCKED                0x102

/* physical address */
typedef v_u32 v_physaddr;
#define v_pa  v_physaddr;

/*
 * location in board memory (typically 32b aligned)
 */
typedef v_u32 v_foreignAddress;
#define v_fa  v_foreignAddress

/**********/
/* MEMORY */
/**********/

/* Assert */
#ifdef _DEBUG
#include <assert.h>
#define V_ASSERT(x) assert((x))
#else
#define V_ASSERT(x)
#endif

/*********************/
/* FUNCTION PREFIXES */
/*********************/

/* Exported functions */

/* use this when explicitly declaring CDECL-interface funcs */
#define V_CDECL __cdecl

/* use this for all exported functions */
#define V_EXPORTED

/*********/
/* PORTS */
/*********/

typedef struct {
        v_u32      *addr;
        v_u32      port:2;
        v_u32      len:22;
        v_u32      reserved:7;
        v_u32      link:1;
} v_env_dmalist_entry;

#define V_DLLEXPORT	__cdecl
#define V_DLLIMPORT	__cdecl

#endif /*ENVDEP_H*/

