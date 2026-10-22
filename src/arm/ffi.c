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

#ఒకవేళ defined(__arm__) || defined(_M_ARM)
#అన్తర్భావ <fficonfig.h>
#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <tramp.h>
#అన్తర్భావ "internal.h"

#ఒకవేళ defined(_WIN32)
#నిర్వచించు WIN32_LEAN_AND_సగటు
#అన్తర్భావ <windows.h>
#అయితే_ముగింపు

#ఒకవేళ FFI_EXEC_TRAMPOLINE_పట్టిక

#నిర్వచితమైతే __MACH__
#అన్తర్భావ <mach/machine/vm_param.h>
#అయితే_ముగింపు

#లేకపోతే
#నిర్వచితం_కాకపోతే _WIN32
బాహ్య గుర్తులేని పూర్ణాంకం ffi_arm_trampoline[2] FFI_HIDDEN;
#లేకపోతే
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
బాహ్య గుర్తులేని అక్షరం ffi_arm_trampoline[12] FFI_HIDDEN;
#అయితే_ముగింపు
#అయితే_ముగింపు

#ఒకవేళ defined(__FreeBSD__) && defined(__arm__)
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <machine/sysarch.h>
#అయితే_ముగింపు

#ఒకవేళ defined(__QNX__)
#అన్తర్భావ <sys/mman.h>
#అయితే_ముగింపు

/* Forward declares. */
స్థిర పూర్ణాంకం vfp_type_p (స్థిరరాశి ffi_type *);
స్థిర శూన్య layout_vfp_వాదనలు (ffi_cif *);

స్థిర శూన్య *
ffi_align (ffi_type *ty, శూన్య *p)
{
  /* Align if necessary */
  size_t alignment;
#నిర్వచితమైతే _WIN32_WCE
  alignment = 4;
#లేకపోతే
  alignment = ty->alignment;
  ఒకవేళ (alignment < 4)
    alignment = 4;
#అయితే_ముగింపు
  తిరిగి (శూన్య *) FFI_ALIGN (p, alignment);
}

స్థిర size_t
ffi_put_arg (ffi_type *ty, శూన్య *src, శూన్య *dst)
{
  size_t z = ty->కొలత;

  ఎంపిక (ty->type)
    {
    స్థితి FFI_TYPE_SINT8:
      *(UINT32 *)dst = *(SINT8 *)src;
      ఆపు;
    స్థితి FFI_TYPE_UINT8:
      *(UINT32 *)dst = *(UINT8 *)src;
      ఆపు;
    స్థితి FFI_TYPE_SINT16:
      *(UINT32 *)dst = *(SINT16 *)src;
      ఆపు;
    స్థితి FFI_TYPE_UINT16:
      *(UINT32 *)dst = *(UINT16 *)src;
      ఆపు;

    స్థితి FFI_TYPE_INT:
    స్థితి FFI_TYPE_SINT32:
    స్థితి FFI_TYPE_UINT32:
    స్థితి FFI_TYPE_సూచిక:
#నిర్వచితం_కాకపోతే _WIN32
    స్థితి FFI_TYPE_FLOAT:
#అయితే_ముగింపు
      *(UINT32 *)dst = *(UINT32 *)src;
      ఆపు;

#నిర్వచితమైతే _WIN32
    // casting a float* to a UINT32* doesn't work on Windows
    స్థితి FFI_TYPE_FLOAT:
        *(uintptr_t *)dst = 0;
        *(దశాంశం *)dst = *(దశాంశం *)src;
        ఆపు;
#అయితే_ముగింపు

    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_UINT64:
    స్థితి FFI_TYPE_DOUBLE:
      *(UINT64 *)dst = *(UINT64 *)src;
      ఆపు;

    స్థితి FFI_TYPE_STRUCT:
    స్థితి FFI_TYPE_COMPLEX:
      జ్ఞాపకం_నకలు (dst, src, z);
      ఆపు;

    అప్రమేయ:
      abort();
    }

  తిరిగి FFI_ALIGN (z, 4);
}

