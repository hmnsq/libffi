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

#सचे defined(__arm__) || defined(_M_ARM)
#अन्तर्भाव <fficonfig.h>
#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <tramp.h>
#अन्तर्भाव "internal.h"

#सचे defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#अन्तर्भाव <windows.h>
#endif

#सचे FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#अन्तर्भाव <mach/machine/vm_param.h>
#endif

#अञ्ञथा
#ifndef _WIN32
बाह्य unsigned पूर्णाङ्क ffi_arm_trampoline[2] FFI_HIDDEN;
#अञ्ञथा
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
बाह्य unsigned वर्ण ffi_arm_trampoline[12] FFI_HIDDEN;
#endif
#endif

#सचे defined(__FreeBSD__) && defined(__arm__)
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <machine/sysarch.h>
#endif

#सचे defined(__QNX__)
#अन्तर्भाव <sys/mman.h>
#endif

/* Forward declares. */
स्थिर पूर्णाङ्क vfp_type_p (ध्रुव ffi_type *);
स्थिर शून्य layout_vfp_args (ffi_cif *);

स्थिर शून्य *
ffi_align (ffi_type *ty, शून्य *p)
{
  /* Align if necessary */
  size_t alignment;
#ifdef _WIN32_WCE
  alignment = 4;
#अञ्ञथा
  alignment = ty->alignment;
  सचे (alignment < 4)
    alignment = 4;
#endif
  पटिनिवत्त (शून्य *) FFI_ALIGN (p, alignment);
}

स्थिर size_t
ffi_put_arg (ffi_type *ty, शून्य *src, शून्य *dst)
{
  size_t z = ty->size;

  विकल्प (ty->type)
    {
    पक्ष FFI_TYPE_SINT8:
      *(UINT32 *)dst = *(SINT8 *)src;
      भङ्ग;
    पक्ष FFI_TYPE_UINT8:
      *(UINT32 *)dst = *(UINT8 *)src;
      भङ्ग;
    पक्ष FFI_TYPE_SINT16:
      *(UINT32 *)dst = *(SINT16 *)src;
      भङ्ग;
    पक्ष FFI_TYPE_UINT16:
      *(UINT32 *)dst = *(UINT16 *)src;
      भङ्ग;

    पक्ष FFI_TYPE_INT:
    पक्ष FFI_TYPE_SINT32:
    पक्ष FFI_TYPE_UINT32:
    पक्ष FFI_TYPE_POINTER:
#ifndef _WIN32
    पक्ष FFI_TYPE_FLOAT:
#endif
      *(UINT32 *)dst = *(UINT32 *)src;
      भङ्ग;

#ifdef _WIN32
    // casting a float* to a UINT32* doesn't work on Windows
    पक्ष FFI_TYPE_FLOAT:
        *(uintptr_t *)dst = 0;
        *(भिन्न *)dst = *(भिन्न *)src;
        भङ्ग;
#endif

    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_DOUBLE:
      *(UINT64 *)dst = *(UINT64 *)src;
      भङ्ग;

    पक्ष FFI_TYPE_STRUCT:
    पक्ष FFI_TYPE_COMPLEX:
      memcpy (dst, src, z);
      भङ्ग;

    स्वभाव:
      abort();
    }

  पटिनिवत्त FFI_ALIGN (z, 4);
}

/* ffi_prep_args is called once stack space has been allocated
   for the function's arguments.

   The vfp_space parameter is the load area for VFP regs, the return
   value is cif->vfp_used (word bitset of VFP regs used for passing
   arguments). These are only used for the VFP hard-float ABI.
*/
स्थिर शून्य
ffi_prep_args_SYSV (ffi_cif *cif, पूर्णाङ्क flags, शून्य *rvalue,
		    शून्य **avalue, वर्ण *argp)
{
  ffi_type **arg_types = cif->arg_types;
  पूर्णाङ्क i, n;

  सचे (flags == ARM_TYPE_STRUCT)
    {
      *(शून्य **) argp = rvalue;
      argp += 4;
    }

  अत्थाय (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, avalue[i], argp);
    }
}

