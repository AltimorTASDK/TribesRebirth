.486p
.MODEL FLAT, SYSCALL

IFDEF __MASM__
  ARG equ
ENDIF

.DATA

include m_mat.inc
include m_vec.inc

TransformedVertex STRUC
   fStatus     dd ?
   pt_x        dd ?
   pt_y        dd ?
   pt_z        dd ?
   tr_x        dd ?
   tr_y        dd ?
   tr_z        dd ?
   tr_w        dd ?
   tx_x        dd ?
   tx_y        dd ?
   fDist       dd ?
   col_haze    dd ?
   col_red     dd ?
   col_green   dd ?
   col_blue    dd ?
   col_alpha   dd ?
TransformedVertex ENDS

ClipAllVis     equ 0
ClipNear       equ 1h
ClipFar        equ 2h
ClipFarSphere  equ 4h
ClipLeft       equ 8h
ClipRight      equ 10h
ClipTop        equ 20h
ClipBottom     equ 40h
Projected      equ 2000h

useTextures    equ 1
useIntensities equ 2
useHazes       equ 4

TOC            dd ?
TOS            dd ?
fVs            dd ?
fVc            dd ?
fInvNearDist   dd ?

IFDEF __MASM__
m_Point3F_TMat3F_mul PROTO NEAR C, srcPoint:DWORD, mat:DWORD, dstPoint:DWORD
ELSE
EXTRN C m_Point3F_TMat3F_mul:PROC
ENDIF

.CODE


IFNDEF __MASM__
transform_project_prepare PROC C \
   USES  ,
   ARG aTOC: DWORD, aTOS: DWORD, \
       afVs: DWORD, afVc: DWORD, afInvNearDist : DWORD
ELSE
transform_project_prepare PROC C \
   ARG aTOC: DWORD, aTOS: DWORD, \
       afVs: DWORD, afVc: DWORD, afInvNearDist : DWORD
ENDIF
   PUBLIC C transform_project_prepare
      mov      eax, TOC
      mov      edx, afInvNearDist
      mov      eax, aTOC
      mov      fInvNearDist, edx
      mov      edx, aTOS
      mov      TOC, eax
      mov      TOS, edx
      mov      eax, afVs
      mov      edx, afVc
      mov      fVs, eax
      mov      fVc, edx
      ret
transform_project_prepare ENDP


persp_transform_project PROC C \
   USES  ebx edi esi,
   ARG raw: DWORD, pDest: DWORD
   PUBLIC C persp_transform_project

      mov      eax, raw
      mov      esi, pDest

      mov      edx, TOS
      lea      esi, (TransformedVertex PTR [esi]).tr_x
      
IFDEF __MASM__
      invoke     m_Point3F_TMat3F_mul, eax, edx, esi
ELSE
      call     m_Point3F_TMat3F_mul, eax, edx, esi
ENDIF

      mov      edx, TOC
      mov      eax, raw
      fld      fInvNearDist

      fld      (Point4 PTR [eax]).x
      fmul     (RMat3 PTR [edx]).m3_0_1
      fld      (Point4 PTR [eax]).y
      fmul     (RMat3 PTR [edx]).m3_1_1
      fld      (Point4 PTR [eax]).z
      fmul     (RMat3 PTR [edx]).m3_2_1
      fxch     st(1)
      faddp    st(2), st
      fld      (TMat3 PTR [edx]).t3y
      faddp    st(1), st
      mov      esi, pDest
      faddp    st(1), st
      fmulp    st(1), st
      fst      (TransformedVertex PTR [esi]).tr_w

;      m_mul( raw, getTOS(), &pDest->fTransformedPoint );
;      // w = (camera space y) / NearDist
;      TMat3F const & toc = getTOC();
      
;      pDest->fTransformedPoint.w = (raw.x * toc.m[0][1] + 
;                  raw.y * toc.m[1][1] + 
;                  raw.z * toc.m[2][1] + 
;                  toc.p.y) * fInvNearDist;
;      getVisibilityFlags(pDest);

;      pDest->fStatus = ClipAllVis;

      mov      eax, (TransformedVertex PTR [esi]).tr_z
      mov      edx, (TransformedVertex PTR [esi]).tr_w

      test     edx, 07FFFFFFFh
      jz       @noInvW

      fld1
      fdivrp   st(1), st      ; do homogeneous divide on fpu stack while
                              ; we test the visibility flags

