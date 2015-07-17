;=========================================================================
;==   
;== $Workfile:   c_raster.asm  $
;== $Version$
;== $Revision:   1.0  $
;==   
;== DESCRIPTION:  Rasterization routines for circles.  Very complicated.
;==               Suggest looking at the direct memory buffer implementation
;==               to familiarize yourself with the algorithm.  It is much
;==               less complicated.
;==   
;== (c) Copyright 1995, Dynamix Inc.   All rights reserved.
;==   
;=========================================================================

.486
.MODEL FLAT, syscall

IFDEF __MASM__
ARG equ
ELSE
LOCALS
ENDIF

INCLUDE g_raster.inc

Rect STRUC
        left    DD ?
        top     DD ?
        right   DD ?
        bottom  DD ?
Rect ENDS

.CODE

;北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
;       
;       GFXasmCircleFill( rasterList, rect );
;       
;       by Louie McCrady
;       
;       Copyright (c) 1994, All rights reserved
;       
;       This circle function is pretty fast because it performs no
;       multiplies in the inner loops, only a few adds, subs, and shls
;       all of the other constant factors used in the loops have been
;       pre calculated before entering the loops.  
;       
;       This function uses 4way simmetry to calculate 0-90 and refelect
;       the values around the other quadrants.  For perfect circles, a
;       faster algorithm could use 8-way simmetry but would not work
;       with ellipses.  Because of the 4-way simmetry, the raster list 
;       is filled from the top and the bottom towards the middle.
;       
;北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
;
;       rastList - pointer to raster list
;       rect     - pointer to bounding rectangle describing circle
;
;北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

GFXasmCircleFill  PROC C      \
        USES    eax ebx ecx edx esi edi,
        ARG     rastList, rect
        LOCAL   row, col, LX, diameter, diamOdd,        \
                lastRast, RAST_SIZE,                    \
                BresA, BresB, BresD,                    \
                r_sqrd, r2_sqrd, r4_sqrd, r6_sqrd,      \
                a_sqrd, a2_sqrd, a4_sqrd, a6_sqrd
        PUBLIC  C GFXasmCircleFill

        mov     RAST_SIZE,RASTER_SIZE   ;get immediate into local variable

        mov     esi,rect                ;get pointer to bounding rect

        mov     ebx,(Rect PTR [esi]).right         ;get width of circle
        sub     ebx,(Rect PTR [esi]).left
        inc     ebx
        mov     eax,ebx

        shr     eax,1                   ;calculate center column for circle
        adc     eax,(Rect PTR [esi]).left
        mov     LX,eax
        
        mov     eax,(Rect PTR [esi]).bottom        ;get diameter of circle
        sub     eax,(Rect PTR [esi]).top
        inc     eax
        mov     diameter,eax

        shl     eax,16                  ;get circle aspect in 16.16 (diam << 16) / w
        cdq
        div     ebx                     ;aspect 16.16 in eax
        mul     eax                     ;get aspect squared
        shrd    eax,edx,16              ;shift 64-bit value 16 bits
        mov     ecx,eax                 ;save aspect squared as 16.16

        mov     dword ptr col,0         ;seed column position
        mov     dword ptr diamOdd,0     ;seed diameter odd flag

        mov     eax,diameter
        shr     eax,1                   ;div by two to get radius
        rcl     diamOdd,1               ;if carry diameter is odd sized
        mov     row,eax                 ;seed row position
        mul     eax
        mov     r_sqrd,eax              ;save r_sqrd
        shl     eax,16                  ;make fractional int
        div     ecx                     ;a_sqrd is actually r_sqrd/aspect_sqared
        mov     a_sqrd,eax              ;save a_sqrd
        shl     eax,1
        mov     a2_sqrd,eax             ;save a_sqrd * 2
        shl     eax,1
        mov     a4_sqrd,eax             ;save a_sqrd * 4
        add     eax,a2_sqrd
        mov     a6_sqrd,eax             ;save a_sqrd * 6

        mov     eax,r_sqrd
        shl     eax,1
        mov     r2_sqrd,eax             ;save r_sqrd * 2
        shl     eax,1
        mov     r4_sqrd,eax             ;save r_sqrd * 4
        add     eax,r2_sqrd
        mov     r6_sqrd,eax             ;save r_sqrd * 6

;************************** Circle Part I ********************************
        ;
        ; this is for the octant 90 to 45 degrees
        ; calculate Bresenham's decision variable ------------------------
        ; BresD = a2_sqrd*((row-1)*row) + a_sqrd + r2_sqrd*(1-a_sqrd);

        mov     eax,1
        sub     eax,a_sqrd
        mul     r2_sqrd
        add     eax,a_sqrd
        mov     ecx,eax

        mov     eax,row
        dec     eax
        mul     row
        mul     a2_sqrd
        add     eax,ecx

        mov     BresD,eax               ;save Bresenham's decision variable

        mov     eax,diameter            ;get circle diameter
        dec     eax                     ;get logical number of rows
        mul     RAST_SIZE               ;calculate row address of bottom raster
        mov     esi,eax                 ;store in esi

        mov     edi,rastList            ;seed row address of top raster
        add     esi,edi                 ;add row addres of bottom raster
        mov     lastRast,esi            ;save offset to last raster for cleanup

        mov     eax,row                 ;get current row
        mul     a_sqrd
        mov     edx,eax                 ;save in Bresenham's A variable

        mov     ebx,0                   ;seed Bresenham's B variable
        mov     ecx,-1                  ;seed scan distance to 1 pixel
        add     ecx,diamOdd             ;adjust if odd width, (non-symmetric)

