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

#اگر defined(__arm__) || defined(_M_ARM)
#شامل <fficonfig.h>
#شامل <ffi.h>
#شامل <ffi_common.h>
#شامل <stdint.h>
#شامل <stdlib.h>
#شامل <tramp.h>
#شامل "internal.h"

#اگر defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#شامل <windows.h>
#endif

#اگر FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#شامل <mach/machine/vm_param.h>
#endif

#وگرنه
#ifndef _WIN32
بیرونی unsigned صحیح ffi_arm_trampoline[2] FFI_HIDDEN;
#وگرنه
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
بیرونی unsigned نویسه ffi_arm_trampoline[12] FFI_HIDDEN;
#endif
#endif

#اگر defined(__FreeBSD__) && defined(__arm__)
#شامل <sys/types.h>
#شامل <machine/sysarch.h>
#endif

#اگر defined(__QNX__)
#شامل <sys/mman.h>
#endif

/* Forward declares. */
ایستا صحیح vfp_type_p (ثابت ffi_type *);
ایستا تهی layout_vfp_args (ffi_cif *);

ایستا تهی *
ffi_align (ffi_type *ty, تهی *p)
{
  /* Align if necessary */
  size_t alignment;
#ifdef _WIN32_WCE
  alignment = 4;
#وگرنه
  alignment = ty->alignment;
  اگر (alignment < 4)
    alignment = 4;
#endif
  بازگشت (تهی *) FFI_ALIGN (p, alignment);
}

ایستا size_t
ffi_put_arg (ffi_type *ty, تهی *src, تهی *dst)
{
  size_t z = ty->size;

  انتخاب (ty->type)
    {
    حالت FFI_TYPE_SINT8:
      *(UINT32 *)dst = *(SINT8 *)src;
      بشکن;
    حالت FFI_TYPE_UINT8:
      *(UINT32 *)dst = *(UINT8 *)src;
      بشکن;
    حالت FFI_TYPE_SINT16:
      *(UINT32 *)dst = *(SINT16 *)src;
      بشکن;
    حالت FFI_TYPE_UINT16:
      *(UINT32 *)dst = *(UINT16 *)src;
      بشکن;

    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_SINT32:
    حالت FFI_TYPE_UINT32:
    حالت FFI_TYPE_POINTER:
#ifndef _WIN32
    حالت FFI_TYPE_FLOAT:
#endif
      *(UINT32 *)dst = *(UINT32 *)src;
      بشکن;

#ifdef _WIN32
    // casting a float* to a UINT32* doesn't work on Windows
    حالت FFI_TYPE_FLOAT:
        *(uintptr_t *)dst = 0;
        *(اعشاری *)dst = *(اعشاری *)src;
        بشکن;
#endif

    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_DOUBLE:
      *(UINT64 *)dst = *(UINT64 *)src;
      بشکن;

    حالت FFI_TYPE_STRUCT:
    حالت FFI_TYPE_COMPLEX:
      memcpy (dst, src, z);
      بشکن;

    پیشفرض:
      abort();
    }

  بازگشت FFI_ALIGN (z, 4);
}

/* ffi_prep_args is called once stack space has been allocated
   for the function's arguments.

   The vfp_space parameter is the load area for VFP regs, the return
   value is cif->vfp_used (word bitset of VFP regs used for passing
   arguments). These are only used for the VFP hard-float ABI.
*/
ایستا تهی
ffi_prep_args_SYSV (ffi_cif *cif, صحیح flags, تهی *rvalue,
		    تهی **avalue, نویسه *argp)
{
  ffi_type **arg_types = cif->arg_types;
  صحیح i, n;

  اگر (flags == ARM_TYPE_STRUCT)
    {
      *(تهی **) argp = rvalue;
      argp += 4;
    }

  برای (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, avalue[i], argp);
    }
}

