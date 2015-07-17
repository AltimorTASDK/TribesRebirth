; ========================================================
;
;	$Workfile:   rn_fill.asm  $
;	$Version$
;
;    Raster Filler... whee!
;
;  $Revision:$
;       $Log:$
;
; (c) Copyright 1995, Dynamix Inc.  All rights reserved.
; ========================================================


.486
.MODEL FLAT, syscall

; =========================================================
;	DATA
; =========================================================
.DATA

;
; Handy globals.
;
row        dd ?     ; Address of current row on the surface.
stride     dd ?     ; Width of the surface, row step value.
raster     dd ?     ; Address of current raster list entry.
color      dd ?

gdfgrt_marked_writable  dd      0       ; have we marked this function patchable?
gf_stepShade    dd      ?               ; constant shade step for this raster list...
_ebp       dd ?
_esp       dd ?
shademap   dd ?
stepZ      dd ?
dstLast    dd ?
dstLastWord    dd ?
haze_shift db ?

; =========================================================
;	CODE
; =========================================================
.CODE

INCLUDE g_raster.inc
INCLUDE g_macros.inc
INCLUDE g_contxt.inc

IFDEF __MASM__
ARG equ
ENDIF


;================================================================
; NAME
;   gfx_draw_translucent_raster PROC C \
;
; DESCRIPTION
;             Draw a plain-color filled raster.
;             Uses:  x, cnt
;
; ARGUMENTS
;       in_surface:DWORD     Surface to draw to (BYTE PTR)
;       in_width:DWORD       Width of surface
;       in_rast:DWORD        Pointer to raster list (RASTER PTR)
;       in_color:DWORD       Color -- only low byte used
;
;
; RETURNS
; NOTES
;
;================================================================


gfx_draw_translucent_raster PROC C \
        USES    ebx esi edi,
        ARG     in_surface:DWORD,    \
                in_width:DWORD,      \
                in_rast:DWORD
        PUBLIC C gfx_draw_translucent_raster
        start_timer
        mov     esi, in_rast                    ;RasterList

        mov     eax, in_width                   ;Surface width
        mov     ecx, (RASTER_LIST PTR [esi]).top_y
        mov     stride, eax
        mul     ecx                             ;Width * Top
        mov     ebx, in_surface                 ;Surface[0]
        add     ebx, eax                        ;Surface[top], starting row address
        mov     row, ebx

        mov     eax, 0

        mov     ah, BYTE PTR (RASTER_LIST PTR [esi]).rl_color  ;fill eax with color
        mov     _ebp, ebp
        mov     ebp, (RASTER_LIST PTR [esi]).rl_transmap
        mov     esi, (RASTER_LIST PTR [esi]).topRaster

        ; ---------------------------------------------------------
        ; Currently set:
        ;           eax   Color
        ;           edi   destination pointer
        ;           esi   Raster List Address

@@row:
        mov     ebx, row
        test    (RASTER PTR [esi]).f, RASTER_STEP_Y
        jz      @@skip_y                     ;don't move to next surface row
        add     ebx, stride                     ;Step Row address
        mov     row, ebx

@@skip_y:
        test    (RASTER PTR [esi]).f, RASTER_DEAD   ;don't process fill, check surface row inc
        jnz     @@next_row

        mov     edx, (RASTER PTR [esi]).cnt     ;Get width of span (0-n)  where 0 = width 1
        mov     ecx, (RASTER PTR [esi]).x       ;Get X position
        inc     edx
        lea     edi, [ebx + ecx]                ;destination = Row + X

@@main_loop:
        test    edi, 3
        jz      @@dword_check
@@byte_write:
        mov     al, [edi]                       ; get surface pixel
        mov     bl, [ebp+eax]                   ; get translucent color
        mov     [edi], bl
        inc     edi
        dec     edx
        jnz     @@main_loop                     ; do another byte
        jmp     @@next_row                      ; done with this row
@@dword_check:
        test    edx, NOT 3               ;are there any DWORDS to write?
        jz      @@byte_write            ;no, just bytes left
        sub     edx,4                   ;yes, reduce column count by four

@@dword_loop:
        mov     ebx,[edi]               ;get surface pixels
        mov     al,bl                   ;get first pixel
        mov     bl,[ebp+eax]            ;get translucent color
        mov     al,bh
        mov     bh,[ebp+eax]
        bswap   ebx
        mov     al,bl                   ;get first pixel
        mov     bl,[ebp+eax]            ;get translucent color
        mov     al,bh
        mov     bh,[ebp+eax]
        bswap   ebx
        mov     [edi],ebx               ;store DWORD
        add     edi,4                   ;move to next four pixels
        sub     edx,4
        jns     @@dword_loop
        add     edx,4                   ;get number of pixels left over
        jnz     @@byte_write            ;if any left, process as bytes


