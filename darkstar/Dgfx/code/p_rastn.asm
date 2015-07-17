;================================================================
;
; $Workfile:   p_rastn.asm  $
; $Version$
; $Revision:   1.0  $
;
; DESCRIPTION:
;
; (c) Copyright Dynamix Inc.   All rights reserved.
;================================================================

.486p
.MODEL FLAT, SYSCALL
INCLUDE g_macros.inc

IFDEF __MASM__
   ARG equ
ENDIF
 ;------------------------------ DATA SEGMENT
.DATA

INCLUDE g_contxt.inc
INCLUDE g_raster.inc

fpConstant65536 dq 65536.0
fpConstant1div65536   dq 1.52587890625e-5
_esp				dd ?
_ebp				dd ?
controlWord			dd 0F7FH	; round toward zero

.CODE

gfx_calc_int_poly_delta	PROC C \
		USES ebx,
		ARG divVal, stepVal, curVal, nextVal, dstDeltaPtr, dstValuePtr
		public C gfx_calc_int_poly_delta

		fldcw	WORD PTR controlWord
		mov		eax, nextVal
		mov		ebx, curVal
		sub		eax, ebx
		mov		ecx, dstDeltaPtr
		mov		nextVal, eax
		mov		edx, dstValuePtr
		fld		DWORD PTR stepVal
		fld		DWORD PTR divVal
		fimul	nextVal
		fist	DWORD PTR [ecx]
		fmulp	st(1), st
		fistp	nextVal
		add		ebx, nextVal
		mov		DWORD PTR [edx], ebx
		ret
gfx_calc_int_poly_delta ENDP

gfx_calc_float_poly_delta PROC C \
		USES ebx,
		ARG divVal, stepVal, curVal, nextVal, dstDeltaPtr, dstValuePtr
		public C gfx_calc_float_poly_delta

		fldcw	WORD PTR controlWord
		mov		eax, dstDeltaPtr
		mov		ebx, dstValuePtr
		fld		DWORD PTR curVal
		fld		DWORD PTR nextVal
		fsub	st, st(1)
		; st = next-cur, cur
		fmul	DWORD PTR divVal
		fst		DWORD PTR [eax]
		fmul	DWORD PTR stepVal
		faddp	st(1), st
		fstp	DWORD PTR [ebx]
		ret
gfx_calc_float_poly_delta ENDP

;===============================================================
;
; Rasterizer Setup MACRO
;
; Setup code for new poly rasterizers, sets up row address,
; span delta calculator and edge list.
;
;===============================================================

PrimitiveDone MACRO doPop
@@polyDone:
		mov		eax, gfxPRC.prc_destY
ifidni <doPop>, <popStack>
		fstp	st
endif
		mov		gfxPRC.prc_currentY, eax
		jmp		@@endLoop
ENDM

RasterizerSetup MACRO hasSteps
		; set up rowAddr
		mov		eax, gfxRC.rc_stride
		mov		ecx, gfxPRC.prc_currentY
		imul	ecx
		add		eax, gfxRC.rc_rowAddr
		lea		edx, @@polyDone
		mov		gfxRC.rc_rowAddr, eax
		mov		gfxRC.rc_primitiveDoneHook, edx

		; check for span deltas
IFIDNI <hasSteps>, <NoSteps>
		; no steps... clipper should call renderer directly
		mov		eax, gfxRC.rc_renderSpanHook
ELSE
		; else has steps, and clipper should call this after first span
		lea		eax, @@calcSpanSteps
ENDIF
		mov		gfxRC.rc_spanClippedHook, eax

        ; patch in the rasterizer function hooks
        lea		eax, @@rasterLoop
		mov		edx, gfxRC.rc_nextEdgeHook
		mov		gfxRC.rc_rasterLoop, eax
		lea		eax, @@nextSpan
		mov		gfxRC.rc_spanDoneHook, eax

		test	edx, edx
		jnz		@@hasEdgeClip
		mov		gfxRC.rc_nextEdgeHook, eax  ; no next edge clipper
		ret
@@hasEdgeClip:
		; set up edge list
		mov		edx, gfxRC.rc_edgeList
		mov		ecx, gfxPRC.prc_currentY
		mov		eax, (EDGE_LIST PTR [edx]).e_list
		sub		ecx, (EDGE_LIST PTR [edx]).e_top_y
		lea		eax, [eax+ecx*4]
		mov		gfxRC.rc_currentEdgeLine, eax
		ret
ENDM

;===============================================================
;
; TRastX
;
; Triangle flat-filled poly rasterizer.  No texture or shading
; calculations.
;
;===============================================================

TRastX proc C
        public C TRastX
		RasterizerSetup NoSteps
@@rasterLoop:
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp

		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		esi, gfxPRC.prc_currentY

@@startLoop:
		; check done

		cmp		esi, gfxPRC.prc_destY
        je      @@endLoop
		mov		gfxSpan.sp_y, esi

        ; cnt = xRight - xLeft
		; x = xLeft

		sub		eax, ebx
		mov		gfxSpan.sp_x, ebx
		mov		gfxSpan.sp_cnt, eax

        ; jump into clipper

        jmp		DWORD PTR [gfxRC.rc_clipSpanHook]
@@nextSpan:
		; add deltas for left and right x
		mov		esi, gfxPRC.prc_currentY
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		add		esi, 1
		add		eax, gfxPRC.prc_deltaXR
		add		ebx, gfxPRC.prc_deltaXL
		mov		gfxPRC.prc_xRight, eax
		mov		gfxPRC.prc_xLeft, ebx
		mov		edx, gfxRC.rc_stride
		mov		gfxPRC.prc_currentY, esi

		; add offset for next row and edge line

		add		gfxRC.rc_rowAddr, edx
		add		gfxRC.rc_currentEdgeLine, 4
		jmp		@@startLoop
@@endLoop:
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
		PrimitiveDone
TRastX ENDP

;===============================================================
;
; TRastXS
;
; Triangle flat-filled poly rasterizer.  No texture or shading
; calculations.
;
;===============================================================

TRastXS proc C
        public C TRastXS
		RasterizerSetup NoSteps
@@rasterLoop:
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp

		mov		ecx, gfxPRC.prc_iLeft
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		esi, gfxPRC.prc_currentY
		mov		gfxSpan.sp_s, ecx
@@startLoop:
		; check done

		cmp		esi, gfxPRC.prc_destY
        je      @@endLoop
		mov		gfxSpan.sp_y, esi

        ; cnt = xRight - xLeft
		; x = xLeft

		sub		eax, ebx
		mov		gfxSpan.sp_x, ebx
		mov		gfxSpan.sp_cnt, eax

        ; jump into clipper

        jmp		DWORD PTR [gfxRC.rc_clipSpanHook]
@@nextSpan:
		; add deltas for left and right x
		mov		esi, gfxPRC.prc_currentY
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		add		esi, 1
		add		eax, gfxPRC.prc_deltaXR
		add		ebx, gfxPRC.prc_deltaXL
		mov		ecx, gfxPRC.prc_deltaIL
		mov		gfxPRC.prc_xRight, eax
		mov		gfxPRC.prc_xLeft, ebx
		mov		edx, gfxRC.rc_stride
		mov		gfxPRC.prc_currentY, esi
		add		gfxSpan.sp_s, ecx

		; add offset for next row and edge line

		add		gfxRC.rc_rowAddr, edx
		add		gfxRC.rc_currentEdgeLine, 4
		jmp		@@startLoop
@@endLoop:
		mov		ecx, gfxSpan.sp_s
		mov		esp, _esp
		mov		gfxPRC.prc_iLeft, ecx
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
		PrimitiveDone
TRastXS ENDP

;===============================================================
;
; TRastXRGB
;
; Triangle flat-filled poly rasterizer.  No texture or shading
; calculations.
;
;===============================================================

TRastXRGB proc C
        public C TRastXRGB
		RasterizerSetup NoSteps
@@rasterLoop:
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp

		mov		ecx, gfxPRC.prc_rLeft
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		esi, gfxPRC.prc_currentY
		mov		gfxSpan.sp_r, ecx
		mov		ecx, gfxPRC.prc_gLeft
		mov		edx, gfxPRC.prc_bLeft
		mov		gfxSpan.sp_g, ecx
		mov		gfxSpan.sp_b, edx
@@startLoop:
		; check done

		cmp		esi, gfxPRC.prc_destY
        je      @@endLoop
		mov		gfxSpan.sp_y, esi

        ; cnt = xRight - xLeft
		; x = xLeft

		sub		eax, ebx
		mov		gfxSpan.sp_x, ebx
		mov		gfxSpan.sp_cnt, eax

        ; jump into clipper

        jmp		DWORD PTR [gfxRC.rc_clipSpanHook]
@@nextSpan:
		; add deltas for left and right x
		mov		esi, gfxPRC.prc_currentY
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		add		esi, 1
		add		eax, gfxPRC.prc_deltaXR
		add		ebx, gfxPRC.prc_deltaXL
		mov		ecx, gfxPRC.prc_deltaRL
		mov		edx, gfxPRC.prc_deltaGL
		mov		edi, gfxPRC.prc_deltaBL
		add		gfxSpan.sp_r, ecx
		add		gfxSpan.sp_g, edx
		add		gfxSpan.sp_b, edi
		mov		gfxPRC.prc_xRight, eax
		mov		gfxPRC.prc_xLeft, ebx
		mov		edx, gfxRC.rc_stride
		mov		gfxPRC.prc_currentY, esi

		; add offset for next row and edge line

		add		gfxRC.rc_rowAddr, edx
		add		gfxRC.rc_currentEdgeLine, 4
		jmp		@@startLoop
