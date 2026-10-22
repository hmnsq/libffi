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

#ଯଦି defined(__arm__) || defined(_M_ARM)
#ଅନ୍ତର୍ଭାଵ <fficonfig.h>
#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <tramp.h>
#ଅନ୍ତର୍ଭାଵ "internal.h"

#ଯଦି defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#ଅନ୍ତର୍ଭାଵ <windows.h>
#endif

#ଯଦି FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#ଅନ୍ତର୍ଭାଵ <mach/machine/vm_param.h>
#endif

#ଅନ୍ୟଥା
#ifndef _WIN32
ବାହ୍ୟ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ffi_arm_trampoline[2] FFI_HIDDEN;
#ଅନ୍ୟଥା
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
ବାହ୍ୟ unsigned ଅକ୍ଷର ffi_arm_trampoline[12] FFI_HIDDEN;
#endif
#endif

#ଯଦି defined(__FreeBSD__) && defined(__arm__)
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <machine/sysarch.h>
#endif

#ଯଦି defined(__QNX__)
#ଅନ୍ତର୍ଭାଵ <sys/mman.h>
#endif

/* Forward declares. */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା vfp_type_p (ଅଚଳ ffi_type *);
ସ୍ଥିର ଶୂନ୍ୟ layout_vfp_args (ffi_cif *);

ସ୍ଥିର ଶୂନ୍ୟ *
ffi_align (ffi_type *ty, ଶୂନ୍ୟ *p)
{
  /* Align if necessary */
  size_t alignment;
#ifdef _WIN32_WCE
  alignment = 4;
#ଅନ୍ୟଥା
  alignment = ty->alignment;
  ଯଦି (alignment < 4)
    alignment = 4;
#endif
  ଫେରନ୍ତୁ (ଶୂନ୍ୟ *) FFI_ALIGN (p, alignment);
}

ସ୍ଥିର size_t
ffi_put_arg (ffi_type *ty, ଶୂନ୍ୟ *src, ଶୂନ୍ୟ *dst)
{
  size_t z = ty->size;

  ବାଛ (ty->type)
    {
    ସ୍ଥିତି FFI_TYPE_SINT8:
      *(UINT32 *)dst = *(SINT8 *)src;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_UINT8:
      *(UINT32 *)dst = *(UINT8 *)src;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT16:
      *(UINT32 *)dst = *(SINT16 *)src;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_UINT16:
      *(UINT32 *)dst = *(UINT16 *)src;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_INT:
    ସ୍ଥିତି FFI_TYPE_SINT32:
    ସ୍ଥିତି FFI_TYPE_UINT32:
    ସ୍ଥିତି FFI_TYPE_POINTER:
#ifndef _WIN32
    ସ୍ଥିତି FFI_TYPE_FLOAT:
#endif
      *(UINT32 *)dst = *(UINT32 *)src;
      ଭାଙ୍ଗ;

#ifdef _WIN32
    // casting a float* to a UINT32* doesn't work on Windows
    ସ୍ଥିତି FFI_TYPE_FLOAT:
        *(uintptr_t *)dst = 0;
        *(ଦଶମିକ *)dst = *(ଦଶମିକ *)src;
        ଭାଙ୍ଗ;
#endif

    ସ୍ଥିତି FFI_TYPE_SINT64:
    ସ୍ଥିତି FFI_TYPE_UINT64:
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      *(UINT64 *)dst = *(UINT64 *)src;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_STRUCT:
    ସ୍ଥିତି FFI_TYPE_COMPLEX:
      memcpy (dst, src, z);
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      abort();
    }

  ଫେରନ୍ତୁ FFI_ALIGN (z, 4);
}

