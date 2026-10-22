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

#எனில் defined(__arm__) || defined(_M_ARM)
#அந்தர்பாவ <fficonfig.h>
#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <tramp.h>
#அந்தர்பாவ "internal.h"

#எனில் defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#அந்தர்பாவ <windows.h>
#endif

#எனில் FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#அந்தர்பாவ <mach/machine/vm_param.h>
#endif

#இல்லையெனில்
#ifndef _WIN32
வெளி unsigned முழு ffi_arm_trampoline[2] FFI_HIDDEN;
#இல்லையெனில்
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
வெளி unsigned எழுத்து ffi_arm_trampoline[12] FFI_HIDDEN;
#endif
#endif

#எனில் defined(__FreeBSD__) && defined(__arm__)
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <machine/sysarch.h>
#endif

#எனில் defined(__QNX__)
#அந்தர்பாவ <sys/mman.h>
#endif

/* Forward declares. */
நிலையான முழு vfp_type_p (மாறிலி ffi_type *);
நிலையான வெறுமை layout_vfp_args (ffi_cif *);

நிலையான வெறுமை *
ffi_align (ffi_type *ty, வெறுமை *p)
{
  /* Align if necessary */
  size_t alignment;
#ifdef _WIN32_WCE
  alignment = 4;
#இல்லையெனில்
  alignment = ty->alignment;
  எனில் (alignment < 4)
    alignment = 4;
#endif
  திரும்பு (வெறுமை *) FFI_ALIGN (p, alignment);
}

நிலையான size_t
ffi_put_arg (ffi_type *ty, வெறுமை *src, வெறுமை *dst)
{
  size_t z = ty->size;

  தேர்வு (ty->type)
    {
    நிலை FFI_TYPE_SINT8:
      *(UINT32 *)dst = *(SINT8 *)src;
      நிறுத்து;
    நிலை FFI_TYPE_UINT8:
      *(UINT32 *)dst = *(UINT8 *)src;
      நிறுத்து;
    நிலை FFI_TYPE_SINT16:
      *(UINT32 *)dst = *(SINT16 *)src;
      நிறுத்து;
    நிலை FFI_TYPE_UINT16:
      *(UINT32 *)dst = *(UINT16 *)src;
      நிறுத்து;

    நிலை FFI_TYPE_INT:
    நிலை FFI_TYPE_SINT32:
    நிலை FFI_TYPE_UINT32:
    நிலை FFI_TYPE_POINTER:
#ifndef _WIN32
    நிலை FFI_TYPE_FLOAT:
#endif
      *(UINT32 *)dst = *(UINT32 *)src;
      நிறுத்து;

#ifdef _WIN32
    // casting a float* to a UINT32* doesn't work on Windows
    நிலை FFI_TYPE_FLOAT:
        *(uintptr_t *)dst = 0;
        *(தசம *)dst = *(தசம *)src;
        நிறுத்து;
#endif

    நிலை FFI_TYPE_SINT64:
    நிலை FFI_TYPE_UINT64:
    நிலை FFI_TYPE_DOUBLE:
      *(UINT64 *)dst = *(UINT64 *)src;
      நிறுத்து;

    நிலை FFI_TYPE_STRUCT:
    நிலை FFI_TYPE_COMPLEX:
      memcpy (dst, src, z);
      நிறுத்து;

    இயல்பு:
      abort();
    }

  திரும்பு FFI_ALIGN (z, 4);
}

/* ffi_prep_args is called once stack space has been allocated
   for the function's arguments.

   The vfp_space parameter is the load area for VFP regs, the return
   value is cif->vfp_used (word bitset of VFP regs used for passing
   arguments). These are only used for the VFP hard-float ABI.
*/
நிலையான வெறுமை
ffi_prep_args_SYSV (ffi_cif *cif, முழு flags, வெறுமை *rvalue,
		    வெறுமை **avalue, எழுத்து *argp)
{
  ffi_type **arg_types = cif->arg_types;
  முழு i, n;

  எனில் (flags == ARM_TYPE_STRUCT)
    {
      *(வெறுமை **) argp = rvalue;
      argp += 4;
    }

  க்கு (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, avalue[i], argp);
    }
}

