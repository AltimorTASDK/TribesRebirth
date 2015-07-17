;================================================================
;
; $Workfile:   rn_persp.asm  $
; $Version$
; $Revision: $
;
; DESCRIPTION:
;
; (c) Copyright 1997 Dynamix Inc.   All rights reserved.
;================================================================

.486p
.MODEL FLAT, SYSCALL

IFDEF __MASM__
  ARG equ
ENDIF

.DATA

include g_contxt.inc
INCLUDE g_macros.inc

translucencyMapPatch    equ     01010101h
textureMapPatch         equ     01010101h
ssFractionPatch         equ     01010101h
ssWholePatch            equ     01h
hazeMaskPatch           equ     01010101h
hazeStepPatch           equ     01010101h
hazeStepPatchx2         equ     02020202h
; floating point constants:
fpcnst65536             dd      65536.0
fpcnst1div65536         dd      1.52587890625e-5
fpcnst1                 dd      1.0

iteratorNew      dd  512
iteratorNew_new  dd  512
_esp             dd  0
uFracStep        dd  ?
vFracStep        dd  ?
xval             dd  ?
yval             dd  ?
uRol             dd  ?
vRol             dd  ?
innerLoopJump    dd  ?
nextSubSpanJump  dd  ?

currShadeWVal    db  ?
gouraudShadeFPatch dd ?
gouraudShadeWPatch dd ?

coordStarts        dd 320 dup (1.0)

uFracStepPatch  equ 01010101h
vFracStepPatch  equ 01010101h
fsuPatch        equ 01010101h
fssPatch        equ 01010101h
fsswPatch       equ 01h
fsvPatch        equ 01010101h
uMaskPatch      equ 01010101h
vMaskPatch      equ 01010101h

startCoordCalcJump  dd ?
startCoordDoneJump   dd ?
uFracStart           dd ?
vFracStart           dd ?
textureMapStart      dd ?
spanPtr              dd ?


FPNewInfo STRUC
fNum        dd  ?
fInverse    dd  ?
FPNewInfo ENDS

intToFloatTable     dd 1280 dup (?)
spanMaxLenTable     dd 1280 dup (?)
curSubDivSize       dd 0

intToFloatIterator  dd 1279

fpNewInfoTable      FPNewInfo 513 dup ({})

ScaleTexCoords MACRO
        fild    gfxPDC.pdc_textureMapStride
        fld     gfxPDC.pdc_dudx
        fmul    st, st(1)
        fld     gfxPDC.pdc_dudy
        fmul    st, st(2)
        fxch    st(2)
        fmul    gfxPDC.pdc_u0
        fild    gfxPDC.pdc_textureMapHeight
        fld     gfxPDC.pdc_dvdx
        fmul    st, st(1)
        fld     gfxPDC.pdc_dvdy
        fmul    st, st(2)
        fxch    st(2)
        fmul    gfxPDC.pdc_v0
        fxch    st(1)
        fstp    gfxPDC.pdc_dvdx
        fstp    gfxPDC.pdc_v0
        fstp    gfxPDC.pdc_dvdy
        fstp    gfxPDC.pdc_u0
        fstp    gfxPDC.pdc_dudx
        fstp    gfxPDC.pdc_dudy
ENDM

CalcSubDivTable MACRO
        mov     eax, gfxPDC.pdc_subDivSize
        cmp     eax, curSubDivSize
        je      @@noNewSubDivSize
        mov     curSubDivSize, eax
        push    ebx
        push    edi
        lea     edx, spanMaxLenTable
        lea     ebx, curSubDivSize  ; end of the table
        mov     ecx, eax
@@subDivTableLoop:
        mov     [edx], ecx
        add     edx, 4
        dec     ecx
        jnz     @@notZeroSubLen
        mov     ecx, eax
@@notZeroSubLen:
        cmp     edx, ebx
        jb      @@subDivTableLoop

        pop     edi
        pop     ebx
@@noNewSubDivSize:
ENDM

CalcRMod MACRO
@@crmLoop:
        cmp     edx, curSubDivSize
        jl      @@crmDone
        sub     edx, curSubDivSize
        jmp     @@crmLoop
@@crmDone:
        neg     edx
        add     edx, curSubDivSize
ENDM
        
CalcInfo MACRO
        finit
        mov    eax, iteratorNew
        cmp    eax, 0
        je     @@endCalcTableMacro
@@infoLoop:
        lea    edx, [fpNewInfoTable + eax*8]

        fld1
        fild   iteratorNew
        fst    (FPNewInfo PTR [edx]).fNum
        fdivp  st(1), st
        fstp   (FPNewInfo PTR [edx]).fInverse
        dec    eax
        dec    iteratorNew
        jnz    @@infoLoop
        fld1
        lea    edx, [fpNewInfoTable]
        fst    (FPNewInfo PTR [edx]).fNum  ; for 0... don't want any troubles.
        fstp   (FPNewInfo PTR [edx]).fInverse 
@@tableCalc:
        mov     eax, intToFloatIterator
        fild    intToFloatIterator
        dec     intToFloatIterator
        fstp    intToFloatTable[eax*4]
        jns     @@tableCalc
@@endCalcTableMacro:
ENDM



.CODE

; codes for function names...
; UV - textured
; W - perspective corrected
; S - shade interpolated
; C - constant shaded
; A - alpha (translucent)
; T - transparent
; R - wrapped
; 6 - P6 instructions used

; so a renderer for shade interpolated perspective correction would
; be GFXRenderUVWS

;================================================================
; NAME
;   outer_loop_no_interp_shade 
;
; DESCRIPTION
;   outer loop of core render loop... there are two of these,
;   one with interpolated S and one without.
;
;================================================================

outer_loop_no_interp_shade PROC C
        PUBLIC C outer_loop_no_interp_shade
        lea     eax, @@spanEntry
        lea     edx, @@spanLoop
        mov     gfxPDC.pdc_rendSpanEntry, eax
        mov     nextSubSpanJump, edx
        ret
