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

#만약 defined(__arm__) || defined(_M_ARM)
#include <fficonfig.h>
#include <ffi.h>
#include <ffi_common.h>
#include <stdint.h>
#include <stdlib.h>
#include <tramp.h>
#include "internal.h"

#만약 defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#만약 FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#include <mach/machine/vm_param.h>
#endif

#아니면
#ifndef _WIN32
extern unsigned 정수 ffi_arm_trampoline[2] FFI_HIDDEN;
#아니면
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
extern unsigned char ffi_arm_trampoline[12] FFI_HIDDEN;
#endif
#endif

#만약 defined(__FreeBSD__) && defined(__arm__)
#include <sys/types.h>
#include <machine/sysarch.h>
#endif

#만약 defined(__QNX__)
#include <sys/mman.h>
#endif

/* Forward declares. */
static 정수 vfp_type_p (const ffi_type *);
static void layout_vfp_args (ffi_cif *);

static void *
ffi_align (ffi_type *ty, void *p)
{
  /* Align if necessary */
  size_t alignment;
#ifdef _WIN32_WCE
  alignment = 4;
#아니면
  alignment = ty->alignment;
  만약 (alignment < 4)
    alignment = 4;
#endif
  반환 (void *) FFI_ALIGN (p, alignment);
}

static size_t
ffi_put_arg (ffi_type *ty, void *src, void *dst)
{
  size_t z = ty->size;

  switch (ty->type)
    {
    case FFI_TYPE_SINT8:
      *(UINT32 *)dst = *(SINT8 *)src;
      break;
    case FFI_TYPE_UINT8:
      *(UINT32 *)dst = *(UINT8 *)src;
      break;
    case FFI_TYPE_SINT16:
      *(UINT32 *)dst = *(SINT16 *)src;
      break;
    case FFI_TYPE_UINT16:
      *(UINT32 *)dst = *(UINT16 *)src;
      break;

    case FFI_TYPE_INT:
    case FFI_TYPE_SINT32:
    case FFI_TYPE_UINT32:
    case FFI_TYPE_POINTER:
#ifndef _WIN32
    case FFI_TYPE_FLOAT:
#endif
      *(UINT32 *)dst = *(UINT32 *)src;
      break;

#ifdef _WIN32
    // casting a float* to a UINT32* doesn't work on Windows
    case FFI_TYPE_FLOAT:
        *(uintptr_t *)dst = 0;
        *(float *)dst = *(float *)src;
        break;
#endif

    case FFI_TYPE_SINT64:
    case FFI_TYPE_UINT64:
    case FFI_TYPE_DOUBLE:
      *(UINT64 *)dst = *(UINT64 *)src;
      break;

    case FFI_TYPE_STRUCT:
    case FFI_TYPE_COMPLEX:
      memcpy (dst, src, z);
      break;

    default:
      abort();
    }

  반환 FFI_ALIGN (z, 4);
}

/* ffi_prep_args is called once stack space has been allocated
   for the function's arguments.

   The vfp_space parameter is the load area for VFP regs, the return
   value is cif->vfp_used (word bitset of VFP regs used for passing
   arguments). These are only used for the VFP hard-float ABI.
*/
static void
ffi_prep_args_SYSV (ffi_cif *cif, 정수 flags, void *rvalue,
		    void **avalue, char *argp)
{
  ffi_type **arg_types = cif->arg_types;
  정수 i, n;

  만약 (flags == ARM_TYPE_STRUCT)
    {
      *(void **) argp = rvalue;
      argp += 4;
    }

  위해 (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, avalue[i], argp);
    }
}

