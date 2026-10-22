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

#ಯದಿ defined(__arm__) || defined(_M_ARM)
#ಅನ್ತರ್ಭಾವ <fficonfig.h>
#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <tramp.h>
#ಅನ್ತರ್ಭಾವ "internal.h"

#ಯದಿ defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#ಅನ್ತರ್ಭಾವ <windows.h>
#endif

#ಯದಿ FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#ಅನ್ತರ್ಭಾವ <mach/machine/vm_param.h>
#endif

#ಅನ್ಯಥಾ
#ifndef _WIN32
ಬಾಹ್ಯ unsigned ಪೂರ್ಣಾಙ್ಕ ffi_arm_trampoline[2] FFI_HIDDEN;
#ಅನ್ಯಥಾ
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
ಬಾಹ್ಯ unsigned ವರ್ಣ ffi_arm_trampoline[12] FFI_HIDDEN;
#endif
#endif

#ಯದಿ defined(__FreeBSD__) && defined(__arm__)
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <machine/sysarch.h>
#endif

#ಯದಿ defined(__QNX__)
#ಅನ್ತರ್ಭಾವ <sys/mman.h>
#endif

/* Forward declares. */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ vfp_type_p (ಧ್ರುವ ffi_type *);
ಸ್ಥಿರ ಶೂನ್ಯ layout_vfp_args (ffi_cif *);

ಸ್ಥಿರ ಶೂನ್ಯ *
ffi_align (ffi_type *ty, ಶೂನ್ಯ *p)
{
  /* Align if necessary */
  size_t alignment;
#ifdef _WIN32_WCE
  alignment = 4;
#ಅನ್ಯಥಾ
  alignment = ty->alignment;
  ಯದಿ (alignment < 4)
    alignment = 4;
#endif
  ಪ್ರತ್ಯಾಗಮ (ಶೂನ್ಯ *) FFI_ALIGN (p, alignment);
}

ಸ್ಥಿರ size_t
ffi_put_arg (ffi_type *ty, ಶೂನ್ಯ *src, ಶೂನ್ಯ *dst)
{
  size_t z = ty->size;

  ವಿಕಲ್ಪ (ty->type)
    {
    ಪಕ್ಷ FFI_TYPE_SINT8:
      *(UINT32 *)dst = *(SINT8 *)src;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_UINT8:
      *(UINT32 *)dst = *(UINT8 *)src;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT16:
      *(UINT32 *)dst = *(SINT16 *)src;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_UINT16:
      *(UINT32 *)dst = *(UINT16 *)src;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_INT:
    ಪಕ್ಷ FFI_TYPE_SINT32:
    ಪಕ್ಷ FFI_TYPE_UINT32:
    ಪಕ್ಷ FFI_TYPE_POINTER:
#ifndef _WIN32
    ಪಕ್ಷ FFI_TYPE_FLOAT:
#endif
      *(UINT32 *)dst = *(UINT32 *)src;
      ಭಙ್ಗ;

#ifdef _WIN32
    // casting a float* to a UINT32* doesn't work on Windows
    ಪಕ್ಷ FFI_TYPE_FLOAT:
        *(uintptr_t *)dst = 0;
        *(ಭಿನ್ನ *)dst = *(ಭಿನ್ನ *)src;
        ಭಙ್ಗ;
#endif

    ಪಕ್ಷ FFI_TYPE_SINT64:
    ಪಕ್ಷ FFI_TYPE_UINT64:
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      *(UINT64 *)dst = *(UINT64 *)src;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_STRUCT:
    ಪಕ್ಷ FFI_TYPE_COMPLEX:
      memcpy (dst, src, z);
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      abort();
    }

  ಪ್ರತ್ಯಾಗಮ FFI_ALIGN (z, 4);
}

