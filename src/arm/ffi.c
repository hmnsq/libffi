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

#जर defined(__arm__) || defined(_M_ARM)
#अन्तर्भाव <fficonfig.h>
#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <tramp.h>
#अन्तर्भाव "internal.h"

#जर defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#अन्तर्भाव <windows.h>
#endif

#जर FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#अन्तर्भाव <mach/machine/vm_param.h>
#endif

#अन्यथा
#ifndef _WIN32
बाह्य अचिन्हित पूर्णांक ffi_arm_trampoline[2] FFI_HIDDEN;
#अन्यथा
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
बाह्य अचिन्हित अक्षर ffi_arm_trampoline[12] FFI_HIDDEN;
#endif
#endif

#जर defined(__FreeBSD__) && defined(__arm__)
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <machine/sysarch.h>
#endif

#जर defined(__QNX__)
#अन्तर्भाव <sys/mman.h>
#endif

/* Forward declares. */
स्थिर पूर्णांक vfp_type_p (स्थिरांक ffi_type *);
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
  जर (alignment < 4)
    alignment = 4;
#endif
  परत (शून्य *) FFI_ALIGN (p, alignment);
}

स्थिर माप_प्रकार
ffi_put_arg (ffi_type *ty, शून्य *src, शून्य *dst)
{
  माप_प्रकार z = ty->size;

  निवड (ty->type)
    {
    स्थिती FFI_TYPE_SINT8:
      *(UINT32 *)dst = *(SINT8 *)src;
      थांबा;
    स्थिती FFI_TYPE_UINT8:
      *(UINT32 *)dst = *(UINT8 *)src;
      थांबा;
    स्थिती FFI_TYPE_SINT16:
      *(UINT32 *)dst = *(SINT16 *)src;
      थांबा;
    स्थिती FFI_TYPE_UINT16:
      *(UINT32 *)dst = *(UINT16 *)src;
      थांबा;

    स्थिती FFI_TYPE_INT:
    स्थिती FFI_TYPE_SINT32:
    स्थिती FFI_TYPE_UINT32:
    स्थिती FFI_TYPE_POINTER:
#ifndef _WIN32
    स्थिती FFI_TYPE_FLOAT:
#endif
      *(UINT32 *)dst = *(UINT32 *)src;
      थांबा;

#ifdef _WIN32
    // casting a float* to a UINT32* doesn't work on Windows
    स्थिती FFI_TYPE_FLOAT:
        *(uintptr_t *)dst = 0;
        *(दशांश *)dst = *(दशांश *)src;
        थांबा;
#endif

    स्थिती FFI_TYPE_SINT64:
    स्थिती FFI_TYPE_UINT64:
    स्थिती FFI_TYPE_DOUBLE:
      *(UINT64 *)dst = *(UINT64 *)src;
      थांबा;

    स्थिती FFI_TYPE_STRUCT:
    स्थिती FFI_TYPE_COMPLEX:
      स_नकल (dst, src, z);
      थांबा;

    पूर्वनिर्धारित:
      पात();
    }

  परत FFI_ALIGN (z, 4);
}

/* ffi_prep_args is called once stack space has been allocated
   for the function's arguments.

   The vfp_space parameter is the load area for VFP regs, the return
   value is cif->vfp_used (word bitset of VFP regs used for passing
   arguments). These are only used for the VFP hard-float ABI.
*/
स्थिर शून्य
ffi_prep_args_SYSV (ffi_cif *cif, पूर्णांक flags, शून्य *rvalue,
		    शून्य **avalue, अक्षर *argp)
{
  ffi_type **arg_types = cif->arg_types;
  पूर्णांक i, n;

  जर (flags == ARM_TYPE_STRUCT)
    {
      *(शून्य **) argp = rvalue;
      argp += 4;
    }

  साठी (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, avalue[i], argp);
    }
}

