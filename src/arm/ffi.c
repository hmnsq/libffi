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

#જો defined(__arm__) || defined(_M_ARM)
#અન્તર્ભાવ <fficonfig.h>
#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <tramp.h>
#અન્તર્ભાવ "internal.h"

#જો defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#અન્તર્ભાવ <windows.h>
#endif

#જો FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#અન્તર્ભાવ <mach/machine/vm_param.h>
#endif

#નહીંતર
#ifndef _WIN32
બાહ્ય unsigned પૂર્ણાંક ffi_arm_trampoline[2] FFI_HIDDEN;
#નહીંતર
// Declare this as an array of char, instead of array of int,
// otherwise Clang optimizes out the "& 0xFFFFFFFE" for clearing
// the thumb bit.
બાહ્ય unsigned અક્ષર ffi_arm_trampoline[12] FFI_HIDDEN;
#endif
#endif

#જો defined(__FreeBSD__) && defined(__arm__)
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <machine/sysarch.h>
#endif

#જો defined(__QNX__)
#અન્તર્ભાવ <sys/mman.h>
#endif

/* Forward declares. */
સ્થિર પૂર્ણાંક vfp_type_p (અચલ ffi_type *);
સ્થિર શૂન્ય layout_vfp_args (ffi_cif *);

સ્થિર શૂન્ય *
ffi_align (ffi_type *ty, શૂન્ય *p)
{
  /* Align if necessary */
  size_t alignment;
#ifdef _WIN32_WCE
  alignment = 4;
#નહીંતર
  alignment = ty->alignment;
  જો (alignment < 4)
    alignment = 4;
#endif
  પાછા (શૂન્ય *) FFI_ALIGN (p, alignment);
}

સ્થિર size_t
ffi_put_arg (ffi_type *ty, શૂન્ય *src, શૂન્ય *dst)
{
  size_t z = ty->size;

  પસંદગી (ty->type)
    {
    સ્થિતિ FFI_TYPE_SINT8:
      *(UINT32 *)dst = *(SINT8 *)src;
      તોડો;
    સ્થિતિ FFI_TYPE_UINT8:
      *(UINT32 *)dst = *(UINT8 *)src;
      તોડો;
    સ્થિતિ FFI_TYPE_SINT16:
      *(UINT32 *)dst = *(SINT16 *)src;
      તોડો;
    સ્થિતિ FFI_TYPE_UINT16:
      *(UINT32 *)dst = *(UINT16 *)src;
      તોડો;

    સ્થિતિ FFI_TYPE_INT:
    સ્થિતિ FFI_TYPE_SINT32:
    સ્થિતિ FFI_TYPE_UINT32:
    સ્થિતિ FFI_TYPE_POINTER:
#ifndef _WIN32
    સ્થિતિ FFI_TYPE_FLOAT:
#endif
      *(UINT32 *)dst = *(UINT32 *)src;
      તોડો;

#ifdef _WIN32
    // casting a float* to a UINT32* doesn't work on Windows
    સ્થિતિ FFI_TYPE_FLOAT:
        *(uintptr_t *)dst = 0;
        *(દશાંશ *)dst = *(દશાંશ *)src;
        તોડો;
#endif

    સ્થિતિ FFI_TYPE_SINT64:
    સ્થિતિ FFI_TYPE_UINT64:
    સ્થિતિ FFI_TYPE_DOUBLE:
      *(UINT64 *)dst = *(UINT64 *)src;
      તોડો;

    સ્થિતિ FFI_TYPE_STRUCT:
    સ્થિતિ FFI_TYPE_COMPLEX:
      memcpy (dst, src, z);
      તોડો;

    મૂળભૂત:
      abort();
    }

  પાછા FFI_ALIGN (z, 4);
}

/* ffi_prep_args is called once stack space has been allocated
   for the function's arguments.

   The vfp_space parameter is the load area for VFP regs, the return
   value is cif->vfp_used (word bitset of VFP regs used for passing
   arguments). These are only used for the VFP hard-float ABI.
*/
સ્થિર શૂન્ય
ffi_prep_args_SYSV (ffi_cif *cif, પૂર્ણાંક flags, શૂન્ય *rvalue,
		    શૂન્ય **avalue, અક્ષર *argp)
{
  ffi_type **arg_types = cif->arg_types;
  પૂર્ણાંક i, n;

  જો (flags == ARM_TYPE_STRUCT)
    {
      *(શૂન્ય **) argp = rvalue;
      argp += 4;
    }

  માટે (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, avalue[i], argp);
    }
}

