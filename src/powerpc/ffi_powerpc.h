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
#परिभाषय PPC_LD_NONE		0
#परिभाषय PPC_LD_R3		1
#परिभाषय PPC_LD_R3R4		2
#परिभाषय PPC_LD_F32		3
#परिभाषय PPC_LD_F64		4
#परिभाषय PPC_LD_F128		5
#परिभाषय PPC_LD_U8		6
#परिभाषय PPC_LD_S8		7
#परिभाषय PPC_LD_U16		8
#परिभाषय PPC_LD_S16		9

#यदि_अपरिभाषितम् POWERPC64

#परिभाषय PPC_LD_U32		PPC_LD_R3
#परिभाषय PPC_LD_S32		PPC_LD_R3
#परिभाषय PPC_LD_PTR		PPC_LD_R3
#परिभाषय PPC_LD_I64		PPC_LD_R3R4

/* Needed for soft-float long-double. */
#परिभाषय PPC32_LD_R3R6		10

/* Needed for FFI_SYSV small structure returns.  */
#यदि_परिभाषितम् __LITTLE_ENDIAN__
#परिभाषय PPC32_SYSV_LD_STRUCT_3	PPC32_LD_R3
#परिभाषय PPC32_SYSV_LD_STRUCT_5	PPC32_LD_R3R4
#परिभाषय PPC32_SYSV_LD_STRUCT_6	PPC32_LD_R3R4
#परिभाषय PPC32_SYSV_LD_STRUCT_7	PPC32_LD_R3R4
#अन्यथा
#परिभाषय PPC32_SYSV_LD_STRUCT_3	11
#परिभाषय PPC32_SYSV_LD_STRUCT_5	12
#परिभाषय PPC32_SYSV_LD_STRUCT_6	13
#परिभाषय PPC32_SYSV_LD_STRUCT_7	14
#यदि_अन्तः

#अन्यथा /* POWERPC64 */

#परिभाषय PPC_LD_U32		10
#परिभाषय PPC_LD_S32		11
#परिभाषय PPC_LD_PTR		PPC_LD_R3
#परिभाषय PPC_LD_I64		PPC_LD_R3

/* Used by ELFv2 for homogenous structure returns.  */
#परिभाषय PPC64_LD_सदिशम्		12
#परिभाषय PPC64_LD_सदिशम्_HOMOG	13
#परिभाषय PPC64_LD_FLOAT_HOMOG	14
#परिभाषय PPC64_LD_DOUBLE_HOMOG	15
#यदि_परिभाषितम् __LITTLE_ENDIAN__
#परिभाषय PPC64_LD_STRUCT_3	PPC_LD_U32
#परिभाषय PPC64_LD_STRUCT_5	PPC_LD_I64
#परिभाषय PPC64_LD_STRUCT_6	PPC_LD_I64
#परिभाषय PPC64_LD_STRUCT_7	PPC_LD_I64
#अन्यथा
#परिभाषय PPC64_LD_STRUCT_3	16
#परिभाषय PPC64_LD_STRUCT_5	17
#परिभाषय PPC64_LD_STRUCT_6	18
#परिभाषय PPC64_LD_STRUCT_7	19
#यदि_अन्तः

#यदि_अन्तः /* POWERPC64 */

#यदि_अपरिभाषितम् LIBFFI_ASM

गणना {
  /* The assembly depends on these exact flags.  */
  /* These go in cr7 */
  FLAG_RETURNS_SMST     = 1 << (31-31), /* Used for FFI_SYSV small structs.  */
  FLAG_RETURNS_NOTHING  = 1 << (31-30),
  FLAG_RETURNS_FP       = 1 << (31-29),
  FLAG_RETURNS_VEC      = 1 << (31-28),

  /* These go in cr6 */
  FLAG_RETURNS_64_द्वयङ्काः   = 1 << (31-27),
  FLAG_RETURNS_128_द्वयङ्काः  = 1 << (31-26),

  FLAG_COMPAT           = 1 << (31- 8), /* Not used by assembly */

  /* These go in cr1 */
  FLAG_ARG_NEEDS_COPY   = 1 << (31- 7), /* Used by sysv code */
  FLAG_ARG_NEEDS_PSAVE  = FLAG_ARG_NEEDS_COPY, /* Used by linux64 code */
  FLAG_FP_ARGUMENTS     = 1 << (31- 6), /* cr1.eq; specified by ABI */
  FLAG_4_GPR_ARGUMENTS  = 1 << (31- 5),
  FLAG_RETVAL_REFERENCE = 1 << (31- 4),
  FLAG_VEC_ARGUMENTS    = 1 << (31- 3),
};