@@next_row:
        mov     esi, (RASTER PTR [esi]).next    ;Link to next raster address
        test    esi, esi                        ;Null address?
        jnz     @@row                        ;Loop
        mov     ebp, _ebp
        stop_timer
        RET
gfx_draw_translucent_raster ENDP



;================================================================
; NAME
;   gfx_draw_fill_raster PROC C \
;
; DESCRIPTION
;             Draw a plain-color filled raster.
;             Uses:  x, cnt
;
; ARGUMENTS
;       in_surface:DWORD     Surface to draw to (BYTE PTR)
;       in_width:DWORD       Width of surface
;       in_rast:DWORD        Pointer to raster list (RASTER PTR)
;       in_color:DWORD       Color -- only low byte used
;
;
; RETURNS
; NOTES
;
;================================================================

gfx_draw_fill_raster PROC C \
        USES    ebx esi edi,
        ARG     in_surface:DWORD,    \
                in_width:DWORD,      \
                in_rast:DWORD
        PUBLIC C gfx_draw_fill_raster
        start_timer
        mov     esi, in_rast                    ;Raster[0]
        mov     eax, in_width                   ;Surface width
        mov     stride, eax
        mov     ecx, (RASTER_LIST PTR [esi]).top_y
        mul     ecx                             ;Width * Top
        mov     ebx, in_surface                 ;Surface[0]
        add     ebx, eax                        ;Surface[top], starting row address
        mov     row, ebx

		  mov     eax, (RASTER_LIST PTR [esi]).rl_color
        mov     ecx, (RASTER_LIST PTR [esi]).rl_hazeshift
        mov     edi, (RASTER_LIST PTR [esi]).rl_shademap
		  mov		esi, (RASTER_LIST PTR [esi]).topRaster
        test    ecx, ecx
        jnz     @@do_hazed

        mov     ah,al
        mov     ecx,eax
        bswap   eax
        or      eax,ecx                         ;fill each byte of eax with color

        ; ---------------------------------------------------------
        ; Currently set:
        ;           eax   Color
        ;           edi   destination pointer
        ;           esi   Raster List Address

@@ddfr_row:
        mov     ebx, row
        test    (RASTER PTR [esi]).f, RASTER_STEP_Y
        jz      @@ddfr_skip_y                     ;don't move to next surface row
        add     ebx, stride                     ;Step Row address
        mov     row, ebx

@@ddfr_skip_y:
        test    (RASTER PTR [esi]).f, RASTER_DEAD   ;don't process fill, check surface row inc
        jnz     @@ddfr_next_row

        mov     edx, (RASTER PTR [esi]).cnt     ;Get width of span (0-n)  where 0 = width 1
        mov     ecx, (RASTER PTR [esi]).x       ;Get X position
        inc     edx
        lea     edi, [ebx + ecx]                ;destination = Row + X
        cmp     edx, 3
        jle     @@ddfr_tail

        mov     ecx, edi                        ;ASSERT:
        neg     ecx
        and     ecx, 3                          ;  ecx = 0-3 head BYTES

        sub     edx, ecx                        ;  ebx = 0-n center BYTES
        mov     ebx, edx                        ;  edx = 0-3 tail BYTES
        and     edx, 3
        sub     ebx, edx

        ; ---------------------------------------------------------
        ; Currently set:
        ;           ebx   0-3 head BYTES
        ;           ecx   0-n center BYTES
        ;           edx   0-3 tail BYTES
        ;           edi   destination pointer
        ;           esi   Raster List Address

        rep stosb                               ;write 0-3 head BYTES
        mov     ecx, ebx
        shr     ecx, 2                          ;convert BYTES to DWORDS
        rep stosd                               ;write 0-n DWORDS
@@ddfr_tail:
        mov     ecx, edx
        rep stosb                               ;write 0-3 tail BYTES

@@ddfr_next_row:
        mov     esi, (RASTER PTR [esi]).next    ;Link to next raster address
        test    esi, esi                        ;Null address?
        jnz     @@ddfr_row                        ;Loop
        stop_timer
        RET
