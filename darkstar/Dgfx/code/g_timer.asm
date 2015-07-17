; ========================================================
;
;	$Workfile:   g_timer.asm  $
;	$Version$
;
;    Code for the timer - debug stuff
;
;  $Revision:$
;       $Log:$
;	
; (c) Copyright 1995, Dynamix Inc.  All rights reserved.
; ========================================================

.486
.MODEL FLAT, syscall
.DATA

IFDEF __MASM__
ARG equ
ENDIF

TIMER_STACK_SIZE     equ      20
numTimerEntries   dd 0

timerVal    dd    ?
timerStore  dq    ?

timerStack  dq    TIMER_STACK_SIZE     dup (?)


PUBLIC C timerVal
PUBLIC C timerStore

.CODE
INCLUDE g_macros.inc

PUBLIC C gfx_push_time
gfx_push_time PROC C
   mov   eax, numTimerEntries
   lea   ecx, [timerStack+eax*4]
   inc   eax
   mov   numTimerEntries, eax
   cmp   eax, TIMER_STACK_SIZE
   jge   @@end   
   readTime
   mov   [ecx], eax
   mov   [ecx+4], edx
@@end:
   ret
   
gfx_push_time ENDP

PUBLIC C gfx_pop_time
gfx_pop_time PROC C
   readTime
   mov   ecx, numTimerEntries
   dec   ecx
   cmp   ecx, 0
   jl    @@end
   mov   numTimerEntries, ecx
   lea   ecx, [timerStack+ecx*4]
   sub   edx, [ecx+4]
   sbb   eax, [ecx]
@@end:
   ret
gfx_pop_time ENDP
END
