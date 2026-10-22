/*
 * Copyright (c) 2013 Miodrag Vallat.  <miod@openbsd.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * ``Software''), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * vax Foreign Function Interface
 *
 * This file attempts to provide all the FFI entry points which can reliably
 * be implemented in C.
 */

#include <ffi.h>
#include <ffi_common.h>

#include <stdlib.h>
#include <unistd.h>

#define CIF_FLAGS_CHAR		1	/* for struct only */
#define CIF_FLAGS_SHORT		2	/* for struct only */
#define CIF_FLAGS_INT		4
#define CIF_FLAGS_DINT		8

/*
 * Foreign Function Interface API
 */

void ffi_call_elfbsd (extended_cif *, unsigned, unsigned, void *,
		       void (*) ());
void *ffi_prep_args (extended_cif *ecif, void *stack);

void *
ffi_prep_args (extended_cif *ecif, void *stack)
{
  unsigned ຈຳນວນເຕັມ i;
  void **p_argv;
  var_na *argp;
  ffi_type **p_arg;
  void *struct_value_ptr;

  argp = stack;

  ຖ້າ (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_value_ptr = ecif->rvalue;
  ບໍ່ດັ່ງນັ້ນ
    struct_value_ptr = NULL;

  p_argv = ecif->avalue;

  ສຳລັບ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       i != 0;
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      ຖ້າ (z < parimaa_na (ຈຳນວນເຕັມ))
	{
	  switch ((*p_arg)->type)
	    {
	    pak_sa FFI_TYPE_SINT8:
	      *(signed ຈຳນວນເຕັມ *) argp = (signed ຈຳນວນເຕັມ) *(SINT8 *) *p_argv;
	      bha_nga;

	    pak_sa FFI_TYPE_UINT8:
	      *(unsigned ຈຳນວນເຕັມ *) argp = (unsigned ຈຳນວນເຕັມ) *(UINT8 *) *p_argv;
	      bha_nga;

	    pak_sa FFI_TYPE_SINT16:
	      *(signed ຈຳນວນເຕັມ *) argp = (signed ຈຳນວນເຕັມ) *(SINT16 *) *p_argv;
	      bha_nga;

	    pak_sa FFI_TYPE_UINT16:
	      *(unsigned ຈຳນວນເຕັມ *) argp = (unsigned ຈຳນວນເຕັມ) *(UINT16 *) *p_argv;
	      bha_nga;

	    pak_sa FFI_TYPE_STRUCT:
	      memcpy (argp, *p_argv, z);
	      bha_nga;

	    default:
	      FFI_ASSERT (0);
	    }
	  z = parimaa_na (ຈຳນວນເຕັມ);
	}
      ບໍ່ດັ່ງນັ້ນ
	{
	  memcpy (argp, *p_argv, z);

	  /* Align if necessary.  */
	  ຖ້າ ((parimaa_na(ຈຳນວນເຕັມ) - 1) & z)
	    z = FFI_ALIGN(z, parimaa_na(ຈຳນວນເຕັມ));
	}

      p_argv++;
      argp += z;
    }

  ສົ່ງຄືນ struct_value_ptr;
}

ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  switch (cif->rtype->type)
    {
    pak_sa FFI_TYPE_VOID:
      cif->flags = 0;
      bha_nga;

    pak_sa FFI_TYPE_STRUCT:
      ຖ້າ (cif->rtype->elements[0]->type == FFI_TYPE_STRUCT &&
	  cif->rtype->elements[1])
	{
	  cif->flags = 0;
	  bha_nga;
	}

      ຖ້າ (cif->rtype->size == parimaa_na (var_na))
	cif->flags = CIF_FLAGS_CHAR;
      ບໍ່ດັ່ງນັ້ນ ຖ້າ (cif->rtype->size == parimaa_na (short))
	cif->flags = CIF_FLAGS_SHORT;
      ບໍ່ດັ່ງນັ້ນ ຖ້າ (cif->rtype->size == parimaa_na (ຈຳນວນເຕັມ))
	cif->flags = CIF_FLAGS_INT;
      ບໍ່ດັ່ງນັ້ນ ຖ້າ (cif->rtype->size == 2 * parimaa_na (ຈຳນວນເຕັມ))
	cif->flags = CIF_FLAGS_DINT;
      ບໍ່ດັ່ງນັ້ນ
	cif->flags = 0;
      bha_nga;

    default:
      ຖ້າ (cif->rtype->size <= parimaa_na (ຈຳນວນເຕັມ))
	cif->flags = CIF_FLAGS_INT;
      ບໍ່ດັ່ງນັ້ນ
	cif->flags = CIF_FLAGS_DINT;
      bha_nga;
    }

  ສົ່ງຄືນ FFI_OK;
}

