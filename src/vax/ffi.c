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

#define CIF_FLAGS_CHAR		1	/* for struct only */
#define CIF_FLAGS_SHORT		2	/* for struct only */
#define CIF_FLAGS_INT		4
#define CIF_FLAGS_DINT		8

/*
 * Foreign Function Interface API
 */

ಶೂನ್ಯ ffi_call_elfbsd (extended_cif *, unsigned, unsigned, ಶೂನ್ಯ *,
		       ಶೂನ್ಯ (*) ());
ಶೂನ್ಯ *ffi_prep_args (extended_cif *ecif, ಶೂನ್ಯ *ರಾಶಿ);

ಶೂನ್ಯ *
ffi_prep_args (extended_cif *ecif, ಶೂನ್ಯ *ರಾಶಿ)
{
  unsigned ಪೂರ್ಣಾಙ್ಕ i;
  ಶೂನ್ಯ **p_argv;
  ವರ್ಣ *argp;
  ffi_type **p_arg;
  ಶೂನ್ಯ *struct_ಮೌಲ್ಯ_ptr;

  argp = ರಾಶಿ;

  ಯದಿ (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_ಮೌಲ್ಯ_ptr = ecif->rvalue;
  ಅನ್ಯಥಾ
    struct_ಮೌಲ್ಯ_ptr = ರಿಕ್ತ;

  p_argv = ecif->avalue;

  ಪ್ರತಿ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      ಯದಿ (z < ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ))
	{
	  ವಿಕಲ್ಪ ((*p_arg)->type)
	    {
	    ಪಕ್ಷ FFI_TYPE_SINT8:
	      *(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ) *(SINT8 *) *p_argv;
	      ಭಙ್ಗ;

	    ಪಕ್ಷ FFI_TYPE_UINT8:
	      *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ) *(UINT8 *) *p_argv;
	      ಭಙ್ಗ;

	    ಪಕ್ಷ FFI_TYPE_SINT16:
	      *(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ) *(SINT16 *) *p_argv;
	      ಭಙ್ಗ;

	    ಪಕ್ಷ FFI_TYPE_UINT16:
	      *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ) *(UINT16 *) *p_argv;
	      ಭಙ್ಗ;

	    ಪಕ್ಷ FFI_TYPE_STRUCT:
	      memcpy (argp, *p_argv, z);
	      ಭಙ್ಗ;

	    ಸ್ವಭಾವ:
	      FFI_ASSERT (0);
	    }
	  z = ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ);
	}
      ಅನ್ಯಥಾ
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  ಯದಿ ((ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ) - 1) & z)
	    z = FFI_ALIGN(z, ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ));
	}

      p_argv++;
      argp += z;
    }

  ಪ್ರತ್ಯಾಗಮ struct_ಮೌಲ್ಯ_ptr;
}

ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  ವಿಕಲ್ಪ (cif->rtype->type)
    {
    ಪಕ್ಷ FFI_TYPE_VOID:
      cif->flags = 0;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_STRUCT:
      ಯದಿ (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
	  cif->rtype->elements[1])
	{
	  cif->flags = 0;
	  ಭಙ್ಗ;
	}

      ಯದಿ (cif->rtype->size == ಪರಿಮಾಣ (ವರ್ಣ))
	cif->flags = CIF_FLAGS_CHAR;
      ಅನ್ಯಥಾ ಯದಿ (cif->rtype->size == ಪರಿಮಾಣ (short))
	cif->flags = CIF_FLAGS_SHORT;
      ಅನ್ಯಥಾ ಯದಿ (cif->rtype->size == ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ))
	cif->flags = CIF_FLAGS_INT;
      ಅನ್ಯಥಾ ಯದಿ (cif->rtype->size == 2 * ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ))
	cif->flags = CIF_FLAGS_DINT;
      ಅನ್ಯಥಾ
	cif->flags = 0;
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      ಯದಿ (cif->rtype->size <= ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ))
	cif->flags = CIF_FLAGS_INT;
      ಅನ್ಯಥಾ
	cif->flags = CIF_FLAGS_DINT;
      ಭಙ್ಗ;
    }

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn) (), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  ಯದಿ (rvalue == ರಿಕ್ತ
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->flags == 0)
    ecif.rvalue = alloca (cif->rtype->size);
  ಅನ್ಯಥಾ
    ecif.rvalue = rvalue;

  ವಿಕಲ್ಪ (cif->abi)
    {
    ಪಕ್ಷ FFI_ELFBSD:
      ffi_call_elfbsd (&ecif, cif->bytes, cif->flags, ecif.rvalue, fn);
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      FFI_ASSERT (0);
      ಭಙ್ಗ;
    }
}

/*
 * Closure API
 */

ಶೂನ್ಯ ffi_closure_elfbsd (ಶೂನ್ಯ);
ಶೂನ್ಯ ffi_closure_struct_elfbsd (ಶೂನ್ಯ);
unsigned ಪೂರ್ಣಾಙ್ಕ ffi_closure_elfbsd_inner (ffi_closure *, ಶೂನ್ಯ *, ವರ್ಣ *);

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_prep_closure_elfbsd (ffi_cif *cif, ಶೂನ್ಯ **avalue, ವರ್ಣ *stackp)
{
  unsigned ಪೂರ್ಣಾಙ್ಕ i;
  ಶೂನ್ಯ **p_argv;
  ffi_type **p_arg;

  p_argv = avalue;

  ಪ್ರತಿ (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      *p_argv = stackp;

      /* Align if necessary */
      ಯದಿ ((ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ) - 1) & z)
	z = FFI_ALIGN(z, ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ));

      p_argv++;
      stackp += z;
    }
}

unsigned ಪೂರ್ಣಾಙ್ಕ
ffi_closure_elfbsd_inner (ffi_closure *closure, ಶೂನ್ಯ *resp, ವರ್ಣ *ರಾಶಿ)
{
  ffi_cif *cif;
  ಶೂನ್ಯ **arg_area;

  cif = closure->cif;
  arg_area = (ಶೂನ್ಯ **) alloca (cif->nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ *));

  ffi_prep_closure_elfbsd (cif, arg_area, ರಾಶಿ);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  ಪ್ರತ್ಯಾಗಮ cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
		      ಶೂನ್ಯ *user_data, ಶೂನ್ಯ *codeloc)
{
  ವರ್ಣ *tramp = (ವರ್ಣ *) codeloc;
  ಶೂನ್ಯ *fn;

  FFI_ASSERT (cif->abi == FFI_ELFBSD);

  /* entry mask */
  *(unsigned short *)(tramp + 0) = 0x0000;
  /* movl #closure, r0 */
  tramp[2] = 0xd0;
  tramp[3] = 0x8f;
  *(unsigned ಪೂರ್ಣಾಙ್ಕ *)(tramp + 4) = (unsigned ಪೂರ್ಣಾಙ್ಕ) closure;
  tramp[8] = 0x50;

  ಯದಿ (cif->rtype->type == FFI_TYPE_STRUCT
      && !cif->flags)
    fn = &ffi_closure_struct_elfbsd;
  ಅನ್ಯಥಾ
    fn = &ffi_closure_elfbsd;

  /* jmpl #fn */
  tramp[9] = 0x17;
  tramp[10] = 0xef;
  *(unsigned ಪೂರ್ಣಾಙ್ಕ *)(tramp + 11) = (unsigned ಪೂರ್ಣಾಙ್ಕ)fn + 2 -
				  (unsigned ಪೂರ್ಣಾಙ್ಕ)tramp - 9 - 6;

  closure->cif = cif;
  closure->user_data = user_data;
  closure->fun = fun;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}