@noInvW:
      mov      ebx, ClipNear
      test     eax, 080000000h
      jnz      @done
      mov      ebx, ClipFar
      cmp      eax, edx
      jg       @done
      mov      edi, 07FFFFFFFh

      mov      eax, (TransformedVertex PTR [esi]).tr_x
      mov      ecx, (TransformedVertex PTR [esi]).tr_y

      and      eax, edi
      and      ecx, edi
      and      edx, edi

      cmp      eax, edx
      jg       @xOutside
      cmp      ecx, edx
      jg       @yOutside

; project and stuff here
      mov      (TransformedVertex PTR [esi]).fStatus, Projected
      mov      eax, fVs
      mov      ebx, fVc
      
      fld      st
      fld      st

      ; st = 1/w, 1/w, 1/w

      fmul     (TransformedVertex PTR [esi]).tr_x
      fxch     st(1)
      fmul     (TransformedVertex PTR [esi]).tr_y
      fxch     st(1)
      fmul     (Point2 PTR [eax]).x
      fxch     st(1)
      fmul     (Point2 PTR [eax]).y
      fxch     st(1)
      fadd     (Point2 PTR [ebx]).x
      fxch     st(1)
      fadd     (Point2 PTR [ebx]).y
      fxch     st(2)

      fstp     (TransformedVertex PTR [esi]).pt_z
      fstp     (TransformedVertex PTR [esi]).pt_x
      fstp     (TransformedVertex PTR [esi]).pt_y
      mov      eax, Projected
      ret
@xOutside:
      mov      ebx, ClipLeft
      mov      eax, (TransformedVertex PTR [esi]).tr_x
      test     eax, 080000000h
      jnz      @done
      
      mov      (TransformedVertex PTR [esi]).fStatus, ClipRight
      mov      eax, ClipRight
      fstp     st
      ret
@yOutside:
      mov      ecx, (TransformedVertex PTR [esi]).tr_y
      mov      ebx, ClipTop
      test     ecx, 080000000h
      jz       @done
      mov      (TransformedVertex PTR [esi]).fStatus, ClipBottom
      mov      eax, ClipBottom
      fstp     st
      ret
@done:
      mov      (TransformedVertex PTR [esi]).fStatus, ebx
      mov      eax, ebx
      fstp     st
      ret
persp_transform_project ENDP

ortho_transform_project PROC C \
   USES  ebx edi esi,
   ARG raw: DWORD, pDest: DWORD
   PUBLIC C ortho_transform_project

      mov      eax, raw
      mov      esi, pDest

      mov      edx, TOS
      lea      esi, (TransformedVertex PTR [esi]).tr_x
      
IFDEF __MASM__
      invoke     m_Point3F_TMat3F_mul, eax, edx, esi
ELSE
      call     m_Point3F_TMat3F_mul, eax, edx, esi
ENDIF

      mov      edx, TOC
      mov      eax, raw
      mov      esi, pDest

;      m_mul( raw, getTOS(), (Point3F *) &pDest->fTransformedPoint );
;      pDest->fTransformedPoint.w = fInvNearDist;
;      getVisibilityFlags(pDest);

      fld      fInvNearDist
      fst      (TransformedVertex PTR [esi]).tr_w

      mov      eax, (TransformedVertex PTR [esi]).tr_z
      mov      edx, (TransformedVertex PTR [esi]).tr_w

      fld1
      fdivrp   st(1), st      ; do homogeneous divide on fpu stack while
                              ; we test the visibility flags
 
      mov      ebx, ClipNear
      test     eax, 080000000h
      jnz      @odone
      mov      ebx, ClipFar
      cmp      eax, edx
      jg       @odone
      mov      edi, 07FFFFFFFh

      mov      eax, (TransformedVertex PTR [esi]).tr_x
      mov      ecx, (TransformedVertex PTR [esi]).tr_y

      and      eax, edi
      and      ecx, edi
      and      edx, edi

      cmp      eax, edx
      jg       @oxOutside
      cmp      ecx, edx
      jg       @oyOutside

; project and stuff here
      mov      (TransformedVertex PTR [esi]).fStatus, Projected
      mov      eax, fVs
      mov      ebx, fVc
      
      fld      st
      fld      st

      ; st = 1/w, 1/w, 1/w

      fmul     (TransformedVertex PTR [esi]).tr_x
      fxch     st(1)
      fmul     (TransformedVertex PTR [esi]).tr_y
      fxch     st(1)
      fmul     (Point2 PTR [eax]).x
      fxch     st(1)
      fmul     (Point2 PTR [eax]).y
      fxch     st(1)
      fadd     (Point2 PTR [ebx]).x
      fxch     st(1)
      fadd     (Point2 PTR [ebx]).y
      fxch     st(2)

      fmul     (TransformedVertex PTR [esi]).tr_z

      fstp     (TransformedVertex PTR [esi]).pt_z
      fstp     (TransformedVertex PTR [esi]).pt_x
      fstp     (TransformedVertex PTR [esi]).pt_y
      mov      eax, Projected
      ret
