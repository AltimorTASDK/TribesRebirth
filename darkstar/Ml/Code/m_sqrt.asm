;
; $Workfile:   m_sqrt.asm  $
; $Revision:   1.1  $
; $Version$
; $Date:   08 Mar 1995 08:44:04  $
; $Log:   S:\phoenix\ml\vcs\m_sqrt.asv  $
;	
;	   Rev 1.1   08 Mar 1995 08:44:04   RICKO
;	Now compiles with both TASM and MASM
;	
;	   Rev 1.0   07 Mar 1995 13:16:46   NANCYH
;	Initial revision.
;



.486p
.MODEL FLAT, SYSCALL

IFDEF __MASM__
ARG equ 
ENDIF 

.CODE


;Description:
;	
;	Quickly finds the square root of a number using only shifts and adds.
;
;	The basis of the algorithm is the equation:
;
;		(p+p')^2 = p^2 + p'^2 + 2 * p * 2
;
;Algorithm:
;	
;	Goal: Find the largest integer r such that r^2 <= n.
;
;
;	Find the largest integers p and s such that:
;		p^2 <= n and p = 2^s
;
;	Let 
;		current_root = p = 2^s
;		current_square = p*p = 2^(2*s)
;		
;	For all successively smaller pairs of integers p' & s' such that:
;		p = 2^s and p > 0
;	{	
;	Let
;		shifted_root = 2 * p * p'
;			     = p << (s' + 1)
;
;		new_square = current_root + shifted_root + current_square
;
;	If
;		new_square <= n
;
;	Then
;		current_root = current_root + p'
;		current_square = new_square
;	}			

; 
; Register usage:
;	EDX	N
;       ECX     Current root bit position ( for 2^2*k calcs).
;       EBX   	Current root * (2 ^ current power).
;	EAX	current square
;	BP	current root mask
;	ESI	shifted root
;	EDI	new square
;
; NOTE: see the C version for additional 'explanation'
;
;

m_sqrt_ PROC C \
	USES	ebx edi esi,
	ARG	n
	PUBLIC  C m_sqrt_

	mov	edx,n			; EDX = n

	push	ebp

        ;================  Setup for the loop 
        
	sub	ebx,ebx			; handle case n==0
	or	edx,edx
	jz	done

	bsr	ecx,edx			; ECX = curbit
	shr	ecx,1

	inc	ebx			; BX = cur_root = 1 << curbit
	shl	ebx,cl

	jcxz	done			; if n==0,1,2, we're done

	mov	eax,ebx			; EAX = cursq = 1 << (2*curbit)
	shl	eax,cl	

	mov	ebp,ebx			; EBP = cur_mask = cur_root >> 1
	shr	ebp,1

	dec	ecx			; curbit--

shifted_root:
	mov	esi,ebx			; ESI = shifted_root
	shl	esi,cl			;     =	cur_root << curbit+1
	shl	esi,1


new_sq:
	mov	edi,ebp			; EDI = newsq = cursq + shifted_root
	shl	edi,cl			;     + 2^(curbit*2)
	add	edi,eax
	add	edi,esi

	dec	ecx			; curbit--

	cmp	edi,edx			; newsq < n ?
	ja	too_big

	add	ebx,ebp			; cur_root += cur_mask
	shr	ebp,1			; cur_mask >>= 1
	jz	done			; cur_mask == 0 ?

	mov	eax,edi			; cursq = newsq

	jmp	short shifted_root
	
too_big:
	shr	ebp,1			; cur_mask >>= 1
	jz	done			; cur_mask == 0 ?
	shr	esi,1			; shifted_root >>= 1

	jmp	short new_sq

done:
	pop	ebp

	mov	eax,ebx			; return cur_root!

	RET
m_sqrt_	ENDP


END

