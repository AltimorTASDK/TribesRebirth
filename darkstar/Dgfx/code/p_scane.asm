; xend is now an intel instruction, so i had to change it to x_end

.486
.MODEL FLAT, syscall

.DATA

INCLUDE g_macros.inc

Point STRUC
	p_x		dd ?
	p_y		dd ?
Point ENDS

PolySurface STRUC
   xstart   dd ?
   x_end     dd ?
   myPoly   dd ?
   nextPoly dd ?
	spanList	dd	?
	curblock	dd	?
   curW     dd ?
	flags		dd	?
	dwdx		dq	?
	dwdy		dq	?
	w0			dq	?
PolySurface ENDS

ClippedPolyStore STRUC
   xstart   dd ?
   x_end     dd ?
   myPoly   dd ?
   nextPoly dd ?
ClippedPolyStore ENDS

PolyEdge STRUC
	pnext		   dd	?
	pprev		   dd	?
	boty  	   dd	?
	x			   dd	?
	deltax		dd	?
	leading		dd	?
	ptrailsurf	dd	?
	pleadsurf	dd	?
PolyEdge ENDS

FLAG_ALWAYS_BEHIND   equ   1
FLAG_TRANSPARENT	   equ	2
FLAG_BACKGROUND_FILL equ   4
FLAG_ALREADYDRAWN    equ   8

MAX_CLIP_SPANS    equ      255
MAX_POLYS			equ		3000
MAX_SCREEN_HEIGHT	equ		2048
MAX_SPANS			equ		30000

_newedges			dd	MAX_SCREEN_HEIGHT dup (0)
_edgehead	PolyEdge < 0, 0, MAX_SCREEN_HEIGHT, 080000000h, 0, 0, 0, 0>
_edgetail	PolyEdge < 0, 0, MAX_SCREEN_HEIGHT, 07FFFFFFFh, 0, 0, 0, 0>
_maxedge	PolyEdge {}
_surfstack	PolySurface {}
_spanListPtr	dd ?
_maxspans      dd ?
newEdgePtr		dd ?
newEdgeList    dd ?
curY			   dd ?
fixedPointX		dd ?
curX			   dd ?
_nextFreeBlock	dd ?
nextSave       dd ?
saveX          dd ?
status         dd ?
clipIndex      dd ?
transPolyList  dd ?
backgroundPoly dd ?
XFixedPointScale	dd	1048576.0
XFixedPointInvScale	dd	9.5367431640625e-7
XFixedPointShift	equ 20
XFixedPointCeilStep	equ 000FFFFFh
XFixedPointHalfStep  equ 0007FFFFh
forwardStep			equ 0400h

_dstY			dd ?
_pavailedge		dd ?
					   
roundupCtrlWord		   dw	0b7fh
roundnormalCtrlWord	   dw	0f7fh

ClipSurfaces   ClippedPolyStore (MAX_CLIP_SPANS+1) dup (<0,0,0,0>)


maxSpanCount	equ	MAX_SPANS

PUBLIC _newedges
PUBLIC _edgehead
PUBLIC _edgetail
PUBLIC _maxedge
PUBLIC _surfstack
PUBLIC _spanListPtr
PUBLIC _pavailedge
PUBLIC _nextFreeBlock
PUBLIC _maxspans

IFDEF __MASM__
	EXTRN _vList:DWORD
	EXTRN _vertexCount:DWORD
	EXTRN _kList:DWORD
ELSE
	EXTRN _vList
	EXTRN _vertexCount
	EXTRN _kList
ENDIF

.CODE

IFDEF __MASM__
ARG equ
ENDIF

gfx_add_edges PROC C,
	ARG		screenpoly
	LOCAL	yi, ynv, topy
	PUBLIC C gfx_add_edges

	push	ebx
	push	ecx
	push	edi
	push	esi

	; eax holds i
	; ebx is nextvert
	mov		eax, _vertexCount
	mov		ebx, 0

	dec		eax
	fldcw	   roundupCtrlWord		

	mov		edi, _vList[eax * 4]
	mov		esi, _pavailedge
	fld		(Point PTR [edi]).p_y
	fist	   yi
	fld		(Point PTR [edi]).p_x

	; loop through the vertices...
@@loop:
	; st = vList[i].x, vList[i].y
	; also, rounding mode should be set to round up.
	mov		edi, _vList[ebx*4]
	fld		(Point PTR [edi]).p_y
	fist	   ynv
	fld		(Point PTR [edi]).p_x

	; topy = yi, bottomy = ynv
	mov		ecx, yi
	mov		edx, ynv
	cmp		ecx, edx
	; are they the same?
	je		   @@loopEnd
	; top in ecx, bottom in edx.
	mov		(PolyEdge PTR [esi]).leading, 0
	jl		   @@skipReverse
	mov		ecx, ynv
	mov		edx, yi
	; not a leading edge any more
	mov		(PolyEdge PTR [esi]).leading, 1
@@skipReverse:
	; now top is in ecx, bottom in edx, edi is scratch
	mov		edi, _kList[eax*4]
	cmp		edi, 0
	je		   @@edgeNotShared
	mov		eax, _newedges[ecx*4]	
	; leading edge key must match for edge to be shared.
	cmp		eax, 0
	je		   @@edgeNotShared
@@sharedEdgeLoop:
	cmp		(PolyEdge PTR [eax]).leading, edi
	jne		@@nextSharedEdge
	; edge keys match...
	cmp		(PolyEdge PTR [esi]).leading, 1
	; the edge we're trying to insert is a leading edge, so it
	; will put it's surface into pleadsurf
	jne		@@sharedEdgeTrailing
	cmp		(PolyEdge PTR [eax]).pleadsurf, 0
	jne		@@edgeNotShared
	mov		edi, screenpoly
	mov		(PolyEdge PTR [eax]).pleadsurf, edi
	jmp		@@loopEnd
@@sharedEdgeTrailing:
	; in this case, just link in the new surface.
	cmp		(PolyEdge PTR [eax]).ptrailsurf, 0
	jne		@@edgeNotShared
	mov		edi, screenpoly
	mov		(PolyEdge PTR [eax]).ptrailsurf, edi
	jmp		@@loopEnd
@@nextSharedEdge:
	mov		eax, (PolyEdge PTR [eax]).pnext
	cmp		eax, 0
	jnz		@@sharedEdgeLoop
@@edgeNotShared:
	fldcw	   roundnormalCtrlWord
	; st = vList[nv].x, vList[nv].y, vList[i].x, vList[i].y
	fld		st(2)
	fsub	   st, st(1)
	fld		st(4)
	fsub	   st, st(3)
	mov		topy, ecx
	fdivp	   st(1), st

	cmp		(PolyEdge PTR [esi]).leading, 1
	mov		(PolyEdge PTR [esi]).leading, edi ; put the edge key in
	mov		edi, screenpoly
	jne		@@surfInTrailing
	mov		(PolyEdge PTR [esi]).pleadsurf, edi
	mov		(PolyEdge PTR [esi]).ptrailsurf, 0
	jmp		@@surfSet
@@surfInTrailing:
	mov		(PolyEdge PTR [esi]).pleadsurf, 0
	mov		(PolyEdge PTR [esi]).ptrailsurf, edi

	; formula for start x is: x' = x0 + (y' - y0) * dx/dy
	; stack = dx/dy, nv.x, nv.y, i.x, i.y

@@surfSet:
	fild	   topy
	fsub	   st, st(5)
	fmul	   st, st(1)
	fxch	   st(1)
	fmul	   XFixedPointScale
	fxch	   st(1)
	; stack = xs, dx/dy, nv.x, nv.y, i.x, i.y
	fadd	   st, st(4)
	fxch	   st(1)
	fistp	   (PolyEdge PTR [esi]).deltax
	fmul	   XFixedPointScale
	fistp	   (PolyEdge PTR [esi]).x
	fldcw	   roundupCtrlWord		

	; edi is insert list pointer.
	; ok to mangle eax and ecx after this.
	lea		edi, _newedges[ecx*4]
	mov		eax, _newedges[ecx*4]
	; edi = pedge, eax = *pedge.
	mov		ecx, (PolyEdge PTR [esi]).x
	cmp		eax, 0
	je		   @@insertPointFound
