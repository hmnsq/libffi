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

#تعریف_کن AARCH64_RET_VOID	0
#تعریف_کن AARCH64_RET_INT64	1
#تعریف_کن AARCH64_RET_INT128	2

#تعریف_کن AARCH64_RET_UNUSED3	3
#تعریف_کن AARCH64_RET_UNUSED4	4
#تعریف_کن AARCH64_RET_UNUSED5	5
#تعریف_کن AARCH64_RET_UNUSED6	6
#تعریف_کن AARCH64_RET_UNUSED7	7

/* Note that FFI_TYPE_FLOAT == 2, _DOUBLE == 3, _LONGDOUBLE == 4,
   so _S4 through _Q1 are layed out as (TYPE * 4) + (4 - COUNT).  */
#تعریف_کن AARCH64_RET_S4		8
#تعریف_کن AARCH64_RET_S3		9
#تعریف_کن AARCH64_RET_S2		10
#تعریف_کن AARCH64_RET_S1		11

#تعریف_کن AARCH64_RET_D4		12
#تعریف_کن AARCH64_RET_D3		13
#تعریف_کن AARCH64_RET_D2		14
#تعریف_کن AARCH64_RET_D1		15

#تعریف_کن AARCH64_RET_Q4		16
#تعریف_کن AARCH64_RET_Q3		17
#تعریف_کن AARCH64_RET_Q2		18
#تعریف_کن AARCH64_RET_Q1		19

/* Note that each of the sub-64-bit integers gets two entries.  */
#تعریف_کن AARCH64_RET_UINT8	20
#تعریف_کن AARCH64_RET_UINT16	22
#تعریف_کن AARCH64_RET_UINT32	24

#تعریف_کن AARCH64_RET_SINT8	26
#تعریف_کن AARCH64_RET_SINT16	28
#تعریف_کن AARCH64_RET_SINT32	30

#تعریف_کن AARCH64_RET_ماسک	31

#تعریف_کن AARCH64_RET_درون_MEM	(1 << 5)
#تعریف_کن AARCH64_RET_NEED_COPY	(1 << 6)

#تعریف_کن AARCH64_FLAG_ARG_V_بیت	7
#تعریف_کن AARCH64_FLAG_ARG_V	(1 << AARCH64_FLAG_ARG_V_بیت)
#تعریف_کن AARCH64_FLAG_VARARG	(1 << 8)

#تعریف_کن N_X_ARG_REG		8
#تعریف_کن N_V_ARG_REG		8
#تعریف_کن CALL_CONTEXT_اندازه	(N_V_ARG_REG * 16 + N_X_ARG_REG * 8)

#إذا defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 16K is chosen to
 * cover the base page sizes of 4K and 16K.
 */
#تعریف_کن AARCH64_TRAMP_MAP_لغزش	14
#تعریف_کن AARCH64_TRAMP_MAP_اندازه	(1 << AARCH64_TRAMP_MAP_لغزش)
#تعریف_کن AARCH64_TRAMP_اندازه	32

#پایان_اگر

#اگر_تعریف_شده __AARCH64EB__
# define BE(X)	X
#وإلا
# define BE(X)	0
#پایان_اگر
/* Helpers for writing assembly compatible with arm ptr auth */
#اگر_تعریف_شده LIBFFI_ASM

  #إذا defined(HAVE_ARM64E_PTRAUTH)
  /* ARM64E ABI For Darwin */
  #تعریف_کن SIGN_LR pacibsp
  #تعریف_کن SIGN_LR_WITH_REG(x) pacib lr, x
  #تعریف_کن AUTH_LR_AND_RET retab
  #تعریف_کن AUTH_LR_WITH_REG(x) autib lr, x
  #تعریف_کن BRANCH_AND_LINK_مقصد_REG blraaz
  #تعریف_کن SIGN_LR_LINUX_ONLY
  #تعریف_کن BRANCH_مقصد_REG braaz
  #تعریف_کن PAC_CFI_پنجره_SAVE
  #تعریف_کن AARCH64_اشاره_گر_AUTH 0
  /* Linux PAC Support */
  #وإلاإذا defined(__ARM_FEATURE_PAC_DEFAULT)
    #تعریف_کن AARCH64_اشاره_گر_AUTH (1 << 1)
    #تعریف_کن PAC_CFI_پنجره_SAVE cfi_پنجره_save
    #تعریف_کن TMP_REG x9
    #تعریف_کن BRANCH_مقصد_REG br
    #تعریف_کن BRANCH_AND_LINK_مقصد_REG blr
	#تعریف_کن SIGN_LR_LINUX_ONLY SIGN_LR
    /* Which key to sign with? */
    #إذا (__ARM_FEATURE_PAC_DEFAULT & 1) == 1
      /* Signed with A-key */
      #تعریف_کن SIGN_LR            hint #25  /* paciasp */
      #تعریف_کن AUTH_LR            hint #29  /* autiasp */
    #وإلا
      /* Signed with B-key */
      #تعریف_کن SIGN_LR            hint #27  /* pacibsp */
      #تعریف_کن AUTH_LR            hint #31  /* autibsp */
    #پایان_اگر /* __ARM_FEATURE_PAC_DEFAULT */
    #تعریف_کن AUTH_LR_WITH_REG(x) _auth_lr_with_reg x
.macro _auth_lr_with_reg modifier
    mov TMP_REG, sp
    mov sp, \modifier
    AUTH_LR
    mov sp, TMP_REG
.endm
  #تعریف_کن SIGN_LR_WITH_REG(x) _sign_lr_with_reg x
.macro _sign_lr_with_reg modifier
    mov TMP_REG, sp
    mov sp, \modifier
    SIGN_LR
    mov sp, TMP_REG
.endm
  #تعریف_کن AUTH_LR_AND_RET _auth_lr_and_ret modifier
.macro _auth_lr_and_ret modifier
    AUTH_LR
    ret
.endm
  #لغو_تعریف TMP_REG

  /* No Pointer Auth */
  #وإلا
    #تعریف_کن SIGN_LR
    #تعریف_کن SIGN_LR_WITH_REG(x)
    #تعریف_کن AUTH_LR_AND_RET ret
    #تعریف_کن AUTH_LR_WITH_REG(x)
    #تعریف_کن BRANCH_AND_LINK_مقصد_REG blr
    #تعریف_کن SIGN_LR_LINUX_ONLY
    #تعریف_کن BRANCH_مقصد_REG br
    #تعریف_کن PAC_CFI_پنجره_SAVE
    #تعریف_کن AARCH64_اشاره_گر_AUTH 0
  #پایان_اگر /* HAVE_ARM64E_PTRAUTH */
#پایان_اگر /* LIBFFI_ASM */