/* ffi_prep_args is called once stack space has been allocated
   for the function's arguments.

   The vfp_space parameter is the load area for VFP regs, the return
   value is cif->vfp_used (word bitset of VFP regs used for passing
   arguments). These are only used for the VFP hard-float ABI.
*/
ସ୍ଥିର ଶୂନ୍ୟ
ffi_prep_args_SYSV (ffi_cif *cif, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ଶୂନ୍ୟ *rvalue,
		    ଶୂନ୍ୟ **avalue, ଅକ୍ଷର *argp)
{
  ffi_type **arg_types = cif->arg_types;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, n;

  ଯଦି (flags == ARM_TYPE_STRUCT)
    {
      *(ଶୂନ୍ୟ **) argp = rvalue;
      argp += 4;
    }

  ପାଇଁ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, avalue[i], argp);
    }
}

ସ୍ଥିର ଶୂନ୍ୟ
ffi_prep_args_VFP (ffi_cif *cif, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ଶୂନ୍ୟ *rvalue,
                   ଶୂନ୍ୟ **avalue, ଅକ୍ଷର *ଗଦା, ଅକ୍ଷର *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, n, vi = 0;
  ଅକ୍ଷର *argp, *regp, *eo_regp;
  ଅକ୍ଷର ଗଦା_used = 0;
  ଅକ୍ଷର done_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = ଗଦା;
  eo_regp = argp = regp + 16;

  /* If the function returns an FFI_TYPE_STRUCT in memory,
     that address is passed in r0 to the function.  */
  ଯଦି (flags == ARM_TYPE_STRUCT)
    {
      *(ଶୂନ୍ୟ **) regp = rvalue;
      regp += 4;
    }

  ପାଇଁ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      ଶୂନ୍ୟ *a = avalue[i];
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_vfp_type = vfp_type_p (ty);

      /* Allocated in VFP registers. */
      ଯଦି (vi < cif->vfp_nargs && is_vfp_type)
	{
	  ଅକ୍ଷର *vfp_slot = vfp_space + cif->vfp_args[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_slot);
	  ଜାରି;
	}
      /* Try allocating in core registers. */
      ଅନ୍ୟଥା ଯଦି (!done_with_regs && !is_vfp_type)
	{
	  ଅକ୍ଷର *tregp = ffi_align (ty, regp);
	  size_t size = ty->size;
	  size = (size < 4) ? 4 : size;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  ଯଦି (tregp + size <= eo_regp)
	    {
	      regp = tregp + ffi_put_arg (ty, a, tregp);
	      done_with_regs = (regp == argp);
	      // ensure we did not write into the stack area
	      FFI_ASSERT (regp <= argp);
	      ଜାରି;
	    }
	  /* In case there are no arguments in the stack area yet,
	     the argument is passed in the remaining core registers
	     and on the stack.  */
	  ଅନ୍ୟଥା ଯଦି (!ଗଦା_used)
	    {
	      ଗଦା_used = 1;
	      done_with_regs = 1;
	      argp = tregp + ffi_put_arg (ty, a, tregp);
	      FFI_ASSERT (eo_regp < argp);
	      ଜାରି;
	    }
	}
      /* Base case, arguments are passed on the stack */
      ଗଦା_used = 1;
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, a, argp);
    }
}

