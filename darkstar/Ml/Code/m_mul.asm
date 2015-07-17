;
; $Workfile:   m_mul.asm  $
; $Revision:   1.1  $
; $Version$
; $Date:   08 Mar 1995 08:44:02  $
; $Log:   S:\phoenix\ml\vcs\m_mul.asv  $
;	
;	   Rev 1.1   08 Mar 1995 08:44:02   RICKO
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
; Int64 m_mul( const Int32, const Int32 )
; return the result of the multiplication
;
m_mul32 PROC C \
        ,
	ARG	result,a,b
	PUBLIC	C m_mul32

	mov	ecx,result
	mov	eax,a
	imul	b
	mov	[ecx],eax
	mov	[ecx+4],edx
	mov	eax,ecx

	RET
m_mul32 ENDP

END


