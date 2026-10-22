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

#यदि defined(__arm__) || defined(_M_ARM)
#अन्तर्भाव <fficonfig.h>
#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <tramp.h>
#अन्तर्भाव "internal.h"

#यदि defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#अन्तर्भाव <windows.h>
#endif

#यदि FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#अन्तर्भाव <mach/machine/vm_param.h>
#endif

#अन्यथा
#ifndef _WIN32
बाह्य अचिन्हित पूर्णाङ्क ffi_arm_trampoline[2] FFI_HIDDEN;
#अन्यथा
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
बाह्य अचिन्हित वर्ण ffi_arm_trampoline[12] FFI_HIDDEN;
#endif
#endif

#यदि defined(__FreeBSD__) && defined(__arm__)
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <machine/sysarch.h>
#endif

#यदि defined(__QNX__)
#अन्तर्भाव <sys/mman.h>
#endif

/* Forward declares. */
स्थिर पूर्णाङ्क vfp_type_p (ध्रुव ffi_type *);
स्थिर शून्य layout_vfp_args (ffi_cif *);

स्थिर शून्य *
ffi_align (ffi_type *ty, शून्य *p)
{
  /* Align if necessary */
  माप_प्रकार alignment;
#ifdef _WIN32_WCE
  alignment = 4;
#अन्यथा
  alignment = ty->alignment;
  यदि (alignment < 4)
    alignment = 4;
#endif
  प्रत्यागम (शून्य *) FFI_ALIGN (p, alignment);
}

स्थिर माप_प्रकार
ffi_put_arg (ffi_type *ty, शून्य *src, शून्य *dst)
{
  माप_प्रकार z = ty->size;

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
      स_नकल (dst, src, z);
      भङ्ग;

    स्वभाव:
      पात();
    }

  प्रत्यागम FFI_ALIGN (z, 4);
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

  यदि (flags == ARM_TYPE_STRUCT)
    {
      *(शून्य **) argp = rvalue;
      argp += 4;
    }

  प्रति (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, avalue[i], argp);
    }
}

स्थिर शून्य
ffi_prep_args_VFP (ffi_cif *cif, पूर्णाङ्क flags, शून्य *rvalue,
                   शून्य **avalue, वर्ण *ढेर, वर्ण *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  पूर्णाङ्क i, n, vi = 0;
  वर्ण *argp, *regp, *eo_regp;
  वर्ण ढेर_used = 0;
  वर्ण done_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = ढेर;
  eo_regp = argp = regp + 16;

  /* If the function returns an FFI_TYPE_STRUCT in memory,
     that address is passed in r0 to the function.  */
  यदि (flags == ARM_TYPE_STRUCT)
    {
      *(शून्य **) regp = rvalue;
      regp += 4;
    }

  प्रति (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      शून्य *a = avalue[i];
      पूर्णाङ्क is_vfp_type = vfp_type_p (ty);

      /* Allocated in VFP registers. */
      यदि (vi < cif->vfp_nargs && is_vfp_type)
	{
	  वर्ण *vfp_slot = vfp_space + cif->vfp_args[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_slot);
	  अनुवर्त;
	}
      /* Try allocating in core registers. */
      अन्यथा यदि (!done_with_regs && !is_vfp_type)
	{
	  वर्ण *tregp = ffi_align (ty, regp);
	  माप_प्रकार size = ty->size;
	  size = (size < 4) ? 4 : size;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  यदि (tregp + size <= eo_regp)
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
	  अन्यथा यदि (!ढेर_used)
	    {
	      ढेर_used = 1;
	      done_with_regs = 1;
	      argp = tregp + ffi_put_arg (ty, a, tregp);
	      FFI_ASSERT (eo_regp < argp);
	      अनुवर्त;
	    }
	}
      /* Base case, arguments are passed on the stack */
      ढेर_used = 1;
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, a, argp);
    }
}

