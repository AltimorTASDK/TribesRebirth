; ---------------------------------------------------------------------------

;	$Workfile:   m_fix.asm  $
;	$Version$
;	$Revision:   1.3  $
;	$Date:   14 Dec 1995 19:01:16  $
;	$Log:   R:\darkstar\develop\core\ml\vcs\m_fix.asv  $
;	

; ---------------------------------------------------------------------------

include types.inc

.DATA
scaleAmount     dd      65536.0

AS_INTEGER_ONE       EQU   3f800000h
SCALE_UP             EQU   4b000000h
SCALE_DN             EQU   34000000h    ; 1.0/SCALE_UP
NEG_LOG2E            EQU   0bf317218h    ; -log2(e)

.CODE

; ---------------------------------------------------------------------------
; Converts a float value to a fixed-point value.
; C near-callable as:
;	void cdecl m_RealF_to_RealX( const RealF &, RealX * );

BeginFastCallProc       m_RealF_to_RealX, fv, iv
        fld     scaleAmount
        fld     dword ptr [fv]
        fmulp   st(1),st
        fistp   dword ptr [iv]
EndFastCallProc


; ---------------------------------------------------------------------------
; Converts a float value to an integer value.
; C near-callable as:
;	void cdecl m_RealF_to_Int32( const RealF &, Int32 * );

BeginFastCallProc       m_RealF_to_Int32, fv, iv
        fld     dword ptr [fv]
        fistp   dword ptr [iv]
EndFastCallProc



;------------------------------------------------------------------------------
;
m_sqrt_RealF PROC C  
        PUBLIC C m_sqrt_RealF
        fld     dword ptr [esp+4]       
        fsqrt   
        RET
m_sqrt_RealF ENDP


;------------------------------------------------------------------------------
;
; Uses algorithm from article by Jim Blinn titled 'Floating-Point Tricks'
; IEEE Computer Graphics and Applications July/August 1997
; p80-84
;
; Computes approximate SquareRoot
; 6.5% MAX error using this approximation
;
m_sqrtf_RealF PROC C  
        PUBLIC C m_sqrtf_RealF

        mov     eax, dword ptr [esp+4]
        shr     eax, 1
        add     eax, AS_INTEGER_ONE SHR 1 
        mov     dword ptr [esp+4], eax
        fld     dword ptr [esp+4]

        RET 

m_sqrtf_RealF ENDP

;------------------------------------------------------------------------------
;
; Uses algorithm from article by Jim Blinn titled 'Floating-Point Tricks'
; IEEE Computer Graphics and Applications July/August 1997
; p80-84
;
; Computes approximate 1/SquareRoot
; 6.5% MAX error using this approximation
;
m_invsqrtf_RealF PROC C  
        PUBLIC C m_invsqrtf_RealF

        mov     eax, (AS_INTEGER_ONE + (AS_INTEGER_ONE SHR 1))
        mov     ecx, dword PTR [esp+4]
        shr     ecx, 1
        sub     eax, ecx
        mov     dword ptr [esp+4], eax
        fld     dword ptr [esp+4]

        RET 

m_invsqrtf_RealF ENDP


;------------------------------------------------------------------------------
m_log2_RealF PROC C  
        PUBLIC C m_log2_RealF

        fld1
        fld     dword PTR [esp+4]
        fyl2x

        RET 

m_log2_RealF ENDP


;------------------------------------------------------------------------------
;
; Uses algorithm from article by Jim Blinn titled 'Floating-Point Tricks'
; IEEE Computer Graphics and Applications July/August 1997
; p80-84
;
m_log2f_RealF PROC C  
        PUBLIC C m_log2f_RealF

        mov     eax, dword PTR [esp+4]
        sub     eax, AS_INTEGER_ONE
        mov     dword PTR [esp+4], eax
        fild    dword PTR [esp+4]
        mov     dword PTR [esp+4], SCALE_DN
        fmul    dword PTR [esp+4]

        RET 

m_log2f_RealF ENDP


;------------------------------------------------------------------------------
;
; Uses algorithm from article by Jim Blinn titled 'Floating-Point Tricks'
; IEEE Computer Graphics and Applications July/August 1997
; p80-84
;
m_pow2f_RealF PROC C  
        PUBLIC C m_pow2f_RealF

        fld     dword PTR [esp+4]
        mov     dword PTR [esp+4], SCALE_UP
        fmul    dword PTR [esp+4]
        fistp   dword PTR [esp+4]
        add     dword PTR [esp+4], AS_INTEGER_ONE
        fld     dword PTR [esp+4]

        RET 

m_pow2f_RealF ENDP


;------------------------------------------------------------------------------
;
; Uses algorithm from article by Jim Blinn titled 'Floating-Point Tricks'
; IEEE Computer Graphics and Applications July/August 1997
; p80-84
;
;  w should be normalized 0-1
;  d is fog density 0-1
;
;  f = e^(-wd) = 2^w(-d x log2e)
;  result = 1-f         where reault is fog intensity 0 to 1
;
m_fog_RealF PROC C  
        PUBLIC C m_fog_RealF

        fld1                              ; 1
        fld     dword PTR [esp+8]         ; d, 1
        mov     dword PTR [esp+8], NEG_LOG2E
        fmul    dword PTR [esp+8]
        fmul    dword PTR [esp+4]         ; w(d x -log2(e) ), 1

        ; do 2^x approximation
        mov     dword PTR [esp+4], SCALE_UP
        fmul    dword PTR [esp+4]
        fistp   dword PTR [esp+4]
        add     dword PTR [esp+4], AS_INTEGER_ONE
        fld     dword PTR [esp+4]         ; f, 1
        fsubp   st(1), st                 ; 1-f

        RET 
m_fog_RealF ENDP


END

