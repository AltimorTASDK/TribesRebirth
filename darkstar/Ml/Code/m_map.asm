;
; $Workfile:   m_map.asm  $
; $Revision:   1.1  $
; $Version$
; $Date:   08 Mar 1995 08:44:02  $
; $Log:   S:\phoenix\ml\vcs\m_map.asv  $
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
; Int32 m_map ( Int32 dval, Int32 dmin, Int32 dmax, Int32 rmin, Int32 rmax )
; map dval in (dmin,dmax) into (rmin,rmax)
; result = rmin + ((dval-dmin)*(rmax-rmin)+correction)/(dmax-dmin)
; where correction = (rmax >= rmin) ? abs(dmax-dmin)-1 : -(abs(dmax-dmin)-1)
;
m_map32 PROC C \
	USES	ebx esi,
	ARG	dval,dmin,dmax,rmin,rmax
	PUBLIC  C m_map32

	mov	eax,dmax	; ecx = dmax-dmin
	sub	eax,dmin
	mov	ecx,eax

	cdq			; esi:ebx = abs(dmax-dmin) - 1
	xor	eax,edx
	sub	eax,edx
	dec	eax
	cdq
	mov	esi,edx
	mov	ebx,eax

	mov	edx,rmax	; edx = rmax-rmin
	sub	edx,rmin

	jge	pos_range

;	neg	edx		; negate esi:ebx MODIFIED as per Paul Bowman
	neg	ebx
	sbb	esi,0
	
pos_range:
	mov	eax,dval
	sub	eax,dmin
	
	imul	edx

	add	eax,ebx
	adc	edx,esi

	idiv	ecx

	add	eax,rmin
	
	RET
m_map32 ENDP

END


