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
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_NONE		0
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_R3		1
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_R3R4		2
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_F32		3
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_F64		4
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_F128		5
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_U8		6
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_S8		7
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_U16		8
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_S16		9

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ POWERPC64

#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_U32		PPC_LD_R3
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_S32		PPC_LD_R3
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_PTR		PPC_LD_R3
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_I64		PPC_LD_R3R4

/* Needed for soft-float long-double. */
#ವ್ಯಾಖ್ಯಾನಿಸು PPC32_LD_R3R6		10

/* Needed for FFI_SYSV small structure returns.  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __LITTLE_ENDIAN__
#ವ್ಯಾಖ್ಯಾನಿಸು PPC32_SYSV_LD_STRUCT_3	PPC32_LD_R3
#ವ್ಯಾಖ್ಯಾನಿಸು PPC32_SYSV_LD_STRUCT_5	PPC32_LD_R3R4
#ವ್ಯಾಖ್ಯಾನಿಸು PPC32_SYSV_LD_STRUCT_6	PPC32_LD_R3R4
#ವ್ಯಾಖ್ಯಾನಿಸು PPC32_SYSV_LD_STRUCT_7	PPC32_LD_R3R4
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು PPC32_SYSV_LD_STRUCT_3	11
#ವ್ಯಾಖ್ಯಾನಿಸು PPC32_SYSV_LD_STRUCT_5	12
#ವ್ಯಾಖ್ಯಾನಿಸು PPC32_SYSV_LD_STRUCT_6	13
#ವ್ಯಾಖ್ಯಾನಿಸು PPC32_SYSV_LD_STRUCT_7	14
#ಆದರೆ_ಅಂತ್ಯ

#ಇಲ್ಲದಿದ್ದರೆ /* POWERPC64 */

#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_U32		10
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_S32		11
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_PTR		PPC_LD_R3
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_I64		PPC_LD_R3

/* Used by ELFv2 for homogenous structure returns.  */
#ವ್ಯಾಖ್ಯಾನಿಸು PPC64_LD_ಸದಿಶ		12
#ವ್ಯಾಖ್ಯಾನಿಸು PPC64_LD_ಸದಿಶ_HOMOG	13
#ವ್ಯಾಖ್ಯಾನಿಸು PPC64_LD_FLOAT_HOMOG	14
#ವ್ಯಾಖ್ಯಾನಿಸು PPC64_LD_DOUBLE_HOMOG	15
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __LITTLE_ENDIAN__
#ವ್ಯಾಖ್ಯಾನಿಸು PPC64_LD_STRUCT_3	PPC_LD_U32
#ವ್ಯಾಖ್ಯಾನಿಸು PPC64_LD_STRUCT_5	PPC_LD_I64
#ವ್ಯಾಖ್ಯಾನಿಸು PPC64_LD_STRUCT_6	PPC_LD_I64
#ವ್ಯಾಖ್ಯಾನಿಸು PPC64_LD_STRUCT_7	PPC_LD_I64
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು PPC64_LD_STRUCT_3	16
#ವ್ಯಾಖ್ಯಾನಿಸು PPC64_LD_STRUCT_5	17
#ವ್ಯಾಖ್ಯಾನಿಸು PPC64_LD_STRUCT_6	18
#ವ್ಯಾಖ್ಯಾನಿಸು PPC64_LD_STRUCT_7	19
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* POWERPC64 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LIBFFI_ASM

ಎಣಿಕೆ {
  /* The assembly depends on these exact flags.  */
  /* These go in cr7 */
  FLAG_RETURNS_SMST     = 1 << (31-31), /* Used for FFI_SYSV small structs.  */
  FLAG_RETURNS_NOTHING  = 1 << (31-30),
  FLAG_RETURNS_FP       = 1 << (31-29),
  FLAG_RETURNS_VEC      = 1 << (31-28),

  /* These go in cr6 */
  FLAG_RETURNS_64_ಬಿಟ್ಟುಗಳು   = 1 << (31-27),
  FLAG_RETURNS_128_ಬಿಟ್ಟುಗಳು  = 1 << (31-26),

  FLAG_COMPAT           = 1 << (31- 8), /* Not used by assembly */

  /* These go in cr1 */
  FLAG_ARG_NEEDS_COPY   = 1 << (31- 7), /* Used by sysv code */
  FLAG_ARG_NEEDS_PSAVE  = FLAG_ARG_NEEDS_COPY, /* Used by linux64 code */
  FLAG_FP_ARGUMENTS     = 1 << (31- 6), /* cr1.eq; specified by ABI */
  FLAG_4_GPR_ARGUMENTS  = 1 << (31- 5),
  FLAG_RETVAL_REFERENCE = 1 << (31- 4),
  FLAG_VEC_ARGUMENTS    = 1 << (31- 3),
};

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಒಕ್ಕೂಟ
{
  ದಶಮಾಂಶ f;
  ದ್ವಿಗುಣ d;
} ffi_dblfl;

