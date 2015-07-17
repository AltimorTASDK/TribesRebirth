;================================================================
;
;         $Author: Dmoore $
;           $Date: 1/26/98 10:37p $
;       $Revision: 6 $
;        $Archive: /Niqolas/code/vector.asm $
;
; DESCRIPTION:
;
; (c) Copyright Tensor Products, Inc.   All rights reserved.
;================================================================

.486p
.MODEL FLAT, SYSCALL

IFDEF __MASM__
   ARG equ
ENDIF

.DATA

include vector.inc


fpcnst0              dq    0.0
fpcnst1              dq    1.0
fpcnstLudicrous      dq    1.0e200

currentWeight  dq    ?
junkFp         dq    ?

_numVectors    dd    ?
_numDim        dd    ?
_pVectors      dd    ?

_ebp           dd    ?
_numDimensions dd    ?
lastVectorPtr  dd    ?


.CODE


asmGetStandardDev PROC C       \
      USES eax ebx ecx edx edi esi,
      ARG pCenterOfMass:DWORD, \
          pVectors:DWORD,      \
          numVectors:DWORD
      public C asmGetStandardDev

   ; Invariant register assignments:
   ;  ecx -    count remaining vectors
   ;  edx -    count remaining dimensions
   ;  esi -    point to elements of pCenterOfMass
   ;  edi -    points to current sortVector
   

   mov   esi, pCenterOfMass
   mov   esi, (quantVector PTR [esi]).pElem
   
   mov   edi, pVectors
   
   mov   ecx, numVectors
   mov   _numVectors, ecx
   
   mov   edx, (sortVector PTR [edi]).pQuantVector
   mov   edx, (quantVector PTR [edx]).numDim
   mov   _numDim, edx

   ; Top most stack entry is standard deviation
   fld   fpcnst0

   ; Begin Vector Loop...
   mov   ecx, _numVectors
@@vectorLoopSD:
   ; Load zero to escape special case crap.
   fld   fpcnst0

   mov   eax, 0         ; eax will hold offset into array of doubles
                        ; ebx holds current vectors pElem
   mov   ebx, (sortVector PTR [edi]).pQuantVector
   
   ; Store off the wieght of the vector
   fld   (quantVector PTR [ebx]).weight
   fstp  currentWeight

   mov   ebx, (quantVector PTR [ebx]).pElem
   mov   edx, _numDim
@@dimensionLoopSD:
   fld   currentWeight
   fld   QWORD PTR [esi + eax]
   fld   QWORD PTR [ebx + eax]
   ; st: pElem(currDim), COM(currDim), currentWeight, sumSoFar
   fsub
   fmul  st(0), st(0)
   fmul
   fadd
   ; st: sumSoFar

   add   eax, 8
   dec   edx
   jnz   @@dimensionLoopSD

   ; When we come out of this loop, the standard Dev for the previous vector
   ; is at the top of the stack.  Add it to the total deviation, which is the
   ; next entry up.
   fadd

   ; step to the next vector
   add   edi, SVEC_SIZE

   dec   ecx
   jnz   @@vectorLoopSD

   ; And we're done!  We return the standardDev in the first entry of
   ; the stack anyways, so we'll just leave it there...
   ret
asmGetStandardDev ENDP



asmCalcCenterOfMass PROC C       \
      USES eax ebx ecx edx edi esi,
      ARG pCenterOfMass:DWORD, \
          pVectors:DWORD,      \
          numVectors:DWORD,    \
          totalWeight:QWORD
      public C asmCalcCenterOfMass

   ; Invariant register assignments:
   ;  ecx -    count remaining vectors
   ;  edx -    count remaining dimensions
   ;  esi -    point to elements of pCenterOfMass
   ;  edi -    points to current sortVector
   

   mov   esi, pCenterOfMass
   mov   esi, (quantVector PTR [esi]).pElem
   
   mov   edi, pVectors
   mov   _pVectors, edi
   
   mov   ecx, numVectors
   mov   _numVectors, ecx

   mov   eax, 0         ; eax will hold offset into array of doubles
                        ; ebx holds current vectors pElem
   
   mov   edx, (sortVector PTR [edi]).pQuantVector
   mov   edx, (quantVector PTR [edx]).numDim
   
   fld   fpcnst1
   fld   totalWeight
   fdiv
@@dimensionLoopCM:
   ; Average for this dimension is first on stack.
   fld   fpcnst0

   mov   edi, _pVectors
   mov   ecx, _numVectors