@@insertPointLoop:
	cmp		ecx, (PolyEdge PTR [eax]).x
	jle		@@insertPointFound
	lea		edi, (PolyEdge PTR [eax]).pnext
	mov		eax, (PolyEdge PTR [eax]).pnext
	cmp		eax, 0
	jne		@@insertPointLoop
@@insertPointFound:
	; ok, I reversed the order of things here.
	; used to be that this is where we checked for
	; shared edges.
	
	; no more.
	; we did that above, so just insert if we got to here.
	dec		edx
	mov		(PolyEdge PTR [esi]).pnext, eax
   mov      (PolyEdge PTR [esi]).boty, edx
	mov		DWORD PTR [edi], esi
	add		esi, SIZE PolyEdge ; go on to the next edge.
@@loopEnd:
	; clear off the bottom fpu registers.
	; gets rid of the extra garbage.
	ffree	   st(2)
	ffree	   st(3)

	mov		eax, ebx
	inc		ebx
	mov		ecx, ynv
	cmp		ebx, _vertexCount
	mov		yi, ecx
	jl		   @@loop
	mov		_pavailedge, esi
	
	ffree	   st(1)
	ffree	   st(0)
	fldcw	   roundnormalCtrlWord

	pop		esi
	pop		edi
	pop		ecx
	pop		ebx
	ret

gfx_add_edges ENDP


;-----------------------------------------------------------------
;
; gfx_scan_edges
;
;-----------------------------------------------------------------

@insertReverse:
   ; registers saved are eax, ebx
   ; poly to insert in edi
   lea      edx, transPolyList
   mov      ecx, transPolyList
   ; transPtr in edx
   ; *transPtr in ecx
   mov      esi, (PolySurface PTR [edi]).xstart

@insertReverseLoop:
   cmp      ecx, 0
   je       @insertReverseLoopDone
   cmp      esi, (PolySurface PTR [ecx]).xstart
   jge      @insertReverseLoopDone
   lea      edx, (PolySurface PTR [ecx]).nextPoly
   mov      ecx, (PolySurface PTR [ecx]).nextPoly
   jmp      @insertReverseLoop
@insertReverseLoopDone:
   mov      (PolySurface PTR [edi]).nextPoly, ecx
   mov      DWORD PTR [edx], edi
   ret

@emitSpan:
   ; registers saved are eax, ebx
   ; xstart in ecx, x_end in edi
   ; surface ptr in edx
   cmp      ecx, edi
   jge      @emitDone
   sub      edi, ecx
   mov      esi, curY

   ; count in edi
   ; xstart in ecx
   ; y in esi
   shl      esi, 22
   mov      ebp, (PolySurface PTR [edx]).curblock
   shl      ecx, 11
   or       esi, edi

   mov      edi, ebp
   and      ebp, 0Fh

   or       esi, ecx
   mov      ecx, _spanListPtr

   cmp      ebp, 0Fh
   jne      @emitNoNewBlock
   
   mov      ebp, _nextFreeBlock
   add      ebp, 16
   cmp      ebp, _maxspans
   jg       @emitAbort
   mov      _nextFreeBlock, ebp
   mov      [ecx + edi * 4], ebp
   mov      [ecx + ebp * 4], esi
   inc      ebp
   mov      (PolySurface PTR [edx]).curblock, ebp
   ret
@emitNoNewBlock:
   mov      [ecx + edi * 4], esi
   inc      edi
   mov      (PolySurface PTR [edx]).curblock, edi
   ret
@emitDone:
   ret
@emitAbort:
   add      esp, 4
   jmp      @abort


@resort:
   ; registers saved are eax, ebx
   ; start in ecx, sort val in ebx
   mov      edi, (PolySurface PTR [ecx]).nextPoly
   mov      esi, (PolySurface PTR [edx]).xstart

@resortLoop:
   cmp      edi, 0
   je       @resortLoopDone
   cmp      esi, (PolySurface PTR [edi]).xstart
   jle      @resortLoopDone
   mov      ecx, edi
   mov      edi, (PolySurface PTR [edi]).nextPoly
   jmp      @resortLoop

@resortLoopDone:
   mov      (PolySurface PTR [edx]).nextPoly, edi
   mov      (PolySurface PTR [ecx]).nextPoly, edx
   ret

gfx_scan_edges PROC C,
	ARG		dstY
	PUBLIC C gfx_scan_edges
	mov		eax, dstY
	mov		_dstY, eax

	push	   ebx
	push	   ecx
	push	   edi
	push	   esi
	push	   ebp

   mov      _edgehead.pnext, OFFSET _edgetail
   mov      _edgetail.pprev, OFFSET _edgehead
	mov		newEdgePtr, OFFSET _newedges
	mov		curY, 0
@@yLoop:
	mov		ebx, curY
	cmp		ebx, _dstY
	jge		@@exit

   ; aelWalk is in edi
   ; addWalk is in esi

	mov		eax, newEdgePtr
   mov      edi, _edgehead.pnext
   mov      esi, [eax]
   mov      DWORD PTR [eax], 0 ; clear out the new edge ptr.
	add		eax, 4
	mov		newEdgePtr, eax

   ; loop through aelWalk, removing done edges, stepping x
   ; and inserting new edges

@@aelWalkLoop:
   cmp      edi, 0
   je       @@constructActivePolyList
@@testNextActiveEdge:   
   mov      ebx, curY
   mov      ecx, (PolyEdge PTR [edi]).boty

   cmp      ecx, ebx ; if lasty < curY, remove the edge
   jge      @@stepActiveEdge

   ; next->pprev = prev
   ; prev->pnext = next
   ; aelWalk = next
   mov      edx, (PolyEdge PTR [edi]).pnext
   mov      ecx, (PolyEdge PTR [edi]).pprev

   cmp      edx, 0
   mov      edi, edx

   mov      (PolyEdge PTR [edx]).pprev, ecx
   mov      (PolyEdge PTR [ecx]).pnext, edx

   jne      @@testNextActiveEdge
   jmp      @@constructActivePolyList

@@stepActiveEdge:
   mov      eax, (PolyEdge PTR [edi]).x
   mov      ecx, (PolyEdge PTR [edi]).pnext

   mov      ebx, (PolyEdge PTR [edi]).deltax
   mov      nextSave, ecx

   add      eax, ebx ; step edge x
   mov      ebx, (PolyEdge PTR [edi]).pleadsurf

   mov      ecx, eax
	add		eax, XFixedPointCeilStep

	shr		eax, XFixedPointShift
   mov      edx, (PolyEdge PTR [edi]).ptrailsurf

   cmp      ebx, 0
   mov      (PolyEdge PTR [edi]).x, ecx

   mov      ebp, (PolyEdge PTR [edi]).pprev

   je       @@noAELLeadSurface
   mov      (PolySurface PTR [ebx]).xstart, eax
   and      (PolySurface PTR [ebx]).flags, NOT FLAG_ALREADYDRAWN
@@noAELLeadSurface:
   cmp      edx, 0
   je       @@checkSorting

   mov      (PolySurface PTR [edx]).x_end, eax

@@checkSorting:
   ; edi's current x is in ecx
   ; prev is in ebp

   cmp      ecx, (PolyEdge PTR [ebp]).x
   jge      @@insertNewEdges
@@nextSortCheck:
   mov      eax, (PolyEdge PTR [ebp]).pprev
   mov      ebx, (PolyEdge PTR [edi]).pnext

   ; pprev in ebp
   ; pprev->pprev in eax
   ; pnext in ebx

   ; prev->next = pnext
   mov      (PolyEdge PTR [ebp]).pnext, ebx
   ; aelWalk->pprev = pprev->pprev
   mov      (PolyEdge PTR [edi]).pprev, eax
   ; next->pprev = prev
   mov      (PolyEdge PTR [ebx]).pprev, ebp
   ; aelWalk->pnext = prev
   mov      (PolyEdge PTR [edi]).pnext, ebp
   ; pprev->pprev = aelWalk
   mov      (PolyEdge PTR [ebp]).pprev, edi
   ; aelWalk->pprev->pnext = aelWalk
   mov      (PolyEdge PTR [eax]).pnext, edi

   mov      ebp, eax

   cmp      ecx, (PolyEdge PTR [eax]).x
   jl       @@nextSortCheck

