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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_VOID	0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_INT64	1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_INT128	2

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_UNUSED3	3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_UNUSED4	4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_UNUSED5	5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_UNUSED6	6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_UNUSED7	7

/* Note that FFI_TYPE_FLOAT == 2, _DOUBLE == 3, _LONGDOUBLE == 4,
   so _S4 through _Q1 are layed out as (TYPE * 4) + (4 - COUNT).  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_S4		8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_S3		9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_S2		10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_S1		11

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_D4		12
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_D3		13
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_D2		14
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_D1		15

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_Q4		16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_Q3		17
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_Q2		18
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_Q1		19

/* Note that each of the sub-64-bit integers gets two entries.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_UINT8	20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_UINT16	22
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_UINT32	24

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_SINT8	26
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_SINT16	28
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_SINT32	30

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_ਨਕਾਬ	31

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_ਵਿੱਚ_MEM	(1 << 5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_RET_NEED_COPY	(1 << 6)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_FLAG_ARG_V_ਦੋਆਧਾਰੀ_ਅੰਕ	7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_FLAG_ARG_V	(1 << AARCH64_FLAG_ARG_V_ਦੋਆਧਾਰੀ_ਅੰਕ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_FLAG_VARARG	(1 << 8)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ N_X_ARG_REG		8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ N_V_ARG_REG		8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CALL_CONTEXT_ਮਾਪ	(N_V_ARG_REG * 16 + N_X_ARG_REG * 8)

#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 16K is chosen to
 * cover the base page sizes of 4K and 16K.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_TRAMP_MAP_ਖਿਸਕਾਓ	14
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_TRAMP_MAP_ਮਾਪ	(1 << AARCH64_TRAMP_MAP_ਖਿਸਕਾਓ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_TRAMP_ਮਾਪ	32

#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __AARCH64EB__
# define BE(X)	X
#ਨਹੀਂਤਾਂ
# define BE(X)	0
#ਜੇ_ਅੰਤ
/* Helpers for writing assembly compatible with arm ptr auth */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LIBFFI_ASM

  #ਜੇ defined(HAVE_ARM64E_PTRAUTH)
  /* ARM64E ABI For Darwin */
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIGN_LR pacibsp
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIGN_LR_WITH_REG(x) pacib lr, x
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AUTH_LR_AND_RET retab
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AUTH_LR_WITH_REG(x) autib lr, x
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BRANCH_AND_LINK_ਮੰਜ਼ਿਲ_REG blraaz
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIGN_LR_LINUX_ONLY
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BRANCH_ਮੰਜ਼ਿਲ_REG braaz
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAC_CFI_ਖਿੜਕੀ_SAVE
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_ਸੂਚਕ_AUTH 0
  /* Linux PAC Support */
  #ਜਾਂਜੇ defined(__ARM_FEATURE_PAC_DEFAULT)
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_ਸੂਚਕ_AUTH (1 << 1)
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAC_CFI_ਖਿੜਕੀ_SAVE cfi_ਖਿੜਕੀ_save
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TMP_REG x9
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BRANCH_ਮੰਜ਼ਿਲ_REG br
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BRANCH_AND_LINK_ਮੰਜ਼ਿਲ_REG blr
	#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIGN_LR_LINUX_ONLY SIGN_LR
    /* Which key to sign with? */
    #ਜੇ (__ARM_FEATURE_PAC_DEFAULT & 1) == 1
      /* Signed with A-key */
      #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIGN_LR            hint #25  /* paciasp */
      #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AUTH_LR            hint #29  /* autiasp */
    #ਨਹੀਂਤਾਂ
      /* Signed with B-key */
      #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIGN_LR            hint #27  /* pacibsp */
      #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AUTH_LR            hint #31  /* autibsp */
    #ਜੇ_ਅੰਤ /* __ARM_FEATURE_PAC_DEFAULT */
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AUTH_LR_WITH_REG(x) _auth_lr_with_reg x
.macro _auth_lr_with_reg modifier
    mov TMP_REG, sp
    mov sp, \modifier
    AUTH_LR
    mov sp, TMP_REG
.endm
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIGN_LR_WITH_REG(x) _sign_lr_with_reg x
.macro _sign_lr_with_reg modifier
    mov TMP_REG, sp
    mov sp, \modifier
    SIGN_LR
    mov sp, TMP_REG
.endm
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AUTH_LR_AND_RET _auth_lr_and_ret modifier
.macro _auth_lr_and_ret modifier
    AUTH_LR
    ret
.endm
  #ਪਰਿਭਾਸ਼ਾ_ਰੱਦ TMP_REG

  /* No Pointer Auth */
  #ਨਹੀਂਤਾਂ
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIGN_LR
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIGN_LR_WITH_REG(x)
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AUTH_LR_AND_RET ret
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AUTH_LR_WITH_REG(x)
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BRANCH_AND_LINK_ਮੰਜ਼ਿਲ_REG blr
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIGN_LR_LINUX_ONLY
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BRANCH_ਮੰਜ਼ਿਲ_REG br
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAC_CFI_ਖਿੜਕੀ_SAVE
    #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AARCH64_ਸੂਚਕ_AUTH 0
  #ਜੇ_ਅੰਤ /* HAVE_ARM64E_PTRAUTH */
#ਜੇ_ਅੰਤ /* LIBFFI_ASM */