@@endLoop:
		mov		ecx, gfxSpan.sp_r
		mov		edx, gfxSpan.sp_g
		mov		edi, gfxSpan.sp_b
		mov		esp, _esp
		mov		gfxPRC.prc_rLeft, ecx
		mov		gfxPRC.prc_gLeft, edx
		mov		gfxPRC.prc_bLeft, edi
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
		PrimitiveDone
TRastXRGB ENDP

;===============================================================
;
; TRastXH
;
; Triangle flat-filled hazed poly rasterizer.  No texture or
; shading calculations.  Interpolates haze.
;
;===============================================================

TRastXH proc C
        public C TRastXH
		RasterizerSetup noSteps
@@rasterLoop:
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp

		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		ecx, gfxPRC.prc_hLeft
		mov		esi, gfxPRC.prc_currentY
		mov		gfxSpan.sp_h, ecx

@@startLoop:

		; set up span

		cmp		esi, gfxPRC.prc_destY
        je      @@endLoop
		mov		gfxSpan.sp_y, esi

        ; cnt = xRight - xLeft
		; x = xLeft

		sub		eax, ebx
		mov		gfxSpan.sp_x, ebx
		mov		gfxSpan.sp_cnt, eax

        ; jump into clipper

        jmp		DWORD PTR [gfxRC.rc_clipSpanHook]
@@nextSpan:
		; add deltas

		mov		esi, gfxPRC.prc_currentY
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		ecx, gfxSpan.sp_h
		add		esi, 1
		add		eax, gfxPRC.prc_deltaXR
		add		ebx, gfxPRC.prc_deltaXL
		add		ecx, gfxPRC.prc_deltaHL
		mov		gfxPRC.prc_xRight, eax
		mov		gfxPRC.prc_xLeft, ebx
		mov		gfxSpan.sp_h, ecx
		mov		edx, gfxRC.rc_stride
		mov		gfxPRC.prc_currentY, esi

		; add row offset and edge list offset

		add		gfxRC.rc_rowAddr, edx
		add		gfxRC.rc_currentEdgeLine, 4
		jmp		@@startLoop
@@endLoop:
		mov		ecx, gfxSpan.sp_h
		mov		esp, _esp
		mov		gfxPRC.prc_hLeft, ecx
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
		PrimitiveDone
TRastXH ENDP


;===============================================================
;
; TRastXUV
;
; Textured triangle rasterizer.
;
;===============================================================

TRastXUV proc C
        public C TRastXUV
		RasterizerSetup noSteps
@@rasterLoop:
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp

		; Prime the cache for gfxSpan.sp_
		mov		ecx, gfxSpan.sp_x

		; keep vLeft and uLeft on the FPU stack.

		fld		gfxPRC.prc_uLeft
		fld		gfxPRC.prc_vLeft
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		esi, gfxPRC.prc_currentY

@@startLoop:
		; set up span
		cmp		esi, gfxPRC.prc_destY
        je      @@endLoop
		mov		ecx, gfxPRC.prc_hLeft
		mov		gfxSpan.sp_y, esi
		add		ecx, gfxPRC.prc_hRight
        ; cnt = xRight - xLeft
		; x = xLeft
		sub		eax, ebx
		shr		ecx, 1
		mov		gfxSpan.sp_x, ebx
		and		ecx, gfxRC.rc_hazeMask
		mov		gfxSpan.sp_cnt, eax
		add		ecx, gfxRC.rc_shadeMap
		mov		gfxSpan.sp_h, ecx
		; store current u and v
		fst		gfxSpan.sp_v
		fxch	st(1)
		fst		gfxSpan.sp_u
		fxch	st(1)
        ; jump into the clipper
        jmp		DWORD PTR [gfxRC.rc_clipSpanHook]
@@nextSpan:
		; add deltas

		mov		esi, gfxPRC.prc_currentY
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		fadd	gfxPRC.prc_deltaVL
		fxch	st(1)
		fadd	gfxPRC.prc_deltaUL
		fxch	st(1)
		add		esi, 1
		add		eax, gfxPRC.prc_deltaXR
		add		ebx, gfxPRC.prc_deltaXL
		mov		gfxPRC.prc_xRight, eax
		mov		gfxPRC.prc_xLeft, ebx
		mov		edx, gfxRC.rc_stride
		mov		gfxPRC.prc_currentY, esi

		; add next row offset

		add		gfxRC.rc_rowAddr, edx
		mov		ecx, gfxPRC.prc_deltaHL
		mov		edx, gfxPRC.prc_deltaHR
		add		gfxPRC.prc_hLeft, ecx
		add		gfxPRC.prc_hRight, edx
		add		gfxRC.rc_currentEdgeLine, 4
		jmp		@@startLoop
@@endLoop:
		; store u and v back into gfxPRC

		fstp	gfxPRC.prc_vLeft
		fstp	gfxPRC.prc_uLeft
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
		PrimitiveDone
TRastXUV ENDP

;===============================================================
;
; TRastXUVW
;
; Persepective textured triangle rasterizer.
;
;===============================================================

TRastXUVW proc C
        public C TRastXUVW
		RasterizerSetup noSteps
@@rasterLoop:
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp

		; Prime the cache for gfxSpan.sp_
		mov		ecx, gfxSpan.sp_x

		fld		gfxPRC.prc_uLeft
		fld		gfxPRC.prc_vLeft
		fld		gfxPRC.prc_wLeft
		fstp	gfxSpan.sp_w
		fstp	gfxSpan.sp_v
		fstp	gfxSpan.sp_u

		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		esi, gfxPRC.prc_currentY

@@startLoop:
		; set up span

		cmp		esi, gfxPRC.prc_destY
        je      @@endLoop
		mov		gfxSpan.sp_y, esi
		mov		ecx, gfxPRC.prc_hRight
        ; cnt = xRight - xLeft
		; x = xLeft

		add		ecx, gfxPRC.prc_hLeft
		sub		eax, ebx
		shr		ecx, 1
		mov		gfxSpan.sp_x, ebx
		and		ecx, gfxRC.rc_hazeMask
		mov		gfxSpan.sp_cnt, eax
		add		ecx, gfxRC.rc_shadeMap
		mov		gfxSpan.sp_h, ecx
        ; jump into the clipper

        jmp		DWORD PTR [gfxRC.rc_clipSpanHook]
@@nextSpan:
		; add deltas

		mov		esi, gfxPRC.prc_currentY
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		fld		gfxSpan.sp_w
		fadd	gfxPRC.prc_deltaWL
		fld		gfxSpan.sp_v
		fadd	gfxPRC.prc_deltaVL
		fld		gfxSpan.sp_u
		fadd	gfxPRC.prc_deltaUL
		fxch	st(2)
		fstp	gfxSpan.sp_w
		fstp	gfxSpan.sp_v
		fstp	gfxSpan.sp_u

		add		esi, 1
		add		eax, gfxPRC.prc_deltaXR
		add		ebx, gfxPRC.prc_deltaXL
		mov		gfxPRC.prc_xRight, eax
		mov		gfxPRC.prc_xLeft, ebx
		mov		ecx, gfxPRC.prc_deltaHL
		mov		edx, gfxPRC.prc_deltaHR
		add		gfxPRC.prc_hLeft, ecx
		add		gfxPRC.prc_hRight, edx
		mov		edx, gfxRC.rc_stride
		mov		gfxPRC.prc_currentY, esi

		; add next row offset

		add		gfxRC.rc_rowAddr, edx
		add		gfxRC.rc_currentEdgeLine, 4
		jmp		@@startLoop
@@endLoop:
		; store u, v and w back into gfxPRC

		fld		gfxSpan.sp_u
		fld		gfxSpan.sp_v
		fld		gfxSpan.sp_w
		fstp	gfxPRC.prc_wLeft
		fstp	gfxPRC.prc_vLeft
		fstp	gfxPRC.prc_uLeft
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
		PrimitiveDone
TRastXUVW ENDP

;===============================================================
;
; TRastXUVWH
;
; Persepective textured triangle rasterizer.
;
;===============================================================

TRastXUVWH proc C
        public C TRastXUVWH
		RasterizerSetup noSteps
@@rasterLoop:
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp

		; Prime the cache for gfxSpan.sp_
		mov		ecx, gfxSpan.sp_x

		fld		gfxPRC.prc_uLeft
		fld		gfxPRC.prc_vLeft
		fld		gfxPRC.prc_wLeft
		fstp	gfxSpan.sp_w
		fstp	gfxSpan.sp_v
		fstp	gfxSpan.sp_u

		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		ecx, gfxPRC.prc_hLeft
		mov		esi, gfxPRC.prc_currentY
		mov		gfxSpan.sp_h, ecx

@@startLoop:
		; set up span

		cmp		esi, gfxPRC.prc_destY
        je      @@endLoop
		mov		gfxSpan.sp_y, esi
        ; cnt = xRight - xLeft
		; x = xLeft

		sub		eax, ebx
		mov		gfxSpan.sp_x, ebx
		mov		gfxSpan.sp_cnt, eax
        ; jump into the clipper

        jmp		DWORD PTR [gfxRC.rc_clipSpanHook]
