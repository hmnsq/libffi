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
#परिभाषय WIN32_LEAN_AND_माध्यम्
#अन्तर्भाव <windows.h>
#यदि_अन्तः

#यदि FFI_EXEC_TRAMPOLINE_सारणी

#यदि_परिभाषितम् __MACH__
#अन्तर्भाव <mach/machine/vm_param.h>
#यदि_अन्तः

#अन्यथा
#यदि_अपरिभाषितम् _WIN32
बाह्य अचिह्नित पूर्णाङ्क ffi_arm_trampoline[2] FFI_HIDDEN;
#अन्यथा
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
बाह्य अचिह्नित वर्ण ffi_arm_trampoline[12] FFI_HIDDEN;
#यदि_अन्तः
#यदि_अन्तः

#यदि defined(__FreeBSD__) && defined(__arm__)
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <machine/sysarch.h>
#यदि_अन्तः

#यदि defined(__QNX__)
#अन्तर्भाव <sys/mman.h>
#यदि_अन्तः

/* Forward declares. */
स्थिर पूर्णाङ्क vfp_type_p (ध्रुव ffi_type *);
स्थिर शून्य layout_vfp_तर्काः (ffi_cif *);

स्थिर शून्य *
ffi_align (ffi_type *ty, शून्य *p)
{
  /* Align if necessary */
  size_t alignment;
#यदि_परिभाषितम् _WIN32_WCE
  alignment = 4;
#अन्यथा
  alignment = ty->alignment;
  यदि (alignment < 4)
    alignment = 4;
#यदि_अन्तः
  प्रत्यागम (शून्य *) FFI_ALIGN (p, alignment);
}

स्थिर size_t
ffi_put_arg (ffi_type *ty, शून्य *src, शून्य *dst)
{
  size_t z = ty->आकार;

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
#यदि_अपरिभाषितम् _WIN32
    पक्ष FFI_TYPE_FLOAT:
#यदि_अन्तः
      *(UINT32 *)dst = *(UINT32 *)src;
      भङ्ग;

#यदि_परिभाषितम् _WIN32
    // casting a float* to a UINT32* doesn't work on Windows
    पक्ष FFI_TYPE_FLOAT:
        *(uintptr_t *)dst = 0;
        *(भिन्न *)dst = *(भिन्न *)src;
        भङ्ग;
#यदि_अन्तः

    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_DOUBLE:
      *(UINT64 *)dst = *(UINT64 *)src;
      भङ्ग;

    पक्ष FFI_TYPE_STRUCT:
    पक्ष FFI_TYPE_COMPLEX:
      स्मृतिं_प्रतिलिख (dst, src, z);
      भङ्ग;

    स्वभाव:
      abort();
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
ffi_prep_तर्काः_SYSV (ffi_cif *cif, पूर्णाङ्क flags, शून्य *rvalue,
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
ffi_prep_तर्काः_VFP (ffi_cif *cif, पूर्णाङ्क flags, शून्य *rvalue,
                   शून्य **avalue, वर्ण *राशिः, वर्ण *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  पूर्णाङ्क i, n, vi = 0;
  वर्ण *argp, *regp, *eo_regp;
  वर्ण राशिः_प्रयुक्तम् = 0;
  वर्ण समाप्तम्_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = राशिः;
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
	  वर्ण *vfp_slot = vfp_space + cif->vfp_तर्काः[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_slot);
	  अनुवर्त;
	}
      /* Try allocating in core registers. */
      अन्यथा यदि (!समाप्तम्_with_regs && !is_vfp_type)
	{
	  वर्ण *tregp = ffi_align (ty, regp);
	  size_t आकार = ty->आकार;
	  आकार = (आकार < 4) ? 4 : आकार;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  यदि (tregp + आकार <= eo_regp)
	    {
	      regp = tregp + ffi_put_arg (ty, a, tregp);
	      समाप्तम्_with_regs = (regp == argp);
	      // ensure we did not write into the stack area
	      FFI_ASSERT (regp <= argp);
	      अनुवर्त;
	    }
	  /* In case there are no arguments in the stack area yet,
	     the argument is passed in the remaining core registers
	     and on the stack.  */
	  अन्यथा यदि (!राशिः_प्रयुक्तम्)
	    {
	      राशिः_प्रयुक्तम् = 1;
	      समाप्तम्_with_regs = 1;
	      argp = tregp + ffi_put_arg (ty, a, tregp);
	      FFI_ASSERT (eo_regp < argp);
	      अनुवर्त;
	    }
	}
      /* Base case, arguments are passed on the stack */
      राशिः_प्रयुक्तम् = 1;
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, a, argp);
    }
}

