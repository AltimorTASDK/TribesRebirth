
;================================================================
;
; $Workfile:   rn_fltn.asm  $
; $Version$
; $Revision: $
;
; DESCRIPTION:
;
; (c) Copyright Dynamix Inc.   All rights reserved.
;================================================================

.486p
.MODEL FLAT, SYSCALL

IFDEF __MASM__
  ARG equ
ENDIF

.DATA

include g_contxt.inc
INCLUDE g_macros.inc
include m_vec.inc

normc				dq		0.0
u1					dq		0.0
u2					dq		0.0
u3					dq		0.0
v1					dq		0.0
v2					dq		0.0
v3					dq		0.0

GFX_MAX_POLY_VERTEX  equ   100

controlWord			dd		0F7FH	; round toward zero
fpcnst65536			dd		65536.0
fpcnstnscale		dd		-65536.0
fpcnstscale			dd		65536.0 ; used to be 16M, changed causa tex coords.
fpcnst1div65536		dd		1.52587890625e-5
fpcnst1				dd		1.0
fpcnstneg1			dd		-1.0
fpcnst0				dd		0.0
testval				dd		0
_esp				dd		0

redScale			dd		0.22
greenScale			dd		0.67
blueScale			dd		0.11

_vList				dd		GFX_MAX_POLY_VERTEX dup	(?)
_cList				dd		GFX_MAX_POLY_VERTEX dup	(?)
_tList				dd		GFX_MAX_POLY_VERTEX dup	(?)
_kList				dd		GFX_MAX_POLY_VERTEX dup	(?)
_vertexCount		dd		0
nextLefts			dd 		GFX_MAX_POLY_VERTEX dup	(?)
nextRights			dd		GFX_MAX_POLY_VERTEX dup	(?)
iy					dd		GFX_MAX_POLY_VERTEX dup	(?)

left	dd ?
right	dd ?
bot		dd ?
top		dd ?
c0		dd ?
t0		dd ?
nextSpan	dd ?
saveBase	dd ?
saveLastSpan dd ?
saveShade dd ?

ColorInfo STRUC
	ci_haze		dd ?
	ci_red		dd ?
	ci_green	dd ?
	ci_blue		dd ?
	ci_alpha	dd ?
ColorInfo ENDS


IFDEF __MASM__
	EXTRN _spanListPtr:DWORD
ELSE
	EXTRN _spanListPtr
ENDIF

PUBLIC _vList
PUBLIC _cList
PUBLIC _tList
PUBLIC _kList
PUBLIC _vertexCount

.CODE

DeltaCalcTriUVQWS PROC C,
		ARG pdc:DWORD
		public C DeltaCalcTriUVQWS

		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp
		
		mov		ebp, pdc

		fldcw	WORD PTR controlWord
		mov		eax, _vList[0]
		mov		ebx, _vList[4]
		mov		ecx, _vList[8]

		fld		(Point3 PTR [ecx]).x
		fsub	(Point3 PTR [ebx]).x
		fld		(Point3 PTR [ecx]).y
		fsub	(Point3 PTR [ebx]).y
		fld		(Point3 PTR [eax]).x
		fsub	(Point3 PTR [ebx]).x
		fld		(Point3 PTR [eax]).y
		fsub	(Point3 PTR [ebx]).y
		; st = v2, v1, u2, u1
		fxch	st(3)
		fst		u1
		fxch	st(1)
		fst		v1
		; st = v1, u1, u2, v2
		fxch	st(2)
		fst		u2
		fxch	st(3)
		fst		v2
		; st = v2, u1, v1, u2
		fmulp	st(1), st
		; u1v2
		fxch	st(1)
		fmulp	st(2), st
		; st = u1v2, u2v1
		fxch	st(1)
		fsubp	st(1), st

		fld		fpcnst0
		fcomp	st(1)
		fnstsw	ax
		fstp	normc
		; if st(1) was <= 0, then return false.
		sahf
		jb		@@notBackfaced
		mov		eax, 0
		jmp		@@end  ;this is the test for normc = 0  - collinear points, etc.
@@notBackfaced:
		mov		eax, _vList[0]

		; okie.. normc is outta da way.
		fld		(Point3 PTR [ecx]).z
		fld		(Point3 PTR [ebx]).z
		fld		(Point3 PTR [eax]).z
		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdx

		mov		edx, _cList[0]
		mov		edi, _cList[4]
		mov		esi, _cList[8]

		fld		(ColorInfo PTR [esi]).ci_red
		fmul	redScale
		fld		(ColorInfo PTR [esi]).ci_green
		fmul	greenScale
		fld		(ColorInfo PTR [esi]).ci_blue
		fmul	blueScale
		fxch	st(2)
 		faddp	st(1), st
		faddp	st(1), st

		fld		(ColorInfo PTR [edi]).ci_red
		fmul	redScale
		fld		(ColorInfo PTR [edi]).ci_green
		fmul	greenScale
		fld		(ColorInfo PTR [edi]).ci_blue
		fmul	blueScale
		fxch	st(2)
 		faddp	st(1), st
		faddp	st(1), st

		fld		(ColorInfo PTR [edx]).ci_red
		fmul	redScale
		fld		(ColorInfo PTR [edx]).ci_green
		fmul	greenScale
		fld		(ColorInfo PTR [edx]).ci_blue
		fmul	blueScale
		fxch	st(2)
 		faddp	st(1), st
		faddp	st(1), st

		fst		saveShade

		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdx

		mov		edx, _tList[0]
		mov		edi, _tList[4]
		mov		esi, _tList[8]

		; now calc u3, v3 for each of the step values we want...
		; u3 = p2->u * p2->w - p1->u * p1->w
		; v3 = p0->u * p0->w - p1->u * p1->w
		
		fld		(Point2 PTR [esi]).x
		fmul	(Point3 PTR [ecx]).z
		fld		(Point2 PTR [edi]).x
		fmul	(Point3 PTR [ebx]).z
		fld		(Point2 PTR [edx]).x
		fmul	(Point3 PTR [eax]).z
		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudx

		fld		(Point2 PTR [esi]).y
		fmul	(Point3 PTR [ecx]).z
		fld		(Point2 PTR [edi]).y
		fmul	(Point3 PTR [ebx]).z
		fld		(Point2 PTR [edx]).y
		fmul	(Point3 PTR [eax]).z
		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdx

		; now we divide everything by -normc.

		fld		fpcnstneg1
		fdiv	normc
		fld		fpcnstscale
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudx
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fmul	st, st(2)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdx
		fmul	st, st(3)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdy
		fmulp	st(4), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdy

		; st = -1/normc

		fld		(GFXPolyDrawContext PTR [ebp]).pdc_shadeScale
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dsdx
		fmul	st, st(1)
		fxch	st(1)
		fmul	(GFXPolyDrawContext PTR [ebp]).pdc_dsdy
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdy

		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fmulp	st(2), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		
		
		; calculate starting coordinate values...

		mov		eax, _vList[0]

		mov		ecx, _cList[0]

		fld		saveShade ; we saved this earlier
		fmul	(GFXPolyDrawContext PTR [ebp]).pdc_shadeScale
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dsdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dsdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_s0

		mov		ecx, _tList[0]

		fld		(Point2 PTR [ecx]).x
		fmul	(Point3 PTR [eax]).z
		fmul	fpcnstscale
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_u0
										
		fld		(Point2 PTR [ecx]).y
		fmul	(Point3 PTR [eax]).z
		fmul	fpcnstscale
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_v0

		fld		(Point3 PTR [eax]).z
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_w0
		mov		eax, 1
