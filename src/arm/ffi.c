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

#إذا defined(__arm__) || defined(_M_ARM)
#ضمن <fficonfig.h>
#ضمن <ffi.h>
#ضمن <ffi_common.h>
#ضمن <stdint.h>
#ضمن <stdlib.h>
#ضمن <tramp.h>
#ضمن "internal.h"

#إذا defined(_WIN32)
#وصف_ڪريو WIN32_LEAN_AND_سراسري
#ضمن <windows.h>
#جيڪڏهن_پورو

#إذا FFI_EXEC_TRAMPOLINE_جدول

#جيڪڏهن_وصف_ٿيل __MACH__
#ضمن <mach/machine/vm_param.h>
#جيڪڏهن_پورو

#وإلا
#جيڪڏهن_وصف_نه_ٿيل _WIN32
خارجي بنانشان صحيح ffi_arm_trampoline[2] FFI_HIDDEN;
#وإلا
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
خارجي بنانشان حرف ffi_arm_trampoline[12] FFI_HIDDEN;
#جيڪڏهن_پورو
#جيڪڏهن_پورو

#إذا defined(__FreeBSD__) && defined(__arm__)
#ضمن <sys/types.h>
#ضمن <machine/sysarch.h>
#جيڪڏهن_پورو

#إذا defined(__QNX__)
#ضمن <sys/mman.h>
#جيڪڏهن_پورو

/* Forward declares. */
ساكن صحيح vfp_type_p (ثابت ffi_type *);
ساكن فراغ layout_vfp_دليلون (ffi_cif *);

ساكن فراغ *
ffi_align (ffi_type *ty, فراغ *p)
{
  /* Align if necessary */
  size_t alignment;
#جيڪڏهن_وصف_ٿيل _WIN32_WCE
  alignment = 4;
#وإلا
  alignment = ty->alignment;
  إذا (alignment < 4)
    alignment = 4;
#جيڪڏهن_پورو
  أرجع (فراغ *) FFI_ALIGN (p, alignment);
}

ساكن size_t
ffi_put_arg (ffi_type *ty, فراغ *src, فراغ *dst)
{
  size_t z = ty->سائيز;

  بدل (ty->type)
    {
    حالة FFI_TYPE_SINT8:
      *(UINT32 *)dst = *(SINT8 *)src;
      اكسر;
    حالة FFI_TYPE_UINT8:
      *(UINT32 *)dst = *(UINT8 *)src;
      اكسر;
    حالة FFI_TYPE_SINT16:
      *(UINT32 *)dst = *(SINT16 *)src;
      اكسر;
    حالة FFI_TYPE_UINT16:
      *(UINT32 *)dst = *(UINT16 *)src;
      اكسر;

    حالة FFI_TYPE_INT:
    حالة FFI_TYPE_SINT32:
    حالة FFI_TYPE_UINT32:
    حالة FFI_TYPE_اشارو:
#جيڪڏهن_وصف_نه_ٿيل _WIN32
    حالة FFI_TYPE_FLOAT:
#جيڪڏهن_پورو
      *(UINT32 *)dst = *(UINT32 *)src;
      اكسر;

#جيڪڏهن_وصف_ٿيل _WIN32
    // casting a float* to a UINT32* doesn't work on Windows
    حالة FFI_TYPE_FLOAT:
        *(uintptr_t *)dst = 0;
        *(عائم *)dst = *(عائم *)src;
        اكسر;
#جيڪڏهن_پورو

    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_DOUBLE:
      *(UINT64 *)dst = *(UINT64 *)src;
      اكسر;

    حالة FFI_TYPE_STRUCT:
    حالة FFI_TYPE_COMPLEX:
      يادگيري_نقل (dst, src, z);
      اكسر;

    افتراضي:
      abort();
    }

  أرجع FFI_ALIGN (z, 4);
}

/* ffi_prep_args is called once stack space has been allocated
   for the function's arguments.

   The vfp_space parameter is the load area for VFP regs, the return
   value is cif->vfp_used (word bitset of VFP regs used for passing
   arguments). These are only used for the VFP hard-float ABI.
*/
ساكن فراغ
ffi_prep_دليلون_SYSV (ffi_cif *cif, صحيح flags, فراغ *rvalue,
		    فراغ **avalue, حرف *argp)
{
  ffi_type **arg_types = cif->arg_types;
  صحيح i, n;

  إذا (flags == ARM_TYPE_STRUCT)
    {
      *(فراغ **) argp = rvalue;
      argp += 4;
    }

  لكل (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, avalue[i], argp);
    }
}