/* Perform machine dependent cif processing */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  पूर्णाङ्क flags = 0, cabi = cif->abi;
  माप_प्रकार bytes = cif->bytes;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  यदि (cabi == FFI_VFP)
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
      यदि (cabi == FFI_VFP)
	{
	  पूर्णाङ्क h = vfp_type_p (cif->rtype);

	  flags = ARM_TYPE_VFP_N;
	  यदि (h == 0x100 + FFI_TYPE_FLOAT)
	    flags = ARM_TYPE_VFP_S;
	  यदि (h == 0x100 + FFI_TYPE_DOUBLE)
	    flags = ARM_TYPE_VFP_D;
	  यदि (h != 0)
	      भङ्ग;
	}

      /* A Composite Type not larger than 4 bytes is returned in r0.
	 A Composite Type larger than 4 bytes, or whose size cannot
	 be determined statically ... is stored in memory at an
	 address passed [in r0].  */
      यदि (cif->rtype->size <= 4)
	flags = ARM_TYPE_INT;
      अन्यथा
	{
	  flags = ARM_TYPE_STRUCT;
	  bytes += 4;
	}
      भङ्ग;

    स्वभाव:
      पात();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  bytes = FFI_ALIGN (bytes, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  यदि (bytes < 4*4)
    bytes = 4*4;

  cif->bytes = bytes;
  cif->flags = flags;

  प्रत्यागम FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  अचिन्हित पूर्णाङ्क nfixedargs, अचिन्हित पूर्णाङ्क ntotalargs)
{
  /* VFP variadic calls actually use the SYSV ABI */
  यदि (cif->abi == FFI_VFP)
    cif->abi = FFI_SYSV;

  प्रत्यागम ffi_prep_cif_machdep (cif);
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

बाह्य शून्य ffi_call_SYSV (शून्य *ढेर, संरचना call_frame *,
			   शून्य (*fn) (शून्य)) FFI_HIDDEN;
बाह्य शून्य ffi_call_VFP (शून्य *vfp_space, संरचना call_frame *,
			   शून्य (*fn) (शून्य), अचिन्हित vfp_used) FFI_HIDDEN;

स्थिर शून्य
ffi_call_int (ffi_cif * cif, शून्य (*fn) (शून्य), शून्य *rvalue,
	      शून्य **avalue, शून्य *closure)
{
  पूर्णाङ्क flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  माप_प्रकार bytes, rsize, vfp_size;
  वर्ण *ढेर, *vfp_space, *new_rvalue;
  संरचना call_frame *frame;

  rsize = 0;
  यदि (rvalue == रिक्त)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      यदि (flags == ARM_TYPE_STRUCT)
	rsize = rtype->size;
      अन्यथा
	flags = ARM_TYPE_VOID;
    }
  अन्यथा यदि (flags == ARM_TYPE_VFP_N)
    {
      /* Largest case is double x 4. */
      rsize = 32;
    }
  अन्यथा यदि (flags == ARM_TYPE_INT && rtype->type == FFI_TYPE_STRUCT)
    rsize = 4;

  /* Largest case.  */
  vfp_size = (cif->abi == FFI_VFP && cif->vfp_used ? 8*8: 0);

  bytes = cif->bytes;
  ढेर = alloca (vfp_size + bytes + परिमाण(संरचना call_frame) + rsize);

  vfp_space = रिक्त;
  यदि (vfp_size)
    {
      vfp_space = ढेर;
      ढेर += vfp_size;
    }

  frame = (संरचना call_frame *)(ढेर + bytes);

  new_rvalue = rvalue;
  यदि (rsize)
    new_rvalue = (शून्य *)(frame + 1);

  frame->rvalue = new_rvalue;
  frame->flags = flags;
  frame->closure = closure;

  यदि (vfp_space)
    {
      ffi_prep_args_VFP (cif, flags, new_rvalue, avalue, ढेर, vfp_space);
      ffi_call_VFP (vfp_space, frame, fn, cif->vfp_used);
    }
  अन्यथा
    {
      ffi_prep_args_SYSV (cif, flags, new_rvalue, avalue, ढेर);
      ffi_call_SYSV (ढेर, frame, fn);
    }

  यदि (rvalue && rvalue != new_rvalue)
    स_नकल (rvalue, new_rvalue, rtype->size);
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

  यदि (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(शून्य **) argp;
      argp += 4;
    }
  अन्यथा
    {
      यदि (cif->rtype->size && cif->rtype->size < 4)
        *(uint32_t *) rvalue = 0;
    }

  प्रति (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      माप_प्रकार z = ty->size;

      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      यदि (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (शून्य *) (argp + 4 - z);
      अन्यथा
#endif
      avalue[i] = (शून्य *) argp;
      argp += z;
    }

  प्रत्यागम rvalue;
}

स्थिर शून्य *
ffi_prep_incoming_args_VFP (ffi_cif *cif, शून्य *rvalue, वर्ण *ढेर,
			    वर्ण *vfp_space, शून्य **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  पूर्णाङ्क i, n, vi = 0;
  वर्ण *argp, *regp, *eo_regp;
  वर्ण done_with_regs = 0;
  वर्ण ढेर_used = 0;

  regp = ढेर;
  eo_regp = argp = regp + 16;

  यदि (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(शून्य **) regp;
      regp += 4;
    }

  प्रति (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      पूर्णाङ्क is_vfp_type = vfp_type_p (ty);
      माप_प्रकार z = ty->size;

      यदि (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_args[vi++] * 4;
	  अनुवर्त;
	}
      अन्यथा यदि (!done_with_regs && !is_vfp_type)
	{
	  वर्ण *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  यदि (tregp + z <= eo_regp || !ढेर_used)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#ifdef __ARMEB__
	      यदि (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (शून्य *) (tregp + 4 - z);
	      अन्यथा
#endif
	      avalue[i] = (शून्य *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      यदि (regp > eo_regp)
		{
		  FFI_ASSERT (!ढेर_used);
		  argp = regp;
		}
	      यदि (regp >= eo_regp)
		{
		  done_with_regs = 1;
		  ढेर_used = 1;
		}
	      अनुवर्त;
	    }
	}

      ढेर_used = 1;
      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      यदि (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (शून्य *) (argp + 4 - z);
      अन्यथा
#endif
      avalue[i] = (शून्य *) argp;
      argp += z;
    }

  प्रत्यागम rvalue;
}

#यदि FFI_CLOSURES

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
  प्रत्यागम cif->flags;
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
  प्रत्यागम cif->flags;
}

शून्य ffi_closure_SYSV (शून्य) FFI_HIDDEN;
शून्य ffi_closure_VFP (शून्य) FFI_HIDDEN;
#यदि defined(FFI_EXEC_STATIC_TRAMP)
शून्य ffi_closure_SYSV_alt (शून्य) FFI_HIDDEN;
शून्य ffi_closure_VFP_alt (शून्य) FFI_HIDDEN;
#endif

#ifdef FFI_GO_CLOSURES
शून्य ffi_go_closure_SYSV (शून्य) FFI_HIDDEN;
शून्य ffi_go_closure_VFP (शून्य) FFI_HIDDEN;
#endif

/* the cif must already be prep'ed */

#यदि defined(__FreeBSD__) && defined(__arm__)
#define __clear_cache(start, end) कुरु { \
		संरचना arm_sync_icache_args ua; 		\
								\
		ua.addr = (uintptr_t)(start);			\
		ua.len = (वर्ण *)(end) - (वर्ण *)start;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} यावत् (0);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure * closure,
		      ffi_cif * cif,
		      शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		      शून्य *user_data, शून्य *codeloc)
{
  शून्य (*closure_func) (शून्य) = ffi_closure_SYSV;

  यदि (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      यदि (cif->vfp_used)
	closure_func = ffi_closure_VFP;
    }
  अन्यथा यदि (cif->abi != FFI_SYSV)
    प्रत्यागम FFI_BAD_ABI;

#यदि FFI_EXEC_TRAMPOLINE_TABLE
  शून्य **config = (शून्य **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = closure_func;
#अन्यथा

# यदि defined(FFI_EXEC_STATIC_TRAMP)
  यदि (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      यदि (closure_func == ffi_closure_SYSV)
        closure_func = ffi_closure_SYSV_alt;
      अन्यथा
        closure_func = ffi_closure_VFP_alt;
      ffi_tramp_set_parms (closure->ftramp, closure_func, closure);
      गच्छ out;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  स_नकल(closure->tramp, ffi_arm_trampoline, 8);
# अन्यथा
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  स_नकल(closure->tramp, (शून्य*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_CLOSURE_OFFSET);
# endif

# यदि defined(__QNX__)
  msync (closure->tramp, 8, MS_INVALIDATE_ICACHE);	/* clear data map */
  msync (codeloc, 8, MS_INVALIDATE_ICACHE);		/* clear insn map */
# elif defined(_WIN32)
  FlushInstructionCache(GetCurrentProcess(), closure->tramp, FFI_TRAMPOLINE_SIZE);
# अन्यथा
  __clear_cache(closure->tramp, closure->tramp + 8);	/* clear data map */
  __clear_cache(codeloc, codeloc + 8);			/* clear insn map */
# endif
# ifdef _WIN32
  *(शून्य(**)(शून्य))(closure->tramp + FFI_TRAMPOLINE_CLOSURE_FUNCTION) = closure_func;
# अन्यथा
  *(शून्य (**)(शून्य))(closure->tramp + 8) = closure_func;
# endif
# यदि defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  प्रत्यागम FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *))
{
  शून्य (*closure_func) (शून्य) = ffi_go_closure_SYSV;

  यदि (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      यदि (cif->vfp_used)
	closure_func = ffi_go_closure_VFP;
    }
  अन्यथा यदि (cif->abi != FFI_SYSV)
    प्रत्यागम FFI_BAD_ABI;

  closure->tramp = closure_func;
  closure->cif = cif;
  closure->fun = fun;

  प्रत्यागम FFI_OK;
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

  यदि (elements != रिक्त)
    प्रति (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        यदि (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            यदि (ret < 0)
              अनुवर्त;
          }
        भङ्ग;
      }

  प्रत्यागम ret;
}

/* A subroutine of vfp_type_p.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

स्थिर पूर्णाङ्क
is_hfa1 (ध्रुव ffi_type *ty, पूर्णाङ्क candidate)
{
  ffi_type **elements = ty->elements;
  पूर्णाङ्क i;

  यदि (elements != रिक्त)
    प्रति (i = 0; elements[i]; ++i)
      {
        पूर्णाङ्क t = elements[i]->type;
        यदि (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
          {
            यदि (!is_hfa1 (elements[i], candidate))
              प्रत्यागम 0;
          }
        अन्यथा यदि (t != candidate)
          प्रत्यागम 0;
      }

  प्रत्यागम 1;
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
  माप_प्रकार size, ele_गिनती;

  /* Quickest tests first.  */
  candidate = ty->type;
  विकल्प (ty->type)
    {
    स्वभाव:
      प्रत्यागम 0;
    पक्ष FFI_TYPE_FLOAT:
    पक्ष FFI_TYPE_DOUBLE:
      ele_गिनती = 1;
      गच्छ done;
    पक्ष FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      यदि (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	प्रत्यागम 0;
      ele_गिनती = 2;
      गच्छ done;
    पक्ष FFI_TYPE_STRUCT:
      भङ्ग;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  size = ty->size;
  यदि (size < 4 || size > 32)
    प्रत्यागम 0;

  /* Find the type of the first non-structure member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  यदि (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_COMPLEX)
    {
      प्रति (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          यदि (candidate >= 0)
            भङ्ग;
        }
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  विकल्प (candidate)
    {
    पक्ष FFI_TYPE_FLOAT:
      ele_गिनती = size / परिमाण(भिन्न);
      यदि (size != ele_गिनती * परिमाण(भिन्न))
        प्रत्यागम 0;
      भङ्ग;
    पक्ष FFI_TYPE_DOUBLE:
      ele_गिनती = size / परिमाण(द्विगुण);
      यदि (size != ele_गिनती * परिमाण(द्विगुण))
        प्रत्यागम 0;
      भङ्ग;
    स्वभाव:
      प्रत्यागम 0;
    }
  यदि (ele_गिनती > 4)
    प्रत्यागम 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  प्रति (i = 0; elements[i]; ++i)
    {
      पूर्णाङ्क t = elements[i]->type;
      यदि (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          यदि (!is_hfa1 (elements[i], candidate))
            प्रत्यागम 0;
        }
      अन्यथा यदि (t != candidate)
        प्रत्यागम 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  प्रत्यागम (ele_गिनती << 8) | candidate;
}

स्थिर पूर्णाङ्क
place_vfp_arg (ffi_cif *cif, पूर्णाङ्क h)
{
  अचिन्हित लघु reg = cif->vfp_reg_free;
  पूर्णाङ्क align = 1, nregs = h >> 8;

  यदि ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  यदि ((reg & 1) && align == 2)
    reg++;

  यावत् (reg + nregs <= 16)
    {
      पूर्णाङ्क s, new_used = 0;
      प्रति (s = reg; s < reg + nregs; s++)
	{
	  new_used |= (1 << s);
	  यदि (cif->vfp_used & (1 << s))
	    {
	      reg += align;
	      गच्छ next_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_used |= new_used;
      cif->vfp_args[cif->vfp_nargs++] = (चिन्हित वर्ण)reg;

      /* Update vfp_reg_free. */
      यदि (cif->vfp_used & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  यावत् (cif->vfp_used & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      प्रत्यागम 0;
    next_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_used = 0xFFFF;
  प्रत्यागम 1;
}

स्थिर शून्य
layout_vfp_args (ffi_cif * cif)
{
  अचिन्हित पूर्णाङ्क i;
  /* Init VFP fields */
  cif->vfp_used = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  स_रखो (cif->vfp_args, -1, 16);	/* Init to -1. */

  प्रति (i = 0; i < cif->nargs; i++)
    {
      पूर्णाङ्क h = vfp_type_p (cif->arg_types[i]);
      यदि (h && place_vfp_arg (cif, h) == 1)
	भङ्ग;
    }
}

#यदि defined(FFI_EXEC_STATIC_TRAMP)
शून्य *
ffi_tramp_arch (माप_प्रकार *tramp_size, माप_प्रकार *map_size)
{
  बाह्य शून्य *trampoline_code_table;

  *tramp_size = ARM_TRAMP_SIZE;
  *map_size = ARM_TRAMP_MAP_SIZE;
  प्रत्यागम &trampoline_code_table;
}
#endif

#endif /* __arm__ or _M_ARM */