@@insertNewEdges:
   cmp      esi, 0
   je       @@nextAELCheck
   mov      eax, (PolyEdge PTR [esi]).x
   cmp      ecx, eax
   jle      @@nextAELCheck

   mov      ebx, (PolyEdge PTR [esi]).pleadsurf
	add		eax, XFixedPointCeilStep

	shr		eax, XFixedPointShift
   mov      edx, (PolyEdge PTR [esi]).ptrailsurf

   cmp      ebx, 0
   je       @@noAddLeadSurface

   mov      (PolySurface PTR [ebx]).xstart, eax
   and      (PolySurface PTR [ebx]).flags, NOT FLAG_ALREADYDRAWN
@@noAddLeadSurface:
   cmp      edx, 0
   je       @@insertEdge
   mov      (PolySurface PTR [edx]).x_end, eax
@@insertEdge:
   mov      edx, (PolyEdge PTR [edi]).pprev
   ; nextAdd = addWalk->pnext
   mov      eax, (PolyEdge PTR [esi]).pnext
   ; addWalk->pnext = aelWalk
   mov      (PolyEdge PTR [esi]).pnext, edi
   ; addWalk->pprev = aelWalk->pprev
   mov      (PolyEdge PTR [esi]).pprev, edx
   ; addWalk->pprev->pnext = addWalk
   mov      (PolyEdge PTR [edx]).pnext, esi
   ; aelWalk->pprev = addWalk
   mov      (PolyEdge PTR [edi]).pprev, esi
   mov      esi, eax
   jmp      @@insertNewEdges

@@nextAELCheck:
   mov      edi, nextSave
   cmp      edi, 0
   jne      @@testNextActiveEdge

@@constructActivePolyList:
   ; activePolyList = eax = 0
   ; transPolyList = ebx = 0
   ; aelWalk = edi = edgetail.pprev

	fild	   curY				; fp stack = y

   mov      edi, _edgetail.pprev
   mov      eax, 0
   mov      ebx, 0
   mov      backgroundPoly, 0
   cmp      edi, OFFSET _edgehead
   je       @@APLConstructed
@@constructNext:
   fld      st
   mov      ecx, (PolyEdge PTR [edi]).pleadsurf
   mov      edx, (PolyEdge PTR [edi]).pprev
   cmp      ecx, 0
   je       @@nextConstructEdge
   fmul     (PolySurface PTR [ecx]).dwdy
   mov      ebp, (PolySurface PTR [ecx]).xstart
   mov      esi, (PolySurface PTR [ecx]).x_end
   cmp      ebp, esi
   jge      @@nextConstructEdge
   mov      esi, (PolySurface PTR [ecx]).flags
   fadd     (PolySurface PTR [ecx]).w0
   test     esi, FLAG_ALREADYDRAWN
   jnz      @@nextConstructEdge
   test     esi, FLAG_TRANSPARENT
   jz       @@constructPolySolid
@@constructPolyTransparent:
   mov      (PolySurface PTR [ecx]).nextPoly, ebx
   mov      ebx, ecx
   jmp      @@finishConstruct
@@constructPolySolid:
   mov      (PolySurface PTR [ecx]).nextPoly, eax
   mov      eax, ecx
@@finishConstruct:
   or       esi, FLAG_ALREADYDRAWN
   fst      (PolySurface PTR [ecx]).curW
   mov      (PolySurface PTR [ecx]).flags, esi
@@nextConstructEdge:
   fstp     st
   mov      edi, edx
   cmp      edx, OFFSET _edgehead
   jne      @@constructNext
@@APLConstructed:

   fstp     st

   ; curPoly in eax
   ; transPolyList in ebx

   mov      clipIndex, 0
   mov      transPolyList, ebx

   cmp      eax, 0
   je       @@emitLastSpansAndLoop
   ; at the start of the loop, eax holds curPoly, ebx holds transPolyList
@@testTransPolyList:
   cmp      ebx, 0
   je       @@skipTransClip

   mov      transPolyList, 0
@@transPolyLoop:   
   mov      ecx, (PolySurface PTR [ebx]).xstart
   mov      edx, (PolySurface PTR [eax]).x_end
   cmp      ecx, edx
   jge      @@transListProcessed
   mov      edi, (PolySurface PTR [ebx]).x_end
   mov      esi, (PolySurface PTR [eax]).xstart
   cmp      edi, esi
   jg       @@transPolyOverlapped
   ; emit(transPoly>xstart, transPoly->x_end, transPoly->myPoly)
   mov      edx, (PolySurface PTR [ebx]).myPoly
   mov      ebx, (PolySurface PTR [ebx]).nextPoly
   call     @emitSpan
   cmp      ebx, 0
   jne      @@transPolyLoop
   jmp      @@transListProcessed
@@transPolyOverlapped:
   ; transPoly->xstart in ecx
   ; curPoly->xstart in esi
   test     (PolySurface PTR [eax]).flags, FLAG_ALWAYS_BEHIND
   jnz      @@transCurPolyAlwaysBehind
   cmp      ecx, esi
   jg       @@transXStDone
   mov      ecx, esi
@@transXStDone:
   mov      saveX, ecx
   mov      ecx, (PolySurface PTR [ebx]).myPoly
   mov      esi, (PolySurface PTR [eax]).myPoly
   ; st = y
   fld      (PolySurface PTR [esi]).dwdx
   fld      (PolySurface PTR [ecx]).dwdx
   fld      (PolySurface PTR [esi]).curW
   fld      (PolySurface PTR [ecx]).curW

   ; st = tp->curW, cp->curW, tp->dwdx, cp->dwdx, y

   fild     saveX
   fld      st

   ; st0    st1      st2      st3      st4      st5      st6      st7
   ; x      x        tp->curW cp->curW tp->dw   cp->dw   curY

   fmul     st, st(5)
   fxch     st(1)
   
   ; x      cp->cx   tp->curW cp->curW tp->dw   cp->dw   curY
   dec      edx
   dec      edi
   fmul     st, st(4)
   ; tp->cx cp->cx   tp->curW cp->curW tp->dw   cp->dw   curY
   fxch     st(1)
   fadd     st, st(3)
   fxch     st(1)
   fadd     st, st(2)
   ; tp->w  cp->w    tp->curW cp->curW tp->dw   cp->dw   curY

   ; transPoly->x_end-1 in edi
   ; curPoly->x_end-1 in edx
   cmp      edx, edi
   jle      @@transx_endDone
   mov      edx, edi
@@transx_endDone:
   mov      saveX, edx
   fsubp    st(1), st

   fild     saveX
   fld      st

   fxch     st(2)
   fstp     status
   
   ; x      x        tp->curW cp->curW tp->dw   cp->dw   curY
   fmul     st, st(5)
   fxch     st(1)
   
   ; x      cp->cx   tp->curW cp->curW tp->dw   cp->dw   curY
   fmul     st, st(4)
   ; tp->cx cp->cx   tp->curW cp->curW tp->dw   cp->dw   curY
   fxch     st(1)
   fadd     st, st(3)
   fxch     st(1)
   fadd     st, st(2)
   ; tp->w  cp->w    tp->curW cp->curW tp->dw   cp->dw   curY
   mov      esi, status
   fsubp    st(1), st
   mov      edi, status
   fstp     status
   mov      edx, status

   ; startStatus in esi, edi
   ; endStatus in edx

   or       edi, edx
   and      edx, esi

   ; statusOr in edi
   ; statusAnd in edx
   ; startStatus still in esi
   ; curPoly in eax, transPoly in ebx, transPoly->myPoly in ecx

   ; if statusOr sign bit is not set then startStatus >= 0 && endStatus >= 0
   ; else if statusAnd sign bit is set then startStatus <= 0 && endStatus <= 0
   ; else there was an intersection

   test     edi, 080000000h
   jnz      @@transAllBehindTest
   mov      edx, ecx
   mov      ecx, (PolySurface PTR [ebx]).xstart
   mov      edi, (PolySurface PTR [eax]).xstart
   call     @emitSpan
   mov      ecx, (PolySurface PTR [eax]).x_end
   mov      edi, ebx
   mov      edx, (PolySurface PTR [ebx]).x_end
   mov      ebx, (PolySurface PTR [ebx]).nextPoly

   cmp      ecx, edx
   jge      @@nextTransCheck
   mov      (PolySurface PTR [edi]).xstart, ecx

   ; tricky me...
   ; push the return address and jump (avoids an extra jump or extra code)

   push     OFFSET @@nextTransCheck
   jmp      @insertReverse