@@vectorLoopCM:
   mov   ebx, (sortVector PTR [edi]).pQuantVector
   fld   (quantVector PTR [ebx]).weight
   mov   ebx, (quantVector PTR [ebx]).pElem
   fld   QWORD PTR [ebx + eax]
   fmul
   fadd

   add   edi, SVEC_SIZE
   dec   ecx
   jnz   @@vectorLoopCM
   
   fxch  st(1)
   fld   st(0)
   fxch  st(2)
   fmul

   fstp   QWORD PTR [esi + eax]

   add   eax, 8
   dec   edx
   jnz   @@dimensionLoopCM

   ; Restore balanced fp stack...
   fstp  junkFp

   ret
asmCalcCenterOfMass ENDP


asmCalcCovarMatrix PROC C       \
      USES eax ebx ecx edx edi esi,
      ARG pCenterOfMass:DWORD, \
          pOutputMatrix:DWORD, \
          numDimensions:DWORD, \
          pVectors:DWORD,      \
          numVectors:DWORD,    \
          totalWeight:QWORD
      public C asmCalcCovarMatrix

   mov   eax, 0                              ; EAX Tracks J index

   mov   esi, pCenterOfMass                  ; ESI points to COM

@@covarJLoopPt:
   cmp   eax, numDimensions
   jge   @@doneWithCovarMatrix

   mov   ebx, 0                              ; EBX Tracks I index

@@covarILoopPt:
   cmp   ebx, eax
   jg    @@doneWithRow
   
   ; To prevent any first time through nonsense
   fld   fpcnst0

   mov   ecx, numVectors
   mov   edx, pVectors
@@vectorLoop:
   mov   edi, (sortVector PTR [edx]).pQuantVector
   add   edx, SVEC_SIZE

   fld   (quantVector PTR [edi]).weight
   mov   edi, (quantVector PTR [edi]).pElem

                                             ; Implied top of stack is sumTotal
   fld   QWORD PTR [edi + eax * 8]           ; ST = v_j, , weight
   fld   QWORD PTR [esi + eax * 8]           ; ST = cm_j, v_j, weight
   fsub                                      ; ST = (v_j - cm_j), weight

   fmul                                      ; ST = ((v_j - cm_j) * weight)
   fld   QWORD PTR [edi + ebx * 8]           ; ST = v_i, ((v_j - cm_j) * weight)
   fld   QWORD PTR [esi + ebx * 8]           ; ST = cm_i, v_i, ((v_j - cm_j) * weight)
   fsub                                      ; ST = (v_i - cm_i), ((v_j - cm_j) * weight)

   fmul                                      ; ST = ((v_i * cm_i) * (v_j - cm_j) * weight))

   fadd

   dec   ecx
   jnz   @@vectorLoop

   ; Write to appropriate output point...
   ;
   fld   totalWeight
   fdiv

   mov   edi, [pOutputMatrix]
   mov   edi, [edi + ebx * 4]
   fstp  QWORD PTR [edi + eax * 8]

   inc   ebx
   jmp   @@covarILoopPt

@@doneWithRow:

   inc   eax
   jmp   @@covarJLoopPt

@@doneWithCovarMatrix:

   ret
asmCalcCovarMatrix ENDP


asmFindClosestVectorNonEx PROC C       \
      USES ebx ecx edx edi esi,
      ARG pTargetVector:DWORD,         \
          pSourceVectors:DWORD,        \
          numSourceVectors:DWORD,      \
          numDimensions:DWORD
      public C asmFindClosestVectorNonEx

   mov   eax, QVEC_SIZE
   mov   ebx, numSourceVectors
   imul  ebx
   add   eax, pSourceVectors
   mov   lastVectorPtr, eax

   mov   ebx, numDimensions
   mov   _numDimensions, ebx

   mov   ebx, NOT 0                          ; EBX holds minSoFar
   fld   fpcnstLudicrous                     ; Load ludicrous onto the stack

   mov   esi, pTargetVector
   mov   esi, (quantVector PTR [esi]).pElem

   mov   _ebp, ebp
   mov   ecx, [pSourceVectors]               ; ECX is current vector ptr
   mov   ebp, 0
@@sourceVectorLoop:
   mov   edi, (quantVector PTR [ecx]).pElem
   fld   fpcnst0

   mov   edx, 0
@@dimLoop:
   fld   QWORD PTR [esi + edx * 8]
   fld   QWORD PTR [edi + edx * 8]
   fsub

   fld   st
   fmul
   fadd

   inc   edx
   cmp   edx, _numDimensions
   jl    @@dimLoop

   fcom   st(1)
   fnstsw ax
   test   ah,1
   je     @@notLessThan

   fxch  st(1)
   mov   ebx, ebp

@@notLessThan:
   fstp  junkFp

   inc   ebp
   add   ecx, QVEC_SIZE
   cmp   ecx, lastVectorPtr
   jl    @@sourceVectorLoop

   ; Restore balanced fp stack...
   fstp  junkFp

   mov   eax, ebx
   mov   ebp, _ebp

   ret
asmFindClosestVectorNonEx ENDP




END
