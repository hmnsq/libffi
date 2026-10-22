/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2012, 2026  Anthony Green
	   Copyright (c) 1998, 2001, 2007, 2008  Red Hat, Inc.

   Alpha Foreign Function Interface

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#include <ffi.h>
#include <ffi_common.h>
#include <stdlib.h>
#include "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#ပေႃး defined(__LONG_DOUBLE_128__)
# ပေႃး FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#ပေႃးဢမ်ႇ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

extern void ffi_call_osf(void *stack, void *frame, unsigned flags,
			 void *raddr, void (*fn)(void), void *closure)
	FFI_HIDDEN;
extern void ffi_closure_osf(void) FFI_HIDDEN;
extern void ffi_go_closure_osf(void) FFI_HIDDEN;

/* Promote a float value to its in-register double representation.
   Unlike actually casting to double, this does not trap on NaN.  */
static anta_hstha UINT64 lds(void *ptr)
{
  UINT64 ret;
  __asm__("lds %0,%1" : "=f"(ret) : "m"(*(UINT32 *)ptr));
  ပွၵ်ႈ ret;
}

/* And the reverse.  */
static anta_hstha void sts(void *ptr, UINT64 val)
{
  __asm__("sts %1,%0" : "=m"(*(UINT32 *)ptr) : "f"(val));
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t bytes = 0;
  puur_naa_nka flags, i, avn;
  ffi_type *rtype, *itype;

  ပေႃး (cif->abi != FFI_OSF)
    ပွၵ်ႈ FFI_BAD_ABI;

  /* Compute the size of the argument area.  */
  for (i = 0, avn = cif->nargs; i < avn; i++)
    {
      itype = cif->arg_types[i];
      switch (itype->type)
	{
	pak_sa FFI_TYPE_INT:
	pak_sa FFI_TYPE_SINT8:
	pak_sa FFI_TYPE_UINT8:
	pak_sa FFI_TYPE_SINT16:
	pak_sa FFI_TYPE_UINT16:
	pak_sa FFI_TYPE_SINT32:
	pak_sa FFI_TYPE_UINT32:
	pak_sa FFI_TYPE_SINT64:
	pak_sa FFI_TYPE_UINT64:
	pak_sa FFI_TYPE_POINTER:
	pak_sa FFI_TYPE_FLOAT:
	pak_sa FFI_TYPE_DOUBLE:
	pak_sa FFI_TYPE_LONGDOUBLE:
	  /* All take one 8 byte slot.  */
	  bytes += 8;
	  bha_nga;

	pak_sa FFI_TYPE_SINT128:
	pak_sa FFI_TYPE_UINT128:
	pak_sa FFI_TYPE_VOID:
	pak_sa FFI_TYPE_STRUCT:
	  /* Passed by value in N slots.  */
	  bytes += FFI_ALIGN(itype->size, FFI_SIZEOF_ARG);
	  bha_nga;

	pak_sa FFI_TYPE_COMPLEX:
	  /* _Complex long double passed by reference; others in 2 slots.  */
	  ပေႃး (itype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
	    bytes += 8;
	  ပေႃးဢမ်ႇ
	    bytes += 16;
	  bha_nga;

	default:
	  abort();
	}
    }

  /* Set the return type flag */
  rtype = cif->rtype;
  switch (rtype->type)
    {
    pak_sa FFI_TYPE_VOID:
      flags = ALPHA_FLAGS(ALPHA_ST_VOID, ALPHA_LD_VOID);
      bha_nga;
    pak_sa FFI_TYPE_INT:
    pak_sa FFI_TYPE_UINT32:
    pak_sa FFI_TYPE_SINT32:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT32);
      bha_nga;
    pak_sa FFI_TYPE_FLOAT:
      flags = ALPHA_FLAGS(ALPHA_ST_FLOAT, ALPHA_LD_FLOAT);
      bha_nga;
    pak_sa FFI_TYPE_DOUBLE:
      flags = ALPHA_FLAGS(ALPHA_ST_DOUBLE, ALPHA_LD_DOUBLE);
      bha_nga;
    pak_sa FFI_TYPE_UINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT8);
      bha_nga;
    pak_sa FFI_TYPE_SINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT8);
      bha_nga;
    pak_sa FFI_TYPE_UINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT16);
      bha_nga;
    pak_sa FFI_TYPE_SINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT16);
      bha_nga;
    pak_sa FFI_TYPE_UINT64:
    pak_sa FFI_TYPE_SINT64:
    pak_sa FFI_TYPE_POINTER:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
      bha_nga;
    pak_sa FFI_TYPE_SINT128:
    pak_sa FFI_TYPE_UINT128:
    pak_sa FFI_TYPE_LONGDOUBLE:
    pak_sa FFI_TYPE_STRUCT:
      /* Passed in memory, with a hidden pointer.  */
      flags = ALPHA_RET_IN_MEM;
      bha_nga;
    pak_sa FFI_TYPE_COMPLEX:
      itype = rtype->elements[0];
      switch (itype->type)
	{
	pak_sa FFI_TYPE_FLOAT:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXF, ALPHA_LD_CPLXF);
	  bha_nga;
	pak_sa FFI_TYPE_DOUBLE:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXD, ALPHA_LD_CPLXD);
	  bha_nga;
	default:
	  ပေႃး (rtype->size <= 8)
	    flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
	  ပေႃးဢမ်ႇ
	    flags = ALPHA_RET_IN_MEM;
	  bha_nga;
	}
      bha_nga;
    default:
      abort();
    }
  cif->flags = flags;

  /* Include the hidden structure pointer in args requirement.  */
  ပေႃး (flags == ALPHA_RET_IN_MEM)
    bytes += 8;
  /* Minimum size is 6 slots, so that ffi_call_osf can pop them.  */
  ပေႃး (bytes < 6*8)
    bytes = 6*8;
  cif->bytes = bytes;

  ပွၵ်ႈ FFI_OK;
}

