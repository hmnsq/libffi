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

#ಆದರೆ defined(__arm__) || defined(_M_ARM)
#ಅನ್ತರ್ಭಾವ <fficonfig.h>
#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <tramp.h>
#ಅನ್ತರ್ಭಾವ "internal.h"

#ಆದರೆ defined(_WIN32)
#ವ್ಯಾಖ್ಯಾನಿಸು WIN32_LEAN_AND_ಸರಾಸರಿ
#ಅನ್ತರ್ಭಾವ <windows.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ FFI_EXEC_TRAMPOLINE_ಕೋಷ್ಟಕ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __MACH__
#ಅನ್ತರ್ಭಾವ <mach/machine/vm_param.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _WIN32
ಬಾಹ್ಯ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ffi_arm_trampoline[2] FFI_HIDDEN;
#ಇಲ್ಲದಿದ್ದರೆ
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
ಬಾಹ್ಯ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ffi_arm_trampoline[12] FFI_HIDDEN;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(__FreeBSD__) && defined(__arm__)
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <machine/sysarch.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(__QNX__)
#ಅನ್ತರ್ಭಾವ <sys/mman.h>
#ಆದರೆ_ಅಂತ್ಯ

/* Forward declares. */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vfp_type_p (ಸ್ಥಿರಾಂಕ ffi_type *);
ಸ್ಥಿರ ಶೂನ್ಯ layout_vfp_ವಾದಗಳು (ffi_cif *);

ಸ್ಥಿರ ಶೂನ್ಯ *
ffi_align (ffi_type *ty, ಶೂನ್ಯ *p)
{
  /* Align if necessary */
  size_t alignment;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32_WCE
  alignment = 4;
#ಇಲ್ಲದಿದ್ದರೆ
  alignment = ty->alignment;
  ಆದರೆ (alignment < 4)
    alignment = 4;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ *) FFI_ALIGN (p, alignment);
}

ಸ್ಥಿರ size_t
ffi_put_arg (ffi_type *ty, ಶೂನ್ಯ *src, ಶೂನ್ಯ *dst)
{
  size_t z = ty->ಗಾತ್ರ_2;

  ಆಯ್ಕೆ (ty->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
      *(UINT32 *)dst = *(SINT8 *)src;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
      *(UINT32 *)dst = *(UINT8 *)src;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
      *(UINT32 *)dst = *(SINT16 *)src;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
      *(UINT32 *)dst = *(UINT16 *)src;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_INT:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _WIN32
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
#ಆದರೆ_ಅಂತ್ಯ
      *(UINT32 *)dst = *(UINT32 *)src;
      ನಿಲ್ಲಿಸು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
    // casting a float* to a UINT32* doesn't work on Windows
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
        *(uintptr_t *)dst = 0;
        *(ದಶಮಾಂಶ *)dst = *(ದಶಮಾಂಶ *)src;
        ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      *(UINT64 *)dst = *(UINT64 *)src;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
    ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
      ಸ್ಮರಣೆ_ನಕಲು (dst, src, z);
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      abort();
    }

  ಹಿಂತಿರುಗಿ FFI_ALIGN (z, 4);
}