@@nextSpan:
		; add deltas

		mov		esi, gfxPRC.prc_currentY
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		fld		gfxSpan.sp_w
		fadd	gfxPRC.prc_deltaWL
		fld		gfxSpan.sp_v
		fadd	gfxPRC.prc_deltaVL
		fld		gfxSpan.sp_u
		fadd	gfxPRC.prc_deltaUL
		fxch	st(2)
		fstp	gfxSpan.sp_w
		fstp	gfxSpan.sp_v
		fstp	gfxSpan.sp_u

		add		esi, 1
		add		eax, gfxPRC.prc_deltaXR
		add		ebx, gfxPRC.prc_deltaXL
		mov		ecx, gfxPRC.prc_deltaHL
		mov		gfxPRC.prc_xRight, eax
		mov		gfxPRC.prc_xLeft, ebx
		add		gfxSpan.sp_h, ecx
		mov		edx, gfxRC.rc_stride
		mov		gfxPRC.prc_currentY, esi

		; add next row offset

		add		gfxRC.rc_rowAddr, edx
		add		gfxRC.rc_currentEdgeLine, 4
		jmp		@@startLoop
@@endLoop:
		; store u, v and w back into gfxPRC
		mov		ecx, gfxSpan.sp_h
		fld		gfxSpan.sp_u
		mov		gfxPRC.prc_hLeft, ecx
		fld		gfxSpan.sp_v
		fld		gfxSpan.sp_w
		fstp	gfxPRC.prc_wLeft
		fstp	gfxPRC.prc_vLeft
		fstp	gfxPRC.prc_uLeft
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
		PrimitiveDone
TRastXUVWH ENDP

;===============================================================
;
; TRastXUVS
;
; Textured, shaded triangle rasterizer.
;
;===============================================================

TRastXUVS proc C
        public C TRastXUVS
		RasterizerSetup noSteps
@@rasterLoop:
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp

		; Prime the cache for gfxSpan.sp_
		mov		ecx, gfxSpan.sp_x

		; keep vLeft and uLeft in FPU registers

		fld		gfxPRC.prc_uLeft
		fld		gfxPRC.prc_vLeft
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		ecx, gfxPRC.prc_iLeft
		mov		esi, gfxPRC.prc_currentY
		mov		gfxSpan.sp_s, ecx
@@startLoop:
		; set up span
		cmp		esi, gfxPRC.prc_destY
        je      @@endLoop
		mov		ecx, gfxPRC.prc_hLeft
		mov		gfxSpan.sp_y, esi
		add		ecx, gfxPRC.prc_hRight
        ; cnt = xRight - xLeft
		; x = xLeft
		sub		eax, ebx
		shr		ecx, 1
		mov		gfxSpan.sp_x, ebx
		and		ecx, gfxRC.rc_hazeMask
		mov		gfxSpan.sp_cnt, eax
		add		ecx, gfxRC.rc_shadeMap
		mov		gfxSpan.sp_h, ecx
		; store current u and v
		fst		gfxSpan.sp_v
		fxch	st(1)
		fst		gfxSpan.sp_u
		fxch	st(1)
        ; jump into the clipper
        jmp		DWORD PTR [gfxRC.rc_clipSpanHook]
@@nextSpan:
		; add deltas
		mov		esi, gfxPRC.prc_currentY
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		fadd	gfxPRC.prc_deltaVL
		fxch	st(1)
		fadd	gfxPRC.prc_deltaUL
		fxch	st(1)
		mov		ecx, gfxPRC.prc_deltaIL
		add		esi, 1
		add		eax, gfxPRC.prc_deltaXR
		add		ebx, gfxPRC.prc_deltaXL
		add		gfxSpan.sp_s, ecx
		mov		gfxPRC.prc_xRight, eax
		mov		gfxPRC.prc_xLeft, ebx
		mov		edx, gfxRC.rc_stride
		mov		gfxPRC.prc_currentY, esi
		; add next row offset
		add		gfxRC.rc_rowAddr, edx
		mov		ecx, gfxPRC.prc_deltaHL
		mov		edx, gfxPRC.prc_deltaHR
		add		gfxPRC.prc_hLeft, ecx
		add		gfxPRC.prc_hRight, edx
		add		gfxRC.rc_currentEdgeLine, 4
		jmp		@@startLoop
@@endLoop:
		; restore values into the PRC
		fstp	gfxPRC.prc_vLeft
		fstp	gfxPRC.prc_uLeft
		mov		ecx, gfxSpan.sp_s
		mov		esp, _esp
		mov		gfxPRC.prc_iLeft, ecx
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
		PrimitiveDone
TRastXUVS ENDP

;===============================================================
;
; TRastXUVRGB
;
; Textured, shaded triangle rasterizer.
;
;===============================================================

TRastXUVRGB proc C
        public C TRastXUVRGB
		RasterizerSetup noSteps
@@rasterLoop:
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp

		; Prime the cache for gfxSpan.sp_
		mov		ecx, gfxSpan.sp_x

		; keep vLeft and uLeft in FPU registers

		fld		gfxPRC.prc_uLeft
		fld		gfxPRC.prc_vLeft
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		ecx, gfxPRC.prc_rLeft
		mov		edx, gfxPRC.prc_gLeft
		mov		edi, gfxPRC.prc_bLeft
		mov		esi, gfxPRC.prc_currentY
		mov		gfxSpan.sp_r, ecx
		mov		gfxSpan.sp_g, edx
		mov		gfxSpan.sp_b, edi
@@startLoop:
		; set up span
		cmp		esi, gfxPRC.prc_destY
        je      @@endLoop
		mov		ecx, gfxPRC.prc_hLeft
		mov		gfxSpan.sp_y, esi
		add		ecx, gfxPRC.prc_hRight
        ; cnt = xRight - xLeft
		; x = xLeft
		sub		eax, ebx
		shr		ecx, 1
		mov		gfxSpan.sp_x, ebx
		and		ecx, gfxRC.rc_hazeMask
		mov		gfxSpan.sp_cnt, eax
		add		ecx, gfxRC.rc_shadeMap
		mov		gfxSpan.sp_h, ecx
		; store current u and v
		fst		gfxSpan.sp_v
		fxch	st(1)
		fst		gfxSpan.sp_u
		fxch	st(1)
        ; jump into the clipper
        jmp		DWORD PTR [gfxRC.rc_clipSpanHook]
@@nextSpan:
		; add deltas
		mov		esi, gfxPRC.prc_currentY
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		fadd	gfxPRC.prc_deltaVL
		fxch	st(1)
		fadd	gfxPRC.prc_deltaUL
		fxch	st(1)
		mov		ecx, gfxPRC.prc_deltaIL
		add		esi, 1
		add		eax, gfxPRC.prc_deltaXR
		add		ebx, gfxPRC.prc_deltaXL
		add		gfxSpan.sp_s, ecx
		mov		gfxPRC.prc_xRight, eax
		mov		gfxPRC.prc_xLeft, ebx
		mov		edx, gfxRC.rc_stride
		mov		gfxPRC.prc_currentY, esi
		; add next row offset
		add		gfxRC.rc_rowAddr, edx
		mov		ecx, gfxSpan.sp_r
		mov		edx, gfxSpan.sp_g
		mov		edi, gfxSpan.sp_b
		add		ecx, gfxPRC.prc_deltaRL
		add		edx, gfxPRC.prc_deltaGL
		add		edi, gfxPRC.prc_deltaBL
		mov		gfxSpan.sp_r, ecx
		mov		gfxSpan.sp_g, edx
		mov		gfxSpan.sp_b, edi
		add		gfxRC.rc_currentEdgeLine, 4
		jmp		@@startLoop
@@endLoop:
		; restore values into the PRC
		fstp	gfxPRC.prc_vLeft
		fstp	gfxPRC.prc_uLeft
		mov		ecx, gfxSpan.sp_r
		mov		edx, gfxSpan.sp_g
		mov		edi, gfxSpan.sp_b
		mov		esp, _esp
		mov		gfxPRC.prc_rLeft, ecx
		mov		gfxPRC.prc_gLeft, edx
		mov		gfxPRC.prc_bLeft, edi
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
		PrimitiveDone
TRastXUVRGB ENDP

;===============================================================
;
; TRastXUVWS
;
; Persepective textured triangle rasterizer.
;
;===============================================================

TRastXUVWS proc C
        public C TRastXUVWS
		RasterizerSetup noSteps
@@rasterLoop:
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp

		; Prime the cache for gfxSpan.sp_
		mov		ecx, gfxSpan.sp_x

		fld		gfxPRC.prc_uLeft
		fld		gfxPRC.prc_vLeft
		fld		gfxPRC.prc_wLeft
		fstp	gfxSpan.sp_w
		fstp	gfxSpan.sp_v
		fstp	gfxSpan.sp_u

		mov		ecx, gfxPRC.prc_iLeft
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		esi, gfxPRC.prc_currentY
		mov		gfxSpan.sp_s, ecx