@@end:
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
DeltaCalcTriUVQWS ENDP

DeltaCalcTriUVQWH PROC C,
		ARG pdc:DWORD
		public C DeltaCalcTriUVQWH

		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp
		
		mov		ebp, pdc

		fldcw	WORD PTR controlWord
		mov		eax, _vList[0]
		mov		ebx, _vList[4]
		mov		ecx, _vList[8]

		fld		(Point3 PTR [ecx]).x
		fsub	(Point3 PTR [ebx]).x
		fld		(Point3 PTR [ecx]).y
		fsub	(Point3 PTR [ebx]).y
		fld		(Point3 PTR [eax]).x
		fsub	(Point3 PTR [ebx]).x
		fld		(Point3 PTR [eax]).y
		fsub	(Point3 PTR [ebx]).y
		; st = v2, v1, u2, u1
		fxch	st(3)
		fst		u1
		fxch	st(1)
		fst		v1
		; st = v1, u1, u2, v2
		fxch	st(2)
		fst		u2
		fxch	st(3)
		fst		v2
		; st = v2, u1, v1, u2
		fmulp	st(1), st
		; u1v2
		fxch	st(1)
		fmulp	st(2), st
		; st = u1v2, u2v1
		fxch	st(1)
		fsubp	st(1), st
		fld		fpcnst0
		fcomp	st(1)
		fnstsw	ax
		fstp	normc
		; if st(1) was <= 0, then return false.
		sahf
		jb		@@notBackfaced
		mov		eax, 0
		jmp		@@end  ;this is the test for normc = 0  - collinear points, etc.
@@notBackfaced:
		mov		eax, _vList[0]

		; okie.. normc is outta da way.
		fld		(Point3 PTR [ecx]).z
		fld		(Point3 PTR [ebx]).z
		fld		(Point3 PTR [eax]).z
		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdx

		mov		edx, _cList[0]
		mov		edi, _cList[4]
		mov		esi, _cList[8]

		fld		(ColorInfo PTR [esi]).ci_haze
		fld		(ColorInfo PTR [edi]).ci_haze
		fld		(ColorInfo PTR [edx]).ci_haze

		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdx

		mov		edx, _tList[0]
		mov		edi, _tList[4]
		mov		esi, _tList[8]

		; now calc u3, v3 for each of the step values we want...
		; u3 = p2->u * p2->w - p1->u * p1->w
		; v3 = p0->u * p0->w - p1->u * p1->w
		
		fld		(Point2 PTR [esi]).x
		fmul	(Point3 PTR [ecx]).z
		fld		(Point2 PTR [edi]).x
		fmul	(Point3 PTR [ebx]).z
		fld		(Point2 PTR [edx]).x
		fmul	(Point3 PTR [eax]).z
		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudx

		fld		(Point2 PTR [esi]).y
		fmul	(Point3 PTR [ecx]).z
		fld		(Point2 PTR [edi]).y
		fmul	(Point3 PTR [ebx]).z
		fld		(Point2 PTR [edx]).y
		fmul	(Point3 PTR [eax]).z
		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdx

		; now we divide everything by -normc.

		fld		fpcnstneg1
		fdiv	normc
		fld		fpcnstscale
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudx
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fmul	st, st(2)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdx
		fmul	st, st(3)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdy
		fmulp	st(4), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdy

		; st = -1/normc

		fld		(GFXPolyDrawContext PTR [ebp]).pdc_shadeScale
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dsdx
		fmul	st, st(1)
		fxch	st(1)
		fmul	(GFXPolyDrawContext PTR [ebp]).pdc_dsdy
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdy

		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fmulp	st(2), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		
		
		; calculate starting coordinate values...

		mov		eax, _vList[0]

		mov		ecx, _cList[0]

		fld		(ColorInfo PTR [ecx]).ci_haze ; we saved this earlier
		fmul	(GFXPolyDrawContext PTR [ebp]).pdc_shadeScale
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dsdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dsdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_s0

		mov		ecx, _tList[0]

		fld		(Point2 PTR [ecx]).x
		fmul	(Point3 PTR [eax]).z
		fmul	fpcnstscale
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_u0
										
		fld		(Point2 PTR [ecx]).y
		fmul	(Point3 PTR [eax]).z
		fmul	fpcnstscale
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_v0

		fld		(Point3 PTR [eax]).z
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_w0
		mov		eax, 1
@@end:
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
DeltaCalcTriUVQWH ENDP

DeltaCalcTriUVWS PROC C,
		ARG pdc:DWORD
		public C DeltaCalcTriUVWS

		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp
		
		mov		ebp, pdc

		fldcw	WORD PTR controlWord
		mov		eax, _vList[0]
		mov		ebx, _vList[4]
		mov		ecx, _vList[8]

		fld		(Point3 PTR [ecx]).x
		fsub	(Point3 PTR [ebx]).x
		fld		(Point3 PTR [ecx]).y
		fsub	(Point3 PTR [ebx]).y
		fld		(Point3 PTR [eax]).x
		fsub	(Point3 PTR [ebx]).x
		fld		(Point3 PTR [eax]).y
		fsub	(Point3 PTR [ebx]).y
		; st = v2, v1, u2, u1
		fxch	st(3)
		fst		u1
		fxch	st(1)
		fst		v1
		; st = v1, u1, u2, v2
		fxch	st(2)
		fst		u2
		fxch	st(3)
		fst		v2
		; st = v2, u1, v1, u2
		fmulp	st(1), st
		; u1v2
		fxch	st(1)
		fmulp	st(2), st
		; st = u1v2, u2v1
		fxch	st(1)
		fsubp	st(1), st
		fld		fpcnst0
		fcomp	st(1)
		fnstsw	ax
		fstp	normc
		; if st(1) was <= 0, then return false.
		sahf
		jb		@@notBackfaced
		mov		eax, 0
		jmp		@@end  ;this is the test for normc = 0  - collinear points, etc.