/* ffi_prep_args is called once stack space has been allocated
   for the function's arguments.

   The vfp_space parameter is the load area for VFP regs, the return
   value is cif->vfp_used (word bitset of VFP regs used for passing
   arguments). These are only used for the VFP hard-float ABI.
*/
ಸ್ಥಿರ ಶೂನ್ಯ
ffi_prep_ವಾದಗಳು_SYSV (ffi_cif *cif, ಪೂರ್ಣಾಂಕ flags, ಶೂನ್ಯ *rvalue,
		    ಶೂನ್ಯ **avalue, ಅಕ್ಷರ *argp)
{
  ffi_type **arg_types = cif->arg_types;
  ಪೂರ್ಣಾಂಕ i, n;

  ಆದರೆ (flags == ARM_TYPE_STRUCT)
    {
      *(ಶೂನ್ಯ **) argp = rvalue;
      argp += 4;
    }

  ಗಾಗಿ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, avalue[i], argp);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_prep_ವಾದಗಳು_VFP (ffi_cif *cif, ಪೂರ್ಣಾಂಕ flags, ಶೂನ್ಯ *rvalue,
                   ಶೂನ್ಯ **avalue, ಅಕ್ಷರ *ರಾಶಿ, ಅಕ್ಷರ *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  ಪೂರ್ಣಾಂಕ i, n, vi = 0;
  ಅಕ್ಷರ *argp, *regp, *eo_regp;
  ಅಕ್ಷರ ರಾಶಿ_ಬಳಸಿದ = 0;
  ಅಕ್ಷರ ಮುಗಿದ_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = ರಾಶಿ;
  eo_regp = argp = regp + 16;

  /* If the function returns an FFI_TYPE_STRUCT in memory,
     that address is passed in r0 to the function.  */
  ಆದರೆ (flags == ARM_TYPE_STRUCT)
    {
      *(ಶೂನ್ಯ **) regp = rvalue;
      regp += 4;
    }

  ಗಾಗಿ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      ಶೂನ್ಯ *a = avalue[i];
      ಪೂರ್ಣಾಂಕ is_vfp_type = vfp_type_p (ty);

      /* Allocated in VFP registers. */
      ಆದರೆ (vi < cif->vfp_nargs && is_vfp_type)
	{
	  ಅಕ್ಷರ *vfp_ಸ್ಥಾನ = vfp_space + cif->vfp_ವಾದಗಳು[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_ಸ್ಥಾನ);
	  ಮುಂದುವರಿ;
	}
      /* Try allocating in core registers. */
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!ಮುಗಿದ_with_regs && !is_vfp_type)
	{
	  ಅಕ್ಷರ *tregp = ffi_align (ty, regp);
	  size_t ಗಾತ್ರ_2 = ty->ಗಾತ್ರ_2;
	  ಗಾತ್ರ_2 = (ಗಾತ್ರ_2 < 4) ? 4 : ಗಾತ್ರ_2;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  ಆದರೆ (tregp + ಗಾತ್ರ_2 <= eo_regp)
	    {
	      regp = tregp + ffi_put_arg (ty, a, tregp);
	      ಮುಗಿದ_with_regs = (regp == argp);
	      // ensure we did not write into the stack area
	      FFI_ASSERT (regp <= argp);
	      ಮುಂದುವರಿ;
	    }
	  /* In case there are no arguments in the stack area yet,
	     the argument is passed in the remaining core registers
	     and on the stack.  */
	  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!ರಾಶಿ_ಬಳಸಿದ)
	    {
	      ರಾಶಿ_ಬಳಸಿದ = 1;
	      ಮುಗಿದ_with_regs = 1;
	      argp = tregp + ffi_put_arg (ty, a, tregp);
	      FFI_ASSERT (eo_regp < argp);
	      ಮುಂದುವರಿ;
	    }
	}
      /* Base case, arguments are passed on the stack */
      ರಾಶಿ_ಬಳಸಿದ = 1;
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, a, argp);
    }
}

