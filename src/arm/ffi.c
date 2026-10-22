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

#ਜੇ defined(__arm__) || defined(_M_ARM)
#ਅਨ੍ਤਰ੍ਭਾਵ <fficonfig.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <tramp.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "internal.h"

#ਜੇ defined(_WIN32)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WIN32_LEAN_AND_ਔਸਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <windows.h>
#ਜੇ_ਅੰਤ

#ਜੇ FFI_EXEC_TRAMPOLINE_ਸਾਰਣੀ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __MACH__
#ਅਨ੍ਤਰ੍ਭਾਵ <mach/machine/vm_param.h>
#ਜੇ_ਅੰਤ

#ਨਹੀਂਤਾਂ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _WIN32
ਬਾਹਰੀ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ffi_arm_trampoline[2] FFI_HIDDEN;
#ਨਹੀਂਤਾਂ
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
ਬਾਹਰੀ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ffi_arm_trampoline[12] FFI_HIDDEN;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

#ਜੇ defined(__FreeBSD__) && defined(__arm__)
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <machine/sysarch.h>
#ਜੇ_ਅੰਤ

#ਜੇ defined(__QNX__)
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/mman.h>
#ਜੇ_ਅੰਤ

/* Forward declares. */
ਸਥਿਰ ਪੂਰਨ vfp_type_p (ਅਚਲ ffi_type *);
ਸਥਿਰ ਖਾਲੀ layout_vfp_ਦਲੀਲਾਂ (ffi_cif *);

ਸਥਿਰ ਖਾਲੀ *
ffi_align (ffi_type *ty, ਖਾਲੀ *p)
{
  /* Align if necessary */
  size_t alignment;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32_WCE
  alignment = 4;
#ਨਹੀਂਤਾਂ
  alignment = ty->alignment;
  ਜੇ (alignment < 4)
    alignment = 4;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ (ਖਾਲੀ *) FFI_ALIGN (p, alignment);
}

ਸਥਿਰ size_t
ffi_put_arg (ffi_type *ty, ਖਾਲੀ *src, ਖਾਲੀ *dst)
{
  size_t z = ty->ਮਾਪ;

  ਚੋਣ (ty->type)
    {
    ਹਾਲਤ FFI_TYPE_SINT8:
      *(UINT32 *)dst = *(SINT8 *)src;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT8:
      *(UINT32 *)dst = *(UINT8 *)src;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT16:
      *(UINT32 *)dst = *(SINT16 *)src;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT16:
      *(UINT32 *)dst = *(UINT16 *)src;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_INT:
    ਹਾਲਤ FFI_TYPE_SINT32:
    ਹਾਲਤ FFI_TYPE_UINT32:
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _WIN32
    ਹਾਲਤ FFI_TYPE_FLOAT:
#ਜੇ_ਅੰਤ
      *(UINT32 *)dst = *(UINT32 *)src;
      ਰੋਕੋ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
    // casting a float* to a UINT32* doesn't work on Windows
    ਹਾਲਤ FFI_TYPE_FLOAT:
        *(uintptr_t *)dst = 0;
        *(ਦਸ਼ਮਲਵ *)dst = *(ਦਸ਼ਮਲਵ *)src;
        ਰੋਕੋ;
#ਜੇ_ਅੰਤ

    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      *(UINT64 *)dst = *(UINT64 *)src;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_STRUCT:
    ਹਾਲਤ FFI_TYPE_COMPLEX:
      ਸਮ੍ਰਿਤੀ_ਨਕਲ (dst, src, z);
      ਰੋਕੋ;

    ਮੂਲ:
      abort();
    }

  ਵਾਪਸ FFI_ALIGN (z, 4);
}

/* ffi_prep_args is called once stack space has been allocated
   for the function's arguments.

   The vfp_space parameter is the load area for VFP regs, the return
   value is cif->vfp_used (word bitset of VFP regs used for passing
   arguments). These are only used for the VFP hard-float ABI.
*/
ਸਥਿਰ ਖਾਲੀ
ffi_prep_ਦਲੀਲਾਂ_SYSV (ffi_cif *cif, ਪੂਰਨ flags, ਖਾਲੀ *rvalue,
		    ਖਾਲੀ **avalue, ਅੱਖਰ *argp)
{
  ffi_type **arg_types = cif->arg_types;
  ਪੂਰਨ i, n;

  ਜੇ (flags == ARM_TYPE_STRUCT)
    {
      *(ਖਾਲੀ **) argp = rvalue;
      argp += 4;
    }

  ਲਈ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, avalue[i], argp);
    }
}