ساكن فراغ
ffi_prep_دليلون_VFP (ffi_cif *cif, صحيح flags, فراغ *rvalue,
                   فراغ **avalue, حرف *ڍير, حرف *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  صحيح i, n, vi = 0;
  حرف *argp, *regp, *eo_regp;
  حرف ڍير_استعمال_ٿيل = 0;
  حرف پورو_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = ڍير;
  eo_regp = argp = regp + 16;

  /* If the function returns an FFI_TYPE_STRUCT in memory,
     that address is passed in r0 to the function.  */
  إذا (flags == ARM_TYPE_STRUCT)
    {
      *(فراغ **) regp = rvalue;
      regp += 4;
    }

  لكل (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      فراغ *a = avalue[i];
      صحيح is_vfp_type = vfp_type_p (ty);

      /* Allocated in VFP registers. */
      إذا (vi < cif->vfp_nargs && is_vfp_type)
	{
	  حرف *vfp_جڳهه = vfp_space + cif->vfp_دليلون[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_جڳهه);
	  استمر;
	}
      /* Try allocating in core registers. */
      وإلا إذا (!پورو_with_regs && !is_vfp_type)
	{
	  حرف *tregp = ffi_align (ty, regp);
	  size_t سائيز = ty->سائيز;
	  سائيز = (سائيز < 4) ? 4 : سائيز;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  إذا (tregp + سائيز <= eo_regp)
	    {
	      regp = tregp + ffi_put_arg (ty, a, tregp);
	      پورو_with_regs = (regp == argp);
	      // ensure we did not write into the stack area
	      FFI_ASSERT (regp <= argp);
	      استمر;
	    }
	  /* In case there are no arguments in the stack area yet,
	     the argument is passed in the remaining core registers
	     and on the stack.  */
	  وإلا إذا (!ڍير_استعمال_ٿيل)
	    {
	      ڍير_استعمال_ٿيل = 1;
	      پورو_with_regs = 1;
	      argp = tregp + ffi_put_arg (ty, a, tregp);
	      FFI_ASSERT (eo_regp < argp);
	      استمر;
	    }
	}
      /* Base case, arguments are passed on the stack */
      ڍير_استعمال_ٿيل = 1;
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, a, argp);
    }
}

/* Perform machine dependent cif processing */
ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  صحيح flags = 0, cabi = cif->abi;
  size_t بائيٽون = cif->بائيٽون;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  إذا (cabi == FFI_VFP)
    layout_vfp_دليلون (cif);

  /* Set the return type flag */
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_VOID:
      flags = ARM_TYPE_VOID;
      اكسر;

    حالة FFI_TYPE_INT:
    حالة FFI_TYPE_UINT8:
    حالة FFI_TYPE_SINT8:
    حالة FFI_TYPE_UINT16:
    حالة FFI_TYPE_SINT16:
    حالة FFI_TYPE_UINT32:
    حالة FFI_TYPE_SINT32:
    حالة FFI_TYPE_اشارو:
      flags = ARM_TYPE_INT;
      اكسر;

    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
      flags = ARM_TYPE_INT64;
      اكسر;

    حالة FFI_TYPE_FLOAT:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_S : ARM_TYPE_INT);
      اكسر;
    حالة FFI_TYPE_DOUBLE:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_D : ARM_TYPE_INT64);
      اكسر;

    حالة FFI_TYPE_STRUCT:
    حالة FFI_TYPE_COMPLEX:
      إذا (cabi == FFI_VFP)
	{
	  صحيح h = vfp_type_p (cif->rtype);

	  flags = ARM_TYPE_VFP_N;
	  إذا (h == 0x100 + FFI_TYPE_FLOAT)
	    flags = ARM_TYPE_VFP_S;
	  إذا (h == 0x100 + FFI_TYPE_DOUBLE)
	    flags = ARM_TYPE_VFP_D;
	  إذا (h != 0)
	      اكسر;
	}

      /* A Composite Type not larger than 4 bytes is returned in r0.
	 A Composite Type larger than 4 bytes, or whose size cannot
	 be determined statically ... is stored in memory at an
	 address passed [in r0].  */
      إذا (cif->rtype->سائيز <= 4)
	flags = ARM_TYPE_INT;
      وإلا
	{
	  flags = ARM_TYPE_STRUCT;
	  بائيٽون += 4;
	}
      اكسر;

    افتراضي:
      abort();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  بائيٽون = FFI_ALIGN (بائيٽون, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  إذا (بائيٽون < 4*4)
    بائيٽون = 4*4;

  cif->بائيٽون = بائيٽون;
  cif->flags = flags;

  أرجع FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  بنانشان صحيح nfixedargs, بنانشان صحيح ntotalargs)
{
  /* VFP variadic calls actually use the SYSV ABI */
  إذا (cif->abi == FFI_VFP)
    cif->abi = FFI_SYSV;

  أرجع ffi_prep_cif_machdep (cif);
}