@@spanEntry:
        ; on entry, edi holds xLeft, esi holds integer count
        ; eax holds y value.
        mov     xval, edi
        mov     yval, eax

        fild    xval
        fld     gfxPDC.pdc_dudx
        fmul    st, st(1)
        fld     gfxPDC.pdc_dvdx
        fmul    st, st(2)
        fild    yval
        fxch    st(3)
        fmul    gfxPDC.pdc_dwdx
        ; st = x*dwdx, x*dvdx, x*dudx, y
        fld     gfxPDC.pdc_dudy
        fmul    st, st(4)
        fld     gfxPDC.pdc_dvdy
        fmul    st, st(5)
        fld     gfxPDC.pdc_dwdy
        fmulp   st(6), st
        fld     gfxPDC.pdc_u0
        ; st = u0, dvdy * y, dudy * y, dwdx * x, dvdx * x, dudx * x, dwdy * y
        faddp   st(5), st
        fld     gfxPDC.pdc_v0
        faddp   st(4), st
        fld     gfxPDC.pdc_w0
        faddp   st(3), st
        ; st = dvdy * y, dudy * y, dwdx *x+w0, dvdx *x+v0, dudx*x+u0, dwdy *y
        faddp   st(3), st
        faddp   st(3), st
        faddp   st(3), st
        fxch    st(1)
        fxch    st(2)
        ; st = w, v, u

        ; stack = wstart, vstart, ustart
        fld     fpcnst1
        fdiv    st, st(1)

        ; st(0)    st(1)    st(2)    st(3)    st(4)    st(5)
        ; FIX1/w   w        v        u

        ; spanLen in esi
        mov     edx, edi ; edx = start
        add     edi, gfxPDC.pdc_rowAddr
        mov     ecx, gfxPDC.pdc_subDivSize
        cmp     esi, ecx
        jle     @@spanLess
        mov     ecx, spanMaxLenTable[edx*4]
        mov     edx, ecx
        cmp     ecx, 3
        jg      @@calcSpanLen
        ; uh oh... span is too short (can't be less than 4 pixels
        ; for the start span.
        add     ecx, gfxPDC.pdc_subDivSize
        cmp     esi, ecx
        jle     @@spanLess ; here we go
        mov     edx, ecx ; edx is index into the fpNewInfoTable - determines how far we
                         ; step u, v, w (used to hit end point at end of span)
        jmp     @@calcSpanLen
@@spanLess:
        mov     ecx, esi ; subSpanLen = spanLen
        lea     edx, [esi-1] ; last span, so pretend it's one shorter.
@@calcSpanLen:
        sub     esi, ecx

        ; subSpanLen in ecx, spanLen remaining in esi, start in edi

        ; now we calculate start u and start v, leaving them on the stack
        ; and forward step w by spanLen, then do the divide

        fld     (FPNewInfo PTR [edx * 8 + fpNewInfoTable]).fNum
        fld     st(1)
                            ; st(0)    st(1)    st(2)    st(3)    st(4)    st(5)
                            ; FX1/w    spLen    FX1/w    w        v        u
        fmul    st, st(5)   ; startU   spLen    FX1/w    w        v        u
        fxch    st(1)       ; spLen    startU   FX1/w          "
        fmul    gfxPDC.pdc_dwdx ; dW     startU   FX1/w        "
        fxch    st(2)         ;1/w     startU   dW             "
        fmul    st, st(4)   ; startV   startU   dw             "
        fxch    st(2)       ; dw       startU   startV         "
        faddp   st(3), st   ; startU   startV   w+dw     v        u
        fld     fpcnst1 ; FIX1     startU   startV   w+dw     v        u
        fdiv    st, st(3)   ; FX1/(w+dw) startU startV   w+dw     v        u
@@spanLoop:
        ; we have subSpanLen in ecx, span length remaining in esi, start in edi
        ; calculate new spanLen and new subSpanLen
        mov     ebx, gfxPDC.pdc_subDivSize
        mov     eax, esi
        mov     ebp, ecx
        cmp     esi, 0
        jle     @@zeroSubSpanLen
        cmp     esi, ebx
        jle     @@spanLessInLoop
        mov     edx, ebx ; newSubSpanLen = subDivSize
        sub     eax, ebx
        jmp     @@calcSpanRemaining
@@spanLessInLoop:
        lea     edx, [esi - 1] ; newSubSpanLen (for calc w) = spanLen - 1
        mov     ebx, esi ; actual newSubSpanLen = spanLen
        mov     eax, 0
@@calcSpanRemaining:
        ; prestep w
        fld     (FPNewInfo PTR [edx * 8 + fpNewInfoTable]).fNum
        fmul    gfxPDC.pdc_dwdx
        faddp   st(4), st
        jmp     @@saveSpanInfo
@@zeroSubSpanLen:
        mov     ebx, 0
        dec     ebp ; this span we're drawing is going to be the last..
@@saveSpanInfo:
        mov     gfxPDC.pdc_spanLen, eax
        mov     gfxPDC.pdc_subSpanLen, ebx

        ; now let's get ready to draw the span...

                            ; st(0)    st(1)    st(2)    st(3)    st(4)    st(5)    st(6)    st(7)
                            ; FX1/(w+dw) startU startV   w+dw     v        u
        fxch    st(1)       ; startU   1/We     startV   We       V        U
        fist    gfxPDC.pdc_fu
        fxch    st(2)       ; startV   1/We     startU   We       V        U
        fist    gfxPDC.pdc_fv
        fld     (FPNewInfo PTR [ebp * 8 + fpNewInfoTable]).fNum
        fld     st          ; subSplen subSplen startV   1/We     startU   We       V        U
        fmul    gfxPDC.pdc_dudx  ; du       subSplen  ""
        fxch    st(1)
        fmul    gfxPDC.pdc_dvdx  ; dv       du       startV   1/We     startU   We       V        U
        fxch    st(1)       ; du       dv   ""
        faddp   st(7), st   ; dv       startV   1/We     startU   We       V        U
        faddp   st(5), st   ; startV   1/We     startU   We       Ve       Ue
        ; now calculate new starts and deltas
        fld     st(1)       ; 1/We     startV   1/We     startU   We       Ve       Ue
        fmul    st, st(6)   ; endU     startV   1/We     startU   We       Ve       Ue
        fxch    st(2)
        fmul    st, st(5)   ; endV     startV   endU     startU   We       Ve       Ue
        fxch    st(2)
        fsubr   st(3), st   ; endU     startV   endV     deltaU   We       Ve       Ue
        fxch    st(2)
        fsubr   st(1), st   ; endV     deltaV   endU     deltaU   We       Ve       Ue
        fld     (FPNewInfo PTR [ebp * 8 + fpNewInfoTable]).fInverse
                            ; 1/splen  endV     deltaV   endU     deltaU   We       Ve       Ue
        fmul    st(2), st   ; 1/splen  endV     stepV    endU     deltaU   We       Ve       Ue
        fmulp   st(4), st   ; endV     stepV    endU     stepU    We ...
        fxch    st(1)       ; stepV    endV     endU     stepU    We       Ve       Ue
        fistp   gfxPDC.pdc_fsv
                            ; endV     endU     stepU    We...
        fxch    st(2)
        fistp   gfxPDC.pdc_fsu
                            ; nxtStartU nxtStartV W      V        U
        fld     fpcnst1 ; FIX1     startU   startV   W        V        U
        fdiv    st, st(3)
        jmp     [innerLoopJump]
outer_loop_no_interp_shade ENDP


;================================================================
; NAME
;   outer_loop_interp_shade 
;
; DESCRIPTION
;   outer loop of core render loop... there are two of these,
;   one with interpolated S and one without.
;
;================================================================

outer_loop_interp_shade PROC C
        PUBLIC C outer_loop_interp_shade
        lea     eax, @@spanEntry
        lea     edx, @@spanLoop
        mov     gfxPDC.pdc_rendSpanEntry, eax
        mov     nextSubSpanJump, edx
        ret
@@spanEntry:
        ; on entry, edi holds xLeft, esi holds integer count
        ; eax holds y value.
        mov     xval, edi
        mov     yval, eax

        fild    xval
        fild    yval
        ; st = y, x

        fld     gfxPDC.pdc_dudx
        fmul    st, st(2)
        fld     gfxPDC.pdc_dudy
        fmul    st, st(2)
        fld     gfxPDC.pdc_dvdx
        fmul    st, st(4)
        fld     gfxPDC.pdc_dvdy
        fmul    st, st(4)
        ; st = dvdy*y, dvdx*x, dudy*y, dudx*x, y, x
        fxch    st(3)
        faddp   st(2), st
        ; st = dvdx * x, dudx*x+dudy*y,dvdy*y, y, x
        faddp   st(2), st
        ; u-u0, v-v0, y, x
        fld     gfxPDC.pdc_dsdx
        fmul    st, st(4)
        fld     gfxPDC.pdc_dsdy
        fmul    st, st(4)
        fld     gfxPDC.pdc_dwdx
        fmulp   st(6), st
        fld     gfxPDC.pdc_dwdy
        fmulp   st(5), st
        ;st = dsy, dsx, u-u0, v-v0, dwy, dwx
        faddp   st(1), st
        ;st = s-s0, u-u0, v-v0, dwy,dwx
        fxch    st(1)
        fadd    gfxPDC.pdc_u0
        ;st = u, s-s0, v-v0, dwy,dwx
        fxch    st(2)
        fadd    gfxPDC.pdc_v0
        ;st = v, s-s0, u, dwy,dwx
        fxch    st(3)
        faddp   st(4), st
        ;st = s-s0, u, v, w-w0
        fadd    gfxPDC.pdc_s0
        fxch    st(3)
        fadd    gfxPDC.pdc_w0
        fxch    st(3)
        fistp   gfxPDC.pdc_fs       
        fxch    st(2)

        ; st = w, v, u
        ; stack = wstart, vstart, ustart
        fld     fpcnst1
        fdiv    st, st(1)

        ; st(0)    st(1)    st(2)    st(3)    st(4)    st(5)
        ; FIX1/w   w        v        u

        ; spanLen in esi
        mov     eax, gfxPDC.pdc_fs ; eax is used to calculate the shademap
        add     edi, gfxPDC.pdc_rowAddr
        shr     eax, 8 ; we want the whole portion of the shade in the upper byte of the low word.
        mov     ecx, gfxPDC.pdc_subDivSize
        or      eax, gfxPDC.pdc_shadeMap ; or in the shadeMap.
        mov     edx, edi ; edx = start
        mov     ebx, gfxPDC.pdc_fs ; prepare fractional part.
        mov     gfxPDC.pdc_shadeSave, eax
        mov     currShadeWVal, ah
        shl     ebx, 16
        mov     gfxPDC.pdc_s_s_fraction, ebx

        ; patch in shade delta
        fld     gfxPDC.pdc_dsdx
        fistp   gfxPDC.pdc_fss
        mov     eax, gouraudShadeFPatch
        mov     ebx, gfxPDC.pdc_fss
        rol     ebx, 16
        mov     [eax - 4], ebx
        mov     eax, gouraudShadeWPatch
        mov     [eax - 1], bl

        cmp     esi, ecx
        jle     @@spanLess
        mov     ecx, xval
        mov     ecx, spanMaxLenTable[ecx*4]
        mov     edx, ecx ; edx is index into the fpNewInfoTable - determines how far we
                         ; step u, v, w (used to hit end point at end of span)
        jmp     @@calcSpanLen
@@spanLess:
        mov     ecx, esi ; subSpanLen = spanLen
        lea     edx, [esi-1] ; last span, so pretend it's one shorter.
@@calcSpanLen:
        sub     esi, ecx

        ; subSpanLen in ecx, spanLen remaining in esi, start in edi

        ; now we calculate start u and start v, leaving them on the stack
        ; and forward step w by spanLen, then do the divide

        fld     (FPNewInfo PTR [edx * 8 + fpNewInfoTable]).fNum
        fld     st(1)
                            ; st(0)    st(1)    st(2)    st(3)    st(4)    st(5)
                            ; FX1/w    spLen    FX1/w    w        v        u
        fmul    st, st(5)   ; startU   spLen    FX1/w    w        v        u
        fxch    st(1)       ; spLen    startU   FX1/w          "
        fmul    gfxPDC.pdc_dwdx ; dW     startU   FX1/w        "
        fxch    st(2)         ;1/w     startU   dW             "
        fmul    st, st(4)   ; startV   startU   dw             "
        fxch    st(2)       ; dw       startU   startV         "
        faddp   st(3), st   ; startU   startV   w+dw     v        u
        fld     fpcnst1 ; FIX1     startU   startV   w+dw     v        u
        fdiv    st, st(3)   ; FX1/(w+dw) startU startV   w+dw     v        u
@@spanLoop:
        ; we have subSpanLen in ecx, span length remaining in esi, start in edi
        ; calculate new spanLen and new subSpanLen
        mov     ebx, gfxPDC.pdc_subDivSize
        mov     eax, esi
        mov     ebp, ecx
        cmp     esi, 0
        jle     @@zeroSubSpanLen
        cmp     esi, ebx
        jle     @@spanLessInLoop
        mov     edx, ebx ; newSubSpanLen = subDivSize
        sub     eax, ebx
        jmp     @@calcSpanRemaining
@@spanLessInLoop:
        lea     edx, [esi - 1] ; newSubSpanLen (for calc w) = spanLen - 1
        mov     ebx, esi ; actual newSubSpanLen = spanLen
        mov     eax, 0
@@calcSpanRemaining:
        ; prestep w
        fld     (FPNewInfo PTR [edx * 8 + fpNewInfoTable]).fNum
        fmul    gfxPDC.pdc_dwdx
        faddp   st(4), st
        jmp     @@saveSpanInfo
@@zeroSubSpanLen:
        mov     ebx, 0
        dec     ebp ; this span we're drawing is going to be the last..
@@saveSpanInfo:
        mov     gfxPDC.pdc_spanLen, eax
        mov     gfxPDC.pdc_subSpanLen, ebx

        ; now let's get ready to draw the span...

                            ; st(0)    st(1)    st(2)    st(3)    st(4)    st(5)    st(6)    st(7)
                            ; FX1/(w+dw) startU startV   w+dw     v        u
        fxch    st(1)       ; startU   1/We     startV   We       V        U
        fist    gfxPDC.pdc_fu
        fxch    st(2)       ; startV   1/We     startU   We       V        U
        fist    gfxPDC.pdc_fv
        fld     (FPNewInfo PTR [ebp * 8 + fpNewInfoTable]).fNum
        fld     st          ; subSplen subSplen startV   1/We     startU   We       V        U
        fmul    gfxPDC.pdc_dudx  ; du       subSplen  ""
        fxch    st(1)
        fmul    gfxPDC.pdc_dvdx  ; dv       du       startV   1/We     startU   We       V        U
        fxch    st(1)       ; du       dv   ""
        faddp   st(7), st   ; dv       startV   1/We     startU   We       V        U
        faddp   st(5), st   ; startV   1/We     startU   We       Ve       Ue
        ; now calculate new starts and deltas
        fld     st(1)       ; 1/We     startV   1/We     startU   We       Ve       Ue
        fmul    st, st(6)   ; endU     startV   1/We     startU   We       Ve       Ue
        fxch    st(2)
        fmul    st, st(5)   ; endV     startV   endU     startU   We       Ve       Ue
        fxch    st(2)
        fsubr   st(3), st   ; endU     startV   endV     deltaU   We       Ve       Ue
        fxch    st(2)
        fsubr   st(1), st   ; endV     deltaV   endU     deltaU   We       Ve       Ue
        fld     (FPNewInfo PTR [ebp * 8 + fpNewInfoTable]).fInverse
                            ; 1/splen  endV     deltaV   endU     deltaU   We       Ve       Ue
        fmul    st(2), st   ; 1/splen  endV     stepV    endU     deltaU   We       Ve       Ue
        fmulp   st(4), st   ; endV     stepV    endU     stepU    We ...
        fxch    st(1)       ; stepV    endV     endU     stepU    We       Ve       Ue
        fistp   gfxPDC.pdc_fsv
                            ; endV     endU     stepU    We...
        fxch    st(2)
        fistp   gfxPDC.pdc_fsu
                            ; nxtStartU nxtStartV W      V        U
        fld     fpcnst1 ; FIX1     startU   startV   W        V        U
        fdiv    st, st(3)
        jmp     [innerLoopJump]
outer_loop_interp_shade ENDP



;================================================================
; NAME
;   outer_loop_no_interp_shade_new
;
; DESCRIPTION
;   outer loop of core render loop... there are two of these,
;   one with interpolated S and one without.
;
;================================================================

outer_loop_no_interp_shade_new PROC C
        PUBLIC C outer_loop_no_interp_shade_new
        lea     eax, @@spanEntry
        mov     gfxPDC.pdc_rendSpanEntry, eax
        lea     eax, @@startCoordDone
        mov     startCoordDoneJump, eax
        ret
@@spanEntry:
        ; on entry, edi holds xLeft, esi holds integer count
        ; eax holds y value.
        mov     ecx, edi            ; ecx = xLeft

        ; Prep for row start coords
        ;
        mov     yval, eax           
        fild    yval                ; st= y
        fld     gfxPDC.pdc_dudy     ;   = dudy, y
        fxch    st(1)               ;   = y, dudy
        fmul    st(1), st           ;   = y, ydu
        fld     gfxPDC.pdc_dvdy     ;   = dvdy, y, ydu
        fxch    st(1)               ;   = y, dvdu, ydu
        fmul    st(1), st           ;   = y, ydv, ydu

        ; DMMNOTE: Why here?
        lea     edx, coordStarts
        mov     xval, ecx

        fmul    gfxPDC.pdc_dwdy     ;   = ydw, ydv, ydu

        ; Get row starting coords...
        ;
        fxch    st(2)               ;   = ydu, ydv, ydw
        fadd    gfxPDC.pdc_u0       ;   = u0y, ydv, ydw
        fxch    st(1)               ;   = ydv, u0y, ydw
        fadd    gfxPDC.pdc_v0       ;   = v0y, u0y, ydw
        fxch    st(2)               ;   = ydw, u0y, v0y
        fadd    gfxPDC.pdc_w0       ;   = w0y, u0y, v0y


        mov     ebx, esi                    ; couunt decrementer (ebx = count)
        mov     ebp, spanMaxLenTable[ecx*4]
        cmp     ebp, 3                      ; first span must be >= 4
        jg      @@startXLoop
        add     ebp, gfxPDC.pdc_subDivSize
@@startXLoop:
        ; st  = w0y, u0y, v0y
        ; ecx = xval
        ; edx = coordStarts (current entry...)
        ; ebx = spanCount
        ; ebp = maxSubSpanLength

        fild    xval                ;st = x     w0y  u0y  v0y
        fld     gfxPDC.pdc_dudx     ;   = dudx  x    w0y  u0y  v0y
        fmul    st, st(1)           ;   = xdu   x    w0y  u0y  v0y
        fld     gfxPDC.pdc_dvdx     ;   = dvdx  xdu  x    w0y  u0y  v0y
        fmul    st, st(2)           ;   = xdv   xdu  x    w0y  u0y  v0y
        fld     gfxPDC.pdc_dwdx     ;   = dwdx  xdv  xdu  x    w0y  u0y  v0y
        fmulp   st(3), st           ;   = xdv   xdu  xdw  w0y  u0y  v0y
        fxch    st(1)               ;   = xdu   xdv  xdw  w0y  u0y  v0y
        fadd    st, st(4)           ;   = u     xdv  xdw  w0y  u0y  v0y
        fxch    st(1)               ;   = xdv   u    xdw  w0y  u0y  v0y
        fadd    st, st(5)           ;   = v     u    xdw  w0y  u0y  v0y
        fxch    st(2)               ;   = xdw   u    v    w0y  u0y  v0y
        fadd    st, st(3)           ;   = w     u    v    w0y  u0y  v0y

        fxch    st(1)               ;   = u     w    v    w0y  u0y  v0y
        fstp    DWORD PTR [edx]     ;   = w     v    w0y  u0y  v0y
        fstp    DWORD PTR [edx+4]   ;   = v     w0y  u0y  v0y
        fstp    DWORD PTR [edx+8]   ;   = w0y   u0y  v0y

        cmp     ebx, 0              ; is the count 0?
        je      @@spanSetupDone     ;  we're done

        ; packing order is u, w, v

        mov     eax, 0              ; eax will hold the new xval
        cmp     ebp, ebx            ; is the subspan more than the remaining cnt?
        jle     @@spanShorter
        mov     eax, -1             ; if so, pretend that the new xval is one back
        mov     ebp, ebx            ;  from where it actually is...
@@spanShorter:
        add     eax, ecx                    ; eax = currentX
        add     ecx, ebp                    ; currentX += subspanLen (newX)
        add     eax, ebp                    ; eax += subSpanLen      (newX)
        mov     DWORD PTR [edx+12], ebp     ; move out count to coordstarts
        mov     xval, eax                   ; store off new xval (needed above)
        sub     ebx, ebp                    ; dec remaining span length
        add     edx, 16                     ; step to next coordstarts entry
        mov     ebp, gfxPDC.pdc_subDivSize  ; maxSpanLength = g_subSpanLen
        jmp     @@startXLoop                ; loop it!
@@spanSetupDone:
        mov     DWORD PTR [edx+12], 0
        ; pop off start coords...
     
        ; edx = start of coordStarts
        lea     edx, coordStarts
        fstp    st
        fstp    st
        fstp    st

        ; We need to set up the values for the last subspan...
        ;
        fld1                        ;st = 1
        fld     DWORD PTR [edx+4]   ;   = w   1
        fdivp   st(1), st           ;   = 1/w

        ; Pack all possible integer instructions here...
        ;
        add     edi, gfxPDC.pdc_rowAddr
        add     esi, edi
        dec     esi
        mov     gfxPDC.pdc_dstLast, esi

        ;; DMMNOTE: moved into "shadow" of the fdiv
        add     edx, 12
        mov     spanPtr, edx

        fld     DWORD PTR [edx - 12]    ;st = u    1/w
        fmul    st, st(1)               ;   = u/w  1/w
        fld     DWORD PTR [edx - 4]     ;   = v    u/w  1/w
        fmulp   st(2), st               ;   = u/w  v/w

        ; Set up uStart and vStart DMMNOTE: Move this into interp function?
        jmp     [startCoordCalcJump]
@@startCoordDone:
        mov     edx, spanPtr
        fld     fpcnst1
        fld     DWORD PTR [edx+8]
        fdivp   st(1), st
        ; st = 1/wn, us, vs
        mov     ecx, DWORD PTR [edx] ; span length
        jmp     [innerLoopJump]
outer_loop_no_interp_shade_new ENDP


;================================================================
; NAME
;   GFXRenderSpanUVWNew
;
; DESCRIPTION
;   inner loops for new perspective renderers - this function
;   has two separate inner loops, one that does power of 2 width
;   and one that does arbitrary stride.
;   a test in the beginning determines which one to use.
;
;================================================================

.data
    UVWNewwritable dd  0
.code

GFXRenderSpanUVWNew PROC C
        PUBLIC C GFXRenderSpanUVWNew
        mark_writable UVWNewwritable
        CalcInfo
@@start_proc:
        call    outer_loop_no_interp_shade_new
        mov     gfxPDC.pdc_subDivSize, 24
        ScaleTexCoords
        CalcSubDivTable
        mov     edx, gfxPDC.pdc_textureMapStride
        ; if stride & (-stride) == stride, it's power of 2

        mov     eax, edx
        neg     edx
        and     edx, eax
        cmp     eax, edx
        jne     @@arbStride ;; force arb stride for test...

        lea     eax, @@innerLoopPower2
        lea     edx, @@startCoordCalcPower2
        mov     innerLoopJump, eax
        mov     startCoordCalcJump, edx
        mov     edx, -1
        mov     eax, gfxPDC.pdc_textureMapStride
@@textureShiftLoopCalc:
        shr     eax, 1
        inc     edx
        cmp     eax, 0
        jg      @@textureShiftLoopCalc
        mov     gfxPDC.pdc_textureMapShift, edx
        ret
@@arbStride:
        lea     eax, @@innerLoopArbStride
        lea     edx, @@startCoordCalcArbStride
        mov     innerLoopJump, eax
        mov     startCoordCalcJump, edx
        ret
@@startCoordCalcArbStride:
        ; st = startU, startV
        ; subSpanLen in ecx, spanLen remaining in esi, start in edi
        fist    gfxPDC.pdc_fu
        fxch    st(1)
        fist    gfxPDC.pdc_fv
        fxch    st(1)

        mov     edx, gfxPDC.pdc_textureMapStride
        mov     eax, gfxPDC.pdc_fv
        sar     eax, 16              ; get whole portion of v
        mov     ebx, gfxPDC.pdc_fv   ; prepare for fraction of v
      
        imul    edx                  ; whole v *= textureMapStride
      
        shl     ebx, 16              ; get fraction v
        mov     edx, gfxPDC.pdc_fu   ; prepare for whole u
      
        sar     edx, 16              ; edx = whole u
        mov     vFracStart, ebx      ; save off fraction v
      
        add     eax, edx             ; eax = v*stride + u
        mov     ebx, gfxPDC.pdc_fu   ; prepare for fraction u
      
        shl     ebx, 16              ; ebx = fraction u
        mov     edx, gfxPDC.pdc_textureMap
      
        add     eax, edx             ; eax = tex + v*stride + u
        mov     uFracStart, ebx      ; save off ufrac
      
        mov     textureMapStart, eax ; save off startPtr
        jmp     [startCoordDoneJump] ; jump back.

@@innerLoopArbStride:
        ; X | X X X
        ; calculate dstLast and dstLastWord

        ; edx holds spanPtr
        ; ecx holds nextSpanLen

        fld     DWORD PTR [edx+4] ; u
        fmul    st, st(1)
        fld     DWORD PTR [edx+12] ; v
        fmulp   st(2), st

        ; st = ue, ve, us, vs

        fxch    st(2)
        fsubr   st, st(2)
        ; deltau, ve, ue, vs
        fxch    st(1)
        fxch    st(3)
        fsubr   st, st(3)
        ; st = deltav, deltau, ue, ve
        fld     (FPNewInfo PTR [ecx*8 + fpNewInfoTable]).fInverse
        fmul    st(1), st
        add     edx, 16
        fmulp   st(2), st
        fistp   gfxPDC.pdc_fsv
        fistp   gfxPDC.pdc_fsu
        fld     fpcnst1
        fld     DWORD PTR [edx+8]
        fdivp   st(1), st
        mov     spanPtr, edx

        add     ecx, edi
        mov     eax, gfxPDC.pdc_fsv

        sub     ecx, 4
        mov     ebp, gfxPDC.pdc_fsu

        sar     eax, 16                                 ; eax = stepVWhole
        and     ecx, NOT 3

        sar     ebp, 16                                 ; ebp = stepUWhole
        mov     gfxPDC.pdc_dstLastWord, ecx

        ; gfxPDC.pdc_strideTable  = stride + stride * stepVWhole + stepUWhole
        ; gfxPDC.pdc_strideLookup = stride * stepVWhole + stepUWhole
        ;
        mov     ecx, gfxPDC.pdc_textureMapStride        ; ecx = stride

        imul    ecx         ; eax = stride*stepVW

        add     eax, ebp    ; eax = stride*stepVW + stepUW

        mov     gfxPDC.pdc_strideLookup, eax
        add     eax, ecx    ; eax = stride + stride*stepVW + stepUW

        mov     gfxPDC.pdc_strideTable, eax

        ; esi, edi, dstLast, uFracStep, vFracStep, strideTable setup
        ; now for uFrac, vFrac and we're golden.
        ;
        mov     esp, gfxPDC.pdc_fsu

        shl     esp, 16
        mov     ecx, gfxPDC.pdc_fsv

        shl     ecx, 16
        mov     esi, textureMapStart

        mov     ebx, vFracStart
        mov     edx, uFracStart

        mov     eax, gfxPDC.pdc_dstLastWord
        add     ebx, ecx

        sbb     ebp, ebp
        mov     uFracStep, esp
        cmp     edi, eax
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]


        jg      @@checkLast
        test    edi, 3
        jz      @@dwordLoop