स्थिर शून्य
ffi_prep_args_VFP (ffi_cif *cif, पूर्णांक flags, शून्य *rvalue,
                   शून्य **avalue, अक्षर *चळत, अक्षर *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  पूर्णांक i, n, vi = 0;
  अक्षर *argp, *regp, *eo_regp;
  अक्षर चळत_used = 0;
  अक्षर done_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = चळत;
  eo_regp = argp = regp + 16;

  /* If the function returns an FFI_TYPE_STRUCT in memory,
     that address is passed in r0 to the function.  */
  जर (flags == ARM_TYPE_STRUCT)
    {
      *(शून्य **) regp = rvalue;
      regp += 4;
    }

  साठी (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      शून्य *a = avalue[i];
      पूर्णांक is_vfp_type = vfp_type_p (ty);

      /* Allocated in VFP registers. */
      जर (vi < cif->vfp_nargs && is_vfp_type)
	{
	  अक्षर *vfp_slot = vfp_space + cif->vfp_args[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_slot);
	  सुरूठेवा;
	}
      /* Try allocating in core registers. */
      अन्यथा जर (!done_with_regs && !is_vfp_type)
	{
	  अक्षर *tregp = ffi_align (ty, regp);
	  माप_प्रकार size = ty->size;
	  size = (size < 4) ? 4 : size;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  जर (tregp + size <= eo_regp)
	    {
	      regp = tregp + ffi_put_arg (ty, a, tregp);
	      done_with_regs = (regp == argp);
	      // ensure we did not write into the stack area
	      FFI_ASSERT (regp <= argp);
	      सुरूठेवा;
	    }
	  /* In case there are no arguments in the stack area yet,
	     the argument is passed in the remaining core registers
	     and on the stack.  */
	  अन्यथा जर (!चळत_used)
	    {
	      चळत_used = 1;
	      done_with_regs = 1;
	      argp = tregp + ffi_put_arg (ty, a, tregp);
	      FFI_ASSERT (eo_regp < argp);
	      सुरूठेवा;
	    }
	}
      /* Base case, arguments are passed on the stack */
      चळत_used = 1;
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, a, argp);
    }
}

/* Perform machine dependent cif processing */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  पूर्णांक flags = 0, cabi = cif->abi;
  माप_प्रकार bytes = cif->bytes;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  जर (cabi == FFI_VFP)
    layout_vfp_args (cif);

  /* Set the return type flag */
  निवड (cif->rtype->type)
    {
    स्थिती FFI_TYPE_VOID:
      flags = ARM_TYPE_VOID;
      थांबा;

    स्थिती FFI_TYPE_INT:
    स्थिती FFI_TYPE_UINT8:
    स्थिती FFI_TYPE_SINT8:
    स्थिती FFI_TYPE_UINT16:
    स्थिती FFI_TYPE_SINT16:
    स्थिती FFI_TYPE_UINT32:
    स्थिती FFI_TYPE_SINT32:
    स्थिती FFI_TYPE_POINTER:
      flags = ARM_TYPE_INT;
      थांबा;

    स्थिती FFI_TYPE_SINT64:
    स्थिती FFI_TYPE_UINT64:
      flags = ARM_TYPE_INT64;
      थांबा;

    स्थिती FFI_TYPE_FLOAT:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_S : ARM_TYPE_INT);
      थांबा;
    स्थिती FFI_TYPE_DOUBLE:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_D : ARM_TYPE_INT64);
      थांबा;

    स्थिती FFI_TYPE_STRUCT:
    स्थिती FFI_TYPE_COMPLEX:
      जर (cabi == FFI_VFP)
	{
	  पूर्णांक h = vfp_type_p (cif->rtype);

	  flags = ARM_TYPE_VFP_N;
	  जर (h == 0x100 + FFI_TYPE_FLOAT)
	    flags = ARM_TYPE_VFP_S;
	  जर (h == 0x100 + FFI_TYPE_DOUBLE)
	    flags = ARM_TYPE_VFP_D;
	  जर (h != 0)
	      थांबा;
	}

      /* A Composite Type not larger than 4 bytes is returned in r0.
	 A Composite Type larger than 4 bytes, or whose size cannot
	 be determined statically ... is stored in memory at an
	 address passed [in r0].  */
      जर (cif->rtype->size <= 4)
	flags = ARM_TYPE_INT;
      अन्यथा
	{
	  flags = ARM_TYPE_STRUCT;
	  bytes += 4;
	}
      थांबा;

    पूर्वनिर्धारित:
      पात();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  bytes = FFI_ALIGN (bytes, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  जर (bytes < 4*4)
    bytes = 4*4;

  cif->bytes = bytes;
  cif->flags = flags;

  परत FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  अचिन्हित पूर्णांक nfixedargs, अचिन्हित पूर्णांक ntotalargs)
{
  /* VFP variadic calls actually use the SYSV ABI */
  जर (cif->abi == FFI_VFP)
    cif->abi = FFI_SYSV;

  परत ffi_prep_cif_machdep (cif);
}

/* Prototypes for assembly functions, in sysv.S.  */

