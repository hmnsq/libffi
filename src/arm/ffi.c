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
#تعریف_کړه WIN32_LEAN_AND_منځنۍ
#ضمن <windows.h>
#د_که_پای

#إذا FFI_EXEC_TRAMPOLINE_جدول

#که_تعریف_شوی __MACH__
#ضمن <mach/machine/vm_param.h>
#د_که_پای

#وإلا
#که_نه_تعریف_شوی _WIN32
خارجي بینښه صحيح ffi_arm_trampoline[2] FFI_HIDDEN;
#وإلا
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
خارجي بینښه حرف ffi_arm_trampoline[12] FFI_HIDDEN;
#د_که_پای
#د_که_پای

#إذا defined(__FreeBSD__) && defined(__arm__)
#ضمن <sys/types.h>
#ضمن <machine/sysarch.h>
#د_که_پای

#إذا defined(__QNX__)
#ضمن <sys/mman.h>
#د_که_پای

/* Forward declares. */
ساكن صحيح vfp_type_p (ثابت ffi_type *);
ساكن فراغ layout_vfp_دلیلونه (ffi_cif *);

ساكن فراغ *
ffi_align (ffi_type *ty, فراغ *p)
{
  /* Align if necessary */
  size_t alignment;
#که_تعریف_شوی _WIN32_WCE
  alignment = 4;
#وإلا
  alignment = ty->alignment;
  إذا (alignment < 4)
    alignment = 4;
#د_که_پای
  أرجع (فراغ *) FFI_ALIGN (p, alignment);
}

ساكن size_t
ffi_put_arg (ffi_type *ty, فراغ *src, فراغ *dst)
{
  size_t z = ty->اندازه;

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
    حالة FFI_TYPE_اشاره_ګر:
#که_نه_تعریف_شوی _WIN32
    حالة FFI_TYPE_FLOAT:
#د_که_پای
      *(UINT32 *)dst = *(UINT32 *)src;
      اكسر;

#که_تعریف_شوی _WIN32
    // casting a float* to a UINT32* doesn't work on Windows
    حالة FFI_TYPE_FLOAT:
        *(uintptr_t *)dst = 0;
        *(عائم *)dst = *(عائم *)src;
        اكسر;
#د_که_پای

    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_DOUBLE:
      *(UINT64 *)dst = *(UINT64 *)src;
      اكسر;

    حالة FFI_TYPE_STRUCT:
    حالة FFI_TYPE_COMPLEX:
      حافظه_نقل (dst, src, z);
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
ffi_prep_دلیلونه_SYSV (ffi_cif *cif, صحيح flags, فراغ *rvalue,
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
ffi_prep_دلیلونه_VFP (ffi_cif *cif, صحيح flags, فراغ *rvalue,
                   فراغ **avalue, حرف *ډېری, حرف *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  صحيح i, n, vi = 0;
  حرف *argp, *regp, *eo_regp;
  حرف ډېری_کارول_شوی = 0;
  حرف بشپړ_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = ډېری;
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
	  حرف *vfp_خونه = vfp_space + cif->vfp_دلیلونه[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_خونه);
	  استمر;
	}
      /* Try allocating in core registers. */
      وإلا إذا (!بشپړ_with_regs && !is_vfp_type)
	{
	  حرف *tregp = ffi_align (ty, regp);
	  size_t اندازه = ty->اندازه;
	  اندازه = (اندازه < 4) ? 4 : اندازه;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  إذا (tregp + اندازه <= eo_regp)
	    {
	      regp = tregp + ffi_put_arg (ty, a, tregp);
	      بشپړ_with_regs = (regp == argp);
	      // ensure we did not write into the stack area
	      FFI_ASSERT (regp <= argp);
	      استمر;
	    }
	  /* In case there are no arguments in the stack area yet,
	     the argument is passed in the remaining core registers
	     and on the stack.  */
	  وإلا إذا (!ډېری_کارول_شوی)
	    {
	      ډېری_کارول_شوی = 1;
	      بشپړ_with_regs = 1;
	      argp = tregp + ffi_put_arg (ty, a, tregp);
	      FFI_ASSERT (eo_regp < argp);
	      استمر;
	    }
	}
      /* Base case, arguments are passed on the stack */
      ډېری_کارول_شوی = 1;
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, a, argp);
    }
}

