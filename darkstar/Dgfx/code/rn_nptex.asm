; ========================================================
;
;   $Workfile:   rn_nptex.asm  $
;   $Version$
;
;    Non-perspective textured renderers
;
;  $Revision:$
;       $Log:$
;
; (c) Copyright 1997, Dynamix Inc.  All rights reserved.
; ========================================================

.486
.MODEL FLAT, syscall

; =========================================================
;   DATA
; =========================================================
.DATA

INCLUDE g_contxt.inc
INCLUDE g_macros.inc

uMask                       dd ?
vMask                       dd ?
uRol                        dd ?
vRol                        dd ?
fpConstant1div65536         dd 1.52587890625e-5
yval                        dd ?
xval                        dd ?
uFracStep                   dd ?
vFracStep                   dd ?

textureMapPatch             equ 01010101h
translucencyMapPatch        equ 01010101h
suWholePatch                equ 01h
ssFractionPatch             equ 01010101h
dstLastWordPatch            equ 01010101h

uFracStepPatch          equ 01010101h
vFracStepPatch          equ 01010101h
fsuPatch                equ 01010101h
fssPatch                equ 01010101h
fsswPatch               equ 01h
fsvPatch                equ 01010101h
uMaskPatch              equ 01010101h
vMaskPatch              equ 01010101h

_esp        dd ?

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

.CODE

IFDEF __MASM__
ARG equ
ENDIF

;================================================================
; NAME
;   GFXRenderSpanUVCT
;
; DESCRIPTION
;   draw a single textured span passed in gfxSpan
;
; ARGUMENTS
;
;================================================================

GFXRenderSpanUVCT PROC C
        PUBLIC C GFXRenderSpanUVCT
        ScaleTexCoords
        push    ebx
        push    ecx
        push    edi
        lea     eax, @@drawSpan
        mov     gfxPDC.pdc_rendSpanEntry, eax
        fld     gfxPDC.pdc_dudx
        fistp   gfxPDC.pdc_fsu
        fld     gfxPDC.pdc_dvdx
        fistp   gfxPDC.pdc_fsv

        ; gfxPDC.pdc_strideTable = stride + stride * stepVWhole + stepUWhole
        ; gfxPDC.pdc_strideLookup = stride * stepVWhole + stepUWhole

        mov     eax, gfxPDC.pdc_fsv
        mov     ebx, gfxPDC.pdc_fsu
        sar     ebx, 16
        mov     edx, eax
        shl     edx, 16
        mov     gfxPDC.pdc_fsv, edx
        mov     ecx, gfxPDC.pdc_textureMapStride
        sar     eax, 16
        imul    ecx
        add     eax, ebx
        mov     gfxPDC.pdc_strideLookup, eax
        add     eax, ecx
        mov     gfxPDC.pdc_strideTable, eax
        mov     ebx, gfxPDC.pdc_fsu
        shl     ebx, 16
        mov     gfxPDC.pdc_fsu, ebx

        pop     edi
        pop     ecx
        pop     ebx
        RET
@@drawSpan:
        ; edi has xLeft, esi has count, eax is y-val.
        mov     yval, eax
        mov     xval, edi

        fild    xval
        fild    yval

        fld     gfxPDC.pdc_dudx
        fmul    st, st(2)
        fld     gfxPDC.pdc_dudy
        fmul    st, st(2)
        ; st = dudy * y, dudx * x, y, x
        fld     gfxPDC.pdc_dvdy
        fmulp   st(3), st
        faddp   st(1), st
        ; st = dudy * y + dudx * x, dvdy * y, x
        fld     gfxPDC.pdc_dvdx
        fmulp   st(3), st
        fadd    gfxPDC.pdc_u0
        fxch    st(1)
        faddp   st(2), st
        ; st = u, dvdx * x + dvdy * y
        fistp   gfxPDC.pdc_fu
        fadd    gfxPDC.pdc_v0
        fistp   gfxPDC.pdc_fv

        add     edi, gfxPDC.pdc_rowAddr
        mov     ebp, esi
        add     ebp, edi ; ebp has dstLast
        mov     ecx, gfxPDC.pdc_textureMapStride ; we will mul v by stride
        dec     ebp
        mov     eax, gfxPDC.pdc_fv ; put start v in eax for mul.
        mov     gfxPDC.pdc_dstLast, ebp
        dec     ebp
        and     ebp, NOT 1 ; calc the dstLastWord.
        mov     ebx, eax ; save off v... we'll use it for frac v.
        sar     eax, 16 ; get whole portion of v.
        mov     esp, ebp ; esp gets dstLastWord
        shl     ebx, 16 ; ebx has frac portion of start v.

        imul    ecx

        mov     esi, eax ; esi is where the textureMap ptr will end up.
        mov     edx, gfxPDC.pdc_fu
        sar     edx, 16
        add     esi, edx
        mov     edx, gfxPDC.pdc_fu
        shl     edx, 16
        xor     eax, eax
        add     esi, gfxPDC.pdc_textureMap

        ; set esi, edi, (ebp), edx (fracU), ebx (fracV)
        ; ecx = gfxPDC.pdc_fsv
        mov     ecx, gfxPDC.pdc_fsv
        mov     esp, gfxPDC.pdc_fsu
        mov     eax, gfxPDC.pdc_shadeMap
        ; get ebp set for the first pixel.

