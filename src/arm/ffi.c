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

#যদি defined(__arm__) || defined(_M_ARM)
#অন্তর্ভাব <fficonfig.h>
#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <tramp.h>
#অন্তর্ভাব "internal.h"

#যদি defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#অন্তর্ভাব <windows.h>
#endif

#যদি FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#অন্তর্ভাব <mach/machine/vm_param.h>
#endif

#অন্যথা
#ifndef _WIN32
বাহ্য unsigned পূর্ণাঙ্ক ffi_arm_trampoline[2] FFI_HIDDEN;
#অন্যথা
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
বাহ্য unsigned বর্ণ ffi_arm_trampoline[12] FFI_HIDDEN;
#endif
#endif

#যদি defined(__FreeBSD__) && defined(__arm__)
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <machine/sysarch.h>
#endif

#যদি defined(__QNX__)
#অন্তর্ভাব <sys/mman.h>
#endif

/* Forward declares. */
স্থির পূর্ণাঙ্ক vfp_type_p (ধ্রুব ffi_type *);
স্থির শূন্য layout_vfp_args (ffi_cif *);

স্থির শূন্য *
ffi_align (ffi_type *ty, শূন্য *p)
{
  /* Align if necessary */
  size_t alignment;
#ifdef _WIN32_WCE
  alignment = 4;
#অন্যথা
  alignment = ty->alignment;
  যদি (alignment < 4)
    alignment = 4;
#endif
  প্রত্যাগম (শূন্য *) FFI_ALIGN (p, alignment);
}

স্থির size_t
ffi_put_arg (ffi_type *ty, শূন্য *src, শূন্য *dst)
{
  size_t z = ty->size;

  বিকল্প (ty->type)
    {
    পক্ষ FFI_TYPE_SINT8:
      *(UINT32 *)dst = *(SINT8 *)src;
      ভঙ্গ;
    পক্ষ FFI_TYPE_UINT8:
      *(UINT32 *)dst = *(UINT8 *)src;
      ভঙ্গ;
    পক্ষ FFI_TYPE_SINT16:
      *(UINT32 *)dst = *(SINT16 *)src;
      ভঙ্গ;
    পক্ষ FFI_TYPE_UINT16:
      *(UINT32 *)dst = *(UINT16 *)src;
      ভঙ্গ;

    পক্ষ FFI_TYPE_INT:
    পক্ষ FFI_TYPE_SINT32:
    পক্ষ FFI_TYPE_UINT32:
    পক্ষ FFI_TYPE_POINTER:
#ifndef _WIN32
    পক্ষ FFI_TYPE_FLOAT:
#endif
      *(UINT32 *)dst = *(UINT32 *)src;
      ভঙ্গ;

#ifdef _WIN32
    // casting a float* to a UINT32* doesn't work on Windows
    পক্ষ FFI_TYPE_FLOAT:
        *(uintptr_t *)dst = 0;
        *(ভিন্ন *)dst = *(ভিন্ন *)src;
        ভঙ্গ;
#endif

    পক্ষ FFI_TYPE_SINT64:
    পক্ষ FFI_TYPE_UINT64:
    পক্ষ FFI_TYPE_DOUBLE:
      *(UINT64 *)dst = *(UINT64 *)src;
      ভঙ্গ;

    পক্ষ FFI_TYPE_STRUCT:
    পক্ষ FFI_TYPE_COMPLEX:
      memcpy (dst, src, z);
      ভঙ্গ;

    স্বভাব:
      abort();
    }

  প্রত্যাগম FFI_ALIGN (z, 4);
}

/* ffi_prep_args is called once stack space has been allocated
   for the function's arguments.

   The vfp_space parameter is the load area for VFP regs, the return
   value is cif->vfp_used (word bitset of VFP regs used for passing
   arguments). These are only used for the VFP hard-float ABI.
*/
স্থির শূন্য
ffi_prep_args_SYSV (ffi_cif *cif, পূর্ণাঙ্ক flags, শূন্য *rvalue,
		    শূন্য **avalue, বর্ণ *argp)
{
  ffi_type **arg_types = cif->arg_types;
  পূর্ণাঙ্ক i, n;

  যদি (flags == ARM_TYPE_STRUCT)
    {
      *(শূন্য **) argp = rvalue;
      argp += 4;
    }

  প্রতি (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, avalue[i], argp);
    }
}