@@pixelLoop:
        inc     edi
        mov     al, [esi]
        add     edx, esp
        adc     esi, ebp
        add     ebx, ecx
        sbb     ebp, ebp
        test        edi, 3
        mov     [edi-1], al
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]
        jnz     @@pixelLoop
        
@@dwordLoop:    ; 18 cycles...
        mov     al, [esi]
        add     edx, esp
        adc     esi, ebp
        add     ebx, ecx
        sbb     ebp, ebp
        add     edi, 4
        ror     eax, 8
        add     edx, esp
        mov     al, [esi]
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]
        adc     esi, ebp
        add     ebx, ecx
        sbb     ebp, ebp
        nop
        mov     ah, [esi]
        nop
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]
        add     edx, esp
        adc     esi, ebp
        add     ebx, ecx
        sbb     ebp, ebp
        nop
        ror     eax, 16
        add     edx, esp
        mov     al, [esi]
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]
        adc     esi, ebp
        add     ebx, ecx
        sbb     ebp, ebp
        mov     esp, gfxPDC.pdc_dstLastWord
        ror     eax, 8
        cmp     edi, esp
        mov     [edi-4], eax
        mov     esp, uFracStep
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]
        jle     @@dwordLoop
; INNER LOOP ENDS
        ; prepare to loop back
@@checkLast:
        mov     eax, spanPtr
        sub     ebx, ecx    ; undo last add
        mov     vFracStart, ebx ; save off current u, v
        mov     uFracStart, edx ; values
        mov     textureMapStart, esi ; save off textureMapPtr.

        mov     ecx, DWORD PTR [eax]
        mov     edx, eax
        cmp     ecx, 0
        je      @@finalLoopCheck
        jmp     [innerLoopJump]
@@finalLoopCheck:
        cmp     edi, gfxPDC.pdc_dstLast
        ja      @@nextSpan
        mov     edx, uFracStart
        mov     ecx, gfxPDC.pdc_fsv
        shl     ecx, 16
        add     ebx, ecx
@@finalPixelLoop:
        mov     al, [esi]
        add     edx, esp ; uFracStep
        adc     esi, ebp
        add     ebx, ecx
        sbb     ebp, ebp
        mov     [edi], al
        inc     edi
        mov     eax, gfxPDC.pdc_dstLast
        mov     ebp, gfxPDC.pdc_strideLookup[ebp * 4]
        cmp     edi, eax
        jle     @@finalPixelLoop
@@nextSpan:
        fstp    st
        fstp    st
        fstp    st
        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]

        ; -----------------------
        ; -----------------------
        ; here is the power of 2
        ; inner loop 

@@startCoordCalcPower2:
      ; st = startU, startV
        ; subSpanLen in ecx, spanLen remaining in esi, start in edi
        fist    gfxPDC.pdc_fu
        fxch    st(1)
        fist    gfxPDC.pdc_fv
        fxch    st(1)

        mov     edx, ecx
        mov     ecx, gfxPDC.pdc_textureMapShift
        mov     eax, gfxPDC.pdc_fv
        sar     eax, 16              ; get whole portion of v
        mov     ebx, gfxPDC.pdc_fv   ; prepare for fraction of v
      
        shl     eax, cl              ; whole v *= textureMapStride
        mov     ecx, edx             ; restore ecx.
      
        shl     ebx, 16              ; get fraction v
        mov     edx, gfxPDC.pdc_fu   ; prepare for whole u
      
        sar     edx, 16              ; edx = whole u
        mov     vFracStart, ebx      ; save off fraction v
      
        add     eax, edx             ; eax = v*stride + u
        mov     ebx, gfxPDC.pdc_fu   ; prepare for fraction u
      
        shl     ebx, 16              ; ebx = fraction u
        mov     edx, gfxPDC.pdc_textureMap
      
        add     eax, edx             ; eax = tex + v*stride + u
        mov     uFracStart, ebx      ; save off ufrac
      
        mov     textureMapStart, eax ; save off startPtr
        jmp     [startCoordDoneJump] ; jump back.

        ; -----------------------
        ; -----------------------
        ; here is the power of 2
        ; inner loop 

