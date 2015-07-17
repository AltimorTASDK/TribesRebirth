; ========================================================
;
;         $Author: Dmoore $
;           $Date: 2/04/97 10:01p $
;       $Revision: 11 $
;        $Archive: /NicQolas/code/sVector.cpp $
;
;    Assembly version of sVector.h
;	
; (c) Copyright Tensor Products Inc.  All rights reserved.
; ========================================================

; ========================================================

quantVector STRUC
   numDim   DD ?
   pElem    DD ?
   weight   DQ ?
quantVector ENDS

sortVector STRUC
   pQuantVector   DD ?
   sortKey        DQ ?
sortVector ENDS


QVEC_SIZE equ SIZE quantVector
SVEC_SIZE equ SIZE sortVector