/* Prototypes for assembly functions, in sysv.S.  */

بنية call_چوکٺو
{
  فراغ *fp;
  فراغ *lr;
  فراغ *rvalue;
  صحيح flags;
  فراغ *گهيرو;
};

خارجي فراغ ffi_call_SYSV (فراغ *ڍير, بنية call_چوکٺو *,
			   فراغ (*fn) (فراغ)) FFI_HIDDEN;
خارجي فراغ ffi_call_VFP (فراغ *vfp_space, بنية call_چوکٺو *,
			   فراغ (*fn) (فراغ), بنانشان vfp_استعمال_ٿيل) FFI_HIDDEN;

ساكن فراغ
ffi_call_int (ffi_cif * cif, فراغ (*fn) (فراغ), فراغ *rvalue,
	      فراغ **avalue, فراغ *گهيرو)
{
  صحيح flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  size_t بائيٽون, rsize, vfp_سائيز;
  حرف *ڍير, *vfp_space, *new_rvalue;
  بنية call_چوکٺو *چوکٺو;

  rsize = 0;
  إذا (rvalue == عدم)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      إذا (flags == ARM_TYPE_STRUCT)
	rsize = rtype->سائيز;
      وإلا
	flags = ARM_TYPE_VOID;
    }
  وإلا إذا (flags == ARM_TYPE_VFP_N)
    {
      /* Largest case is double x 4. */
      rsize = 32;
    }
  وإلا إذا (flags == ARM_TYPE_INT && rtype->type == FFI_TYPE_STRUCT)
    rsize = 4;

  /* Largest case.  */
  vfp_سائيز = (cif->abi == FFI_VFP && cif->vfp_استعمال_ٿيل ? 8*8: 0);

  بائيٽون = cif->بائيٽون;
  ڍير = alloca (vfp_سائيز + بائيٽون + حجم(بنية call_چوکٺو) + rsize);

  vfp_space = عدم;
  إذا (vfp_سائيز)
    {
      vfp_space = ڍير;
      ڍير += vfp_سائيز;
    }

  چوکٺو = (بنية call_چوکٺو *)(ڍير + بائيٽون);

  new_rvalue = rvalue;
  إذا (rsize)
    new_rvalue = (فراغ *)(چوکٺو + 1);

  چوکٺو->rvalue = new_rvalue;
  چوکٺو->flags = flags;
  چوکٺو->گهيرو = گهيرو;

  إذا (vfp_space)
    {
      ffi_prep_دليلون_VFP (cif, flags, new_rvalue, avalue, ڍير, vfp_space);
      ffi_call_VFP (vfp_space, چوکٺو, fn, cif->vfp_استعمال_ٿيل);
    }
  وإلا
    {
      ffi_prep_دليلون_SYSV (cif, flags, new_rvalue, avalue, ڍير);
      ffi_call_SYSV (ڍير, چوکٺو, fn);
    }

  إذا (rvalue && rvalue != new_rvalue)
    يادگيري_نقل (rvalue, new_rvalue, rtype->سائيز);
}

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, عدم);
}

#جيڪڏهن_وصف_ٿيل FFI_GO_CLOSURES
فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue,
	     فراغ **avalue, فراغ *گهيرو)
{
  ffi_call_int (cif, fn, rvalue, avalue, گهيرو);
}
#جيڪڏهن_پورو