@@innerLoopPower2:
        ; X | X X X
        ; calculate dstLast and dstLastWord
        ; edx holds spanPtr
        ; ecx holds nextSpanLen

        fld     DWORD PTR [edx+4] ; u
        fmul    st, st(1)
        fld     DWORD PTR [edx+12] ; v
        fmulp   st(2), st

        ; st = ue, ve, us, vs

        fsubr   st(2), st
        fxch    st(1)
        fsubr   st(3), st
      
        ; st = ve, ue, du, dv
        fld     (FPNewInfo PTR [ecx*8 + fpNewInfoTable]).fInverse
        fmul    st(4), st
        add     edx, 16
        fmulp   st(3), st
        fxch    st(3)
        fistp   gfxPDC.pdc_fsv
        fxch    st(1)
        fistp   gfxPDC.pdc_fsu
        fld     fpcnst1
        fld     DWORD PTR [edx+8]
        fdivp   st(1), st
        mov     spanPtr, edx

        add     ecx, edi
        mov     eax, gfxPDC.pdc_fsv

        sub     ecx, 4
        mov     ebp, gfxPDC.pdc_fsu

        sar     eax, 16
        and     ecx, NOT 3

        sar     ebp, 16
        mov     gfxPDC.pdc_dstLastWord, ecx

        mov     ecx, gfxPDC.pdc_textureMapShift

        ; gfxPDC.pdc_strideTable  = stride + stride * stepVWhole + stepUWhole
        ; gfxPDC.pdc_strideLookup = stride * stepVWhole + stepUWhole

        shl     eax, cl
        mov     ebx, gfxPDC.pdc_textureMapStride

        add     eax, ebp

        mov     gfxPDC.pdc_strideLookup, eax
        add     eax, ebx

        mov     gfxPDC.pdc_strideTable, eax
        mov     esp, gfxPDC.pdc_fsu

        ; esi, edi, dstLast, uFracStep, vFracStep, strideTable setup
        ; now for uFrac, vFrac and we're golden.

        shl     esp, 16
        mov     ecx, gfxPDC.pdc_fsv

        shl     ecx, 16
        mov     esi, textureMapStart

        mov     ebx, vFracStart
        mov     edx, uFracStart

        mov     eax, gfxPDC.pdc_dstLastWord
        add     ebx, ecx

        sbb     ebp, ebp
        mov     uFracStep, esp
        cmp     edi, eax
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]

        ; the three branches below take us into the render
        ; loops above.

        jg      @@checkLast
        test    edi, 3
        jz      @@dwordLoop
        jmp     @@pixelLoop
@@end_proc:
GFXRenderSpanUVWNew ENDP




;================================================================
; NAME
;   GFXRenderSpanUVW
;
; DESCRIPTION
;   inner loops for new perspective renderers - this function
;   has two separate inner loops, one that does power of 2 width
;   and one that does arbitrary stride.
;   a test in the beginning determines which one to use.
;
;================================================================

.data
    UVWwritable dd  0
.code

GFXRenderSpanUVW PROC C
        PUBLIC C GFXRenderSpanUVW
        mark_writable UVWwritable
        CalcInfo
@@start_proc:
        call    outer_loop_no_interp_shade
        mov     gfxPDC.pdc_subDivSize, 24
        ScaleTexCoords
        CalcSubDivTable
        mov     edx, gfxPDC.pdc_textureMapStride

        ; if stride & (-stride) == stride, it's power of 2
        mov     eax, edx
        neg     edx
        and     edx, eax
        cmp     eax, edx
        jne     @@arbStride

        lea     eax, @@innerLoopPower2
        mov     innerLoopJump, eax
        mov     edx, -1
        mov     eax, gfxPDC.pdc_textureMapStride
@@textureShiftLoopCalc:
        shr     eax, 1
        inc     edx
        cmp     eax, 0
        jg      @@textureShiftLoopCalc
        mov     gfxPDC.pdc_textureMapShift, edx
        ret
@@arbStride:
        lea     eax, @@innerLoopArbStride
        mov     innerLoopJump, eax
        ret
@@innerLoopArbStride:
        ; X | X X X
        ; calculate dstLast and dstLastWord
        add     ecx, edi
        mov     esi, gfxPDC.pdc_textureMap

        dec     ecx
        mov     eax, gfxPDC.pdc_fv

        sar     eax, 16
        mov     gfxPDC.pdc_dstLast, ecx

        mov     ebx, gfxPDC.pdc_fu
        sub     ecx, 3

        sar     ebx, 16
        and     ecx, NOT 3

        mov     gfxPDC.pdc_dstLastWord, ecx
        mov     ecx, gfxPDC.pdc_textureMapStride

        imul    ecx

        add     esi, eax
        mov     eax, gfxPDC.pdc_fsv

        add     esi, ebx
        mov     edx, eax
        ; esi, edi, dstLast are ready...

        shl     edx, 16
        mov     ebp, gfxPDC.pdc_fsu
        
        sar     eax, 16
        mov     vFracStep, edx

        ; gfxRC.rc_strideTable = stride + stride * stepVWhole + stepUWhole
        ; gfxRC.rc_strideLookup = stride * stepVWhole + stepUWhole

        sar     ebp, 16
        imul    ecx

        add     eax, ebp
        mov     ebp, gfxPDC.pdc_fsu

        shl     ebp, 16
        mov     gfxPDC.pdc_strideLookup, eax

        add     eax, ecx
        mov     uFracStep, ebp

        mov     ecx, vFracStep

        ; esi, edi, dstLast, uFracStep, vFracStep, strideTable setup
        ; now for uFrac, vFrac and we're golden.

        mov     ebx, gfxPDC.pdc_fv

        shl     ebx, 16
        mov     edx, gfxPDC.pdc_fu

        shl     edx, 16
        add     ebx, ecx

        sbb     ebp, ebp
        mov     gfxPDC.pdc_strideTable, eax

        mov     esp, uFracStep
        mov     eax, gfxPDC.pdc_dstLastWord

        cmp     edi, eax
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]

        jg      @@checkLast
        test    edi, 3
        jz      @@dwordLoop
@@pixelLoop:
        inc     edi
        mov     al, [esi]
;       add     edx, uFracStep
        add     edx, esp
        adc     esi, ebp
        add     ebx, ecx
        sbb     ebp, ebp
        test        edi, 3
        mov     [edi-1], al
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]
        jnz     @@pixelLoop
        
@@dwordLoop:    ; 18 cycles...
        mov     al, [esi]
        add     edx, esp
        adc     esi, ebp
        add     ebx, ecx
        sbb     ebp, ebp
        add     edi, 4
        ror     eax, 8
        add     edx, esp
        mov     al, [esi]
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]
        adc     esi, ebp
        add     ebx, ecx
        sbb     ebp, ebp
        nop
        mov     ah, [esi]
        nop
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]
        add     edx, esp
        adc     esi, ebp
        add     ebx, ecx
        sbb     ebp, ebp
        nop
        ror     eax, 16
        add     edx, esp
        mov     al, [esi]
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]
        adc     esi, ebp
        add     ebx, ecx
        sbb     ebp, ebp
        mov     esp, gfxPDC.pdc_dstLastWord
        ror     eax, 8
        cmp     edi, esp
        mov     [edi-4], eax
        mov     esp, uFracStep
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]
        jle     @@dwordLoop
; INNER LOOP ENDS
        ; prepare to loop back
@@checkLast:
        cmp     edi, gfxPDC.pdc_dstLast
        jbe     @@finalPixelLoop
        mov     ecx, gfxPDC.pdc_subSpanLen
        mov     esi, gfxPDC.pdc_spanLen
        cmp     ecx, 0
        je      @@nextSpan
        jmp     [nextSubSpanJump]
@@finalPixelLoop:
        mov     al, [esi]
        add     edx, esp ; uFracStep
        adc     esi, ebp
        add     ebx, ecx
        sbb     ebp, ebp
        mov     [edi], al
        inc     edi
        mov     eax, gfxPDC.pdc_dstLast
        mov     ebp, gfxPDC.pdc_strideLookup[ebp * 4]
        cmp     edi, eax
        jle     @@finalPixelLoop
@@nextSpan:
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]

        ; -----------------------
        ; here is the power of 2
        ; inner loop 
        ; -----------------------

@@innerLoopPower2:
        ; X | X X X
        ; calculate dstLast and dstLastWord
        add     ecx, edi
        mov     esi, gfxPDC.pdc_textureMap

        dec     ecx
        mov     eax, gfxPDC.pdc_fv

        sar     eax, 16
        mov     gfxPDC.pdc_dstLast, ecx

        mov     ebx, gfxPDC.pdc_fu
        sub     ecx, 3

        sar     ebx, 16
        and     ecx, NOT 3

        mov     gfxPDC.pdc_dstLastWord, ecx
        mov     ecx, gfxPDC.pdc_textureMapShift

        shl     eax, cl
        add     esi, eax
        
        mov     eax, gfxPDC.pdc_fsv
        add     esi, ebx
        
        mov     edx, eax
        ; esi, edi, dstLast are ready...

        shl     edx, 16
        mov     ebp, gfxPDC.pdc_fsu
        
        sar     eax, 16
        mov     vFracStep, edx

        ; gfxRC.rc_strideTable = stride + stride * stepVWhole + stepUWhole
        ; gfxRC.rc_strideLookup = stride * stepVWhole + stepUWhole

        sar     ebp, 16
        shl     eax, cl

        add     eax, ebp
        mov     ebp, gfxPDC.pdc_fsu

        shl     ebp, 16
        mov     gfxPDC.pdc_strideLookup, eax

        add     eax, gfxPDC.pdc_textureMapStride
        mov     uFracStep, ebp

        mov     ecx, vFracStep

        ; esi, edi, dstLast, uFracStep, vFracStep, strideTable setup
        ; now for uFrac, vFrac and we're golden.

        mov     ebx, gfxPDC.pdc_fv

        shl     ebx, 16
        mov     edx, gfxPDC.pdc_fu

        shl     edx, 16
        add     ebx, ecx

        sbb     ebp, ebp
        mov     gfxPDC.pdc_strideTable, eax

        mov     esp, uFracStep
        mov     eax, gfxPDC.pdc_dstLastWord

        cmp     edi, eax
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]

        ; the three branches below take us into the render
        ; loops above.

        jg      @@checkLast
        test    edi, 3
        jz      @@dwordLoop
        jmp     @@pixelLoop
@@end_proc:
GFXRenderSpanUVW ENDP

;================================================================
; NAME
;   GFXRenderSpanUVWC
;
; DESCRIPTION
;   inner loops for new perspective renderers - this function
;   has two separate inner loops, one that does power of 2 width
;   and one that does arbitrary stride.
;   a test in the beginning determines which one to use.
;
;================================================================

.data
    UVWCwritable    dd  0
.code

GFXRenderSpanUVWC PROC C
        PUBLIC C GFXRenderSpanUVWC
        mark_writable UVWCwritable
        CalcInfo
@@start_proc:
        call    outer_loop_no_interp_shade
        mov     gfxPDC.pdc_subDivSize, 24
        ScaleTexCoords
        CalcSubDivTable
        mov     edx, gfxPDC.pdc_textureMapStride
        ; if stride & (-stride) == stride, it's power of 2

        mov     eax, edx
        neg     edx
        and     edx, eax
        cmp     eax, edx
        jne     @@arbStride

        lea     eax, @@innerLoopPower2
        mov     innerLoopJump, eax
        mov     edx, -1
        mov     eax, gfxPDC.pdc_textureMapStride
@@textureShiftLoopCalc:
        shr     eax, 1
        inc     edx
        cmp     eax, 0
        jg      @@textureShiftLoopCalc
        mov     gfxPDC.pdc_textureMapShift, edx
        ret
@@arbStride:
        lea     eax, @@innerLoopArbStride
        mov     innerLoopJump, eax
        ret
@@innerLoopArbStride:
        ; X | X X X
        ; calculate dstLast and dstLastWord
        add     ecx, edi
        mov     esi, gfxPDC.pdc_textureMap

        dec     ecx
        mov     eax, gfxPDC.pdc_fv

        sar     eax, 16
        mov     gfxPDC.pdc_dstLast, ecx

        mov     ebx, gfxPDC.pdc_fu
        sub     ecx, 3

        sar     ebx, 16
        and     ecx, NOT 3

        mov     gfxPDC.pdc_dstLastWord, ecx
        mov     ecx, gfxPDC.pdc_textureMapStride

        imul    ecx

        add     esi, eax
        mov     eax, gfxPDC.pdc_fsv

        add     esi, ebx
        mov     edx, eax
        ; esi, edi, dstLast are ready...

        shl     edx, 16
        mov     ebp, gfxPDC.pdc_fsu
        
        sar     eax, 16
        mov     vFracStep, edx

        ; gfxRC.rc_strideTable = stride + stride * stepVWhole + stepUWhole
        ; gfxRC.rc_strideLookup = stride * stepVWhole + stepUWhole

        sar     ebp, 16
        imul    ecx

        add     eax, ebp
        mov     ebp, gfxPDC.pdc_fsu

        shl     ebp, 16
        mov     gfxPDC.pdc_strideLookup, eax

        add     eax, ecx
        mov     uFracStep, ebp

        mov     ecx, vFracStep

        ; esi, edi, dstLast, uFracStep, vFracStep, strideTable setup
        ; now for uFrac, vFrac and we're golden.

        mov     ebx, gfxPDC.pdc_fv

        shl     ebx, 16
        mov     edx, gfxPDC.pdc_fu

        shl     edx, 16
        add     ebx, ecx

        sbb     ebp, ebp
        mov     gfxPDC.pdc_strideTable, eax

        mov     esp, uFracStep
        mov     eax, gfxPDC.pdc_dstLastWord

        mov     eax, gfxPDC.pdc_shadeMap
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]

@@pixelLoop:
        inc     edi
        mov     al, [esi]

        add     edx, esp
        adc     esi, ebp

        add     ebx, ecx
        sbb     ebp, ebp

        mov     al, [eax]
        add     esp, 0          ;; DMMNOTE for shading...
        @@shadeFStepPatch1:

        adc     ah,  0          ;; DMMNOTE for gouraud shading...
        @@shadeWStepPatch1:
        cmp     edi, gfxPDC.pdc_dstLast

        mov     [edi-1], al
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]

        jbe     @@pixelLoop