@@pixelLoop:        
        add     ebx, ecx
        sbb     ebp, ebp

        inc     edi
        mov     al, [esi]

        add     edx, esp
        mov     ebp, gfxPDC.pdc_strideLookup[ebp*4]

        adc     esi, ebp
        mov     ebp, gfxPDC.pdc_dstLast

        cmp     al, 0
        mov     al, [eax]

        je      @@endComp
        mov     [edi-1], al

@@endComp:
        cmp     edi, ebp
        jbe     @@pixelLoop

        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]
GFXRenderSpanUVCT ENDP

;================================================================
; NAME
;   GFXRenderSpanUVA
;
; DESCRIPTION
;   draw a single textured span passed in gfxSpan
;
; ARGUMENTS
;
;================================================================

GFXRenderSpanUVA PROC C
        PUBLIC C GFXRenderSpanUVA
        ScaleTexCoords
        push    ebx
        push    ecx
        push    edi
        lea     eax, @@drawSpan
        mov     gfxPDC.pdc_rendSpanEntry, eax
        fld     gfxPDC.pdc_dudx
        fistp   gfxPDC.pdc_fsu
        fld     gfxPDC.pdc_dvdx
        fistp   gfxPDC.pdc_fsv

        ; gfxPDC.pdc_strideTable = stride + stride * stepVWhole + stepUWhole
        ; gfxPDC.pdc_strideLookup = stride * stepVWhole + stepUWhole

        mov     eax, gfxPDC.pdc_fsv
        mov     ebx, gfxPDC.pdc_fsu
        sar     ebx, 16
        mov     edx, eax
        shl     edx, 16
        mov     gfxPDC.pdc_fsv, edx
        mov     ecx, gfxPDC.pdc_textureMapStride
        sar     eax, 16
        imul    ecx
        add     eax, ebx
        mov     gfxPDC.pdc_strideLookup, eax
        add     eax, ecx
        mov     gfxPDC.pdc_strideTable, eax
        mov     ebx, gfxPDC.pdc_fsu
        shl     ebx, 16
        mov     gfxPDC.pdc_fsu, ebx

        pop     edi
        pop     ecx
        pop     ebx
        RET
@@drawSpan:
        ; edi has xLeft, esi has count, eax is y-val.
        mov     yval, eax
        mov     xval, edi

        fild    xval
        fild    yval

        fld     gfxPDC.pdc_dudx
        fmul    st, st(2)
        fld     gfxPDC.pdc_dudy
        fmul    st, st(2)
        ; st = dudy * y, dudx * x, y, x
        fld     gfxPDC.pdc_dvdy
        fmulp   st(3), st
        faddp   st(1), st
        ; st = dudy * y + dudx * x, dvdy * y, x
        fld     gfxPDC.pdc_dvdx
        fmulp   st(3), st
        fadd    gfxPDC.pdc_u0
        fxch    st(1)
        faddp   st(2), st
        ; st = u, dvdx * x + dvdy * y
        fistp   gfxPDC.pdc_fu
        fadd    gfxPDC.pdc_v0
        fistp   gfxPDC.pdc_fv

        add     edi, gfxPDC.pdc_rowAddr
        mov     ebp, esi
        add     ebp, edi ; ebp has dstLast
        mov     ecx, gfxPDC.pdc_textureMapStride ; we will mul v by stride
        dec     ebp
        mov     eax, gfxPDC.pdc_fv ; put start v in eax for mul.
        mov     gfxPDC.pdc_dstLast, ebp
        dec     ebp
        and     ebp, NOT 1 ; calc the dstLastWord.
        mov     ebx, eax ; save off v... we'll use it for frac v.
        sar     eax, 16 ; get whole portion of v.
        mov     esp, ebp ; esp gets dstLastWord
        shl     ebx, 16 ; ebx has frac portion of start v.

        imul    ecx

        mov     esi, eax ; esi is where the textureMap ptr will end up.
        mov     edx, gfxPDC.pdc_fu
        sar     edx, 16
        add     esi, edx
        mov     edx, gfxPDC.pdc_fu
        shl     edx, 16
        xor     eax, eax
        add     esi, gfxPDC.pdc_textureMap

        ; set esi, edi, (ebp), edx (fracU), ebx (fracV)
        ; ecx = gfxPDC.pdc_fsv
        mov     esp, gfxPDC.pdc_fsu
      mov      ecx, gfxPDC.pdc_shadeMap
        ; get ebp set for the first pixel.
      mov      ebp, gfxPDC.pdc_fsv
      
@@pixelLoop:        
        add     ebx, ebp
      mov      al, [edi]
        
        sbb     ebp, ebp
      mov      ah, [esi]
      
      inc      edi
      add      edx, esp

      mov      ebp, gfxPDC.pdc_strideLookup[ebp*4]
      mov      al, [ecx + eax]

      adc      esi, ebp
      mov      [edi-1], al

      mov      ebp, gfxPDC.pdc_dstLast
      cmp      edi, ebp

      mov      ebp, gfxPDC.pdc_fsv
      jbe      @@pixelLoop

        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]
GFXRenderSpanUVA ENDP

;================================================================
; NAME
;   GFXRenderSpanUVC
;
; DESCRIPTION
;   draw a single textured span passed in gfxSpan
;
; ARGUMENTS
;
;================================================================