સ્થિર શૂન્ય
ffi_prep_args_VFP (ffi_cif *cif, પૂર્ણાંક flags, શૂન્ય *rvalue,
                   શૂન્ય **avalue, અક્ષર *ઢગલો, અક્ષર *vfp_space)
{
  ffi_type **arg_types = cif->arg_types;
  પૂર્ણાંક i, n, vi = 0;
  અક્ષર *argp, *regp, *eo_regp;
  અક્ષર ઢગલો_used = 0;
  અક્ષર done_with_regs = 0;

  /* The first 4 words on the stack are used for values
     passed in core registers.  */
  regp = ઢગલો;
  eo_regp = argp = regp + 16;

  /* If the function returns an FFI_TYPE_STRUCT in memory,
     that address is passed in r0 to the function.  */
  જો (flags == ARM_TYPE_STRUCT)
    {
      *(શૂન્ય **) regp = rvalue;
      regp += 4;
    }

  માટે (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      શૂન્ય *a = avalue[i];
      પૂર્ણાંક is_vfp_type = vfp_type_p (ty);

      /* Allocated in VFP registers. */
      જો (vi < cif->vfp_nargs && is_vfp_type)
	{
	  અક્ષર *vfp_slot = vfp_space + cif->vfp_args[vi++] * 4;
	  ffi_put_arg (ty, a, vfp_slot);
	  ચાલુ;
	}
      /* Try allocating in core registers. */
      નહીંતર જો (!done_with_regs && !is_vfp_type)
	{
	  અક્ષર *tregp = ffi_align (ty, regp);
	  size_t size = ty->size;
	  size = (size < 4) ? 4 : size;	// pad
	  /* Check if there is space left in the aligned register
	     area to place the argument.  */
	  જો (tregp + size <= eo_regp)
	    {
	      regp = tregp + ffi_put_arg (ty, a, tregp);
	      done_with_regs = (regp == argp);
	      // ensure we did not write into the stack area
	      FFI_ASSERT (regp <= argp);
	      ચાલુ;
	    }
	  /* In case there are no arguments in the stack area yet,
	     the argument is passed in the remaining core registers
	     and on the stack.  */
	  નહીંતર જો (!ઢગલો_used)
	    {
	      ઢગલો_used = 1;
	      done_with_regs = 1;
	      argp = tregp + ffi_put_arg (ty, a, tregp);
	      FFI_ASSERT (eo_regp < argp);
	      ચાલુ;
	    }
	}
      /* Base case, arguments are passed on the stack */
      ઢગલો_used = 1;
      argp = ffi_align (ty, argp);
      argp += ffi_put_arg (ty, a, argp);
    }
}