; INNER LOOP ENDS
        ; prepare to loop back
@@checkLast:
        mov     ecx, gfxPDC.pdc_subSpanLen
        mov     esi, gfxPDC.pdc_spanLen
        cmp     ecx, 0
        je      @@nextSpan
        jmp     [nextSubSpanJump]
@@nextSpan:
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]

        ; -----------------------
        ; here is the power of 2
        ; inner loop 
        ; -----------------------

@@innerLoopPower2:
        ; X | X X X
        ; calculate dstLast and dstLastWord
        add     ecx, edi
        mov     esi, gfxPDC.pdc_textureMap

        dec     ecx
        mov     eax, gfxPDC.pdc_fv

        sar     eax, 16
        mov     gfxPDC.pdc_dstLast, ecx

        mov     ebx, gfxPDC.pdc_fu
        sub     ecx, 3

        sar     ebx, 16
        and     ecx, NOT 3

        mov     gfxPDC.pdc_dstLastWord, ecx
        mov     ecx, gfxPDC.pdc_textureMapShift

        shl     eax, cl
        add     esi, eax
        
        mov     eax, gfxPDC.pdc_fsv
        add     esi, ebx
        
        mov     edx, eax
        ; esi, edi, dstLast are ready...

        shl     edx, 16
        mov     ebp, gfxPDC.pdc_fsu
        
        sar     eax, 16
        mov     vFracStep, edx

        ; gfxRC.rc_strideTable = stride + stride * stepVWhole + stepUWhole
        ; gfxRC.rc_strideLookup = stride * stepVWhole + stepUWhole

        sar     ebp, 16
        shl     eax, cl

        add     eax, ebp
        mov     ebp, gfxPDC.pdc_fsu

        shl     ebp, 16
        mov     gfxPDC.pdc_strideLookup, eax

        add     eax, gfxPDC.pdc_textureMapStride
        mov     uFracStep, ebp

        mov     ecx, vFracStep

        ; esi, edi, dstLast, uFracStep, vFracStep, strideTable setup
        ; now for uFrac, vFrac and we're golden.

        mov     ebx, gfxPDC.pdc_fv

        shl     ebx, 16
        mov     edx, gfxPDC.pdc_fu

        shl     edx, 16
        add     ebx, ecx

        sbb     ebp, ebp
        mov     gfxPDC.pdc_strideTable, eax

        mov     esp, uFracStep
        mov     eax, gfxPDC.pdc_dstLastWord

        cmp     edi, eax
        mov     eax, gfxPDC.pdc_shadeMap
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]

        ; the three branches below take us into the render
        ; loops above.

        jmp     @@pixelLoop
@@end_proc:
GFXRenderSpanUVWC ENDP


;================================================================
; NAME
;   GFXRenderSpanUVWC
;
; DESCRIPTION
;   inner loops for new perspective renderers - this function
;   has two separate inner loops, one that does power of 2 width
;   and one that does arbitrary stride.
;   a test in the beginning determines which one to use.
;
;================================================================

.data
    UVWAwritable    dd  0
.code

GFXRenderSpanUVWA PROC C
        PUBLIC C GFXRenderSpanUVWA
        mark_writable UVWAwritable
        CalcInfo
@@start_proc:
        call    outer_loop_no_interp_shade
        mov     gfxPDC.pdc_subDivSize, 24
        ScaleTexCoords
        CalcSubDivTable
        mov     edx, gfxPDC.pdc_textureMapStride
        ; if stride & (-stride) == stride, it's power of 2

        mov     eax, edx
        neg     edx
        and     edx, eax
        cmp     eax, edx
        jne     @@arbStride

        lea     eax, @@innerLoopPower2
        mov     innerLoopJump, eax
        mov     edx, -1
        mov     eax, gfxPDC.pdc_textureMapStride
@@textureShiftLoopCalc:
        shr     eax, 1
        inc     edx
        cmp     eax, 0
        jg      @@textureShiftLoopCalc
        mov     gfxPDC.pdc_textureMapShift, edx
        ret
@@arbStride:
        lea     eax, @@innerLoopArbStride
        mov     innerLoopJump, eax
        ret
@@innerLoopArbStride:
        ; X | X X X
        ; calculate dstLast and dstLastWord
        add     ecx, edi
        mov     esi, gfxPDC.pdc_textureMap

        dec     ecx
        mov     eax, gfxPDC.pdc_fv

        sar     eax, 16
        mov     gfxPDC.pdc_dstLast, ecx

        mov     ebx, gfxPDC.pdc_fu
        sub     ecx, 3

        sar     ebx, 16
        and     ecx, NOT 3

        mov     gfxPDC.pdc_dstLastWord, ecx
        mov     ecx, gfxPDC.pdc_textureMapStride

        imul    ecx

        add     esi, eax
        mov     eax, gfxPDC.pdc_fsv

        add     esi, ebx
        mov     edx, eax
        ; esi, edi, dstLast are ready...

        shl     edx, 16
        mov     ebp, gfxPDC.pdc_fsu
        
        sar     eax, 16
        mov     vFracStep, edx

        ; gfxRC.rc_strideTable = stride + stride * stepVWhole + stepUWhole
        ; gfxRC.rc_strideLookup = stride * stepVWhole + stepUWhole

        sar     ebp, 16
        imul    ecx

        add     eax, ebp
        mov     ebp, gfxPDC.pdc_fsu

        shl     ebp, 16
        mov     gfxPDC.pdc_strideLookup, eax

        add     eax, ecx
        mov     uFracStep, ebp

        mov     ecx, vFracStep

        ; esi, edi, dstLast, uFracStep, vFracStep, strideTable setup
        ; now for uFrac, vFrac and we're golden.

        mov     ebx, gfxPDC.pdc_fv

        shl     ebx, 16
        mov     edx, gfxPDC.pdc_fu

        shl     edx, 16
        add     ebx, ecx

        sbb     ebp, ebp
        mov     gfxPDC.pdc_strideTable, eax

        mov     esp, uFracStep

        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]

@@pixelLoop:
        xor     eax, eax
        inc     edi

        mov     ah,  [esi]
        add     edx, esp

        mov     al,  [edi - 1]
        adc     esi, ebp

        add     eax, gfxPDC.pdc_shadeMap
        add     ebx, ecx

        sbb     ebp, ebp
        add     esp, 0          ;; DMMNOTE for shading...

        mov     al, [eax]
        @@shadeFStepPatch1:
        cmp     edi, gfxPDC.pdc_dstLast

        mov     [edi-1], al
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]

        jbe     @@pixelLoop
;        inc     edi
;        mov     ah, [esi]
;
;        add     edx, esp
;        mov     al, [edi - 1]
;
;        adc     esi, ebp
;        add     ebx, ecx
;
;        sbb     ebp, ebp
;        mov     al, [eax]
;
;        add     esp, 0          ;; DMMNOTE for shading...
;        @@shadeFStepPatch1:
;        adc     ah,  0          ;; DMMNOTE for gouraud shading...
;        @@shadeWStepPatch1:
;
;        cmp     edi, gfxPDC.pdc_dstLast
;        mov     [edi-1], al
;
;        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]
;        jbe     @@pixelLoop
; INNER LOOP ENDS
        ; prepare to loop back
@@checkLast:
        mov     ecx, gfxPDC.pdc_subSpanLen
        mov     esi, gfxPDC.pdc_spanLen
        cmp     ecx, 0
        je      @@nextSpan
        jmp     [nextSubSpanJump]
@@nextSpan:
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]

        ; -----------------------
        ; here is the power of 2
        ; inner loop 
        ; -----------------------

@@innerLoopPower2:
        ; X | X X X
        ; calculate dstLast and dstLastWord
        add     ecx, edi
        mov     esi, gfxPDC.pdc_textureMap

        dec     ecx
        mov     eax, gfxPDC.pdc_fv

        sar     eax, 16
        mov     gfxPDC.pdc_dstLast, ecx

        mov     ebx, gfxPDC.pdc_fu
        sub     ecx, 3

        sar     ebx, 16
        and     ecx, NOT 3

        mov     gfxPDC.pdc_dstLastWord, ecx
        mov     ecx, gfxPDC.pdc_textureMapShift

        shl     eax, cl
        add     esi, eax
        
        mov     eax, gfxPDC.pdc_fsv
        add     esi, ebx
        
        mov     edx, eax
        ; esi, edi, dstLast are ready...

        shl     edx, 16
        mov     ebp, gfxPDC.pdc_fsu
        
        sar     eax, 16
        mov     vFracStep, edx

        ; gfxRC.rc_strideTable = stride + stride * stepVWhole + stepUWhole
        ; gfxRC.rc_strideLookup = stride * stepVWhole + stepUWhole

        sar     ebp, 16
        shl     eax, cl

        add     eax, ebp
        mov     ebp, gfxPDC.pdc_fsu

        shl     ebp, 16
        mov     gfxPDC.pdc_strideLookup, eax

        add     eax, gfxPDC.pdc_textureMapStride
        mov     uFracStep, ebp

        mov     ecx, vFracStep

        ; esi, edi, dstLast, uFracStep, vFracStep, strideTable setup
        ; now for uFrac, vFrac and we're golden.

        mov     ebx, gfxPDC.pdc_fv

        shl     ebx, 16
        mov     edx, gfxPDC.pdc_fu

        shl     edx, 16
        add     ebx, ecx

        sbb     ebp, ebp
        mov     gfxPDC.pdc_strideTable, eax

        mov     esp, uFracStep
        mov     eax, gfxPDC.pdc_dstLastWord

        cmp     edi, eax
        mov     eax, gfxPDC.pdc_shadeMap
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]

        ; the three branches below take us into the render
        ; loops above.

        jmp     @@pixelLoop
@@end_proc:
GFXRenderSpanUVWA ENDP


;================================================================
; NAME
;   GFXRenderSpanUVWS (DMMNOTE: CHEESE!)
;
; DESCRIPTION
;   inner loops for new perspective renderers - this function
;   has two separate inner loops, one that does power of 2 width
;   and one that does arbitrary stride.
;   a test in the beginning determines which one to use.
;
;================================================================

.data
    UVWSwritable    dd  0
.code

GFXRenderSpanUVWS PROC C
        PUBLIC C GFXRenderSpanUVWS
        mark_writable UVWSwritable
        CalcInfo
@@start_proc:
        ; set up patch values...
        lea     eax, @@shadeFStepPatch1
        lea     edx, @@shadeWStepPatch1
        mov     gouraudShadeFPatch, eax
        mov     gouraudShadeWPatch, edx

        call    outer_loop_interp_shade
        mov     gfxPDC.pdc_subDivSize, 24
        ScaleTexCoords
        CalcSubDivTable
        mov     edx, gfxPDC.pdc_textureMapStride
        ; if stride & (-stride) == stride, it's power of 2

        mov     eax, edx
        neg     edx
        and     edx, eax
        cmp     eax, edx
        jne     @@arbStride

        lea     eax, @@innerLoopPower2
        mov     innerLoopJump, eax
        mov     edx, -1
        mov     eax, gfxPDC.pdc_textureMapStride
@@textureShiftLoopCalc:
        shr     eax, 1
        inc     edx
        cmp     eax, 0
        jg      @@textureShiftLoopCalc
        mov     gfxPDC.pdc_textureMapShift, edx
        ret
@@arbStride:
        lea     eax, @@innerLoopArbStride
        mov     innerLoopJump, eax
        ret
@@innerLoopArbStride:
        ; X | X X X
        ; calculate dstLast and dstLastWord
        add     ecx, edi
        mov     esi, gfxPDC.pdc_textureMap

        dec     ecx
        mov     eax, gfxPDC.pdc_fv

        sar     eax, 16
        mov     gfxPDC.pdc_dstLast, ecx

        mov     ebx, gfxPDC.pdc_fu
        sub     ecx, 3

        sar     ebx, 16
        and     ecx, NOT 3

        mov     gfxPDC.pdc_dstLastWord, ecx
        mov     ecx, gfxPDC.pdc_textureMapStride

        imul    ecx

        add     esi, eax
        mov     eax, gfxPDC.pdc_fsv

        add     esi, ebx
        mov     edx, eax
        ; esi, edi, dstLast are ready...

        shl     edx, 16
        mov     ebp, gfxPDC.pdc_fsu
        
        sar     eax, 16
        mov     vFracStep, edx

        ; gfxRC.rc_strideTable = stride + stride * stepVWhole + stepUWhole
        ; gfxRC.rc_strideLookup = stride * stepVWhole + stepUWhole

        sar     ebp, 16
        imul    ecx

        add     eax, ebp
        mov     ebp, gfxPDC.pdc_fsu

        shl     ebp, 16
        mov     gfxPDC.pdc_strideLookup, eax

        add     eax, ecx
        mov     uFracStep, ebp

        mov     ecx, vFracStep

        ; esi, edi, dstLast, uFracStep, vFracStep, strideTable setup
        ; now for uFrac, vFrac and we're golden.

        mov     ebx, gfxPDC.pdc_fv

        shl     ebx, 16
        mov     edx, gfxPDC.pdc_fu

        shl     edx, 16
        add     ebx, ecx

        sbb     ebp, ebp
        mov     gfxPDC.pdc_strideTable, eax

        mov     esp, uFracStep
        mov     eax, gfxPDC.pdc_dstLastWord

        mov     eax, gfxPDC.pdc_shadeSave
        mov     ah,  currShadeWVal
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]

