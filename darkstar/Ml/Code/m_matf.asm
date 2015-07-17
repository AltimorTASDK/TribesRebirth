;
; $Workfile:   m_matf.asm  $
; $Revision:   1.7  $
; $Version: 1.6.2 $
; $Date:   14 Dec 1995 19:01:38  $
; $Log:   R:\darkstar\develop\core\ml\vcs\m_matf.asv  $
;


INCLUDE m_mat.inc
.DATA

FLOAT_ONE       equ     1065353216

FLOAT_2PI       DD      6.283185307

AS_INTEGER_ONE  EQU     3f800000h

.CODE

;----------------------------------------------------------------------------
;
; RealF m_Point3F_len( const Point3 * src );
;
;
; src: ( x, y, z )
;
; return = sqrt(x*x + y*y + z*z)
;----------------------------------------------------------------------------

m_Point3F_len  PROC C \
        ;USES    ,
        ;ARG     src: PTR Point3
        DLL_EXPORT  C m_Point3F_len

        mov     eax,[esp+4]     ; src

        fld     (Point3 PTR [eax]).x
        fmul    (Point3 PTR [eax]).x
        fld     (Point3 PTR [eax]).y
        fmul    (Point3 PTR [eax]).y
        fld     (Point3 PTR [eax]).z
        fmul    (Point3 PTR [eax]).z

        fxch
        faddp   st(2),st
        faddp   st(1),st
        fsqrt

        RET
m_Point3F_len  ENDP

;----------------------------------------------------------------------------
;
; RealF m_Point3F_lenf( const Point3 * src );
;
;
; src: ( x, y, z )
;
; return =  sqrt(x*x + y*y + z*z)  (approximation)
;
; Uses algorithm from article by Jim Blinn titled 'Floating-Point Tricks'
; IEEE Computer Graphics and Applications July/August 1997
; p80-84
;
; 6.5% MAX error using this approximation
;----------------------------------------------------------------------------

m_Point3F_lenf  PROC C \
        ;USES    ,
        ;ARG     src: PTR Point3
        DLL_EXPORT  C m_Point3F_lenf

        mov     eax,[esp+4]     ; src

        fld     (Point3 PTR [eax]).x
        fmul    (Point3 PTR [eax]).x
        fld     (Point3 PTR [eax]).y
        fmul    (Point3 PTR [eax]).y
        fld     (Point3 PTR [eax]).z
        fmul    (Point3 PTR [eax]).z

        fxch
        faddp   st(2),st
        faddp   st(1),st
        
        fstp    dword ptr [esp+4]         ; clobber the point pointer
        mov     eax, dword ptr [esp+4]
        shr     eax, 1
        add     eax, AS_INTEGER_ONE SHR 1 
        mov     dword ptr [esp+4], eax
        fld     dword ptr [esp+4]

        RET
m_Point3F_lenf  ENDP


;----------------------------------------------------------------------------
;
; bool m_Point3F_normalizef( const Point3 * src );
;
;
; src: ( x, y, z )
;
; return =  src/src.lenf() sqrt(x*x + y*y + z*z)  (approximation)
;
; Uses algorithm from article by Jim Blinn titled 'Floating-Point Tricks'
; IEEE Computer Graphics and Applications July/August 1997
; p80-84
;
; 9% MAX error using this approximation
;----------------------------------------------------------------------------

m_Point3F_normalizef  PROC C \
        ;USES    ,
        ;ARG     src: PTR Point3
        DLL_EXPORT  C m_Point3F_normalizef

        mov     eax,[esp+4]     ; src

        fld     (Point3 PTR [eax]).x         
        fmul    (Point3 PTR [eax]).x         ; xx
        fld     (Point3 PTR [eax]).y
        fmul    (Point3 PTR [eax]).y         ; yy, xx
        fld     (Point3 PTR [eax]).z
        fmul    (Point3 PTR [eax]).z         ; zz, yy, xx

        fxch
        faddp   st(2),st
        faddp   st(1),st                     ; (xx+yy+zz)
        
        fstp    dword PTR [esp+4]         
        mov     ecx, (AS_INTEGER_ONE + (AS_INTEGER_ONE SHR 1))
        mov     edx, dword PTR [esp+4]
        shr     edx, 1
        sub     ecx, edx
        mov     dword PTR [esp+4], ecx    
        fld     dword PTR [esp+4]            ; (1/len)
        fld     (Point3 PTR [eax]).x         ; x, (1/len)
        fmul    st, st(1)
        fstp    (Point3 PTR [eax]).x         
        fld     (Point3 PTR [eax]).y         ; x, (1/len)
        fmul    st, st(1)
        fstp    (Point3 PTR [eax]).y         
        fmul    (Point3 PTR [eax]).z
        fstp    (Point3 PTR [eax]).z         

        RET
m_Point3F_normalizef  ENDP


;----------------------------------------------------------------------------
;
; void m_Point3F_Point3I( const Point3F * src, Point3I * dst );
;
;
; src: ( x, y, z )
; dst: ( Int32(x), Int32(y), Int32(z) )
;
;----------------------------------------------------------------------------

m_Point3F_Point3I  PROC C \
        ;USES    ,
        ;ARG     src: PTR Point3F
        ;ARG     dst: PTR Point3I
        PUBLIC  C m_Point3F_Point3I

        mov     eax,[esp+4]     ; src
        mov     ecx,[esp+8]     ; dst

        fld     (Point3 PTR [eax]).x
        fistp   (Point3 PTR [ecx]).x
        fld     (Point3 PTR [eax]).y
        fistp   (Point3 PTR [ecx]).y
        fld     (Point3 PTR [eax]).z
        fistp   (Point3 PTR [ecx]).z

        RET
m_Point3F_Point3I  ENDP

;----------------------------------------------------------------------------
;
; void m_Point3F_RMat3F_mul( const Point3 * src, const RMat3 * rmat,
;                            Point3 *dst );
;
;
; src: ( x, y )
;
; rmat:       ( a b
;               c d )
;
; dest = (x*a + y*c, x*b + y*d)
;----------------------------------------------------------------------------