;--------------------- Circle Loop for first octant ---------------------
;       
;       ebx = Bresenham's B accumulator
;       ecx = distance between points on same line
;       edx = Bresenham's A accumulator variable for column position
;       esi = offset to bottom of circle raster line
;       edi = offset to top of circle raster line
;
        cmp     edx,ebx                 ;(row*a_sqrd) <= (col*r_sqrd)?
        jle     circf_loop1_done        ;no, finished with top loop

circf_part1_loop:
        cmp     dword ptr BresD,0       ;(BresD>=0)
        jl      circf_noBjmp1           ;no, stay on this row and move to next column
        
        mov     eax,LX                          ;yes, store information for this row
        mov     (RASTER PTR [edi]).x,eax        ;store left X for top half of circle
        mov     (RASTER PTR [esi]).x,eax        ;store left X for bottom half of circle
        mov     (RASTER PTR [edi]).cnt,ecx      ;store scan length
        mov     (RASTER PTR [esi]).cnt,ecx

        mov     (RASTER PTR [edi]).f, RASTER_STEP_Y     ;indicate to move to next line
        mov     (RASTER PTR [esi]).f, RASTER_STEP_Y     ;indicate to move to next line

        mov     eax,edi
        add     edi,RASTER_SIZE         ;get pointer to next raster line
        mov     (RASTER PTR [eax]).next, edi    ; store it in this raster line's next ptr

        mov     eax,esi
        sub     esi,RASTER_SIZE         ;move to previous row in bottom part of circle
        mov     (RASTER PTR [esi]).next, eax    ; store old raster line's next ptr

        dec     row                     ;row--
        sub     edx,a_sqrd              ;adjust Bresenham's A variable
        mov     eax,edx
        shl     eax,2                   ;mul by 4
        sub     BresD,eax               ;BresD -= 4*a_sqrd*row

circf_noBjmp1:
        dec     LX                      ;move to previous left column
        add     ecx,2                   ;increment distance between left & right points
        inc     col                     ;col++

        mov     eax,r6_sqrd             ;6*r_sqrd
        lea     eax,[eax+ebx*4]         ;4*ebx + 6*r_sqrd
        add     BresD,eax               ;BresD += 4*ebx + 6*r_sqrd
        add     ebx,r_sqrd              ;adjust Bresenham's B variable, ebx = col*r_sqrd

        cmp     edx,ebx                 ;(row*a_sqrd) <= (col*r_sqrd)?
        jg      circf_part1_loop        ;no, finished with top loop

circf_loop1_done:
;************************** Circle Part II *******************************
        ;
        ; this is for the octant 45 to 0 degrees
        ; calculate decision variable ------------------------------------
        ; BresD = r2_sqrd*(col+1)*col + a2_sqrd*(row*(row-2)+1) + (1-a2_sqrd)*r_sqrd

        mov     eax,col
        inc     eax
        mul     col
        mul     r2_sqrd
        mov     BresD,eax

        mov     eax,row
        sub     eax,2
        mul     row
        inc     eax
        mul     a2_sqrd
        add     BresD,eax

        mov     eax,1
        sub     eax,a2_sqrd
        mul     r_sqrd
        add     BresD,eax               ;save Bresenham's decision variable

        mov     eax,row
        mul     a4_sqrd
        mov     ebx,eax

        mov     eax,col
        mul     r4_sqrd
        mov     BresA,eax               ;save Bresenham's A variable

        mov     edx,BresD               ;put BresD in edx

;--------------------- Circle Loop for second octant ---------------------
;
;       eax = scratch
;       ebx = Bresenham's B accumulator
;       ecx = distance between points on same line
;       edx = Bresenham's D accumulator variable for column position
;       esi = offset to bottom of circle
;       edi = offset to top of circle
;

circf_part2_loop:
        mov     eax,LX                          ;get left X
        mov     (RASTER PTR [edi]).x,eax        ;store left X for top half of circle
        mov     (RASTER PTR [esi]).x,eax        ;store left X for bottom half of circle
        mov     (RASTER PTR [edi]).cnt,ecx      ;store scan length
        mov     (RASTER PTR [esi]).cnt,ecx
        mov     (RASTER PTR [edi]).f, RASTER_STEP_Y     ;indicate to move to next line
        mov     (RASTER PTR [esi]).f, RASTER_STEP_Y     ;indicate to move to next line

        lea     eax,[edi+RASTER_SIZE]   ;get pointer to next raster line
        mov     (RASTER PTR [edi]).next, eax    ; store it in this raster line's next ptr
        add     edi,RASTER_SIZE

        lea     eax,[esi+RASTER_SIZE]   ;get pointer to next raster line 
        mov     (RASTER PTR [esi]).next, eax    ; store old raster line's next ptr
        sub     esi,RASTER_SIZE

        cmp     edx,0                   ;(edx<=0)
        jg      circf_noBjmp2           ;handle Bresenham's jump to next line

        inc     col
        add     ecx,2                   ;inc column distance between points
        dec     LX                      ;move to previous left column

        mov     eax,BresA
        add     eax,r4_sqrd
        mov     BresA,eax
        add     edx,eax                 ;adjust Bresenham's D variable

circf_noBjmp2:
        add     edx,a6_sqrd
        sub     edx,ebx                 ;edx += a6_sqrd - ebx
        sub     ebx,a4_sqrd             ;adjust Bresenham's B variable, ebx = a4_sqrd * row

        dec     row
        jns     short circf_part2_loop  ;continue, else exit

        mov     edi,rastList            ;get pointer to first raster line
        mov     (RASTER PTR [edi]).f, 0 ;first raster line does not start with y skip
        mov     edi,lastRast            ;get offset to last raster line
        mov     (RASTER PTR [edi]).next,0  ;clear last raster line's next pointer

        ret

