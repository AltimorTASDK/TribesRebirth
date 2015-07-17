; ========================================================
;
;   $Workfile:   rn_nontx.asm  $
;   $Version$
;
;    Filled span renderers
;
;  $Revision:$
;       $Log:$
;
; (c) Copyright 1995, Dynamix Inc.  All rights reserved.
; ========================================================


.486
.MODEL FLAT, syscall

.data
INCLUDE g_macros.inc
INCLUDE g_contxt.inc

IFDEF __MASM__
ARG equ
ENDIF

yval        dd    ?
xval        dd    ?

dstLastByte dd    ?

.code

GFXRenderSpan PROC C
    PUBLIC C GFXRenderSpan
    ; fill the color field with the whole DWORD
    mov     eax, gfxPDC.pdc_color
    mov     ah,al
    mov     ecx,eax
    bswap   eax
    or      eax,ecx
    mov     gfxPDC.pdc_colorsv, eax
    lea     eax, @@drawSpan
    mov     gfxPDC.pdc_rendSpanEntry, eax
    RET
@@drawSpan:
    ; edi = xLeft, esi = count, eax = yval.
    mov     edx, esi
    mov     eax, gfxPDC.pdc_colorsv

    ; count in edx, start in edi, color in eax
    mov     ebp, gfxPDC.pdc_rowAddr
    lea     edi, [edi + ebp]
    cmp     edx, 3
    jle     @@tail

    mov     ecx, edi
    neg     ecx

    and     ecx, 3      ; ecx holds 0-3 head bytes

    sub     edx, ecx
    mov     ebx, edx
    and     edx, 3      ; edx holds 0-3 tail bytes
    sub     ebx, edx
    shr     ebx, 2      ; ebx holds 0-n center words
    rep     stosb       ; write 0-3 head BYTES
    mov     ecx, ebx
    rep     stosd         ;write 0-n DWORDS
@@tail:
    mov     ecx, edx
    rep stosb         ;write 0-3 tail BYTES
    jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]
GFXRenderSpan ENDP


GFXRenderSpanC PROC C
        PUBLIC C GFXRenderSpanC

        ; fill the color field with the whole DWORD
        mov     eax, gfxPDC.pdc_color
        mov     ah,al
        mov     ecx,eax
        bswap   eax
        or      eax,ecx
        mov     gfxPDC.pdc_colorsv, eax
        lea     eax, @@drawSpan
        mov     gfxPDC.pdc_rendSpanEntry, eax
        RET
@@drawSpan:
        ; edi = xLeft, esi = count, eax = yval.
        mov     edx, esi
        mov     eax, gfxPDC.pdc_colorsv

        ; count in edx, start in edi, color in eax
        mov     ebp, gfxPDC.pdc_rowAddr
        lea     edi, [edi + ebp]
        cmp     edx, 3
        jle     @@tail

        mov     ecx, edi
        neg     ecx

        and     ecx, 3      ; ecx holds 0-3 head bytes

        sub     edx, ecx
        mov     ebx, edx
        and     edx, 3      ; edx holds 0-3 tail bytes
        sub     ebx, edx
        shr     ebx, 2      ; ebx holds 0-n center words
        rep stosb           ; write 0-3 head BYTES
        mov     ecx, ebx
        rep stosd         ;write 0-n DWORDS
@@tail:
        mov     ecx, edx
        rep stosb         ;write 0-3 tail BYTES
        jmp     DWORD PTR [gfxPDC.pdc_rendSpanExit]
GFXRenderSpanC ENDP


GFXRenderSpanS PROC C
        PUBLIC C GFXRenderSpanS
        lea     eax, @@drawSpan
        fld     gfxPDC.pdc_dsdx
        fistp   gfxPDC.pdc_fss
        mov     gfxPDC.pdc_rendSpanEntry, eax
        ret
@@drawSpan:
        ; gotta calc shade value for start of span.

        mov     yval, eax
        mov     xval, edi
        fld     gfxPDC.pdc_dsdx
        fimul   xval
        fld     gfxPDC.pdc_dsdy
        fimul   yval
        fxch    st(1)
        fadd    gfxPDC.pdc_s0
        faddp   st(1), st
        fistp   gfxPDC.pdc_fs

        mov     eax, gfxPDC.pdc_fs
        mov     ecx, gfxPDC.pdc_shadeMap
        rol     eax, 16
        mov     edx, gfxPDC.pdc_fss
        rol     edx, 16
        mov     cl, BYTE PTR [gfxPDC.pdc_color]
        add     edi, gfxPDC.pdc_rowAddr
        or      ch, al
        add     esi, edi
        ;regs: eax - s.f
        ;      ebx - pixels
        ;      ecx - shademap s.w color
        ;      edx - ss.f    ss.w
        ;      edi - dstPtr

        mov     ebp, esi
        sub     esi, 1
        sub     ebp, 4
        and     ebp, NOT 3

        ;      esi - dstLast
        ;      ebp - dstLastDword

        cmp     edi, ebp
        jg      @@lastPixels
        test    edi, 3
        jz      @@dwordLoop