ساكن فراغ *
ffi_prep_incoming_دليلون_SYSV (ffi_cif *cif, فراغ *rvalue,
			     حرف *argp, فراغ **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  صحيح i, n;

  إذا (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(فراغ **) argp;
      argp += 4;
    }
  وإلا
    {
      إذا (cif->rtype->سائيز && cif->rtype->سائيز < 4)
        *(uint32_t *) rvalue = 0;
    }

  لكل (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->سائيز;

      argp = ffi_align (ty, argp);
#جيڪڏهن_وصف_ٿيل __ARMEB__
      إذا (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (فراغ *) (argp + 4 - z);
      وإلا
#جيڪڏهن_پورو
      avalue[i] = (فراغ *) argp;
      argp += z;
    }

  أرجع rvalue;
}

ساكن فراغ *
ffi_prep_incoming_دليلون_VFP (ffi_cif *cif, فراغ *rvalue, حرف *ڍير,
			    حرف *vfp_space, فراغ **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  صحيح i, n, vi = 0;
  حرف *argp, *regp, *eo_regp;
  حرف پورو_with_regs = 0;
  حرف ڍير_استعمال_ٿيل = 0;

  regp = ڍير;
  eo_regp = argp = regp + 16;

  إذا (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(فراغ **) regp;
      regp += 4;
    }

  لكل (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      صحيح is_vfp_type = vfp_type_p (ty);
      size_t z = ty->سائيز;

      إذا (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_دليلون[vi++] * 4;
	  استمر;
	}
      وإلا إذا (!پورو_with_regs && !is_vfp_type)
	{
	  حرف *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  إذا (tregp + z <= eo_regp || !ڍير_استعمال_ٿيل)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#جيڪڏهن_وصف_ٿيل __ARMEB__
	      إذا (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (فراغ *) (tregp + 4 - z);
	      وإلا
#جيڪڏهن_پورو
	      avalue[i] = (فراغ *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      إذا (regp > eo_regp)
		{
		  FFI_ASSERT (!ڍير_استعمال_ٿيل);
		  argp = regp;
		}
	      إذا (regp >= eo_regp)
		{
		  پورو_with_regs = 1;
		  ڍير_استعمال_ٿيل = 1;
		}
	      استمر;
	    }
	}

      ڍير_استعمال_ٿيل = 1;
      argp = ffi_align (ty, argp);
#جيڪڏهن_وصف_ٿيل __ARMEB__
      إذا (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (فراغ *) (argp + 4 - z);
      وإلا
#جيڪڏهن_پورو
      avalue[i] = (فراغ *) argp;
      argp += z;
    }

  أرجع rvalue;
}

#إذا FFI_CLOSURES

بنية گهيرو_چوکٺو
{
  حرف vfp_space[8*8] __attribute__((aligned(8)));
  حرف نتيجو[8*4];
  حرف argp[];
};

صحيح FFI_HIDDEN
ffi_گهيرو_inner_SYSV (ffi_cif *cif,
		        فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		        فراغ *user_معلومات,
		        بنية گهيرو_چوکٺو *چوکٺو)
{
  فراغ **avalue = (فراغ **) alloca (cif->nargs * حجم (فراغ *));
  فراغ *rvalue = ffi_prep_incoming_دليلون_SYSV (cif, چوکٺو->نتيجو,
					      چوکٺو->argp, avalue);
  fun (cif, rvalue, avalue, user_معلومات);
  أرجع cif->flags;
}

صحيح FFI_HIDDEN
ffi_گهيرو_inner_VFP (ffi_cif *cif,
		       فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		       فراغ *user_معلومات,
		       بنية گهيرو_چوکٺو *چوکٺو)
{
  فراغ **avalue = (فراغ **) alloca (cif->nargs * حجم (فراغ *));
  فراغ *rvalue = ffi_prep_incoming_دليلون_VFP (cif, چوکٺو->نتيجو, چوکٺو->argp,
					     چوکٺو->vfp_space, avalue);
  fun (cif, rvalue, avalue, user_معلومات);
  أرجع cif->flags;
}

فراغ ffi_گهيرو_SYSV (فراغ) FFI_HIDDEN;
فراغ ffi_گهيرو_VFP (فراغ) FFI_HIDDEN;
#إذا defined(FFI_EXEC_STATIC_TRAMP)
فراغ ffi_گهيرو_SYSV_alt (فراغ) FFI_HIDDEN;
فراغ ffi_گهيرو_VFP_alt (فراغ) FFI_HIDDEN;
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل FFI_GO_CLOSURES
فراغ ffi_go_گهيرو_SYSV (فراغ) FFI_HIDDEN;
فراغ ffi_go_گهيرو_VFP (فراغ) FFI_HIDDEN;
#جيڪڏهن_پورو

/* the cif must already be prep'ed */

#إذا defined(__FreeBSD__) && defined(__arm__)
#وصف_ڪريو __clear_cache(شروعات, پڇاڙي) افعل { \
		بنية arm_sync_icache_دليلون ua; 		\
								\
		ua.addr = (uintptr_t)(شروعات);			\
		ua.len = (حرف *)(پڇاڙي) - (حرف *)شروعات;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} طالما (0);
#جيڪڏهن_پورو

ffi_حالت
ffi_prep_گهيرو_loc (ffi_گهيرو * گهيرو,
		      ffi_cif * cif,
		      فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		      فراغ *user_معلومات, فراغ *codeloc)
{
  فراغ (*گهيرو_func) (فراغ) = ffi_گهيرو_SYSV;

  إذا (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      إذا (cif->vfp_استعمال_ٿيل)
	گهيرو_func = ffi_گهيرو_VFP;
    }
  وإلا إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;

#إذا FFI_EXEC_TRAMPOLINE_جدول
  فراغ **config = (فراغ **)((uint8_t *)codeloc - صفحو_MAX_سائيز);
  config[0] = گهيرو;
  config[1] = گهيرو_func;
#وإلا

# إذا defined(FFI_EXEC_STATIC_TRAMP)
  إذا (ffi_tramp_is_present(گهيرو))
    {
      /* Initialize the static trampoline's parameters. */
      إذا (گهيرو_func == ffi_گهيرو_SYSV)
        گهيرو_func = ffi_گهيرو_SYSV_alt;
      وإلا
        گهيرو_func = ffi_گهيرو_VFP_alt;
      ffi_tramp_سيٽ_parms (گهيرو->ftramp, گهيرو_func, گهيرو);
      اذهب ٻاهريون;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  يادگيري_نقل(گهيرو->tramp, ffi_arm_trampoline, 8);
# وإلا
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  يادگيري_نقل(گهيرو->tramp, (فراغ*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_گهيرو_وٿي);
# endif

# إذا defined(__QNX__)
  msync (گهيرو->tramp, 8, MS_INVALIDATE_ICACHE);	/* clear data map */
  msync (codeloc, 8, MS_INVALIDATE_ICACHE);		/* clear insn map */
# elif defined(_WIN32)
  FlushInstructionCache(GetCurrentProcess(), گهيرو->tramp, FFI_TRAMPOLINE_سائيز);
# وإلا
  __clear_cache(گهيرو->tramp, گهيرو->tramp + 8);	/* clear data map */
  __clear_cache(codeloc, codeloc + 8);			/* clear insn map */
# endif
# ifdef _WIN32
  *(فراغ(**)(فراغ))(گهيرو->tramp + FFI_TRAMPOLINE_گهيرو_FUNCTION) = گهيرو_func;
# وإلا
  *(فراغ (**)(فراغ))(گهيرو->tramp + 8) = گهيرو_func;
# endif
# إذا defined(FFI_EXEC_STATIC_TRAMP)
ٻاهريون:
# endif
#جيڪڏهن_پورو

  گهيرو->cif = cif;
  گهيرو->fun = fun;
  گهيرو->user_معلومات = user_معلومات;

  أرجع FFI_OK;
}

#جيڪڏهن_وصف_ٿيل FFI_GO_CLOSURES
ffi_حالت
ffi_prep_go_گهيرو (ffi_go_گهيرو *گهيرو, ffi_cif *cif,
		     فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *))
{
  فراغ (*گهيرو_func) (فراغ) = ffi_go_گهيرو_SYSV;

  إذا (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      إذا (cif->vfp_استعمال_ٿيل)
	گهيرو_func = ffi_go_گهيرو_VFP;
    }
  وإلا إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;

  گهيرو->tramp = گهيرو_func;
  گهيرو->cif = cif;
  گهيرو->fun = fun;

  أرجع FFI_OK;
}
#جيڪڏهن_پورو

#جيڪڏهن_پورو /* FFI_CLOSURES */

/* Below are routines for VFP hard-float support. */

/* A subroutine of vfp_type_p.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

ساكن صحيح
is_hfa0 (ثابت ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  صحيح i, ret = -1;

  إذا (elements != عدم)
    لكل (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        إذا (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            إذا (ret < 0)
              استمر;
          }
        اكسر;
      }

  أرجع ret;
}

/* A subroutine of vfp_type_p.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

ساكن صحيح
is_hfa1 (ثابت ffi_type *ty, صحيح candidate)
{
  ffi_type **elements = ty->elements;
  صحيح i;

  إذا (elements != عدم)
    لكل (i = 0; elements[i]; ++i)
      {
        صحيح t = elements[i]->type;
        إذا (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
          {
            إذا (!is_hfa1 (elements[i], candidate))
              أرجع 0;
          }
        وإلا إذا (t != candidate)
          أرجع 0;
      }

  أرجع 1;
}

/* Determine if TY is an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is a floating point scalar.

   Returns non-zero iff TY is an HFA.  The result is an encoded value where
   bits 0-7 contain the type code, and bits 8-10 contain the element count.  */

ساكن صحيح
vfp_type_p (ثابت ffi_type *ty)
{
  ffi_type **elements;
  صحيح candidate, i;
  size_t سائيز, ele_ڳڻپ;

  /* Quickest tests first.  */
  candidate = ty->type;
  بدل (ty->type)
    {
    افتراضي:
      أرجع 0;
    حالة FFI_TYPE_FLOAT:
    حالة FFI_TYPE_DOUBLE:
      ele_ڳڻپ = 1;
      اذهب پورو;
    حالة FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      إذا (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	أرجع 0;
      ele_ڳڻپ = 2;
      اذهب پورو;
    حالة FFI_TYPE_STRUCT:
      اكسر;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  سائيز = ty->سائيز;
  إذا (سائيز < 4 || سائيز > 32)
    أرجع 0;

  /* Find the type of the first non-structure member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  إذا (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_COMPLEX)
    {
      لكل (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          إذا (candidate >= 0)
            اكسر;
        }
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  بدل (candidate)
    {
    حالة FFI_TYPE_FLOAT:
      ele_ڳڻپ = سائيز / حجم(عائم);
      إذا (سائيز != ele_ڳڻپ * حجم(عائم))
        أرجع 0;
      اكسر;
    حالة FFI_TYPE_DOUBLE:
      ele_ڳڻپ = سائيز / حجم(مزدوج);
      إذا (سائيز != ele_ڳڻپ * حجم(مزدوج))
        أرجع 0;
      اكسر;
    افتراضي:
      أرجع 0;
    }
  إذا (ele_ڳڻپ > 4)
    أرجع 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  لكل (i = 0; elements[i]; ++i)
    {
      صحيح t = elements[i]->type;
      إذا (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          إذا (!is_hfa1 (elements[i], candidate))
            أرجع 0;
        }
      وإلا إذا (t != candidate)
        أرجع 0;
    }

  /* All tests succeeded.  Encode the result.  */
 پورو:
  أرجع (ele_ڳڻپ << 8) | candidate;
}

ساكن صحيح
رکو_vfp_arg (ffi_cif *cif, صحيح h)
{
  بنانشان ننڍو reg = cif->vfp_reg_free;
  صحيح align = 1, nregs = h >> 8;

  إذا ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  إذا ((reg & 1) && align == 2)
    reg++;

  طالما (reg + nregs <= 16)
    {
      صحيح s, new_استعمال_ٿيل = 0;
      لكل (s = reg; s < reg + nregs; s++)
	{
	  new_استعمال_ٿيل |= (1 << s);
	  إذا (cif->vfp_استعمال_ٿيل & (1 << s))
	    {
	      reg += align;
	      اذهب ايندڙ_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_استعمال_ٿيل |= new_استعمال_ٿيل;
      cif->vfp_دليلون[cif->vfp_nargs++] = (نشانوارو حرف)reg;

      /* Update vfp_reg_free. */
      إذا (cif->vfp_استعمال_ٿيل & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  طالما (cif->vfp_استعمال_ٿيل & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      أرجع 0;
    ايندڙ_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_استعمال_ٿيل = 0xFFFF;
  أرجع 1;
}

ساكن فراغ
layout_vfp_دليلون (ffi_cif * cif)
{
  بنانشان صحيح i;
  /* Init VFP fields */
  cif->vfp_استعمال_ٿيل = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  memset (cif->vfp_دليلون, -1, 16);	/* Init to -1. */

  لكل (i = 0; i < cif->nargs; i++)
    {
      صحيح h = vfp_type_p (cif->arg_types[i]);
      إذا (h && رکو_vfp_arg (cif, h) == 1)
	اكسر;
    }
}

#إذا defined(FFI_EXEC_STATIC_TRAMP)
فراغ *
ffi_tramp_arch (size_t *tramp_سائيز, size_t *map_سائيز)
{
  خارجي فراغ *trampoline_ڪوڊ_جدول;

  *tramp_سائيز = ARM_TRAMP_سائيز;
  *map_سائيز = ARM_TRAMP_MAP_سائيز;
  أرجع &trampoline_ڪوڊ_جدول;
}
#جيڪڏهن_پورو

#جيڪڏهن_پورو /* __arm__ or _M_ARM */