GFXasmCircleFill  ENDP


;北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
;       
;       GFXasmCircle( rastList, centerX, radius, aspect )
;       
;       by Louie McCrady
;       
;       Copyright (c) 1994, All rights reserved
;       
;       This circle function is pretty fast because it performs no
;       multiplies in the inner loops, only a few adds, subs, and shls
;       all of the other constant factors used in the loops have been
;       pre calculated before entering the loops.  
;       
;       This function uses 4way simmetry to calculate 0-90 and refelect
;       the values around the other quadrants.  For perfect circles, a
;       faster algorithm could use 8-way simmetry but would not work
;       with ellipses.  
;
;       Because of the 4-way simmetry, the raster list is filled from 
;       the top and the bottom towards the middle.  Because there is a
;       left and right side of the circle, the left side of the circle
;       is stored in the top half of the raster list, and the right side
;       of the circle is stored in the bottom half.
;       
;       To implement this symmetry, in the raster list, with registers,
;       edi points to the top half of the circle, esi points to the 
;       bottom half.  edx is used as an offset to the right side of 
;       the circle.  Therefore you get combinations like [edi], [esi],
;       [edi+edx], and [esi+edx], to access the 4 symmetric points on 
;       the circle.
;       
;北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
;
;       rastList - pointer to raster list
;       Xc       - circles center X coordinate
;       radius   - distance from top of circle to center
;       aspect   - 16.16 ratio of h/w
;
;北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

GFXasmCircle  PROC C      \
        USES    eax ebx ecx edx esi edi,
        ARG     rastList, rect
        LOCAL   row, col, Xc, diameter, diamOdd,        \
                lastRast, RAST_SIZE, rastRghtOff,       \
                BresA, BresB, BresD,                    \
                r_sqrd, r2_sqrd, r4_sqrd, r6_sqrd,      \
                a_sqrd, a2_sqrd, a4_sqrd, a6_sqrd
        PUBLIC  C GFXasmCircle

        mov     RAST_SIZE,RASTER_SIZE   ;get immediate into local variable

        mov     esi,rect                ;get pointer to bounding rect

        mov     ebx,(Rect PTR [esi]).right         ;get width of circle
        sub     ebx,(Rect PTR [esi]).left
        inc     ebx
        mov     eax,ebx

        shr     eax,1                   ;calculate center column for circle
        adc     eax,(Rect PTR [esi]).left
        mov     Xc,eax
        
        mov     eax,(Rect PTR [esi]).bottom        ;get diameter of circle
        sub     eax,(Rect PTR [esi]).top
        inc     eax
        mov     diameter,eax

        shl     eax,16                  ;get circle aspect in 16.16 (diam << 16) / w
        cdq
        div     ebx                     ;aspect 16.16 in eax
        mul     eax                     ;get aspect squared
        shrd    eax,edx,16              ;shift 64-bit value 16 bits
        mov     ecx,eax                 ;save aspect squared as 16.16

        mov     dword ptr col,0         ;seed column position
        mov     dword ptr diamOdd,0     ;seed diameter odd flag

        mov     eax,diameter            ;get diameter
        shr     eax,1                   ;div by two to get radius
        rcl     diamOdd,1               ;if carry, diameter is odd sized
        mov     row,eax                 ;seed row position
        mul     eax
        mov     r_sqrd,eax              ;save r_sqrd
        shl     eax,16                  ;make fractional int
        div     ecx                     ;a_sqrd is actually r_sqrd/aspect_sqared
        mov     a_sqrd,eax              ;save a_sqrd
        shl     eax,1
        mov     a2_sqrd,eax             ;save a_sqrd * 2
        shl     eax,1
        mov     a4_sqrd,eax             ;save a_sqrd * 4
        add     eax,a2_sqrd
        mov     a6_sqrd,eax             ;save a_sqrd * 6

        mov     eax,r_sqrd
        shl     eax,1
        mov     r2_sqrd,eax             ;save r_sqrd * 2
        shl     eax,1
        mov     r4_sqrd,eax             ;save r_sqrd * 4
        add     eax,r2_sqrd
        mov     r6_sqrd,eax             ;save r_sqrd * 6

;************************** Circle Part I ********************************
        ;
        ; this is for the octant 90 to 45 degrees
        ; calculate Bresenham's decision variable ------------------------
        ; BresD = a2_sqrd*((row-1)*row) + a_sqrd + r2_sqrd*(1-a_sqrd);

        mov     eax,1
        sub     eax,a_sqrd
        mul     r2_sqrd
        add     eax,a_sqrd
        mov     ecx,eax

        mov     eax,row
        dec     eax
        mul     row
        mul     a2_sqrd
        add     eax,ecx

        mov     BresD,eax               ;save Bresenham's decision variable

        mov     eax,row                 ;get current row
        mul     a_sqrd
        mov     BresA,eax               ;save in Bresenham's A variable

        mov     eax,diameter            ;get circle diameter
        dec     eax                     ;get logical number of rows
        mul     RAST_SIZE               ;calculate row address of bottom raster
        mov     esi,eax                 ;store in esi
        add     eax,RAST_SIZE           ;get offset to right side raster list
        mov     rastRghtOff,eax         ;save offst to start of right side raster list
        mov     edx,eax                 ;seed offset to right side raster

        mov     edi,rastList            ;seed row address of top raster
        add     esi,edi                 ;add row addres of bottom raster
        mov     lastRast,esi            ;get bottom of left side
        add     lastRast,edx            ;add offset to bottom of right side, (end of raster list)

        mov     ebx,0                   ;seed Bresenham's B variable
        mov     ecx,-1                  ;starting span length
        add     ecx,diamOdd             ;adjust if odd width, (non-symmetric)