/* Perform machine dependent cif processing */
ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ಪೂರ್ಣಾಂಕ flags = 0, cabi = cif->abi;
  size_t ಬೈಟುಗಳು = cif->ಬೈಟುಗಳು;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  ಆದರೆ (cabi == FFI_VFP)
    layout_vfp_ವಾದಗಳು (cif);

  /* Set the return type flag */
  ಆಯ್ಕೆ (cif->rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_VOID:
      flags = ARM_TYPE_VOID;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_INT:
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
      flags = ARM_TYPE_INT;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
      flags = ARM_TYPE_INT64;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_S : ARM_TYPE_INT);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_D : ARM_TYPE_INT64);
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
    ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
      ಆದರೆ (cabi == FFI_VFP)
	{
	  ಪೂರ್ಣಾಂಕ h = vfp_type_p (cif->rtype);

	  flags = ARM_TYPE_VFP_N;
	  ಆದರೆ (h == 0x100 + FFI_TYPE_FLOAT)
	    flags = ARM_TYPE_VFP_S;
	  ಆದರೆ (h == 0x100 + FFI_TYPE_DOUBLE)
	    flags = ARM_TYPE_VFP_D;
	  ಆದರೆ (h != 0)
	      ನಿಲ್ಲಿಸು;
	}

      /* A Composite Type not larger than 4 bytes is returned in r0.
	 A Composite Type larger than 4 bytes, or whose size cannot
	 be determined statically ... is stored in memory at an
	 address passed [in r0].  */
      ಆದರೆ (cif->rtype->ಗಾತ್ರ_2 <= 4)
	flags = ARM_TYPE_INT;
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  flags = ARM_TYPE_STRUCT;
	  ಬೈಟುಗಳು += 4;
	}
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      abort();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  ಬೈಟುಗಳು = FFI_ALIGN (ಬೈಟುಗಳು, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  ಆದರೆ (ಬೈಟುಗಳು < 4*4)
    ಬೈಟುಗಳು = 4*4;

  cif->ಬೈಟುಗಳು = ಬೈಟುಗಳು;
  cif->flags = flags;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nfixedargs, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ntotalargs)
{
  /* VFP variadic calls actually use the SYSV ABI */
  ಆದರೆ (cif->abi == FFI_VFP)
    cif->abi = FFI_SYSV;

  ಹಿಂತಿರುಗಿ ffi_prep_cif_machdep (cif);
}

/* Prototypes for assembly functions, in sysv.S.  */

