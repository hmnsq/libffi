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

#اگر defined(__arm__) || defined(_M_ARM)
#شامل <fficonfig.h>
#شامل <ffi.h>
#شامل <ffi_common.h>
#شامل <stdint.h>
#شامل <stdlib.h>
#شامل <tramp.h>
#شامل "internal.h"

#اگر defined(_WIN32)
#تعریف_کرو WIN32_LEAN_AND_اوسط
#شامل <windows.h>
#اگر_ختم

#اگر FFI_EXEC_TRAMPOLINE_جدول

#اگر_تعریف_شدہ __MACH__
#شامل <mach/machine/vm_param.h>
#اگر_ختم

#ورنہ
#اگر_غیر_تعریف_شدہ _WIN32
خارجی غیرعلامتی صحیح ffi_arm_trampoline[2] FFI_HIDDEN;
#ورنہ
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
خارجی غیرعلامتی حرف ffi_arm_trampoline[12] FFI_HIDDEN;
#اگر_ختم
#اگر_ختم

#اگر defined(__FreeBSD__) && defined(__arm__)
#شامل <sys/types.h>
#شامل <machine/sysarch.h>
#اگر_ختم

#اگر defined(__QNX__)
#شامل <sys/mman.h>
#اگر_ختم

/* Forward declares. */
ساکن صحیح vfp_type_p (ثابت ffi_type *);
ساکن خالی layout_vfp_دلائل (ffi_cif *);

ساکن خالی *
ffi_align (ffi_type *ty, خالی *p)
{
  /* Align if necessary */
  size_t alignment;
#اگر_تعریف_شدہ _WIN32_WCE
  alignment = 4;
#ورنہ
  alignment = ty->alignment;
  اگر (alignment < 4)
    alignment = 4;
#اگر_ختم
  واپس (خالی *) FFI_ALIGN (p, alignment);
}

ساکن size_t
ffi_put_arg (ffi_type *ty, خالی *src, خالی *dst)
{
  size_t z = ty->سائز;

  انتخاب (ty->type)
    {
    حالت FFI_TYPE_SINT8:
      *(UINT32 *)dst = *(SINT8 *)src;
      توڑو;
    حالت FFI_TYPE_UINT8:
      *(UINT32 *)dst = *(UINT8 *)src;
      توڑو;
    حالت FFI_TYPE_SINT16:
      *(UINT32 *)dst = *(SINT16 *)src;
      توڑو;
    حالت FFI_TYPE_UINT16:
      *(UINT32 *)dst = *(UINT16 *)src;
      توڑو;

    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_SINT32:
    حالت FFI_TYPE_UINT32:
    حالت FFI_TYPE_اشارہ_گر:
#اگر_غیر_تعریف_شدہ _WIN32
    حالت FFI_TYPE_FLOAT:
#اگر_ختم
      *(UINT32 *)dst = *(UINT32 *)src;
      توڑو;

#اگر_تعریف_شدہ _WIN32
    // casting a float* to a UINT32* doesn't work on Windows
    حالت FFI_TYPE_FLOAT:
        *(uintptr_t *)dst = 0;
        *(اعشاری *)dst = *(اعشاری *)src;
        توڑو;
#اگر_ختم

    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_DOUBLE:
      *(UINT64 *)dst = *(UINT64 *)src;
      توڑو;

    حالت FFI_TYPE_STRUCT:
    حالت FFI_TYPE_COMPLEX:
      حافظہ_نقل (dst, src, z);
      توڑو;

    طےشدہ:
      abort();
    }

  واپس FFI_ALIGN (z, 4);
}

/* ffi_prep_args is called once stack space has been allocated
   for the function's arguments.

   The vfp_space parameter is the load area for VFP regs, the return
   value is cif->vfp_used (word bitset of VFP regs used for passing
   arguments). These are only used for the VFP hard-float ABI.
*/
ساکن خالی
ffi_prep_دلائل_SYSV (ffi_cif *cif, صحیح flags, خالی *rvalue,
		    خالی **avalue, حرف *argp)
{
  ffi_type **arg_types = cif->arg_types;
  صحیح i, n;

  اگر (flags == ARM_TYPE_STRUCT)
    {
      *(خالی **) argp = rvalue;
      argp += 4;
    }

  برائے (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, avalue[i], argp);
    }
}