GFXRenderSpanUVC PROC C
        PUBLIC C GFXRenderSpanUVC
        ScaleTexCoords
        push    ebx
        push    ecx
        push    edi
        lea     eax, @@drawSpan
        mov     gfxPDC.pdc_rendSpanEntry, eax
        fld     gfxPDC.pdc_dudx
        fistp   gfxPDC.pdc_fsu
        fld     gfxPDC.pdc_dvdx
        fistp   gfxPDC.pdc_fsv

        ; gfxPDC.pdc_strideTable = stride + stride * stepVWhole + stepUWhole
        ; gfxPDC.pdc_strideLookup = stride * stepVWhole + stepUWhole

        mov     eax, gfxPDC.pdc_fsv
        mov     ebx, gfxPDC.pdc_fsu
        sar     ebx, 16
        mov     edx, eax
        shl     edx, 16
        mov     gfxPDC.pdc_fsv, edx
        mov     ecx, gfxPDC.pdc_textureMapStride
        sar     eax, 16
        imul    ecx
        add     eax, ebx
        mov     gfxPDC.pdc_strideLookup, eax
        add     eax, ecx
        mov     gfxPDC.pdc_strideTable, eax
        mov     ebx, gfxPDC.pdc_fsu
        shl     ebx, 16
        mov     gfxPDC.pdc_fsu, ebx

        pop     edi
        pop     ecx
        pop     ebx
        RET
@@drawSpan:
        ; edi has xLeft, esi has count, eax is y-val.
        mov     yval, eax
        mov     xval, edi

        fild    xval
        fild    yval

        fld     gfxPDC.pdc_dudx
        fmul    st, st(2)
        fld     gfxPDC.pdc_dudy
        fmul    st, st(2)
        ; st = dudy * y, dudx * x, y, x
        fld     gfxPDC.pdc_dvdy
        fmulp   st(3), st
        faddp   st(1), st
        ; st = dudy * y + dudx * x, dvdy * y, x
        fld     gfxPDC.pdc_dvdx
        fmulp   st(3), st
        fadd    gfxPDC.pdc_u0
        fxch    st(1)
        faddp   st(2), st
        ; st = u, dvdx * x + dvdy * y
        fistp   gfxPDC.pdc_fu
        fadd    gfxPDC.pdc_v0
        fistp   gfxPDC.pdc_fv

        add     edi, gfxPDC.pdc_rowAddr
        mov     ebp, esi
        add     ebp, edi ; ebp has dstLast
        mov     ecx, gfxPDC.pdc_textureMapStride ; we will mul v by stride
        dec     ebp
        mov     eax, gfxPDC.pdc_fv ; put start v in eax for mul.
        mov     gfxPDC.pdc_dstLast, ebp
        dec     ebp
        and     ebp, NOT 1 ; calc the dstLastWord.
        mov     ebx, eax ; save off v... we'll use it for frac v.
        sar     eax, 16 ; get whole portion of v.
        mov     esp, ebp ; esp gets dstLastWord
        shl     ebx, 16 ; ebx has frac portion of start v.

        imul    ecx

        mov     esi, eax ; esi is where the textureMap ptr will end up.
        mov     edx, gfxPDC.pdc_fu
        sar     edx, 16
        add     esi, edx
        mov     edx, gfxPDC.pdc_fu
        shl     edx, 16
        xor     eax, eax
        add     esi, gfxPDC.pdc_textureMap

        ; set esi, edi, (ebp), edx (fracU), ebx (fracV)
        ; ecx = gfxPDC.pdc_fsv
        mov     ecx, gfxPDC.pdc_fsv
        mov     esp, gfxPDC.pdc_fsu
      mov      eax, gfxPDC.pdc_shadeMap
        ; get ebp set for the first pixel.

@@pixelLoop:        
        add     ebx, ecx
        sbb     ebp, ebp

      inc       edi
      mov      al, [esi]

      add      edx, esp
      mov      ebp, gfxPDC.pdc_strideLookup[ebp*4]

      adc      esi, ebp
      mov      al, [eax]

      mov      [edi-1], al
      mov      ebp, gfxPDC.pdc_dstLast

      cmp      edi, ebp
      jbe      @@pixelLoop

        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]
GFXRenderSpanUVC ENDP

;================================================================
; NAME
;   GFXRenderSpanUVS
;
; DESCRIPTION
;   draw a single textured span passed in gfxSpan
;
; ARGUMENTS
;
;================================================================

.data
UVSwritable dd 0
.code

GFXRenderSpanUVS PROC C
        PUBLIC C GFXRenderSpanUVS
        mark_writable UVSwritable
@@start_proc:
        ScaleTexCoords
        push    ebx
        push    ecx
        push    edi
        lea     eax, @@drawSpan
        mov     gfxPDC.pdc_rendSpanEntry, eax
        fld     gfxPDC.pdc_dudx
        fistp   gfxPDC.pdc_fsu
        fld     gfxPDC.pdc_dvdx
        fistp   gfxPDC.pdc_fsv
        fld     gfxPDC.pdc_dsdx
        fistp   gfxPDC.pdc_fss

        ; gfxPDC.pdc_strideTable = stride + stride * stepVWhole + stepUWhole
        ; gfxPDC.pdc_strideLookup = stride * stepVWhole + stepUWhole

        mov     eax, gfxPDC.pdc_fsv
        mov     ebx, gfxPDC.pdc_fsu
        sar     ebx, 16
        mov     edx, eax
        shl     edx, 16
        mov     ecx, gfxPDC.pdc_textureMapStride
        sar     eax, 16
        mov     DWORD PTR [@@vFracStepPatch1 - 4], edx                              
        mov     DWORD PTR [@@vFracStepPatch2 - 4], edx                              
        mov     DWORD PTR [@@vFracStepPatch3 - 4], edx                              
        mov     DWORD PTR [@@vFracStepPatch4 - 4], edx                              
        imul    ecx
        add     eax, ebx
        mov     gfxPDC.pdc_strideLookup, eax
        add     eax, ecx
        mov     gfxPDC.pdc_strideTable, eax

        mov     eax, gfxPDC.pdc_fsu
        shl     eax, 16
        mov     DWORD PTR [@@uFracStepPatch1 - 4], eax
        mov     DWORD PTR [@@uFracStepPatch2 - 4], eax
        mov     DWORD PTR [@@uFracStepPatch3 - 4], eax

        mov     eax, gfxPDC.pdc_fss
        rol     eax, 16
        mov     DWORD PTR [@@fssPatch1 - 4], eax
        mov     BYTE PTR  [@@fsswPatch1 - 1], al
        mov     DWORD PTR [@@fssPatch2 - 4], eax
        mov     BYTE PTR  [@@fsswPatch2 - 1], al
        mov     DWORD PTR [@@fssPatch3 - 4], eax
        mov     BYTE PTR  [@@fsswPatch3 - 1], al

        pop     edi
        pop     ecx
        pop     ebx
        RET
