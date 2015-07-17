;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;±   
;± $Workfile:   b_blit.asm  $
;± $Version: 1.0.a1 $
;± $Revision:   1.0  $
;±   
;± DESCRIPTION: Rectangle clipped bitmap routines 
;±   
;± (c) Copyright 1995, Dynamix Inc.   All rights reserved.
;±
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

.486
.MODEL FLAT, syscall

IFDEF __MASM__
ARG equ
markFunctionWritable    PROTO C, functionStart:DWORD, functionLength:DWORD
ELSE
LOCALS
extrn C markFunctionWritable:PROC
invoke  equ     call
ENDIF

GFX_FLIP_NONE   EQU     0
GFX_FLIP_X      EQU     1
GFX_FLIP_Y      EQU     2

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
.DATA

_ebp                 dd      ?
lastDWord            dd      ?
lastDWordLine        dd      ?
dstStride            dd      ?

; NOTE: the next two lines must stay in this order
XarrCC  dd      ?                       ;increment if Carry
XarrNC  dd      ?                       ;increment if no Carry
YarrCC  dd      ?                       ;increment if Carry
YarrNC  dd      ?                       ;increment if no Carry

gdstbm_marked_writeable dd              0               ;translucent render bitmap marked writeable
gdtbm_marked_writeable  dd              0               ;translucent draw bitmap marked writeable

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
.CODE

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;±
;±  NAME
;±      gfx_draw_bm
;±      
;±  DESCRIPTION
;±      Copies a block of memory from a sub-region of a source bitmap to
;±      a sub-region of same size in a destination bitmap.
;±      
;±  ARGUMENTS
;±      in_dst_offset   - starting address of destination block
;±      in_src_offset   - starting address of source block
;±      in_srcW         - width, in bytes, of source block
;±      in_srcH         - number of rows in source block
;±      in_src_scan     - offset from end of scan to start of next
;±      in_dst_scan     - offset from end of scan to start of next
;±      in_flip         - GFX flip flag
;±      
;±  RETURNS
;±      nothing
;±      
;±  NOTES
;±      does DWORD writes that are DWORD aligned.
;±      
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

gfx_draw_bm PROC C \
        USES    ebx ecx edx esi edi,
        ARG     in_dst_offset,in_src_offset,in_srcW,in_srcH,in_src_scan,in_dst_scan,in_flip
        PUBLIC  C gfx_draw_bm

        mov     esi,in_src_offset       ;get source offset
        mov     edi,in_dst_offset       ;get destination offset

        mov     ecx,in_srcW             ;get width of bitmap
        mov     edx,in_srcH
        test    in_flip,GFX_FLIP_X      ;flip left-to-right?
        jnz     @@main_loopX            ;yes

@@main_loop:
        test    edi,3                   ;are we on a DWORD boundary?
        jz      @@dword_check           ;=0 Yes
@@byte_write:
        mov     al,[esi]                ;get a byte
        inc     esi                     ;next source pixel
        mov     [edi], al               ;store the byte
        inc     edi                     ;next destination pixel
        dec     ecx                     ;decrement remaining count
        jnz     @@main_loop             ;no, do another byte
        jmp     short @@done_row        ;finished, next row
@@dword_check:
        test    ecx,NOT 3               ;yes, but are there any DWORDS to write?
        jz      @@byte_write            ;no, finish with bytes
        sub     ecx,4                   ;yes, decrement count by one DWORD
                
@@dword_loop:
		xor		eax, eax				; Read the 4 source pixels
		mov		bl, BYTE PTR [esi+2]
		mov		al, BYTE PTR [esi]
		mov		bh, BYTE PTR [esi+3]
		mov		ah, BYTE PTR [esi+1]
		shl		ebx, 16
		or		eax, ebx
		add		esi, 4
        mov     [edi],eax               ;store to destination pixels
        add     edi,4                   ;increment dest pointer
        sub     ecx,4                   ;decrement column count four bytes
        jns     @@dword_loop            ;next four pixels
        add     ecx,4                   ;get number of pixels left over
        jnz     @@byte_write            ;if any left, process bytes

@@done_row:
        add     esi,in_src_scan         ;wrap to next source offset
        add     edi,in_dst_scan         ;wrap to next dest offset
        mov     ecx,in_srcW             ;get width of bitmap
        dec     edx                     ;decrement row count
        jnz     @@main_loop             ;process next row
        jmp     short @@exit              ;done, exit

;------ flipped left-to-right ----------

@@main_loopX:
        test    edi,3                   ;are we on a DWORD boundary?
        jz      @@dword_checkX          ;=0 Yes
@@byte_writeX:
        mov     al,[esi]                ;get a byte
        dec     esi                     ;next source pixel
        mov     [edi], al               ;store the byte
        inc     edi                     ;next destination pixel
        dec     ecx                     ;decrement remaining count
        jnz     @@main_loopX            ;no, do another byte
        jmp     short @@done_rowX       ;finished, next row
@@dword_checkX:
        test    ecx,NOT 3               ;yes, but are there any DWORDS to write?
        jz      @@byte_writeX           ;no, finish with bytes
        sub     ecx,4                   ;yes, decrement count by one DWORD
        sub     esi,3
                
@@dword_loopX:
		xor		eax, eax				; Read the 4 source pixels
		mov		bl, BYTE PTR [esi+1]
		mov		al, BYTE PTR [esi+3]
		mov		bh, BYTE PTR [esi]
		mov		ah, BYTE PTR [esi+2]
		shl		ebx, 16
		or		eax, ebx
        sub     esi,4                   ;increment source pointer
        mov     [edi],eax               ;store to destination pixels
        add     edi,4                   ;increment dest pointer
        sub     ecx,4                   ;decrement column count four bytes
        jns     @@dword_loopX           ;next four pixels
        add     esi,3
        add     ecx,4                   ;get number of pixels left over
        jnz     @@byte_writeX           ;if any left, process bytes

@@done_rowX:
        add     esi,in_src_scan         ;wrap to next source offset
        add     edi,in_dst_scan         ;wrap to next dest offset
        mov     ecx,in_srcW             ;get width of bitmap
        dec     edx                     ;decrement row count
        jnz     @@main_loopX            ;process next row
@@exit:
        ret

gfx_draw_bm ENDP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;±
;±  NAME
;±      gfx_draw_bm
;±      
;±  DESCRIPTION
;±      Copies a block of memory from a sub-region of a source bitmap to
;±      a sub-region of same size in a destination bitmap.
;±      
;±  ARGUMENTS
;±      in_dst_offset   - starting address of destination block
;±      in_src_offset   - starting address of source block
;±      in_srcW         - width, in bytes, of source block
;±      in_srcH         - number of rows in source block
;±      in_src_scan     - offset from end of scan to start of next
;±      in_dst_scan     - offset from end of scan to start of next
;±      in_flip         - GFX flip flag
;±      
;±  RETURNS
;±      nothing
;±      
;±  NOTES
;±      does DWORD writes that are DWORD aligned.
;±      
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

gfx_draw_bm_s PROC C \
        USES    ebx ecx edx esi edi,
        ARG     in_dst_offset,in_src_offset,in_srcW,in_srcH,in_src_scan,in_dst_scan,in_flip,in_shadeTable
        PUBLIC  C gfx_draw_bm_s

        mov     esi,in_src_offset       ;get source offset
        mov     edi,in_dst_offset       ;get destination offset

        mov     ecx,in_srcW             ;get width of bitmap
        mov     edx,in_srcH
		mov  	eax, in_shadeTable
        test    in_flip,GFX_FLIP_X      ;flip left-to-right?
        jnz     @@main_loopX            ;yes

@@main_loop:
@@byte_write:
        mov     al,[esi]                ;get a byte
        inc     esi                     ;next source pixel
		mov 	al, [eax]				; look up
        mov     [edi], al               ;store the byte
        inc     edi                     ;next destination pixel
        dec     ecx                     ;decrement remaining count
        jnz     @@byte_write            ;no, do another byte
        jmp     short @@done_row        ;finished, next row

@@done_row:
        add     esi,in_src_scan         ;wrap to next source offset
        add     edi,in_dst_scan         ;wrap to next dest offset
        mov     ecx,in_srcW             ;get width of bitmap
        dec     edx                     ;decrement row count
        jnz     @@main_loop             ;process next row
        jmp     short @@exit              ;done, exit

;------ flipped left-to-right ----------

@@main_loopX:
@@byte_writeX:
        mov     al,[esi]                ;get a byte
        dec     esi                     ;next source pixel
		mov		al, [eax]				; lookup
        mov     [edi], al               ;store the byte
        inc     edi                     ;next destination pixel
        dec     ecx                     ;decrement remaining count
        jnz     @@byte_writeX           ;no, do another byte
        jmp     short @@done_rowX       ;finished, next row
@@done_rowX:
        add     esi,in_src_scan         ;wrap to next source offset
        add     edi,in_dst_scan         ;wrap to next dest offset
        mov     ecx,in_srcW             ;get width of bitmap
        dec     edx                     ;decrement row count
        jnz     @@main_loopX            ;process next row
