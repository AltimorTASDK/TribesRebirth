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
#ifndef RLEX_H	
#define RLEX_H

#include "redline.h"
#include "rlfifoex.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Specular extension
 */

/* Extension constants */

typedef enum {
    V_SPECULAR_ENABLE=1,
    V_SPECULAR_DISABLE=0
} v_SpecularEnable;


/* Typedefs for extension functions */

typedef	vl_error (V_DLLIMPORT *VL_SetSpecularEnable_Type)(v_cmdbuffer *cmdbuffer, v_u32 specular_enable);

typedef vl_error (V_DLLIMPORT *VL_SetSpecularColorRGB_Type)(v_cmdbuffer *cmdbuffer, v_u32 specular_color);

typedef vl_error (V_DLLIMPORT *VL_SetSpecularColorBGR_Type)(v_cmdbuffer *cmdbuffer, v_u32 specular_color);


#ifdef __cplusplus
}
#endif

#endif /*RLEX_H*/

