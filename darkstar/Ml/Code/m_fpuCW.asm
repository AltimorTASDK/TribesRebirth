;
; Dgfx software rendering (not hardware rendering) changes the control 
; states of the FPU during its render cycle.  To maximize performance, 
; DGFX software does not set and reset the FPU control state.  Hence, 
; after the execution of 3d rendering through DGFX software, the control
; state of the FPU may not always be consistent.  Hence any routines that
; rely on FPU being in a certain state should use the following functions
; to set and reset the FPU state
;

.486p
.MODEL FLAT, SYSCALL

IFDEF __MASM__
ARG equ 
ENDIF 

.DATA

wData DW 0000h

.CODE

;
; Int16 fpuGetControlState()
;
m_fpuGetControlState PROC C \
   , 
	ARG dummy
   PUBLIC C m_fpuGetControlState

   fstcw wData      ; load the FPU control word into memory wordData
   fwait		    ; wait for the operation to complete
   
   mov   ax, wData  ; store result in the return register AX

   RET
m_fpuGetControlState	ENDP

;
; void fpuSetControlState(Int16 wState)
;
m_fpuSetControlState PROC C \
   ,
   ARG wState : WORD
   PUBLIC C m_fpuSetControlState

   fclex		  ; you HAVE to clear pending interrupts before
				  ; changing FPU exception bits, or any re-set
				  ; exceptions will fire on fldcw

   fldcw wState   ; set the FPU control word
   fwait		  ; wait for the operation to complete

   RET
m_fpuSetControlState ENDP

END