@@exit:
        ret

gfx_draw_bm_s ENDP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;±
;±  NAME
;±      gfx_draw_transparent_bm
;±      
;±  DESCRIPTION
;±      Copies a block of memory from a sub-region of a source bitmap to
;±      a sub-region of same size in a destination bitmap.
;±      Color 0 is skipped.
;±      
;±  ARGUMENTS
;±      in_dst_offset   - starting address of destination block
;±      in_src_offset   - starting address of source block
;±      in_srcW         - width, in bytes, of source block
;±      in_srcH         - number of rows in source block
;±      in_src_scan     - offset from end of scan to start of next
;±      in_dst_scan     - offset from end of scan to start of next
;±      in_flip         - GFX flip flag
;±      
;±  RETURNS
;±      nothing
;±      
;±  NOTES
;±      does DWORD writes that are DWORD aligned.
;±      
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

gfx_draw_transparent_bm PROC C \
        USES    ebx ecx edx esi edi,
        ARG     in_dst_offset,in_src_offset,in_srcW,in_srcH,in_src_scan,in_dst_scan,in_flip
        PUBLIC  C gfx_draw_transparent_bm

        mov     esi,in_src_offset       ;get source offset
        mov     edi,in_dst_offset       ;get destination offset

        mov     ecx,in_srcW             ;get width of bitmap
        mov     edx,1                   ;assume no flip_X
        test    in_flip,GFX_FLIP_X      ;flip left-to-right?
        jz      @@main_loop             ;no
        mov     edx,-1                  ;yes, flip source

@@main_loop:
        test    edi,3                   ;are we on a DWORD boundary?
        jz      @@dword_check           ;=0 Yes
@@byte_write:
        mov     al,[esi]                ;get a byte
        add     esi,edx                 ;next source pixel
        cmp     al,0                    ;skip color?
        je      @@byte_skip             ;yes
        mov     [edi], al               ;store the byte
@@byte_skip:
        inc     edi                     ;next destination pixel
        dec     ecx                     ;decrement remaining count
        jnz     @@main_loop             ;no, do another byte
        jmp     short @@done_row        ;finished, next row
@@dword_check:
        test    ecx,NOT 3               ;yes, but are there any DWORDS to write?
        jz      @@byte_write            ;no, finish with bytes
        sub     ecx,4                   ;yes, decrement count by one DWORD
                
@@dword_loop:
        mov     ebx,[edi]               ;get current dest pixels
        mov     al,[esi]                ;get source pixel
        add     esi,edx                 ;move to next source pixel
        cmp     al,0                    ;transparent color?
        jne     @@byte1_skip            ;no, use color from source
        mov     al,bl                   ;yes, use color from dest
@@byte1_skip:
        mov     ah,[esi]                ;get source pixel
        add     esi,edx                 ;move to next source pixel
        cmp     ah,0                    ;transparent color?
        jne     @@byte2_skip            ;no, use color from source
        mov     ah,bh                   ;yes, use color from dest
@@byte2_skip:
        bswap   eax
        bswap   ebx
        mov     ah,[esi]                ;get source pixel
        add     esi,edx                 ;move to next source pixel
        cmp     ah,0                    ;transparent color?
        jne     @@byte3_skip            ;no, use color from source
        mov     ah,bh                   ;yes, use color from dest
@@byte3_skip:
        mov     al,[esi]                ;get source pixel
        add     esi,edx                 ;move to next source pixel
        cmp     al,0                    ;transparent color?
        jne     @@byte4_skip            ;no, use color from source
        mov     al,bl                   ;yes, use color from dest
@@byte4_skip:
        bswap   eax

        mov     [edi],eax               ;store dword

        add     edi,4                   ;increment dest pointer
        sub     ecx,4                   ;decrement column count four bytes
        jns     @@dword_loop            ;next four pixels
        add     ecx,4                   ;get number of pixels left over
        jnz     @@byte_write            ;if any left, process bytes

@@done_row:
        add     esi,in_src_scan         ;wrap to next source offset
        add     edi,in_dst_scan         ;wrap to next dest offset
        mov     ecx,in_srcW             ;get width of bitmap
        dec     in_srcH                 ;decrement row count
        jnz     @@main_loop             ;process next row

        ret

gfx_draw_transparent_bm ENDP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;±
;±  NAME
;±      gfx_draw_transparent_bm
;±      
;±  DESCRIPTION
;±      Copies a block of memory from a sub-region of a source bitmap to
;±      a sub-region of same size in a destination bitmap.
;±      Color 0 is skipped.
;±      
;±  ARGUMENTS
;±      in_dst_offset   - starting address of destination block
;±      in_src_offset   - starting address of source block
;±      in_srcW         - width, in bytes, of source block
;±      in_srcH         - number of rows in source block
;±      in_src_scan     - offset from end of scan to start of next
;±      in_dst_scan     - offset from end of scan to start of next
;±      in_flip         - GFX flip flag
;±      
;±  RETURNS
;±      nothing
;±      
;±  NOTES
;±      does DWORD writes that are DWORD aligned.
;±      
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

gfx_draw_transparent_bm_s PROC C \
        USES    ebx ecx edx esi edi,
        ARG     in_dst_offset,in_src_offset,in_srcW,in_srcH,in_src_scan,in_dst_scan,in_flip,shadeTable
        PUBLIC  C gfx_draw_transparent_bm_s

        mov     esi,in_src_offset       ;get source offset
        mov     edi,in_dst_offset       ;get destination offset

        mov     ecx,in_srcW             ;get width of bitmap
        mov     edx,1                   ;assume no flip_X
		mov 	eax, shadeTable
        test    in_flip,GFX_FLIP_X      ;flip left-to-right?
        jz      @@main_loop             ;no
        mov     edx,-1                  ;yes, flip source

@@main_loop:
@@byte_write:
        mov     al,[esi]                ;get a byte
        add     esi,edx                 ;next source pixel
        cmp     al,0                    ;skip color?
		mov		al, [eax]
        je      @@byte_skip             ;yes
        mov     BYTE PTR [edi], al      ;store the byte
@@byte_skip:
        inc     edi                     ;next destination pixel
        dec     ecx                     ;decrement remaining count
        jnz     @@main_loop             ;no, do another byte

@@done_row:
        add     esi,in_src_scan         ;wrap to next source offset
        add     edi,in_dst_scan         ;wrap to next dest offset
        mov     ecx,in_srcW             ;get width of bitmap
        dec     in_srcH                 ;decrement row count
        jnz     @@main_loop             ;process next row

        ret

gfx_draw_transparent_bm_s ENDP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;±
;±  NAME
;±      gfx_draw_translucent_bm
;±      
;±  DESCRIPTION
;±      Copies a block of memory from a sub-region of a source bitmap to
;±      a sub-region of same size in a destination bitmap.
;±      Source color is combined with destination color for translucency
;±      
;±  ARGUMENTS
;±      in_dst_offset   - starting address of destination block
;±      in_src_offset   - starting address of source block
;±      in_srcW         - width, in bytes, of source block
;±      in_srcH         - number of rows in source block
;±      in_src_scan     - offset from end of scan to start of next
;±      in_dst_scan     - offset from end of scan to start of next
;±      in_flip         - GFX flip flag
;±      in_table        - pointer to translucent table base address
;±      
;±  RETURNS
;±      nothing
;±      
;±  NOTES
;±      does DWORD writes that are DWORD aligned.
;±      
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

gfx_draw_translucent_bm PROC C \
        USES    ebx ecx edx esi edi,
        ARG     in_dst_offset,in_src_offset,in_srcW,in_srcH,in_src_scan,in_dst_scan,in_flip,in_table
        PUBLIC  C gfx_draw_translucent_bm

        ; check to see if function has been marked as writeable for code patching
        cmp     gdtbm_marked_writeable,0        ;has function been marked writeable?
        jne     gdtbm_start_proc                ;yes, skip this part

        lea     esi,[gdtbm_start_proc]          ;get starting address of function
        mov     eax,gdtbm_end_proc - gdtbm_start_proc   ;get size of function
        invoke  markFunctionWritable, esi, eax  ;mark code seg writeable
        mov     gdtbm_marked_writeable,1        ;indicate function writeable

gdtbm_start_proc:
        ; start by patching code with constant values
        lea     edi,[@@patch1 - 4]      ;get address of patch
        mov     eax,in_table            ;get patch value
        mov     [edi],eax               ;store patch in code
        lea     edi,[@@patch2 - 4]      ;get address of patch
        mov     [edi],eax               ;store patch in code
        lea     edi,[@@patch3 - 4]      ;get address of patch
        mov     [edi],eax               ;store patch in code
        lea     edi,[@@patch4 - 4]      ;get address of patch
        mov     [edi],eax               ;store patch in code
        lea     edi,[@@patch5 - 4]      ;get address of patch
        mov     [edi],eax               ;store patch in code

        mov     esi,in_src_offset       ;get source offset
        mov     edi,in_dst_offset       ;get destination offset

        mov     ebx,0                   ;clear high word of table index

        mov     edx,in_srcW             ;get width of bitmap
        mov     ecx,1                   ;assume no flip_X
        test    in_flip,GFX_FLIP_X      ;flip left-to-right?
        jz      @@main_loop             ;no
        mov     ecx,-1                  ;yes, flip source