@@notBackfaced:
		mov		eax, _vList[0]

		; okie.. normc is outta da way.
		fld		(Point3 PTR [ecx]).z
		fld		(Point3 PTR [ebx]).z
		fld		(Point3 PTR [eax]).z
		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdx

		mov		edx, _cList[0]
		mov		edi, _cList[4]
		mov		esi, _cList[8]

		fld		(ColorInfo PTR [esi]).ci_red
		fmul	redScale
		fld		(ColorInfo PTR [esi]).ci_green
		fmul	greenScale
		fld		(ColorInfo PTR [esi]).ci_blue
		fmul	blueScale
		fxch	st(2)
 		faddp	st(1), st
		faddp	st(1), st

		fld		(ColorInfo PTR [edi]).ci_red
		fmul	redScale
		fld		(ColorInfo PTR [edi]).ci_green
		fmul	greenScale
		fld		(ColorInfo PTR [edi]).ci_blue
		fmul	blueScale
		fxch	st(2)
 		faddp	st(1), st
		faddp	st(1), st

		fld		(ColorInfo PTR [edx]).ci_red
		fmul	redScale
		fld		(ColorInfo PTR [edx]).ci_green
		fmul	greenScale
		fld		(ColorInfo PTR [edx]).ci_blue
		fmul	blueScale
		fxch	st(2)
 		faddp	st(1), st
		faddp	st(1), st

		fst		saveShade

		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdx

		; now calc u3, v3 for each of the step values we want...
		; u3 = p2->u * p2->w - p1->u * p1->w
		; v3 = p0->u * p0->w - p1->u * p1->w
		
		mov		edx, _tList[0]
		mov		edi, _tList[4]
		mov		esi, _tList[8]

		fld		(Point2 PTR [esi]).x
		fld		(Point2 PTR [edi]).x
		fld		(Point2 PTR [edx]).x
		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudx

		fld		(Point2 PTR [esi]).y
		fld		(Point2 PTR [edi]).y
		fld		(Point2 PTR [edx]).y
		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdx

		; now we divide everything by -normc.

		fld		fpcnstneg1
		fdiv	normc
		fld		fpcnstscale
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudx
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fmul	st, st(2)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdx
		fmul	st, st(3)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdy
		fmulp	st(4), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdy

		; st = -1/normc

		fld		(GFXPolyDrawContext PTR [ebp]).pdc_shadeScale
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dsdx
		fmul	st, st(1)
		fxch	st(1)
		fmul	(GFXPolyDrawContext PTR [ebp]).pdc_dsdy
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdy

		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fmulp	st(2), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		
		; calculate starting coordinate values...
		mov		eax, _vList[0]
		mov		ecx, _cList[0]

		fld		saveShade ; we saved this earlier
		fmul	(GFXPolyDrawContext PTR [ebp]).pdc_shadeScale
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dsdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dsdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_s0

		mov		ecx, _tList[0]

		fld		(Point2 PTR [ecx]).x
		fmul	fpcnstscale
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_u0
										
		fld		(Point2 PTR [ecx]).y
		fmul	fpcnstscale
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_v0

		fld		(Point3 PTR [eax]).z
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_w0
		mov		eax, 1
@@end:
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
DeltaCalcTriUVWS ENDP

DeltaCalcTriUVWH PROC C,
		ARG pdc:DWORD
		public C DeltaCalcTriUVWH

		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp
		
		mov		ebp, pdc

		fldcw	WORD PTR controlWord
		mov		eax, _vList[0]
		mov		ebx, _vList[4]
		mov		ecx, _vList[8]

		fld		(Point3 PTR [ecx]).x
		fsub	(Point3 PTR [ebx]).x
		fld		(Point3 PTR [ecx]).y
		fsub	(Point3 PTR [ebx]).y
		fld		(Point3 PTR [eax]).x
		fsub	(Point3 PTR [ebx]).x
		fld		(Point3 PTR [eax]).y
		fsub	(Point3 PTR [ebx]).y
		; st = v2, v1, u2, u1
		fxch	st(3)
		fst		u1
		fxch	st(1)
		fst		v1
		; st = v1, u1, u2, v2
		fxch	st(2)
		fst		u2
		fxch	st(3)
		fst		v2
		; st = v2, u1, v1, u2
		fmulp	st(1), st
		; u1v2
		fxch	st(1)
		fmulp	st(2), st
		; st = u1v2, u2v1
		fxch	st(1)
		fsubp	st(1), st

		fld		fpcnst0
		fcomp	st(1)
		fnstsw	ax
		fstp	normc
		; if st(1) was <= 0, then return false.
		sahf
		jb		@@notBackfaced
		mov		eax, 0
		jmp		@@end  ;this is the test for normc = 0  - collinear points, etc.
@@notBackfaced:
		mov		eax, _vList[0]

		; okie.. normc is outta da way.
		fld		(Point3 PTR [ecx]).z
		fld		(Point3 PTR [ebx]).z
		fld		(Point3 PTR [eax]).z
		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdx

		mov		edx, _cList[0]
		mov		edi, _cList[4]
		mov		esi, _cList[8]

		fld		(ColorInfo PTR [esi]).ci_haze
		fld		(ColorInfo PTR [edi]).ci_haze
		fld		(ColorInfo PTR [edx]).ci_haze

		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdx

		; now calc u3, v3 for each of the step values we want...
		; u3 = p2->u * p2->w - p1->u * p1->w
		; v3 = p0->u * p0->w - p1->u * p1->w
		
		mov		edx, _tList[0]
		mov		edi, _tList[4]
		mov		esi, _tList[8]

		fld		(Point2 PTR [esi]).x
		fld		(Point2 PTR [edi]).x
		fld		(Point2 PTR [edx]).x
		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudx

		fld		(Point2 PTR [esi]).y
		fld		(Point2 PTR [edi]).y
		fld		(Point2 PTR [edx]).y
		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdx

		; now we divide everything by -normc.

		fld		fpcnstneg1
		fdiv	normc
		fld		fpcnstscale
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudx
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fmul	st, st(2)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdx
		fmul	st, st(3)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdy
		fmulp	st(4), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdy

		; st = -1/normc

		fld		(GFXPolyDrawContext PTR [ebp]).pdc_shadeScale
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dsdx
		fmul	st, st(1)
		fxch	st(1)
		fmul	(GFXPolyDrawContext PTR [ebp]).pdc_dsdy
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdy

		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fmulp	st(2), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		
		; calculate starting coordinate values...
		mov		eax, _vList[0]
		mov		ecx, _cList[0]

		fld		(ColorInfo PTR [ecx]).ci_haze
		fmul	(GFXPolyDrawContext PTR [ebp]).pdc_shadeScale
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dsdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dsdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_s0

		mov		ecx, _tList[0]

		fld		(Point2 PTR [ecx]).x
		fmul	fpcnstscale
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_u0
										
		fld		(Point2 PTR [ecx]).y
		fmul	fpcnstscale
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_v0

		fld		(Point3 PTR [eax]).z
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_w0
		mov		eax, 1
@@end:
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
DeltaCalcTriUVWH ENDP

DeltaCalcTriWS PROC C,
		ARG pdc:DWORD
		public C DeltaCalcTriWS

		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp
		
		mov		ebp, pdc

		fldcw	WORD PTR controlWord
		mov		eax, _vList[0]
		mov		ebx, _vList[4]
		mov		ecx, _vList[8]

		fld		(Point3 PTR [ecx]).x
		fsub	(Point3 PTR [ebx]).x
		fld		(Point3 PTR [ecx]).y
		fsub	(Point3 PTR [ebx]).y
		fld		(Point3 PTR [eax]).x
		fsub	(Point3 PTR [ebx]).x
		fld		(Point3 PTR [eax]).y
		fsub	(Point3 PTR [ebx]).y
		; st = v2, v1, u2, u1
		fxch	st(3)
		fst		u1
		fxch	st(1)
		fst		v1
		; st = v1, u1, u2, v2
		fxch	st(2)
		fst		u2
		fxch	st(3)
		fst		v2
		; st = v2, u1, v1, u2
		fmulp	st(1), st
		; u1v2
		fxch	st(1)
		fmulp	st(2), st
		; st = u1v2, u2v1
		fxch	st(1)
		fsubp	st(1), st
		fld		fpcnst0
		fcomp	st(1)
		fnstsw	ax
		fstp	normc
		; if st(1) was <= 0, then return false.
		sahf
		jb		@@notBackfaced
		mov		eax, 0
		jmp		@@end  ;this is the test for normc = 0  - collinear points, etc.
