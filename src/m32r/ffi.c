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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

#அந்தர்பாவ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack
   space has been allocated for the function's arguments.  */

வெறுமை ffi_prep_args(எழுத்து *அடுக்கு, extended_cif *ecif)
{
  unsigned முழு i;
  முழு tmp;
  unsigned முழு avn;
  வெறுமை **p_argv;
  எழுத்து *argp;
  ffi_type **p_arg;

  tmp = 0;
  argp = அடுக்கு;

  எனில் (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 8)
    {
      *(வெறுமை **) argp = ecif->rvalue;
      argp += 4;
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  க்கு (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0) && (avn != 0);
       i--, p_arg++)
    {
      size_t z;

      /* Align if necessary.  */
      எனில் (((*p_arg)->alignment - 1) & (unsigned) argp)
	argp = (எழுத்து *) FFI_ALIGN (argp, (*p_arg)->alignment);

      எனில் (avn != 0)
	{
	  avn--;
	  z = (*p_arg)->size;
	  எனில் (z < அளவு (முழு))
	    {
	      z = அளவு (முழு);

	      தேர்வு ((*p_arg)->type)
		{
		நிலை FFI_TYPE_SINT8:
		  *(signed முழு *) argp = (signed முழு)*(SINT8 *)(* p_argv);
		  நிறுத்து;

		நிலை FFI_TYPE_UINT8:
		  *(unsigned முழு *) argp = (unsigned முழு)*(UINT8 *)(* p_argv);
		  நிறுத்து;

		நிலை FFI_TYPE_SINT16:
		  *(signed முழு *) argp = (signed முழு)*(SINT16 *)(* p_argv);
		  நிறுத்து;

		நிலை FFI_TYPE_UINT16:
		  *(unsigned முழு *) argp = (unsigned முழு)*(UINT16 *)(* p_argv);
		  நிறுத்து;

		நிலை FFI_TYPE_STRUCT:
	  	  z = (*p_arg)->size;
	  	  எனில் ((*p_arg)->alignment != 1)
		    memcpy (argp, *p_argv, z);
		  இல்லையெனில்
		    memcpy (argp + 4 - z, *p_argv, z);
	  	  z = அளவு (முழு);
		  நிறுத்து;

		இயல்பு:
		  FFI_ASSERT(0);
		}
	    }
	  இல்லையெனில் எனில் (z == அளவு (முழு))
	    {
	       *(unsigned முழு *) argp = (unsigned முழு)*(UINT32 *)(* p_argv);
	    }
	  இல்லையெனில்
	    {
	      எனில் ((*p_arg)->type == FFI_TYPE_STRUCT)
	        {
		  எனில் (z > 8)
		    {
		      *(unsigned முழு *) argp = (unsigned முழு)(வெறுமை *)(* p_argv);
		      z = அளவு(வெறுமை *);
		    }
		  இல்லையெனில்
		    {
	              memcpy(argp, *p_argv, z);
		      z = 8;
		    }
	        }
	      இல்லையெனில்
	        {
		  /* Double or long long 64bit.  */
	          memcpy (argp, *p_argv, z);
	        }
	    }
	  p_argv++;
	  argp += z;
	}
    }

  திரும்பு;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag.  */
  தேர்வு (cif->rtype->type)
    {
    நிலை FFI_TYPE_VOID:
      cif->flags = (unsigned) cif->rtype->type;
      நிறுத்து;

    நிலை FFI_TYPE_STRUCT:
      எனில் (cif->rtype->size <= 4)
	cif->flags = FFI_TYPE_INT;

      இல்லையெனில் எனில் (cif->rtype->size <= 8)
	cif->flags = FFI_TYPE_DOUBLE;

      இல்லையெனில்
	cif->flags = (unsigned) cif->rtype->type;
      நிறுத்து;

    நிலை FFI_TYPE_SINT64:
    நிலை FFI_TYPE_UINT64:
    நிலை FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      நிறுத்து;

    நிலை FFI_TYPE_FLOAT:
    இயல்பு:
      cif->flags = FFI_TYPE_INT;
      நிறுத்து;
    }

  திரும்பு FFI_OK;
}

வெளி வெறுமை ffi_call_SYSV(வெறுமை (*)(எழுத்து *, extended_cif *), extended_cif *,
			  unsigned, unsigned, unsigned *, வெறுமை (*fn)(வெறுமை));

வெறுமை ffi_call(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  வெறுமை **avalue_copy = வெற்று;
  முழு i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have
     a return value address then we need to make one.  */
  எனில் ((rvalue == வெற்று) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  இல்லையெனில்
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  க்கு (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      முழு size = at->size;
      எனில் (at->type == FFI_TYPE_STRUCT && size > 4)
        {
          எழுத்து *argcopy = alloca (size);
          எனில் (avalue_copy == வெற்று)
            {
              avalue_copy = alloca (nargs * அளவு (வெறுமை *));
              memcpy (avalue_copy, avalue, nargs * அளவு (வெறுமை *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  தேர்வு (cif->abi)
    {
    நிலை FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes,
		    cif->flags, ecif.rvalue, fn);
      எனில் (cif->rtype->type == FFI_TYPE_STRUCT)
	{
	  முழு size = cif->rtype->size;
	  முழு align = cif->rtype->alignment;

	  எனில் (size < 4)
	    {
	      எனில் (align == 1)
	        *(unsigned long *)(ecif.rvalue) <<= (4 - size) * 8;
	    }
	  இல்லையெனில் எனில் (4 < size && size < 8)
	    {
	      எனில் (align == 1)
		{
		  memcpy (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	      இல்லையெனில் எனில் (align == 2)
		{
		  எனில் (size & 1)
		    size += 1;

		  எனில் (size != 8)
		    memcpy (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	    }
	}
      நிறுத்து;

    இயல்பு:
      FFI_ASSERT(0);
      நிறுத்து;
    }
}
