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

#अगर defined(__arm__) || defined(_M_ARM)
#समावेश <fficonfig.h>
#समावेश <ffi.h>
#समावेश <ffi_common.h>
#समावेश <stdint.h>
#समावेश <stdlib.h>
#समावेश <tramp.h>
#समावेश "internal.h"

#अगर defined(_WIN32)
#घोषणा WIN32_LEAN_AND_MEAN
#समावेश <windows.h>
#पूर्ण_अगर

#अगर FFI_EXEC_TRAMPOLINE_TABLE

#अगर_घोषित __MACH__
#समावेश <mach/machine/vm_param.h>
#पूर्ण_अगर

#अन्यथा
#अगर_अघोषित _WIN32
बाह्य अचिन्हित पूर्णांक ffi_arm_trampoline[2] FFI_HIDDEN;
#अन्यथा
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
बाह्य अचिन्हित अक्षर ffi_arm_trampoline[12] FFI_HIDDEN;
#पूर्ण_अगर
#पूर्ण_अगर

#अगर defined(__FreeBSD__) && defined(__arm__)
#समावेश <sys/types.h>
#समावेश <machine/sysarch.h>
#पूर्ण_अगर

#अगर defined(__QNX__)
#समावेश <sys/mman.h>
#पूर्ण_अगर

/* Forward declares. */
अटल पूर्णांक vfp_type_p (स्थिर ffi_type *);
अटल व्योम layout_vfp_args (ffi_cif *);

अटल व्योम *
ffi_align (ffi_type *ty, व्योम *p)
{
  /* Align if necessary */
  माप_प्रकार alignment;
#अगर_घोषित _WIN32_WCE
  alignment = 4;
#अन्यथा
  alignment = ty->alignment;
  अगर (alignment < 4)
    alignment = 4;
#पूर्ण_अगर
  वापस (व्योम *) FFI_ALIGN (p, alignment);
}

अटल माप_प्रकार
ffi_put_arg (ffi_type *ty, व्योम *src, व्योम *dst)
{
  माप_प्रकार z = ty->आकार;

  चयन (ty->type)
    {
    हाल FFI_TYPE_SINT8:
      *(UINT32 *)dst = *(SINT8 *)src;
      अवरोध;
    हाल FFI_TYPE_UINT8:
      *(UINT32 *)dst = *(UINT8 *)src;
      अवरोध;
    हाल FFI_TYPE_SINT16:
      *(UINT32 *)dst = *(SINT16 *)src;
      अवरोध;
    हाल FFI_TYPE_UINT16:
      *(UINT32 *)dst = *(UINT16 *)src;
      अवरोध;

    हाल FFI_TYPE_INT:
    हाल FFI_TYPE_SINT32:
    हाल FFI_TYPE_UINT32:
    हाल FFI_TYPE_POINTER:
#अगर_अघोषित _WIN32
    हाल FFI_TYPE_FLOAT:
#पूर्ण_अगर
      *(UINT32 *)dst = *(UINT32 *)src;
      अवरोध;

#अगर_घोषित _WIN32
    // casting a float* to a UINT32* doesn't work on Windows
    हाल FFI_TYPE_FLOAT:
        *(uintptr_t *)dst = 0;
        *(भग्न *)dst = *(भग्न *)src;
        अवरोध;
#पूर्ण_अगर

    हाल FFI_TYPE_SINT64:
    हाल FFI_TYPE_UINT64:
    हाल FFI_TYPE_DOUBLE:
      *(UINT64 *)dst = *(UINT64 *)src;
      अवरोध;

    हाल FFI_TYPE_STRUCT:
    हाल FFI_TYPE_COMPLEX:
      स_नकल (dst, src, z);
      अवरोध;

    शेष:
      पात();
    }

  वापस FFI_ALIGN (z, 4);
}

/* ffi_prep_args is called once stack space has been allocated
   for the function's arguments.

   The vfp_space parameter is the load area for VFP regs, the return
   value is cif->vfp_used (word bitset of VFP regs used for passing
   arguments). These are only used for the VFP hard-float ABI.
*/
अटल व्योम
ffi_prep_args_SYSV (ffi_cif *cif, पूर्णांक flags, व्योम *rvalue,
		    व्योम **avalue, अक्षर *argp)
{
  ffi_type **arg_types = cif->arg_types;
  पूर्णांक i, n;

  अगर (flags == ARM_TYPE_STRUCT)
    {
      *(व्योम **) argp = rvalue;
      argp += 4;
    }

  क्रम (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, avalue[i], argp);
    }
}

