; ========================================================
;
;	$Workfile:   rn_filln.asm  $
;	$Version$
;
;    Filled span renderers
;
;  $Revision:$
;       $Log:$
;
; (c) Copyright 1995, Dynamix Inc.  All rights reserved.
; ========================================================


.486
.MODEL FLAT, syscall

.data
INCLUDE g_macros.inc
INCLUDE g_contxt.inc

IFDEF __MASM__
ARG equ
ENDIF

yval		dd ?
xval		dd ?

.code

gfx_draw_fill_span_new PROC C
		PUBLIC C gfx_draw_fill_span_new
		; fill the color field with the whole DWORD
		mov		eax, gfxPDC.pdc_color
		mov		ah,al
		mov		ecx,eax
		bswap	eax
		or		eax,ecx
		mov		gfxPDC.pdc_colorsv, eax
		lea		eax, @@drawSpan
		mov		gfxPDC.pdc_rendSpanEntry, eax
		RET
@@drawSpan:
		; edi = xLeft, esi = count, eax = yval.
		mov		edx, esi
		mov		eax, gfxPDC.pdc_colorsv

		; count in edx, start in edi, color in eax
        mov     ebp, gfxPDC.pdc_rowAddr
		lea		edi, [edi + ebp]
		cmp		edx, 3
		jle		@@tail

		mov		ecx, edi
		neg		ecx

		and		ecx, 3		; ecx holds 0-3 head bytes

		sub		edx, ecx
		mov		ebx, edx
		and		edx, 3		; edx holds 0-3 tail bytes
		sub		ebx, edx
		shr		ebx, 2		; ebx holds 0-n center words
        rep stosb           ; write 0-3 head BYTES
        mov     ecx, ebx
        rep stosd			;write 0-n DWORDS
@@tail:
        mov     ecx, edx
        rep stosb			;write 0-3 tail BYTES
		jmp		DWORD PTR [gfxPDC.pdc_rendSpanExit]
gfx_draw_fill_span_new ENDP

gfx_draw_gouraud_span_new PROC C
		PUBLIC C gfx_draw_gouraud_span_new
		lea		eax, @@drawSpan
		fld		gfxPDC.pdc_dsdx
		fistp	gfxPDC.pdc_fss
		mov		gfxPDC.pdc_rendSpanEntry, eax
		ret
@@drawSpan:
		; gotta calc shade value for start of span.

		mov		yval, eax
		mov		xval, edi
		fld		gfxPDC.pdc_dsdx
		fimul	xval
		fld		gfxPDC.pdc_dsdy
		fimul	yval
		fxch	st(1)
		fadd	gfxPDC.pdc_s0
		faddp	st(1), st
		fistp	gfxPDC.pdc_fs

		mov		eax, gfxPDC.pdc_fs
		mov		ecx, gfxPDC.pdc_shadeMap
		rol		eax, 16
		mov		edx, gfxPDC.pdc_fss
		rol		edx, 16
		mov		cl, BYTE PTR [gfxPDC.pdc_color]
		add		edi, gfxPDC.pdc_rowAddr
		or		ch, al
		add		esi, edi
		;regs: eax - s.f
		;      ebx - pixels
		;      ecx - shademap s.w color
		;      edx - ss.f    ss.w
		;      edi - dstPtr

		mov		ebp, esi
		sub		esi, 1
		sub		ebp, 4
		and		ebp, NOT 3

		;      esi - dstLast
		;      ebp - dstLastDword

		cmp		edi, ebp
		jg		@@lastPixels
		test	edi, 3
		jz		@@dwordLoop
@@firstPixels:
		inc		edi
		mov		bl, [ecx]
		add		eax, edx
		adc		ch, dl
		mov		[edi-1], bl
		test	edi, 3
		jnz		@@firstPixels
		cmp		edi, ebp
		jg		@@lastPixels
@@dwordLoop:
		mov		bl, [ecx]
		add		eax, edx
		adc		ch, dl
		add		edi, 4
		mov		bh, [ecx]
		add		eax, edx
		adc		ch, dl
		bswap	ebx
		mov		bh, [ecx]
		add		eax, edx
		adc		ch, dl
		mov		bl, [ecx]
		add		eax, edx
		adc		ch, dl
		bswap	ebx
		mov		[edi-4], ebx
		cmp		edi, ebp
		jle		@@dwordLoop
@@lastPixels:
		cmp		edi, esi
		jg		@@nextSpan
@@lastPixels2:
		inc		edi
		mov		bl, [ecx]
		add		eax, edx
		adc		ch, dl
		mov		[edi-1], bl
		cmp		edi, esi
		jle		@@lastPixels2
