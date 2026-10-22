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
  unsigned ចំនួនគត់ i;
  ចំនួនគត់ tmp;
  unsigned ចំនួនគត់ avn;
  void **p_argv;
  var_na *argp;
  ffi_type **p_arg;

  tmp = 0;
  argp = stack;

  ប្រសិនបើ (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 8)
    {
      *(void **) argp = ecif->rvalue;
      argp += 4;
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  សម្រាប់ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0) && (avn != 0);
       i--, p_arg++)
    {
      size_t z;

      /* Align if necessary.  */
      ប្រសិនបើ (((*p_arg)->alignment - 1) & (unsigned) argp)
	argp = (var_na *) FFI_ALIGN (argp, (*p_arg)->alignment);

      ប្រសិនបើ (avn != 0)
	{
	  avn--;
	  z = (*p_arg)->size;
	  ប្រសិនបើ (z < parimaa_na (ចំនួនគត់))
	    {
	      z = parimaa_na (ចំនួនគត់);

	      switch ((*p_arg)->type)
		{
		pak_sa FFI_TYPE_SINT8:
		  *(signed ចំនួនគត់ *) argp = (signed ចំនួនគត់)*(SINT8 *)(* p_argv);
		  bha_nga;

		pak_sa FFI_TYPE_UINT8:
		  *(unsigned ចំនួនគត់ *) argp = (unsigned ចំនួនគត់)*(UINT8 *)(* p_argv);
		  bha_nga;

		pak_sa FFI_TYPE_SINT16:
		  *(signed ចំនួនគត់ *) argp = (signed ចំនួនគត់)*(SINT16 *)(* p_argv);
		  bha_nga;

		pak_sa FFI_TYPE_UINT16:
		  *(unsigned ចំនួនគត់ *) argp = (unsigned ចំនួនគត់)*(UINT16 *)(* p_argv);
		  bha_nga;

		pak_sa FFI_TYPE_STRUCT:
	  	  z = (*p_arg)->size;
	  	  ប្រសិនបើ ((*p_arg)->alignment != 1)
		    memcpy (argp, *p_argv, z);
		  បើមិនដូច្នេះ
		    memcpy (argp + 4 - z, *p_argv, z);
	  	  z = parimaa_na (ចំនួនគត់);
		  bha_nga;

		default:
		  FFI_ASSERT(0);
		}
	    }
	  បើមិនដូច្នេះ ប្រសិនបើ (z == parimaa_na (ចំនួនគត់))
	    {
	       *(unsigned ចំនួនគត់ *) argp = (unsigned ចំនួនគត់)*(UINT32 *)(* p_argv);
	    }
	  បើមិនដូច្នេះ
	    {
	      ប្រសិនបើ ((*p_arg)->type == FFI_TYPE_STRUCT)
	        {
		  ប្រសិនបើ (z > 8)
		    {
		      *(unsigned ចំនួនគត់ *) argp = (unsigned ចំនួនគត់)(void *)(* p_argv);
		      z = parimaa_na(void *);
		    }
		  បើមិនដូច្នេះ
		    {
	              memcpy(argp, *p_argv, z);
		      z = 8;
		    }
	        }
	      បើមិនដូច្នេះ
	        {
		  /* Double or long long 64bit.  */
	          memcpy (argp, *p_argv, z);
	        }
	    }
	  p_argv++;
	  argp += z;
	}
    }

  ត្រឡប់;
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
      ប្រសិនបើ (cif->rtype->size <= 4)
	cif->flags = FFI_TYPE_INT;

      បើមិនដូច្នេះ ប្រសិនបើ (cif->rtype->size <= 8)
	cif->flags = FFI_TYPE_DOUBLE;

      បើមិនដូច្នេះ
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

  ត្រឡប់ FFI_OK;
}

extern void ffi_call_SYSV(void (*)(var_na *, extended_cif *), extended_cif *,
			  unsigned, unsigned, unsigned *, void (*fn)(void));

void ffi_call(ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  void **avalue_copy = NULL;
  ចំនួនគត់ i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have
     a return value address then we need to make one.  */
  ប្រសិនបើ ((rvalue == NULL) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  បើមិនដូច្នេះ
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  សម្រាប់ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ចំនួនគត់ size = at->size;
      ប្រសិនបើ (at->type == FFI_TYPE_STRUCT && size > 4)
        {
          var_na *argcopy = alloca (size);
          ប្រសិនបើ (avalue_copy == NULL)
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
      ប្រសិនបើ (cif->rtype->type == FFI_TYPE_STRUCT)
	{
	  ចំនួនគត់ size = cif->rtype->size;
	  ចំនួនគត់ align = cif->rtype->alignment;

	  ប្រសិនបើ (size < 4)
	    {
	      ប្រសិនបើ (align == 1)
	        *(unsigned long *)(ecif.rvalue) <<= (4 - size) * 8;
	    }
	  បើមិនដូច្នេះ ប្រសិនបើ (4 < size && size < 8)
	    {
	      ប្រសិនបើ (align == 1)
		{
		  memcpy (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	      បើមិនដូច្នេះ ប្រសិនបើ (align == 2)
		{
		  ប្រសិនបើ (size & 1)
		    size += 1;

		  ប្រសិនបើ (size != 8)
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