@@pixelLoop:
        inc     edi
        mov     al, [esi]

        add     edx, esp
        adc     esi, ebp

        add     ebx, ecx
        mov     esp, gfxPDC.pdc_s_s_fraction
        
        sbb     ebp, ebp
        mov     al, [eax]

        add     esp, fssPatch
        @@shadeFStepPatch1:
        adc     ah,  fsswPatch
        @@shadeWStepPatch1:

        mov     gfxPDC.pdc_s_s_fraction, esp
        cmp     edi, gfxPDC.pdc_dstLast

        mov     [edi-1], al
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]

        mov     esp, uFracStep
        jbe     @@pixelLoop
; INNER LOOP ENDS
        ; prepare to loop back
@@checkLast:
        mov     currShadeWVal, ah
        mov     ecx, gfxPDC.pdc_subSpanLen
        mov     esi, gfxPDC.pdc_spanLen
        cmp     ecx, 0
        je      @@nextSpan
        jmp     [nextSubSpanJump]
@@nextSpan:
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]

        ; -----------------------
        ; here is the power of 2
        ; inner loop 
        ; -----------------------

@@innerLoopPower2:
        ; X | X X X
        ; calculate dstLast and dstLastWord
        add     ecx, edi
        mov     esi, gfxPDC.pdc_textureMap

        dec     ecx
        mov     eax, gfxPDC.pdc_fv

        sar     eax, 16
        mov     gfxPDC.pdc_dstLast, ecx

        mov     ebx, gfxPDC.pdc_fu
        sub     ecx, 3

        sar     ebx, 16
        and     ecx, NOT 3

        mov     gfxPDC.pdc_dstLastWord, ecx
        mov     ecx, gfxPDC.pdc_textureMapShift

        shl     eax, cl
        add     esi, eax
        
        mov     eax, gfxPDC.pdc_fsv
        add     esi, ebx
        
        mov     edx, eax
        ; esi, edi, dstLast are ready...

        shl     edx, 16
        mov     ebp, gfxPDC.pdc_fsu
        
        sar     eax, 16
        mov     vFracStep, edx

        ; gfxRC.rc_strideTable = stride + stride * stepVWhole + stepUWhole
        ; gfxRC.rc_strideLookup = stride * stepVWhole + stepUWhole

        sar     ebp, 16
        shl     eax, cl

        add     eax, ebp
        mov     ebp, gfxPDC.pdc_fsu

        shl     ebp, 16
        mov     gfxPDC.pdc_strideLookup, eax

        add     eax, gfxPDC.pdc_textureMapStride
        mov     uFracStep, ebp

        mov     ecx, vFracStep

        ; esi, edi, dstLast, uFracStep, vFracStep, strideTable setup
        ; now for uFrac, vFrac and we're golden.

        mov     ebx, gfxPDC.pdc_fv

        shl     ebx, 16
        mov     edx, gfxPDC.pdc_fu

        shl     edx, 16
        add     ebx, ecx

        sbb     ebp, ebp
        mov     gfxPDC.pdc_strideTable, eax

        mov     esp, uFracStep
        mov     eax, gfxPDC.pdc_dstLastWord

        cmp     edi, eax
        mov     eax, gfxPDC.pdc_shadeSave
        mov     ah,  currShadeWVal
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]

        ; the three branches below take us into the render
        ; loops above.

        jmp     @@pixelLoop
@@end_proc:
GFXRenderSpanUVWS ENDP


;================================================================
; NAME
;   GFXRenderSpanUVWS (DMMNOTE: CHEESE!)
;
; DESCRIPTION
;   inner loops for new perspective renderers - this function
;   has two separate inner loops, one that does power of 2 width
;   and one that does arbitrary stride.
;   a test in the beginning determines which one to use.
;
;================================================================

.data
    UVWSTwritable    dd  0
.code

GFXRenderSpanUVWST PROC C
        PUBLIC C GFXRenderSpanUVWST
        mark_writable UVWSTwritable
        CalcInfo
@@start_proc:
        ; set up patch values...
        lea     eax, @@shadeFStepPatch1
        lea     edx, @@shadeWStepPatch1
        mov     gouraudShadeFPatch, eax
        mov     gouraudShadeWPatch, edx

        call    outer_loop_interp_shade
        mov     gfxPDC.pdc_subDivSize, 24
        ScaleTexCoords
        CalcSubDivTable
        mov     edx, gfxPDC.pdc_textureMapStride
        ; if stride & (-stride) == stride, it's power of 2

        mov     eax, edx
        neg     edx
        and     edx, eax
        cmp     eax, edx
        jne     @@arbStride

        lea     eax, @@innerLoopPower2
        mov     innerLoopJump, eax
        mov     edx, -1
        mov     eax, gfxPDC.pdc_textureMapStride
@@textureShiftLoopCalc:
        shr     eax, 1
        inc     edx
        cmp     eax, 0
        jg      @@textureShiftLoopCalc
        mov     gfxPDC.pdc_textureMapShift, edx
        ret
@@arbStride:
        lea     eax, @@innerLoopArbStride
        mov     innerLoopJump, eax
        ret
@@innerLoopArbStride:
        ; X | X X X
        ; calculate dstLast and dstLastWord
        add     ecx, edi
        mov     esi, gfxPDC.pdc_textureMap

        dec     ecx
        mov     eax, gfxPDC.pdc_fv

        sar     eax, 16
        mov     gfxPDC.pdc_dstLast, ecx

        mov     ebx, gfxPDC.pdc_fu
        sub     ecx, 3

        sar     ebx, 16
        and     ecx, NOT 3

        mov     gfxPDC.pdc_dstLastWord, ecx
        mov     ecx, gfxPDC.pdc_textureMapStride

        imul    ecx

        add     esi, eax
        mov     eax, gfxPDC.pdc_fsv

        add     esi, ebx
        mov     edx, eax
        ; esi, edi, dstLast are ready...

        shl     edx, 16
        mov     ebp, gfxPDC.pdc_fsu
        
        sar     eax, 16
        mov     vFracStep, edx

        ; gfxRC.rc_strideTable = stride + stride * stepVWhole + stepUWhole
        ; gfxRC.rc_strideLookup = stride * stepVWhole + stepUWhole

        sar     ebp, 16
        imul    ecx

        add     eax, ebp
        mov     ebp, gfxPDC.pdc_fsu

        shl     ebp, 16
        mov     gfxPDC.pdc_strideLookup, eax

        add     eax, ecx
        mov     uFracStep, ebp

        mov     ecx, vFracStep

        ; esi, edi, dstLast, uFracStep, vFracStep, strideTable setup
        ; now for uFrac, vFrac and we're golden.

        mov     ebx, gfxPDC.pdc_fv

        shl     ebx, 16
        mov     edx, gfxPDC.pdc_fu

        shl     edx, 16
        add     ebx, ecx

        sbb     ebp, ebp
        mov     gfxPDC.pdc_strideTable, eax

        mov     esp, uFracStep
        mov     eax, gfxPDC.pdc_dstLastWord

        mov     eax, gfxPDC.pdc_shadeSave
        mov     ah,  currShadeWVal
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]

@@pixelLoop:
        inc     edi
        mov     al, [esi]

        add     edx, esp
        adc     esi, ebp

        add     ebx, ecx
        mov     esp, gfxPDC.pdc_s_s_fraction
        
        sbb     ebp, ebp
        add     esp, fssPatch
        @@shadeFStepPatch1:

        adc     ah,  fsswPatch
        @@shadeWStepPatch1:
        mov     gfxPDC.pdc_s_s_fraction, esp

        cmp     al, 0
        mov     al, [eax]

        je      @@skipWrite
        mov     [edi-1], al

@@skipWrite:
        cmp     edi, gfxPDC.pdc_dstLast
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]

        mov     esp, uFracStep
        jbe     @@pixelLoop
; INNER LOOP ENDS
        ; prepare to loop back
@@checkLast:
        mov     currShadeWVal, ah
        mov     ecx, gfxPDC.pdc_subSpanLen
        mov     esi, gfxPDC.pdc_spanLen
        cmp     ecx, 0
        je      @@nextSpan
        jmp     [nextSubSpanJump]
@@nextSpan:
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]

        ; -----------------------
        ; here is the power of 2
        ; inner loop 
        ; -----------------------

@@innerLoopPower2:
        ; X | X X X
        ; calculate dstLast and dstLastWord
        add     ecx, edi
        mov     esi, gfxPDC.pdc_textureMap

        dec     ecx
        mov     eax, gfxPDC.pdc_fv

        sar     eax, 16
        mov     gfxPDC.pdc_dstLast, ecx

        mov     ebx, gfxPDC.pdc_fu
        sub     ecx, 3

        sar     ebx, 16
        and     ecx, NOT 3

        mov     gfxPDC.pdc_dstLastWord, ecx
        mov     ecx, gfxPDC.pdc_textureMapShift

        shl     eax, cl
        add     esi, eax
        
        mov     eax, gfxPDC.pdc_fsv
        add     esi, ebx
        
        mov     edx, eax
        ; esi, edi, dstLast are ready...

        shl     edx, 16
        mov     ebp, gfxPDC.pdc_fsu
        
        sar     eax, 16
        mov     vFracStep, edx

        ; gfxRC.rc_strideTable = stride + stride * stepVWhole + stepUWhole
        ; gfxRC.rc_strideLookup = stride * stepVWhole + stepUWhole

        sar     ebp, 16
        shl     eax, cl

        add     eax, ebp
        mov     ebp, gfxPDC.pdc_fsu

        shl     ebp, 16
        mov     gfxPDC.pdc_strideLookup, eax

        add     eax, gfxPDC.pdc_textureMapStride
        mov     uFracStep, ebp

        mov     ecx, vFracStep

        ; esi, edi, dstLast, uFracStep, vFracStep, strideTable setup
        ; now for uFrac, vFrac and we're golden.

        mov     ebx, gfxPDC.pdc_fv

        shl     ebx, 16
        mov     edx, gfxPDC.pdc_fu

        shl     edx, 16
        add     ebx, ecx

        sbb     ebp, ebp
        mov     gfxPDC.pdc_strideTable, eax

        mov     esp, uFracStep
        mov     eax, gfxPDC.pdc_dstLastWord

        cmp     edi, eax
        mov     eax, gfxPDC.pdc_shadeSave
        mov     ah,  currShadeWVal
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]

        ; the three branches below take us into the render
        ; loops above.

        jmp     @@pixelLoop
@@end_proc:
GFXRenderSpanUVWST ENDP


;================================================================
; NAME
;   GFXRenderSpanUVWCT
;
; DESCRIPTION
;   inner loops for new perspective renderers - this function
;   has two separate inner loops, one that does power of 2 width
;   and one that does arbitrary stride.
;   a test in the beginning determines which one to use.
;
;================================================================

.data
    UVWCTwritable    dd  0
.code

GFXRenderSpanUVWCT PROC C
        PUBLIC C GFXRenderSpanUVWCT
        mark_writable UVWCTwritable
        CalcInfo
@@start_proc:
        call    outer_loop_no_interp_shade
        mov     gfxPDC.pdc_subDivSize, 24
        ScaleTexCoords
        CalcSubDivTable
        mov     edx, gfxPDC.pdc_textureMapStride
        ; if stride & (-stride) == stride, it's power of 2

        mov     eax, edx
        neg     edx
        and     edx, eax
        cmp     eax, edx
        jne     @@arbStride

        lea     eax, @@innerLoopPower2
        mov     innerLoopJump, eax
        mov     edx, -1
        mov     eax, gfxPDC.pdc_textureMapStride
@@textureShiftLoopCalc:
        shr     eax, 1
        inc     edx
        cmp     eax, 0
        jg      @@textureShiftLoopCalc
        mov     gfxPDC.pdc_textureMapShift, edx
        ret
@@arbStride:
        lea     eax, @@innerLoopArbStride
        mov     innerLoopJump, eax
        ret
