
IFDEF __MASM__
IFNDEF timerVal
  EXTERN C timerVal:DWORD
  EXTERN C timerStore:QWORD
ENDIF
  markFunctionWritable    proto C, startOfFunc:DWORD, fLength:DWORD
ELSE
  LOCALS
  extrn C markFunctionWritable:PROC
IFNDEF timerVal
  extrn C timerVal:DWORD
  extrn C timerStore:QWORD
ENDIF
  invoke equ call
ENDIF

; mark_writable
; marks an asm function as writable.  uses eax, ecx, edx.
; you must define the symbols @@start_proc and @@end_proc
; which are the boundaries of the writable area.
 
mark_writable MACRO marked_writable_var
        mov    eax, &marked_writable_var
        cmp    eax, 1
        je     @@start_proc
        mov    ecx, @@end_proc - @@start_proc
        lea    edx, [@@start_proc]
        invoke markFunctionWritable, edx, ecx
        mov    &marked_writable_var, 1
ENDM

; start_timer
; starts the tick counter
; when stop_timer is invoked, the difference is placed in the
; external variable "timerVal"
; uses (trashes) eax, ecx, edx

readTime MACRO
        db 0Fh, 31h
ENDM

IFDEF TIMER
start_timer MACRO
        lea    ecx, timerStore
        readTime
        mov    [ecx], eax
        mov    [ecx+4], edx
ENDM

stop_timer MACRO
        readTime
        lea    ecx, timerStore
        sub    edx, [ecx+4]
        sbb    eax, [ecx]
        mov    timerVal, eax
ENDM

ELSE
start_timer EQU
stop_timer EQU
ENDIF