;--------------------- Circle Loop for first octant ---------------------
;       
;       eax = scratch
;       ebx = Bresenham's B accumulator
;       ecx = span length
;       edx = offset to left side scans in raster list, stored in bottom half of raster list
;       edi = offset to top of circle raster line
;       esi = offset to bottom of circle raster line
;
;       Xc  = center column
;       row = keeps track of row from center
;       col = keeps track of col from center
;       BresA = keeps track of Bresenham's A variable
;       BresD = keeps track of Bresenham's Decision variable
;
        ; store starting values on new raster line
        mov     eax,Xc                          ;get center of circle X
        mov     (RASTER PTR [edi+edx]).x,eax    ;store right X for top half of circle
        mov     (RASTER PTR [esi+edx]).x,eax    ;store right X for bottom half of circle
        mov     (RASTER PTR [esi]).f,RASTER_STEP_Y      ;indicate to move to next line
        mov     (RASTER PTR [edi]).f,0          ;first RASTER_STEP_Y is implied
        mov     (RASTER PTR [edi+edx]).f,0      ;all right flags do not have STEP_Y
        mov     (RASTER PTR [esi+edx]).f,0      ;all right flags do not have STEP_Y

        cmp     BresA,ebx               ;(row*a_sqrd) <= (col*r_sqrd)?
        jle     circ_loop1_done         ;no, finished with top loop

circ_part1_loop:
        cmp     dword ptr BresD,0               ;(BresD>=0)
        jl      circ_noBjmp1                    ;yes, move to next column

        ; store finishing values to left&right, top&bottom raster lines (4-way symmetry)
        mov     eax,Xc                          ;get center x
        sub     eax,col                         ;subtract to get left x
        mov     (RASTER PTR [edi]).x,eax        ;store left X for top half of circle
        mov     (RASTER PTR [esi]).x,eax        ;store left X for bottom half of circle
        mov     (RASTER PTR [edi]).cnt,ecx      ;store scan length for left side
        mov     (RASTER PTR [esi]).cnt,ecx      ;store scan length for left side
        mov     (RASTER PTR [edi+edx]).cnt,ecx  ;store scan length for right side
        mov     (RASTER PTR [esi+edx]).cnt,ecx  ;store scan length for right side

        ; store next pointers to left&right, top&bottom raster lines (4-way symmetry)
        lea     eax,[edi+edx]                   ;get offset to right raster line
        mov     (RASTER PTR [edi]).next,eax     ;store it in left's raster next ptr
        lea     eax,[edi+RASTER_SIZE]           ;get offset to next left raster line
        mov     (RASTER PTR [edi+edx]).next,eax ;store it in right's raster next ptr
        mov     edi,eax                         ;store in raster offset

        lea     eax,[esi+edx]                   ;get offset to right raster line
        mov     (RASTER PTR [esi]).next,eax     ;store it in left's raster next ptr
        lea     eax,[esi+RASTER_SIZE]           ;get offset to next left raster line
        mov     (RASTER PTR [esi+edx]).next,eax ;store it in right's raster next ptr
        sub     esi,RASTER_SIZE                 ;get offset to previous raster line

        ; store starting values on new raster lines
        mov     eax,Xc                          ;get center X
        add     eax,col                         ;add col to get current right X
        add     eax,diamOdd                     ;adjust if diameter is odd 
        mov     (RASTER PTR [edi+edx]).x,eax    ;store right X for top half of circle
        mov     (RASTER PTR [esi+edx]).x,eax    ;store right X for bottom half of circle
        mov     (RASTER PTR [edi+edx]).f,0      ;all right flags do not have STEP_Y
        mov     (RASTER PTR [esi+edx]).f,0      ;all right flags do not have STEP_Y
        mov     (RASTER PTR [edi]).f,RASTER_STEP_Y      ;all left flags have STEP_Y
        mov     (RASTER PTR [esi]).f,RASTER_STEP_Y      ;all left flags have STEP_Y

        ; adjust Bresenham's A&D variables
        dec     row                     ;row--
        mov     eax,BresA               ;get Bresenham's A variable
        sub     eax,a_sqrd              ;adjust Bresenham's A variable
        mov     BresA,eax               ;store it back, (this is the only write to this var)
        shl     eax,2                   ;mul by 4
        sub     BresD,eax               ;BresD -= 4*a_sqrd*row

        mov     ecx,-1                  ;initialize span length

circ_noBjmp1:
        inc     ecx                     ;increment column count
        inc     col                     ;col++
        ; adjust Bresenham's B&D variables
        mov     eax,r6_sqrd             ;6*r_sqrd
        lea     eax,[eax+ebx*4]         ;4*ebx + 6*r_sqrd
        add     BresD,eax               ;BresD += 4*ebx + 6*r_sqrd
        add     ebx,r_sqrd              ;adjust Bresenham's B variable, ebx = col*r_sqrd

        cmp     BresA,ebx               ;(row*a_sqrd) <= (col*r_sqrd)?
        jg      circ_part1_loop         ;plot next points?

circ_loop1_done:
;************************** Circle Part II *******************************
        ;
        ; this is for the octant 45 to 0 degrees
        ; calculate decision variable ------------------------------------
        ; BresD = r2_sqrd*(col+1)*col + a2_sqrd*(row*(row-2)+1) + (1-a2_sqrd)*r_sqrd

        mov     eax,col
        inc     eax
        mul     col
        mul     r2_sqrd
        mov     BresD,eax

        mov     eax,row
        sub     eax,2
        mul     row
        inc     eax
        mul     a2_sqrd
        add     BresD,eax

        mov     eax,1
        sub     eax,a2_sqrd
        mul     r_sqrd
        add     BresD,eax               ;save Bresenham's decision variable

        mov     eax,row
        mul     a4_sqrd
        mov     ebx,eax

        mov     eax,col
        mul     r4_sqrd
        mov     BresA,eax               ;save Bresenham's A variable

        mov     edx,rastRghtOff         ;offset to right scans in raster list

        mov     eax,diamOdd
        add     row,eax