/* Perform machine dependent cif processing */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  પૂર્ણાંક flags = 0, cabi = cif->abi;
  size_t bytes = cif->bytes;

  /* Map out the register placements of VFP register args.  The VFP
     hard-float calling conventions are slightly more sophisticated
     than the base calling conventions, so we do it here instead of
     in ffi_prep_args(). */
  જો (cabi == FFI_VFP)
    layout_vfp_args (cif);

  /* Set the return type flag */
  પસંદગી (cif->rtype->type)
    {
    સ્થિતિ FFI_TYPE_VOID:
      flags = ARM_TYPE_VOID;
      તોડો;

    સ્થિતિ FFI_TYPE_INT:
    સ્થિતિ FFI_TYPE_UINT8:
    સ્થિતિ FFI_TYPE_SINT8:
    સ્થિતિ FFI_TYPE_UINT16:
    સ્થિતિ FFI_TYPE_SINT16:
    સ્થિતિ FFI_TYPE_UINT32:
    સ્થિતિ FFI_TYPE_SINT32:
    સ્થિતિ FFI_TYPE_POINTER:
      flags = ARM_TYPE_INT;
      તોડો;

    સ્થિતિ FFI_TYPE_SINT64:
    સ્થિતિ FFI_TYPE_UINT64:
      flags = ARM_TYPE_INT64;
      તોડો;

    સ્થિતિ FFI_TYPE_FLOAT:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_S : ARM_TYPE_INT);
      તોડો;
    સ્થિતિ FFI_TYPE_DOUBLE:
      flags = (cabi == FFI_VFP ? ARM_TYPE_VFP_D : ARM_TYPE_INT64);
      તોડો;

    સ્થિતિ FFI_TYPE_STRUCT:
    સ્થિતિ FFI_TYPE_COMPLEX:
      જો (cabi == FFI_VFP)
	{
	  પૂર્ણાંક h = vfp_type_p (cif->rtype);

	  flags = ARM_TYPE_VFP_N;
	  જો (h == 0x100 + FFI_TYPE_FLOAT)
	    flags = ARM_TYPE_VFP_S;
	  જો (h == 0x100 + FFI_TYPE_DOUBLE)
	    flags = ARM_TYPE_VFP_D;
	  જો (h != 0)
	      તોડો;
	}

      /* A Composite Type not larger than 4 bytes is returned in r0.
	 A Composite Type larger than 4 bytes, or whose size cannot
	 be determined statically ... is stored in memory at an
	 address passed [in r0].  */
      જો (cif->rtype->size <= 4)
	flags = ARM_TYPE_INT;
      નહીંતર
	{
	  flags = ARM_TYPE_STRUCT;
	  bytes += 4;
	}
      તોડો;

    મૂળભૂત:
      abort();
    }

  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't harm anything
     when it isn't needed.  */
  bytes = FFI_ALIGN (bytes, 8);

  /* Minimum stack space is the 4 register arguments that we pop.  */
  જો (bytes < 4*4)
    bytes = 4*4;

  cif->bytes = bytes;
  cif->flags = flags;

  પાછા FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif * cif,
			  unsigned પૂર્ણાંક nfixedargs, unsigned પૂર્ણાંક ntotalargs)
{
  /* VFP variadic calls actually use the SYSV ABI */
  જો (cif->abi == FFI_VFP)
    cif->abi = FFI_SYSV;

  પાછા ffi_prep_cif_machdep (cif);
}

/* Prototypes for assembly functions, in sysv.S.  */

સંરચના call_frame
{
  શૂન્ય *fp;
  શૂન્ય *lr;
  શૂન્ય *rvalue;
  પૂર્ણાંક flags;
  શૂન્ય *closure;
};

બાહ્ય શૂન્ય ffi_call_SYSV (શૂન્ય *ઢગલો, સંરચના call_frame *,
			   શૂન્ય (*fn) (શૂન્ય)) FFI_HIDDEN;
બાહ્ય શૂન્ય ffi_call_VFP (શૂન્ય *vfp_space, સંરચના call_frame *,
			   શૂન્ય (*fn) (શૂન્ય), unsigned vfp_used) FFI_HIDDEN;

સ્થિર શૂન્ય
ffi_call_int (ffi_cif * cif, શૂન્ય (*fn) (શૂન્ય), શૂન્ય *rvalue,
	      શૂન્ય **avalue, શૂન્ય *closure)
{
  પૂર્ણાંક flags = cif->flags;
  ffi_type *rtype = cif->rtype;
  size_t bytes, rsize, vfp_size;
  અક્ષર *ઢગલો, *vfp_space, *new_rvalue;
  સંરચના call_frame *frame;

  rsize = 0;
  જો (rvalue == ખાલી)
    {
      /* If the return value is a struct and we don't have a return
	 value address then we need to make one.  Otherwise the return
	 value is in registers and we can ignore them.  */
      જો (flags == ARM_TYPE_STRUCT)
	rsize = rtype->size;
      નહીંતર
	flags = ARM_TYPE_VOID;
    }
  નહીંતર જો (flags == ARM_TYPE_VFP_N)
    {
      /* Largest case is double x 4. */
      rsize = 32;
    }
  નહીંતર જો (flags == ARM_TYPE_INT && rtype->type == FFI_TYPE_STRUCT)
    rsize = 4;

  /* Largest case.  */
  vfp_size = (cif->abi == FFI_VFP && cif->vfp_used ? 8*8: 0);

  bytes = cif->bytes;
  ઢગલો = alloca (vfp_size + bytes + કદ(સંરચના call_frame) + rsize);

  vfp_space = ખાલી;
  જો (vfp_size)
    {
      vfp_space = ઢગલો;
      ઢગલો += vfp_size;
    }

  frame = (સંરચના call_frame *)(ઢગલો + bytes);

  new_rvalue = rvalue;
  જો (rsize)
    new_rvalue = (શૂન્ય *)(frame + 1);

  frame->rvalue = new_rvalue;
  frame->flags = flags;
  frame->closure = closure;

  જો (vfp_space)
    {
      ffi_prep_args_VFP (cif, flags, new_rvalue, avalue, ઢગલો, vfp_space);
      ffi_call_VFP (vfp_space, frame, fn, cif->vfp_used);
    }
  નહીંતર
    {
      ffi_prep_args_SYSV (cif, flags, new_rvalue, avalue, ઢગલો);
      ffi_call_SYSV (ઢગલો, frame, fn);
    }

  જો (rvalue && rvalue != new_rvalue)
    memcpy (rvalue, new_rvalue, rtype->size);
}