@@startLoop:
		; set up span

		cmp		esi, gfxPRC.prc_destY
        je      @@endLoop
		mov		gfxSpan.sp_y, esi
		mov		ecx, gfxPRC.prc_hRight
        ; cnt = xRight - xLeft
		; x = xLeft

		add		ecx, gfxPRC.prc_hLeft
		sub		eax, ebx
		shr		ecx, 1
		mov		gfxSpan.sp_x, ebx
		and		ecx, gfxRC.rc_hazeMask
		mov		gfxSpan.sp_cnt, eax
		add		ecx, gfxRC.rc_shadeMap
		mov		gfxSpan.sp_h, ecx
        ; jump into the clipper

        jmp		DWORD PTR [gfxRC.rc_clipSpanHook]
@@nextSpan:
		; add deltas

		mov		esi, gfxPRC.prc_currentY
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		fld		gfxSpan.sp_w
		fadd	gfxPRC.prc_deltaWL
		fld		gfxSpan.sp_v
		fadd	gfxPRC.prc_deltaVL
		fld		gfxSpan.sp_u
		fadd	gfxPRC.prc_deltaUL
		fxch	st(2)
		fstp	gfxSpan.sp_w
		fstp	gfxSpan.sp_v
		fstp	gfxSpan.sp_u

		add		esi, 1
		mov		ecx, gfxPRC.prc_deltaIL
		add		eax, gfxPRC.prc_deltaXR
		add		ebx, gfxPRC.prc_deltaXL
		mov		gfxPRC.prc_xRight, eax
		mov		gfxPRC.prc_xLeft, ebx
		add		gfxSpan.sp_s, ecx
		mov		edx, gfxRC.rc_stride
		mov		gfxPRC.prc_currentY, esi

		; add next row offset

		add		gfxRC.rc_rowAddr, edx
		mov		ecx, gfxPRC.prc_deltaHL
		mov		edx, gfxPRC.prc_deltaHR
		add		gfxPRC.prc_hLeft, ecx
		add		gfxPRC.prc_hRight, edx
		add		gfxRC.rc_currentEdgeLine, 4
		jmp		@@startLoop
@@endLoop:
		; store u, v and w back into gfxPRC

		fld		gfxSpan.sp_u
		fld		gfxSpan.sp_v
		fld		gfxSpan.sp_w
		fstp	gfxPRC.prc_wLeft
		fstp	gfxPRC.prc_vLeft
		fstp	gfxPRC.prc_uLeft
		mov		ecx, gfxSpan.sp_s
		mov		esp, _esp
		mov		gfxPRC.prc_iLeft, ecx
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
		PrimitiveDone
TRastXUVWS ENDP

;===============================================================
;
; PRastXUV
;
; Textured polygon rasterizer.
;
;===============================================================

PRastXUV proc C
        public C PRastXUV
		RasterizerSetup hasSteps
@@rasterLoop:
		fldcw	WORD PTR controlWord
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp

		; Prime the cache for gfxSpan.sp_
		mov		ecx, gfxSpan.sp_x

		; keep uL, vL, uR, vR in FPU registers

		fld		gfxPRC.prc_uLeft
		fld		gfxPRC.prc_vLeft
		fld		gfxPRC.prc_uRight
		fld		gfxPRC.prc_vRight
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		esi, gfxPRC.prc_currentY

@@startLoop:
		; set up span

		cmp		esi, gfxPRC.prc_destY
        je      @@endLoop
		mov		gfxSpan.sp_y, esi

        ; cnt = xRight - xLeft
		; x = xLeft

        lea		ecx, @@calcSpanSteps
		sub		eax, ebx
		jz		@@noPopNextSpan
		mov		gfxSpan.sp_x, ebx
		mov		gfxSpan.sp_cnt, eax
		mov		gfxRC.rc_spanClippedHook, ecx

        ; put  1/span length on the stack - do the divide while clipping

        fld		fpConstant65536
        fidiv	gfxSpan.sp_cnt			; st=1/cnt,vRight,uRight,vLeft,uLeft
        ; jump into clipper
        jmp		DWORD PTR [gfxRC.rc_clipSpanHook]
@@calcSpanSteps:

								; st(0)  st(1)  st(2)  st(3)  st(4)  st(5)
								; 1/cnt  vR     uR     vL     uL
		fld		st(2)			; uR     1/cnt  vR     uR     vL     uL
		fsub	st, st(5)		; uR-uL  1/cnt  vR     uR     vl     uL
		fld		st(2)			; vR     uR-uL  1/cnt  vR     uR     vL    uL
		fsub	st, st(5)       ; vR-vL  uR-uL  1/cnt  vR     uR     vL    uL
		fxch	st(1)			; uR-uL  vR-vL  1/cnt  vR     uR     vL    uL
		fmul	st, st(2)		; su     vR-vL  1/cnt  vR     uR     vL    uL
		mov		eax, gfxRC.rc_renderSpanHook
		mov		ecx, gfxPRC.prc_hLeft
		fxch	st(1)			; vR-vL  su     1/cnt  vR     uR     vL    uL
		fmulp	st(2), st		; sv     su     1/cnt  vR     uR     vL     uL
		add		ecx, gfxPRC.prc_hRight
		shr		ecx, 1
		and		ecx, gfxRC.rc_hazeMask
		fst		gfxSpan.sp_su
		fistp	gfxSpan.sp_fsu		; sv     vR     uR     vL     uL
		fst		gfxSpan.sp_sv
		fist	gfxSpan.sp_fsv
		fxch	st(4)			; uL     vR     uR     vL     sv
		fst		gfxSpan.sp_u
		fxch	st(3)           ; vL     vR     uR     uL     sv
		fst		gfxSpan.sp_v
		fxch	st(3)           ; uL     vR     uR     vL     sv
		fxch	st(4)           ; sv     vR     uR     vL     uL
		add		ecx, gfxRC.rc_shadeMap
		mov		gfxRC.rc_spanClippedHook, eax
		mov		gfxSpan.sp_h, ecx
		jmp		DWORD PTR [gfxRC.rc_renderSpanHook]
@@nextSpan:
		fstp	st
@@noPopNextSpan:

		; add in some deltas

		mov		esi, gfxPRC.prc_currentY
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
								; st(0)  st(1)  st(2)  st(3)  st(4)  st(5)
								; vR     uR     vL     uL
		fadd	gfxPRC.prc_deltaVR
		fxch	st(1)
		fadd	gfxPRC.prc_deltaUR
		fxch	st(2)
		fadd	gfxPRC.prc_deltaVL
		fxch	st(3)
		fadd	gfxPRC.prc_deltaUL
		fxch	st(3)
		fxch	st(2)
		fxch	st(1)
		add		esi, 1
		add		eax, gfxPRC.prc_deltaXR
		add		ebx, gfxPRC.prc_deltaXL
		mov		gfxPRC.prc_xRight, eax
		mov		gfxPRC.prc_xLeft, ebx
		mov		edx, gfxRC.rc_stride
		mov		gfxPRC.prc_currentY, esi

		; add in next row offset

		add		gfxRC.rc_rowAddr, edx
		mov		ecx, gfxPRC.prc_deltaHL
		mov		edx, gfxPRC.prc_deltaHR
		add		gfxPRC.prc_hLeft, ecx
		add		gfxPRC.prc_hRight, edx
		add		gfxRC.rc_currentEdgeLine, 4
		jmp		@@startLoop
@@endLoop:
		; restore u,v to gfxPRC

		fstp	gfxPRC.prc_vRight
		fstp	gfxPRC.prc_uRight
		fstp	gfxPRC.prc_vLeft
		fstp	gfxPRC.prc_uLeft
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
		PrimitiveDone popStack
PRastXUV ENDP

;===============================================================
;
; PRastXUVS
;
; Textured, gouraud shaded polygon rasterizer.
;
;===============================================================

PRastXUVS proc C
        public C PRastXUVS
		RasterizerSetup hasSteps
@@rasterLoop:
		fldcw	WORD PTR controlWord
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp

		; Prime the cache for gfxSpan.sp_
		mov		ecx, gfxSpan.sp_x

		fld		gfxPRC.prc_uLeft
		fld		gfxPRC.prc_vLeft
		fld		gfxPRC.prc_uRight
		fld		gfxPRC.prc_vRight
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		ecx, gfxPRC.prc_iLeft
		mov		esi, gfxPRC.prc_currentY
		mov		gfxSpan.sp_s, ecx
@@startLoop:
		cmp		esi, gfxPRC.prc_destY
        je      @@endLoop
		mov		gfxSpan.sp_y, esi

        ; cnt = xRight - xLeft
		; x = xLeft
        lea		ecx, @@calcSpanSteps
		sub		eax, ebx
		jz		@@noPopNextSpan
		mov		gfxSpan.sp_x, ebx
		mov		gfxSpan.sp_cnt, eax
		mov		gfxRC.rc_spanClippedHook, ecx
        fld		fpConstant65536
        fidiv	gfxSpan.sp_cnt			; st=1/cnt,vRight,uRight,vLeft,uLeft
        jmp		DWORD PTR [gfxRC.rc_clipSpanHook]
