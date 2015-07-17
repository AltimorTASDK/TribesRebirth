;=========================================================================
;==   
;== $Workfile:$
;== $Version$
;== $Revision:$
;==   
;== DESCRIPTION:
;==   Extremely fast linear interpolation routine
;==   
;== (c) Copyright 1995, Dynamix Inc.   All rights reserved.
;==   
;=========================================================================

.386
.MODEL FLAT, syscall

INCLUDE g_raster.inc

.DATA   ; these variables must remain in this order

        SpanColCY       dd      0       ; amount added to X when carry on LvarA
        SpanCol         dd      0       ; amount added to X when no carry

        SpanCntCY       dd      0       ; span width when carry on LvarA
        SpanCnt         dd      0       ; span width when no carry

.CODE

IFDEF __MASM__
ARG equ
ELSE
LOCALS
ENDIF

Point STRUC
        px      dd ?
        py      dd ?
Point ENDS

;北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
;     
;     GFXasmLine2D(rastList,x1,y1,x2,y2)  Rasterizes a line into rastList
;     
;     This routine rasterizes a line using two unique techniques.  The
;     first technique allows the rasterizer to step through the rows
;     storing the starting X and the Span or Span+1 in a single 
;     calculation per row.  Therefore, the number of iterations through
;     the inner loop is determined by the number of scan lines.  The 
;     second technique involves a modified Bresenham's algorithm that
;     normalizes the decision variable so that the adjustment to
;     determine the span is done with a single addition to eliminate
;     any jumps or adjustments to the decision variable accumulator.
;     These combined techniques provide an extremely fast rasterization.
;     
;北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
;       
;       rastList   - pointer to the rasterization list
;       pt1        - starting x,y point
;       pt2        - ending x,y point
;       
;北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

GFXasmLine2D    PROC C \
        USES    eax ebx ecx esi edi,
        ARG     rastList, pt1, pt2
        LOCAL   x1,y1,x2,y2, Lvar, Span, LastRast, Slope, signX
        PUBLIC  C GFXasmLine2D

;-------------------- data prep for Bresenham's Line ---------------------

        mov     esi,pt1                 ;get pointer to first point        
        mov     eax,(Point PTR [esi]).px ;get coordinates into local variables
        mov     x1,eax
        mov     eax,(Point PTR [esi]).py
        mov     y1,eax
        mov     esi,pt2                 ;get pointer to second point
        mov     eax,(Point PTR [esi]).px
        mov     x2,eax
        mov     eax,(Point PTR [esi]).py
        mov     y2,eax

        mov     edi,rastList            ;get pointer to raster list

        mov     ecx,y2                  ;seed row count
        mov     ebx,x2                  ;all lines are drawn top to bottom
        cmp     y1,ecx
        jle     NoSwapY
        xchg    ebx,x1                  ;swap x1,y1 with x2,y2
        mov     x2,ebx
        xchg    ecx,y1
        mov     y2,ecx
NoSwapY:
        sub     ecx,y1                  ;get number of rows
        mov     eax,RASTER_SIZE         ;get size of raster struct
        mul     ecx                     ;get distance to bottom of raster list
        mov     LastRast,eax            ;save offset to last raster line

        inc     ecx                     ;get logical number of rows

        mov     signX,1                 ;seed direction of line
        sub     ebx,x1
        jns     LtoR                    ;line is drawn left to right
        neg     signX                   ;seed direction of line
LtoR:
        add     ebx,signX               ;adjust for logical number of cols
        mov     eax,ebx                 ;get number of cols
        cdq                             ;span = edx:eax/ebx or col/row
        idiv    ecx                     ;span in eax, remainder in edx

        mov     Slope,eax               ;save Slope
        cmp     eax,0                   ;is slope steep?
        je      SlopeSteep              ;yes

        mov     SpanCol,eax             ;no, store spans for columns and counts
        mov     SpanColCY,eax
        cmp     signX,0                 ;is line r to l?
        jge     SpanPos                 ;yes
        neg     eax                     ;no, change sign of column amount
SpanPos:
        dec     eax                     ;span count is one less than column amount
        mov     SpanCnt,eax
        mov     SpanCntCY,eax
        cmp     edx,0                   ;was there a residual Lvar amount?
        je      SlopeDone               ;no, ratio was a perfect multiple
        mov     eax,signX
        add     SpanColCY,eax           ;yes, adjust spans for when carry occurs
        inc     SpanCntCY
        jmp     short SlopeDone

SlopeSteep:
        mov     SpanCnt,eax             ;steep slope has slightly different spans
        mov     SpanCntCY,eax
        mov     SpanCol,eax
        mov     eax,signX
        mov     SpanColCY,eax

SlopeDone:
        mov     eax,0                   ;Now calculate Lvar
        cmp     edx,0                   ;is edx negetive
        jge     LdivPos
        neg     edx
