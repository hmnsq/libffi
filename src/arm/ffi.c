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
#تعریف_کرو WIN32_LEAN_AND_اوسط
#ضمن <windows.h>
#جے_انت

#إذا FFI_EXEC_TRAMPOLINE_جدول

#جے_تعریف_شدہ __MACH__
#ضمن <mach/machine/vm_param.h>
#جے_انت

#وإلا
#جے_بنا_تعریف _WIN32
خارجي بنانشانی صحيح ffi_arm_trampoline[2] FFI_HIDDEN;
#وإلا
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
خارجي بنانشانی حرف ffi_arm_trampoline[12] FFI_HIDDEN;
#جے_انت
#جے_انت

#إذا defined(__FreeBSD__) && defined(__arm__)
#ضمن <sys/types.h>
#ضمن <machine/sysarch.h>
#جے_انت

#إذا defined(__QNX__)
#ضمن <sys/mman.h>
#جے_انت

/* Forward declares. */
ساكن صحيح vfp_type_p (ثابت ffi_type *);
ساكن فراغ layout_vfp_دلیلاں (ffi_cif *);

ساكن فراغ *
ffi_align (ffi_type *ty, فراغ *p)
{
  /* Align if necessary */
  size_t alignment;
#جے_تعریف_شدہ _WIN32_WCE
  alignment = 4;
#وإلا
  alignment = ty->alignment;
  إذا (alignment < 4)
    alignment = 4;
#جے_انت
  أرجع (فراغ *) FFI_ALIGN (p, alignment);
}

ساكن size_t
ffi_put_arg (ffi_type *ty, فراغ *src, فراغ *dst)
{
  size_t z = ty->ناپ;

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
    حالة FFI_TYPE_اشارہ_گر:
#جے_بنا_تعریف _WIN32
    حالة FFI_TYPE_FLOAT:
#جے_انت
      *(UINT32 *)dst = *(UINT32 *)src;
      اكسر;

#جے_تعریف_شدہ _WIN32
    // casting a float* to a UINT32* doesn't work on Windows
    حالة FFI_TYPE_FLOAT:
        *(uintptr_t *)dst = 0;
        *(عائم *)dst = *(عائم *)src;
        اكسر;
#جے_انت

    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_DOUBLE:
      *(UINT64 *)dst = *(UINT64 *)src;
      اكسر;

    حالة FFI_TYPE_STRUCT:
    حالة FFI_TYPE_COMPLEX:
      حافظہ_نقل (dst, src, z);
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
ffi_prep_دلیلاں_SYSV (ffi_cif *cif, صحيح flags, فراغ *rvalue,
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
ffi_prep_دلیلاں_VFP (ffi_cif *cif, صحيح flags, فراغ *rvalue,
                   فراغ **avalue, حرف *ڈھیر, حرف *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  صحيح i, n, vi = 0;
  حرف *argp, *regp, *eo_regp;
  حرف ڈھیر_ورتیا = 0;
  حرف پورا_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = ڈھیر;
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
	  حرف *vfp_تھاں = vfp_space + cif->vfp_دلیلاں[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_تھاں);
	  استمر;
	}
      /* Try allocating in core registers. */
      وإلا إذا (!پورا_with_regs && !is_vfp_type)
	{
	  حرف *tregp = ffi_align (ty, regp);
	  size_t ناپ = ty->ناپ;
	  ناپ = (ناپ < 4) ? 4 : ناپ;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  إذا (tregp + ناپ <= eo_regp)
	    {
	      regp = tregp + ffi_put_arg (ty, a, tregp);
	      پورا_with_regs = (regp == argp);
	      // ensure we did not write into the stack area
	      FFI_ASSERT (regp <= argp);
	      استمر;
	    }
	  /* In case there are no arguments in the stack area yet,
	     the argument is passed in the remaining core registers
	     and on the stack.  */
	  وإلا إذا (!ڈھیر_ورتیا)
	    {
	      ڈھیر_ورتیا = 1;
	      پورا_with_regs = 1;
	      argp = tregp + ffi_put_arg (ty, a, tregp);
	      FFI_ASSERT (eo_regp < argp);
	      استمر;
	    }
	}
      /* Base case, arguments are passed on the stack */
      ڈھیر_ورتیا = 1;
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, a, argp);
    }
}