@@calcSpanSteps:

								; st(0)  st(1)  st(2)  st(3)  st(4)  st(5)
								; 1/cnt  vR     uR     vL     uL
		fld		st(2)			; uR     1/cnt  vR     uR     vL     uL
		fsub	st, st(5)		; uR-uL  1/cnt  vR     uR     vl     uL
		fld		st(2)			; vR     uR-uL  1/cnt  vR     uR     vL    uL
		fsub	st, st(5)       ; vR-vL  uR-uL  1/cnt  vR     uR     vL    uL
		fxch	st(1)			; uR-uL  vR-vL  1/cnt  vR     uR     vL    uL
		fmul	st, st(2)		; su     vR-vL  1/cnt  vR     uR     vL    uL
		mov		ecx, gfxPRC.prc_hRight
		mov		ebx, gfxPRC.prc_iRight
		add		ecx, gfxPRC.prc_hLeft
		sub		ebx, gfxSpan.sp_s
		fxch	st(1)			; vR-vL  su     1/cnt  vR     uR     vL    uL
		fmul	st, st(2)		; sv     su     1/cnt  vR     uR     vL     uL
		shr		ecx, 1
		mov		eax, gfxRC.rc_renderSpanHook
		mov		gfxSpan.sp_s_s, ebx
		and		ecx, gfxRC.rc_hazeMask
		fxch	st(2)
		fimul	gfxSpan.sp_s_s
		add		ecx, gfxRC.rc_shadeMap
		mov		gfxSpan.sp_h, ecx
		fistp	gfxSpan.sp_s_s
		fst		gfxSpan.sp_su
		fistp	gfxSpan.sp_fsu		; sv     vR     uR     vL     uL
		fst		gfxSpan.sp_sv
		fist	gfxSpan.sp_fsv
		fxch	st(4)			; uL     vR     uR     vL     sv
		fst		gfxSpan.sp_u
		fxch	st(3)           ; vL     vR     uR     uL     sv
		fst		gfxSpan.sp_v
		fxch	st(3)           ; uL     vR     uR     vL     sv
		fxch	st(4)           ; sv     vR     uR     vL     uL

		mov		gfxRC.rc_spanClippedHook, eax
		jmp		DWORD PTR [gfxRC.rc_renderSpanHook]
@@nextSpan:
		fstp	st
@@noPopNextSpan:
		mov		esi, gfxPRC.prc_currentY
		mov		eax, gfxPRC.prc_deltaIL
		mov		ebx, gfxPRC.prc_deltaIR
		add		gfxSpan.sp_s, eax
		add		gfxPRC.prc_iRight, ebx
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
								; st(0)  st(1)  st(2)  st(3)  st(4)  st(5)
								; vR     uR     vL     uL
		fadd	gfxPRC.prc_deltaVR
		fxch	st(1)
		fadd	gfxPRC.prc_deltaUR
		fxch	st(2)
		fadd	gfxPRC.prc_deltaVL
		fxch	st(3)
		fadd	gfxPRC.prc_deltaUL
		fxch	st(3)
		fxch	st(2)
		fxch	st(1)
		add		esi, 1
		add		eax, gfxPRC.prc_deltaXR
		add		ebx, gfxPRC.prc_deltaXL
		mov		gfxPRC.prc_xRight, eax
		mov		gfxPRC.prc_xLeft, ebx
		mov		edx, gfxRC.rc_stride
		mov		gfxPRC.prc_currentY, esi
		add		gfxRC.rc_rowAddr, edx
		mov		ecx, gfxPRC.prc_deltaHL
		mov		edx, gfxPRC.prc_deltaHR
		add		gfxPRC.prc_hLeft, ecx
		add		gfxPRC.prc_hRight, edx
		add		gfxRC.rc_currentEdgeLine, 4
		jmp		@@startLoop
@@endLoop:
		mov		eax, gfxSpan.sp_s
		fstp	gfxPRC.prc_vRight
		fstp	gfxPRC.prc_uRight
		fstp	gfxPRC.prc_vLeft
		fstp	gfxPRC.prc_uLeft
		mov		gfxPRC.prc_iLeft, eax
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
		PrimitiveDone popStack
PRastXUVS ENDP

;===============================================================
;
; PRastXUVRGB
;
; Textured, gouraud shaded polygon rasterizer.
;
;===============================================================

PRastXUVRGB proc C
        public C PRastXUVRGB
		RasterizerSetup hasSteps
@@rasterLoop:
		fldcw	WORD PTR controlWord
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp

		; Prime the cache for gfxSpan.sp_
		mov		ecx, gfxSpan.sp_x

		fld		gfxPRC.prc_uLeft
		fld		gfxPRC.prc_vLeft
		fld		gfxPRC.prc_uRight
		fld		gfxPRC.prc_vRight
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		esi, gfxPRC.prc_currentY
		mov		ecx, gfxPRC.prc_rLeft
		mov		edx, gfxPRC.prc_gLeft
		mov		edi, gfxPRC.prc_bLeft
		mov		gfxSpan.sp_r, ecx
		mov		gfxSpan.sp_g, edx
		mov		gfxSpan.sp_b, edi
@@startLoop:
		cmp		esi, gfxPRC.prc_destY
        je      @@endLoop
		mov		gfxSpan.sp_y, esi

        ; cnt = xRight - xLeft
		; x = xLeft
        lea		ecx, @@calcSpanSteps
		sub		eax, ebx
		jz		@@noPopNextSpan
		mov		gfxSpan.sp_x, ebx
		mov		gfxSpan.sp_cnt, eax
		mov		gfxRC.rc_spanClippedHook, ecx
        fld		fpConstant65536
        fidiv	gfxSpan.sp_cnt			; st=1/cnt,vRight,uRight,vLeft,uLeft
		mov		eax, gfxPRC.prc_rRight
		mov		ecx, gfxPRC.prc_gRight
		mov		edx, gfxPRC.prc_bRight
		sub		eax, gfxSpan.sp_r
		sub		ecx, gfxSpan.sp_g
		sub		edx, gfxSpan.sp_b
		mov		gfxSpan.sp_sr, eax
		mov		gfxSpan.sp_sg, ecx
		mov		gfxSpan.sp_sb, edx
        jmp		DWORD PTR [gfxRC.rc_clipSpanHook]
@@calcSpanSteps:

								; st(0)  st(1)  st(2)  st(3)  st(4)  st(5)
								; 1/cnt  vR     uR     vL     uL
		fld		st(2)			; uR     1/cnt  vR     uR     vL     uL
		fsub	st, st(5)		; uR-uL  1/cnt  vR     uR     vl     uL
		fld		st(2)			; vR     uR-uL  1/cnt  vR     uR     vL    uL
		fsub	st, st(5)       ; vR-vL  uR-uL  1/cnt  vR     uR     vL    uL
		fxch	st(1)			; uR-uL  vR-vL  1/cnt  vR     uR     vL    uL
		fmul	st, st(2)		; su     vR-vL  1/cnt  vR     uR     vL    uL
		fxch	st(1)			; vR-vL  su     1/cnt  vR     uR     vL    uL
		fmul	st, st(2)		; sv     su     1/cnt  vR     uR     vL     uL
		mov		eax, gfxRC.rc_renderSpanHook
		fxch	st(2)

		fld		st
		fimul	gfxSpan.sp_sr
		fistp	gfxSpan.sp_sr
		fld		st
		fimul	gfxSpan.sp_sg
		fxch	st(1)
		fimul	gfxSpan.sp_sb
		fxch	st(1)
		fistp	gfxSpan.sp_sb
		fistp	gfxSpan.sp_sg

		fst		gfxSpan.sp_su
		fistp	gfxSpan.sp_fsu		; sv     vR     uR     vL     uL
		fst		gfxSpan.sp_sv
		fist	gfxSpan.sp_fsv
		fxch	st(4)			; uL     vR     uR     vL     sv
		fst		gfxSpan.sp_u
		fxch	st(3)           ; vL     vR     uR     uL     sv
		fst		gfxSpan.sp_v
		fxch	st(3)           ; uL     vR     uR     vL     sv
		fxch	st(4)           ; sv     vR     uR     vL     uL

		mov		gfxRC.rc_spanClippedHook, eax
		jmp		DWORD PTR [gfxRC.rc_renderSpanHook]
@@nextSpan:
		fstp	st
@@noPopNextSpan:
		mov		esi, gfxPRC.prc_currentY
		mov		eax, gfxPRC.prc_deltaIL
		mov		ebx, gfxPRC.prc_deltaIR
		add		gfxSpan.sp_s, eax
		add		gfxPRC.prc_iRight, ebx
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
								; st(0)  st(1)  st(2)  st(3)  st(4)  st(5)
								; vR     uR     vL     uL
		fadd	gfxPRC.prc_deltaVR
		fxch	st(1)
		fadd	gfxPRC.prc_deltaUR
		fxch	st(2)
		fadd	gfxPRC.prc_deltaVL
		fxch	st(3)
		fadd	gfxPRC.prc_deltaUL
		fxch	st(3)
		fxch	st(2)
		fxch	st(1)
		add		esi, 1
		add		eax, gfxPRC.prc_deltaXR
		add		ebx, gfxPRC.prc_deltaXL

		mov		gfxPRC.prc_xRight, eax
		mov		gfxPRC.prc_xLeft, ebx
		mov		ecx, gfxPRC.prc_rRight
		mov		edx, gfxSpan.sp_r
		add		ecx, gfxPRC.prc_deltaRR
		add		edx, gfxPRC.prc_deltaRL
		mov		gfxPRC.prc_rRight, ecx
		mov		gfxSpan.sp_r, edx
		mov		ecx, gfxPRC.prc_gRight
		mov		edx, gfxSpan.sp_g
		add		ecx, gfxPRC.prc_deltaGR
		add		edx, gfxPRC.prc_deltaGL
		mov		gfxPRC.prc_gRight, ecx
		mov		gfxSpan.sp_g, edx
		mov		ecx, gfxPRC.prc_bRight
		mov		edx, gfxSpan.sp_b
		add		ecx, gfxPRC.prc_deltaBR
		add		edx, gfxPRC.prc_deltaBL
		mov		gfxPRC.prc_bRight, ecx
		mov		gfxSpan.sp_b, edx

		mov		edx, gfxRC.rc_stride
		mov		gfxPRC.prc_currentY, esi
		add		gfxRC.rc_rowAddr, edx
		add		gfxRC.rc_currentEdgeLine, 4
		jmp		@@startLoop