LdivPos:
        div     ecx                     ;divide residual/row to get .32 Lvar
        bt      eax,1fh                 ;is Lvar > or < 1/2?  test bit to find out
        sbb     ebx,ebx                 ;get index based on how bit was set
        mov     esi,SpanCnt[ebx*4]      ;load initial span based on index
        inc     eax                     ;minor adjustment for rounding errors
        mov     Lvar,eax

        ; now figure out the seed value for the Lvar accumulator (rather tricky)
        cmp     Slope,0                 ;is slope steep?
        je      SteepSeed               ;yes, seed Lvar accumulator for steep line

        ;flat seed ----------------------
        mov     edx,ecx                 ;for flat slope, seed is 
        shr     edx,1                   ;((row/2).32)/row + Lvar.32
        mov     eax,0                   ;for example, if Lvar is 2/5, seed is 5/7
        div     ecx                     ;if Lvar is 3/5, seed is 5/5
        add     eax,Lvar
        jmp     short SeedDone

        ;steep seed ---------------------
SteepSeed:
        mov     edx,x2                  ;for steep slope, seed is
        sub     edx,x1                  ;((col/2).32)/row + Lvar.32
        jns     ssPos
        neg     edx
ssPos:
        inc     edx                     ;the Lvar.32 is only added due to the 
        mov     ebx,0
        mov     eax,edx
        shr     eax,1
        add     eax,edx
        cmp     ecx,eax
        jg      steep_adj       ;no carry on first step
        mov     ebx,-1          ;carry on first step
steep_adj:

        shr     edx,1                   ;arrangement of the algorithm below.
        mov     eax,0                   ;if you work this out on paper, you 
        div     ecx                     ;probably wouldn't add this amount
        add     eax,Lvar                ;ex. Lvar is 7/11, seed is 3/11 + Lvar

SeedDone:                               ;ex. Lvar is 5/7,  seed is 2/7  + Lvar
        mov     edx,x1                  ;seed starting column position
        cmp     signX,0                 ;is line l to r?
        jg      l_rast_lr               ;yes,

        add     edx,SpanCol[ebx*4]      ;add span to adjust for line ending
        inc     edx                     ;at x1, add 1 to land exactly on x1 for flat lines
        cmp     Slope,0
        jne     rl_steep                ;skip adjustment if line is steep
        dec     edx
        sub     eax,Lvar 
        sub     edx,SpanCol[ebx*4]      ;add span to adjust for line ending
rl_steep:

;-------------------------------------------------------------------------
;       eax     Lvar Accumulator
;       ebx     span index
;       ecx     number of rows
;       edx     starting x value
;       esi     initial span = 1/2 base span+1, (to center line)
;       edi     pointer to raster list entry
;       Lvar    Lvar
;-------------------------------------------------------------------------

l_rast_rl:
        mov     (RASTER PTR [edi]).x,edx        ;store left X
        mov     (RASTER PTR [edi]).cnt,esi      ;store span
        mov     (RASTER PTR [edi]).f,RASTER_STEP_Y      ;set rows flag
        lea     esi,[edi+RASTER_SIZE]           ;get offset to next line
        mov     (RASTER PTR [edi]).next,esi     ;store offset to next row
        mov     edi,esi                         ;move to next raster line

        add     eax,Lvar                        ;add Lvar to Lvar accumulator
        sbb     ebx,ebx                         ;get index into ebx
        
        add     edx,SpanCol[ebx*4]              ;add span to left X offset
        mov     esi,SpanCnt[ebx*4]              ;get span count index
        
        dec     ecx                             ;reduce row count
        jnz     l_rast_rl                       ;process next raster entry

        jmp     short l_rast_done               ;exit

l_rast_lr:
        mov     (RASTER PTR [edi]).x,edx        ;store left X
        mov     (RASTER PTR [edi]).cnt,esi      ;store span
        mov     (RASTER PTR [edi]).f,RASTER_STEP_Y      ;set rows flag
        lea     esi,[edi+RASTER_SIZE]           ;get offset to next line
        mov     (RASTER PTR [edi]).next,esi     ;store offset to next row
        mov     edi,esi                         ;move to next raster line
        
        add     edx,SpanCol[ebx*4]              ;add span to left X offset
        add     eax,Lvar                        ;add Lvar to Lvar accumulator
        sbb     ebx,ebx                         ;get index into ebx
        mov     esi,SpanCnt[ebx*4]              ;get span count index
        
        dec     ecx                             ;reduce row count
        jnz     l_rast_lr                       ;process next raster entry

l_rast_done:
        sub     edi,RASTER_SIZE                 ;get offset to last raster line used
        mov     (RASTER PTR [edi]).next,0       ;seed last rows next ptr
        mov     esi,rastList                    ;get pointer to start of raster list
        mov     (RASTER PTR [esi]).f,0          ;no STEP_Y on first line

        RET

GFXasmLine2D  ENDP

;北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
;     
;     GFXasmLine2D_g(rastList,x1,y1,x2,y2,s1,s2)  
;     
;     Rasterizes a line into rastList with gouraud shading
;     
;北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
;       
;       rastList   - pointer to the rasterization list
;       pt1        - starting x,y point
;       pt2        - ending x,y point
;       s1,s2      - beginning and ending shade values (0-255)
;       
;北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

GFXasmLine2D_g  PROC C \
        USES    eax ebx ecx esi edi,
        ARG     rastList, pt1, pt2, s1,s2
        LOCAL   Lvar, Span, LastRast, Slope, signX
        PUBLIC  C GFXasmLine2D_g

        RET

GFXasmLine2D_g  ENDP

        END