@@firstPixels:
        inc     edi
        mov     bl, [ecx]
        add     eax, edx
        adc     ch, dl
        mov     [edi-1], bl
        test    edi, 3
        jnz     @@firstPixels
        cmp     edi, ebp
        jg      @@lastPixels
@@dwordLoop:
        mov     bl, [ecx]
        add     eax, edx
        adc     ch, dl
        add     edi, 4
        mov     bh, [ecx]
        add     eax, edx
        adc     ch, dl
        bswap   ebx
        mov     bh, [ecx]
        add     eax, edx
        adc     ch, dl
        mov     bl, [ecx]
        add     eax, edx
        adc     ch, dl
        bswap   ebx
        mov     [edi-4], ebx
        cmp     edi, ebp
        jle     @@dwordLoop
@@lastPixels:
        cmp     edi, esi
        jg      @@nextSpan
@@lastPixels2:
        inc     edi
        mov     bl, [ecx]
        add     eax, edx
        adc     ch, dl
        mov     [edi-1], bl
        cmp     edi, esi
        jle     @@lastPixels2
@@nextSpan:
        jmp     [gfxPDC.pdc_rendSpanExit]
GFXRenderSpanS ENDP


;------------------------------------------------------------------------------
; NAME 
;    GFXRenderSpanA
;    
; DESCRIPTION 
;    Renders a constant color alpha span, slowly...
;       DMMNOTE SLOWSLOWSLOW!
;    
; ARGUMENTS 
;    
;    
; RETURNS 
;    
;    
; NOTES 
;    
;------------------------------------------------------------------------------
GFXRenderSpanA PROC C
        PUBLIC C GFXRenderSpanA
        lea     eax, @@drawSpan
        mov     gfxPDC.pdc_rendSpanEntry, eax
        ret
@@drawSpan:
      ; edi has xLeft, esi has count, eax is y-val.
      add   edi, gfxPDC.pdc_rowAddr
      add   esi, edi
      
      dec   esi
      
      mov   dstLastByte, esi
      inc   esi
      
      mov   eax, gfxPDC.pdc_color
      mov   ecx, gfxPDC.pdc_shadeMap

      shl   eax, 8
      and   esi, NOT 3                    ; esi is now dstLastWordPlus1

      add   ecx, eax
      
      mov   eax, ecx
      mov   ebx, ecx
      
      mov   edx, esi
      
      sub   edx, edi
      
      cmp   edx, 3
      jle   @@lastPixels
      
      test  edi, 011b
      mov   edx, ecx

      jz    @@dwordLoop

@@firstPixelLoop:
      mov   al, [edi]
      inc   edi
      
      mov   al, [eax]
      
      test  edi, 03h
      mov   [edi - 1], al
      
      jnz   @@firstPixelLoop

@@dwordLoop:
      mov   al, [edi]               ; 2 cycles per pix., probably as good as it gets...
      mov   bl, [edi + 1]

      mov   cl, [edi + 2]
      mov   dl, [edi + 3]
      
      add   edi, 4
      mov   al, [eax]
      
      mov   bl, [ebx]
      mov   cl, [ecx]
      
      mov   dl, [edx]
      mov   [edi - 4], al
      
      mov   [edi - 3], bl
      mov   [edi - 2], cl
      
      mov   [edi - 1], dl
      cmp   edi, esi                ; dstLastWordPlus1
      
      jne   @@dwordLoop
      
@@lastPixels:
      cmp   edi, dstLastByte

      mov   esi, dstLastByte
      jg    @@skipLastPixels
      
@@lastPixelLoop:
      mov   al, [edi]
      inc   edi
      
      mov   al, [eax]
      
      cmp   edi, esi
      mov   [edi - 1], al
      
      jle   @@lastPixelLoop

@@skipLastPixels:
      jmp   DWORD PTR [gfxPDC.pdc_rendSpanExit]
GFXRenderSpanA ENDP

END