স্থির শূন্য
ffi_prep_args_VFP (ffi_cif *cif, পূর্ণাঙ্ক flags, শূন্য *rvalue,
                   শূন্য **avalue, বর্ণ *স্তূপ, বর্ণ *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  পূর্ণাঙ্ক i, n, vi = 0;
  বর্ণ *argp, *regp, *eo_regp;
  বর্ণ স্তূপ_used = 0;
  বর্ণ done_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = স্তূপ;
  eo_regp = argp = regp + 16;

  /* If the function returns an FFI_TYPE_STRUCT in memory,
     that address is passed in r0 to the function.  */
  যদি (flags == ARM_TYPE_STRUCT)
    {
      *(শূন্য **) regp = rvalue;
      regp += 4;
    }

  প্রতি (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      শূন্য *a = avalue[i];
      পূর্ণাঙ্ক is_vfp_type = vfp_type_p (ty);

      /* Allocated in VFP registers. */
      যদি (vi < cif->vfp_nargs && is_vfp_type)
	{
	  বর্ণ *vfp_slot = vfp_space + cif->vfp_args[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_slot);
	  অনুবর্ত;
	}
      /* Try allocating in core registers. */
      অন্যথা যদি (!done_with_regs && !is_vfp_type)
	{
	  বর্ণ *tregp = ffi_align (ty, regp);
	  size_t size = ty->size;
	  size = (size < 4) ? 4 : size;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  যদি (tregp + size <= eo_regp)
	    {
	      regp = tregp + ffi_put_arg (ty, a, tregp);
	      done_with_regs = (regp == argp);
	      // ensure we did not write into the stack area
	      FFI_ASSERT (regp <= argp);
	      অনুবর্ত;
	    }
	  /* In case there are no arguments in the stack area yet,
	     the argument is passed in the remaining core registers
	     and on the stack.  */
	  অন্যথা যদি (!স্তূপ_used)
	    {
	      স্তূপ_used = 1;
	      done_with_regs = 1;
	      argp = tregp + ffi_put_arg (ty, a, tregp);
	      FFI_ASSERT (eo_regp < argp);
	      অনুবর্ত;
	    }
	}
      /* Base case, arguments are passed on the stack */
      স্তূপ_used = 1;
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, a, argp);
    }
}

/* Perform machine dependent cif processing */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  পূর্ণাঙ্ক flags = 0, cabi = cif->abi;
  size_t bytes = cif->bytes;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  যদি (cabi == FFI_VFP)
    layout_vfp_args (cif);

  /* Set the return type flag */
  বিকল্প (cif->rtype->type)
    {
    পক্ষ FFI_TYPE_VOID:
      flags = ARM_TYPE_VOID;
      ভঙ্গ;

    পক্ষ FFI_TYPE_INT:
    পক্ষ FFI_TYPE_UINT8:
    পক্ষ FFI_TYPE_SINT8:
    পক্ষ FFI_TYPE_UINT16:
    পক্ষ FFI_TYPE_SINT16:
    পক্ষ FFI_TYPE_UINT32:
    পক্ষ FFI_TYPE_SINT32:
    পক্ষ FFI_TYPE_POINTER:
      flags = ARM_TYPE_INT;
      ভঙ্গ;

    পক্ষ FFI_TYPE_SINT64:
    পক্ষ FFI_TYPE_UINT64:
      flags = ARM_TYPE_INT64;
      ভঙ্গ;

    পক্ষ FFI_TYPE_FLOAT:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_S : ARM_TYPE_INT);
      ভঙ্গ;
    পক্ষ FFI_TYPE_DOUBLE:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_D : ARM_TYPE_INT64);
      ভঙ্গ;

    পক্ষ FFI_TYPE_STRUCT:
    পক্ষ FFI_TYPE_COMPLEX:
      যদি (cabi == FFI_VFP)
	{
	  পূর্ণাঙ্ক h = vfp_type_p (cif->rtype);

	  flags = ARM_TYPE_VFP_N;
	  যদি (h == 0x100 + FFI_TYPE_FLOAT)
	    flags = ARM_TYPE_VFP_S;
	  যদি (h == 0x100 + FFI_TYPE_DOUBLE)
	    flags = ARM_TYPE_VFP_D;
	  যদি (h != 0)
	      ভঙ্গ;
	}

      /* A Composite Type not larger than 4 bytes is returned in r0.
	 A Composite Type larger than 4 bytes, or whose size cannot
	 be determined statically ... is stored in memory at an
	 address passed [in r0].  */
      যদি (cif->rtype->size <= 4)
	flags = ARM_TYPE_INT;
      অন্যথা
	{
	  flags = ARM_TYPE_STRUCT;
	  bytes += 4;
	}
      ভঙ্গ;

    স্বভাব:
      abort();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  bytes = FFI_ALIGN (bytes, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  যদি (bytes < 4*4)
    bytes = 4*4;

  cif->bytes = bytes;
  cif->flags = flags;

  প্রত্যাগম FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  unsigned পূর্ণাঙ্ক nfixedargs, unsigned পূর্ণাঙ্ক ntotalargs)
{
  /* VFP variadic calls actually use the SYSV ABI */
  যদি (cif->abi == FFI_VFP)
    cif->abi = FFI_SYSV;

  প্রত্যাগম ffi_prep_cif_machdep (cif);
}

