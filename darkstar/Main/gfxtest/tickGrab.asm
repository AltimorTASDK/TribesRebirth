;------------------------------------------------------------------------------
; Description 
;    
; $Workfile$
; $Revision$
; $Author  $
; $Modtime $
;
;------------------------------------------------------------------------------

IFDEF __MASM__
.586p
ELSE
.486p
ENDIF

.MODEL FLAT, SYSCALL
 
IFDEF __MASM__
  ARG equ
ELSE
  RDTSC equ db 0fh, 31h
  LOCALS
ENDIF


 
 ;------------------------------ DATA SEGMENT
.DATA
 
 ;------------------------------ CODE SEGMENT
.CODE
grabTick PROC C
    PUBLIC C grabTick

    push edx
    RDTSC       ; places cycle count in edx:eax, return value is in eax
    pop  edx
    ret
grabTick ENDP
 
END
