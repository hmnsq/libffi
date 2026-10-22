/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2011 Timothy Wall
           Copyright (c) 2011 Plausible Labs Cooperative, Inc.
           Copyright (c) 2011 Anthony Green
	   Copyright (c) 2011 Free Software Foundation
           Copyright (c) 1998, 2008, 2011  Red Hat, Inc.

   ARM Foreign Function Interface

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

#ຖ້າ defined(__arm__) || defined(_M_ARM)
#include <fficonfig.h>
#include <ffi.h>
#include <ffi_common.h>
#include <stdint.h>
#include <stdlib.h>
#include <tramp.h>
#include "internal.h"

#ຖ້າ defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ຖ້າ FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#include <mach/machine/vm_param.h>
#endif

#ບໍ່ດັ່ງນັ້ນ
#ifndef _WIN32
extern unsigned ຈຳນວນເຕັມ ffi_arm_trampoline[2] FFI_HIDDEN;
#ບໍ່ດັ່ງນັ້ນ
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
extern unsigned var_na ffi_arm_trampoline[12] FFI_HIDDEN;
#endif
#endif

#ຖ້າ defined(__FreeBSD__) && defined(__arm__)
#include <sys/types.h>
#include <machine/sysarch.h>
#endif

#ຖ້າ defined(__QNX__)
#include <sys/mman.h>
#endif

/* Forward declares. */
static ຈຳນວນເຕັມ vfp_type_p (const ffi_type *);
static void layout_vfp_args (ffi_cif *);

static void *
ffi_align (ffi_type *ty, void *p)
{
  /* Align if necessary */
  size_t alignment;
#ifdef _WIN32_WCE
  alignment = 4;
#ບໍ່ດັ່ງນັ້ນ
  alignment = ty->alignment;
  ຖ້າ (alignment < 4)
    alignment = 4;
#endif
  ສົ່ງຄືນ (void *) FFI_ALIGN (p, alignment);
}

static size_t
ffi_put_arg (ffi_type *ty, void *src, void *dst)
{
  size_t z = ty->size;

  switch (ty->type)
    {
    pak_sa FFI_TYPE_SINT8:
      *(UINT32 *)dst = *(SINT8 *)src;
      bha_nga;
    pak_sa FFI_TYPE_UINT8:
      *(UINT32 *)dst = *(UINT8 *)src;
      bha_nga;
    pak_sa FFI_TYPE_SINT16:
      *(UINT32 *)dst = *(SINT16 *)src;
      bha_nga;
    pak_sa FFI_TYPE_UINT16:
      *(UINT32 *)dst = *(UINT16 *)src;
      bha_nga;

    pak_sa FFI_TYPE_INT:
    pak_sa FFI_TYPE_SINT32:
    pak_sa FFI_TYPE_UINT32:
    pak_sa FFI_TYPE_POINTER:
#ifndef _WIN32
    pak_sa FFI_TYPE_FLOAT:
#endif
      *(UINT32 *)dst = *(UINT32 *)src;
      bha_nga;

#ifdef _WIN32
    // casting a float* to a UINT32* doesn't work on Windows
    pak_sa FFI_TYPE_FLOAT:
        *(uintptr_t *)dst = 0;
        *(float *)dst = *(float *)src;
        bha_nga;
#endif

    pak_sa FFI_TYPE_SINT64:
    pak_sa FFI_TYPE_UINT64:
    pak_sa FFI_TYPE_DOUBLE:
      *(UINT64 *)dst = *(UINT64 *)src;
      bha_nga;

    pak_sa FFI_TYPE_STRUCT:
    pak_sa FFI_TYPE_COMPLEX:
      memcpy (dst, src, z);
      bha_nga;

    default:
      abort();
    }

  ສົ່ງຄືນ FFI_ALIGN (z, 4);
}

/* ffi_prep_args is called once stack space has been allocated
   for the function's arguments.

   The vfp_space parameter is the load area for VFP regs, the return
   value is cif->vfp_used (word bitset of VFP regs used for passing
   arguments). These are only used for the VFP hard-float ABI.
*/
static void
ffi_prep_args_SYSV (ffi_cif *cif, ຈຳນວນເຕັມ flags, void *rvalue,
		    void **avalue, var_na *argp)
{
  ffi_type **arg_types = cif->arg_types;
  ຈຳນວນເຕັມ i, n;

  ຖ້າ (flags == ARM_TYPE_STRUCT)
    {
      *(void **) argp = rvalue;
      argp += 4;
    }

  ສຳລັບ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, avalue[i], argp);
    }
}

