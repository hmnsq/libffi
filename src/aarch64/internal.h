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

#تعریف_کړه AARCH64_RET_VOID	0
#تعریف_کړه AARCH64_RET_INT64	1
#تعریف_کړه AARCH64_RET_INT128	2

#تعریف_کړه AARCH64_RET_UNUSED3	3
#تعریف_کړه AARCH64_RET_UNUSED4	4
#تعریف_کړه AARCH64_RET_UNUSED5	5
#تعریف_کړه AARCH64_RET_UNUSED6	6
#تعریف_کړه AARCH64_RET_UNUSED7	7

/* Note that FFI_TYPE_FLOAT == 2, _DOUBLE == 3, _LONGDOUBLE == 4,
   so _S4 through _Q1 are layed out as (TYPE * 4) + (4 - COUNT).  */
#تعریف_کړه AARCH64_RET_S4		8
#تعریف_کړه AARCH64_RET_S3		9
#تعریف_کړه AARCH64_RET_S2		10
#تعریف_کړه AARCH64_RET_S1		11

#تعریف_کړه AARCH64_RET_D4		12
#تعریف_کړه AARCH64_RET_D3		13
#تعریف_کړه AARCH64_RET_D2		14
#تعریف_کړه AARCH64_RET_D1		15

#تعریف_کړه AARCH64_RET_Q4		16
#تعریف_کړه AARCH64_RET_Q3		17
#تعریف_کړه AARCH64_RET_Q2		18
#تعریف_کړه AARCH64_RET_Q1		19

/* Note that each of the sub-64-bit integers gets two entries.  */
#تعریف_کړه AARCH64_RET_UINT8	20
#تعریف_کړه AARCH64_RET_UINT16	22
#تعریف_کړه AARCH64_RET_UINT32	24

#تعریف_کړه AARCH64_RET_SINT8	26
#تعریف_کړه AARCH64_RET_SINT16	28
#تعریف_کړه AARCH64_RET_SINT32	30

#تعریف_کړه AARCH64_RET_ماسک	31

#تعریف_کړه AARCH64_RET_دننه_MEM	(1 << 5)
#تعریف_کړه AARCH64_RET_NEED_COPY	(1 << 6)

#تعریف_کړه AARCH64_FLAG_ARG_V_بټ	7
#تعریف_کړه AARCH64_FLAG_ARG_V	(1 << AARCH64_FLAG_ARG_V_بټ)
#تعریف_کړه AARCH64_FLAG_VARARG	(1 << 8)

#تعریف_کړه N_X_ARG_REG		8
#تعریف_کړه N_V_ARG_REG		8
#تعریف_کړه CALL_CONTEXT_اندازه	(N_V_ARG_REG * 16 + N_X_ARG_REG * 8)

#إذا defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 16K is chosen to
 * cover the base page sizes of 4K and 16K.
 */
#تعریف_کړه AARCH64_TRAMP_MAP_ښويدنه	14
#تعریف_کړه AARCH64_TRAMP_MAP_اندازه	(1 << AARCH64_TRAMP_MAP_ښويدنه)
#تعریف_کړه AARCH64_TRAMP_اندازه	32

#د_که_پای

#که_تعریف_شوی __AARCH64EB__
# define BE(X)	X
#وإلا
# define BE(X)	0
#د_که_پای
/* Helpers for writing assembly compatible with arm ptr auth */
#که_تعریف_شوی LIBFFI_ASM

  #إذا defined(HAVE_ARM64E_PTRAUTH)
  /* ARM64E ABI For Darwin */
  #تعریف_کړه SIGN_LR pacibsp
  #تعریف_کړه SIGN_LR_WITH_REG(x) pacib lr, x
  #تعریف_کړه AUTH_LR_AND_RET retab
  #تعریف_کړه AUTH_LR_WITH_REG(x) autib lr, x
  #تعریف_کړه BRANCH_AND_LINK_منزل_REG blraaz
  #تعریف_کړه SIGN_LR_LINUX_ONLY
  #تعریف_کړه BRANCH_منزل_REG braaz
  #تعریف_کړه PAC_CFI_کړکۍ_SAVE
  #تعریف_کړه AARCH64_اشاره_ګر_AUTH 0
  /* Linux PAC Support */
  #وإلاإذا defined(__ARM_FEATURE_PAC_DEFAULT)
    #تعریف_کړه AARCH64_اشاره_ګر_AUTH (1 << 1)
    #تعریف_کړه PAC_CFI_کړکۍ_SAVE cfi_کړکۍ_save
    #تعریف_کړه TMP_REG x9
    #تعریف_کړه BRANCH_منزل_REG br
    #تعریف_کړه BRANCH_AND_LINK_منزل_REG blr
	#تعریف_کړه SIGN_LR_LINUX_ONLY SIGN_LR
    /* Which key to sign with? */
    #إذا (__ARM_FEATURE_PAC_DEFAULT & 1) == 1
      /* Signed with A-key */
      #تعریف_کړه SIGN_LR            hint #25  /* paciasp */
      #تعریف_کړه AUTH_LR            hint #29  /* autiasp */
    #وإلا
      /* Signed with B-key */
      #تعریف_کړه SIGN_LR            hint #27  /* pacibsp */
      #تعریف_کړه AUTH_LR            hint #31  /* autibsp */
    #د_که_پای /* __ARM_FEATURE_PAC_DEFAULT */
    #تعریف_کړه AUTH_LR_WITH_REG(x) _auth_lr_with_reg x
.macro _auth_lr_with_reg modifier
    mov TMP_REG, sp
    mov sp, \modifier
    AUTH_LR
    mov sp, TMP_REG
.endm
  #تعریف_کړه SIGN_LR_WITH_REG(x) _sign_lr_with_reg x
.macro _sign_lr_with_reg modifier
    mov TMP_REG, sp
    mov sp, \modifier
    SIGN_LR
    mov sp, TMP_REG
.endm
  #تعریف_کړه AUTH_LR_AND_RET _auth_lr_and_ret modifier
.macro _auth_lr_and_ret modifier
    AUTH_LR
    ret
.endm
  #تعریف_لغوه TMP_REG

  /* No Pointer Auth */
  #وإلا
    #تعریف_کړه SIGN_LR
    #تعریف_کړه SIGN_LR_WITH_REG(x)
    #تعریف_کړه AUTH_LR_AND_RET ret
    #تعریف_کړه AUTH_LR_WITH_REG(x)
    #تعریف_کړه BRANCH_AND_LINK_منزل_REG blr
    #تعریف_کړه SIGN_LR_LINUX_ONLY
    #تعریف_کړه BRANCH_منزل_REG br
    #تعریف_کړه PAC_CFI_کړکۍ_SAVE
    #تعریف_کړه AARCH64_اشاره_ګر_AUTH 0
  #د_که_پای /* HAVE_ARM64E_PTRAUTH */
#د_که_پای /* LIBFFI_ASM */
