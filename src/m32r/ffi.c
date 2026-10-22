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

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack
   space has been allocated for the function's arguments.  */

تهی ffi_prep_args(نویسه *پشته, extended_cif *ecif)
{
  unsigned صحیح i;
  صحیح tmp;
  unsigned صحیح avn;
  تهی **p_argv;
  نویسه *argp;
  ffi_type **p_arg;

  tmp = 0;
  argp = پشته;

  اگر (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 8)
    {
      *(تهی **) argp = ecif->rvalue;
      argp += 4;
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  برای (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0) && (avn != 0);
       i--, p_arg++)
    {
      size_t z;

      /* Align if necessary.  */
      اگر (((*p_arg)->alignment - 1) & (unsigned) argp)
	argp = (نویسه *) FFI_ALIGN (argp, (*p_arg)->alignment);

      اگر (avn != 0)
	{
	  avn--;
	  z = (*p_arg)->size;
	  اگر (z < اندازه (صحیح))
	    {
	      z = اندازه (صحیح);

	      انتخاب ((*p_arg)->type)
		{
		حالت FFI_TYPE_SINT8:
		  *(signed صحیح *) argp = (signed صحیح)*(SINT8 *)(* p_argv);
		  بشکن;

		حالت FFI_TYPE_UINT8:
		  *(unsigned صحیح *) argp = (unsigned صحیح)*(UINT8 *)(* p_argv);
		  بشکن;

		حالت FFI_TYPE_SINT16:
		  *(signed صحیح *) argp = (signed صحیح)*(SINT16 *)(* p_argv);
		  بشکن;

		حالت FFI_TYPE_UINT16:
		  *(unsigned صحیح *) argp = (unsigned صحیح)*(UINT16 *)(* p_argv);
		  بشکن;

		حالت FFI_TYPE_STRUCT:
	  	  z = (*p_arg)->size;
	  	  اگر ((*p_arg)->alignment != 1)
		    memcpy (argp, *p_argv, z);
		  وگرنه
		    memcpy (argp + 4 - z, *p_argv, z);
	  	  z = اندازه (صحیح);
		  بشکن;

		پیشفرض:
		  FFI_ASSERT(0);
		}
	    }
	  وگرنه اگر (z == اندازه (صحیح))
	    {
	       *(unsigned صحیح *) argp = (unsigned صحیح)*(UINT32 *)(* p_argv);
	    }
	  وگرنه
	    {
	      اگر ((*p_arg)->type == FFI_TYPE_STRUCT)
	        {
		  اگر (z > 8)
		    {
		      *(unsigned صحیح *) argp = (unsigned صحیح)(تهی *)(* p_argv);
		      z = اندازه(تهی *);
		    }
		  وگرنه
		    {
	              memcpy(argp, *p_argv, z);
		      z = 8;
		    }
	        }
	      وگرنه
	        {
		  /* Double or long long 64bit.  */
	          memcpy (argp, *p_argv, z);
	        }
	    }
	  p_argv++;
	  argp += z;
	}
    }

  بازگشت;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag.  */
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_VOID:
      cif->flags = (unsigned) cif->rtype->type;
      بشکن;

    حالت FFI_TYPE_STRUCT:
      اگر (cif->rtype->size <= 4)
	cif->flags = FFI_TYPE_INT;

      وگرنه اگر (cif->rtype->size <= 8)
	cif->flags = FFI_TYPE_DOUBLE;

      وگرنه
	cif->flags = (unsigned) cif->rtype->type;
      بشکن;

    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      بشکن;

    حالت FFI_TYPE_FLOAT:
    پیشفرض:
      cif->flags = FFI_TYPE_INT;
      بشکن;
    }

  بازگشت FFI_OK;
}

بیرونی تهی ffi_call_SYSV(تهی (*)(نویسه *, extended_cif *), extended_cif *,
			  unsigned, unsigned, unsigned *, تهی (*fn)(تهی));

تهی ffi_call(ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue, تهی **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  تهی **avalue_copy = پوچ;
  صحیح i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have
     a return value address then we need to make one.  */
  اگر ((rvalue == پوچ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  وگرنه
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  برای (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      صحیح size = at->size;
      اگر (at->type == FFI_TYPE_STRUCT && size > 4)
        {
          نویسه *argcopy = alloca (size);
          اگر (avalue_copy == پوچ)
            {
              avalue_copy = alloca (nargs * اندازه (تهی *));
              memcpy (avalue_copy, avalue, nargs * اندازه (تهی *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  انتخاب (cif->abi)
    {
    حالت FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes,
		    cif->flags, ecif.rvalue, fn);
      اگر (cif->rtype->type == FFI_TYPE_STRUCT)
	{
	  صحیح size = cif->rtype->size;
	  صحیح align = cif->rtype->alignment;

	  اگر (size < 4)
	    {
	      اگر (align == 1)
	        *(unsigned long *)(ecif.rvalue) <<= (4 - size) * 8;
	    }
	  وگرنه اگر (4 < size && size < 8)
	    {
	      اگر (align == 1)
		{
		  memcpy (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	      وگرنه اگر (align == 2)
		{
		  اگر (size & 1)
		    size += 1;

		  اگر (size != 8)
		    memcpy (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	    }
	}
      بشکن;

    پیشفرض:
      FFI_ASSERT(0);
      بشکن;
    }
}