ایستا تهی
ffi_prep_args_VFP (ffi_cif *cif, صحیح flags, تهی *rvalue,
                   تهی **avalue, نویسه *پشته, نویسه *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  صحیح i, n, vi = 0;
  نویسه *argp, *regp, *eo_regp;
  نویسه پشته_used = 0;
  نویسه done_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = پشته;
  eo_regp = argp = regp + 16;

  /* If the function returns an FFI_TYPE_STRUCT in memory,
     that address is passed in r0 to the function.  */
  اگر (flags == ARM_TYPE_STRUCT)
    {
      *(تهی **) regp = rvalue;
      regp += 4;
    }

  برای (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      تهی *a = avalue[i];
      صحیح is_vfp_type = vfp_type_p (ty);

      /* Allocated in VFP registers. */
      اگر (vi < cif->vfp_nargs && is_vfp_type)
	{
	  نویسه *vfp_slot = vfp_space + cif->vfp_args[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_slot);
	  ادامه;
	}
      /* Try allocating in core registers. */
      وگرنه اگر (!done_with_regs && !is_vfp_type)
	{
	  نویسه *tregp = ffi_align (ty, regp);
	  size_t size = ty->size;
	  size = (size < 4) ? 4 : size;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  اگر (tregp + size <= eo_regp)
	    {
	      regp = tregp + ffi_put_arg (ty, a, tregp);
	      done_with_regs = (regp == argp);
	      // ensure we did not write into the stack area
	      FFI_ASSERT (regp <= argp);
	      ادامه;
	    }
	  /* In case there are no arguments in the stack area yet,
	     the argument is passed in the remaining core registers
	     and on the stack.  */
	  وگرنه اگر (!پشته_used)
	    {
	      پشته_used = 1;
	      done_with_regs = 1;
	      argp = tregp + ffi_put_arg (ty, a, tregp);
	      FFI_ASSERT (eo_regp < argp);
	      ادامه;
	    }
	}
      /* Base case, arguments are passed on the stack */
      پشته_used = 1;
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, a, argp);
    }
}

/* Perform machine dependent cif processing */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  صحیح flags = 0, cabi = cif->abi;
  size_t bytes = cif->bytes;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  اگر (cabi == FFI_VFP)
    layout_vfp_args (cif);

  /* Set the return type flag */
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_VOID:
      flags = ARM_TYPE_VOID;
      بشکن;

    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_UINT8:
    حالت FFI_TYPE_SINT8:
    حالت FFI_TYPE_UINT16:
    حالت FFI_TYPE_SINT16:
    حالت FFI_TYPE_UINT32:
    حالت FFI_TYPE_SINT32:
    حالت FFI_TYPE_POINTER:
      flags = ARM_TYPE_INT;
      بشکن;

    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
      flags = ARM_TYPE_INT64;
      بشکن;

    حالت FFI_TYPE_FLOAT:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_S : ARM_TYPE_INT);
      بشکن;
    حالت FFI_TYPE_DOUBLE:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_D : ARM_TYPE_INT64);
      بشکن;

    حالت FFI_TYPE_STRUCT:
    حالت FFI_TYPE_COMPLEX:
      اگر (cabi == FFI_VFP)
	{
	  صحیح h = vfp_type_p (cif->rtype);

	  flags = ARM_TYPE_VFP_N;
	  اگر (h == 0x100 + FFI_TYPE_FLOAT)
	    flags = ARM_TYPE_VFP_S;
	  اگر (h == 0x100 + FFI_TYPE_DOUBLE)
	    flags = ARM_TYPE_VFP_D;
	  اگر (h != 0)
	      بشکن;
	}

      /* A Composite Type not larger than 4 bytes is returned in r0.
	 A Composite Type larger than 4 bytes, or whose size cannot
	 be determined statically ... is stored in memory at an
	 address passed [in r0].  */
      اگر (cif->rtype->size <= 4)
	flags = ARM_TYPE_INT;
      وگرنه
	{
	  flags = ARM_TYPE_STRUCT;
	  bytes += 4;
	}
      بشکن;

    پیشفرض:
      abort();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  bytes = FFI_ALIGN (bytes, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  اگر (bytes < 4*4)
    bytes = 4*4;

  cif->bytes = bytes;
  cif->flags = flags;

  بازگشت FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  unsigned صحیح nfixedargs, unsigned صحیح ntotalargs)
{
  /* VFP variadic calls actually use the SYSV ABI */
  اگر (cif->abi == FFI_VFP)
    cif->abi = FFI_SYSV;

  بازگشت ffi_prep_cif_machdep (cif);
}

/* Prototypes for assembly functions, in sysv.S.  */

ساختار call_frame
{
  تهی *fp;
  تهی *lr;
  تهی *rvalue;
  صحیح flags;
  تهی *closure;
};

بیرونی تهی ffi_call_SYSV (تهی *پشته, ساختار call_frame *,
			   تهی (*fn) (تهی)) FFI_HIDDEN;
بیرونی تهی ffi_call_VFP (تهی *vfp_space, ساختار call_frame *,
			   تهی (*fn) (تهی), unsigned vfp_used) FFI_HIDDEN;

