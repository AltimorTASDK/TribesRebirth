.486p
.MODEL FLAT, SYSCALL

.data
m_stdval dd 00000000h
roundnormalCtrlWord	   dw	0f7fh
 
.code
__ftol PROC C
	PUBLIC __ftol
	fldcw WORD PTR roundnormalCtrlWord
	fwait
	fistp m_stdval
	mov eax, m_stdval
	RET
__ftol ENDP

_stricmp	segment virtual
	align	2
@_stricmp	proc	near
	push      ebp
	mov       ebp,esp
	push      ebx
	push      esi

	mov	 esi, dword ptr [ebp+8]
	mov	 edx, dword ptr [ebp+12]
	xor	 eax, eax
	xor	 ebx, ebx
@@loop:
	mov	 al, [esi]
	mov	 bl, [edx]
	cmp	 al, 'a'
	jl        short @@t1
	cmp	 al, 'z'
	jg        short @@t1
	sub	 al, ('a' - 'A')
@@t1:
	cmp	 bl, 'a'
	jl        short @@t2
	cmp	 bl, 'z'
	jg        short @@t2
	sub	 bl, ('a' - 'A')
@@t2:
	sub	 eax, ebx
	jne       short @@done
	cmp	 ebx, 0
	je        short @@done
	inc	 esi
	inc	 edx
	jmp       @@loop
@@done:
	pop       esi
	pop       ebx
	pop       ebp
	ret 
@_stricmp	endp
_stricmp	ends
end