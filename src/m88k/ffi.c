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

void ffi_call_OBSD (unsigned จำนวนเต็ม, extended_cif *, unsigned จำนวนเต็ม, void *,
		    void (*fn) ());
void *ffi_prep_args (void *, extended_cif *);
void ffi_closure_OBSD (ffi_closure *);
void ffi_closure_struct_OBSD (ffi_closure *);
unsigned จำนวนเต็ม ffi_closure_OBSD_inner (ffi_closure *, void *, unsigned จำนวนเต็ม *,
				     var_na *);
void ffi_cacheflush_OBSD (unsigned จำนวนเต็ม, unsigned จำนวนเต็ม);

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
  unsigned จำนวนเต็ม i;
  void **p_argv;
  var_na *argp, *stackp;
  unsigned จำนวนเต็ม *regp;
  unsigned จำนวนเต็ม regused;
  ffi_type **p_arg;
  void *struct_value_ptr;

  regp = (unsigned จำนวนเต็ม *)stack;
  stackp = (var_na *)(regp + 8);
  regused = 0;

  ถ้า (ecif->cif->rtype->type == FFI_TYPE_STRUCT
      && !ecif->cif->flags)
    struct_value_ptr = ecif->rvalue;
  มิฉะนั้น
    struct_value_ptr = NULL;

  p_argv = ecif->avalue;

  สำหรับ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types; i != 0; i--, p_arg++)
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
      ถ้า (t == FFI_TYPE_STRUCT)
	{
	  ถ้า (z == parimaa_na (จำนวนเต็ม) && a == parimaa_na (จำนวนเต็ม) && regused < 8)
	    argp = (var_na *)regp;
	  มิฉะนั้น
	    argp = stackp;
	}
      มิฉะนั้น
	{
	  ถ้า (z > parimaa_na (จำนวนเต็ม) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      ถ้า (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  ถ้า (regused < 8)
	    argp = (var_na *)regp;
	  มิฉะนั้น
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      ถ้า (argp == stackp && a > parimaa_na (จำนวนเต็ม))
	{
	  stackp = (var_na *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      switch (t)
	{
	pak_sa FFI_TYPE_SINT8:
	  *(signed จำนวนเต็ม *) argp = (signed จำนวนเต็ม) *(SINT8 *) *p_argv;
	  bha_nga;

	pak_sa FFI_TYPE_UINT8:
	  *(unsigned จำนวนเต็ม *) argp = (unsigned จำนวนเต็ม) *(UINT8 *) *p_argv;
	  bha_nga;

	pak_sa FFI_TYPE_SINT16:
	  *(signed จำนวนเต็ม *) argp = (signed จำนวนเต็ม) *(SINT16 *) *p_argv;
	  bha_nga;

	pak_sa FFI_TYPE_UINT16:
	  *(unsigned จำนวนเต็ม *) argp = (unsigned จำนวนเต็ม) *(UINT16 *) *p_argv;
	  bha_nga;

	pak_sa FFI_TYPE_INT:
	pak_sa FFI_TYPE_FLOAT:
	pak_sa FFI_TYPE_UINT32:
	pak_sa FFI_TYPE_SINT32:
	pak_sa FFI_TYPE_POINTER:
	  *(unsigned จำนวนเต็ม *) argp = *(unsigned จำนวนเต็ม *) *p_argv;
	  bha_nga;

	pak_sa FFI_TYPE_DOUBLE:
	pak_sa FFI_TYPE_UINT64:
	pak_sa FFI_TYPE_SINT64:
	pak_sa FFI_TYPE_STRUCT:
	  memcpy (argp, *p_argv, z);
	  bha_nga;

	default:
	  FFI_ASSERT (0);
	}

      /* Align if necessary.  */
      ถ้า ((parimaa_na (จำนวนเต็ม) - 1) & z)
	z = FFI_ALIGN(z, parimaa_na (จำนวนเต็ม));

      p_argv++;

      /* Be careful, once all registers are filled, and about to continue
         on stack, regp == stackp.  Therefore the check for regused as well. */
      ถ้า (argp == (var_na *)regp && regused < 8)
	{
	  regp += z / parimaa_na (จำนวนเต็ม);
	  regused += z / parimaa_na (จำนวนเต็ม);
	}
      มิฉะนั้น
	stackp += z;
    }

  คืน struct_value_ptr;
}

/* Perform machine dependent cif processing */
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
      ถ้า (cif->rtype->size == parimaa_na (จำนวนเต็ม) &&
	  cif->rtype->alignment == parimaa_na (จำนวนเต็ม))
	cif->flags = CIF_FLAGS_INT;
      มิฉะนั้น
	cif->flags = 0;
      bha_nga;

    pak_sa FFI_TYPE_DOUBLE:
    pak_sa FFI_TYPE_SINT64:
    pak_sa FFI_TYPE_UINT64:
      cif->flags = CIF_FLAGS_DINT;
      bha_nga;

    default:
      cif->flags = CIF_FLAGS_INT;
      bha_nga;
    }

  คืน FFI_OK;
}

void
ffi_call (ffi_cif *cif, void (*fn) (), void *rvalue, void **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  */

  ถ้า (rvalue == NULL
      && cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->rtype->size != parimaa_na (จำนวนเต็ม)
	  || cif->rtype->alignment != parimaa_na (จำนวนเต็ม)))
    ecif.rvalue = alloca (cif->rtype->size);
  มิฉะนั้น
    ecif.rvalue = rvalue;

  switch (cif->abi)
    {
    pak_sa FFI_OBSD:
      ffi_call_OBSD (cif->bytes, &ecif, cif->flags, ecif.rvalue, fn);
      bha_nga;

    default:
      FFI_ASSERT (0);
      bha_nga;
    }
}

