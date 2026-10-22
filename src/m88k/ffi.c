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
 * m88k Foreign Function Interface
 *
 * This file attempts to provide all the FFI entry points which can reliably
 * be implemented in C.
 *
 * Only OpenBSD/m88k is currently supported; other platforms (such as
 * Motorola's SysV/m88k) could be supported with the following tweaks:
 *
 * - non-OpenBSD systems use an `outgoing parameter area' as part of the
 *   88BCS calling convention, which is not supported under OpenBSD from
 *   release 3.6 onwards.  Supporting it should be as easy as taking it
 *   into account when adjusting the stack, in the assembly code.
 *
 * - the logic deciding whether a function argument gets passed through
 *   registers, or on the stack, has changed several times in OpenBSD in
 *   edge cases (especially for structs larger than 32 bytes being passed
 *   by value). The code below attemps to match the logic used by the
 *   system compiler of OpenBSD 5.3, i.e. gcc 3.3.6 with many m88k backend
 *   fixes.
 */

#include <ffi.h>
#include <ffi_common.h>

#include <stdlib.h>
#include <unistd.h>

void ffi_call_OBSD (unsigned ціле, extended_cif *, unsigned ціле, void *,
		    void (*fn) ());
void *ffi_prep_args (void *, extended_cif *);
void ffi_closure_OBSD (ffi_closure *);
void ffi_closure_struct_OBSD (ffi_closure *);
unsigned ціле ffi_closure_OBSD_inner (ffi_closure *, void *, unsigned ціле *,
				     char *);
void ffi_cacheflush_OBSD (unsigned ціле, unsigned ціле);

#define CIF_FLAGS_INT		(1 << 0)
#define CIF_FLAGS_DINT		(1 << 1)

/*
 * Foreign Function Interface API
 */

/* ffi_prep_args is called by the assembly routine once stack space has
   been allocated for the function's arguments.  */

void *
ffi_prep_args (void *stack, extended_cif *ecif)
{
  unsigned ціле i;
  void **p_argv;
  char *argp, *stackp;
  unsigned ціле *regp;
  unsigned ціле regused;
  ffi_type **p_arg;
  void *struct_value_ptr;

  regp = (unsigned ціле *)stack;
  stackp = (char *)(regp + 8);
  regused = 0;

  якщо (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_value_ptr = ecif->rvalue;
  інакше
    struct_value_ptr = NULL;

  p_argv = ecif->avalue;

  для (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;
      unsigned short t, a;

      z = (*p_arg)->size;
      t = (*p_arg)->type;
      a = (*p_arg)->alignment;

      /*
       * Figure out whether the argument can be passed through registers
       * or on the stack.
       * The rule is that registers can only receive simple types not larger
       * than 64 bits, or structs the exact size of a register and aligned to
       * the size of a register.
       */
      якщо (t == FFI_TYPE_STRUCT)
	{
	  якщо (z == sizeof (ціле) && a == sizeof (ціле) && regused < 8)
	    argp = (char *)regp;
	  інакше
	    argp = stackp;
	}
      інакше
	{
	  якщо (z > sizeof (ціле) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      якщо (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  якщо (regused < 8)
	    argp = (char *)regp;
	  інакше
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      якщо (argp == stackp && a > sizeof (ціле))
	{
	  stackp = (char *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      switch (t)
	{
	case FFI_TYPE_SINT8:
	  *(signed ціле *) argp = (signed ціле) *(SINT8 *) *p_argv;
	  break;

	case FFI_TYPE_UINT8:
	  *(unsigned ціле *) argp = (unsigned ціле) *(UINT8 *) *p_argv;
	  break;

	case FFI_TYPE_SINT16:
	  *(signed ціле *) argp = (signed ціле) *(SINT16 *) *p_argv;
	  break;

	case FFI_TYPE_UINT16:
	  *(unsigned ціле *) argp = (unsigned ціле) *(UINT16 *) *p_argv;
	  break;

	case FFI_TYPE_INT:
	case FFI_TYPE_FLOAT:
	case FFI_TYPE_UINT32:
	case FFI_TYPE_SINT32:
	case FFI_TYPE_POINTER:
	  *(unsigned ціле *) argp = *(unsigned ціле *) *p_argv;
	  break;

	case FFI_TYPE_DOUBLE:
	case FFI_TYPE_UINT64:
	case FFI_TYPE_SINT64:
	case FFI_TYPE_STRUCT:
	  memcpy (argp, *p_argv, z);
	  break;

	default:
	  FFI_ASSERT (0);
	}

      /* Align if necessary.  */
      якщо ((sizeof (ціле) - 1) & z)
	z = FFI_ALIGN(z, sizeof (ціле));

      p_argv++;

      /* Be careful, once all registers are filled, and about to continue
         on stack, regp == stackp.  Therefore the check for regused as well. */
      якщо (argp == (char *)regp && regused < 8)
	{
	  regp += z / sizeof (ціле);
	  regused += z / sizeof (ціле);
	}
      інакше
	stackp += z;
    }

  повернути struct_value_ptr;
}

/* Perform machine dependent cif processing */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* Set the return type flag */
  switch (cif->rtype->type)
    {
    case FFI_TYPE_VOID:
      cif->flags = 0;
      break;

    case FFI_TYPE_STRUCT:
      якщо (cif->rtype->size == sizeof (ціле) &&
	  cif->rtype->alignment == sizeof (ціле))
	cif->flags = CIF_FLAGS_INT;
      інакше
	cif->flags = 0;
      break;

    case FFI_TYPE_DOUBLE:
    case FFI_TYPE_SINT64:
    case FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      break;

    default:
      cif->flags = CIF_FLAGS_INT;
      break;
    }

  повернути FFI_OK;
}

void
ffi_call (ffi_cif *cif, void (*fn) (), void *rvalue, void **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  якщо (rvalue == NULL
      && cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->rtype->size != sizeof (ціле)
	  || cif->rtype->alignment != sizeof (ціле)))
    ecif.rvalue = alloca (cif->rtype->size);
  інакше
    ecif.rvalue = rvalue;

  switch (cif->abi)
    {
    case FFI_OBSD:
      ffi_call_OBSD (cif->bytes, &ecif, cif->flags, ecif.rvalue, fn);
      break;

    default:
      FFI_ASSERT (0);
      break;
    }
}

/*
 * Closure API
 */

static void
ffi_prep_closure_args_OBSD (ffi_cif *cif, void **avalue, unsigned ціле *regp,
			    char *stackp)
{
  unsigned ціле i;
  void **p_argv;
  char *argp;
  unsigned ціле regused;
  ffi_type **p_arg;

  regused = 0;

  p_argv = avalue;

  для (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
    {
      size_t z;
      unsigned short t, a;

      z = (*p_arg)->size;
      t = (*p_arg)->type;
      a = (*p_arg)->alignment;

      /*
       * Figure out whether the argument has been passed through registers
       * or on the stack.
       * The rule is that registers can only receive simple types not larger
       * than 64 bits, or structs the exact size of a register and aligned to
       * the size of a register.
       */
      якщо (t == FFI_TYPE_STRUCT)
	{
	  якщо (z == sizeof (ціле) && a == sizeof (ціле) && regused < 8)
	    argp = (char *)regp;
	  інакше
	    argp = stackp;
	}
      інакше
	{
	  якщо (z > sizeof (ціле) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      якщо (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  якщо (regused < 8)
	    argp = (char *)regp;
	  інакше
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      якщо (argp == stackp && a > sizeof (ціле))
	{
	  stackp = (char *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      якщо (z < sizeof (ціле) && t != FFI_TYPE_STRUCT)
	*p_argv = (void *) (argp + sizeof (ціле) - z);
      інакше
	*p_argv = (void *) argp;

      /* Align if necessary */
      якщо ((sizeof (ціле) - 1) & z)
	z = FFI_ALIGN(z, sizeof (ціле));

      p_argv++;

      /* Be careful, once all registers are exhausted, and about to fetch from
	 stack, regp == stackp.  Therefore the check for regused as well. */
      якщо (argp == (char *)regp && regused < 8)
	{
	  regp += z / sizeof (ціле);
	  regused += z / sizeof (ціле);
	}
      інакше
	stackp += z;
    }
}

unsigned ціле
ffi_closure_OBSD_inner (ffi_closure *closure, void *resp, unsigned ціле *regp,
			char *stackp)
{
  ffi_cif *cif;
  void **arg_area;

  cif = closure->cif;
  arg_area = (void**) alloca (cif->nargs * sizeof (void *));

  ffi_prep_closure_args_OBSD(cif, arg_area, regp, stackp);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  повернути cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure, ffi_cif* cif,
		      void (*fun)(ffi_cif*,void*,void**,void*),
		      void *user_data, void *codeloc)
{
  unsigned ціле *tramp = (unsigned ціле *) codeloc;
  void *fn;

  FFI_ASSERT (cif->abi == FFI_OBSD);

  якщо (cif->rtype->type == FFI_TYPE_STRUCT && !cif->flags)
    fn = &ffi_closure_struct_OBSD;
  інакше
    fn = &ffi_closure_OBSD;

  /* or.u %r10, %r0, %hi16(fn) */
  tramp[0] = 0x5d400000 | (((unsigned ціле)fn) >> 16);
  /* or.u %r13, %r0, %hi16(closure) */
  tramp[1] = 0x5da00000 | ((unsigned ціле)closure >> 16);
  /* or %r10, %r10, %lo16(fn) */
  tramp[2] = 0x594a0000 | (((unsigned ціле)fn) & 0xffff);
  /* jmp.n %r10 */
  tramp[3] = 0xf400c40a;
  /* or %r13, %r13, %lo16(closure) */
  tramp[4] = 0x59ad0000 | ((unsigned ціле)closure & 0xffff);

  ffi_cacheflush_OBSD((unsigned ціле)codeloc, FFI_TRAMPOLINE_SIZE);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  повернути FFI_OK;
}