@@notBackfaced:
		mov		eax, _vList[0]

		; okie.. normc is outta da way.
		fld		(Point3 PTR [ecx]).z
		fld		(Point3 PTR [ebx]).z
		fld		(Point3 PTR [eax]).z
		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdx

		mov		edx, _cList[0]
		mov		edi, _cList[4]
		mov		esi, _cList[8]

		fld		(ColorInfo PTR [esi]).ci_red
		fmul	redScale
		fld		(ColorInfo PTR [esi]).ci_green
		fmul	greenScale
		fld		(ColorInfo PTR [esi]).ci_blue
		fmul	blueScale
		fxch	st(2)
 		faddp	st(1), st
		faddp	st(1), st

		fld		(ColorInfo PTR [edi]).ci_red
		fmul	redScale
		fld		(ColorInfo PTR [edi]).ci_green
		fmul	greenScale
		fld		(ColorInfo PTR [edi]).ci_blue
		fmul	blueScale
		fxch	st(2)
 		faddp	st(1), st
		faddp	st(1), st

		fld		(ColorInfo PTR [edx]).ci_red
		fmul	redScale
		fld		(ColorInfo PTR [edx]).ci_green
		fmul	greenScale
		fld		(ColorInfo PTR [edx]).ci_blue
		fmul	blueScale
		fxch	st(2)
 		faddp	st(1), st
		faddp	st(1), st

		fst		saveShade

		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdx

		; now we divide everything by -normc.

		fld		fpcnstneg1
		fdiv	normc

		; st = -1/normc

		fld		(GFXPolyDrawContext PTR [ebp]).pdc_shadeScale
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dsdx
		fmul	st, st(1)
		fxch	st(1)
		fmul	(GFXPolyDrawContext PTR [ebp]).pdc_dsdy
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdy

		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fmulp	st(2), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		
		; calculate starting coordinate values...
		mov		eax, _vList[0]
		mov		ecx, _cList[0]

		fld		saveShade ; we saved this earlier
		fmul	(GFXPolyDrawContext PTR [ebp]).pdc_shadeScale
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dsdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dsdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_s0

		fld		(Point3 PTR [eax]).z
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_w0
		mov		eax, 1
@@end:
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
DeltaCalcTriWS ENDP

DeltaCalcTriWH PROC C,
		ARG pdc:DWORD
		public C DeltaCalcTriWH

		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp
		
		mov		ebp, pdc

		fldcw	WORD PTR controlWord
		mov		eax, _vList[0]
		mov		ebx, _vList[4]
		mov		ecx, _vList[8]

		fld		(Point3 PTR [ecx]).x
		fsub	(Point3 PTR [ebx]).x
		fld		(Point3 PTR [ecx]).y
		fsub	(Point3 PTR [ebx]).y
		fld		(Point3 PTR [eax]).x
		fsub	(Point3 PTR [ebx]).x
		fld		(Point3 PTR [eax]).y
		fsub	(Point3 PTR [ebx]).y
		; st = v2, v1, u2, u1
		fxch	st(3)
		fst		u1
		fxch	st(1)
		fst		v1
		; st = v1, u1, u2, v2
		fxch	st(2)
		fst		u2
		fxch	st(3)
		fst		v2
		; st = v2, u1, v1, u2
		fmulp	st(1), st
		; u1v2
		fxch	st(1)
		fmulp	st(2), st
		; st = u1v2, u2v1
		fxch	st(1)
		fsubp	st(1), st
		fld		fpcnst0
		fcomp	st(1)
		fnstsw	ax
		fstp	normc
		; if st(1) was <= 0, then return false.
		sahf
		jb		@@notBackfaced
		mov		eax, 0
		jmp		@@end  ;this is the test for normc = 0  - collinear points, etc.
@@notBackfaced:
		mov		eax, _vList[0]

		; okie.. normc is outta da way.
		fld		(Point3 PTR [ecx]).z
		fld		(Point3 PTR [ebx]).z
		fld		(Point3 PTR [eax]).z
		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdx

		mov		edx, _cList[0]
		mov		edi, _cList[4]
		mov		esi, _cList[8]

		fld		(ColorInfo PTR [esi]).ci_haze
		fld		(ColorInfo PTR [edi]).ci_haze
		fld		(ColorInfo PTR [edx]).ci_haze

		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdx

		; now we divide everything by -normc.

		fld		fpcnstneg1
		fdiv	normc

		; st = -1/normc

		fld		(GFXPolyDrawContext PTR [ebp]).pdc_shadeScale
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dsdx
		fmul	st, st(1)
		fxch	st(1)
		fmul	(GFXPolyDrawContext PTR [ebp]).pdc_dsdy
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dsdy

		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fmulp	st(2), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		
		; calculate starting coordinate values...
		mov		eax, _vList[0]
		mov		ecx, _cList[0]

		fld		(ColorInfo PTR [ecx]).ci_haze
		fmul	(GFXPolyDrawContext PTR [ebp]).pdc_shadeScale
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dsdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dsdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_s0

		fld		(Point3 PTR [eax]).z
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_w0
		mov		eax, 1
@@end:
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
DeltaCalcTriWH ENDP

DeltaCalcTriUVW PROC C,
		ARG pdc:DWORD
		public C DeltaCalcTriUVW

		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp
		
		mov		ebp, pdc

		fldcw	WORD PTR controlWord
		mov		eax, _vList[0]
		mov		ebx, _vList[4]
		mov		ecx, _vList[8]
		mov		edx, _tList[0]
		mov		edi, _tList[4]
		mov		esi, _tList[8]

		fld		(Point3 PTR [ecx]).x
		fsub	(Point3 PTR [ebx]).x
		fld		(Point3 PTR [ecx]).y
		fsub	(Point3 PTR [ebx]).y
		fld		(Point3 PTR [eax]).x
		fsub	(Point3 PTR [ebx]).x
		fld		(Point3 PTR [eax]).y
		fsub	(Point3 PTR [ebx]).y
		; st = v2, v1, u2, u1
		fxch	st(3)
		fst		u1
		fxch	st(1)
		fst		v1
		; st = v1, u1, u2, v2
		fxch	st(2)
		fst		u2
		fxch	st(3)
		fst		v2
		; st = v2, u1, v1, u2
		fmulp	st(1), st
		; u1v2
		fxch	st(1)
		fmulp	st(2), st
		; st = u1v2, u2v1
		fxch	st(1)
		fsubp	st(1), st

		fld		fpcnst0
		fcomp	st(1)
		fnstsw	ax
		fstp	normc
		; if st(1) was <= 0, then return false.
		sahf
		jb		@@notBackfaced
		mov		eax, 0
		jmp		@@end  ;this is the test for normc = 0  - collinear points, etc.
@@notBackfaced:
		mov		eax, _vList[0]

		; okie.. normc is outta da way.
		fld		(Point3 PTR [ecx]).z
		fld		(Point3 PTR [ebx]).z
		fld		(Point3 PTR [eax]).z
		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdx

		; now calc u3, v3 for each of the step values we want...
		; u3 = p2->u * p2->w - p1->u * p1->w
		; v3 = p0->u * p0->w - p1->u * p1->w
		
		fld		(Point2 PTR [esi]).x
		fld		(Point2 PTR [edi]).x
		fld		(Point2 PTR [edx]).x
		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudx

		fld		(Point2 PTR [esi]).y
		fld		(Point2 PTR [edi]).y
		fld		(Point2 PTR [edx]).y
		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdx

		; now we divide everything by -normc.

		fld		fpcnstneg1
		fdiv	normc
		fld		fpcnstscale
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudx
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fmul	st, st(2)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdx
		fmul	st, st(3)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdy
		fmulp	st(4), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdy

		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fmulp	st(2), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		
		; calculate starting coordinate values...
		mov		eax, _vList[0]
		mov		ecx, _tList[0]

		fld		(Point2 PTR [ecx]).x
		fmul	fpcnstscale
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_u0
										
		fld		(Point2 PTR [ecx]).y
		fmul	fpcnstscale
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_v0

		fld		(Point3 PTR [eax]).z
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_w0
		mov		eax, 1