#ಆದರೆ defined(__FLOAT128_TYPE__) && defined(__HAVE_FLOAT128)
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ _Float128 float128;
#ಅಥವಾಆದರೆ defined(__FLOAT128__)
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ __float128 float128;
#ಇಲ್ಲದಿದ್ದರೆ
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಅಕ್ಷರ float128[16] __attribute__((aligned(16)));
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ FFI_HIDDEN ffi_ಆವರಣ_SYSV (ಶೂನ್ಯ);
ಶೂನ್ಯ FFI_HIDDEN ffi_go_ಆವರಣ_sysv (ಶೂನ್ಯ);
ಶೂನ್ಯ FFI_HIDDEN ffi_call_SYSV(extended_cif *, ಶೂನ್ಯ (*)(ಶೂನ್ಯ), ಶೂನ್ಯ *,
			      ಚಿಹ್ನೆರಹಿತ, ಶೂನ್ಯ *, ಪೂರ್ಣಾಂಕ);

ಶೂನ್ಯ FFI_HIDDEN ffi_prep_types_sysv (ffi_abi);
ffi_ಸ್ಥಿತಿ FFI_HIDDEN ffi_prep_cif_sysv (ffi_cif *);
ffi_ಸ್ಥಿತಿ FFI_HIDDEN ffi_prep_ಆವರಣ_loc_sysv (ffi_ಆವರಣ *,
						 ffi_cif *,
						 ಶೂನ್ಯ (*) (ffi_cif *, ಶೂನ್ಯ *,
							   ಶೂನ್ಯ **, ಶೂನ್ಯ *),
						 ಶೂನ್ಯ *, ಶೂನ್ಯ *);
ಪೂರ್ಣಾಂಕ FFI_HIDDEN ffi_ಆವರಣ_helper_SYSV (ffi_cif *,
					ಶೂನ್ಯ (*) (ffi_cif *, ಶೂನ್ಯ *,
						  ಶೂನ್ಯ **, ಶೂನ್ಯ *),
					ಶೂನ್ಯ *, ಶೂನ್ಯ *, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *,
					ffi_dblfl *, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *);

ಶೂನ್ಯ FFI_HIDDEN ffi_call_LINUX64(extended_cif *, ಶೂನ್ಯ (*) (ಶೂನ್ಯ), ಶೂನ್ಯ *,
				 ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ, ಶೂನ್ಯ *, ದೀರ್ಘ);
ಶೂನ್ಯ FFI_HIDDEN ffi_ಆವರಣ_LINUX64 (ಶೂನ್ಯ);
ಶೂನ್ಯ FFI_HIDDEN ffi_go_ಆವರಣ_linux64 (ಶೂನ್ಯ);

ಶೂನ್ಯ FFI_HIDDEN ffi_prep_types_linux64 (ffi_abi);
ffi_ಸ್ಥಿತಿ FFI_HIDDEN ffi_prep_cif_linux64 (ffi_cif *);
ffi_ಸ್ಥಿತಿ FFI_HIDDEN ffi_prep_cif_linux64_var (ffi_cif *, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ,
						ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ FFI_HIDDEN ffi_prep_args64 (extended_cif *, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *ಸ್ಥಿರಾಂಕ);
ffi_ಸ್ಥಿತಿ FFI_HIDDEN ffi_prep_ಆವರಣ_loc_linux64 (ffi_ಆವರಣ *, ffi_cif *,
						    ಶೂನ್ಯ (*) (ffi_cif *, ಶೂನ್ಯ *,
							      ಶೂನ್ಯ **, ಶೂನ್ಯ *),
						    ಶೂನ್ಯ *, ಶೂನ್ಯ *);
ಪೂರ್ಣಾಂಕ FFI_HIDDEN ffi_ಆವರಣ_helper_LINUX64 (ffi_cif *,
					   ಶೂನ್ಯ (*) (ffi_cif *, ಶೂನ್ಯ *,
						     ಶೂನ್ಯ **, ಶೂನ್ಯ *),
					   ಶೂನ್ಯ *, ಶೂನ್ಯ *,
					   ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *, ffi_dblfl *,
					   float128 *);

#ಆದರೆ_ಅಂತ್ಯ /* !LIBFFI_ASM */
