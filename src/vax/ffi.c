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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIF_FLAGS_CHAR		1	/* for struct only */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIF_FLAGS_SHORT		2	/* for struct only */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIF_FLAGS_INT		4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CIF_FLAGS_DINT		8

/*
 * Foreign Function Interface API
 */

ਖਾਲੀ ffi_call_elfbsd (extended_cif *, ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ, ਖਾਲੀ *,
		       ਖਾਲੀ (*) ());
ਖਾਲੀ *ffi_prep_ਦਲੀਲਾਂ (extended_cif *ecif, ਖਾਲੀ *ਢੇਰ);

ਖਾਲੀ *
ffi_prep_ਦਲੀਲਾਂ (extended_cif *ecif, ਖਾਲੀ *ਢੇਰ)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਖਾਲੀ **p_argv;
  ਅੱਖਰ *argp;
  ffi_type **p_arg;
  ਖਾਲੀ *struct_ਮੁੱਲ_ptr;

  argp = ਢੇਰ;

  ਜੇ (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_ਮੁੱਲ_ptr = ecif->rvalue;
  ਨਹੀਂਤਾਂ
    struct_ਮੁੱਲ_ptr = ਨਿਰਾ;

  p_argv = ecif->avalue;

  ਲਈ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->ਮਾਪ;
      ਜੇ (z < ਆਕਾਰ (ਪੂਰਨ))
	{
	  ਚੋਣ ((*p_arg)->type)
	    {
	    ਹਾਲਤ FFI_TYPE_SINT8:
	      *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ) *(SINT8 *) *p_argv;
	      ਰੋਕੋ;

	    ਹਾਲਤ FFI_TYPE_UINT8:
	      *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) *(UINT8 *) *p_argv;
	      ਰੋਕੋ;

	    ਹਾਲਤ FFI_TYPE_SINT16:
	      *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ) *(SINT16 *) *p_argv;
	      ਰੋਕੋ;

	    ਹਾਲਤ FFI_TYPE_UINT16:
	      *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) *(UINT16 *) *p_argv;
	      ਰੋਕੋ;

	    ਹਾਲਤ FFI_TYPE_STRUCT:
	      ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, *p_argv, z);
	      ਰੋਕੋ;

	    ਮੂਲ:
	      FFI_ASSERT (0);
	    }
	  z = ਆਕਾਰ (ਪੂਰਨ);
	}
      ਨਹੀਂਤਾਂ
	{
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, *p_argv, z);

	  /* Align if necessary.  */
	  ਜੇ ((ਆਕਾਰ(ਪੂਰਨ) - 1) & z)
	    z = FFI_ALIGN(z, ਆਕਾਰ(ਪੂਰਨ));
	}

      p_argv++;
      argp += z;
    }

  ਵਾਪਸ struct_ਮੁੱਲ_ptr;
}

ffi_ਸਥਿਤੀ
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  ਚੋਣ (cif->rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_VOID:
      cif->flags = 0;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_STRUCT:
      ਜੇ (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
	  cif->rtype->elements[1])
	{
	  cif->flags = 0;
	  ਰੋਕੋ;
	}

      ਜੇ (cif->rtype->ਮਾਪ == ਆਕਾਰ (ਅੱਖਰ))
	cif->flags = CIF_FLAGS_CHAR;
      ਨਹੀਂਤਾਂ ਜੇ (cif->rtype->ਮਾਪ == ਆਕਾਰ (ਛੋਟਾ))
	cif->flags = CIF_FLAGS_SHORT;
      ਨਹੀਂਤਾਂ ਜੇ (cif->rtype->ਮਾਪ == ਆਕਾਰ (ਪੂਰਨ))
	cif->flags = CIF_FLAGS_INT;
      ਨਹੀਂਤਾਂ ਜੇ (cif->rtype->ਮਾਪ == 2 * ਆਕਾਰ (ਪੂਰਨ))
	cif->flags = CIF_FLAGS_DINT;
      ਨਹੀਂਤਾਂ
	cif->flags = 0;
      ਰੋਕੋ;

    ਮੂਲ:
      ਜੇ (cif->rtype->ਮਾਪ <= ਆਕਾਰ (ਪੂਰਨ))
	cif->flags = CIF_FLAGS_INT;
      ਨਹੀਂਤਾਂ
	cif->flags = CIF_FLAGS_DINT;
      ਰੋਕੋ;
    }

  ਵਾਪਸ FFI_OK;
}