/* Perform machine dependent cif processing */
ffi_स्थितिः FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  पूर्णाङ्क flags = 0, cabi = cif->abi;
  size_t अष्टकानि = cif->अष्टकानि;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  यदि (cabi == FFI_VFP)
    layout_vfp_तर्काः (cif);

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
      यदि (cif->rtype->आकार <= 4)
	flags = ARM_TYPE_INT;
      अन्यथा
	{
	  flags = ARM_TYPE_STRUCT;
	  अष्टकानि += 4;
	}
      भङ्ग;

    स्वभाव:
      abort();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  अष्टकानि = FFI_ALIGN (अष्टकानि, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  यदि (अष्टकानि < 4*4)
    अष्टकानि = 4*4;

  cif->अष्टकानि = अष्टकानि;
  cif->flags = flags;

  प्रत्यागम FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_स्थितिः FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  अचिह्नित पूर्णाङ्क nfixedargs, अचिह्नित पूर्णाङ्क ntotalargs)
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
  शून्य *संवृतिः;
};

बाह्य शून्य ffi_call_SYSV (शून्य *राशिः, संरचना call_frame *,
			   शून्य (*fn) (शून्य)) FFI_HIDDEN;
बाह्य शून्य ffi_call_VFP (शून्य *vfp_space, संरचना call_frame *,
			   शून्य (*fn) (शून्य), अचिह्नित vfp_प्रयुक्तम्) FFI_HIDDEN;

स्थिर शून्य
ffi_call_int (ffi_cif * cif, शून्य (*fn) (शून्य), शून्य *rvalue,
	      शून्य **avalue, शून्य *संवृतिः)
{
  पूर्णाङ्क flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  size_t अष्टकानि, rsize, vfp_आकार;
  वर्ण *राशिः, *vfp_space, *new_rvalue;
  संरचना call_frame *frame;

  rsize = 0;
  यदि (rvalue == रिक्त)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      यदि (flags == ARM_TYPE_STRUCT)
	rsize = rtype->आकार;
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
  vfp_आकार = (cif->abi == FFI_VFP && cif->vfp_प्रयुक्तम् ? 8*8: 0);

  अष्टकानि = cif->अष्टकानि;
  राशिः = alloca (vfp_आकार + अष्टकानि + परिमाण(संरचना call_frame) + rsize);

  vfp_space = रिक्त;
  यदि (vfp_आकार)
    {
      vfp_space = राशिः;
      राशिः += vfp_आकार;
    }

  frame = (संरचना call_frame *)(राशिः + अष्टकानि);

  new_rvalue = rvalue;
  यदि (rsize)
    new_rvalue = (शून्य *)(frame + 1);

  frame->rvalue = new_rvalue;
  frame->flags = flags;
  frame->संवृतिः = संवृतिः;

  यदि (vfp_space)
    {
      ffi_prep_तर्काः_VFP (cif, flags, new_rvalue, avalue, राशिः, vfp_space);
      ffi_call_VFP (vfp_space, frame, fn, cif->vfp_प्रयुक्तम्);
    }
  अन्यथा
    {
      ffi_prep_तर्काः_SYSV (cif, flags, new_rvalue, avalue, राशिः);
      ffi_call_SYSV (राशिः, frame, fn);
    }

  यदि (rvalue && rvalue != new_rvalue)
    स्मृतिं_प्रतिलिख (rvalue, new_rvalue, rtype->आकार);
}

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, रिक्त);
}

#यदि_परिभाषितम् FFI_GO_CLOSURES
शून्य
ffi_call_go (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue,
	     शून्य **avalue, शून्य *संवृतिः)
{
  ffi_call_int (cif, fn, rvalue, avalue, संवृतिः);
}
#यदि_अन्तः