நிலையான வெறுமை
ffi_prep_args_VFP (ffi_cif *cif, முழு flags, வெறுமை *rvalue,
                   வெறுமை **avalue, எழுத்து *அடுக்கு, எழுத்து *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  முழு i, n, vi = 0;
  எழுத்து *argp, *regp, *eo_regp;
  எழுத்து அடுக்கு_used = 0;
  எழுத்து done_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = அடுக்கு;
  eo_regp = argp = regp + 16;

  /* If the function returns an FFI_TYPE_STRUCT in memory,
     that address is passed in r0 to the function.  */
  எனில் (flags == ARM_TYPE_STRUCT)
    {
      *(வெறுமை **) regp = rvalue;
      regp += 4;
    }

  க்கு (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      வெறுமை *a = avalue[i];
      முழு is_vfp_type = vfp_type_p (ty);

      /* Allocated in VFP registers. */
      எனில் (vi < cif->vfp_nargs && is_vfp_type)
	{
	  எழுத்து *vfp_slot = vfp_space + cif->vfp_args[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_slot);
	  தொடர்;
	}
      /* Try allocating in core registers. */
      இல்லையெனில் எனில் (!done_with_regs && !is_vfp_type)
	{
	  எழுத்து *tregp = ffi_align (ty, regp);
	  size_t size = ty->size;
	  size = (size < 4) ? 4 : size;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  எனில் (tregp + size <= eo_regp)
	    {
	      regp = tregp + ffi_put_arg (ty, a, tregp);
	      done_with_regs = (regp == argp);
	      // ensure we did not write into the stack area
	      FFI_ASSERT (regp <= argp);
	      தொடர்;
	    }
	  /* In case there are no arguments in the stack area yet,
	     the argument is passed in the remaining core registers
	     and on the stack.  */
	  இல்லையெனில் எனில் (!அடுக்கு_used)
	    {
	      அடுக்கு_used = 1;
	      done_with_regs = 1;
	      argp = tregp + ffi_put_arg (ty, a, tregp);
	      FFI_ASSERT (eo_regp < argp);
	      தொடர்;
	    }
	}
      /* Base case, arguments are passed on the stack */
      அடுக்கு_used = 1;
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, a, argp);
    }
}

/* Perform machine dependent cif processing */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  முழு flags = 0, cabi = cif->abi;
  size_t bytes = cif->bytes;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  எனில் (cabi == FFI_VFP)
    layout_vfp_args (cif);

  /* Set the return type flag */
  தேர்வு (cif->rtype->type)
    {
    நிலை FFI_TYPE_VOID:
      flags = ARM_TYPE_VOID;
      நிறுத்து;

    நிலை FFI_TYPE_INT:
    நிலை FFI_TYPE_UINT8:
    நிலை FFI_TYPE_SINT8:
    நிலை FFI_TYPE_UINT16:
    நிலை FFI_TYPE_SINT16:
    நிலை FFI_TYPE_UINT32:
    நிலை FFI_TYPE_SINT32:
    நிலை FFI_TYPE_POINTER:
      flags = ARM_TYPE_INT;
      நிறுத்து;

    நிலை FFI_TYPE_SINT64:
    நிலை FFI_TYPE_UINT64:
      flags = ARM_TYPE_INT64;
      நிறுத்து;

    நிலை FFI_TYPE_FLOAT:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_S : ARM_TYPE_INT);
      நிறுத்து;
    நிலை FFI_TYPE_DOUBLE:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_D : ARM_TYPE_INT64);
      நிறுத்து;

    நிலை FFI_TYPE_STRUCT:
    நிலை FFI_TYPE_COMPLEX:
      எனில் (cabi == FFI_VFP)
	{
	  முழு h = vfp_type_p (cif->rtype);

	  flags = ARM_TYPE_VFP_N;
	  எனில் (h == 0x100 + FFI_TYPE_FLOAT)
	    flags = ARM_TYPE_VFP_S;
	  எனில் (h == 0x100 + FFI_TYPE_DOUBLE)
	    flags = ARM_TYPE_VFP_D;
	  எனில் (h != 0)
	      நிறுத்து;
	}

      /* A Composite Type not larger than 4 bytes is returned in r0.
	 A Composite Type larger than 4 bytes, or whose size cannot
	 be determined statically ... is stored in memory at an
	 address passed [in r0].  */
      எனில் (cif->rtype->size <= 4)
	flags = ARM_TYPE_INT;
      இல்லையெனில்
	{
	  flags = ARM_TYPE_STRUCT;
	  bytes += 4;
	}
      நிறுத்து;

    இயல்பு:
      abort();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  bytes = FFI_ALIGN (bytes, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  எனில் (bytes < 4*4)
    bytes = 4*4;

  cif->bytes = bytes;
  cif->flags = flags;

  திரும்பு FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  unsigned முழு nfixedargs, unsigned முழு ntotalargs)
{
  /* VFP variadic calls actually use the SYSV ABI */
  எனில் (cif->abi == FFI_VFP)
    cif->abi = FFI_SYSV;

  திரும்பு ffi_prep_cif_machdep (cif);
}