@@end:
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
DeltaCalcTriUVW ENDP

DeltaCalcTriUVQW PROC C,
		ARG pdc:DWORD
		public C DeltaCalcTriUVQW

		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp
		
		mov		ebp, pdc

		fldcw	WORD PTR controlWord
		mov		eax, _vList[0]
		mov		ebx, _vList[4]
		mov		ecx, _vList[8]
		mov		edx, _tList[0]
		mov		edi, _tList[4]
		mov		esi, _tList[8]

		fld		(Point3 PTR [ecx]).x
		fsub	(Point3 PTR [ebx]).x
		fld		(Point3 PTR [ecx]).y
		fsub	(Point3 PTR [ebx]).y
		fld		(Point3 PTR [eax]).x
		fsub	(Point3 PTR [ebx]).x
		fld		(Point3 PTR [eax]).y
		fsub	(Point3 PTR [ebx]).y
		; st = v2, v1, u2, u1
		fxch	st(3)
		fst		u1
		fxch	st(1)
		fst		v1
		; st = v1, u1, u2, v2
		fxch	st(2)
		fst		u2
		fxch	st(3)
		fst		v2
		; st = v2, u1, v1, u2
		fmulp	st(1), st
		; u1v2
		fxch	st(1)
		fmulp	st(2), st
		; st = u1v2, u2v1
		fxch	st(1)
		fsubp	st(1), st
		fld		fpcnst0
		fcomp	st(1)
		fnstsw	ax
		fstp	normc
		; if st(1) was <= 0, then return false.
		sahf
		jb		@@notBackfaced
		mov		eax, 0
		jmp		@@end  ;this is the test for normc = 0  - collinear points, etc.
@@notBackfaced:
		mov		eax, _vList[0]

		; okie.. normc is outta da way.
		fld		(Point3 PTR [ecx]).z
		fld		(Point3 PTR [ebx]).z
		fld		(Point3 PTR [eax]).z
		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdx

		; now calc u3, v3 for each of the step values we want...
		; u3 = p2->u * p2->w - p1->u * p1->w
		; v3 = p0->u * p0->w - p1->u * p1->w
		
		fld		(Point2 PTR [esi]).x
		fmul	(Point3 PTR [ecx]).z
		fld		(Point2 PTR [edi]).x
		fmul	(Point3 PTR [ebx]).z
		fld		(Point2 PTR [edx]).x
		fmul	(Point3 PTR [eax]).z
		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudx

		fld		(Point2 PTR [esi]).y
		fmul	(Point3 PTR [ecx]).z
		fld		(Point2 PTR [edi]).y
		fmul	(Point3 PTR [ebx]).z
		fld		(Point2 PTR [edx]).y
		fmul	(Point3 PTR [eax]).z
		fsub	st, st(1)
		fxch	st(1)
		fsubp	st(2), st
		; st = v3, u3
		
		fld		st
		fmul	u2
		fxch	st(1)
		fmul	u1
		; st = u1v3, u2v3, u3
		fld		st(2)
		fmul	v1
		fxch	st(3)
		fmul	v2
		; st = u3v2, u1v3, u2v3, u3v1
		fxch	st(1)
		fsubp	st(3), st
		; st = u3v2, u2v3, u3v1 - u1v3
		fsubp	st(1), st
		; st = u2v3 - u3v2, u3v1 - u1v3
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdx

		; now we divide everything by -normc.

		fld		fpcnstneg1
		fdiv	normc
		fld		fpcnstscale
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudx
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fmul	st, st(2)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdx
		fmul	st, st(3)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdy
		fmulp	st(4), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dudx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dvdy

		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fmulp	st(2), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		
		; calculate starting coordinate values...
		mov		eax, _vList[0]
		mov		ecx, _tList[0]

		fld		(Point2 PTR [ecx]).x
		fmul	(Point3 PTR [eax]).z
		fmul	fpcnstscale
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dudy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_u0
										
		fld		(Point2 PTR [ecx]).y
		fmul	(Point3 PTR [eax]).z
		fmul	fpcnstscale
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dvdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_v0

		fld		(Point3 PTR [eax]).z
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [ebp]).pdc_dwdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [ebp]).pdc_w0
		mov		eax, 1
@@end:
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
DeltaCalcTriUVQW ENDP


DeltaCalcPolyUVQW PROC C,
		ARG pdc:DWORD
		public C DeltaCalcPolyUVQW

		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp
		
		fldcw	WORD PTR controlWord
		mov		edi, pdc
		; p0 = eax, p1 = ebx, C0 = ecx, C1 = edx, curPoly = edi, scratch = esi
		; iterator in ebp
		mov		ebp, _vertexCount
		dec		ebp
		mov		eax, _vList[ebp*4]
		; save off c0 and t0 for later.
		mov		ebx, _tList[ebp*4]
		mov		t0, ebx
		mov		ebx, _cList[ebp*4]
		mov		c0, ebx
		; clear out the current poly stuff...

		fld		fpcnst0
		fst		(GFXPolyDrawContext PTR [edi]).pdc_dudx				
		fst		(GFXPolyDrawContext PTR [edi]).pdc_dudy				
		fst		(GFXPolyDrawContext PTR [edi]).pdc_dvdx				
		fst		(GFXPolyDrawContext PTR [edi]).pdc_dvdy				
		fst		(GFXPolyDrawContext PTR [edi]).pdc_dwdx				
		fst		(GFXPolyDrawContext PTR [edi]).pdc_dwdy
		fstp	normc

		mov		ebp, 0
		; loop...
