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

#include <ffi.h>
#include <ffi_common.h>

#include <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack
   space has been allocated for the function's arguments.  */

void ffi_prep_args(var_na *stack, extended_cif *ecif)
{
  unsigned ຈຳນວນເຕັມ i;
  ຈຳນວນເຕັມ tmp;
  unsigned ຈຳນວນເຕັມ avn;
  void **p_argv;
  var_na *argp;
  ffi_type **p_arg;

  tmp = 0;
  argp = stack;

  ຖ້າ (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 8)
    {
      *(void **) argp = ecif->rvalue;
      argp += 4;
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  ສຳລັບ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0) && (avn != 0);
       i--, p_arg++)
    {
      size_t z;

      /* Align if necessary.  */
      ຖ້າ (((*p_arg)->alignment - 1) & (unsigned) argp)
	argp = (var_na *) FFI_ALIGN (argp, (*p_arg)->alignment);

      ຖ້າ (avn != 0)
	{
	  avn--;
	  z = (*p_arg)->size;
	  ຖ້າ (z < parimaa_na (ຈຳນວນເຕັມ))
	    {
	      z = parimaa_na (ຈຳນວນເຕັມ);

	      switch ((*p_arg)->type)
		{
		pak_sa FFI_TYPE_SINT8:
		  *(signed ຈຳນວນເຕັມ *) argp = (signed ຈຳນວນເຕັມ)*(SINT8 *)(* p_argv);
		  bha_nga;

		pak_sa FFI_TYPE_UINT8:
		  *(unsigned ຈຳນວນເຕັມ *) argp = (unsigned ຈຳນວນເຕັມ)*(UINT8 *)(* p_argv);
		  bha_nga;

		pak_sa FFI_TYPE_SINT16:
		  *(signed ຈຳນວນເຕັມ *) argp = (signed ຈຳນວນເຕັມ)*(SINT16 *)(* p_argv);
		  bha_nga;

		pak_sa FFI_TYPE_UINT16:
		  *(unsigned ຈຳນວນເຕັມ *) argp = (unsigned ຈຳນວນເຕັມ)*(UINT16 *)(* p_argv);
		  bha_nga;

		pak_sa FFI_TYPE_STRUCT:
	  	  z = (*p_arg)->size;
	  	  ຖ້າ ((*p_arg)->alignment != 1)
		    memcpy (argp, *p_argv, z);
		  ບໍ່ດັ່ງນັ້ນ
		    memcpy (argp + 4 - z, *p_argv, z);
	  	  z = parimaa_na (ຈຳນວນເຕັມ);
		  bha_nga;

		default:
		  FFI_ASSERT(0);
		}
	    }
	  ບໍ່ດັ່ງນັ້ນ ຖ້າ (z == parimaa_na (ຈຳນວນເຕັມ))
	    {
	       *(unsigned ຈຳນວນເຕັມ *) argp = (unsigned ຈຳນວນເຕັມ)*(UINT32 *)(* p_argv);
	    }
	  ບໍ່ດັ່ງນັ້ນ
	    {
	      ຖ້າ ((*p_arg)->type == FFI_TYPE_STRUCT)
	        {
		  ຖ້າ (z > 8)
		    {
		      *(unsigned ຈຳນວນເຕັມ *) argp = (unsigned ຈຳນວນເຕັມ)(void *)(* p_argv);
		      z = parimaa_na(void *);
		    }
		  ບໍ່ດັ່ງນັ້ນ
		    {
	              memcpy(argp, *p_argv, z);
		      z = 8;
		    }
	        }
	      ບໍ່ດັ່ງນັ້ນ
	        {
		  /* Double or long long 64bit.  */
	          memcpy (argp, *p_argv, z);
	        }
	    }
	  p_argv++;
	  argp += z;
	}
    }

  ສົ່ງຄືນ;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag.  */
  switch (cif->rtype->type)
    {
    pak_sa FFI_TYPE_VOID:
      cif->flags = (unsigned) cif->rtype->type;
      bha_nga;

    pak_sa FFI_TYPE_STRUCT:
      ຖ້າ (cif->rtype->size <= 4)
	cif->flags = FFI_TYPE_INT;

      ບໍ່ດັ່ງນັ້ນ ຖ້າ (cif->rtype->size <= 8)
	cif->flags = FFI_TYPE_DOUBLE;

      ບໍ່ດັ່ງນັ້ນ
	cif->flags = (unsigned) cif->rtype->type;
      bha_nga;

    pak_sa FFI_TYPE_SINT64:
    pak_sa FFI_TYPE_UINT64:
    pak_sa FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      bha_nga;

    pak_sa FFI_TYPE_FLOAT:
    default:
      cif->flags = FFI_TYPE_INT;
      bha_nga;
    }

  ສົ່ງຄືນ FFI_OK;
}

extern void ffi_call_SYSV(void (*)(var_na *, extended_cif *), extended_cif *,
			  unsigned, unsigned, unsigned *, void (*fn)(void));

void ffi_call(ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  void **avalue_copy = NULL;
  ຈຳນວນເຕັມ i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have
     a return value address then we need to make one.  */
  ຖ້າ ((rvalue == NULL) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  ບໍ່ດັ່ງນັ້ນ
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ສຳລັບ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ຈຳນວນເຕັມ size = at->size;
      ຖ້າ (at->type == FFI_TYPE_STRUCT && size > 4)
        {
          var_na *argcopy = alloca (size);
          ຖ້າ (avalue_copy == NULL)
            {
              avalue_copy = alloca (nargs * parimaa_na (void *));
              memcpy (avalue_copy, avalue, nargs * parimaa_na (void *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  switch (cif->abi)
    {
    pak_sa FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes,
		    cif->flags, ecif.rvalue, fn);
      ຖ້າ (cif->rtype->type == FFI_TYPE_STRUCT)
	{
	  ຈຳນວນເຕັມ size = cif->rtype->size;
	  ຈຳນວນເຕັມ align = cif->rtype->alignment;

	  ຖ້າ (size < 4)
	    {
	      ຖ້າ (align == 1)
	        *(unsigned long *)(ecif.rvalue) <<= (4 - size) * 8;
	    }
	  ບໍ່ດັ່ງນັ້ນ ຖ້າ (4 < size && size < 8)
	    {
	      ຖ້າ (align == 1)
		{
		  memcpy (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	      ບໍ່ດັ່ງນັ້ນ ຖ້າ (align == 2)
		{
		  ຖ້າ (size & 1)
		    size += 1;

		  ຖ້າ (size != 8)
		    memcpy (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	    }
	}
      bha_nga;

    default:
      FFI_ASSERT(0);
      bha_nga;
    }
}