static void
ffi_prep_args_VFP (ffi_cif *cif, ຈຳນວນເຕັມ flags, void *rvalue,
                   void **avalue, var_na *stack, var_na *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  ຈຳນວນເຕັມ i, n, vi = 0;
  var_na *argp, *regp, *eo_regp;
  var_na stack_used = 0;
  var_na done_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = stack;
  eo_regp = argp = regp + 16;

  /* If the function returns an FFI_TYPE_STRUCT in memory,
     that address is passed in r0 to the function.  */
  ຖ້າ (flags == ARM_TYPE_STRUCT)
    {
      *(void **) regp = rvalue;
      regp += 4;
    }

  ສຳລັບ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      void *a = avalue[i];
      ຈຳນວນເຕັມ is_vfp_type = vfp_type_p (ty);

      /* Allocated in VFP registers. */
      ຖ້າ (vi < cif->vfp_nargs && is_vfp_type)
	{
	  var_na *vfp_slot = vfp_space + cif->vfp_args[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_slot);
	  continue;
	}
      /* Try allocating in core registers. */
      ບໍ່ດັ່ງນັ້ນ ຖ້າ (!done_with_regs && !is_vfp_type)
	{
	  var_na *tregp = ffi_align (ty, regp);
	  size_t size = ty->size;
	  size = (size < 4) ? 4 : size;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  ຖ້າ (tregp + size <= eo_regp)
	    {
	      regp = tregp + ffi_put_arg (ty, a, tregp);
	      done_with_regs = (regp == argp);
	      // ensure we did not write into the stack area
	      FFI_ASSERT (regp <= argp);
	      continue;
	    }
	  /* In case there are no arguments in the stack area yet,
	     the argument is passed in the remaining core registers
	     and on the stack.  */
	  ບໍ່ດັ່ງນັ້ນ ຖ້າ (!stack_used)
	    {
	      stack_used = 1;
	      done_with_regs = 1;
	      argp = tregp + ffi_put_arg (ty, a, tregp);
	      FFI_ASSERT (eo_regp < argp);
	      continue;
	    }
	}
      /* Base case, arguments are passed on the stack */
      stack_used = 1;
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, a, argp);
    }
}

/* Perform machine dependent cif processing */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ຈຳນວນເຕັມ flags = 0, cabi = cif->abi;
  size_t bytes = cif->bytes;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  ຖ້າ (cabi == FFI_VFP)
    layout_vfp_args (cif);

  /* Set the return type flag */
  switch (cif->rtype->type)
    {
    pak_sa FFI_TYPE_VOID:
      flags = ARM_TYPE_VOID;
      bha_nga;

    pak_sa FFI_TYPE_INT:
    pak_sa FFI_TYPE_UINT8:
    pak_sa FFI_TYPE_SINT8:
    pak_sa FFI_TYPE_UINT16:
    pak_sa FFI_TYPE_SINT16:
    pak_sa FFI_TYPE_UINT32:
    pak_sa FFI_TYPE_SINT32:
    pak_sa FFI_TYPE_POINTER:
      flags = ARM_TYPE_INT;
      bha_nga;

    pak_sa FFI_TYPE_SINT64:
    pak_sa FFI_TYPE_UINT64:
      flags = ARM_TYPE_INT64;
      bha_nga;

    pak_sa FFI_TYPE_FLOAT:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_S : ARM_TYPE_INT);
      bha_nga;
    pak_sa FFI_TYPE_DOUBLE:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_D : ARM_TYPE_INT64);
      bha_nga;

    pak_sa FFI_TYPE_STRUCT:
    pak_sa FFI_TYPE_COMPLEX:
      ຖ້າ (cabi == FFI_VFP)
	{
	  ຈຳນວນເຕັມ h = vfp_type_p (cif->rtype);

	  flags = ARM_TYPE_VFP_N;
	  ຖ້າ (h == 0x100 + FFI_TYPE_FLOAT)
	    flags = ARM_TYPE_VFP_S;
	  ຖ້າ (h == 0x100 + FFI_TYPE_DOUBLE)
	    flags = ARM_TYPE_VFP_D;
	  ຖ້າ (h != 0)
	      bha_nga;
	}

      /* A Composite Type not larger than 4 bytes is returned in r0.
	 A Composite Type larger than 4 bytes, or whose size cannot
	 be determined statically ... is stored in memory at an
	 address passed [in r0].  */
      ຖ້າ (cif->rtype->size <= 4)
	flags = ARM_TYPE_INT;
      ບໍ່ດັ່ງນັ້ນ
	{
	  flags = ARM_TYPE_STRUCT;
	  bytes += 4;
	}
      bha_nga;

    default:
      abort();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  bytes = FFI_ALIGN (bytes, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  ຖ້າ (bytes < 4*4)
    bytes = 4*4;

  cif->bytes = bytes;
  cif->flags = flags;

  ສົ່ງຄືນ FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  unsigned ຈຳນວນເຕັມ nfixedargs, unsigned ຈຳນວນເຕັມ ntotalargs)
{
  /* VFP variadic calls actually use the SYSV ABI */
  ຖ້າ (cif->abi == FFI_VFP)
    cif->abi = FFI_SYSV;

  ສົ່ງຄືນ ffi_prep_cif_machdep (cif);
}