શૂન્ય
ffi_call (ffi_cif *cif, શૂન્ય (*fn) (શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ખાલી);
}

#ifdef FFI_GO_CLOSURES
શૂન્ય
ffi_call_go (ffi_cif *cif, શૂન્ય (*fn) (શૂન્ય), શૂન્ય *rvalue,
	     શૂન્ય **avalue, શૂન્ય *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif

સ્થિર શૂન્ય *
ffi_prep_incoming_args_SYSV (ffi_cif *cif, શૂન્ય *rvalue,
			     અક્ષર *argp, શૂન્ય **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  પૂર્ણાંક i, n;

  જો (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(શૂન્ય **) argp;
      argp += 4;
    }
  નહીંતર
    {
      જો (cif->rtype->size && cif->rtype->size < 4)
        *(uint32_t *) rvalue = 0;
    }

  માટે (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      size_t z = ty->size;

      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      જો (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (શૂન્ય *) (argp + 4 - z);
      નહીંતર
#endif
      avalue[i] = (શૂન્ય *) argp;
      argp += z;
    }

  પાછા rvalue;
}

સ્થિર શૂન્ય *
ffi_prep_incoming_args_VFP (ffi_cif *cif, શૂન્ય *rvalue, અક્ષર *ઢગલો,
			    અક્ષર *vfp_space, શૂન્ય **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  પૂર્ણાંક i, n, vi = 0;
  અક્ષર *argp, *regp, *eo_regp;
  અક્ષર done_with_regs = 0;
  અક્ષર ઢગલો_used = 0;

  regp = ઢગલો;
  eo_regp = argp = regp + 16;

  જો (cif->flags == ARM_TYPE_STRUCT)
    {
      rvalue = *(શૂન્ય **) regp;
      regp += 4;
    }

  માટે (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = arg_types[i];
      પૂર્ણાંક is_vfp_type = vfp_type_p (ty);
      size_t z = ty->size;

      જો (vi < cif->vfp_nargs && is_vfp_type)
	{
	  avalue[i] = vfp_space + cif->vfp_args[vi++] * 4;
	  ચાલુ;
	}
      નહીંતર જો (!done_with_regs && !is_vfp_type)
	{
	  અક્ષર *tregp = ffi_align (ty, regp);

	  z = (z < 4) ? 4 : z;	// pad

	  /* If the arguments either fits into the registers or uses registers
	     and stack, while we haven't read other things from the stack */
	  જો (tregp + z <= eo_regp || !ઢગલો_used)
	    {
	      /* Because we're little endian, this is what it turns into.  */
#ifdef __ARMEB__
	      જો (z < 4 && ty->type != FFI_TYPE_STRUCT)
		avalue[i] = (શૂન્ય *) (tregp + 4 - z);
	      નહીંતર
#endif
	      avalue[i] = (શૂન્ય *) tregp;
	      regp = tregp + z;

	      /* If we read past the last core register, make sure we
		 have not read from the stack before and continue
		 reading after regp.  */
	      જો (regp > eo_regp)
		{
		  FFI_ASSERT (!ઢગલો_used);
		  argp = regp;
		}
	      જો (regp >= eo_regp)
		{
		  done_with_regs = 1;
		  ઢગલો_used = 1;
		}
	      ચાલુ;
	    }
	}

      ઢગલો_used = 1;
      argp = ffi_align (ty, argp);
#ifdef __ARMEB__
      જો (z < 4 && ty->type != FFI_TYPE_STRUCT)
	avalue[i] = (શૂન્ય *) (argp + 4 - z);
      નહીંતર
#endif
      avalue[i] = (શૂન્ય *) argp;
      argp += z;
    }

  પાછા rvalue;
}

#જો FFI_CLOSURES

સંરચના closure_frame
{
  અક્ષર vfp_space[8*8] __attribute__((aligned(8)));
  અક્ષર result[8*4];
  અક્ષર argp[];
};

પૂર્ણાંક FFI_HIDDEN
ffi_closure_inner_SYSV (ffi_cif *cif,
		        શૂન્ય (*fun) (ffi_cif *, શૂન્ય *, શૂન્ય **, શૂન્ય *),
		        શૂન્ય *user_data,
		        સંરચના closure_frame *frame)
{
  શૂન્ય **avalue = (શૂન્ય **) alloca (cif->nargs * કદ (શૂન્ય *));
  શૂન્ય *rvalue = ffi_prep_incoming_args_SYSV (cif, frame->result,
					      frame->argp, avalue);
  fun (cif, rvalue, avalue, user_data);
  પાછા cif->flags;
}

પૂર્ણાંક FFI_HIDDEN
ffi_closure_inner_VFP (ffi_cif *cif,
		       શૂન્ય (*fun) (ffi_cif *, શૂન્ય *, શૂન્ય **, શૂન્ય *),
		       શૂન્ય *user_data,
		       સંરચના closure_frame *frame)
{
  શૂન્ય **avalue = (શૂન્ય **) alloca (cif->nargs * કદ (શૂન્ય *));
  શૂન્ય *rvalue = ffi_prep_incoming_args_VFP (cif, frame->result, frame->argp,
					     frame->vfp_space, avalue);
  fun (cif, rvalue, avalue, user_data);
  પાછા cif->flags;
}

શૂન્ય ffi_closure_SYSV (શૂન્ય) FFI_HIDDEN;
શૂન્ય ffi_closure_VFP (શૂન્ય) FFI_HIDDEN;
#જો defined(FFI_EXEC_STATIC_TRAMP)
શૂન્ય ffi_closure_SYSV_alt (શૂન્ય) FFI_HIDDEN;
શૂન્ય ffi_closure_VFP_alt (શૂન્ય) FFI_HIDDEN;
#endif

#ifdef FFI_GO_CLOSURES
શૂન્ય ffi_go_closure_SYSV (શૂન્ય) FFI_HIDDEN;
શૂન્ય ffi_go_closure_VFP (શૂન્ય) FFI_HIDDEN;
#endif

/* the cif must already be prep'ed */

#જો defined(__FreeBSD__) && defined(__arm__)
#define __clear_cache(start, end) કરો { \
		સંરચના arm_sync_icache_args ua; 		\
								\
		ua.addr = (uintptr_t)(start);			\
		ua.len = (અક્ષર *)(end) - (અક્ષર *)start;		\
		sysarch(ARM_SYNC_ICACHE, &ua);			\
	} જ્યારે (0);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure * closure,
		      ffi_cif * cif,
		      શૂન્ય (*fun) (ffi_cif *, શૂન્ય *, શૂન્ય **, શૂન્ય *),
		      શૂન્ય *user_data, શૂન્ય *codeloc)
{
  શૂન્ય (*closure_func) (શૂન્ય) = ffi_closure_SYSV;

  જો (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      જો (cif->vfp_used)
	closure_func = ffi_closure_VFP;
    }
  નહીંતર જો (cif->abi != FFI_SYSV)
    પાછા FFI_BAD_ABI;

#જો FFI_EXEC_TRAMPOLINE_TABLE
  શૂન્ય **config = (શૂન્ય **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = closure_func;
#નહીંતર

# જો defined(FFI_EXEC_STATIC_TRAMP)
  જો (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      જો (closure_func == ffi_closure_SYSV)
        closure_func = ffi_closure_SYSV_alt;
      નહીંતર
        closure_func = ffi_closure_VFP_alt;
      ffi_tramp_set_parms (closure->ftramp, closure_func, closure);
      જાઓ out;
    }
# endif

  /* Initialize the dynamic trampoline. */
# ifndef _WIN32
  memcpy(closure->tramp, ffi_arm_trampoline, 8);
# નહીંતર
  // cast away function type so MSVC doesn't set the lower bit of the function pointer
  memcpy(closure->tramp, (શૂન્ય*)((uintptr_t)ffi_arm_trampoline & 0xFFFFFFFE), FFI_TRAMPOLINE_CLOSURE_OFFSET);
# endif

# જો defined(__QNX__)
  msync (closure->tramp, 8, MS_INVALIDATE_ICACHE);	/* clear data map */
  msync (codeloc, 8, MS_INVALIDATE_ICACHE);		/* clear insn map */
# elif defined(_WIN32)
  FlushInstructionCache(GetCurrentProcess(), closure->tramp, FFI_TRAMPOLINE_SIZE);
# નહીંતર
  __clear_cache(closure->tramp, closure->tramp + 8);	/* clear data map */
  __clear_cache(codeloc, codeloc + 8);			/* clear insn map */
# endif
# ifdef _WIN32
  *(શૂન્ય(**)(શૂન્ય))(closure->tramp + FFI_TRAMPOLINE_CLOSURE_FUNCTION) = closure_func;
# નહીંતર
  *(શૂન્ય (**)(શૂન્ય))(closure->tramp + 8) = closure_func;
# endif
# જો defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  પાછા FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     શૂન્ય (*fun) (ffi_cif *, શૂન્ય *, શૂન્ય **, શૂન્ય *))
{
  શૂન્ય (*closure_func) (શૂન્ય) = ffi_go_closure_SYSV;

  જો (cif->abi == FFI_VFP)
    {
      /* We only need take the vfp path if there are vfp arguments.  */
      જો (cif->vfp_used)
	closure_func = ffi_go_closure_VFP;
    }
  નહીંતર જો (cif->abi != FFI_SYSV)
    પાછા FFI_BAD_ABI;

  closure->tramp = closure_func;
  closure->cif = cif;
  closure->fun = fun;

  પાછા FFI_OK;
}
#endif

#endif /* FFI_CLOSURES */

/* Below are routines for VFP hard-float support. */

/* A subroutine of vfp_type_p.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

સ્થિર પૂર્ણાંક
is_hfa0 (અચલ ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  પૂર્ણાંક i, ret = -1;

  જો (elements != ખાલી)
    માટે (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        જો (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            જો (ret < 0)
              ચાલુ;
          }
        તોડો;
      }

  પાછા ret;
}

/* A subroutine of vfp_type_p.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

સ્થિર પૂર્ણાંક
is_hfa1 (અચલ ffi_type *ty, પૂર્ણાંક candidate)
{
  ffi_type **elements = ty->elements;
  પૂર્ણાંક i;

  જો (elements != ખાલી)
    માટે (i = 0; elements[i]; ++i)
      {
        પૂર્ણાંક t = elements[i]->type;
        જો (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
          {
            જો (!is_hfa1 (elements[i], candidate))
              પાછા 0;
          }
        નહીંતર જો (t != candidate)
          પાછા 0;
      }

  પાછા 1;
}

/* Determine if TY is an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is a floating point scalar.

   Returns non-zero iff TY is an HFA.  The result is an encoded value where
   bits 0-7 contain the type code, and bits 8-10 contain the element count.  */

સ્થિર પૂર્ણાંક
vfp_type_p (અચલ ffi_type *ty)
{
  ffi_type **elements;
  પૂર્ણાંક candidate, i;
  size_t size, ele_ગણતરી;

  /* Quickest tests first.  */
  candidate = ty->type;
  પસંદગી (ty->type)
    {
    મૂળભૂત:
      પાછા 0;
    સ્થિતિ FFI_TYPE_FLOAT:
    સ્થિતિ FFI_TYPE_DOUBLE:
      ele_ગણતરી = 1;
      જાઓ done;
    સ્થિતિ FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      જો (candidate != FFI_TYPE_FLOAT && candidate != FFI_TYPE_DOUBLE)
	પાછા 0;
      ele_ગણતરી = 2;
      જાઓ done;
    સ્થિતિ FFI_TYPE_STRUCT:
      તોડો;
    }

  /* No HFA types are smaller than 4 bytes, or larger than 32 bytes.  */
  size = ty->size;
  જો (size < 4 || size > 32)
    પાછા 0;

  /* Find the type of the first non-structure member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  જો (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_COMPLEX)
    {
      માટે (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          જો (candidate >= 0)
            તોડો;
        }
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  પસંદગી (candidate)
    {
    સ્થિતિ FFI_TYPE_FLOAT:
      ele_ગણતરી = size / કદ(દશાંશ);
      જો (size != ele_ગણતરી * કદ(દશાંશ))
        પાછા 0;
      તોડો;
    સ્થિતિ FFI_TYPE_DOUBLE:
      ele_ગણતરી = size / કદ(દ્વિગુણ);
      જો (size != ele_ગણતરી * કદ(દ્વિગુણ))
        પાછા 0;
      તોડો;
    મૂળભૂત:
      પાછા 0;
    }
  જો (ele_ગણતરી > 4)
    પાછા 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  માટે (i = 0; elements[i]; ++i)
    {
      પૂર્ણાંક t = elements[i]->type;
      જો (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          જો (!is_hfa1 (elements[i], candidate))
            પાછા 0;
        }
      નહીંતર જો (t != candidate)
        પાછા 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  પાછા (ele_ગણતરી << 8) | candidate;
}

સ્થિર પૂર્ણાંક
place_vfp_arg (ffi_cif *cif, પૂર્ણાંક h)
{
  unsigned short reg = cif->vfp_reg_free;
  પૂર્ણાંક align = 1, nregs = h >> 8;

  જો ((h & 0xff) == FFI_TYPE_DOUBLE)
    align = 2, nregs *= 2;

  /* Align register number. */
  જો ((reg & 1) && align == 2)
    reg++;

  જ્યારે (reg + nregs <= 16)
    {
      પૂર્ણાંક s, new_used = 0;
      માટે (s = reg; s < reg + nregs; s++)
	{
	  new_used |= (1 << s);
	  જો (cif->vfp_used & (1 << s))
	    {
	      reg += align;
	      જાઓ next_reg;
	    }
	}
      /* Found regs to allocate. */
      cif->vfp_used |= new_used;
      cif->vfp_args[cif->vfp_nargs++] = (signed અક્ષર)reg;

      /* Update vfp_reg_free. */
      જો (cif->vfp_used & (1 << cif->vfp_reg_free))
	{
	  reg += nregs;
	  જ્યારે (cif->vfp_used & (1 << reg))
	    reg += 1;
	  cif->vfp_reg_free = reg;
	}
      પાછા 0;
    next_reg:;
    }
  // done, mark all regs as used
  cif->vfp_reg_free = 16;
  cif->vfp_used = 0xFFFF;
  પાછા 1;
}

સ્થિર શૂન્ય
layout_vfp_args (ffi_cif * cif)
{
  unsigned પૂર્ણાંક i;
  /* Init VFP fields */
  cif->vfp_used = 0;
  cif->vfp_nargs = 0;
  cif->vfp_reg_free = 0;
  memset (cif->vfp_args, -1, 16);	/* Init to -1. */

  માટે (i = 0; i < cif->nargs; i++)
    {
      પૂર્ણાંક h = vfp_type_p (cif->arg_types[i]);
      જો (h && place_vfp_arg (cif, h) == 1)
	તોડો;
    }
}

#જો defined(FFI_EXEC_STATIC_TRAMP)
શૂન્ય *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  બાહ્ય શૂન્ય *trampoline_code_table;

  *tramp_size = ARM_TRAMP_SIZE;
  *map_size = ARM_TRAMP_MAP_SIZE;
  પાછા &trampoline_code_table;
}
#endif

#endif /* __arm__ or _M_ARM */