@@drawSpan:
        ; edi has xLeft, esi has count, eax is yVal.
        mov     yval, eax
        mov     xval, edi

        fild    xval
        fild    yval

        fld     gfxPDC.pdc_dsdx
        fmul    st, st(2)
        fld     gfxPDC.pdc_dsdy
        fmul    st, st(2)
        fadd    gfxPDC.pdc_s0
        faddp   st(1), st
        fistp   gfxPDC.pdc_fs

        fld     gfxPDC.pdc_dudx
        fmul    st, st(2)
        fld     gfxPDC.pdc_dudy
        fmul    st, st(2)
        ; st = dudy * y, dudx * x, y, x
        fld     gfxPDC.pdc_dvdy
        fmulp   st(3), st
        faddp   st(1), st
        ; st = dudy * y + dudx * x, dvdy * y, x
        fld     gfxPDC.pdc_dvdx
        fmulp   st(3), st
        fadd    gfxPDC.pdc_u0
        fxch    st(1)
        faddp   st(2), st
        ; st = u, dvdx * x + dvdy * y
        fistp   gfxPDC.pdc_fu
        fadd    gfxPDC.pdc_v0
        fistp   gfxPDC.pdc_fv

        add     edi, gfxPDC.pdc_rowAddr
        mov     ebp, esi
        add     ebp, edi ; ebp has dstLast
        mov     ecx, gfxPDC.pdc_textureMapStride ; we will mul v by stride
        dec     ebp
        mov     eax, gfxPDC.pdc_fv ; put start v in eax for mul.
        mov     gfxPDC.pdc_dstLast, ebp
        dec     ebp
        and     ebp, NOT 1 ; calc the dstLastWord.
        mov     ebx, eax ; save off v... we'll use it for frac v.
        sar     eax, 16 ; get whole portion of v.
        mov     DWORD PTR [@@dstLastWordPatch1 - 4], ebp
        shl     ebx, 16 ; ebx has frac portion of start v.
        mov     gfxPDC.pdc_dstLastWord, ebp

        imul    ecx

        mov     esi, eax ; esi is where the textureMap ptr will end up.
        mov     edx, gfxPDC.pdc_fu
        sar     edx, 16
        mov     ecx, gfxPDC.pdc_fu
        shl     ecx, 16
        add     esi, edx
        mov     edx, ecx
        mov     ecx, gfxPDC.pdc_fs
        rol     ecx, 16
        xor     eax, eax
        add     esi, gfxPDC.pdc_textureMap
        mov     ah, cl
        
        ; ebx = vFrac
        ; edx = uFrac
        ; esi = texture + uWhole + vWhole * stride
        ; ebp = ?
        ; edi = dstPtr


        and     ecx, 0FFFF0000h
        or      eax, gfxPDC.pdc_shadeMap
        mov     esp, ecx
        
        test    edi, 1
        jz      @@checkDone
        ; used eax, ebx, edi, esi, edx, ebp, esp
        mov     al, [esi]
        add     ebx, vFracStepPatch
                @@vFracStepPatch1:
        sbb     ebp, ebp
        inc     edi
        mov     al, [eax]
        add     esp, fssPatch
                @@fssPatch1:
        adc     ah, fsswPatch
                @@fsswPatch1:
        add     edx, uFracStepPatch
                @@uFracStepPatch1:
        adc     esi, gfxPDC.pdc_strideLookup[ebp * 4]
        mov     [edi-1], al
@@checkDone:
        cmp     edi, gfxPDC.pdc_dstLastWord
        ja      @@checkLast
        add     ebx, vFracStepPatch
                @@vFracStepPatch2:
        sbb     ebp, ebp
@@wordLoop:
        mov     al, [esi]
        add     edx, uFracStepPatch
                @@uFracStepPatch2:

        adc     esi, gfxPDC.pdc_strideLookup[ebp * 4]
        add     ebx, vFracStepPatch
                @@vFracStepPatch3:

        sbb     ebp, ebp
        mov     cl, [eax]

        mov     al, [esi]
        add     esp, fssPatch
                @@fssPatch2:
        
        adc     ah, fsswPatch
                @@fsswPatch2:
        add     edx, uFracStepPatch
                @@uFracStepPatch3:
        
        adc     esi, gfxPDC.pdc_strideLookup[ebp * 4]
        add     edi, 2

        mov     ch, [eax]
        add     esp, fssPatch
                @@fssPatch3:
        
        adc     ah, fsswPatch
                @@fsswPatch3:
        add     ebx, vFracStepPatch
                @@vFracStepPatch4:

        sbb     ebp, ebp
        cmp     edi, dstLastWordPatch
                @@dstLastWordPatch1:

        mov     [edi-2], cx
        jbe     @@wordLoop
