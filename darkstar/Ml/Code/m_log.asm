;
; $Workfile:   m_log.asm  $
; $Revision:   1.2  $
; $Version$
; $Date:   15 Sep 1995 09:59:22  $
; $Log:   R:\darkstar\develop\ml\vcs\m_log.asv  $
;	
;	   Rev 1.2   15 Sep 1995 09:59:22   CAEDMONI
;	args/returns changed from UInt32 to Int32
;	
;	   Rev 1.1   08 Mar 1995 08:44:26   RICKO
;	Now compiles with both TASM and MASM
;	
;	   Rev 1.0   07 Mar 1995 13:16:44   NANCYH
;	Initial revision.
;  



.486p
.MODEL FLAT, SYSCALL

IFDEF __MASM__
ARG equ 
ENDIF 

.CODE

;
; Int32 m_log2 ( Int32 l )
;
; l >= 0
;
; return bounding power of two (number of highest bit) in high word.
; In low word, return fractional component.  If l==0, returns 0.
;
m_log2_ PROC C \
        ,
	ARG	l
	PUBLIC  C m_log2_

	mov	edx,l		; edx = l

	bsr	eax,edx		; eax = log2(l)
	jz	zero
	mov	cl,32
	sub	cl,al		; cl = 32 - log2(l)
	shl	eax,16		; put log2 in high word

	shl	edx,cl		; shift up l so fraction is in hi word
	shr	edx,16		; get it into the low word of edx
	mov	ax,dx		; low word=fraction, hi word=log2(l)
	ret
zero:
	xor	eax,eax
	RET
m_log2_ ENDP


END


