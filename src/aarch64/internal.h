/* 
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
``Software''), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_VOID	0
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_INT64	1
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_INT128	2

#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_UNUSED3	3
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_UNUSED4	4
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_UNUSED5	5
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_UNUSED6	6
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_UNUSED7	7

/* Note that FFI_TYPE_FLOAT == 2, _DOUBLE == 3, _LONGDOUBLE == 4,
   so _S4 through _Q1 are layed out as (TYPE * 4) + (4 - COUNT).  */
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_S4		8
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_S3		9
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_S2		10
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_S1		11

#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_D4		12
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_D3		13
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_D2		14
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_D1		15

#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_Q4		16
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_Q3		17
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_Q2		18
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_Q1		19

/* Note that each of the sub-64-bit integers gets two entries.  */
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_UINT8	20
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_UINT16	22
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_UINT32	24

#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_SINT8	26
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_SINT16	28
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_SINT32	30

#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_ಮುಖವಾಡ	31

#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_ಒಳ_MEM	(1 << 5)
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_RET_NEED_COPY	(1 << 6)

#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_FLAG_ARG_V_ಬಿಟ್	7
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_FLAG_ARG_V	(1 << AARCH64_FLAG_ARG_V_ಬಿಟ್)
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_FLAG_VARARG	(1 << 8)

#ವ್ಯಾಖ್ಯಾನಿಸು N_X_ARG_REG		8
#ವ್ಯಾಖ್ಯಾನಿಸು N_V_ARG_REG		8
#ವ್ಯಾಖ್ಯಾನಿಸು CALL_CONTEXT_ಗಾತ್ರ	(N_V_ARG_REG * 16 + N_X_ARG_REG * 8)

#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 16K is chosen to
 * cover the base page sizes of 4K and 16K.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_TRAMP_MAP_ಸರಿಸು	14
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_TRAMP_MAP_ಗಾತ್ರ	(1 << AARCH64_TRAMP_MAP_ಸರಿಸು)
#ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_TRAMP_ಗಾತ್ರ	32

#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __AARCH64EB__
# define BE(X)	X
#ಇಲ್ಲದಿದ್ದರೆ
# define BE(X)	0
#ಆದರೆ_ಅಂತ್ಯ
/* Helpers for writing assembly compatible with arm ptr auth */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LIBFFI_ASM

  #ಆದರೆ defined(HAVE_ARM64E_PTRAUTH)
  /* ARM64E ABI For Darwin */
  #ವ್ಯಾಖ್ಯಾನಿಸು SIGN_LR pacibsp
  #ವ್ಯಾಖ್ಯಾನಿಸು SIGN_LR_WITH_REG(x) pacib lr, x
  #ವ್ಯಾಖ್ಯಾನಿಸು AUTH_LR_AND_RET retab
  #ವ್ಯಾಖ್ಯಾನಿಸು AUTH_LR_WITH_REG(x) autib lr, x
  #ವ್ಯಾಖ್ಯಾನಿಸು BRANCH_AND_LINK_ಗಮ್ಯ_REG blraaz
  #ವ್ಯಾಖ್ಯಾನಿಸು SIGN_LR_LINUX_ONLY
  #ವ್ಯಾಖ್ಯಾನಿಸು BRANCH_ಗಮ್ಯ_REG braaz
  #ವ್ಯಾಖ್ಯಾನಿಸು PAC_CFI_ಕಿಟಕಿ_SAVE
  #ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_ಸೂಚಕ_AUTH 0
  /* Linux PAC Support */
  #ಅಥವಾಆದರೆ defined(__ARM_FEATURE_PAC_DEFAULT)
    #ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_ಸೂಚಕ_AUTH (1 << 1)
    #ವ್ಯಾಖ್ಯಾನಿಸು PAC_CFI_ಕಿಟಕಿ_SAVE cfi_ಕಿಟಕಿ_save
    #ವ್ಯಾಖ್ಯಾನಿಸು TMP_REG x9
    #ವ್ಯಾಖ್ಯಾನಿಸು BRANCH_ಗಮ್ಯ_REG br
    #ವ್ಯಾಖ್ಯಾನಿಸು BRANCH_AND_LINK_ಗಮ್ಯ_REG blr
	#ವ್ಯಾಖ್ಯಾನಿಸು SIGN_LR_LINUX_ONLY SIGN_LR
    /* Which key to sign with? */
    #ಆದರೆ (__ARM_FEATURE_PAC_DEFAULT & 1) == 1
      /* Signed with A-key */
      #ವ್ಯಾಖ್ಯಾನಿಸು SIGN_LR            hint #25  /* paciasp */
      #ವ್ಯಾಖ್ಯಾನಿಸು AUTH_LR            hint #29  /* autiasp */
    #ಇಲ್ಲದಿದ್ದರೆ
      /* Signed with B-key */
      #ವ್ಯಾಖ್ಯಾನಿಸು SIGN_LR            hint #27  /* pacibsp */
      #ವ್ಯಾಖ್ಯಾನಿಸು AUTH_LR            hint #31  /* autibsp */
    #ಆದರೆ_ಅಂತ್ಯ /* __ARM_FEATURE_PAC_DEFAULT */
    #ವ್ಯಾಖ್ಯಾನಿಸು AUTH_LR_WITH_REG(x) _auth_lr_with_reg x
.macro _auth_lr_with_reg modifier
    mov TMP_REG, sp
    mov sp, \modifier
    AUTH_LR
    mov sp, TMP_REG
.endm
  #ವ್ಯಾಖ್ಯಾನಿಸು SIGN_LR_WITH_REG(x) _sign_lr_with_reg x
.macro _sign_lr_with_reg modifier
    mov TMP_REG, sp
    mov sp, \modifier
    SIGN_LR
    mov sp, TMP_REG
.endm
  #ವ್ಯಾಖ್ಯಾನಿಸು AUTH_LR_AND_RET _auth_lr_and_ret modifier
.macro _auth_lr_and_ret modifier
    AUTH_LR
    ret
.endm
  #ವ್ಯಾಖ್ಯಾನ_ರದ್ದು TMP_REG

  /* No Pointer Auth */
  #ಇಲ್ಲದಿದ್ದರೆ
    #ವ್ಯಾಖ್ಯಾನಿಸು SIGN_LR
    #ವ್ಯಾಖ್ಯಾನಿಸು SIGN_LR_WITH_REG(x)
    #ವ್ಯಾಖ್ಯಾನಿಸು AUTH_LR_AND_RET ret
    #ವ್ಯಾಖ್ಯಾನಿಸು AUTH_LR_WITH_REG(x)
    #ವ್ಯಾಖ್ಯಾನಿಸು BRANCH_AND_LINK_ಗಮ್ಯ_REG blr
    #ವ್ಯಾಖ್ಯಾನಿಸು SIGN_LR_LINUX_ONLY
    #ವ್ಯಾಖ್ಯಾನಿಸು BRANCH_ಗಮ್ಯ_REG br
    #ವ್ಯಾಖ್ಯಾನಿಸು PAC_CFI_ಕಿಟಕಿ_SAVE
    #ವ್ಯಾಖ್ಯಾನಿಸು AARCH64_ಸೂಚಕ_AUTH 0
  #ಆದರೆ_ಅಂತ್ಯ /* HAVE_ARM64E_PTRAUTH */
#ಆದರೆ_ಅಂತ್ಯ /* LIBFFI_ASM */