@@checkLast:
        cmp     edi, gfxPDC.pdc_dstLast
        ja      @@nextRow
        mov     al, [esi]
        mov     cl, [eax]
        mov     [edi], cl
@@nextRow:
        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]
@@end_proc:
GFXRenderSpanUVS ENDP

;================================================================
; NAME
;   GFXRenderSpanUVST
;
; DESCRIPTION
;   draw a single textured span passed in gfxSpan
;
; ARGUMENTS
;
;================================================================

.data
UVSTwritable dd 0
.code

GFXRenderSpanUVST PROC C
        PUBLIC C GFXRenderSpanUVST
        mark_writable UVSTwritable
@@start_proc:
        ScaleTexCoords
        push    ebx
        push    ecx
        push    edi
        lea     eax, @@drawSpan
        mov     gfxPDC.pdc_rendSpanEntry, eax
        fld     gfxPDC.pdc_dudx
        fistp   gfxPDC.pdc_fsu
        fld     gfxPDC.pdc_dvdx
        fistp   gfxPDC.pdc_fsv
        fld     gfxPDC.pdc_dsdx
        fistp   gfxPDC.pdc_fss

        ; gfxPDC.pdc_strideTable = stride + stride * stepVWhole + stepUWhole
        ; gfxPDC.pdc_strideLookup = stride * stepVWhole + stepUWhole

        mov     eax, gfxPDC.pdc_fsv
        mov     ebx, gfxPDC.pdc_fsu
        sar     ebx, 16
        mov     edx, eax
        shl     edx, 16
        mov     ecx, gfxPDC.pdc_textureMapStride
        sar     eax, 16
        mov     DWORD PTR [@@vFracStepPatch1 - 4], edx                              
        imul    ecx
        add     eax, ebx
        mov     gfxPDC.pdc_strideLookup, eax
        add     eax, ecx
        mov     gfxPDC.pdc_strideTable, eax

        mov     eax, gfxPDC.pdc_fsu
        shl     eax, 16
        mov     DWORD PTR [@@uFracStepPatch1 - 4], eax

        mov     eax, gfxPDC.pdc_fss
        rol     eax, 16
        mov     DWORD PTR [@@fssPatch1 - 4], eax
        mov     BYTE PTR [@@fsswPatch1 - 1], al

        pop     edi
        pop     ecx
        pop     ebx
        RET
@@drawSpan:
        ; edi has xLeft, esi has count, eax is y-val.
        mov     yval, eax
        mov     xval, edi

        fild    xval
        fild    yval

        fld     gfxPDC.pdc_dsdx
        fmul    st, st(2)
        fld     gfxPDC.pdc_dsdy
        fmul    st, st(2)
        fadd    gfxPDC.pdc_s0
        faddp   st(1), st
        fistp   gfxPDC.pdc_fs

        fld     gfxPDC.pdc_dudx
        fmul    st, st(2)
        fld     gfxPDC.pdc_dudy
        fmul    st, st(2)
        ; st = dudy * y, dudx * x, y, x
        fld     gfxPDC.pdc_dvdy
        fmulp   st(3), st
        faddp   st(1), st
        ; st = dudy * y + dudx * x, dvdy * y, x
        fld     gfxPDC.pdc_dvdx
        fmulp   st(3), st
        fadd    gfxPDC.pdc_u0
        fxch    st(1)
        faddp   st(2), st
        ; st = u, dvdx * x + dvdy * y
        fistp   gfxPDC.pdc_fu
        fadd    gfxPDC.pdc_v0
        fistp   gfxPDC.pdc_fv

        add     edi, gfxPDC.pdc_rowAddr
        mov     ebp, esi
        add     ebp, edi ; ebp has dstLast
        mov     ecx, gfxPDC.pdc_textureMapStride ; we will mul v by stride
        dec     ebp
        mov     eax, gfxPDC.pdc_fv ; put start v in eax for mul.
        mov     gfxPDC.pdc_dstLast, ebp
        dec     ebp
        and     ebp, NOT 1 ; calc the dstLastWord.
        mov     ebx, eax ; save off v... we'll use it for frac v.
        sar     eax, 16 ; get whole portion of v.
;        mov     DWORD PTR [@@dstLastWordPatch1 - 4], ebp      ;;;;; DMMNOTENOTENOTE: Fix this...
        shl     ebx, 16 ; ebx has frac portion of start v.
        mov     gfxPDC.pdc_dstLastWord, ebp

        imul    ecx

        mov     esi, eax ; esi is where the textureMap ptr will end up.
        mov     edx, gfxPDC.pdc_fu
        sar     edx, 16
        mov     ecx, gfxPDC.pdc_fu
        shl     ecx, 16
        add     esi, edx
        mov     edx, ecx
        mov     ecx, gfxPDC.pdc_fs
        rol     ecx, 16
        xor     eax, eax
        add     esi, gfxPDC.pdc_textureMap
        mov     ah, cl
        
        ; ebx = vFrac
        ; edx = uFrac
        ; esi = texture + uWhole + vWhole * stride
        ; ebp = ?
        ; edi = dstPtr


        and     ecx, 0FFFF0000h
        or      eax, gfxPDC.pdc_shadeMap
        mov     esp, ecx

