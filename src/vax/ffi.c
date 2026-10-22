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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <unistd.h>

#define CIF_FLAGS_CHAR		1	/* for struct only */
#define CIF_FLAGS_SHORT		2	/* for struct only */
#define CIF_FLAGS_INT		4
#define CIF_FLAGS_DINT		8

/*
 * Foreign Function Interface API
 */

शून्य ffi_call_elfbsd (extended_cif *, unsigned, unsigned, शून्य *,
		       शून्य (*) ());
शून्य *ffi_prep_args (extended_cif *ecif, शून्य *राशिः);

शून्य *
ffi_prep_args (extended_cif *ecif, शून्य *राशिः)
{
  unsigned पूर्णाङ्क i;
  शून्य **p_argv;
  वर्ण *argp;
  ffi_type **p_arg;
  शून्य *struct_मानम्_ptr;

  argp = राशिः;

  जइ (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_मानम्_ptr = ecif->rvalue;
  अण्णहा
    struct_मानम्_ptr = रिक्त;

  p_argv = ecif->avalue;

  प्रति (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      जइ (z < परिमाण (पूर्णाङ्क))
	{
	  विकल्प ((*p_arg)->type)
	    {
	    पक्ष FFI_TYPE_SINT8:
	      *(signed पूर्णाङ्क *) argp = (signed पूर्णाङ्क) *(SINT8 *) *p_argv;
	      भङ्ग;

	    पक्ष FFI_TYPE_UINT8:
	      *(unsigned पूर्णाङ्क *) argp = (unsigned पूर्णाङ्क) *(UINT8 *) *p_argv;
	      भङ्ग;

	    पक्ष FFI_TYPE_SINT16:
	      *(signed पूर्णाङ्क *) argp = (signed पूर्णाङ्क) *(SINT16 *) *p_argv;
	      भङ्ग;

	    पक्ष FFI_TYPE_UINT16:
	      *(unsigned पूर्णाङ्क *) argp = (unsigned पूर्णाङ्क) *(UINT16 *) *p_argv;
	      भङ्ग;

	    पक्ष FFI_TYPE_STRUCT:
	      memcpy (argp, *p_argv, z);
	      भङ्ग;

	    स्वभाव:
	      FFI_ASSERT (0);
	    }
	  z = परिमाण (पूर्णाङ्क);
	}
      अण्णहा
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  जइ ((परिमाण(पूर्णाङ्क) - 1) & z)
	    z = FFI_ALIGN(z, परिमाण(पूर्णाङ्क));
	}

      p_argv++;
      argp += z;
    }

  प्रत्यागम struct_मानम्_ptr;
}

ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  विकल्प (cif->rtype->type)
    {
    पक्ष FFI_TYPE_VOID:
      cif->flags = 0;
      भङ्ग;

    पक्ष FFI_TYPE_STRUCT:
      जइ (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
	  cif->rtype->elements[1])
	{
	  cif->flags = 0;
	  भङ्ग;
	}

      जइ (cif->rtype->size == परिमाण (वर्ण))
	cif->flags = CIF_FLAGS_CHAR;
      अण्णहा जइ (cif->rtype->size == परिमाण (short))
	cif->flags = CIF_FLAGS_SHORT;
      अण्णहा जइ (cif->rtype->size == परिमाण (पूर्णाङ्क))
	cif->flags = CIF_FLAGS_INT;
      अण्णहा जइ (cif->rtype->size == 2 * परिमाण (पूर्णाङ्क))
	cif->flags = CIF_FLAGS_DINT;
      अण्णहा
	cif->flags = 0;
      भङ्ग;

    स्वभाव:
      जइ (cif->rtype->size <= परिमाण (पूर्णाङ्क))
	cif->flags = CIF_FLAGS_INT;
      अण्णहा
	cif->flags = CIF_FLAGS_DINT;
      भङ्ग;
    }

  प्रत्यागम FFI_OK;
}

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn) (), शून्य *rvalue, शून्य **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  जइ (rvalue == रिक्त
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->flags == 0)
    ecif.rvalue = alloca (cif->rtype->size);
  अण्णहा
    ecif.rvalue = rvalue;

  विकल्प (cif->abi)
    {
    पक्ष FFI_ELFBSD:
      ffi_call_elfbsd (&ecif, cif->bytes, cif->flags, ecif.rvalue, fn);
      भङ्ग;

    स्वभाव:
      FFI_ASSERT (0);
      भङ्ग;
    }
}

/*
 * Closure API
 */

शून्य ffi_closure_elfbsd (शून्य);
शून्य ffi_closure_struct_elfbsd (शून्य);
unsigned पूर्णाङ्क ffi_closure_elfbsd_inner (ffi_closure *, शून्य *, वर्ण *);

स्थिर शून्य
ffi_prep_closure_elfbsd (ffi_cif *cif, शून्य **avalue, वर्ण *stackp)
{
  unsigned पूर्णाङ्क i;
  शून्य **p_argv;
  ffi_type **p_arg;

  p_argv = avalue;

  प्रति (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      *p_argv = stackp;

      /* Align if necessary */
      जइ ((परिमाण (पूर्णाङ्क) - 1) & z)
	z = FFI_ALIGN(z, परिमाण (पूर्णाङ्क));

      p_argv++;
      stackp += z;
    }
}

unsigned पूर्णाङ्क
ffi_closure_elfbsd_inner (ffi_closure *closure, शून्य *resp, वर्ण *राशिः)
{
  ffi_cif *cif;
  शून्य **arg_area;

  cif = closure->cif;
  arg_area = (शून्य **) alloca (cif->nargs * परिमाण (शून्य *));

  ffi_prep_closure_elfbsd (cif, arg_area, राशिः);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  प्रत्यागम cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      शून्य (*fun)(ffi_cif *, शून्य *, शून्य **, शून्य *),
		      शून्य *user_data, शून्य *codeloc)
{
  वर्ण *tramp = (वर्ण *) codeloc;
  शून्य *fn;

  FFI_ASSERT (cif->abi == FFI_ELFBSD);

  /* entry mask */
  *(unsigned short *)(tramp + 0) = 0x0000;
  /* movl #closure, r0 */
  tramp[2] = 0xd0;
  tramp[3] = 0x8f;
  *(unsigned पूर्णाङ्क *)(tramp + 4) = (unsigned पूर्णाङ्क) closure;
  tramp[8] = 0x50;

  जइ (cif->rtype->type == FFI_TYPE_STRUCT
      && !cif->flags)
    fn = &ffi_closure_struct_elfbsd;
  अण्णहा
    fn = &ffi_closure_elfbsd;

  /* jmpl #fn */
  tramp[9] = 0x17;
  tramp[10] = 0xef;
  *(unsigned पूर्णाङ्क *)(tramp + 11) = (unsigned पूर्णाङ्क)fn + 2 -
				  (unsigned पूर्णाङ्क)tramp - 9 - 6;

  closure->cif = cif;
  closure->user_data = user_data;
  closure->fun = fun;

  प्रत्यागम FFI_OK;
}
