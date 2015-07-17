; ---------------------------------------------------------------------------

;	$Workfile:   m_fix.asm  $
;	$Version$
;	$Revision:   1.3  $
;	$Date:   14 Dec 1995 19:01:16  $
;	$Log:   R:\darkstar\develop\core\ml\vcs\m_fix.asv  $
;	
;	   Rev 1.3   14 Dec 1995 19:01:16   CAEDMONI
;	converted to fastcall
;	
;	   Rev 1.2   11 Dec 1995 19:00:44   CAEDMONI
;	converted to fastcall
;	
;	   Rev 1.1   08 Mar 1995 08:44:08   RICKO
;	Now compiles with both TASM and MASM
;	
;	   Rev 1.0   07 Mar 1995 13:16:42   NANCYH
;	Initial revision.
;	

; ---------------------------------------------------------------------------

include types.inc

.CODE

; ---------------------------------------------------------------------------
; Divides one fixed-point value by another.
; C near-callable as:
;	Fix16 cdecl m_divff16(Fix16 Dividend, Fix16 Divisor);
;	Fix16 cdecl m_divff16R(Fix16 Dividend, Fix16 Divisor);

BeginFastCallProc       m_divff16, rDividend, rDivisor
        mov     ecx,rDivisor
	mov     edx,rDividend
	sub     eax,eax
	shrd    eax,edx,16      ;position so that result ends up
	sar     edx,16          ; in EAX
	idiv    ecx
EndFastCallProc


m_divff16R PROC C \
	USES	ebx,
	ARG	Dividend,Divisor
        PUBLIC  C m_divff16R

	sub     ecx,ecx         ;assume positive result
	mov     eax,Dividend
	and     eax,eax         ;positive dividend?
	jns     FDP1            ;yes
	inc     ecx             ;mark it's a negative dividend
	neg	eax             ;make the dividend positive
FDP1: 
	sub     edx,edx         ;make it a 64-bit dividend, then shift
                        	; left 16 bits so that result will be
                        	; in EAX
	rol     eax,16          ;put fractional part of dividend in
                        	; high word of EAX
	mov     dx,ax           ;put whole part of dividend in DX
	sub     ax,ax           ;clear low word of EAX
	mov     ebx,DWORD PTR Divisor
	and     ebx,ebx         ;positive divisor?
	jns     FDP2            ;yes
	dec     ecx             ;mark it's a negative divisor
	neg     ebx             ;make divisor positive
FDP2:   
	div     ebx             ;divide
	shr     ebx,1           ;divisor/2, minus 1 if the divisor is
	adc     ebx,0           ; even
	dec     ebx
	cmp     ebx,edx         ;set Carry if the remainder is at least
	adc     eax,0           ; half as large as the divisor, then
	                        ; use that to round up if necessary
	and     ecx,ecx         ;should the result be made negative?
	jz      FDP3            ;no
	neg     eax             ;yes, negate it
FDP3:

	RET
m_divff16R ENDP


; ---------------------------------------------------------------------------
; Multiplies two fixed-point values together.
; C near-callable as:
;       Fix16 m_mulff16(Fix16 M1, Fix16 M2);
;       Fix16 m_mulff16R(Fix16 M1, Fix16 M2);

BeginFastCallProc       m_mulff16, M1, M2
        ;mov	eax,M1          ; M1 is passed in eax
        imul	M2		;multiply
        shrd	eax,edx,16	;put the whole thing back into eax
EndFastCallProc


BeginFastCallProc       m_mulff16R, M1, M2
        ;mov	eax,M1          ; M1 is passed in eax
        imul	M2		;multiply
        add	eax,8000h       ;round by adding 2^(-17)
	adc	edx,0
        shrd	eax,edx,16	;put the whole thing back into eax
EndFastCallProc


; ---------------------------------------------------------------------------
; Multiplies fixed-point value by 32 bit integer
; C near-callable as:
;       Int32 m_mulf16(Int32 M1, Fix16 M2);

BeginFastCallProc       m_mulf16, M1, M2
        ;mov	eax,M1          ; M1 is passed in eax
        imul	M2		;multiply
        shrd	eax,edx,16	;put the whole thing back into eax
EndFastCallProc


BeginFastCallProc       m_mulf16R, M1, M2
        ;mov	eax,M1          ; M1 is passed in eax
        imul	M2		;multiply
        add	eax,8000h       ;round by adding 2^(-17)
	adc	edx,0
        shrd	eax,edx,16	;put the whole thing back into eax
EndFastCallProc



END

