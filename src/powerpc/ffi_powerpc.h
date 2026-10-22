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
#నిర్వచించు PPC_LD_NONE		0
#నిర్వచించు PPC_LD_R3		1
#నిర్వచించు PPC_LD_R3R4		2
#నిర్వచించు PPC_LD_F32		3
#నిర్వచించు PPC_LD_F64		4
#నిర్వచించు PPC_LD_F128		5
#నిర్వచించు PPC_LD_U8		6
#నిర్వచించు PPC_LD_S8		7
#నిర్వచించు PPC_LD_U16		8
#నిర్వచించు PPC_LD_S16		9

#నిర్వచితం_కాకపోతే POWERPC64

#నిర్వచించు PPC_LD_U32		PPC_LD_R3
#నిర్వచించు PPC_LD_S32		PPC_LD_R3
#నిర్వచించు PPC_LD_PTR		PPC_LD_R3
#నిర్వచించు PPC_LD_I64		PPC_LD_R3R4

/* Needed for soft-float long-double. */
#నిర్వచించు PPC32_LD_R3R6		10

/* Needed for FFI_SYSV small structure returns.  */
#నిర్వచితమైతే __LITTLE_ENDIAN__
#నిర్వచించు PPC32_SYSV_LD_STRUCT_3	PPC32_LD_R3
#నిర్వచించు PPC32_SYSV_LD_STRUCT_5	PPC32_LD_R3R4
#నిర్వచించు PPC32_SYSV_LD_STRUCT_6	PPC32_LD_R3R4
#నిర్వచించు PPC32_SYSV_LD_STRUCT_7	PPC32_LD_R3R4
#లేకపోతే
#నిర్వచించు PPC32_SYSV_LD_STRUCT_3	11
#నిర్వచించు PPC32_SYSV_LD_STRUCT_5	12
#నిర్వచించు PPC32_SYSV_LD_STRUCT_6	13
#నిర్వచించు PPC32_SYSV_LD_STRUCT_7	14
#అయితే_ముగింపు

#లేకపోతే /* POWERPC64 */

#నిర్వచించు PPC_LD_U32		10
#నిర్వచించు PPC_LD_S32		11
#నిర్వచించు PPC_LD_PTR		PPC_LD_R3
#నిర్వచించు PPC_LD_I64		PPC_LD_R3

/* Used by ELFv2 for homogenous structure returns.  */
#నిర్వచించు PPC64_LD_సదిశ		12
#నిర్వచించు PPC64_LD_సదిశ_HOMOG	13
#నిర్వచించు PPC64_LD_FLOAT_HOMOG	14
#నిర్వచించు PPC64_LD_DOUBLE_HOMOG	15
#నిర్వచితమైతే __LITTLE_ENDIAN__
#నిర్వచించు PPC64_LD_STRUCT_3	PPC_LD_U32
#నిర్వచించు PPC64_LD_STRUCT_5	PPC_LD_I64
#నిర్వచించు PPC64_LD_STRUCT_6	PPC_LD_I64
#నిర్వచించు PPC64_LD_STRUCT_7	PPC_LD_I64
#లేకపోతే
#నిర్వచించు PPC64_LD_STRUCT_3	16
#నిర్వచించు PPC64_LD_STRUCT_5	17
#నిర్వచించు PPC64_LD_STRUCT_6	18
#నిర్వచించు PPC64_LD_STRUCT_7	19
#అయితే_ముగింపు

#అయితే_ముగింపు /* POWERPC64 */

#నిర్వచితం_కాకపోతే LIBFFI_ASM

గణన {
  /* The assembly depends on these exact flags.  */
  /* These go in cr7 */
  FLAG_RETURNS_SMST     = 1 << (31-31), /* Used for FFI_SYSV small structs.  */
  FLAG_RETURNS_NOTHING  = 1 << (31-30),
  FLAG_RETURNS_FP       = 1 << (31-29),
  FLAG_RETURNS_VEC      = 1 << (31-28),

  /* These go in cr6 */
  FLAG_RETURNS_64_బిట్లు   = 1 << (31-27),
  FLAG_RETURNS_128_బిట్లు  = 1 << (31-26),

  FLAG_COMPAT           = 1 << (31- 8), /* Not used by assembly */

  /* These go in cr1 */
  FLAG_ARG_NEEDS_COPY   = 1 << (31- 7), /* Used by sysv code */
  FLAG_ARG_NEEDS_PSAVE  = FLAG_ARG_NEEDS_COPY, /* Used by linux64 code */
  FLAG_FP_ARGUMENTS     = 1 << (31- 6), /* cr1.eq; specified by ABI */
  FLAG_4_GPR_ARGUMENTS  = 1 << (31- 5),
  FLAG_RETVAL_REFERENCE = 1 << (31- 4),
  FLAG_VEC_ARGUMENTS    = 1 << (31- 3),
};

