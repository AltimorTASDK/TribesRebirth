; ========================================================
;
;	$Workfile:   g_timer.asm  $
;	$Version$
;
;    Code for the timer - debug stuff
;
;  $Revision:$
;       $Log:$
;	
; (c) Copyright 1995, Dynamix Inc.  All rights reserved.
; ========================================================

.486
.MODEL FLAT, syscall
.DATA

IFDEF __MASM__
ARG equ
ENDIF

.CODE

PUBLIC C exec_cpuid 
exec_cpuid PROC C USES ebx ecx edx edi esi

   pushfd
   pop      eax
   mov      ebx, eax
   xor      eax, 200000h
   push     eax
   popfd
   pushfd
   pop      eax
   xor      eax, ebx
   je       @@no_cpuid

   mov      eax, 0
   
   db 0Fh, 0A2h               ; CPUID

   cmp      ebx, 0756E6547h   ; uneG = "GenuineIntel"
   jne      @@test_amd
   mov      eax, 0
   jmp      @@exit

@@test_amd:
   cmp      ebx, 068747541h   ; htuA = "AuthenticAMD"
   jne      @@test_cyrix
   mov      eax, 1
   jmp      @@exit

@@test_cyrix:
   cmp      ebx, 069727943h   ; iryC = "CyrixInstead"
   jne      @@no_cpuid
   mov      eax, 2
   jmp      @@exit

@@no_cpuid:
   mov      eax, 3

@@exit:
   ret

exec_cpuid ENDP

END