ਸਥਿਰ ਖਾਲੀ
ffi_prep_ਦਲੀਲਾਂ_VFP (ffi_cif *cif, ਪੂਰਨ flags, ਖਾਲੀ *rvalue,
                   ਖਾਲੀ **avalue, ਅੱਖਰ *ਢੇਰ, ਅੱਖਰ *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  ਪੂਰਨ i, n, vi = 0;
  ਅੱਖਰ *argp, *regp, *eo_regp;
  ਅੱਖਰ ਢੇਰ_ਵਰਤਿਆ = 0;
  ਅੱਖਰ ਪੂਰਾ_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = ਢੇਰ;
  eo_regp = argp = regp + 16;

  /* If the function returns an FFI_TYPE_STRUCT in memory,
     that address is passed in r0 to the function.  */
  ਜੇ (flags == ARM_TYPE_STRUCT)
    {
      *(ਖਾਲੀ **) regp = rvalue;
      regp += 4;
    }

  ਲਈ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      ਖਾਲੀ *a = avalue[i];
      ਪੂਰਨ is_vfp_type = vfp_type_p (ty);

      /* Allocated in VFP registers. */
      ਜੇ (vi < cif->vfp_nargs && is_vfp_type)
	{
	  ਅੱਖਰ *vfp_ਥਾਂ = vfp_space + cif->vfp_ਦਲੀਲਾਂ[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_ਥਾਂ);
	  ਜਾਰੀ;
	}
      /* Try allocating in core registers. */
      ਨਹੀਂਤਾਂ ਜੇ (!ਪੂਰਾ_with_regs && !is_vfp_type)
	{
	  ਅੱਖਰ *tregp = ffi_align (ty, regp);
	  size_t ਮਾਪ = ty->ਮਾਪ;
	  ਮਾਪ = (ਮਾਪ < 4) ? 4 : ਮਾਪ;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  ਜੇ (tregp + ਮਾਪ <= eo_regp)
	    {
	      regp = tregp + ffi_put_arg (ty, a, tregp);
	      ਪੂਰਾ_with_regs = (regp == argp);
	      // ensure we did not write into the stack area
	      FFI_ASSERT (regp <= argp);
	      ਜਾਰੀ;
	    }
	  /* In case there are no arguments in the stack area yet,
	     the argument is passed in the remaining core registers
	     and on the stack.  */
	  ਨਹੀਂਤਾਂ ਜੇ (!ਢੇਰ_ਵਰਤਿਆ)
	    {
	      ਢੇਰ_ਵਰਤਿਆ = 1;
	      ਪੂਰਾ_with_regs = 1;
	      argp = tregp + ffi_put_arg (ty, a, tregp);
	      FFI_ASSERT (eo_regp < argp);
	      ਜਾਰੀ;
	    }
	}
      /* Base case, arguments are passed on the stack */
      ਢੇਰ_ਵਰਤਿਆ = 1;
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, a, argp);
    }
}