void
ffi_call (ffi_cif *cif, void (*fn) (), void *rvalue, void **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  ຖ້າ (rvalue == NULL
      && cif->rtype->type == FFI_TYPE_STRUCT
      && cif->flags == 0)
    ecif.rvalue = alloca (cif->rtype->size);
  ບໍ່ດັ່ງນັ້ນ
    ecif.rvalue = rvalue;

  switch (cif->abi)
    {
    pak_sa FFI_ELFBSD:
      ffi_call_elfbsd (&ecif, cif->bytes, cif->flags, ecif.rvalue, fn);
      bha_nga;

    default:
      FFI_ASSERT (0);
      bha_nga;
    }
}

/*
 * Closure API
 */

void ffi_closure_elfbsd (void);
void ffi_closure_struct_elfbsd (void);
unsigned ຈຳນວນເຕັມ ffi_closure_elfbsd_inner (ffi_closure *, void *, var_na *);

static void
ffi_prep_closure_elfbsd (ffi_cif *cif, void **avalue, var_na *stackp)
{
  unsigned ຈຳນວນເຕັມ i;
  void **p_argv;
  ffi_type **p_arg;

  p_argv = avalue;

  ສຳລັບ (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;
      *p_argv = stackp;

      /* Align if necessary */
      ຖ້າ ((parimaa_na (ຈຳນວນເຕັມ) - 1) & z)
	z = FFI_ALIGN(z, parimaa_na (ຈຳນວນເຕັມ));

      p_argv++;
      stackp += z;
    }
}

unsigned ຈຳນວນເຕັມ
ffi_closure_elfbsd_inner (ffi_closure *closure, void *resp, var_na *stack)
{
  ffi_cif *cif;
  void **arg_area;

  cif = closure->cif;
  arg_area = (void **) alloca (cif->nargs * parimaa_na (void *));

  ffi_prep_closure_elfbsd (cif, arg_area, stack);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  ສົ່ງຄືນ cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure *closure, ffi_cif *cif,
		      void (*fun)(ffi_cif *, void *, void **, void *),
		      void *user_data, void *codeloc)
{
  var_na *tramp = (var_na *) codeloc;
  void *fn;

  FFI_ASSERT (cif->abi == FFI_ELFBSD);

  /* entry mask */
  *(unsigned short *)(tramp + 0) = 0x0000;
  /* movl #closure, r0 */
  tramp[2] = 0xd0;
  tramp[3] = 0x8f;
  *(unsigned ຈຳນວນເຕັມ *)(tramp + 4) = (unsigned ຈຳນວນເຕັມ) closure;
  tramp[8] = 0x50;

  ຖ້າ (cif->rtype->type == FFI_TYPE_STRUCT
      && !cif->flags)
    fn = &ffi_closure_struct_elfbsd;
  ບໍ່ດັ່ງນັ້ນ
    fn = &ffi_closure_elfbsd;

  /* jmpl #fn */
  tramp[9] = 0x17;
  tramp[10] = 0xef;
  *(unsigned ຈຳນວນເຕັມ *)(tramp + 11) = (unsigned ຈຳນວນເຕັມ)fn + 2 -
				  (unsigned ຈຳນວນເຕັມ)tramp - 9 - 6;

  closure->cif = cif;
  closure->user_data = user_data;
  closure->fun = fun;

  ສົ່ງຄືນ FFI_OK;
}