@@do_hazed:
        ; color in eax, haze_shift in ecx, shademap in edi
        mov     _ebp, ebp
        mov     _esp, esp
        mov     haze_shift, cl
        mov     shademap, edi
        mov     color, eax
@@ddfrh_row:
        mov     ebx, row
        test    (RASTER PTR [esi]).f, RASTER_STEP_Y
        jz      @@ddfrh_skip_y                     ;don't move to next surface row
        add     ebx, stride                     ;Step Row address
        mov     row, ebx

@@ddfrh_skip_y:
        test    (RASTER PTR [esi]).f, RASTER_DEAD   ;don't process fill, check surface row inc
        jnz     @@ddfrh_next_row

        mov     cl, haze_shift
        mov     edx, (RASTER PTR [esi]).z
        mov     esp, (RASTER PTR [esi]).sz
        mov     ebp, (RASTER PTR [esi]).cnt
        mov     raster, esi
        mov     esi, (RASTER PTR [esi]).x

        lea     edi, [ebx + esi]

        ; Currently set:
        ; ebx - row
        ; ecx - haze_shift in cl
        ; edx - haze
        ; edi - dest ptr
        ; esi - x
        ; ebp - count - 1
        ; esp - step haze

        add     ebp, edi ; ebp = destLast
        mov     dstLast, ebp
        sub     ebp, 4
        and     ebp, NOT 3
        mov     dstLastWord, ebp

        cmp     edi, ebp
        mov     ebp, shademap
        add     ebp, color
        mov     eax, 0FFFFFFFFh
        jg      @@haze_tail
        test    edi, 3
        jz      @@dword_writes
@@first_bytes:
        mov     esi, edx
        shr     esi, 16
        shl     esi, cl
        add     esi, ebp
        inc     edi
        add     edx, esp

        mov     al, [esi]
        mov     [edi-1], al
        test    edi, 3
        jnz     @@first_bytes
        shl     esp, 1
@@dword_writes:
        mov     esi, edx
        shr     esi, 16
        shl     esi, cl
        add     esi, ebp
        add     edi, 4
        add     edx, esp

        mov     al, [esi]
        mov     ah, al

        mov     esi, edx
        shr     esi, 16
        shl     esi, cl
        add     esi, ebp
        add     edx, esp
        bswap   eax

        mov     al, [esi]
        mov     ah, al
        bswap   eax
        cmp     edi, dstLastWord
        mov     [edi-4], eax
        jle     @@dword_writes
        cmp     edi, dstLast
        jg      @@haze_end
        shr     esp, 1
@@haze_tail:
        mov     esi, edx
        shr     esi, 16
        shl     esi, cl
        add     esi, ebp
        add     edx, esp
        inc     edi

        mov     ah, [esi]
        cmp     edi, dstLast
        mov     [edi-1], ah

        jle     @@haze_tail
@@haze_end:
        mov     esi, raster

@@ddfrh_next_row:
        mov     esi, (RASTER PTR [esi]).next    ;Link to next raster address
        test    esi, esi                        ;Null address?
        jnz     @@ddfrh_row                     ;Loop

        mov     ebp, _ebp
        mov     esp, _esp
        stop_timer
        RET
gfx_draw_fill_raster ENDP


;================================================================
; NAME
;   gfx_draw_fill_gour_raster PROC C \
;
; DESCRIPTION
;             Draw a filled, gouraud-shaded.
;             Uses:  x, s, s_s, cnt
;
; ARGUMENTS
;       in_surface:DWORD     Surface to draw to (BYTE PTR)
;       in_width:DWORD       Width of surface
;       in_rast:DWORD        Pointer to raster list (RASTER PTR)
;       in_top:DWORD         Top index
;       in_color:DWORD       Color -- only low byte used
;       in_shademap:DWORD    Shademap, 256 colors per shade.
;
;
; RETURNS
; NOTES
;
;================================================================
gfx_draw_fill_gour_raster PROC C \
        USES    ebx esi edi,
        ARG     in_surface:DWORD,    \
                in_width:DWORD,      \
                in_rast:DWORD
        PUBLIC C gfx_draw_fill_gour_raster

        ; ---------------------------------------------------------
        ; Setup for Outer Loop:
        ;              [row]       Address of row[idx]
        ;              [stride]     Width of the surface, row step value
        ;              [raster]    Address of raster[idx]
        ;
        ;              esi         Address of shademap.
        ;
        start_timer
        mov     eax, in_rast         ; Raster[0]
        mov     ecx, (RASTER_LIST PTR [eax]).top_y
        mov     esi, (RASTER_LIST PTR [eax]).rl_shademap ; shadmap ->esi
        mov     ebx, (RASTER_LIST PTR [eax]).rl_color
        mov     color, ebx

        mov     eax, (RASTER_LIST PTR [eax]).topRaster
        mov     raster, eax          ; => raster

        mov     eax, in_width        ; Surface width
        mov     stride, eax          ; => width

        mul     ecx                  ; width * top
        add     eax, in_surface      ; Surface[top]
        mov     row, eax             ; => row


        ; ---------------------------------------------------------
        ; Outer Loop -- Loop from Top to Bottom raster
        ;
        mov     eax, raster

        ; ---------------------------------------------------------
        ; Currently set:
        ;              [row]       Address of row[idx]
        ;              [width]     Width of the surface, row step value
        ;              [raster]    Address of raster[idx]
        ;              esi         Address of shademap
        ;
        ; Setup for Inner Loop:
        ;              eax         ... working register
        ;              edi         Point address = row + raster.x
        ;              ebx         raster.s -- shade value
        ;              edx         raster.s_s -- step in s
        ;              ecx         pixel counter == raster.cnt
        ;
        ; eax == [raster]
        ;