@@main_loop:
        test    edi,3                   ;are we on a DWORD boundary?
        jz      @@dword_check           ;=0 Yes
@@byte_write:
        mov     bl,[edi]                ;get destination pixel in low byte
        mov     bh,[esi]                ;get a byte
        mov     al,[ebx+01010101h]      ;load transparent color
                @@patch1:
        add     esi,ecx                 ;next source pixel
        mov     [edi], al               ;store the byte
        inc     edi                     ;next destination pixel
        dec     edx                     ;decrement remaining count
        jnz     @@main_loop             ;no, do another byte
        jmp     short @@done_row        ;finished, next row
@@dword_check:
        test    edx,NOT 3               ;yes, but are there any DWORDS to write?
        jz      @@byte_write            ;no, finish with bytes
        sub     edx,4                   ;yes, decrement count by one DWORD
                
@@dword_loop:
        mov     bl,[edi]                ;get destinatioin pixel
        mov     bh,[esi]                ;get source pixel
        mov     al,[ebx+02020202h]      ;get translucent color from table
                @@patch2:
        add     esi,ecx                 ;move to next source pixel

        mov     bl,[edi+1]              ;get destinatioin pixel
        mov     bh,[esi]                ;get source pixel
        mov     ah,[ebx+03030303h]      ;get translucent color from table
                @@patch3:
        add     esi,ecx                 ;move to next source pixel

        bswap   eax
        mov     bl,[edi+2]              ;get destinatioin pixel
        mov     bh,[esi]                ;get source pixel
        mov     ah,[ebx+04040404h]      ;get translucent color from table
                @@patch4:
        add     esi,ecx                 ;move to next source pixel

        mov     bl,[edi+3]              ;get destinatioin pixel
        mov     bh,[esi]                ;get source pixel
        mov     al,[ebx+05050505h]      ;get translucent color from table
                @@patch5:
        add     esi,ecx                 ;move to next source pixel
        bswap   eax

        mov     [edi],eax               ;store dword

        add     edi,4                   ;increment dest pointer
        sub     edx,4                   ;decrement column count four bytes
        jns     @@dword_loop            ;next four pixels
        add     edx,4                   ;get number of pixels left over
        jnz     @@byte_write            ;if any left, process bytes

@@done_row:
        add     esi,in_src_scan         ;wrap to next source offset
        add     edi,in_dst_scan         ;wrap to next dest offset
        mov     edx,in_srcW             ;get width of bitmap
        dec     in_srcH                 ;decrement row count
        jnz     @@main_loop             ;process next row

        ret

gdtbm_end_proc:
gfx_draw_translucent_bm ENDP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;±
;±  NAME
;±      gfx_draw_stretch_bm
;±      
;±  DESCRIPTION
;±      Copies a block of memory from a sub-region of a source bitmap to
;±      a sub-region of different size in a destination bitmap.
;±      
;±  ARGUMENTS
;±      in_src_offset   - starting address of source block
;±      in_srcClipW     - clipped source width
;±      in_srcClipH     - clipped source height
;±      in_srcW         - original unclipped source width
;±      in_srcH         - original unclipped source height
;±      in_dst_offset   - starting address of destination block
;±      in_dst_stride   - offset to next dest row
;±      in_dstClipW     - clipped dest width
;±      in_dstClipH     - clipped dest height
;±      in_dstW         - original unclipped dest width
;±      in_dstH         - original unclipped dest height
;±      in_adjX         - x adjustment for Lvar accumulator
;±      in_adjY         - y adjustment for Lvar accumulator
;±      in_flip         - GFX flip flag
;±      
;±  RETURNS
;±      nothing
;±      
;±  NOTES
;±      does DWORD writes that are DWORD aligned.
;±      
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

gfx_draw_stretch_bm PROC C \
        USES  ebx esi edi,
        ARG   in_src_offset, in_src_stride, in_srcClipW, in_srcClipH,   \
              in_srcW, in_srcH, in_dst_offset, in_dst_stride,          \
              in_dstClipW,             \
              in_dstClipH,             \
              in_dstW,                 \
              in_dstH,                 \
              in_adjX,                 \
              in_adjY,                 \
              in_flip
              LOCAL   LvarH,Bttl,count
        PUBLIC  C gfx_draw_stretch_bm

        mov     eax,in_srcH             ;Get source height
        mov     edx,0
        div     in_dstH                 ;div by dest height for Lvar
                                        ;and row multiplication factor
        mov     ebx,in_src_stride       ;get source row scan offset
        push    edx                     ;save remainder for Lvar calculation
        mul     ebx                     ;get row increment amount
        mov     YarrCC,eax              ;save increment for when carry occurs
        add     YarrCC,ebx
        mov     YarrNC,eax              ;save increment for when no carry occurs
        mov     eax,0
        pop     edx
        div     in_dstH                 ;get Lvar
        inc     eax ; adjustment for hex rounding
        mov     LvarH,eax               ;save Lvar for height
        mul     in_adjY                 ;get row total adjustment due to clipping
        mov     Bttl,eax                ;save adjusted starting total

        mov     eax,in_srcW             ;Get source width
        mov     edx,0
        div     in_dstW                 ;div by dest width for Lvar
        mov     ecx,eax                 ;save column incrmentor
        mov     eax,0
        div     in_dstW                 ;get Lvar
        inc     eax ; adjustment for hex rounding
        mov     ebx,eax                 ;save Lvar for width
        mul     in_adjX                 ;get column total adjustment due to clipping
        mov     in_adjX,eax             ;save it back to this variable

        mov     esi,in_src_offset       ;get source offset
        mov     edi,in_dst_offset       ;get destination offset

        cmp     in_flip,0               ;are we doing right-to-left draw?
        jne     @@row_loopX             ;yes

@@row_loop:
        mov     eax,in_dstClipW         ;get width of destination
        mov     count,eax               ;save to a local variable
        mov     edx,in_adjX             ;get Lvar width seed
        push    esi                     ;save row offset

        test    edi,3                   ;are we on a DWORD boundary?
        jz      @@dword_check           ;yes, see if there are any DWORDS to write

        ;------------ inner loop

@@byte_loop:
        mov     al,[esi]                ;get source pixel
        add     edx,ebx                 ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        mov     [edi],al                ;store opaque color
        inc     edi                     ;advance to next dest pixel
        dec     count                   ;decrement column count
        jz      @@done_row              ;done
        test    edi,3                   ;are we on a DWORD boundary?
        jnz     @@byte_loop             ;no, do another byte
@@dword_check:
        test    count, NOT 3            ;yes, but are there any DWORDS to write
        jz      @@byte_loop             ;no, finish with bytes
        sub     count,4                 ;yes, decrement count by four pixels

@@dword_loop:
        mov     al,[esi]                ;get source pixel
        add     edx,ebx                 ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        mov     ah,[esi]                ;get source pixel
        add     edx,ebx                 ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        bswap   eax                     ;swap low word with high, WARNING *** i486 or better
        mov     ah,[esi]                ;get source pixel
        add     edx,ebx                 ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        mov     al,[esi]                ;get source pixel
        add     edx,ebx                 ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        bswap   eax

        mov     [edi],eax               ;store opaque color

        add     edi,4                   ;advance to next dest pixel
        sub     count,4                 ;decrement column count
        jns     @@dword_loop            ;next 4 pixels
        add     count,4                 ;get number of pixels left over
        jnz     @@byte_loop             ;if any left over, process as bytes

@@done_row:
        pop     esi                     ;get back row offset
        add     edi,in_dst_stride       ;wrap to next dest row
        mov     eax,Bttl
        add     eax,LvarH               ;add Lvar increment
        mov     Bttl,eax
        sbb     edx,edx                 ;get y increment array index
        add     esi,YarrNC[edx*4]       ;increment row based on carry
        dec     in_dstClipH             ;decrement row count
        jnz     @@row_loop              ;process next row

        ret

;------- flip right-to-left

@@row_loopX:
        mov     eax,in_dstClipW         ;get width of destination
        mov     count,eax               ;save to a local variable for dword count
        mov     edx,in_adjX             ;get Lvar width seed
        push    esi                     ;save row offset

        not     edi
        test    edi,3                   ;are we on a DWORD boundary?
        not     edi
        jz      @@dword_checkX          ;yes, see if there are any DWORDS to write

        ;------------ inner loop
        
@@byte_loopX:
        mov     al,[esi]                ;get source pixel
        add     edx,ebx                 ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        mov     [edi],al                ;store opaque color
        dec     edi                     ;advance to next dest pixel
        dec     count                   ;decrement column count
        jz      @@done_rowX             ;done
        not     edi
        test    edi,3                   ;are we on a DWORD boundary?
        not     edi
        jnz     @@byte_loopX            ;no, do another byte