ರಚನೆ call_ಚೌಕಟ್ಟು
{
  ಶೂನ್ಯ *fp;
  ಶೂನ್ಯ *lr;
  ಶೂನ್ಯ *rvalue;
  ಪೂರ್ಣಾಂಕ flags;
  ಶೂನ್ಯ *ಆವರಣ;
};

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV (ಶೂನ್ಯ *ರಾಶಿ, ರಚನೆ call_ಚೌಕಟ್ಟು *,
			   ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ)) FFI_HIDDEN;
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_VFP (ಶೂನ್ಯ *vfp_space, ರಚನೆ call_ಚೌಕಟ್ಟು *,
			   ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಚಿಹ್ನೆರಹಿತ vfp_ಬಳಸಿದ) FFI_HIDDEN;

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int (ffi_cif * cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	      ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ)
{
  ಪೂರ್ಣಾಂಕ flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  size_t ಬೈಟುಗಳು, rsize, vfp_ಗಾತ್ರ;
  ಅಕ್ಷರ *ರಾಶಿ, *vfp_space, *new_rvalue;
  ರಚನೆ call_ಚೌಕಟ್ಟು *ಚೌಕಟ್ಟು;

  rsize = 0;
  ಆದರೆ (rvalue == ಖಾಲಿ)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      ಆದರೆ (flags == ARM_TYPE_STRUCT)
	rsize = rtype->ಗಾತ್ರ_2;
      ಇಲ್ಲದಿದ್ದರೆ
	flags = ARM_TYPE_VOID;
    }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (flags == ARM_TYPE_VFP_N)
    {
      /* Largest case is double x 4. */
      rsize = 32;
    }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (flags == ARM_TYPE_INT && rtype->type == FFI_TYPE_STRUCT)
    rsize = 4;

  /* Largest case.  */
  vfp_ಗಾತ್ರ = (cif->abi == FFI_VFP && cif->vfp_ಬಳಸಿದ ? 8*8: 0);

  ಬೈಟುಗಳು = cif->ಬೈಟುಗಳು;
  ರಾಶಿ = alloca (vfp_ಗಾತ್ರ + ಬೈಟುಗಳು + ಗಾತ್ರ(ರಚನೆ call_ಚೌಕಟ್ಟು) + rsize);

  vfp_space = ಖಾಲಿ;
  ಆದರೆ (vfp_ಗಾತ್ರ)
    {
      vfp_space = ರಾಶಿ;
      ರಾಶಿ += vfp_ಗಾತ್ರ;
    }

  ಚೌಕಟ್ಟು = (ರಚನೆ call_ಚೌಕಟ್ಟು *)(ರಾಶಿ + ಬೈಟುಗಳು);

  new_rvalue = rvalue;
  ಆದರೆ (rsize)
    new_rvalue = (ಶೂನ್ಯ *)(ಚೌಕಟ್ಟು + 1);

  ಚೌಕಟ್ಟು->rvalue = new_rvalue;
  ಚೌಕಟ್ಟು->flags = flags;
  ಚೌಕಟ್ಟು->ಆವರಣ = ಆವರಣ;

  ಆದರೆ (vfp_space)
    {
      ffi_prep_ವಾದಗಳು_VFP (cif, flags, new_rvalue, avalue, ರಾಶಿ, vfp_space);
      ffi_call_VFP (vfp_space, ಚೌಕಟ್ಟು, fn, cif->vfp_ಬಳಸಿದ);
    }
  ಇಲ್ಲದಿದ್ದರೆ
    {
      ffi_prep_ವಾದಗಳು_SYSV (cif, flags, new_rvalue, avalue, ರಾಶಿ);
      ffi_call_SYSV (ರಾಶಿ, ಚೌಕಟ್ಟು, fn);
    }

  ಆದರೆ (rvalue && rvalue != new_rvalue)
    ಸ್ಮರಣೆ_ನಕಲು (rvalue, new_rvalue, rtype->ಗಾತ್ರ_2);
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ಖಾಲಿ);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_GO_CLOSURES
ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ)
{
  ffi_call_int (cif, fn, rvalue, avalue, ಆವರಣ);
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಶೂನ್ಯ *
ffi_prep_incoming_ವಾದಗಳು_SYSV (ffi_cif *cif, ಶೂನ್ಯ *rvalue,
			     ಅಕ್ಷರ *argp, ಶೂನ್ಯ **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  ಪೂರ್ಣಾಂಕ i, n;

  ಆದರೆ (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(ಶೂನ್ಯ **) argp;
      argp += 4;
    }
  ಇಲ್ಲದಿದ್ದರೆ
    {
      ಆದರೆ (cif->rtype->ಗಾತ್ರ_2 && cif->rtype->ಗಾತ್ರ_2 < 4)
        *(uint32_t *) rvalue = 0;
    }

  ಗಾಗಿ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->ಗಾತ್ರ_2;

      argp = ffi_align (ty, argp);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __ARMEB__
      ಆದರೆ (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (ಶೂನ್ಯ *) (argp + 4 - z);
      ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
      avalue[i] = (ಶೂನ್ಯ *) argp;
      argp += z;
    }

  ಹಿಂತಿರುಗಿ rvalue;
}

ಸ್ಥಿರ ಶೂನ್ಯ *
ffi_prep_incoming_ವಾದಗಳು_VFP (ffi_cif *cif, ಶೂನ್ಯ *rvalue, ಅಕ್ಷರ *ರಾಶಿ,
			    ಅಕ್ಷರ *vfp_space, ಶೂನ್ಯ **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  ಪೂರ್ಣಾಂಕ i, n, vi = 0;
  ಅಕ್ಷರ *argp, *regp, *eo_regp;
  ಅಕ್ಷರ ಮುಗಿದ_with_regs = 0;
  ಅಕ್ಷರ ರಾಶಿ_ಬಳಸಿದ = 0;

  regp = ರಾಶಿ;
  eo_regp = argp = regp + 16;

  ಆದರೆ (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(ಶೂನ್ಯ **) regp;
      regp += 4;
    }

  ಗಾಗಿ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      ಪೂರ್ಣಾಂಕ is_vfp_type = vfp_type_p (ty);
      size_t z = ty->ಗಾತ್ರ_2;

      ಆದರೆ (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_ವಾದಗಳು[vi++] * 4;
	  ಮುಂದುವರಿ;
	}
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!ಮುಗಿದ_with_regs && !is_vfp_type)
	{
	  ಅಕ್ಷರ *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  ಆದರೆ (tregp + z <= eo_regp || !ರಾಶಿ_ಬಳಸಿದ)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __ARMEB__
	      ಆದರೆ (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (ಶೂನ್ಯ *) (tregp + 4 - z);
	      ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
	      avalue[i] = (ಶೂನ್ಯ *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      ಆದರೆ (regp > eo_regp)
		{
		  FFI_ASSERT (!ರಾಶಿ_ಬಳಸಿದ);
		  argp = regp;
		}
	      ಆದರೆ (regp >= eo_regp)
		{
		  ಮುಗಿದ_with_regs = 1;
		  ರಾಶಿ_ಬಳಸಿದ = 1;
		}
	      ಮುಂದುವರಿ;
	    }
	}

      ರಾಶಿ_ಬಳಸಿದ = 1;
      argp = ffi_align (ty, argp);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __ARMEB__
      ಆದರೆ (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (ಶೂನ್ಯ *) (argp + 4 - z);
      ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
      avalue[i] = (ಶೂನ್ಯ *) argp;
      argp += z;
    }

  ಹಿಂತಿರುಗಿ rvalue;
}

#ಆದರೆ FFI_CLOSURES

ರಚನೆ ಆವರಣ_ಚೌಕಟ್ಟು
{
  ಅಕ್ಷರ vfp_space[8*8] __attribute__((aligned(8)));
  ಅಕ್ಷರ ಫಲಿತಾಂಶ[8*4];
  ಅಕ್ಷರ argp[];
};

ಪೂರ್ಣಾಂಕ FFI_HIDDEN
ffi_ಆವರಣ_inner_SYSV (ffi_cif *cif,
		        ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
		        ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		        ರಚನೆ ಆವರಣ_ಚೌಕಟ್ಟು *ಚೌಕಟ್ಟು)
{
  ಶೂನ್ಯ **avalue = (ಶೂನ್ಯ **) alloca (cif->nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));
  ಶೂನ್ಯ *rvalue = ffi_prep_incoming_ವಾದಗಳು_SYSV (cif, ಚೌಕಟ್ಟು->ಫಲಿತಾಂಶ,
					      ಚೌಕಟ್ಟು->argp, avalue);
  fun (cif, rvalue, avalue, user_ದತ್ತಾಂಶ);
  ಹಿಂತಿರುಗಿ cif->flags;
}