/* Prototypes for assembly functions, in sysv.S.  */

கட்டமைப்பு call_frame
{
  வெறுமை *fp;
  வெறுமை *lr;
  வெறுமை *rvalue;
  முழு flags;
  வெறுமை *closure;
};

வெளி வெறுமை ffi_call_SYSV (வெறுமை *அடுக்கு, கட்டமைப்பு call_frame *,
			   வெறுமை (*fn) (வெறுமை)) FFI_HIDDEN;
வெளி வெறுமை ffi_call_VFP (வெறுமை *vfp_space, கட்டமைப்பு call_frame *,
			   வெறுமை (*fn) (வெறுமை), unsigned vfp_used) FFI_HIDDEN;

நிலையான வெறுமை
ffi_call_int (ffi_cif * cif, வெறுமை (*fn) (வெறுமை), வெறுமை *rvalue,
	      வெறுமை **avalue, வெறுமை *closure)
{
  முழு flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  size_t bytes, rsize, vfp_size;
  எழுத்து *அடுக்கு, *vfp_space, *new_rvalue;
  கட்டமைப்பு call_frame *frame;

  rsize = 0;
  எனில் (rvalue == வெற்று)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      எனில் (flags == ARM_TYPE_STRUCT)
	rsize = rtype->size;
      இல்லையெனில்
	flags = ARM_TYPE_VOID;
    }
  இல்லையெனில் எனில் (flags == ARM_TYPE_VFP_N)
    {
      /* Largest case is double x 4. */
      rsize = 32;
    }
  இல்லையெனில் எனில் (flags == ARM_TYPE_INT && rtype->type == FFI_TYPE_STRUCT)
    rsize = 4;

  /* Largest case.  */
  vfp_size = (cif->abi == FFI_VFP && cif->vfp_used ? 8*8: 0);

  bytes = cif->bytes;
  அடுக்கு = alloca (vfp_size + bytes + அளவு(கட்டமைப்பு call_frame) + rsize);

  vfp_space = வெற்று;
  எனில் (vfp_size)
    {
      vfp_space = அடுக்கு;
      அடுக்கு += vfp_size;
    }

  frame = (கட்டமைப்பு call_frame *)(அடுக்கு + bytes);

  new_rvalue = rvalue;
  எனில் (rsize)
    new_rvalue = (வெறுமை *)(frame + 1);

  frame->rvalue = new_rvalue;
  frame->flags = flags;
  frame->closure = closure;

  எனில் (vfp_space)
    {
      ffi_prep_args_VFP (cif, flags, new_rvalue, avalue, அடுக்கு, vfp_space);
      ffi_call_VFP (vfp_space, frame, fn, cif->vfp_used);
    }
  இல்லையெனில்
    {
      ffi_prep_args_SYSV (cif, flags, new_rvalue, avalue, அடுக்கு);
      ffi_call_SYSV (அடுக்கு, frame, fn);
    }

  எனில் (rvalue && rvalue != new_rvalue)
    memcpy (rvalue, new_rvalue, rtype->size);
}

வெறுமை
ffi_call (ffi_cif *cif, வெறுமை (*fn) (வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, வெற்று);
}