@@transAllBehindTest:
   test     edx, 080000000h
   jz       @@transIntersected
@@transCurPolyBehind:
   mov      edi, ebx
   mov      ebx, (PolySurface PTR [ebx]).nextPoly
   ; if curPoly is totally behind transPoly
   ; just insertReverse this sucker and continue on
   call     @insertReverse
@@nextTransCheck:
   fstp     st
   fstp     st
   fstp     st
   fstp     st
@@nextTransCheckPopped:
   cmp      ebx, 0
   jne      @@transPolyLoop
   jmp      @@transListProcessed
@@transCurPolyAlwaysBehind:
   mov      edi, ebx
   mov      ebx, (PolySurface PTR [ebx]).nextPoly
   ; if curPoly is totally behind transPoly
   ; just insertReverse this sucker and continue on
   call     @insertReverse
   cmp      ebx, 0
   jne      @@transPolyLoop
   jmp      @@transListProcessed
@@transIntersected:
   ;  st0      st1      st2      st3      st4      st5      st6      st7
   ;  tp->curW cp->curW tp->dw   cp->dw   curY
   
   ; xPoint = (tp->curW - cp->curW) / (cp->dwdx - tp->dwdx)

   fsubrp      st(1), st

   ;  tcw-ccw  tp->dwdx cp->dwdx curY
   fxch        st(1)
   fsubp       st(2), st
   ; tcw-ccw   cdw-tdw  curY
   fdivrp      st(1), st

   ; if startStatus > 0
   test     esi, 080000000h
   jnz      @@transPolyIntersectBehind

   mov      edx, ecx
   mov      ecx, (PolySurface PTR [ebx]).xstart
   mov      edi, (PolySurface PTR [eax]).xstart
   call     @emitSpan
   mov      edi, ebx
   push     OFFSET @@nextTransCheckPopped
   fistp    (PolySurface PTR [ebx]).xstart
   mov      ebx, (PolySurface PTR [ebx]).nextPoly
   jmp      @insertReverse

@@transPolyIntersectBehind:
   mov      edx, clipIndex
   mov      edi, (PolySurface PTR [ebx]).x_end
   mov      esi, (PolySurface PTR [eax]).x_end
   mov      ebp, edx
   cmp      edi, esi
   jle      @@transNoExtraSpan
   cmp      edx, MAX_CLIP_SPANS
   jge      @@transNoExtraSpan
   shl      ebp, 4   ;sizeof(ClippedPolyStore) is 16 bytes
   inc      edx
   lea      ebp, [ClipSurfaces + ebp]
   mov      clipIndex, edx

   ; agi DOH

   mov      (PolySurface PTR [ebp]).xstart, esi
   mov      (PolySurface PTR [ebp]).x_end, edi
   mov      (PolySurface PTR [ebp]).myPoly, ecx
   mov      edi, ebp
   call     @insertReverse
@@transNoExtraSpan:
   fistp    (PolySurface PTR [ebx]).x_end
   mov      edi, ebx
   mov      ebx, (PolySurface PTR [ebx]).nextPoly
   call     @insertReverse      
   cmp      ebx, 0
   jne      @@transPolyLoop
@@transListProcessed:
   mov      ecx, transPolyList
   cmp      ebx, 0
   jz       @@reverseList

   cmp      ecx, 0
   jz       @@listReversed

   ; transPoly in ebx, transPolyList in ecx
@@insertTransListLoop:
   ; transTemp in ebp, transTemp->next in esi
   mov      esi, (PolySurface PTR [ebx]).nextPoly
   mov      ebp, ebx
   mov      edx, (PolySurface PTR [ebx]).xstart
   mov      edi, (PolySurface PTR [ecx]).xstart
   cmp      edi, edx
   jle      @@reverseList
@@insertTransListSubLoop:
   cmp      esi, 0
   je       @@insertTransListBreak
   cmp      edi, (PolySurface PTR [esi]).xstart
   jle      @@insertTransListBreak
   mov      ebp, esi
   mov      esi, (PolySurface PTR [esi]).nextPoly
   jmp      @@insertTransListSubLoop
@@insertTransListBreak:
   ; temp = transPolyList->nextPoly
   mov      edx, (PolySurface PTR [ecx]).nextPoly
   ; transPolyList->nextPoly = transTemp->nextPoly
   mov      (PolySurface PTR [ecx]).nextPoly, esi
   ; transTemp->nextPoly = transPolyList
   mov      (PolySurface PTR [ebp]).nextPoly, ecx
   mov      ecx, edx
   cmp      edx, 0
   jne      @@insertTransListLoop
   mov      transPolyList, ebx
   jmp      @@skipTransClip
@@reverseList:
   ; just reverse the transPolyList
   cmp      ecx, 0
   jz       @@listReversed
@@reverseListLoop:
   ; temp = transPolyList->nextPoly
   mov      edx, (PolySurface PTR [ecx]).nextPoly
   ; transPolyList->nextPoly = transPoly
   mov      (PolySurface PTR [ecx]).nextPoly, ebx
   ; transPoly= transPolyList
   mov      ebx, ecx
   ; transPolyList = temp
   mov      ecx, edx
   cmp      edx, 0
   jne      @@reverseListLoop
@@listReversed:
   mov      transPolyList, ebx
@@skipTransClip:
   ; ok, from this point on ebx is nextPoly
   mov      ebx, (PolySurface PTR [eax]).nextPoly
   cmp      ebx, 0
   je       @@emitLastSpansAndLoop
   mov      edi, (PolySurface PTR [eax]).x_end
   mov      ecx, (PolySurface PTR [ebx]).xstart
   cmp      ecx, edi
   jl       @@spansOverlapInX
   mov      ecx, (PolySurface PTR [eax]).xstart
   mov      edx, (PolySurface PTR [eax]).myPoly
   mov      eax, ebx
   mov      ebx, transPolyList
   push     OFFSET @@testTransPolyList
   jmp      @emitSpan

@@spansOverlapInX:
   mov      ebp, (PolySurface PTR [eax]).flags
   mov      ecx, (PolySurface PTR [eax]).myPoly
   mov      esi, (PolySurface PTR [ebx]).flags
   mov      edx, (PolySurface PTR [ebx]).myPoly

   test     ebp, FLAG_ALWAYS_BEHIND
   jnz      @@nextPolyTotallyInFront
   test     esi, FLAG_ALWAYS_BEHIND
   jnz      @@curPolyTotallyInFront

   fild     (PolySurface PTR [ebx]).xstart
   mov      esi, (PolySurface PTR [ebx]).x_end
   cmp      esi, edi
   jl       @@minCalced
   mov      esi, edi
@@minCalced:
   dec      esi
   fld      st

   fmul     (PolySurface PTR [ecx]).dwdx
   fld      (PolySurface PTR [ecx]).curW
   fxch     st(2)
   mov      saveX, esi
   fmul     (PolySurface PTR [edx]).dwdx
   fxch     st(1)
   faddp    st(2), st
   fild     saveX
   fxch     st(1)
   fadd     (PolySurface PTR [edx]).curW
   fxch     st(1)

   ; st = x_end nextPolyWStart curPolyWStart y
   fld      st
   ; st = x_end x_end nextPolyWStart curPolyWStart y
   fmul     (PolySurface PTR [ecx]).dwdx
   ; st = curXE x_end nextPolyWStart curPolyWStart y
   fxch     st(2)
   fsubp    st(3), st
   ; st = x_end curXE startStatus y
   fmul     (PolySurface PTR [edx]).dwdx
   fxch     st(1)
   fadd     (PolySurface PTR [ecx]).curW
   fxch     st(2)
   fstp     status
   fadd     (PolySurface PTR [edx]).curW
   mov      edi, status
   mov      esi, status
   fsubp    st(1), st
   fstp     status
   mov      ebp, status

   and      edi, ebp
   or       ebp, esi

   ; startStatus in esi
   ; orStatus in ebp
   ; andStatus in edi

   ; curPoly in eax, nextPoly in ebx
   ; curPoly->myPoly in ecx, nextPoly->myPoly in edx
   
   test     ebp, 080000000h
   jnz      @@testCurPolyBehind