;--------------------- Circle Loop for second octant ---------------------
;
;       eax = scratch
;       ebx = Bresenham's B accumulator
;       ecx = count from previous line
;       edx = offset to left side scans in raster list, stored in bottom half of raster list
;       edi = offset to top of circle raster line
;       esi = offset to bottom of circle raster line
;
;       Xc  = center column
;       row = keeps track of row from center
;       col = keeps track of col from center
;       BresA = keeps track of Bresenham's A variable
;       BresD = keeps track of Bresenham's Decision variable
;
circ_part2_loop:
        ; store finishing values to left&right, top&bottom raster lines (4-way symmetry)
        mov     eax,Xc                          ;get center X
        sub     eax,col                         ;subtract to get left x
        mov     (RASTER PTR [edi]).x,eax        ;store left X for top half of circle
        mov     (RASTER PTR [esi]).x,eax        ;store left X for bottom half of circle
        mov     (RASTER PTR [edi]).cnt,ecx      ;store scan length for left side
        mov     (RASTER PTR [esi]).cnt,ecx      ;store scan length for left side
        mov     (RASTER PTR [edi+edx]).cnt,ecx  ;store scan length for right side
        mov     (RASTER PTR [esi+edx]).cnt,ecx  ;store scan length for right side
        mov     ecx,0                           ;column widths are 0 from now on

        ; store next pointers to left&right, top&bottom raster lines (4-way symmetry)
        lea     eax,[edi+edx]                   ;get offset to right raster line
        mov     (RASTER PTR [edi]).next,eax     ;store it in left's raster next ptr
        lea     eax,[edi+RASTER_SIZE]           ;get offset to next left raster line
        mov     (RASTER PTR [edi+edx]).next,eax ;store it in right's raster next ptr
        mov     edi,eax                         ;store in raster offset

        lea     eax,[esi+edx]                   ;get offset to right raster line
        mov     (RASTER PTR [esi]).next,eax     ;store it in left's raster next ptr
        lea     eax,[esi+RASTER_SIZE]           ;get offset to next left raster line
        mov     (RASTER PTR [esi+edx]).next,eax ;store it in right's raster next ptr
        sub     esi,RASTER_SIZE                 ;get offset to previous raster line

        dec     row                             ;reduce row count
        jle     circ_done                       ;finished, exit

        ; adjust Bresenham's A&D variables
        cmp     BresD,0                 ;(edx<=0)
        jg      circ_noBjmp2            ;handle Bresenham's jump to next line

        inc     col

        mov     eax,BresA
        add     eax,r4_sqrd
        mov     BresA,eax
        add     BresD,eax               ;adjust Bresenham's D variable

circ_noBjmp2:
        ; store starting values on new raster lines
        mov     eax,Xc                          ;get center X
        add     eax,col                         ;add col to get current right X
        dec     eax
        add     eax,diamOdd                     ;adjust if diameter is odd
        mov     (RASTER PTR [edi+edx]).x,eax    ;store right X for top half of circle
        mov     (RASTER PTR [esi+edx]).x,eax    ;store right X for bottom half of circle
        mov     (RASTER PTR [edi+edx]).f,0      ;all right flags do not have STEP_Y
        mov     (RASTER PTR [esi+edx]).f,0      ;all right flags do not have STEP_Y
        mov     (RASTER PTR [edi]).f,RASTER_STEP_Y      ;all left flags have STEP_Y
        mov     (RASTER PTR [esi]).f,RASTER_STEP_Y      ;all left flags have STEP_Y

        ; adjust Bresenham's B&D variables
        mov     eax,BresD               ;adjust Bresenham's B variable, ebx = a4_sqrd * row
        add     eax,a6_sqrd
        sub     eax,ebx                 ;edx += a6_sqrd - ebx
        mov     BresD,eax
        sub     ebx,a4_sqrd             ;B = a4_sqrd*row

        jmp     circ_part2_loop         ;continue, else exit

circ_done:
        ; finished, now store special ending values in last raster list entries

        mov     esi,lastRast                    ;get offset to last raster list line
        mov     (RASTER PTR [esi]).next,0       ;set last raster's next ptr

        ret

GFXasmCircle  ENDP
        
        







;北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
;       
;       GFXasmCircleClip( rasterList, rect );
;       
;       This function clips the raster list to the circular region
;       defined by the rectangle.  It can be used if you wish to clip
;       circular regions from a raster list.
;       
; Several assumptions have been made about the condition of the
; raster list.  
; 
; The most important assumption is that the raster list is a 
; solid object and contains no split scan lines.  This allows
; the routine to take advantage of the circles top-bottom symmetry.
; To change this would require a major rewrite.
; 
; The routine also assumes that the rectangle describing the clip
; region does not go off the bottom of the raster list. The bottom
; of the clip rect should not point to a row that is off the bottom
; of the raster list or else you'll be clipping into memory.
;       
;北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
;
;       rastList - pointer to raster list
;       rect     - pointer to bounding rectangle describing circle
;
;北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

