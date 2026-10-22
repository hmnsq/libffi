/* -----------------------------------------------------------------------
   ffi_powerpc.h - Copyright (C) 2013 IBM
                   Copyright (C) 2011, 2026 Anthony Green
                   Copyright (C) 2011 Kyle Moffett
                   Copyright (C) 2008 Red Hat, Inc
                   Copyright (C) 2007, 2008 Free Software Foundation, Inc
                   Copyright (c) 1998 Geoffrey Keating

   PowerPC Foreign Function Interface

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

/*
 * Closure jump table indexes
 */

/* Common to ppc32 and ppc64 */
#تعریف_کرو PPC_LD_NONE		0
#تعریف_کرو PPC_LD_R3		1
#تعریف_کرو PPC_LD_R3R4		2
#تعریف_کرو PPC_LD_F32		3
#تعریف_کرو PPC_LD_F64		4
#تعریف_کرو PPC_LD_F128		5
#تعریف_کرو PPC_LD_U8		6
#تعریف_کرو PPC_LD_S8		7
#تعریف_کرو PPC_LD_U16		8
#تعریف_کرو PPC_LD_S16		9

#جے_بنا_تعریف POWERPC64

#تعریف_کرو PPC_LD_U32		PPC_LD_R3
#تعریف_کرو PPC_LD_S32		PPC_LD_R3
#تعریف_کرو PPC_LD_PTR		PPC_LD_R3
#تعریف_کرو PPC_LD_I64		PPC_LD_R3R4

/* Needed for soft-float long-double. */
#تعریف_کرو PPC32_LD_R3R6		10

/* Needed for FFI_SYSV small structure returns.  */
#جے_تعریف_شدہ __LITTLE_ENDIAN__
#تعریف_کرو PPC32_SYSV_LD_STRUCT_3	PPC32_LD_R3
#تعریف_کرو PPC32_SYSV_LD_STRUCT_5	PPC32_LD_R3R4
#تعریف_کرو PPC32_SYSV_LD_STRUCT_6	PPC32_LD_R3R4
#تعریف_کرو PPC32_SYSV_LD_STRUCT_7	PPC32_LD_R3R4
#وإلا
#تعریف_کرو PPC32_SYSV_LD_STRUCT_3	11
#تعریف_کرو PPC32_SYSV_LD_STRUCT_5	12
#تعریف_کرو PPC32_SYSV_LD_STRUCT_6	13
#تعریف_کرو PPC32_SYSV_LD_STRUCT_7	14
#جے_انت

#وإلا /* POWERPC64 */

#تعریف_کرو PPC_LD_U32		10
#تعریف_کرو PPC_LD_S32		11
#تعریف_کرو PPC_LD_PTR		PPC_LD_R3
#تعریف_کرو PPC_LD_I64		PPC_LD_R3

/* Used by ELFv2 for homogenous structure returns.  */
#تعریف_کرو PPC64_LD_سمتیہ		12
#تعریف_کرو PPC64_LD_سمتیہ_HOMOG	13
#تعریف_کرو PPC64_LD_FLOAT_HOMOG	14
#تعریف_کرو PPC64_LD_DOUBLE_HOMOG	15
#جے_تعریف_شدہ __LITTLE_ENDIAN__
#تعریف_کرو PPC64_LD_STRUCT_3	PPC_LD_U32
#تعریف_کرو PPC64_LD_STRUCT_5	PPC_LD_I64
#تعریف_کرو PPC64_LD_STRUCT_6	PPC_LD_I64
#تعریف_کرو PPC64_LD_STRUCT_7	PPC_LD_I64
#وإلا
#تعریف_کرو PPC64_LD_STRUCT_3	16
#تعریف_کرو PPC64_LD_STRUCT_5	17
#تعریف_کرو PPC64_LD_STRUCT_6	18
#تعریف_کرو PPC64_LD_STRUCT_7	19
#جے_انت

#جے_انت /* POWERPC64 */

#جے_بنا_تعریف LIBFFI_ASM