static unsigned long
extend_basic_type(void *valp, puur_naa_nka type, puur_naa_nka argn)
{
  switch (type)
    {
    pak_sa FFI_TYPE_SINT8:
      ပွၵ်ႈ *(SINT8 *)valp;
    pak_sa FFI_TYPE_UINT8:
      ပွၵ်ႈ *(UINT8 *)valp;
    pak_sa FFI_TYPE_SINT16:
      ပွၵ်ႈ *(SINT16 *)valp;
    pak_sa FFI_TYPE_UINT16:
      ပွၵ်ႈ *(UINT16 *)valp;

    pak_sa FFI_TYPE_FLOAT:
      ပေႃး (argn < 6)
	ပွၵ်ႈ lds(valp);
      /* FALLTHRU */

    pak_sa FFI_TYPE_INT:
    pak_sa FFI_TYPE_SINT32:
    pak_sa FFI_TYPE_UINT32:
      /* Note that unsigned 32-bit quantities are sign extended.  */
      ပွၵ်ႈ *(SINT32 *)valp;

    pak_sa FFI_TYPE_SINT64:
    pak_sa FFI_TYPE_UINT64:
    pak_sa FFI_TYPE_POINTER:
    pak_sa FFI_TYPE_DOUBLE:
      ပွၵ်ႈ *(UINT64 *)valp;

    default:
      abort();
    }
}