/* Perform machine dependent cif processing */
ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  صحيح flags = 0, cabi = cif->abi;
  size_t بایټونه = cif->بایټونه;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  إذا (cabi == FFI_VFP)
    layout_vfp_دلیلونه (cif);

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
    حالة FFI_TYPE_اشاره_ګر:
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
      إذا (cif->rtype->اندازه <= 4)
	flags = ARM_TYPE_INT;
      وإلا
	{
	  flags = ARM_TYPE_STRUCT;
	  بایټونه += 4;
	}
      اكسر;

    افتراضي:
      abort();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  بایټونه = FFI_ALIGN (بایټونه, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  إذا (بایټونه < 4*4)
    بایټونه = 4*4;

  cif->بایټونه = بایټونه;
  cif->flags = flags;

  أرجع FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  بینښه صحيح nfixedargs, بینښه صحيح ntotalargs)
{
  /* VFP variadic calls actually use the SYSV ABI */
  إذا (cif->abi == FFI_VFP)
    cif->abi = FFI_SYSV;

  أرجع ffi_prep_cif_machdep (cif);
}

/* Prototypes for assembly functions, in sysv.S.  */

بنية call_چوکاټ
{
  فراغ *fp;
  فراغ *lr;
  فراغ *rvalue;
  صحيح flags;
  فراغ *تړون;
};

خارجي فراغ ffi_call_SYSV (فراغ *ډېری, بنية call_چوکاټ *,
			   فراغ (*fn) (فراغ)) FFI_HIDDEN;
خارجي فراغ ffi_call_VFP (فراغ *vfp_space, بنية call_چوکاټ *,
			   فراغ (*fn) (فراغ), بینښه vfp_کارول_شوی) FFI_HIDDEN;

ساكن فراغ
ffi_call_int (ffi_cif * cif, فراغ (*fn) (فراغ), فراغ *rvalue,
	      فراغ **avalue, فراغ *تړون)
{
  صحيح flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  size_t بایټونه, rsize, vfp_اندازه;
  حرف *ډېری, *vfp_space, *new_rvalue;
  بنية call_چوکاټ *چوکاټ;

  rsize = 0;
  إذا (rvalue == عدم)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      إذا (flags == ARM_TYPE_STRUCT)
	rsize = rtype->اندازه;
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
  vfp_اندازه = (cif->abi == FFI_VFP && cif->vfp_کارول_شوی ? 8*8: 0);

  بایټونه = cif->بایټونه;
  ډېری = alloca (vfp_اندازه + بایټونه + حجم(بنية call_چوکاټ) + rsize);

  vfp_space = عدم;
  إذا (vfp_اندازه)
    {
      vfp_space = ډېری;
      ډېری += vfp_اندازه;
    }

  چوکاټ = (بنية call_چوکاټ *)(ډېری + بایټونه);

  new_rvalue = rvalue;
  إذا (rsize)
    new_rvalue = (فراغ *)(چوکاټ + 1);

  چوکاټ->rvalue = new_rvalue;
  چوکاټ->flags = flags;
  چوکاټ->تړون = تړون;

  إذا (vfp_space)
    {
      ffi_prep_دلیلونه_VFP (cif, flags, new_rvalue, avalue, ډېری, vfp_space);
      ffi_call_VFP (vfp_space, چوکاټ, fn, cif->vfp_کارول_شوی);
    }
  وإلا
    {
      ffi_prep_دلیلونه_SYSV (cif, flags, new_rvalue, avalue, ډېری);
      ffi_call_SYSV (ډېری, چوکاټ, fn);
    }

  إذا (rvalue && rvalue != new_rvalue)
    حافظه_نقل (rvalue, new_rvalue, rtype->اندازه);
}

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, عدم);
}

