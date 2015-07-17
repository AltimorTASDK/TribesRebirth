;
; $Workfile:   m_bscan.asm  $
; $Revision:   1.2  $
; $Version$
; $Date:   15 Sep 1995 09:59:06  $
; $Log:   R:\darkstar\develop\ml\vcs\m_bscan.asv  $
;	
;	   Rev 1.2   15 Sep 1995 09:59:06   CAEDMONI
;	args/returns changed from UInt32 to Int32
;	
;	   Rev 1.1   08 Mar 1995 08:43:52   RICKO
;	Now compiles with both TASM and MASM
;	
;	   Rev 1.0   07 Mar 1995 13:16:40   NANCYH
;	Initial revision.
;  



.486p
.MODEL FLAT, SYSCALL

IFDEF __MASM__
ARG equ 
ENDIF 

.CODE

;
; Int32 m_bsr ( Int32 val )
;
; val >= 0
;
; return index of highest bit set.  returns -1 if val == 0.
;
m_bsr_	PROC C \
        ,
	ARG	val
	PUBLIC	C m_bsr_

	mov	edx,val		; edx = val
	bsr	eax,edx
	jz	bsrZero
	RET
bsrZero:
	xor	eax,eax
	dec	eax
	RET
m_bsr_ ENDP


;
; Int32 m_bsf ( Int32 val )
;
; val >= 0
;
; return index of lowest bit set.  returns -1 if val == 0.
;
m_bsf_	PROC C \
        ,
	ARG	val
	PUBLIC	C m_bsf_

	mov	edx,val		; edx = val
	bsf	eax,edx
	jz	bsfZero
	RET
bsfZero:
	xor	eax,eax
	dec	eax
	RET
m_bsf_ ENDP

END