ਖਾਲੀ
ffi_call (ffi_cif *cif, ਖਾਲੀ (*fn) (), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  ਜੇ (rvalue == ਨਿਰਾ
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->flags == 0)
    ecif.rvalue = alloca (cif->rtype->ਮਾਪ);
  ਨਹੀਂਤਾਂ
    ecif.rvalue = rvalue;

  ਚੋਣ (cif->abi)
    {
    ਹਾਲਤ FFI_ELFBSD:
      ffi_call_elfbsd (&ecif, cif->ਬਾਈਟਾਂ, cif->flags, ecif.rvalue, fn);
      ਰੋਕੋ;

    ਮੂਲ:
      FFI_ASSERT (0);
      ਰੋਕੋ;
    }
}

/*
 * Closure API
 */

ਖਾਲੀ ffi_ਘੇਰਾ_elfbsd (ਖਾਲੀ);
ਖਾਲੀ ffi_ਘੇਰਾ_struct_elfbsd (ਖਾਲੀ);
ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ffi_ਘੇਰਾ_elfbsd_inner (ffi_ਘੇਰਾ *, ਖਾਲੀ *, ਅੱਖਰ *);

ਸਥਿਰ ਖਾਲੀ
ffi_prep_ਘੇਰਾ_elfbsd (ffi_cif *cif, ਖਾਲੀ **avalue, ਅੱਖਰ *stackp)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਖਾਲੀ **p_argv;
  ffi_type **p_arg;

  p_argv = avalue;

  ਲਈ (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->ਮਾਪ;
      *p_argv = stackp;

      /* Align if necessary */
      ਜੇ ((ਆਕਾਰ (ਪੂਰਨ) - 1) & z)
	z = FFI_ALIGN(z, ਆਕਾਰ (ਪੂਰਨ));

      p_argv++;
      stackp += z;
    }
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ
ffi_ਘੇਰਾ_elfbsd_inner (ffi_ਘੇਰਾ *ਘੇਰਾ, ਖਾਲੀ *resp, ਅੱਖਰ *ਢੇਰ)
{
  ffi_cif *cif;
  ਖਾਲੀ **arg_area;

  cif = ਘੇਰਾ->cif;
  arg_area = (ਖਾਲੀ **) alloca (cif->nargs * ਆਕਾਰ (ਖਾਲੀ *));

  ffi_prep_ਘੇਰਾ_elfbsd (cif, arg_area, ਢੇਰ);

  (ਘੇਰਾ->fun) (cif, resp, arg_area, ਘੇਰਾ->user_ਅੰਕੜੇ);

  ਵਾਪਸ cif->flags;
}

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ *ਘੇਰਾ, ffi_cif *cif,
		      ਖਾਲੀ (*fun)(ffi_cif *, ਖਾਲੀ *, ਖਾਲੀ **, ਖਾਲੀ *),
		      ਖਾਲੀ *user_ਅੰਕੜੇ, ਖਾਲੀ *codeloc)
{
  ਅੱਖਰ *tramp = (ਅੱਖਰ *) codeloc;
  ਖਾਲੀ *fn;

  FFI_ASSERT (cif->abi == FFI_ELFBSD);

  /* entry mask */
  *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ *)(tramp + 0) = 0x0000;
  /* movl #closure, r0 */
  tramp[2] = 0xd0;
  tramp[3] = 0x8f;
  *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *)(tramp + 4) = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) ਘੇਰਾ;
  tramp[8] = 0x50;

  ਜੇ (cif->rtype->type == FFI_TYPE_STRUCT
      && !cif->flags)
    fn = &ffi_ਘੇਰਾ_struct_elfbsd;
  ਨਹੀਂਤਾਂ
    fn = &ffi_ਘੇਰਾ_elfbsd;

  /* jmpl #fn */
  tramp[9] = 0x17;
  tramp[10] = 0xef;
  *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *)(tramp + 11) = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)fn + 2 -
				  (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)tramp - 9 - 6;

  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;
  ਘੇਰਾ->fun = fun;

  ਵਾਪਸ FFI_OK;
}