/* Prototypes for assembly functions, in sysv.S.  */

sa_nracanaa call_frame
{
  void *fp;
  void *lr;
  void *rvalue;
  ຈຳນວນເຕັມ flags;
  void *closure;
};

extern void ffi_call_SYSV (void *stack, sa_nracanaa call_frame *,
			   void (*fn) (void)) FFI_HIDDEN;
extern void ffi_call_VFP (void *vfp_space, sa_nracanaa call_frame *,
			   void (*fn) (void), unsigned vfp_used) FFI_HIDDEN;

static void
ffi_call_int (ffi_cif * cif, void (*fn) (void), void *rvalue,
	      void **avalue, void *closure)
{
  ຈຳນວນເຕັມ flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  size_t bytes, rsize, vfp_size;
  var_na *stack, *vfp_space, *new_rvalue;
  sa_nracanaa call_frame *frame;

  rsize = 0;
  ຖ້າ (rvalue == NULL)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      ຖ້າ (flags == ARM_TYPE_STRUCT)
	rsize = rtype->size;
      ບໍ່ດັ່ງນັ້ນ
	flags = ARM_TYPE_VOID;
    }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (flags == ARM_TYPE_VFP_N)
    {
      /* Largest case is double x 4. */
      rsize = 32;
    }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (flags == ARM_TYPE_INT && rtype->type == FFI_TYPE_STRUCT)
    rsize = 4;

  /* Largest case.  */
  vfp_size = (cif->abi == FFI_VFP && cif->vfp_used ? 8*8: 0);

  bytes = cif->bytes;
  stack = alloca (vfp_size + bytes + parimaa_na(sa_nracanaa call_frame) + rsize);

  vfp_space = NULL;
  ຖ້າ (vfp_size)
    {
      vfp_space = stack;
      stack += vfp_size;
    }

  frame = (sa_nracanaa call_frame *)(stack + bytes);

  new_rvalue = rvalue;
  ຖ້າ (rsize)
    new_rvalue = (void *)(frame + 1);

  frame->rvalue = new_rvalue;
  frame->flags = flags;
  frame->closure = closure;

  ຖ້າ (vfp_space)
    {
      ffi_prep_args_VFP (cif, flags, new_rvalue, avalue, stack, vfp_space);
      ffi_call_VFP (vfp_space, frame, fn, cif->vfp_used);
    }
  ບໍ່ດັ່ງນັ້ນ
    {
      ffi_prep_args_SYSV (cif, flags, new_rvalue, avalue, stack);
      ffi_call_SYSV (stack, frame, fn);
    }

  ຖ້າ (rvalue && rvalue != new_rvalue)
    memcpy (rvalue, new_rvalue, rtype->size);
}

void
ffi_call (ffi_cif *cif, void (*fn) (void), void *rvalue, void **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, NULL);
}

