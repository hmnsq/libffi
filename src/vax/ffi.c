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

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdlib.h>
#شامل <unistd.h>

#define CIF_FLAGS_CHAR		1	/* for struct only */
#define CIF_FLAGS_SHORT		2	/* for struct only */
#define CIF_FLAGS_INT		4
#define CIF_FLAGS_DINT		8

/*
 * Foreign Function Interface API
 */

تهی ffi_call_elfbsd (extended_cif *, unsigned, unsigned, تهی *,
		       تهی (*) ());
تهی *ffi_prep_args (extended_cif *ecif, تهی *پشته);

تهی *
ffi_prep_args (extended_cif *ecif, تهی *پشته)
{
  unsigned صحیح i;
  تهی **p_argv;
  نویسه *argp;
  ffi_type **p_arg;
  تهی *struct_مقدار_ptr;

  argp = پشته;

  اگر (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_مقدار_ptr = ecif->rvalue;
  وگرنه
    struct_مقدار_ptr = پوچ;

  p_argv = ecif->avalue;

  برای (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      اگر (z < اندازه (صحیح))
	{
	  انتخاب ((*p_arg)->type)
	    {
	    حالت FFI_TYPE_SINT8:
	      *(signed صحیح *) argp = (signed صحیح) *(SINT8 *) *p_argv;
	      بشکن;

	    حالت FFI_TYPE_UINT8:
	      *(unsigned صحیح *) argp = (unsigned صحیح) *(UINT8 *) *p_argv;
	      بشکن;

	    حالت FFI_TYPE_SINT16:
	      *(signed صحیح *) argp = (signed صحیح) *(SINT16 *) *p_argv;
	      بشکن;

	    حالت FFI_TYPE_UINT16:
	      *(unsigned صحیح *) argp = (unsigned صحیح) *(UINT16 *) *p_argv;
	      بشکن;

	    حالت FFI_TYPE_STRUCT:
	      memcpy (argp, *p_argv, z);
	      بشکن;

	    پیشفرض:
	      FFI_ASSERT (0);
	    }
	  z = اندازه (صحیح);
	}
      وگرنه
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  اگر ((اندازه(صحیح) - 1) & z)
	    z = FFI_ALIGN(z, اندازه(صحیح));
	}

      p_argv++;
      argp += z;
    }

  بازگشت struct_مقدار_ptr;
}

ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_VOID:
      cif->flags = 0;
      بشکن;

    حالت FFI_TYPE_STRUCT:
      اگر (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
	  cif->rtype->elements[1])
	{
	  cif->flags = 0;
	  بشکن;
	}

      اگر (cif->rtype->size == اندازه (نویسه))
	cif->flags = CIF_FLAGS_CHAR;
      وگرنه اگر (cif->rtype->size == اندازه (short))
	cif->flags = CIF_FLAGS_SHORT;
      وگرنه اگر (cif->rtype->size == اندازه (صحیح))
	cif->flags = CIF_FLAGS_INT;
      وگرنه اگر (cif->rtype->size == 2 * اندازه (صحیح))
	cif->flags = CIF_FLAGS_DINT;
      وگرنه
	cif->flags = 0;
      بشکن;

    پیشفرض:
      اگر (cif->rtype->size <= اندازه (صحیح))
	cif->flags = CIF_FLAGS_INT;
      وگرنه
	cif->flags = CIF_FLAGS_DINT;
      بشکن;
    }

  بازگشت FFI_OK;
}

تهی
ffi_call (ffi_cif *cif, تهی (*fn) (), تهی *rvalue, تهی **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  اگر (rvalue == پوچ
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->flags == 0)
    ecif.rvalue = alloca (cif->rtype->size);
  وگرنه
    ecif.rvalue = rvalue;

  انتخاب (cif->abi)
    {
    حالت FFI_ELFBSD:
      ffi_call_elfbsd (&ecif, cif->bytes, cif->flags, ecif.rvalue, fn);
      بشکن;

    پیشفرض:
      FFI_ASSERT (0);
      بشکن;
    }
}

/*
 * Closure API
 */

تهی ffi_closure_elfbsd (تهی);
تهی ffi_closure_struct_elfbsd (تهی);
unsigned صحیح ffi_closure_elfbsd_inner (ffi_closure *, تهی *, نویسه *);

ایستا تهی
ffi_prep_closure_elfbsd (ffi_cif *cif, تهی **avalue, نویسه *stackp)
{
  unsigned صحیح i;
  تهی **p_argv;
  ffi_type **p_arg;

  p_argv = avalue;

  برای (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      *p_argv = stackp;

      /* Align if necessary */
      اگر ((اندازه (صحیح) - 1) & z)
	z = FFI_ALIGN(z, اندازه (صحیح));

      p_argv++;
      stackp += z;
    }
}

unsigned صحیح
ffi_closure_elfbsd_inner (ffi_closure *closure, تهی *resp, نویسه *پشته)
{
  ffi_cif *cif;
  تهی **arg_area;

  cif = closure->cif;
  arg_area = (تهی **) alloca (cif->nargs * اندازه (تهی *));

  ffi_prep_closure_elfbsd (cif, arg_area, پشته);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  بازگشت cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      تهی (*fun)(ffi_cif *, تهی *, تهی **, تهی *),
		      تهی *user_data, تهی *codeloc)
{
  نویسه *tramp = (نویسه *) codeloc;
  تهی *fn;

  FFI_ASSERT (cif->abi == FFI_ELFBSD);

  /* entry mask */
  *(unsigned short *)(tramp + 0) = 0x0000;
  /* movl #closure, r0 */
  tramp[2] = 0xd0;
  tramp[3] = 0x8f;
  *(unsigned صحیح *)(tramp + 4) = (unsigned صحیح) closure;
  tramp[8] = 0x50;

  اگر (cif->rtype->type == FFI_TYPE_STRUCT
      && !cif->flags)
    fn = &ffi_closure_struct_elfbsd;
  وگرنه
    fn = &ffi_closure_elfbsd;

  /* jmpl #fn */
  tramp[9] = 0x17;
  tramp[10] = 0xef;
  *(unsigned صحیح *)(tramp + 11) = (unsigned صحیح)fn + 2 -
				  (unsigned صحیح)tramp - 9 - 6;

  closure->cif = cif;
  closure->user_data = user_data;
  closure->fun = fun;

  بازگشت FFI_OK;
}
