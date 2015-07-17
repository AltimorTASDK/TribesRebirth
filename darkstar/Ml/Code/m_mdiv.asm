;
; $Workfile:   m_mdiv.asm  $
; $Revision:   1.0  $
; $Version$
; $Date:   06 Jun 1995 11:14:24  $
; $Log:   S:\phoenix\ml\vcs\m_mdiv.asv  $
;	
;	   Rev 1.0   06 Jun 1995 11:14:24   CAEDMONI
;	Initial revision.
;	



.486p
.MODEL FLAT, SYSCALL

IFDEF __MASM__
ARG equ 
ENDIF 

.CODE

;
; Int32 m_muldiv( const Int32 numer, const Int32 multp, const Int32 denom )
;
m_muldiv32 PROC C \
        ,
	ARG	numer,multp,denom
	PUBLIC	C m_muldiv32

        mov     eax,numer
        imul    multp
	idiv	denom

	RET
m_muldiv32 ENDP

;
; UInt32 m_muludiv( const Int32 numer, const Int32 multp, const UInt32 denom )
;
m_muludiv32 PROC C \
        ,
        ARG     numer,multp,denom
        PUBLIC  C m_muludiv32

        mov     eax,numer
        mov     edx,0
        mul     multp
        div     denom

        RET
m_muludiv32 ENDP

END