@@row:
        mov     ebx, row                ; get row address

        test    (RASTER PTR [eax]).f, RASTER_STEP_Y
        jz      @@skip_y

        add     ebx, stride             ; step to next row on the surface
        mov     row, ebx                ; store row address

@@skip_y:
        test    (RASTER PTR [eax]).f, RASTER_DEAD
        jnz     @@next_row

        mov     edi, (RASTER PTR [eax]).x       ; raster.x
        add     edi, ebx                        ; raster.x + row

        mov     ebx, (RASTER PTR [eax]).rs       ; raster.s
        mov     edx, (RASTER PTR [eax]).srs     ; raster.s_s

        rol     ebx, 16                         ; -> low.high
        rol     edx, 16                         ; -> low.high

        mov     ecx, (RASTER PTR [eax]).cnt     ; raster.cnt => ecx
        add     ecx, edi                        ; ecx = dstLast
        mov     dstLast, ecx
        sub     ecx, 4
        and     ecx, NOT 3
        mov     dstLastWord, ecx
        cmp     edi, ecx
        mov     eax, color
        jg      @@tail
        test    edi, 3
        jz      @@dword_writes

@@first_bytes:

        inc     edi                     ; step to next point on surface
        mov     ah, bl                  ; shade into high-byte
        add     ebx, edx                ; step s
        adc     ebx, 0                  ; (wrap carry flag)
        test    edi, 3                  ; see if we're done with the first bytes
        mov     cl, [eax + esi]         ; shademap[ shade, color ]
        mov     [edi-1], cl             ; draw to the surface
        jnz     @@first_bytes
@@dword_writes:
        mov     ah, bl
        add     ebx, edx                ; step s
        adc     ebx, 0                  ; (wrap carry flag)
        mov     cl, [eax + esi]         ; shademap[ shade, color ]
        mov     ah, bl
        add     ebx, edx                ; step s
        adc     ebx, 0                  ; (wrap carry flag)
        mov     ch, [eax + esi]         ; shademap[ shade, color ]
        bswap   ecx
        add     edi, 4
        mov     ah, bl
        add     ebx, edx                ; step s
        adc     ebx, 0                  ; (wrap carry flag)
        mov     ch, [eax + esi]         ; shademap[ shade, color ]
        mov     ah, bl
        add     ebx, edx                ; step s
        adc     ebx, 0                  ; (wrap carry flag)
        mov     cl, [eax + esi]         ; shademap[ shade, color ]
        bswap   ecx
        cmp     edi, dstLastWord
        mov     [edi-4], ecx
        jle     @@dword_writes
        cmp     edi, dstLast
        jg      @@next_row
@@tail:
        inc     edi
        mov     ah, bl
        add     ebx, edx                ; step s
        adc     ebx, 0                  ; (wrap carry flag)
        cmp     edi, dstLast
        mov     cl, [eax + esi]         ; shademap[ shade, color ]
        mov     [edi-1], cl
        jle     @@tail

@@next_row:
        mov     eax, raster             ; get raster address
        mov     eax, (RASTER PTR [eax]).next  ; Jump to next raster address
        mov     raster, eax             ; store raster address

        test    eax, eax                ; Null address?
        jnz     @@row               ; loop

        stop_timer
        RET

gfx_draw_fill_gour_raster ENDP


