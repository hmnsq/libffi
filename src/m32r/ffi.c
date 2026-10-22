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

ಶೂನ್ಯ ffi_prep_ವಾದಗಳು(ಅಕ್ಷರ *ರಾಶಿ, extended_cif *ecif)
{
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ tmp;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ avn;
  ಶೂನ್ಯ **p_argv;
  ಅಕ್ಷರ *argp;
  ffi_type **p_arg;

  tmp = 0;
  argp = ರಾಶಿ;

  ಆದರೆ (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->ಗಾತ್ರ_2 > 8)
    {
      *(ಶೂನ್ಯ **) argp = ecif->rvalue;
      argp += 4;
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  ಗಾಗಿ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0) && (avn != 0);
       i--, p_arg++)
    {
      size_t z;

      /* Align if necessary.  */
      ಆದರೆ (((*p_arg)->alignment - 1) & (ಚಿಹ್ನೆರಹಿತ) argp)
	argp = (ಅಕ್ಷರ *) FFI_ALIGN (argp, (*p_arg)->alignment);

      ಆದರೆ (avn != 0)
	{
	  avn--;
	  z = (*p_arg)->ಗಾತ್ರ_2;
	  ಆದರೆ (z < ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ))
	    {
	      z = ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ);

	      ಆಯ್ಕೆ ((*p_arg)->type)
		{
		ಸ್ಥಿತಿ FFI_TYPE_SINT8:
		  *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ)*(SINT8 *)(* p_argv);
		  ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ FFI_TYPE_UINT8:
		  *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)*(UINT8 *)(* p_argv);
		  ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ FFI_TYPE_SINT16:
		  *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ)*(SINT16 *)(* p_argv);
		  ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ FFI_TYPE_UINT16:
		  *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)*(UINT16 *)(* p_argv);
		  ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  	  z = (*p_arg)->ಗಾತ್ರ_2;
	  	  ಆದರೆ ((*p_arg)->alignment != 1)
		    ಸ್ಮರಣೆ_ನಕಲು (argp, *p_argv, z);
		  ಇಲ್ಲದಿದ್ದರೆ
		    ಸ್ಮರಣೆ_ನಕಲು (argp + 4 - z, *p_argv, z);
	  	  z = ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ);
		  ನಿಲ್ಲಿಸು;

		ಪೂರ್ವನಿಯೋಜಿತ:
		  FFI_ASSERT(0);
		}
	    }
	  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (z == ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ))
	    {
	       *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)*(UINT32 *)(* p_argv);
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      ಆದರೆ ((*p_arg)->type == FFI_TYPE_STRUCT)
	        {
		  ಆದರೆ (z > 8)
		    {
		      *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) argp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)(ಶೂನ್ಯ *)(* p_argv);
		      z = ಗಾತ್ರ(ಶೂನ್ಯ *);
		    }
		  ಇಲ್ಲದಿದ್ದರೆ
		    {
	              ಸ್ಮರಣೆ_ನಕಲು(argp, *p_argv, z);
		      z = 8;
		    }
	        }
	      ಇಲ್ಲದಿದ್ದರೆ
	        {
		  /* Double or long long 64bit.  */
	          ಸ್ಮರಣೆ_ನಕಲು (argp, *p_argv, z);
	        }
	    }
	  p_argv++;
	  argp += z;
	}
    }

  ಹಿಂತಿರುಗಿ;
}

/* Perform machine dependent cif processing.  */
ffi_ಸ್ಥಿತಿ
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag.  */
  ಆಯ್ಕೆ (cif->rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_VOID:
      cif->flags = (ಚಿಹ್ನೆರಹಿತ) cif->rtype->type;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      ಆದರೆ (cif->rtype->ಗಾತ್ರ_2 <= 4)
	cif->flags = FFI_TYPE_INT;

      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cif->rtype->ಗಾತ್ರ_2 <= 8)
	cif->flags = FFI_TYPE_DOUBLE;

      ಇಲ್ಲದಿದ್ದರೆ
	cif->flags = (ಚಿಹ್ನೆರಹಿತ) cif->rtype->type;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
    ಪೂರ್ವನಿಯೋಜಿತ:
      cif->flags = FFI_TYPE_INT;
      ನಿಲ್ಲಿಸು;
    }

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV(ಶೂನ್ಯ (*)(ಅಕ್ಷರ *, extended_cif *), extended_cif *,
			  ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ *, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ));

ಶೂನ್ಯ ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  ಶೂನ್ಯ **avalue_copy = ಖಾಲಿ;
  ಪೂರ್ಣಾಂಕ i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have
     a return value address then we need to make one.  */
  ಆದರೆ ((rvalue == ಖಾಲಿ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->ಗಾತ್ರ_2);
    }
  ಇಲ್ಲದಿದ್ದರೆ
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ಗಾಗಿ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = at->ಗಾತ್ರ_2;
      ಆದರೆ (at->type == FFI_TYPE_STRUCT && ಗಾತ್ರ_2 > 4)
        {
          ಅಕ್ಷರ *argcopy = alloca (ಗಾತ್ರ_2);
          ಆದರೆ (avalue_copy == ಖಾಲಿ)
            {
              avalue_copy = alloca (nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));
              ಸ್ಮರಣೆ_ನಕಲು (avalue_copy, avalue, nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));
              avalue = avalue_copy;
            }
          ಸ್ಮರಣೆ_ನಕಲು (argcopy, avalue[i], ಗಾತ್ರ_2);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  ಆಯ್ಕೆ (cif->abi)
    {
    ಸ್ಥಿತಿ FFI_SYSV:
      ffi_call_SYSV(ffi_prep_ವಾದಗಳು, &ecif, cif->ಬೈಟುಗಳು,
		    cif->flags, ecif.rvalue, fn);
      ಆದರೆ (cif->rtype->type == FFI_TYPE_STRUCT)
	{
	  ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = cif->rtype->ಗಾತ್ರ_2;
	  ಪೂರ್ಣಾಂಕ align = cif->rtype->alignment;

	  ಆದರೆ (ಗಾತ್ರ_2 < 4)
	    {
	      ಆದರೆ (align == 1)
	        *(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *)(ecif.rvalue) <<= (4 - ಗಾತ್ರ_2) * 8;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (4 < ಗಾತ್ರ_2 && ಗಾತ್ರ_2 < 8)
	    {
	      ಆದರೆ (align == 1)
		{
		  ಸ್ಮರಣೆ_ನಕಲು (ecif.rvalue, ecif.rvalue + 8-ಗಾತ್ರ_2, ಗಾತ್ರ_2);
		}
	      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (align == 2)
		{
		  ಆದರೆ (ಗಾತ್ರ_2 & 1)
		    ಗಾತ್ರ_2 += 1;

		  ಆದರೆ (ಗಾತ್ರ_2 != 8)
		    ಸ್ಮರಣೆ_ನಕಲು (ecif.rvalue, ecif.rvalue + 8-ಗಾತ್ರ_2, ಗಾತ್ರ_2);
		}
	    }
	}
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      FFI_ASSERT(0);
      ನಿಲ್ಲಿಸು;
    }
}