GFXasmCircleClip  PROC C      \
        USES    eax ebx ecx edx esi edi,
        ARG     rastList, rect
        LOCAL   row, col, LX, diameter, diamOdd,        \
                lastRast, RAST_SIZE,                    \
                BresA, BresB, BresD,                    \
                r_sqrd, r2_sqrd, r4_sqrd, r6_sqrd,      \
                a_sqrd, a2_sqrd, a4_sqrd, a6_sqrd
        PUBLIC  C GFXasmCircleClip

        mov     RAST_SIZE,RASTER_SIZE   ;get immediate into local variable

        mov     esi,rect                ;get pointer to bounding rect

        mov     ebx,(Rect PTR [esi]).right         ;get width of circle
        sub     ebx,(Rect PTR [esi]).left
        inc     ebx
        mov     eax,ebx

        shr     eax,1                   ;calculate center column for circle
        adc     eax,(Rect PTR [esi]).left
        mov     LX,eax
        
        mov     eax,(Rect PTR [esi]).bottom        ;get diameter of circle
        sub     eax,(Rect PTR [esi]).top
        inc     eax
        mov     diameter,eax

        shl     eax,16                  ;get circle aspect in 16.16 (diam << 16) / w
        cdq
        div     ebx                     ;aspect 16.16 in eax
        mul     eax                     ;get aspect squared
        shrd    eax,edx,16              ;shift 64-bit value 16 bits
        mov     ecx,eax                 ;save aspect squared as 16.16

        mov     dword ptr col,0         ;seed column position
        mov     dword ptr diamOdd,0     ;seed diameter odd flag

        mov     eax,diameter
        shr     eax,1                   ;div by two to get radius
        rcl     diamOdd,1               ;if carry diameter is odd sized
        mov     row,eax                 ;seed row position
        mul     eax
        mov     r_sqrd,eax              ;save r_sqrd
        shl     eax,16                  ;make fractional int
        div     ecx                     ;a_sqrd is actually r_sqrd/aspect_sqared
        mov     a_sqrd,eax              ;save a_sqrd
        shl     eax,1
        mov     a2_sqrd,eax             ;save a_sqrd * 2
        shl     eax,1
        mov     a4_sqrd,eax             ;save a_sqrd * 4
        add     eax,a2_sqrd
        mov     a6_sqrd,eax             ;save a_sqrd * 6

        mov     eax,r_sqrd
        shl     eax,1
        mov     r2_sqrd,eax             ;save r_sqrd * 2
        shl     eax,1
        mov     r4_sqrd,eax             ;save r_sqrd * 4
        add     eax,r2_sqrd
        mov     r6_sqrd,eax             ;save r_sqrd * 6

;************************** Circle Part I ********************************
        ;
        ; this is for the octant 90 to 45 degrees
        ; calculate Bresenham's decision variable ------------------------
        ; BresD = a2_sqrd*((row-1)*row) + a_sqrd + r2_sqrd*(1-a_sqrd);

        mov     eax,1
        sub     eax,a_sqrd
        mul     r2_sqrd
        add     eax,a_sqrd
        mov     ecx,eax

        mov     eax,row
        dec     eax
        mul     row
        mul     a2_sqrd
        add     eax,ecx

        mov     BresD,eax               ;save Bresenham's decision variable

        mov     eax,diameter            ;get circle diameter
        dec     eax                     ;get logical number of rows
        mul     RAST_SIZE               ;calculate row address of bottom raster
        mov     esi,eax                 ;store in esi

        mov     edi,rastList            ;seed row address of top raster
        add     esi,edi                 ;add row addres of bottom raster
        mov     lastRast,esi            ;save offset to last raster for cleanup

        mov     eax,row                 ;get current row
        mul     a_sqrd
        mov     edx,eax                 ;save in Bresenham's A variable

        mov     ebx,0                   ;seed Bresenham's B variable
        mov     ecx,-1                  ;seed scan distance to 1 pixel
        add     ecx,diamOdd             ;adjust if odd width, (non-symmetric)

;--------------------- Circle Loop for first octant ---------------------
;       
;       ebx = Bresenham's B accumulator
;       ecx = distance between points on same line
;       edx = Bresenham's A accumulator variable for column position
;       esi = offset to bottom of circle raster line
;       edi = offset to top of circle raster line
;
        cmp     edx,ebx                 ;(row*a_sqrd) <= (col*r_sqrd)?
        jle     circc_loop1_done        ;no, finished with top loop

circc_part1_loop:
        cmp     dword ptr BresD,0       ;(BresD>=0)
        jl      circc_noBjmp1           ;no, stay on this row and move to next column

        ; check clip for top right part of circle
        mov     eax,LX                          ;get left clip offset
        add     eax,ecx                         ;add to right clip offset
        sub     eax,(RASTER PTR [edi]).x        ;get left X for top half of circle
        sub     eax,(RASTER PTR [edi]).cnt      ;add offset to right x for top half
        jns     circc_cx1_tleft_check           ;x is to left of right clip
        add     (RASTER PTR [edi]).cnt,eax      ;check left side against right clip
        jns     circc_cx1_tright_adjust         ;did adjustment intirely clip scan?
        or      (RASTER PTR [edi]).f,RASTER_DEAD  ;yes, then entire raster clipped
        jmp     short circc_cx1_tleft_check
circc_cx1_tright_adjust:
        mov     eax,LX                          ;right side is clipped
        add     eax,ecx
        mov     (RASTER PTR [edi]).rx,eax       ;store new right coordinate
        mov     eax,(RASTER PTR [edi]).x        ;have to store lx if you change rx
        mov     (RASTER PTR [edi]).lx,eax
        or      (RASTER PTR [edi]).f,RASTER_RCLIPPED  ;indicate raster is right clipped
        
        ; check clip for top left part of circle