static void
ffi_prep_args_VFP (ffi_cif *cif, 정수 flags, void *rvalue,
                   void **avalue, char *stack, char *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  정수 i, n, vi = 0;
  char *argp, *regp, *eo_regp;
  char stack_used = 0;
  char done_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = stack;
  eo_regp = argp = regp + 16;

  /* If the function returns an FFI_TYPE_STRUCT in memory,
     that address is passed in r0 to the function.  */
  만약 (flags == ARM_TYPE_STRUCT)
    {
      *(void **) regp = rvalue;
      regp += 4;
    }

  위해 (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      void *a = avalue[i];
      정수 is_vfp_type = vfp_type_p (ty);

      /* Allocated in VFP registers. */
      만약 (vi < cif->vfp_nargs && is_vfp_type)
	{
	  char *vfp_slot = vfp_space + cif->vfp_args[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_slot);
	  continue;
	}
      /* Try allocating in core registers. */
      아니면 만약 (!done_with_regs && !is_vfp_type)
	{
	  char *tregp = ffi_align (ty, regp);
	  size_t size = ty->size;
	  size = (size < 4) ? 4 : size;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  만약 (tregp + size <= eo_regp)
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
	  아니면 만약 (!stack_used)
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
  정수 flags = 0, cabi = cif->abi;
  size_t bytes = cif->bytes;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  만약 (cabi == FFI_VFP)
    layout_vfp_args (cif);

  /* Set the return type flag */
  switch (cif->rtype->type)
    {
    case FFI_TYPE_VOID:
      flags = ARM_TYPE_VOID;
      break;

    case FFI_TYPE_INT:
    case FFI_TYPE_UINT8:
    case FFI_TYPE_SINT8:
    case FFI_TYPE_UINT16:
    case FFI_TYPE_SINT16:
    case FFI_TYPE_UINT32:
    case FFI_TYPE_SINT32:
    case FFI_TYPE_POINTER:
      flags = ARM_TYPE_INT;
      break;

    case FFI_TYPE_SINT64:
    case FFI_TYPE_UINT64:
      flags = ARM_TYPE_INT64;
      break;

    case FFI_TYPE_FLOAT:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_S : ARM_TYPE_INT);
      break;
    case FFI_TYPE_DOUBLE:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_D : ARM_TYPE_INT64);
      break;

    case FFI_TYPE_STRUCT:
    case FFI_TYPE_COMPLEX:
      만약 (cabi == FFI_VFP)
	{
	  정수 h = vfp_type_p (cif->rtype);

	  flags = ARM_TYPE_VFP_N;
	  만약 (h == 0x100 + FFI_TYPE_FLOAT)
	    flags = ARM_TYPE_VFP_S;
	  만약 (h == 0x100 + FFI_TYPE_DOUBLE)
	    flags = ARM_TYPE_VFP_D;
	  만약 (h != 0)
	      break;
	}

      /* A Composite Type not larger than 4 bytes is returned in r0.
	 A Composite Type larger than 4 bytes, or whose size cannot
	 be determined statically ... is stored in memory at an
	 address passed [in r0].  */
      만약 (cif->rtype->size <= 4)
	flags = ARM_TYPE_INT;
      아니면
	{
	  flags = ARM_TYPE_STRUCT;
	  bytes += 4;
	}
      break;

    default:
      abort();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  bytes = FFI_ALIGN (bytes, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  만약 (bytes < 4*4)
    bytes = 4*4;

  cif->bytes = bytes;
  cif->flags = flags;

  반환 FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  unsigned 정수 nfixedargs, unsigned 정수 ntotalargs)
{
  /* VFP variadic calls actually use the SYSV ABI */
  만약 (cif->abi == FFI_VFP)
    cif->abi = FFI_SYSV;

  반환 ffi_prep_cif_machdep (cif);
}

/* Prototypes for assembly functions, in sysv.S.  */

struct call_frame
{
  void *fp;
  void *lr;
  void *rvalue;
  정수 flags;
  void *closure;
};

extern void ffi_call_SYSV (void *stack, struct call_frame *,
			   void (*fn) (void)) FFI_HIDDEN;
extern void ffi_call_VFP (void *vfp_space, struct call_frame *,
			   void (*fn) (void), unsigned vfp_used) FFI_HIDDEN;

static void
ffi_call_int (ffi_cif * cif, void (*fn) (void), void *rvalue,
	      void **avalue, void *closure)
{
  정수 flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  size_t bytes, rsize, vfp_size;
  char *stack, *vfp_space, *new_rvalue;
  struct call_frame *frame;

  rsize = 0;
  만약 (rvalue == NULL)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      만약 (flags == ARM_TYPE_STRUCT)
	rsize = rtype->size;
      아니면
	flags = ARM_TYPE_VOID;
    }
  아니면 만약 (flags == ARM_TYPE_VFP_N)
    {
      /* Largest case is double x 4. */
      rsize = 32;
    }
  아니면 만약 (flags == ARM_TYPE_INT && rtype->type == FFI_TYPE_STRUCT)
    rsize = 4;

  /* Largest case.  */
  vfp_size = (cif->abi == FFI_VFP && cif->vfp_used ? 8*8: 0);

  bytes = cif->bytes;
  stack = alloca (vfp_size + bytes + sizeof(struct call_frame) + rsize);

  vfp_space = NULL;
  만약 (vfp_size)
    {
      vfp_space = stack;
      stack += vfp_size;
    }

  frame = (struct call_frame *)(stack + bytes);

  new_rvalue = rvalue;
  만약 (rsize)
    new_rvalue = (void *)(frame + 1);

  frame->rvalue = new_rvalue;
  frame->flags = flags;
  frame->closure = closure;

  만약 (vfp_space)
    {
      ffi_prep_args_VFP (cif, flags, new_rvalue, avalue, stack, vfp_space);
      ffi_call_VFP (vfp_space, frame, fn, cif->vfp_used);
    }
  아니면
    {
      ffi_prep_args_SYSV (cif, flags, new_rvalue, avalue, stack);
      ffi_call_SYSV (stack, frame, fn);
    }

  만약 (rvalue && rvalue != new_rvalue)
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
			     char *argp, void **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  정수 i, n;

  만약 (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(void **) argp;
      argp += 4;
    }
  아니면
    {
      만약 (cif->rtype->size && cif->rtype->size < 4)
        *(uint32_t *) rvalue = 0;
    }

  위해 (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;

      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      만약 (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (void *) (argp + 4 - z);
      아니면
#endif
      avalue[i] = (void *) argp;
      argp += z;
    }

  반환 rvalue;
}

static void *
ffi_prep_incoming_args_VFP (ffi_cif *cif, void *rvalue, char *stack,
			    char *vfp_space, void **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  정수 i, n, vi = 0;
  char *argp, *regp, *eo_regp;
  char done_with_regs = 0;
  char stack_used = 0;

  regp = stack;
  eo_regp = argp = regp + 16;

  만약 (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(void **) regp;
      regp += 4;
    }

  위해 (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      정수 is_vfp_type = vfp_type_p (ty);
      size_t z = ty->size;

      만약 (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_args[vi++] * 4;
	  continue;
	}
      아니면 만약 (!done_with_regs && !is_vfp_type)
	{
	  char *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  만약 (tregp + z <= eo_regp || !stack_used)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#ifdef __ARMEB__
	      만약 (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (void *) (tregp + 4 - z);
	      아니면
#endif
	      avalue[i] = (void *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      만약 (regp > eo_regp)
		{
		  FFI_ASSERT (!stack_used);
		  argp = regp;
		}
	      만약 (regp >= eo_regp)
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
      만약 (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (void *) (argp + 4 - z);
      아니면
#endif
      avalue[i] = (void *) argp;
      argp += z;
    }

  반환 rvalue;
}

#만약 FFI_CLOSURES

struct closure_frame
{
  char vfp_space[8*8] __attribute__((aligned(8)));
  char result[8*4];
  char argp[];
};

정수 FFI_HIDDEN
ffi_closure_inner_SYSV (ffi_cif *cif,
		        void (*fun) (ffi_cif *, void *, void **, void *),
		        void *user_data,
		        struct closure_frame *frame)
{
  void **avalue = (void **) alloca (cif->nargs * sizeof (void *));
  void *rvalue = ffi_prep_incoming_args_SYSV (cif, frame->result,
					      frame->argp, avalue);
  fun (cif, rvalue, avalue, user_data);
  반환 cif->flags;
}

정수 FFI_HIDDEN
ffi_closure_inner_VFP (ffi_cif *cif,
		       void (*fun) (ffi_cif *, void *, void **, void *),
		       void *user_data,
		       struct closure_frame *frame)
{
  void **avalue = (void **) alloca (cif->nargs * sizeof (void *));
  void *rvalue = ffi_prep_incoming_args_VFP (cif, frame->result, frame->argp,
					     frame->vfp_space, avalue);
  fun (cif, rvalue, avalue, user_data);
  반환 cif->flags;
}

void ffi_closure_SYSV (void) FFI_HIDDEN;
void ffi_closure_VFP (void) FFI_HIDDEN;
#만약 defined(FFI_EXEC_STATIC_TRAMP)
void ffi_closure_SYSV_alt (void) FFI_HIDDEN;
void ffi_closure_VFP_alt (void) FFI_HIDDEN;
#endif

#ifdef FFI_GO_CLOSURES
void ffi_go_closure_SYSV (void) FFI_HIDDEN;
void ffi_go_closure_VFP (void) FFI_HIDDEN;
#endif

/* the cif must already be prep'ed */

#만약 defined(__FreeBSD__) && defined(__arm__)
#define __clear_cache(start, end) do { \
		struct arm_sync_icache_args ua; 		\
								\
		ua.addr = (uintptr_t)(start);			\
		ua.len = (char *)(end) - (char *)start;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} 동안 (0);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure * closure,
		      ffi_cif * cif,
		      void (*fun) (ffi_cif *, void *, void **, void *),
		      void *user_data, void *codeloc)
{
  void (*closure_func) (void) = ffi_closure_SYSV;

  만약 (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      만약 (cif->vfp_used)
	closure_func = ffi_closure_VFP;
    }
  아니면 만약 (cif->abi != FFI_SYSV)
    반환 FFI_BAD_ABI;

#만약 FFI_EXEC_TRAMPOLINE_TABLE
  void **config = (void **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = closure_func;
#아니면

# 만약 defined(FFI_EXEC_STATIC_TRAMP)
  만약 (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      만약 (closure_func == ffi_closure_SYSV)
        closure_func = ffi_closure_SYSV_alt;
      아니면
        closure_func = ffi_closure_VFP_alt;
      ffi_tramp_set_parms (closure->ftramp, closure_func, closure);
      goto out;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  memcpy(closure->tramp, ffi_arm_trampoline, 8);
# 아니면
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  memcpy(closure->tramp, (void*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_CLOSURE_OFFSET);
# endif

# 만약 defined(__QNX__)
  msync (closure->tramp, 8, MS_INVALIDATE_ICACHE);	/* clear data map */
  msync (codeloc, 8, MS_INVALIDATE_ICACHE);		/* clear insn map */
# elif defined(_WIN32)
  FlushInstructionCache(GetCurrentProcess(), closure->tramp, FFI_TRAMPOLINE_SIZE);
# 아니면
  __clear_cache(closure->tramp, closure->tramp + 8);	/* clear data map */
  __clear_cache(codeloc, codeloc + 8);			/* clear insn map */
# endif
# ifdef _WIN32
  *(void(**)(void))(closure->tramp + FFI_TRAMPOLINE_CLOSURE_FUNCTION) = closure_func;
# 아니면
  *(void (**)(void))(closure->tramp + 8) = closure_func;
# endif
# 만약 defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  반환 FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     void (*fun) (ffi_cif *, void *, void **, void *))
{
  void (*closure_func) (void) = ffi_go_closure_SYSV;

  만약 (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      만약 (cif->vfp_used)
	closure_func = ffi_go_closure_VFP;
    }
  아니면 만약 (cif->abi != FFI_SYSV)
    반환 FFI_BAD_ABI;

  closure->tramp = closure_func;
  closure->cif = cif;
  closure->fun = fun;

  반환 FFI_OK;
}
#endif

#endif /* FFI_CLOSURES */

/* Below are routines for VFP hard-float support. */

/* A subroutine of vfp_type_p.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

static 정수
is_hfa0 (const ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  정수 i, ret = -1;

  만약 (elements != NULL)
    위해 (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        만약 (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            만약 (ret < 0)
              continue;
          }
        break;
      }

  반환 ret;
}

/* A subroutine of vfp_type_p.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

static 정수
is_hfa1 (const ffi_type *ty, 정수 candidate)
{
  ffi_type **elements = ty->elements;
  정수 i;

  만약 (elements != NULL)
    위해 (i = 0; elements[i]; ++i)
      {
        정수 t = elements[i]->type;
        만약 (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
          {
            만약 (!is_hfa1 (elements[i], candidate))
              반환 0;
          }
        아니면 만약 (t != candidate)
          반환 0;
      }

  반환 1;
}

/* Determine if TY is an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is a floating point scalar.

   Returns non-zero iff TY is an HFA.  The result is an encoded value where
   bits 0-7 contain the type code, and bits 8-10 contain the element count.  */

static 정수
vfp_type_p (const ffi_type *ty)
{
  ffi_type **elements;
  정수 candidate, i;
  size_t size, ele_count;

  /* Quickest tests first.  */
  candidate = ty->type;
  switch (ty->type)
    {
    default:
      반환 0;
    case FFI_TYPE_FLOAT:
    case FFI_TYPE_DOUBLE:
      ele_count = 1;
      goto done;
    case FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      만약 (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	반환 0;
      ele_count = 2;
      goto done;
    case FFI_TYPE_STRUCT:
      break;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  size = ty->size;
  만약 (size < 4 || size > 32)
    반환 0;

  /* Find the type of the first non-structure member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  만약 (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_COMPLEX)
    {
      위해 (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          만약 (candidate >= 0)
            break;
        }
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  switch (candidate)
    {
    case FFI_TYPE_FLOAT:
      ele_count = size / sizeof(float);
      만약 (size != ele_count * sizeof(float))
        반환 0;
      break;
    case FFI_TYPE_DOUBLE:
      ele_count = size / sizeof(double);
      만약 (size != ele_count * sizeof(double))
        반환 0;
      break;
    default:
      반환 0;
    }
  만약 (ele_count > 4)
    반환 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  위해 (i = 0; elements[i]; ++i)
    {
      정수 t = elements[i]->type;
      만약 (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          만약 (!is_hfa1 (elements[i], candidate))
            반환 0;
        }
      아니면 만약 (t != candidate)
        반환 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  반환 (ele_count << 8) | candidate;
}

static 정수
place_vfp_arg (ffi_cif *cif, 정수 h)
{
  unsigned short reg = cif->vfp_reg_free;
  정수 align = 1, nregs = h >> 8;

  만약 ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  만약 ((reg & 1) && align == 2)
    reg++;

  동안 (reg + nregs <= 16)
    {
      정수 s, new_used = 0;
      위해 (s = reg; s < reg + nregs; s++)
	{
	  new_used |= (1 << s);
	  만약 (cif->vfp_used & (1 << s))
	    {
	      reg += align;
	      goto next_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_used |= new_used;
      cif->vfp_args[cif->vfp_nargs++] = (signed char)reg;

      /* Update vfp_reg_free. */
      만약 (cif->vfp_used & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  동안 (cif->vfp_used & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      반환 0;
    next_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_used = 0xFFFF;
  반환 1;
}

static void
layout_vfp_args (ffi_cif * cif)
{
  unsigned 정수 i;
  /* Init VFP fields */
  cif->vfp_used = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  memset (cif->vfp_args, -1, 16);	/* Init to -1. */

  위해 (i = 0; i < cif->nargs; i++)
    {
      정수 h = vfp_type_p (cif->arg_types[i]);
      만약 (h && place_vfp_arg (cif, h) == 1)
	break;
    }
}

#만약 defined(FFI_EXEC_STATIC_TRAMP)
void *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  extern void *trampoline_code_table;

  *tramp_size = ARM_TRAMP_SIZE;
  *map_size = ARM_TRAMP_MAP_SIZE;
  반환 &trampoline_code_table;
}
#endif

#endif /* __arm__ or _M_ARM */
