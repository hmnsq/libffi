/*
 * Copyright (c) 2013 Miodrag Vallat.  <miod@openbsd.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * ``Software''), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * vax Foreign Function Interface
 *
 * This file attempts to provide all the FFI entry points which can reliably
 * be implemented in C.
 */

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ವ್ಯಾಖ್ಯಾನಿಸು CIF_FLAGS_CHAR		1	/* for struct only */
#ವ್ಯಾಖ್ಯಾನಿಸು CIF_FLAGS_SHORT		2	/* for struct only */
#ವ್ಯಾಖ್ಯಾನಿಸು CIF_FLAGS_INT		4
#ವ್ಯಾಖ್ಯಾನಿಸು CIF_FLAGS_DINT		8

/*
 * Foreign Function Interface API
 */

ಶೂನ್ಯ ffi_call_elfbsd (extended_cif *, ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ, ಶೂನ್ಯ *,
		       ಶೂನ್ಯ (*) ());
ಶೂನ್ಯ *ffi_prep_ವಾದಗಳು (extended_cif *ecif, ಶೂನ್ಯ *ರಾಶಿ);

ಶೂನ್ಯ *
ffi_prep_ವಾದಗಳು (extended_cif *ecif, ಶೂನ್ಯ *ರಾಶಿ)
{
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ಶೂನ್ಯ **p_argv;
  ಅಕ್ಷರ *argp;
  ffi_type **p_arg;
  ಶೂನ್ಯ *struct_ಮೌಲ್ಯ_ptr;

  argp = ರಾಶಿ;

  ಆದರೆ (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_ಮೌಲ್ಯ_ptr = ecif->rvalue;
  ಇಲ್ಲದಿದ್ದರೆ
    struct_ಮೌಲ್ಯ_ptr = ಖಾಲಿ;

  p_argv = ecif->avalue;

  ಗಾಗಿ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->ಗಾತ್ರ_2;
      ಆದರೆ (z < ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ))
	{
	  ಆಯ್ಕೆ ((*p_arg)->type)
	    {
	    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	      *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ) *(SINT8 *) *p_argv;
	      ನಿಲ್ಲಿಸು;

	    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	      *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) *(UINT8 *) *p_argv;
	      ನಿಲ್ಲಿಸು;

	    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	      *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ) *(SINT16 *) *p_argv;
	      ನಿಲ್ಲಿಸು;

	    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	      *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) *(UINT16 *) *p_argv;
	      ನಿಲ್ಲಿಸು;

	    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	      ಸ್ಮರಣೆ_ನಕಲು (argp, *p_argv, z);
	      ನಿಲ್ಲಿಸು;

	    ಪೂರ್ವನಿಯೋಜಿತ:
	      FFI_ASSERT (0);
	    }
	  z = ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ);
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  ಸ್ಮರಣೆ_ನಕಲು (argp, *p_argv, z);

	  /* Align if necessary.  */
	  ಆದರೆ ((ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ) - 1) & z)
	    z = FFI_ALIGN(z, ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ));
	}

      p_argv++;
      argp += z;
    }

  ಹಿಂತಿರುಗಿ struct_ಮೌಲ್ಯ_ptr;
}