ಪೂರ್ಣಾಂಕ FFI_HIDDEN
ffi_ಆವರಣ_inner_VFP (ffi_cif *cif,
		       ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
		       ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		       ರಚನೆ ಆವರಣ_ಚೌಕಟ್ಟು *ಚೌಕಟ್ಟು)
{
  ಶೂನ್ಯ **avalue = (ಶೂನ್ಯ **) alloca (cif->nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));
  ಶೂನ್ಯ *rvalue = ffi_prep_incoming_ವಾದಗಳು_VFP (cif, ಚೌಕಟ್ಟು->ಫಲಿತಾಂಶ, ಚೌಕಟ್ಟು->argp,
					     ಚೌಕಟ್ಟು->vfp_space, avalue);
  fun (cif, rvalue, avalue, user_ದತ್ತಾಂಶ);
  ಹಿಂತಿರುಗಿ cif->flags;
}

ಶೂನ್ಯ ffi_ಆವರಣ_SYSV (ಶೂನ್ಯ) FFI_HIDDEN;
ಶೂನ್ಯ ffi_ಆವರಣ_VFP (ಶೂನ್ಯ) FFI_HIDDEN;
#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
ಶೂನ್ಯ ffi_ಆವರಣ_SYSV_alt (ಶೂನ್ಯ) FFI_HIDDEN;
ಶೂನ್ಯ ffi_ಆವರಣ_VFP_alt (ಶೂನ್ಯ) FFI_HIDDEN;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_GO_CLOSURES
ಶೂನ್ಯ ffi_go_ಆವರಣ_SYSV (ಶೂನ್ಯ) FFI_HIDDEN;
ಶೂನ್ಯ ffi_go_ಆವರಣ_VFP (ಶೂನ್ಯ) FFI_HIDDEN;
#ಆದರೆ_ಅಂತ್ಯ