संरचना call_frame
{
  शून्य *fp;
  शून्य *lr;
  शून्य *rvalue;
  पूर्णांक flags;
  शून्य *closure;
};

बाह्य शून्य ffi_call_SYSV (शून्य *चळत, संरचना call_frame *,
			   शून्य (*fn) (शून्य)) FFI_HIDDEN;
बाह्य शून्य ffi_call_VFP (शून्य *vfp_space, संरचना call_frame *,
			   शून्य (*fn) (शून्य), अचिन्हित vfp_used) FFI_HIDDEN;

स्थिर शून्य
ffi_call_int (ffi_cif * cif, शून्य (*fn) (शून्य), शून्य *rvalue,
	      शून्य **avalue, शून्य *closure)
{
  पूर्णांक flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  माप_प्रकार bytes, rsize, vfp_size;
  अक्षर *चळत, *vfp_space, *new_rvalue;
  संरचना call_frame *frame;

  rsize = 0;
  जर (rvalue == रिक्त)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      जर (flags == ARM_TYPE_STRUCT)
	rsize = rtype->size;
      अन्यथा
	flags = ARM_TYPE_VOID;
    }
  अन्यथा जर (flags == ARM_TYPE_VFP_N)
    {
      /* Largest case is double x 4. */
      rsize = 32;
    }
  अन्यथा जर (flags == ARM_TYPE_INT && rtype->type == FFI_TYPE_STRUCT)
    rsize = 4;

  /* Largest case.  */
  vfp_size = (cif->abi == FFI_VFP && cif->vfp_used ? 8*8: 0);

  bytes = cif->bytes;
  चळत = alloca (vfp_size + bytes + आकार(संरचना call_frame) + rsize);

  vfp_space = रिक्त;
  जर (vfp_size)
    {
      vfp_space = चळत;
      चळत += vfp_size;
    }

  frame = (संरचना call_frame *)(चळत + bytes);

  new_rvalue = rvalue;
  जर (rsize)
    new_rvalue = (शून्य *)(frame + 1);

  frame->rvalue = new_rvalue;
  frame->flags = flags;
  frame->closure = closure;

  जर (vfp_space)
    {
      ffi_prep_args_VFP (cif, flags, new_rvalue, avalue, चळत, vfp_space);
      ffi_call_VFP (vfp_space, frame, fn, cif->vfp_used);
    }
  अन्यथा
    {
      ffi_prep_args_SYSV (cif, flags, new_rvalue, avalue, चळत);
      ffi_call_SYSV (चळत, frame, fn);
    }

  जर (rvalue && rvalue != new_rvalue)
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
			     अक्षर *argp, शून्य **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  पूर्णांक i, n;

  जर (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(शून्य **) argp;
      argp += 4;
    }
  अन्यथा
    {
      जर (cif->rtype->size && cif->rtype->size < 4)
        *(uint32_t *) rvalue = 0;
    }

  साठी (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      माप_प्रकार z = ty->size;

      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      जर (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (शून्य *) (argp + 4 - z);
      अन्यथा
#endif
      avalue[i] = (शून्य *) argp;
      argp += z;
    }

  परत rvalue;
}