#ifdef FFI_GO_CLOSURES
வெறுமை
ffi_call_go (ffi_cif *cif, வெறுமை (*fn) (வெறுமை), வெறுமை *rvalue,
	     வெறுமை **avalue, வெறுமை *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif

நிலையான வெறுமை *
ffi_prep_incoming_args_SYSV (ffi_cif *cif, வெறுமை *rvalue,
			     எழுத்து *argp, வெறுமை **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  முழு i, n;

  எனில் (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(வெறுமை **) argp;
      argp += 4;
    }
  இல்லையெனில்
    {
      எனில் (cif->rtype->size && cif->rtype->size < 4)
        *(uint32_t *) rvalue = 0;
    }

  க்கு (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;

      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      எனில் (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (வெறுமை *) (argp + 4 - z);
      இல்லையெனில்
#endif
      avalue[i] = (வெறுமை *) argp;
      argp += z;
    }

  திரும்பு rvalue;
}

நிலையான வெறுமை *
ffi_prep_incoming_args_VFP (ffi_cif *cif, வெறுமை *rvalue, எழுத்து *அடுக்கு,
			    எழுத்து *vfp_space, வெறுமை **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  முழு i, n, vi = 0;
  எழுத்து *argp, *regp, *eo_regp;
  எழுத்து done_with_regs = 0;
  எழுத்து அடுக்கு_used = 0;

  regp = அடுக்கு;
  eo_regp = argp = regp + 16;

  எனில் (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(வெறுமை **) regp;
      regp += 4;
    }

  க்கு (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      முழு is_vfp_type = vfp_type_p (ty);
      size_t z = ty->size;

      எனில் (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_args[vi++] * 4;
	  தொடர்;
	}
      இல்லையெனில் எனில் (!done_with_regs && !is_vfp_type)
	{
	  எழுத்து *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  எனில் (tregp + z <= eo_regp || !அடுக்கு_used)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#ifdef __ARMEB__
	      எனில் (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (வெறுமை *) (tregp + 4 - z);
	      இல்லையெனில்
#endif
	      avalue[i] = (வெறுமை *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      எனில் (regp > eo_regp)
		{
		  FFI_ASSERT (!அடுக்கு_used);
		  argp = regp;
		}
	      எனில் (regp >= eo_regp)
		{
		  done_with_regs = 1;
		  அடுக்கு_used = 1;
		}
	      தொடர்;
	    }
	}

      அடுக்கு_used = 1;
      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      எனில் (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (வெறுமை *) (argp + 4 - z);
      இல்லையெனில்
#endif
      avalue[i] = (வெறுமை *) argp;
      argp += z;
    }

  திரும்பு rvalue;
}

#எனில் FFI_CLOSURES

கட்டமைப்பு closure_frame
{
  எழுத்து vfp_space[8*8] __attribute__((aligned(8)));
  எழுத்து result[8*4];
  எழுத்து argp[];
};

முழு FFI_HIDDEN
ffi_closure_inner_SYSV (ffi_cif *cif,
		        வெறுமை (*fun) (ffi_cif *, வெறுமை *, வெறுமை **, வெறுமை *),
		        வெறுமை *user_data,
		        கட்டமைப்பு closure_frame *frame)
{
  வெறுமை **avalue = (வெறுமை **) alloca (cif->nargs * அளவு (வெறுமை *));
  வெறுமை *rvalue = ffi_prep_incoming_args_SYSV (cif, frame->result,
					      frame->argp, avalue);
  fun (cif, rvalue, avalue, user_data);
  திரும்பு cif->flags;
}

முழு FFI_HIDDEN
ffi_closure_inner_VFP (ffi_cif *cif,
		       வெறுமை (*fun) (ffi_cif *, வெறுமை *, வெறுமை **, வெறுமை *),
		       வெறுமை *user_data,
		       கட்டமைப்பு closure_frame *frame)
{
  வெறுமை **avalue = (வெறுமை **) alloca (cif->nargs * அளவு (வெறுமை *));
  வெறுமை *rvalue = ffi_prep_incoming_args_VFP (cif, frame->result, frame->argp,
					     frame->vfp_space, avalue);
  fun (cif, rvalue, avalue, user_data);
  திரும்பு cif->flags;
}

வெறுமை ffi_closure_SYSV (வெறுமை) FFI_HIDDEN;
வெறுமை ffi_closure_VFP (வெறுமை) FFI_HIDDEN;
#எனில் defined(FFI_EXEC_STATIC_TRAMP)
வெறுமை ffi_closure_SYSV_alt (வெறுமை) FFI_HIDDEN;
வெறுமை ffi_closure_VFP_alt (வெறுமை) FFI_HIDDEN;
#endif

#ifdef FFI_GO_CLOSURES
வெறுமை ffi_go_closure_SYSV (வெறுமை) FFI_HIDDEN;
வெறுமை ffi_go_closure_VFP (வெறுமை) FFI_HIDDEN;
#endif

/* the cif must already be prep'ed */

#எனில் defined(__FreeBSD__) && defined(__arm__)
#define __clear_cache(start, end) செய் { \
		கட்டமைப்பு arm_sync_icache_args ua; 		\
								\
		ua.addr = (uintptr_t)(start);			\
		ua.len = (எழுத்து *)(end) - (எழுத்து *)start;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} போது (0);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure * closure,
		      ffi_cif * cif,
		      வெறுமை (*fun) (ffi_cif *, வெறுமை *, வெறுமை **, வெறுமை *),
		      வெறுமை *user_data, வெறுமை *codeloc)
{
  வெறுமை (*closure_func) (வெறுமை) = ffi_closure_SYSV;

  எனில் (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      எனில் (cif->vfp_used)
	closure_func = ffi_closure_VFP;
    }
  இல்லையெனில் எனில் (cif->abi != FFI_SYSV)
    திரும்பு FFI_BAD_ABI;

#எனில் FFI_EXEC_TRAMPOLINE_TABLE
  வெறுமை **config = (வெறுமை **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = closure_func;
#இல்லையெனில்

# எனில் defined(FFI_EXEC_STATIC_TRAMP)
  எனில் (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      எனில் (closure_func == ffi_closure_SYSV)
        closure_func = ffi_closure_SYSV_alt;
      இல்லையெனில்
        closure_func = ffi_closure_VFP_alt;
      ffi_tramp_set_parms (closure->ftramp, closure_func, closure);
      செல் out;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  memcpy(closure->tramp, ffi_arm_trampoline, 8);
# இல்லையெனில்
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  memcpy(closure->tramp, (வெறுமை*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_CLOSURE_OFFSET);
# endif

# எனில் defined(__QNX__)
  msync (closure->tramp, 8, MS_INVALIDATE_ICACHE);	/* clear data map */
  msync (codeloc, 8, MS_INVALIDATE_ICACHE);		/* clear insn map */
# elif defined(_WIN32)
  FlushInstructionCache(GetCurrentProcess(), closure->tramp, FFI_TRAMPOLINE_SIZE);
# இல்லையெனில்
  __clear_cache(closure->tramp, closure->tramp + 8);	/* clear data map */
  __clear_cache(codeloc, codeloc + 8);			/* clear insn map */
# endif
# ifdef _WIN32
  *(வெறுமை(**)(வெறுமை))(closure->tramp + FFI_TRAMPOLINE_CLOSURE_FUNCTION) = closure_func;
# இல்லையெனில்
  *(வெறுமை (**)(வெறுமை))(closure->tramp + 8) = closure_func;
# endif
# எனில் defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  திரும்பு FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     வெறுமை (*fun) (ffi_cif *, வெறுமை *, வெறுமை **, வெறுமை *))
{
  வெறுமை (*closure_func) (வெறுமை) = ffi_go_closure_SYSV;

  எனில் (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      எனில் (cif->vfp_used)
	closure_func = ffi_go_closure_VFP;
    }
  இல்லையெனில் எனில் (cif->abi != FFI_SYSV)
    திரும்பு FFI_BAD_ABI;

  closure->tramp = closure_func;
  closure->cif = cif;
  closure->fun = fun;

  திரும்பு FFI_OK;
}
#endif

#endif /* FFI_CLOSURES */

/* Below are routines for VFP hard-float support. */

/* A subroutine of vfp_type_p.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

நிலையான முழு
is_hfa0 (மாறிலி ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  முழு i, ret = -1;

  எனில் (elements != வெற்று)
    க்கு (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        எனில் (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            எனில் (ret < 0)
              தொடர்;
          }
        நிறுத்து;
      }

  திரும்பு ret;
}

/* A subroutine of vfp_type_p.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

நிலையான முழு
is_hfa1 (மாறிலி ffi_type *ty, முழு candidate)
{
  ffi_type **elements = ty->elements;
  முழு i;

  எனில் (elements != வெற்று)
    க்கு (i = 0; elements[i]; ++i)
      {
        முழு t = elements[i]->type;
        எனில் (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
          {
            எனில் (!is_hfa1 (elements[i], candidate))
              திரும்பு 0;
          }
        இல்லையெனில் எனில் (t != candidate)
          திரும்பு 0;
      }

  திரும்பு 1;
}

/* Determine if TY is an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is a floating point scalar.

   Returns non-zero iff TY is an HFA.  The result is an encoded value where
   bits 0-7 contain the type code, and bits 8-10 contain the element count.  */

நிலையான முழு
vfp_type_p (மாறிலி ffi_type *ty)
{
  ffi_type **elements;
  முழு candidate, i;
  size_t size, ele_மொத்தம்;

  /* Quickest tests first.  */
  candidate = ty->type;
  தேர்வு (ty->type)
    {
    இயல்பு:
      திரும்பு 0;
    நிலை FFI_TYPE_FLOAT:
    நிலை FFI_TYPE_DOUBLE:
      ele_மொத்தம் = 1;
      செல் done;
    நிலை FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      எனில் (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	திரும்பு 0;
      ele_மொத்தம் = 2;
      செல் done;
    நிலை FFI_TYPE_STRUCT:
      நிறுத்து;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  size = ty->size;
  எனில் (size < 4 || size > 32)
    திரும்பு 0;

  /* Find the type of the first non-structure member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  எனில் (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_COMPLEX)
    {
      க்கு (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          எனில் (candidate >= 0)
            நிறுத்து;
        }
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  தேர்வு (candidate)
    {
    நிலை FFI_TYPE_FLOAT:
      ele_மொத்தம் = size / அளவு(தசம);
      எனில் (size != ele_மொத்தம் * அளவு(தசம))
        திரும்பு 0;
      நிறுத்து;
    நிலை FFI_TYPE_DOUBLE:
      ele_மொத்தம் = size / அளவு(இரட்டை);
      எனில் (size != ele_மொத்தம் * அளவு(இரட்டை))
        திரும்பு 0;
      நிறுத்து;
    இயல்பு:
      திரும்பு 0;
    }
  எனில் (ele_மொத்தம் > 4)
    திரும்பு 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  க்கு (i = 0; elements[i]; ++i)
    {
      முழு t = elements[i]->type;
      எனில் (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          எனில் (!is_hfa1 (elements[i], candidate))
            திரும்பு 0;
        }
      இல்லையெனில் எனில் (t != candidate)
        திரும்பு 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  திரும்பு (ele_மொத்தம் << 8) | candidate;
}

நிலையான முழு
place_vfp_arg (ffi_cif *cif, முழு h)
{
  unsigned short reg = cif->vfp_reg_free;
  முழு align = 1, nregs = h >> 8;

  எனில் ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  எனில் ((reg & 1) && align == 2)
    reg++;

  போது (reg + nregs <= 16)
    {
      முழு s, new_used = 0;
      க்கு (s = reg; s < reg + nregs; s++)
	{
	  new_used |= (1 << s);
	  எனில் (cif->vfp_used & (1 << s))
	    {
	      reg += align;
	      செல் next_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_used |= new_used;
      cif->vfp_args[cif->vfp_nargs++] = (signed எழுத்து)reg;

      /* Update vfp_reg_free. */
      எனில் (cif->vfp_used & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  போது (cif->vfp_used & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      திரும்பு 0;
    next_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_used = 0xFFFF;
  திரும்பு 1;
}

நிலையான வெறுமை
layout_vfp_args (ffi_cif * cif)
{
  unsigned முழு i;
  /* Init VFP fields */
  cif->vfp_used = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  memset (cif->vfp_args, -1, 16);	/* Init to -1. */

  க்கு (i = 0; i < cif->nargs; i++)
    {
      முழு h = vfp_type_p (cif->arg_types[i]);
      எனில் (h && place_vfp_arg (cif, h) == 1)
	நிறுத்து;
    }
}

#எனில் defined(FFI_EXEC_STATIC_TRAMP)
வெறுமை *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  வெளி வெறுமை *trampoline_code_table;

  *tramp_size = ARM_TRAMP_SIZE;
  *map_size = ARM_TRAMP_MAP_SIZE;
  திரும்பு &trampoline_code_table;
}
#endif

#endif /* __arm__ or _M_ARM */