@@setupLoopStart:
		; p1 = vList[i]
		mov		ebx, _vList[ebp*4]
		; p1 = ebx, p0 = eax
		mov		ecx, t0
		mov		edx, _tList[ebp*4]
		mov		t0, edx	; save off t0 for the next time around
		fld		(Point3 PTR [eax]).y
		fsub	(Point3 PTR [ebx]).y

		fld		(Point2 PTR [ecx]).x
		fmul	(Point3 PTR [eax]).z
		fld		(Point2 PTR [edx]).x
		fmul	(Point3 PTR [ebx]).z
		faddp	st(1), st
		fmul	st, st(1)
		fadd	(GFXPolyDrawContext PTR [edi]).pdc_dudx
		fstp	(GFXPolyDrawContext PTR [edi]).pdc_dudx

		fld		(Point2 PTR [ecx]).y
		fmul	(Point3 PTR [eax]).z
		fld		(Point2 PTR [edx]).y
		fmul	(Point3 PTR [ebx]).z
		faddp	st(1), st
		fmul	st, st(1)
		fadd	(GFXPolyDrawContext PTR [edi]).pdc_dvdx
		fstp	(GFXPolyDrawContext PTR [edi]).pdc_dvdx

		fld		(Point3 PTR [eax]).z
		fadd	(Point3 PTR [ebx]).z
		fmulp	st(1), st
		fadd	(GFXPolyDrawContext PTR [edi]).pdc_dwdx
		fstp	(GFXPolyDrawContext PTR [edi]).pdc_dwdx

		fld		(Point3 PTR [eax]).x
		fadd	(Point3 PTR [ebx]).x
		
		fld		(Point2 PTR [ecx]).x
		fmul	(Point3 PTR [eax]).z
		fld		(Point2 PTR [edx]).x
		fmul	(Point3 PTR [ebx]).z
		fsubp	st(1), st
		fmul	st, st(1)
		fadd	(GFXPolyDrawContext PTR [edi]).pdc_dudy
		fstp	(GFXPolyDrawContext PTR [edi]).pdc_dudy
			
		fld		(Point2 PTR [ecx]).y
		fmul	(Point3 PTR [eax]).z
		fld		(Point2 PTR [edx]).y
		fmul	(Point3 PTR [ebx]).z
		fsubp	st(1), st
		fmul	st, st(1)
		fadd	(GFXPolyDrawContext PTR [edi]).pdc_dvdy
		fstp	(GFXPolyDrawContext PTR [edi]).pdc_dvdy

		fld		(Point3 PTR [eax]).z
		fsub	(Point3 PTR [ebx]).z
		fmulp	st(1), st
		fadd	(GFXPolyDrawContext PTR [edi]).pdc_dwdy
		fstp	(GFXPolyDrawContext PTR [edi]).pdc_dwdy

		fld		(Point3 PTR [eax]).x
		fsub	(Point3 PTR [ebx]).x
		fld		(Point3 PTR [eax]).y
		fadd	(Point3 PTR [ebx]).y
		fmulp	st(1), st
		fadd	normc
		fstp	normc
		mov		eax, ebx ; p0 = p1
		inc		ebp
		cmp		ebp, _vertexCount
		jl		@@setupLoopStart
		fld		normc
		fld		fpcnst0
		fcompp	
		fnstsw	ax
		; if st(1) was <= 0, then return false.
		sahf
		jb		@@notBackfaced
		mov		eax, 0
		jmp		@@end  ;this is the test for normc = 0  - collinear points, etc.
@@notBackfaced:

		fld		fpcnstneg1
		fdiv	normc
		fld		fpcnstscale
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [edi]).pdc_dudx
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [edi]).pdc_dudy
		fmul	st, st(2)
		fld		(GFXPolyDrawContext PTR [edi]).pdc_dvdx
		fmul	st, st(3)
		fld		(GFXPolyDrawContext PTR [edi]).pdc_dvdy
		fmulp	st(4), st
		fstp	(GFXPolyDrawContext PTR [edi]).pdc_dvdx
		fstp	(GFXPolyDrawContext PTR [edi]).pdc_dudy
		fstp	(GFXPolyDrawContext PTR [edi]).pdc_dudx
		fstp	(GFXPolyDrawContext PTR [edi]).pdc_dvdy

		fld		(GFXPolyDrawContext PTR [edi]).pdc_dwdx
		fmul	st, st(1)
		fld		(GFXPolyDrawContext PTR [edi]).pdc_dwdy
		fmulp	st(2), st
		fstp	(GFXPolyDrawContext PTR [edi]).pdc_dwdx
		fstp	(GFXPolyDrawContext PTR [edi]).pdc_dwdy
		
		; calculate starting coordinate values...
		mov		eax, _vList[0]
		mov		ecx, _tList[0]

		fld		(Point2 PTR [ecx]).x
		fmul	(Point3 PTR [eax]).z
		fmul	fpcnstscale
		fld		(GFXPolyDrawContext PTR [edi]).pdc_dudx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [edi]).pdc_dudy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [edi]).pdc_u0
										
		fld		(Point2 PTR [ecx]).y
		fmul	(Point3 PTR [eax]).z
		fmul	fpcnstscale
		fld		(GFXPolyDrawContext PTR [edi]).pdc_dvdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [edi]).pdc_dvdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [edi]).pdc_v0

		fld		(Point3 PTR [eax]).z
		fld		(GFXPolyDrawContext PTR [edi]).pdc_dwdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [edi]).pdc_dwdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [edi]).pdc_w0
		mov		eax, 1
@@end:
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
DeltaCalcPolyUVQW ENDP

DeltaCalcPolyW PROC C,
		ARG pdc:DWORD
		public C DeltaCalcPolyW

		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp
		
		fldcw	WORD PTR controlWord
		mov		edi, pdc
		; p0 = eax, p1 = ebx, C0 = ecx, C1 = edx, curPoly = edi, scratch = esi
		; iterator in ebp
		mov		ebp, _vertexCount
		dec		ebp
		mov		eax, _vList[ebp*4]
		; clear out the current poly stuff...

		fld		fpcnst0
		fst		(GFXPolyDrawContext PTR [edi]).pdc_dwdx				
		fst		(GFXPolyDrawContext PTR [edi]).pdc_dwdy
		fstp	normc

		mov		ebp, 0
		; loop...
@@setupLoopStart:
		; p1 = vList[i]
		mov		ebx, _vList[ebp*4]
		; p1 = ebx, p0 = eax
		fld		(Point3 PTR [eax]).y
		fsub	(Point3 PTR [ebx]).y

		fld		(Point3 PTR [eax]).z
		fadd	(Point3 PTR [ebx]).z
		fmulp	st(1), st
		fadd	(GFXPolyDrawContext PTR [edi]).pdc_dwdx
		fstp	(GFXPolyDrawContext PTR [edi]).pdc_dwdx

		fld		(Point3 PTR [eax]).x
		fadd	(Point3 PTR [ebx]).x
		
		fld		(Point3 PTR [eax]).z
		fsub	(Point3 PTR [ebx]).z
		fmulp	st(1), st
		fadd	(GFXPolyDrawContext PTR [edi]).pdc_dwdy
		fstp	(GFXPolyDrawContext PTR [edi]).pdc_dwdy

		fld		(Point3 PTR [eax]).x
		fsub	(Point3 PTR [ebx]).x
		fld		(Point3 PTR [eax]).y
		fadd	(Point3 PTR [ebx]).y
		fmulp	st(1), st
		fadd	normc
		fstp	normc
		mov		eax, ebx ; p0 = p1
		inc		ebp
		cmp		ebp, _vertexCount
		jl		@@setupLoopStart

		fld		normc
		fld		fpcnst0
		fcompp	
		fnstsw	ax
		; if st(1) was <= 0, then return false.
		sahf
		jb		@@notBackfaced
		mov		eax, 0
		jmp		@@end  ;this is the test for normc = 0  - collinear points, etc.
@@notBackfaced:

		fld		fpcnstneg1
		fdiv	normc

		fld		(GFXPolyDrawContext PTR [edi]).pdc_dwdx
		fmul	st, st(1)
		fxch	st(1)
		fmul	(GFXPolyDrawContext PTR [edi]).pdc_dwdy
		fxch	st(1)
		fstp	(GFXPolyDrawContext PTR [edi]).pdc_dwdx
		fstp	(GFXPolyDrawContext PTR [edi]).pdc_dwdy
		
		; calculate starting coordinate values...
		mov		eax, _vList[0]

		fld		(Point3 PTR [eax]).z
		fld		(GFXPolyDrawContext PTR [edi]).pdc_dwdx
		fmul	(Point3 PTR [eax]).x
		fld		(GFXPolyDrawContext PTR [edi]).pdc_dwdy
		fmul	(Point3 PTR [eax]).y
		fxch	st(1)
		fsubp	st(2), st
		fsubp	st(1), st
		fstp	(GFXPolyDrawContext PTR [edi]).pdc_w0
		mov		eax, 1
