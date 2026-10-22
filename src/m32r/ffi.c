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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

#ଅନ୍ତର୍ଭାଵ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack
   space has been allocated for the function's arguments.  */

ଶୂନ୍ୟ ffi_prep_args(ଅକ୍ଷର *ଗଦା, extended_cif *ecif)
{
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmp;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା avn;
  ଶୂନ୍ୟ **p_argv;
  ଅକ୍ଷର *argp;
  ffi_type **p_arg;

  tmp = 0;
  argp = ଗଦା;

  ଯଦି (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 8)
    {
      *(ଶୂନ୍ୟ **) argp = ecif->rvalue;
      argp += 4;
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  ପାଇଁ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0) && (avn != 0);
       i--, p_arg++)
    {
      size_t z;

      /* Align if necessary.  */
      ଯଦି (((*p_arg)->alignment - 1) & (unsigned) argp)
	argp = (ଅକ୍ଷର *) FFI_ALIGN (argp, (*p_arg)->alignment);

      ଯଦି (avn != 0)
	{
	  avn--;
	  z = (*p_arg)->size;
	  ଯଦି (z < ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	    {
	      z = ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

	      ବାଛ ((*p_arg)->type)
		{
		ସ୍ଥିତି FFI_TYPE_SINT8:
		  *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(SINT8 *)(* p_argv);
		  ଭାଙ୍ଗ;

		ସ୍ଥିତି FFI_TYPE_UINT8:
		  *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT8 *)(* p_argv);
		  ଭାଙ୍ଗ;

		ସ୍ଥିତି FFI_TYPE_SINT16:
		  *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(SINT16 *)(* p_argv);
		  ଭାଙ୍ଗ;

		ସ୍ଥିତି FFI_TYPE_UINT16:
		  *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT16 *)(* p_argv);
		  ଭାଙ୍ଗ;

		ସ୍ଥିତି FFI_TYPE_STRUCT:
	  	  z = (*p_arg)->size;
	  	  ଯଦି ((*p_arg)->alignment != 1)
		    memcpy (argp, *p_argv, z);
		  ଅନ୍ୟଥା
		    memcpy (argp + 4 - z, *p_argv, z);
	  	  z = ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
		  ଭାଙ୍ଗ;

		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		  FFI_ASSERT(0);
		}
	    }
	  ଅନ୍ୟଥା ଯଦି (z == ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	    {
	       *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT32 *)(* p_argv);
	    }
	  ଅନ୍ୟଥା
	    {
	      ଯଦି ((*p_arg)->type == FFI_TYPE_STRUCT)
	        {
		  ଯଦି (z > 8)
		    {
		      *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(ଶୂନ୍ୟ *)(* p_argv);
		      z = ଆକାର(ଶୂନ୍ୟ *);
		    }
		  ଅନ୍ୟଥା
		    {
	              memcpy(argp, *p_argv, z);
		      z = 8;
		    }
	        }
	      ଅନ୍ୟଥା
	        {
		  /* Double or long long 64bit.  */
	          memcpy (argp, *p_argv, z);
	        }
	    }
	  p_argv++;
	  argp += z;
	}
    }

  ଫେରନ୍ତୁ;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag.  */
  ବାଛ (cif->rtype->type)
    {
    ସ୍ଥିତି FFI_TYPE_VOID:
      cif->flags = (unsigned) cif->rtype->type;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_STRUCT:
      ଯଦି (cif->rtype->size <= 4)
	cif->flags = FFI_TYPE_INT;

      ଅନ୍ୟଥା ଯଦି (cif->rtype->size <= 8)
	cif->flags = FFI_TYPE_DOUBLE;

      ଅନ୍ୟଥା
	cif->flags = (unsigned) cif->rtype->type;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_SINT64:
    ସ୍ଥିତି FFI_TYPE_UINT64:
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_FLOAT:
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      cif->flags = FFI_TYPE_INT;
      ଭାଙ୍ଗ;
    }

  ଫେରନ୍ତୁ FFI_OK;
}

ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_SYSV(ଶୂନ୍ୟ (*)(ଅକ୍ଷର *, extended_cif *), extended_cif *,
			  unsigned, unsigned, unsigned *, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ));

ଶୂନ୍ୟ ffi_call(ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  ଶୂନ୍ୟ **avalue_copy = ଖାଲି;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have
     a return value address then we need to make one.  */
  ଯଦି ((rvalue == ଖାଲି) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  ଅନ୍ୟଥା
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ପାଇଁ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = at->size;
      ଯଦି (at->type == FFI_TYPE_STRUCT && size > 4)
        {
          ଅକ୍ଷର *argcopy = alloca (size);
          ଯଦି (avalue_copy == ଖାଲି)
            {
              avalue_copy = alloca (nargs * ଆକାର (ଶୂନ୍ୟ *));
              memcpy (avalue_copy, avalue, nargs * ଆକାର (ଶୂନ୍ୟ *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  ବାଛ (cif->abi)
    {
    ସ୍ଥିତି FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes,
		    cif->flags, ecif.rvalue, fn);
      ଯଦି (cif->rtype->type == FFI_TYPE_STRUCT)
	{
	  ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = cif->rtype->size;
	  ପୂର୍ଣ୍ଣସଂଖ୍ୟା align = cif->rtype->alignment;

	  ଯଦି (size < 4)
	    {
	      ଯଦି (align == 1)
	        *(unsigned long *)(ecif.rvalue) <<= (4 - size) * 8;
	    }
	  ଅନ୍ୟଥା ଯଦି (4 < size && size < 8)
	    {
	      ଯଦି (align == 1)
		{
		  memcpy (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	      ଅନ୍ୟଥା ଯଦି (align == 2)
		{
		  ଯଦି (size & 1)
		    size += 1;

		  ଯଦି (size != 8)
		    memcpy (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	    }
	}
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      FFI_ASSERT(0);
      ଭାଙ୍ଗ;
    }
}
