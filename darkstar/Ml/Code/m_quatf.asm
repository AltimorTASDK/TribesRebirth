;------------------------------------------------------------------------------
; Description 
;    
; $Workfile$
; $Revision$
; $Author  $
; $Modtime $
;
;------------------------------------------------------------------------------


INCLUDE m_quat.inc

.DATA
NEG_HALF   DD      -0.5


.CODE


;----------------------------------------------------------------------------
;
; void m_QuatF_set( QuatF * quat, const Euler& e );
;
;   Real cx, sx;
;   Real cy, sy;
;   Real cz, sz;
;   Real cycz, sysz, sycz, cysz;
;
;   m_sincos( -e.x * Real(0.5), &sx, &cx );
;   m_sincos( -e.y * Real(0.5), &sy, &cy );
;   m_sincos( -e.z * Real(0.5), &sz, &cz );
;
;   // Qyaw(z)   = [ (0, 0, sin z/2), cos z/2 ]
;   // Qpitch(x) = [ (sin x/2, 0, 0), cos x/2 ]
;   // Qroll(y)  = [ (0, sin y/2, 0), cos y/2 ]
;   // this = Qresult = Qyaw*Qpitch*Qroll  ZXY
;   // 
;   // The code that folows is a simplification of:
;   //    roll*=pitch;
;   //    roll*=yaw;
;   //    *this = roll;
;
;      cycz = cy*cz;   
;      sysz = sy*sz; 
;      sycz = sy*cz; 
;      cysz = cy*sz;
;      w = cycz*cx + sysz*sx;
;      x = cycz*sx + sysz*cx;
;      y = sycz*cx - cysz*sx;
;      z = cysz*cx - sycz*sx;
;
;----------------------------------------------------------------------------

m_QuatF_set  PROC C \
;
        PUBLIC  C m_QuatF_set

        mov     edx,[esp+4]     ; quat
        mov     ecx,[esp+8]     ; euler
        mov     eax, NEG_HALF

        fld     (Point3 PTR [ecx]).x            ; x
        fmul    NEG_HALF                        ; x'
        fsincos                                 ; cx, sx

        fld     (Point3 PTR [ecx]).y            ; y, cx, sx
        fmul    NEG_HALF                        ; y', cx, sx
        fsincos                                 ; cy, sy, cx, sx

        fld     (Point3 PTR [ecx]).z            ; z, cy, sy, cx, sx
        fmul    NEG_HALF                        ; z', cy, sy, cx, sx
        fsincos                                 ; cz, sz, cy, sy, cx, sx

        fld     st                              ; cz, cz, sz, cy, sy, cx, sx
        fmul    st, st(3)                       ; cycz, cz, sz, cy, sy, cx, sx

        fxch    st(1)                           ; cz, cycz, sz, cy, sy, cx, sx
        fmul    st, st(4)                       ; sycz, cycz, sz, cy, sy, cx, sx
        fxch    st(4)                           ; sy, cycz, sz, cy, sycz, cx, sx
        fmul    st, st(2)                       ; sysz, cycz, sz, cy, sycz, cx, sx
        fxch    st(2)                           ; sz, cycz, sysz, cy, sycz, cx, sx
        fmulp   st(3), st                       ; cycz, sysz, cysz, sycz, cx, sx

        ; calc W
        fld     st                              ; cycz, cycz, sysz, cysz, sycz, cx, sx
        fmul    st, st(5)                       ; cyczcx, cycz, sysz, cysz, sycz, cx, sx
        fld     st(2)                           ; sysz, cyczcx, cycz, sysz, cysz, sycz, cx, sx
        fmul    st, st(7)                       ; syszsx, cyczcx, cycz, sysz, cysz, sycz, cx, sx
        faddp   st(1), st                       ; syszsx+cyczcx, cycz, sysz, cysz, sycz, cx, sx
        fstp    (QuatF PTR [edx]).w            ; cycz, sysz, cysz, sycz, cx, sx

        ; calc X
        fmul    st, st(5)                       ; cyczsz, sysz, cysz, sycz, cx, sx
        fxch    st(1)                           ; sysz, cyczsz, cysz, sycz, cx, sx
        fmul    st, st(4)                       ; syszcx, cyczsz, cysz, sycz, cx, sx
        faddp   st(1), st                       ; syszcx+cyczsz, cysz, sycz, cx, sx
        fstp    (QuatF PTR [edx]).x            ; cysz, sycz, cx, sx

        ; calc Y
        fld     st(1)                           ; sycz, cysz, sycz, cx, sx
        fmul    st, st(3)                       ; syczcx, cysz, sycz, cx, sx
        fld     st(1)                           ; cysz, syczcx, cysz, sycz, cx, sx
        fmul    st, st(5)                       ; cyszsx, syczcx, cysz, sycz, cx, sx
        fsubp   st(1), st                       ; syczcx-cyszsx, cysz, sycz, cx, sx
        fstp    (QuatF PTR [edx]).y            ; cysz, sycz, cx, sx

        ; calc Z
        fmulp   st(2), st                       ; sycz, cyszcx, sx
        fmulp   st(2), st                       ; cyszcx, syczsx
        fsubrp  st(1), st                       ; cyszcx-syczsx
        fstp    (QuatF PTR [edx]).z            ;

        RET
m_QuatF_set ENDP




end