#که_تعریف_شوی FFI_GO_CLOSURES
فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue,
	     فراغ **avalue, فراغ *تړون)
{
  ffi_call_int (cif, fn, rvalue, avalue, تړون);
}
#د_که_پای

ساكن فراغ *
ffi_prep_incoming_دلیلونه_SYSV (ffi_cif *cif, فراغ *rvalue,
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
      إذا (cif->rtype->اندازه && cif->rtype->اندازه < 4)
        *(uint32_t *) rvalue = 0;
    }

  لكل (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->اندازه;

      argp = ffi_align (ty, argp);
#که_تعریف_شوی __ARMEB__
      إذا (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (فراغ *) (argp + 4 - z);
      وإلا
#د_که_پای
      avalue[i] = (فراغ *) argp;
      argp += z;
    }

  أرجع rvalue;
}

ساكن فراغ *
ffi_prep_incoming_دلیلونه_VFP (ffi_cif *cif, فراغ *rvalue, حرف *ډېری,
			    حرف *vfp_space, فراغ **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  صحيح i, n, vi = 0;
  حرف *argp, *regp, *eo_regp;
  حرف بشپړ_with_regs = 0;
  حرف ډېری_کارول_شوی = 0;

  regp = ډېری;
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
      size_t z = ty->اندازه;

      إذا (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_دلیلونه[vi++] * 4;
	  استمر;
	}
      وإلا إذا (!بشپړ_with_regs && !is_vfp_type)
	{
	  حرف *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  إذا (tregp + z <= eo_regp || !ډېری_کارول_شوی)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#که_تعریف_شوی __ARMEB__
	      إذا (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (فراغ *) (tregp + 4 - z);
	      وإلا
#د_که_پای
	      avalue[i] = (فراغ *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      إذا (regp > eo_regp)
		{
		  FFI_ASSERT (!ډېری_کارول_شوی);
		  argp = regp;
		}
	      إذا (regp >= eo_regp)
		{
		  بشپړ_with_regs = 1;
		  ډېری_کارول_شوی = 1;
		}
	      استمر;
	    }
	}

      ډېری_کارول_شوی = 1;
      argp = ffi_align (ty, argp);
#که_تعریف_شوی __ARMEB__
      إذا (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (فراغ *) (argp + 4 - z);
      وإلا
#د_که_پای
      avalue[i] = (فراغ *) argp;
      argp += z;
    }

  أرجع rvalue;
}

#إذا FFI_CLOSURES

بنية تړون_چوکاټ
{
  حرف vfp_space[8*8] __attribute__((aligned(8)));
  حرف پایله[8*4];
  حرف argp[];
};

صحيح FFI_HIDDEN
ffi_تړون_inner_SYSV (ffi_cif *cif,
		        فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		        فراغ *user_معلومات,
		        بنية تړون_چوکاټ *چوکاټ)
{
  فراغ **avalue = (فراغ **) alloca (cif->nargs * حجم (فراغ *));
  فراغ *rvalue = ffi_prep_incoming_دلیلونه_SYSV (cif, چوکاټ->پایله,
					      چوکاټ->argp, avalue);
  fun (cif, rvalue, avalue, user_معلومات);
  أرجع cif->flags;
}

صحيح FFI_HIDDEN
ffi_تړون_inner_VFP (ffi_cif *cif,
		       فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		       فراغ *user_معلومات,
		       بنية تړون_چوکاټ *چوکاټ)
{
  فراغ **avalue = (فراغ **) alloca (cif->nargs * حجم (فراغ *));
  فراغ *rvalue = ffi_prep_incoming_دلیلونه_VFP (cif, چوکاټ->پایله, چوکاټ->argp,
					     چوکاټ->vfp_space, avalue);
  fun (cif, rvalue, avalue, user_معلومات);
  أرجع cif->flags;
}

فراغ ffi_تړون_SYSV (فراغ) FFI_HIDDEN;
فراغ ffi_تړون_VFP (فراغ) FFI_HIDDEN;
#إذا defined(FFI_EXEC_STATIC_TRAMP)
فراغ ffi_تړون_SYSV_alt (فراغ) FFI_HIDDEN;
فراغ ffi_تړون_VFP_alt (فراغ) FFI_HIDDEN;
#د_که_پای

#که_تعریف_شوی FFI_GO_CLOSURES
فراغ ffi_go_تړون_SYSV (فراغ) FFI_HIDDEN;
فراغ ffi_go_تړون_VFP (فراغ) FFI_HIDDEN;
#د_که_پای

/* the cif must already be prep'ed */

#إذا defined(__FreeBSD__) && defined(__arm__)
#تعریف_کړه __clear_cache(پیل, پای_ټکی) افعل { \
		بنية arm_sync_icache_دلیلونه ua; 		\
								\
		ua.addr = (uintptr_t)(پیل);			\
		ua.len = (حرف *)(پای_ټکی) - (حرف *)پیل;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} طالما (0);