@@pixelLoop:
        mov     al, [esi]
        add     ebx, vFracStepPatch
                @@vFracStepPatch1:

        sbb     ebp, ebp
        inc     edi

        cmp     al, 0
        mov     al, [eax]

        je      @@skipWrite
        mov     [edi-1], al

@@skipWrite:
        add     esp, fssPatch
                @@fssPatch1:
        adc     ah, fsswPatch
                @@fsswPatch1:

        add     edx, uFracStepPatch
                @@uFracStepPatch1:
        adc     esi, gfxPDC.pdc_strideLookup[ebp * 4]

        cmp     edi, gfxPDC.pdc_dstLast
        jbe     @@pixelLoop
@@nextRow:
        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]
@@end_proc:
GFXRenderSpanUVST ENDP


;================================================================
; NAME
;   GFXRenderSpanUVSR
;
; DESCRIPTION
;   draw a single textured span passed in gfxSpan
;
; ARGUMENTS
;
;================================================================

.data
UVSRwritable    dd  0
.code

GFXRenderSpanUVSR PROC C
        PUBLIC C GFXRenderSpanUVSR
        mark_writable UVSRwritable
@@start_proc:
        ScaleTexCoords
        push    ebx
        push    ecx
        mov     ebx, gfxPDC.pdc_textureMap
        mov     DWORD PTR [@@textureMapPatch1 - 4], ebx
        lea     eax, @@drawSpan
        mov     gfxPDC.pdc_rendSpanEntry, eax
        fld     gfxPDC.pdc_dudx
        fistp   gfxPDC.pdc_fsu
        fld     gfxPDC.pdc_dvdx
        fistp   gfxPDC.pdc_fsv
        fld     gfxPDC.pdc_dsdx
        fistp   gfxPDC.pdc_fss

        ; Calculate uRol, vRol, uMask and vMask
        mov     eax, gfxPDC.pdc_textureMapStride
        dec     eax
        ; eax has uMask
        mov     DWORD PTR [@@uMaskPatch1 - 4], eax
        mov     edx, 0
        mov     uRol, 16 ; uRol is always 16 bits.
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

        mov     eax, gfxPDC.pdc_fss
        rol     eax, 16
        mov     gfxPDC.pdc_fss, eax

        mov     ecx, uRol
        mov     eax, gfxPDC.pdc_fsu
        rol     eax, cl
        mov     DWORD PTR [@@fsuPatch1 - 4], eax
        mov     eax, gfxPDC.pdc_fsv
        mov     ecx, vRol
        rol     eax, cl
        mov     DWORD PTR [@@fsvPatch1 - 4], eax
        pop     ecx
        pop     ebx
        RET
@@drawSpan:
        ; edi has xLeft, esi has count, eax is y-val.
        mov     yval, eax
        mov     xval, edi

        fild    xval
        fild    yval

        fld     gfxPDC.pdc_dsdx
        fmul    st, st(2)
        fld     gfxPDC.pdc_dsdy
        fmul    st, st(2)
        fadd    gfxPDC.pdc_s0
        faddp   st(1), st
        fistp   gfxPDC.pdc_fs

        fld     gfxPDC.pdc_dudx
        fmul    st, st(2)
        fld     gfxPDC.pdc_dudy
        fmul    st, st(2)
        ; st = dudy * y, dudx * x, y, x
        fld     gfxPDC.pdc_dvdy
        fmulp   st(3), st
        faddp   st(1), st
        ; st = dudy * y + dudx * x, dvdy * y, x
        fld     gfxPDC.pdc_dvdx
        fmulp   st(3), st
        fadd    gfxPDC.pdc_u0
        fxch    st(1)
        faddp   st(2), st
        ; st = u, dvdx * x + dvdy * y
        fistp   gfxPDC.pdc_fu
        fadd    gfxPDC.pdc_v0
        fistp   gfxPDC.pdc_fv

        add     edi, gfxPDC.pdc_rowAddr
        mov     ebp, esi
        add     ebp, edi ; ebp has dstLast
        dec     ebp
        mov     gfxPDC.pdc_dstLast, ebp

        mov     eax, gfxPDC.pdc_fu
        mov     edx, gfxPDC.pdc_fv
        mov     ecx, uRol
        rol     eax, cl
        mov     ecx, vRol
        rol     edx, cl
        ; eax has fu, edx has fv...

        mov     ebp, gfxPDC.pdc_textureMap
        mov     ebx, gfxPDC.pdc_fs
        rol     ebx, 16
        mov     ebp, ebx
        and     ebp, 0FFFF0000h
        mov     bh, bl
        and     ebx, 0FF00h
        or      ebx, gfxPDC.pdc_shadeMap
        mov     esp, gfxPDC.pdc_dstLast

@@pixelLoop:
        mov     esi, eax    ;t1 = u
        mov     ecx, edx    ;t2 = v

        and     esi, uMaskPatch ; mask off non-whole portions of the u/v values
                @@uMaskPatch1:
        and     ecx, vMaskPatch 
                @@vMaskPatch1:

        or      esi, ecx        ; combine the result.
        add     eax, fsuPatch
                @@fsuPatch1:

        adc     eax, 0
        add     edx, fsvPatch
                @@fsvPatch1:

        mov     bl, [esi + textureMapPatch] ; read the texture
                @@textureMapPatch1:
        mov     ecx, gfxPDC.pdc_fss

        adc     edx, 0
        inc     edi

        add     ebp, ecx
        mov     ch, [ebx]

        adc     bh, cl
        cmp     edi, esp
    
        mov     [edi-1], ch
        jbe     @@pixelLoop