ایستا تهی
ffi_call_int (ffi_cif * cif, تهی (*fn) (تهی), تهی *rvalue,
	      تهی **avalue, تهی *closure)
{
  صحیح flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  size_t bytes, rsize, vfp_size;
  نویسه *پشته, *vfp_space, *new_rvalue;
  ساختار call_frame *frame;

  rsize = 0;
  اگر (rvalue == پوچ)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      اگر (flags == ARM_TYPE_STRUCT)
	rsize = rtype->size;
      وگرنه
	flags = ARM_TYPE_VOID;
    }
  وگرنه اگر (flags == ARM_TYPE_VFP_N)
    {
      /* Largest case is double x 4. */
      rsize = 32;
    }
  وگرنه اگر (flags == ARM_TYPE_INT && rtype->type == FFI_TYPE_STRUCT)
    rsize = 4;

  /* Largest case.  */
  vfp_size = (cif->abi == FFI_VFP && cif->vfp_used ? 8*8: 0);

  bytes = cif->bytes;
  پشته = alloca (vfp_size + bytes + اندازه(ساختار call_frame) + rsize);

  vfp_space = پوچ;
  اگر (vfp_size)
    {
      vfp_space = پشته;
      پشته += vfp_size;
    }

  frame = (ساختار call_frame *)(پشته + bytes);

  new_rvalue = rvalue;
  اگر (rsize)
    new_rvalue = (تهی *)(frame + 1);

  frame->rvalue = new_rvalue;
  frame->flags = flags;
  frame->closure = closure;

  اگر (vfp_space)
    {
      ffi_prep_args_VFP (cif, flags, new_rvalue, avalue, پشته, vfp_space);
      ffi_call_VFP (vfp_space, frame, fn, cif->vfp_used);
    }
  وگرنه
    {
      ffi_prep_args_SYSV (cif, flags, new_rvalue, avalue, پشته);
      ffi_call_SYSV (پشته, frame, fn);
    }

  اگر (rvalue && rvalue != new_rvalue)
    memcpy (rvalue, new_rvalue, rtype->size);
}

تهی
ffi_call (ffi_cif *cif, تهی (*fn) (تهی), تهی *rvalue, تهی **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, پوچ);
}