@@dword_checkX:
        test    count,NOT 3             ;yes, but are there any DWORDS to write?
        jz      @@byte_loopX            ;no, finish with bytes
        sub     edi,3                   ;yes, adjust destination
        sub     count,4                 ;decrement count by four pixels

@@dword_loopX:
        mov     al,[esi]                ;get source pixel
        add     edx,ebx                 ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        mov     ah,[esi]                ;get source pixel
        add     edx,ebx                 ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        bswap   eax                     ;swap low word with high, WARNING *** i486 or better
        mov     ah,[esi]                ;get source pixel
        add     edx,ebx                 ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        mov     al,[esi]                ;get source pixel
        add     edx,ebx                 ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry

        mov     [edi],eax               ;store opaque color

        sub     edi,4                   ;advance to next dest pixel
        sub     count,4                 ;decrement column count
        jns     @@dword_loopX           ;next 4 pixels
        add     edi,3                   ;adjust back to get on pixel boundary
        add     count,4                 ;get number of pixels left over
        jnz     @@byte_loopX            ;if any left over, process as bytes
@@done_rowX:
        inc     edi                     ;adjust for next scan line
        pop     esi                     ;get back row offset
        add     edi,in_dst_stride       ;wrap to next dest row
        dec     edi
        mov     eax,Bttl
        add     eax,LvarH               ;add Lvar increment
        mov     Bttl,eax
        sbb     edx,edx                 ;get y increment array index
        add     esi,YarrNC[edx*4]       ;increment row based on carry
        dec     in_dstClipH             ;decrement row count
        jnz     @@row_loopX             ;process next row

        ret

gfx_draw_stretch_bm ENDP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;±
;±  NAME
;±      gfx_draw_stretch_bm
;±      
;±  DESCRIPTION
;±      Copies a block of memory from a sub-region of a source bitmap to
;±      a sub-region of different size in a destination bitmap.
;±      
;±  ARGUMENTS
;±      in_src_offset   - starting address of source block
;±      in_srcClipW     - clipped source width
;±      in_srcClipH     - clipped source height
;±      in_srcW         - original unclipped source width
;±      in_srcH         - original unclipped source height
;±      in_dst_offset   - starting address of destination block
;±      in_dst_stride   - offset to next dest row
;±      in_dstClipW     - clipped dest width
;±      in_dstClipH     - clipped dest height
;±      in_dstW         - original unclipped dest width
;±      in_dstH         - original unclipped dest height
;±      in_adjX         - x adjustment for Lvar accumulator
;±      in_adjY         - y adjustment for Lvar accumulator
;±      in_flip         - GFX flip flag
;±      
;±  RETURNS
;±      nothing
;±      
;±  NOTES
;±      does DWORD writes that are DWORD aligned.
;±      
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

gfx_draw_stretch_bm_s PROC C \
        USES  ebx esi edi,
        ARG   in_src_offset, in_src_stride, in_srcClipW, in_srcClipH,   \
              in_srcW, in_srcH, in_dst_offset, in_dst_stride,          \
              in_dstClipW, \
              in_dstClipH, \
              in_dstW, \
              in_dstH, \
              in_adjX, \
              in_adjY, \
              in_flip, \
              in_shadeMap
              LOCAL   LvarH,Bttl,count
        PUBLIC  C gfx_draw_stretch_bm_s

        mov     eax,in_srcH             ;Get source height
        mov     edx,0
        div     in_dstH                 ;div by dest height for Lvar
                                        ;and row multiplication factor
        mov     ebx,in_src_stride       ;get source row scan offset
        push    edx                     ;save remainder for Lvar calculation
        mul     ebx                     ;get row increment amount
        mov     YarrCC,eax              ;save increment for when carry occurs
        add     YarrCC,ebx
        mov     YarrNC,eax              ;save increment for when no carry occurs
        mov     eax,0
        pop     edx
        div     in_dstH                 ;get Lvar
        inc     eax ; adjustment for hex rounding
        mov     LvarH,eax               ;save Lvar for height
        mul     in_adjY                 ;get row total adjustment due to clipping
        mov     Bttl,eax                ;save adjusted starting total

        mov     eax,in_srcW             ;Get source width
        mov     edx,0
        div     in_dstW                 ;div by dest width for Lvar
        mov     ecx,eax                 ;save column incrmentor
        mov     eax,0
        div     in_dstW                 ;get Lvar
        inc     eax ; adjustment for hex rounding
        mov     ebx,eax                 ;save Lvar for width
        mul     in_adjX                 ;get column total adjustment due to clipping
        mov     in_adjX,eax             ;save it back to this variable

        mov     esi,in_src_offset       ;get source offset
        mov     edi,in_dst_offset       ;get destination offset

        cmp     in_flip,0               ;are we doing right-to-left draw?
        jne     @@row_loopX             ;yes

@@row_loop:
        mov     eax,in_dstClipW         ;get width of destination
        mov     count,eax               ;save to a local variable
        mov     edx,in_adjX             ;get Lvar width seed
        push    esi                     ;save row offset

        ;------------ inner loop

        mov     eax, in_shadeMap
@@byte_loop:
        mov     al,[esi]                ;get source pixel
        add     edx,ebx                 ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        mov     al, [eax]               ;lookup
        inc     edi                     ;advance to next dest pixel
        mov     [edi-1],al                ;store opaque color
        dec     count                   ;decrement column count
        jnz     @@byte_loop             ;done?

@@done_row:
        pop     esi                     ;get back row offset
        add     edi,in_dst_stride       ;wrap to next dest row
        mov     eax,Bttl
        add     eax,LvarH               ;add Lvar increment
        mov     Bttl,eax
        sbb     edx,edx                 ;get y increment array index
        add     esi,YarrNC[edx*4]       ;increment row based on carry
        dec     in_dstClipH             ;decrement row count
        jnz     @@row_loop              ;process next row

        ret

;------- flip right-to-left

@@row_loopX:
        mov     eax,in_dstClipW         ;get width of destination
        mov     count,eax               ;save to a local variable for dword count
        mov     edx,in_adjX             ;get Lvar width seed
        push    esi                     ;save row offset

        ;------------ inner loop
        mov     eax, in_shadeMap
@@byte_loopX:
        mov     al,[esi]                ;get source pixel
        add     edx,ebx                 ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        mov     [edi],al                ;store opaque color
        dec     edi                     ;advance to next dest pixel
        dec     count                   ;decrement column count
        jnz     @@byte_loopX            ;done?

@@done_rowX:
        inc     edi                     ;adjust for next scan line
        pop     esi                     ;get back row offset
        add     edi,in_dst_stride       ;wrap to next dest row
        dec     edi
        mov     eax,Bttl
        add     eax,LvarH               ;add Lvar increment
        mov     Bttl,eax
        sbb     edx,edx                 ;get y increment array index
        add     esi,YarrNC[edx*4]       ;increment row based on carry
        dec     in_dstClipH             ;decrement row count
        jnz     @@row_loopX             ;process next row

        ret

gfx_draw_stretch_bm_s ENDP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;±
;±  NAME
;±      gfx_draw_stretch_transparent_bm
;±      
;±  DESCRIPTION
;±      Copies a block of memory from a sub-region of a source bitmap to
;±      a sub-region of different size in a destination bitmap.
;±      0 color is treated as transparent.
;±      
;±  ARGUMENTS
;±      in_src_offset   - starting address of source block
;±      in_srcClipW     - clipped source width
;±      in_srcClipH     - clipped source height
;±      in_srcW         - original unclipped source width
;±      in_srcH         - original unclipped source height
;±      in_dst_offset   - starting address of destination block
;±      in_dst_stride   - offset to next dest row
;±      in_dstClipW     - clipped dest width
;±      in_dstClipH     - clipped dest height
;±      in_dstW         - original unclipped dest width
;±      in_dstH         - original unclipped dest height
;±      in_adjX         - x adjustment for Lvar accumulator
;±      in_adjY         - y adjustment for Lvar accumulator
;±      in_flip         - GFX flip flag
;±      
;±  RETURNS
;±      nothing
;±      
;±  NOTES
;±      does DWORD writes that are DWORD aligned.
;±      
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