/* ffi_prep_args is called once stack space has been allocated
   for the function's arguments.

   The vfp_space parameter is the load area for VFP regs, the return
   value is cif->vfp_used (word bitset of VFP regs used for passing
   arguments). These are only used for the VFP hard-float ABI.
*/
స్థిర శూన్య
ffi_prep_వాదనలు_SYSV (ffi_cif *cif, పూర్ణాంకం flags, శూన్య *rvalue,
		    శూన్య **avalue, అక్షరం *argp)
{
  ffi_type **arg_types = cif->arg_types;
  పూర్ణాంకం i, n;

  ఒకవేళ (flags == ARM_TYPE_STRUCT)
    {
      *(శూన్య **) argp = rvalue;
      argp += 4;
    }

  కొరకు (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, avalue[i], argp);
    }
}

స్థిర శూన్య
ffi_prep_వాదనలు_VFP (ffi_cif *cif, పూర్ణాంకం flags, శూన్య *rvalue,
                   శూన్య **avalue, అక్షరం *దొంతర, అక్షరం *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  పూర్ణాంకం i, n, vi = 0;
  అక్షరం *argp, *regp, *eo_regp;
  అక్షరం దొంతర_వాడిన = 0;
  అక్షరం పూర్తి_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = దొంతర;
  eo_regp = argp = regp + 16;

  /* If the function returns an FFI_TYPE_STRUCT in memory,
     that address is passed in r0 to the function.  */
  ఒకవేళ (flags == ARM_TYPE_STRUCT)
    {
      *(శూన్య **) regp = rvalue;
      regp += 4;
    }

  కొరకు (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      శూన్య *a = avalue[i];
      పూర్ణాంకం is_vfp_type = vfp_type_p (ty);

      /* Allocated in VFP registers. */
      ఒకవేళ (vi < cif->vfp_nargs && is_vfp_type)
	{
	  అక్షరం *vfp_స్థానం = vfp_space + cif->vfp_వాదనలు[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_స్థానం);
	  కొనసాగు;
	}
      /* Try allocating in core registers. */
      లేకపోతే ఒకవేళ (!పూర్తి_with_regs && !is_vfp_type)
	{
	  అక్షరం *tregp = ffi_align (ty, regp);
	  size_t కొలత = ty->కొలత;
	  కొలత = (కొలత < 4) ? 4 : కొలత;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  ఒకవేళ (tregp + కొలత <= eo_regp)
	    {
	      regp = tregp + ffi_put_arg (ty, a, tregp);
	      పూర్తి_with_regs = (regp == argp);
	      // ensure we did not write into the stack area
	      FFI_ASSERT (regp <= argp);
	      కొనసాగు;
	    }
	  /* In case there are no arguments in the stack area yet,
	     the argument is passed in the remaining core registers
	     and on the stack.  */
	  లేకపోతే ఒకవేళ (!దొంతర_వాడిన)
	    {
	      దొంతర_వాడిన = 1;
	      పూర్తి_with_regs = 1;
	      argp = tregp + ffi_put_arg (ty, a, tregp);
	      FFI_ASSERT (eo_regp < argp);
	      కొనసాగు;
	    }
	}
      /* Base case, arguments are passed on the stack */
      దొంతర_వాడిన = 1;
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, a, argp);
    }
}