/* ffi_prep_args is called once stack space has been allocated
   for the function's arguments.

   The vfp_space parameter is the load area for VFP regs, the return
   value is cif->vfp_used (word bitset of VFP regs used for passing
   arguments). These are only used for the VFP hard-float ABI.
*/
ಸ್ಥಿರ ಶೂನ್ಯ
ffi_prep_args_SYSV (ffi_cif *cif, ಪೂರ್ಣಾಙ್ಕ flags, ಶೂನ್ಯ *rvalue,
		    ಶೂನ್ಯ **avalue, ವರ್ಣ *argp)
{
  ffi_type **arg_types = cif->arg_types;
  ಪೂರ್ಣಾಙ್ಕ i, n;

  ಯದಿ (flags == ARM_TYPE_STRUCT)
    {
      *(ಶೂನ್ಯ **) argp = rvalue;
      argp += 4;
    }

  ಪ್ರತಿ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, avalue[i], argp);
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_prep_args_VFP (ffi_cif *cif, ಪೂರ್ಣಾಙ್ಕ flags, ಶೂನ್ಯ *rvalue,
                   ಶೂನ್ಯ **avalue, ವರ್ಣ *ರಾಶಿ, ವರ್ಣ *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  ಪೂರ್ಣಾಙ್ಕ i, n, vi = 0;
  ವರ್ಣ *argp, *regp, *eo_regp;
  ವರ್ಣ ರಾಶಿ_used = 0;
  ವರ್ಣ done_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = ರಾಶಿ;
  eo_regp = argp = regp + 16;

  /* If the function returns an FFI_TYPE_STRUCT in memory,
     that address is passed in r0 to the function.  */
  ಯದಿ (flags == ARM_TYPE_STRUCT)
    {
      *(ಶೂನ್ಯ **) regp = rvalue;
      regp += 4;
    }

  ಪ್ರತಿ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      ಶೂನ್ಯ *a = avalue[i];
      ಪೂರ್ಣಾಙ್ಕ is_vfp_type = vfp_type_p (ty);

      /* Allocated in VFP registers. */
      ಯದಿ (vi < cif->vfp_nargs && is_vfp_type)
	{
	  ವರ್ಣ *vfp_slot = vfp_space + cif->vfp_args[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_slot);
	  ಅನುವರ್ತ;
	}
      /* Try allocating in core registers. */
      ಅನ್ಯಥಾ ಯದಿ (!done_with_regs && !is_vfp_type)
	{
	  ವರ್ಣ *tregp = ffi_align (ty, regp);
	  size_t size = ty->size;
	  size = (size < 4) ? 4 : size;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  ಯದಿ (tregp + size <= eo_regp)
	    {
	      regp = tregp + ffi_put_arg (ty, a, tregp);
	      done_with_regs = (regp == argp);
	      // ensure we did not write into the stack area
	      FFI_ASSERT (regp <= argp);
	      ಅನುವರ್ತ;
	    }
	  /* In case there are no arguments in the stack area yet,
	     the argument is passed in the remaining core registers
	     and on the stack.  */
	  ಅನ್ಯಥಾ ಯದಿ (!ರಾಶಿ_used)
	    {
	      ರಾಶಿ_used = 1;
	      done_with_regs = 1;
	      argp = tregp + ffi_put_arg (ty, a, tregp);
	      FFI_ASSERT (eo_regp < argp);
	      ಅನುವರ್ತ;
	    }
	}
      /* Base case, arguments are passed on the stack */
      ರಾಶಿ_used = 1;
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, a, argp);
    }
}