@@endLoop:
		mov		ecx, gfxSpan.sp_r
		mov		edx, gfxSpan.sp_g
		mov		edi, gfxSpan.sp_b
		mov		gfxPRC.prc_rLeft, ecx
		mov		gfxPRC.prc_gLeft, edx
		mov		gfxPRC.prc_bLeft, edi
		fstp	gfxPRC.prc_vRight
		fstp	gfxPRC.prc_uRight
		fstp	gfxPRC.prc_vLeft
		fstp	gfxPRC.prc_uLeft
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
		PrimitiveDone popStack
PRastXUVRGB ENDP

;===============================================================
;
; PRastXS
;
; Gouraud shaded poly rasterizer
;
;===============================================================

PRastXS proc C
        public C PRastXS
		RasterizerSetup hasSteps
@@rasterLoop:
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp

		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		esi, gfxPRC.prc_currentY

@@startLoop:
		cmp		esi, gfxPRC.prc_destY
        je      @@endLoop
		mov		gfxSpan.sp_y, esi

        ; cnt = xRight - xLeft
		; x = xLeft
		sub		eax, ebx
		mov		gfxSpan.sp_x, ebx
		mov		gfxSpan.sp_cnt, eax
		mov		eax, gfxPRC.prc_iRight
		mov		ebx, gfxPRC.prc_iLeft
		sub		eax, ebx
		mov		gfxSpan.sp_s, ebx
        fld		fpConstant65536
        fidiv	gfxSpan.sp_cnt
		mov		gfxSpan.sp_s_s, eax
        jmp		DWORD PTR [gfxRC.rc_clipSpanHook]
@@calcSpanSteps:
		fimul	gfxSpan.sp_s_s
		mov		ecx, gfxPRC.prc_hLeft
		mov		eax, gfxRC.rc_renderSpanHook
		add		ecx, gfxPRC.prc_hRight
		mov		gfxRC.rc_spanClippedHook, eax
		shr		ecx, 1
		and		ecx, gfxRC.rc_hazeMask
		add		ecx, gfxRC.rc_shadeMap
		fist	gfxSpan.sp_s_s
		mov		gfxSpan.sp_h, ecx
		jmp		DWORD PTR [gfxRC.rc_renderSpanHook]
@@nextSpan:
		ffree	st
		lea		edi, @@calcSpanSteps
		mov		esi, gfxPRC.prc_currentY
		mov		gfxRC.rc_spanClippedHook, edi
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		ecx, gfxPRC.prc_iRight
		mov		edx, gfxPRC.prc_iLeft
		add		esi, 1
		add		eax, gfxPRC.prc_deltaXR
		add		ebx, gfxPRC.prc_deltaXL
		add		ecx, gfxPRC.prc_deltaIR
		add		edx, gfxPRC.prc_deltaIL
		mov		gfxPRC.prc_xRight, eax
		mov		gfxPRC.prc_xLeft, ebx
		mov		gfxPRC.prc_iRight, ecx
		mov		gfxPRC.prc_iLeft, edx
		mov		edx, gfxRC.rc_stride
		mov		gfxPRC.prc_currentY, esi
		add		gfxRC.rc_rowAddr, edx
		mov		ecx, gfxPRC.prc_deltaHL
		mov		edx, gfxPRC.prc_deltaHR
		add		gfxPRC.prc_hLeft, ecx
		add		gfxPRC.prc_hRight, edx
		add		gfxRC.rc_currentEdgeLine, 4
		jmp		@@startLoop
@@endLoop:
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
		PrimitiveDone popStack
PRastXS ENDP

;===============================================================
;
; PRastXRGB
;
; Gouraud shaded poly rasterizer
;
;===============================================================

PRastXRGB proc C
        public C PRastXRGB
		RasterizerSetup hasSteps
@@rasterLoop:
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp

		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		esi, gfxPRC.prc_currentY

		mov		ecx, gfxPRC.prc_rLeft
		mov		edx, gfxPRC.prc_gLeft
		mov		edi, gfxPRC.prc_bLeft
		mov		gfxSpan.sp_r, ecx
		mov		gfxSpan.sp_g, edx
		mov		gfxSpan.sp_b, edi
@@startLoop:
		cmp		esi, gfxPRC.prc_destY
        je      @@endLoop
		mov		gfxSpan.sp_y, esi

        ; cnt = xRight - xLeft
		; x = xLeft
		sub		eax, ebx
		jz		@@nextSpan
		mov		gfxSpan.sp_x, ebx
		mov		gfxSpan.sp_cnt, eax
        fld		fpConstant65536
        fidiv	gfxSpan.sp_cnt
		mov		eax, gfxPRC.prc_rRight
		mov		ecx, gfxPRC.prc_gRight
		mov		edx, gfxPRC.prc_bRight
		sub		eax, gfxSpan.sp_r
		sub		ecx, gfxSpan.sp_g
		sub		edx, gfxSpan.sp_b
		mov		gfxSpan.sp_sr, eax
		mov		gfxSpan.sp_sg, ecx
		mov		gfxSpan.sp_sb, edx
        jmp		DWORD PTR [gfxRC.rc_clipSpanHook]
@@calcSpanSteps:
		fld		st
		fld		st
		fimul	gfxSpan.sp_sr
		mov		eax, gfxRC.rc_renderSpanHook
		mov		gfxRC.rc_spanClippedHook, eax
		fistp	gfxSpan.sp_sr
		fimul	gfxSpan.sp_sg
		fistp	gfxSpan.sp_sg
		fimul	gfxSpan.sp_sb
		fistp	gfxSpan.sp_sb
		jmp		DWORD PTR [gfxRC.rc_renderSpanHook]
@@nextSpan:
		ffree	st
		lea		edi, @@calcSpanSteps
		mov		esi, gfxPRC.prc_currentY
		mov		gfxRC.rc_spanClippedHook, edi
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		add		esi, 1
		add		eax, gfxPRC.prc_deltaXR
		add		ebx, gfxPRC.prc_deltaXL

		mov		ecx, gfxPRC.prc_rRight
		mov		edx, gfxSpan.sp_r
		add		ecx, gfxPRC.prc_deltaRR
		add		edx, gfxPRC.prc_deltaRL
		mov		gfxPRC.prc_rRight, ecx
		mov		gfxSpan.sp_r, edx
		mov		ecx, gfxPRC.prc_gRight
		mov		edx, gfxSpan.sp_g
		add		ecx, gfxPRC.prc_deltaGR
		add		edx, gfxPRC.prc_deltaGL
		mov		gfxPRC.prc_gRight, ecx
		mov		gfxSpan.sp_g, edx
		mov		ecx, gfxPRC.prc_bRight
		mov		edx, gfxSpan.sp_b
		add		ecx, gfxPRC.prc_deltaBR
		add		edx, gfxPRC.prc_deltaBL
		mov		gfxPRC.prc_bRight, ecx
		mov		gfxSpan.sp_b, edx

		mov		gfxPRC.prc_xRight, eax
		mov		gfxPRC.prc_xLeft, ebx
		mov		edx, gfxRC.rc_stride
		mov		gfxPRC.prc_currentY, esi
		add		gfxRC.rc_rowAddr, edx
		add		gfxRC.rc_currentEdgeLine, 4
		jmp		@@startLoop
@@endLoop:
		mov		ecx, gfxSpan.sp_r
		mov		edx, gfxSpan.sp_g
		mov		edi, gfxSpan.sp_b
		mov		gfxPRC.prc_rLeft, ecx
		mov		gfxPRC.prc_gLeft, edx
		mov		gfxPRC.prc_bLeft, edi
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
		PrimitiveDone popStack
PRastXRGB ENDP

;=================================================
;
; PRastXH
;
; Hazed poly rasterizer
;
;===============================================================

PRastXH proc C
        public C PRastXH
		RasterizerSetup hasSteps
@@rasterLoop:
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp

		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		esi, gfxPRC.prc_currentY

@@startLoop:
		cmp		esi, gfxPRC.prc_destY
        je      @@endLoop
		mov		gfxSpan.sp_y, esi

        ; cnt = xRight - xLeft
		; x = xLeft
		sub		eax, ebx
		mov		gfxSpan.sp_x, ebx
		mov		gfxSpan.sp_cnt, eax
		mov		eax, gfxPRC.prc_hRight
		mov		ebx, gfxPRC.prc_hLeft
		sub		eax, ebx
		jz		@@nextSpan
		mov		gfxSpan.sp_h, ebx
        fld		fpConstant65536
        fidiv	gfxSpan.sp_cnt
		mov		gfxSpan.sp_sh, eax
        jmp		DWORD PTR [gfxRC.rc_clipSpanHook]