/* Perform machine dependent cif processing */
ffi_ਸਥਿਤੀ FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ਪੂਰਨ flags = 0, cabi = cif->abi;
  size_t ਬਾਈਟਾਂ = cif->ਬਾਈਟਾਂ;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  ਜੇ (cabi == FFI_VFP)
    layout_vfp_ਦਲੀਲਾਂ (cif);

  /* Set the return type flag */
  ਚੋਣ (cif->rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_VOID:
      flags = ARM_TYPE_VOID;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_INT:
    ਹਾਲਤ FFI_TYPE_UINT8:
    ਹਾਲਤ FFI_TYPE_SINT8:
    ਹਾਲਤ FFI_TYPE_UINT16:
    ਹਾਲਤ FFI_TYPE_SINT16:
    ਹਾਲਤ FFI_TYPE_UINT32:
    ਹਾਲਤ FFI_TYPE_SINT32:
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
      flags = ARM_TYPE_INT;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_UINT64:
      flags = ARM_TYPE_INT64;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_FLOAT:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_S : ARM_TYPE_INT);
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_D : ARM_TYPE_INT64);
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_STRUCT:
    ਹਾਲਤ FFI_TYPE_COMPLEX:
      ਜੇ (cabi == FFI_VFP)
	{
	  ਪੂਰਨ h = vfp_type_p (cif->rtype);

	  flags = ARM_TYPE_VFP_N;
	  ਜੇ (h == 0x100 + FFI_TYPE_FLOAT)
	    flags = ARM_TYPE_VFP_S;
	  ਜੇ (h == 0x100 + FFI_TYPE_DOUBLE)
	    flags = ARM_TYPE_VFP_D;
	  ਜੇ (h != 0)
	      ਰੋਕੋ;
	}

      /* A Composite Type not larger than 4 bytes is returned in r0.
	 A Composite Type larger than 4 bytes, or whose size cannot
	 be determined statically ... is stored in memory at an
	 address passed [in r0].  */
      ਜੇ (cif->rtype->ਮਾਪ <= 4)
	flags = ARM_TYPE_INT;
      ਨਹੀਂਤਾਂ
	{
	  flags = ARM_TYPE_STRUCT;
	  ਬਾਈਟਾਂ += 4;
	}
      ਰੋਕੋ;

    ਮੂਲ:
      abort();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  ਬਾਈਟਾਂ = FFI_ALIGN (ਬਾਈਟਾਂ, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  ਜੇ (ਬਾਈਟਾਂ < 4*4)
    ਬਾਈਟਾਂ = 4*4;

  cif->ਬਾਈਟਾਂ = ਬਾਈਟਾਂ;
  cif->flags = flags;

  ਵਾਪਸ FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_ਸਥਿਤੀ FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nfixedargs, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ntotalargs)
{
  /* VFP variadic calls actually use the SYSV ABI */
  ਜੇ (cif->abi == FFI_VFP)
    cif->abi = FFI_SYSV;

  ਵਾਪਸ ffi_prep_cif_machdep (cif);
}

/* Prototypes for assembly functions, in sysv.S.  */

ਬਣਤਰ call_ਚੌਖਟਾ
{
  ਖਾਲੀ *fp;
  ਖਾਲੀ *lr;
  ਖਾਲੀ *rvalue;
  ਪੂਰਨ flags;
  ਖਾਲੀ *ਘੇਰਾ;
};

ਬਾਹਰੀ ਖਾਲੀ ffi_call_SYSV (ਖਾਲੀ *ਢੇਰ, ਬਣਤਰ call_ਚੌਖਟਾ *,
			   ਖਾਲੀ (*fn) (ਖਾਲੀ)) FFI_HIDDEN;
ਬਾਹਰੀ ਖਾਲੀ ffi_call_VFP (ਖਾਲੀ *vfp_space, ਬਣਤਰ call_ਚੌਖਟਾ *,
			   ਖਾਲੀ (*fn) (ਖਾਲੀ), ਬਿਨਾਨਿਸ਼ਾਨੀ vfp_ਵਰਤਿਆ) FFI_HIDDEN;

