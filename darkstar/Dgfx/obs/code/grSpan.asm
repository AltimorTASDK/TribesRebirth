;------------------------------------------------------------------------------
; Description 
;    
; $Workfile$
; $Revision$
; $Author  $
; $Modtime $
;
;------------------------------------------------------------------------------

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
 
gr_create_spans PROC C \
        USES    esi edi eax ebx ecx edx,
        ARG     in_pSpans:DWORD,    \
                in_firstSpan:DWORD,  \
                in_lastSpan:DWORD,   \
                out_pSpans:DWORD
        PUBLIC C gr_create_spans

    mov edi, out_pSpans
    mov esi, in_pSpans
    mov ecx, in_firstSpan
    
@@loopNewBlock:
    xor edx, edx

@@spanLoop:
    mov eax, [esi + ecx*4]
    inc ecx

    mov ebx, eax
    add edi, 8

    shr eax, 11
    and ebx, 07ffh          ; ebx = cnt

    mov [edi - 4], ebx
    mov ebx, eax

    shr eax, 11
    and ebx, 07ffh          ; eax = y, ebx = x

    shl ebx, 16
    inc edx

    or  eax, ebx
    cmp ecx, in_lastSpan    ; last span?

    mov [edi-8], eax
    je  @@loopExit

    cmp edx, 15
    jne @@spanLoop
    
    mov ecx, [esi + ecx*4]
    jmp @@loopNewBlock
@@loopExit:
    RET
gr_create_spans ENDP
 
 
END