/* Perform machine dependent cif processing */
ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  صحيح flags = 0, cabi = cif->abi;
  size_t بائٹاں = cif->بائٹاں;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  إذا (cabi == FFI_VFP)
    layout_vfp_دلیلاں (cif);

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
    حالة FFI_TYPE_اشارہ_گر:
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
      إذا (cif->rtype->ناپ <= 4)
	flags = ARM_TYPE_INT;
      وإلا
	{
	  flags = ARM_TYPE_STRUCT;
	  بائٹاں += 4;
	}
      اكسر;

    افتراضي:
      abort();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  بائٹاں = FFI_ALIGN (بائٹاں, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  إذا (بائٹاں < 4*4)
    بائٹاں = 4*4;

  cif->بائٹاں = بائٹاں;
  cif->flags = flags;

  أرجع FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  بنانشانی صحيح nfixedargs, بنانشانی صحيح ntotalargs)
{
  /* VFP variadic calls actually use the SYSV ABI */
  إذا (cif->abi == FFI_VFP)
    cif->abi = FFI_SYSV;

  أرجع ffi_prep_cif_machdep (cif);
}

/* Prototypes for assembly functions, in sysv.S.  */

بنية call_چوکھٹا
{
  فراغ *fp;
  فراغ *lr;
  فراغ *rvalue;
  صحيح flags;
  فراغ *گھیرا;
};

خارجي فراغ ffi_call_SYSV (فراغ *ڈھیر, بنية call_چوکھٹا *,
			   فراغ (*fn) (فراغ)) FFI_HIDDEN;
خارجي فراغ ffi_call_VFP (فراغ *vfp_space, بنية call_چوکھٹا *,
			   فراغ (*fn) (فراغ), بنانشانی vfp_ورتیا) FFI_HIDDEN;

ساكن فراغ
ffi_call_int (ffi_cif * cif, فراغ (*fn) (فراغ), فراغ *rvalue,
	      فراغ **avalue, فراغ *گھیرا)
{
  صحيح flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  size_t بائٹاں, rsize, vfp_ناپ;
  حرف *ڈھیر, *vfp_space, *new_rvalue;
  بنية call_چوکھٹا *چوکھٹا;

  rsize = 0;
  إذا (rvalue == عدم)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      إذا (flags == ARM_TYPE_STRUCT)
	rsize = rtype->ناپ;
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
  vfp_ناپ = (cif->abi == FFI_VFP && cif->vfp_ورتیا ? 8*8: 0);

  بائٹاں = cif->بائٹاں;
  ڈھیر = alloca (vfp_ناپ + بائٹاں + حجم(بنية call_چوکھٹا) + rsize);

  vfp_space = عدم;
  إذا (vfp_ناپ)
    {
      vfp_space = ڈھیر;
      ڈھیر += vfp_ناپ;
    }

  چوکھٹا = (بنية call_چوکھٹا *)(ڈھیر + بائٹاں);

  new_rvalue = rvalue;
  إذا (rsize)
    new_rvalue = (فراغ *)(چوکھٹا + 1);

  چوکھٹا->rvalue = new_rvalue;
  چوکھٹا->flags = flags;
  چوکھٹا->گھیرا = گھیرا;

  إذا (vfp_space)
    {
      ffi_prep_دلیلاں_VFP (cif, flags, new_rvalue, avalue, ڈھیر, vfp_space);
      ffi_call_VFP (vfp_space, چوکھٹا, fn, cif->vfp_ورتیا);
    }
  وإلا
    {
      ffi_prep_دلیلاں_SYSV (cif, flags, new_rvalue, avalue, ڈھیر);
      ffi_call_SYSV (ڈھیر, چوکھٹا, fn);
    }

  إذا (rvalue && rvalue != new_rvalue)
    حافظہ_نقل (rvalue, new_rvalue, rtype->ناپ);
}

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, عدم);
}

