;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;±±   
;±±  $Workfile:   MipBlt.asm  $
;±±    
;±±  DESCRIPTION:
;±±     Function to blit a bitmap to a composite bitmap with flips & rotations
;±±        
;±±  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
;±± 
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

.486
.MODEL FLAT, syscall

;------------------------------ Code-patching helper routine...

IFDEF __MASM__
 ARG equ
ELSE
 LOCALS
ENDIF

;------------------------------ Writable code segment markers...
.DATA

.CODE

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;±
;±  NAME
;±      MipBlt
;±      
;±  DESCRIPTION
;±      Transfers a MipMapped bitmap from a source bitmap to a sub-region
;±      on the destination bitmap.
;±      
;±      src_start   - Ptr to the source bitmap (with mipmaps)
;±      src_inc     - amount to increment source ptr for each pixel
;±      dst_start   - ptr to the destination bitmap bits
;±      dst_width   - number of dwords to move into destination
;±      src_width   - number of bytes to move from source
;±      src_adjust  - amount to add to source at end of each row
;±      dst_adjust  - amount to add to dest at end of each row
;±      
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

MipBlt  PROC C      \
        USES    eax ebx ecx edx esi edi,
        ARG     src_start, src_inc, dst_start, dst_width, src_width, src_adjust, dst_adjust
        PUBLIC  C MipBlt

        mov     esi,src_start           ; get src bits
        mov     edx,src_inc             ; get src byte increment
        mov     edi,dst_start           ; get dst bits
        mov     ebx,dst_width           ; # of dwords in dst width
        mov     ecx,src_width           ; get number of rows

        cmp     ebx,0                   ; are there dwords to move?
        jnz     @@dwordLoop             ; yes, then move dwords

; special case to move WORD sized mipmaps

        mov     al,[esi]
        add     esi,edx                 ; add offset to next src pixel
        mov     ah,[esi]
        add     esi,edx                 ; add offset to next src pixel
        mov     [edi],ax

        add     edi,2                   ; point to next word
        add     edi,dst_adjust          ; adjust edi to next row
        add     esi,src_adjust          ; adjust esi to next source start

        mov     al,[esi]
        add     esi,edx                 ; add offset to next src pixel
        mov     ah,[esi]
        add     esi,edx                 ; add offset to next src pixel
        mov     [edi],ax
        ret

; might have a special case when flag is plain or xflip to grab
; dwords from the source.

@@dwordLoop:
        mov     al,[esi]
        add     esi,edx                 ; add offset to next src pixel
        mov     ah,[esi]
        add     esi,edx                 ; add offset to next src pixel
        shl     eax,16
        mov     al,[esi]
        add     esi,edx                 ; add offset to next src pixel
        mov     ah,[esi]
        add     esi,edx                 ; add offset to next src pixel
        rol     eax,16
        mov     [edi],eax               ; store dword to destination
        
        add     edi,4                   ; point to next dword
        dec     ebx                     ; done with this row?
        jnz     @@dwordLoop             ; no, process next dword

        add     edi,dst_adjust          ; adjust edi to next row
        add     esi,src_adjust          ; adjust esi to next source start
        mov     ebx,dst_width
        dec     ecx                     ; decrement row count
        jnz     @@dwordLoop

        ret
MipBlt  ENDP
        END