ਸਥਿਰ ਖਾਲੀ
ffi_call_int (ffi_cif * cif, ਖਾਲੀ (*fn) (ਖਾਲੀ), ਖਾਲੀ *rvalue,
	      ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
  ਪੂਰਨ flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  size_t ਬਾਈਟਾਂ, rsize, vfp_ਮਾਪ;
  ਅੱਖਰ *ਢੇਰ, *vfp_space, *new_rvalue;
  ਬਣਤਰ call_ਚੌਖਟਾ *ਚੌਖਟਾ;

  rsize = 0;
  ਜੇ (rvalue == ਨਿਰਾ)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      ਜੇ (flags == ARM_TYPE_STRUCT)
	rsize = rtype->ਮਾਪ;
      ਨਹੀਂਤਾਂ
	flags = ARM_TYPE_VOID;
    }
  ਨਹੀਂਤਾਂ ਜੇ (flags == ARM_TYPE_VFP_N)
    {
      /* Largest case is double x 4. */
      rsize = 32;
    }
  ਨਹੀਂਤਾਂ ਜੇ (flags == ARM_TYPE_INT && rtype->type == FFI_TYPE_STRUCT)
    rsize = 4;

  /* Largest case.  */
  vfp_ਮਾਪ = (cif->abi == FFI_VFP && cif->vfp_ਵਰਤਿਆ ? 8*8: 0);

  ਬਾਈਟਾਂ = cif->ਬਾਈਟਾਂ;
  ਢੇਰ = alloca (vfp_ਮਾਪ + ਬਾਈਟਾਂ + ਆਕਾਰ(ਬਣਤਰ call_ਚੌਖਟਾ) + rsize);

  vfp_space = ਨਿਰਾ;
  ਜੇ (vfp_ਮਾਪ)
    {
      vfp_space = ਢੇਰ;
      ਢੇਰ += vfp_ਮਾਪ;
    }

  ਚੌਖਟਾ = (ਬਣਤਰ call_ਚੌਖਟਾ *)(ਢੇਰ + ਬਾਈਟਾਂ);

  new_rvalue = rvalue;
  ਜੇ (rsize)
    new_rvalue = (ਖਾਲੀ *)(ਚੌਖਟਾ + 1);

  ਚੌਖਟਾ->rvalue = new_rvalue;
  ਚੌਖਟਾ->flags = flags;
  ਚੌਖਟਾ->ਘੇਰਾ = ਘੇਰਾ;

  ਜੇ (vfp_space)
    {
      ffi_prep_ਦਲੀਲਾਂ_VFP (cif, flags, new_rvalue, avalue, ਢੇਰ, vfp_space);
      ffi_call_VFP (vfp_space, ਚੌਖਟਾ, fn, cif->vfp_ਵਰਤਿਆ);
    }
  ਨਹੀਂਤਾਂ
    {
      ffi_prep_ਦਲੀਲਾਂ_SYSV (cif, flags, new_rvalue, avalue, ਢੇਰ);
      ffi_call_SYSV (ਢੇਰ, ਚੌਖਟਾ, fn);
    }

  ਜੇ (rvalue && rvalue != new_rvalue)
    ਸਮ੍ਰਿਤੀ_ਨਕਲ (rvalue, new_rvalue, rtype->ਮਾਪ);
}