ساکن خالی
ffi_prep_دلائل_VFP (ffi_cif *cif, صحیح flags, خالی *rvalue,
                   خالی **avalue, حرف *ڈھیر, حرف *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  صحیح i, n, vi = 0;
  حرف *argp, *regp, *eo_regp;
  حرف ڈھیر_مستعمل = 0;
  حرف مکمل_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = ڈھیر;
  eo_regp = argp = regp + 16;

  /* If the function returns an FFI_TYPE_STRUCT in memory,
     that address is passed in r0 to the function.  */
  اگر (flags == ARM_TYPE_STRUCT)
    {
      *(خالی **) regp = rvalue;
      regp += 4;
    }

  برائے (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      خالی *a = avalue[i];
      صحیح is_vfp_type = vfp_type_p (ty);

      /* Allocated in VFP registers. */
      اگر (vi < cif->vfp_nargs && is_vfp_type)
	{
	  حرف *vfp_خانہ = vfp_space + cif->vfp_دلائل[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_خانہ);
	  جاری;
	}
      /* Try allocating in core registers. */
      ورنہ اگر (!مکمل_with_regs && !is_vfp_type)
	{
	  حرف *tregp = ffi_align (ty, regp);
	  size_t سائز = ty->سائز;
	  سائز = (سائز < 4) ? 4 : سائز;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  اگر (tregp + سائز <= eo_regp)
	    {
	      regp = tregp + ffi_put_arg (ty, a, tregp);
	      مکمل_with_regs = (regp == argp);
	      // ensure we did not write into the stack area
	      FFI_ASSERT (regp <= argp);
	      جاری;
	    }
	  /* In case there are no arguments in the stack area yet,
	     the argument is passed in the remaining core registers
	     and on the stack.  */
	  ورنہ اگر (!ڈھیر_مستعمل)
	    {
	      ڈھیر_مستعمل = 1;
	      مکمل_with_regs = 1;
	      argp = tregp + ffi_put_arg (ty, a, tregp);
	      FFI_ASSERT (eo_regp < argp);
	      جاری;
	    }
	}
      /* Base case, arguments are passed on the stack */
      ڈھیر_مستعمل = 1;
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, a, argp);
    }
}

/* Perform machine dependent cif processing */
ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  صحیح flags = 0, cabi = cif->abi;
  size_t بائٹیں = cif->بائٹیں;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  اگر (cabi == FFI_VFP)
    layout_vfp_دلائل (cif);

  /* Set the return type flag */
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_VOID:
      flags = ARM_TYPE_VOID;
      توڑو;

    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_UINT8:
    حالت FFI_TYPE_SINT8:
    حالت FFI_TYPE_UINT16:
    حالت FFI_TYPE_SINT16:
    حالت FFI_TYPE_UINT32:
    حالت FFI_TYPE_SINT32:
    حالت FFI_TYPE_اشارہ_گر:
      flags = ARM_TYPE_INT;
      توڑو;

    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
      flags = ARM_TYPE_INT64;
      توڑو;

    حالت FFI_TYPE_FLOAT:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_S : ARM_TYPE_INT);
      توڑو;
    حالت FFI_TYPE_DOUBLE:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_D : ARM_TYPE_INT64);
      توڑو;

    حالت FFI_TYPE_STRUCT:
    حالت FFI_TYPE_COMPLEX:
      اگر (cabi == FFI_VFP)
	{
	  صحیح h = vfp_type_p (cif->rtype);

	  flags = ARM_TYPE_VFP_N;
	  اگر (h == 0x100 + FFI_TYPE_FLOAT)
	    flags = ARM_TYPE_VFP_S;
	  اگر (h == 0x100 + FFI_TYPE_DOUBLE)
	    flags = ARM_TYPE_VFP_D;
	  اگر (h != 0)
	      توڑو;
	}

      /* A Composite Type not larger than 4 bytes is returned in r0.
	 A Composite Type larger than 4 bytes, or whose size cannot
	 be determined statically ... is stored in memory at an
	 address passed [in r0].  */
      اگر (cif->rtype->سائز <= 4)
	flags = ARM_TYPE_INT;
      ورنہ
	{
	  flags = ARM_TYPE_STRUCT;
	  بائٹیں += 4;
	}
      توڑو;

    طےشدہ:
      abort();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  بائٹیں = FFI_ALIGN (بائٹیں, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  اگر (بائٹیں < 4*4)
    بائٹیں = 4*4;

  cif->بائٹیں = بائٹیں;
  cif->flags = flags;

  واپس FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  غیرعلامتی صحیح nfixedargs, غیرعلامتی صحیح ntotalargs)
{
  /* VFP variadic calls actually use the SYSV ABI */
  اگر (cif->abi == FFI_VFP)
    cif->abi = FFI_SYSV;

  واپس ffi_prep_cif_machdep (cif);
}