circc_cx1_tleft_check:
        mov     eax,(RASTER PTR [edi]).x        ;get left X for top half of circle
        sub     eax,LX                          ;get difference between coordinates
        jns     circc_cx1_bottom_check          ;x is to right of left clip x
        add     (RASTER PTR [edi]).cnt,eax      ;check right side against left clip
        jns     circc_cx1_tleft_adjust          ;did adjustment entirely clip scan?
        or      (RASTER PTR [edi]).f,RASTER_DEAD  ;yes, then entire raster clipped
        jmp     short circc_cx1_bottom_check
circc_cx1_tleft_adjust:
        mov     eax,LX                          ;get new left coordinate
        mov     (RASTER PTR [edi]).lx,eax       ;store new left clipped coordinate
;        mov     (RASTER PTR [edi]).x,eax
        add     eax,(RASTER PTR [edi]).cnt      ;add offset to right side edge
        mov     (RASTER PTR [edi]).rx,eax       ;store new right edge
        or      (RASTER PTR [edi]).f,RASTER_LCLIPPED  ;indicate raster is left clipped

        ; check clip for bottom right part of circle
circc_cx1_bottom_check:
        mov     eax,LX                          ;get left clip offset
        add     eax,ecx                         ;add to right clip offset
        sub     eax,(RASTER PTR [esi]).x        ;get left X for top half of circle
        sub     eax,(RASTER PTR [esi]).cnt      ;add offset to right x for top half
        jns     circc_cx1_bleft_check           ;x is to left of right clip
        add     (RASTER PTR [esi]).cnt,eax      ;check left side against right clip
        jns     circc_cx1_bright_adjust         ;did adjustment intirely clip scan?
        or      (RASTER PTR [esi]).f,RASTER_DEAD  ;yes, then entire raster clipped
        jmp     short circc_cx1_bleft_check
circc_cx1_bright_adjust:
        mov     eax,LX                          ;right side is clipped
        add     eax,ecx
        mov     (RASTER PTR [esi]).rx,eax       ;store new right coordinate
        mov     eax,(RASTER PTR [esi]).x
        mov     (RASTER PTR [esi]).lx,eax
        or      (RASTER PTR [esi]).f,RASTER_RCLIPPED  ;indicate raster is right clipped

        ; check clip for bottom left part of circle
circc_cx1_bleft_check:
        mov     eax,(RASTER PTR [esi]).x        ;store left X for bottom half of circle
        sub     eax,LX                          ;get difference between coordinates        
        jns     circc_cx1_next                  ;x is to right of clip x
        add     (RASTER PTR [esi]).cnt,eax      ;adjust scan length
        jns     circc_cx1_bleft_adjust          ;did adjustment entirely clip scan?
        or      (RASTER PTR [esi]).f,RASTER_DEAD  ;yes, then entire raster clipped
        jmp     short circc_cx1_next
circc_cx1_bleft_adjust:
        mov     eax,LX                          ;get new left coordinate
        mov     (RASTER PTR [esi]).lx,eax       ;store new left clipped coordinate
;        mov     (RASTER PTR [esi]).x,eax        ;store new left clipped coordinate
        add     eax,(RASTER PTR [esi]).cnt      ;add offset to right side edge
        mov     (RASTER PTR [esi]).rx,eax       ;store new right edge
        or      (RASTER PTR [esi]).f,RASTER_LCLIPPED  ;indicate raster is left clipped

circc_cx1_next:
        add     edi,RASTER_SIZE         ;move to next raster in top part of circle
        sub     esi,RASTER_SIZE         ;move to previous row in bottom part of circle

        dec     row                     ;row--
        sub     edx,a_sqrd              ;adjust Bresenham's A variable
        mov     eax,edx
        shl     eax,2                   ;mul by 4
        sub     BresD,eax               ;BresD -= 4*a_sqrd*row

circc_noBjmp1:
        dec     LX                      ;move to previous left column
        add     ecx,2                   ;increment distance between left & right points
        inc     col                     ;col++

        mov     eax,r6_sqrd             ;6*r_sqrd
        lea     eax,[eax+ebx*4]         ;4*ebx + 6*r_sqrd
        add     BresD,eax               ;BresD += 4*ebx + 6*r_sqrd
        add     ebx,r_sqrd              ;adjust Bresenham's B variable, ebx = col*r_sqrd

        cmp     edx,ebx                 ;(row*a_sqrd) <= (col*r_sqrd)?
        jg      circc_part1_loop        ;no, finished with top loop

circc_loop1_done:
;************************** Circle Part II *******************************
        ;
        ; this is for the octant 45 to 0 degrees
        ; calculate decision variable ------------------------------------
        ; BresD = r2_sqrd*(col+1)*col + a2_sqrd*(row*(row-2)+1) + (1-a2_sqrd)*r_sqrd

        mov     eax,col
        inc     eax
        mul     col
        mul     r2_sqrd
        mov     BresD,eax

        mov     eax,row
        sub     eax,2
        mul     row
        inc     eax
        mul     a2_sqrd
        add     BresD,eax

        mov     eax,1
        sub     eax,a2_sqrd
        mul     r_sqrd
        add     BresD,eax               ;save Bresenham's decision variable

        mov     eax,row
        mul     a4_sqrd
        mov     ebx,eax

        mov     eax,col
        mul     r4_sqrd
        mov     BresA,eax               ;save Bresenham's A variable

        mov     edx,BresD               ;put BresD in edx

;--------------------- Circle Loop for second octant ---------------------
;
;       eax = scratch
;       ebx = Bresenham's B accumulator
;       ecx = distance between points on same line
;       edx = Bresenham's D accumulator variable for column position
;       esi = offset to bottom of circle
;       edi = offset to top of circle
;