स्थिर शून्य
ffi_prep_args_VFP (ffi_cif *cif, पूर्णाङ्क flags, शून्य *rvalue,
                   शून्य **avalue, वर्ण *राशिः, वर्ण *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  पूर्णाङ्क i, n, vi = 0;
  वर्ण *argp, *regp, *eo_regp;
  वर्ण राशिः_used = 0;
  वर्ण done_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = राशिः;
  eo_regp = argp = regp + 16;

  /* If the function returns an FFI_TYPE_STRUCT in memory,
     that address is passed in r0 to the function.  */
  सचे (flags == ARM_TYPE_STRUCT)
    {
      *(शून्य **) regp = rvalue;
      regp += 4;
    }

  अत्थाय (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      शून्य *a = avalue[i];
      पूर्णाङ्क is_vfp_type = vfp_type_p (ty);

      /* Allocated in VFP registers. */
      सचे (vi < cif->vfp_nargs && is_vfp_type)
	{
	  वर्ण *vfp_slot = vfp_space + cif->vfp_args[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_slot);
	  अनुवर्त;
	}
      /* Try allocating in core registers. */
      अञ्ञथा सचे (!done_with_regs && !is_vfp_type)
	{
	  वर्ण *tregp = ffi_align (ty, regp);
	  size_t size = ty->size;
	  size = (size < 4) ? 4 : size;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  सचे (tregp + size <= eo_regp)
	    {
	      regp = tregp + ffi_put_arg (ty, a, tregp);
	      done_with_regs = (regp == argp);
	      // ensure we did not write into the stack area
	      FFI_ASSERT (regp <= argp);
	      अनुवर्त;
	    }
	  /* In case there are no arguments in the stack area yet,
	     the argument is passed in the remaining core registers
	     and on the stack.  */
	  अञ्ञथा सचे (!राशिः_used)
	    {
	      राशिः_used = 1;
	      done_with_regs = 1;
	      argp = tregp + ffi_put_arg (ty, a, tregp);
	      FFI_ASSERT (eo_regp < argp);
	      अनुवर्त;
	    }
	}
      /* Base case, arguments are passed on the stack */
      राशिः_used = 1;
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, a, argp);
    }
}

/* Perform machine dependent cif processing */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  पूर्णाङ्क flags = 0, cabi = cif->abi;
  size_t bytes = cif->bytes;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  सचे (cabi == FFI_VFP)
    layout_vfp_args (cif);

  /* Set the return type flag */
  विकल्प (cif->rtype->type)
    {
    पक्ष FFI_TYPE_VOID:
      flags = ARM_TYPE_VOID;
      भङ्ग;

    पक्ष FFI_TYPE_INT:
    पक्ष FFI_TYPE_UINT8:
    पक्ष FFI_TYPE_SINT8:
    पक्ष FFI_TYPE_UINT16:
    पक्ष FFI_TYPE_SINT16:
    पक्ष FFI_TYPE_UINT32:
    पक्ष FFI_TYPE_SINT32:
    पक्ष FFI_TYPE_POINTER:
      flags = ARM_TYPE_INT;
      भङ्ग;

    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
      flags = ARM_TYPE_INT64;
      भङ्ग;

    पक्ष FFI_TYPE_FLOAT:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_S : ARM_TYPE_INT);
      भङ्ग;
    पक्ष FFI_TYPE_DOUBLE:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_D : ARM_TYPE_INT64);
      भङ्ग;

    पक्ष FFI_TYPE_STRUCT:
    पक्ष FFI_TYPE_COMPLEX:
      सचे (cabi == FFI_VFP)
	{
	  पूर्णाङ्क h = vfp_type_p (cif->rtype);

	  flags = ARM_TYPE_VFP_N;
	  सचे (h == 0x100 + FFI_TYPE_FLOAT)
	    flags = ARM_TYPE_VFP_S;
	  सचे (h == 0x100 + FFI_TYPE_DOUBLE)
	    flags = ARM_TYPE_VFP_D;
	  सचे (h != 0)
	      भङ्ग;
	}

      /* A Composite Type not larger than 4 bytes is returned in r0.
	 A Composite Type larger than 4 bytes, or whose size cannot
	 be determined statically ... is stored in memory at an
	 address passed [in r0].  */
      सचे (cif->rtype->size <= 4)
	flags = ARM_TYPE_INT;
      अञ्ञथा
	{
	  flags = ARM_TYPE_STRUCT;
	  bytes += 4;
	}
      भङ्ग;

    स्वभाव:
      abort();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  bytes = FFI_ALIGN (bytes, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  सचे (bytes < 4*4)
    bytes = 4*4;

  cif->bytes = bytes;
  cif->flags = flags;

  पटिनिवत्त FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  unsigned पूर्णाङ्क nfixedargs, unsigned पूर्णाङ्क ntotalargs)
{
  /* VFP variadic calls actually use the SYSV ABI */
  सचे (cif->abi == FFI_VFP)
    cif->abi = FFI_SYSV;

  पटिनिवत्त ffi_prep_cif_machdep (cif);
}

