;
; $Workfile:   m_atrig.asm  $
; $Revision:   1.1  $
; $Version$
; $Date:   08 Mar 1995 08:44:00  $
; $Log:   S:\phoenix\ml\vcs\m_atrig.asv  $
;	
;	   Rev 1.1   08 Mar 1995 08:44:00   RICKO
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

.DATA

asintab label word
	dw  0000h, 0014h, 0028h, 003dh, 0051h, 0065h, 007ah, 008eh
	dw  00a2h, 00b7h, 00cbh, 00e0h, 00f4h, 0108h, 011dh, 0131h
	dw  0146h, 015ah, 016eh, 0183h, 0197h, 01abh, 01c0h, 01d4h
	dw  01e9h, 01fdh, 0211h, 0226h, 023ah, 024fh, 0263h, 0277h
	dw  028ch, 02a0h, 02b5h, 02c9h, 02ddh, 02f2h, 0306h, 031bh
	dw  032fh, 0344h, 0358h, 036dh, 0381h, 0395h, 03aah, 03beh
	dw  03d3h, 03e7h, 03fch, 0410h, 0425h, 0439h, 044eh, 0462h
	dw  0477h, 048bh, 04a0h, 04b4h, 04c9h, 04ddh, 04f2h, 0506h
	dw  051bh, 052fh, 0544h, 0558h, 056dh, 0581h, 0596h, 05abh
	dw  05bfh, 05d4h, 05e8h, 05fdh, 0612h, 0626h, 063bh, 064fh
	dw  0664h, 0679h, 068dh, 06a2h, 06b7h, 06cbh, 06e0h, 06f4h
	dw  0709h, 071eh, 0733h, 0747h, 075ch, 0771h, 0785h, 079ah
	dw  07afh, 07c4h, 07d8h, 07edh, 0802h, 0817h, 082bh, 0840h
	dw  0855h, 086ah, 087fh, 0893h, 08a8h, 08bdh, 08d2h, 08e7h
	dw  08fch, 0911h, 0926h, 093ah, 094fh, 0964h, 0979h, 098eh
	dw  09a3h, 09b8h, 09cdh, 09e2h, 09f7h, 0a0ch, 0a21h, 0a36h
	dw  0a4bh, 0a60h, 0a75h, 0a8ah, 0a9fh, 0ab4h, 0ac9h, 0adfh
	dw  0af4h, 0b09h, 0b1eh, 0b33h, 0b48h, 0b5eh, 0b73h, 0b88h
	dw  0b9dh, 0bb2h, 0bc8h, 0bddh, 0bf2h, 0c07h, 0c1dh, 0c32h
	dw  0c47h, 0c5dh, 0c72h, 0c87h, 0c9dh, 0cb2h, 0cc8h, 0cddh
	dw  0cf3h, 0d08h, 0d1dh, 0d33h, 0d48h, 0d5eh, 0d73h, 0d89h
	dw  0d9fh, 0db4h, 0dcah, 0ddfh, 0df5h, 0e0bh, 0e20h, 0e36h
	dw  0e4ch, 0e61h, 0e77h, 0e8dh, 0ea3h, 0eb8h, 0eceh, 0ee4h
	dw  0efah, 0f10h, 0f25h, 0f3bh, 0f51h, 0f67h, 0f7dh, 0f93h
	dw  0fa9h, 0fbfh, 0fd5h, 0febh, 1001h, 1017h, 102dh, 1043h
	dw  1059h, 106fh, 1086h, 109ch, 10b2h, 10c8h, 10deh, 10f5h
	dw  110bh, 1121h, 1138h, 114eh, 1164h, 117bh, 1191h, 11a8h
	dw  11beh, 11d5h, 11ebh, 1202h, 1218h, 122fh, 1245h, 125ch
	dw  1273h, 1289h, 12a0h, 12b7h, 12cdh, 12e4h, 12fbh, 1312h
	dw  1329h, 133fh, 1356h, 136dh, 1384h, 139bh, 13b2h, 13c9h
	dw  13e0h, 13f7h, 140eh, 1426h, 143dh, 1454h, 146bh, 1482h
	dw  149ah, 14b1h, 14c8h, 14e0h, 14f7h, 150eh, 1526h, 153dh
	dw  1555h, 156ch, 1584h, 159ch, 15b3h, 15cbh, 15e3h, 15fah
	dw  1612h, 162ah, 1642h, 1659h, 1671h, 1689h, 16a1h, 16b9h
	dw  16d1h, 16e9h, 1701h, 171ah, 1732h, 174ah, 1762h, 177ah
	dw  1793h, 17abh, 17c4h, 17dch, 17f4h, 180dh, 1825h, 183eh
	dw  1857h, 186fh, 1888h, 18a1h, 18bah, 18d2h, 18ebh, 1904h
	dw  191dh, 1936h, 194fh, 1968h, 1981h, 199ah, 19b4h, 19cdh
	dw  19e6h, 1a00h, 1a19h, 1a32h, 1a4ch, 1a65h, 1a7fh, 1a99h
	dw  1ab2h, 1acch, 1ae6h, 1b00h, 1b19h, 1b33h, 1b4dh, 1b67h
	dw  1b81h, 1b9ch, 1bb6h, 1bd0h, 1beah, 1c05h, 1c1fh, 1c39h
	dw  1c54h, 1c6eh, 1c89h, 1ca4h, 1cbeh, 1cd9h, 1cf4h, 1d0fh
	dw  1d2ah, 1d45h, 1d60h, 1d7bh, 1d96h, 1db2h, 1dcdh, 1de8h
	dw  1e04h, 1e1fh, 1e3bh, 1e57h, 1e73h, 1e8eh, 1eaah, 1ec6h
	dw  1ee2h, 1efeh, 1f1ah, 1f37h, 1f53h, 1f6fh, 1f8ch, 1fa8h
	dw  1fc5h, 1fe2h, 1ffeh, 201bh, 2038h, 2055h, 2072h, 208fh
	dw  20adh, 20cah, 20e7h, 2105h, 2123h, 2140h, 215eh, 217ch
	dw  219ah, 21b8h, 21d6h, 21f4h, 2213h, 2231h, 2250h, 226eh
	dw  228dh, 22ach, 22cbh, 22eah, 2309h, 2328h, 2348h, 2367h
	dw  2387h, 23a7h, 23c6h, 23e6h, 2407h, 2427h, 2447h, 2467h
	dw  2488h, 24a9h, 24cah, 24ebh, 250ch, 252dh, 254eh, 2570h
	dw  2591h, 25b3h, 25d5h, 25f7h, 2619h, 263ch, 265eh, 2681h
	dw  26a4h, 26c7h, 26eah, 270eh, 2731h, 2755h, 2779h, 279dh
	dw  27c1h, 27e5h, 280ah, 282fh, 2854h, 2879h, 289fh, 28c4h
	dw  28eah, 2910h, 2936h, 295dh, 2984h, 29abh, 29d2h, 29f9h
	dw  2a21h, 2a49h, 2a71h, 2a9ah, 2ac2h, 2aech, 2b15h, 2b3eh