/* Perform machine dependent cif processing */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = 0, cabi = cif->abi;
  size_t bytes = cif->bytes;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  ଯଦି (cabi == FFI_VFP)
    layout_vfp_args (cif);

  /* Set the return type flag */
  ବାଛ (cif->rtype->type)
    {
    ସ୍ଥିତି FFI_TYPE_VOID:
      flags = ARM_TYPE_VOID;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_INT:
    ସ୍ଥିତି FFI_TYPE_UINT8:
    ସ୍ଥିତି FFI_TYPE_SINT8:
    ସ୍ଥିତି FFI_TYPE_UINT16:
    ସ୍ଥିତି FFI_TYPE_SINT16:
    ସ୍ଥିତି FFI_TYPE_UINT32:
    ସ୍ଥିତି FFI_TYPE_SINT32:
    ସ୍ଥିତି FFI_TYPE_POINTER:
      flags = ARM_TYPE_INT;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_SINT64:
    ସ୍ଥିତି FFI_TYPE_UINT64:
      flags = ARM_TYPE_INT64;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_FLOAT:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_S : ARM_TYPE_INT);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_D : ARM_TYPE_INT64);
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_STRUCT:
    ସ୍ଥିତି FFI_TYPE_COMPLEX:
      ଯଦି (cabi == FFI_VFP)
	{
	  ପୂର୍ଣ୍ଣସଂଖ୍ୟା h = vfp_type_p (cif->rtype);

	  flags = ARM_TYPE_VFP_N;
	  ଯଦି (h == 0x100 + FFI_TYPE_FLOAT)
	    flags = ARM_TYPE_VFP_S;
	  ଯଦି (h == 0x100 + FFI_TYPE_DOUBLE)
	    flags = ARM_TYPE_VFP_D;
	  ଯଦି (h != 0)
	      ଭାଙ୍ଗ;
	}

      /* A Composite Type not larger than 4 bytes is returned in r0.
	 A Composite Type larger than 4 bytes, or whose size cannot
	 be determined statically ... is stored in memory at an
	 address passed [in r0].  */
      ଯଦି (cif->rtype->size <= 4)
	flags = ARM_TYPE_INT;
      ଅନ୍ୟଥା
	{
	  flags = ARM_TYPE_STRUCT;
	  bytes += 4;
	}
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      abort();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  bytes = FFI_ALIGN (bytes, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  ଯଦି (bytes < 4*4)
    bytes = 4*4;

  cif->bytes = bytes;
  cif->flags = flags;

  ଫେରନ୍ତୁ FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfixedargs, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ntotalargs)
{
  /* VFP variadic calls actually use the SYSV ABI */
  ଯଦି (cif->abi == FFI_VFP)
    cif->abi = FFI_SYSV;

  ଫେରନ୍ତୁ ffi_prep_cif_machdep (cif);
}

/* Prototypes for assembly functions, in sysv.S.  */

ସଂରଚନା call_frame
{
  ଶୂନ୍ୟ *fp;
  ଶୂନ୍ୟ *lr;
  ଶୂନ୍ୟ *rvalue;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags;
  ଶୂନ୍ୟ *closure;
};

ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_SYSV (ଶୂନ୍ୟ *ଗଦା, ସଂରଚନା call_frame *,
			   ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ)) FFI_HIDDEN;
ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_VFP (ଶୂନ୍ୟ *vfp_space, ସଂରଚନା call_frame *,
			   ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ), unsigned vfp_used) FFI_HIDDEN;

ସ୍ଥିର ଶୂନ୍ୟ
ffi_call_int (ffi_cif * cif, ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
	      ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *closure)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  size_t bytes, rsize, vfp_size;
  ଅକ୍ଷର *ଗଦା, *vfp_space, *new_rvalue;
  ସଂରଚନା call_frame *frame;

  rsize = 0;
  ଯଦି (rvalue == ଖାଲି)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      ଯଦି (flags == ARM_TYPE_STRUCT)
	rsize = rtype->size;
      ଅନ୍ୟଥା
	flags = ARM_TYPE_VOID;
    }
  ଅନ୍ୟଥା ଯଦି (flags == ARM_TYPE_VFP_N)
    {
      /* Largest case is double x 4. */
      rsize = 32;
    }
  ଅନ୍ୟଥା ଯଦି (flags == ARM_TYPE_INT && rtype->type == FFI_TYPE_STRUCT)
    rsize = 4;

  /* Largest case.  */
  vfp_size = (cif->abi == FFI_VFP && cif->vfp_used ? 8*8: 0);

  bytes = cif->bytes;
  ଗଦା = alloca (vfp_size + bytes + ଆକାର(ସଂରଚନା call_frame) + rsize);

  vfp_space = ଖାଲି;
  ଯଦି (vfp_size)
    {
      vfp_space = ଗଦା;
      ଗଦା += vfp_size;
    }

  frame = (ସଂରଚନା call_frame *)(ଗଦା + bytes);

  new_rvalue = rvalue;
  ଯଦି (rsize)
    new_rvalue = (ଶୂନ୍ୟ *)(frame + 1);

  frame->rvalue = new_rvalue;
  frame->flags = flags;
  frame->closure = closure;

  ଯଦି (vfp_space)
    {
      ffi_prep_args_VFP (cif, flags, new_rvalue, avalue, ଗଦା, vfp_space);
      ffi_call_VFP (vfp_space, frame, fn, cif->vfp_used);
    }
  ଅନ୍ୟଥା
    {
      ffi_prep_args_SYSV (cif, flags, new_rvalue, avalue, ଗଦା);
      ffi_call_SYSV (ଗଦା, frame, fn);
    }

  ଯଦି (rvalue && rvalue != new_rvalue)
    memcpy (rvalue, new_rvalue, rtype->size);
}