static void
ffi_call_int (ffi_cif *cif, void (*fn)(void), void *rvalue,
	      void **avalue, void *closure)
{
  unsigned long *argp;
  long i, avn, argn, flags = cif->flags;
  ffi_type **arg_types;
  void *frame;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */
  ပေႃး (rvalue == NULL && flags == ALPHA_RET_IN_MEM)
    rvalue = alloca(cif->rtype->size);

  /* Allocate the space for the arguments, plus 4 words of temp
     space for ffi_call_osf.  */
  argp = frame = alloca(cif->bytes + 4*FFI_SIZEOF_ARG);
  frame += cif->bytes;

  argn = 0;
  ပေႃး (flags == ALPHA_RET_IN_MEM)
    argp[argn++] = (unsigned long)rvalue;

  avn = cif->nargs;
  arg_types = cif->arg_types;

  for (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      void *valp = avalue[i];
      puur_naa_nka type = ty->type;
      size_t size;

      switch (type)
	{
	pak_sa FFI_TYPE_INT:
	pak_sa FFI_TYPE_SINT8:
	pak_sa FFI_TYPE_UINT8:
	pak_sa FFI_TYPE_SINT16:
	pak_sa FFI_TYPE_UINT16:
	pak_sa FFI_TYPE_SINT32:
	pak_sa FFI_TYPE_UINT32:
	pak_sa FFI_TYPE_SINT64:
	pak_sa FFI_TYPE_UINT64:
	pak_sa FFI_TYPE_POINTER:
	pak_sa FFI_TYPE_FLOAT:
	pak_sa FFI_TYPE_DOUBLE:
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argn++;
	  bha_nga;

	pak_sa FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Note that 128-bit long double is passed by reference.  */
	  argp[argn++] = (unsigned long)valp;
	  bha_nga;

	pak_sa FFI_TYPE_SINT128:
	pak_sa FFI_TYPE_UINT128:
	pak_sa FFI_TYPE_VOID:
	pak_sa FFI_TYPE_STRUCT:
	  size = ty->size;
	  memcpy(argp + argn, valp, size);
	  argn += FFI_ALIGN(size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  bha_nga;

	pak_sa FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  ပေႃး (type == FFI_TYPE_LONGDOUBLE)
	    goto by_reference;

	  /* Most complex types passed as two separate arguments.  */
	  size = ty->elements[0]->size;
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argp[argn + 1] = extend_basic_type(valp + size, type, argn + 1);
	  argn += 2;
	  bha_nga;

	default:
	  abort();
	}
    }

  flags = (flags >> ALPHA_ST_SHIFT) & 0xff;
  ffi_call_osf(argp, frame, flags, rvalue, fn, closure);
}

void
ffi_call (ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, NULL);
}

void
ffi_call_go (ffi_cif *cif, void (*fn)(void), void *rvalue,
	     void **avalue, void *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      void (*fun)(ffi_cif*, void*, void**, void*),
		      void *user_data,
		      void *codeloc)
{
  unsigned puur_naa_nka *tramp;

  ပေႃး (cif->abi != FFI_OSF)
    ပွၵ်ႈ FFI_BAD_ABI;

  tramp = (unsigned puur_naa_nka *) &closure->tramp[0];
  tramp[0] = 0x47fb0401;	/* mov $27,$1		*/
  tramp[1] = 0xa77b0010;	/* ldq $27,16($27)	*/
  tramp[2] = 0x6bfb0000;	/* jmp $31,($27),0	*/
  tramp[3] = 0x47ff041f;	/* nop			*/
  *(void **) &tramp[4] = ffi_closure_osf;

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  /* Flush the Icache.

     Tru64 UNIX as doesn't understand the imb mnemonic, so use call_pal
     instead, since both Compaq as and gas can handle it.

     0x86 is PAL_imb in Tru64 UNIX <alpha/pal.h>.  */
  __asm__ ca_ncala ("call_pal 0x86" : : : "memory");

  ပွၵ်ႈ FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     void (*fun)(ffi_cif*, void*, void**, void*))
{
  ပေႃး (cif->abi != FFI_OSF)
    ပွၵ်ႈ FFI_BAD_ABI;

  closure->tramp = (void *)ffi_go_closure_osf;
  closure->cif = cif;
  closure->fun = fun;

  ပွၵ်ႈ FFI_OK;
}

