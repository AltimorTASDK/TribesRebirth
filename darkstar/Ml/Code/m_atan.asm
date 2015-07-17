;
; $Workfile:   m_atan.asm  $
; $Revision:   1.1  $
; $Version: 1.6.2 $
; $Date:   08 Mar 1995 08:44:18  $
; $Log:   S:\phoenix\ml\vcs\m_atan.asv  $
;	
;	   Rev 1.1   08 Mar 1995 08:44:18   RICKO
;	Now compiles with both TASM and MASM
;	
;	   Rev 1.0   07 Mar 1995 13:16:40   NANCYH
;	Initial revision.
;  

.486p
.MODEL FLAT, SYSCALL
 
IFDEF __MASM__
ARG equ
ENDIF
 
 ;------------------------------ DATA SEGMENT
.DATA
 

;  This table holds angles for 512 different x:y ratios between 0 and 45
;  degrees.  The values in the table are stored in fractional ints 14.2
;  and must be added to their index (in 14.2) to arrive at the actual
;  angle in 14.2 format.  This gives you a resolution of 16,384 hex degrees
;  for a 360 degree circle.  Since our circles are 0-65k, this angle must
;  be multiplied by 4.

atantab label byte
        db        0,  1,  2,  3,  4,  5,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16
        db       17, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 32, 33, 34
        db       35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 48, 49, 50, 51
        db       52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67
        db       68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83
        db       84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 97, 98
        db       99,100,101,102,103,104,105,106,107,107,108,109,110,111,112,113
        db      114,114,115,116,117,118,119,119,120,121,122,123,124,124,125,126
        db      127,128,128,129,130,131,131,132,133,134,135,135,136,137,137,138
        db      139,140,140,141,142,142,143,144,145,145,146,147,147,148,149,149
        db      150,150,151,152,152,153,154,154,155,155,156,157,157,158,158,159
        db      159,160,160,161,162,162,163,163,164,164,165,165,166,166,167,167
        db      168,168,168,169,169,170,170,171,171,171,172,172,173,173,173,174
        db      174,175,175,175,176,176,176,177,177,177,178,178,178,179,179,179
        db      179,180,180,180,180,181,181,181,181,182,182,182,182,182,183,183
        db      183,183,183,184,184,184,184,184,184,184,184,185,185,185,185,185
        db      185,185,185,185,185,185,185,185,185,185,185,185,185,185,185,185
        db      185,185,185,185,185,185,185,185,185,185,185,185,185,184,184,184
        db      184,184,184,184,184,183,183,183,183,183,182,182,182,182,182,181
        db      181,181,181,180,180,180,180,179,179,179,179,178,178,178,177,177
        db      177,176,176,176,175,175,174,174,174,173,173,172,172,172,171,171
        db      170,170,169,169,169,168,168,167,167,166,166,165,165,164,164,163
        db      163,162,161,161,160,160,159,159,158,157,157,156,156,155,154,154
        db      153,152,152,151,150,150,149,148,148,147,146,146,145,144,143,143
        db      142,141,140,140,139,138,137,137,136,135,134,133,133,132,131,130
        db      129,129,128,127,126,125,124,123,122,122,121,120,119,118,117,116
        db      115,114,113,112,111,110,110,109,108,107,106,105,104,103,102,101
        db      100, 99, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84
        db       82, 81, 80, 79, 78, 77, 76, 74, 73, 72, 71, 70, 69, 67, 66, 65
        db       64, 63, 61, 60, 59, 58, 57, 55, 54, 53, 52, 50, 49, 48, 46, 45
        db       44, 43, 41, 40, 39, 37, 36, 35, 33, 32, 31, 29, 28, 27, 25, 24
        db       23, 21, 20, 18, 17, 16, 14, 13, 11, 10,  9,  7,  6,  4,  3,  1
        db        0

.CODE

m_atan_  PROC C \
        USES    ebx esi edi,
        ARG     deltax, deltay
        PUBLIC  C m_atan_

        mov     eax,deltax
        cdq                             ; get ecx=abs(deltax), esi=sign
        mov     esi,edx
        xor     eax,edx
        sub     eax,edx
        mov     ecx,eax

        mov     eax,deltay
        cdq                             ; get eax=abs(deltay), edi=sign
        mov     edi,edx
        xor     eax,edx
        sub     eax,edx

        jz      adjust                  ; since dy = 0, assume angle = 0
                                        ; adjusted for quadrant.
                                        ; this early out also avoids divide
                                        ; by zero.

got_signs:

        cmp     eax,ecx                 ; is angle 0-45 degrees?
        jg      dy_greater              ; angle is 45-90 degrees
dx_greater:
        cdq
        shld    edx,eax,9               ; edx:eax = delta_y *  512
        shl     eax,9

        mov     ebx,ecx                 ; account for rounding
        sar     ebx,1
        add     eax,ebx
        
        div     ecx
        cmp     eax,0
        je      adjust

        movzx   edx,atantab[eax]        ; get fractional angle difference
        shl     eax,2                   ; convert index to 14.2 fractional int
        add     eax,edx                 ; add difference from table

        jmp     short adjust

dy_greater:
        xchg    eax,ecx
        cdq
        shld    edx,eax,9               ; edx:eax = delta_x *  512
        shl     eax,9
        
        mov     ebx,ecx                 ; account for rounding
        sar     ebx,1
        add     eax,ebx

        div     ecx
        cmp     eax,0
        je      dy_gskip
        
        movzx   edx,atantab[eax]        ; get fractional angle difference
        shl     eax,2                   ; convert index to 14.2 fractional int
        add     eax,edx
dy_gskip:
        sub     eax,4096
        neg     eax                     ; eax = 4096 - table value

adjust:
        cmp     esi,0                   ; dx < 0?
        jge     dx_pos                  ; no
        sub     eax,8192                ; reflect about the y-axis
        neg     eax                     ; (adjustment for -dx)
dx_pos:
        cmp     edi,0                   ; dy < 0?
        jge     at_ret                  ; no
        sub     eax,16384               ; reflect about the x-axis
        neg     eax                     ; (adjustment for -dy)

at_ret:
        shl     eax,2
        and     eax,0ffffh              ; angle must be between 0 & 65535
        RET

m_atan_ ENDP

END

