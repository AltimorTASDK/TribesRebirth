.486
.MODEL FLAT, syscall

.DATA

INCLUDE g_macros.inc

rowCount         dd ?
colCount         dd ?
decMapStride     dd ?
gctWritable      dd ?
mapStridex2      dd ?
lightLeft        dd ?
lightRight       dd ?
llStep           dd ?
lrStep           dd ?
dstLastPtrSquare dd ?
dstLastPtr       dd ?
bitmapStrideAdd  dd ?
cacheStrideAdd   dd ?
patchedShadeShift equ 10h


NumNoiseEntries     equ 2047

_sg_randomNoise      dd NumNoiseEntries dup(0)
_g_currNoiseIndex    dd 0

PUBLIC _sg_randomNoise
PUBLIC _g_currNoiseIndex

.CODE

IFDEF __MASM__
ARG equ
ENDIF

gfx_cache_texture PROC C,
    ARG srcPtr, bitsPtr, lightPtr, squareSize, squareShift, \
    mapStride, bitmapStride, cacheStride, shadeMap, shadeShift
    PUBLIC C gfx_cache_texture
    mark_writable gctWritable
@@start_proc:
    push    ebx
    push    edi
    push    esi
    mov     colCount, 0
    mov     eax, mapStride
    dec     eax
    mov     decMapStride, eax

    mov     eax, bitmapStride
    sub     eax, squareSize
    mov     bitmapStrideAdd, eax    ; what we add to the ptr after
                                    ; each scan in a square
    
    mov     eax, cacheStride
    sub     eax, squareSize
    mov     cacheStrideAdd, eax     ; what we add to the ptr after
                                    ; each scan in a square (for cache pointer)
                                    
    mov     eax, shadeShift
    mov     BYTE PTR [@@shadeShiftPatch1 - 1], al

    mov     eax, mapStride
    shl     eax, 1
    mov     mapStridex2, eax
@@columnLoop:
    mov     eax, colCount
    cmp     eax, decMapStride
    jge     @@done
    inc     eax
    mov     colCount, eax
    mov     rowCount, 0
@@rowLoop:
    mov     eax, rowCount
    cmp     eax, decMapStride
    jge     @@rowDone

    inc     eax
    mov     rowCount, eax
    mov     ecx, squareShift
    
    ; register usage for inner loop:
    ; eax - lightLeft
    ; ebx - lightRight
    ; ecx - squareShift
    ; edx - llStep
    ; edi - lrStep
    ; esi - light map ptr.

    mov     esi, lightPtr               ; esi gets pointer into the light map.
    xor     eax, eax                    ; clear out lightLeft
    xor     ebx, ebx                    ; and lightRight
    mov     ax, WORD PTR [esi]          ; eax is lightLeft
    mov     bx, WORD PTR [esi+2]        ; ebx is lightRight
    add     esi, mapStridex2            ; add to get the next map coords.
    and     eax, 0F000h                 ; mask off all but the alpha
    and     ebx, 0F000h                 ; same for right

    mov     edx, eax                    ; prepare to fill lower nibbles of
    shl     eax, 4
    mov     edi, ebx                    ; the numbers
    shl     ebx, 4
    or      eax, edx
    shl     eax, 4
    or      ebx, edi
    shl     ebx, 4
    or      eax, edx
    shl     eax, 4
    or      ebx, edi
    shl     ebx, 4
    or      eax, edx
    shl     eax, 4
    or      ebx, edi
    shl     ebx, 4                      ; now eax, ebx are 0xXXXX0000

    mov     ax, WORD PTR [esi]          ; lower left corner of light square
    mov     bx, WORD PTR [esi+2]        ; lower right
    sub     esi, mapStridex2            ; we need that back where it was
    add     esi, 2                      ; but +2 for the next light square

    shr     ax, 12                      ; mask of the non-alpha
    shr     bx, 12                      ; for doing mono-chromatic lighting

    mov     edx, eax
    mov     edi, ebx
    shl     edx, 4
    shl     edi, 4
    or      edx, eax
    shl     edx, 4
    or      edi, ebx
    shl     edi, 4
    or      edx, eax
    shl     edx, 4
    or      edi, ebx
    shl     edi, 4
    or      edx, eax
    shr     eax, 16
    or      edi, ebx
    shr     ebx, 16
    and     edx, 0FFFFh
    and     edi, 0FFFFh

    sub     edx, eax                    ; calculate the delta (bottom - top)
    sub     edi, ebx
    sar     edx, cl                     ; and shift by squareShift to calc
    mov     lightLeft, eax              ; write out lightLeft
    mov     lightRight, ebx             ; and lightRight
    sar     edi, cl                     ; per row step
    cmp      edx, 0
   jge      @@llStepgz
   inc      edx