m_Point2F_RMat2F_mul  PROC C \
        ;USES    ,
        ;ARG     src: PTR Point2, \
        ;        rmat: PTR RMat2, \
        ;        dst: PTR Point2
        PUBLIC  C m_Point2F_RMat2F_mul

        mov     ecx,[esp+4]     ; src
        mov     edx,[esp+8]     ; rmat
        mov     eax,[esp+12]    ; dst

        fld     (Point2 PTR [ecx]).x            ; x
        fmul    (TMat2 PTR [edx]).m2_0_0        ; x*a
        fld     (Point2 PTR [ecx]).y            ; y,x*a
        fmul    (TMat2 PTR [edx]).m2_1_0        ; y*c,x*a
        faddp   st(1),st                        ; x'
        fstp    (Point2 PTR [eax]).x

        fld     (Point2 PTR [ecx]).x            ; x
        fmul    (TMat2 PTR [edx]).m2_0_1        ; x*b
        fld     (Point2 PTR [ecx]).y            ; y,x*b
        fmul    (TMat2 PTR [edx]).m2_1_1        ; y*d,x*b
        faddp   st(1),st                        ; y'
        fstp    (Point2 PTR [eax]).x

        RET
m_Point2F_RMat2F_mul  ENDP


;----------------------------------------------------------------------------
;
; void m_Point2F_TMat2F_mul( const Point2 * src, const TMat2 * tmat,
;                            Point2 *dst );
;
;
; src: ( x, y )
;
; tmat:       ( a b
;               c d
;               e f )
;
; dest = (x*a + y*c + e, x*b + y*d + f)
;----------------------------------------------------------------------------

m_Point2F_TMat2F_mul  PROC C \
        ;USES    ,
        ;ARG     src: PTR Point2, \
        ;        tmat: PTR TMat2, \
        ;        dst: PTR Point2
        PUBLIC  C m_Point2F_TMat2F_mul

        mov     ecx,[esp+4]     ; src
        mov     edx,[esp+8]     ; tmat
        mov     eax,[esp+12]    ; dst

        fld     (Point2 PTR [ecx]).x            ; x
        fmul    (TMat2 PTR [edx]).m2_0_0        ; x*a
        fld     (Point2 PTR [ecx]).y            ; y,x*a
        fmul    (TMat2 PTR [edx]).m2_1_0        ; y*c,x*a
        fxch    st(1)                           ; x*a,y*c
        fadd    (TMat2 PTR [edx]).t2x           ; x*a+e,y*c
        faddp   st(1),st                        ; x'
        fstp    (Point2 PTR [eax]).x

        fld     (Point2 PTR [ecx]).x            ; x
        fmul    (TMat2 PTR [edx]).m2_0_1        ; x*b
        fld     (Point2 PTR [ecx]).y            ; y,x*b
        fmul    (TMat2 PTR [edx]).m2_1_1        ; y*d,x*b
        fxch    st(1)                           ; x*b,y*d
        fadd    (TMat2 PTR [edx]).t2x           ; x*b+f,y*d
        faddp   st(1),st                        ; y'
        fstp    (Point2 PTR [eax]).x

        RET
m_Point2F_TMat2F_mul  ENDP


;----------------------------------------------------------------------------
;
; void m_Point3F_RMat3F_mul( const Point3 * src, const RMat3 * rmat,
;                            Point3 *dst );
;
;
; src: ( x, y, z )
;
; rmat:       ( a b c
;               d e f
;               g h i )
;
; dest = (x*a + y*d + z*g, x*b + y*e + z*h, x*c + y*f + z*i)
;----------------------------------------------------------------------------

m_Point3F_RMat3F_mul  PROC C \
        ;USES    ,
        ;ARG     src: PTR Point3, \
        ;        rmat: PTR RMat3, \
        ;        dst: PTR Point3
        PUBLIC  C m_Point3F_RMat3F_mul

        mov     ecx,[esp+4]     ; src
        mov     edx,[esp+8]     ; rmat
        mov     eax,[esp+12]    ; dst

        fld     (Point3 PTR [ecx]).x            ; x
        fmul    (RMat3 PTR [edx]).m3_0_0        ; x*a
        fld     (Point3 PTR [ecx]).y            ; y,x*a
        fmul    (RMat3 PTR [edx]).m3_1_0        ; y*d,x*a
        fld     (Point3 PTR [ecx]).z            ; z,y*d,x*a
        fmul    (RMat3 PTR [edx]).m3_2_0        ; z*g,y*d,x*a
        fxch    st(1)                           ; y*d,z*g,x*a
        faddp   st(2),st                        ; z*g,x*a+y*d
        faddp   st(1),st                        ; x*a+y*d+z*g
        fstp    (Point3 PTR [eax]).x

        fld     (Point3 PTR [ecx]).x            ; x
        fmul    (RMat3 PTR [edx]).m3_0_1        ; x*b
        fld     (Point3 PTR [ecx]).y            ; y,x*b
        fmul    (RMat3 PTR [edx]).m3_1_1        ; y*e,x*b
        fld     (Point3 PTR [ecx]).z            ; z,y*e,x*b
        fmul    (RMat3 PTR [edx]).m3_2_1        ; z*h,y*e,x*b
        fxch    st(1)                           ; y*e,z*h,x*b
        faddp   st(2),st                        ; z*h,x*b+y*e
        faddp   st(1),st                        ; x*b+y*e+z*h
        fstp    (Point3 PTR [eax]).y

        fld     (Point3 PTR [ecx]).x            ; x
        fmul    (RMat3 PTR [edx]).m3_0_2        ; x*c
        fld     (Point3 PTR [ecx]).y            ; y,x*c
        fmul    (RMat3 PTR [edx]).m3_1_2        ; y*f,x*c
        fld     (Point3 PTR [ecx]).z            ; z,y*f,x*c
        fmul    (RMat3 PTR [edx]).m3_2_2        ; z*i,y*f,x*c
        fxch    st(1)                           ; y*f,z*i,x*c
        faddp   st(2),st                        ; z*i,x*c+y*f
        faddp   st(1),st                        ; x*c+y*f+z*i
        fstp    (Point3 PTR [eax]).z
        RET
