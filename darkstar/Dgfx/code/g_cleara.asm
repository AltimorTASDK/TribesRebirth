;================================================================
;   
; $Workfile:   g_cleara.asm  $
; $Version$
; $Revision:   1.3  $
;   
; DESCRIPTION:
;   
; (c) Copyright Dynamix Inc.   All rights reserved.
;================================================================

.486p
.MODEL FLAT, SYSCALL
 
IFDEF __MASM__
ARG equ
ELSE
LOCALS
ENDIF
 
 ;------------------------------ DATA SEGMENT
.DATA
 
 ;------------------------------ CODE SEGMENT
.CODE
 
_gfxClearScreen PROC C \
        USES ebx edi esi,
        ARG in_pDst, \
            in_rows, \
            in_width,\
            in_dstNextScan, \
            in_color
        PUBLIC C _gfxClearScreen

        mov     edi, in_pDst            ;load destination pointer
        mov     eax, in_color           ;load color
        mov     ah, al                  ;fill all 4 bytes in EAX with color
        mov     edx, eax
        shl     eax, 16
        mov     ax, dx
        mov     esi, in_rows            ;load the number of rows

@@heightLoop:
        mov     edx, in_width           ;load the width of the rows
        cmp     edx, 3
        jle     @@tail

        mov     ecx, edi                        ;ASSERT:
        neg     ecx
        and     ecx, 3                          ;  ecx = 0-3 head BYTES

        sub     edx, ecx                        ;  ebx = 0-n center BYTES
        mov     ebx, edx                        ;  edx = 0-3 tail BYTES
        and     edx, 3
        sub     ebx, edx

        ; ---------------------------------------------------------  
        ; Currently set:
        ;           eax   color    
        ;           ebx   0-3 head BYTES
        ;           ecx   0-n center BYTES
        ;           edx   0-3 tail BYTES
        ;           edi   destination pointer
        ;           esi   height

        rep stosb                               ;write 0-3 head BYTES
        mov     ecx, ebx
        shr     ecx, 2                          ;convert BYTES to DWORDS
        rep stosd                               ;write 0-n DWORDS
@@tail:
        mov     ecx, edx
        rep stosb                               ;write 0-3 tail BYTES

@@nextScan:
        add     edi,in_dstNextScan      ;wrap to next dest offset
        dec     esi                     ;decrement row count
        jnz     @@heightLoop            ;process next row

        RET

_gfxClearScreen ENDP

END