@oxOutside:
      mov      ebx, ClipLeft
      mov      eax, (TransformedVertex PTR [esi]).tr_x
      test     eax, 080000000h
      jnz      @odone
      
      mov      (TransformedVertex PTR [esi]).fStatus, ClipRight
      mov      eax, ClipRight
      fstp     st
      ret
@oyOutside:
      mov      ecx, (TransformedVertex PTR [esi]).tr_y
      mov      ebx, ClipTop
      test     ecx, 080000000h
      jz       @odone
      mov      (TransformedVertex PTR [esi]).fStatus, ClipBottom
      mov      eax, ClipBottom
      fstp     st
      ret
@odone:
      mov      (TransformedVertex PTR [esi]).fStatus, ebx
      mov      eax, ebx
      fstp     st
      ret
ortho_transform_project ENDP


;      AssertFatal( pVert  != 0, 
;         "TS::Camera::TransformProject2: invalid destination pointer." );
;      double invW = 1.0f / pVert->fTransformedPoint.w;
;      
;      pVert->fPoint.x = float(pVert->fTransformedPoint.x * invW * fVs.x + fVc.x);
;      pVert->fPoint.y = float(pVert->fTransformedPoint.y * invW * fVs.y + fVc.y);
;      pVert->fPoint.z = float(invW);
;
;      pVert->fStatus |= TransformedVertex::Projected;

.data

.code

clip_intersect PROC C \
   USES  ebx edi esi,
   ARG p1: DWORD, p2: DWORD, dest: DWORD, farDist: DWORD, in_flags: DWORD, planeIndex:DWORD, rect: DWORD, isOrtho: DWORD
   PUBLIC C clip_intersect

      mov      ecx, planeIndex
      mov      edi, p1
      mov      esi, p2
      mov      eax, dest
      mov      ebx, ClipAllVis
      jmp      @@tCalcJTable[ecx*4]
@@nearPlaneClip:
      ; t = pT1->z / (pT1->z - pT2->z)
   
      fld      (TransformedVertex PTR [edi]).tr_z
      fld      st
      fsub     (TransformedVertex PTR [esi]).tr_z
      fdivp    st(1), st
      jmp      @@interpolatePoint
@@farPlaneClip:
      ; t = (pT1->w - pT1->z) / (pT1->w - pT1->z - (pT2->w - pT2->z));
   
      fld      (TransformedVertex PTR [edi]).tr_w
      fsub     (TransformedVertex PTR [edi]).tr_z
      fld      (TransformedVertex PTR [esi]).tr_w
      fsub     (TransformedVertex PTR [esi]).tr_z

      ;st(1) = t1->w - t1->z
      ;st(0) = t2->w - t2->z

      fsubr    st, st(1)
      fdivp    st(1), st
      jmp      @@interpolatePoint
@@farSphereClip:
      ; t = (farDist - pT1->fDist) / (pT2->fDist - pT1->fDist);

      fld      farDist
      fsub     (TransformedVertex PTR [edi]).fDist
      fld      (TransformedVertex PTR [esi]).fDist
      fsub     (TransformedVertex PTR [edi]).fDist
      fdivp    st(1), st
      jmp      @@interpolatePoint
@@leftPlaneClip:
      ; t = (pT1->x + pT1->w) / (pT1->x - pT2->x + pT1->w - pT2->w);

      fld      (TransformedVertex PTR [edi]).tr_x
      fadd     (TransformedVertex PTR [edi]).tr_w
      fld      (TransformedVertex PTR [esi]).tr_x
      fadd     (TransformedVertex PTR [esi]).tr_w
      fld      st(1)
      fsubrp   st(1), st
      fdivp    st(1), st
      jmp      @@interpolatePoint
@@rightPlaneClip:
      ; t = (pT1->x - pT1->w) / (pT1->x - pT2->x + pT2->w - pT1->w);

      fld      (TransformedVertex PTR [edi]).tr_x
      fsub     (TransformedVertex PTR [edi]).tr_w
      fld      (TransformedVertex PTR [esi]).tr_x
      fsub     (TransformedVertex PTR [esi]).tr_w
      fld      st(1)
      fsubrp   st(1), st
      fdivp    st(1), st
      jmp      @@interpolatePoint