@@calcSpanSteps:
		mov		eax, gfxRC.rc_renderSpanHook
		fimul	gfxSpan.sp_sh
		fist	gfxSpan.sp_sh
		mov		gfxRC.rc_spanClippedHook, eax
		jmp		DWORD PTR [gfxRC.rc_renderSpanHook]
@@nextSpan:
		ffree	st
		lea		edi, @@calcSpanSteps
		mov		esi, gfxPRC.prc_currentY
		mov		gfxRC.rc_spanClippedHook, edi
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		ecx, gfxPRC.prc_hRight
		mov		edx, gfxPRC.prc_hLeft
		add		esi, 1
		add		eax, gfxPRC.prc_deltaXR
		add		ebx, gfxPRC.prc_deltaXL
		add		ecx, gfxPRC.prc_deltaHR
		add		edx, gfxPRC.prc_deltaHL
		mov		gfxPRC.prc_xRight, eax
		mov		gfxPRC.prc_xLeft, ebx
		mov		gfxPRC.prc_hRight, ecx
		mov		gfxPRC.prc_hLeft, edx
		mov		edx, gfxRC.rc_stride
		mov		gfxPRC.prc_currentY, esi
		add		gfxRC.rc_rowAddr, edx
		add		gfxRC.rc_currentEdgeLine, 4
		jmp		@@startLoop
@@endLoop:
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
		PrimitiveDone popStack
PRastXH ENDP

;===============================================================
;
; PRastXUVW
;
; Perspective textured polygon rasterizer.
;
;===============================================================

PRastXUVW proc C
        public C PRastXUVW
		RasterizerSetup hasSteps
@@rasterLoop:
		fldcw	WORD PTR controlWord
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp

		; Prime the cache for gfxSpan.sp_
		mov		ecx, gfxSpan.sp_x

		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		esi, gfxPRC.prc_currentY
		fld		gfxPRC.prc_uLeft
		fld		gfxPRC.prc_vLeft
		fld		gfxPRC.prc_wLeft
		fstp	gfxSpan.sp_w
		fstp	gfxSpan.sp_v
		fstp	gfxSpan.sp_u

@@startLoop:
		; set up span

		cmp		esi, gfxPRC.prc_destY
        je      @@endLoop
		mov		gfxSpan.sp_y, esi

        ; cnt = xRight - xLeft
		; x = xLeft

        lea		ecx, @@calcSpanSteps
		sub		eax, ebx
		jz		@@nextSpan
		mov		gfxSpan.sp_x, ebx
		mov		gfxSpan.sp_cnt, eax
		mov		gfxRC.rc_spanClippedHook, ecx

        ; put  1/span length on the stack - do the divide while clipping

        fld		fpConstant65536
        fidiv	gfxSpan.sp_cnt			; st=1/cnt
        ; jump into clipper
        jmp		DWORD PTR [gfxRC.rc_clipSpanHook]
@@calcSpanSteps:

									; st(0)  st(1)  st(2)  st(3)  st(4)  st(5)
									; 1/cnt
		fld		gfxPRC.prc_uRight	; uR     1/cnt
		fsub	gfxSpan.sp_u		; uR-uL  1/cnt
		fld		gfxPRC.prc_vRight   ; vR     uR-uL  1/cnt
		fsub	gfxSpan.sp_v        ; vR-vL  uR-uL  1/cnt
		fld     gfxPRC.prc_wRight   ; wR     vR-vL  uR-uL  1/cnt
		fsub	gfxSpan.sp_w        ; wR-wL  vR-vL  uR-uL  1/cnt
		fxch	st(3)               ; 1/cnt  vR-vL  uR-uL  wR-wL
		fmul	st(1), st
		mov		eax, gfxRC.rc_renderSpanHook
		mov		ecx, gfxPRC.prc_hLeft
		add		ecx, gfxPRC.prc_hRight
		fmul	st(2), st
		shr		ecx, 1
		and		ecx, gfxRC.rc_hazeMask
		add		ecx, gfxRC.rc_shadeMap
		fmulp   st(3), st			; sv     su     sw

		fstp	gfxSpan.sp_sv
		fstp	gfxSpan.sp_su
		fstp	gfxSpan.sp_sw		; EMPTY
		mov		gfxSpan.sp_h, ecx
		mov		gfxRC.rc_spanClippedHook, eax
		jmp		DWORD PTR [gfxRC.rc_renderSpanHook]
@@nextSpan:
		ffree	st
		; add in some deltas
								; st(0)  st(1)  st(2)  st(3)  st(4)  st(5)
								; EMPTY
		fld		gfxPRC.prc_uRight
		fadd	gfxPRC.prc_deltaUR
		fld		gfxPRC.prc_vRight
		fadd	gfxPRC.prc_deltaVR
		fld		gfxPRC.prc_wRight
		fadd	gfxPRC.prc_deltaWR
		fld		gfxSpan.sp_u
		fadd	gfxPRC.prc_deltaUL
		fld		gfxSpan.sp_v
		fadd	gfxPRC.prc_deltaVL
		fld		gfxSpan.sp_w
		fadd	gfxPRC.prc_deltaWL
		mov		esi, gfxPRC.prc_currentY
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		add		esi, 1

		fstp	gfxSpan.sp_w
		fstp	gfxSpan.sp_v
		fstp	gfxSpan.sp_u
		fstp	gfxPRC.prc_wRight
		fstp	gfxPRC.prc_vRight
		fstp	gfxPRC.prc_uRight

		add		eax, gfxPRC.prc_deltaXR
		add		ebx, gfxPRC.prc_deltaXL
		mov		gfxPRC.prc_xRight, eax
		mov		gfxPRC.prc_xLeft, ebx
		mov		edx, gfxRC.rc_stride
		mov		gfxPRC.prc_currentY, esi

		; add in next row offset

		add		gfxRC.rc_rowAddr, edx
		mov		ecx, gfxPRC.prc_deltaHL
		mov		edx, gfxPRC.prc_deltaHR
		add		gfxPRC.prc_hLeft, ecx
		add		gfxPRC.prc_hRight, edx
		add		gfxRC.rc_currentEdgeLine, 4
		jmp		@@startLoop
@@endLoop:
		; restore u,v to gfxPRC
		fld		gfxSpan.sp_u
		fld		gfxSpan.sp_v
		fld		gfxSpan.sp_w
		fstp	gfxPRC.prc_wLeft
		fstp	gfxPRC.prc_vLeft
		fstp	gfxPRC.prc_uLeft
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
		PrimitiveDone popStack
PRastXUVW ENDP

;===============================================================
;
; PRastXUVWS
;
; Perspective textured polygon rasterizer.
;
;===============================================================

PRastXUVWS proc C
        public C PRastXUVWS
		RasterizerSetup hasSteps
@@rasterLoop:
		fldcw	WORD PTR controlWord
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp

		; Prime the cache for gfxSpan.sp_
		mov		ecx, gfxSpan.sp_x

		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		ecx, gfxPRC.prc_iLeft
		mov		esi, gfxPRC.prc_currentY
		mov		gfxSpan.sp_s, ecx
		fld		gfxPRC.prc_uLeft
		fld		gfxPRC.prc_vLeft
		fld		gfxPRC.prc_wLeft
		fstp	gfxSpan.sp_w
		fstp	gfxSpan.sp_v
		fstp	gfxSpan.sp_u

@@startLoop:
		; set up span

		cmp		esi, gfxPRC.prc_destY
        je      @@endLoop
		mov		gfxSpan.sp_y, esi

        ; cnt = xRight - xLeft
		; x = xLeft

        lea		ecx, @@calcSpanSteps
		sub		eax, ebx
		jz		@@noPopNextSpan
		mov		gfxSpan.sp_x, ebx
		mov		gfxSpan.sp_cnt, eax
		mov		gfxRC.rc_spanClippedHook, ecx

        ; put  1/span length on the stack - do the divide while clipping

        fld		fpConstant65536
        fidiv	gfxSpan.sp_cnt			; st=1/cnt
        ; jump into clipper
        jmp		DWORD PTR [gfxRC.rc_clipSpanHook]
@@calcSpanSteps:

									; st(0)  st(1)  st(2)  st(3)  st(4)  st(5)
									; 1/cnt
		fld		gfxPRC.prc_uRight	; uR     1/cnt
		fsub	gfxSpan.sp_u		; uR-uL  1/cnt
		fld		gfxPRC.prc_vRight   ; vR     uR-uL  1/cnt
		fsub	gfxSpan.sp_v        ; vR-vL  uR-uL  1/cnt
		fld     gfxPRC.prc_wRight   ; wR     vR-vL  uR-uL  1/cnt
		fsub	gfxSpan.sp_w        ; wR-wL  vR-vL  uR-uL  1/cnt
		fxch	st(3)               ; 1/cnt  vR-vL  uR-uL  wR-wL
		fmul	st(1), st
		mov		eax, gfxRC.rc_renderSpanHook
		mov		ecx, gfxPRC.prc_hLeft
		add		ecx, gfxPRC.prc_hRight
		fmul	st(2), st
		fild	gfxPRC.prc_iRight
		fxch	st(1)
		fmul    st(4), st			; 1/cnt  iR     sv     su     sw
		fxch	st(1)
		fisub	gfxSpan.sp_s
		shr		ecx, 1
		and		ecx, gfxRC.rc_hazeMask
		fmulp	st(1), st			; ss     sv     su     sw
		add		ecx, gfxRC.rc_shadeMap
		mov		gfxSpan.sp_h, ecx
		fistp	gfxSpan.sp_s_s

		fstp	gfxSpan.sp_sv
		fstp	gfxSpan.sp_su
		fstp	gfxSpan.sp_sw		; EMPTY
		mov		gfxRC.rc_spanClippedHook, eax
		jmp		DWORD PTR [gfxRC.rc_renderSpanHook]
