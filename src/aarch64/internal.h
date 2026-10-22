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

#నిర్వచించు AARCH64_RET_VOID	0
#నిర్వచించు AARCH64_RET_INT64	1
#నిర్వచించు AARCH64_RET_INT128	2

#నిర్వచించు AARCH64_RET_UNUSED3	3
#నిర్వచించు AARCH64_RET_UNUSED4	4
#నిర్వచించు AARCH64_RET_UNUSED5	5
#నిర్వచించు AARCH64_RET_UNUSED6	6
#నిర్వచించు AARCH64_RET_UNUSED7	7

/* Note that FFI_TYPE_FLOAT == 2, _DOUBLE == 3, _LONGDOUBLE == 4,
   so _S4 through _Q1 are layed out as (TYPE * 4) + (4 - COUNT).  */
#నిర్వచించు AARCH64_RET_S4		8
#నిర్వచించు AARCH64_RET_S3		9
#నిర్వచించు AARCH64_RET_S2		10
#నిర్వచించు AARCH64_RET_S1		11

#నిర్వచించు AARCH64_RET_D4		12
#నిర్వచించు AARCH64_RET_D3		13
#నిర్వచించు AARCH64_RET_D2		14
#నిర్వచించు AARCH64_RET_D1		15

#నిర్వచించు AARCH64_RET_Q4		16
#నిర్వచించు AARCH64_RET_Q3		17
#నిర్వచించు AARCH64_RET_Q2		18
#నిర్వచించు AARCH64_RET_Q1		19

/* Note that each of the sub-64-bit integers gets two entries.  */
#నిర్వచించు AARCH64_RET_UINT8	20
#నిర్వచించు AARCH64_RET_UINT16	22
#నిర్వచించు AARCH64_RET_UINT32	24

#నిర్వచించు AARCH64_RET_SINT8	26
#నిర్వచించు AARCH64_RET_SINT16	28
#నిర్వచించు AARCH64_RET_SINT32	30

#నిర్వచించు AARCH64_RET_ముసుగు	31

#నిర్వచించు AARCH64_RET_లోపల_MEM	(1 << 5)
#నిర్వచించు AARCH64_RET_NEED_COPY	(1 << 6)

#నిర్వచించు AARCH64_FLAG_ARG_V_బిట్టు	7
#నిర్వచించు AARCH64_FLAG_ARG_V	(1 << AARCH64_FLAG_ARG_V_బిట్టు)
#నిర్వచించు AARCH64_FLAG_VARARG	(1 << 8)

#నిర్వచించు N_X_ARG_REG		8
#నిర్వచించు N_V_ARG_REG		8
#నిర్వచించు CALL_CONTEXT_కొలత	(N_V_ARG_REG * 16 + N_X_ARG_REG * 8)

#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
/*
 * For the trampoline code table mapping, a mapping size of 16K is chosen to
 * cover the base page sizes of 4K and 16K.
 */
#నిర్వచించు AARCH64_TRAMP_MAP_జరుపు	14
#నిర్వచించు AARCH64_TRAMP_MAP_కొలత	(1 << AARCH64_TRAMP_MAP_జరుపు)
#నిర్వచించు AARCH64_TRAMP_కొలత	32

#అయితే_ముగింపు

#నిర్వచితమైతే __AARCH64EB__
# define BE(X)	X
#లేకపోతే
# define BE(X)	0
#అయితే_ముగింపు
/* Helpers for writing assembly compatible with arm ptr auth */
#నిర్వచితమైతే LIBFFI_ASM

  #ఒకవేళ defined(HAVE_ARM64E_PTRAUTH)
  /* ARM64E ABI For Darwin */
  #నిర్వచించు SIGN_LR pacibsp
  #నిర్వచించు SIGN_LR_WITH_REG(x) pacib lr, x
  #నిర్వచించు AUTH_LR_AND_RET retab
  #నిర్వచించు AUTH_LR_WITH_REG(x) autib lr, x
  #నిర్వచించు BRANCH_AND_LINK_గమ్యం_REG blraaz
  #నిర్వచించు SIGN_LR_LINUX_ONLY
  #నిర్వచించు BRANCH_గమ్యం_REG braaz
  #నిర్వచించు PAC_CFI_కిటికీ_SAVE
  #నిర్వచించు AARCH64_సూచిక_AUTH 0
  /* Linux PAC Support */
  #లేదావేళ defined(__ARM_FEATURE_PAC_DEFAULT)
    #నిర్వచించు AARCH64_సూచిక_AUTH (1 << 1)
    #నిర్వచించు PAC_CFI_కిటికీ_SAVE cfi_కిటికీ_save
    #నిర్వచించు TMP_REG x9
    #నిర్వచించు BRANCH_గమ్యం_REG br
    #నిర్వచించు BRANCH_AND_LINK_గమ్యం_REG blr
	#నిర్వచించు SIGN_LR_LINUX_ONLY SIGN_LR
    /* Which key to sign with? */
    #ఒకవేళ (__ARM_FEATURE_PAC_DEFAULT & 1) == 1
      /* Signed with A-key */
      #నిర్వచించు SIGN_LR            hint #25  /* paciasp */
      #నిర్వచించు AUTH_LR            hint #29  /* autiasp */
    #లేకపోతే
      /* Signed with B-key */
      #నిర్వచించు SIGN_LR            hint #27  /* pacibsp */
      #నిర్వచించు AUTH_LR            hint #31  /* autibsp */
    #అయితే_ముగింపు /* __ARM_FEATURE_PAC_DEFAULT */
    #నిర్వచించు AUTH_LR_WITH_REG(x) _auth_lr_with_reg x
.macro _auth_lr_with_reg modifier
    mov TMP_REG, sp
    mov sp, \modifier
    AUTH_LR
    mov sp, TMP_REG
.endm
  #నిర్వచించు SIGN_LR_WITH_REG(x) _sign_lr_with_reg x
.macro _sign_lr_with_reg modifier
    mov TMP_REG, sp
    mov sp, \modifier
    SIGN_LR
    mov sp, TMP_REG
.endm
  #నిర్వచించు AUTH_LR_AND_RET _auth_lr_and_ret modifier
.macro _auth_lr_and_ret modifier
    AUTH_LR
    ret
.endm
  #నిర్వచనం_రద్దు TMP_REG

  /* No Pointer Auth */
  #లేకపోతే
    #నిర్వచించు SIGN_LR
    #నిర్వచించు SIGN_LR_WITH_REG(x)
    #నిర్వచించు AUTH_LR_AND_RET ret
    #నిర్వచించు AUTH_LR_WITH_REG(x)
    #నిర్వచించు BRANCH_AND_LINK_గమ్యం_REG blr
    #నిర్వచించు SIGN_LR_LINUX_ONLY
    #నిర్వచించు BRANCH_గమ్యం_REG br
    #నిర్వచించు PAC_CFI_కిటికీ_SAVE
    #నిర్వచించు AARCH64_సూచిక_AUTH 0
  #అయితే_ముగింపు /* HAVE_ARM64E_PTRAUTH */
#అయితే_ముగింపు /* LIBFFI_ASM */