स्थिर शून्य *
ffi_prep_incoming_तर्काः_SYSV (ffi_cif *cif, शून्य *rvalue,
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
      यदि (cif->rtype->आकार && cif->rtype->आकार < 4)
        *(uint32_t *) rvalue = 0;
    }

  प्रति (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->आकार;

      argp = ffi_align (ty, argp);
#यदि_परिभाषितम् __ARMEB__
      यदि (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (शून्य *) (argp + 4 - z);
      अन्यथा
#यदि_अन्तः
      avalue[i] = (शून्य *) argp;
      argp += z;
    }

  प्रत्यागम rvalue;
}

स्थिर शून्य *
ffi_prep_incoming_तर्काः_VFP (ffi_cif *cif, शून्य *rvalue, वर्ण *राशिः,
			    वर्ण *vfp_space, शून्य **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  पूर्णाङ्क i, n, vi = 0;
  वर्ण *argp, *regp, *eo_regp;
  वर्ण समाप्तम्_with_regs = 0;
  वर्ण राशिः_प्रयुक्तम् = 0;

  regp = राशिः;
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
      size_t z = ty->आकार;

      यदि (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_तर्काः[vi++] * 4;
	  अनुवर्त;
	}
      अन्यथा यदि (!समाप्तम्_with_regs && !is_vfp_type)
	{
	  वर्ण *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  यदि (tregp + z <= eo_regp || !राशिः_प्रयुक्तम्)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#यदि_परिभाषितम् __ARMEB__
	      यदि (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (शून्य *) (tregp + 4 - z);
	      अन्यथा
#यदि_अन्तः
	      avalue[i] = (शून्य *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      यदि (regp > eo_regp)
		{
		  FFI_ASSERT (!राशिः_प्रयुक्तम्);
		  argp = regp;
		}
	      यदि (regp >= eo_regp)
		{
		  समाप्तम्_with_regs = 1;
		  राशिः_प्रयुक्तम् = 1;
		}
	      अनुवर्त;
	    }
	}

      राशिः_प्रयुक्तम् = 1;
      argp = ffi_align (ty, argp);
#यदि_परिभाषितम् __ARMEB__
      यदि (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (शून्य *) (argp + 4 - z);
      अन्यथा
#यदि_अन्तः
      avalue[i] = (शून्य *) argp;
      argp += z;
    }

  प्रत्यागम rvalue;
}

#यदि FFI_CLOSURES

संरचना संवृतिः_frame
{
  वर्ण vfp_space[8*8] __attribute__((aligned(8)));
  वर्ण परिणाम[8*4];
  वर्ण argp[];
};

पूर्णाङ्क FFI_HIDDEN
ffi_संवृतिः_inner_SYSV (ffi_cif *cif,
		        शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		        शून्य *user_दत्त,
		        संरचना संवृतिः_frame *frame)
{
  शून्य **avalue = (शून्य **) alloca (cif->nargs * परिमाण (शून्य *));
  शून्य *rvalue = ffi_prep_incoming_तर्काः_SYSV (cif, frame->परिणाम,
					      frame->argp, avalue);
  fun (cif, rvalue, avalue, user_दत्त);
  प्रत्यागम cif->flags;
}

पूर्णाङ्क FFI_HIDDEN
ffi_संवृतिः_inner_VFP (ffi_cif *cif,
		       शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		       शून्य *user_दत्त,
		       संरचना संवृतिः_frame *frame)
{
  शून्य **avalue = (शून्य **) alloca (cif->nargs * परिमाण (शून्य *));
  शून्य *rvalue = ffi_prep_incoming_तर्काः_VFP (cif, frame->परिणाम, frame->argp,
					     frame->vfp_space, avalue);
  fun (cif, rvalue, avalue, user_दत्त);
  प्रत्यागम cif->flags;
}

शून्य ffi_संवृतिः_SYSV (शून्य) FFI_HIDDEN;
शून्य ffi_संवृतिः_VFP (शून्य) FFI_HIDDEN;
#यदि defined(FFI_EXEC_STATIC_TRAMP)
शून्य ffi_संवृतिः_SYSV_alt (शून्य) FFI_HIDDEN;
शून्य ffi_संवृतिः_VFP_alt (शून्य) FFI_HIDDEN;
#यदि_अन्तः

#यदि_परिभाषितम् FFI_GO_CLOSURES
शून्य ffi_go_संवृतिः_SYSV (शून्य) FFI_HIDDEN;
शून्य ffi_go_संवृतिः_VFP (शून्य) FFI_HIDDEN;
#यदि_अन्तः

/* the cif must already be prep'ed */

#यदि defined(__FreeBSD__) && defined(__arm__)
#परिभाषय __clear_cache(start, अन्तः) कुरु { \
		संरचना arm_sync_icache_तर्काः ua; 		\
								\
		ua.addr = (uintptr_t)(start);			\
		ua.len = (वर्ण *)(अन्तः) - (वर्ण *)start;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} यावत् (0);