@@curPolyTotallyInFront:
   ; curPoly is totally in front
   mov      ecx, (PolySurface PTR [eax]).x_end
   mov      edi, (PolySurface PTR [ebx]).nextPoly
   mov      edx, (PolySurface PTR [ebx]).x_end
   mov      (PolySurface PTR [eax]).nextPoly, edi
   cmp      ecx, edx
   jge      @@skipTransClip
   mov      (PolySurface PTR [ebx]).xstart, ecx
   mov      ecx, eax
   mov      edx, ebx
   push     OFFSET @@skipTransClip
   jmp      @resort
@@testCurPolyBehind:
   ; check if curPoly is totally behind nextPoly
   test     edi, 080000000h
   jz       @@curAndNextPolyIntersected

@@nextPolyTotallyInFront:
   mov      edx, ecx
   mov      ecx, (PolySurface PTR [eax]).xstart
   mov      edi, (PolySurface PTR [ebx]).xstart
   call     @emitSpan
   mov      edi, (PolySurface PTR [eax]).x_end
   mov      ecx, ebx
   mov      esi, (PolySurface PTR [ebx]).x_end
   mov      edx, eax
   mov      eax, ebx
   mov      ebx, transPolyList
   cmp      edi, esi
   jle      @@testTransPolyList
   push     OFFSET @@testTransPolyList
   mov      (PolySurface PTR [edx]).xstart, esi
   jmp      @resort
@@outOfClipSpans:
   fstp     st
   jmp      @@curPolyTotallyInFront
@@curAndNextPolyIntersected:   
   fld      (PolySurface PTR [edx]).curW
   fsub     (PolySurface PTR [ecx]).curW
   fld      (PolySurface PTR [ecx]).dwdx
   fsub     (PolySurface PTR [edx]).dwdx
   mov      edx, (PolySurface PTR [eax]).x_end
   mov      ebp, clipIndex
   mov      edi, (PolySurface PTR [ebx]).x_end
   fdivp    st(1), st

   test     esi, 080000000h
   jnz      @@intersectStatusNegative
   cmp      edx, edi
   jle      @@positiveNoExtraSpan
   cmp      ebp, MAX_CLIP_SPANS
   jge      @@outOfClipSpans
   lea      esi, [ebp*8]
   inc      ebp
   lea      esi, [ClipSurfaces + esi*2]
   mov      clipIndex, ebp
   mov      (PolySurface PTR [esi]).xstart, edi
   mov      (PolySurface PTR [esi]).x_end, edx
   mov      (PolySurface PTR [esi]).myPoly, ecx
   mov      ecx, eax
   mov      edx, esi
   call     @resort
@@positiveNoExtraSpan:
   fistp    (PolySurface PTR [eax]).x_end
   mov      esi, (PolySurface PTR [ebx]).nextPoly
   mov      edi, (PolySurface PTR [eax]).x_end
   mov      (PolySurface PTR [eax]).nextPoly, esi
   mov      (PolySurface PTR [ebx]).xstart, edi
   mov      ecx, eax
   mov      edx, ebx
   push     OFFSET @@skipTransClip
   jmp      @resort
@@intersectStatusNegative:
   ; nextPoly starts in front
   cmp      edi, edx
   jle      @@negativeNoExtraSpan
   cmp      ebp, MAX_CLIP_SPANS
   jge      @@outOfClipSpans
   lea      esi, [ebp*8]
   inc      ebp
   lea      esi, [ClipSurfaces + esi*2]
   mov      clipIndex, ebp
   mov      ecx, (PolySurface PTR [ebx]).myPoly
   mov      (PolySurface PTR [esi]).xstart, edx
   mov      (PolySurface PTR [esi]).x_end, edi
   mov      (PolySurface PTR [esi]).myPoly, ecx
   mov      edx, esi
   mov      ecx, ebx
   call     @resort
@@negativeNoExtraSpan:
   mov      ecx, (PolySurface PTR [eax]).xstart
   mov      edx, (PolySurface PTR [eax]).myPoly
   mov      edi, (PolySurface PTR [ebx]).xstart
   call     @emitSpan
   fistp    (PolySurface PTR [eax]).xstart
   mov      edx, eax
   mov      edi, (PolySurface PTR [eax]).xstart
   mov      eax, ebx
   mov      (PolySurface PTR [ebx]).x_end, edi
   mov      ecx, ebx
   mov      ebx, transPolyList
   push     OFFSET @@testTransPolyList
   jmp      @resort
@@emitLastSpansAndLoop:
   cmp      eax, 0
   je       @@transEndLoopTest
   mov      ecx, (PolySurface PTR [eax]).xstart
   mov      edi, (PolySurface PTR [eax]).x_end
   mov      edx, (PolySurface PTR [eax]).myPoly
   call     @emitSpan
@@transEndLoopTest:
   mov      ebx, transPolyList
   cmp      ebx, 0
   je       @@scanlineDone
@@transEmitLoop:
   mov      ecx, (PolySurface PTR [ebx]).xstart
   mov      edi, (PolySurface PTR [ebx]).x_end
   mov      edx, (PolySurface PTR [ebx]).myPoly
   mov      ebx, (PolySurface PTR [ebx]).nextPoly
   call     @emitSpan
   cmp      ebx, 0
   jne      @@transEmitLoop
@@scanlineDone:
   inc      curY
   jmp      @@yLoop

@@exit:
	mov		eax, _nextFreeBlock ; return the number of used spans.
	pop		ebp
	pop		esi
	pop		edi
	pop		ecx
	pop		ebx
	ret

   ; if we had to abort half way through, clear out the
   ; start edges.

IFDEF __MASM__
	PUBLIC @abort
@abort::
ELSE
@abort:
ENDIF
	ffree	st(1)
	ffree	st
	lea		eax, [_newedges]
   mov      ebx, _dstY
@@clearLoop:
   mov      DWORD PTR [eax], 0
   add      eax, 4
   dec      ebx
   jnz      @@clearLoop
   jmp      @@exit
gfx_scan_edges ENDP

gfx_scan_edges_z PROC C,
	ARG		dstY
	PUBLIC C gfx_scan_edges_z
	mov		eax, dstY
	mov		_dstY, eax

	push	   ebx
	push	   ecx
	push	   edi
	push	   esi
	push	   ebp

   mov      _edgehead.pnext, OFFSET _edgetail
   mov      _edgetail.pprev, OFFSET _edgehead
	mov		newEdgePtr, OFFSET _newedges
	mov		curY, 0
@@yLoop:
	mov		ebx, curY
	cmp		ebx, _dstY
	jge		@@exit

   ; aelWalk is in edi
   ; addWalk is in esi

	mov		eax, newEdgePtr
   mov      edi, _edgehead.pnext
   mov      esi, [eax]
   mov      DWORD PTR [eax], 0 ; clear out the new edge ptr.
	add		eax, 4
	mov		newEdgePtr, eax

   ; loop through aelWalk, removing done edges, stepping x
   ; and inserting new edges

@@aelWalkLoop:
   cmp      edi, 0
   je       @@constructActivePolyList
@@testNextActiveEdge:   
   mov      ebx, curY
   mov      ecx, (PolyEdge PTR [edi]).boty

   cmp      ecx, ebx ; if lasty < curY, remove the edge
   jge      @@stepActiveEdge

   ; next->pprev = prev
   ; prev->pnext = next
   ; aelWalk = next
   mov      edx, (PolyEdge PTR [edi]).pnext
   mov      ecx, (PolyEdge PTR [edi]).pprev

   cmp      edx, 0
   mov      edi, edx

   mov      (PolyEdge PTR [edx]).pprev, ecx
   mov      (PolyEdge PTR [ecx]).pnext, edx

   jne      @@testNextActiveEdge
   jmp      @@constructActivePolyList