अटल व्योम
ffi_prep_args_VFP (ffi_cif *cif, पूर्णांक flags, व्योम *rvalue,
                   व्योम **avalue, अक्षर *ढेर, अक्षर *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  पूर्णांक i, n, vi = 0;
  अक्षर *argp, *regp, *eo_regp;
  अक्षर ढेर_used = 0;
  अक्षर done_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = ढेर;
  eo_regp = argp = regp + 16;

  /* If the function returns an FFI_TYPE_STRUCT in memory,
     that address is passed in r0 to the function.  */
  अगर (flags == ARM_TYPE_STRUCT)
    {
      *(व्योम **) regp = rvalue;
      regp += 4;
    }

  क्रम (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      व्योम *a = avalue[i];
      पूर्णांक is_vfp_type = vfp_type_p (ty);

      /* Allocated in VFP registers. */
      अगर (vi < cif->vfp_nargs && is_vfp_type)
	{
	  अक्षर *vfp_slot = vfp_space + cif->vfp_args[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_slot);
	  जारी;
	}
      /* Try allocating in core registers. */
      अन्यथा अगर (!done_with_regs && !is_vfp_type)
	{
	  अक्षर *tregp = ffi_align (ty, regp);
	  माप_प्रकार आकार = ty->आकार;
	  आकार = (आकार < 4) ? 4 : आकार;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  अगर (tregp + आकार <= eo_regp)
	    {
	      regp = tregp + ffi_put_arg (ty, a, tregp);
	      done_with_regs = (regp == argp);
	      // ensure we did not write into the stack area
	      FFI_ASSERT (regp <= argp);
	      जारी;
	    }
	  /* In case there are no arguments in the stack area yet,
	     the argument is passed in the remaining core registers
	     and on the stack.  */
	  अन्यथा अगर (!ढेर_used)
	    {
	      ढेर_used = 1;
	      done_with_regs = 1;
	      argp = tregp + ffi_put_arg (ty, a, tregp);
	      FFI_ASSERT (eo_regp < argp);
	      जारी;
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
  पूर्णांक flags = 0, cabi = cif->abi;
  माप_प्रकार bytes = cif->bytes;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  अगर (cabi == FFI_VFP)
    layout_vfp_args (cif);

  /* Set the return type flag */
  चयन (cif->rtype->type)
    {
    हाल FFI_TYPE_VOID:
      flags = ARM_TYPE_VOID;
      अवरोध;

    हाल FFI_TYPE_INT:
    हाल FFI_TYPE_UINT8:
    हाल FFI_TYPE_SINT8:
    हाल FFI_TYPE_UINT16:
    हाल FFI_TYPE_SINT16:
    हाल FFI_TYPE_UINT32:
    हाल FFI_TYPE_SINT32:
    हाल FFI_TYPE_POINTER:
      flags = ARM_TYPE_INT;
      अवरोध;

    हाल FFI_TYPE_SINT64:
    हाल FFI_TYPE_UINT64:
      flags = ARM_TYPE_INT64;
      अवरोध;

    हाल FFI_TYPE_FLOAT:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_S : ARM_TYPE_INT);
      अवरोध;
    हाल FFI_TYPE_DOUBLE:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_D : ARM_TYPE_INT64);
      अवरोध;

    हाल FFI_TYPE_STRUCT:
    हाल FFI_TYPE_COMPLEX:
      अगर (cabi == FFI_VFP)
	{
	  पूर्णांक h = vfp_type_p (cif->rtype);

	  flags = ARM_TYPE_VFP_N;
	  अगर (h == 0x100 + FFI_TYPE_FLOAT)
	    flags = ARM_TYPE_VFP_S;
	  अगर (h == 0x100 + FFI_TYPE_DOUBLE)
	    flags = ARM_TYPE_VFP_D;
	  अगर (h != 0)
	      अवरोध;
	}

      /* A Composite Type not larger than 4 bytes is returned in r0.
	 A Composite Type larger than 4 bytes, or whose size cannot
	 be determined statically ... is stored in memory at an
	 address passed [in r0].  */
      अगर (cif->rtype->आकार <= 4)
	flags = ARM_TYPE_INT;
      अन्यथा
	{
	  flags = ARM_TYPE_STRUCT;
	  bytes += 4;
	}
      अवरोध;

    शेष:
      पात();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  bytes = FFI_ALIGN (bytes, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  अगर (bytes < 4*4)
    bytes = 4*4;

  cif->bytes = bytes;
  cif->flags = flags;

  वापस FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  अचिन्हित पूर्णांक nfixedargs, अचिन्हित पूर्णांक ntotalargs)
{
  /* VFP variadic calls actually use the SYSV ABI */
  अगर (cif->abi == FFI_VFP)
    cif->abi = FFI_SYSV;

  वापस ffi_prep_cif_machdep (cif);
}

/* Prototypes for assembly functions, in sysv.S.  */

काष्ठा call_frame
{
  व्योम *fp;
  व्योम *lr;
  व्योम *rvalue;
  पूर्णांक flags;
  व्योम *closure;
};

बाह्य व्योम ffi_call_SYSV (व्योम *ढेर, काष्ठा call_frame *,
			   व्योम (*fn) (व्योम)) FFI_HIDDEN;
बाह्य व्योम ffi_call_VFP (व्योम *vfp_space, काष्ठा call_frame *,
			   व्योम (*fn) (व्योम), अचिन्हित vfp_used) FFI_HIDDEN;

अटल व्योम
ffi_call_int (ffi_cif * cif, व्योम (*fn) (व्योम), व्योम *rvalue,
	      व्योम **avalue, व्योम *closure)
{
  पूर्णांक flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  माप_प्रकार bytes, rsize, vfp_आकार;
  अक्षर *ढेर, *vfp_space, *new_rvalue;
  काष्ठा call_frame *frame;

  rsize = 0;
  अगर (rvalue == शून्य)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      अगर (flags == ARM_TYPE_STRUCT)
	rsize = rtype->आकार;
      अन्यथा
	flags = ARM_TYPE_VOID;
    }
  अन्यथा अगर (flags == ARM_TYPE_VFP_N)
    {
      /* Largest case is double x 4. */
      rsize = 32;
    }
  अन्यथा अगर (flags == ARM_TYPE_INT && rtype->type == FFI_TYPE_STRUCT)
    rsize = 4;

  /* Largest case.  */
  vfp_आकार = (cif->abi == FFI_VFP && cif->vfp_used ? 8*8: 0);

  bytes = cif->bytes;
  ढेर = alloca (vfp_आकार + bytes + माप(काष्ठा call_frame) + rsize);

  vfp_space = शून्य;
  अगर (vfp_आकार)
    {
      vfp_space = ढेर;
      ढेर += vfp_आकार;
    }

  frame = (काष्ठा call_frame *)(ढेर + bytes);

  new_rvalue = rvalue;
  अगर (rsize)
    new_rvalue = (व्योम *)(frame + 1);

  frame->rvalue = new_rvalue;
  frame->flags = flags;
  frame->closure = closure;

  अगर (vfp_space)
    {
      ffi_prep_args_VFP (cif, flags, new_rvalue, avalue, ढेर, vfp_space);
      ffi_call_VFP (vfp_space, frame, fn, cif->vfp_used);
    }
  अन्यथा
    {
      ffi_prep_args_SYSV (cif, flags, new_rvalue, avalue, ढेर);
      ffi_call_SYSV (ढेर, frame, fn);
    }

  अगर (rvalue && rvalue != new_rvalue)
    स_नकल (rvalue, new_rvalue, rtype->आकार);
}

व्योम
ffi_call (ffi_cif *cif, व्योम (*fn) (व्योम), व्योम *rvalue, व्योम **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, शून्य);
}

#अगर_घोषित FFI_GO_CLOSURES
व्योम
ffi_call_go (ffi_cif *cif, व्योम (*fn) (व्योम), व्योम *rvalue,
	     व्योम **avalue, व्योम *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#पूर्ण_अगर

अटल व्योम *
ffi_prep_incoming_args_SYSV (ffi_cif *cif, व्योम *rvalue,
			     अक्षर *argp, व्योम **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  पूर्णांक i, n;

  अगर (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(व्योम **) argp;
      argp += 4;
    }
  अन्यथा
    {
      अगर (cif->rtype->आकार && cif->rtype->आकार < 4)
        *(uint32_t *) rvalue = 0;
    }

  क्रम (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      माप_प्रकार z = ty->आकार;

      argp = ffi_align (ty, argp);
#अगर_घोषित __ARMEB__
      अगर (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (व्योम *) (argp + 4 - z);
      अन्यथा
#पूर्ण_अगर
      avalue[i] = (व्योम *) argp;
      argp += z;
    }

  वापस rvalue;
}

अटल व्योम *
ffi_prep_incoming_args_VFP (ffi_cif *cif, व्योम *rvalue, अक्षर *ढेर,
			    अक्षर *vfp_space, व्योम **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  पूर्णांक i, n, vi = 0;
  अक्षर *argp, *regp, *eo_regp;
  अक्षर done_with_regs = 0;
  अक्षर ढेर_used = 0;

  regp = ढेर;
  eo_regp = argp = regp + 16;

  अगर (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(व्योम **) regp;
      regp += 4;
    }

  क्रम (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      पूर्णांक is_vfp_type = vfp_type_p (ty);
      माप_प्रकार z = ty->आकार;

      अगर (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_args[vi++] * 4;
	  जारी;
	}
      अन्यथा अगर (!done_with_regs && !is_vfp_type)
	{
	  अक्षर *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  अगर (tregp + z <= eo_regp || !ढेर_used)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#अगर_घोषित __ARMEB__
	      अगर (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (व्योम *) (tregp + 4 - z);
	      अन्यथा
#पूर्ण_अगर
	      avalue[i] = (व्योम *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      अगर (regp > eo_regp)
		{
		  FFI_ASSERT (!ढेर_used);
		  argp = regp;
		}
	      अगर (regp >= eo_regp)
		{
		  done_with_regs = 1;
		  ढेर_used = 1;
		}
	      जारी;
	    }
	}

      ढेर_used = 1;
      argp = ffi_align (ty, argp);
#अगर_घोषित __ARMEB__
      अगर (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (व्योम *) (argp + 4 - z);
      अन्यथा
#पूर्ण_अगर
      avalue[i] = (व्योम *) argp;
      argp += z;
    }

  वापस rvalue;
}

#अगर FFI_CLOSURES

काष्ठा closure_frame
{
  अक्षर vfp_space[8*8] __attribute__((aligned(8)));
  अक्षर परिणाम[8*4];
  अक्षर argp[];
};

पूर्णांक FFI_HIDDEN
ffi_closure_inner_SYSV (ffi_cif *cif,
		        व्योम (*fun) (ffi_cif *, व्योम *, व्योम **, व्योम *),
		        व्योम *user_तथ्य,
		        काष्ठा closure_frame *frame)
{
  व्योम **avalue = (व्योम **) alloca (cif->nargs * माप (व्योम *));
  व्योम *rvalue = ffi_prep_incoming_args_SYSV (cif, frame->परिणाम,
					      frame->argp, avalue);
  fun (cif, rvalue, avalue, user_तथ्य);
  वापस cif->flags;
}

पूर्णांक FFI_HIDDEN
ffi_closure_inner_VFP (ffi_cif *cif,
		       व्योम (*fun) (ffi_cif *, व्योम *, व्योम **, व्योम *),
		       व्योम *user_तथ्य,
		       काष्ठा closure_frame *frame)
{
  व्योम **avalue = (व्योम **) alloca (cif->nargs * माप (व्योम *));
  व्योम *rvalue = ffi_prep_incoming_args_VFP (cif, frame->परिणाम, frame->argp,
					     frame->vfp_space, avalue);
  fun (cif, rvalue, avalue, user_तथ्य);
  वापस cif->flags;
}

व्योम ffi_closure_SYSV (व्योम) FFI_HIDDEN;
व्योम ffi_closure_VFP (व्योम) FFI_HIDDEN;
#अगर defined(FFI_EXEC_STATIC_TRAMP)
व्योम ffi_closure_SYSV_alt (व्योम) FFI_HIDDEN;
व्योम ffi_closure_VFP_alt (व्योम) FFI_HIDDEN;
#पूर्ण_अगर

#अगर_घोषित FFI_GO_CLOSURES
व्योम ffi_go_closure_SYSV (व्योम) FFI_HIDDEN;
व्योम ffi_go_closure_VFP (व्योम) FFI_HIDDEN;
#पूर्ण_अगर

/* the cif must already be prep'ed */

#अगर defined(__FreeBSD__) && defined(__arm__)
#घोषणा __clear_cache(start, end) करो { \
		काष्ठा arm_sync_icache_args ua; 		\
								\
		ua.addr = (uintptr_t)(start);			\
		ua.len = (अक्षर *)(end) - (अक्षर *)start;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} जबतक (0);
#पूर्ण_अगर

ffi_status
ffi_prep_closure_loc (ffi_closure * closure,
		      ffi_cif * cif,
		      व्योम (*fun) (ffi_cif *, व्योम *, व्योम **, व्योम *),
		      व्योम *user_तथ्य, व्योम *codeloc)
{
  व्योम (*closure_func) (व्योम) = ffi_closure_SYSV;

  अगर (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      अगर (cif->vfp_used)
	closure_func = ffi_closure_VFP;
    }
  अन्यथा अगर (cif->abi != FFI_SYSV)
    वापस FFI_BAD_ABI;

#अगर FFI_EXEC_TRAMPOLINE_TABLE
  व्योम **config = (व्योम **)((uint8_t *)codeloc - PAGE_MAX_आकार);
  config[0] = closure;
  config[1] = closure_func;
#अन्यथा

# अगर defined(FFI_EXEC_STATIC_TRAMP)
  अगर (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      अगर (closure_func == ffi_closure_SYSV)
        closure_func = ffi_closure_SYSV_alt;
      अन्यथा
        closure_func = ffi_closure_VFP_alt;
      ffi_tramp_set_parms (closure->ftramp, closure_func, closure);
      जाओ out;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  स_नकल(closure->tramp, ffi_arm_trampoline, 8);
# अन्यथा
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  स_नकल(closure->tramp, (व्योम*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_CLOSURE_OFFSET);
# endif

# अगर defined(__QNX__)
  msync (closure->tramp, 8, MS_INVALIDATE_ICACHE);	/* clear data map */
  msync (codeloc, 8, MS_INVALIDATE_ICACHE);		/* clear insn map */
# elif defined(_WIN32)
  FlushInstructionCache(GetCurrentProcess(), closure->tramp, FFI_TRAMPOLINE_आकार);
# अन्यथा
  __clear_cache(closure->tramp, closure->tramp + 8);	/* clear data map */
  __clear_cache(codeloc, codeloc + 8);			/* clear insn map */
# endif
# ifdef _WIN32
  *(व्योम(**)(व्योम))(closure->tramp + FFI_TRAMPOLINE_CLOSURE_FUNCTION) = closure_func;
# अन्यथा
  *(व्योम (**)(व्योम))(closure->tramp + 8) = closure_func;
# endif
# अगर defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#पूर्ण_अगर

  closure->cif = cif;
  closure->fun = fun;
  closure->user_तथ्य = user_तथ्य;

  वापस FFI_OK;
}

#अगर_घोषित FFI_GO_CLOSURES
ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     व्योम (*fun) (ffi_cif *, व्योम *, व्योम **, व्योम *))
{
  व्योम (*closure_func) (व्योम) = ffi_go_closure_SYSV;

  अगर (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      अगर (cif->vfp_used)
	closure_func = ffi_go_closure_VFP;
    }
  अन्यथा अगर (cif->abi != FFI_SYSV)
    वापस FFI_BAD_ABI;

  closure->tramp = closure_func;
  closure->cif = cif;
  closure->fun = fun;

  वापस FFI_OK;
}
#पूर्ण_अगर

#पूर्ण_अगर /* FFI_CLOSURES */

/* Below are routines for VFP hard-float support. */

/* A subroutine of vfp_type_p.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

अटल पूर्णांक
is_hfa0 (स्थिर ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  पूर्णांक i, ret = -1;

  अगर (elements != शून्य)
    क्रम (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        अगर (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            अगर (ret < 0)
              जारी;
          }
        अवरोध;
      }

  वापस ret;
}

/* A subroutine of vfp_type_p.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

अटल पूर्णांक
is_hfa1 (स्थिर ffi_type *ty, पूर्णांक candidate)
{
  ffi_type **elements = ty->elements;
  पूर्णांक i;

  अगर (elements != शून्य)
    क्रम (i = 0; elements[i]; ++i)
      {
        पूर्णांक t = elements[i]->type;
        अगर (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
          {
            अगर (!is_hfa1 (elements[i], candidate))
              वापस 0;
          }
        अन्यथा अगर (t != candidate)
          वापस 0;
      }

  वापस 1;
}

/* Determine if TY is an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is a floating point scalar.

   Returns non-zero iff TY is an HFA.  The result is an encoded value where
   bits 0-7 contain the type code, and bits 8-10 contain the element count.  */

अटल पूर्णांक
vfp_type_p (स्थिर ffi_type *ty)
{
  ffi_type **elements;
  पूर्णांक candidate, i;
  माप_प्रकार आकार, ele_गिनती;

  /* Quickest tests first.  */
  candidate = ty->type;
  चयन (ty->type)
    {
    शेष:
      वापस 0;
    हाल FFI_TYPE_FLOAT:
    हाल FFI_TYPE_DOUBLE:
      ele_गिनती = 1;
      जाओ done;
    हाल FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      अगर (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	वापस 0;
      ele_गिनती = 2;
      जाओ done;
    हाल FFI_TYPE_STRUCT:
      अवरोध;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  आकार = ty->आकार;
  अगर (आकार < 4 || आकार > 32)
    वापस 0;

  /* Find the type of the first non-structure member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  अगर (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_COMPLEX)
    {
      क्रम (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          अगर (candidate >= 0)
            अवरोध;
        }
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  चयन (candidate)
    {
    हाल FFI_TYPE_FLOAT:
      ele_गिनती = आकार / माप(भग्न);
      अगर (आकार != ele_गिनती * माप(भग्न))
        वापस 0;
      अवरोध;
    हाल FFI_TYPE_DOUBLE:
      ele_गिनती = आकार / माप(द्विगुन);
      अगर (आकार != ele_गिनती * माप(द्विगुन))
        वापस 0;
      अवरोध;
    शेष:
      वापस 0;
    }
  अगर (ele_गिनती > 4)
    वापस 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  क्रम (i = 0; elements[i]; ++i)
    {
      पूर्णांक t = elements[i]->type;
      अगर (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          अगर (!is_hfa1 (elements[i], candidate))
            वापस 0;
        }
      अन्यथा अगर (t != candidate)
        वापस 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  वापस (ele_गिनती << 8) | candidate;
}

अटल पूर्णांक
place_vfp_arg (ffi_cif *cif, पूर्णांक h)
{
  अचिन्हित लघु reg = cif->vfp_reg_free;
  पूर्णांक align = 1, nregs = h >> 8;

  अगर ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  अगर ((reg & 1) && align == 2)
    reg++;

  जबतक (reg + nregs <= 16)
    {
      पूर्णांक s, new_used = 0;
      क्रम (s = reg; s < reg + nregs; s++)
	{
	  new_used |= (1 << s);
	  अगर (cif->vfp_used & (1 << s))
	    {
	      reg += align;
	      जाओ अग्र_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_used |= new_used;
      cif->vfp_args[cif->vfp_nargs++] = (चिन्हित अक्षर)reg;

      /* Update vfp_reg_free. */
      अगर (cif->vfp_used & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  जबतक (cif->vfp_used & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      वापस 0;
    अग्र_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_used = 0xFFFF;
  वापस 1;
}

अटल व्योम
layout_vfp_args (ffi_cif * cif)
{
  अचिन्हित पूर्णांक i;
  /* Init VFP fields */
  cif->vfp_used = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  स_रखो (cif->vfp_args, -1, 16);	/* Init to -1. */

  क्रम (i = 0; i < cif->nargs; i++)
    {
      पूर्णांक h = vfp_type_p (cif->arg_types[i]);
      अगर (h && place_vfp_arg (cif, h) == 1)
	अवरोध;
    }
}

#अगर defined(FFI_EXEC_STATIC_TRAMP)
व्योम *
ffi_tramp_arch (माप_प्रकार *tramp_आकार, माप_प्रकार *map_आकार)
{
  बाह्य व्योम *trampoline_code_table;

  *tramp_आकार = ARM_TRAMP_आकार;
  *map_आकार = ARM_TRAMP_MAP_आकार;
  वापस &trampoline_code_table;
}
#पूर्ण_अगर

#पूर्ण_अगर /* __arm__ or _M_ARM */
