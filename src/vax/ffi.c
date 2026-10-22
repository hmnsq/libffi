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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <unistd.h>

#define CIF_FLAGS_CHAR		1	/* for struct only */
#define CIF_FLAGS_SHORT		2	/* for struct only */
#define CIF_FLAGS_INT		4
#define CIF_FLAGS_DINT		8

/*
 * Foreign Function Interface API
 */

வெறுமை ffi_call_elfbsd (extended_cif *, unsigned, unsigned, வெறுமை *,
		       வெறுமை (*) ());
வெறுமை *ffi_prep_args (extended_cif *ecif, வெறுமை *அடுக்கு);

வெறுமை *
ffi_prep_args (extended_cif *ecif, வெறுமை *அடுக்கு)
{
  unsigned முழு i;
  வெறுமை **p_argv;
  எழுத்து *argp;
  ffi_type **p_arg;
  வெறுமை *struct_மதிப்பு_ptr;

  argp = அடுக்கு;

  எனில் (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_மதிப்பு_ptr = ecif->rvalue;
  இல்லையெனில்
    struct_மதிப்பு_ptr = வெற்று;

  p_argv = ecif->avalue;

  க்கு (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      எனில் (z < அளவு (முழு))
	{
	  தேர்வு ((*p_arg)->type)
	    {
	    நிலை FFI_TYPE_SINT8:
	      *(signed முழு *) argp = (signed முழு) *(SINT8 *) *p_argv;
	      நிறுத்து;

	    நிலை FFI_TYPE_UINT8:
	      *(unsigned முழு *) argp = (unsigned முழு) *(UINT8 *) *p_argv;
	      நிறுத்து;

	    நிலை FFI_TYPE_SINT16:
	      *(signed முழு *) argp = (signed முழு) *(SINT16 *) *p_argv;
	      நிறுத்து;

	    நிலை FFI_TYPE_UINT16:
	      *(unsigned முழு *) argp = (unsigned முழு) *(UINT16 *) *p_argv;
	      நிறுத்து;

	    நிலை FFI_TYPE_STRUCT:
	      memcpy (argp, *p_argv, z);
	      நிறுத்து;

	    இயல்பு:
	      FFI_ASSERT (0);
	    }
	  z = அளவு (முழு);
	}
      இல்லையெனில்
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  எனில் ((அளவு(முழு) - 1) & z)
	    z = FFI_ALIGN(z, அளவு(முழு));
	}

      p_argv++;
      argp += z;
    }

  திரும்பு struct_மதிப்பு_ptr;
}

ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  தேர்வு (cif->rtype->type)
    {
    நிலை FFI_TYPE_VOID:
      cif->flags = 0;
      நிறுத்து;

    நிலை FFI_TYPE_STRUCT:
      எனில் (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
	  cif->rtype->elements[1])
	{
	  cif->flags = 0;
	  நிறுத்து;
	}

      எனில் (cif->rtype->size == அளவு (எழுத்து))
	cif->flags = CIF_FLAGS_CHAR;
      இல்லையெனில் எனில் (cif->rtype->size == அளவு (short))
	cif->flags = CIF_FLAGS_SHORT;
      இல்லையெனில் எனில் (cif->rtype->size == அளவு (முழு))
	cif->flags = CIF_FLAGS_INT;
      இல்லையெனில் எனில் (cif->rtype->size == 2 * அளவு (முழு))
	cif->flags = CIF_FLAGS_DINT;
      இல்லையெனில்
	cif->flags = 0;
      நிறுத்து;

    இயல்பு:
      எனில் (cif->rtype->size <= அளவு (முழு))
	cif->flags = CIF_FLAGS_INT;
      இல்லையெனில்
	cif->flags = CIF_FLAGS_DINT;
      நிறுத்து;
    }

  திரும்பு FFI_OK;
}

வெறுமை
ffi_call (ffi_cif *cif, வெறுமை (*fn) (), வெறுமை *rvalue, வெறுமை **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  எனில் (rvalue == வெற்று
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->flags == 0)
    ecif.rvalue = alloca (cif->rtype->size);
  இல்லையெனில்
    ecif.rvalue = rvalue;

  தேர்வு (cif->abi)
    {
    நிலை FFI_ELFBSD:
      ffi_call_elfbsd (&ecif, cif->bytes, cif->flags, ecif.rvalue, fn);
      நிறுத்து;

    இயல்பு:
      FFI_ASSERT (0);
      நிறுத்து;
    }
}

/*
 * Closure API
 */

வெறுமை ffi_closure_elfbsd (வெறுமை);
வெறுமை ffi_closure_struct_elfbsd (வெறுமை);
unsigned முழு ffi_closure_elfbsd_inner (ffi_closure *, வெறுமை *, எழுத்து *);

நிலையான வெறுமை
ffi_prep_closure_elfbsd (ffi_cif *cif, வெறுமை **avalue, எழுத்து *stackp)
{
  unsigned முழு i;
  வெறுமை **p_argv;
  ffi_type **p_arg;

  p_argv = avalue;

  க்கு (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      *p_argv = stackp;

      /* Align if necessary */
      எனில் ((அளவு (முழு) - 1) & z)
	z = FFI_ALIGN(z, அளவு (முழு));

      p_argv++;
      stackp += z;
    }
}

unsigned முழு
ffi_closure_elfbsd_inner (ffi_closure *closure, வெறுமை *resp, எழுத்து *அடுக்கு)
{
  ffi_cif *cif;
  வெறுமை **arg_area;

  cif = closure->cif;
  arg_area = (வெறுமை **) alloca (cif->nargs * அளவு (வெறுமை *));

  ffi_prep_closure_elfbsd (cif, arg_area, அடுக்கு);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  திரும்பு cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      வெறுமை (*fun)(ffi_cif *, வெறுமை *, வெறுமை **, வெறுமை *),
		      வெறுமை *user_data, வெறுமை *codeloc)
{
  எழுத்து *tramp = (எழுத்து *) codeloc;
  வெறுமை *fn;

  FFI_ASSERT (cif->abi == FFI_ELFBSD);

  /* entry mask */
  *(unsigned short *)(tramp + 0) = 0x0000;
  /* movl #closure, r0 */
  tramp[2] = 0xd0;
  tramp[3] = 0x8f;
  *(unsigned முழு *)(tramp + 4) = (unsigned முழு) closure;
  tramp[8] = 0x50;

  எனில் (cif->rtype->type == FFI_TYPE_STRUCT
      && !cif->flags)
    fn = &ffi_closure_struct_elfbsd;
  இல்லையெனில்
    fn = &ffi_closure_elfbsd;

  /* jmpl #fn */
  tramp[9] = 0x17;
  tramp[10] = 0xef;
  *(unsigned முழு *)(tramp + 11) = (unsigned முழு)fn + 2 -
				  (unsigned முழு)tramp - 9 - 6;

  closure->cif = cif;
  closure->user_data = user_data;
  closure->fun = fun;

  திரும்பு FFI_OK;
}
