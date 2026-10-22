/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2004  Renesas Technology
           Copyright (c) 2008  Red Hat, Inc.
           Copyright (c) 2022, 2026  Anthony Green

   M32R Foreign Function Interface

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
   IN NO EVENT SHALL RENESAS TECHNOLOGY BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack
   space has been allocated for the function's arguments.  */

ਖਾਲੀ ffi_prep_ਦਲੀਲਾਂ(ਅੱਖਰ *ਢੇਰ, extended_cif *ecif)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਪੂਰਨ tmp;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ avn;
  ਖਾਲੀ **p_argv;
  ਅੱਖਰ *argp;
  ffi_type **p_arg;

  tmp = 0;
  argp = ਢੇਰ;

  ਜੇ (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->ਮਾਪ > 8)
    {
      *(ਖਾਲੀ **) argp = ecif->rvalue;
      argp += 4;
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  ਲਈ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0) && (avn != 0);
       i--, p_arg++)
    {
      size_t z;

      /* Align if necessary.  */
      ਜੇ (((*p_arg)->alignment - 1) & (ਬਿਨਾਨਿਸ਼ਾਨੀ) argp)
	argp = (ਅੱਖਰ *) FFI_ALIGN (argp, (*p_arg)->alignment);

      ਜੇ (avn != 0)
	{
	  avn--;
	  z = (*p_arg)->ਮਾਪ;
	  ਜੇ (z < ਆਕਾਰ (ਪੂਰਨ))
	    {
	      z = ਆਕਾਰ (ਪੂਰਨ);

	      ਚੋਣ ((*p_arg)->type)
		{
		ਹਾਲਤ FFI_TYPE_SINT8:
		  *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ)*(SINT8 *)(* p_argv);
		  ਰੋਕੋ;

		ਹਾਲਤ FFI_TYPE_UINT8:
		  *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT8 *)(* p_argv);
		  ਰੋਕੋ;

		ਹਾਲਤ FFI_TYPE_SINT16:
		  *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ)*(SINT16 *)(* p_argv);
		  ਰੋਕੋ;

		ਹਾਲਤ FFI_TYPE_UINT16:
		  *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT16 *)(* p_argv);
		  ਰੋਕੋ;

		ਹਾਲਤ FFI_TYPE_STRUCT:
	  	  z = (*p_arg)->ਮਾਪ;
	  	  ਜੇ ((*p_arg)->alignment != 1)
		    ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, *p_argv, z);
		  ਨਹੀਂਤਾਂ
		    ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp + 4 - z, *p_argv, z);
	  	  z = ਆਕਾਰ (ਪੂਰਨ);
		  ਰੋਕੋ;

		ਮੂਲ:
		  FFI_ASSERT(0);
		}
	    }
	  ਨਹੀਂਤਾਂ ਜੇ (z == ਆਕਾਰ (ਪੂਰਨ))
	    {
	       *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT32 *)(* p_argv);
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      ਜੇ ((*p_arg)->type == FFI_TYPE_STRUCT)
	        {
		  ਜੇ (z > 8)
		    {
		      *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(ਖਾਲੀ *)(* p_argv);
		      z = ਆਕਾਰ(ਖਾਲੀ *);
		    }
		  ਨਹੀਂਤਾਂ
		    {
	              ਸਮ੍ਰਿਤੀ_ਨਕਲ(argp, *p_argv, z);
		      z = 8;
		    }
	        }
	      ਨਹੀਂਤਾਂ
	        {
		  /* Double or long long 64bit.  */
	          ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, *p_argv, z);
	        }
	    }
	  p_argv++;
	  argp += z;
	}
    }

  ਵਾਪਸ;
}

/* Perform machine dependent cif processing.  */
ffi_ਸਥਿਤੀ
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag.  */
  ਚੋਣ (cif->rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_VOID:
      cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ) cif->rtype->type;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_STRUCT:
      ਜੇ (cif->rtype->ਮਾਪ <= 4)
	cif->flags = FFI_TYPE_INT;

      ਨਹੀਂਤਾਂ ਜੇ (cif->rtype->ਮਾਪ <= 8)
	cif->flags = FFI_TYPE_DOUBLE;

      ਨਹੀਂਤਾਂ
	cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ) cif->rtype->type;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_FLOAT:
    ਮੂਲ:
      cif->flags = FFI_TYPE_INT;
      ਰੋਕੋ;
    }

  ਵਾਪਸ FFI_OK;
}

ਬਾਹਰੀ ਖਾਲੀ ffi_call_SYSV(ਖਾਲੀ (*)(ਅੱਖਰ *, extended_cif *), extended_cif *,
			  ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ *, ਖਾਲੀ (*fn)(ਖਾਲੀ));

ਖਾਲੀ ffi_call(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  ਖਾਲੀ **avalue_copy = ਨਿਰਾ;
  ਪੂਰਨ i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have
     a return value address then we need to make one.  */
  ਜੇ ((rvalue == ਨਿਰਾ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->ਮਾਪ);
    }
  ਨਹੀਂਤਾਂ
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ਲਈ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ਪੂਰਨ ਮਾਪ = at->ਮਾਪ;
      ਜੇ (at->type == FFI_TYPE_STRUCT && ਮਾਪ > 4)
        {
          ਅੱਖਰ *argcopy = alloca (ਮਾਪ);
          ਜੇ (avalue_copy == ਨਿਰਾ)
            {
              avalue_copy = alloca (nargs * ਆਕਾਰ (ਖਾਲੀ *));
              ਸਮ੍ਰਿਤੀ_ਨਕਲ (avalue_copy, avalue, nargs * ਆਕਾਰ (ਖਾਲੀ *));
              avalue = avalue_copy;
            }
          ਸਮ੍ਰਿਤੀ_ਨਕਲ (argcopy, avalue[i], ਮਾਪ);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  ਚੋਣ (cif->abi)
    {
    ਹਾਲਤ FFI_SYSV:
      ffi_call_SYSV(ffi_prep_ਦਲੀਲਾਂ, &ecif, cif->ਬਾਈਟਾਂ,
		    cif->flags, ecif.rvalue, fn);
      ਜੇ (cif->rtype->type == FFI_TYPE_STRUCT)
	{
	  ਪੂਰਨ ਮਾਪ = cif->rtype->ਮਾਪ;
	  ਪੂਰਨ align = cif->rtype->alignment;

	  ਜੇ (ਮਾਪ < 4)
	    {
	      ਜੇ (align == 1)
	        *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *)(ecif.rvalue) <<= (4 - ਮਾਪ) * 8;
	    }
	  ਨਹੀਂਤਾਂ ਜੇ (4 < ਮਾਪ && ਮਾਪ < 8)
	    {
	      ਜੇ (align == 1)
		{
		  ਸਮ੍ਰਿਤੀ_ਨਕਲ (ecif.rvalue, ecif.rvalue + 8-ਮਾਪ, ਮਾਪ);
		}
	      ਨਹੀਂਤਾਂ ਜੇ (align == 2)
		{
		  ਜੇ (ਮਾਪ & 1)
		    ਮਾਪ += 1;

		  ਜੇ (ਮਾਪ != 8)
		    ਸਮ੍ਰਿਤੀ_ਨਕਲ (ecif.rvalue, ecif.rvalue + 8-ਮਾਪ, ਮਾਪ);
		}
	    }
	}
      ਰੋਕੋ;

    ਮੂਲ:
      FFI_ASSERT(0);
      ਰੋਕੋ;
    }
}
