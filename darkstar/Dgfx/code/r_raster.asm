;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;±   
;± $Workfile:   r_raster.asm  $
;± $Version$
;± $Revision:   1.1  $
;±   
;± DESCRIPTION:  Rasterization routines for rectangles.
;±   
;± (c) Copyright 1995, Dynamix Inc.   All rights reserved.
;±   
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

.486
.MODEL FLAT, syscall
.data

INCLUDE g_contxt.inc
INCLUDE g_raster.inc

_esp	dd ?
;------------------------------ Code-patching helper routine...

IFDEF __MASM__
 ARG equ
ELSE
 LOCALS
ENDIF
.code

RRastX proc C
		public C RRastX
		mov		eax, gfxRC.rc_stride
		mov		ecx, gfxPRC.prc_currentY
		imul	ecx
		add		eax, gfxRC.rc_rowAddr
		lea		edx, @@endLoop
		mov		gfxRC.rc_rowAddr, eax
		mov		gfxRC.rc_primitiveDoneHook, edx
		mov		eax, gfxRC.rc_renderSpanHook
		lea		edx, @@rasterLoop
		mov		gfxRC.rc_spanClippedHook, eax
		mov		gfxRC.rc_rasterLoop, edx
		mov		edx, gfxRC.rc_nextEdgeHook
		lea		eax, @@nextSpan
		mov		gfxRC.rc_spanDoneHook, eax
		test	edx, edx
		jnz		@@hasEdgeClip
		mov		gfxRC.rc_nextEdgeHook, eax
		ret
@@hasEdgeClip:
		mov		edx, gfxRC.rc_edgeList
		mov		ecx, gfxPRC.prc_currentY
		mov		eax, (EDGE_LIST PTR [edx]).e_list
		sub		ecx, (EDGE_LIST PTR [edx]).e_top_y
		lea		eax, [eax+ecx*4]
		mov		gfxRC.rc_currentEdgeLine, eax
		ret
@@rasterLoop:
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp
		mov		esi, gfxPRC.prc_currentY
@@startLoop:
		cmp		esi, gfxPRC.prc_destY
		je		@@endLoop
		mov		gfxSpan.sp_y, esi
		jmp		DWORD PTR [gfxRC.rc_clipSpanHook]
@@nextSpan:	
		mov		esi, gfxPRC.prc_currentY
		mov		edx, gfxRC.rc_rowAddr
		mov		ecx, gfxRC.rc_currentEdgeLine
		mov		ebx, gfxRC.rc_stride
		add		esi, 1
		add		edx, ebx
		add		ecx, 4
		mov		gfxPRC.prc_currentY, esi
		mov		gfxRC.rc_rowAddr, edx
		mov		gfxRC.rc_currentEdgeLine, ecx
		jmp		@@startLoop		
@@endLoop:
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
RRastX ENDP
END