/* Perform machine dependent cif processing */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ಪೂರ್ಣಾಙ್ಕ flags = 0, cabi = cif->abi;
  size_t bytes = cif->bytes;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  ಯದಿ (cabi == FFI_VFP)
    layout_vfp_args (cif);

  /* Set the return type flag */
  ವಿಕಲ್ಪ (cif->rtype->type)
    {
    ಪಕ್ಷ FFI_TYPE_VOID:
      flags = ARM_TYPE_VOID;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_INT:
    ಪಕ್ಷ FFI_TYPE_UINT8:
    ಪಕ್ಷ FFI_TYPE_SINT8:
    ಪಕ್ಷ FFI_TYPE_UINT16:
    ಪಕ್ಷ FFI_TYPE_SINT16:
    ಪಕ್ಷ FFI_TYPE_UINT32:
    ಪಕ್ಷ FFI_TYPE_SINT32:
    ಪಕ್ಷ FFI_TYPE_POINTER:
      flags = ARM_TYPE_INT;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_SINT64:
    ಪಕ್ಷ FFI_TYPE_UINT64:
      flags = ARM_TYPE_INT64;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_FLOAT:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_S : ARM_TYPE_INT);
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_D : ARM_TYPE_INT64);
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_STRUCT:
    ಪಕ್ಷ FFI_TYPE_COMPLEX:
      ಯದಿ (cabi == FFI_VFP)
	{
	  ಪೂರ್ಣಾಙ್ಕ h = vfp_type_p (cif->rtype);

	  flags = ARM_TYPE_VFP_N;
	  ಯದಿ (h == 0x100 + FFI_TYPE_FLOAT)
	    flags = ARM_TYPE_VFP_S;
	  ಯದಿ (h == 0x100 + FFI_TYPE_DOUBLE)
	    flags = ARM_TYPE_VFP_D;
	  ಯದಿ (h != 0)
	      ಭಙ್ಗ;
	}

      /* A Composite Type not larger than 4 bytes is returned in r0.
	 A Composite Type larger than 4 bytes, or whose size cannot
	 be determined statically ... is stored in memory at an
	 address passed [in r0].  */
      ಯದಿ (cif->rtype->size <= 4)
	flags = ARM_TYPE_INT;
      ಅನ್ಯಥಾ
	{
	  flags = ARM_TYPE_STRUCT;
	  bytes += 4;
	}
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      abort();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  bytes = FFI_ALIGN (bytes, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  ಯದಿ (bytes < 4*4)
    bytes = 4*4;

  cif->bytes = bytes;
  cif->flags = flags;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  unsigned ಪೂರ್ಣಾಙ್ಕ nfixedargs, unsigned ಪೂರ್ಣಾಙ್ಕ ntotalargs)
{
  /* VFP variadic calls actually use the SYSV ABI */
  ಯದಿ (cif->abi == FFI_VFP)
    cif->abi = FFI_SYSV;

  ಪ್ರತ್ಯಾಗಮ ffi_prep_cif_machdep (cif);
}

/* Prototypes for assembly functions, in sysv.S.  */

ಸಂರಚನಾ call_frame
{
  ಶೂನ್ಯ *fp;
  ಶೂನ್ಯ *lr;
  ಶೂನ್ಯ *rvalue;
  ಪೂರ್ಣಾಙ್ಕ flags;
  ಶೂನ್ಯ *closure;
};

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV (ಶೂನ್ಯ *ರಾಶಿ, ಸಂರಚನಾ call_frame *,
			   ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ)) FFI_HIDDEN;
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_VFP (ಶೂನ್ಯ *vfp_space, ಸಂರಚನಾ call_frame *,
			   ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), unsigned vfp_used) FFI_HIDDEN;

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int (ffi_cif * cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	      ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *closure)
{
  ಪೂರ್ಣಾಙ್ಕ flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  size_t bytes, rsize, vfp_size;
  ವರ್ಣ *ರಾಶಿ, *vfp_space, *new_rvalue;
  ಸಂರಚನಾ call_frame *frame;

  rsize = 0;
  ಯದಿ (rvalue == ರಿಕ್ತ)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      ಯದಿ (flags == ARM_TYPE_STRUCT)
	rsize = rtype->size;
      ಅನ್ಯಥಾ
	flags = ARM_TYPE_VOID;
    }
  ಅನ್ಯಥಾ ಯದಿ (flags == ARM_TYPE_VFP_N)
    {
      /* Largest case is double x 4. */
      rsize = 32;
    }
  ಅನ್ಯಥಾ ಯದಿ (flags == ARM_TYPE_INT && rtype->type == FFI_TYPE_STRUCT)
    rsize = 4;

  /* Largest case.  */
  vfp_size = (cif->abi == FFI_VFP && cif->vfp_used ? 8*8: 0);

  bytes = cif->bytes;
  ರಾಶಿ = alloca (vfp_size + bytes + ಪರಿಮಾಣ(ಸಂರಚನಾ call_frame) + rsize);

  vfp_space = ರಿಕ್ತ;
  ಯದಿ (vfp_size)
    {
      vfp_space = ರಾಶಿ;
      ರಾಶಿ += vfp_size;
    }

  frame = (ಸಂರಚನಾ call_frame *)(ರಾಶಿ + bytes);

  new_rvalue = rvalue;
  ಯದಿ (rsize)
    new_rvalue = (ಶೂನ್ಯ *)(frame + 1);

  frame->rvalue = new_rvalue;
  frame->flags = flags;
  frame->closure = closure;

  ಯದಿ (vfp_space)
    {
      ffi_prep_args_VFP (cif, flags, new_rvalue, avalue, ರಾಶಿ, vfp_space);
      ffi_call_VFP (vfp_space, frame, fn, cif->vfp_used);
    }
  ಅನ್ಯಥಾ
    {
      ffi_prep_args_SYSV (cif, flags, new_rvalue, avalue, ರಾಶಿ);
      ffi_call_SYSV (ರಾಶಿ, frame, fn);
    }

  ಯದಿ (rvalue && rvalue != new_rvalue)
    memcpy (rvalue, new_rvalue, rtype->size);
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ರಿಕ್ತ);
}