/* Perform machine dependent cif processing */
ffi_స్థితి FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  పూర్ణాంకం flags = 0, cabi = cif->abi;
  size_t బైట్లు = cif->బైట్లు;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  ఒకవేళ (cabi == FFI_VFP)
    layout_vfp_వాదనలు (cif);

  /* Set the return type flag */
  ఎంపిక (cif->rtype->type)
    {
    స్థితి FFI_TYPE_VOID:
      flags = ARM_TYPE_VOID;
      ఆపు;

    స్థితి FFI_TYPE_INT:
    స్థితి FFI_TYPE_UINT8:
    స్థితి FFI_TYPE_SINT8:
    స్థితి FFI_TYPE_UINT16:
    స్థితి FFI_TYPE_SINT16:
    స్థితి FFI_TYPE_UINT32:
    స్థితి FFI_TYPE_SINT32:
    స్థితి FFI_TYPE_సూచిక:
      flags = ARM_TYPE_INT;
      ఆపు;

    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_UINT64:
      flags = ARM_TYPE_INT64;
      ఆపు;

    స్థితి FFI_TYPE_FLOAT:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_S : ARM_TYPE_INT);
      ఆపు;
    స్థితి FFI_TYPE_DOUBLE:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_D : ARM_TYPE_INT64);
      ఆపు;

    స్థితి FFI_TYPE_STRUCT:
    స్థితి FFI_TYPE_COMPLEX:
      ఒకవేళ (cabi == FFI_VFP)
	{
	  పూర్ణాంకం h = vfp_type_p (cif->rtype);

	  flags = ARM_TYPE_VFP_N;
	  ఒకవేళ (h == 0x100 + FFI_TYPE_FLOAT)
	    flags = ARM_TYPE_VFP_S;
	  ఒకవేళ (h == 0x100 + FFI_TYPE_DOUBLE)
	    flags = ARM_TYPE_VFP_D;
	  ఒకవేళ (h != 0)
	      ఆపు;
	}

      /* A Composite Type not larger than 4 bytes is returned in r0.
	 A Composite Type larger than 4 bytes, or whose size cannot
	 be determined statically ... is stored in memory at an
	 address passed [in r0].  */
      ఒకవేళ (cif->rtype->కొలత <= 4)
	flags = ARM_TYPE_INT;
      లేకపోతే
	{
	  flags = ARM_TYPE_STRUCT;
	  బైట్లు += 4;
	}
      ఆపు;

    అప్రమేయ:
      abort();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  బైట్లు = FFI_ALIGN (బైట్లు, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  ఒకవేళ (బైట్లు < 4*4)
    బైట్లు = 4*4;

  cif->బైట్లు = బైట్లు;
  cif->flags = flags;

  తిరిగి FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_స్థితి FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  గుర్తులేని పూర్ణాంకం nfixedargs, గుర్తులేని పూర్ణాంకం ntotalargs)
{
  /* VFP variadic calls actually use the SYSV ABI */
  ఒకవేళ (cif->abi == FFI_VFP)
    cif->abi = FFI_SYSV;

  తిరిగి ffi_prep_cif_machdep (cif);
}

/* Prototypes for assembly functions, in sysv.S.  */

నిర్మాణం call_చట్రం
{
  శూన్య *fp;
  శూన్య *lr;
  శూన్య *rvalue;
  పూర్ణాంకం flags;
  శూన్య *సంవృతం;
};

బాహ్య శూన్య ffi_call_SYSV (శూన్య *దొంతర, నిర్మాణం call_చట్రం *,
			   శూన్య (*fn) (శూన్య)) FFI_HIDDEN;
బాహ్య శూన్య ffi_call_VFP (శూన్య *vfp_space, నిర్మాణం call_చట్రం *,
			   శూన్య (*fn) (శూన్య), గుర్తులేని vfp_వాడిన) FFI_HIDDEN;

స్థిర శూన్య
ffi_call_int (ffi_cif * cif, శూన్య (*fn) (శూన్య), శూన్య *rvalue,
	      శూన్య **avalue, శూన్య *సంవృతం)
{
  పూర్ణాంకం flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  size_t బైట్లు, rsize, vfp_కొలత;
  అక్షరం *దొంతర, *vfp_space, *new_rvalue;
  నిర్మాణం call_చట్రం *చట్రం;

  rsize = 0;
  ఒకవేళ (rvalue == ఖాళీ)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      ఒకవేళ (flags == ARM_TYPE_STRUCT)
	rsize = rtype->కొలత;
      లేకపోతే
	flags = ARM_TYPE_VOID;
    }
  లేకపోతే ఒకవేళ (flags == ARM_TYPE_VFP_N)
    {
      /* Largest case is double x 4. */
      rsize = 32;
    }
  లేకపోతే ఒకవేళ (flags == ARM_TYPE_INT && rtype->type == FFI_TYPE_STRUCT)
    rsize = 4;

  /* Largest case.  */
  vfp_కొలత = (cif->abi == FFI_VFP && cif->vfp_వాడిన ? 8*8: 0);

  బైట్లు = cif->బైట్లు;
  దొంతర = alloca (vfp_కొలత + బైట్లు + పరిమాణం(నిర్మాణం call_చట్రం) + rsize);

  vfp_space = ఖాళీ;
  ఒకవేళ (vfp_కొలత)
    {
      vfp_space = దొంతర;
      దొంతర += vfp_కొలత;
    }

  చట్రం = (నిర్మాణం call_చట్రం *)(దొంతర + బైట్లు);

  new_rvalue = rvalue;
  ఒకవేళ (rsize)
    new_rvalue = (శూన్య *)(చట్రం + 1);

  చట్రం->rvalue = new_rvalue;
  చట్రం->flags = flags;
  చట్రం->సంవృతం = సంవృతం;

  ఒకవేళ (vfp_space)
    {
      ffi_prep_వాదనలు_VFP (cif, flags, new_rvalue, avalue, దొంతర, vfp_space);
      ffi_call_VFP (vfp_space, చట్రం, fn, cif->vfp_వాడిన);
    }
  లేకపోతే
    {
      ffi_prep_వాదనలు_SYSV (cif, flags, new_rvalue, avalue, దొంతర);
      ffi_call_SYSV (దొంతర, చట్రం, fn);
    }

  ఒకవేళ (rvalue && rvalue != new_rvalue)
    జ్ఞాపకం_నకలు (rvalue, new_rvalue, rtype->కొలత);
}