;================================================================
; NAME
;   gfx_draw_fill_gour_raster_tri PROC C \
;
; DESCRIPTION
;             Draw a filled, gouraud-shaded, with triangle optimizations
;             Uses:  x, s, s_s, cnt
;
; ARGUMENTS
;       in_surface:DWORD     Surface to draw to (BYTE PTR)
;       in_width:DWORD       Width of surface
;       in_rastList:DWORD        Pointer to raster list (RASTER_LIST PTR)
;       in_color:DWORD       Color -- only low byte used
;       in_shademap:DWORD    Shademap, 256 colors per shade.
;
;
; RETURNS
; NOTES
;
;================================================================
gfx_draw_fill_gour_raster_tri PROC C \
        USES    ebx esi edi,
        ARG     in_surface:DWORD,    \
                in_width:DWORD,      \
                in_rastList:DWORD
        PUBLIC C gfx_draw_fill_gour_raster_tri

        start_timer
        mark_writable gdfgrt_marked_writable

@@start_proc:
        ; ---------------------------------------------------------
        ; Setup for Outer Loop:
        ;              [row]       Address of row[idx]
        ;              [stride]     Width of the surface, row step value
        ;              [raster]    Address of raster[idx]
        ;
        ;              esi         Address of shademap.
        ;

        mov     eax, in_rastList         ; Raster list...

        mov     ebx, (RASTER_LIST PTR [eax]).rl_stepRS      ; Get the constant shade step
        mov     gf_stepShade, ebx

        mov     ecx, (RASTER_LIST PTR [eax]).top_y
        mov     ebx, (RASTER_LIST PTR [eax]).rl_color
        mov     color, ebx
        mov     esi, (RASTER_LIST PTR [eax]).rl_shademap ; Shademap => esi
        mov     eax, (RASTER_LIST PTR [eax]).topRaster
        mov     raster, eax          ; => raster

        mov     eax, in_width        ; Surface width
        mov     stride, eax          ; => width

        mul     ecx                  ; width * top
        add     eax, in_surface      ; Surface[top]
        mov     row, eax             ; => row

        ; ---------------------------------------------------------
        ; Outer Loop -- Loop from Top to Bottom raster
        ;
        mov     eax, raster

        ; ---------------------------------------------------------
        ; Currently set:
        ;              [row]       Address of row[idx]
        ;              [width]     Width of the surface, row step value
        ;              [raster]    Address of raster[idx]
        ;              esi         Address of shademap
        ;
        ; Setup for Inner Loop:
        ;              eax         ... working register
        ;              edi         Point address = row + raster.x
        ;              ebx         raster.s -- shade value
        ;              edx         raster.s_s -- step in s
        ;              ecx         pixel counter == raster.cnt
        ;
        ; eax == [raster]
        ;

@@row:
        mov     ebx, row                ; get row address

        test    (RASTER PTR [eax]).f, RASTER_STEP_Y
        jz      @@skip_y

        add     ebx, stride             ; step to next row on the surface
        mov     row, ebx                ; store row address

@@skip_y:
        test    (RASTER PTR [eax]).f, RASTER_DEAD
        jnz     @@next_row

        mov     edi, (RASTER PTR [eax]).x       ; raster.x
        add     edi, ebx                        ; raster.x + row

        mov     ebx, (RASTER PTR [eax]).rs       ; raster.s
        mov     edx, gf_stepShade

        rol     ebx, 16                         ; -> low.high
        rol     edx, 16                         ; -> low.high

        mov     ecx, (RASTER PTR [eax]).cnt     ; raster.cnt => ecx

  ; ---------------------------------------------------------
  ; Inner Loop -- Loop from Left to Right, drawing as we go.
  ;
@@pnt:
        mov     eax, color
        mov     ah, bl                  ; shade into high-byte
        mov     al, [eax + esi]         ; shademap[ shade, color ]
        mov     [edi], al               ; draw to the surface
        inc     edi                     ; step to next point on surface

        add     ebx, edx                ; step s
        adc     ebx, 0                  ; (wrap carry flag)

        dec     ecx                     ; decrement pixel counter
        jns     @@pnt               ; loop!

@@next_row:
        mov     eax, raster             ; get raster address
        mov     eax, (RASTER PTR [eax]).next  ; Jump to next raster address
        mov     raster, eax             ; store raster address

        test    eax, eax                ; Null address?
        jnz     @@row               ; loop
        stop_timer
        RET

@@end_proc:
gfx_draw_fill_gour_raster_tri ENDP

END
