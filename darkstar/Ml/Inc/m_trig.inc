;
; $Workfile:   m_trig.inc  $
; $Revision:   1.3  $
; $Version$
; $Date:   11 Dec 1995 17:17:00  $
; $Log:   R:\darkstar\develop\core\ml\vcs\m_trig.inv  $
;	
;	   Rev 1.3   11 Dec 1995 17:17:00   CAEDMONI
;	fixed point defines actually moved into types.h
;	
;	   Rev 1.2   11 Dec 1995 17:02:30   CAEDMONI
;	moved fixed point constant defines into base.h
;	
;	   Rev 1.1   08 Mar 1995 08:46:54   RICKO
;	updated to compile with TASM and MASM
;	
;	   Rev 1.0   07 Mar 1995 13:16:16   NANCYH
;	Initial revision.
;

include types.inc

;
; Fix16 m_cos( Angle )
;
; Call with angle
; returns cos(angle) in eax
;
; uses eax,edx
;
m_cos	MACRO	angle
      LOCAL	below90,done

      mov   eax,angle
      and   eax,0ffffh  ; get fraction part of angle
      cwde

      sar   eax,4		; 12 bit table
      adc   eax,0       ; account for rounding

      cdq               ; cos(-x) == cos(x), so make positive
      xor   eax,edx
      sub   eax,edx

      cmp   eax,0400h	; in first quadrant, so just do it
      jl    below90

      neg   eax		; cos(x) == -cos(180-x)
      add   eax,0800h	; cos(x) == -cos(180-x)
      movzx eax,costab[eax*2]
      neg   eax
      jmp   short done

below90:
      movzx	eax,costab[eax*2]
done:
      shl   eax,2       ; convert from 14 bit fraction to 16 bit
ENDM

;
; Fix16 m_sin( Angle )
;
; Call with angle in eax
; returns cos(angle) in eax
;
; uses eax,edx
;
m_sin	MACRO	angle
      mov   eax,angle
      sub	eax,04000h
      m_cos	eax
ENDM