/* Prototypes for assembly functions, in sysv.S.  */

संरचना call_frame
{
  शून्य *fp;
  शून्य *lr;
  शून्य *rvalue;
  पूर्णाङ्क flags;
  शून्य *closure;
};

बाह्य शून्य ffi_call_SYSV (शून्य *राशिः, संरचना call_frame *,
			   शून्य (*fn) (शून्य)) FFI_HIDDEN;
बाह्य शून्य ffi_call_VFP (शून्य *vfp_space, संरचना call_frame *,
			   शून्य (*fn) (शून्य), unsigned vfp_used) FFI_HIDDEN;

स्थिर शून्य
ffi_call_int (ffi_cif * cif, शून्य (*fn) (शून्य), शून्य *rvalue,
	      शून्य **avalue, शून्य *closure)
{
  पूर्णाङ्क flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  size_t bytes, rsize, vfp_size;
  वर्ण *राशिः, *vfp_space, *new_rvalue;
  संरचना call_frame *frame;

  rsize = 0;
  सचे (rvalue == रिक्त)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      सचे (flags == ARM_TYPE_STRUCT)
	rsize = rtype->size;
      अञ्ञथा
	flags = ARM_TYPE_VOID;
    }
  अञ्ञथा सचे (flags == ARM_TYPE_VFP_N)
    {
      /* Largest case is double x 4. */
      rsize = 32;
    }
  अञ्ञथा सचे (flags == ARM_TYPE_INT && rtype->type == FFI_TYPE_STRUCT)
    rsize = 4;

  /* Largest case.  */
  vfp_size = (cif->abi == FFI_VFP && cif->vfp_used ? 8*8: 0);

  bytes = cif->bytes;
  राशिः = alloca (vfp_size + bytes + परिमाण(संरचना call_frame) + rsize);

  vfp_space = रिक्त;
  सचे (vfp_size)
    {
      vfp_space = राशिः;
      राशिः += vfp_size;
    }

  frame = (संरचना call_frame *)(राशिः + bytes);

  new_rvalue = rvalue;
  सचे (rsize)
    new_rvalue = (शून्य *)(frame + 1);

  frame->rvalue = new_rvalue;
  frame->flags = flags;
  frame->closure = closure;

  सचे (vfp_space)
    {
      ffi_prep_args_VFP (cif, flags, new_rvalue, avalue, राशिः, vfp_space);
      ffi_call_VFP (vfp_space, frame, fn, cif->vfp_used);
    }
  अञ्ञथा
    {
      ffi_prep_args_SYSV (cif, flags, new_rvalue, avalue, राशिः);
      ffi_call_SYSV (राशिः, frame, fn);
    }

  सचे (rvalue && rvalue != new_rvalue)
    memcpy (rvalue, new_rvalue, rtype->size);
}

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, रिक्त);
}