m_Point3F_RMat3F_mul  ENDP


;----------------------------------------------------------------------------
;
; void m_Point3F_TMat3F_mul( const Point3 * src, const TMat3 * tmat,
;                            Point3 *dst );
;
;
; src: ( x, y, z )
;
; tmat:       ( a b c
;               d e f
;               g h i
;               j k l )
;
; dest = (x*a + y*d + z*g + j, x*b + y*e + z*h + k, x*c + y*f + z*i + l)
;----------------------------------------------------------------------------

m_Point3F_TMat3F_mul  PROC C \
        ;USES    ,
        ;ARG     src: PTR Point3, \
        ;        tmat: PTR TMat3, \
        ;        dst: PTR Point3
        PUBLIC  C m_Point3F_TMat3F_mul

        mov     ecx,[esp+4]     ; src
        mov     edx,[esp+8]     ; tmat
        mov     eax,[esp+12]    ; dst

        fld     (Point3 PTR [ecx]).x            ; x
        fmul    (TMat3 PTR [edx]).m3_0_0        ; x*a
        fld     (Point3 PTR [ecx]).y            ; y,x*a
        fmul    (TMat3 PTR [edx]).m3_1_0        ; y*d,x*a
        fld     (Point3 PTR [ecx]).z            ; z,y*d,x*a

        fmul    (TMat3 PTR [edx]).m3_2_0        ; z*g,y*d,x*a
        fxch    st(1)                           ; y*d,z*g,x*a

        faddp   st(2),st                        ; z*g,x*a+y*d

        fld     (Point3 PTR [ecx]).x            ; x,z*g,x*a+y*d
        fxch    st(1)                           ; z*g,x,x*a+y*d

        faddp   st(2),st                        ; x,x*a+y*d+z*g
        fmul    (TMat3 PTR [edx]).m3_0_1        ; x*b,x*a+y*d+z*g
        fld     (Point3 PTR [ecx]).y            ; y,x*b,x*a+y*d+z*g

        fmul    (TMat3 PTR [edx]).m3_1_1        ; y*e,x*b,x*a+y*d+z*g
        fxch    st(2)                           ; x*a+y*d+z*g,x*b,y*e

        fadd    (TMat3 PTR [edx]).t3x           ; x',x*b,y*e
        fld     (Point3 PTR [ecx]).z            ; z,x',x*b,y*e

        fmul    (TMat3 PTR [edx]).m3_2_1        ; z*h,x',x*b,y*e
        fxch    st(1)                           ; x',z*h,x*b,y*e

        fstp    (Point3 PTR [eax]).x            ; z*h,x*b,y*e

        fadd    (TMat3 PTR [edx]).t3y           ; z*h+k,x*b,y*e
        fxch    st(1)                           ; x*b,z*h+k,y*e
        faddp   st(2),st                        ; z*h+k,x*b+y*e
        fld     (Point3 PTR [ecx]).x            ; x, z*h+k,x*b+y*e
        fmul    (TMat3 PTR [edx]).m3_0_2        ; x*c,z*h+k,x*b+y*e
        fxch    st(1)                           ; z*h+k,x*c,x*b+y*e
        faddp   st(2),st                        ; x*c,y'
        fld     (Point3 PTR [ecx]).y            ; y,x*c,y'
        fmul    (TMat3 PTR [edx]).m3_1_2        ; y*f,x*c,y'
        fld     (Point3 PTR [ecx]).z            ; z,y*f,x*c,y'
        fmul    (TMat3 PTR [edx]).m3_2_2        ; z*i,y*f,x*c,y'
        fxch    st(3)                           ; y',y*f,x*c,z*i
        fstp    (Point3 PTR [eax]).y            ; y*f,x*c,z*i
        faddp   st(1),st                        ; x*c+y*f,z*i
        fxch    st(1)                           ; z*i,x*c+y*f
        fadd    (TMat3 PTR [edx]).t3z           ; z*i+l,x*c+y*f
        faddp   st(1),st                        ; z'
        fstp    (Point3 PTR [eax]).z            ;
        RET
m_Point3F_TMat3F_mul  ENDP


;----------------------------------------------------------------------------
;
; void m_Point3F_RMat3F_mulInverse( const Point3 * src, const RMat3 * rmat,
;                            Point3 *dst );
;
;
; src: ( x, y, z )
;
; rmat:       ( a b c
;               d e f
;               g h i )
;
; dest = (x*a + y*b + z*c, x*d + y*e + z*f, x*g + y*h + z*i)
;----------------------------------------------------------------------------

m_Point3F_RMat3F_mulInverse  PROC C \
        ;USES    ,
        ;ARG     src: PTR Point3, \
        ;        rmat: PTR RMat3, \
        ;        dst: PTR Point3
        PUBLIC  C m_Point3F_RMat3F_mulInverse

        mov     ecx,[esp+4]     ; src
        mov     edx,[esp+8]     ; rmat
        mov     eax,[esp+12]    ; dst

        fld     (Point3 PTR [ecx]).x            ; x
        fmul    (RMat3 PTR [edx]).m3_0_0        ; x*a
        fld     (Point3 PTR [ecx]).y            ; y,x*a
        fmul    (RMat3 PTR [edx]).m3_0_1        ; y*b,x*a
        fld     (Point3 PTR [ecx]).z            ; z,y*b,x*a
        fmul    (RMat3 PTR [edx]).m3_0_2        ; z*c,y*b,x*a
        fxch    st(1)                           ; y*b,z*c,x*a
        faddp   st(2),st                        ; z*c,x*a+y*b
        faddp   st(1),st                        ; x*a+y*b+z*c
        fstp    (Point3 PTR [eax]).x

        fld     (Point3 PTR [ecx]).x            ; x
        fmul    (RMat3 PTR [edx]).m3_1_0        ; x*d
        fld     (Point3 PTR [ecx]).y            ; y,x*d
        fmul    (RMat3 PTR [edx]).m3_1_1        ; y*e,x*d
        fld     (Point3 PTR [ecx]).z            ; z,y*e,x*d
        fmul    (RMat3 PTR [edx]).m3_1_2        ; z*f,y*e,x*d
        fxch    st(1)                           ; y*e,z*f,x*d
        faddp   st(2),st                        ; z*f,x*d+y*e
        faddp   st(1),st                        ; x*d+y*e+z*f
        fstp    (Point3 PTR [eax]).y

        fld     (Point3 PTR [ecx]).x            ; x
        fmul    (RMat3 PTR [edx]).m3_2_0        ; x*g
        fld     (Point3 PTR [ecx]).y            ; y,x*g
        fmul    (RMat3 PTR [edx]).m3_2_1        ; y*h,x*g
        fld     (Point3 PTR [ecx]).z            ; z,y*h,x*g
        fmul    (RMat3 PTR [edx]).m3_2_2        ; z*i,y*h,x*g
        fxch    st(1)                           ; y*h,z*i,x*g
        faddp   st(2),st                        ; z*i,x*g+y*h
        faddp   st(1),st                        ; x*g+y*h+z*i
        fstp    (Point3 PTR [eax]).z
        RET
