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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <unistd.h>

#define CIF_FLAGS_CHAR		1	/* for struct only */
#define CIF_FLAGS_SHORT		2	/* for struct only */
#define CIF_FLAGS_INT		4
#define CIF_FLAGS_DINT		8

/*
 * Foreign Function Interface API
 */

শূন্য ffi_call_elfbsd (extended_cif *, unsigned, unsigned, শূন্য *,
		       শূন্য (*) ());
শূন্য *ffi_prep_args (extended_cif *ecif, শূন্য *স্তূপ);

শূন্য *
ffi_prep_args (extended_cif *ecif, শূন্য *স্তূপ)
{
  unsigned পূর্ণসংখ্যা i;
  শূন্য **p_argv;
  অক্ষর *argp;
  ffi_type **p_arg;
  শূন্য *struct_মান_ptr;

  argp = স্তূপ;

  যদি (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_মান_ptr = ecif->rvalue;
  অন্যথা
    struct_মান_ptr = ফাঁকা;

  p_argv = ecif->avalue;

  জন্য (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->আকার_2;
      যদি (z < আকার (পূর্ণসংখ্যা))
	{
	  নির্বাচন ((*p_arg)->type)
	    {
	    ক্ষেত্র FFI_TYPE_SINT8:
	      *(signed পূর্ণসংখ্যা *) argp = (signed পূর্ণসংখ্যা) *(SINT8 *) *p_argv;
	      বিরতি;

	    ক্ষেত্র FFI_TYPE_UINT8:
	      *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা) *(UINT8 *) *p_argv;
	      বিরতি;

	    ক্ষেত্র FFI_TYPE_SINT16:
	      *(signed পূর্ণসংখ্যা *) argp = (signed পূর্ণসংখ্যা) *(SINT16 *) *p_argv;
	      বিরতি;

	    ক্ষেত্র FFI_TYPE_UINT16:
	      *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা) *(UINT16 *) *p_argv;
	      বিরতি;

	    ক্ষেত্র FFI_TYPE_STRUCT:
	      memcpy (argp, *p_argv, z);
	      বিরতি;

	    পূর্বনির্ধারিত:
	      FFI_ASSERT (0);
	    }
	  z = আকার (পূর্ণসংখ্যা);
	}
      অন্যথা
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  যদি ((আকার(পূর্ণসংখ্যা) - 1) & z)
	    z = FFI_ALIGN(z, আকার(পূর্ণসংখ্যা));
	}

      p_argv++;
      argp += z;
    }

  ফেরত struct_মান_ptr;
}

ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  নির্বাচন (cif->rtype->type)
    {
    ক্ষেত্র FFI_TYPE_VOID:
      cif->flags = 0;
      বিরতি;

    ক্ষেত্র FFI_TYPE_STRUCT:
      যদি (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
	  cif->rtype->elements[1])
	{
	  cif->flags = 0;
	  বিরতি;
	}

      যদি (cif->rtype->আকার_2 == আকার (অক্ষর))
	cif->flags = CIF_FLAGS_CHAR;
      অন্যথা যদি (cif->rtype->আকার_2 == আকার (short))
	cif->flags = CIF_FLAGS_SHORT;
      অন্যথা যদি (cif->rtype->আকার_2 == আকার (পূর্ণসংখ্যা))
	cif->flags = CIF_FLAGS_INT;
      অন্যথা যদি (cif->rtype->আকার_2 == 2 * আকার (পূর্ণসংখ্যা))
	cif->flags = CIF_FLAGS_DINT;
      অন্যথা
	cif->flags = 0;
      বিরতি;

    পূর্বনির্ধারিত:
      যদি (cif->rtype->আকার_2 <= আকার (পূর্ণসংখ্যা))
	cif->flags = CIF_FLAGS_INT;
      অন্যথা
	cif->flags = CIF_FLAGS_DINT;
      বিরতি;
    }

  ফেরত FFI_OK;
}

শূন্য
ffi_call (ffi_cif *cif, শূন্য (*fn) (), শূন্য *rvalue, শূন্য **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  যদি (rvalue == ফাঁকা
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->flags == 0)
    ecif.rvalue = alloca (cif->rtype->আকার_2);
  অন্যথা
    ecif.rvalue = rvalue;

  নির্বাচন (cif->abi)
    {
    ক্ষেত্র FFI_ELFBSD:
      ffi_call_elfbsd (&ecif, cif->bytes, cif->flags, ecif.rvalue, fn);
      বিরতি;

    পূর্বনির্ধারিত:
      FFI_ASSERT (0);
      বিরতি;
    }
}

/*
 * Closure API
 */

শূন্য ffi_closure_elfbsd (শূন্য);
শূন্য ffi_closure_struct_elfbsd (শূন্য);
unsigned পূর্ণসংখ্যা ffi_closure_elfbsd_inner (ffi_closure *, শূন্য *, অক্ষর *);

স্থির শূন্য
ffi_prep_closure_elfbsd (ffi_cif *cif, শূন্য **avalue, অক্ষর *stackp)
{
  unsigned পূর্ণসংখ্যা i;
  শূন্য **p_argv;
  ffi_type **p_arg;

  p_argv = avalue;

  জন্য (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->আকার_2;
      *p_argv = stackp;

      /* Align if necessary */
      যদি ((আকার (পূর্ণসংখ্যা) - 1) & z)
	z = FFI_ALIGN(z, আকার (পূর্ণসংখ্যা));

      p_argv++;
      stackp += z;
    }
}

unsigned পূর্ণসংখ্যা
ffi_closure_elfbsd_inner (ffi_closure *closure, শূন্য *resp, অক্ষর *স্তূপ)
{
  ffi_cif *cif;
  শূন্য **arg_area;

  cif = closure->cif;
  arg_area = (শূন্য **) alloca (cif->nargs * আকার (শূন্য *));

  ffi_prep_closure_elfbsd (cif, arg_area, স্তূপ);

  (closure->fun) (cif, resp, arg_area, closure->user_তথ্য);

  ফেরত cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      শূন্য (*fun)(ffi_cif *, শূন্য *, শূন্য **, শূন্য *),
		      শূন্য *user_তথ্য, শূন্য *codeloc)
{
  অক্ষর *tramp = (অক্ষর *) codeloc;
  শূন্য *fn;

  FFI_ASSERT (cif->abi == FFI_ELFBSD);

  /* entry mask */
  *(unsigned short *)(tramp + 0) = 0x0000;
  /* movl #closure, r0 */
  tramp[2] = 0xd0;
  tramp[3] = 0x8f;
  *(unsigned পূর্ণসংখ্যা *)(tramp + 4) = (unsigned পূর্ণসংখ্যা) closure;
  tramp[8] = 0x50;

  যদি (cif->rtype->type == FFI_TYPE_STRUCT
      && !cif->flags)
    fn = &ffi_closure_struct_elfbsd;
  অন্যথা
    fn = &ffi_closure_elfbsd;

  /* jmpl #fn */
  tramp[9] = 0x17;
  tramp[10] = 0xef;
  *(unsigned পূর্ণসংখ্যা *)(tramp + 11) = (unsigned পূর্ণসংখ্যা)fn + 2 -
				  (unsigned পূর্ণসংখ্যা)tramp - 9 - 6;

  closure->cif = cif;
  closure->user_তথ্য = user_তথ্য;
  closure->fun = fun;

  ফেরত FFI_OK;
}
