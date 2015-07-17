;
; $Workfile:   m_div.asm  $
; $Revision:   1.2  $
; $Version$
; $Date:   06 Jun 1995 11:17:18  $
; $Log:   S:\phoenix\ml\vcs\m_div.asv  $
;	
;	   Rev 1.2   06 Jun 1995 11:17:18   CAEDMONI
;	fixed register overwrite bug
;	
;	   Rev 1.1   08 Mar 1995 08:43:56   RICKO
;	Now compiles with both TASM and MASM
;	
;	   Rev 1.0   07 Mar 1995 13:16:42   NANCYH
;	Initial revision.
;  



.486p
.MODEL FLAT, SYSCALL

IFDEF __MASM__
ARG equ 
ENDIF 

.CODE

;
; div_t32 m_div( const Int64 & numer, const Int32 denom )
; divide numer by denom and return quotient and remainder.
;
m_div32 PROC C \
        ,
	ARG	result,numer,denom
	PUBLIC	C m_div32

	mov	ecx,numer
	mov	eax,[ecx]
	mov	edx,[ecx+4]
	idiv	denom
	mov	ecx,result
	mov	[ecx],eax
	mov	[ecx+4],edx
	mov	eax,ecx

	RET
m_div32 ENDP

END