#ifdef FFI_GO_CLOSURES
शून्य
ffi_call_go (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue,
	     शून्य **avalue, शून्य *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif

स्थिर शून्य *
ffi_prep_incoming_args_SYSV (ffi_cif *cif, शून्य *rvalue,
			     वर्ण *argp, शून्य **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  पूर्णाङ्क i, n;

  सचे (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(शून्य **) argp;
      argp += 4;
    }
  अञ्ञथा
    {
      सचे (cif->rtype->size && cif->rtype->size < 4)
        *(uint32_t *) rvalue = 0;
    }

  अत्थाय (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;

      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      सचे (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (शून्य *) (argp + 4 - z);
      अञ्ञथा
#endif
      avalue[i] = (शून्य *) argp;
      argp += z;
    }

  पटिनिवत्त rvalue;
}

स्थिर शून्य *
ffi_prep_incoming_args_VFP (ffi_cif *cif, शून्य *rvalue, वर्ण *राशिः,
			    वर्ण *vfp_space, शून्य **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  पूर्णाङ्क i, n, vi = 0;
  वर्ण *argp, *regp, *eo_regp;
  वर्ण done_with_regs = 0;
  वर्ण राशिः_used = 0;

  regp = राशिः;
  eo_regp = argp = regp + 16;

  सचे (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(शून्य **) regp;
      regp += 4;
    }

  अत्थाय (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      पूर्णाङ्क is_vfp_type = vfp_type_p (ty);
      size_t z = ty->size;

      सचे (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_args[vi++] * 4;
	  अनुवर्त;
	}
      अञ्ञथा सचे (!done_with_regs && !is_vfp_type)
	{
	  वर्ण *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  सचे (tregp + z <= eo_regp || !राशिः_used)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#ifdef __ARMEB__
	      सचे (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (शून्य *) (tregp + 4 - z);
	      अञ्ञथा
#endif
	      avalue[i] = (शून्य *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      सचे (regp > eo_regp)
		{
		  FFI_ASSERT (!राशिः_used);
		  argp = regp;
		}
	      सचे (regp >= eo_regp)
		{
		  done_with_regs = 1;
		  राशिः_used = 1;
		}
	      अनुवर्त;
	    }
	}

      राशिः_used = 1;
      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      सचे (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (शून्य *) (argp + 4 - z);
      अञ्ञथा
#endif
      avalue[i] = (शून्य *) argp;
      argp += z;
    }

  पटिनिवत्त rvalue;
}

#सचे FFI_CLOSURES

संरचना closure_frame
{
  वर्ण vfp_space[8*8] __attribute__((aligned(8)));
  वर्ण result[8*4];
  वर्ण argp[];
};

पूर्णाङ्क FFI_HIDDEN
ffi_closure_inner_SYSV (ffi_cif *cif,
		        शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		        शून्य *user_data,
		        संरचना closure_frame *frame)
{
  शून्य **avalue = (शून्य **) alloca (cif->nargs * परिमाण (शून्य *));
  शून्य *rvalue = ffi_prep_incoming_args_SYSV (cif, frame->result,
					      frame->argp, avalue);
  fun (cif, rvalue, avalue, user_data);
  पटिनिवत्त cif->flags;
}

पूर्णाङ्क FFI_HIDDEN
ffi_closure_inner_VFP (ffi_cif *cif,
		       शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		       शून्य *user_data,
		       संरचना closure_frame *frame)
{
  शून्य **avalue = (शून्य **) alloca (cif->nargs * परिमाण (शून्य *));
  शून्य *rvalue = ffi_prep_incoming_args_VFP (cif, frame->result, frame->argp,
					     frame->vfp_space, avalue);
  fun (cif, rvalue, avalue, user_data);
  पटिनिवत्त cif->flags;
}

शून्य ffi_closure_SYSV (शून्य) FFI_HIDDEN;
शून्य ffi_closure_VFP (शून्य) FFI_HIDDEN;
#सचे defined(FFI_EXEC_STATIC_TRAMP)
शून्य ffi_closure_SYSV_alt (शून्य) FFI_HIDDEN;
शून्य ffi_closure_VFP_alt (शून्य) FFI_HIDDEN;
#endif

#ifdef FFI_GO_CLOSURES
शून्य ffi_go_closure_SYSV (शून्य) FFI_HIDDEN;
शून्य ffi_go_closure_VFP (शून्य) FFI_HIDDEN;
#endif

/* the cif must already be prep'ed */

#सचे defined(__FreeBSD__) && defined(__arm__)
#define __clear_cache(start, end) करोहि { \
		संरचना arm_sync_icache_args ua; 		\
								\
		ua.addr = (uintptr_t)(start);			\
		ua.len = (वर्ण *)(end) - (वर्ण *)start;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} याव (0);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure * closure,
		      ffi_cif * cif,
		      शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		      शून्य *user_data, शून्य *codeloc)
{
  शून्य (*closure_func) (शून्य) = ffi_closure_SYSV;

  सचे (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      सचे (cif->vfp_used)
	closure_func = ffi_closure_VFP;
    }
  अञ्ञथा सचे (cif->abi != FFI_SYSV)
    पटिनिवत्त FFI_BAD_ABI;

#सचे FFI_EXEC_TRAMPOLINE_TABLE
  शून्य **config = (शून्य **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = closure_func;
#अञ्ञथा

# सचे defined(FFI_EXEC_STATIC_TRAMP)
  सचे (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      सचे (closure_func == ffi_closure_SYSV)
        closure_func = ffi_closure_SYSV_alt;
      अञ्ञथा
        closure_func = ffi_closure_VFP_alt;
      ffi_tramp_set_parms (closure->ftramp, closure_func, closure);
      गच्छ out;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  memcpy(closure->tramp, ffi_arm_trampoline, 8);
# अञ्ञथा
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  memcpy(closure->tramp, (शून्य*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_CLOSURE_OFFSET);
# endif

# सचे defined(__QNX__)
  msync (closure->tramp, 8, MS_INVALIDATE_ICACHE);	/* clear data map */
  msync (codeloc, 8, MS_INVALIDATE_ICACHE);		/* clear insn map */
# elif defined(_WIN32)
  FlushInstructionCache(GetCurrentProcess(), closure->tramp, FFI_TRAMPOLINE_SIZE);
# अञ्ञथा
  __clear_cache(closure->tramp, closure->tramp + 8);	/* clear data map */
  __clear_cache(codeloc, codeloc + 8);			/* clear insn map */
# endif
# ifdef _WIN32
  *(शून्य(**)(शून्य))(closure->tramp + FFI_TRAMPOLINE_CLOSURE_FUNCTION) = closure_func;
# अञ्ञथा
  *(शून्य (**)(शून्य))(closure->tramp + 8) = closure_func;
# endif
# सचे defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  पटिनिवत्त FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *))
{
  शून्य (*closure_func) (शून्य) = ffi_go_closure_SYSV;

  सचे (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      सचे (cif->vfp_used)
	closure_func = ffi_go_closure_VFP;
    }
  अञ्ञथा सचे (cif->abi != FFI_SYSV)
    पटिनिवत्त FFI_BAD_ABI;

  closure->tramp = closure_func;
  closure->cif = cif;
  closure->fun = fun;

  पटिनिवत्त FFI_OK;
}
#endif

#endif /* FFI_CLOSURES */

/* Below are routines for VFP hard-float support. */

/* A subroutine of vfp_type_p.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

स्थिर पूर्णाङ्क
is_hfa0 (ध्रुव ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  पूर्णाङ्क i, ret = -1;

  सचे (elements != रिक्त)
    अत्थाय (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        सचे (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            सचे (ret < 0)
              अनुवर्त;
          }
        भङ्ग;
      }

  पटिनिवत्त ret;
}

/* A subroutine of vfp_type_p.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

स्थिर पूर्णाङ्क
is_hfa1 (ध्रुव ffi_type *ty, पूर्णाङ्क candidate)
{
  ffi_type **elements = ty->elements;
  पूर्णाङ्क i;

  सचे (elements != रिक्त)
    अत्थाय (i = 0; elements[i]; ++i)
      {
        पूर्णाङ्क t = elements[i]->type;
        सचे (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
          {
            सचे (!is_hfa1 (elements[i], candidate))
              पटिनिवत्त 0;
          }
        अञ्ञथा सचे (t != candidate)
          पटिनिवत्त 0;
      }

  पटिनिवत्त 1;
}

/* Determine if TY is an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is a floating point scalar.

   Returns non-zero iff TY is an HFA.  The result is an encoded value where
   bits 0-7 contain the type code, and bits 8-10 contain the element count.  */

स्थिर पूर्णाङ्क
vfp_type_p (ध्रुव ffi_type *ty)
{
  ffi_type **elements;
  पूर्णाङ्क candidate, i;
  size_t size, ele_गणनम्;

  /* Quickest tests first.  */
  candidate = ty->type;
  विकल्प (ty->type)
    {
    स्वभाव:
      पटिनिवत्त 0;
    पक्ष FFI_TYPE_FLOAT:
    पक्ष FFI_TYPE_DOUBLE:
      ele_गणनम् = 1;
      गच्छ done;
    पक्ष FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      सचे (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	पटिनिवत्त 0;
      ele_गणनम् = 2;
      गच्छ done;
    पक्ष FFI_TYPE_STRUCT:
      भङ्ग;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  size = ty->size;
  सचे (size < 4 || size > 32)
    पटिनिवत्त 0;

  /* Find the type of the first non-structure member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  सचे (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_COMPLEX)
    {
      अत्थाय (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          सचे (candidate >= 0)
            भङ्ग;
        }
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  विकल्प (candidate)
    {
    पक्ष FFI_TYPE_FLOAT:
      ele_गणनम् = size / परिमाण(भिन्न);
      सचे (size != ele_गणनम् * परिमाण(भिन्न))
        पटिनिवत्त 0;
      भङ्ग;
    पक्ष FFI_TYPE_DOUBLE:
      ele_गणनम् = size / परिमाण(द्विगुण);
      सचे (size != ele_गणनम् * परिमाण(द्विगुण))
        पटिनिवत्त 0;
      भङ्ग;
    स्वभाव:
      पटिनिवत्त 0;
    }
  सचे (ele_गणनम् > 4)
    पटिनिवत्त 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  अत्थाय (i = 0; elements[i]; ++i)
    {
      पूर्णाङ्क t = elements[i]->type;
      सचे (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          सचे (!is_hfa1 (elements[i], candidate))
            पटिनिवत्त 0;
        }
      अञ्ञथा सचे (t != candidate)
        पटिनिवत्त 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  पटिनिवत्त (ele_गणनम् << 8) | candidate;
}

स्थिर पूर्णाङ्क
place_vfp_arg (ffi_cif *cif, पूर्णाङ्क h)
{
  unsigned short reg = cif->vfp_reg_free;
  पूर्णाङ्क align = 1, nregs = h >> 8;

  सचे ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  सचे ((reg & 1) && align == 2)
    reg++;

  याव (reg + nregs <= 16)
    {
      पूर्णाङ्क s, new_used = 0;
      अत्थाय (s = reg; s < reg + nregs; s++)
	{
	  new_used |= (1 << s);
	  सचे (cif->vfp_used & (1 << s))
	    {
	      reg += align;
	      गच्छ next_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_used |= new_used;
      cif->vfp_args[cif->vfp_nargs++] = (signed वर्ण)reg;

      /* Update vfp_reg_free. */
      सचे (cif->vfp_used & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  याव (cif->vfp_used & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      पटिनिवत्त 0;
    next_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_used = 0xFFFF;
  पटिनिवत्त 1;
}

स्थिर शून्य
layout_vfp_args (ffi_cif * cif)
{
  unsigned पूर्णाङ्क i;
  /* Init VFP fields */
  cif->vfp_used = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  memset (cif->vfp_args, -1, 16);	/* Init to -1. */

  अत्थाय (i = 0; i < cif->nargs; i++)
    {
      पूर्णाङ्क h = vfp_type_p (cif->arg_types[i]);
      सचे (h && place_vfp_arg (cif, h) == 1)
	भङ्ग;
    }
}

#सचे defined(FFI_EXEC_STATIC_TRAMP)
शून्य *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  बाह्य शून्य *trampoline_code_table;

  *tramp_size = ARM_TRAMP_SIZE;
  *map_size = ARM_TRAMP_MAP_SIZE;
  पटिनिवत्त &trampoline_code_table;
}
#endif

#endif /* __arm__ or _M_ARM */