/* Prototypes for assembly functions, in sysv.S.  */

ساخت call_چوکھٹا
{
  خالی *fp;
  خالی *lr;
  خالی *rvalue;
  صحیح flags;
  خالی *احاطہ;
};

خارجی خالی ffi_call_SYSV (خالی *ڈھیر, ساخت call_چوکھٹا *,
			   خالی (*fn) (خالی)) FFI_HIDDEN;
خارجی خالی ffi_call_VFP (خالی *vfp_space, ساخت call_چوکھٹا *,
			   خالی (*fn) (خالی), غیرعلامتی vfp_مستعمل) FFI_HIDDEN;

ساکن خالی
ffi_call_int (ffi_cif * cif, خالی (*fn) (خالی), خالی *rvalue,
	      خالی **avalue, خالی *احاطہ)
{
  صحیح flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  size_t بائٹیں, rsize, vfp_سائز;
  حرف *ڈھیر, *vfp_space, *new_rvalue;
  ساخت call_چوکھٹا *چوکھٹا;

  rsize = 0;
  اگر (rvalue == تہی)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      اگر (flags == ARM_TYPE_STRUCT)
	rsize = rtype->سائز;
      ورنہ
	flags = ARM_TYPE_VOID;
    }
  ورنہ اگر (flags == ARM_TYPE_VFP_N)
    {
      /* Largest case is double x 4. */
      rsize = 32;
    }
  ورنہ اگر (flags == ARM_TYPE_INT && rtype->type == FFI_TYPE_STRUCT)
    rsize = 4;

  /* Largest case.  */
  vfp_سائز = (cif->abi == FFI_VFP && cif->vfp_مستعمل ? 8*8: 0);

  بائٹیں = cif->بائٹیں;
  ڈھیر = alloca (vfp_سائز + بائٹیں + حجم(ساخت call_چوکھٹا) + rsize);

  vfp_space = تہی;
  اگر (vfp_سائز)
    {
      vfp_space = ڈھیر;
      ڈھیر += vfp_سائز;
    }

  چوکھٹا = (ساخت call_چوکھٹا *)(ڈھیر + بائٹیں);

  new_rvalue = rvalue;
  اگر (rsize)
    new_rvalue = (خالی *)(چوکھٹا + 1);

  چوکھٹا->rvalue = new_rvalue;
  چوکھٹا->flags = flags;
  چوکھٹا->احاطہ = احاطہ;

  اگر (vfp_space)
    {
      ffi_prep_دلائل_VFP (cif, flags, new_rvalue, avalue, ڈھیر, vfp_space);
      ffi_call_VFP (vfp_space, چوکھٹا, fn, cif->vfp_مستعمل);
    }
  ورنہ
    {
      ffi_prep_دلائل_SYSV (cif, flags, new_rvalue, avalue, ڈھیر);
      ffi_call_SYSV (ڈھیر, چوکھٹا, fn);
    }

  اگر (rvalue && rvalue != new_rvalue)
    حافظہ_نقل (rvalue, new_rvalue, rtype->سائز);
}

خالی
ffi_call (ffi_cif *cif, خالی (*fn) (خالی), خالی *rvalue, خالی **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, تہی);
}

#اگر_تعریف_شدہ FFI_GO_CLOSURES
خالی
ffi_call_go (ffi_cif *cif, خالی (*fn) (خالی), خالی *rvalue,
	     خالی **avalue, خالی *احاطہ)
{
  ffi_call_int (cif, fn, rvalue, avalue, احاطہ);
}
#اگر_ختم