శూన్య
ffi_call (ffi_cif *cif, శూన్య (*fn) (శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ఖాళీ);
}

#నిర్వచితమైతే FFI_GO_CLOSURES
శూన్య
ffi_call_go (ffi_cif *cif, శూన్య (*fn) (శూన్య), శూన్య *rvalue,
	     శూన్య **avalue, శూన్య *సంవృతం)
{
  ffi_call_int (cif, fn, rvalue, avalue, సంవృతం);
}
#అయితే_ముగింపు

స్థిర శూన్య *
ffi_prep_incoming_వాదనలు_SYSV (ffi_cif *cif, శూన్య *rvalue,
			     అక్షరం *argp, శూన్య **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  పూర్ణాంకం i, n;

  ఒకవేళ (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(శూన్య **) argp;
      argp += 4;
    }
  లేకపోతే
    {
      ఒకవేళ (cif->rtype->కొలత && cif->rtype->కొలత < 4)
        *(uint32_t *) rvalue = 0;
    }

  కొరకు (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->కొలత;

      argp = ffi_align (ty, argp);
#నిర్వచితమైతే __ARMEB__
      ఒకవేళ (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (శూన్య *) (argp + 4 - z);
      లేకపోతే
#అయితే_ముగింపు
      avalue[i] = (శూన్య *) argp;
      argp += z;
    }

  తిరిగి rvalue;
}

స్థిర శూన్య *
ffi_prep_incoming_వాదనలు_VFP (ffi_cif *cif, శూన్య *rvalue, అక్షరం *దొంతర,
			    అక్షరం *vfp_space, శూన్య **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  పూర్ణాంకం i, n, vi = 0;
  అక్షరం *argp, *regp, *eo_regp;
  అక్షరం పూర్తి_with_regs = 0;
  అక్షరం దొంతర_వాడిన = 0;

  regp = దొంతర;
  eo_regp = argp = regp + 16;

  ఒకవేళ (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(శూన్య **) regp;
      regp += 4;
    }

  కొరకు (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      పూర్ణాంకం is_vfp_type = vfp_type_p (ty);
      size_t z = ty->కొలత;

      ఒకవేళ (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_వాదనలు[vi++] * 4;
	  కొనసాగు;
	}
      లేకపోతే ఒకవేళ (!పూర్తి_with_regs && !is_vfp_type)
	{
	  అక్షరం *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  ఒకవేళ (tregp + z <= eo_regp || !దొంతర_వాడిన)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#నిర్వచితమైతే __ARMEB__
	      ఒకవేళ (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (శూన్య *) (tregp + 4 - z);
	      లేకపోతే
#అయితే_ముగింపు
	      avalue[i] = (శూన్య *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      ఒకవేళ (regp > eo_regp)
		{
		  FFI_ASSERT (!దొంతర_వాడిన);
		  argp = regp;
		}
	      ఒకవేళ (regp >= eo_regp)
		{
		  పూర్తి_with_regs = 1;
		  దొంతర_వాడిన = 1;
		}
	      కొనసాగు;
	    }
	}

      దొంతర_వాడిన = 1;
      argp = ffi_align (ty, argp);
#నిర్వచితమైతే __ARMEB__
      ఒకవేళ (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (శూన్య *) (argp + 4 - z);
      లేకపోతే
#అయితే_ముగింపు
      avalue[i] = (శూన్య *) argp;
      argp += z;
    }

  తిరిగి rvalue;
}

#ఒకవేళ FFI_CLOSURES

నిర్మాణం సంవృతం_చట్రం
{
  అక్షరం vfp_space[8*8] __attribute__((aligned(8)));
  అక్షరం ఫలితం[8*4];
  అక్షరం argp[];
};

పూర్ణాంకం FFI_HIDDEN
ffi_సంవృతం_inner_SYSV (ffi_cif *cif,
		        శూన్య (*fun) (ffi_cif *, శూన్య *, శూన్య **, శూన్య *),
		        శూన్య *user_దత్తాంశం,
		        నిర్మాణం సంవృతం_చట్రం *చట్రం)
{
  శూన్య **avalue = (శూన్య **) alloca (cif->nargs * పరిమాణం (శూన్య *));
  శూన్య *rvalue = ffi_prep_incoming_వాదనలు_SYSV (cif, చట్రం->ఫలితం,
					      చట్రం->argp, avalue);
  fun (cif, rvalue, avalue, user_దత్తాంశం);
  తిరిగి cif->flags;
}

పూర్ణాంకం FFI_HIDDEN
ffi_సంవృతం_inner_VFP (ffi_cif *cif,
		       శూన్య (*fun) (ffi_cif *, శూన్య *, శూన్య **, శూన్య *),
		       శూన్య *user_దత్తాంశం,
		       నిర్మాణం సంవృతం_చట్రం *చట్రం)
{
  శూన్య **avalue = (శూన్య **) alloca (cif->nargs * పరిమాణం (శూన్య *));
  శూన్య *rvalue = ffi_prep_incoming_వాదనలు_VFP (cif, చట్రం->ఫలితం, చట్రం->argp,
					     చట్రం->vfp_space, avalue);
  fun (cif, rvalue, avalue, user_దత్తాంశం);
  తిరిగి cif->flags;
}

శూన్య ffi_సంవృతం_SYSV (శూన్య) FFI_HIDDEN;
శూన్య ffi_సంవృతం_VFP (శూన్య) FFI_HIDDEN;
#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
శూన్య ffi_సంవృతం_SYSV_alt (శూన్య) FFI_HIDDEN;
శూన్య ffi_సంవృతం_VFP_alt (శూన్య) FFI_HIDDEN;
#అయితే_ముగింపు

#నిర్వచితమైతే FFI_GO_CLOSURES
శూన్య ffi_go_సంవృతం_SYSV (శూన్య) FFI_HIDDEN;
శూన్య ffi_go_సంవృతం_VFP (శూన్య) FFI_HIDDEN;
#అయితే_ముగింపు

/* the cif must already be prep'ed */

#ఒకవేళ defined(__FreeBSD__) && defined(__arm__)
#నిర్వచించు __clear_cache(ప్రారంభం, చివర) చేయి { \
		నిర్మాణం arm_sync_icache_వాదనలు ua; 		\
								\
		ua.addr = (uintptr_t)(ప్రారంభం);			\
		ua.len = (అక్షరం *)(చివర) - (అక్షరం *)ప్రారంభం;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} అయితే (0);
#అయితే_ముగింపు

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం * సంవృతం,
		      ffi_cif * cif,
		      శూన్య (*fun) (ffi_cif *, శూన్య *, శూన్య **, శూన్య *),
		      శూన్య *user_దత్తాంశం, శూన్య *codeloc)
{
  శూన్య (*సంవృతం_func) (శూన్య) = ffi_సంవృతం_SYSV;

  ఒకవేళ (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      ఒకవేళ (cif->vfp_వాడిన)
	సంవృతం_func = ffi_సంవృతం_VFP;
    }
  లేకపోతే ఒకవేళ (cif->abi != FFI_SYSV)
    తిరిగి FFI_BAD_ABI;

#ఒకవేళ FFI_EXEC_TRAMPOLINE_పట్టిక
  శూన్య **config = (శూన్య **)((uint8_t *)codeloc - పుట_MAX_కొలత);
  config[0] = సంవృతం;
  config[1] = సంవృతం_func;
#లేకపోతే

# ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
  ఒకవేళ (ffi_tramp_is_present(సంవృతం))
    {
      /* Initialize the static trampoline's parameters. */
      ఒకవేళ (సంవృతం_func == ffi_సంవృతం_SYSV)
        సంవృతం_func = ffi_సంవృతం_SYSV_alt;
      లేకపోతే
        సంవృతం_func = ffi_సంవృతం_VFP_alt;
      ffi_tramp_సమితి_parms (సంవృతం->ftramp, సంవృతం_func, సంవృతం);
      వెళ్ళు బహిర్గతం;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  జ్ఞాపకం_నకలు(సంవృతం->tramp, ffi_arm_trampoline, 8);
# లేకపోతే
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  జ్ఞాపకం_నకలు(సంవృతం->tramp, (శూన్య*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_సంవృతం_స్థానభ్రంశం);
# endif

# ఒకవేళ defined(__QNX__)
  msync (సంవృతం->tramp, 8, MS_INVALIDATE_ICACHE);	/* clear data map */
  msync (codeloc, 8, MS_INVALIDATE_ICACHE);		/* clear insn map */
# elif defined(_WIN32)
  FlushInstructionCache(GetCurrentProcess(), సంవృతం->tramp, FFI_TRAMPOLINE_కొలత);
# లేకపోతే
  __clear_cache(సంవృతం->tramp, సంవృతం->tramp + 8);	/* clear data map */
  __clear_cache(codeloc, codeloc + 8);			/* clear insn map */
# endif
# ifdef _WIN32
  *(శూన్య(**)(శూన్య))(సంవృతం->tramp + FFI_TRAMPOLINE_సంవృతం_FUNCTION) = సంవృతం_func;
# లేకపోతే
  *(శూన్య (**)(శూన్య))(సంవృతం->tramp + 8) = సంవృతం_func;
# endif
# ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
బహిర్గతం:
# endif
#అయితే_ముగింపు

  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;

  తిరిగి FFI_OK;
}

#నిర్వచితమైతే FFI_GO_CLOSURES
ffi_స్థితి
ffi_prep_go_సంవృతం (ffi_go_సంవృతం *సంవృతం, ffi_cif *cif,
		     శూన్య (*fun) (ffi_cif *, శూన్య *, శూన్య **, శూన్య *))
{
  శూన్య (*సంవృతం_func) (శూన్య) = ffi_go_సంవృతం_SYSV;

  ఒకవేళ (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      ఒకవేళ (cif->vfp_వాడిన)
	సంవృతం_func = ffi_go_సంవృతం_VFP;
    }
  లేకపోతే ఒకవేళ (cif->abi != FFI_SYSV)
    తిరిగి FFI_BAD_ABI;

  సంవృతం->tramp = సంవృతం_func;
  సంవృతం->cif = cif;
  సంవృతం->fun = fun;

  తిరిగి FFI_OK;
}
#అయితే_ముగింపు

#అయితే_ముగింపు /* FFI_CLOSURES */

/* Below are routines for VFP hard-float support. */

/* A subroutine of vfp_type_p.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

స్థిర పూర్ణాంకం
is_hfa0 (స్థిరరాశి ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  పూర్ణాంకం i, ret = -1;

  ఒకవేళ (elements != ఖాళీ)
    కొరకు (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        ఒకవేళ (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            ఒకవేళ (ret < 0)
              కొనసాగు;
          }
        ఆపు;
      }

  తిరిగి ret;
}

/* A subroutine of vfp_type_p.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

స్థిర పూర్ణాంకం
is_hfa1 (స్థిరరాశి ffi_type *ty, పూర్ణాంకం candidate)
{
  ffi_type **elements = ty->elements;
  పూర్ణాంకం i;

  ఒకవేళ (elements != ఖాళీ)
    కొరకు (i = 0; elements[i]; ++i)
      {
        పూర్ణాంకం t = elements[i]->type;
        ఒకవేళ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
          {
            ఒకవేళ (!is_hfa1 (elements[i], candidate))
              తిరిగి 0;
          }
        లేకపోతే ఒకవేళ (t != candidate)
          తిరిగి 0;
      }

  తిరిగి 1;
}

/* Determine if TY is an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is a floating point scalar.

   Returns non-zero iff TY is an HFA.  The result is an encoded value where
   bits 0-7 contain the type code, and bits 8-10 contain the element count.  */

స్థిర పూర్ణాంకం
vfp_type_p (స్థిరరాశి ffi_type *ty)
{
  ffi_type **elements;
  పూర్ణాంకం candidate, i;
  size_t కొలత, ele_లెక్క;

  /* Quickest tests first.  */
  candidate = ty->type;
  ఎంపిక (ty->type)
    {
    అప్రమేయ:
      తిరిగి 0;
    స్థితి FFI_TYPE_FLOAT:
    స్థితి FFI_TYPE_DOUBLE:
      ele_లెక్క = 1;
      వెళ్ళు పూర్తి;
    స్థితి FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      ఒకవేళ (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	తిరిగి 0;
      ele_లెక్క = 2;
      వెళ్ళు పూర్తి;
    స్థితి FFI_TYPE_STRUCT:
      ఆపు;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  కొలత = ty->కొలత;
  ఒకవేళ (కొలత < 4 || కొలత > 32)
    తిరిగి 0;

  /* Find the type of the first non-structure member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  ఒకవేళ (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_COMPLEX)
    {
      కొరకు (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          ఒకవేళ (candidate >= 0)
            ఆపు;
        }
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  ఎంపిక (candidate)
    {
    స్థితి FFI_TYPE_FLOAT:
      ele_లెక్క = కొలత / పరిమాణం(దశాంశం);
      ఒకవేళ (కొలత != ele_లెక్క * పరిమాణం(దశాంశం))
        తిరిగి 0;
      ఆపు;
    స్థితి FFI_TYPE_DOUBLE:
      ele_లెక్క = కొలత / పరిమాణం(ద్విగుణ);
      ఒకవేళ (కొలత != ele_లెక్క * పరిమాణం(ద్విగుణ))
        తిరిగి 0;
      ఆపు;
    అప్రమేయ:
      తిరిగి 0;
    }
  ఒకవేళ (ele_లెక్క > 4)
    తిరిగి 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  కొరకు (i = 0; elements[i]; ++i)
    {
      పూర్ణాంకం t = elements[i]->type;
      ఒకవేళ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          ఒకవేళ (!is_hfa1 (elements[i], candidate))
            తిరిగి 0;
        }
      లేకపోతే ఒకవేళ (t != candidate)
        తిరిగి 0;
    }

  /* All tests succeeded.  Encode the result.  */
 పూర్తి:
  తిరిగి (ele_లెక్క << 8) | candidate;
}

స్థిర పూర్ణాంకం
ఉంచు_vfp_arg (ffi_cif *cif, పూర్ణాంకం h)
{
  గుర్తులేని పొట్టి reg = cif->vfp_reg_free;
  పూర్ణాంకం align = 1, nregs = h >> 8;

  ఒకవేళ ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  ఒకవేళ ((reg & 1) && align == 2)
    reg++;

  అయితే (reg + nregs <= 16)
    {
      పూర్ణాంకం s, new_వాడిన = 0;
      కొరకు (s = reg; s < reg + nregs; s++)
	{
	  new_వాడిన |= (1 << s);
	  ఒకవేళ (cif->vfp_వాడిన & (1 << s))
	    {
	      reg += align;
	      వెళ్ళు తదుపరి_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_వాడిన |= new_వాడిన;
      cif->vfp_వాదనలు[cif->vfp_nargs++] = (గుర్తుగల అక్షరం)reg;

      /* Update vfp_reg_free. */
      ఒకవేళ (cif->vfp_వాడిన & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  అయితే (cif->vfp_వాడిన & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      తిరిగి 0;
    తదుపరి_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_వాడిన = 0xFFFF;
  తిరిగి 1;
}

స్థిర శూన్య
layout_vfp_వాదనలు (ffi_cif * cif)
{
  గుర్తులేని పూర్ణాంకం i;
  /* Init VFP fields */
  cif->vfp_వాడిన = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  memset (cif->vfp_వాదనలు, -1, 16);	/* Init to -1. */

  కొరకు (i = 0; i < cif->nargs; i++)
    {
      పూర్ణాంకం h = vfp_type_p (cif->arg_types[i]);
      ఒకవేళ (h && ఉంచు_vfp_arg (cif, h) == 1)
	ఆపు;
    }
}

#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
శూన్య *
ffi_tramp_arch (size_t *tramp_కొలత, size_t *map_కొలత)
{
  బాహ్య శూన్య *trampoline_సంకేతం_పట్టిక;

  *tramp_కొలత = ARM_TRAMP_కొలత;
  *map_కొలత = ARM_TRAMP_MAP_కొలత;
  తిరిగి &trampoline_సంకేతం_పట్టిక;
}
#అయితే_ముగింపు

#అయితే_ముగింపు /* __arm__ or _M_ARM */
