;
; $Workfile:   m_vec.inc  $
; $Revision:   1.2  $
; $Version: 1.6.2 $
; $Date:   04 Aug 1995 10:39:14  $
; $Log:   S:\phoenix\ml\vcs\m_vec.inv  $
;	
;	   Rev 1.2   04 Aug 1995 10:39:14   MARKF
;	Added Vector4 and Point4 types
;	
;	   Rev 1.1   08 Mar 1995 08:45:18   RICKO
;	Updated to compile with TASM and MASM
;	
;	   Rev 1.0   07 Mar 1995 13:16:18   NANCYH
;	Initial revision.
;

INCLUDE m_trig.inc


Point2 STRUC
	x	DD ?
	y	DD ?
Point2 ENDS


Point3 STRUC 
	x	DD ?
	y	DD ?
	z	DD ?
Point3 ENDS

Point4 STRUC
	x	DD ?
	y	DD ?
	z	DD ?
        w       DD ?
Point4 ENDS

Vector2 STRUC
	x	DD ?
	y	DD ?
Vector2 ENDS


Vector3 STRUC
	x	DD ?
	y	DD ?
	z	DD ?
Vector3 ENDS

Vector4 STRUC
        x       DD ?
        y       DD ?
        z       DD ?
        w       DD ?
Vector4 ENDS
