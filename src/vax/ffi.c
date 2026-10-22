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
  unsigned পূর্ণাঙ্ক i;
  শূন্য **p_argv;
  বর্ণ *argp;
  ffi_type **p_arg;
  শূন্য *struct_মান_ptr;

  argp = স্তূপ;

  যদি (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_মান_ptr = ecif->rvalue;
  অন্যথা
    struct_মান_ptr = রিক্ত;

  p_argv = ecif->avalue;

  প্রতি (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      যদি (z < পরিমাণ (পূর্ণাঙ্ক))
	{
	  বিকল্প ((*p_arg)->type)
	    {
	    পক্ষ FFI_TYPE_SINT8:
	      *(signed পূর্ণাঙ্ক *) argp = (signed পূর্ণাঙ্ক) *(SINT8 *) *p_argv;
	      ভঙ্গ;

	    পক্ষ FFI_TYPE_UINT8:
	      *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক) *(UINT8 *) *p_argv;
	      ভঙ্গ;

	    পক্ষ FFI_TYPE_SINT16:
	      *(signed পূর্ণাঙ্ক *) argp = (signed পূর্ণাঙ্ক) *(SINT16 *) *p_argv;
	      ভঙ্গ;

	    পক্ষ FFI_TYPE_UINT16:
	      *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক) *(UINT16 *) *p_argv;
	      ভঙ্গ;

	    পক্ষ FFI_TYPE_STRUCT:
	      memcpy (argp, *p_argv, z);
	      ভঙ্গ;

	    স্বভাব:
	      FFI_ASSERT (0);
	    }
	  z = পরিমাণ (পূর্ণাঙ্ক);
	}
      অন্যথা
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  যদি ((পরিমাণ(পূর্ণাঙ্ক) - 1) & z)
	    z = FFI_ALIGN(z, পরিমাণ(পূর্ণাঙ্ক));
	}

      p_argv++;
      argp += z;
    }

  প্রত্যাগম struct_মান_ptr;
}

ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  বিকল্প (cif->rtype->type)
    {
    পক্ষ FFI_TYPE_VOID:
      cif->flags = 0;
      ভঙ্গ;

    পক্ষ FFI_TYPE_STRUCT:
      যদি (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
	  cif->rtype->elements[1])
	{
	  cif->flags = 0;
	  ভঙ্গ;
	}

      যদি (cif->rtype->size == পরিমাণ (বর্ণ))
	cif->flags = CIF_FLAGS_CHAR;
      অন্যথা যদি (cif->rtype->size == পরিমাণ (short))
	cif->flags = CIF_FLAGS_SHORT;
      অন্যথা যদি (cif->rtype->size == পরিমাণ (পূর্ণাঙ্ক))
	cif->flags = CIF_FLAGS_INT;
      অন্যথা যদি (cif->rtype->size == 2 * পরিমাণ (পূর্ণাঙ্ক))
	cif->flags = CIF_FLAGS_DINT;
      অন্যথা
	cif->flags = 0;
      ভঙ্গ;

    স্বভাব:
      যদি (cif->rtype->size <= পরিমাণ (পূর্ণাঙ্ক))
	cif->flags = CIF_FLAGS_INT;
      অন্যথা
	cif->flags = CIF_FLAGS_DINT;
      ভঙ্গ;
    }

  প্রত্যাগম FFI_OK;
}

শূন্য
ffi_call (ffi_cif *cif, শূন্য (*fn) (), শূন্য *rvalue, শূন্য **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  যদি (rvalue == রিক্ত
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->flags == 0)
    ecif.rvalue = alloca (cif->rtype->size);
  অন্যথা
    ecif.rvalue = rvalue;

  বিকল্প (cif->abi)
    {
    পক্ষ FFI_ELFBSD:
      ffi_call_elfbsd (&ecif, cif->bytes, cif->flags, ecif.rvalue, fn);
      ভঙ্গ;

    স্বভাব:
      FFI_ASSERT (0);
      ভঙ্গ;
    }
}

/*
 * Closure API
 */

শূন্য ffi_closure_elfbsd (শূন্য);
শূন্য ffi_closure_struct_elfbsd (শূন্য);
unsigned পূর্ণাঙ্ক ffi_closure_elfbsd_inner (ffi_closure *, শূন্য *, বর্ণ *);

স্থির শূন্য
ffi_prep_closure_elfbsd (ffi_cif *cif, শূন্য **avalue, বর্ণ *stackp)
{
  unsigned পূর্ণাঙ্ক i;
  শূন্য **p_argv;
  ffi_type **p_arg;

  p_argv = avalue;

  প্রতি (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      *p_argv = stackp;

      /* Align if necessary */
      যদি ((পরিমাণ (পূর্ণাঙ্ক) - 1) & z)
	z = FFI_ALIGN(z, পরিমাণ (পূর্ণাঙ্ক));

      p_argv++;
      stackp += z;
    }
}

unsigned পূর্ণাঙ্ক
ffi_closure_elfbsd_inner (ffi_closure *closure, শূন্য *resp, বর্ণ *স্তূপ)
{
  ffi_cif *cif;
  শূন্য **arg_area;

  cif = closure->cif;
  arg_area = (শূন্য **) alloca (cif->nargs * পরিমাণ (শূন্য *));

  ffi_prep_closure_elfbsd (cif, arg_area, স্তূপ);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  প্রত্যাগম cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      শূন্য (*fun)(ffi_cif *, শূন্য *, শূন্য **, শূন্য *),
		      শূন্য *user_data, শূন্য *codeloc)
{
  বর্ণ *tramp = (বর্ণ *) codeloc;
  শূন্য *fn;

  FFI_ASSERT (cif->abi == FFI_ELFBSD);

  /* entry mask */
  *(unsigned short *)(tramp + 0) = 0x0000;
  /* movl #closure, r0 */
  tramp[2] = 0xd0;
  tramp[3] = 0x8f;
  *(unsigned পূর্ণাঙ্ক *)(tramp + 4) = (unsigned পূর্ণাঙ্ক) closure;
  tramp[8] = 0x50;

  যদি (cif->rtype->type == FFI_TYPE_STRUCT
      && !cif->flags)
    fn = &ffi_closure_struct_elfbsd;
  অন্যথা
    fn = &ffi_closure_elfbsd;

  /* jmpl #fn */
  tramp[9] = 0x17;
  tramp[10] = 0xef;
  *(unsigned পূর্ণাঙ্ক *)(tramp + 11) = (unsigned পূর্ণাঙ্ক)fn + 2 -
				  (unsigned পূর্ণাঙ্ক)tramp - 9 - 6;

  closure->cif = cif;
  closure->user_data = user_data;
  closure->fun = fun;

  প্রত্যাগম FFI_OK;
}