@@nextRow:
        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]
@@end_proc:
GFXRenderSpanUVSR ENDP

;================================================================
; NAME
;   GFXRenderSpanUV
;
; DESCRIPTION
;   draw a single textured span passed in gfxSpan
;
; ARGUMENTS
;
;================================================================

GFXRenderSpanUV PROC C
        PUBLIC C GFXRenderSpanUV
        ScaleTexCoords
        push    ebx
        push    ecx
        push    edi
        lea     eax, @@drawSpan
        mov     gfxPDC.pdc_rendSpanEntry, eax
        fld     gfxPDC.pdc_dudx
        fistp   gfxPDC.pdc_fsu
        fld     gfxPDC.pdc_dvdx
        fistp   gfxPDC.pdc_fsv

        ; gfxPDC.pdc_strideTable = stride + stride * stepVWhole + stepUWhole
        ; gfxPDC.pdc_strideLookup = stride * stepVWhole + stepUWhole

        mov     eax, gfxPDC.pdc_fsv
        mov     ebx, gfxPDC.pdc_fsu
        sar     ebx, 16
        mov     edx, eax
        shl     edx, 16
        mov     gfxPDC.pdc_fsv, edx
        mov     ecx, gfxPDC.pdc_textureMapStride
        sar     eax, 16
        imul    ecx
        add     eax, ebx
        mov     gfxPDC.pdc_strideLookup, eax
        add     eax, ecx
        mov     gfxPDC.pdc_strideTable, eax
        mov     ebx, gfxPDC.pdc_fsu
        shl     ebx, 16
        mov     gfxPDC.pdc_fsu, ebx

        pop     edi
        pop     ecx
        pop     ebx
        RET
@@drawSpan:
        ; edi has xLeft, esi has count, eax is y-val.
        mov     yval, eax
        mov     xval, edi

        fild    xval
        fild    yval

        fld     gfxPDC.pdc_dudx
        fmul    st, st(2)
        fld     gfxPDC.pdc_dudy
        fmul    st, st(2)
        ; st = dudy * y, dudx * x, y, x
        fld     gfxPDC.pdc_dvdy
        fmulp   st(3), st
        faddp   st(1), st
        ; st = dudy * y + dudx * x, dvdy * y, x
        fld     gfxPDC.pdc_dvdx
        fmulp   st(3), st
        fadd    gfxPDC.pdc_u0
        fxch    st(1)
        faddp   st(2), st
        ; st = u, dvdx * x + dvdy * y
        fistp   gfxPDC.pdc_fu
        fadd    gfxPDC.pdc_v0
        fistp   gfxPDC.pdc_fv

        add     edi, gfxPDC.pdc_rowAddr
        mov     ebp, esi
        add     ebp, edi ; ebp has dstLast
        mov     ecx, gfxPDC.pdc_textureMapStride ; we will mul v by stride
        dec     ebp
        mov     eax, gfxPDC.pdc_fv ; put start v in eax for mul.
        mov     gfxPDC.pdc_dstLast, ebp
        dec     ebp
        and     ebp, NOT 1 ; calc the dstLastWord.
        mov     ebx, eax ; save off v... we'll use it for frac v.
        sar     eax, 16 ; get whole portion of v.
        mov     esp, ebp ; esp gets dstLastWord
        shl     ebx, 16 ; ebx has frac portion of start v.

        imul    ecx

        mov     esi, eax ; esi is where the textureMap ptr will end up.
        mov     edx, gfxPDC.pdc_fu
        sar     edx, 16
        add     esi, edx
        mov     edx, gfxPDC.pdc_fu
        shl     edx, 16
        xor     eax, eax
        add     esi, gfxPDC.pdc_textureMap

        ; set esi, edi, (ebp), edx (fracU), ebx (fracV)
        ; ecx = gfxPDC.pdc_fsv
        mov     ecx, gfxPDC.pdc_fsv
        mov     eax, gfxPDC.pdc_fsu

        ; get ebp set for the first pixel.
        add     ebx, ecx
        sbb     ebp, ebp

        test    edi, 1
        jz      @@checkDone
        ;Draw a pixel to Get us on a WORD boundry
        inc     edi
        mov     bl, [esi]
        add     edx, eax
        adc     esi, gfxPDC.pdc_strideLookup[ebp*4]
        add     ebx, ecx
        sbb     ebp, ebp
        mov     [edi-1], bl
@@checkDone:
        cmp     edi, esp ;dstLastWord
        ja      @@checkLast
@@wordLoop:
        mov     bl, [esi]
        add     edx, eax

        adc     esi, gfxPDC.pdc_strideLookup[ebp*4]
        add     ebx, ecx

        sbb     ebp, ebp
        add     edi, 2

        mov     bh, [esi]
        add     edx, eax
        
        adc     esi, gfxPDC.pdc_strideLookup[ebp*4]
        add     ebx, ecx

        sbb     ebp, ebp
        mov     [edi-2], bx
        
        cmp     edi, esp
        jbe     @@wordLoop
@@checkLast:
        cmp     edi, gfxPDC.pdc_dstLast
        ja      @@nextRow

        ;Draw the last pixel if not WORD aligned
        mov     bl, [esi]
        mov     [edi], bl               ;dest = pixel
@@nextRow:
        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]
GFXRenderSpanUV ENDP

;================================================================
; NAME
;   GFXRenderSpanUVT
;
; DESCRIPTION
;   draw a single textured span passed in gfxSpan
;
; ARGUMENTS
;
;================================================================