@@llStepgz:
   cmp      edi, 0
   jge      @@saveSteps
   inc      edi
@@saveSteps:
    mov     llStep, edx                 ; write out the light step values
    mov     lrStep, edi

    mov     lightPtr, esi               ; write the light pointer back out.

    ; Now we have to prepare for the inner loop...
    ; lightLeft and lightRight are in eax, ebx respectively
    ; and squareShift is in ecx.
            
    ; first calculate dstLastWord for the entire square
    ; equal to stride << squareShift + bmpPtr - bitmapStrideAdd
    mov     edx, cacheStride
    mov     edi, bitsPtr                ; edi is the dstPtr into the cache
    shl     edx, cl
    mov     esi, srcPtr                 ; esi is the source texture ptr.
    add     edx, edi
    sub     edx, cacheStrideAdd
    mov     dstLastPtrSquare, edx       ; to terminate outer loop.
    mov     edx, shadeMap

@@squareLoop:
    sub     ebx, eax                    ; lightVal in eax,
    sar     ebx, cl                     ; lightStep in ebx
    mov     ecx, squareSize             ; ecx holds dstLast + 1
    add     ecx, edi
    mov     dstLastPtr, ecx
    cmp     ebx, 0
    jge     @@pixelLoop
    inc     ebx
@@pixelLoop:
    ; ecx is our scratch register.
    mov     ecx, _g_currNoiseIndex
    inc     ecx
    cmp     ecx, NumNoiseEntries
    jl      @@skipReset
    xor     ecx, ecx
@@skipReset:
    mov     _g_currNoiseIndex, ecx
    mov     ecx, DWORD PTR [_sg_randomNoise + ecx*4]
    add     ecx, eax                    ; put lightVal in scratch
    cmp     ecx, 0ffffh
    jnge    @@skipUpClamp
    mov     ecx, 0ffffh
    jmp     @@endClamp
@@skipUpClamp:
    cmp     ecx, 0
    jge     @@endClamp
    mov     ecx, 0
@@endClamp:
    add     eax, ebx                    ; lightVal += lightStep
    sar     ecx, patchedShadeShift      ; lightVal >> shadeShift
            @@shadeShiftPatch1:
    mov     dl, [esi]                   ; dl = *srcPtr
    mov     cl, 0                       ; clear out low byte of shade index
    inc     edi                         ; dstPtr++
    inc     esi                         ; srcPtr++
    cmp     edi, dstLastPtr             ; compare dstPtr to dstLast
    mov     cl, [edx + ecx]             ; read from shadeMap
    mov     [edi-1], cl                 ; write shaded pixel.
    jl      @@pixelLoop                 ; jump if still looping.
    

    mov     eax, lightLeft              ; increment left and right
    mov     ebx, lightRight             ; light values and place them
    add     eax, llStep                 ; back in registers.
    add     ebx, lrStep
    mov     lightLeft, eax
    mov     lightRight, ebx
    mov     ecx, squareShift

    add     edi, cacheStrideAdd         ; add to the dstPtr
    add     esi, bitmapStrideAdd        ; and srcPtr to get to the next row.    

    cmp     edi, dstLastPtrSquare       ; check if we finished the square
    jl      @@squareLoop                ; if not, loop back

    mov     eax, squareSize             ; increment the source and
    add     bitsPtr, eax                ; destination pointers by
    add     srcPtr, eax                 ; one square for next square in row
        
    jmp     @@rowLoop
@@rowDone:
    add     lightPtr, 2                 ; increment the light pointer
    mov     eax, bitmapStride           ; we have to add to get
    mov     ebx, eax                    ; to the next row in the
    shl     eax, cl                     ; src and dest bitmaps.
    sub     eax, ebx
    mov     ebx, cacheStride
    add     srcPtr, eax
    mov     eax, ebx                    ; get the cache stride add...
    shl     eax, cl
    sub     eax, ebx
    add     bitsPtr, eax
    jmp     @@columnLoop
@@done:
    pop    esi
    pop    edi
    pop    ebx
    ret
@@end_proc:
gfx_cache_texture ENDP

END
