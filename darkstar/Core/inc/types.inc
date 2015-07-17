;
; $Workfile:   types.inc  $
; $Revision:   1.1  $
; $Version: 1.6.2 $
; $Date:   14 Dec 1995 18:57:18  $
; $Log:   R:\darkstar\develop\core\base\vcs\types.inv  $
;	
;	   Rev 1.1   14 Dec 1995 18:57:18   CAEDMONI
;	?
;	
;	   Rev 1.0   11 Dec 1995 17:11:50   CAEDMONI
;	Initial revision.
;	

.486p
.MODEL FLAT, SYSCALL
 
IFDEF __MASM__
ARG equ
DLL_EXPORT equ PUBLIC
ELSE
DLL_EXPORT equ PUBLICDLL
EXTERNDEF       equ     GLOBAL
ENDIF


;----------------------------------------------------------------------------
; usefull floating point constants:
;

FLOAT_ONE       equ     1065353216

.DATA

EXTERNDEF  _RealF_One    : dword
EXTERNDEF  _RealF_Half   : dword
EXTERNDEF  _RealF_Zero   : dword
EXTERNDEF  _RealF_Pi     : dword
EXTERNDEF  _RealF_2Pi    : dword

;----------------------------------------------------------------------------
; usefull fixed point constants:
;

FIX_SHIFT	EQU	16
FIX_ROUND	EQU	08000h
FIX_ONE	EQU	(1 SHL FIX_SHIFT)


;----------------------------------------------------------------------------
; Color structs:

Color STRUC
	red	DD ?
	green	DD ?
	blue	DD ?
Color ENDS

;----------------------------------------------------------------------------
;
; Register Argument macros - for __fastcall calling convention
;
; NOTE: A maximum of three arguments are supported.  Each must passed in
; a DWORD.  For other conditions, don't use these macros.
; Args are put in eax,edx,ecx

;----------------------------------------------------------------------------

RArg1   equ     eax
RArg2   equ     edx
RArg3   equ     ecx

;----------------------------------------------------------------------------
; Symantec doesn't support fastcall - we'll just read the args off the stack
;
ifdef SYMANTEC

BeginFastCallProc       MACRO   name:REQ, arg1:REQ, arg2, arg3
        RArgProcName    textequ     <_&name&>
        % &RArgProcName PROC
        % PUBLIC &RArgProcName
        IFNB    <arg1>
                &arg1   equ     RArg1
                mov     eax, [esp+4]
                saveecx  =      0
        ENDIF
        IFNB    <arg2>
                &arg2   equ     RArg2
                mov     edx, [esp+8]
                saveecx  =      0
        ENDIF
        IFNB    <arg3>
                &arg3   equ     RArg3
                saveecx  =      1
                push    ecx
                mov     ecx, [esp+16]
        ENDIF
ENDM

EndFastCallProc MACRO
IF saveecx EQ 1
        pop     ecx
ENDIF
% RET 0
% &RArgProcName ENDP
RArgProcName    textequ <>
ENDM

endif

;----------------------------------------------------------------------------
; Watcom uses eax,edx,ebx
; Mangles name by appending '-'
;
ifdef WATCOM

BeginFastCallProc       MACRO   name:REQ, arg1:REQ, arg2, arg3
        RArgProcName    textequ     <&name&_>
        % &RArgProcName PROC
        % PUBLIC &RArgProcName
        IFNB    <arg1>
                &arg1   equ     RArg1
                saveecx  =      0
        ENDIF
        IFNB    <arg2>
                &arg2   equ     RArg2
                saveecx  =      0
        ENDIF
        IFNB    <arg3>
                &arg3   equ     RArg3
                saveecx  =      1
                push    ecx
                mov     ecx,ebx
        ENDIF
ENDM

EndFastCallProc MACRO
IF saveecx EQ 1
        pop     ecx
ENDIF
% RET 0
% &RArgProcName ENDP
RArgProcName    textequ <>
ENDM

endif

;----------------------------------------------------------------------------
; Borland uses eax,edx,ecx
; Mangles name by prepending '@'
;
ifdef BORLAND

BeginFastCallProc       MACRO   name:REQ, arg1:REQ, arg2, arg3
        RArgProcName    equ     <@&name&>
        % &RArgProcName PROC
        % PUBLIC &RArgProcName
        IFNB    <arg1>
                &arg1   equ     RArg1
        ENDIF
        IFNB    <arg2>
                &arg2   equ     RArg2
        ENDIF
        IFNB    <arg3>
                &arg3   equ     RArg3
        ENDIF
ENDM

EndFastCallProc MACRO
% RET 0
% &RArgProcName ENDP
PURGE   RArgProcName
ENDM

endif

;----------------------------------------------------------------------------
; MSVC uses ecx,edx,[esp+4]
; mangles name by prepending '@' and appending '@' + size of args in bytes
;
ifdef MSVC

BeginFastCallProc       MACRO   name:REQ, arg1:REQ, arg2, arg3
        IFNB    <arg3>
                .ERRB   <arg2>
                ssize    equ     <12>
                retsize  equ     <4>
        ELSE
                IFNB    <arg2>
                ssize    equ     <8>
                retsize  equ     <0>
                ELSE
                ssize    equ     <4>
                retsize  equ     <0>
                ENDIF
        ENDIF
        RArgProcName    textequ     <@&name&@&ssize&>
        % &RArgProcName PROC
        % PUBLIC &RArgProcName
        mov     eax,ecx
        IFNB    <arg3>
                mov     ecx,[esp+4]
        ENDIF
        IFNB    <arg1>
                &arg1   equ     RArg1
        ENDIF
        IFNB    <arg2>
                &arg2   equ     RArg2
        ENDIF
        IFNB    <arg3>
                &arg3   equ     RArg3
        ENDIF
ENDM

EndFastCallProc MACRO
% RET &retsize
% &RArgProcName ENDP
RArgProcName    textequ <>
ENDM

endif
;
;
;----------------------------------------------------------------------------