@@stepActiveEdge:
   mov      eax, (PolyEdge PTR [edi]).x
   mov      ecx, (PolyEdge PTR [edi]).pnext

   mov      ebx, (PolyEdge PTR [edi]).deltax
   mov      nextSave, ecx

   add      eax, ebx ; step edge x
   mov      ebx, (PolyEdge PTR [edi]).pleadsurf

   mov      ecx, eax
	add		eax, XFixedPointCeilStep

	shr		eax, XFixedPointShift
   mov      edx, (PolyEdge PTR [edi]).ptrailsurf

   cmp      ebx, 0
   mov      (PolyEdge PTR [edi]).x, ecx

   mov      ebp, (PolyEdge PTR [edi]).pprev

   je       @@noAELLeadSurface
   mov      (PolySurface PTR [ebx]).xstart, eax
   and      (PolySurface PTR [ebx]).flags, NOT FLAG_ALREADYDRAWN
@@noAELLeadSurface:
   cmp      edx, 0
   je       @@checkSorting

   mov      (PolySurface PTR [edx]).x_end, eax

@@checkSorting:
   ; edi's current x is in ecx
   ; prev is in ebp

   cmp      ecx, (PolyEdge PTR [ebp]).x
   jge      @@insertNewEdges
@@nextSortCheck:
   mov      eax, (PolyEdge PTR [ebp]).pprev
   mov      ebx, (PolyEdge PTR [edi]).pnext

   ; pprev in ebp
   ; pprev->pprev in eax
   ; pnext in ebx

   ; prev->next = pnext
   mov      (PolyEdge PTR [ebp]).pnext, ebx
   ; aelWalk->pprev = pprev->pprev
   mov      (PolyEdge PTR [edi]).pprev, eax
   ; next->pprev = prev
   mov      (PolyEdge PTR [ebx]).pprev, ebp
   ; aelWalk->pnext = prev
   mov      (PolyEdge PTR [edi]).pnext, ebp
   ; pprev->pprev = aelWalk
   mov      (PolyEdge PTR [ebp]).pprev, edi
   ; aelWalk->pprev->pnext = aelWalk
   mov      (PolyEdge PTR [eax]).pnext, edi

   mov      ebp, eax

   cmp      ecx, (PolyEdge PTR [eax]).x
   jl       @@nextSortCheck

@@insertNewEdges:
   cmp      esi, 0
   je       @@nextAELCheck
   mov      eax, (PolyEdge PTR [esi]).x
   cmp      ecx, eax
   jle      @@nextAELCheck

   mov      ebx, (PolyEdge PTR [esi]).pleadsurf
	add		eax, XFixedPointCeilStep

	shr		eax, XFixedPointShift
   mov      edx, (PolyEdge PTR [esi]).ptrailsurf

   cmp      ebx, 0
   je       @@noAddLeadSurface

   mov      (PolySurface PTR [ebx]).xstart, eax
   and      (PolySurface PTR [ebx]).flags, NOT FLAG_ALREADYDRAWN
@@noAddLeadSurface:
   cmp      edx, 0
   je       @@insertEdge
   mov      (PolySurface PTR [edx]).x_end, eax
@@insertEdge:
   mov      edx, (PolyEdge PTR [edi]).pprev
   ; nextAdd = addWalk->pnext
   mov      eax, (PolyEdge PTR [esi]).pnext
   ; addWalk->pnext = aelWalk
   mov      (PolyEdge PTR [esi]).pnext, edi
   ; addWalk->pprev = aelWalk->pprev
   mov      (PolyEdge PTR [esi]).pprev, edx
   ; addWalk->pprev->pnext = addWalk
   mov      (PolyEdge PTR [edx]).pnext, esi
   ; aelWalk->pprev = addWalk
   mov      (PolyEdge PTR [edi]).pprev, esi
   mov      esi, eax
   jmp      @@insertNewEdges

@@nextAELCheck:
   mov      edi, nextSave
   cmp      edi, 0
   jne      @@testNextActiveEdge

@@constructActivePolyList:
   ; activePolyList = eax = 0
   ; transPolyList = ebx = 0
   ; aelWalk = edi = edgetail.pprev

	fild	   curY				; fp stack = y

   mov      edi, _edgetail.pprev
   mov      eax, 0
   mov      ebx, 0
   mov      backgroundPoly, 0
   cmp      edi, OFFSET _edgehead
   je       @@APLConstructed
@@constructNext:
   fld      st
   mov      ecx, (PolyEdge PTR [edi]).pleadsurf
   mov      edx, (PolyEdge PTR [edi]).pprev
   cmp      ecx, 0
   je       @@nextConstructEdge
   fmul     (PolySurface PTR [ecx]).dwdy
   mov      ebp, (PolySurface PTR [ecx]).xstart
   mov      esi, (PolySurface PTR [ecx]).x_end
   cmp      ebp, esi
   jge      @@nextConstructEdge
   mov      esi, (PolySurface PTR [ecx]).flags
   fadd     (PolySurface PTR [ecx]).w0
   test     esi, FLAG_ALREADYDRAWN
   jnz      @@nextConstructEdge
   test     esi, FLAG_TRANSPARENT
   jz       @@constructPolySolid
@@constructPolyTransparent:
   mov      (PolySurface PTR [ecx]).nextPoly, ebx
   mov      ebx, ecx
   jmp      @@finishConstruct
@@constructPolySolid:
   mov      (PolySurface PTR [ecx]).nextPoly, eax
   mov      eax, ecx
@@finishConstruct:
   or       esi, FLAG_ALREADYDRAWN
   fst      (PolySurface PTR [ecx]).curW
   mov      (PolySurface PTR [ecx]).flags, esi
@@nextConstructEdge:
   fstp     st
   mov      edi, edx
   cmp      edx, OFFSET _edgehead
   jne      @@constructNext
@@APLConstructed:

   fstp     st

   ; curPoly in eax
   ; transPolyList in ebx

   mov      clipIndex, 0
   mov      transPolyList, ebx

   cmp      eax, 0
   je       @@emitLastSpansAndLoop
   ; at the start of the loop, eax holds curPoly, ebx holds transPolyList
@@testTransPolyList:
   cmp      ebx, 0
   je       @@skipTransClip

   mov      transPolyList, 0
@@transPolyLoop:   
   mov      ecx, (PolySurface PTR [ebx]).xstart
   mov      edx, (PolySurface PTR [eax]).x_end
   cmp      ecx, edx
   jge      @@transListProcessed
   mov      edi, (PolySurface PTR [ebx]).x_end
   mov      esi, (PolySurface PTR [eax]).xstart
   cmp      edi, esi
   jg       @@transPolyOverlapped
   ; emit(transPoly>xstart, transPoly->x_end, transPoly->myPoly)
   mov      edx, (PolySurface PTR [ebx]).myPoly
   mov      ebx, (PolySurface PTR [ebx]).nextPoly
   call     @emitSpan
   cmp      ebx, 0
   jne      @@transPolyLoop
   jmp      @@transListProcessed
@@transPolyOverlapped:
   ; transPoly->xstart in ecx
   ; curPoly->xstart in esi
   test     (PolySurface PTR [eax]).flags, FLAG_ALWAYS_BEHIND
   jnz      @@transCurPolyAlwaysBehind
   cmp      ecx, esi
   jg       @@transXStDone
   mov      ecx, esi
@@transXStDone:
   mov      saveX, ecx
   mov      ecx, (PolySurface PTR [ebx]).myPoly
   mov      esi, (PolySurface PTR [eax]).myPoly
   ; st = y
   fld      (PolySurface PTR [esi]).dwdx
   fld      (PolySurface PTR [ecx]).dwdx
   fld      (PolySurface PTR [esi]).curW
   fld      (PolySurface PTR [ecx]).curW

   ; st = tp->curW, cp->curW, tp->dwdx, cp->dwdx, y

   fild     saveX
   fld      st

   ; st0    st1      st2      st3      st4      st5      st6      st7
   ; x      x        tp->curW cp->curW tp->dw   cp->dw   curY

   fmul     st, st(5)
   fxch     st(1)
   
   ; x      cp->cx   tp->curW cp->curW tp->dw   cp->dw   curY
   dec      edx
   dec      edi
   fmul     st, st(4)
   ; tp->cx cp->cx   tp->curW cp->curW tp->dw   cp->dw   curY
   fxch     st(1)
   fadd     st, st(3)
   fxch     st(1)
   fadd     st, st(2)
   ; tp->w  cp->w    tp->curW cp->curW tp->dw   cp->dw   curY

   ; transPoly->x_end-1 in edi
   ; curPoly->x_end-1 in edx
   cmp      edx, edi
   jle      @@transx_endDone
   mov      edx, edi