ਖਾਲੀ
ffi_call (ffi_cif *cif, ਖਾਲੀ (*fn) (ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ਨਿਰਾ);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_GO_CLOSURES
ਖਾਲੀ
ffi_call_go (ffi_cif *cif, ਖਾਲੀ (*fn) (ਖਾਲੀ), ਖਾਲੀ *rvalue,
	     ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
  ffi_call_int (cif, fn, rvalue, avalue, ਘੇਰਾ);
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਖਾਲੀ *
ffi_prep_incoming_ਦਲੀਲਾਂ_SYSV (ffi_cif *cif, ਖਾਲੀ *rvalue,
			     ਅੱਖਰ *argp, ਖਾਲੀ **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  ਪੂਰਨ i, n;

  ਜੇ (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(ਖਾਲੀ **) argp;
      argp += 4;
    }
  ਨਹੀਂਤਾਂ
    {
      ਜੇ (cif->rtype->ਮਾਪ && cif->rtype->ਮਾਪ < 4)
        *(uint32_t *) rvalue = 0;
    }

  ਲਈ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->ਮਾਪ;

      argp = ffi_align (ty, argp);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __ARMEB__
      ਜੇ (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (ਖਾਲੀ *) (argp + 4 - z);
      ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
      avalue[i] = (ਖਾਲੀ *) argp;
      argp += z;
    }

  ਵਾਪਸ rvalue;
}

ਸਥਿਰ ਖਾਲੀ *
ffi_prep_incoming_ਦਲੀਲਾਂ_VFP (ffi_cif *cif, ਖਾਲੀ *rvalue, ਅੱਖਰ *ਢੇਰ,
			    ਅੱਖਰ *vfp_space, ਖਾਲੀ **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  ਪੂਰਨ i, n, vi = 0;
  ਅੱਖਰ *argp, *regp, *eo_regp;
  ਅੱਖਰ ਪੂਰਾ_with_regs = 0;
  ਅੱਖਰ ਢੇਰ_ਵਰਤਿਆ = 0;

  regp = ਢੇਰ;
  eo_regp = argp = regp + 16;

  ਜੇ (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(ਖਾਲੀ **) regp;
      regp += 4;
    }

  ਲਈ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      ਪੂਰਨ is_vfp_type = vfp_type_p (ty);
      size_t z = ty->ਮਾਪ;

      ਜੇ (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_ਦਲੀਲਾਂ[vi++] * 4;
	  ਜਾਰੀ;
	}
      ਨਹੀਂਤਾਂ ਜੇ (!ਪੂਰਾ_with_regs && !is_vfp_type)
	{
	  ਅੱਖਰ *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  ਜੇ (tregp + z <= eo_regp || !ਢੇਰ_ਵਰਤਿਆ)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __ARMEB__
	      ਜੇ (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (ਖਾਲੀ *) (tregp + 4 - z);
	      ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
	      avalue[i] = (ਖਾਲੀ *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      ਜੇ (regp > eo_regp)
		{
		  FFI_ASSERT (!ਢੇਰ_ਵਰਤਿਆ);
		  argp = regp;
		}
	      ਜੇ (regp >= eo_regp)
		{
		  ਪੂਰਾ_with_regs = 1;
		  ਢੇਰ_ਵਰਤਿਆ = 1;
		}
	      ਜਾਰੀ;
	    }
	}

      ਢੇਰ_ਵਰਤਿਆ = 1;
      argp = ffi_align (ty, argp);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __ARMEB__
      ਜੇ (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (ਖਾਲੀ *) (argp + 4 - z);
      ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
      avalue[i] = (ਖਾਲੀ *) argp;
      argp += z;
    }

  ਵਾਪਸ rvalue;
}

#ਜੇ FFI_CLOSURES

ਬਣਤਰ ਘੇਰਾ_ਚੌਖਟਾ
{
  ਅੱਖਰ vfp_space[8*8] __attribute__((aligned(8)));
  ਅੱਖਰ ਨਤੀਜਾ[8*4];
  ਅੱਖਰ argp[];
};

ਪੂਰਨ FFI_HIDDEN
ffi_ਘੇਰਾ_inner_SYSV (ffi_cif *cif,
		        ਖਾਲੀ (*fun) (ffi_cif *, ਖਾਲੀ *, ਖਾਲੀ **, ਖਾਲੀ *),
		        ਖਾਲੀ *user_ਅੰਕੜੇ,
		        ਬਣਤਰ ਘੇਰਾ_ਚੌਖਟਾ *ਚੌਖਟਾ)
{
  ਖਾਲੀ **avalue = (ਖਾਲੀ **) alloca (cif->nargs * ਆਕਾਰ (ਖਾਲੀ *));
  ਖਾਲੀ *rvalue = ffi_prep_incoming_ਦਲੀਲਾਂ_SYSV (cif, ਚੌਖਟਾ->ਨਤੀਜਾ,
					      ਚੌਖਟਾ->argp, avalue);
  fun (cif, rvalue, avalue, user_ਅੰਕੜੇ);
  ਵਾਪਸ cif->flags;
}

ਪੂਰਨ FFI_HIDDEN
ffi_ਘੇਰਾ_inner_VFP (ffi_cif *cif,
		       ਖਾਲੀ (*fun) (ffi_cif *, ਖਾਲੀ *, ਖਾਲੀ **, ਖਾਲੀ *),
		       ਖਾਲੀ *user_ਅੰਕੜੇ,
		       ਬਣਤਰ ਘੇਰਾ_ਚੌਖਟਾ *ਚੌਖਟਾ)
{
  ਖਾਲੀ **avalue = (ਖਾਲੀ **) alloca (cif->nargs * ਆਕਾਰ (ਖਾਲੀ *));
  ਖਾਲੀ *rvalue = ffi_prep_incoming_ਦਲੀਲਾਂ_VFP (cif, ਚੌਖਟਾ->ਨਤੀਜਾ, ਚੌਖਟਾ->argp,
					     ਚੌਖਟਾ->vfp_space, avalue);
  fun (cif, rvalue, avalue, user_ਅੰਕੜੇ);
  ਵਾਪਸ cif->flags;
}

ਖਾਲੀ ffi_ਘੇਰਾ_SYSV (ਖਾਲੀ) FFI_HIDDEN;
ਖਾਲੀ ffi_ਘੇਰਾ_VFP (ਖਾਲੀ) FFI_HIDDEN;
#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
ਖਾਲੀ ffi_ਘੇਰਾ_SYSV_alt (ਖਾਲੀ) FFI_HIDDEN;
ਖਾਲੀ ffi_ਘੇਰਾ_VFP_alt (ਖਾਲੀ) FFI_HIDDEN;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_GO_CLOSURES
ਖਾਲੀ ffi_go_ਘੇਰਾ_SYSV (ਖਾਲੀ) FFI_HIDDEN;
ਖਾਲੀ ffi_go_ਘੇਰਾ_VFP (ਖਾਲੀ) FFI_HIDDEN;
#ਜੇ_ਅੰਤ

/* the cif must already be prep'ed */

#ਜੇ defined(__FreeBSD__) && defined(__arm__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __clear_cache(ਸ਼ੁਰੂ, ਸਿਰਾ) ਕਰੋ { \
		ਬਣਤਰ arm_sync_icache_ਦਲੀਲਾਂ ua; 		\
								\
		ua.addr = (uintptr_t)(ਸ਼ੁਰੂ);			\
		ua.len = (ਅੱਖਰ *)(ਸਿਰਾ) - (ਅੱਖਰ *)ਸ਼ੁਰੂ;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} ਜਦੋਂ (0);
#ਜੇ_ਅੰਤ

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ * ਘੇਰਾ,
		      ffi_cif * cif,
		      ਖਾਲੀ (*fun) (ffi_cif *, ਖਾਲੀ *, ਖਾਲੀ **, ਖਾਲੀ *),
		      ਖਾਲੀ *user_ਅੰਕੜੇ, ਖਾਲੀ *codeloc)
{
  ਖਾਲੀ (*ਘੇਰਾ_func) (ਖਾਲੀ) = ffi_ਘੇਰਾ_SYSV;

  ਜੇ (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      ਜੇ (cif->vfp_ਵਰਤਿਆ)
	ਘੇਰਾ_func = ffi_ਘੇਰਾ_VFP;
    }
  ਨਹੀਂਤਾਂ ਜੇ (cif->abi != FFI_SYSV)
    ਵਾਪਸ FFI_BAD_ABI;

#ਜੇ FFI_EXEC_TRAMPOLINE_ਸਾਰਣੀ
  ਖਾਲੀ **config = (ਖਾਲੀ **)((uint8_t *)codeloc - ਪੰਨਾ_MAX_ਮਾਪ);
  config[0] = ਘੇਰਾ;
  config[1] = ਘੇਰਾ_func;
#ਨਹੀਂਤਾਂ

# ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
  ਜੇ (ffi_tramp_is_present(ਘੇਰਾ))
    {
      /* Initialize the static trampoline's parameters. */
      ਜੇ (ਘੇਰਾ_func == ffi_ਘੇਰਾ_SYSV)
        ਘੇਰਾ_func = ffi_ਘੇਰਾ_SYSV_alt;
      ਨਹੀਂਤਾਂ
        ਘੇਰਾ_func = ffi_ਘੇਰਾ_VFP_alt;
      ffi_tramp_ਸੈੱਟ_parms (ਘੇਰਾ->ftramp, ਘੇਰਾ_func, ਘੇਰਾ);
      ਜਾਓ ਬਾਹਰਲਾ;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਘੇਰਾ->tramp, ffi_arm_trampoline, 8);
# ਨਹੀਂਤਾਂ
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਘੇਰਾ->tramp, (ਖਾਲੀ*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_ਘੇਰਾ_ਵਿੱਥ);
# endif

# ਜੇ defined(__QNX__)
  msync (ਘੇਰਾ->tramp, 8, MS_INVALIDATE_ICACHE);	/* clear data map */
  msync (codeloc, 8, MS_INVALIDATE_ICACHE);		/* clear insn map */
# elif defined(_WIN32)
  FlushInstructionCache(GetCurrentProcess(), ਘੇਰਾ->tramp, FFI_TRAMPOLINE_ਮਾਪ);
# ਨਹੀਂਤਾਂ
  __clear_cache(ਘੇਰਾ->tramp, ਘੇਰਾ->tramp + 8);	/* clear data map */
  __clear_cache(codeloc, codeloc + 8);			/* clear insn map */
# endif
# ifdef _WIN32
  *(ਖਾਲੀ(**)(ਖਾਲੀ))(ਘੇਰਾ->tramp + FFI_TRAMPOLINE_ਘੇਰਾ_FUNCTION) = ਘੇਰਾ_func;
# ਨਹੀਂਤਾਂ
  *(ਖਾਲੀ (**)(ਖਾਲੀ))(ਘੇਰਾ->tramp + 8) = ਘੇਰਾ_func;
# endif
# ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
ਬਾਹਰਲਾ:
# endif
#ਜੇ_ਅੰਤ

  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

  ਵਾਪਸ FFI_OK;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_GO_CLOSURES
ffi_ਸਥਿਤੀ
ffi_prep_go_ਘੇਰਾ (ffi_go_ਘੇਰਾ *ਘੇਰਾ, ffi_cif *cif,
		     ਖਾਲੀ (*fun) (ffi_cif *, ਖਾਲੀ *, ਖਾਲੀ **, ਖਾਲੀ *))
{
  ਖਾਲੀ (*ਘੇਰਾ_func) (ਖਾਲੀ) = ffi_go_ਘੇਰਾ_SYSV;

  ਜੇ (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      ਜੇ (cif->vfp_ਵਰਤਿਆ)
	ਘੇਰਾ_func = ffi_go_ਘੇਰਾ_VFP;
    }
  ਨਹੀਂਤਾਂ ਜੇ (cif->abi != FFI_SYSV)
    ਵਾਪਸ FFI_BAD_ABI;

  ਘੇਰਾ->tramp = ਘੇਰਾ_func;
  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;

  ਵਾਪਸ FFI_OK;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* FFI_CLOSURES */

/* Below are routines for VFP hard-float support. */

/* A subroutine of vfp_type_p.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

ਸਥਿਰ ਪੂਰਨ
is_hfa0 (ਅਚਲ ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  ਪੂਰਨ i, ret = -1;

  ਜੇ (elements != ਨਿਰਾ)
    ਲਈ (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        ਜੇ (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            ਜੇ (ret < 0)
              ਜਾਰੀ;
          }
        ਰੋਕੋ;
      }

  ਵਾਪਸ ret;
}

/* A subroutine of vfp_type_p.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

ਸਥਿਰ ਪੂਰਨ
is_hfa1 (ਅਚਲ ffi_type *ty, ਪੂਰਨ candidate)
{
  ffi_type **elements = ty->elements;
  ਪੂਰਨ i;

  ਜੇ (elements != ਨਿਰਾ)
    ਲਈ (i = 0; elements[i]; ++i)
      {
        ਪੂਰਨ t = elements[i]->type;
        ਜੇ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
          {
            ਜੇ (!is_hfa1 (elements[i], candidate))
              ਵਾਪਸ 0;
          }
        ਨਹੀਂਤਾਂ ਜੇ (t != candidate)
          ਵਾਪਸ 0;
      }

  ਵਾਪਸ 1;
}

/* Determine if TY is an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is a floating point scalar.

   Returns non-zero iff TY is an HFA.  The result is an encoded value where
   bits 0-7 contain the type code, and bits 8-10 contain the element count.  */

ਸਥਿਰ ਪੂਰਨ
vfp_type_p (ਅਚਲ ffi_type *ty)
{
  ffi_type **elements;
  ਪੂਰਨ candidate, i;
  size_t ਮਾਪ, ele_ਗਿਣਤੀ;

  /* Quickest tests first.  */
  candidate = ty->type;
  ਚੋਣ (ty->type)
    {
    ਮੂਲ:
      ਵਾਪਸ 0;
    ਹਾਲਤ FFI_TYPE_FLOAT:
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      ele_ਗਿਣਤੀ = 1;
      ਜਾਓ ਪੂਰਾ;
    ਹਾਲਤ FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      ਜੇ (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	ਵਾਪਸ 0;
      ele_ਗਿਣਤੀ = 2;
      ਜਾਓ ਪੂਰਾ;
    ਹਾਲਤ FFI_TYPE_STRUCT:
      ਰੋਕੋ;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  ਮਾਪ = ty->ਮਾਪ;
  ਜੇ (ਮਾਪ < 4 || ਮਾਪ > 32)
    ਵਾਪਸ 0;

  /* Find the type of the first non-structure member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  ਜੇ (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_COMPLEX)
    {
      ਲਈ (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          ਜੇ (candidate >= 0)
            ਰੋਕੋ;
        }
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  ਚੋਣ (candidate)
    {
    ਹਾਲਤ FFI_TYPE_FLOAT:
      ele_ਗਿਣਤੀ = ਮਾਪ / ਆਕਾਰ(ਦਸ਼ਮਲਵ);
      ਜੇ (ਮਾਪ != ele_ਗਿਣਤੀ * ਆਕਾਰ(ਦਸ਼ਮਲਵ))
        ਵਾਪਸ 0;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      ele_ਗਿਣਤੀ = ਮਾਪ / ਆਕਾਰ(ਦੁੱਗਣਾ);
      ਜੇ (ਮਾਪ != ele_ਗਿਣਤੀ * ਆਕਾਰ(ਦੁੱਗਣਾ))
        ਵਾਪਸ 0;
      ਰੋਕੋ;
    ਮੂਲ:
      ਵਾਪਸ 0;
    }
  ਜੇ (ele_ਗਿਣਤੀ > 4)
    ਵਾਪਸ 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  ਲਈ (i = 0; elements[i]; ++i)
    {
      ਪੂਰਨ t = elements[i]->type;
      ਜੇ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          ਜੇ (!is_hfa1 (elements[i], candidate))
            ਵਾਪਸ 0;
        }
      ਨਹੀਂਤਾਂ ਜੇ (t != candidate)
        ਵਾਪਸ 0;
    }

  /* All tests succeeded.  Encode the result.  */
 ਪੂਰਾ:
  ਵਾਪਸ (ele_ਗਿਣਤੀ << 8) | candidate;
}

ਸਥਿਰ ਪੂਰਨ
ਰੱਖੋ_vfp_arg (ffi_cif *cif, ਪੂਰਨ h)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ reg = cif->vfp_reg_free;
  ਪੂਰਨ align = 1, nregs = h >> 8;

  ਜੇ ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  ਜੇ ((reg & 1) && align == 2)
    reg++;

  ਜਦੋਂ (reg + nregs <= 16)
    {
      ਪੂਰਨ s, new_ਵਰਤਿਆ = 0;
      ਲਈ (s = reg; s < reg + nregs; s++)
	{
	  new_ਵਰਤਿਆ |= (1 << s);
	  ਜੇ (cif->vfp_ਵਰਤਿਆ & (1 << s))
	    {
	      reg += align;
	      ਜਾਓ ਅਗਲਾ_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_ਵਰਤਿਆ |= new_ਵਰਤਿਆ;
      cif->vfp_ਦਲੀਲਾਂ[cif->vfp_nargs++] = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ)reg;

      /* Update vfp_reg_free. */
      ਜੇ (cif->vfp_ਵਰਤਿਆ & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  ਜਦੋਂ (cif->vfp_ਵਰਤਿਆ & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      ਵਾਪਸ 0;
    ਅਗਲਾ_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_ਵਰਤਿਆ = 0xFFFF;
  ਵਾਪਸ 1;
}

ਸਥਿਰ ਖਾਲੀ
layout_vfp_ਦਲੀਲਾਂ (ffi_cif * cif)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  /* Init VFP fields */
  cif->vfp_ਵਰਤਿਆ = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  memset (cif->vfp_ਦਲੀਲਾਂ, -1, 16);	/* Init to -1. */

  ਲਈ (i = 0; i < cif->nargs; i++)
    {
      ਪੂਰਨ h = vfp_type_p (cif->arg_types[i]);
      ਜੇ (h && ਰੱਖੋ_vfp_arg (cif, h) == 1)
	ਰੋਕੋ;
    }
}

#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
ਖਾਲੀ *
ffi_tramp_arch (size_t *tramp_ਮਾਪ, size_t *map_ਮਾਪ)
{
  ਬਾਹਰੀ ਖਾਲੀ *trampoline_ਕੋਡ_ਸਾਰਣੀ;

  *tramp_ਮਾਪ = ARM_TRAMP_ਮਾਪ;
  *map_ਮਾਪ = ARM_TRAMP_MAP_ਮਾਪ;
  ਵਾਪਸ &trampoline_ਕੋਡ_ਸਾਰਣੀ;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* __arm__ or _M_ARM */