@@nextSpan:
		jmp		[gfxPDC.pdc_rendSpanExit]
gfx_draw_gouraud_span_new ENDP

gfx_draw_translucent_span PROC C
		PUBLIC C gfx_draw_translucent_span
		lea		eax, @@drawSpan
		mov		gfxRC.rc_renderSpanHook, eax
		ret
@@drawSpan:
		; Fixed point xLeft in edi, xRight in esi...
		shr		esi, 16
		shr		edi, 16
		sub		esi, edi
		xor		ecx, ecx
		mov		esp, gfxRC.rc_translucencyMap
		mov		ch, BYTE PTR [gfxRC.rc_color]
		add		edi, gfxRC.rc_rowAddr
		add		esi, edi
		mov		eax, ecx
		mov		edx, ecx
		;regs: eax, edx, ecx - transmap
		;      ebx - pixels
		;      ecx - transmap col 0
		;      edi - dstPtr

		mov		ebp, esi
		sub		esi, 1
		sub		ebp, 4
		and		ebp, NOT 3

		;      esi - dstLast
		;      ebp - dstLastDword

		cmp		edi, ebp
		jg		@@lastPixels
		test	edi, 3
		jz		@@dwordLoop
@@firstPixels:
		mov		cl, [edi]
		inc		edi
		mov		bl, [ecx+esp]
		mov		[edi-1], bl
		test	edi, 3
		jnz		@@firstPixels
		cmp		edi, ebp
		jg		@@lastPixels
@@dwordLoop:
		mov		cl, [edi]
		mov		al, [edi+1]
		mov		dl, [edi+2]
		add		edi, 4
		mov		bl, [ecx+esp]
		mov		cl, [edi-1]
		mov		bh, [eax+esp]
		bswap	ebx
		mov		bh, [edx+esp]
		mov		bl, [ecx+esp]
		bswap	ebx
		mov		[edi-4], ebx
		cmp		edi, ebp
		jle		@@dwordLoop
@@lastPixels:
		cmp		edi, esi
		jg		@@nextSpan
@@lastPixels2:
		mov		cl, [edi]
		inc		edi
		mov		bl, [ecx+esp]
		mov		[edi-1], bl
		cmp		edi, esi
		jle		@@lastPixels2
@@nextSpan:
		jmp		[gfxRC.rc_nextEdgeHook]
gfx_draw_translucent_span ENDP

gfx_draw_gouraud_span PROC C
		PUBLIC C gfx_draw_gouraud_span
		lea		eax, @@drawSpan
		mov		gfxRC.rc_renderSpanHook, eax
		ret
@@drawSpan:
		; Fixed point xLeft in edi, xRight in esi...
		shr		esi, 16
		mov		eax, edi
		shr		edi, 16
		sub		esi, edi
		sub		eax, gfxSpan.sp_x ; forward step value
		imul	gfxSpan.sp_s_s
		shrd	eax, edx, 16
		add		eax, gfxSpan.sp_s
		mov		ecx, gfxRC.rc_shadeMap
		rol		eax, 16
		mov		edx, gfxSpan.sp_s_s
		rol		edx, 16
		mov		cl, BYTE PTR [gfxRC.rc_color]
		add		edi, gfxRC.rc_rowAddr
		or		ch, al
		add		esi, edi
		;regs: eax - s.f
		;      ebx - pixels
		;      ecx - shademap s.w color
		;      edx - ss.f    ss.w
		;      edi - dstPtr

		mov		ebp, esi
		sub		esi, 1
		sub		ebp, 4
		and		ebp, NOT 3

		;      esi - dstLast
		;      ebp - dstLastDword

		cmp		edi, ebp
		jg		@@lastPixels
		test	edi, 3
		jz		@@dwordLoop
@@firstPixels:
		inc		edi
		mov		bl, [ecx]
		add		eax, edx
		adc		ch, dl
		mov		[edi-1], bl
		test	edi, 3
		jnz		@@firstPixels
		cmp		edi, ebp
		jg		@@lastPixels
@@dwordLoop:
		mov		bl, [ecx]
		add		eax, edx
		adc		ch, dl
		add		edi, 4
		mov		bh, [ecx]
		add		eax, edx
		adc		ch, dl
		bswap	ebx
		mov		bh, [ecx]
		add		eax, edx
		adc		ch, dl
		mov		bl, [ecx]
		add		eax, edx
		adc		ch, dl
		bswap	ebx
		mov		[edi-4], ebx
		cmp		edi, ebp
		jle		@@dwordLoop
@@lastPixels:
		cmp		edi, esi
		jg		@@nextSpan