gfx_draw_stretch_transparent_bm PROC C \
        USES  ebx esi edi,
        ARG   in_src_offset, in_src_stride, in_srcClipW, in_srcClipH,        \
              in_srcW, in_srcH, in_dst_offset, in_dst_stride,           \
              in_dstClipW, in_dstClipH,             \
              in_dstW, in_dstH, in_adjX, in_adjY,                 \
              in_flip
              LOCAL   LvarH,LvarW,Bttl,count
        PUBLIC  C gfx_draw_stretch_transparent_bm

        mov     eax,in_srcH             ;Get source height
        mov     edx,0
        div     in_dstH                 ;div by dest height for Lvar
                                        ;and row multiplication factor
        mov     ebx,in_src_stride       ;get source row scan offset
        push    edx                     ;save remainder for Lvar calculation
        mul     ebx                     ;get row increment amount
        mov     YarrCC,eax              ;save increment for when carry occurs
        add     YarrCC,ebx
        mov     YarrNC,eax              ;save increment for when no carry occurs
        mov     eax,0
        pop     edx
        div     in_dstH                 ;get Lvar
        inc     eax ; adjustment for hex rounding
        mov     LvarH,eax               ;save Lvar for height
        mul     in_adjY                 ;get row total adjustment due to clipping
        mov     Bttl,eax                ;save adjusted starting total

        mov     eax,in_srcW             ;Get source width
        mov     edx,0
        div     in_dstW                 ;div by dest width for Lvar
        mov     ecx,eax                 ;save column incrmentor
        mov     eax,0
        div     in_dstW                 ;get Lvar
        inc     eax ; adjustment for hex rounding
        mov     LvarW,eax               ;save Lvar for width
        mul     in_adjX                 ;get column total adjustment due to clipping
        mov     in_adjX,eax             ;save it back to this variable

        mov     esi,in_src_offset       ;get source offset
        mov     edi,in_dst_offset       ;get destination offset

        cmp     in_flip,0               ;are we doing right-to-left draw?
        jne     @@row_loopX             ;yes

@@row_loop:
        mov     eax,in_dstClipW         ;get width of destination
        mov     count,eax               ;save to a local variable
        mov     edx,in_adjX             ;get Lvar width seed
        push    esi                     ;save row offset

        test    edi,3                   ;are we on a DWORD boundary?
        jz      @@dword_check           ;yes, see if there are any DWORDS to write

        ;------------ inner loop

@@byte_loop:
        mov     al,[esi]                ;get source pixel
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        cmp     al,0                    ;skip color?
        je      @@byte_skip             ;yes
        mov     [edi],al                ;store opaque color
@@byte_skip:
        inc     edi                     ;advance to next dest pixel
        dec     count                   ;decrement column count
        jz      @@done_row              ;done
        test    edi,3                   ;are we on a DWORD boundary?
        jnz     @@byte_loop             ;no, do another byte
@@dword_check:
        test    count,NOT 3             ;yes, but are there any DWORDS to write
        jz      @@byte_loop             ;no, finish with bytes
        sub     count,4                 ;yes, decrement count by four pixels

@@dword_loop:
        mov     ebx,[edi]               ;get original source color
        mov     al,[esi]                ;get source pixel
        cmp     al,0                    ;transparent color?
        jne     @@byte1_skip            ;no, use source color
        mov     al,bl                   ;yes, use original color
@@byte1_skip:
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        mov     ah,[esi]                ;get source pixel
        cmp     ah,0                    ;transparent color?
        jne     @@byte2_skip            ;no, use source color
        mov     ah,bh                   ;yes, use original color
@@byte2_skip:
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        bswap   eax                     ;swap low word with high, WARNING *** i486 or better
        bswap   ebx                     ;swap low word with high, WARNING *** i486 or better
        mov     ah,[esi]                ;get source pixel
        cmp     ah,0                    ;transparent color?
        jne     @@byte3_skip            ;no, use source color
        mov     ah,bh                   ;yes, use original color
@@byte3_skip:
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        mov     al,[esi]                ;get source pixel
        cmp     al,0                    ;transparent color?
        jne     @@byte4_skip            ;no, use source color
        mov     al,bl                   ;yes, use original color
@@byte4_skip:
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        bswap   eax

        mov     [edi],eax               ;store opaque color

        add     edi,4                   ;advance to next dest pixel
        sub     count,4                 ;decrement column count
        jns     @@dword_loop            ;next 4 pixels
        add     count,4                 ;get number of pixels left over
        jnz     @@byte_loop             ;if any left over, process as bytes

@@done_row:
        pop     esi                     ;get back row offset
        add     edi,in_dst_stride       ;wrap to next dest row
        mov     eax,Bttl
        add     eax,LvarH               ;add Lvar increment
        mov     Bttl,eax
        sbb     edx,edx                 ;get y increment array index
        add     esi,YarrNC[edx*4]       ;increment row based on carry
        dec     in_dstClipH             ;decrement row count
        jnz     @@row_loop              ;process next row

        ret

;------- flip right-to-left

@@row_loopX:
        mov     eax,in_dstClipW         ;get width of destination
        mov     count,eax               ;save to a local variable for dword count
        mov     edx,in_adjX             ;get Lvar width seed
        push    esi                     ;save row offset

        not     edi
        test    edi,3                   ;are we on a DWORD boundary?
        not     edi
        jz      @@dword_checkX          ;yes, see if there are any DWORDS to write

        ;------------ inner loop
        
@@byte_loopX:
        mov     al,[esi]                ;get source pixel
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        cmp     al,0                    ;skip color?
        je      @@byte_skipX            ;yes
        mov     [edi],al                ;store opaque color
@@byte_skipX:
        dec     edi                     ;advance to next dest pixel
        dec     count                   ;decrement column count
        jz      @@done_rowX             ;done
        not     edi
        test    edi,3                   ;are we on a DWORD boundary?
        not     edi
        jnz     @@byte_loopX            ;no, do another byte

@@dword_checkX:
        test    count,NOT 3             ;yes, but are there any DWORDS to write?
        jz      @@byte_loopX            ;no, finish with bytes
        sub     edi,3                   ;yes, adjust destination
        sub     count,4                 ;decrement count by four pixels

@@dword_loopX:
        mov     ebx,[edi]               ;get original source pixels
        bswap   ebx
        mov     al,[esi]                ;get source pixel
        cmp     al,0                    ;transparent color?
        jne     @@byte1_skipX           ;no, use source color
        mov     al,bl                   ;yes, use original color
@@byte1_skipX:
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        mov     ah,[esi]                ;get source pixel
        cmp     ah,0                    ;transparent color?
        jne     @@byte2_skipX           ;no, use source color
        mov     ah,bh                   ;yes, use original color
@@byte2_skipX:
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        bswap   eax                     ;swap low word with high, WARNING *** i486 or better
        bswap   ebx                     ;swap low word with high, WARNING *** i486 or better
        mov     ah,[esi]                ;get source pixel
        cmp     ah,0                    ;transparent color?
        jne     @@byte3_skipX           ;no, use source color
        mov     ah,bh                   ;yes, use original color
@@byte3_skipX:
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        mov     al,[esi]                ;get source pixel
        cmp     al,0                    ;transparent color?
        jne     @@byte4_skipX           ;no, use source color
        mov     al,bl                   ;yes, use original color
@@byte4_skipX:
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry

        mov     [edi],eax               ;store opaque color

        sub     edi,4                   ;advance to next dest pixel
        sub     count,4                 ;decrement column count
        jns     @@dword_loopX           ;next 4 pixels
        add     edi,3                   ;adjust back to get on pixel boundary
        add     count,4                 ;get number of pixels left over
        jnz     @@byte_loopX            ;if any left over, process as bytes
@@done_rowX:
        inc     edi                     ;adjust for next scan line
        pop     esi                     ;get back row offset
        add     edi,in_dst_stride       ;wrap to next dest row
        dec     edi
        mov     eax,Bttl
        add     eax,LvarH               ;add Lvar increment
        mov     Bttl,eax
        sbb     edx,edx                 ;get y increment array index
        add     esi,YarrNC[edx*4]       ;increment row based on carry
        dec     in_dstClipH             ;decrement row count
        jnz     @@row_loopX             ;process next row

        ret

gfx_draw_stretch_transparent_bm ENDP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;±
;±  NAME
;±      gfx_draw_stretch_transparent_bm
;±      
;±  DESCRIPTION
;±      Copies a block of memory from a sub-region of a source bitmap to
;±      a sub-region of different size in a destination bitmap.
;±      0 color is treated as transparent.
;±      
;±  ARGUMENTS
;±      in_src_offset   - starting address of source block
;±      in_srcClipW     - clipped source width
;±      in_srcClipH     - clipped source height
;±      in_srcW         - original unclipped source width
;±      in_srcH         - original unclipped source height
;±      in_dst_offset   - starting address of destination block
;±      in_dst_stride   - offset to next dest row
;±      in_dstClipW     - clipped dest width
;±      in_dstClipH     - clipped dest height
;±      in_dstW         - original unclipped dest width
;±      in_dstH         - original unclipped dest height
;±      in_adjX         - x adjustment for Lvar accumulator
;±      in_adjY         - y adjustment for Lvar accumulator
;±      in_flip         - GFX flip flag
;±      
;±  RETURNS
;±      nothing
;±      
;±  NOTES
;±      does DWORD writes that are DWORD aligned.
;±      
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