#جے_تعریف_شدہ FFI_GO_CLOSURES
فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue,
	     فراغ **avalue, فراغ *گھیرا)
{
  ffi_call_int (cif, fn, rvalue, avalue, گھیرا);
}
#جے_انت

ساكن فراغ *
ffi_prep_incoming_دلیلاں_SYSV (ffi_cif *cif, فراغ *rvalue,
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
      إذا (cif->rtype->ناپ && cif->rtype->ناپ < 4)
        *(uint32_t *) rvalue = 0;
    }

  لكل (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->ناپ;

      argp = ffi_align (ty, argp);
#جے_تعریف_شدہ __ARMEB__
      إذا (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (فراغ *) (argp + 4 - z);
      وإلا
#جے_انت
      avalue[i] = (فراغ *) argp;
      argp += z;
    }

  أرجع rvalue;
}

ساكن فراغ *
ffi_prep_incoming_دلیلاں_VFP (ffi_cif *cif, فراغ *rvalue, حرف *ڈھیر,
			    حرف *vfp_space, فراغ **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  صحيح i, n, vi = 0;
  حرف *argp, *regp, *eo_regp;
  حرف پورا_with_regs = 0;
  حرف ڈھیر_ورتیا = 0;

  regp = ڈھیر;
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
      size_t z = ty->ناپ;

      إذا (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_دلیلاں[vi++] * 4;
	  استمر;
	}
      وإلا إذا (!پورا_with_regs && !is_vfp_type)
	{
	  حرف *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  إذا (tregp + z <= eo_regp || !ڈھیر_ورتیا)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#جے_تعریف_شدہ __ARMEB__
	      إذا (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (فراغ *) (tregp + 4 - z);
	      وإلا
#جے_انت
	      avalue[i] = (فراغ *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      إذا (regp > eo_regp)
		{
		  FFI_ASSERT (!ڈھیر_ورتیا);
		  argp = regp;
		}
	      إذا (regp >= eo_regp)
		{
		  پورا_with_regs = 1;
		  ڈھیر_ورتیا = 1;
		}
	      استمر;
	    }
	}

      ڈھیر_ورتیا = 1;
      argp = ffi_align (ty, argp);
#جے_تعریف_شدہ __ARMEB__
      إذا (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (فراغ *) (argp + 4 - z);
      وإلا
#جے_انت
      avalue[i] = (فراغ *) argp;
      argp += z;
    }

  أرجع rvalue;
}

#إذا FFI_CLOSURES

بنية گھیرا_چوکھٹا
{
  حرف vfp_space[8*8] __attribute__((aligned(8)));
  حرف نتیجہ[8*4];
  حرف argp[];
};

صحيح FFI_HIDDEN
ffi_گھیرا_inner_SYSV (ffi_cif *cif,
		        فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		        فراغ *user_انکڑے,
		        بنية گھیرا_چوکھٹا *چوکھٹا)
{
  فراغ **avalue = (فراغ **) alloca (cif->nargs * حجم (فراغ *));
  فراغ *rvalue = ffi_prep_incoming_دلیلاں_SYSV (cif, چوکھٹا->نتیجہ,
					      چوکھٹا->argp, avalue);
  fun (cif, rvalue, avalue, user_انکڑے);
  أرجع cif->flags;
}

صحيح FFI_HIDDEN
ffi_گھیرا_inner_VFP (ffi_cif *cif,
		       فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		       فراغ *user_انکڑے,
		       بنية گھیرا_چوکھٹا *چوکھٹا)
{
  فراغ **avalue = (فراغ **) alloca (cif->nargs * حجم (فراغ *));
  فراغ *rvalue = ffi_prep_incoming_دلیلاں_VFP (cif, چوکھٹا->نتیجہ, چوکھٹا->argp,
					     چوکھٹا->vfp_space, avalue);
  fun (cif, rvalue, avalue, user_انکڑے);
  أرجع cif->flags;
}

فراغ ffi_گھیرا_SYSV (فراغ) FFI_HIDDEN;
فراغ ffi_گھیرا_VFP (فراغ) FFI_HIDDEN;
#إذا defined(FFI_EXEC_STATIC_TRAMP)
فراغ ffi_گھیرا_SYSV_alt (فراغ) FFI_HIDDEN;
فراغ ffi_گھیرا_VFP_alt (فراغ) FFI_HIDDEN;
#جے_انت

#جے_تعریف_شدہ FFI_GO_CLOSURES
فراغ ffi_go_گھیرا_SYSV (فراغ) FFI_HIDDEN;
فراغ ffi_go_گھیرا_VFP (فراغ) FFI_HIDDEN;
#جے_انت

/* the cif must already be prep'ed */

#إذا defined(__FreeBSD__) && defined(__arm__)
#تعریف_کرو __clear_cache(شروع, سرا) افعل { \
		بنية arm_sync_icache_دلیلاں ua; 		\
								\
		ua.addr = (uintptr_t)(شروع);			\
		ua.len = (حرف *)(سرا) - (حرف *)شروع;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} طالما (0);