రకనిర్వచనం సంఘం
{
  దశాంశం f;
  ద్విగుణ d;
} ffi_dblfl;

#ఒకవేళ defined(__FLOAT128_TYPE__) && defined(__HAVE_FLOAT128)
రకనిర్వచనం _Float128 float128;
#లేదావేళ defined(__FLOAT128__)
రకనిర్వచనం __float128 float128;
#లేకపోతే
రకనిర్వచనం అక్షరం float128[16] __attribute__((aligned(16)));
#అయితే_ముగింపు

శూన్య FFI_HIDDEN ffi_సంవృతం_SYSV (శూన్య);
శూన్య FFI_HIDDEN ffi_go_సంవృతం_sysv (శూన్య);
శూన్య FFI_HIDDEN ffi_call_SYSV(extended_cif *, శూన్య (*)(శూన్య), శూన్య *,
			      గుర్తులేని, శూన్య *, పూర్ణాంకం);

శూన్య FFI_HIDDEN ffi_prep_types_sysv (ffi_abi);
ffi_స్థితి FFI_HIDDEN ffi_prep_cif_sysv (ffi_cif *);
ffi_స్థితి FFI_HIDDEN ffi_prep_సంవృతం_loc_sysv (ffi_సంవృతం *,
						 ffi_cif *,
						 శూన్య (*) (ffi_cif *, శూన్య *,
							   శూన్య **, శూన్య *),
						 శూన్య *, శూన్య *);
పూర్ణాంకం FFI_HIDDEN ffi_సంవృతం_helper_SYSV (ffi_cif *,
					శూన్య (*) (ffi_cif *, శూన్య *,
						  శూన్య **, శూన్య *),
					శూన్య *, శూన్య *, గుర్తులేని దీర్ఘ *,
					ffi_dblfl *, గుర్తులేని దీర్ఘ *);

శూన్య FFI_HIDDEN ffi_call_LINUX64(extended_cif *, శూన్య (*) (శూన్య), శూన్య *,
				 గుర్తులేని దీర్ఘ, శూన్య *, దీర్ఘ);
శూన్య FFI_HIDDEN ffi_సంవృతం_LINUX64 (శూన్య);
శూన్య FFI_HIDDEN ffi_go_సంవృతం_linux64 (శూన్య);

శూన్య FFI_HIDDEN ffi_prep_types_linux64 (ffi_abi);
ffi_స్థితి FFI_HIDDEN ffi_prep_cif_linux64 (ffi_cif *);
ffi_స్థితి FFI_HIDDEN ffi_prep_cif_linux64_var (ffi_cif *, గుర్తులేని పూర్ణాంకం,
						గుర్తులేని పూర్ణాంకం);
శూన్య FFI_HIDDEN ffi_prep_args64 (extended_cif *, గుర్తులేని దీర్ఘ *స్థిరరాశి);
ffi_స్థితి FFI_HIDDEN ffi_prep_సంవృతం_loc_linux64 (ffi_సంవృతం *, ffi_cif *,
						    శూన్య (*) (ffi_cif *, శూన్య *,
							      శూన్య **, శూన్య *),
						    శూన్య *, శూన్య *);
పూర్ణాంకం FFI_HIDDEN ffi_సంవృతం_helper_LINUX64 (ffi_cif *,
					   శూన్య (*) (ffi_cif *, శూన్య *,
						     శూన్య **, శూన్య *),
					   శూన్య *, శూన్య *,
					   గుర్తులేని దీర్ఘ *, ffi_dblfl *,
					   float128 *);

#అయితే_ముగింపు /* !LIBFFI_ASM */