gfx_draw_stretch_transparent_bm_s PROC C \
        USES  ebx esi edi,
        ARG   in_src_offset, in_src_stride, in_srcClipW, in_srcClipH,        \
              in_srcW, in_srcH, in_dst_offset, in_dst_stride,           \
              in_dstClipW, in_dstClipH,             \
              in_dstW, in_dstH, in_adjX, in_adjY,                 \
              in_flip, in_shadeMap
              LOCAL   LvarH,LvarW,Bttl,count
        PUBLIC  C gfx_draw_stretch_transparent_bm_s

        mov     eax,in_srcH             ;Get source height
        mov     edx,0
        div     in_dstH                 ;div by dest height for Lvar
                                        ;and row multiplication factor
        mov     ebx,in_src_stride       ;get source row scan offset
        push    edx                     ;save remainder for Lvar calculation
        mul     ebx                     ;get row increment amount
        mov     YarrCC,eax              ;save increment for when carry occurs
        add     YarrCC,ebx
        mov     YarrNC,eax              ;save increment for when no carry occurs
        mov     eax,0
        pop     edx
        div     in_dstH                 ;get Lvar
        inc     eax ; adjustment for hex rounding
        mov     LvarH,eax               ;save Lvar for height
        mul     in_adjY                 ;get row total adjustment due to clipping
        mov     Bttl,eax                ;save adjusted starting total

        mov     eax,in_srcW             ;Get source width
        mov     edx,0
        div     in_dstW                 ;div by dest width for Lvar
        mov     ecx,eax                 ;save column incrmentor
        mov     eax,0
        div     in_dstW                 ;get Lvar
        inc     eax ; adjustment for hex rounding
        mov     LvarW,eax               ;save Lvar for width
        mul     in_adjX                 ;get column total adjustment due to clipping
        mov     in_adjX,eax             ;save it back to this variable

        mov     esi,in_src_offset       ;get source offset
        mov     edi,in_dst_offset       ;get destination offset

        cmp     in_flip,0               ;are we doing right-to-left draw?
        jne     @@row_loopX             ;yes

@@row_loop:
        mov     eax,in_dstClipW         ;get width of destination
        mov     count,eax               ;save to a local variable
        mov     edx,in_adjX             ;get Lvar width seed
        push    esi                     ;save row offset

        ;------------ inner loop
        mov     eax, in_shadeMap
@@byte_loop:
        mov     al,[esi]                ;get source pixel
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        cmp     al,0                    ;skip color?
        mov     al, [eax]
        je      @@byte_skip             ;yes
        mov     [edi],al                ;store opaque color
@@byte_skip:
        inc     edi                     ;advance to next dest pixel
        dec     count                   ;decrement column count
        jnz     @@byte_loop             ;done

@@done_row:
        pop     esi                     ;get back row offset
        add     edi,in_dst_stride       ;wrap to next dest row
        mov     eax,Bttl
        add     eax,LvarH               ;add Lvar increment
        mov     Bttl,eax
        sbb     edx,edx                 ;get y increment array index
        add     esi,YarrNC[edx*4]       ;increment row based on carry
        dec     in_dstClipH             ;decrement row count
        jnz     @@row_loop              ;process next row

        ret

;------- flip right-to-left

@@row_loopX:
        mov     eax,in_dstClipW         ;get width of destination
        mov     count,eax               ;save to a local variable for dword count
        mov     edx,in_adjX             ;get Lvar width seed
        push    esi                     ;save row offset

        ;------------ inner loop
        
        mov     eax, in_shadeMap
@@byte_loopX:
        mov     al,[esi]                ;get source pixel
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        cmp     al,0                    ;skip color?
        mov     al, [eax]
        je      @@byte_skipX            ;yes
        mov     [edi],al                ;store opaque color
@@byte_skipX:
        dec     edi                     ;advance to next dest pixel
        dec     count                   ;decrement column count
        jnz     @@byte_loopX            ;done?

@@done_rowX:
        inc     edi                     ;adjust for next scan line
        pop     esi                     ;get back row offset
        add     edi,in_dst_stride       ;wrap to next dest row
        dec     edi
        mov     eax,Bttl
        add     eax,LvarH               ;add Lvar increment
        mov     Bttl,eax
        sbb     edx,edx                 ;get y increment array index
        add     esi,YarrNC[edx*4]       ;increment row based on carry
        dec     in_dstClipH             ;decrement row count
        jnz     @@row_loopX             ;process next row

        ret

gfx_draw_stretch_transparent_bm_s ENDP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;±
;±  NAME
;±      gfx_draw_stretch_translucent_bm
;±      
;±  DESCRIPTION
;±      Copies a block of memory from a sub-region of a source bitmap to
;±      a sub-region of different size in a destination bitmap.
;±      Source color is combined with destination color for translucency
;±      
;±  ARGUMENTS
;±      in_src_offset   - starting address of source block
;±      in_srcClipW     - clipped source width
;±      in_srcClipH     - clipped source height
;±      in_srcW         - original unclipped source width
;±      in_srcH         - original unclipped source height
;±      in_dst_offset   - starting address of destination block
;±      in_dst_stride   - offset to next dest row
;±      in_dstClipW     - clipped dest width
;±      in_dstClipH     - clipped dest height
;±      in_dstW         - original unclipped dest width
;±      in_dstH         - original unclipped dest height
;±      in_adjX         - x adjustment for Lvar accumulator
;±      in_adjY         - y adjustment for Lvar accumulator
;±      in_flip         - GFX flip flag
;±      in_table        - pointer to translucent table base address
;±      
;±  RETURNS
;±      nothing
;±      
;±  NOTES
;±      does DWORD writes that are DWORD aligned.
;±      
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

gfx_draw_stretch_translucent_bm PROC C  \
        USES  ebx esi edi,
        ARG   in_src_offset, in_src_stride, in_srcClipW, in_srcClipH,  \
              in_srcW, in_srcH, in_dst_offset, in_dst_stride,          \
              in_dstClipW, in_dstClipH, in_dstW, in_dstH,              \
              in_adjX,                  \
              in_adjY,                  \
              in_flip,                  \
              in_table
              LOCAL   LvarH,LvarW,Bttl,count
        PUBLIC  C gfx_draw_stretch_translucent_bm
        ; check to see if function has been marked as writeable for code patching
        cmp     gdstbm_marked_writeable,0        ;has function been marked writeable?
        jne     gdstbm_start_proc                ;yes, skip this part

        lea     esi,[gdstbm_start_proc]          ;get starting address of function
        mov     eax,gdstbm_end_proc - gdstbm_start_proc   ;get size of function
        invoke  markFunctionWritable, esi, eax   ;mark code seg writeable
        mov     gdstbm_marked_writeable,1        ;indicate function writeable

gdstbm_start_proc:
        ; start by patching code with constant values
        test    in_flip,GFX_FLIP_X      ;is bitmap flipped left-to-right
        jnz     @@patch_flipX           ;yes, patch flipped X values

        lea     edi,[@@patch1 - 4]      ;get address of patch
        mov     eax,in_table            ;get patch value
        mov     [edi],eax               ;store patch in code
        lea     edi,[@@patch2 - 4]      ;get address of patch
        mov     [edi],eax               ;store patch in code
        lea     edi,[@@patch3 - 4]      ;get address of patch
        mov     [edi],eax               ;store patch in code
        lea     edi,[@@patch4 - 4]      ;get address of patch
        mov     [edi],eax               ;store patch in code
        lea     edi,[@@patch5 - 4]      ;get address of patch
        mov     [edi],eax               ;store patch in code
        jmp     short @@setup           ;continue with setup

@@patch_flipX:
        lea     edi,[@@patch1X - 4]     ;get address of patch
        mov     eax,in_table            ;get patch value
        mov     [edi],eax               ;store patch in code
        lea     edi,[@@patch2X - 4]     ;get address of patch
        mov     [edi],eax               ;store patch in code
        lea     edi,[@@patch3X - 4]     ;get address of patch
        mov     [edi],eax               ;store patch in code
        lea     edi,[@@patch4X - 4]     ;get address of patch
        mov     [edi],eax               ;store patch in code
        lea     edi,[@@patch5X - 4]     ;get address of patch
        mov     [edi],eax               ;store patch in code

@@setup:
        ;set up rest of the registers
        mov     eax,in_srcH             ;Get source height
        mov     edx,0
        div     in_dstH                 ;div by dest height for Lvar
                                        ;and row multiplication factor
        mov     ebx,in_src_stride       ;get source row scan offset
        push    edx                     ;save remainder for Lvar calculation
        mul     ebx                     ;get row increment amount
        mov     YarrCC,eax              ;save increment for when carry occurs
        add     YarrCC,ebx
        mov     YarrNC,eax              ;save increment for when no carry occurs
        mov     eax,0
        pop     edx
        div     in_dstH                 ;get Lvar
        inc     eax ; adjustment for hex rounding
        mov     LvarH,eax               ;save Lvar for height
        mul     in_adjY                 ;get row total adjustment due to clipping
        mov     Bttl,eax                ;save adjusted starting total

        mov     eax,in_srcW             ;Get source width
        mov     edx,0
        div     in_dstW                 ;div by dest width for Lvar
        mov     ecx,eax                 ;save column incrmentor
        mov     eax,0
        div     in_dstW                 ;get Lvar
        inc     eax ; adjustment for hex rounding
        mov     LvarW,eax               ;save Lvar for width
        mul     in_adjX                 ;get column total adjustment due to clipping
        mov     in_adjX,eax             ;save it back to this variable

        mov     esi,in_src_offset       ;get source offset
        mov     edi,in_dst_offset       ;get destination offset

        mov     ebx,0                   ;clear translucent index register

        cmp     in_flip,0               ;are we doing right-to-left draw?
        jne     @@row_loopX             ;yes

