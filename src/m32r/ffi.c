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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack
   space has been allocated for the function's arguments.  */

ಶೂನ್ಯ ffi_prep_args(ವರ್ಣ *ರಾಶಿ, extended_cif *ecif)
{
  unsigned ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ tmp;
  unsigned ಪೂರ್ಣಾಙ್ಕ avn;
  ಶೂನ್ಯ **p_argv;
  ವರ್ಣ *argp;
  ffi_type **p_arg;

  tmp = 0;
  argp = ರಾಶಿ;

  ಯದಿ (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 8)
    {
      *(ಶೂನ್ಯ **) argp = ecif->rvalue;
      argp += 4;
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  ಪ್ರತಿ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0) && (avn != 0);
       i--, p_arg++)
    {
      size_t z;

      /* Align if necessary.  */
      ಯದಿ (((*p_arg)->alignment - 1) & (unsigned) argp)
	argp = (ವರ್ಣ *) FFI_ALIGN (argp, (*p_arg)->alignment);

      ಯದಿ (avn != 0)
	{
	  avn--;
	  z = (*p_arg)->size;
	  ಯದಿ (z < ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ))
	    {
	      z = ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ);

	      ವಿಕಲ್ಪ ((*p_arg)->type)
		{
		ಪಕ್ಷ FFI_TYPE_SINT8:
		  *(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ)*(SINT8 *)(* p_argv);
		  ಭಙ್ಗ;

		ಪಕ್ಷ FFI_TYPE_UINT8:
		  *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT8 *)(* p_argv);
		  ಭಙ್ಗ;

		ಪಕ್ಷ FFI_TYPE_SINT16:
		  *(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ)*(SINT16 *)(* p_argv);
		  ಭಙ್ಗ;

		ಪಕ್ಷ FFI_TYPE_UINT16:
		  *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT16 *)(* p_argv);
		  ಭಙ್ಗ;

		ಪಕ್ಷ FFI_TYPE_STRUCT:
	  	  z = (*p_arg)->size;
	  	  ಯದಿ ((*p_arg)->alignment != 1)
		    memcpy (argp, *p_argv, z);
		  ಅನ್ಯಥಾ
		    memcpy (argp + 4 - z, *p_argv, z);
	  	  z = ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ);
		  ಭಙ್ಗ;

		ಸ್ವಭಾವ:
		  FFI_ASSERT(0);
		}
	    }
	  ಅನ್ಯಥಾ ಯದಿ (z == ಪರಿಮಾಣ (ಪೂರ್ಣಾಙ್ಕ))
	    {
	       *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT32 *)(* p_argv);
	    }
	  ಅನ್ಯಥಾ
	    {
	      ಯದಿ ((*p_arg)->type == FFI_TYPE_STRUCT)
	        {
		  ಯದಿ (z > 8)
		    {
		      *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)(ಶೂನ್ಯ *)(* p_argv);
		      z = ಪರಿಮಾಣ(ಶೂನ್ಯ *);
		    }
		  ಅನ್ಯಥಾ
		    {
	              memcpy(argp, *p_argv, z);
		      z = 8;
		    }
	        }
	      ಅನ್ಯಥಾ
	        {
		  /* Double or long long 64bit.  */
	          memcpy (argp, *p_argv, z);
	        }
	    }
	  p_argv++;
	  argp += z;
	}
    }

  ಪ್ರತ್ಯಾಗಮ;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag.  */
  ವಿಕಲ್ಪ (cif->rtype->type)
    {
    ಪಕ್ಷ FFI_TYPE_VOID:
      cif->flags = (unsigned) cif->rtype->type;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_STRUCT:
      ಯದಿ (cif->rtype->size <= 4)
	cif->flags = FFI_TYPE_INT;

      ಅನ್ಯಥಾ ಯದಿ (cif->rtype->size <= 8)
	cif->flags = FFI_TYPE_DOUBLE;

      ಅನ್ಯಥಾ
	cif->flags = (unsigned) cif->rtype->type;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_SINT64:
    ಪಕ್ಷ FFI_TYPE_UINT64:
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_FLOAT:
    ಸ್ವಭಾವ:
      cif->flags = FFI_TYPE_INT;
      ಭಙ್ಗ;
    }

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV(ಶೂನ್ಯ (*)(ವರ್ಣ *, extended_cif *), extended_cif *,
			  unsigned, unsigned, unsigned *, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ));

ಶೂನ್ಯ ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  ಶೂನ್ಯ **avalue_copy = ರಿಕ್ತ;
  ಪೂರ್ಣಾಙ್ಕ i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have
     a return value address then we need to make one.  */
  ಯದಿ ((rvalue == ರಿಕ್ತ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  ಅನ್ಯಥಾ
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ಪ್ರತಿ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ಪೂರ್ಣಾಙ್ಕ size = at->size;
      ಯದಿ (at->type == FFI_TYPE_STRUCT && size > 4)
        {
          ವರ್ಣ *argcopy = alloca (size);
          ಯದಿ (avalue_copy == ರಿಕ್ತ)
            {
              avalue_copy = alloca (nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ *));
              memcpy (avalue_copy, avalue, nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  ವಿಕಲ್ಪ (cif->abi)
    {
    ಪಕ್ಷ FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes,
		    cif->flags, ecif.rvalue, fn);
      ಯದಿ (cif->rtype->type == FFI_TYPE_STRUCT)
	{
	  ಪೂರ್ಣಾಙ್ಕ size = cif->rtype->size;
	  ಪೂರ್ಣಾಙ್ಕ align = cif->rtype->alignment;

	  ಯದಿ (size < 4)
	    {
	      ಯದಿ (align == 1)
	        *(unsigned long *)(ecif.rvalue) <<= (4 - size) * 8;
	    }
	  ಅನ್ಯಥಾ ಯದಿ (4 < size && size < 8)
	    {
	      ಯದಿ (align == 1)
		{
		  memcpy (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	      ಅನ್ಯಥಾ ಯದಿ (align == 2)
		{
		  ಯದಿ (size & 1)
		    size += 1;

		  ಯದಿ (size != 8)
		    memcpy (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	    }
	}
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      FFI_ASSERT(0);
      ಭಙ್ಗ;
    }
}
