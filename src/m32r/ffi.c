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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>

#അന്തര്ഭാവ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack
   space has been allocated for the function's arguments.  */

ശൂന്യം ffi_prep_args(അക്ഷരം *അട്ടി, extended_cif *ecif)
{
  unsigned പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ tmp;
  unsigned പൂർണ്ണസംഖ്യ avn;
  ശൂന്യം **p_argv;
  അക്ഷരം *argp;
  ffi_type **p_arg;

  tmp = 0;
  argp = അട്ടി;

  എങ്കിൽ (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 8)
    {
      *(ശൂന്യം **) argp = ecif->rvalue;
      argp += 4;
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  വേണ്ടി (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0) && (avn != 0);
       i--, p_arg++)
    {
      size_t z;

      /* Align if necessary.  */
      എങ്കിൽ (((*p_arg)->alignment - 1) & (unsigned) argp)
	argp = (അക്ഷരം *) FFI_ALIGN (argp, (*p_arg)->alignment);

      എങ്കിൽ (avn != 0)
	{
	  avn--;
	  z = (*p_arg)->size;
	  എങ്കിൽ (z < വലുപ്പം (പൂർണ്ണസംഖ്യ))
	    {
	      z = വലുപ്പം (പൂർണ്ണസംഖ്യ);

	      തിരഞ്ഞെടുപ്പ് ((*p_arg)->type)
		{
		അവസ്ഥ FFI_TYPE_SINT8:
		  *(signed പൂർണ്ണസംഖ്യ *) argp = (signed പൂർണ്ണസംഖ്യ)*(SINT8 *)(* p_argv);
		  നിർത്തുക;

		അവസ്ഥ FFI_TYPE_UINT8:
		  *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ)*(UINT8 *)(* p_argv);
		  നിർത്തുക;

		അവസ്ഥ FFI_TYPE_SINT16:
		  *(signed പൂർണ്ണസംഖ്യ *) argp = (signed പൂർണ്ണസംഖ്യ)*(SINT16 *)(* p_argv);
		  നിർത്തുക;

		അവസ്ഥ FFI_TYPE_UINT16:
		  *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ)*(UINT16 *)(* p_argv);
		  നിർത്തുക;

		അവസ്ഥ FFI_TYPE_STRUCT:
	  	  z = (*p_arg)->size;
	  	  എങ്കിൽ ((*p_arg)->alignment != 1)
		    memcpy (argp, *p_argv, z);
		  അല്ലെങ്കിൽ
		    memcpy (argp + 4 - z, *p_argv, z);
	  	  z = വലുപ്പം (പൂർണ്ണസംഖ്യ);
		  നിർത്തുക;

		സ്ഥിരസ്ഥിതി:
		  FFI_ASSERT(0);
		}
	    }
	  അല്ലെങ്കിൽ എങ്കിൽ (z == വലുപ്പം (പൂർണ്ണസംഖ്യ))
	    {
	       *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ)*(UINT32 *)(* p_argv);
	    }
	  അല്ലെങ്കിൽ
	    {
	      എങ്കിൽ ((*p_arg)->type == FFI_TYPE_STRUCT)
	        {
		  എങ്കിൽ (z > 8)
		    {
		      *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ)(ശൂന്യം *)(* p_argv);
		      z = വലുപ്പം(ശൂന്യം *);
		    }
		  അല്ലെങ്കിൽ
		    {
	              memcpy(argp, *p_argv, z);
		      z = 8;
		    }
	        }
	      അല്ലെങ്കിൽ
	        {
		  /* Double or long long 64bit.  */
	          memcpy (argp, *p_argv, z);
	        }
	    }
	  p_argv++;
	  argp += z;
	}
    }

  തിരികെ;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag.  */
  തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
    {
    അവസ്ഥ FFI_TYPE_VOID:
      cif->flags = (unsigned) cif->rtype->type;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_STRUCT:
      എങ്കിൽ (cif->rtype->size <= 4)
	cif->flags = FFI_TYPE_INT;

      അല്ലെങ്കിൽ എങ്കിൽ (cif->rtype->size <= 8)
	cif->flags = FFI_TYPE_DOUBLE;

      അല്ലെങ്കിൽ
	cif->flags = (unsigned) cif->rtype->type;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_SINT64:
    അവസ്ഥ FFI_TYPE_UINT64:
    അവസ്ഥ FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_FLOAT:
    സ്ഥിരസ്ഥിതി:
      cif->flags = FFI_TYPE_INT;
      നിർത്തുക;
    }

  തിരികെ FFI_OK;
}

ബാഹ്യ ശൂന്യം ffi_call_SYSV(ശൂന്യം (*)(അക്ഷരം *, extended_cif *), extended_cif *,
			  unsigned, unsigned, unsigned *, ശൂന്യം (*fn)(ശൂന്യം));

ശൂന്യം ffi_call(ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  ശൂന്യം **avalue_copy = ശൂന്യ;
  പൂർണ്ണസംഖ്യ i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have
     a return value address then we need to make one.  */
  എങ്കിൽ ((rvalue == ശൂന്യ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  അല്ലെങ്കിൽ
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  വേണ്ടി (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      പൂർണ്ണസംഖ്യ size = at->size;
      എങ്കിൽ (at->type == FFI_TYPE_STRUCT && size > 4)
        {
          അക്ഷരം *argcopy = alloca (size);
          എങ്കിൽ (avalue_copy == ശൂന്യ)
            {
              avalue_copy = alloca (nargs * വലുപ്പം (ശൂന്യം *));
              memcpy (avalue_copy, avalue, nargs * വലുപ്പം (ശൂന്യം *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
    അവസ്ഥ FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes,
		    cif->flags, ecif.rvalue, fn);
      എങ്കിൽ (cif->rtype->type == FFI_TYPE_STRUCT)
	{
	  പൂർണ്ണസംഖ്യ size = cif->rtype->size;
	  പൂർണ്ണസംഖ്യ align = cif->rtype->alignment;

	  എങ്കിൽ (size < 4)
	    {
	      എങ്കിൽ (align == 1)
	        *(unsigned long *)(ecif.rvalue) <<= (4 - size) * 8;
	    }
	  അല്ലെങ്കിൽ എങ്കിൽ (4 < size && size < 8)
	    {
	      എങ്കിൽ (align == 1)
		{
		  memcpy (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	      അല്ലെങ്കിൽ എങ്കിൽ (align == 2)
		{
		  എങ്കിൽ (size & 1)
		    size += 1;

		  എങ്കിൽ (size != 8)
		    memcpy (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	    }
	}
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      FFI_ASSERT(0);
      നിർത്തുക;
    }
}