@@row_loop:
        mov     eax,in_dstClipW         ;get width of destination
        mov     count,eax               ;save to a local variable
        mov     edx,in_adjX             ;get Lvar width seed
        push    esi                     ;save row offset

        test    edi,3                   ;are we on a DWORD boundary?
        jz      @@dword_check           ;yes, see if there are any DWORDS to write

        ;------------ inner loop

@@byte_loop:
        mov     bl,[edi]                ;get dest pixel
        mov     bh,[esi]                ;get source pixel
        mov     al,[ebx+01010101h]      ;get translucent color
                @@patch1:
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        mov     [edi],al                ;store opaque color
        inc     edi                     ;advance to next dest pixel
        dec     count                   ;decrement column count
        jz      @@done_row              ;done
        test    edi,3                   ;are we on a DWORD boundary?
        jnz     @@byte_loop             ;no, do another byte
@@dword_check:
        test    count,NOT 3             ;yes, but are there any DWORDS to write
        jz      @@byte_loop             ;no, finish with bytes
        sub     count,4                 ;yes, decrement count by four pixels

@@dword_loop:
        mov     bl,[edi]                ;get dest pixel
        mov     bh,[esi]                ;get source pixel
        mov     al,[ebx+02020202h]      ;get translucent color from table
                @@patch2:
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        mov     bl,[edi+1]              ;get dest pixel
        mov     bh,[esi]                ;get source pixel
        mov     ah,[ebx+03030303h]      ;get translucent color from table
                @@patch3:
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        bswap   eax                     ;swap low word with high, WARNING *** i486 or better
        mov     bl,[edi+2]              ;get dest pixel
        mov     bh,[esi]                ;get source pixel
        mov     ah,[ebx+04040404h]      ;get translucent color from table
                @@patch4:
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        mov     bl,[edi+3]              ;get dest pixel
        mov     bh,[esi]                ;get source pixel
        mov     al,[ebx+05050505h]      ;get translucent color from table
                @@patch5:
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        bswap   eax

        mov     [edi],eax               ;store opaque color

        add     edi,4                   ;advance to next dest pixel
        sub     count,4                 ;decrement column count
        jns     @@dword_loop            ;next 4 pixels
        add     count,4                 ;get number of pixels left over
        jnz     @@byte_loop             ;if any left over, process as bytes

@@done_row:
        pop     esi                     ;get back row offset
        add     edi,in_dst_stride       ;wrap to next dest row
        mov     eax,Bttl
        add     eax,LvarH               ;add Lvar increment
        mov     Bttl,eax
        sbb     edx,edx                 ;get y increment array index
        add     esi,YarrNC[edx*4]       ;increment row based on carry
        dec     in_dstClipH             ;decrement row count
        jnz     @@row_loop              ;process next row

        ret

;------- flip right-to-left

@@row_loopX:
        mov     eax,in_dstClipW         ;get width of destination
        mov     count,eax               ;save to a local variable for dword count
        mov     edx,in_adjX             ;get Lvar width seed
        push    esi                     ;save row offset

        not     edi
        test    edi,3                   ;are we on a DWORD boundary?
        not     edi
        jz      @@dword_checkX          ;yes, see if there are any DWORDS to write

        ;------------ inner loop
        
@@byte_loopX:
        mov     bl,[edi]                ;get dest pixel
        mov     bh,[esi]                ;get source pixel
        mov     al,[ebx+01010101h]      ;get translucent color
                @@patch1X:
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        mov     [edi],al                ;store opaque color
        dec     edi                     ;advance to next dest pixel
        dec     count                   ;decrement column count
        jz      @@done_rowX             ;done
        not     edi
        test    edi,3                   ;are we on a DWORD boundary?
        not     edi
        jnz     @@byte_loopX            ;no, do another byte

@@dword_checkX:
        test    count,NOT 3             ;yes, but are there any DWORDS to write?
        jz      @@byte_loopX            ;no, finish with bytes
        sub     edi,3                   ;yes, adjust destination
        sub     count,4                 ;decrement count by four pixels

@@dword_loopX:
        mov     bl,[edi+3]              ;get dest pixel
        mov     bh,[esi]                ;get source pixel
        mov     al,[ebx+02020202h]      ;get translucent color from table
                @@patch2X:
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        mov     bl,[edi+2]              ;get dest pixel
        mov     bh,[esi]                ;get source pixel
        mov     ah,[ebx+03030303h]      ;get translucent color from table
                @@patch3X:
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        bswap   eax                     ;swap low word with high, WARNING *** i486 or better
        mov     bl,[edi+1]              ;get dest pixel
        mov     bh,[esi]                ;get source pixel
        mov     ah,[ebx+04040404h]      ;get translucent color from table
                @@patch4X:
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry
        mov     bl,[edi]                ;get dest pixel
        mov     bh,[esi]                ;get source pixel
        mov     al,[ebx+05050505h]      ;get translucent color from table
                @@patch5X:
        add     edx,LvarW               ;add Lvar increment
        adc     esi,ecx                 ;add increment + carry

        mov     [edi],eax               ;store opaque color

        sub     edi,4                   ;advance to next dest pixel
        sub     count,4                 ;decrement column count
        jns     @@dword_loopX           ;next 4 pixels
        add     edi,3                   ;adjust back to get on pixel boundary
        add     count,4                 ;get number of pixels left over
        jnz     @@byte_loopX            ;if any left over, process as bytes
@@done_rowX:
        inc     edi                     ;adjust for next scan line
        pop     esi                     ;get back row offset
        add     edi,in_dst_stride       ;wrap to next dest row
        dec     edi
        mov     eax,Bttl
        add     eax,LvarH               ;add Lvar increment
        mov     Bttl,eax
        sbb     edx,edx                 ;get y increment array index
        add     esi,YarrNC[edx*4]       ;increment row based on carry
        dec     in_dstClipH             ;decrement row count
        jnz     @@row_loopX             ;process next row

        ret

gdstbm_end_proc:
gfx_draw_stretch_translucent_bm ENDP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;±
;±  NAME
;±      gfx_draw_bm_16
;±      
;±  DESCRIPTION
;±      Copies a block of memory from a sub-region of a source 8-bit bitmap to
;±      a sub-region of same size in a destination 16-bit bitmap.
;±      
;±  ARGUMENTS
;±      in_dst_offset   - starting address of destination block
;±      in_src_offset   - starting address of source block
;±      in_srcW         - width, in bytes, of source block
;±      in_srcH         - number of rows in source block
;±      in_src_scan     - offset from end of scan to start of next
;±      in_dst_scan     - offset from end of scan to start of next
;±      in_flip         - GFX flip flag
;±      in_table        - 16-bit color DWORD lookup table
;±      
;±  RETURNS
;±      nothing
;±      
;±  NOTES
;±      does DWORD writes that are DWORD aligned.
;±      
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

gfx_draw_bm_16 PROC C \
        USES    ebx ecx edx esi edi,
        ARG     in_dst_offset,in_src_offset,in_srcW,in_srcH,in_src_scan,in_dst_scan,in_flip,in_table
        PUBLIC  C gfx_draw_bm_16

        mov     ebx,in_table            ;remap table
		  shr     ebx,2                   ;so we can index DWORDS as [table * 4]
        mov     esi,in_src_offset       ;get source offset
        mov     edi,in_dst_offset       ;get destination offset

        mov     ecx,in_srcW             ;get width of bitmap
        mov     edx,in_srcH
        test    in_flip,GFX_FLIP_X      ;flip left-to-right?
        jnz     @@main_loopX            ;yes
        
@@main_loop:
        test    edi,3                   ;are we on a DWORD boundary?
        jz      @@dword_check           ;=0 Yes
@@word_write:
        mov     bl,[esi]                ;get a byte
        inc     esi                     ;next source pixel
        mov     eax,[ebx*4]             ;load the 16-bit pix from the table
        mov     [edi], ax               ;store the pixel on the screen
        add     edi,2                   ;next destination pixel
        dec     ecx                     ;decrement remaining count
        jnz     @@main_loop             ;no, do another byte
        jmp     short @@done_row        ;finished, next row
@@dword_check:
        test    ecx,NOT 1               ;yes, but are there any DWORDS to write?
        jz      @@word_write            ;no, finish with bytes
        sub     ecx,2                   ;yes, decrement count by one DWORD's worth
                
@@dword_loop:
        mov     bl,[esi+1]
		  add     esi, 2
        add     edi,4                   ;increment dest pointer
		  mov     eax,[ebx*4]
		  mov     bl,[esi-2]
		  shl     eax,16
		  or      eax,[ebx*4]
        mov     [edi-4], eax
        sub     ecx,2                   ;decrement column count four bytes
        jns     @@dword_loop            ;next two pixels
        add     ecx,2                   ;get number of pixels left over
        jnz     @@word_write            ;if any left, process bytes