#د_که_پای

ffi_حالت
ffi_prep_تړون_loc (ffi_تړون * تړون,
		      ffi_cif * cif,
		      فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		      فراغ *user_معلومات, فراغ *codeloc)
{
  فراغ (*تړون_func) (فراغ) = ffi_تړون_SYSV;

  إذا (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      إذا (cif->vfp_کارول_شوی)
	تړون_func = ffi_تړون_VFP;
    }
  وإلا إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;

#إذا FFI_EXEC_TRAMPOLINE_جدول
  فراغ **config = (فراغ **)((uint8_t *)codeloc - پاڼه_MAX_اندازه);
  config[0] = تړون;
  config[1] = تړون_func;
#وإلا

# إذا defined(FFI_EXEC_STATIC_TRAMP)
  إذا (ffi_tramp_is_present(تړون))
    {
      /* Initialize the static trampoline's parameters. */
      إذا (تړون_func == ffi_تړون_SYSV)
        تړون_func = ffi_تړون_SYSV_alt;
      وإلا
        تړون_func = ffi_تړون_VFP_alt;
      ffi_tramp_سېټ_parms (تړون->ftramp, تړون_func, تړون);
      اذهب بهر;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  حافظه_نقل(تړون->tramp, ffi_arm_trampoline, 8);
# وإلا
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  حافظه_نقل(تړون->tramp, (فراغ*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_تړون_دننه_واټن);
# endif

# إذا defined(__QNX__)
  msync (تړون->tramp, 8, MS_INVALIDATE_ICACHE);	/* clear data map */
  msync (codeloc, 8, MS_INVALIDATE_ICACHE);		/* clear insn map */
# elif defined(_WIN32)
  FlushInstructionCache(GetCurrentProcess(), تړون->tramp, FFI_TRAMPOLINE_اندازه);
# وإلا
  __clear_cache(تړون->tramp, تړون->tramp + 8);	/* clear data map */
  __clear_cache(codeloc, codeloc + 8);			/* clear insn map */
# endif
# ifdef _WIN32
  *(فراغ(**)(فراغ))(تړون->tramp + FFI_TRAMPOLINE_تړون_FUNCTION) = تړون_func;
# وإلا
  *(فراغ (**)(فراغ))(تړون->tramp + 8) = تړون_func;
# endif
# إذا defined(FFI_EXEC_STATIC_TRAMP)
بهر:
# endif
#د_که_پای

  تړون->cif = cif;
  تړون->fun = fun;
  تړون->user_معلومات = user_معلومات;

  أرجع FFI_OK;
}

#که_تعریف_شوی FFI_GO_CLOSURES
ffi_حالت
ffi_prep_go_تړون (ffi_go_تړون *تړون, ffi_cif *cif,
		     فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *))
{
  فراغ (*تړون_func) (فراغ) = ffi_go_تړون_SYSV;

  إذا (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      إذا (cif->vfp_کارول_شوی)
	تړون_func = ffi_go_تړون_VFP;
    }
  وإلا إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;

  تړون->tramp = تړون_func;
  تړون->cif = cif;
  تړون->fun = fun;

  أرجع FFI_OK;
}
#د_که_پای

#د_که_پای /* FFI_CLOSURES */

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
  size_t اندازه, ele_شمېر;

  /* Quickest tests first.  */
  candidate = ty->type;
  بدل (ty->type)
    {
    افتراضي:
      أرجع 0;
    حالة FFI_TYPE_FLOAT:
    حالة FFI_TYPE_DOUBLE:
      ele_شمېر = 1;
      اذهب بشپړ;
    حالة FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      إذا (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	أرجع 0;
      ele_شمېر = 2;
      اذهب بشپړ;
    حالة FFI_TYPE_STRUCT:
      اكسر;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  اندازه = ty->اندازه;
  إذا (اندازه < 4 || اندازه > 32)
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
      ele_شمېر = اندازه / حجم(عائم);
      إذا (اندازه != ele_شمېر * حجم(عائم))
        أرجع 0;
      اكسر;
    حالة FFI_TYPE_DOUBLE:
      ele_شمېر = اندازه / حجم(مزدوج);
      إذا (اندازه != ele_شمېر * حجم(مزدوج))
        أرجع 0;
      اكسر;
    افتراضي:
      أرجع 0;
    }
  إذا (ele_شمېر > 4)
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
 بشپړ:
  أرجع (ele_شمېر << 8) | candidate;
}

ساكن صحيح
ځای_پرځای_کړه_vfp_arg (ffi_cif *cif, صحيح h)
{
  بینښه لنډ reg = cif->vfp_reg_free;
  صحيح align = 1, nregs = h >> 8;

  إذا ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  إذا ((reg & 1) && align == 2)
    reg++;

  طالما (reg + nregs <= 16)
    {
      صحيح s, new_کارول_شوی = 0;
      لكل (s = reg; s < reg + nregs; s++)
	{
	  new_کارول_شوی |= (1 << s);
	  إذا (cif->vfp_کارول_شوی & (1 << s))
	    {
	      reg += align;
	      اذهب بل_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_کارول_شوی |= new_کارول_شوی;
      cif->vfp_دلیلونه[cif->vfp_nargs++] = (نښهدار حرف)reg;

      /* Update vfp_reg_free. */
      إذا (cif->vfp_کارول_شوی & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  طالما (cif->vfp_کارول_شوی & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      أرجع 0;
    بل_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_کارول_شوی = 0xFFFF;
  أرجع 1;
}

ساكن فراغ
layout_vfp_دلیلونه (ffi_cif * cif)
{
  بینښه صحيح i;
  /* Init VFP fields */
  cif->vfp_کارول_شوی = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  memset (cif->vfp_دلیلونه, -1, 16);	/* Init to -1. */

  لكل (i = 0; i < cif->nargs; i++)
    {
      صحيح h = vfp_type_p (cif->arg_types[i]);
      إذا (h && ځای_پرځای_کړه_vfp_arg (cif, h) == 1)
	اكسر;
    }
}

#إذا defined(FFI_EXEC_STATIC_TRAMP)
فراغ *
ffi_tramp_arch (size_t *tramp_اندازه, size_t *map_اندازه)
{
  خارجي فراغ *trampoline_کوډ_جدول;

  *tramp_اندازه = ARM_TRAMP_اندازه;
  *map_اندازه = ARM_TRAMP_MAP_اندازه;
  أرجع &trampoline_کوډ_جدول;
}
#د_که_پای

#د_که_پای /* __arm__ or _M_ARM */