#ifdef FFI_GO_CLOSURES
تهی
ffi_call_go (ffi_cif *cif, تهی (*fn) (تهی), تهی *rvalue,
	     تهی **avalue, تهی *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif

ایستا تهی *
ffi_prep_incoming_args_SYSV (ffi_cif *cif, تهی *rvalue,
			     نویسه *argp, تهی **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  صحیح i, n;

  اگر (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(تهی **) argp;
      argp += 4;
    }
  وگرنه
    {
      اگر (cif->rtype->size && cif->rtype->size < 4)
        *(uint32_t *) rvalue = 0;
    }

  برای (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;

      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      اگر (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (تهی *) (argp + 4 - z);
      وگرنه
#endif
      avalue[i] = (تهی *) argp;
      argp += z;
    }

  بازگشت rvalue;
}

ایستا تهی *
ffi_prep_incoming_args_VFP (ffi_cif *cif, تهی *rvalue, نویسه *پشته,
			    نویسه *vfp_space, تهی **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  صحیح i, n, vi = 0;
  نویسه *argp, *regp, *eo_regp;
  نویسه done_with_regs = 0;
  نویسه پشته_used = 0;

  regp = پشته;
  eo_regp = argp = regp + 16;

  اگر (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(تهی **) regp;
      regp += 4;
    }

  برای (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      صحیح is_vfp_type = vfp_type_p (ty);
      size_t z = ty->size;

      اگر (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_args[vi++] * 4;
	  ادامه;
	}
      وگرنه اگر (!done_with_regs && !is_vfp_type)
	{
	  نویسه *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  اگر (tregp + z <= eo_regp || !پشته_used)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#ifdef __ARMEB__
	      اگر (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (تهی *) (tregp + 4 - z);
	      وگرنه
#endif
	      avalue[i] = (تهی *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      اگر (regp > eo_regp)
		{
		  FFI_ASSERT (!پشته_used);
		  argp = regp;
		}
	      اگر (regp >= eo_regp)
		{
		  done_with_regs = 1;
		  پشته_used = 1;
		}
	      ادامه;
	    }
	}

      پشته_used = 1;
      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      اگر (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (تهی *) (argp + 4 - z);
      وگرنه
#endif
      avalue[i] = (تهی *) argp;
      argp += z;
    }

  بازگشت rvalue;
}

#اگر FFI_CLOSURES

ساختار closure_frame
{
  نویسه vfp_space[8*8] __attribute__((aligned(8)));
  نویسه result[8*4];
  نویسه argp[];
};

صحیح FFI_HIDDEN
ffi_closure_inner_SYSV (ffi_cif *cif,
		        تهی (*fun) (ffi_cif *, تهی *, تهی **, تهی *),
		        تهی *user_data,
		        ساختار closure_frame *frame)
{
  تهی **avalue = (تهی **) alloca (cif->nargs * اندازه (تهی *));
  تهی *rvalue = ffi_prep_incoming_args_SYSV (cif, frame->result,
					      frame->argp, avalue);
  fun (cif, rvalue, avalue, user_data);
  بازگشت cif->flags;
}

صحیح FFI_HIDDEN
ffi_closure_inner_VFP (ffi_cif *cif,
		       تهی (*fun) (ffi_cif *, تهی *, تهی **, تهی *),
		       تهی *user_data,
		       ساختار closure_frame *frame)
{
  تهی **avalue = (تهی **) alloca (cif->nargs * اندازه (تهی *));
  تهی *rvalue = ffi_prep_incoming_args_VFP (cif, frame->result, frame->argp,
					     frame->vfp_space, avalue);
  fun (cif, rvalue, avalue, user_data);
  بازگشت cif->flags;
}

تهی ffi_closure_SYSV (تهی) FFI_HIDDEN;
تهی ffi_closure_VFP (تهی) FFI_HIDDEN;
#اگر defined(FFI_EXEC_STATIC_TRAMP)
تهی ffi_closure_SYSV_alt (تهی) FFI_HIDDEN;
تهی ffi_closure_VFP_alt (تهی) FFI_HIDDEN;
#endif

#ifdef FFI_GO_CLOSURES
تهی ffi_go_closure_SYSV (تهی) FFI_HIDDEN;
تهی ffi_go_closure_VFP (تهی) FFI_HIDDEN;
#endif

/* the cif must already be prep'ed */

#اگر defined(__FreeBSD__) && defined(__arm__)
#define __clear_cache(start, end) انجامبده { \
		ساختار arm_sync_icache_args ua; 		\
								\
		ua.addr = (uintptr_t)(start);			\
		ua.len = (نویسه *)(end) - (نویسه *)start;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} وقتی (0);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure * closure,
		      ffi_cif * cif,
		      تهی (*fun) (ffi_cif *, تهی *, تهی **, تهی *),
		      تهی *user_data, تهی *codeloc)
{
  تهی (*closure_func) (تهی) = ffi_closure_SYSV;

  اگر (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      اگر (cif->vfp_used)
	closure_func = ffi_closure_VFP;
    }
  وگرنه اگر (cif->abi != FFI_SYSV)
    بازگشت FFI_BAD_ABI;

#اگر FFI_EXEC_TRAMPOLINE_TABLE
  تهی **config = (تهی **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = closure_func;
#وگرنه

# اگر defined(FFI_EXEC_STATIC_TRAMP)
  اگر (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      اگر (closure_func == ffi_closure_SYSV)
        closure_func = ffi_closure_SYSV_alt;
      وگرنه
        closure_func = ffi_closure_VFP_alt;
      ffi_tramp_set_parms (closure->ftramp, closure_func, closure);
      برو out;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  memcpy(closure->tramp, ffi_arm_trampoline, 8);
# وگرنه
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  memcpy(closure->tramp, (تهی*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_CLOSURE_OFFSET);
# endif

# اگر defined(__QNX__)
  msync (closure->tramp, 8, MS_INVALIDATE_ICACHE);	/* clear data map */
  msync (codeloc, 8, MS_INVALIDATE_ICACHE);		/* clear insn map */
# elif defined(_WIN32)
  FlushInstructionCache(GetCurrentProcess(), closure->tramp, FFI_TRAMPOLINE_SIZE);
# وگرنه
  __clear_cache(closure->tramp, closure->tramp + 8);	/* clear data map */
  __clear_cache(codeloc, codeloc + 8);			/* clear insn map */
# endif
# ifdef _WIN32
  *(تهی(**)(تهی))(closure->tramp + FFI_TRAMPOLINE_CLOSURE_FUNCTION) = closure_func;
# وگرنه
  *(تهی (**)(تهی))(closure->tramp + 8) = closure_func;
# endif
# اگر defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  بازگشت FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     تهی (*fun) (ffi_cif *, تهی *, تهی **, تهی *))
{
  تهی (*closure_func) (تهی) = ffi_go_closure_SYSV;

  اگر (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      اگر (cif->vfp_used)
	closure_func = ffi_go_closure_VFP;
    }
  وگرنه اگر (cif->abi != FFI_SYSV)
    بازگشت FFI_BAD_ABI;

  closure->tramp = closure_func;
  closure->cif = cif;
  closure->fun = fun;

  بازگشت FFI_OK;
}
#endif

#endif /* FFI_CLOSURES */

/* Below are routines for VFP hard-float support. */

/* A subroutine of vfp_type_p.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

ایستا صحیح
is_hfa0 (ثابت ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  صحیح i, ret = -1;

  اگر (elements != پوچ)
    برای (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        اگر (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            اگر (ret < 0)
              ادامه;
          }
        بشکن;
      }

  بازگشت ret;
}

/* A subroutine of vfp_type_p.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

ایستا صحیح
is_hfa1 (ثابت ffi_type *ty, صحیح candidate)
{
  ffi_type **elements = ty->elements;
  صحیح i;

  اگر (elements != پوچ)
    برای (i = 0; elements[i]; ++i)
      {
        صحیح t = elements[i]->type;
        اگر (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
          {
            اگر (!is_hfa1 (elements[i], candidate))
              بازگشت 0;
          }
        وگرنه اگر (t != candidate)
          بازگشت 0;
      }

  بازگشت 1;
}

/* Determine if TY is an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is a floating point scalar.

   Returns non-zero iff TY is an HFA.  The result is an encoded value where
   bits 0-7 contain the type code, and bits 8-10 contain the element count.  */

ایستا صحیح
vfp_type_p (ثابت ffi_type *ty)
{
  ffi_type **elements;
  صحیح candidate, i;
  size_t size, ele_شمار;

  /* Quickest tests first.  */
  candidate = ty->type;
  انتخاب (ty->type)
    {
    پیشفرض:
      بازگشت 0;
    حالت FFI_TYPE_FLOAT:
    حالت FFI_TYPE_DOUBLE:
      ele_شمار = 1;
      برو done;
    حالت FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      اگر (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	بازگشت 0;
      ele_شمار = 2;
      برو done;
    حالت FFI_TYPE_STRUCT:
      بشکن;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  size = ty->size;
  اگر (size < 4 || size > 32)
    بازگشت 0;

  /* Find the type of the first non-structure member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  اگر (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_COMPLEX)
    {
      برای (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          اگر (candidate >= 0)
            بشکن;
        }
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  انتخاب (candidate)
    {
    حالت FFI_TYPE_FLOAT:
      ele_شمار = size / اندازه(اعشاری);
      اگر (size != ele_شمار * اندازه(اعشاری))
        بازگشت 0;
      بشکن;
    حالت FFI_TYPE_DOUBLE:
      ele_شمار = size / اندازه(دوگانه);
      اگر (size != ele_شمار * اندازه(دوگانه))
        بازگشت 0;
      بشکن;
    پیشفرض:
      بازگشت 0;
    }
  اگر (ele_شمار > 4)
    بازگشت 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  برای (i = 0; elements[i]; ++i)
    {
      صحیح t = elements[i]->type;
      اگر (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          اگر (!is_hfa1 (elements[i], candidate))
            بازگشت 0;
        }
      وگرنه اگر (t != candidate)
        بازگشت 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  بازگشت (ele_شمار << 8) | candidate;
}

ایستا صحیح
place_vfp_arg (ffi_cif *cif, صحیح h)
{
  unsigned short reg = cif->vfp_reg_free;
  صحیح align = 1, nregs = h >> 8;

  اگر ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  اگر ((reg & 1) && align == 2)
    reg++;

  وقتی (reg + nregs <= 16)
    {
      صحیح s, new_used = 0;
      برای (s = reg; s < reg + nregs; s++)
	{
	  new_used |= (1 << s);
	  اگر (cif->vfp_used & (1 << s))
	    {
	      reg += align;
	      برو next_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_used |= new_used;
      cif->vfp_args[cif->vfp_nargs++] = (signed نویسه)reg;

      /* Update vfp_reg_free. */
      اگر (cif->vfp_used & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  وقتی (cif->vfp_used & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      بازگشت 0;
    next_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_used = 0xFFFF;
  بازگشت 1;
}

ایستا تهی
layout_vfp_args (ffi_cif * cif)
{
  unsigned صحیح i;
  /* Init VFP fields */
  cif->vfp_used = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  memset (cif->vfp_args, -1, 16);	/* Init to -1. */

  برای (i = 0; i < cif->nargs; i++)
    {
      صحیح h = vfp_type_p (cif->arg_types[i]);
      اگر (h && place_vfp_arg (cif, h) == 1)
	بشکن;
    }
}

#اگر defined(FFI_EXEC_STATIC_TRAMP)
تهی *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  بیرونی تهی *trampoline_code_table;

  *tramp_size = ARM_TRAMP_SIZE;
  *map_size = ARM_TRAMP_MAP_SIZE;
  بازگشت &trampoline_code_table;
}
#endif

#endif /* __arm__ or _M_ARM */