स्थिर शून्य *
ffi_prep_incoming_args_VFP (ffi_cif *cif, शून्य *rvalue, अक्षर *चळत,
			    अक्षर *vfp_space, शून्य **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  पूर्णांक i, n, vi = 0;
  अक्षर *argp, *regp, *eo_regp;
  अक्षर done_with_regs = 0;
  अक्षर चळत_used = 0;

  regp = चळत;
  eo_regp = argp = regp + 16;

  जर (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(शून्य **) regp;
      regp += 4;
    }

  साठी (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      पूर्णांक is_vfp_type = vfp_type_p (ty);
      माप_प्रकार z = ty->size;

      जर (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_args[vi++] * 4;
	  सुरूठेवा;
	}
      अन्यथा जर (!done_with_regs && !is_vfp_type)
	{
	  अक्षर *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  जर (tregp + z <= eo_regp || !चळत_used)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#ifdef __ARMEB__
	      जर (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (शून्य *) (tregp + 4 - z);
	      अन्यथा
#endif
	      avalue[i] = (शून्य *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      जर (regp > eo_regp)
		{
		  FFI_ASSERT (!चळत_used);
		  argp = regp;
		}
	      जर (regp >= eo_regp)
		{
		  done_with_regs = 1;
		  चळत_used = 1;
		}
	      सुरूठेवा;
	    }
	}

      चळत_used = 1;
      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      जर (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (शून्य *) (argp + 4 - z);
      अन्यथा
#endif
      avalue[i] = (शून्य *) argp;
      argp += z;
    }

  परत rvalue;
}

#जर FFI_CLOSURES

संरचना closure_frame
{
  अक्षर vfp_space[8*8] __attribute__((aligned(8)));
  अक्षर result[8*4];
  अक्षर argp[];
};

पूर्णांक FFI_HIDDEN
ffi_closure_inner_SYSV (ffi_cif *cif,
		        शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		        शून्य *user_data,
		        संरचना closure_frame *frame)
{
  शून्य **avalue = (शून्य **) alloca (cif->nargs * आकार (शून्य *));
  शून्य *rvalue = ffi_prep_incoming_args_SYSV (cif, frame->result,
					      frame->argp, avalue);
  fun (cif, rvalue, avalue, user_data);
  परत cif->flags;
}

पूर्णांक FFI_HIDDEN
ffi_closure_inner_VFP (ffi_cif *cif,
		       शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		       शून्य *user_data,
		       संरचना closure_frame *frame)
{
  शून्य **avalue = (शून्य **) alloca (cif->nargs * आकार (शून्य *));
  शून्य *rvalue = ffi_prep_incoming_args_VFP (cif, frame->result, frame->argp,
					     frame->vfp_space, avalue);
  fun (cif, rvalue, avalue, user_data);
  परत cif->flags;
}

शून्य ffi_closure_SYSV (शून्य) FFI_HIDDEN;
शून्य ffi_closure_VFP (शून्य) FFI_HIDDEN;
#जर defined(FFI_EXEC_STATIC_TRAMP)
शून्य ffi_closure_SYSV_alt (शून्य) FFI_HIDDEN;
शून्य ffi_closure_VFP_alt (शून्य) FFI_HIDDEN;
#endif

#ifdef FFI_GO_CLOSURES
शून्य ffi_go_closure_SYSV (शून्य) FFI_HIDDEN;
शून्य ffi_go_closure_VFP (शून्य) FFI_HIDDEN;
#endif

/* the cif must already be prep'ed */

#जर defined(__FreeBSD__) && defined(__arm__)
#define __clear_cache(start, end) करा { \
		संरचना arm_sync_icache_args ua; 		\
								\
		ua.addr = (uintptr_t)(start);			\
		ua.len = (अक्षर *)(end) - (अक्षर *)start;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} पर्यंत (0);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure * closure,
		      ffi_cif * cif,
		      शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		      शून्य *user_data, शून्य *codeloc)
{
  शून्य (*closure_func) (शून्य) = ffi_closure_SYSV;

  जर (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      जर (cif->vfp_used)
	closure_func = ffi_closure_VFP;
    }
  अन्यथा जर (cif->abi != FFI_SYSV)
    परत FFI_BAD_ABI;

#जर FFI_EXEC_TRAMPOLINE_TABLE
  शून्य **config = (शून्य **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = closure_func;
#अन्यथा

# जर defined(FFI_EXEC_STATIC_TRAMP)
  जर (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      जर (closure_func == ffi_closure_SYSV)
        closure_func = ffi_closure_SYSV_alt;
      अन्यथा
        closure_func = ffi_closure_VFP_alt;
      ffi_tramp_set_parms (closure->ftramp, closure_func, closure);
      जा out;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  स_नकल(closure->tramp, ffi_arm_trampoline, 8);
# अन्यथा
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  स_नकल(closure->tramp, (शून्य*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_CLOSURE_OFFSET);
# endif

# जर defined(__QNX__)
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
# जर defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  परत FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *))
{
  शून्य (*closure_func) (शून्य) = ffi_go_closure_SYSV;

  जर (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      जर (cif->vfp_used)
	closure_func = ffi_go_closure_VFP;
    }
  अन्यथा जर (cif->abi != FFI_SYSV)
    परत FFI_BAD_ABI;

  closure->tramp = closure_func;
  closure->cif = cif;
  closure->fun = fun;

  परत FFI_OK;
}
#endif

#endif /* FFI_CLOSURES */

/* Below are routines for VFP hard-float support. */

/* A subroutine of vfp_type_p.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

स्थिर पूर्णांक
is_hfa0 (स्थिरांक ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  पूर्णांक i, ret = -1;

  जर (elements != रिक्त)
    साठी (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        जर (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            जर (ret < 0)
              सुरूठेवा;
          }
        थांबा;
      }

  परत ret;
}

/* A subroutine of vfp_type_p.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

स्थिर पूर्णांक
is_hfa1 (स्थिरांक ffi_type *ty, पूर्णांक candidate)
{
  ffi_type **elements = ty->elements;
  पूर्णांक i;

  जर (elements != रिक्त)
    साठी (i = 0; elements[i]; ++i)
      {
        पूर्णांक t = elements[i]->type;
        जर (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
          {
            जर (!is_hfa1 (elements[i], candidate))
              परत 0;
          }
        अन्यथा जर (t != candidate)
          परत 0;
      }

  परत 1;
}

/* Determine if TY is an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is a floating point scalar.

   Returns non-zero iff TY is an HFA.  The result is an encoded value where
   bits 0-7 contain the type code, and bits 8-10 contain the element count.  */

स्थिर पूर्णांक
vfp_type_p (स्थिरांक ffi_type *ty)
{
  ffi_type **elements;
  पूर्णांक candidate, i;
  माप_प्रकार size, ele_मोजणी;

  /* Quickest tests first.  */
  candidate = ty->type;
  निवड (ty->type)
    {
    पूर्वनिर्धारित:
      परत 0;
    स्थिती FFI_TYPE_FLOAT:
    स्थिती FFI_TYPE_DOUBLE:
      ele_मोजणी = 1;
      जा done;
    स्थिती FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      जर (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	परत 0;
      ele_मोजणी = 2;
      जा done;
    स्थिती FFI_TYPE_STRUCT:
      थांबा;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  size = ty->size;
  जर (size < 4 || size > 32)
    परत 0;

  /* Find the type of the first non-structure member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  जर (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_COMPLEX)
    {
      साठी (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          जर (candidate >= 0)
            थांबा;
        }
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  निवड (candidate)
    {
    स्थिती FFI_TYPE_FLOAT:
      ele_मोजणी = size / आकार(दशांश);
      जर (size != ele_मोजणी * आकार(दशांश))
        परत 0;
      थांबा;
    स्थिती FFI_TYPE_DOUBLE:
      ele_मोजणी = size / आकार(द्विगुण);
      जर (size != ele_मोजणी * आकार(द्विगुण))
        परत 0;
      थांबा;
    पूर्वनिर्धारित:
      परत 0;
    }
  जर (ele_मोजणी > 4)
    परत 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  साठी (i = 0; elements[i]; ++i)
    {
      पूर्णांक t = elements[i]->type;
      जर (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          जर (!is_hfa1 (elements[i], candidate))
            परत 0;
        }
      अन्यथा जर (t != candidate)
        परत 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  परत (ele_मोजणी << 8) | candidate;
}

स्थिर पूर्णांक
place_vfp_arg (ffi_cif *cif, पूर्णांक h)
{
  अचिन्हित लघु reg = cif->vfp_reg_free;
  पूर्णांक align = 1, nregs = h >> 8;

  जर ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  जर ((reg & 1) && align == 2)
    reg++;

  पर्यंत (reg + nregs <= 16)
    {
      पूर्णांक s, new_used = 0;
      साठी (s = reg; s < reg + nregs; s++)
	{
	  new_used |= (1 << s);
	  जर (cif->vfp_used & (1 << s))
	    {
	      reg += align;
	      जा next_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_used |= new_used;
      cif->vfp_args[cif->vfp_nargs++] = (चिन्हित अक्षर)reg;

      /* Update vfp_reg_free. */
      जर (cif->vfp_used & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  पर्यंत (cif->vfp_used & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      परत 0;
    next_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_used = 0xFFFF;
  परत 1;
}

स्थिर शून्य
layout_vfp_args (ffi_cif * cif)
{
  अचिन्हित पूर्णांक i;
  /* Init VFP fields */
  cif->vfp_used = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  स_रखो (cif->vfp_args, -1, 16);	/* Init to -1. */

  साठी (i = 0; i < cif->nargs; i++)
    {
      पूर्णांक h = vfp_type_p (cif->arg_types[i]);
      जर (h && place_vfp_arg (cif, h) == 1)
	थांबा;
    }
}

#जर defined(FFI_EXEC_STATIC_TRAMP)
शून्य *
ffi_tramp_arch (माप_प्रकार *tramp_size, माप_प्रकार *map_size)
{
  बाह्य शून्य *trampoline_code_table;

  *tramp_size = ARM_TRAMP_SIZE;
  *map_size = ARM_TRAMP_MAP_SIZE;
  परत &trampoline_code_table;
}
#endif

#endif /* __arm__ or _M_ARM */