@@topPlaneClip:
      ; t = (pT1->y - pT1->w) / (pT1->y - pT2->y + pT2->w - pT1->w);

      fld      (TransformedVertex PTR [edi]).tr_y
      fsub     (TransformedVertex PTR [edi]).tr_w
      fld      (TransformedVertex PTR [esi]).tr_y
      fsub     (TransformedVertex PTR [esi]).tr_w
      fld      st(1)
      fsubrp   st(1), st
      fdivp    st(1), st
      jmp      @@interpolatePoint
@@bottomPlaneClip:
      ; t = (pT1->y + pT1->w) / (pT1->y - pT2->y + pT1->w - pT2->w);

      fld      (TransformedVertex PTR [edi]).tr_y
      fadd     (TransformedVertex PTR [edi]).tr_w
      fld      (TransformedVertex PTR [esi]).tr_y
      fadd     (TransformedVertex PTR [esi]).tr_w
      fld      st(1)
      fsubrp   st(1), st
      fdivp    st(1), st
@@interpolatePoint:
      ; interpolate the w, z and farDist coordinates of the point... the other parts are
   
      fld      (TransformedVertex PTR [esi]).col_haze
      fsub     (TransformedVertex PTR [edi]).col_haze
      fld      (TransformedVertex PTR [esi]).tr_w
      fld      (TransformedVertex PTR [esi]).tr_z
      fld      (TransformedVertex PTR [esi]).fDist
      fld      (TransformedVertex PTR [edi]).tr_w
      fsub     st(3), st
      fld      (TransformedVertex PTR [edi]).tr_z
      fsub     st(3), st
      fld      (TransformedVertex PTR [edi]).fDist
      fsub     st(3), st
      ; st = d1, z1, w1, d2-d1, z2-z1, w2-w1, h2-h1, t
   
      fxch     st(7)
      ; st = t, z1, w1, d2-d1, z2-z1, w2-w1, d1
      fmul     st(6), st
      fmul     st(5), st
      fmul     st(4), st
      fmul     st(3), st
      fxch     st(7)
      faddp    st(3), st
      faddp    st(3), st
      faddp    st(3), st
      fld      (TransformedVertex PTR [edi]).col_haze
      faddp    st(4), st
      fstp     (TransformedVertex PTR [eax]).fDist
      fstp     (TransformedVertex PTR [eax]).tr_z
      fstp     (TransformedVertex PTR [eax]).tr_w
      fstp     (TransformedVertex PTR [eax]).col_haze

      ; now do plane-specific interps...
      jmp      @@interpJTable[ecx*4]
@@yInterp:
      fld      (TransformedVertex PTR [esi]).tr_x
      fsub     (TransformedVertex PTR [edi]).tr_x
      mov      edx, (TransformedVertex PTR [eax]).tr_w
      cmp      ecx, 6
      fmul     st, st(1)
      jne      @@noNegY
      or       edx, 080000000h
@@noNegY:
      mov      (TransformedVertex PTR [eax]).tr_y, edx
      fadd     (TransformedVertex PTR [edi]).tr_x
      fstp     (TransformedVertex PTR [eax]).tr_x
      jmp      @@project
@@xInterp:
      fld      (TransformedVertex PTR [esi]).tr_y
      fsub     (TransformedVertex PTR [edi]).tr_y
      mov      edx, (TransformedVertex PTR [eax]).tr_w
      cmp      ecx, 3
      fmul     st, st(1)
      jne      @@noNegX
      or       edx, 080000000h
@@noNegX:
      mov      (TransformedVertex PTR [eax]).tr_x, edx
      fadd     (TransformedVertex PTR [edi]).tr_y
      fstp     (TransformedVertex PTR [eax]).tr_y
      jmp      @@yVisTest
@@nearFarInterp:
      fld      (TransformedVertex PTR [esi]).tr_x
      fld      (TransformedVertex PTR [esi]).tr_y
      fld      (TransformedVertex PTR [edi]).tr_x
      fsub     st(2), st
      fld      (TransformedVertex PTR [edi]).tr_y
      fsub     st(2), st
      ;st = y1, x1, y2-y1, x2-x1, t

      fxch     st(4)
      fmul     st(3), st
      fmul     st(2), st
      ;st = t, x1, t(y2-y1), t(x2-x1), y1
      fxch     st(1)
      faddp    st(3), st
      fxch     st(3)
      faddp    st(1), st
      fxch     st(1)
      fstp     (TransformedVertex PTR [eax]).tr_x
      fstp     (TransformedVertex PTR [eax]).tr_y

      ; do visibility checks on X
      mov      edx, (TransformedVertex PTR [eax]).tr_x
      mov      ecx, (TransformedVertex PTR [eax]).tr_w

      and      edx, 07FFFFFFFh
      cmp      edx, ecx
      jle      @@yVisTest

      ; outside of X view volume...
      mov      edx, (TransformedVertex PTR [eax]).tr_x
      mov      ebx, ClipLeft
      test     edx, 080000000h
      jnz      @@interpTextures  ; it's off the left side
      mov      ebx, ClipRight
      jmp      @@interpTextures  ; off to the right
