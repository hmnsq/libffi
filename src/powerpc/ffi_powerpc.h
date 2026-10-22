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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_NONE		0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_R3		1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_R3R4		2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_F32		3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_F64		4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_F128		5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_U8		6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_S8		7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_U16		8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_S16		9

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ POWERPC64

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_U32		PPC_LD_R3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_S32		PPC_LD_R3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_PTR		PPC_LD_R3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_I64		PPC_LD_R3R4

/* Needed for soft-float long-double. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC32_LD_R3R6		10

/* Needed for FFI_SYSV small structure returns.  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC32_SYSV_LD_STRUCT_3	PPC32_LD_R3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC32_SYSV_LD_STRUCT_5	PPC32_LD_R3R4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC32_SYSV_LD_STRUCT_6	PPC32_LD_R3R4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC32_SYSV_LD_STRUCT_7	PPC32_LD_R3R4
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC32_SYSV_LD_STRUCT_3	11
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC32_SYSV_LD_STRUCT_5	12
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC32_SYSV_LD_STRUCT_6	13
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC32_SYSV_LD_STRUCT_7	14
#ਜੇ_ਅੰਤ

#ਨਹੀਂਤਾਂ /* POWERPC64 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_U32		10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_S32		11
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_PTR		PPC_LD_R3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_I64		PPC_LD_R3

/* Used by ELFv2 for homogenous structure returns.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC64_LD_ਸਦਿਸ਼		12
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC64_LD_ਸਦਿਸ਼_HOMOG	13
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC64_LD_FLOAT_HOMOG	14
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC64_LD_DOUBLE_HOMOG	15
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC64_LD_STRUCT_3	PPC_LD_U32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC64_LD_STRUCT_5	PPC_LD_I64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC64_LD_STRUCT_6	PPC_LD_I64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC64_LD_STRUCT_7	PPC_LD_I64
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC64_LD_STRUCT_3	16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC64_LD_STRUCT_5	17
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC64_LD_STRUCT_6	18
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC64_LD_STRUCT_7	19
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* POWERPC64 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LIBFFI_ASM

ਗਿਣਤੀ {
  /* The assembly depends on these exact flags.  */
  /* These go in cr7 */
  FLAG_RETURNS_SMST     = 1 << (31-31), /* Used for FFI_SYSV small structs.  */
  FLAG_RETURNS_NOTHING  = 1 << (31-30),
  FLAG_RETURNS_FP       = 1 << (31-29),
  FLAG_RETURNS_VEC      = 1 << (31-28),

  /* These go in cr6 */
  FLAG_RETURNS_64_ਬਿੱਟ   = 1 << (31-27),
  FLAG_RETURNS_128_ਬਿੱਟ  = 1 << (31-26),

  FLAG_COMPAT           = 1 << (31- 8), /* Not used by assembly */

  /* These go in cr1 */
  FLAG_ARG_NEEDS_COPY   = 1 << (31- 7), /* Used by sysv code */
  FLAG_ARG_NEEDS_PSAVE  = FLAG_ARG_NEEDS_COPY, /* Used by linux64 code */
  FLAG_FP_ARGUMENTS     = 1 << (31- 6), /* cr1.eq; specified by ABI */
  FLAG_4_GPR_ARGUMENTS  = 1 << (31- 5),
  FLAG_RETVAL_REFERENCE = 1 << (31- 4),
  FLAG_VEC_ARGUMENTS    = 1 << (31- 3),
};

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਸੰਘ
{
  ਦਸ਼ਮਲਵ f;
  ਦੁੱਗਣਾ d;
} ffi_dblfl;

#ਜੇ defined(__FLOAT128_TYPE__) && defined(__HAVE_FLOAT128)
ਕਿਸਮਪਰਿਭਾਸ਼ਾ _Float128 float128;
#ਜਾਂਜੇ defined(__FLOAT128__)
ਕਿਸਮਪਰਿਭਾਸ਼ਾ __float128 float128;
#ਨਹੀਂਤਾਂ
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਅੱਖਰ float128[16] __attribute__((aligned(16)));
#ਜੇ_ਅੰਤ

ਖਾਲੀ FFI_HIDDEN ffi_ਘੇਰਾ_SYSV (ਖਾਲੀ);
ਖਾਲੀ FFI_HIDDEN ffi_go_ਘੇਰਾ_sysv (ਖਾਲੀ);
ਖਾਲੀ FFI_HIDDEN ffi_call_SYSV(extended_cif *, ਖਾਲੀ (*)(ਖਾਲੀ), ਖਾਲੀ *,
			      ਬਿਨਾਨਿਸ਼ਾਨੀ, ਖਾਲੀ *, ਪੂਰਨ);

ਖਾਲੀ FFI_HIDDEN ffi_prep_types_sysv (ffi_abi);
ffi_ਸਥਿਤੀ FFI_HIDDEN ffi_prep_cif_sysv (ffi_cif *);
ffi_ਸਥਿਤੀ FFI_HIDDEN ffi_prep_ਘੇਰਾ_loc_sysv (ffi_ਘੇਰਾ *,
						 ffi_cif *,
						 ਖਾਲੀ (*) (ffi_cif *, ਖਾਲੀ *,
							   ਖਾਲੀ **, ਖਾਲੀ *),
						 ਖਾਲੀ *, ਖਾਲੀ *);
ਪੂਰਨ FFI_HIDDEN ffi_ਘੇਰਾ_helper_SYSV (ffi_cif *,
					ਖਾਲੀ (*) (ffi_cif *, ਖਾਲੀ *,
						  ਖਾਲੀ **, ਖਾਲੀ *),
					ਖਾਲੀ *, ਖਾਲੀ *, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *,
					ffi_dblfl *, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *);

ਖਾਲੀ FFI_HIDDEN ffi_call_LINUX64(extended_cif *, ਖਾਲੀ (*) (ਖਾਲੀ), ਖਾਲੀ *,
				 ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ, ਖਾਲੀ *, ਲੰਮਾ);
ਖਾਲੀ FFI_HIDDEN ffi_ਘੇਰਾ_LINUX64 (ਖਾਲੀ);
ਖਾਲੀ FFI_HIDDEN ffi_go_ਘੇਰਾ_linux64 (ਖਾਲੀ);

ਖਾਲੀ FFI_HIDDEN ffi_prep_types_linux64 (ffi_abi);
ffi_ਸਥਿਤੀ FFI_HIDDEN ffi_prep_cif_linux64 (ffi_cif *);
ffi_ਸਥਿਤੀ FFI_HIDDEN ffi_prep_cif_linux64_var (ffi_cif *, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ,
						ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ);
ਖਾਲੀ FFI_HIDDEN ffi_prep_args64 (extended_cif *, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *ਅਚਲ);
ffi_ਸਥਿਤੀ FFI_HIDDEN ffi_prep_ਘੇਰਾ_loc_linux64 (ffi_ਘੇਰਾ *, ffi_cif *,
						    ਖਾਲੀ (*) (ffi_cif *, ਖਾਲੀ *,
							      ਖਾਲੀ **, ਖਾਲੀ *),
						    ਖਾਲੀ *, ਖਾਲੀ *);
ਪੂਰਨ FFI_HIDDEN ffi_ਘੇਰਾ_helper_LINUX64 (ffi_cif *,
					   ਖਾਲੀ (*) (ffi_cif *, ਖਾਲੀ *,
						     ਖਾਲੀ **, ਖਾਲੀ *),
					   ਖਾਲੀ *, ਖਾਲੀ *,
					   ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *, ffi_dblfl *,
					   float128 *);

#ਜੇ_ਅੰਤ /* !LIBFFI_ASM */