m_Point3F_RMat3F_mulInverse  ENDP


;----------------------------------------------------------------------------
;
; void m_Point3F_TMat3F_mulInverse( const Point3 * src, const TMat3 * tmat,
;                            Point3 *dst );
;
;
; src: ( x, y, z )
;
; tmat:       ( a b c
;               d e f
;               g h i
;               j k l )
;
; with u=x-j, v=y-k, w=z-l
;
; dest = (u*a + v*b + w*c, u*d + v*e + w*f, u*g + v*h + w*i)
;----------------------------------------------------------------------------

m_Point3F_TMat3F_mulInverse  PROC C \
        ;USES    ,
        ;ARG     src: PTR Point3, \
        ;        tmat: PTR TMat3, \
        ;        dst: PTR Point3
        PUBLIC  C m_Point3F_TMat3F_mulInverse

        mov     ecx,[esp+4]     ; src
        mov     edx,[esp+8]     ; tmat
        mov     eax,[esp+12]    ; dst

        fld     (Point3 PTR [ecx]).x            ; x
		fsub	(TMat3 PTR [edx]).t3x			; u
        fld     (Point3 PTR [ecx]).y            ; y,u
		fsub	(TMat3 PTR [edx]).t3y			; v,u
        fld     (Point3 PTR [ecx]).z            ; z,v,u
		fsub	(TMat3 PTR [edx]).t3z			; w,v,u

		fld		st(2)							; u,w,v,u
        fmul    (TMat3 PTR [edx]).m3_0_0        ; u*a,w,v,u
		fld		st(2)							; v,u*a,w,v,u
        fmul    (TMat3 PTR [edx]).m3_0_1        ; v*b,u*a,w,v,u
		fld		st(2)							; w,v*b,u*a,w,v,u
        fmul    (TMat3 PTR [edx]).m3_0_2        ; w*c,v*b,u*a,w,v,u
		fxch	st(1)							; v*b,w*c,u*a,w,v,u
		faddp	st(2),st						; w*c,u*a+v*b,w,v,u

		fld		st(2)							; w,w*c,u*a+v*b,w,v,u
        fmul    (TMat3 PTR [edx]).m3_1_2        ; w*f,w*c,u*a+v*b,w,v,u
		fxch	st(1)							; w*c,w*f,u*a+v*b,w,v,u
		faddp	st(2),st						; w*f,u*a+v*b+w*c,w,v,u
		fld		st(3)							; v,w*f,u*a+v*b+w*c,w,v,u
        fmul    (TMat3 PTR [edx]).m3_1_1        ; v*e,w*f,u*a+v*b+w*c,w,v,u
		fxch	st(2)							; u*a+v*b+w*c,w*f,v*e,w,v,u
        fstp    (Point3 PTR [eax]).x            ; w*f,v*e,w,v,u

		fld		st(4)							; u,w*f,v*e,w,v,u
        fmul    (TMat3 PTR [edx]).m3_1_0        ; u*d,w*f,v*e,w,v,u
		fxch	st(1)							; w*f,u*d,v*e,w,v,u
		faddp	st(2),st						; u*d,v*e+w*f,w,v,u
		fxch	st(2)							; w,v*e+w*f,u*d,v,u
        fmul    (TMat3 PTR [edx]).m3_2_2        ; w*i,v*e+w*f,u*d,v,u

		fxch	st(1)							; v*e+w*f,w*i,u*d,v,u
		faddp	st(2),st						; w*i,u*d+v*e+w*f,v,u

		fxch	st(2)							; v,u*d+v*e+w*f,w*i,u
        fmul    (TMat3 PTR [edx]).m3_2_1        ; v*h,u*d+v*e+w*f,w*i,u

		fxch	st(1)							; u*d+v*e+w*f,v*h,w*i,u
        fstp    (Point3 PTR [eax]).y            ; v*h,w*i,u

		fxch	st(2)							; u,w*i,v*h
        fmul    (TMat3 PTR [edx]).m3_2_0        ; u*g,w*i,v*h
		fxch	st(1)							; w*i,u*g,v*h
		faddp	st(2),st						; u*g,v*h+w*i
		faddp	st(1),st						; u*g+v*h+w*i
        fstp    (Point3 PTR [eax]).z            ;		f
        RET
m_Point3F_TMat3F_mulInverse  ENDP