/* Prototypes for assembly functions, in sysv.S.  */

সংরচনা call_frame
{
  শূন্য *fp;
  শূন্য *lr;
  শূন্য *rvalue;
  পূর্ণাঙ্ক flags;
  শূন্য *closure;
};

বাহ্য শূন্য ffi_call_SYSV (শূন্য *স্তূপ, সংরচনা call_frame *,
			   শূন্য (*fn) (শূন্য)) FFI_HIDDEN;
বাহ্য শূন্য ffi_call_VFP (শূন্য *vfp_space, সংরচনা call_frame *,
			   শূন্য (*fn) (শূন্য), unsigned vfp_used) FFI_HIDDEN;

স্থির শূন্য
ffi_call_int (ffi_cif * cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue,
	      শূন্য **avalue, শূন্য *closure)
{
  পূর্ণাঙ্ক flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  size_t bytes, rsize, vfp_size;
  বর্ণ *স্তূপ, *vfp_space, *new_rvalue;
  সংরচনা call_frame *frame;

  rsize = 0;
  যদি (rvalue == রিক্ত)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      যদি (flags == ARM_TYPE_STRUCT)
	rsize = rtype->size;
      অন্যথা
	flags = ARM_TYPE_VOID;
    }
  অন্যথা যদি (flags == ARM_TYPE_VFP_N)
    {
      /* Largest case is double x 4. */
      rsize = 32;
    }
  অন্যথা যদি (flags == ARM_TYPE_INT && rtype->type == FFI_TYPE_STRUCT)
    rsize = 4;

  /* Largest case.  */
  vfp_size = (cif->abi == FFI_VFP && cif->vfp_used ? 8*8: 0);

  bytes = cif->bytes;
  স্তূপ = alloca (vfp_size + bytes + পরিমাণ(সংরচনা call_frame) + rsize);

  vfp_space = রিক্ত;
  যদি (vfp_size)
    {
      vfp_space = স্তূপ;
      স্তূপ += vfp_size;
    }

  frame = (সংরচনা call_frame *)(স্তূপ + bytes);

  new_rvalue = rvalue;
  যদি (rsize)
    new_rvalue = (শূন্য *)(frame + 1);

  frame->rvalue = new_rvalue;
  frame->flags = flags;
  frame->closure = closure;

  যদি (vfp_space)
    {
      ffi_prep_args_VFP (cif, flags, new_rvalue, avalue, স্তূপ, vfp_space);
      ffi_call_VFP (vfp_space, frame, fn, cif->vfp_used);
    }
  অন্যথা
    {
      ffi_prep_args_SYSV (cif, flags, new_rvalue, avalue, স্তূপ);
      ffi_call_SYSV (স্তূপ, frame, fn);
    }

  যদি (rvalue && rvalue != new_rvalue)
    memcpy (rvalue, new_rvalue, rtype->size);
}

শূন্য
ffi_call (ffi_cif *cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, রিক্ত);
}

