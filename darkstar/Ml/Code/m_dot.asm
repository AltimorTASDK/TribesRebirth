;
; $Workfile:   m_dot.asm  $
; $Revision:   1.4  $
; $Version$
; $Date:   14 Dec 1995 18:59:06  $
; $Log:   R:\darkstar\develop\core\ml\vcs\m_dot.asv  $
;	
;	   Rev 1.4   14 Dec 1995 18:59:06   CAEDMONI
;	converted to fastcall
;	
;	   Rev 1.3   10 Nov 1995 11:35:22   CAEDMONI
;	added new version of m_dotSign(Point3I,Point3I,Int32) with tolerance
;	
;	   Rev 1.2   05 Apr 1995 11:35:44   RICKO
;	Integrate DarkStar Changes
;	
;	   Rev 1.1   08 Mar 1995 08:43:54   RICKO
;	Now compiles with both TASM and MASM
;	
;	   Rev 1.0   07 Mar 1995 13:16:42   NANCYH
;	Initial revision.
;	



INCLUDE m_vec.inc

.CODE


;Int32 m_dotSign2( const Point2& a, const Point2& b );
;{
;	return (a.x*b.x + a.y*b.y + a.z*b.z) > 0 ? 1 : -1;
;}

m_dotSign2 PROC C \
	USES	ebx esi edi,
	ARG	a: PTR Point2, \
                b: PTR Point2
	PUBLIC	C m_dotSign2

	mov	esi,a
	mov	edi,b

	mov	eax,(Point2 Ptr [esi]).x
	imul	(Point2 Ptr [edi]).x
	mov	ecx,eax
	mov	ebx,edx

	mov	eax,(Point2 Ptr [esi]).y
	imul	(Point2 Ptr [edi]).y
	add	eax,ecx
	adc	edx,ebx

	jl	negative
	mov	eax,1
	RET
negative:
	mov	eax,-1
	RET

m_dotSign2 ENDP


;Int32 m_dotSign3( const Point3& a, const Point3& b );
;{
;	return (a.x*b.x + a.y*b.y + a.z*b.z) > 0 ? 1 : -1;
;}

m_dotSign3 PROC C \
	USES	ebx esi edi,
	ARG	a,b
	PUBLIC	C m_dotSign3

	mov	esi,a
	mov	edi,b

	mov	eax,(Point3 PTR [esi]).x
	imul	(Point3 PTR [edi]).x
	mov	ecx,eax
	mov	ebx,edx

	mov	eax,(Point3 PTR [esi]).y
	imul	(Point3 PTR [edi]).y
	add	ecx,eax
	adc	ebx,edx

	mov	eax,(Point3 PTR [esi]).z
	imul	(Point3 PTR [edi]).z
	add	eax,ecx
	adc	edx,ebx

	jl	neg3
	mov	eax,1
	RET
neg3:
	mov	eax,-1
	RET

m_dotSign3 ENDP

;Int32 m_dotSign3e( const Point3& a, const Point3& b, int error );
;{
;	return (a.x*b.x + a.y*b.y + a.z*b.z) > 0 ? 1 : -1;
;}

m_dotSign3e PROC C \
	USES	ebx esi edi,
	ARG	a,b,e
	PUBLIC	C m_dotSign3e

	mov	esi,a
	mov	edi,b

	mov	eax,(Point3 PTR [esi]).x
	imul	(Point3 PTR [edi]).x
	mov	ecx,eax
	mov	ebx,edx

	mov	eax,(Point3 PTR [esi]).y
	imul	(Point3 PTR [edi]).y
	add	ecx,eax
	adc	ebx,edx

	mov	eax,(Point3 PTR [esi]).z
	imul	(Point3 PTR [edi]).z
	add	eax,ecx
	adc	edx,ebx

	add	eax,e
	adc	edx,0

	jl	@neg
	mov	eax,1
	RET
@neg:
	mov	eax,-1
	RET

m_dotSign3e ENDP

m_asm_dot2 PROC C \
	USES esi edi,
	ARG a: PTR Point2, \
	    b: PTR Point2
	PUBLIC C m_asm_dot2

	mov   edi, [a]
	mov   esi, [b]
	fld   (Point2 PTR [edi]).x
	fmul  (Point2 PTR [esi]).x
	fld   (Point2 PTR [edi]).y
	fmul  (Point2 PTR [esi]).y
	faddp st(1), st
	RET

m_asm_dot2 ENDP

m_asm_dot3 PROC C \
	USES esi edi,
	ARG a: PTR Point3, \
	    b: PTR Point3
	PUBLIC C m_asm_dot3

	mov   edi, [a]
	mov   esi, [b]
	fld   (Point3 PTR [edi]).x
	fmul  (Point3 PTR [esi]).x
	fld   (Point3 PTR [edi]).y
	fmul  (Point3 PTR [esi]).y
	fld   (Point3 PTR [edi]).z
	fmul  (Point3 PTR [esi]).z
	fxch  st(1)
	faddp st(2), st
	faddp st(1), st
	RET

m_asm_dot3 ENDP

m_asm_cross3 PROC C \
	USES esi edi eax,
	ARG a: PTR Point3, \
        b: PTR Point3, \
        r: PTR Point3
	PUBLIC C m_asm_cross3

	mov    edi, [a]
	mov    esi, [b]
	mov    eax, [r]

	fld    (Point3 PTR [edi]).x       ;
	fmul   (Point3 PTR [esi]).y       ; a.x * b.y
	fld    (Point3 PTR [edi]).z       ;
	fmul   (Point3 PTR [esi]).x       ; a.z * b.x
	fld    (Point3 PTR [edi]).y       ;
	fmul   (Point3 PTR [esi]).z       ; a.y * b.z
	fld    (Point3 PTR [edi]).z       ;
	fmul   (Point3 PTR [esi]).y       ; a.z * b.y
	fld    (Point3 PTR [edi]).x       ;
	fmul   (Point3 PTR [esi]).z       ; a.x * b.z
	fld    (Point3 PTR [edi]).y       ;
	fmul   (Point3 PTR [esi]).x       ; a.y * b.x

	fsubp  st(5), st     ; (a.x * b.y) - (a.y * b.x)
	fsubp  st(3), st     ; (a.z * b.x) - (a.x * b.z)
	fsubp  st(1), st     ; (a.y * b.z) - (a.z * b.y)

	fstp   (Point3 PTR [eax]).x
	fstp   (Point3 PTR [eax]).y
	fstp   (Point3 PTR [eax]).z
	RET

m_asm_cross3 ENDP

END