ساکن خالی *
ffi_prep_incoming_دلائل_SYSV (ffi_cif *cif, خالی *rvalue,
			     حرف *argp, خالی **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  صحیح i, n;

  اگر (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(خالی **) argp;
      argp += 4;
    }
  ورنہ
    {
      اگر (cif->rtype->سائز && cif->rtype->سائز < 4)
        *(uint32_t *) rvalue = 0;
    }

  برائے (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->سائز;

      argp = ffi_align (ty, argp);
#اگر_تعریف_شدہ __ARMEB__
      اگر (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (خالی *) (argp + 4 - z);
      ورنہ
#اگر_ختم
      avalue[i] = (خالی *) argp;
      argp += z;
    }

  واپس rvalue;
}

ساکن خالی *
ffi_prep_incoming_دلائل_VFP (ffi_cif *cif, خالی *rvalue, حرف *ڈھیر,
			    حرف *vfp_space, خالی **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  صحیح i, n, vi = 0;
  حرف *argp, *regp, *eo_regp;
  حرف مکمل_with_regs = 0;
  حرف ڈھیر_مستعمل = 0;

  regp = ڈھیر;
  eo_regp = argp = regp + 16;

  اگر (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(خالی **) regp;
      regp += 4;
    }

  برائے (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      صحیح is_vfp_type = vfp_type_p (ty);
      size_t z = ty->سائز;

      اگر (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_دلائل[vi++] * 4;
	  جاری;
	}
      ورنہ اگر (!مکمل_with_regs && !is_vfp_type)
	{
	  حرف *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  اگر (tregp + z <= eo_regp || !ڈھیر_مستعمل)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#اگر_تعریف_شدہ __ARMEB__
	      اگر (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (خالی *) (tregp + 4 - z);
	      ورنہ
#اگر_ختم
	      avalue[i] = (خالی *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      اگر (regp > eo_regp)
		{
		  FFI_ASSERT (!ڈھیر_مستعمل);
		  argp = regp;
		}
	      اگر (regp >= eo_regp)
		{
		  مکمل_with_regs = 1;
		  ڈھیر_مستعمل = 1;
		}
	      جاری;
	    }
	}

      ڈھیر_مستعمل = 1;
      argp = ffi_align (ty, argp);
#اگر_تعریف_شدہ __ARMEB__
      اگر (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (خالی *) (argp + 4 - z);
      ورنہ
#اگر_ختم
      avalue[i] = (خالی *) argp;
      argp += z;
    }

  واپس rvalue;
}

#اگر FFI_CLOSURES

ساخت احاطہ_چوکھٹا
{
  حرف vfp_space[8*8] __attribute__((aligned(8)));
  حرف نتیجہ[8*4];
  حرف argp[];
};

صحیح FFI_HIDDEN
ffi_احاطہ_inner_SYSV (ffi_cif *cif,
		        خالی (*fun) (ffi_cif *, خالی *, خالی **, خالی *),
		        خالی *user_ڈیٹا,
		        ساخت احاطہ_چوکھٹا *چوکھٹا)
{
  خالی **avalue = (خالی **) alloca (cif->nargs * حجم (خالی *));
  خالی *rvalue = ffi_prep_incoming_دلائل_SYSV (cif, چوکھٹا->نتیجہ,
					      چوکھٹا->argp, avalue);
  fun (cif, rvalue, avalue, user_ڈیٹا);
  واپس cif->flags;
}

صحیح FFI_HIDDEN
ffi_احاطہ_inner_VFP (ffi_cif *cif,
		       خالی (*fun) (ffi_cif *, خالی *, خالی **, خالی *),
		       خالی *user_ڈیٹا,
		       ساخت احاطہ_چوکھٹا *چوکھٹا)
{
  خالی **avalue = (خالی **) alloca (cif->nargs * حجم (خالی *));
  خالی *rvalue = ffi_prep_incoming_دلائل_VFP (cif, چوکھٹا->نتیجہ, چوکھٹا->argp,
					     چوکھٹا->vfp_space, avalue);
  fun (cif, rvalue, avalue, user_ڈیٹا);
  واپس cif->flags;
}

خالی ffi_احاطہ_SYSV (خالی) FFI_HIDDEN;
خالی ffi_احاطہ_VFP (خالی) FFI_HIDDEN;
#اگر defined(FFI_EXEC_STATIC_TRAMP)
خالی ffi_احاطہ_SYSV_alt (خالی) FFI_HIDDEN;
خالی ffi_احاطہ_VFP_alt (خالی) FFI_HIDDEN;
#اگر_ختم

#اگر_تعریف_شدہ FFI_GO_CLOSURES
خالی ffi_go_احاطہ_SYSV (خالی) FFI_HIDDEN;
خالی ffi_go_احاطہ_VFP (خالی) FFI_HIDDEN;
#اگر_ختم

/* the cif must already be prep'ed */

#اگر defined(__FreeBSD__) && defined(__arm__)
#تعریف_کرو __clear_cache(آغاز, انتہا) کرو { \
		ساخت arm_sync_icache_دلائل ua; 		\
								\
		ua.addr = (uintptr_t)(آغاز);			\
		ua.len = (حرف *)(انتہا) - (حرف *)آغاز;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} جب (0);
#اگر_ختم

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ * احاطہ,
		      ffi_cif * cif,
		      خالی (*fun) (ffi_cif *, خالی *, خالی **, خالی *),
		      خالی *user_ڈیٹا, خالی *codeloc)
{
  خالی (*احاطہ_func) (خالی) = ffi_احاطہ_SYSV;

  اگر (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      اگر (cif->vfp_مستعمل)
	احاطہ_func = ffi_احاطہ_VFP;
    }
  ورنہ اگر (cif->abi != FFI_SYSV)
    واپس FFI_BAD_ABI;

#اگر FFI_EXEC_TRAMPOLINE_جدول
  خالی **config = (خالی **)((uint8_t *)codeloc - صفحہ_MAX_سائز);
  config[0] = احاطہ;
  config[1] = احاطہ_func;
#ورنہ

# اگر defined(FFI_EXEC_STATIC_TRAMP)
  اگر (ffi_tramp_is_present(احاطہ))
    {
      /* Initialize the static trampoline's parameters. */
      اگر (احاطہ_func == ffi_احاطہ_SYSV)
        احاطہ_func = ffi_احاطہ_SYSV_alt;
      ورنہ
        احاطہ_func = ffi_احاطہ_VFP_alt;
      ffi_tramp_سیٹ_parms (احاطہ->ftramp, احاطہ_func, احاطہ);
      جاؤ برآمد;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  حافظہ_نقل(احاطہ->tramp, ffi_arm_trampoline, 8);
# ورنہ
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  حافظہ_نقل(احاطہ->tramp, (خالی*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_احاطہ_انحراف);
# endif

# اگر defined(__QNX__)
  msync (احاطہ->tramp, 8, MS_INVALIDATE_ICACHE);	/* clear data map */
  msync (codeloc, 8, MS_INVALIDATE_ICACHE);		/* clear insn map */
# elif defined(_WIN32)
  FlushInstructionCache(GetCurrentProcess(), احاطہ->tramp, FFI_TRAMPOLINE_سائز);
# ورنہ
  __clear_cache(احاطہ->tramp, احاطہ->tramp + 8);	/* clear data map */
  __clear_cache(codeloc, codeloc + 8);			/* clear insn map */
# endif
# ifdef _WIN32
  *(خالی(**)(خالی))(احاطہ->tramp + FFI_TRAMPOLINE_احاطہ_FUNCTION) = احاطہ_func;
# ورنہ
  *(خالی (**)(خالی))(احاطہ->tramp + 8) = احاطہ_func;
# endif
# اگر defined(FFI_EXEC_STATIC_TRAMP)
برآمد:
# endif
#اگر_ختم

  احاطہ->cif = cif;
  احاطہ->fun = fun;
  احاطہ->user_ڈیٹا = user_ڈیٹا;

  واپس FFI_OK;
}

#اگر_تعریف_شدہ FFI_GO_CLOSURES
ffi_حالت
ffi_prep_go_احاطہ (ffi_go_احاطہ *احاطہ, ffi_cif *cif,
		     خالی (*fun) (ffi_cif *, خالی *, خالی **, خالی *))
{
  خالی (*احاطہ_func) (خالی) = ffi_go_احاطہ_SYSV;

  اگر (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      اگر (cif->vfp_مستعمل)
	احاطہ_func = ffi_go_احاطہ_VFP;
    }
  ورنہ اگر (cif->abi != FFI_SYSV)
    واپس FFI_BAD_ABI;

  احاطہ->tramp = احاطہ_func;
  احاطہ->cif = cif;
  احاطہ->fun = fun;

  واپس FFI_OK;
}
#اگر_ختم

#اگر_ختم /* FFI_CLOSURES */

/* Below are routines for VFP hard-float support. */

/* A subroutine of vfp_type_p.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

ساکن صحیح
is_hfa0 (ثابت ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  صحیح i, ret = -1;

  اگر (elements != تہی)
    برائے (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        اگر (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            اگر (ret < 0)
              جاری;
          }
        توڑو;
      }

  واپس ret;
}

/* A subroutine of vfp_type_p.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

ساکن صحیح
is_hfa1 (ثابت ffi_type *ty, صحیح candidate)
{
  ffi_type **elements = ty->elements;
  صحیح i;

  اگر (elements != تہی)
    برائے (i = 0; elements[i]; ++i)
      {
        صحیح t = elements[i]->type;
        اگر (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
          {
            اگر (!is_hfa1 (elements[i], candidate))
              واپس 0;
          }
        ورنہ اگر (t != candidate)
          واپس 0;
      }

  واپس 1;
}

/* Determine if TY is an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is a floating point scalar.

   Returns non-zero iff TY is an HFA.  The result is an encoded value where
   bits 0-7 contain the type code, and bits 8-10 contain the element count.  */

ساکن صحیح
vfp_type_p (ثابت ffi_type *ty)
{
  ffi_type **elements;
  صحیح candidate, i;
  size_t سائز, ele_گنتی;

  /* Quickest tests first.  */
  candidate = ty->type;
  انتخاب (ty->type)
    {
    طےشدہ:
      واپس 0;
    حالت FFI_TYPE_FLOAT:
    حالت FFI_TYPE_DOUBLE:
      ele_گنتی = 1;
      جاؤ مکمل;
    حالت FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      اگر (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	واپس 0;
      ele_گنتی = 2;
      جاؤ مکمل;
    حالت FFI_TYPE_STRUCT:
      توڑو;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  سائز = ty->سائز;
  اگر (سائز < 4 || سائز > 32)
    واپس 0;

  /* Find the type of the first non-structure member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  اگر (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_COMPLEX)
    {
      برائے (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          اگر (candidate >= 0)
            توڑو;
        }
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  انتخاب (candidate)
    {
    حالت FFI_TYPE_FLOAT:
      ele_گنتی = سائز / حجم(اعشاری);
      اگر (سائز != ele_گنتی * حجم(اعشاری))
        واپس 0;
      توڑو;
    حالت FFI_TYPE_DOUBLE:
      ele_گنتی = سائز / حجم(دوہرا);
      اگر (سائز != ele_گنتی * حجم(دوہرا))
        واپس 0;
      توڑو;
    طےشدہ:
      واپس 0;
    }
  اگر (ele_گنتی > 4)
    واپس 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  برائے (i = 0; elements[i]; ++i)
    {
      صحیح t = elements[i]->type;
      اگر (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          اگر (!is_hfa1 (elements[i], candidate))
            واپس 0;
        }
      ورنہ اگر (t != candidate)
        واپس 0;
    }

  /* All tests succeeded.  Encode the result.  */
 مکمل:
  واپس (ele_گنتی << 8) | candidate;
}

ساکن صحیح
رکھو_vfp_arg (ffi_cif *cif, صحیح h)
{
  غیرعلامتی مختصر reg = cif->vfp_reg_free;
  صحیح align = 1, nregs = h >> 8;

  اگر ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  اگر ((reg & 1) && align == 2)
    reg++;

  جب (reg + nregs <= 16)
    {
      صحیح s, new_مستعمل = 0;
      برائے (s = reg; s < reg + nregs; s++)
	{
	  new_مستعمل |= (1 << s);
	  اگر (cif->vfp_مستعمل & (1 << s))
	    {
	      reg += align;
	      جاؤ اگلا_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_مستعمل |= new_مستعمل;
      cif->vfp_دلائل[cif->vfp_nargs++] = (علامتی حرف)reg;

      /* Update vfp_reg_free. */
      اگر (cif->vfp_مستعمل & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  جب (cif->vfp_مستعمل & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      واپس 0;
    اگلا_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_مستعمل = 0xFFFF;
  واپس 1;
}

ساکن خالی
layout_vfp_دلائل (ffi_cif * cif)
{
  غیرعلامتی صحیح i;
  /* Init VFP fields */
  cif->vfp_مستعمل = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  memset (cif->vfp_دلائل, -1, 16);	/* Init to -1. */

  برائے (i = 0; i < cif->nargs; i++)
    {
      صحیح h = vfp_type_p (cif->arg_types[i]);
      اگر (h && رکھو_vfp_arg (cif, h) == 1)
	توڑو;
    }
}

#اگر defined(FFI_EXEC_STATIC_TRAMP)
خالی *
ffi_tramp_arch (size_t *tramp_سائز, size_t *map_سائز)
{
  خارجی خالی *trampoline_رمز_جدول;

  *tramp_سائز = ARM_TRAMP_سائز;
  *map_سائز = ARM_TRAMP_MAP_سائز;
  واپس &trampoline_رمز_جدول;
}
#اگر_ختم

#اگر_ختم /* __arm__ or _M_ARM */
