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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

#અન્તર્ભાવ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack
   space has been allocated for the function's arguments.  */

શૂન્ય ffi_prep_args(અક્ષર *ઢગલો, extended_cif *ecif)
{
  unsigned પૂર્ણાંક i;
  પૂર્ણાંક tmp;
  unsigned પૂર્ણાંક avn;
  શૂન્ય **p_argv;
  અક્ષર *argp;
  ffi_type **p_arg;

  tmp = 0;
  argp = ઢગલો;

  જો (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 8)
    {
      *(શૂન્ય **) argp = ecif->rvalue;
      argp += 4;
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  માટે (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0) && (avn != 0);
       i--, p_arg++)
    {
      size_t z;

      /* Align if necessary.  */
      જો (((*p_arg)->alignment - 1) & (unsigned) argp)
	argp = (અક્ષર *) FFI_ALIGN (argp, (*p_arg)->alignment);

      જો (avn != 0)
	{
	  avn--;
	  z = (*p_arg)->size;
	  જો (z < કદ (પૂર્ણાંક))
	    {
	      z = કદ (પૂર્ણાંક);

	      પસંદગી ((*p_arg)->type)
		{
		સ્થિતિ FFI_TYPE_SINT8:
		  *(signed પૂર્ણાંક *) argp = (signed પૂર્ણાંક)*(SINT8 *)(* p_argv);
		  તોડો;

		સ્થિતિ FFI_TYPE_UINT8:
		  *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)*(UINT8 *)(* p_argv);
		  તોડો;

		સ્થિતિ FFI_TYPE_SINT16:
		  *(signed પૂર્ણાંક *) argp = (signed પૂર્ણાંક)*(SINT16 *)(* p_argv);
		  તોડો;

		સ્થિતિ FFI_TYPE_UINT16:
		  *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)*(UINT16 *)(* p_argv);
		  તોડો;

		સ્થિતિ FFI_TYPE_STRUCT:
	  	  z = (*p_arg)->size;
	  	  જો ((*p_arg)->alignment != 1)
		    memcpy (argp, *p_argv, z);
		  નહીંતર
		    memcpy (argp + 4 - z, *p_argv, z);
	  	  z = કદ (પૂર્ણાંક);
		  તોડો;

		મૂળભૂત:
		  FFI_ASSERT(0);
		}
	    }
	  નહીંતર જો (z == કદ (પૂર્ણાંક))
	    {
	       *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)*(UINT32 *)(* p_argv);
	    }
	  નહીંતર
	    {
	      જો ((*p_arg)->type == FFI_TYPE_STRUCT)
	        {
		  જો (z > 8)
		    {
		      *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)(શૂન્ય *)(* p_argv);
		      z = કદ(શૂન્ય *);
		    }
		  નહીંતર
		    {
	              memcpy(argp, *p_argv, z);
		      z = 8;
		    }
	        }
	      નહીંતર
	        {
		  /* Double or long long 64bit.  */
	          memcpy (argp, *p_argv, z);
	        }
	    }
	  p_argv++;
	  argp += z;
	}
    }

  પાછા;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag.  */
  પસંદગી (cif->rtype->type)
    {
    સ્થિતિ FFI_TYPE_VOID:
      cif->flags = (unsigned) cif->rtype->type;
      તોડો;

    સ્થિતિ FFI_TYPE_STRUCT:
      જો (cif->rtype->size <= 4)
	cif->flags = FFI_TYPE_INT;

      નહીંતર જો (cif->rtype->size <= 8)
	cif->flags = FFI_TYPE_DOUBLE;

      નહીંતર
	cif->flags = (unsigned) cif->rtype->type;
      તોડો;

    સ્થિતિ FFI_TYPE_SINT64:
    સ્થિતિ FFI_TYPE_UINT64:
    સ્થિતિ FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      તોડો;

    સ્થિતિ FFI_TYPE_FLOAT:
    મૂળભૂત:
      cif->flags = FFI_TYPE_INT;
      તોડો;
    }

  પાછા FFI_OK;
}

બાહ્ય શૂન્ય ffi_call_SYSV(શૂન્ય (*)(અક્ષર *, extended_cif *), extended_cif *,
			  unsigned, unsigned, unsigned *, શૂન્ય (*fn)(શૂન્ય));

શૂન્ય ffi_call(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  શૂન્ય **avalue_copy = ખાલી;
  પૂર્ણાંક i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have
     a return value address then we need to make one.  */
  જો ((rvalue == ખાલી) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  નહીંતર
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  માટે (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      પૂર્ણાંક size = at->size;
      જો (at->type == FFI_TYPE_STRUCT && size > 4)
        {
          અક્ષર *argcopy = alloca (size);
          જો (avalue_copy == ખાલી)
            {
              avalue_copy = alloca (nargs * કદ (શૂન્ય *));
              memcpy (avalue_copy, avalue, nargs * કદ (શૂન્ય *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  પસંદગી (cif->abi)
    {
    સ્થિતિ FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes,
		    cif->flags, ecif.rvalue, fn);
      જો (cif->rtype->type == FFI_TYPE_STRUCT)
	{
	  પૂર્ણાંક size = cif->rtype->size;
	  પૂર્ણાંક align = cif->rtype->alignment;

	  જો (size < 4)
	    {
	      જો (align == 1)
	        *(unsigned long *)(ecif.rvalue) <<= (4 - size) * 8;
	    }
	  નહીંતર જો (4 < size && size < 8)
	    {
	      જો (align == 1)
		{
		  memcpy (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	      નહીંતર જો (align == 2)
		{
		  જો (size & 1)
		    size += 1;

		  જો (size != 8)
		    memcpy (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	    }
	}
      તોડો;

    મૂળભૂત:
      FFI_ASSERT(0);
      તોડો;
    }
}
