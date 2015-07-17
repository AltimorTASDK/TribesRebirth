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
#ifndef RLFIFOEX_H	
#define RLFIFOEX_H

#include "rlfifo.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Specular extension
 */

/* Vertex structure definitions */

/* Note:
   x,y,i,r,g,b,a,f terms are s15.16 fixed point
   u,v terms are 32 bit signed with binary point determined by
     value of UMultiplier/VMultiplier
   q terms are 8.24 unsigned, with the high 8 bits zero if at all
     possible
   bgr or rgb terms have 3 x 8bit RGB or BGR channels in the low 24 bits
*/

typedef struct {
       V_PACKED    argb;
       V_PACKED    fs; /* Fog Rs Gs Bs */
       V_SIGNED    x;
       V_SIGNED    y;
       V_UNSIGNED  z;
       V_SIGNED    u;
       V_SIGNED    v;
       V_UNSIGNED  q;
} v_kasfxyzuvq;

typedef struct {
       V_PACKED    rgb;
       V_SIGNED    s;
       V_SIGNED    x;
       V_SIGNED    y;
       V_UNSIGNED  z;
       V_SIGNED    u;
       V_SIGNED    v;
       V_UNSIGNED  q;
} v_ksxyzuvq;

typedef struct {
       V_PACKED    rgb;
       V_SIGNED    s;
       V_SIGNED    x;
       V_SIGNED    y;
       V_SIGNED    u;
       V_SIGNED	   v;
       V_UNSIGNED  q;
} v_ksxyuvq;


/*
 * Corresponding vertex types codes
 * The following letters represent the packed attributes:
 *     S	-R2G2B2 (specular)
 */

#define V_FIFO_KaSFXYZUVQ       32
#define V_FIFO_KSXYUVQ          33
#define V_FIFO_KSXYZUVQ         34


#define V_FIFO_SPECCOLORRGB_REG                       92
#define V_FIFO_SPECCOLORRGB_OP                         2
#define V_FIFO_SPECCOLORRGB_MASK                       0
#define V_FIFO_SPECCOLORRGB_SHIFT                      0
#define V_FIFO_SPECCOLORRGB																\
   V_FIFO_SSMSK(V_FIFO_SPECCOLORRGB_MASK)+V_FIFO_SSOP(V_FIFO_SPECCOLORRGB_OP)			\
   +V_FIFO_SSSHFT(V_FIFO_SPECCOLORRGB_SHIFT)+V_FIFO_SSREG(V_FIFO_SPECCOLORRGB_REG)


#ifdef __cplusplus
}
#endif

#endif /*RLFIFOEX_H*/