@@transx_endDone:
   mov      saveX, edx
   fsubrp   st(1), st

   fild     saveX
   fld      st

   fxch     st(2)
   fstp     status
   
   ; x      x        tp->curW cp->curW tp->dw   cp->dw   curY
   fmul     st, st(5)
   fxch     st(1)
   
   ; x      cp->cx   tp->curW cp->curW tp->dw   cp->dw   curY
   fmul     st, st(4)
   ; tp->cx cp->cx   tp->curW cp->curW tp->dw   cp->dw   curY
   fxch     st(1)
   fadd     st, st(3)
   fxch     st(1)
   fadd     st, st(2)
   ; tp->w  cp->w    tp->curW cp->curW tp->dw   cp->dw   curY
   mov      esi, status
   fsubrp   st(1), st
   mov      edi, status
   fstp     status
   mov      edx, status

   ; startStatus in esi, edi
   ; endStatus in edx

   or       edi, edx
   and      edx, esi

   ; statusOr in edi
   ; statusAnd in edx
   ; startStatus still in esi
   ; curPoly in eax, transPoly in ebx, transPoly->myPoly in ecx

   ; if statusOr sign bit is not set then startStatus >= 0 && endStatus >= 0
   ; else if statusAnd sign bit is set then startStatus <= 0 && endStatus <= 0
   ; else there was an intersection

   test     edi, 080000000h
   jnz      @@transAllBehindTest
   mov      edx, ecx
   mov      ecx, (PolySurface PTR [ebx]).xstart
   mov      edi, (PolySurface PTR [eax]).xstart
   call     @emitSpan
   mov      ecx, (PolySurface PTR [eax]).x_end
   mov      edi, ebx
   mov      edx, (PolySurface PTR [ebx]).x_end
   mov      ebx, (PolySurface PTR [ebx]).nextPoly

   cmp      ecx, edx
   jge      @@nextTransCheck
   mov      (PolySurface PTR [edi]).xstart, ecx

   ; tricky me...
   ; push the return address and jump (avoids an extra jump or extra code)

   push     OFFSET @@nextTransCheck
   jmp      @insertReverse
@@transAllBehindTest:
   test     edx, 080000000h
   jz       @@transIntersected
@@transCurPolyBehind:
   mov      edi, ebx
   mov      ebx, (PolySurface PTR [ebx]).nextPoly
   ; if curPoly is totally behind transPoly
   ; just insertReverse this sucker and continue on
   call     @insertReverse
@@nextTransCheck:
   fstp     st
   fstp     st
   fstp     st
   fstp     st
@@nextTransCheckPopped:
   cmp      ebx, 0
   jne      @@transPolyLoop
   jmp      @@transListProcessed
@@transCurPolyAlwaysBehind:
   mov      edi, ebx
   mov      ebx, (PolySurface PTR [ebx]).nextPoly
   ; if curPoly is totally behind transPoly
   ; just insertReverse this sucker and continue on
   call     @insertReverse
   cmp      ebx, 0
   jne      @@transPolyLoop
   jmp      @@transListProcessed
@@transIntersected:
   ;  st0      st1      st2      st3      st4      st5      st6      st7
   ;  tp->curW cp->curW tp->dw   cp->dw   curY
   
   ; xPoint = (tp->curW - cp->curW) / (cp->dwdx - tp->dwdx)

   fsubrp      st(1), st

   ;  tcw-ccw  tp->dwdx cp->dwdx curY
   fxch        st(1)
   fsubp       st(2), st
   ; tcw-ccw   cdw-tdw  curY
   fdivrp      st(1), st

   ; if startStatus > 0
   test     esi, 080000000h
   jnz      @@transPolyIntersectBehind

   mov      edx, ecx
   mov      ecx, (PolySurface PTR [ebx]).xstart
   mov      edi, (PolySurface PTR [eax]).xstart
   call     @emitSpan
   mov      edi, ebx
   push     OFFSET @@nextTransCheckPopped
   fistp    (PolySurface PTR [ebx]).xstart
   mov      ebx, (PolySurface PTR [ebx]).nextPoly
   jmp      @insertReverse

@@transPolyIntersectBehind:
   mov      edx, clipIndex
   mov      edi, (PolySurface PTR [ebx]).x_end
   mov      esi, (PolySurface PTR [eax]).x_end
   mov      ebp, edx
   cmp      edi, esi
   jle      @@transNoExtraSpan
   cmp      edx, MAX_CLIP_SPANS
   jge      @@transNoExtraSpan
   shl      ebp, 4   ;sizeof(ClippedPolyStore) is 16 bytes
   inc      edx
   lea      ebp, [ClipSurfaces + ebp]
   mov      clipIndex, edx

   ; agi DOH

   mov      (PolySurface PTR [ebp]).xstart, esi
   mov      (PolySurface PTR [ebp]).x_end, edi
   mov      (PolySurface PTR [ebp]).myPoly, ecx
   mov      edi, ebp
   call     @insertReverse
@@transNoExtraSpan:
   fistp    (PolySurface PTR [ebx]).x_end
   mov      edi, ebx
   mov      ebx, (PolySurface PTR [ebx]).nextPoly
   call     @insertReverse      
   cmp      ebx, 0
   jne      @@transPolyLoop
@@transListProcessed:
   mov      ecx, transPolyList
   cmp      ebx, 0
   jz       @@reverseList

   cmp      ecx, 0
   jz       @@listReversed

   ; transPoly in ebx, transPolyList in ecx
@@insertTransListLoop:
   ; transTemp in ebp, transTemp->next in esi
   mov      esi, (PolySurface PTR [ebx]).nextPoly
   mov      ebp, ebx
   mov      edx, (PolySurface PTR [ebx]).xstart
   mov      edi, (PolySurface PTR [ecx]).xstart
   cmp      edi, edx
   jle      @@reverseList
@@insertTransListSubLoop:
   cmp      esi, 0
   je       @@insertTransListBreak
   cmp      edi, (PolySurface PTR [esi]).xstart
   jle      @@insertTransListBreak
   mov      ebp, esi
   mov      esi, (PolySurface PTR [esi]).nextPoly
   jmp      @@insertTransListSubLoop
@@insertTransListBreak:
   ; temp = transPolyList->nextPoly
   mov      edx, (PolySurface PTR [ecx]).nextPoly
   ; transPolyList->nextPoly = transTemp->nextPoly
   mov      (PolySurface PTR [ecx]).nextPoly, esi
   ; transTemp->nextPoly = transPolyList
   mov      (PolySurface PTR [ebp]).nextPoly, ecx
   mov      ecx, edx
   cmp      edx, 0
   jne      @@insertTransListLoop
   mov      transPolyList, ebx
   jmp      @@skipTransClip
@@reverseList:
   ; just reverse the transPolyList
   cmp      ecx, 0
   jz       @@listReversed
@@reverseListLoop:
   ; temp = transPolyList->nextPoly
   mov      edx, (PolySurface PTR [ecx]).nextPoly
   ; transPolyList->nextPoly = transPoly
   mov      (PolySurface PTR [ecx]).nextPoly, ebx
   ; transPoly= transPolyList
   mov      ebx, ecx
   ; transPolyList = temp
   mov      ecx, edx
   cmp      edx, 0
   jne      @@reverseListLoop
@@listReversed:
   mov      transPolyList, ebx