#جے_انت

ffi_حالت
ffi_prep_گھیرا_loc (ffi_گھیرا * گھیرا,
		      ffi_cif * cif,
		      فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		      فراغ *user_انکڑے, فراغ *codeloc)
{
  فراغ (*گھیرا_func) (فراغ) = ffi_گھیرا_SYSV;

  إذا (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      إذا (cif->vfp_ورتیا)
	گھیرا_func = ffi_گھیرا_VFP;
    }
  وإلا إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;

#إذا FFI_EXEC_TRAMPOLINE_جدول
  فراغ **config = (فراغ **)((uint8_t *)codeloc - ورقہ_MAX_ناپ);
  config[0] = گھیرا;
  config[1] = گھیرا_func;
#وإلا

# إذا defined(FFI_EXEC_STATIC_TRAMP)
  إذا (ffi_tramp_is_present(گھیرا))
    {
      /* Initialize the static trampoline's parameters. */
      إذا (گھیرا_func == ffi_گھیرا_SYSV)
        گھیرا_func = ffi_گھیرا_SYSV_alt;
      وإلا
        گھیرا_func = ffi_گھیرا_VFP_alt;
      ffi_tramp_سیٹ_parms (گھیرا->ftramp, گھیرا_func, گھیرا);
      اذهب باہرلا;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  حافظہ_نقل(گھیرا->tramp, ffi_arm_trampoline, 8);
# وإلا
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  حافظہ_نقل(گھیرا->tramp, (فراغ*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_گھیرا_وتھ);
# endif

# إذا defined(__QNX__)
  msync (گھیرا->tramp, 8, MS_INVALIDATE_ICACHE);	/* clear data map */
  msync (codeloc, 8, MS_INVALIDATE_ICACHE);		/* clear insn map */
# elif defined(_WIN32)
  FlushInstructionCache(GetCurrentProcess(), گھیرا->tramp, FFI_TRAMPOLINE_ناپ);
# وإلا
  __clear_cache(گھیرا->tramp, گھیرا->tramp + 8);	/* clear data map */
  __clear_cache(codeloc, codeloc + 8);			/* clear insn map */
# endif
# ifdef _WIN32
  *(فراغ(**)(فراغ))(گھیرا->tramp + FFI_TRAMPOLINE_گھیرا_FUNCTION) = گھیرا_func;
# وإلا
  *(فراغ (**)(فراغ))(گھیرا->tramp + 8) = گھیرا_func;
# endif
# إذا defined(FFI_EXEC_STATIC_TRAMP)
باہرلا:
# endif
#جے_انت

  گھیرا->cif = cif;
  گھیرا->fun = fun;
  گھیرا->user_انکڑے = user_انکڑے;

  أرجع FFI_OK;
}

#جے_تعریف_شدہ FFI_GO_CLOSURES
ffi_حالت
ffi_prep_go_گھیرا (ffi_go_گھیرا *گھیرا, ffi_cif *cif,
		     فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *))
{
  فراغ (*گھیرا_func) (فراغ) = ffi_go_گھیرا_SYSV;

  إذا (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      إذا (cif->vfp_ورتیا)
	گھیرا_func = ffi_go_گھیرا_VFP;
    }
  وإلا إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;

  گھیرا->tramp = گھیرا_func;
  گھیرا->cif = cif;
  گھیرا->fun = fun;

  أرجع FFI_OK;
}
#جے_انت

#جے_انت /* FFI_CLOSURES */

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
  size_t ناپ, ele_گنتی;

  /* Quickest tests first.  */
  candidate = ty->type;
  بدل (ty->type)
    {
    افتراضي:
      أرجع 0;
    حالة FFI_TYPE_FLOAT:
    حالة FFI_TYPE_DOUBLE:
      ele_گنتی = 1;
      اذهب پورا;
    حالة FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      إذا (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	أرجع 0;
      ele_گنتی = 2;
      اذهب پورا;
    حالة FFI_TYPE_STRUCT:
      اكسر;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  ناپ = ty->ناپ;
  إذا (ناپ < 4 || ناپ > 32)
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
      ele_گنتی = ناپ / حجم(عائم);
      إذا (ناپ != ele_گنتی * حجم(عائم))
        أرجع 0;
      اكسر;
    حالة FFI_TYPE_DOUBLE:
      ele_گنتی = ناپ / حجم(مزدوج);
      إذا (ناپ != ele_گنتی * حجم(مزدوج))
        أرجع 0;
      اكسر;
    افتراضي:
      أرجع 0;
    }
  إذا (ele_گنتی > 4)
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
 پورا:
  أرجع (ele_گنتی << 8) | candidate;
}

ساكن صحيح
رکھو_vfp_arg (ffi_cif *cif, صحيح h)
{
  بنانشانی چھوٹا reg = cif->vfp_reg_free;
  صحيح align = 1, nregs = h >> 8;

  إذا ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  إذا ((reg & 1) && align == 2)
    reg++;

  طالما (reg + nregs <= 16)
    {
      صحيح s, new_ورتیا = 0;
      لكل (s = reg; s < reg + nregs; s++)
	{
	  new_ورتیا |= (1 << s);
	  إذا (cif->vfp_ورتیا & (1 << s))
	    {
	      reg += align;
	      اذهب اگلا_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_ورتیا |= new_ورتیا;
      cif->vfp_دلیلاں[cif->vfp_nargs++] = (نشانیوالا حرف)reg;

      /* Update vfp_reg_free. */
      إذا (cif->vfp_ورتیا & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  طالما (cif->vfp_ورتیا & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      أرجع 0;
    اگلا_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_ورتیا = 0xFFFF;
  أرجع 1;
}

ساكن فراغ
layout_vfp_دلیلاں (ffi_cif * cif)
{
  بنانشانی صحيح i;
  /* Init VFP fields */
  cif->vfp_ورتیا = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  memset (cif->vfp_دلیلاں, -1, 16);	/* Init to -1. */

  لكل (i = 0; i < cif->nargs; i++)
    {
      صحيح h = vfp_type_p (cif->arg_types[i]);
      إذا (h && رکھو_vfp_arg (cif, h) == 1)
	اكسر;
    }
}

#إذا defined(FFI_EXEC_STATIC_TRAMP)
فراغ *
ffi_tramp_arch (size_t *tramp_ناپ, size_t *map_ناپ)
{
  خارجي فراغ *trampoline_کوڈ_جدول;

  *tramp_ناپ = ARM_TRAMP_ناپ;
  *map_ناپ = ARM_TRAMP_MAP_ناپ;
  أرجع &trampoline_کوڈ_جدول;
}
#جے_انت

#جے_انت /* __arm__ or _M_ARM */