;	dw  2b68h, 2b93h, 2bbdh, 2be8h, 2c13h, 2c3fh, 2c6bh, 2c97h
;	dw  2cc4h, 2cf1h, 2d1eh, 2d4ch, 2d7ah, 2da8h, 2dd8h, 2e07h
;	dw  2e37h, 2e67h, 2e98h, 2ecah, 2efch, 2f2eh, 2f61h, 2f95h
;	dw  2fc9h, 2ffeh, 3034h, 306ah, 30a1h, 30d9h, 3111h, 314ah

;	dw  3184h, 31bfh, 31fbh, 3238h, 3276h, 32b5h, 32f5h, 3337h
;	dw  3379h, 33bdh, 3403h, 344ah, 3493h, 34ddh, 352ah, 3578h
;	dw  35c9h, 361dh, 3673h, 36cch, 3729h, 378ah, 37efh, 3859h
;	dw  38c9h, 3941h, 39c1h, 3a4dh, 3ae7h, 3b96h, 3c65h, 3d73h

;	dw  4000h

	dw  2b68h, 2b6eh, 2b73h, 2b78h, 2b7dh, 2b83h, 2b88h, 2b8dh
	dw  2b93h, 2b98h, 2b9dh, 2ba3h, 2ba8h, 2badh, 2bb3h, 2bb8h
	dw  2bbdh, 2bc3h, 2bc8h, 2bcdh, 2bd3h, 2bd8h, 2bddh, 2be3h
	dw  2be8h, 2bedh, 2bf3h, 2bf8h, 2bfeh, 2c03h, 2c08h, 2c0eh
	dw  2c13h, 2c19h, 2c1eh, 2c24h, 2c29h, 2c2eh, 2c34h, 2c39h
	dw  2c3fh, 2c44h, 2c4ah, 2c4fh, 2c55h, 2c5ah, 2c60h, 2c65h
	dw  2c6bh, 2c70h, 2c76h, 2c7bh, 2c81h, 2c86h, 2c8ch, 2c91h
	dw  2c97h, 2c9ch, 2ca2h, 2ca8h, 2cadh, 2cb3h, 2cb8h, 2cbeh
 
	dw  2cc4h, 2cc9h, 2ccfh, 2cd4h, 2cdah, 2ce0h, 2ce5h, 2cebh
	dw  2cf1h, 2cf6h, 2cfch, 2d02h, 2d07h, 2d0dh, 2d13h, 2d18h
	dw  2d1eh, 2d24h, 2d29h, 2d2fh, 2d35h, 2d3ah, 2d40h, 2d46h
	dw  2d4ch, 2d51h, 2d57h, 2d5dh, 2d63h, 2d68h, 2d6eh, 2d74h
	dw  2d7ah, 2d80h, 2d85h, 2d8bh, 2d91h, 2d97h, 2d9dh, 2da3h
	dw  2da8h, 2daeh, 2db4h, 2dbah, 2dc0h, 2dc6h, 2dcch, 2dd2h
	dw  2dd8h, 2dddh, 2de3h, 2de9h, 2defh, 2df5h, 2dfbh, 2e01h
	dw  2e07h, 2e0dh, 2e13h, 2e19h, 2e1fh, 2e25h, 2e2bh, 2e31h

	dw  2e37h, 2e3dh, 2e43h, 2e49h, 2e4fh, 2e55h, 2e5bh, 2e61h
	dw  2e67h, 2e6dh, 2e74h, 2e7ah, 2e80h, 2e86h, 2e8ch, 2e92h
	dw  2e98h, 2e9eh, 2ea5h, 2eabh, 2eb1h, 2eb7h, 2ebdh, 2ec4h
	dw  2ecah, 2ed0h, 2ed6h, 2edch, 2ee3h, 2ee9h, 2eefh, 2ef5h
	dw  2efch, 2f02h, 2f08h, 2f0fh, 2f15h, 2f1bh, 2f22h, 2f28h
	dw  2f2eh, 2f35h, 2f3bh, 2f41h, 2f48h, 2f4eh, 2f54h, 2f5bh
	dw  2f61h, 2f68h, 2f6eh, 2f75h, 2f7bh, 2f82h, 2f88h, 2f8eh
	dw  2f95h, 2f9bh, 2fa2h, 2fa8h, 2fafh, 2fb6h, 2fbch, 2fc3h

	dw  2fc9h, 2fd0h, 2fd6h, 2fddh, 2fe4h, 2feah, 2ff1h, 2ff7h
	dw  2ffeh, 3005h, 300bh, 3012h, 3019h, 301fh, 3026h, 302dh
	dw  3034h, 303ah, 3041h, 3048h, 304fh, 3055h, 305ch, 3063h
	dw  306ah, 3071h, 3078h, 307eh, 3085h, 308ch, 3093h, 309ah
	dw  30a1h, 30a8h, 30afh, 30b6h, 30bdh, 30c4h, 30cbh, 30d1h
	dw  30d9h, 30e0h, 30e7h, 30eeh, 30f5h, 30fch, 3103h, 310ah
	dw  3111h, 3118h, 311fh, 3126h, 312dh, 3135h, 313ch, 3143h
	dw  314ah, 3151h, 3159h, 3160h, 3167h, 316eh, 3176h, 317dh

	dw  3184h, 318ch, 3193h, 319ah, 31a2h, 31a9h, 31b0h, 31b8h
	dw  31bfh, 31c7h, 31ceh, 31d6h, 31ddh, 31e5h, 31ech, 31f4h
	dw  31fbh, 3203h, 320ah, 3212h, 321ah, 3221h, 3229h, 3230h
	dw  3238h, 3240h, 3248h, 324fh, 3257h, 325fh, 3266h, 326eh
	dw  3276h, 327eh, 3286h, 328eh, 3295h, 329dh, 32a5h, 32adh
	dw  32b5h, 32bdh, 32c5h, 32cdh, 32d5h, 32ddh, 32e5h, 32edh
	dw  32f5h, 32fdh, 3305h, 330eh, 3316h, 331eh, 3326h, 332eh
	dw  3337h, 333fh, 3347h, 334fh, 3358h, 3360h, 3368h, 3371h

	dw  3379h, 3382h, 338ah, 3393h, 339bh, 33a4h, 33ach, 33b5h
	dw  33bdh, 33c6h, 33cfh, 33d7h, 33e0h, 33e9h, 33f1h, 33fah
	dw  3403h, 340ch, 3414h, 341dh, 3426h, 342fh, 3438h, 3441h
	dw  344ah, 3453h, 345ch, 3465h, 346eh, 3477h, 3480h, 3489h
	dw  3493h, 349ch, 34a5h, 34aeh, 34b8h, 34c1h, 34cah, 34d4h
	dw  34ddh, 34e7h, 34f0h, 34fah, 3503h, 350dh, 3516h, 3520h
	dw  352ah, 3533h, 353dh, 3547h, 3551h, 355ah, 3564h, 356eh
	dw  3578h, 3582h, 358ch, 3596h, 35a0h, 35aah, 35b5h, 35bfh

	dw  35c9h, 35d3h, 35deh, 35e8h, 35f2h, 35fdh, 3607h, 3612h
	dw  361dh, 3627h, 3632h, 363ch, 3647h, 3652h, 365dh, 3668h
	dw  3673h, 367eh, 3689h, 3694h, 369fh, 36aah, 36b5h, 36c1h
	dw  36cch, 36d7h, 36e3h, 36eeh, 36fah, 3706h, 3711h, 371dh
	dw  3729h, 3735h, 3741h, 374dh, 3759h, 3765h, 3771h, 377dh
	dw  378ah, 3796h, 37a2h, 37afh, 37bbh, 37c8h, 37d5h, 37e2h
	dw  37efh, 37fch, 3809h, 3816h, 3823h, 3830h, 383eh, 384bh
	dw  3859h, 3867h, 3874h, 3882h, 3890h, 389eh, 38adh, 38bbh

	dw  38c9h, 38d8h, 38e6h, 38f5h, 3904h, 3913h, 3922h, 3931h
	dw  3941h, 3950h, 3960h, 3970h, 3980h, 3990h, 39a0h, 39b1h
	dw  39c1h, 39d2h, 39e3h, 39f4h, 3a05h, 3a17h, 3a29h, 3a3ah
	dw  3a4dh, 3a5fh, 3a72h, 3a84h, 3a98h, 3aabh, 3abfh, 3ad3h
	dw  3ae7h, 3afbh, 3b10h, 3b26h, 3b3bh, 3b51h, 3b68h, 3b7fh
	dw  3b96h, 3baeh, 3bc6h, 3bdfh, 3bf8h, 3c12h, 3c2dh, 3c49h
	dw  3c65h, 3c83h, 3ca1h, 3cc0h, 3ce1h, 3d03h, 3d27h, 3d4ch
	dw  3d73h, 3d9eh, 3dcbh, 3dfch, 3e33h, 3e70h, 3ebah, 3f19h

	dw  4000h

.CODE

;
; Angle m_asin (const Fix16 x);
;
; returns the arc sine of x. x must be within -1 <= x <= 1
;
m_asin_ PROC C \
        ,
	ARG	x
	PUBLIC	C m_asin_

	mov	eax,x

	cdq			; eax = abs(x), edx = -1,0 (sign of x)
	xor	eax,edx
	sub	eax,edx		

	sar	eax, 4		; 0-0x10000 => 0-0x1000
        adc     eax,0           ; round up/down
	cmp	eax, 448*8-1	; is answer in first 448 table entries?
	ja	bigtable
smalltable:
        sar     eax,3		; 0-0x1000 => 0-0x200
        adc     eax,0           ; round up/down
        movzx   eax,asintab[eax*2]

	xor	eax,edx		; flip sign back, if changed
	sub	eax,edx

	RET

bigtable:
        movzx   eax,asintab[eax*2]-448*7*2

	xor	eax,edx		; flip sign back, if changed
	sub	eax,edx

	RET

m_asin_ ENDP

END