@@end:
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
DeltaCalcPolyW ENDP

NewRasterizer PROC C
		public C NewRasterizer

		lea		eax, @@spanExit
		mov		gfxPDC.pdc_rendSpanExit, eax
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		_esp, esp
		fldcw	WORD PTR controlWord
; setup code...
		; normally, edi will point to some kind of poly structure
		; for holding deltas and such, but for now, just make it
		; point to the pdc.

		lea		edi, gfxPDC
		
		mov		bot, 0
		mov		top, 0
		; p0 = eax, p1 = ebx, C0 = ecx, C1 = edx, curPoly = edi, scratch = esi
		; iterator in ebp
		mov		ebp, _vertexCount
		dec		ebp
		mov		eax, _vList[ebp*4]
		mov		ebp, 0
		; loop...
@@setupLoopStart:
		; p1 = vList[i]
		mov		ebx, _vList[ebp*4]
		mov		ecx, 0FFFFFFFFh ; -1
		fld		(Point3 PTR [ebx]).y
		fmul	fpcnst65536
		add		ecx, ebp ; ecx = ebp - 1
		mov		nextLefts[ebp*4], ecx
		add		ecx, 2
		fistp	iy[ebp*4]
		mov		nextRights[ebp*4], ecx
		mov		ecx, iy[ebp*4]

		add		ecx, 0FFFFh
		mov		edx, top

		shr		ecx, 16
		mov		esi, bot
		mov		iy[ebp*4], ecx
		cmp		ecx, esi ; compare against bot.
		jle		@@noNewBot
		mov		bot, ecx ; bot is actual value, whereas top is index
@@noNewBot:
		mov		esi, iy[edx*4]
		; ecx = iy[i], esi = iy[top]
		cmp		ecx, esi
		jge		@@noNewTop
		mov		top, ebp
@@noNewTop:
		mov		eax, ebx ; p0 = p1
		inc		ebp
		cmp		ebp, _vertexCount
		jl		@@setupLoopStart

		; check for trivial line case of poly.
		
		mov		eax, top
		mov		ebx, bot
		mov		ecx, iy[eax*4]
		cmp		ebx, ecx ; does top == bot?
		je		@@end

		mov		eax, gfxPDC.pdc_rowStride
		imul	ecx
		add		eax, gfxPDC.pdc_rowAddr
		mov		gfxPDC.pdc_rowAddr, eax

		; calculate steps for the scanline...
		; scale by texture scale.

		dec		ebp ; vertexCount - 1

		mov		nextLefts[0], ebp
		mov		nextRights[ebp*4], 0

		mov		eax, top
		mov		edx, eax ; right in edx
		mov		ebp, iy[eax*4]	; ebp = currentY
		mov		gfxPDC.pdc_currentY, ebp
@@leftLoop:
		mov		ebx, nextLefts[eax*4]
		mov		ecx, iy[eax*4]
		; while iy[left] == iy[nextLefts[left]]
		;	left = nextLefts[left]
		; ecx = iy[left], ebx = nextLefts[left]
		
		cmp		ecx, iy[ebx*4]
		jne		@@leftOk
		mov		eax, ebx
		jmp		@@leftLoop
@@leftOk:
		; left in eax
		; nextLeft in ebx
		mov		left, ebx ; save off left for next time (left = nleft)
		; may as well calc the deltas/start x's here.
		mov		ecx, _vList[eax*4] ; ecx = vList[left]
		mov		esi, _vList[ebx*4] ; esi = vList[nleft]
		; dxl = (nleft->x - left->x) / (nleft->y - left->y)
		fld		(Point3 PTR [esi]).x
		fsub	(Point3 PTR [ecx]).x
		fld		(Point3 PTR [esi]).y
		fsub	(Point3 PTR [ecx]).y
		fdivp	st(1), st
		; xl = left->x + (iy[left] - left->y) * dxl
		fild	iy[eax*4]
		fsub	(Point3 PTR [ecx]).y
		fmul	st, st(1)
		fadd	(Point3 PTR [ecx]).x
		fld		fpcnst65536
		fmul	st(2), st
		fmulp	st(1), st
		fxch	st(1)
		fistp	gfxPDC.pdc_deltaXL
		fistp	gfxPDC.pdc_xLeft
		add		gfxPDC.pdc_xLeft, 0FFFFh

		mov		ebp, iy[ebx*4] ; ebp = iy[nleft] - we'll compare below.

		mov		eax, edx ; right held in edx
@@rightLoop:
		mov		ebx, nextRights[eax*4]
		mov		ecx, iy[eax*4]
		; while iy[right] == iy[nextRights[right]]
		;	right = nextRights[right]
		; ecx = iy[right], ebx = nextRights[right]
		
		cmp		ecx, iy[ebx*4]
		jne		@@rightOk
		mov		eax, ebx
		jmp		@@rightLoop
@@rightOk:
		; right in eax
		; nextRight in ebx
		mov		right, ebx ; save off right for next time (right = nright)
		; may as well calc the deltas/start x's here.
		mov		ecx, _vList[eax*4] ; ecx = vList[right]
		mov		esi, _vList[ebx*4] ; esi = vList[nright]
		; dxl = (nright->x - right->x) / (nright->y - right->y)
		fld		(Point3 PTR [esi]).x
		fsub	(Point3 PTR [ecx]).x
		fld		(Point3 PTR [esi]).y
		fsub	(Point3 PTR [ecx]).y
		fdivp	st(1), st
		; xl = right->x + (iy[right] - right->y) * dxl
		fild	iy[eax*4]
		fsub	(Point3 PTR [ecx]).y
		fmul	st, st(1)
		fadd	(Point3 PTR [ecx]).x
		fld		fpcnst65536
		fmul	st(2), st
		fmulp	st(1), st
		fxch	st(1)
		fistp	gfxPDC.pdc_deltaXR
		fistp	gfxPDC.pdc_xRight
		add		gfxPDC.pdc_xRight, 0FFFFh
		mov		edx, iy[ebx*4]

; here's where we loop back to...
; make sure that iy[nleft] is in ebp and iy[nright] is in edx

@@mainLoop:
		cmp		edx, ebp
		jge		@@nleftSmaller
		mov		ebp, edx
@@nleftSmaller:
		mov		gfxPDC.pdc_destY, ebp

		mov		edi, gfxPDC.pdc_xLeft
		mov		esi, gfxPDC.pdc_xRight
		shr		edi, 16
		mov		eax, gfxPDC.pdc_currentY
		shr		esi, 16
@@rowLoop:
		sub		esi, edi
		jle		@@spanExit
		jmp		[gfxPDC.pdc_rendSpanEntry]
@@spanExit:
		mov		edi, gfxPDC.pdc_xLeft
		mov		esi, gfxPDC.pdc_xRight
		add		edi, gfxPDC.pdc_deltaXL
		add		esi, gfxPDC.pdc_deltaXR
		mov		gfxPDC.pdc_xLeft, edi
		mov		gfxPDC.pdc_xRight, esi
		shr		edi, 16
		mov		edx, gfxPDC.pdc_rowAddr
		shr		esi, 16
		mov		eax, gfxPDC.pdc_currentY
		add		edx, gfxPDC.pdc_rowStride
		add		eax, 1
		mov		gfxPDC.pdc_rowAddr, edx
		mov		gfxPDC.pdc_currentY, eax
		cmp		eax, gfxPDC.pdc_destY
		jl		@@rowLoop
		; ok - time to find out where we are

		mov		ecx, gfxPDC.pdc_currentY
		mov		eax, left
		mov		ebx, eax
		cmp		ecx, bot ; did we hit the bottom?
		je		@@end

		cmp		ecx, iy[eax*4] ; did we hit the end of the left edge?
		jne		@@checkRightEnd
		; cy in ecx, left in eax
		mov		ebx, nextLefts[eax*4] ; nleft in ebx
		; while (cy >= iy[nleft]) nleft = nextLefts[nleft]