#ifdef FFI_GO_CLOSURES
শূন্য
ffi_call_go (ffi_cif *cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue,
	     শূন্য **avalue, শূন্য *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif

স্থির শূন্য *
ffi_prep_incoming_args_SYSV (ffi_cif *cif, শূন্য *rvalue,
			     বর্ণ *argp, শূন্য **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  পূর্ণাঙ্ক i, n;

  যদি (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(শূন্য **) argp;
      argp += 4;
    }
  অন্যথা
    {
      যদি (cif->rtype->size && cif->rtype->size < 4)
        *(uint32_t *) rvalue = 0;
    }

  প্রতি (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;

      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      যদি (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (শূন্য *) (argp + 4 - z);
      অন্যথা
#endif
      avalue[i] = (শূন্য *) argp;
      argp += z;
    }

  প্রত্যাগম rvalue;
}

স্থির শূন্য *
ffi_prep_incoming_args_VFP (ffi_cif *cif, শূন্য *rvalue, বর্ণ *স্তূপ,
			    বর্ণ *vfp_space, শূন্য **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  পূর্ণাঙ্ক i, n, vi = 0;
  বর্ণ *argp, *regp, *eo_regp;
  বর্ণ done_with_regs = 0;
  বর্ণ স্তূপ_used = 0;

  regp = স্তূপ;
  eo_regp = argp = regp + 16;

  যদি (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(শূন্য **) regp;
      regp += 4;
    }

  প্রতি (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      পূর্ণাঙ্ক is_vfp_type = vfp_type_p (ty);
      size_t z = ty->size;

      যদি (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_args[vi++] * 4;
	  অনুবর্ত;
	}
      অন্যথা যদি (!done_with_regs && !is_vfp_type)
	{
	  বর্ণ *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  যদি (tregp + z <= eo_regp || !স্তূপ_used)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#ifdef __ARMEB__
	      যদি (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (শূন্য *) (tregp + 4 - z);
	      অন্যথা
#endif
	      avalue[i] = (শূন্য *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      যদি (regp > eo_regp)
		{
		  FFI_ASSERT (!স্তূপ_used);
		  argp = regp;
		}
	      যদি (regp >= eo_regp)
		{
		  done_with_regs = 1;
		  স্তূপ_used = 1;
		}
	      অনুবর্ত;
	    }
	}

      স্তূপ_used = 1;
      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      যদি (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (শূন্য *) (argp + 4 - z);
      অন্যথা
#endif
      avalue[i] = (শূন্য *) argp;
      argp += z;
    }

  প্রত্যাগম rvalue;
}

#যদি FFI_CLOSURES

সংরচনা closure_frame
{
  বর্ণ vfp_space[8*8] __attribute__((aligned(8)));
  বর্ণ result[8*4];
  বর্ণ argp[];
};

পূর্ণাঙ্ক FFI_HIDDEN
ffi_closure_inner_SYSV (ffi_cif *cif,
		        শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *),
		        শূন্য *user_data,
		        সংরচনা closure_frame *frame)
{
  শূন্য **avalue = (শূন্য **) alloca (cif->nargs * পরিমাণ (শূন্য *));
  শূন্য *rvalue = ffi_prep_incoming_args_SYSV (cif, frame->result,
					      frame->argp, avalue);
  fun (cif, rvalue, avalue, user_data);
  প্রত্যাগম cif->flags;
}

পূর্ণাঙ্ক FFI_HIDDEN
ffi_closure_inner_VFP (ffi_cif *cif,
		       শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *),
		       শূন্য *user_data,
		       সংরচনা closure_frame *frame)
{
  শূন্য **avalue = (শূন্য **) alloca (cif->nargs * পরিমাণ (শূন্য *));
  শূন্য *rvalue = ffi_prep_incoming_args_VFP (cif, frame->result, frame->argp,
					     frame->vfp_space, avalue);
  fun (cif, rvalue, avalue, user_data);
  প্রত্যাগম cif->flags;
}

শূন্য ffi_closure_SYSV (শূন্য) FFI_HIDDEN;
শূন্য ffi_closure_VFP (শূন্য) FFI_HIDDEN;
#যদি defined(FFI_EXEC_STATIC_TRAMP)
শূন্য ffi_closure_SYSV_alt (শূন্য) FFI_HIDDEN;
শূন্য ffi_closure_VFP_alt (শূন্য) FFI_HIDDEN;
#endif

#ifdef FFI_GO_CLOSURES
শূন্য ffi_go_closure_SYSV (শূন্য) FFI_HIDDEN;
শূন্য ffi_go_closure_VFP (শূন্য) FFI_HIDDEN;
#endif

/* the cif must already be prep'ed */

#যদি defined(__FreeBSD__) && defined(__arm__)
#define __clear_cache(start, end) কুরু { \
		সংরচনা arm_sync_icache_args ua; 		\
								\
		ua.addr = (uintptr_t)(start);			\
		ua.len = (বর্ণ *)(end) - (বর্ণ *)start;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} যাবত্ (0);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure * closure,
		      ffi_cif * cif,
		      শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *),
		      শূন্য *user_data, শূন্য *codeloc)
{
  শূন্য (*closure_func) (শূন্য) = ffi_closure_SYSV;

  যদি (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      যদি (cif->vfp_used)
	closure_func = ffi_closure_VFP;
    }
  অন্যথা যদি (cif->abi != FFI_SYSV)
    প্রত্যাগম FFI_BAD_ABI;

#যদি FFI_EXEC_TRAMPOLINE_TABLE
  শূন্য **config = (শূন্য **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = closure_func;
#অন্যথা

# যদি defined(FFI_EXEC_STATIC_TRAMP)
  যদি (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      যদি (closure_func == ffi_closure_SYSV)
        closure_func = ffi_closure_SYSV_alt;
      অন্যথা
        closure_func = ffi_closure_VFP_alt;
      ffi_tramp_set_parms (closure->ftramp, closure_func, closure);
      গচ্ছ out;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  memcpy(closure->tramp, ffi_arm_trampoline, 8);
# অন্যথা
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  memcpy(closure->tramp, (শূন্য*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_CLOSURE_OFFSET);
# endif

# যদি defined(__QNX__)
  msync (closure->tramp, 8, MS_INVALIDATE_ICACHE);	/* clear data map */
  msync (codeloc, 8, MS_INVALIDATE_ICACHE);		/* clear insn map */
# elif defined(_WIN32)
  FlushInstructionCache(GetCurrentProcess(), closure->tramp, FFI_TRAMPOLINE_SIZE);
# অন্যথা
  __clear_cache(closure->tramp, closure->tramp + 8);	/* clear data map */
  __clear_cache(codeloc, codeloc + 8);			/* clear insn map */
# endif
# ifdef _WIN32
  *(শূন্য(**)(শূন্য))(closure->tramp + FFI_TRAMPOLINE_CLOSURE_FUNCTION) = closure_func;
# অন্যথা
  *(শূন্য (**)(শূন্য))(closure->tramp + 8) = closure_func;
# endif
# যদি defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  প্রত্যাগম FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *))
{
  শূন্য (*closure_func) (শূন্য) = ffi_go_closure_SYSV;

  যদি (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      যদি (cif->vfp_used)
	closure_func = ffi_go_closure_VFP;
    }
  অন্যথা যদি (cif->abi != FFI_SYSV)
    প্রত্যাগম FFI_BAD_ABI;

  closure->tramp = closure_func;
  closure->cif = cif;
  closure->fun = fun;

  প্রত্যাগম FFI_OK;
}
#endif

#endif /* FFI_CLOSURES */

/* Below are routines for VFP hard-float support. */

/* A subroutine of vfp_type_p.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

স্থির পূর্ণাঙ্ক
is_hfa0 (ধ্রুব ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  পূর্ণাঙ্ক i, ret = -1;

  যদি (elements != রিক্ত)
    প্রতি (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        যদি (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            যদি (ret < 0)
              অনুবর্ত;
          }
        ভঙ্গ;
      }

  প্রত্যাগম ret;
}

/* A subroutine of vfp_type_p.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

স্থির পূর্ণাঙ্ক
is_hfa1 (ধ্রুব ffi_type *ty, পূর্ণাঙ্ক candidate)
{
  ffi_type **elements = ty->elements;
  পূর্ণাঙ্ক i;

  যদি (elements != রিক্ত)
    প্রতি (i = 0; elements[i]; ++i)
      {
        পূর্ণাঙ্ক t = elements[i]->type;
        যদি (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
          {
            যদি (!is_hfa1 (elements[i], candidate))
              প্রত্যাগম 0;
          }
        অন্যথা যদি (t != candidate)
          প্রত্যাগম 0;
      }

  প্রত্যাগম 1;
}

/* Determine if TY is an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is a floating point scalar.

   Returns non-zero iff TY is an HFA.  The result is an encoded value where
   bits 0-7 contain the type code, and bits 8-10 contain the element count.  */

স্থির পূর্ণাঙ্ক
vfp_type_p (ধ্রুব ffi_type *ty)
{
  ffi_type **elements;
  পূর্ণাঙ্ক candidate, i;
  size_t size, ele_গুনতি;

  /* Quickest tests first.  */
  candidate = ty->type;
  বিকল্প (ty->type)
    {
    স্বভাব:
      প্রত্যাগম 0;
    পক্ষ FFI_TYPE_FLOAT:
    পক্ষ FFI_TYPE_DOUBLE:
      ele_গুনতি = 1;
      গচ্ছ done;
    পক্ষ FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      যদি (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	প্রত্যাগম 0;
      ele_গুনতি = 2;
      গচ্ছ done;
    পক্ষ FFI_TYPE_STRUCT:
      ভঙ্গ;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  size = ty->size;
  যদি (size < 4 || size > 32)
    প্রত্যাগম 0;

  /* Find the type of the first non-structure member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  যদি (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_COMPLEX)
    {
      প্রতি (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          যদি (candidate >= 0)
            ভঙ্গ;
        }
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  বিকল্প (candidate)
    {
    পক্ষ FFI_TYPE_FLOAT:
      ele_গুনতি = size / পরিমাণ(ভিন্ন);
      যদি (size != ele_গুনতি * পরিমাণ(ভিন্ন))
        প্রত্যাগম 0;
      ভঙ্গ;
    পক্ষ FFI_TYPE_DOUBLE:
      ele_গুনতি = size / পরিমাণ(দ্বিগুণ);
      যদি (size != ele_গুনতি * পরিমাণ(দ্বিগুণ))
        প্রত্যাগম 0;
      ভঙ্গ;
    স্বভাব:
      প্রত্যাগম 0;
    }
  যদি (ele_গুনতি > 4)
    প্রত্যাগম 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  প্রতি (i = 0; elements[i]; ++i)
    {
      পূর্ণাঙ্ক t = elements[i]->type;
      যদি (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          যদি (!is_hfa1 (elements[i], candidate))
            প্রত্যাগম 0;
        }
      অন্যথা যদি (t != candidate)
        প্রত্যাগম 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  প্রত্যাগম (ele_গুনতি << 8) | candidate;
}

স্থির পূর্ণাঙ্ক
place_vfp_arg (ffi_cif *cif, পূর্ণাঙ্ক h)
{
  unsigned short reg = cif->vfp_reg_free;
  পূর্ণাঙ্ক align = 1, nregs = h >> 8;

  যদি ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  যদি ((reg & 1) && align == 2)
    reg++;

  যাবত্ (reg + nregs <= 16)
    {
      পূর্ণাঙ্ক s, new_used = 0;
      প্রতি (s = reg; s < reg + nregs; s++)
	{
	  new_used |= (1 << s);
	  যদি (cif->vfp_used & (1 << s))
	    {
	      reg += align;
	      গচ্ছ next_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_used |= new_used;
      cif->vfp_args[cif->vfp_nargs++] = (signed বর্ণ)reg;

      /* Update vfp_reg_free. */
      যদি (cif->vfp_used & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  যাবত্ (cif->vfp_used & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      প্রত্যাগম 0;
    next_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_used = 0xFFFF;
  প্রত্যাগম 1;
}

স্থির শূন্য
layout_vfp_args (ffi_cif * cif)
{
  unsigned পূর্ণাঙ্ক i;
  /* Init VFP fields */
  cif->vfp_used = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  memset (cif->vfp_args, -1, 16);	/* Init to -1. */

  প্রতি (i = 0; i < cif->nargs; i++)
    {
      পূর্ণাঙ্ক h = vfp_type_p (cif->arg_types[i]);
      যদি (h && place_vfp_arg (cif, h) == 1)
	ভঙ্গ;
    }
}

#যদি defined(FFI_EXEC_STATIC_TRAMP)
শূন্য *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  বাহ্য শূন্য *trampoline_code_table;

  *tramp_size = ARM_TRAMP_SIZE;
  *map_size = ARM_TRAMP_MAP_SIZE;
  প্রত্যাগম &trampoline_code_table;
}
#endif

#endif /* __arm__ or _M_ARM */
