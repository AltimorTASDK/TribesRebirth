;================================================================
;   
; $Workfile:   g_bm.asm  $
; $Version$
; $Revision:   1.0  $
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
 

;================================================================
; NAME
;   gfxBitmapInvert 
;   
; DESCRIPTION
;   Convert a DIB from Top-Down to Bottom-UP and vice-versa
;   
; ARGUMENTS 
;   pTop        - pointer to (0,0) in the bitmap source
;   pBottom     - pointer to (0,n) in the bitmap source where
;                  'n' is the last scanline in the bitmap     
; RETURNS 
;   none
;   
; NOTES 
;   
;================================================================
gfxBitmapInvert PROC C \
        USES esi edi,
        ARG  pTop, pBottom, stride
        PUBLIC C gfxBitmapInvert

        mov     esi, pTop
        mov     edi, pBottom
                                
@@heightLoop:
        mov     ecx, stride     ;get stride DIV 4
        shr     ecx, 2          ;all DIB's are DWORD scan width guarenteed
@@scanLoop:
        mov     eax, [esi]      ;get a DWORD from the TOP
        mov     edx, [edi]      ;get a DWORD from the BOTTOM
        mov     [esi], edx      ;and switch them
        mov     [edi], eax
        add     esi, 4          ;advance pTop to right
        add     edi, 4          ;advance pBottom to right
        dec     ecx             ;is this scan done?
        jnz     @@scanLoop
                                ;pTop is at next scan
        sub     edi, stride     ;move pBottom up one scan
        sub     edi, stride
        cmp     esi, edi        ;while pTop < pBottom swap scans
        jl      @@heightLoop

        ret              
gfxBitmapInvert ENDP

END