प्रकारनिर्देश सङ्घ
{
  भिन्न f;
  द्विगुण d;
} ffi_dblfl;

#यदि defined(__FLOAT128_TYPE__) && defined(__HAVE_FLOAT128)
प्रकारनिर्देश _Float128 float128;
#नोचेद्यदि defined(__FLOAT128__)
प्रकारनिर्देश __float128 float128;
#अन्यथा
प्रकारनिर्देश वर्ण float128[16] __attribute__((aligned(16)));
#यदि_अन्तः

शून्य FFI_HIDDEN ffi_संवृतिः_SYSV (शून्य);
शून्य FFI_HIDDEN ffi_go_संवृतिः_sysv (शून्य);
शून्य FFI_HIDDEN ffi_call_SYSV(extended_cif *, शून्य (*)(शून्य), शून्य *,
			      अचिह्नित, शून्य *, पूर्णाङ्क);

शून्य FFI_HIDDEN ffi_prep_types_sysv (ffi_abi);
ffi_स्थितिः FFI_HIDDEN ffi_prep_cif_sysv (ffi_cif *);
ffi_स्थितिः FFI_HIDDEN ffi_prep_संवृतिः_loc_sysv (ffi_संवृतिः *,
						 ffi_cif *,
						 शून्य (*) (ffi_cif *, शून्य *,
							   शून्य **, शून्य *),
						 शून्य *, शून्य *);
पूर्णाङ्क FFI_HIDDEN ffi_संवृतिः_helper_SYSV (ffi_cif *,
					शून्य (*) (ffi_cif *, शून्य *,
						  शून्य **, शून्य *),
					शून्य *, शून्य *, अचिह्नित दीर्घ *,
					ffi_dblfl *, अचिह्नित दीर्घ *);

शून्य FFI_HIDDEN ffi_call_LINUX64(extended_cif *, शून्य (*) (शून्य), शून्य *,
				 अचिह्नित दीर्घ, शून्य *, दीर्घ);
शून्य FFI_HIDDEN ffi_संवृतिः_LINUX64 (शून्य);
शून्य FFI_HIDDEN ffi_go_संवृतिः_linux64 (शून्य);

शून्य FFI_HIDDEN ffi_prep_types_linux64 (ffi_abi);
ffi_स्थितिः FFI_HIDDEN ffi_prep_cif_linux64 (ffi_cif *);
ffi_स्थितिः FFI_HIDDEN ffi_prep_cif_linux64_var (ffi_cif *, अचिह्नित पूर्णाङ्क,
						अचिह्नित पूर्णाङ्क);
शून्य FFI_HIDDEN ffi_prep_args64 (extended_cif *, अचिह्नित दीर्घ *ध्रुव);
ffi_स्थितिः FFI_HIDDEN ffi_prep_संवृतिः_loc_linux64 (ffi_संवृतिः *, ffi_cif *,
						    शून्य (*) (ffi_cif *, शून्य *,
							      शून्य **, शून्य *),
						    शून्य *, शून्य *);
पूर्णाङ्क FFI_HIDDEN ffi_संवृतिः_helper_LINUX64 (ffi_cif *,
					   शून्य (*) (ffi_cif *, शून्य *,
						     शून्य **, शून्य *),
					   शून्य *, शून्य *,
					   अचिह्नित दीर्घ *, ffi_dblfl *,
					   float128 *);

#यदि_अन्तः /* !LIBFFI_ASM */