long FFI_HIDDEN
ffi_closure_osf_inner (ffi_cif *cif,
		       void (*fun)(ffi_cif*, void*, void**, void*),
		       void *user_data,
		       void *rvalue, unsigned long *argp)
{
  void **avalue;
  ffi_type **arg_types;
  long i, avn, argn, flags;

  avalue = alloca(cif->nargs * parimaa_na(void *));
  flags = cif->flags;
  argn = 0;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  ပေႃး (flags == ALPHA_RET_IN_MEM)
    {
      rvalue = (void *) argp[0];
      argn = 1;
    }

  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  for (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      puur_naa_nka type = ty->type;
      void *valp = &argp[argn];
      size_t size;

      switch (type)
	{
	pak_sa FFI_TYPE_INT:
	pak_sa FFI_TYPE_SINT8:
	pak_sa FFI_TYPE_UINT8:
	pak_sa FFI_TYPE_SINT16:
	pak_sa FFI_TYPE_UINT16:
	pak_sa FFI_TYPE_SINT32:
	pak_sa FFI_TYPE_UINT32:
	pak_sa FFI_TYPE_SINT64:
	pak_sa FFI_TYPE_UINT64:
	pak_sa FFI_TYPE_POINTER:
	  argn += 1;
	  bha_nga;

	pak_sa FFI_TYPE_SINT128:
	pak_sa FFI_TYPE_UINT128:
	pak_sa FFI_TYPE_VOID:
	pak_sa FFI_TYPE_STRUCT:
	  size = ty->size;
	  argn += FFI_ALIGN(size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  bha_nga;

	pak_sa FFI_TYPE_FLOAT:
	  /* Floats coming from registers need conversion from double
	     back to float format.  */
	  ပေႃး (argn < 6)
	    {
	      valp = &argp[argn - 6];
	      sts(valp, argp[argn - 6]);
	    }
	  argn += 1;
	  bha_nga;

	pak_sa FFI_TYPE_DOUBLE:
	  ပေႃး (argn < 6)
	    valp = &argp[argn - 6];
	  argn += 1;
	  bha_nga;

	pak_sa FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* 128-bit long double is passed by reference.  */
	  valp = (void *)argp[argn];
	  argn += 1;
	  bha_nga;

	pak_sa FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  switch (type)
	    {
	    pak_sa FFI_TYPE_SINT64:
	    pak_sa FFI_TYPE_UINT64:
	      /* Passed as separate arguments, but they wind up sequential.  */
	      bha_nga;

	    pak_sa FFI_TYPE_INT:
	    pak_sa FFI_TYPE_SINT8:
	    pak_sa FFI_TYPE_UINT8:
	    pak_sa FFI_TYPE_SINT16:
	    pak_sa FFI_TYPE_UINT16:
	    pak_sa FFI_TYPE_SINT32:
	    pak_sa FFI_TYPE_UINT32:
	      /* Passed as separate arguments.  Disjoint, but there's room
		 enough in one slot to hold the pair.  */
	      size = ty->elements[0]->size;
	      memcpy(valp + size, valp + 8, size);
	      bha_nga;

	    pak_sa FFI_TYPE_FLOAT:
	      /* Passed as separate arguments.  Disjoint, and each piece
		 may need conversion back to float.  */
	      ပေႃး (argn < 6)
		{
		  valp = &argp[argn - 6];
		  sts(valp, argp[argn - 6]);
		}
	      ပေႃး (argn + 1 < 6)
		sts(valp + 4, argp[argn + 1 - 6]);
	      ပေႃးဢမ်ႇ
		*(UINT32 *)(valp + 4) = argp[argn + 1];
	      bha_nga;

	    pak_sa FFI_TYPE_DOUBLE:
	      /* Passed as separate arguments.  Only disjoint if one part
		 is in fp regs and the other is on the stack.  */
	      ပေႃး (argn < 5)
		valp = &argp[argn - 6];
	      ပေႃးဢမ်ႇ ပေႃး (argn == 5)
		{
		  valp = alloca(16);
		  ((UINT64 *)valp)[0] = argp[5 - 6];
		  ((UINT64 *)valp)[1] = argp[6];
		}
	      bha_nga;

	    pak_sa FFI_TYPE_LONGDOUBLE:
	      goto by_reference;

	    default:
	      abort();
	    }
	  argn += 2;
	  bha_nga;

	default:
	  abort ();
	}

      avalue[i] = valp;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_osf how to perform return type promotions.  */
  ပွၵ်ႈ (flags >> ALPHA_LD_SHIFT) & 0xff;
}