;----------------------------------------------------------------------------
;
; void m_TMat3F_TMat3F_mul( const TMat3 * tmat1, const TMat3 * tmat2,
;                            TMat3 *dst );
;
;
; tmat1:       ( a1 b1 c1
;                d1 e1 f1
;                g1 h1 i1
;                j1 k1 l1 )
;
; tmat2:       ( a2 b2 c2
;                d2 e2 f2
;                g2 h2 i2
;                j2 k2 l2 )
;
;
; dest:        ( a' b' c'
;                d' e' f'
;                g' h' i'
;                j' k' l' )
;
; a' = a1 * a2 + b1 * d2 + c1 * g2
; b' = a1 * b2 + b1 * e2 + c1 * h2
; c' = a1 * c2 + b1 * f2 + c1 * i2
; d' = d1 * a2 + e1 * d2 + f1 * g2
; e' = d1 * b2 + e1 * e2 + f1 * h2
; f' = d1 * c2 + e1 * f2 + f1 * i2
; g' = g1 * a2 + h1 * d2 + i1 * g2
; h' = g1 * b2 + h1 * e2 + i1 * h2
; i' = g1 * c2 + h1 * f2 + i1 * i2
;
; j' = j1 * a2 + k1 * d2 + l1 * g2 + j2
; k' = j1 * b2 + k1 * e2 + l1 * h2 + k2
; l' = j1 * c2 + k1 * f2 + l1 * i2 + l2
;
; dest.flags = tmat1.flags | tmat2.flags
;
;----------------------------------------------------------------------------

m_TMat3F_TMat3F_mul  PROC C \
        ;USES    ,
        ;ARG     tmat1: PTR TMat3, \
        ;        tmat2: PTR TMat3, \
        ;        dst: PTR TMat3
        PUBLIC  C m_TMat3F_TMat3F_mul

        mov     ecx,[esp+4]     ; tmat1
        mov     edx,[esp+8]     ; tmat2
        mov     eax,[esp+12]    ; dst

		fld		(TMat3 PTR [ecx]).m3_0_0		; a1
		fmul	(TMat3 PTR [edx]).m3_0_0		; a1 * a2
		fld		(TMat3 PTR [ecx]).m3_0_1		; b1, a1 * a2
		fmul	(TMat3 PTR [edx]).m3_1_0		; b1 * d2, a1 * a2
		fld		(TMat3 PTR [ecx]).m3_0_2		; c1, b1 * d2, a1 * a2
		fmul	(TMat3 PTR [edx]).m3_2_0		; c1 * g2, b1 * d2, a1 * a2
		fxch	st(1)           				; b1 * d2, c1 * g2, a1 * a2
		faddp	st(2),st        				; c1 * g2, a1 * a2 + b1 * d2
		fld		(TMat3 PTR [ecx]).m3_0_0		; a1, c1 * g2, a1 * a2 + b1 * d2
		fmul	(TMat3 PTR [edx]).m3_0_1		; a1 * b2, c1 * g2, a1 * a2 + b1 * d2
		fxch	st(1)							; c1 * g2, a1 * b2, a1 * a2 + b1 * d2
		faddp	st(2),st						; a1 * b2, a'
		fld		(TMat3 PTR [ecx]).m3_0_1		; b1, a1 * b2, a'
		fmul	(TMat3 PTR [edx]).m3_1_1		; b1 * e2, a1 * b2, a' 
		fxch	st(2)							; a', b1 * e2, a1 * b2
		fstp    (TMat3 PTR [eax]).m3_0_0		; b1 * e2, a1 * b2

		faddp	st(1),st						; a1 * b2 + b1 * e2
		fld		(TMat3 PTR [ecx]).m3_0_2		; c1, a1 * b2 + b1 * e2
		fmul	(TMat3 PTR [edx]).m3_2_1		; c1 * h2, a1 * b2 + b1 * e2
		fld		(TMat3 PTR [ecx]).m3_0_0		; a1, c1 * h2, a1 * b2 + b1 * e2
		fmul	(TMat3 PTR [edx]).m3_0_2		; a1 * c2, c1 * h2, a1 * b2 + b1 * e2
		fxch    st(1)							; c1 * h2, a1 * c2, a1 * b2 + b1 * e2
		faddp	st(2),st						; a1 * c2, b'
		fld		(TMat3 PTR [ecx]).m3_0_1		; b1, a1 * c2, b'
		fmul	(TMat3 PTR [edx]).m3_1_2		; b1 * f2, a1 * c2, b'
		fxch	st(2)							; b', a1 * c2, b1 * f2
		fstp	(TMat3 PTR [eax]).m3_0_1		; a1 * c2, b1 * f2

		faddp	st(1),st						; a1 * c2 + b1 * f2
		fld		(TMat3 PTR [ecx]).m3_0_2		; c1, a1 * c2 + b1 * f2
		fmul	(TMat3 PTR [edx]).m3_2_2		; c1 * i2, a1 * c2 + b1 * f2
		fld		(TMat3 PTR [ecx]).m3_1_0		; d1, c1 * i2, a1 * c2 + b1 * f2
		fmul	(TMat3 PTR [edx]).m3_0_0		; d1 * a2, c1 * i2, a1 * c2 + b1 * f2
		fxch	st(1)							; c1 * i2, d1 * a2, a1 * c2 + b1 * f2
		faddp	st(2),st						; d1 * a2, c'
		fld		(TMat3 PTR [ecx]).m3_1_1		; e1, d1 * a2, c'
		fmul	(TMat3 PTR [edx]).m3_1_0		; e1 * d2, d1 * a2, c'
		fxch	st(2)							; c', e1 * d2, d1 * a2
		fstp	(TMat3 PTR [eax]).m3_0_2		; e1 * d2, d1 * a2

		faddp	st(1),st						; d1 * a2 + e1 * d2
		fld		(TMat3 PTR [ecx]).m3_1_2		; f1, d1 * a2 + e1 * d2
		fmul	(TMat3 PTR [edx]).m3_2_0		; f1 * g2, d1 * a2 + e1 * d2
		fld		(TMat3 PTR [ecx]).m3_1_0		; d1, f1 * g2, d1 * a2 + e1 * d2
		fmul	(TMat3 PTR [edx]).m3_0_1		; d1 * b2, f1 * g2, d1 * a2 + e1 * d2
		fxch	st(1)							; f1 * g2, d1 * b2, d1 * a2 + e1 * d2
		faddp	st(2),st						; f1 * g2, d'
		fld		(TMat3 PTR [ecx]).m3_1_1		; e1, f1 * g2, d'
		fmul	(TMat3 PTR [edx]).m3_1_1		; e1 * e2, f1 * g2, d'
		fxch	st(2)							; d', f1 * g2, e1 * e2
		fstp	(TMat3 PTR [eax]).m3_1_0		; e1 * d2, d1 * b2

		faddp	st(1),st						; d1 * b2 + e1 * d2
		fld		(TMat3 PTR [ecx]).m3_1_2		; f1, d1 * b2 + e1 * d2
		fmul	(TMat3 PTR [edx]).m3_2_1		; f1 * h2, d1 * b2 + e1 * d2
		fld		(TMat3 PTR [ecx]).m3_1_0		; d1, f1 * h2, d1 * b2 + e1 * d2
		fmul	(TMat3 PTR [edx]).m3_0_2		; d1 * c2, f1 * h2, d1 * b2 + e1 * d2
		fxch	st(1)							; f1 * h2, d1 * c2, d1 * b2 + e1 * d2
		faddp	st(2),st						; d1 * c2, e'
		fld		(TMat3 PTR [ecx]).m3_1_1		; e1, d1 * c2, e'
		fmul	(TMat3 PTR [edx]).m3_1_2		; e1 * f2, d1 * c2, e'
		fxch	st(2)							; e', e1 * f2, d1 * c2
		fstp	(TMat3 PTR [eax]).m3_1_1		; e1 * f2, d1 * c2

		faddp	st(1),st						; d1 * c2 + e1 * f2
		fld		(TMat3 PTR [ecx]).m3_1_2		; f1, d1 * c2 + e1 * f2
		fmul	(TMat3 PTR [edx]).m3_2_2		; f1 * i2, d1 * c2 + e1 * f2
		fld		(TMat3 PTR [ecx]).m3_2_0		; g1, f1 * i2, d1 * c2 + e1 * f2
		fmul	(TMat3 PTR [edx]).m3_0_0		; g1 * a2, f1 * i2, d1 * c2 + e1 * f2
		fxch	st(1)							; f1 * i2, g1 * a2, d1 * c2 + e1 * f2
		faddp	st(2),st						; g1 * a2, f'
		fld		(TMat3 PTR [ecx]).m3_2_1		; h1, g1 * a2, f'
		fmul	(TMat3 PTR [edx]).m3_1_0		; h1 * d2, g1 * a2, f'
		fxch	st(2)							; f', h1 * d2, g1 * a2
		fstp	(TMat3 PTR [eax]).m3_1_2		; h1 * d2, g1 * a2

		faddp	st(1),st						; g1 * a2 + h1 * d2
		fld		(TMat3 PTR [ecx]).m3_2_2		; i1, g1 * a2 + h1 * d2
		fmul	(TMat3 PTR [edx]).m3_2_0		; i1 * g2, g1 * a2 + h1 * d2
		fld		(TMat3 PTR [ecx]).m3_2_0		; g1, i1 * g2, g1 * a2 + h1 * d2
		fmul	(TMat3 PTR [edx]).m3_0_1		; g1 * b2, i1 * g2, g1 * a2 + h1 * d2
		fxch	st(1)							; i1 * g2, g1 * b2, g1 * a2 + h1 * d2
		faddp	st(2),st						; g1 * b2, g'
		fld		(TMat3 PTR [ecx]).m3_2_1		; h1, g1 * b2, g'
		fmul	(TMat3 PTR [edx]).m3_1_1		; h1 * e2, g1 * b2, g'
		fxch	st(2)							; g', h1 * e2, g1 * b2
		fstp	(TMat3 PTR [eax]).m3_2_0		; h1 * e2, g1 * b2

		faddp	st(1),st						; g1 * b2 + h1 * e2
		fld		(TMat3 PTR [ecx]).m3_2_2		; i1, g1 * b2 + h1 * e2
		fmul	(TMat3 PTR [edx]).m3_2_1		; i1 * h2, g1 * b2 + h1 * e2
		fld		(TMat3 PTR [ecx]).m3_2_0		; g1, i1 * h2, g1 * b2 + h1 * e2
		fmul	(TMat3 PTR [edx]).m3_0_2		; g1 * c2, i1 * h2, g1 * b2 + h1 * e2
		fxch	st(1)							; i1 * h2, g1 * c2, g1 * b2 + h1 * e2
		faddp	st(2),st						; g1 * c2, h'
		fld		(TMat3 PTR [ecx]).m3_2_1		; h1, g1 * c2, h'
		fmul	(TMat3 PTR [edx]).m3_1_2		; h1 * f2, g1 * c2, h'
		fxch	st(2)							; h', h1 * f2, g1 * c2
		fstp	(TMat3 PTR [eax]).m3_2_1		; h1 * f2, g1 * c2

		faddp	st(1),st						; g1 * c2 + h1 * f2
		fld		(TMat3 PTR [ecx]).m3_2_2		; i1, g1 * c2 + h1 * f2
		fmul	(TMat3 PTR [edx]).m3_2_2		; i1 * i2, g1 * c2 + h1 * f2
		fld		(TMat3 PTR [ecx]).t3x			; j1, i1 * i2, g1 * c2 + h1 * f2
		fmul	(TMat3 PTR [edx]).m3_0_0		; j1 * a2, i1 * i2, g1 * c2 + h1 * f2
		fxch	st(1)							; i1 * i2, j1 * a2, g1 * c2 + h1 * f2
		faddp	st(2),st						; j1 * a2, i'
		fld		(TMat3 PTR [ecx]).t3y			; k1, j1 * a2, i'
		fmul	(TMat3 PTR [edx]).m3_1_0		; k1 * d2, j1 * a2, i'
		fxch	st(2)							; i', k1 * d2, j1 * a2
		fstp	(TMat3 PTR [eax]).m3_2_2		; k1 * d2, j1 * a2

		faddp	st(1),st						; j1 * a2 + k1 * d2
		fld		(TMat3 PTR [ecx]).t3z			; l1, j1 * a2 + k1 * d2
		fmul	(TMat3 PTR [edx]).m3_2_0		; l1 * g2, j1 * a2 + k1 * d2
		fxch	st(1)							; j1 * a2 + k1 * d2, l1 * g2
		fadd	(TMat3 PTR [edx]).t3x			; j1 * a2 + k1 * d2 + j2, l1 * g2
		fld		(TMat3 PTR [ecx]).t3x			; j1, j1 * a2 + k1 * d2 + j2, l1 * g2
		fmul	(TMat3 PTR [edx]).m3_0_1		; j1 * b2, j1 * a2 + k1 * d2 + j2, l1 * g2
		fxch	st(1)							; j1 * a2 + k1 * d2 + j2, j1 * b2, l1 * g2
		faddp	st(2),st						; j1 * b2, j'
		fld		(TMat3 PTR [ecx]).t3y			; k1, j1 * b2, j'
		fmul	(TMat3 PTR [edx]).m3_1_1		; k1 * e2, j1 * b2, j'
		fxch	st(2)							; j', k1 * e2, j1 * b2
		fstp	(TMat3 PTR [eax]).t3x			; k1 * e2, j1 * b2

		faddp	st(1),st						; j1 * b2 + k1 * e2
		fld		(TMat3 PTR [ecx]).t3z			; l1, j1 * b2 + k1 * e2
		fmul	(TMat3 PTR [edx]).m3_2_1		; l1 * h2, j1 * b2 + k1 * e2
		fxch	st(1)							; j1 * b2 + k1 * e2, l1 * h2
		fadd	(TMat3 PTR [edx]).t3y			; j1 * b2 + k1 * e2 + k2, l1 * h2
		fld		(TMat3 PTR [ecx]).t3x			; j1, j1 * b2 + k1 * e2 + k2, l1 * h2
		fmul	(TMat3 PTR [edx]).m3_0_2		; j1 * c2, j1 * b2 + k1 * e2 + k2, l1 * h2
		fxch	st(1)							; j1 * b2 + k1 * e2 + k2, j1 * c2, l1 * h2
		faddp	st(2),st						; j1 * c2, k'
		fld		(TMat3 PTR [ecx]).t3y			; k1, j1 * c2, k'
		fmul	(TMat3 PTR [edx]).m3_1_2		; k1 * f2, j1 * c2, k'
		fxch	st(2)							; k', k1 * f2, j1 * c2
		fstp	(TMat3 PTR [eax]).t3y			; k1 * f2, j1 * c2

		faddp	st(1),st						; j1 * c2 + k1 * f2
		fld		(TMat3 PTR [ecx]).t3z			; l1, j1 * c2 + k1 * f2
		fmul	(TMat3 PTR [edx]).m3_2_2		; l1 * i2, j1 * c2 + k1 * f2
		fxch	st(1)							; j1 * c2 + k1 * f2, l1 * i2
		fadd	(TMat3 PTR [edx]).t3z			; j1 * c2 + k1 * f2 + l2, l1 * i2
		mov		ecx,(TMat3 PTR [ecx]).flags
		faddp	st(1),st						; l'
		or		ecx,(TMat3 PTR [edx]).flags
		fstp	(TMat3 PTR [eax]).t3z			;
		mov 	(TMat3 PTR [eax]).flags, ecx
        RET
m_TMat3F_TMat3F_mul  ENDP


;----------------------------------------------------------------------------
;
; void m_RMat3F_set( RMat3 * rmat, const Euler& e );
;
;----------------------------------------------------------------------------
m_RMat3F_set  PROC C \
        ;USES    ,
        ;ARG     rmat: PTR RMat3, \
        ;        euler: PTR Point3
        PUBLIC  C m_RMat3F_set

        mov     edx,[esp+4]     ; rmat
        mov     ecx,[esp+8]     ; euler

        fld     (Point3 PTR [ecx]).x
        ftst
        fstsw   ax
        sahf
        fld     (Point3 PTR [ecx]).y
        jnz     @3axis

        ftst
        fstsw   ax
        sahf
        fld     (Point3 PTR [ecx]).z
        jnz     @2axis

        ; get rid of x,y
        ffree   st(2)
        ffree   st(1)

        ftst
        fstsw   ax
        sahf
        jnz     @1axis
@0axis:
   ; the matrix looks like this:
   ;  1        0        0
   ;  0        1        0
   ;  0        0        1
   ;
   ; num_axis = 0
        ; get rid of z, get 1.0
        ffree   st(0)
        fld1

        sub     eax,eax

        fst     (RMat3 PTR [edx]).m3_0_0
        mov     (RMat3 PTR [edx]).m3_0_1,eax
        mov     (RMat3 PTR [edx]).m3_1_0,eax
        fst     (RMat3 PTR [edx]).m3_1_1
        mov     (RMat3 PTR [edx]).m3_0_2,eax
        mov     (RMat3 PTR [edx]).m3_1_2,eax
        mov     (RMat3 PTR [edx]).m3_2_0,eax
        mov     (RMat3 PTR [edx]).m3_2_1,eax
        fstp    (RMat3 PTR [edx]).m3_2_2

	mov	eax,(RMat3 PTR [edx]).flags
	and	eax,NOT (Matrix_HasScale OR Matrix_HasRotation)
	mov	(RMat3 PTR [edx]).flags,eax

        jmp     @done

@1axis:

   ; the matrix looks like this:
   ;  cos(z)   sin(z)   0
   ;  -sin(z)  cos(z)   0
   ;  0        0        1
   ;
   ; num_axis = 1
        fsincos
        sub     eax,eax
        mov     (RMat3 PTR [edx]).m3_0_2,eax
        mov     (RMat3 PTR [edx]).m3_1_2,eax
        mov     (RMat3 PTR [edx]).m3_2_0,eax
        mov     (RMat3 PTR [edx]).m3_2_1,eax
        mov     eax,FLOAT_ONE
        mov     (RMat3 PTR [edx]).m3_2_2,eax
        fst     (RMat3 PTR [edx]).m3_0_0     ; cos(z)
        fxch
        fst     (RMat3 PTR [edx]).m3_0_1     ; sin(z)
        fchs
        fxch
        fstp    (RMat3 PTR [edx]).m3_1_1    ; cos(z)
        fstp    (RMat3 PTR [edx]).m3_1_0    ; -sin(z)

	mov	eax,(RMat3 PTR [edx]).flags
	or	eax,Matrix_HasRotation
	and	eax,NOT (Matrix_HasScale)
	mov	(RMat3 PTR [edx]).flags,eax

        jmp     @done

@3axis:
        fld     (Point3 PTR [ecx]).z
@2axis:

   ; the matrix looks like this:
   ;  r1 - (r4 * sin(x))     r2 + (r3 * sin(x))   -cos(x) * sin(y)
   ;  -cos(x) * sin(z)       cos(x) * cos(z)      sin(x)
   ;  r3 + (r2 * sin(x))     r4 - (r1 * sin(x))   cos(x) * cos(y)
   ;
   ; where:
   ;  r1 = cos(y) * cos(z)
   ;  r2 = cos(y) * sin(z)
   ;  r3 = sin(y) * cos(z)
   ;  r4 = sin(y) * sin(z)
   ;
   ; num_axis = 3

        fsincos                                 ; cz, sz, y, x
        fxch    st(2)                           ; y, sz, cz, x
        fsincos                                 ; cy, sy, sz, cz, x
        fxch    st(4)                           ; x, sy, sz, cz, cy
        fsincos                                 ; cx, sx, sy, sz, cz, cy

        fld     st(5)                           ; cy, ...
        fmul    st,st(1)                        ; cx*cy, cx, sx, sy, sz, ...

        fld     st(5)                           ; cz, m3_2_2, cx, ...
        fmul    st,st(2)                        ; cx*cz, m3_2_2, ...
        fxch    st(1)                           ; m3_2_2, m3_1_1, ...
        fstp    (RMat3 PTR [edx]).m3_2_2
        fstp    (RMat3 PTR [edx]).m3_1_1

        fchs                                    ; -cx, sx, sy, sz, cz, cy
        fld     st(2)                           ; sy, -cx, ...
        fmul    st, st(1)                       ; -cx*sy, ...
        fxch    st(1)                           ; -cx, m3_0_2, sx, sy, sz, ...

        fmul    st, st(4)                       ; -cx*sz, m3_0_2, sx, sy, ...
        fxch    st(1)                           ; m3_0_2, m3_1_0, sx, sy, ...
        fstp    (RMat3 PTR [edx]).m3_0_2        ; m3_1_0, sx, sy, sz, ...
        fstp    (RMat3 PTR [edx]).m3_1_0        ; sx, sy, sz, cz, cy

        fxch    st(4)                           ; cy, sy, sz, cz, sx

        fld     st(0)                           ; cy, cy, ...
        fmul    st, st(3)                       ; cy*sz, cy, sy, sz, cz, sx
        fxch    st(1)                           ; cy, cy*sz, sy, sz, cz, sx
        fmul    st, st(4)                       ; cy*cz, cy*sz, sy, sz, cz, sx
        fxch    st(2)                           ; sy, cy*sz, cy*cz, sz, cz, sx
        fmul    st(3), st
        fmulp   st(4), st                       ; cy*sz, cy*cz, sy*sz, sy*cz, sx

        fxch    st(4)                           ; sx, cy*cz, sy*sz, sy*cz, cy*sz
        fst     (RMat3 PTR [edx]).m3_1_2        ; sx, cy*cz, sy*sz, sy*cz, cy*sz
        fld     st(2)                           ; sy*sz, sx, cy*cz, sy*sz, sy*cz, cy*sz
        fld     st(2)                           ; cy*cz, sy*sz, sx, cy*cz, sy*sz, sy*cz, cy*sz
        fld     st(2)                           ; sx, cy*cz, sy*sz, sx, cy*cz, sy*sz, ...

        fmulp   st(1),st                        ; sx*cy*cz, sy*sz, sx, cy*cz, sy*sz, ...
        fld     st(2)                           ; sx, sx*cy*cz, sy*sz, sx, cy*cz, sy*sz, ...
        fxch    st(3)                           ; sx, sx*cy*cz, sy*sz, sx, cy*cz, sy*sz, ...
        fmulp   st(2),st                        ; sx*cy*cz, sx*sy*sz, sx, cy*cz, sy*sz, ...
        fsubp   st(4),st                        ; sx*sy*sz, sx, cy*cz, m3_2_1, ...
        fsubp   st(2),st                        ; sx, m3_0_0, m3_2_1, sy*cz, cy*sz

        fxch    st(2)                           ; m3_2_1, m3_0_0, sx, sy*cz, cy*sz
        fstp    (RMat3 PTR [edx]).m3_2_1        ; m3_0_0, sx, sy*cz, cy*sz
        fstp    (RMat3 PTR [edx]).m3_0_0        ; sx, sy*cz, cy*sz

        fld     st(2)                           ; cy*sz, sx, sy*cz, cy*sz
        fld     st(2)                           ; sy*cz, cy*sz, sx, sy*cz, cy*sz
        fld     st(2)                           ; sx, sy*cz, cy*sz, sx, sy*cz, cy*sz
        fxch    st(1)                           ; sy*cz, sx, cy*sz, sx, sy*cz, cy*sz
        fmulp   st(3),st                        ; sx, cy*sz, sx*sy*cz, sy*cz, cy*sz
        fmulp   st(1),st                        ; sx*cy*sz, sx*sy*cz, sy*cz, cy*sz
        fxch    st(1)                           ; sx*sy*cz, sx*cy*sz, sy*cz, cy*sz
        faddp   st(3),st                        ; sx*cy*sz, sy*cz, m3_0_1
        faddp   st(1),st                        ; m3_2_0, m3_0_1

        fxch    st(1)                           ; m3_0_1, m3_2_0
        fstp    (RMat3 PTR [edx]).m3_0_1        ; m3_2_0
        fstp    (RMat3 PTR [edx]).m3_2_0

	mov	eax,(RMat3 PTR [edx]).flags
	or	eax,Matrix_HasRotation
	and	eax, NOT (Matrix_HasScale)
	mov	(RMat3 PTR [edx]).flags,eax

        jmp     @done

@done:
        RET
m_RMat3F_set  ENDP

end