#ifdef FFI_GO_CLOSURES
ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif

ಸ್ಥಿರ ಶೂನ್ಯ *
ffi_prep_incoming_args_SYSV (ffi_cif *cif, ಶೂನ್ಯ *rvalue,
			     ವರ್ಣ *argp, ಶೂನ್ಯ **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  ಪೂರ್ಣಾಙ್ಕ i, n;

  ಯದಿ (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(ಶೂನ್ಯ **) argp;
      argp += 4;
    }
  ಅನ್ಯಥಾ
    {
      ಯದಿ (cif->rtype->size && cif->rtype->size < 4)
        *(uint32_t *) rvalue = 0;
    }

  ಪ್ರತಿ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;

      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      ಯದಿ (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (ಶೂನ್ಯ *) (argp + 4 - z);
      ಅನ್ಯಥಾ
#endif
      avalue[i] = (ಶೂನ್ಯ *) argp;
      argp += z;
    }

  ಪ್ರತ್ಯಾಗಮ rvalue;
}

ಸ್ಥಿರ ಶೂನ್ಯ *
ffi_prep_incoming_args_VFP (ffi_cif *cif, ಶೂನ್ಯ *rvalue, ವರ್ಣ *ರಾಶಿ,
			    ವರ್ಣ *vfp_space, ಶೂನ್ಯ **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  ಪೂರ್ಣಾಙ್ಕ i, n, vi = 0;
  ವರ್ಣ *argp, *regp, *eo_regp;
  ವರ್ಣ done_with_regs = 0;
  ವರ್ಣ ರಾಶಿ_used = 0;

  regp = ರಾಶಿ;
  eo_regp = argp = regp + 16;

  ಯದಿ (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(ಶೂನ್ಯ **) regp;
      regp += 4;
    }

  ಪ್ರತಿ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      ಪೂರ್ಣಾಙ್ಕ is_vfp_type = vfp_type_p (ty);
      size_t z = ty->size;

      ಯದಿ (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_args[vi++] * 4;
	  ಅನುವರ್ತ;
	}
      ಅನ್ಯಥಾ ಯದಿ (!done_with_regs && !is_vfp_type)
	{
	  ವರ್ಣ *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  ಯದಿ (tregp + z <= eo_regp || !ರಾಶಿ_used)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#ifdef __ARMEB__
	      ಯದಿ (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (ಶೂನ್ಯ *) (tregp + 4 - z);
	      ಅನ್ಯಥಾ
#endif
	      avalue[i] = (ಶೂನ್ಯ *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      ಯದಿ (regp > eo_regp)
		{
		  FFI_ASSERT (!ರಾಶಿ_used);
		  argp = regp;
		}
	      ಯದಿ (regp >= eo_regp)
		{
		  done_with_regs = 1;
		  ರಾಶಿ_used = 1;
		}
	      ಅನುವರ್ತ;
	    }
	}

      ರಾಶಿ_used = 1;
      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      ಯದಿ (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (ಶೂನ್ಯ *) (argp + 4 - z);
      ಅನ್ಯಥಾ
#endif
      avalue[i] = (ಶೂನ್ಯ *) argp;
      argp += z;
    }

  ಪ್ರತ್ಯಾಗಮ rvalue;
}

#ಯದಿ FFI_CLOSURES

ಸಂರಚನಾ closure_frame
{
  ವರ್ಣ vfp_space[8*8] __attribute__((aligned(8)));
  ವರ್ಣ result[8*4];
  ವರ್ಣ argp[];
};

ಪೂರ್ಣಾಙ್ಕ FFI_HIDDEN
ffi_closure_inner_SYSV (ffi_cif *cif,
		        ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
		        ಶೂನ್ಯ *user_data,
		        ಸಂರಚನಾ closure_frame *frame)
{
  ಶೂನ್ಯ **avalue = (ಶೂನ್ಯ **) alloca (cif->nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ *));
  ಶೂನ್ಯ *rvalue = ffi_prep_incoming_args_SYSV (cif, frame->result,
					      frame->argp, avalue);
  fun (cif, rvalue, avalue, user_data);
  ಪ್ರತ್ಯಾಗಮ cif->flags;
}

