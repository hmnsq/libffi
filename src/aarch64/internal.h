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

#घोषणा AARCH64_RET_VOID	0
#घोषणा AARCH64_RET_INT64	1
#घोषणा AARCH64_RET_INT128	2

#घोषणा AARCH64_RET_UNUSED3	3
#घोषणा AARCH64_RET_UNUSED4	4
#घोषणा AARCH64_RET_UNUSED5	5
#घोषणा AARCH64_RET_UNUSED6	6
#घोषणा AARCH64_RET_UNUSED7	7

/* Note that FFI_TYPE_FLOAT == 2, _DOUBLE == 3, _LONGDOUBLE == 4,
   so _S4 through _Q1 are layed out as (TYPE * 4) + (4 - COUNT).  */
#घोषणा AARCH64_RET_S4		8
#घोषणा AARCH64_RET_S3		9
#घोषणा AARCH64_RET_S2		10
#घोषणा AARCH64_RET_S1		11

#घोषणा AARCH64_RET_D4		12
#घोषणा AARCH64_RET_D3		13
#घोषणा AARCH64_RET_D2		14
#घोषणा AARCH64_RET_D1		15

#घोषणा AARCH64_RET_Q4		16
#घोषणा AARCH64_RET_Q3		17
#घोषणा AARCH64_RET_Q2		18
#घोषणा AARCH64_RET_Q1		19

/* Note that each of the sub-64-bit integers gets two entries.  */
#घोषणा AARCH64_RET_UINT8	20
#घोषणा AARCH64_RET_UINT16	22
#घोषणा AARCH64_RET_UINT32	24

#घोषणा AARCH64_RET_SINT8	26
#घोषणा AARCH64_RET_SINT16	28
#घोषणा AARCH64_RET_SINT32	30

#घोषणा AARCH64_RET_MASK	31

#घोषणा AARCH64_RET_IN_MEM	(1 << 5)
#घोषणा AARCH64_RET_NEED_COPY	(1 << 6)

#घोषणा AARCH64_FLAG_ARG_V_BIT	7
#घोषणा AARCH64_FLAG_ARG_V	(1 << AARCH64_FLAG_ARG_V_BIT)
#घोषणा AARCH64_FLAG_VARARG	(1 << 8)

#घोषणा N_X_ARG_REG		8
#घोषणा N_V_ARG_REG		8
#घोषणा CALL_CONTEXT_आकार	(N_V_ARG_REG * 16 + N_X_ARG_REG * 8)

#अगर defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 16K is chosen to
 * cover the base page sizes of 4K and 16K.
 */
#घोषणा AARCH64_TRAMP_MAP_SHIFT	14
#घोषणा AARCH64_TRAMP_MAP_आकार	(1 << AARCH64_TRAMP_MAP_SHIFT)
#घोषणा AARCH64_TRAMP_आकार	32

#पूर्ण_अगर

#अगर_घोषित __AARCH64EB__
# define BE(X)	X
#अन्यथा
# define BE(X)	0
#पूर्ण_अगर
/* Helpers for writing assembly compatible with arm ptr auth */
#अगर_घोषित LIBFFI_ASM

  #अगर defined(HAVE_ARM64E_PTRAUTH)
  /* ARM64E ABI For Darwin */
  #घोषणा SIGN_LR pacibsp
  #घोषणा SIGN_LR_WITH_REG(x) pacib lr, x
  #घोषणा AUTH_LR_AND_RET retab
  #घोषणा AUTH_LR_WITH_REG(x) autib lr, x
  #घोषणा BRANCH_AND_LINK_TO_REG blraaz
  #घोषणा SIGN_LR_LINUX_ONLY
  #घोषणा BRANCH_TO_REG braaz
  #घोषणा PAC_CFI_WINDOW_SAVE
  #घोषणा AARCH64_POINTER_AUTH 0
  /* Linux PAC Support */
  #या_अगर defined(__ARM_FEATURE_PAC_DEFAULT)
    #घोषणा AARCH64_POINTER_AUTH (1 << 1)
    #घोषणा PAC_CFI_WINDOW_SAVE cfi_window_save
    #घोषणा TMP_REG x9
    #घोषणा BRANCH_TO_REG br
    #घोषणा BRANCH_AND_LINK_TO_REG blr
	#घोषणा SIGN_LR_LINUX_ONLY SIGN_LR
    /* Which key to sign with? */
    #अगर (__ARM_FEATURE_PAC_DEFAULT & 1) == 1
      /* Signed with A-key */
      #घोषणा SIGN_LR            hint #25  /* paciasp */
      #घोषणा AUTH_LR            hint #29  /* autiasp */
    #अन्यथा
      /* Signed with B-key */
      #घोषणा SIGN_LR            hint #27  /* pacibsp */
      #घोषणा AUTH_LR            hint #31  /* autibsp */
    #पूर्ण_अगर /* __ARM_FEATURE_PAC_DEFAULT */
    #घोषणा AUTH_LR_WITH_REG(x) _auth_lr_with_reg x
.macro _auth_lr_with_reg modifier
    mov TMP_REG, sp
    mov sp, \modifier
    AUTH_LR
    mov sp, TMP_REG
.endm
  #घोषणा SIGN_LR_WITH_REG(x) _sign_lr_with_reg x
.macro _sign_lr_with_reg modifier
    mov TMP_REG, sp
    mov sp, \modifier
    SIGN_LR
    mov sp, TMP_REG
.endm
  #घोषणा AUTH_LR_AND_RET _auth_lr_and_ret modifier
.macro _auth_lr_and_ret modifier
    AUTH_LR
    ret
.endm
  #अघोषित TMP_REG

  /* No Pointer Auth */
  #अन्यथा
    #घोषणा SIGN_LR
    #घोषणा SIGN_LR_WITH_REG(x)
    #घोषणा AUTH_LR_AND_RET ret
    #घोषणा AUTH_LR_WITH_REG(x)
    #घोषणा BRANCH_AND_LINK_TO_REG blr
    #घोषणा SIGN_LR_LINUX_ONLY
    #घोषणा BRANCH_TO_REG br
    #घोषणा PAC_CFI_WINDOW_SAVE
    #घोषणा AARCH64_POINTER_AUTH 0
  #पूर्ण_अगर /* HAVE_ARM64E_PTRAUTH */
#पूर्ण_अगर /* LIBFFI_ASM */