@@nextLeftLoop:
		cmp		ecx, iy[ebx*4]
		jl		@@noNewNextLeft
		mov		ebx, nextLefts[ebx*4]
		jmp		@@nextLeftLoop
@@noNewNextLeft:
		; calculate left side deltas...
		; left in eax
		; nextLeft in ebx
		mov		left, ebx ; save off left for next time (left = nleft)
		; may as well calc the deltas/start x's here.
		mov		ecx, _vList[eax*4] ; ecx = vList[left]
		mov		esi, _vList[ebx*4] ; esi = vList[nleft]
		; dxl = (nleft->x - left->x) / (nleft->y - left->y)
		fld		(Point3 PTR [esi]).x
		fsub	(Point3 PTR [ecx]).x
		fld		(Point3 PTR [esi]).y
		fsub	(Point3 PTR [ecx]).y
		fdivp	st(1), st
		; xl = left->x + (iy[left] - left->y) * dxl
		fild	iy[eax*4]
		fsub	(Point3 PTR [ecx]).y
		fmul	st, st(1)
		fadd	(Point3 PTR [ecx]).x
		fld		fpcnst65536
		fmul	st(2), st
		fmulp	st(1), st
		fxch	st(1)
		fistp	gfxPDC.pdc_deltaXL
		fistp	gfxPDC.pdc_xLeft
		add		gfxPDC.pdc_xLeft, 0FFFFh
@@checkRightEnd:
		mov		ebp, iy[ebx*4] ; ebp = iy[nleft] - we'll compare below.
		mov		ecx, gfxPDC.pdc_currentY
		mov		eax, right
		mov		edx, iy[eax*4]
		cmp		ecx, edx ; did we hit the end of the right edge?
		jne		@@mainLoop ; if not, go back to the top...

		mov		ebx, nextRights[eax*4] ; nright in ebx
		; while (cy >= iy[nright]) nleft = nextLefts[nleft]
@@nextRightLoop:
		cmp		ecx, iy[ebx*4]
		jl		@@noNewNextRight
		mov		ebx, nextRights[ebx*4]
		jmp		@@nextRightLoop
@@noNewNextRight:
		; calculate right side deltas...
		; right in eax
		; nextRight in ebx
		mov		right, ebx ; save off left for next time (left = nleft)
		; may as well calc the deltas/start x's here.
		mov		ecx, _vList[eax*4] ; ecx = vList[right]
		mov		esi, _vList[ebx*4] ; esi = vList[nright]
		; dxl = (nright->x - right->x) / (nright->y - right->y)
		fld		(Point3 PTR [esi]).x
		fsub	(Point3 PTR [ecx]).x
		fld		(Point3 PTR [esi]).y
		fsub	(Point3 PTR [ecx]).y
		fdivp	st(1), st
		; xl = right->x + (iy[right] - right->y) * dxl
		fild	iy[eax*4]
		fsub	(Point3 PTR [ecx]).y
		fmul	st, st(1)
		fadd	(Point3 PTR [ecx]).x
		fld		fpcnst65536
		fmul	st(2), st
		fmulp	st(1), st
		fxch	st(1)
		fistp	gfxPDC.pdc_deltaXR
		fistp	gfxPDC.pdc_xRight
		add		gfxPDC.pdc_xRight, 0FFFFh
		mov		edx, iy[ebx*4]
		jmp		@@mainLoop
@@end:
		mov		esp, _esp
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
NewRasterizer ENDP

RasterizeRect PROC C,
	ARG xstart, count
	public C RasterizeRect

	push	ebx
	push	ecx
	push	esi
	push	edi
	push	ebp
	mov		_esp, esp

	lea		eax, @@spanExit
	mov		gfxPDC.pdc_rendSpanExit, eax

	mov		eax, xstart
	mov		ebx, count
	mov		gfxPDC.pdc_xLeft, eax
	mov		gfxPDC.pdc_deltaXL, ebx ; temp holding space for count

	mov		ecx, gfxPDC.pdc_currentY
	mov		eax, gfxPDC.pdc_rowStride
	imul	ecx
	add		eax, gfxPDC.pdc_rowAddr
	mov		gfxPDC.pdc_rowAddr, eax

@@loop:
	mov		eax, gfxPDC.pdc_currentY
	mov		edi, gfxPDC.pdc_xLeft
	mov		esi, gfxPDC.pdc_deltaXL
	jmp		[gfxPDC.pdc_rendSpanEntry]

@@spanExit:
	mov		eax, gfxPDC.pdc_currentY
	mov		ebx, gfxPDC.pdc_rowAddr
	
	inc		eax
	mov		ecx, gfxPDC.pdc_rowStride
	
	add		ebx, ecx
	mov		edx, gfxPDC.pdc_destY

	mov		gfxPDC.pdc_currentY, eax
	mov		gfxPDC.pdc_rowAddr, ebx

	cmp		eax, edx
	jl		@@loop

	mov		esp, _esp
	pop		ebp
	pop		edi
	pop		esi
	pop		ecx
	pop		ebx

	RET
RasterizeRect ENDP

NewRasterizeSpans PROC C,
		ARG spanList,lastSpan
		public C NewRasterizeSpans

		lea		eax, @@spanExit
		mov		gfxPDC.pdc_rendSpanExit, eax
		push	ebx
		push	esi
		push	edi
		push	ebp
		mov		eax, gfxPDC.pdc_rowAddr
		mov		saveBase, eax
		mov		_esp, esp

		mov		eax, lastSpan
		mov		ecx, spanList
		mov		saveLastSpan, eax       ; no need to check against null spanList,
                                        ; done in EmitPoly
@@loop:
		; span is x, y, count, next
		mov		edx, _spanListPtr
		mov		eax, [edx + ecx * 4]
		inc		ecx
		mov		edi, eax
		mov		nextSpan, ecx
		; span in mem is [Y:10 | X:11 | cnt:11]

		shr		eax, 22	; y is 22 bits up ( upper 10 bits)
		mov		esi, edi
		shr		edi, 11
		and		esi, 07FFh
		and		edi, 07FFh
		mov		ecx, eax

		imul	gfxPDC.pdc_rowStride
		
		add		eax, saveBase
		mov		gfxPDC.pdc_rowAddr, eax
		mov		eax, ecx

		jmp		[gfxPDC.pdc_rendSpanEntry]
@@spanExit:
		mov		eax, nextSpan
		mov		edx, _spanListPtr
		cmp		eax, saveLastSpan
		je		@@done
		mov		ecx, eax
		and		eax, 0Fh
		cmp		eax, 0Fh
		jne		@@loop
		mov		ecx, [edx + ecx * 4]
		jmp		@@loop
@@done:
		mov		esp, _esp
		mov		eax, saveBase
		mov		gfxPDC.pdc_rowAddr, eax
		pop		ebp
		pop		edi
		pop		esi
		pop		ebx
		ret
NewRasterizeSpans ENDP

END