ಪೂರ್ಣಾಙ್ಕ FFI_HIDDEN
ffi_closure_inner_VFP (ffi_cif *cif,
		       ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
		       ಶೂನ್ಯ *user_data,
		       ಸಂರಚನಾ closure_frame *frame)
{
  ಶೂನ್ಯ **avalue = (ಶೂನ್ಯ **) alloca (cif->nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ *));
  ಶೂನ್ಯ *rvalue = ffi_prep_incoming_args_VFP (cif, frame->result, frame->argp,
					     frame->vfp_space, avalue);
  fun (cif, rvalue, avalue, user_data);
  ಪ್ರತ್ಯಾಗಮ cif->flags;
}

ಶೂನ್ಯ ffi_closure_SYSV (ಶೂನ್ಯ) FFI_HIDDEN;
ಶೂನ್ಯ ffi_closure_VFP (ಶೂನ್ಯ) FFI_HIDDEN;
#ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
ಶೂನ್ಯ ffi_closure_SYSV_alt (ಶೂನ್ಯ) FFI_HIDDEN;
ಶೂನ್ಯ ffi_closure_VFP_alt (ಶೂನ್ಯ) FFI_HIDDEN;
#endif

#ifdef FFI_GO_CLOSURES
ಶೂನ್ಯ ffi_go_closure_SYSV (ಶೂನ್ಯ) FFI_HIDDEN;
ಶೂನ್ಯ ffi_go_closure_VFP (ಶೂನ್ಯ) FFI_HIDDEN;
#endif

/* the cif must already be prep'ed */