@@done_row:
        add     esi,in_src_scan         ;wrap to next source offset
        add     edi,in_dst_scan         ;wrap to next dest offset
        mov     ecx,in_srcW             ;get width of bitmap
        dec     edx                     ;decrement row count
        jnz     @@main_loop             ;process next row
        jmp     @@exit              ;done, exit

;------ flipped left-to-right ----------

@@main_loopX:
        test    edi,3                   ;are we on a DWORD boundary?
        jz      @@dword_checkX          ;=0 Yes
@@word_writeX:
        mov     bl,[esi]                ;get a byte
        dec     esi                     ;next source pixel
        mov     eax,[ebx*4]             ;load the 16-bit pix from the table
        mov     [edi], ax               ;store the pixel on the screen
        add     edi,2                   ;next destination pixel
        dec     ecx                     ;decrement remaining count
        jnz     @@main_loopX            ;no, do another byte
        jmp     short @@done_rowX       ;finished, next row
@@dword_checkX:
        test    ecx,NOT 1               ;yes, but are there any DWORDS to write?
        jz      @@word_writeX           ;no, finish with bytes
        sub     ecx,2                   ;yes, decrement count by one DWORD's worth
                
@@dword_loopX:
        mov     bl,[esi-1]
		  sub     esi, 2
        add     edi,4                   ;increment dest pointer
		  mov     eax,[ebx*4]
		  mov     bl,[esi+2]
		  shl     eax,16
		  or      eax,[ebx*4]
        mov     [edi-4], eax
        sub     ecx,2                   ;decrement column count four bytes
        jns     @@dword_loopX           ;next two pixels
        add     ecx,2                   ;get number of pixels left over
        jnz     @@word_writeX           ;if any left, process bytes

@@done_rowX:
        add     esi,in_src_scan         ;wrap to next source offset
        add     edi,in_dst_scan         ;wrap to next dest offset
        mov     ecx,in_srcW             ;get width of bitmap
        dec     edx                     ;decrement row count
        jnz     @@main_loopX            ;process next row

@@exit:
        ret

gfx_draw_bm_16 ENDP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;±
;±  NAME
;±      gfx_draw_transparent_bm_16
;±      
;±  DESCRIPTION
;±      Copies a block of memory from a sub-region of a source bitmap to
;±      a sub-region of same size in a destination bitmap.
;±      Color 0 is skipped.
;±      
;±  ARGUMENTS
;±      in_dst_offset   - starting address of destination block
;±      in_src_offset   - starting address of source block
;±      in_srcW         - width, in bytes, of source block
;±      in_srcH         - number of rows in source block
;±      in_src_scan     - offset from end of scan to start of next
;±      in_dst_scan     - offset from end of scan to start of next
;±      in_flip         - GFX flip flag
;±      in_table        - color look up table
;±      
;±  RETURNS
;±      nothing
;±      
;±  NOTES
;±      does DWORD writes that are DWORD aligned.
;±      
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

gfx_draw_transparent_bm_16 PROC C \
        USES    ebx ecx edx esi edi,
        ARG     in_dst_offset,in_src_offset,in_srcW,in_srcH,in_src_scan,in_dst_scan,in_flip,in_table
        PUBLIC  C gfx_draw_transparent_bm_16

        mov     esi,in_src_offset       ;get source offset
        mov     edi,in_dst_offset       ;get destination offset
        mov     ebx, in_table           ;color remap table
		  shr     ebx, 2                  ;so we can index dwords by table*4
        mov     ecx,in_srcW             ;get width of bitmap
        mov     edx,1                   ;assume no flip_X
        test    in_flip,GFX_FLIP_X      ;flip left-to-right?
        jz      @@main_loop             ;no
        mov     edx,-1                  ;yes, flip source

@@main_loop:
        test    edi,3                   ;are we on a DWORD boundary?
        jz      @@dword_check           ;=0 Yes
@@word_write:
        mov     bl,[esi]                ;get a byte
        add     esi,edx                 ;next source pixel
        cmp     bl,0                    ;skip color?
        je      @@word_skip             ;yes
		  mov     eax, [ebx*4]            ;do color look up
@@write_word:
        mov     [edi], ax               ;store the word
@@word_skip:
        add     edi,2                   ;next destination pixel
        dec     ecx                     ;decrement remaining count
        jnz     @@main_loop             ;no, do another byte
        jmp     short @@done_row        ;finished, next row
@@dword_check:
        test    ecx,NOT 1               ;yes, but are there any DWORDS to write?
        jz      @@word_write            ;no, finish with bytes
                
@@dword_loop:
        mov     bl,[esi]
        add     esi, edx
        cmp     bl, 0
        jz      @@word_skip
		  mov     eax, [ebx*4]
		  mov     bl, [esi]
		  cmp     bl, 0
		  je      @@write_word
		  shl     eax, 16
        or      eax, [ebx*4]
        add     edi,4                   ;increment dest pointer
		  rol     eax, 16
		  add     esi, edx
		  mov     [edi-4], eax
        sub     ecx,2                   ;decrement column count four bytes
        cmp     ecx,1
        jg      @@dword_loop            ;next four pixels
        je      @@word_write            ;if any left, process bytes

@@done_row:
        add     esi,in_src_scan         ;wrap to next source offset
        add     edi,in_dst_scan         ;wrap to next dest offset
        mov     ecx,in_srcW             ;get width of bitmap
        dec     in_srcH                 ;decrement row count
        jnz     @@main_loop             ;process next row

        ret

gfx_draw_transparent_bm_16 ENDP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;
; RemapDIBBits - remaps a bitmap to a given color table
;
; DibOffset is a pointer to the bitmaps bits.  numBits is the number
; of bits in the bitmap.  RemapTblOffset points to a table containing
; 256 bytes of remap information.  For each element in the Remap table
; there is a value indicating the new color value for a source pixel whose
; color value matches the remap table index.  For example
; 
; DibOffset = 2,8,5,4,4,5...
; RemapTble[0] = 0
; RemapTble[1] = 32
; RemapTble[2] = 1
; RemapTble[3] = 24
; RemapTble[4] = 25
; RemapTble[5] = 26
; RemapTble[6] = 5
; RemapTble[7] = 6
; RemapTble[8] = 7...
; 
; After remapping;
; 
; DibOffset = 1,7,26,25,25,26...
;
; The bitmap bits have been translated to their new values.  This might      
; be used to lighten or darken a bitmap for light source shading.  It might
; also be used to simulate palette tricks on a specific bitmap without
; affecting other bitmaps using the same colors.
;
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

RemapDIBBits PROC C \
        USES    ebx ecx esi edi,
        ARG     pSrc,numBits,RemapTblOffset
        PUBLIC  C RemapDIBBits

        mov     esi,pSrc                ;get source offset
        mov     edi,RemapTblOffset      ;get offset to color remap table

        mov     eax,0                   ;clear dword
        mov     ecx,numBits             ;get number of bits in bitmap
@PixelLoop:
        mov     al,[esi]                ;get pixel
        mov     al,[eax+edi]            ;get remapped color
        mov     [esi],al                ;store remapped color
        inc     esi
        dec     ecx                     ;decrement bit count
        jnz     @PixelLoop              ;next pixel

        ret

RemapDIBBits ENDP


gfx_stretch2_blit PROC C \
        USES    eax ebx ecx edx esi edi,
        ARG     in_dst, in_dstStride, \
                in_src, in_srcStride, \
                in_height
        PUBLIC  C gfx_stretch2_blit

   mov      eax, in_dstStride
   mov      ebx, in_height
   add      ebx, ebx
   imul     eax, ebx
   add      eax, in_dst
   mov      lastDWord, eax

   mov      eax, in_dstStride
   mov      dstStride, eax
   add      eax, in_dst
   mov      lastDWordLine, eax

   mov      esi, in_src

   mov      edi, in_dst
   mov      _ebp, ebp

@@outLoop:
   mov      ebp, edi
   add      ebp, dstStride
   mov      lastDWordLine, ebp

@@loop:
   mov      ax, [esi]
   mov      bx, [esi + 2]

   mov      cl, ah
   mov      dl, bh

   mov      ch, ah
   mov      dh, bh

   shl      ecx, 16
   shl      edx, 16

   mov      cl, al
   mov      dl, bl

   mov      ch, al
   mov      dh, bl

   add      esi, 4

   mov      [edi], ecx
   mov      [ebp], ecx

   mov      [edi + 4], edx
   mov      [ebp + 4], edx

   add      edi, 8
   add      ebp, 8

   cmp      edi, lastDWordLine
   jl       @@loop
   
   mov      edi, ebp
   cmp      ebp, lastDWord
   
   jl       @@outLoop

   mov      ebp, _ebp
   ret

gfx_stretch2_blit ENDP

END