ffi_ಸ್ಥಿತಿ
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  ಆಯ್ಕೆ (cif->rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_VOID:
      cif->flags = 0;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      ಆದರೆ (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
	  cif->rtype->elements[1])
	{
	  cif->flags = 0;
	  ನಿಲ್ಲಿಸು;
	}

      ಆದರೆ (cif->rtype->ಗಾತ್ರ_2 == ಗಾತ್ರ (ಅಕ್ಷರ))
	cif->flags = CIF_FLAGS_CHAR;
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cif->rtype->ಗಾತ್ರ_2 == ಗಾತ್ರ (ಗಿಡ್ಡ))
	cif->flags = CIF_FLAGS_SHORT;
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cif->rtype->ಗಾತ್ರ_2 == ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ))
	cif->flags = CIF_FLAGS_INT;
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cif->rtype->ಗಾತ್ರ_2 == 2 * ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ))
	cif->flags = CIF_FLAGS_DINT;
      ಇಲ್ಲದಿದ್ದರೆ
	cif->flags = 0;
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      ಆದರೆ (cif->rtype->ಗಾತ್ರ_2 <= ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ))
	cif->flags = CIF_FLAGS_INT;
      ಇಲ್ಲದಿದ್ದರೆ
	cif->flags = CIF_FLAGS_DINT;
      ನಿಲ್ಲಿಸು;
    }

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn) (), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  ಆದರೆ (rvalue == ಖಾಲಿ
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->flags == 0)
    ecif.rvalue = alloca (cif->rtype->ಗಾತ್ರ_2);
  ಇಲ್ಲದಿದ್ದರೆ
    ecif.rvalue = rvalue;

  ಆಯ್ಕೆ (cif->abi)
    {
    ಸ್ಥಿತಿ FFI_ELFBSD:
      ffi_call_elfbsd (&ecif, cif->ಬೈಟುಗಳು, cif->flags, ecif.rvalue, fn);
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      FFI_ASSERT (0);
      ನಿಲ್ಲಿಸು;
    }
}

/*
 * Closure API
 */

ಶೂನ್ಯ ffi_ಆವರಣ_elfbsd (ಶೂನ್ಯ);
ಶೂನ್ಯ ffi_ಆವರಣ_struct_elfbsd (ಶೂನ್ಯ);
ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ffi_ಆವರಣ_elfbsd_inner (ffi_ಆವರಣ *, ಶೂನ್ಯ *, ಅಕ್ಷರ *);

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_prep_ಆವರಣ_elfbsd (ffi_cif *cif, ಶೂನ್ಯ **avalue, ಅಕ್ಷರ *stackp)
{
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ಶೂನ್ಯ **p_argv;
  ffi_type **p_arg;

  p_argv = avalue;

  ಗಾಗಿ (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->ಗಾತ್ರ_2;
      *p_argv = stackp;

      /* Align if necessary */
      ಆದರೆ ((ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ) - 1) & z)
	z = FFI_ALIGN(z, ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ));

      p_argv++;
      stackp += z;
    }
}

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ
ffi_ಆವರಣ_elfbsd_inner (ffi_ಆವರಣ *ಆವರಣ, ಶೂನ್ಯ *resp, ಅಕ್ಷರ *ರಾಶಿ)
{
  ffi_cif *cif;
  ಶೂನ್ಯ **arg_area;

  cif = ಆವರಣ->cif;
  arg_area = (ಶೂನ್ಯ **) alloca (cif->nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));

  ffi_prep_ಆವರಣ_elfbsd (cif, arg_area, ರಾಶಿ);

  (ಆವರಣ->fun) (cif, resp, arg_area, ಆವರಣ->user_ದತ್ತಾಂಶ);

  ಹಿಂತಿರುಗಿ cif->flags;
}

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ *ಆವರಣ, ffi_cif *cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ, ಶೂನ್ಯ *codeloc)
{
  ಅಕ್ಷರ *tramp = (ಅಕ್ಷರ *) codeloc;
  ಶೂನ್ಯ *fn;

  FFI_ASSERT (cif->abi == FFI_ELFBSD);

  /* entry mask */
  *(ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ *)(tramp + 0) = 0x0000;
  /* movl #closure, r0 */
  tramp[2] = 0xd0;
  tramp[3] = 0x8f;
  *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *)(tramp + 4) = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) ಆವರಣ;
  tramp[8] = 0x50;

  ಆದರೆ (cif->rtype->type == FFI_TYPE_STRUCT
      && !cif->flags)
    fn = &ffi_ಆವರಣ_struct_elfbsd;
  ಇಲ್ಲದಿದ್ದರೆ
    fn = &ffi_ಆವರಣ_elfbsd;

  /* jmpl #fn */
  tramp[9] = 0x17;
  tramp[10] = 0xef;
  *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *)(tramp + 11) = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)fn + 2 -
				  (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)tramp - 9 - 6;

  ಆವರಣ->cif = cif;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;
  ಆವರಣ->fun = fun;

  ಹಿಂತಿರುಗಿ FFI_OK;
}