@@innerLoopArbStride:
        ; X | X X X
        ; calculate dstLast and dstLastWord
        add     ecx, edi
        mov     esi, gfxPDC.pdc_textureMap

        dec     ecx
        mov     eax, gfxPDC.pdc_fv

        sar     eax, 16
        mov     gfxPDC.pdc_dstLast, ecx

        mov     ebx, gfxPDC.pdc_fu
        sub     ecx, 3

        sar     ebx, 16
        and     ecx, NOT 3

        mov     gfxPDC.pdc_dstLastWord, ecx
        mov     ecx, gfxPDC.pdc_textureMapStride

        imul    ecx

        add     esi, eax
        mov     eax, gfxPDC.pdc_fsv

        add     esi, ebx
        mov     edx, eax
        ; esi, edi, dstLast are ready...

        shl     edx, 16
        mov     ebp, gfxPDC.pdc_fsu
        
        sar     eax, 16
        mov     vFracStep, edx

        ; gfxRC.rc_strideTable = stride + stride * stepVWhole + stepUWhole
        ; gfxRC.rc_strideLookup = stride * stepVWhole + stepUWhole

        sar     ebp, 16
        imul    ecx

        add     eax, ebp
        mov     ebp, gfxPDC.pdc_fsu

        shl     ebp, 16
        mov     gfxPDC.pdc_strideLookup, eax

        add     eax, ecx
        mov     uFracStep, ebp

        mov     ecx, vFracStep

        ; esi, edi, dstLast, uFracStep, vFracStep, strideTable setup
        ; now for uFrac, vFrac and we're golden.

        mov     ebx, gfxPDC.pdc_fv

        shl     ebx, 16
        mov     edx, gfxPDC.pdc_fu

        shl     edx, 16
        add     ebx, ecx

        sbb     ebp, ebp
        mov     gfxPDC.pdc_strideTable, eax

        mov     esp, uFracStep
        mov     eax, gfxPDC.pdc_dstLastWord

        mov     eax, gfxPDC.pdc_shadeMap
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]

@@pixelLoop:
        inc     edi
        mov     al, [esi]
        add     edx, esp
        adc     esi, ebp
        add     ebx, ecx
        sbb     ebp, ebp
        cmp     al, 0
        je      @@skipWrite
        mov     al, [eax]
        mov     [edi-1], al
@@skipWrite:
        cmp     edi, gfxPDC.pdc_dstLast
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]
        jbe     @@pixelLoop
; INNER LOOP ENDS
        ; prepare to loop back
@@checkLast:
        mov     ecx, gfxPDC.pdc_subSpanLen
        mov     esi, gfxPDC.pdc_spanLen
        cmp     ecx, 0
        je      @@nextSpan
        jmp     [nextSubSpanJump]
@@nextSpan:
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]

        ; -----------------------
        ; here is the power of 2
        ; inner loop 
        ; -----------------------

@@innerLoopPower2:
        ; X | X X X
        ; calculate dstLast and dstLastWord
        add     ecx, edi
        mov     esi, gfxPDC.pdc_textureMap

        dec     ecx
        mov     eax, gfxPDC.pdc_fv

        sar     eax, 16
        mov     gfxPDC.pdc_dstLast, ecx

        mov     ebx, gfxPDC.pdc_fu
        sub     ecx, 3

        sar     ebx, 16
        and     ecx, NOT 3

        mov     gfxPDC.pdc_dstLastWord, ecx
        mov     ecx, gfxPDC.pdc_textureMapShift

        shl     eax, cl
        add     esi, eax
        
        mov     eax, gfxPDC.pdc_fsv
        add     esi, ebx
        
        mov     edx, eax
        ; esi, edi, dstLast are ready...

        shl     edx, 16
        mov     ebp, gfxPDC.pdc_fsu
        
        sar     eax, 16
        mov     vFracStep, edx

        ; gfxRC.rc_strideTable = stride + stride * stepVWhole + stepUWhole
        ; gfxRC.rc_strideLookup = stride * stepVWhole + stepUWhole

        sar     ebp, 16
        shl     eax, cl

        add     eax, ebp
        mov     ebp, gfxPDC.pdc_fsu

        shl     ebp, 16
        mov     gfxPDC.pdc_strideLookup, eax

        add     eax, gfxPDC.pdc_textureMapStride
        mov     uFracStep, ebp

        mov     ecx, vFracStep

        ; esi, edi, dstLast, uFracStep, vFracStep, strideTable setup
        ; now for uFrac, vFrac and we're golden.

        mov     ebx, gfxPDC.pdc_fv

        shl     ebx, 16
        mov     edx, gfxPDC.pdc_fu

        shl     edx, 16
        add     ebx, ecx

        sbb     ebp, ebp
        mov     gfxPDC.pdc_strideTable, eax

        mov     esp, uFracStep
        mov     eax, gfxPDC.pdc_dstLastWord

        cmp     edi, eax
        mov     eax, gfxPDC.pdc_shadeMap
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]

        ; the three branches below take us into the render
        ; loops above.

        jmp     @@pixelLoop
@@end_proc:
GFXRenderSpanUVWCT ENDP


;================================================================
; NAME
;   GFXRenderSpanUVWR
;
; DESCRIPTION
;    Draws a single span passed in from the edge clipper
;    Uses new single-span technique for big speed
;
;================================================================

.data
    UVWRwritable    dd  0
.code

GFXRenderSpanUVWR PROC C
        PUBLIC C GFXRenderSpanUVWR
        mark_writable UVWRwritable
        CalcInfo
@@start_proc:
        call    outer_loop_no_interp_shade
        lea     eax, @@innerLoop
        mov     innerLoopJump, eax
        mov     gfxPDC.pdc_subDivSize, 24
        ScaleTexCoords
        CalcSubDivTable
        mov     edx, 0
        mov     eax, gfxPDC.pdc_textureMapStride
        dec     eax
        mov     DWORD PTR [@@uMaskPatch1 - 4], eax
        mov     uRol, 16
@@uRloop:
        shr     eax, 1
        inc     edx
        cmp     eax, 0
        ja      @@uRloop
        mov     eax, gfxPDC.pdc_textureMapHeight
        dec     eax
        mov     ecx, edx
        shl     eax, cl
        mov     DWORD PTR [@@vMaskPatch1 - 4], eax
        add     edx, 16
        mov     vRol, edx
        ret
@@innerLoop:
        ; on entry to the inner loop, ecx holds span count
        ; and edi holds address of leftmost pixel in span.

        mov     esi, ecx                      ; store span length in esi
        mov     eax, gfxPDC.pdc_fu      ; prepare u coord for roll.
        mov     ecx, uRol                 ; move u roll count into ecx
        mov     ebp, gfxPDC.pdc_fsu     ; prepare u step for roll.
        rol     eax, cl                    ; rol u
        mov     edx, gfxPDC.pdc_fv      ; prepare v coord for roll.
        rol     ebp, cl                   ; rol u step
        mov     ecx, vRol                 ; load roll count reg with v roll.
        rol     edx, cl                   ; rol v
        add     esi, edi                      ; add span start addr to count
        dec     esi                        ; get address of last pixel in span
        mov     esp, gfxPDC.pdc_fsv    ; prepare v step for roll
        rol     esp, cl                    ; rol v step
        mov     gfxPDC.pdc_dstLast, esi ; save off dstLast
@@pixelLoop:
        mov     esi, eax                      ; move u into temp
        mov     ecx, edx                      ; move v into temp
        and     esi, uMaskPatch           ; mask out fraction u
                @@uMaskPatch1:
        and     ecx, vMaskPatch         ; mask out fraction v
                @@vMaskPatch1:
        or      esi, ecx                         ; build offset in texture map
        mov     ecx, gfxPDC.pdc_textureMap
        add     edi, 1
        add     eax, ebp                      ; step u
        adc     eax, 0                    ; carry over
        add     edx, esp                      ; step v
        adc     edx, 0                    ; carry over
        mov     bl, [esi + ecx]
        mov     ecx, gfxPDC.pdc_dstLast
        mov     [edi-1], bl
        cmp     edi, ecx
        jbe     @@pixelLoop

        mov     ecx, gfxPDC.pdc_subSpanLen
        mov     esi, gfxPDC.pdc_spanLen
        cmp     ecx, 0
        je      @@nextSpan
        jmp     [nextSubSpanJump]
@@nextSpan:
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]
@@end_proc:
GFXRenderSpanUVWR ENDP

;================================================================
; NAME
;   GFXRenderSpanUVWTR
;
; DESCRIPTION
;    Draws a single span passed in from the edge clipper
;    Uses new single-span technique for big speed
;
;================================================================

.data
    UVWTRwritable   dd  0
.code

GFXRenderSpanUVWTR PROC C
        PUBLIC C GFXRenderSpanUVWTR
        mark_writable UVWTRwritable
        CalcInfo
@@start_proc:
        call    outer_loop_no_interp_shade
        lea     eax, @@innerLoop
        mov     innerLoopJump, eax
        mov     gfxPDC.pdc_subDivSize, 24
        ScaleTexCoords
        CalcSubDivTable
        mov     edx, 0
        mov     eax, gfxPDC.pdc_textureMapStride
        dec     eax
        mov     DWORD PTR [@@uMaskPatch1 - 4], eax
        mov     uRol, 16
@@uRloop:
        shr     eax, 1
        inc     edx
        cmp     eax, 0
        ja      @@uRloop
        mov     eax, gfxPDC.pdc_textureMapHeight
        dec     eax
        mov     ecx, edx
        shl     eax, cl
        mov     DWORD PTR [@@vMaskPatch1 - 4], eax
        add     edx, 16
        mov     vRol, edx
        ret
@@innerLoop:
        ; on entry to the inner loop, ecx holds span count
        ; and edi holds address of leftmost pixel in span.

        mov     esi, ecx                         ; store span length in esi
        mov     eax, gfxPDC.pdc_fu         ; prepare u coord for roll.
        mov     ecx, uRol                    ; move u roll count into ecx
        mov     ebp, gfxPDC.pdc_fsu       ; prepare u step for roll.
        rol     eax, cl                       ; rol u
        mov     edx, gfxPDC.pdc_fv        ; prepare v coord for roll.
        rol     ebp, cl                      ; rol u step
        mov     ecx, vRol                    ; load roll count reg with v roll.
        rol     edx, cl                      ; rol v
        add     esi, edi                         ; add span start addr to count
        dec     esi                           ; get address of last pixel in span
        mov     esp, gfxPDC.pdc_fsv       ; prepare v step for roll
        rol     esp, cl                       ; rol v step
        mov     gfxPDC.pdc_dstLast, esi    ; save off dstLast
@@pixelLoop:
        mov     esi, eax                         ; move u into temp
        mov     ecx, edx                         ; move v into temp
        and     esi, uMaskPatch              ; mask out fraction u
                @@uMaskPatch1:
        and     ecx, vMaskPatch              ; mask out fraction v
                @@vMaskPatch1:
        or      esi, ecx                            ; build offset in texture map
        mov     ecx, gfxPDC.pdc_textureMap
        add     edi, 1
        add     eax, ebp                         ; step u
        adc     eax, 0                       ; carry over
        add     edx, esp                         ; step v
        adc     edx, 0                       ; carry over
        mov     bl, [esi + ecx]
        mov     ecx, gfxPDC.pdc_dstLast
        cmp     bl, 0
        je      @@noPixWrite
        mov     [edi-1], bl
@@noPixWrite:
        cmp     edi, ecx
        jbe     @@pixelLoop

        mov     ecx, gfxPDC.pdc_subSpanLen
        mov     esi, gfxPDC.pdc_spanLen
        cmp     ecx, 0
        je      @@nextSpan
        jmp     [nextSubSpanJump]
@@nextSpan:
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]
@@end_proc:
GFXRenderSpanUVWTR ENDP

;================================================================
; NAME
;   GFXRenderSpanUVWCR
;
; DESCRIPTION
;    Draws a single span passed in from the edge clipper
;    Uses new single-span technique for big speed
;
;================================================================

.data
    UVWCRwritable   dd  0
.code

GFXRenderSpanUVWCR PROC C
        PUBLIC C GFXRenderSpanUVWCR
        mark_writable UVWCRwritable
        CalcInfo
@@start_proc:
        call    outer_loop_no_interp_shade
        lea     eax, @@innerLoop
        mov     innerLoopJump, eax
        mov     gfxPDC.pdc_subDivSize, 24
        ScaleTexCoords
        CalcSubDivTable
        mov     eax, gfxPDC.pdc_textureMap
        mov     edx, 0
        mov     DWORD PTR [@@textureMapPatch1 - 4], eax
        mov     eax, gfxPDC.pdc_textureMapStride
        dec     eax
        mov     DWORD PTR [@@uMaskPatch1 - 4], eax
        mov     uRol, 16
@@uRloop:
        shr     eax, 1
        inc     edx
        cmp     eax, 0
        ja      @@uRloop
        mov     eax, gfxPDC.pdc_textureMapHeight
        dec     eax
        mov     ecx, edx
        shl     eax, cl
        mov     DWORD PTR [@@vMaskPatch1 - 4], eax
        add     edx, 16
        mov     vRol, edx
        ret
@@innerLoop:
        ; on entry to the inner loop, ecx holds span count
        ; and edi holds address of leftmost pixel in span.

        mov     esi, ecx                      ; store span length in esi
        mov     esp, gfxPDC.pdc_fu      ; prepare u coord for roll.
        mov     ecx, uRol                 ; move u roll count into ecx
        mov     ebp, gfxPDC.pdc_fsu     ; prepare u step for roll.
        rol     eax, cl                    ; rol u
        mov     edx, gfxPDC.pdc_fv     ; prepare v coord for roll.
        rol     ebp, cl                    ; rol u step
        mov     ecx, vRol                  ; load roll count reg with v roll.
        rol     edx, cl                    ; rol v
        add     esi, edi                      ; add span start addr to count
        dec     esi                       ; get address of last pixel in span
        mov     eax, gfxPDC.pdc_fsv     ; prepare v step for roll
        rol     eax, cl                    ; rol v step
        mov     gfxPDC.pdc_dstLast, esi ; save off dstLast
        mov     ebx, gfxPDC.pdc_shadeMap; load in the shadeMap