ଶୂନ୍ୟ
ffi_call (ffi_cif *cif, ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ଖାଲି);
}

#ifdef FFI_GO_CLOSURES
ଶୂନ୍ୟ
ffi_call_go (ffi_cif *cif, ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
	     ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif

ସ୍ଥିର ଶୂନ୍ୟ *
ffi_prep_incoming_args_SYSV (ffi_cif *cif, ଶୂନ୍ୟ *rvalue,
			     ଅକ୍ଷର *argp, ଶୂନ୍ୟ **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, n;

  ଯଦି (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(ଶୂନ୍ୟ **) argp;
      argp += 4;
    }
  ଅନ୍ୟଥା
    {
      ଯଦି (cif->rtype->size && cif->rtype->size < 4)
        *(uint32_t *) rvalue = 0;
    }

  ପାଇଁ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;

      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      ଯଦି (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (ଶୂନ୍ୟ *) (argp + 4 - z);
      ଅନ୍ୟଥା
#endif
      avalue[i] = (ଶୂନ୍ୟ *) argp;
      argp += z;
    }

  ଫେରନ୍ତୁ rvalue;
}

ସ୍ଥିର ଶୂନ୍ୟ *
ffi_prep_incoming_args_VFP (ffi_cif *cif, ଶୂନ୍ୟ *rvalue, ଅକ୍ଷର *ଗଦା,
			    ଅକ୍ଷର *vfp_space, ଶୂନ୍ୟ **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, n, vi = 0;
  ଅକ୍ଷର *argp, *regp, *eo_regp;
  ଅକ୍ଷର done_with_regs = 0;
  ଅକ୍ଷର ଗଦା_used = 0;

  regp = ଗଦା;
  eo_regp = argp = regp + 16;

  ଯଦି (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(ଶୂନ୍ୟ **) regp;
      regp += 4;
    }

  ପାଇଁ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_vfp_type = vfp_type_p (ty);
      size_t z = ty->size;

      ଯଦି (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_args[vi++] * 4;
	  ଜାରି;
	}
      ଅନ୍ୟଥା ଯଦି (!done_with_regs && !is_vfp_type)
	{
	  ଅକ୍ଷର *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  ଯଦି (tregp + z <= eo_regp || !ଗଦା_used)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#ifdef __ARMEB__
	      ଯଦି (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (ଶୂନ୍ୟ *) (tregp + 4 - z);
	      ଅନ୍ୟଥା
#endif
	      avalue[i] = (ଶୂନ୍ୟ *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      ଯଦି (regp > eo_regp)
		{
		  FFI_ASSERT (!ଗଦା_used);
		  argp = regp;
		}
	      ଯଦି (regp >= eo_regp)
		{
		  done_with_regs = 1;
		  ଗଦା_used = 1;
		}
	      ଜାରି;
	    }
	}

      ଗଦା_used = 1;
      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      ଯଦି (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (ଶୂନ୍ୟ *) (argp + 4 - z);
      ଅନ୍ୟଥା
#endif
      avalue[i] = (ଶୂନ୍ୟ *) argp;
      argp += z;
    }

  ଫେରନ୍ତୁ rvalue;
}

#ଯଦି FFI_CLOSURES

ସଂରଚନା closure_frame
{
  ଅକ୍ଷର vfp_space[8*8] __attribute__((aligned(8)));
  ଅକ୍ଷର result[8*4];
  ଅକ୍ଷର argp[];
};

ପୂର୍ଣ୍ଣସଂଖ୍ୟା FFI_HIDDEN
ffi_closure_inner_SYSV (ffi_cif *cif,
		        ଶୂନ୍ୟ (*fun) (ffi_cif *, ଶୂନ୍ୟ *, ଶୂନ୍ୟ **, ଶୂନ୍ୟ *),
		        ଶୂନ୍ୟ *user_data,
		        ସଂରଚନା closure_frame *frame)
{
  ଶୂନ୍ୟ **avalue = (ଶୂନ୍ୟ **) alloca (cif->nargs * ଆକାର (ଶୂନ୍ୟ *));
  ଶୂନ୍ୟ *rvalue = ffi_prep_incoming_args_SYSV (cif, frame->result,
					      frame->argp, avalue);
  fun (cif, rvalue, avalue, user_data);
  ଫେରନ୍ତୁ cif->flags;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା FFI_HIDDEN
ffi_closure_inner_VFP (ffi_cif *cif,
		       ଶୂନ୍ୟ (*fun) (ffi_cif *, ଶୂନ୍ୟ *, ଶୂନ୍ୟ **, ଶୂନ୍ୟ *),
		       ଶୂନ୍ୟ *user_data,
		       ସଂରଚନା closure_frame *frame)
{
  ଶୂନ୍ୟ **avalue = (ଶୂନ୍ୟ **) alloca (cif->nargs * ଆକାର (ଶୂନ୍ୟ *));
  ଶୂନ୍ୟ *rvalue = ffi_prep_incoming_args_VFP (cif, frame->result, frame->argp,
					     frame->vfp_space, avalue);
  fun (cif, rvalue, avalue, user_data);
  ଫେରନ୍ତୁ cif->flags;
}

ଶୂନ୍ୟ ffi_closure_SYSV (ଶୂନ୍ୟ) FFI_HIDDEN;
ଶୂନ୍ୟ ffi_closure_VFP (ଶୂନ୍ୟ) FFI_HIDDEN;
#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
ଶୂନ୍ୟ ffi_closure_SYSV_alt (ଶୂନ୍ୟ) FFI_HIDDEN;
ଶୂନ୍ୟ ffi_closure_VFP_alt (ଶୂନ୍ୟ) FFI_HIDDEN;
#endif

#ifdef FFI_GO_CLOSURES
ଶୂନ୍ୟ ffi_go_closure_SYSV (ଶୂନ୍ୟ) FFI_HIDDEN;
ଶୂନ୍ୟ ffi_go_closure_VFP (ଶୂନ୍ୟ) FFI_HIDDEN;
#endif

/* the cif must already be prep'ed */

#ଯଦି defined(__FreeBSD__) && defined(__arm__)
#define __clear_cache(start, end) କର { \
		ସଂରଚନା arm_sync_icache_args ua; 		\
								\
		ua.addr = (uintptr_t)(start);			\
		ua.len = (ଅକ୍ଷର *)(end) - (ଅକ୍ଷର *)start;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} ଯେତେବେଳେ (0);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure * closure,
		      ffi_cif * cif,
		      ଶୂନ୍ୟ (*fun) (ffi_cif *, ଶୂନ୍ୟ *, ଶୂନ୍ୟ **, ଶୂନ୍ୟ *),
		      ଶୂନ୍ୟ *user_data, ଶୂନ୍ୟ *codeloc)
{
  ଶୂନ୍ୟ (*closure_func) (ଶୂନ୍ୟ) = ffi_closure_SYSV;

  ଯଦି (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      ଯଦି (cif->vfp_used)
	closure_func = ffi_closure_VFP;
    }
  ଅନ୍ୟଥା ଯଦି (cif->abi != FFI_SYSV)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

#ଯଦି FFI_EXEC_TRAMPOLINE_TABLE
  ଶୂନ୍ୟ **config = (ଶୂନ୍ୟ **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = closure_func;
#ଅନ୍ୟଥା

# ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
  ଯଦି (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ଯଦି (closure_func == ffi_closure_SYSV)
        closure_func = ffi_closure_SYSV_alt;
      ଅନ୍ୟଥା
        closure_func = ffi_closure_VFP_alt;
      ffi_tramp_set_parms (closure->ftramp, closure_func, closure);
      ଯାଅ out;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  memcpy(closure->tramp, ffi_arm_trampoline, 8);
# ଅନ୍ୟଥା
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  memcpy(closure->tramp, (ଶୂନ୍ୟ*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_CLOSURE_OFFSET);
# endif

# ଯଦି defined(__QNX__)
  msync (closure->tramp, 8, MS_INVALIDATE_ICACHE);	/* clear data map */
  msync (codeloc, 8, MS_INVALIDATE_ICACHE);		/* clear insn map */
# elif defined(_WIN32)
  FlushInstructionCache(GetCurrentProcess(), closure->tramp, FFI_TRAMPOLINE_SIZE);
# ଅନ୍ୟଥା
  __clear_cache(closure->tramp, closure->tramp + 8);	/* clear data map */
  __clear_cache(codeloc, codeloc + 8);			/* clear insn map */
# endif
# ifdef _WIN32
  *(ଶୂନ୍ୟ(**)(ଶୂନ୍ୟ))(closure->tramp + FFI_TRAMPOLINE_CLOSURE_FUNCTION) = closure_func;
# ଅନ୍ୟଥା
  *(ଶୂନ୍ୟ (**)(ଶୂନ୍ୟ))(closure->tramp + 8) = closure_func;
# endif
# ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ଫେରନ୍ତୁ FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     ଶୂନ୍ୟ (*fun) (ffi_cif *, ଶୂନ୍ୟ *, ଶୂନ୍ୟ **, ଶୂନ୍ୟ *))
{
  ଶୂନ୍ୟ (*closure_func) (ଶୂନ୍ୟ) = ffi_go_closure_SYSV;

  ଯଦି (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      ଯଦି (cif->vfp_used)
	closure_func = ffi_go_closure_VFP;
    }
  ଅନ୍ୟଥା ଯଦି (cif->abi != FFI_SYSV)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

  closure->tramp = closure_func;
  closure->cif = cif;
  closure->fun = fun;

  ଫେରନ୍ତୁ FFI_OK;
}
#endif

#endif /* FFI_CLOSURES */

/* Below are routines for VFP hard-float support. */

/* A subroutine of vfp_type_p.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
is_hfa0 (ଅଚଳ ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, ret = -1;

  ଯଦି (elements != ଖାଲି)
    ପାଇଁ (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        ଯଦି (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            ଯଦି (ret < 0)
              ଜାରି;
          }
        ଭାଙ୍ଗ;
      }

  ଫେରନ୍ତୁ ret;
}

/* A subroutine of vfp_type_p.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
is_hfa1 (ଅଚଳ ffi_type *ty, ପୂର୍ଣ୍ଣସଂଖ୍ୟା candidate)
{
  ffi_type **elements = ty->elements;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ଯଦି (elements != ଖାଲି)
    ପାଇଁ (i = 0; elements[i]; ++i)
      {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = elements[i]->type;
        ଯଦି (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
          {
            ଯଦି (!is_hfa1 (elements[i], candidate))
              ଫେରନ୍ତୁ 0;
          }
        ଅନ୍ୟଥା ଯଦି (t != candidate)
          ଫେରନ୍ତୁ 0;
      }

  ଫେରନ୍ତୁ 1;
}

/* Determine if TY is an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is a floating point scalar.

   Returns non-zero iff TY is an HFA.  The result is an encoded value where
   bits 0-7 contain the type code, and bits 8-10 contain the element count.  */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
vfp_type_p (ଅଚଳ ffi_type *ty)
{
  ffi_type **elements;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା candidate, i;
  size_t size, ele_ଗଣନା;

  /* Quickest tests first.  */
  candidate = ty->type;
  ବାଛ (ty->type)
    {
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ 0;
    ସ୍ଥିତି FFI_TYPE_FLOAT:
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      ele_ଗଣନା = 1;
      ଯାଅ done;
    ସ୍ଥିତି FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      ଯଦି (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	ଫେରନ୍ତୁ 0;
      ele_ଗଣନା = 2;
      ଯାଅ done;
    ସ୍ଥିତି FFI_TYPE_STRUCT:
      ଭାଙ୍ଗ;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  size = ty->size;
  ଯଦି (size < 4 || size > 32)
    ଫେରନ୍ତୁ 0;

  /* Find the type of the first non-structure member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  ଯଦି (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_COMPLEX)
    {
      ପାଇଁ (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          ଯଦି (candidate >= 0)
            ଭାଙ୍ଗ;
        }
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  ବାଛ (candidate)
    {
    ସ୍ଥିତି FFI_TYPE_FLOAT:
      ele_ଗଣନା = size / ଆକାର(ଦଶମିକ);
      ଯଦି (size != ele_ଗଣନା * ଆକାର(ଦଶମିକ))
        ଫେରନ୍ତୁ 0;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      ele_ଗଣନା = size / ଆକାର(ଦ୍ୱିଗୁଣ);
      ଯଦି (size != ele_ଗଣନା * ଆକାର(ଦ୍ୱିଗୁଣ))
        ଫେରନ୍ତୁ 0;
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ 0;
    }
  ଯଦି (ele_ଗଣନା > 4)
    ଫେରନ୍ତୁ 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  ପାଇଁ (i = 0; elements[i]; ++i)
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = elements[i]->type;
      ଯଦି (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          ଯଦି (!is_hfa1 (elements[i], candidate))
            ଫେରନ୍ତୁ 0;
        }
      ଅନ୍ୟଥା ଯଦି (t != candidate)
        ଫେରନ୍ତୁ 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  ଫେରନ୍ତୁ (ele_ଗଣନା << 8) | candidate;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
place_vfp_arg (ffi_cif *cif, ପୂର୍ଣ୍ଣସଂଖ୍ୟା h)
{
  unsigned short reg = cif->vfp_reg_free;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା align = 1, nregs = h >> 8;

  ଯଦି ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  ଯଦି ((reg & 1) && align == 2)
    reg++;

  ଯେତେବେଳେ (reg + nregs <= 16)
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା s, new_used = 0;
      ପାଇଁ (s = reg; s < reg + nregs; s++)
	{
	  new_used |= (1 << s);
	  ଯଦି (cif->vfp_used & (1 << s))
	    {
	      reg += align;
	      ଯାଅ next_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_used |= new_used;
      cif->vfp_args[cif->vfp_nargs++] = (signed ଅକ୍ଷର)reg;

      /* Update vfp_reg_free. */
      ଯଦି (cif->vfp_used & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  ଯେତେବେଳେ (cif->vfp_used & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      ଫେରନ୍ତୁ 0;
    next_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_used = 0xFFFF;
  ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ଶୂନ୍ୟ
layout_vfp_args (ffi_cif * cif)
{
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  /* Init VFP fields */
  cif->vfp_used = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  memset (cif->vfp_args, -1, 16);	/* Init to -1. */

  ପାଇଁ (i = 0; i < cif->nargs; i++)
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା h = vfp_type_p (cif->arg_types[i]);
      ଯଦି (h && place_vfp_arg (cif, h) == 1)
	ଭାଙ୍ଗ;
    }
}

#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
ଶୂନ୍ୟ *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  ବାହ୍ୟ ଶୂନ୍ୟ *trampoline_code_table;

  *tramp_size = ARM_TRAMP_SIZE;
  *map_size = ARM_TRAMP_MAP_SIZE;
  ଫେରନ୍ତୁ &trampoline_code_table;
}
#endif

#endif /* __arm__ or _M_ARM */