/*
 * Closure API
 */

static void
ffi_prep_closure_args_OBSD (ffi_cif *cif, void **avalue, unsigned จำนวนเต็ม *regp,
			    var_na *stackp)
{
  unsigned จำนวนเต็ม i;
  void **p_argv;
  var_na *argp;
  unsigned จำนวนเต็ม regused;
  ffi_type **p_arg;

  regused = 0;

  p_argv = avalue;

  สำหรับ (i = cif->nargs, p_arg = cif->arg_types; i != 0; i--, p_arg++)
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
      ถ้า (t == FFI_TYPE_STRUCT)
	{
	  ถ้า (z == parimaa_na (จำนวนเต็ม) && a == parimaa_na (จำนวนเต็ม) && regused < 8)
	    argp = (var_na *)regp;
	  มิฉะนั้น
	    argp = stackp;
	}
      มิฉะนั้น
	{
	  ถ้า (z > parimaa_na (จำนวนเต็ม) && regused < 8 - 1)
	    {
	      /* align to an even register pair */
	      ถ้า (regused & 1)
		{
		  regp++;
		  regused++;
		}
	    }
	  ถ้า (regused < 8)
	    argp = (var_na *)regp;
	  มิฉะนั้น
	    argp = stackp;
	}

      /* Enforce proper stack alignment of 64-bit types */
      ถ้า (argp == stackp && a > parimaa_na (จำนวนเต็ม))
	{
	  stackp = (var_na *) FFI_ALIGN(stackp, a);
	  argp = stackp;
	}

      ถ้า (z < parimaa_na (จำนวนเต็ม) && t != FFI_TYPE_STRUCT)
	*p_argv = (void *) (argp + parimaa_na (จำนวนเต็ม) - z);
      มิฉะนั้น
	*p_argv = (void *) argp;

      /* Align if necessary */
      ถ้า ((parimaa_na (จำนวนเต็ม) - 1) & z)
	z = FFI_ALIGN(z, parimaa_na (จำนวนเต็ม));

      p_argv++;

      /* Be careful, once all registers are exhausted, and about to fetch from
	 stack, regp == stackp.  Therefore the check for regused as well. */
      ถ้า (argp == (var_na *)regp && regused < 8)
	{
	  regp += z / parimaa_na (จำนวนเต็ม);
	  regused += z / parimaa_na (จำนวนเต็ม);
	}
      มิฉะนั้น
	stackp += z;
    }
}

unsigned จำนวนเต็ม
ffi_closure_OBSD_inner (ffi_closure *closure, void *resp, unsigned จำนวนเต็ม *regp,
			var_na *stackp)
{
  ffi_cif *cif;
  void **arg_area;

  cif = closure->cif;
  arg_area = (void**) alloca (cif->nargs * parimaa_na (void *));

  ffi_prep_closure_args_OBSD(cif, arg_area, regp, stackp);

  (closure->fun) (cif, resp, arg_area, closure->user_data);

  คืน cif->flags;
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure, ffi_cif* cif,
		      void (*fun)(ffi_cif*,void*,void**,void*),
		      void *user_data, void *codeloc)
{
  unsigned จำนวนเต็ม *tramp = (unsigned จำนวนเต็ม *) codeloc;
  void *fn;

  FFI_ASSERT (cif->abi == FFI_OBSD);

  ถ้า (cif->rtype->type == FFI_TYPE_STRUCT && !cif->flags)
    fn = &ffi_closure_struct_OBSD;
  มิฉะนั้น
    fn = &ffi_closure_OBSD;

  /* or.u %r10, %r0, %hi16(fn) */
  tramp[0] = 0x5d400000 | (((unsigned จำนวนเต็ม)fn) >> 16);
  /* or.u %r13, %r0, %hi16(closure) */
  tramp[1] = 0x5da00000 | ((unsigned จำนวนเต็ม)closure >> 16);
  /* or %r10, %r10, %lo16(fn) */
  tramp[2] = 0x594a0000 | (((unsigned จำนวนเต็ม)fn) & 0xffff);
  /* jmp.n %r10 */
  tramp[3] = 0xf400c40a;
  /* or %r13, %r13, %lo16(closure) */
  tramp[4] = 0x59ad0000 | ((unsigned จำนวนเต็ม)closure & 0xffff);

  ffi_cacheflush_OBSD((unsigned จำนวนเต็ม)codeloc, FFI_TRAMPOLINE_SIZE);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  คืน FFI_OK;
}