@@lastPixels2:
		inc		edi
		mov		bl, [ecx]
		add		eax, edx
		adc		ch, dl
		mov		[edi-1], bl
		cmp		edi, esi
		jle		@@lastPixels2
@@nextSpan:
		jmp		[gfxRC.rc_nextEdgeHook]
gfx_draw_gouraud_span ENDP

gfx_draw_fill_span PROC C
		PUBLIC C gfx_draw_fill_span
		; fill the color field with the whole DWORD
		mov		eax, gfxRC.rc_color
		mov		ah,al
		mov		ecx,eax
		bswap	eax
		or		eax,ecx
		mov		gfxRC.rc_color, eax
		lea		eax, @@drawSpan
		mov		gfxRC.rc_renderSpanHook, eax
		RET
@@drawSpan:
		; Fixed point xLeft in edi, xRight in esi...
		shr		esi, 16
		shr		edi, 16
		mov		edx, esi
		mov		eax, gfxRC.rc_color
		sub		edx, edi

		; count in edx, start in edi, color in eax
        mov     ebp, gfxRC.rc_rowAddr
		lea		edi, [edi + ebp]
		cmp		edx, 3
		jle		@@tail

		mov		ecx, edi
		neg		ecx

		and		ecx, 3		; ecx holds 0-3 head bytes

		sub		edx, ecx
		mov		ebx, edx
		and		edx, 3		; edx holds 0-3 tail bytes
		sub		ebx, edx
		shr		ebx, 2		; ebx holds 0-n center words
        rep stosb           ; write 0-3 head BYTES
        mov     ecx, ebx
        rep stosd			;write 0-n DWORDS
@@tail:
        mov     ecx, edx
        rep stosb			;write 0-3 tail BYTES
		jmp		DWORD PTR [gfxRC.rc_nextEdgeHook]
gfx_draw_fill_span ENDP

gfx_draw_haze_span PROC C
		PUBLIC C gfx_draw_haze_span
		lea		eax, @@drawSpan
		mov		gfxRC.rc_renderSpanHook, eax
		RET
@@drawSpan:
		; forward step h by difference between xLeft and span.x
		; edi has xLeft, esi has xRight

		mov		eax, edi
		mov		ebx, gfxSpan.sp_sh
		sub		eax, gfxSpan.sp_x
		cmp		ebx, 0
		je		@@noMess
		jl		@@incrHaze
		sub		ebx, 2
@@incrHaze:
		add		ebx, 1
@@noMess:
		
		; ebx holds step h value, eax holds intForwardStep
		imul	ebx
		shl		edx, 16
		shr		eax, 16
		or		eax, edx
		add		eax, gfxSpan.sp_h
		; eax holds start haze, ebx holds step haze
		mov		ebp, gfxRC.rc_hazeMask

		shr		edi, 16
		shr		esi, 16
		add		edi, gfxRC.rc_rowAddr
		add		esi, gfxRC.rc_rowAddr

		dec		esi

		; eax - haze level
		; ebx - step haze
		; ebp - hazeMask
		; esp - color
		; edi - dstPtr
		; esi - dstLast

		mov		gfxRC.rc_dstLast, esi
		sub		esi, 4
		and		esi, NOT 3
		mov		esp, esi
		mov		esi, gfxRC.rc_color
		and		esi, 0FFh
		or		esi, gfxRC.rc_shadeMap
		; esi holds dstLastDword

		cmp		edi, esp
		jg		@@hazeTail

        test	edi, 3
		jz		@@dwordWrites
@@firstBytes:
		mov		edx, eax
		add		eax, ebx
		and		edx, ebp
		add		edi, 1
		mov		cl, [edx + esi]
		test	edi, 3
		mov		[edi-1], cl
		jnz		@@firstBytes
@@dwordWrites:
		mov		edx, eax
		add		eax, ebx
		and		edx, ebp
		add		edi, 4
		add		eax, ebx
		mov		cl, [edx + esi]
		mov		ch, cl
		mov		edx, eax
		add		eax, ebx
		and		edx, ebp
		add		eax, ebx
		bswap	ecx
		mov		cl, [edx + esi]
		mov		ch, cl
		cmp		edi, esp
		bswap	ecx
		mov		[edi-4], ecx
		jna		@@dwordWrites
@@hazeTail:
		cmp		edi, gfxRC.rc_dstLast
		ja		@@done
@@almostDone:
		mov		edx, eax
		add		eax, ebx
		and		edx, ebp
		add		edi, 1
		mov		cl, [edx + esi]
		cmp		edi, gfxRC.rc_dstLast
		mov		[edi-1], cl
		jna		@@almostDone
@@done:
		jmp		DWORD PTR [gfxRC.rc_nextEdgeHook]
gfx_draw_haze_span ENDP
END