circc_part2_loop:

        ; check clip for top right part of circle
        mov     eax,LX                          ;get left clip offset
        add     eax,ecx                         ;add to right clip offset
        sub     eax,(RASTER PTR [edi]).x        ;get left X for top half of circle
        sub     eax,(RASTER PTR [edi]).cnt      ;add offset to right x for top half
        jns     circc_cx2_tleft_check           ;x is to left of right clip
        add     (RASTER PTR [edi]).cnt,eax      ;check left side against right clip
        jns     circc_cx2_tright_adjust         ;did adjustment intirely clip scan?
        or      (RASTER PTR [edi]).f,RASTER_DEAD  ;yes, then entire raster clipped
        jmp     short circc_cx2_tleft_check
circc_cx2_tright_adjust:
        mov     eax,LX                          ;right side is clipped
        add     eax,ecx
        mov     (RASTER PTR [edi]).rx,eax       ;store new right coordinate
        mov     eax,(RASTER PTR [edi]).x        ;have to store lx if you change rx
        mov     (RASTER PTR [edi]).lx,eax
        or      (RASTER PTR [edi]).f,RASTER_RCLIPPED  ;indicate raster is right clipped
        
        ; check clip for top left part of circle
circc_cx2_tleft_check:
        mov     eax,(RASTER PTR [edi]).x        ;get left X for top half of circle
        sub     eax,LX                          ;get difference between coordinates
        jns     circc_cx2_bottom_check          ;x is to right of left clip x
        add     (RASTER PTR [edi]).cnt,eax      ;check right side against left clip
        jns     circc_cx2_tleft_adjust          ;did adjustment entirely clip scan?
        or      (RASTER PTR [edi]).f,RASTER_DEAD  ;yes, then entire raster clipped
        jmp     short circc_cx2_bottom_check
circc_cx2_tleft_adjust:
        mov     eax,LX                          ;get new left coordinate
        mov     (RASTER PTR [edi]).lx,eax       ;store new left clipped coordinate
;        mov     (RASTER PTR [edi]).x,eax        ;store new left clipped coordinate
        add     eax,(RASTER PTR [edi]).cnt      ;add offset to right side edge
        mov     (RASTER PTR [edi]).rx,eax       ;store new right edge
        or      (RASTER PTR [edi]).f,RASTER_LCLIPPED  ;indicate raster is left clipped

        ; check clip for bottom right part of circle
circc_cx2_bottom_check:
        mov     eax,LX                          ;get left clip offset
        add     eax,ecx                         ;add to right clip offset
        sub     eax,(RASTER PTR [esi]).x        ;get left X for top half of circle
        sub     eax,(RASTER PTR [esi]).cnt      ;add offset to right x for top half
        jns     circc_cx2_bleft_check           ;x is to left of right clip
        add     (RASTER PTR [esi]).cnt,eax      ;check left side against right clip
        jns     circc_cx2_bright_adjust         ;did adjustment intirely clip scan?
        or      (RASTER PTR [esi]).f,RASTER_DEAD  ;yes, then entire raster clipped
        jmp     short circc_cx2_bleft_check
circc_cx2_bright_adjust:
        mov     eax,LX                          ;right side is clipped
        add     eax,ecx
        mov     (RASTER PTR [esi]).rx,eax       ;store new right coordinate
        mov     eax,(RASTER PTR [esi]).x
        mov     (RASTER PTR [esi]).lx,eax
        or      (RASTER PTR [esi]).f,RASTER_RCLIPPED  ;indicate raster is right clipped

        ; check clip for bottom left part of circle
circc_cx2_bleft_check:
        mov     eax,(RASTER PTR [esi]).x        ;store left X for bottom half of circle
        sub     eax,LX                          ;get difference between coordinates        
        jns     circc_cx2_next                  ;x is to right of clip x
        add     (RASTER PTR [esi]).cnt,eax      ;adjust scan length
        jns     circc_cx2_bleft_adjust          ;did adjustment entirely clip scan?
        or      (RASTER PTR [esi]).f,RASTER_DEAD  ;yes, then entire raster clipped
        jmp     short circc_cx2_next
circc_cx2_bleft_adjust:
        mov     eax,LX                          ;get new left coordinate
        mov     (RASTER PTR [esi]).lx,eax       ;store new left clipped coordinate
;        mov     (RASTER PTR [esi]).x,eax        ;store new left clipped coordinate
        add     eax,(RASTER PTR [esi]).cnt      ;add offset to right side edge
        mov     (RASTER PTR [esi]).rx,eax       ;store new right edge
        or      (RASTER PTR [esi]).f,RASTER_LCLIPPED  ;indicate raster is left clipped

circc_cx2_next:
        add     edi,RASTER_SIZE         ;move to next raster in top part of circle
        sub     esi,RASTER_SIZE         ;move to previous row in bottom part of circle

        cmp     edx,0                   ;(edx<=0)
        jg      circc_noBjmp2           ;handle Bresenham's jump to next line

        inc     col
        add     ecx,2                   ;inc column distance between points
        dec     LX                      ;move to previous left column

        mov     eax,BresA
        add     eax,r4_sqrd
        mov     BresA,eax
        add     edx,eax                 ;adjust Bresenham's D variable

circc_noBjmp2:
        add     edx,a6_sqrd
        sub     edx,ebx                 ;edx += a6_sqrd - ebx
        sub     ebx,a4_sqrd             ;adjust Bresenham's B variable, ebx = a4_sqrd * row

        dec     row
        jns     circc_part2_loop        ;continue, else exit

        ret

GFXasmCircleClip  ENDP

        END
