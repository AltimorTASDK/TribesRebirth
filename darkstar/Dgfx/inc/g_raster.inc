; ========================================================
;
;	$Workfile:$
;	$Version$
;
;    Internal rasterization system.
;
;  $Revision:$
;       $Log:$
;	
; (c) Copyright 1995, Dynamix Inc.  All rights reserved.
; ========================================================

; ========================================================
; For a definition of this structure, see g_raster.h
RASTER STRUC
f       dd ?
x       dd ?
cnt     dd ?
next    dd ?
lx      dd ?
rx      dd ?
u       dd ?
su      dd ?

v       dd ?
sv      dd ?
w       dd ?
sw      dd ?
sz      dd ?
srs     dd ?
z       dd ?
rs      dd ?

g		dd ?
b		dd ?
sg		dd ?
sb		dd ?

RASTER ENDS
RASTER_SIZE     equ     SIZE RASTER

; ========================================================
RASTER_LIST STRUC
top_y           dd ?
height          dd ?
maxRasters      dd ?
unusedRasters   dd ?
freeRaster      dd ?
topRaster       dd ?
list            dd ?

rl_stepU		dd ?
rl_stepV   		dd ?
rl_stepW		dd ?
rl_stepRS   	dd ?
rl_stepG		dd ?
rl_stepB		dd ?
rl_stepZ       dd ?

rl_color       dd ?
rl_texmap      dd ?
rl_shademap    dd ?
rl_transmap    dd ?
rl_hazeshift   dd ?
rl_redMap      dd ?
rl_greenMap    dd ?
rl_blueMap     dd ?   
RASTER_LIST ENDS

; ========================================================
EDGE STRUC
e_lx    dd ?
e_rx    dd ?
e_next  dd ?
EDGE ENDS

; ========================================================
EDGE_LIST STRUC
e_top_y           dd ?
e_height          dd ?      
e_maxEdges        dd ?
e_unusedEdges     dd ?
e_freeEdge        dd ?
e_list            dd ?
e_edgeList        dd ?
e_maxHeight       dd ?
e_usedHeight      dd ?
e_packedEdges     dd ?
EDGE_LIST ENDS

; Raster flags
RASTER_DEAD     equ     (1 SHL 2)       ;bits 2,3,4 are used as an index into
RASTER_LCLIPPED equ     (1 SHL 3)       ;a jump table. DO NOT change their order
RASTER_RCLIPPED equ     (1 SHL 4)       ;DO NOT use bits 0,1
RASTER_STEP_Y   equ     (1 SHL 5) 
RASTER_UNUSED   equ     (1 SHL 31)      ;set if raster is unused
