;
; $Workfile:   m_mat.inc  $
; $Revision:   1.2  $
; $Version: 1.6.2 $
; $Date:   05 Apr 1995 11:33:54  $
;	

INCLUDE m_vec.inc

Matrix_HasRotation 		EQU 1
Matrix_HasTranslation 	EQU 2
Matrix_HasScale 		EQU 4

RMat2 STRUC
	flags	DD ?
	m2_0_0	DD ?
	m2_0_1	DD ?
	m2_1_0	DD ?
	m2_1_1	DD ?
RMat2 ENDS

TMat2 STRUC 
	flags	DD ?
	m2_0_0	DD ?
	m2_0_1	DD ?
	m2_1_0	DD ?
	m2_1_1	DD ?
	t2x	DD ?
	t2y	DD ?
TMat2 ENDS

RMat3 STRUC 
	flags	DD ?
	m3_0_0	DD ?
	m3_0_1	DD ?
	m3_0_2	DD ?
	m3_1_0	DD ?
	m3_1_1	DD ?
	m3_1_2	DD ?
	m3_2_0	DD ?
	m3_2_1	DD ?
	m3_2_2	DD ?
RMat3 ENDS

TMat3 STRUC
	flags	DD ?
	m3_0_0	DD ?
	m3_0_1	DD ?
	m3_0_2	DD ?
	m3_1_0	DD ?
	m3_1_1	DD ?
	m3_1_2	DD ?
	m3_2_0	DD ?
	m3_2_1	DD ?
	m3_2_2	DD ?
	t3x	DD ?
	t3y	DD ?
	t3z	DD ?
TMat3 ENDS



; ----------------------------------------------------------------
; Macro rc2mul
;
; Multiplies row of mat1 by column of mat2 and stores result in mat3
; ebx - mat1 ptr
; esi - mat2 ptr
; edi - mat3 ptr
;
;	uses: eax,ecx,edx,ebp
;
rc2mul MACRO row, col
      mov	eax,(RMat2 PTR [ebx]).m2_&row&_0
      imul	(RMat2 PTR [esi]).m2_0_&col&
      mov       ecx,eax
      mov       ebp,edx

      mov	eax,(RMat2 PTR [ebx]).m2_&row&_1
      imul	(RMat2 PTR [esi]).m2_1_&col&
      add	eax,ecx
      adc       edx,ebp

      add	eax,FIX_ROUND
      adc	edx,0
      shrd	eax,edx,FIX_SHIFT
      mov	(RMat2 PTR [edi]).m2_&row&_&col&,eax
ENDM


; ----------------------------------------------------------------
; Macro vcr2mul,vct2mul,vct3zmul
;
; Multiplies vec1 by column of mat2 and stores result in vec3.
; If mat2 is a RMat2, use vcr2mul; if mat2 is a TMat2, use vct2mul.
; If mat2 is a single (z) axis TMat3, use vct3zmul.
;
; vcr2mul:
;   C.v = (A.x * B.0_c) + (A.y * B.1_c)
;
; vct2mul:
; vct3zmul:
;   C.v = (A.x * B.0_c) + (A.y * B.1_c) + B.tv
;
; ebx - vec1 ptr (A)
; esi - mat2 ptr (B)
; edi - vec3 ptr (C)
;
;	uses: eax,ecx,edx,ebp
;
vc2mul MACRO  col,vec_elem,axis
      mov	eax,(Vector3 PTR [ebx]).x
      imul	(TMat2 PTR [esi]).m2_0_&col&
      mov	ebp,eax
      mov	ecx,edx

      mov	eax,(Vector3 PTR [ebx]).y
      imul	(TMat2 PTR [esi]).m2_1_&col&
      add	eax,ebp
      adc	edx,ecx

      add	eax,FIX_ROUND
      adc	edx,0
      shrd  eax,edx,FIX_SHIFT

      IFNB <axis>
	    IF axis EQ 2 
            add   eax,(TMat2 PTR [esi]).t&axis&vec_elem&
            ENDIF

            IF axis EQ 3
            add   eax,(TMat3 PTR [esi]).t&axis&vec_elem&
            ENDIF
      ENDIF
      mov	(Vector3 PTR [edi]).&vec_elem&,eax	
ENDM	


vcr2mul MACRO   col,vec_elem
      vc2mul  col,vec_elem
      ENDM


vct2mul MACRO   col,vec_elem
      vc2mul  col,vec_elem,2
      ENDM


vct3zmul MACRO   col,vec_elem
      vc2mul  col,vec_elem,3
      ENDM


; ----------------------------------------------------------------
; Macro rc3mul
;
; Multiplies row of mat1 by column of mat2 and stores result in mat3
;
;   C.r_c = (A.r_0 * B.0_c) + (A.r_1 * B.1_c) + (A.r_2 * B.2_c)
;
; ebx - mat1 ptr (A)
; esi - mat2 ptr (B)
; edi - mat3 ptr (C)
;
;	uses: eax,ecx,edx,ebp
;
rc3mul MACRO row, col
      mov   eax,(RMat3 PTR [ebx]).m3_&row&_0
      imul  (RMat3 PTR [esi]).m3_0_&col&
      mov   ecx,eax
      mov   ebp,edx

      mov   eax,(RMat3 PTR [ebx]).m3_&row&_1
      imul  (RMat3 PTR [esi]).m3_1_&col&
      add   ecx,eax
      adc   ebp,edx

      mov   eax,(RMat3 PTR [ebx]).m3_&row&_2
      imul  (RMat3 PTR [esi]).m3_2_&col&
      add   eax,ecx
      adc   edx,ebp

      add   eax,FIX_ROUND
      adc   edx,0
      shrd  eax,edx,FIX_SHIFT
      mov   (RMat3 PTR [edi]).m3_&row&_&col&,eax
      ENDM


; ----------------------------------------------------------------
; Macro vcr3mul,vct3mul
;
; Multiplies vec1 by column of mat2 and stores result in vec3.
; If mat2 is a RMat3, use vcr3mul; if mat2 is a TMat3, use vct3mul.
;
; vcr3mul:
;   C.v = (A.x * B.0_c) + (A.y * B.1_c) + (A.z * B.2_c)
;
; vcr3mul:
;   C.v = (A.x * B.0_c) + (A.y * B.1_c) + (A.z * B.2_c) + B.tv
;
; ebx - vec1 ptr (A)
; esi - mat2 ptr (B)
; edi - vec3 ptr (C)
;
;	uses: eax,ecx,edx,ebp
;
vc3mul MACRO col,vec_elem,axis
      mov   eax,(Vector3 PTR [ebx]).x
      imul  (TMat3 PTR [esi]).m3_0_&col&
      mov   ebp,eax
      mov   ecx,edx

      mov   eax,(Vector3 PTR [ebx]).y
      imul  (TMat3 PTR [esi]).m3_1_&col&
      add   ebp,eax
      adc   ecx,edx

      mov   eax, (Vector3 PTR [ebx]).z
      imul  (TMat3 PTR [esi]).m3_2_&col&
      add   eax,ebp
      adc   edx,ecx

      add   eax,FIX_ROUND
      adc   edx,0
      shrd  eax,edx,FIX_SHIFT
		
      IFNB  <axis>
      add   eax,(TMat3 PTR [esi]).t&axis&vec_elem&
      ENDIF

      mov   (Vector3 PTR [edi]).&vec_elem&,eax
      ENDM


vcr3mul MACRO  col,vec_elem
      vc3mul  col,vec_elem
      ENDM


vct3mul MACRO col,vec_elem
      vc3mul  col,vec_elem,3
      ENDM