تعداد {
  /* The assembly depends on these exact flags.  */
  /* These go in cr7 */
  FLAG_RETURNS_SMST     = 1 << (31-31), /* Used for FFI_SYSV small structs.  */
  FLAG_RETURNS_NOTHING  = 1 << (31-30),
  FLAG_RETURNS_FP       = 1 << (31-29),
  FLAG_RETURNS_VEC      = 1 << (31-28),

  /* These go in cr6 */
  FLAG_RETURNS_64_بٹ   = 1 << (31-27),
  FLAG_RETURNS_128_بٹ  = 1 << (31-26),

  FLAG_COMPAT           = 1 << (31- 8), /* Not used by assembly */

  /* These go in cr1 */
  FLAG_ARG_NEEDS_COPY   = 1 << (31- 7), /* Used by sysv code */
  FLAG_ARG_NEEDS_PSAVE  = FLAG_ARG_NEEDS_COPY, /* Used by linux64 code */
  FLAG_FP_ARGUMENTS     = 1 << (31- 6), /* cr1.eq; specified by ABI */
  FLAG_4_GPR_ARGUMENTS  = 1 << (31- 5),
  FLAG_RETVAL_REFERENCE = 1 << (31- 4),
  FLAG_VEC_ARGUMENTS    = 1 << (31- 3),
};

تعريفنوع اتحاد
{
  عائم f;
  مزدوج d;
} ffi_dblfl;

#إذا defined(__FLOAT128_TYPE__) && defined(__HAVE_FLOAT128)
تعريفنوع _Float128 float128;
#وإلاإذا defined(__FLOAT128__)
تعريفنوع __float128 float128;
#وإلا
تعريفنوع حرف float128[16] __attribute__((aligned(16)));
#جے_انت

فراغ FFI_HIDDEN ffi_گھیرا_SYSV (فراغ);
فراغ FFI_HIDDEN ffi_go_گھیرا_sysv (فراغ);
فراغ FFI_HIDDEN ffi_call_SYSV(extended_cif *, فراغ (*)(فراغ), فراغ *,
			      بنانشانی, فراغ *, صحيح);

فراغ FFI_HIDDEN ffi_prep_types_sysv (ffi_abi);
ffi_حالت FFI_HIDDEN ffi_prep_cif_sysv (ffi_cif *);
ffi_حالت FFI_HIDDEN ffi_prep_گھیرا_loc_sysv (ffi_گھیرا *,
						 ffi_cif *,
						 فراغ (*) (ffi_cif *, فراغ *,
							   فراغ **, فراغ *),
						 فراغ *, فراغ *);
صحيح FFI_HIDDEN ffi_گھیرا_helper_SYSV (ffi_cif *,
					فراغ (*) (ffi_cif *, فراغ *,
						  فراغ **, فراغ *),
					فراغ *, فراغ *, بنانشانی لما *,
					ffi_dblfl *, بنانشانی لما *);

فراغ FFI_HIDDEN ffi_call_LINUX64(extended_cif *, فراغ (*) (فراغ), فراغ *,
				 بنانشانی لما, فراغ *, لما);
فراغ FFI_HIDDEN ffi_گھیرا_LINUX64 (فراغ);
فراغ FFI_HIDDEN ffi_go_گھیرا_linux64 (فراغ);

فراغ FFI_HIDDEN ffi_prep_types_linux64 (ffi_abi);
ffi_حالت FFI_HIDDEN ffi_prep_cif_linux64 (ffi_cif *);
ffi_حالت FFI_HIDDEN ffi_prep_cif_linux64_var (ffi_cif *, بنانشانی صحيح,
						بنانشانی صحيح);
فراغ FFI_HIDDEN ffi_prep_args64 (extended_cif *, بنانشانی لما *ثابت);
ffi_حالت FFI_HIDDEN ffi_prep_گھیرا_loc_linux64 (ffi_گھیرا *, ffi_cif *,
						    فراغ (*) (ffi_cif *, فراغ *,
							      فراغ **, فراغ *),
						    فراغ *, فراغ *);
صحيح FFI_HIDDEN ffi_گھیرا_helper_LINUX64 (ffi_cif *,
					   فراغ (*) (ffi_cif *, فراغ *,
						     فراغ **, فراغ *),
					   فراغ *, فراغ *,
					   بنانشانی لما *, ffi_dblfl *,
					   float128 *);

#جے_انت /* !LIBFFI_ASM */