@@skipTransClip:
   ; ok, from this point on ebx is nextPoly
   mov      ebx, (PolySurface PTR [eax]).nextPoly
   cmp      ebx, 0
   je       @@emitLastSpansAndLoop
   mov      edi, (PolySurface PTR [eax]).x_end
   mov      ecx, (PolySurface PTR [ebx]).xstart
   cmp      ecx, edi
   jl       @@spansOverlapInX
   mov      ecx, (PolySurface PTR [eax]).xstart
   mov      edx, (PolySurface PTR [eax]).myPoly
   mov      eax, ebx
   mov      ebx, transPolyList
   push     OFFSET @@testTransPolyList
   jmp      @emitSpan

@@spansOverlapInX:
   mov      ebp, (PolySurface PTR [eax]).flags
   mov      ecx, (PolySurface PTR [eax]).myPoly
   mov      esi, (PolySurface PTR [ebx]).flags
   mov      edx, (PolySurface PTR [ebx]).myPoly

   test     ebp, FLAG_ALWAYS_BEHIND
   jnz      @@nextPolyTotallyInFront
   test     esi, FLAG_ALWAYS_BEHIND
   jnz      @@curPolyTotallyInFront

   fild     (PolySurface PTR [ebx]).xstart
   mov      esi, (PolySurface PTR [ebx]).x_end
   cmp      esi, edi
   jl       @@minCalced
   mov      esi, edi
@@minCalced:
   dec      esi
   fld      st

   fmul     (PolySurface PTR [ecx]).dwdx
   fld      (PolySurface PTR [ecx]).curW
   fxch     st(2)
   mov      saveX, esi
   fmul     (PolySurface PTR [edx]).dwdx
   fxch     st(1)
   faddp    st(2), st
   fild     saveX
   fxch     st(1)
   fadd     (PolySurface PTR [edx]).curW
   fxch     st(1)

   ; st = x_end nextPolyWStart curPolyWStart y
   fld      st
   ; st = x_end x_end nextPolyWStart curPolyWStart y
   fmul     (PolySurface PTR [ecx]).dwdx
   ; st = curXE x_end nextPolyWStart curPolyWStart y
   fxch     st(2)
   fsubrp   st(3), st
   ; st = x_end curXE startStatus y
   fmul     (PolySurface PTR [edx]).dwdx
   fxch     st(1)
   fadd     (PolySurface PTR [ecx]).curW
   fxch     st(2)
   fstp     status
   fadd     (PolySurface PTR [edx]).curW
   mov      edi, status
   mov      esi, status
   fsubrp   st(1), st
   fstp     status
   mov      ebp, status

   and      edi, ebp
   or       ebp, esi

   ; startStatus in esi
   ; orStatus in ebp
   ; andStatus in edi

   ; curPoly in eax, nextPoly in ebx
   ; curPoly->myPoly in ecx, nextPoly->myPoly in edx
   
   test     ebp, 080000000h
   jnz      @@testCurPolyBehind
@@curPolyTotallyInFront:
   ; curPoly is totally in front
   mov      ecx, (PolySurface PTR [eax]).x_end
   mov      edi, (PolySurface PTR [ebx]).nextPoly
   mov      edx, (PolySurface PTR [ebx]).x_end
   mov      (PolySurface PTR [eax]).nextPoly, edi
   cmp      ecx, edx
   jge      @@skipTransClip
   mov      (PolySurface PTR [ebx]).xstart, ecx
   mov      ecx, eax
   mov      edx, ebx
   push     OFFSET @@skipTransClip
   jmp      @resort
@@testCurPolyBehind:
   ; check if curPoly is totally behind nextPoly
   test     edi, 080000000h
   jz       @@curAndNextPolyIntersected

@@nextPolyTotallyInFront:
   mov      edx, ecx
   mov      ecx, (PolySurface PTR [eax]).xstart
   mov      edi, (PolySurface PTR [ebx]).xstart
   call     @emitSpan
   mov      edi, (PolySurface PTR [eax]).x_end
   mov      ecx, ebx
   mov      esi, (PolySurface PTR [ebx]).x_end
   mov      edx, eax
   mov      eax, ebx
   mov      ebx, transPolyList
   cmp      edi, esi
   jle      @@testTransPolyList
   push     OFFSET @@testTransPolyList
   mov      (PolySurface PTR [edx]).xstart, esi
   jmp      @resort
@@outOfClipSpans:
   fstp     st
   jmp      @@curPolyTotallyInFront
@@curAndNextPolyIntersected:   
   fld      (PolySurface PTR [edx]).curW
   fsub     (PolySurface PTR [ecx]).curW
   fld      (PolySurface PTR [ecx]).dwdx
   fsub     (PolySurface PTR [edx]).dwdx
   mov      edx, (PolySurface PTR [eax]).x_end
   mov      ebp, clipIndex
   mov      edi, (PolySurface PTR [ebx]).x_end
   fdivp    st(1), st

   test     esi, 080000000h
   jnz      @@intersectStatusNegative
   cmp      edx, edi
   jle      @@positiveNoExtraSpan
   cmp      ebp, MAX_CLIP_SPANS
   jge      @@outOfClipSpans
   lea      esi, [ebp*8]
   inc      ebp
   lea      esi, [ClipSurfaces + esi*2]
   mov      clipIndex, ebp
   mov      (PolySurface PTR [esi]).xstart, edi
   mov      (PolySurface PTR [esi]).x_end, edx
   mov      (PolySurface PTR [esi]).myPoly, ecx
   mov      ecx, eax
   mov      edx, esi
   call     @resort
@@positiveNoExtraSpan:
   fistp    (PolySurface PTR [eax]).x_end
   mov      esi, (PolySurface PTR [ebx]).nextPoly
   mov      edi, (PolySurface PTR [eax]).x_end
   mov      (PolySurface PTR [eax]).nextPoly, esi
   mov      (PolySurface PTR [ebx]).xstart, edi
   mov      ecx, eax
   mov      edx, ebx
   push     OFFSET @@skipTransClip
   jmp      @resort
@@intersectStatusNegative:
   ; nextPoly starts in front
   cmp      edi, edx
   jle      @@negativeNoExtraSpan
   cmp      ebp, MAX_CLIP_SPANS
   jge      @@outOfClipSpans
   lea      esi, [ebp*8]
   inc      ebp
   lea      esi, [ClipSurfaces + esi*2]
   mov      clipIndex, ebp
   mov      ecx, (PolySurface PTR [ebx]).myPoly
   mov      (PolySurface PTR [esi]).xstart, edx
   mov      (PolySurface PTR [esi]).x_end, edi
   mov      (PolySurface PTR [esi]).myPoly, ecx
   mov      edx, esi
   mov      ecx, ebx
   call     @resort
@@negativeNoExtraSpan:
   mov      ecx, (PolySurface PTR [eax]).xstart
   mov      edx, (PolySurface PTR [eax]).myPoly
   mov      edi, (PolySurface PTR [ebx]).xstart
   call     @emitSpan
   fistp    (PolySurface PTR [eax]).xstart
   mov      edx, eax
   mov      edi, (PolySurface PTR [eax]).xstart
   mov      eax, ebx
   mov      (PolySurface PTR [ebx]).x_end, edi
   mov      ecx, ebx
   mov      ebx, transPolyList
   push     OFFSET @@testTransPolyList
   jmp      @resort
@@emitLastSpansAndLoop:
   cmp      eax, 0
   je       @@transEndLoopTest
   mov      ecx, (PolySurface PTR [eax]).xstart
   mov      edi, (PolySurface PTR [eax]).x_end
   mov      edx, (PolySurface PTR [eax]).myPoly
   call     @emitSpan
@@transEndLoopTest:
   mov      ebx, transPolyList
   cmp      ebx, 0
   je       @@scanlineDone
@@transEmitLoop:
   mov      ecx, (PolySurface PTR [ebx]).xstart
   mov      edi, (PolySurface PTR [ebx]).x_end
   mov      edx, (PolySurface PTR [ebx]).myPoly
   mov      ebx, (PolySurface PTR [ebx]).nextPoly
   call     @emitSpan
   cmp      ebx, 0
   jne      @@transEmitLoop
@@scanlineDone:
   inc      curY
   jmp      @@yLoop

@@exit:
	mov		eax, _nextFreeBlock ; return the number of used spans.
	pop		ebp
	pop		esi
	pop		edi
	pop		ecx
	pop		ebx
	ret
gfx_scan_edges_z ENDP

END