@@pixelLoop:
        mov     esi, esp                      ; move u into temp
        mov     ecx, edx                      ; move v into temp
        and     esi, uMaskPatch           ; mask out fraction u
                @@uMaskPatch1:
        and     ecx, vMaskPatch           ; mask out fraction v
                @@vMaskPatch1:
        or      esi, ecx                         ; build offset in texture map
        add     esp, ebp                     ; step u
        adc     eax, 0                   ; carry over
        add     edi, 1                   ; increment the dstPtr
        mov     bl, [esi + textureMapPatch] ; read the texture map
                @@textureMapPatch1:
        add     edx, eax                     ; step v
        adc     edx, 0                   ; carry over
        mov     ecx, gfxPDC.pdc_dstLast
        mov     bl, [ebx]                 ; read from the shadeMap
        cmp     edi, ecx                      ; are we at the end?
        mov     [edi-1], bl               ; write it in
        jbe     @@pixelLoop               ; branch back

        mov     ecx, gfxPDC.pdc_subSpanLen
        mov     esi, gfxPDC.pdc_spanLen
        cmp     ecx, 0
        je      @@nextSpan
        jmp     [nextSubSpanJump]
@@nextSpan:
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]
@@end_proc:
GFXRenderSpanUVWCR ENDP

;================================================================
; NAME
;   GFXRenderSpanUVWCTR
;
; DESCRIPTION
;    Draws a single span passed in from the edge clipper
;    Uses new single-span technique for big speed
;
;================================================================

.data
    UVWCTRwritable  dd  0
.code

GFXRenderSpanUVWCTR PROC C
        PUBLIC C GFXRenderSpanUVWCTR
        mark_writable UVWCTRwritable
        CalcInfo
@@start_proc:
        call    outer_loop_no_interp_shade
        lea     eax, @@innerLoop
        mov     innerLoopJump, eax
        mov     gfxPDC.pdc_subDivSize, 24
        ScaleTexCoords
        CalcSubDivTable
        mov     eax, gfxPDC.pdc_textureMap
        mov     edx, 0
        mov     DWORD PTR [@@textureMapPatch1 - 4], eax
        mov     eax, gfxPDC.pdc_textureMapStride
        dec     eax
        mov     DWORD PTR [@@uMaskPatch1 - 4], eax
        mov     uRol, 16
@@uRloop:
        shr     eax, 1
        inc     edx
        cmp     eax, 0
        ja      @@uRloop
        mov     eax, gfxPDC.pdc_textureMapHeight
        dec     eax
        mov     ecx, edx
        shl     eax, cl
        mov     DWORD PTR [@@vMaskPatch1 - 4], eax
        add     edx, 16
        mov     vRol, edx
        ret
@@innerLoop:
        ; on entry to the inner loop, ecx holds span count
        ; and edi holds address of leftmost pixel in span.

        mov     esi, ecx                      ; store span length in esi
        mov     esp, gfxPDC.pdc_fu      ; prepare u coord for roll.
        mov     ecx, uRol                 ; move u roll count into ecx
        mov     ebp, gfxPDC.pdc_fsu     ; prepare u step for roll.
        rol     eax, cl                   ; rol u
        mov     edx, gfxPDC.pdc_fv      ; prepare v coord for roll.
        rol     ebp, cl                   ; rol u step
        mov     ecx, vRol                 ; load roll count reg with v roll.
        rol     edx, cl                   ; rol v
        add     esi, edi                      ; add span start addr to count
        dec     esi                       ; get address of last pixel in span
        mov     eax, gfxPDC.pdc_fsv     ; prepare v step for roll
        rol     eax, cl                    ; rol v step
        mov     gfxPDC.pdc_dstLast, esi ; save off dstLast
        mov     ebx, gfxPDC.pdc_shadeMap; load in the shadeMap
@@pixelLoop:
        mov     esi, esp                      ; move u into temp
        mov     ecx, edx                      ; move v into temp
        and     esi, uMaskPatch           ; mask out fraction u
                @@uMaskPatch1:
        and     ecx, vMaskPatch           ; mask out fraction v
                @@vMaskPatch1:
        or         esi, ecx                   ; build offset in texture map
        add     esp, ebp                      ; step u
        adc     eax, 0                    ; carry over
        add     edi, 1                    ; increment the dstPtr
        mov     bl, [esi + textureMapPatch] ; read the texture map
                @@textureMapPatch1:
        add     edx, eax                      ; step v
        adc     edx, 0                     ; carry over
        mov     ecx, gfxPDC.pdc_dstLast
      cmp      bl, 0
      jne      @@lookupAndWritePixel
      cmp      edi, ecx
      jbe      @@pixelLoop
      jmp      @@nextSpan
@@lookupAndWritePixel:
        mov     bl, [ebx]               ; read from the shadeMap
        cmp     edi, ecx                   ; are we at the end?
        mov     [edi-1], bl             ; write it in
        jbe     @@pixelLoop             ; branch back

        mov     ecx, gfxPDC.pdc_subSpanLen
        mov     esi, gfxPDC.pdc_spanLen
        cmp     ecx, 0
        je      @@nextSpan
        jmp     [nextSubSpanJump]
@@nextSpan:
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        fstp    st
        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]
@@end_proc:
GFXRenderSpanUVWCTR ENDP





;;; DMMNOTE OLD GOURAUD PERSP RENDERER
;;================================================================
;; NAME
;;   GFXRenderSpanUVWS
;;
;; DESCRIPTION
;;   renders a gouraud-shaded perspective textured span
;;   uses the outer_loop_interp_shade setup code...
;;
;;================================================================
;
;.data
;    UVWSwritable    dd 0
;.code
;
;GFXRenderSpanUVWS PROC C,
;        PUBLIC C GFXRenderSpanUVWS
;        mark_writable UVWSwritable
;        CalcInfo
;@@start_proc:
;        call    outer_loop_interp_shade
;        lea     eax, @@innerLoop
;        mov     innerLoopJump, eax
;        mov     gfxPDC.pdc_subDivSize, 24
;        ScaleTexCoords
;        fld     gfxPDC.pdc_dsdx
;        fistp   gfxPDC.pdc_fss
;        mov     eax, gfxPDC.pdc_fss
;        rol     eax, 16
;        mov     edx, eax
;        and     edx, 0FFFF0000h
;        mov     DWORD PTR [@@fssPatch1 - 4], edx
;        mov     BYTE PTR [@@fsswPatch1 - 1], al
;        mov     DWORD PTR [@@fssPatch2 - 4], edx
;        mov     BYTE PTR [@@fsswPatch2 - 1], al
;        mov     DWORD PTR [@@fssPatch3 - 4], edx
;        mov     BYTE PTR [@@fsswPatch3 - 1], al
;        mov     edx, -1
;        mov     eax, gfxPDC.pdc_textureMapStride
;        
;@@textureShiftLoopCalc:
;        inc     edx
;        shr     eax, 1
;        cmp     eax, 0
;        je      @@textureShiftLoopCalc
;        mov     gfxPDC.pdc_textureMapShift, edx
;        CalcSubDivTable
;        ret
;@@innerLoop:
;; INNER LOOP:
;        ; all right, now time to draw the span:
;        ; we have:
;        ; ecx = subSpanLen, edi = destPtr
;
;        ; register settings for the inner loop
;        ; eax : u.f       pixels
;        ; ebx : 0      v.w     u.w
;        ; ecx : v.f    sv.w    su.w
;        ; edx : shadeMap      texel
;        ; edi :     destPtr
;        ; esi : su.f
;        ; ebp : s.f
;        ; esp : sv.f
;
;        ; X | X X X
;        ; calculate dstLast and dstLastWord
;        add     ecx, edi
;        mov     esi, gfxPDC.pdc_textureMap
;        dec     ecx
;        mov     eax, gfxPDC.pdc_fv
;        sar     eax, 16
;        mov     gfxPDC.pdc_dstLast, ecx
;        mov     ebx, gfxPDC.pdc_fu
;        sub     ecx, 1
;        sar     ebx, 16
;        and     ecx, NOT 1
;        mov     gfxPDC.pdc_dstLastWord, ecx
;        mov     ecx, gfxPDC.pdc_textureMapShift
;        shl     eax, cl
;        add     esi, eax
;        mov     eax, gfxPDC.pdc_fsv
;        add     esi, ebx
;        mov     edx, eax
;        ; esi, edi, dstLast are ready...
;
;        shl     edx, 16
;        mov     ebp, gfxPDC.pdc_fsu
;        sar     eax, 16
;        mov     gfxPDC.pdc_fsv, edx
;
;        ; gfxRC.rc_strideTable = stride + stride * stepVWhole + stepUWhole
;        ; gfxRC.rc_strideLookup = stride * stepVWhole + stepUWhole
;
;        sar     ebp, 16
;        shl     eax, cl
;        add     eax, ebp
;        mov     ebp, gfxPDC.pdc_fsu
;        shl     ebp, 16
;        mov     gfxPDC.pdc_strideLookup, eax
;        add     eax, gfxPDC.pdc_textureMapStride
;        mov     esp, ebp ; uFracStep <- ebp
;        mov     gfxPDC.pdc_strideTable, eax
;
;        ; esi, edi, dstLast, uFracStep, vFracStep, strideTable setup
;        ; now for uFrac, vFrac and we're golden.
;
;        mov     ebx, gfxPDC.pdc_fv
;
;        shl     ebx, 16
;        mov     ecx, gfxPDC.pdc_fu
;        shl     ecx, 16
;        ;mov        esp, uFracStep
;        ;mov        ecx, vFracStep
;
;        mov     eax, gfxPDC.pdc_shadeSave
;        mov     edx, gfxPDC.pdc_s_s_fraction
;
;        ;mov        _esp, ecx
;        ;mov        _esp, gfxPDC.pdc_s_s_fraction
;
;        test    edi, 1
;        jz      @@checkDone
;        ; used eax, ebx, edi, esi, edx, ebp, esp
;        mov     al, [esi]
;        add     ebx, gfxPDC.pdc_fsv ;vFracStep
;                @@vFracStepPatch1:
;        sbb     ebp, ebp
;        inc     edi
;        mov     al, [eax]
;        add     edx, fssPatch
;                @@fssPatch1:
;        adc     ah, fsswPatch
;                @@fsswPatch1:
;        add     ecx, esp ; uFracStep
;                @@uFracStepPatch1:
;        adc     esi, gfxPDC.pdc_strideLookup[ebp * 4]
;        mov     [edi-1], al
;@@checkDone:
;        cmp     edi, gfxPDC.pdc_dstLastWord
;        ja      @@checkLast
;        add     ebx, gfxPDC.pdc_fsv ;vFracStep
;                @@vFracStepPatch2:
;        sbb     ebp, ebp
;@@wordLoop:
;        mov     al, [esi]
;        add     ecx, esp ;uFracStep
;        adc     esi, gfxPDC.pdc_strideLookup[ebp * 4]
;        mov     ebp, gfxPDC.pdc_fsv
;        mov     cl, [eax]
;        add     ebx, ebp ;vFracStep
;        sbb     ebp, ebp
;        mov     al, [esi]
;        add     edi, 2
;        add     edx, fssPatch
;                @@fssPatch2:
;        adc     ah, fsswPatch
;                @@fsswPatch2:
;        add     ecx, esp ;uFracStep
;        adc     esi, gfxPDC.pdc_strideLookup[ebp * 4]
;        mov     ebp, gfxPDC.pdc_fsv
;        mov     ch, [eax]
;        add     ebx, ebp ;vFracStep
;        sbb     ebp, ebp
;        add     edx, fssPatch
;                @@fssPatch3:
;        
;        adc     ah, fsswPatch
;                @@fsswPatch3:
;        cmp     edi, gfxPDC.pdc_dstLastWord
;                @@dstLastWordPatch1:
;
;        mov     [edi-2], cx
;        jbe     @@wordLoop
;@@checkLast:
;        mov     ebp, esi
;        mov     ecx, gfxPDC.pdc_subSpanLen
;        mov     esi, gfxPDC.pdc_spanLen
;        mov     gfxPDC.pdc_shadeSave, eax
;        mov     gfxPDC.pdc_s_s_fraction, edx
;        cmp     ecx, 0
;        je      @@spanDoneCheck
;        jmp     [nextSubSpanJump]
;@@spanDoneCheck:
;        cmp     edi, gfxPDC.pdc_dstLast
;        ja      @@nextSpan
;        mov     al, [ebp]
;        mov     cl, [eax]
;        mov     [edi], cl
;@@nextSpan:
;        fstp    st
;        fstp    st
;        fstp    st
;        fstp    st
;        fstp    st
;        fstp    st
;        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]
;@@end_proc:
;GFXRenderSpanUVWS ENDP

END