#ಯದಿ defined(__FreeBSD__) && defined(__arm__)
#define __clear_cache(start, end) ಕುರು { \
		ಸಂರಚನಾ arm_sync_icache_args ua; 		\
								\
		ua.addr = (uintptr_t)(start);			\
		ua.len = (ವರ್ಣ *)(end) - (ವರ್ಣ *)start;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} ಯಾವತ್ (0);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure * closure,
		      ffi_cif * cif,
		      ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
		      ಶೂನ್ಯ *user_data, ಶೂನ್ಯ *codeloc)
{
  ಶೂನ್ಯ (*closure_func) (ಶೂನ್ಯ) = ffi_closure_SYSV;

  ಯದಿ (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      ಯದಿ (cif->vfp_used)
	closure_func = ffi_closure_VFP;
    }
  ಅನ್ಯಥಾ ಯದಿ (cif->abi != FFI_SYSV)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

#ಯದಿ FFI_EXEC_TRAMPOLINE_TABLE
  ಶೂನ್ಯ **config = (ಶೂನ್ಯ **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = closure_func;
#ಅನ್ಯಥಾ

# ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
  ಯದಿ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ಯದಿ (closure_func == ffi_closure_SYSV)
        closure_func = ffi_closure_SYSV_alt;
      ಅನ್ಯಥಾ
        closure_func = ffi_closure_VFP_alt;
      ffi_tramp_set_parms (closure->ftramp, closure_func, closure);
      ಗಚ್ಛ out;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  memcpy(closure->tramp, ffi_arm_trampoline, 8);
# ಅನ್ಯಥಾ
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  memcpy(closure->tramp, (ಶೂನ್ಯ*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_CLOSURE_OFFSET);
# endif

# ಯದಿ defined(__QNX__)
  msync (closure->tramp, 8, MS_INVALIDATE_ICACHE);	/* clear data map */
  msync (codeloc, 8, MS_INVALIDATE_ICACHE);		/* clear insn map */
# elif defined(_WIN32)
  FlushInstructionCache(GetCurrentProcess(), closure->tramp, FFI_TRAMPOLINE_SIZE);
# ಅನ್ಯಥಾ
  __clear_cache(closure->tramp, closure->tramp + 8);	/* clear data map */
  __clear_cache(codeloc, codeloc + 8);			/* clear insn map */
# endif
# ifdef _WIN32
  *(ಶೂನ್ಯ(**)(ಶೂನ್ಯ))(closure->tramp + FFI_TRAMPOLINE_CLOSURE_FUNCTION) = closure_func;
# ಅನ್ಯಥಾ
  *(ಶೂನ್ಯ (**)(ಶೂನ್ಯ))(closure->tramp + 8) = closure_func;
# endif
# ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *))
{
  ಶೂನ್ಯ (*closure_func) (ಶೂನ್ಯ) = ffi_go_closure_SYSV;

  ಯದಿ (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      ಯದಿ (cif->vfp_used)
	closure_func = ffi_go_closure_VFP;
    }
  ಅನ್ಯಥಾ ಯದಿ (cif->abi != FFI_SYSV)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

  closure->tramp = closure_func;
  closure->cif = cif;
  closure->fun = fun;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}
#endif

#endif /* FFI_CLOSURES */

/* Below are routines for VFP hard-float support. */

/* A subroutine of vfp_type_p.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
is_hfa0 (ಧ್ರುವ ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  ಪೂರ್ಣಾಙ್ಕ i, ret = -1;

  ಯದಿ (elements != ರಿಕ್ತ)
    ಪ್ರತಿ (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        ಯದಿ (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            ಯದಿ (ret < 0)
              ಅನುವರ್ತ;
          }
        ಭಙ್ಗ;
      }

  ಪ್ರತ್ಯಾಗಮ ret;
}

/* A subroutine of vfp_type_p.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
is_hfa1 (ಧ್ರುವ ffi_type *ty, ಪೂರ್ಣಾಙ್ಕ candidate)
{
  ffi_type **elements = ty->elements;
  ಪೂರ್ಣಾಙ್ಕ i;

  ಯದಿ (elements != ರಿಕ್ತ)
    ಪ್ರತಿ (i = 0; elements[i]; ++i)
      {
        ಪೂರ್ಣಾಙ್ಕ t = elements[i]->type;
        ಯದಿ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
          {
            ಯದಿ (!is_hfa1 (elements[i], candidate))
              ಪ್ರತ್ಯಾಗಮ 0;
          }
        ಅನ್ಯಥಾ ಯದಿ (t != candidate)
          ಪ್ರತ್ಯಾಗಮ 0;
      }

  ಪ್ರತ್ಯಾಗಮ 1;
}

/* Determine if TY is an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is a floating point scalar.

   Returns non-zero iff TY is an HFA.  The result is an encoded value where
   bits 0-7 contain the type code, and bits 8-10 contain the element count.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
vfp_type_p (ಧ್ರುವ ffi_type *ty)
{
  ffi_type **elements;
  ಪೂರ್ಣಾಙ್ಕ candidate, i;
  size_t size, ele_ಎಣಿಕೆ;

  /* Quickest tests first.  */
  candidate = ty->type;
  ವಿಕಲ್ಪ (ty->type)
    {
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ 0;
    ಪಕ್ಷ FFI_TYPE_FLOAT:
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      ele_ಎಣಿಕೆ = 1;
      ಗಚ್ಛ done;
    ಪಕ್ಷ FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      ಯದಿ (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	ಪ್ರತ್ಯಾಗಮ 0;
      ele_ಎಣಿಕೆ = 2;
      ಗಚ್ಛ done;
    ಪಕ್ಷ FFI_TYPE_STRUCT:
      ಭಙ್ಗ;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  size = ty->size;
  ಯದಿ (size < 4 || size > 32)
    ಪ್ರತ್ಯಾಗಮ 0;

  /* Find the type of the first non-structure member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  ಯದಿ (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_COMPLEX)
    {
      ಪ್ರತಿ (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          ಯದಿ (candidate >= 0)
            ಭಙ್ಗ;
        }
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  ವಿಕಲ್ಪ (candidate)
    {
    ಪಕ್ಷ FFI_TYPE_FLOAT:
      ele_ಎಣಿಕೆ = size / ಪರಿಮಾಣ(ಭಿನ್ನ);
      ಯದಿ (size != ele_ಎಣಿಕೆ * ಪರಿಮಾಣ(ಭಿನ್ನ))
        ಪ್ರತ್ಯಾಗಮ 0;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      ele_ಎಣಿಕೆ = size / ಪರಿಮಾಣ(ದ್ವಿಗುಣ);
      ಯದಿ (size != ele_ಎಣಿಕೆ * ಪರಿಮಾಣ(ದ್ವಿಗುಣ))
        ಪ್ರತ್ಯಾಗಮ 0;
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ 0;
    }
  ಯದಿ (ele_ಎಣಿಕೆ > 4)
    ಪ್ರತ್ಯಾಗಮ 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  ಪ್ರತಿ (i = 0; elements[i]; ++i)
    {
      ಪೂರ್ಣಾಙ್ಕ t = elements[i]->type;
      ಯದಿ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          ಯದಿ (!is_hfa1 (elements[i], candidate))
            ಪ್ರತ್ಯಾಗಮ 0;
        }
      ಅನ್ಯಥಾ ಯದಿ (t != candidate)
        ಪ್ರತ್ಯಾಗಮ 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  ಪ್ರತ್ಯಾಗಮ (ele_ಎಣಿಕೆ << 8) | candidate;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
place_vfp_arg (ffi_cif *cif, ಪೂರ್ಣಾಙ್ಕ h)
{
  unsigned short reg = cif->vfp_reg_free;
  ಪೂರ್ಣಾಙ್ಕ align = 1, nregs = h >> 8;

  ಯದಿ ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  ಯದಿ ((reg & 1) && align == 2)
    reg++;

  ಯಾವತ್ (reg + nregs <= 16)
    {
      ಪೂರ್ಣಾಙ್ಕ s, new_used = 0;
      ಪ್ರತಿ (s = reg; s < reg + nregs; s++)
	{
	  new_used |= (1 << s);
	  ಯದಿ (cif->vfp_used & (1 << s))
	    {
	      reg += align;
	      ಗಚ್ಛ next_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_used |= new_used;
      cif->vfp_args[cif->vfp_nargs++] = (signed ವರ್ಣ)reg;

      /* Update vfp_reg_free. */
      ಯದಿ (cif->vfp_used & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  ಯಾವತ್ (cif->vfp_used & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      ಪ್ರತ್ಯಾಗಮ 0;
    next_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_used = 0xFFFF;
  ಪ್ರತ್ಯಾಗಮ 1;
}

ಸ್ಥಿರ ಶೂನ್ಯ
layout_vfp_args (ffi_cif * cif)
{
  unsigned ಪೂರ್ಣಾಙ್ಕ i;
  /* Init VFP fields */
  cif->vfp_used = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  memset (cif->vfp_args, -1, 16);	/* Init to -1. */

  ಪ್ರತಿ (i = 0; i < cif->nargs; i++)
    {
      ಪೂರ್ಣಾಙ್ಕ h = vfp_type_p (cif->arg_types[i]);
      ಯದಿ (h && place_vfp_arg (cif, h) == 1)
	ಭಙ್ಗ;
    }
}

#ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
ಶೂನ್ಯ *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  ಬಾಹ್ಯ ಶೂನ್ಯ *trampoline_code_table;

  *tramp_size = ARM_TRAMP_SIZE;
  *map_size = ARM_TRAMP_MAP_SIZE;
  ಪ್ರತ್ಯಾಗಮ &trampoline_code_table;
}
#endif

#endif /* __arm__ or _M_ARM */