#यदि_अन्तः

ffi_स्थितिः
ffi_prep_संवृतिः_loc (ffi_संवृतिः * संवृतिः,
		      ffi_cif * cif,
		      शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		      शून्य *user_दत्त, शून्य *codeloc)
{
  शून्य (*संवृतिः_func) (शून्य) = ffi_संवृतिः_SYSV;

  यदि (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      यदि (cif->vfp_प्रयुक्तम्)
	संवृतिः_func = ffi_संवृतिः_VFP;
    }
  अन्यथा यदि (cif->abi != FFI_SYSV)
    प्रत्यागम FFI_BAD_ABI;

#यदि FFI_EXEC_TRAMPOLINE_सारणी
  शून्य **config = (शून्य **)((uint8_t *)codeloc - PAGE_MAX_आकार);
  config[0] = संवृतिः;
  config[1] = संवृतिः_func;
#अन्यथा

# यदि defined(FFI_EXEC_STATIC_TRAMP)
  यदि (ffi_tramp_is_present(संवृतिः))
    {
      /* Initialize the static trampoline's parameters. */
      यदि (संवृतिः_func == ffi_संवृतिः_SYSV)
        संवृतिः_func = ffi_संवृतिः_SYSV_alt;
      अन्यथा
        संवृतिः_func = ffi_संवृतिः_VFP_alt;
      ffi_tramp_समुच्चयः_parms (संवृतिः->ftramp, संवृतिः_func, संवृतिः);
      गच्छ बहिर्गमनम्;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  स्मृतिं_प्रतिलिख(संवृतिः->tramp, ffi_arm_trampoline, 8);
# अन्यथा
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  स्मृतिं_प्रतिलिख(संवृतिः->tramp, (शून्य*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_संवृतिः_विस्थापनम्);
# endif

# यदि defined(__QNX__)
  msync (संवृतिः->tramp, 8, MS_INVALIDATE_ICACHE);	/* clear data map */
  msync (codeloc, 8, MS_INVALIDATE_ICACHE);		/* clear insn map */
# elif defined(_WIN32)
  FlushInstructionCache(GetCurrentProcess(), संवृतिः->tramp, FFI_TRAMPOLINE_आकार);
# अन्यथा
  __clear_cache(संवृतिः->tramp, संवृतिः->tramp + 8);	/* clear data map */
  __clear_cache(codeloc, codeloc + 8);			/* clear insn map */
# endif
# ifdef _WIN32
  *(शून्य(**)(शून्य))(संवृतिः->tramp + FFI_TRAMPOLINE_संवृतिः_FUNCTION) = संवृतिः_func;
# अन्यथा
  *(शून्य (**)(शून्य))(संवृतिः->tramp + 8) = संवृतिः_func;
# endif
# यदि defined(FFI_EXEC_STATIC_TRAMP)
बहिर्गमनम्:
# endif
#यदि_अन्तः

  संवृतिः->cif = cif;
  संवृतिः->fun = fun;
  संवृतिः->user_दत्त = user_दत्त;

  प्रत्यागम FFI_OK;
}

#यदि_परिभाषितम् FFI_GO_CLOSURES
ffi_स्थितिः
ffi_prep_go_संवृतिः (ffi_go_संवृतिः *संवृतिः, ffi_cif *cif,
		     शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *))
{
  शून्य (*संवृतिः_func) (शून्य) = ffi_go_संवृतिः_SYSV;

  यदि (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      यदि (cif->vfp_प्रयुक्तम्)
	संवृतिः_func = ffi_go_संवृतिः_VFP;
    }
  अन्यथा यदि (cif->abi != FFI_SYSV)
    प्रत्यागम FFI_BAD_ABI;

  संवृतिः->tramp = संवृतिः_func;
  संवृतिः->cif = cif;
  संवृतिः->fun = fun;

  प्रत्यागम FFI_OK;
}
#यदि_अन्तः

#यदि_अन्तः /* FFI_CLOSURES */

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
  size_t आकार, ele_गणनम्;

  /* Quickest tests first.  */
  candidate = ty->type;
  विकल्प (ty->type)
    {
    स्वभाव:
      प्रत्यागम 0;
    पक्ष FFI_TYPE_FLOAT:
    पक्ष FFI_TYPE_DOUBLE:
      ele_गणनम् = 1;
      गच्छ समाप्तम्;
    पक्ष FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      यदि (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	प्रत्यागम 0;
      ele_गणनम् = 2;
      गच्छ समाप्तम्;
    पक्ष FFI_TYPE_STRUCT:
      भङ्ग;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  आकार = ty->आकार;
  यदि (आकार < 4 || आकार > 32)
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
      ele_गणनम् = आकार / परिमाण(भिन्न);
      यदि (आकार != ele_गणनम् * परिमाण(भिन्न))
        प्रत्यागम 0;
      भङ्ग;
    पक्ष FFI_TYPE_DOUBLE:
      ele_गणनम् = आकार / परिमाण(द्विगुण);
      यदि (आकार != ele_गणनम् * परिमाण(द्विगुण))
        प्रत्यागम 0;
      भङ्ग;
    स्वभाव:
      प्रत्यागम 0;
    }
  यदि (ele_गणनम् > 4)
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
 समाप्तम्:
  प्रत्यागम (ele_गणनम् << 8) | candidate;
}

स्थिर पूर्णाङ्क
place_vfp_arg (ffi_cif *cif, पूर्णाङ्क h)
{
  अचिह्नित ह्रस्व reg = cif->vfp_reg_free;
  पूर्णाङ्क align = 1, nregs = h >> 8;

  यदि ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  यदि ((reg & 1) && align == 2)
    reg++;

  यावत् (reg + nregs <= 16)
    {
      पूर्णाङ्क s, new_प्रयुक्तम् = 0;
      प्रति (s = reg; s < reg + nregs; s++)
	{
	  new_प्रयुक्तम् |= (1 << s);
	  यदि (cif->vfp_प्रयुक्तम् & (1 << s))
	    {
	      reg += align;
	      गच्छ अग्र_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_प्रयुक्तम् |= new_प्रयुक्तम्;
      cif->vfp_तर्काः[cif->vfp_nargs++] = (चिह्नित वर्ण)reg;

      /* Update vfp_reg_free. */
      यदि (cif->vfp_प्रयुक्तम् & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  यावत् (cif->vfp_प्रयुक्तम् & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      प्रत्यागम 0;
    अग्र_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_प्रयुक्तम् = 0xFFFF;
  प्रत्यागम 1;
}

स्थिर शून्य
layout_vfp_तर्काः (ffi_cif * cif)
{
  अचिह्नित पूर्णाङ्क i;
  /* Init VFP fields */
  cif->vfp_प्रयुक्तम् = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  स्मृतिं_पूरय (cif->vfp_तर्काः, -1, 16);	/* Init to -1. */

  प्रति (i = 0; i < cif->nargs; i++)
    {
      पूर्णाङ्क h = vfp_type_p (cif->arg_types[i]);
      यदि (h && place_vfp_arg (cif, h) == 1)
	भङ्ग;
    }
}

#यदि defined(FFI_EXEC_STATIC_TRAMP)
शून्य *
ffi_tramp_arch (size_t *tramp_आकार, size_t *map_आकार)
{
  बाह्य शून्य *trampoline_संहिता_सारणी;

  *tramp_आकार = ARM_TRAMP_आकार;
  *map_आकार = ARM_TRAMP_MAP_आकार;
  प्रत्यागम &trampoline_संहिता_सारणी;
}
#यदि_अन्तः

#यदि_अन्तः /* __arm__ or _M_ARM */