/* the cif must already be prep'ed */

#ಆದರೆ defined(__FreeBSD__) && defined(__arm__)
#ವ್ಯಾಖ್ಯಾನಿಸು __clear_cache(ಆರಂಭ, ಕೊನೆ) ಮಾಡು { \
		ರಚನೆ arm_sync_icache_ವಾದಗಳು ua; 		\
								\
		ua.addr = (uintptr_t)(ಆರಂಭ);			\
		ua.len = (ಅಕ್ಷರ *)(ಕೊನೆ) - (ಅಕ್ಷರ *)ಆರಂಭ;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} ಆಗ (0);
#ಆದರೆ_ಅಂತ್ಯ

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ * ಆವರಣ,
		      ffi_cif * cif,
		      ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ, ಶೂನ್ಯ *codeloc)
{
  ಶೂನ್ಯ (*ಆವರಣ_func) (ಶೂನ್ಯ) = ffi_ಆವರಣ_SYSV;

  ಆದರೆ (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      ಆದರೆ (cif->vfp_ಬಳಸಿದ)
	ಆವರಣ_func = ffi_ಆವರಣ_VFP;
    }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cif->abi != FFI_SYSV)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

#ಆದರೆ FFI_EXEC_TRAMPOLINE_ಕೋಷ್ಟಕ
  ಶೂನ್ಯ **config = (ಶೂನ್ಯ **)((uint8_t *)codeloc - ಪುಟ_MAX_ಗಾತ್ರ);
  config[0] = ಆವರಣ;
  config[1] = ಆವರಣ_func;
#ಇಲ್ಲದಿದ್ದರೆ

# ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
  ಆದರೆ (ffi_tramp_is_present(ಆವರಣ))
    {
      /* Initialize the static trampoline's parameters. */
      ಆದರೆ (ಆವರಣ_func == ffi_ಆವರಣ_SYSV)
        ಆವರಣ_func = ffi_ಆವರಣ_SYSV_alt;
      ಇಲ್ಲದಿದ್ದರೆ
        ಆವರಣ_func = ffi_ಆವರಣ_VFP_alt;
      ffi_tramp_ಗಣ_parms (ಆವರಣ->ftramp, ಆವರಣ_func, ಆವರಣ);
      ಹೋಗು ಹೊರಹೋಗು;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  ಸ್ಮರಣೆ_ನಕಲು(ಆವರಣ->tramp, ffi_arm_trampoline, 8);
# ಇಲ್ಲದಿದ್ದರೆ
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  ಸ್ಮರಣೆ_ನಕಲು(ಆವರಣ->tramp, (ಶೂನ್ಯ*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_ಆವರಣ_ಸ್ಥಾನಾಂತರ);
# endif

# ಆದರೆ defined(__QNX__)
  msync (ಆವರಣ->tramp, 8, MS_INVALIDATE_ICACHE);	/* clear data map */
  msync (codeloc, 8, MS_INVALIDATE_ICACHE);		/* clear insn map */
# elif defined(_WIN32)
  FlushInstructionCache(GetCurrentProcess(), ಆವರಣ->tramp, FFI_TRAMPOLINE_ಗಾತ್ರ);
# ಇಲ್ಲದಿದ್ದರೆ
  __clear_cache(ಆವರಣ->tramp, ಆವರಣ->tramp + 8);	/* clear data map */
  __clear_cache(codeloc, codeloc + 8);			/* clear insn map */
# endif
# ifdef _WIN32
  *(ಶೂನ್ಯ(**)(ಶೂನ್ಯ))(ಆವರಣ->tramp + FFI_TRAMPOLINE_ಆವರಣ_FUNCTION) = ಆವರಣ_func;
# ಇಲ್ಲದಿದ್ದರೆ
  *(ಶೂನ್ಯ (**)(ಶೂನ್ಯ))(ಆವರಣ->tramp + 8) = ಆವರಣ_func;
# endif
# ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
ಹೊರಹೋಗು:
# endif
#ಆದರೆ_ಅಂತ್ಯ

  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_GO_CLOSURES
ffi_ಸ್ಥಿತಿ
ffi_prep_go_ಆವರಣ (ffi_go_ಆವರಣ *ಆವರಣ, ffi_cif *cif,
		     ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *))
{
  ಶೂನ್ಯ (*ಆವರಣ_func) (ಶೂನ್ಯ) = ffi_go_ಆವರಣ_SYSV;

  ಆದರೆ (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      ಆದರೆ (cif->vfp_ಬಳಸಿದ)
	ಆವರಣ_func = ffi_go_ಆವರಣ_VFP;
    }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cif->abi != FFI_SYSV)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  ಆವರಣ->tramp = ಆವರಣ_func;
  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;

  ಹಿಂತಿರುಗಿ FFI_OK;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* FFI_CLOSURES */

/* Below are routines for VFP hard-float support. */

/* A subroutine of vfp_type_p.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
is_hfa0 (ಸ್ಥಿರಾಂಕ ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  ಪೂರ್ಣಾಂಕ i, ret = -1;

  ಆದರೆ (elements != ಖಾಲಿ)
    ಗಾಗಿ (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        ಆದರೆ (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            ಆದರೆ (ret < 0)
              ಮುಂದುವರಿ;
          }
        ನಿಲ್ಲಿಸು;
      }

  ಹಿಂತಿರುಗಿ ret;
}

/* A subroutine of vfp_type_p.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
is_hfa1 (ಸ್ಥಿರಾಂಕ ffi_type *ty, ಪೂರ್ಣಾಂಕ candidate)
{
  ffi_type **elements = ty->elements;
  ಪೂರ್ಣಾಂಕ i;

  ಆದರೆ (elements != ಖಾಲಿ)
    ಗಾಗಿ (i = 0; elements[i]; ++i)
      {
        ಪೂರ್ಣಾಂಕ t = elements[i]->type;
        ಆದರೆ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
          {
            ಆದರೆ (!is_hfa1 (elements[i], candidate))
              ಹಿಂತಿರುಗಿ 0;
          }
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (t != candidate)
          ಹಿಂತಿರುಗಿ 0;
      }

  ಹಿಂತಿರುಗಿ 1;
}

/* Determine if TY is an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is a floating point scalar.

   Returns non-zero iff TY is an HFA.  The result is an encoded value where
   bits 0-7 contain the type code, and bits 8-10 contain the element count.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
vfp_type_p (ಸ್ಥಿರಾಂಕ ffi_type *ty)
{
  ffi_type **elements;
  ಪೂರ್ಣಾಂಕ candidate, i;
  size_t ಗಾತ್ರ_2, ele_ಎಣಿಕೆ;

  /* Quickest tests first.  */
  candidate = ty->type;
  ಆಯ್ಕೆ (ty->type)
    {
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ 0;
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      ele_ಎಣಿಕೆ = 1;
      ಹೋಗು ಮುಗಿದ;
    ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      ಆದರೆ (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	ಹಿಂತಿರುಗಿ 0;
      ele_ಎಣಿಕೆ = 2;
      ಹೋಗು ಮುಗಿದ;
    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      ನಿಲ್ಲಿಸು;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  ಗಾತ್ರ_2 = ty->ಗಾತ್ರ_2;
  ಆದರೆ (ಗಾತ್ರ_2 < 4 || ಗಾತ್ರ_2 > 32)
    ಹಿಂತಿರುಗಿ 0;

  /* Find the type of the first non-structure member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  ಆದರೆ (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_COMPLEX)
    {
      ಗಾಗಿ (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          ಆದರೆ (candidate >= 0)
            ನಿಲ್ಲಿಸು;
        }
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  ಆಯ್ಕೆ (candidate)
    {
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      ele_ಎಣಿಕೆ = ಗಾತ್ರ_2 / ಗಾತ್ರ(ದಶಮಾಂಶ);
      ಆದರೆ (ಗಾತ್ರ_2 != ele_ಎಣಿಕೆ * ಗಾತ್ರ(ದಶಮಾಂಶ))
        ಹಿಂತಿರುಗಿ 0;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      ele_ಎಣಿಕೆ = ಗಾತ್ರ_2 / ಗಾತ್ರ(ದ್ವಿಗುಣ);
      ಆದರೆ (ಗಾತ್ರ_2 != ele_ಎಣಿಕೆ * ಗಾತ್ರ(ದ್ವಿಗುಣ))
        ಹಿಂತಿರುಗಿ 0;
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ 0;
    }
  ಆದರೆ (ele_ಎಣಿಕೆ > 4)
    ಹಿಂತಿರುಗಿ 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  ಗಾಗಿ (i = 0; elements[i]; ++i)
    {
      ಪೂರ್ಣಾಂಕ t = elements[i]->type;
      ಆದರೆ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          ಆದರೆ (!is_hfa1 (elements[i], candidate))
            ಹಿಂತಿರುಗಿ 0;
        }
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (t != candidate)
        ಹಿಂತಿರುಗಿ 0;
    }

  /* All tests succeeded.  Encode the result.  */
 ಮುಗಿದ:
  ಹಿಂತಿರುಗಿ (ele_ಎಣಿಕೆ << 8) | candidate;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ಇರಿಸು_vfp_arg (ffi_cif *cif, ಪೂರ್ಣಾಂಕ h)
{
  ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ reg = cif->vfp_reg_free;
  ಪೂರ್ಣಾಂಕ align = 1, nregs = h >> 8;

  ಆದರೆ ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  ಆದರೆ ((reg & 1) && align == 2)
    reg++;

  ಆಗ (reg + nregs <= 16)
    {
      ಪೂರ್ಣಾಂಕ s, new_ಬಳಸಿದ = 0;
      ಗಾಗಿ (s = reg; s < reg + nregs; s++)
	{
	  new_ಬಳಸಿದ |= (1 << s);
	  ಆದರೆ (cif->vfp_ಬಳಸಿದ & (1 << s))
	    {
	      reg += align;
	      ಹೋಗು ಮುಂದಿನ_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_ಬಳಸಿದ |= new_ಬಳಸಿದ;
      cif->vfp_ವಾದಗಳು[cif->vfp_nargs++] = (ಚಿಹ್ನೆಯುಳ್ಳ ಅಕ್ಷರ)reg;

      /* Update vfp_reg_free. */
      ಆದರೆ (cif->vfp_ಬಳಸಿದ & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  ಆಗ (cif->vfp_ಬಳಸಿದ & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      ಹಿಂತಿರುಗಿ 0;
    ಮುಂದಿನ_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_ಬಳಸಿದ = 0xFFFF;
  ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ ಶೂನ್ಯ
layout_vfp_ವಾದಗಳು (ffi_cif * cif)
{
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  /* Init VFP fields */
  cif->vfp_ಬಳಸಿದ = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  memset (cif->vfp_ವಾದಗಳು, -1, 16);	/* Init to -1. */

  ಗಾಗಿ (i = 0; i < cif->nargs; i++)
    {
      ಪೂರ್ಣಾಂಕ h = vfp_type_p (cif->arg_types[i]);
      ಆದರೆ (h && ಇರಿಸು_vfp_arg (cif, h) == 1)
	ನಿಲ್ಲಿಸು;
    }
}

#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
ಶೂನ್ಯ *
ffi_tramp_arch (size_t *tramp_ಗಾತ್ರ, size_t *map_ಗಾತ್ರ)
{
  ಬಾಹ್ಯ ಶೂನ್ಯ *trampoline_ಸಂಕೇತ_ಕೋಷ್ಟಕ;

  *tramp_ಗಾತ್ರ = ARM_TRAMP_ಗಾತ್ರ;
  *map_ಗಾತ್ರ = ARM_TRAMP_MAP_ಗಾತ್ರ;
  ಹಿಂತಿರುಗಿ &trampoline_ಸಂಕೇತ_ಕೋಷ್ಟಕ;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* __arm__ or _M_ARM */