GFXRenderSpanUVT PROC C
        PUBLIC C GFXRenderSpanUVT
        ScaleTexCoords
        push    ebx
        push    ecx
        push    edi
        lea     eax, @@drawSpan
        mov     gfxPDC.pdc_rendSpanEntry, eax
        fld     gfxPDC.pdc_dudx
        fistp   gfxPDC.pdc_fsu
        fld     gfxPDC.pdc_dvdx
        fistp   gfxPDC.pdc_fsv

        ; gfxPDC.pdc_strideTable = stride + stride * stepVWhole + stepUWhole
        ; gfxPDC.pdc_strideLookup = stride * stepVWhole + stepUWhole

        mov     eax, gfxPDC.pdc_fsv
        mov     ebx, gfxPDC.pdc_fsu
        sar     ebx, 16
        mov     edx, eax
        shl     edx, 16
        mov     gfxPDC.pdc_fsv, edx
        mov     ecx, gfxPDC.pdc_textureMapStride
        sar     eax, 16
        imul    ecx
        add     eax, ebx
        mov     gfxPDC.pdc_strideLookup, eax
        add     eax, ecx
        mov     gfxPDC.pdc_strideTable, eax
        mov     ebx, gfxPDC.pdc_fsu
        shl     ebx, 16
        mov     gfxPDC.pdc_fsu, ebx

        pop     edi
        pop     ecx
        pop     ebx
        RET
@@drawSpan:
        ; edi has xLeft, esi has count, eax is y-val.
        mov     yval, eax
        mov     xval, edi

        fild    xval
        fild    yval

        fld     gfxPDC.pdc_dudx
        fmul    st, st(2)
        fld     gfxPDC.pdc_dudy
        fmul    st, st(2)
        ; st = dudy * y, dudx * x, y, x
        fld     gfxPDC.pdc_dvdy
        fmulp   st(3), st
        faddp   st(1), st
        ; st = dudy * y + dudx * x, dvdy * y, x
        fld     gfxPDC.pdc_dvdx
        fmulp   st(3), st
        fadd    gfxPDC.pdc_u0
        fxch    st(1)
        faddp   st(2), st
        ; st = u, dvdx * x + dvdy * y
        fistp   gfxPDC.pdc_fu
        fadd    gfxPDC.pdc_v0
        fistp   gfxPDC.pdc_fv

        add     edi, gfxPDC.pdc_rowAddr
        mov     ebp, esi
        add     ebp, edi ; ebp has dstLast
        mov     ecx, gfxPDC.pdc_textureMapStride ; we will mul v by stride
        dec     ebp
        mov     eax, gfxPDC.pdc_fv ; put start v in eax for mul.
        mov     gfxPDC.pdc_dstLast, ebp
        dec     ebp
        and     ebp, NOT 1 ; calc the dstLastWord.
        mov     ebx, eax ; save off v... we'll use it for frac v.
        sar     eax, 16 ; get whole portion of v.
        mov     esp, ebp ; dstLastWord into esp
        shl     ebx, 16 ; ebx has frac portion of start v.

        imul    ecx

        mov     esi, eax ; esi is where the textureMap ptr will end up.
        mov     edx, gfxPDC.pdc_fu
        sar     edx, 16
        add     esi, edx
        mov     edx, gfxPDC.pdc_fu
        shl     edx, 16
        xor     eax, eax
        add     esi, gfxPDC.pdc_textureMap

        ; set esi, edi, (ebp), edx (fracU), ebx (fracV)
        ; ecx = gfxPDC.pdc_fsv
        mov     ecx, gfxPDC.pdc_fsv
        mov     eax, gfxPDC.pdc_fsu

        ; get ebp set for the first pixel.
        add     ebx, ecx
        sbb     ebp, ebp

        test    edi, 1
        jz      @@checkDone
        ;Draw a pixel to Get us on a WORD boundry
@@firstPixel:
        mov     bl, [esi]
        add     edx, eax
        adc     esi, gfxPDC.pdc_strideLookup[ebp*4]
        add     ebx, ecx
        sbb     ebp, ebp
        cmp     bl, 0
        je         @@skipByte
        mov     [edi], bl
@@skipByte:
      inc       edi
@@checkDone:
      cmp      edi, esp ;dstLastWord
      ja       @@checkLast
        test       edi, 1
        jnz     @@firstPixel
@@wordLoop:
        mov     bl, [esi]
        add     edx, eax

        adc     esi, gfxPDC.pdc_strideLookup[ebp*4]
        add     ebx, ecx

        sbb     ebp, ebp
        cmp     bl, 0

        je         @@skipByte
        add     edi, 2

        mov     bh, [esi]
        add     edx, eax
        
        adc     esi, gfxPDC.pdc_strideLookup[ebp*4]
        add     ebx, ecx

        sbb     ebp, ebp
        cmp     bh, 0

        jne     @@wordWrite
        mov     [edi-2], bl

        cmp     edi, esp
        jbe     @@wordLoop

        jmp     @@checkLast
@@wordWrite:
        mov     [edi-2], bx
        cmp     edi, esp
        jbe     @@wordLoop
@@checkLast:
        cmp     edi, gfxPDC.pdc_dstLast
        ja      @@nextRow

        ;Draw the last pixel if not WORD aligned
        mov     bl, [esi]
        cmp     bl, 0
        je      @@nextRow
        mov     [edi], bl               ;dest = pixel
@@nextRow:
        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]
GFXRenderSpanUVT ENDP

END
