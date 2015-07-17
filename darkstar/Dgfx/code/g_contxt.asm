
.486p
.MODEL FLAT, SYSCALL

IFDEF __MASM__
  ARG equ
ELSE
  LOCALS
ENDIF

 ;------------------------------ DATA SEGMENT
.DATA
INCLUDE g_contxt.inc

_gfxPDC  GFXPolyDrawContext    <>
_gfxPRC  GFXPolyRasterContext    <>
_gfxRC   GFXRenderContext        <>
_gfxSpan GFXSpan                 <>

PUBLIC _gfxPDC
PUBLIC _gfxPRC
PUBLIC _gfxRC
PUBLIC _gfxSpan

END	