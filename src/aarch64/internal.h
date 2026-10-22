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

#وصف_ڪريو AARCH64_RET_VOID	0
#وصف_ڪريو AARCH64_RET_INT64	1
#وصف_ڪريو AARCH64_RET_INT128	2

#وصف_ڪريو AARCH64_RET_UNUSED3	3
#وصف_ڪريو AARCH64_RET_UNUSED4	4
#وصف_ڪريو AARCH64_RET_UNUSED5	5
#وصف_ڪريو AARCH64_RET_UNUSED6	6
#وصف_ڪريو AARCH64_RET_UNUSED7	7

/* Note that FFI_TYPE_FLOAT == 2, _DOUBLE == 3, _LONGDOUBLE == 4,
   so _S4 through _Q1 are layed out as (TYPE * 4) + (4 - COUNT).  */
#وصف_ڪريو AARCH64_RET_S4		8
#وصف_ڪريو AARCH64_RET_S3		9
#وصف_ڪريو AARCH64_RET_S2		10
#وصف_ڪريو AARCH64_RET_S1		11

#وصف_ڪريو AARCH64_RET_D4		12
#وصف_ڪريو AARCH64_RET_D3		13
#وصف_ڪريو AARCH64_RET_D2		14
#وصف_ڪريو AARCH64_RET_D1		15

#وصف_ڪريو AARCH64_RET_Q4		16
#وصف_ڪريو AARCH64_RET_Q3		17
#وصف_ڪريو AARCH64_RET_Q2		18
#وصف_ڪريو AARCH64_RET_Q1		19

/* Note that each of the sub-64-bit integers gets two entries.  */
#وصف_ڪريو AARCH64_RET_UINT8	20
#وصف_ڪريو AARCH64_RET_UINT16	22
#وصف_ڪريو AARCH64_RET_UINT32	24

#وصف_ڪريو AARCH64_RET_SINT8	26
#وصف_ڪريو AARCH64_RET_SINT16	28
#وصف_ڪريو AARCH64_RET_SINT32	30

#وصف_ڪريو AARCH64_RET_نقاب	31

#وصف_ڪريو AARCH64_RET_اندريون_MEM	(1 << 5)
#وصف_ڪريو AARCH64_RET_NEED_COPY	(1 << 6)

#وصف_ڪريو AARCH64_FLAG_ARG_V_ٻٽو_انگ	7
#وصف_ڪريو AARCH64_FLAG_ARG_V	(1 << AARCH64_FLAG_ARG_V_ٻٽو_انگ)
#وصف_ڪريو AARCH64_FLAG_VARARG	(1 << 8)

#وصف_ڪريو N_X_ARG_REG		8
#وصف_ڪريو N_V_ARG_REG		8
#وصف_ڪريو CALL_CONTEXT_سائيز	(N_V_ARG_REG * 16 + N_X_ARG_REG * 8)

#إذا defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 16K is chosen to
 * cover the base page sizes of 4K and 16K.
 */
#وصف_ڪريو AARCH64_TRAMP_MAP_سرڪايو	14
#وصف_ڪريو AARCH64_TRAMP_MAP_سائيز	(1 << AARCH64_TRAMP_MAP_سرڪايو)
#وصف_ڪريو AARCH64_TRAMP_سائيز	32

#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل __AARCH64EB__
# define BE(X)	X
#وإلا
# define BE(X)	0
#جيڪڏهن_پورو
/* Helpers for writing assembly compatible with arm ptr auth */
#جيڪڏهن_وصف_ٿيل LIBFFI_ASM

  #إذا defined(HAVE_ARM64E_PTRAUTH)
  /* ARM64E ABI For Darwin */
  #وصف_ڪريو SIGN_LR pacibsp
  #وصف_ڪريو SIGN_LR_WITH_REG(x) pacib lr, x
  #وصف_ڪريو AUTH_LR_AND_RET retab
  #وصف_ڪريو AUTH_LR_WITH_REG(x) autib lr, x
  #وصف_ڪريو BRANCH_AND_LINK_منزل_REG blraaz
  #وصف_ڪريو SIGN_LR_LINUX_ONLY
  #وصف_ڪريو BRANCH_منزل_REG braaz
  #وصف_ڪريو PAC_CFI_دري_SAVE
  #وصف_ڪريو AARCH64_اشارو_AUTH 0
  /* Linux PAC Support */
  #وإلاإذا defined(__ARM_FEATURE_PAC_DEFAULT)
    #وصف_ڪريو AARCH64_اشارو_AUTH (1 << 1)
    #وصف_ڪريو PAC_CFI_دري_SAVE cfi_دري_save
    #وصف_ڪريو TMP_REG x9
    #وصف_ڪريو BRANCH_منزل_REG br
    #وصف_ڪريو BRANCH_AND_LINK_منزل_REG blr
	#وصف_ڪريو SIGN_LR_LINUX_ONLY SIGN_LR
    /* Which key to sign with? */
    #إذا (__ARM_FEATURE_PAC_DEFAULT & 1) == 1
      /* Signed with A-key */
      #وصف_ڪريو SIGN_LR            hint #25  /* paciasp */
      #وصف_ڪريو AUTH_LR            hint #29  /* autiasp */
    #وإلا
      /* Signed with B-key */
      #وصف_ڪريو SIGN_LR            hint #27  /* pacibsp */
      #وصف_ڪريو AUTH_LR            hint #31  /* autibsp */
    #جيڪڏهن_پورو /* __ARM_FEATURE_PAC_DEFAULT */
    #وصف_ڪريو AUTH_LR_WITH_REG(x) _auth_lr_with_reg x
.macro _auth_lr_with_reg modifier
    mov TMP_REG, sp
    mov sp, \modifier
    AUTH_LR
    mov sp, TMP_REG
.endm
  #وصف_ڪريو SIGN_LR_WITH_REG(x) _sign_lr_with_reg x
.macro _sign_lr_with_reg modifier
    mov TMP_REG, sp
    mov sp, \modifier
    SIGN_LR
    mov sp, TMP_REG
.endm
  #وصف_ڪريو AUTH_LR_AND_RET _auth_lr_and_ret modifier
.macro _auth_lr_and_ret modifier
    AUTH_LR
    ret
.endm
  #وصف_رد TMP_REG

  /* No Pointer Auth */
  #وإلا
    #وصف_ڪريو SIGN_LR
    #وصف_ڪريو SIGN_LR_WITH_REG(x)
    #وصف_ڪريو AUTH_LR_AND_RET ret
    #وصف_ڪريو AUTH_LR_WITH_REG(x)
    #وصف_ڪريو BRANCH_AND_LINK_منزل_REG blr
    #وصف_ڪريو SIGN_LR_LINUX_ONLY
    #وصف_ڪريو BRANCH_منزل_REG br
    #وصف_ڪريو PAC_CFI_دري_SAVE
    #وصف_ڪريو AARCH64_اشارو_AUTH 0
  #جيڪڏهن_پورو /* HAVE_ARM64E_PTRAUTH */
#جيڪڏهن_پورو /* LIBFFI_ASM */
