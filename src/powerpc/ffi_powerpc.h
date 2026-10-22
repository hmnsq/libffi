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
#घोषणा PPC_LD_NONE		0
#घोषणा PPC_LD_R3		1
#घोषणा PPC_LD_R3R4		2
#घोषणा PPC_LD_F32		3
#घोषणा PPC_LD_F64		4
#घोषणा PPC_LD_F128		5
#घोषणा PPC_LD_U8		6
#घोषणा PPC_LD_S8		7
#घोषणा PPC_LD_U16		8
#घोषणा PPC_LD_S16		9

#अगर_अघोषित POWERPC64

#घोषणा PPC_LD_U32		PPC_LD_R3
#घोषणा PPC_LD_S32		PPC_LD_R3
#घोषणा PPC_LD_PTR		PPC_LD_R3
#घोषणा PPC_LD_I64		PPC_LD_R3R4

/* Needed for soft-float long-double. */
#घोषणा PPC32_LD_R3R6		10

/* Needed for FFI_SYSV small structure returns.  */
#अगर_घोषित __LITTLE_ENDIAN__
#घोषणा PPC32_SYSV_LD_STRUCT_3	PPC32_LD_R3
#घोषणा PPC32_SYSV_LD_STRUCT_5	PPC32_LD_R3R4
#घोषणा PPC32_SYSV_LD_STRUCT_6	PPC32_LD_R3R4
#घोषणा PPC32_SYSV_LD_STRUCT_7	PPC32_LD_R3R4
#अन्यथा
#घोषणा PPC32_SYSV_LD_STRUCT_3	11
#घोषणा PPC32_SYSV_LD_STRUCT_5	12
#घोषणा PPC32_SYSV_LD_STRUCT_6	13
#घोषणा PPC32_SYSV_LD_STRUCT_7	14
#पूर्ण_अगर

#अन्यथा /* POWERPC64 */

#घोषणा PPC_LD_U32		10
#घोषणा PPC_LD_S32		11
#घोषणा PPC_LD_PTR		PPC_LD_R3
#घोषणा PPC_LD_I64		PPC_LD_R3

/* Used by ELFv2 for homogenous structure returns.  */
#घोषणा PPC64_LD_VECTOR		12
#घोषणा PPC64_LD_VECTOR_HOMOG	13
#घोषणा PPC64_LD_FLOAT_HOMOG	14
#घोषणा PPC64_LD_DOUBLE_HOMOG	15
#अगर_घोषित __LITTLE_ENDIAN__
#घोषणा PPC64_LD_STRUCT_3	PPC_LD_U32
#घोषणा PPC64_LD_STRUCT_5	PPC_LD_I64
#घोषणा PPC64_LD_STRUCT_6	PPC_LD_I64
#घोषणा PPC64_LD_STRUCT_7	PPC_LD_I64
#अन्यथा
#घोषणा PPC64_LD_STRUCT_3	16
#घोषणा PPC64_LD_STRUCT_5	17
#घोषणा PPC64_LD_STRUCT_6	18
#घोषणा PPC64_LD_STRUCT_7	19
#पूर्ण_अगर

#पूर्ण_अगर /* POWERPC64 */

#अगर_अघोषित LIBFFI_ASM

क्रमागत {
  /* The assembly depends on these exact flags.  */
  /* These go in cr7 */
  FLAG_RETURNS_SMST     = 1 << (31-31), /* Used for FFI_SYSV small structs.  */
  FLAG_RETURNS_NOTHING  = 1 << (31-30),
  FLAG_RETURNS_FP       = 1 << (31-29),
  FLAG_RETURNS_VEC      = 1 << (31-28),

  /* These go in cr6 */
  FLAG_RETURNS_64BITS   = 1 << (31-27),
  FLAG_RETURNS_128BITS  = 1 << (31-26),

  FLAG_COMPAT           = 1 << (31- 8), /* Not used by assembly */

  /* These go in cr1 */
  FLAG_ARG_NEEDS_COPY   = 1 << (31- 7), /* Used by sysv code */
  FLAG_ARG_NEEDS_PSAVE  = FLAG_ARG_NEEDS_COPY, /* Used by linux64 code */
  FLAG_FP_ARGUMENTS     = 1 << (31- 6), /* cr1.eq; specified by ABI */
  FLAG_4_GPR_ARGUMENTS  = 1 << (31- 5),
  FLAG_RETVAL_REFERENCE = 1 << (31- 4),
  FLAG_VEC_ARGUMENTS    = 1 << (31- 3),
};

प्रकार जोड़
{
  भग्न f;
  द्विगुन d;
} ffi_dblfl;

#अगर defined(__FLOAT128_TYPE__) && defined(__HAVE_FLOAT128)
प्रकार _Float128 float128;
#या_अगर defined(__FLOAT128__)
प्रकार __float128 float128;
#अन्यथा
प्रकार अक्षर float128[16] __attribute__((aligned(16)));
#पूर्ण_अगर

व्योम FFI_HIDDEN ffi_closure_SYSV (व्योम);
व्योम FFI_HIDDEN ffi_go_closure_sysv (व्योम);
व्योम FFI_HIDDEN ffi_call_SYSV(extended_cif *, व्योम (*)(व्योम), व्योम *,
			      अचिन्हित, व्योम *, पूर्णांक);

व्योम FFI_HIDDEN ffi_prep_types_sysv (ffi_abi);
ffi_status FFI_HIDDEN ffi_prep_cif_sysv (ffi_cif *);
ffi_status FFI_HIDDEN ffi_prep_closure_loc_sysv (ffi_closure *,
						 ffi_cif *,
						 व्योम (*) (ffi_cif *, व्योम *,
							   व्योम **, व्योम *),
						 व्योम *, व्योम *);
पूर्णांक FFI_HIDDEN ffi_closure_helper_SYSV (ffi_cif *,
					व्योम (*) (ffi_cif *, व्योम *,
						  व्योम **, व्योम *),
					व्योम *, व्योम *, अचिन्हित दीर्घ *,
					ffi_dblfl *, अचिन्हित दीर्घ *);

व्योम FFI_HIDDEN ffi_call_LINUX64(extended_cif *, व्योम (*) (व्योम), व्योम *,
				 अचिन्हित दीर्घ, व्योम *, दीर्घ);
व्योम FFI_HIDDEN ffi_closure_LINUX64 (व्योम);
व्योम FFI_HIDDEN ffi_go_closure_linux64 (व्योम);

व्योम FFI_HIDDEN ffi_prep_types_linux64 (ffi_abi);
ffi_status FFI_HIDDEN ffi_prep_cif_linux64 (ffi_cif *);
ffi_status FFI_HIDDEN ffi_prep_cif_linux64_var (ffi_cif *, अचिन्हित पूर्णांक,
						अचिन्हित पूर्णांक);
व्योम FFI_HIDDEN ffi_prep_args64 (extended_cif *, अचिन्हित दीर्घ *स्थिर);
ffi_status FFI_HIDDEN ffi_prep_closure_loc_linux64 (ffi_closure *, ffi_cif *,
						    व्योम (*) (ffi_cif *, व्योम *,
							      व्योम **, व्योम *),
						    व्योम *, व्योम *);
पूर्णांक FFI_HIDDEN ffi_closure_helper_LINUX64 (ffi_cif *,
					   व्योम (*) (ffi_cif *, व्योम *,
						     व्योम **, व्योम *),
					   व्योम *, व्योम *,
					   अचिन्हित दीर्घ *, ffi_dblfl *,
					   float128 *);

#पूर्ण_अगर /* !LIBFFI_ASM */