@@nextSpan:
		fstp	st
@@noPopNextSpan:

		; add in some deltas
								; st(0)  st(1)  st(2)  st(3)  st(4)  st(5)
								; EMPTY
		fld		gfxPRC.prc_uRight
		fadd	gfxPRC.prc_deltaUR
		fld		gfxPRC.prc_vRight
		fadd	gfxPRC.prc_deltaVR
		fld		gfxPRC.prc_wRight
		fadd	gfxPRC.prc_deltaWR
		fld		gfxSpan.sp_u
		fadd	gfxPRC.prc_deltaUL
		fld		gfxSpan.sp_v
		fadd	gfxPRC.prc_deltaVL
		fld		gfxSpan.sp_w
		fadd	gfxPRC.prc_deltaWL
		mov		eax, gfxPRC.prc_deltaIL
		mov		ebx, gfxPRC.prc_deltaIR
		add		gfxSpan.sp_s, eax
		add		gfxPRC.prc_iRight, ebx
		mov		esi, gfxPRC.prc_currentY
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		add		esi, 1

		fstp	gfxSpan.sp_w
		fstp	gfxSpan.sp_v
		fstp	gfxSpan.sp_u
		fstp	gfxPRC.prc_wRight
		fstp	gfxPRC.prc_vRight
		fstp	gfxPRC.prc_uRight

		add		eax, gfxPRC.prc_deltaXR
		add		ebx, gfxPRC.prc_deltaXL
		mov		gfxPRC.prc_xRight, eax
		mov		gfxPRC.prc_xLeft, ebx
		mov		edx, gfxRC.rc_stride
		mov		gfxPRC.prc_currentY, esi

		; add in next row offset

		add		gfxRC.rc_rowAddr, edx
		mov		ecx, gfxPRC.prc_deltaHL
		mov		edx, gfxPRC.prc_deltaHR
		add		gfxPRC.prc_hLeft, ecx
		add		gfxPRC.prc_hRight, edx
		add		gfxRC.rc_currentEdgeLine, 4
		jmp		@@startLoop
@@endLoop:
		; restore u,v to gfxPRC
		fld		gfxSpan.sp_u
		fld		gfxSpan.sp_v
		fld		gfxSpan.sp_w
		fstp	gfxPRC.prc_wLeft
		fstp	gfxPRC.prc_vLeft
		fstp	gfxPRC.prc_uLeft
		mov		ecx, gfxSpan.sp_s
		mov		esp, _esp
		mov		gfxPRC.prc_iLeft, ecx
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
		PrimitiveDone popStack
PRastXUVWS ENDP

;===============================================================
;
; PRastXUVWH
;
; Perspective textured polygon rasterizer.
;
;===============================================================

PRastXUVWH proc C
        public C PRastXUVWH
		RasterizerSetup hasSteps
@@rasterLoop:
		fldcw	WORD PTR controlWord
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp

		; Prime the cache for gfxSpan.sp_
		mov		ecx, gfxSpan.sp_x

		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		mov		ecx, gfxPRC.prc_iLeft
		mov		esi, gfxPRC.prc_currentY
		mov		gfxSpan.sp_s, ecx
		fld		gfxPRC.prc_uLeft
		fld		gfxPRC.prc_vLeft
		fld		gfxPRC.prc_wLeft
		fstp	gfxSpan.sp_w
		fstp	gfxSpan.sp_v
		fstp	gfxSpan.sp_u

@@startLoop:
		; set up span

		cmp		esi, gfxPRC.prc_destY
        je      @@endLoop
		mov		gfxSpan.sp_y, esi

        ; cnt = xRight - xLeft
		; x = xLeft

        lea		ecx, @@calcSpanSteps
		sub		eax, ebx
		jz		@@noPopNextSpan
		mov		gfxSpan.sp_x, ebx
		mov		gfxSpan.sp_cnt, eax
		mov		gfxRC.rc_spanClippedHook, ecx

        ; put  1/span length on the stack - do the divide while clipping

        fld		fpConstant65536
        fidiv	gfxSpan.sp_cnt			; st=1/cnt
        ; jump into clipper
        jmp		DWORD PTR [gfxRC.rc_clipSpanHook]
@@calcSpanSteps:
								  	; st(0)  st(1)  st(2)  st(3)  st(4)  st(5)
									; 1/cnt
		fld		gfxPRC.prc_uRight	; uR     1/cnt
		fsub	gfxSpan.sp_u		; uR-uL  1/cnt
		fld		gfxPRC.prc_vRight   ; vR     uR-uL  1/cnt
		fsub	gfxSpan.sp_v        ; vR-vL  uR-uL  1/cnt
		fld     gfxPRC.prc_wRight   ; wR     vR-vL  uR-uL  1/cnt
		fsub	gfxSpan.sp_w        ; wR-wL  vR-vL  uR-uL  1/cnt
		fxch	st(3)               ; 1/cnt  vR-vL  uR-uL  wR-wL
		fmul	st(1), st
		mov		ebx, gfxPRC.prc_hLeft
		mov		ecx, gfxPRC.prc_hRight
		mov		eax, gfxRC.rc_renderSpanHook
		sub		ecx, ebx
		mov		gfxSpan.sp_h, ebx
		fmul	st(2), st
		mov		gfxSpan.sp_sh, ecx
		fild	gfxSpan.sp_sh
		fxch	st(1)
		fmul    st(4), st			; 1/cnt  hR-hl  sv     su     sw
		fxch	st(1)
		fmulp	st(1), st			; ss     sv     su     sw
		fistp	gfxSpan.sp_sh

		fstp	gfxSpan.sp_sv
		fstp	gfxSpan.sp_su
		fstp	gfxSpan.sp_sw		; EMPTY
		mov		gfxRC.rc_spanClippedHook, eax
		jmp		DWORD PTR [gfxRC.rc_renderSpanHook]
@@nextSpan:
		fstp	st
@@noPopNextSpan:

		; add in some deltas
								; st(0)  st(1)  st(2)  st(3)  st(4)  st(5)
								; EMPTY
		fld		gfxPRC.prc_uRight
		fadd	gfxPRC.prc_deltaUR
		fld		gfxPRC.prc_vRight
		fadd	gfxPRC.prc_deltaVR
		fld		gfxPRC.prc_wRight
		fadd	gfxPRC.prc_deltaWR
		fld		gfxSpan.sp_u
		fadd	gfxPRC.prc_deltaUL
		fld		gfxSpan.sp_v
		fadd	gfxPRC.prc_deltaVL
		fld		gfxSpan.sp_w
		fadd	gfxPRC.prc_deltaWL
		mov		eax, gfxPRC.prc_deltaIL
		mov		ebx, gfxPRC.prc_deltaIR
		add		gfxSpan.sp_s, eax
		add		gfxPRC.prc_iRight, ebx
		mov		esi, gfxPRC.prc_currentY
		mov		eax, gfxPRC.prc_xRight
		mov		ebx, gfxPRC.prc_xLeft
		add		esi, 1

		fstp	gfxSpan.sp_w
		fstp	gfxSpan.sp_v
		fstp	gfxSpan.sp_u
		fstp	gfxPRC.prc_wRight
		fstp	gfxPRC.prc_vRight
		fstp	gfxPRC.prc_uRight

		add		eax, gfxPRC.prc_deltaXR
		add		ebx, gfxPRC.prc_deltaXL
		mov		gfxPRC.prc_xRight, eax
		mov		gfxPRC.prc_xLeft, ebx
		mov		edx, gfxRC.rc_stride
		mov		gfxPRC.prc_currentY, esi

		; add in next row offset

		add		gfxRC.rc_rowAddr, edx
		mov		ecx, gfxPRC.prc_deltaHL
		mov		edx, gfxPRC.prc_deltaHR
		add		gfxPRC.prc_hLeft, ecx
		add		gfxPRC.prc_hRight, edx
		add		gfxRC.rc_currentEdgeLine, 4
		jmp		@@startLoop
@@endLoop:
		; restore u,v to gfxPRC
		fld		gfxSpan.sp_u
		fld		gfxSpan.sp_v
		fld		gfxSpan.sp_w
		fstp	gfxPRC.prc_wLeft
		fstp	gfxPRC.prc_vLeft
		fstp	gfxPRC.prc_uLeft
		mov		ecx, gfxSpan.sp_s
		mov		esp, _esp
		mov		gfxPRC.prc_iLeft, ecx
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
		PrimitiveDone popStack
PRastXUVWH ENDP

END