@@yVisTest:
      mov      edx, (TransformedVertex PTR [eax]).tr_y
      mov      ecx, (TransformedVertex PTR [eax]).tr_w
      and      edx, 07FFFFFFFh
      cmp      edx, ecx
      jle      @@project
      mov      edx, (TransformedVertex PTR [eax]).tr_y
      mov      ebx, ClipBottom
      test     edx, 080000000h
      jnz      @@interpTextures
      mov      ebx, ClipTop
      jmp      @@interpTextures
@@project:
      fld1
      fdiv     (TransformedVertex PTR [eax]).tr_w

      mov      ebx, Projected
      mov      ecx, fVs
      mov      edx, fVc
      
      fld      st
      fld      st

      ; st = 1/w, 1/w, 1/w

      fmul     (TransformedVertex PTR [eax]).tr_x
      fxch     st(1)
      fmul     (TransformedVertex PTR [eax]).tr_y
      fxch     st(1)
      fmul     (Point2 PTR [ecx]).x
      fxch     st(1)
      fmul     (Point2 PTR [ecx]).y
      fxch     st(1)
      fadd     (Point2 PTR [edx]).x
      fxch     st(1)
      fadd     (Point2 PTR [edx]).y
      fxch     st(2)

      test     isOrtho, 1
      jz       @@noOrtho

      fmul     (TransformedVertex PTR [esi]).tr_z
@@noOrtho:
      fstp     (TransformedVertex PTR [eax]).pt_z
      fstp     (TransformedVertex PTR [eax]).pt_x
      fstp     (TransformedVertex PTR [eax]).pt_y
@@interpTextures:
      mov      (TransformedVertex PTR [eax]).fStatus, ebx
      mov      ecx, in_flags
      test     ecx, useTextures
      jz       @@testIntensities

      fld      (TransformedVertex PTR [esi]).tx_x
      fld      (TransformedVertex PTR [esi]).tx_y
      fld      (TransformedVertex PTR [edi]).tx_x
      fsub     st(2), st
      fld      (TransformedVertex PTR [edi]).tx_y
      fsub     st(2), st
      ;st = y1, x1, y2-y1, x2-x1, t

      fxch     st(4)
      fmul     st(3), st
      fmul     st(2), st
      ;st = t, x1, t(y2-y1), t(x2-x1), y1
      fxch     st(1)
      faddp    st(3), st
      fxch     st(3)
      faddp    st(1), st
      fxch     st(1)
      fstp     (TransformedVertex PTR [eax]).tx_x
      fstp     (TransformedVertex PTR [eax]).tx_y
@@testIntensities:
      test     ecx, useIntensities
      jz       @@cidone

      fld      (TransformedVertex PTR [esi]).col_red
      fld      (TransformedVertex PTR [esi]).col_green
      fld      (TransformedVertex PTR [esi]).col_blue
      fld      (TransformedVertex PTR [edi]).col_red
      fsub     st(3), st
      fld      (TransformedVertex PTR [edi]).col_green
      fsub     st(3), st
      fld      (TransformedVertex PTR [edi]).col_blue
      fsub     st(3), st
      ; st = d1, z1, w1, d2-d1, z2-z1, w2-w1, t
   
      fxch     st(6)
      ; st = t, z1, w1, d2-d1, z2-z1, w2-w1, d1
      fmul     st(5), st
      fmul     st(4), st
      fmul     st(3), st
      fxch     st(6)
      faddp    st(3), st
      faddp    st(3), st
      faddp    st(3), st
      fstp     (TransformedVertex PTR [eax]).col_blue
      fstp     (TransformedVertex PTR [eax]).col_green
      fstp     (TransformedVertex PTR [eax]).col_red

@@cidone:
      fstp     st       ; get rid of T.
      mov      eax, ebx
      ret

@@tCalcJTable dd @@nearPlaneClip, @@farPlaneClip, @@farSphereClip, @@leftPlaneClip, @@rightPlaneClip, @@topPlaneClip, @@bottomPlaneClip
@@interpJTable dd @@nearFarInterp, @@nearFarInterp, @@nearFarInterp, @@xInterp, @@xInterp, @@yInterp, @@yInterp

clip_intersect ENDP

END
