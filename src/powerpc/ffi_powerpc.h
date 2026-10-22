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
#وصف_ڪريو PPC_LD_NONE		0
#وصف_ڪريو PPC_LD_R3		1
#وصف_ڪريو PPC_LD_R3R4		2
#وصف_ڪريو PPC_LD_F32		3
#وصف_ڪريو PPC_LD_F64		4
#وصف_ڪريو PPC_LD_F128		5
#وصف_ڪريو PPC_LD_U8		6
#وصف_ڪريو PPC_LD_S8		7
#وصف_ڪريو PPC_LD_U16		8
#وصف_ڪريو PPC_LD_S16		9

#جيڪڏهن_وصف_نه_ٿيل POWERPC64

#وصف_ڪريو PPC_LD_U32		PPC_LD_R3
#وصف_ڪريو PPC_LD_S32		PPC_LD_R3
#وصف_ڪريو PPC_LD_PTR		PPC_LD_R3
#وصف_ڪريو PPC_LD_I64		PPC_LD_R3R4

/* Needed for soft-float long-double. */
#وصف_ڪريو PPC32_LD_R3R6		10

/* Needed for FFI_SYSV small structure returns.  */
#جيڪڏهن_وصف_ٿيل __LITTLE_ENDIAN__
#وصف_ڪريو PPC32_SYSV_LD_STRUCT_3	PPC32_LD_R3
#وصف_ڪريو PPC32_SYSV_LD_STRUCT_5	PPC32_LD_R3R4
#وصف_ڪريو PPC32_SYSV_LD_STRUCT_6	PPC32_LD_R3R4
#وصف_ڪريو PPC32_SYSV_LD_STRUCT_7	PPC32_LD_R3R4
#وإلا
#وصف_ڪريو PPC32_SYSV_LD_STRUCT_3	11
#وصف_ڪريو PPC32_SYSV_LD_STRUCT_5	12
#وصف_ڪريو PPC32_SYSV_LD_STRUCT_6	13
#وصف_ڪريو PPC32_SYSV_LD_STRUCT_7	14
#جيڪڏهن_پورو

#وإلا /* POWERPC64 */

#وصف_ڪريو PPC_LD_U32		10
#وصف_ڪريو PPC_LD_S32		11
#وصف_ڪريو PPC_LD_PTR		PPC_LD_R3
#وصف_ڪريو PPC_LD_I64		PPC_LD_R3

/* Used by ELFv2 for homogenous structure returns.  */
#وصف_ڪريو PPC64_LD_ويڪٽر		12
#وصف_ڪريو PPC64_LD_ويڪٽر_HOMOG	13
#وصف_ڪريو PPC64_LD_FLOAT_HOMOG	14
#وصف_ڪريو PPC64_LD_DOUBLE_HOMOG	15
#جيڪڏهن_وصف_ٿيل __LITTLE_ENDIAN__
#وصف_ڪريو PPC64_LD_STRUCT_3	PPC_LD_U32
#وصف_ڪريو PPC64_LD_STRUCT_5	PPC_LD_I64
#وصف_ڪريو PPC64_LD_STRUCT_6	PPC_LD_I64
#وصف_ڪريو PPC64_LD_STRUCT_7	PPC_LD_I64
#وإلا
#وصف_ڪريو PPC64_LD_STRUCT_3	16
#وصف_ڪريو PPC64_LD_STRUCT_5	17
#وصف_ڪريو PPC64_LD_STRUCT_6	18
#وصف_ڪريو PPC64_LD_STRUCT_7	19
#جيڪڏهن_پورو

#جيڪڏهن_پورو /* POWERPC64 */

#جيڪڏهن_وصف_نه_ٿيل LIBFFI_ASM

تعداد {
  /* The assembly depends on these exact flags.  */
  /* These go in cr7 */
  FLAG_RETURNS_SMST     = 1 << (31-31), /* Used for FFI_SYSV small structs.  */
  FLAG_RETURNS_NOTHING  = 1 << (31-30),
  FLAG_RETURNS_FP       = 1 << (31-29),
  FLAG_RETURNS_VEC      = 1 << (31-28),

  /* These go in cr6 */
  FLAG_RETURNS_64_بٽ   = 1 << (31-27),
  FLAG_RETURNS_128_بٽ  = 1 << (31-26),

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
#جيڪڏهن_پورو

فراغ FFI_HIDDEN ffi_گهيرو_SYSV (فراغ);
فراغ FFI_HIDDEN ffi_go_گهيرو_sysv (فراغ);
فراغ FFI_HIDDEN ffi_call_SYSV(extended_cif *, فراغ (*)(فراغ), فراغ *,
			      بنانشان, فراغ *, صحيح);

فراغ FFI_HIDDEN ffi_prep_types_sysv (ffi_abi);
ffi_حالت FFI_HIDDEN ffi_prep_cif_sysv (ffi_cif *);
ffi_حالت FFI_HIDDEN ffi_prep_گهيرو_loc_sysv (ffi_گهيرو *,
						 ffi_cif *,
						 فراغ (*) (ffi_cif *, فراغ *,
							   فراغ **, فراغ *),
						 فراغ *, فراغ *);
صحيح FFI_HIDDEN ffi_گهيرو_helper_SYSV (ffi_cif *,
					فراغ (*) (ffi_cif *, فراغ *,
						  فراغ **, فراغ *),
					فراغ *, فراغ *, بنانشان ڊگهو *,
					ffi_dblfl *, بنانشان ڊگهو *);

فراغ FFI_HIDDEN ffi_call_LINUX64(extended_cif *, فراغ (*) (فراغ), فراغ *,
				 بنانشان ڊگهو, فراغ *, ڊگهو);
فراغ FFI_HIDDEN ffi_گهيرو_LINUX64 (فراغ);
فراغ FFI_HIDDEN ffi_go_گهيرو_linux64 (فراغ);

فراغ FFI_HIDDEN ffi_prep_types_linux64 (ffi_abi);
ffi_حالت FFI_HIDDEN ffi_prep_cif_linux64 (ffi_cif *);
ffi_حالت FFI_HIDDEN ffi_prep_cif_linux64_var (ffi_cif *, بنانشان صحيح,
						بنانشان صحيح);
فراغ FFI_HIDDEN ffi_prep_args64 (extended_cif *, بنانشان ڊگهو *ثابت);
ffi_حالت FFI_HIDDEN ffi_prep_گهيرو_loc_linux64 (ffi_گهيرو *, ffi_cif *,
						    فراغ (*) (ffi_cif *, فراغ *,
							      فراغ **, فراغ *),
						    فراغ *, فراغ *);
صحيح FFI_HIDDEN ffi_گهيرو_helper_LINUX64 (ffi_cif *,
					   فراغ (*) (ffi_cif *, فراغ *,
						     فراغ **, فراغ *),
					   فراغ *, فراغ *,
					   بنانشان ڊگهو *, ffi_dblfl *,
					   float128 *);

#جيڪڏهن_پورو /* !LIBFFI_ASM */