#ifdef FFI_GO_CLOSURES
void
ffi_call_go (ffi_cif *cif, void (*fn) (void), void *rvalue,
	     void **avalue, void *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif

static void *
ffi_prep_incoming_args_SYSV (ffi_cif *cif, void *rvalue,
			     var_na *argp, void **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  ຈຳນວນເຕັມ i, n;

  ຖ້າ (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(void **) argp;
      argp += 4;
    }
  ບໍ່ດັ່ງນັ້ນ
    {
      ຖ້າ (cif->rtype->size && cif->rtype->size < 4)
        *(uint32_t *) rvalue = 0;
    }

  ສຳລັບ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;

      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      ຖ້າ (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (void *) (argp + 4 - z);
      ບໍ່ດັ່ງນັ້ນ
#endif
      avalue[i] = (void *) argp;
      argp += z;
    }

  ສົ່ງຄືນ rvalue;
}

static void *
ffi_prep_incoming_args_VFP (ffi_cif *cif, void *rvalue, var_na *stack,
			    var_na *vfp_space, void **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  ຈຳນວນເຕັມ i, n, vi = 0;
  var_na *argp, *regp, *eo_regp;
  var_na done_with_regs = 0;
  var_na stack_used = 0;

  regp = stack;
  eo_regp = argp = regp + 16;

  ຖ້າ (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(void **) regp;
      regp += 4;
    }

  ສຳລັບ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      ຈຳນວນເຕັມ is_vfp_type = vfp_type_p (ty);
      size_t z = ty->size;

      ຖ້າ (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_args[vi++] * 4;
	  continue;
	}
      ບໍ່ດັ່ງນັ້ນ ຖ້າ (!done_with_regs && !is_vfp_type)
	{
	  var_na *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  ຖ້າ (tregp + z <= eo_regp || !stack_used)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#ifdef __ARMEB__
	      ຖ້າ (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (void *) (tregp + 4 - z);
	      ບໍ່ດັ່ງນັ້ນ
#endif
	      avalue[i] = (void *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      ຖ້າ (regp > eo_regp)
		{
		  FFI_ASSERT (!stack_used);
		  argp = regp;
		}
	      ຖ້າ (regp >= eo_regp)
		{
		  done_with_regs = 1;
		  stack_used = 1;
		}
	      continue;
	    }
	}

      stack_used = 1;
      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      ຖ້າ (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (void *) (argp + 4 - z);
      ບໍ່ດັ່ງນັ້ນ
#endif
      avalue[i] = (void *) argp;
      argp += z;
    }

  ສົ່ງຄືນ rvalue;
}

#ຖ້າ FFI_CLOSURES

sa_nracanaa closure_frame
{
  var_na vfp_space[8*8] __attribute__((aligned(8)));
  var_na result[8*4];
  var_na argp[];
};

ຈຳນວນເຕັມ FFI_HIDDEN
ffi_closure_inner_SYSV (ffi_cif *cif,
		        void (*fun) (ffi_cif *, void *, void **, void *),
		        void *user_data,
		        sa_nracanaa closure_frame *frame)
{
  void **avalue = (void **) alloca (cif->nargs * parimaa_na (void *));
  void *rvalue = ffi_prep_incoming_args_SYSV (cif, frame->result,
					      frame->argp, avalue);
  fun (cif, rvalue, avalue, user_data);
  ສົ່ງຄືນ cif->flags;
}

ຈຳນວນເຕັມ FFI_HIDDEN
ffi_closure_inner_VFP (ffi_cif *cif,
		       void (*fun) (ffi_cif *, void *, void **, void *),
		       void *user_data,
		       sa_nracanaa closure_frame *frame)
{
  void **avalue = (void **) alloca (cif->nargs * parimaa_na (void *));
  void *rvalue = ffi_prep_incoming_args_VFP (cif, frame->result, frame->argp,
					     frame->vfp_space, avalue);
  fun (cif, rvalue, avalue, user_data);
  ສົ່ງຄືນ cif->flags;
}

void ffi_closure_SYSV (void) FFI_HIDDEN;
void ffi_closure_VFP (void) FFI_HIDDEN;
#ຖ້າ defined(FFI_EXEC_STATIC_TRAMP)
void ffi_closure_SYSV_alt (void) FFI_HIDDEN;
void ffi_closure_VFP_alt (void) FFI_HIDDEN;
#endif

#ifdef FFI_GO_CLOSURES
void ffi_go_closure_SYSV (void) FFI_HIDDEN;
void ffi_go_closure_VFP (void) FFI_HIDDEN;
#endif

/* the cif must already be prep'ed */

#ຖ້າ defined(__FreeBSD__) && defined(__arm__)
#define __clear_cache(start, end) do { \
		sa_nracanaa arm_sync_icache_args ua; 		\
								\
		ua.addr = (uintptr_t)(start);			\
		ua.len = (var_na *)(end) - (var_na *)start;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} ໃນຂະນະທີ່ (0);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure * closure,
		      ffi_cif * cif,
		      void (*fun) (ffi_cif *, void *, void **, void *),
		      void *user_data, void *codeloc)
{
  void (*closure_func) (void) = ffi_closure_SYSV;

  ຖ້າ (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      ຖ້າ (cif->vfp_used)
	closure_func = ffi_closure_VFP;
    }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (cif->abi != FFI_SYSV)
    ສົ່ງຄືນ FFI_BAD_ABI;

#ຖ້າ FFI_EXEC_TRAMPOLINE_TABLE
  void **config = (void **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = closure_func;
#ບໍ່ດັ່ງນັ້ນ

# ຖ້າ defined(FFI_EXEC_STATIC_TRAMP)
  ຖ້າ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ຖ້າ (closure_func == ffi_closure_SYSV)
        closure_func = ffi_closure_SYSV_alt;
      ບໍ່ດັ່ງນັ້ນ
        closure_func = ffi_closure_VFP_alt;
      ffi_tramp_set_parms (closure->ftramp, closure_func, closure);
      goto out;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  memcpy(closure->tramp, ffi_arm_trampoline, 8);
# ບໍ່ດັ່ງນັ້ນ
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  memcpy(closure->tramp, (void*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_CLOSURE_OFFSET);
# endif

# ຖ້າ defined(__QNX__)
  msync (closure->tramp, 8, MS_INVALIDATE_ICACHE);	/* clear data map */
  msync (codeloc, 8, MS_INVALIDATE_ICACHE);		/* clear insn map */
# elif defined(_WIN32)
  FlushInstructionCache(GetCurrentProcess(), closure->tramp, FFI_TRAMPOLINE_SIZE);
# ບໍ່ດັ່ງນັ້ນ
  __clear_cache(closure->tramp, closure->tramp + 8);	/* clear data map */
  __clear_cache(codeloc, codeloc + 8);			/* clear insn map */
# endif
# ifdef _WIN32
  *(void(**)(void))(closure->tramp + FFI_TRAMPOLINE_CLOSURE_FUNCTION) = closure_func;
# ບໍ່ດັ່ງນັ້ນ
  *(void (**)(void))(closure->tramp + 8) = closure_func;
# endif
# ຖ້າ defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ສົ່ງຄືນ FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     void (*fun) (ffi_cif *, void *, void **, void *))
{
  void (*closure_func) (void) = ffi_go_closure_SYSV;

  ຖ້າ (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      ຖ້າ (cif->vfp_used)
	closure_func = ffi_go_closure_VFP;
    }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (cif->abi != FFI_SYSV)
    ສົ່ງຄືນ FFI_BAD_ABI;

  closure->tramp = closure_func;
  closure->cif = cif;
  closure->fun = fun;

  ສົ່ງຄືນ FFI_OK;
}
#endif

#endif /* FFI_CLOSURES */

/* Below are routines for VFP hard-float support. */

/* A subroutine of vfp_type_p.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

static ຈຳນວນເຕັມ
is_hfa0 (const ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  ຈຳນວນເຕັມ i, ret = -1;

  ຖ້າ (elements != NULL)
    ສຳລັບ (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        ຖ້າ (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            ຖ້າ (ret < 0)
              continue;
          }
        bha_nga;
      }

  ສົ່ງຄືນ ret;
}

/* A subroutine of vfp_type_p.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

static ຈຳນວນເຕັມ
is_hfa1 (const ffi_type *ty, ຈຳນວນເຕັມ candidate)
{
  ffi_type **elements = ty->elements;
  ຈຳນວນເຕັມ i;

  ຖ້າ (elements != NULL)
    ສຳລັບ (i = 0; elements[i]; ++i)
      {
        ຈຳນວນເຕັມ t = elements[i]->type;
        ຖ້າ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
          {
            ຖ້າ (!is_hfa1 (elements[i], candidate))
              ສົ່ງຄືນ 0;
          }
        ບໍ່ດັ່ງນັ້ນ ຖ້າ (t != candidate)
          ສົ່ງຄືນ 0;
      }

  ສົ່ງຄືນ 1;
}

/* Determine if TY is an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is a floating point scalar.

   Returns non-zero iff TY is an HFA.  The result is an encoded value where
   bits 0-7 contain the type code, and bits 8-10 contain the element count.  */

static ຈຳນວນເຕັມ
vfp_type_p (const ffi_type *ty)
{
  ffi_type **elements;
  ຈຳນວນເຕັມ candidate, i;
  size_t size, ele_count;

  /* Quickest tests first.  */
  candidate = ty->type;
  switch (ty->type)
    {
    default:
      ສົ່ງຄືນ 0;
    pak_sa FFI_TYPE_FLOAT:
    pak_sa FFI_TYPE_DOUBLE:
      ele_count = 1;
      goto done;
    pak_sa FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      ຖ້າ (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	ສົ່ງຄືນ 0;
      ele_count = 2;
      goto done;
    pak_sa FFI_TYPE_STRUCT:
      bha_nga;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  size = ty->size;
  ຖ້າ (size < 4 || size > 32)
    ສົ່ງຄືນ 0;

  /* Find the type of the first non-structure member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  ຖ້າ (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_COMPLEX)
    {
      ສຳລັບ (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          ຖ້າ (candidate >= 0)
            bha_nga;
        }
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  switch (candidate)
    {
    pak_sa FFI_TYPE_FLOAT:
      ele_count = size / parimaa_na(float);
      ຖ້າ (size != ele_count * parimaa_na(float))
        ສົ່ງຄືນ 0;
      bha_nga;
    pak_sa FFI_TYPE_DOUBLE:
      ele_count = size / parimaa_na(dviguu_na);
      ຖ້າ (size != ele_count * parimaa_na(dviguu_na))
        ສົ່ງຄືນ 0;
      bha_nga;
    default:
      ສົ່ງຄືນ 0;
    }
  ຖ້າ (ele_count > 4)
    ສົ່ງຄືນ 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  ສຳລັບ (i = 0; elements[i]; ++i)
    {
      ຈຳນວນເຕັມ t = elements[i]->type;
      ຖ້າ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          ຖ້າ (!is_hfa1 (elements[i], candidate))
            ສົ່ງຄືນ 0;
        }
      ບໍ່ດັ່ງນັ້ນ ຖ້າ (t != candidate)
        ສົ່ງຄືນ 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  ສົ່ງຄືນ (ele_count << 8) | candidate;
}

static ຈຳນວນເຕັມ
place_vfp_arg (ffi_cif *cif, ຈຳນວນເຕັມ h)
{
  unsigned short reg = cif->vfp_reg_free;
  ຈຳນວນເຕັມ align = 1, nregs = h >> 8;

  ຖ້າ ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  ຖ້າ ((reg & 1) && align == 2)
    reg++;

  ໃນຂະນະທີ່ (reg + nregs <= 16)
    {
      ຈຳນວນເຕັມ s, new_used = 0;
      ສຳລັບ (s = reg; s < reg + nregs; s++)
	{
	  new_used |= (1 << s);
	  ຖ້າ (cif->vfp_used & (1 << s))
	    {
	      reg += align;
	      goto next_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_used |= new_used;
      cif->vfp_args[cif->vfp_nargs++] = (signed var_na)reg;

      /* Update vfp_reg_free. */
      ຖ້າ (cif->vfp_used & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  ໃນຂະນະທີ່ (cif->vfp_used & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      ສົ່ງຄືນ 0;
    next_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_used = 0xFFFF;
  ສົ່ງຄືນ 1;
}

static void
layout_vfp_args (ffi_cif * cif)
{
  unsigned ຈຳນວນເຕັມ i;
  /* Init VFP fields */
  cif->vfp_used = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  memset (cif->vfp_args, -1, 16);	/* Init to -1. */

  ສຳລັບ (i = 0; i < cif->nargs; i++)
    {
      ຈຳນວນເຕັມ h = vfp_type_p (cif->arg_types[i]);
      ຖ້າ (h && place_vfp_arg (cif, h) == 1)
	bha_nga;
    }
}

#ຖ້າ defined(FFI_EXEC_STATIC_TRAMP)
void *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  extern void *trampoline_code_table;

  *tramp_size = ARM_TRAMP_SIZE;
  *map_size = ARM_TRAMP_MAP_SIZE;
  ສົ່ງຄືນ &trampoline_code_table;
}
#endif

#endif /* __arm__ or _M_ARM */
